//
//  UdpSocket.cpp
//  OscppTest
//
//  Created by Tom Mitchell on 16/04/2021.
//  Copyright © 2021 Tom Mitchell. All rights reserved.
//

#include "UdpSocket.h"

#include <atomic>


//Apple
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#if defined(_WIN64)
 using juce_socklen_t       = int;
 using juce_recvsend_size_t = int;
 using SocketHandle         = SOCKET;
 static const SocketHandle invalidSocket = INVALID_SOCKET;
#else
using juce_socklen_t       = unsigned int;  //socklen_t; //apparently this is in the SystemConfiguration framework.
 using juce_recvsend_size_t = unsigned int; //socklen_t;
 using SocketHandle         = int;
 static const SocketHandle invalidSocket = -1;
#endif

//==============================================================================
namespace SocketHelpers
{
    static void initSockets()
    {
       #if defined(_WIN64)
        static bool socketsStarted = false;

        if (! socketsStarted)
        {
            socketsStarted = true;

            WSADATA wsaData;
            const WORD wVersionRequested = MAKEWORD (1, 1);
            WSAStartup (wVersionRequested, &wsaData);
        }
       #endif
    }

    inline bool isValidPortNumber (int port) noexcept
    {
        return port > 0 && port < 65536;
    }

    template <typename Type>
    static bool setOption (SocketHandle handle, int mode, int property, Type value) noexcept
    {
        return setsockopt (handle, mode, property, reinterpret_cast<const char*> (&value), sizeof (value)) == 0;
    }

    template <typename Type>
    static bool setOption (SocketHandle handle, int property, Type value) noexcept
    {
        return setOption (handle, SOL_SOCKET, property, value);
    }

    static bool resetSocketOptions (SocketHandle handle, bool isDatagram, bool allowBroadcast) noexcept
    {
        return handle != invalidSocket
                && setOption (handle, SO_RCVBUF, (int) 65536)
                && setOption (handle, SO_SNDBUF, (int) 65536)
                && (isDatagram ? ((! allowBroadcast) || setOption (handle, SO_BROADCAST, (int) 1))
                               : setOption (handle, IPPROTO_TCP, TCP_NODELAY, (int) 1));
    }

    static void closeSocket (std::atomic<int>& handle, std::mutex& readLock,
                             bool isListener, int portNumber, std::atomic<bool>& connected) noexcept
    {
        const auto h = (SocketHandle) handle.load();
        handle = -1;

       #if JUCE_WINDOWS
        ignoreUnused (portNumber, isListener, readLock);

        if (h != invalidSocket || connected)
            closesocket (h);

        // make sure any read process finishes before we delete the socket
        std::mutex::ScopedLockType lock (readLock);
        connected = false;
       #else
        if (connected)
        {
            connected = false;

            if (isListener)
            {
                // need to do this to interrupt the accept() function..
                //*********************************
                //StreamingSocket temp;
                //temp.connect (IPAddress::local().tostd::string(), portNumber, 1000);
            }
        }

        if (h >= 0)
        {
            // unblock any pending read requests
            ::shutdown (h, SHUT_RDWR);

            ::close (h);
            std::lock_guard<std::mutex> lg (readLock);
        }
       #endif
    }

    static bool bindSocket (SocketHandle handle, int port, const std::string& address) noexcept
    {
        if (handle == invalidSocket || ! isValidPortNumber (port))
            return false;

        struct sockaddr_in addr;
        //zerostruct (addr); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)
        memset ((void*) &addr, 0, sizeof (addr));
        
        addr.sin_family = PF_INET;
        addr.sin_port = htons ((unsigned short) port);
        addr.sin_addr.s_addr = ! address.empty() ? ::inet_addr (address.c_str())
                                                    : htonl (INADDR_ANY);

        return ::bind (handle, (struct sockaddr*) &addr, sizeof (addr)) >= 0;
    }

    static int getBoundPort (SocketHandle handle) noexcept
    {
        if (handle != invalidSocket)
        {
            struct sockaddr_in addr;
            socklen_t len = sizeof (addr);

            if (getsockname (handle, (struct sockaddr*) &addr, &len) == 0)
                return ntohs (addr.sin_port);
        }

        return -1;
    }

    static std::string getConnectedAddress (SocketHandle handle) noexcept
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof (addr);

        if (getpeername (handle, (struct sockaddr*) &addr, &len) >= 0)
            return inet_ntoa (addr.sin_addr);

        return "0.0.0.0";
    }

    static bool setSocketBlockingState (SocketHandle handle, bool shouldBlock) noexcept
    {
       #if defined(_WIN64)
        u_long nonBlocking = shouldBlock ? 0 : (u_long) 1;
        return ioctlsocket (handle, (long) FIONBIO, &nonBlocking) == 0;
       #else
        int socketFlags = fcntl (handle, F_GETFL, 0);

        if (socketFlags == -1)
            return false;

        if (shouldBlock)
            socketFlags &= ~O_NONBLOCK;
        else
            socketFlags |= O_NONBLOCK;

        return fcntl (handle, F_SETFL, socketFlags) == 0;
       #endif
    }

   #if ! defined(_WIN64)
    static bool getSocketBlockingState (SocketHandle handle)
    {
        return (fcntl (handle, F_GETFL, 0) & O_NONBLOCK) == 0;
    }
   #endif

    static int readSocket (SocketHandle handle,
                           void* destBuffer, int maxBytesToRead,
                           std::atomic<bool>& connected,
                           bool blockUntilSpecifiedAmountHasArrived,
                           std::mutex& readLock,
                           std::string* senderIP = nullptr,
                           int* senderPort = nullptr) noexcept
    {
       #if ! JUCE_WINDOWS
        if (blockUntilSpecifiedAmountHasArrived != getSocketBlockingState (handle))
       #endif
            setSocketBlockingState (handle, blockUntilSpecifiedAmountHasArrived);

        int bytesRead = 0;

        while (bytesRead < maxBytesToRead)
        {
            long bytesThisTime = -1;
            auto buffer = static_cast<char*> (destBuffer) + bytesRead;
            auto numToRead = (juce_recvsend_size_t) (maxBytesToRead - bytesRead);

            {
                // avoid race-condition
                std::unique_lock<std::mutex> lock (readLock, std::try_to_lock);
                
                if(!lock.owns_lock())
                {
                    if (senderIP == nullptr || senderPort == nullptr)
                    {
                        bytesThisTime = ::recv (handle, buffer, numToRead, 0);
                    }
                    else
                    {
                        sockaddr_in client;
                        socklen_t clientLen = sizeof (sockaddr);

                        bytesThisTime = ::recvfrom (handle, buffer, numToRead, 0, (sockaddr*) &client, &clientLen);

                        std::string temp;
                        *senderIP = temp.append (inet_ntoa (client.sin_addr), 16);
                        *senderPort = ntohs (client.sin_port);
                    }
                }
            }

            if (bytesThisTime <= 0 || ! connected)
            {
                if (bytesRead == 0 && blockUntilSpecifiedAmountHasArrived)
                    bytesRead = -1;

                break;
            }

            bytesRead = static_cast<int> (bytesRead + bytesThisTime);

            if (! blockUntilSpecifiedAmountHasArrived)
                break;
        }

        return (int) bytesRead;
    }

    static int waitForReadiness (std::atomic<int>& handle, std::mutex& readLock,
                                 bool forReading, int timeoutMsecs) noexcept
    {
        // avoid race-condition
        std::unique_lock<std::mutex> lock (readLock, std::try_to_lock);
        
        if(!lock.owns_lock())
            return -1;

        auto hasErrorOccurred = [&handle]() -> bool
        {
            auto h = (SocketHandle) handle.load();

            if (h == invalidSocket)
                return true;

            int opt;
            juce_socklen_t len = sizeof (opt);

            if (getsockopt (h, SOL_SOCKET, SO_ERROR, (char*) &opt, &len) < 0  || opt != 0)
                return true;

            return false;
        };

        auto h = handle.load();

       #if defined(_WIN64)
        struct timeval timeout;
        struct timeval* timeoutp;

        if (timeoutMsecs >= 0)
        {
            timeout.tv_sec = timeoutMsecs / 1000;
            timeout.tv_usec = (timeoutMsecs % 1000) * 1000;
            timeoutp = &timeout;
        }
        else
        {
            timeoutp = nullptr;
        }

        fd_set rset, wset;
        FD_ZERO (&rset);
        FD_SET ((SOCKET) h, &rset);
        FD_ZERO (&wset);
        FD_SET ((SOCKET) h, &wset);

        fd_set* prset = forReading ? &rset : nullptr;
        fd_set* pwset = forReading ? nullptr : &wset;

        // NB - need to use select() here as WSAPoll is broken on Windows
        if (select ((int) h + 1, prset, pwset, nullptr, timeoutp) < 0 || hasErrorOccurred())
            return -1;

        return FD_ISSET (h, forReading ? &rset : &wset) ? 1 : 0;
      #else
        short eventsFlag = (forReading ? POLLIN : POLLOUT);
        pollfd pfd { (SocketHandle) h, eventsFlag, 0 };

        int result = 0;

        for (;;)
        {
            result = poll (&pfd, 1, timeoutMsecs);

            if (result >= 0 || errno != EINTR)
                break;
        }

        if (result < 0 || hasErrorOccurred())
            return -1;

        return (pfd.revents & eventsFlag) != 0;
      #endif
    }

    static addrinfo* getAddressInfo (bool isDatagram, const std::string& hostName, int portNumber)
    {
        struct addrinfo hints;
        memset ((void*) &hints, 0, sizeof (hints));

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = isDatagram ? SOCK_DGRAM : SOCK_STREAM;
        hints.ai_flags = AI_NUMERICSERV;

        struct addrinfo* info = nullptr;

        if (getaddrinfo (hostName.c_str(), std::to_string(portNumber).c_str(), &hints, &info) == 0)
            return info;

        return nullptr;
    }

    static bool connectSocket (std::atomic<int>& handle,
                               std::mutex& readLock,
                               const std::string& hostName,
                               int portNumber,
                               int timeOutMillisecs) noexcept
    {
        bool success = false;

        if (auto* info = getAddressInfo (false, hostName, portNumber))
        {
            for (auto* i = info; i != nullptr; i = i->ai_next)
            {
                auto newHandle = socket (i->ai_family, i->ai_socktype, 0);

                if (newHandle != invalidSocket)
                {
                    setSocketBlockingState (newHandle, false);
                    auto result = ::connect (newHandle, i->ai_addr, (socklen_t) i->ai_addrlen);
                    success = (result >= 0);

                    if (! success)
                    {
                       #if JUCE_WINDOWS
                        if (result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
                       #else
                        if (errno == EINPROGRESS)
                       #endif
                        {
                            std::atomic<int> cvHandle { (int) newHandle };

                            if (waitForReadiness (cvHandle, readLock, false, timeOutMillisecs) == 1)
                                success = true;
                        }
                    }

                    if (success)
                    {
                        handle = (int) newHandle;
                        break;
                    }

                   #if JUCE_WINDOWS
                    closesocket (newHandle);
                   #else
                    ::close (newHandle);
                   #endif
                }
            }

            freeaddrinfo (info);

            if (success)
            {
                auto h = (SocketHandle) handle.load();
                setSocketBlockingState (h, true);
                resetSocketOptions (h, false, false);
            }
        }

        return success;
    }

    static void makeReusable (int handle) noexcept
    {
        setOption ((SocketHandle) handle, SO_REUSEADDR, (int) 1);
    }

    static bool multicast (int handle, const std::string& multicastIPAddress,
                           const std::string& interfaceIPAddress, bool join) noexcept
    {
        struct ip_mreq mreq;

        memset ((void*) &mreq, 0, sizeof (mreq));
        mreq.imr_multiaddr.s_addr = inet_addr (multicastIPAddress.c_str());
        mreq.imr_interface.s_addr = INADDR_ANY;

        if (! interfaceIPAddress.empty())
            mreq.imr_interface.s_addr = inet_addr (interfaceIPAddress.c_str());

        return setsockopt ((SocketHandle) handle, IPPROTO_IP,
                           join ? IP_ADD_MEMBERSHIP
                                : IP_DROP_MEMBERSHIP,
                           (const char*) &mreq, sizeof (mreq)) == 0;
    }
}


//==============================================================================
//==============================================================================
UdpSocket::UdpSocket (bool canBroadcast)
{
    SocketHelpers::initSockets();

    handle = (int) socket (AF_INET, SOCK_DGRAM, 0);

    if (handle >= 0)
    {
        SocketHelpers::resetSocketOptions ((SocketHandle) handle.load(), true, canBroadcast);
        SocketHelpers::makeReusable (handle);
    }
}

UdpSocket::~UdpSocket()
{
    if (lastServerAddress != nullptr)
        freeaddrinfo (static_cast<struct addrinfo*> (lastServerAddress));

    shutdown();
}

void UdpSocket::shutdown()
{
    if (handle < 0)
        return;

    std::atomic<int> handleCopy { handle.load() };
    handle = -1;

    std::atomic<bool> connected { false };
    SocketHelpers::closeSocket (handleCopy, readLock, false, 0, connected);

    isBound = false;
}

bool UdpSocket::bindToPort (int port)
{
    return bindToPort (port, std::string());
}

bool UdpSocket::bindToPort (int port, const std::string& addr)
{
    assert (SocketHelpers::isValidPortNumber (port));

    if (handle < 0)
        return false;

    if (SocketHelpers::bindSocket ((SocketHandle) handle.load(), port, addr))
    {
        isBound = true;
        lastBindAddress = addr;
        return true;
    }

    return false;
}

int UdpSocket::getBoundPort() const noexcept
{
    return (handle >= 0 && isBound) ? SocketHelpers::getBoundPort ((SocketHandle) handle.load()) : -1;
}

//==============================================================================
int UdpSocket::waitUntilReady (bool readyForReading, int timeoutMsecs)
{
    if (handle < 0)
        return -1;

    return SocketHelpers::waitForReadiness (handle, readLock, readyForReading, timeoutMsecs);
}

int UdpSocket::read (void* destBuffer, int maxBytesToRead, bool shouldBlock)
{
    if (handle < 0 || ! isBound)
        return -1;

    std::atomic<bool> connected { true };
    return SocketHelpers::readSocket ((SocketHandle) handle.load(), destBuffer, maxBytesToRead,
                                      connected, shouldBlock, readLock);
}

int UdpSocket::read (void* destBuffer, int maxBytesToRead, bool shouldBlock, std::string& senderIPAddress, int& senderPort)
{
    if (handle < 0 || ! isBound)
        return -1;

    std::atomic<bool> connected { true };
    return SocketHelpers::readSocket ((SocketHandle) handle.load(), destBuffer, maxBytesToRead, connected,
                                      shouldBlock, readLock, &senderIPAddress, &senderPort);
}

int UdpSocket::write (const std::string& remoteHostname, int remotePortNumber,
                           const void* sourceBuffer, int numBytesToWrite)
{
    assert (SocketHelpers::isValidPortNumber (remotePortNumber));

    if (handle < 0)
        return -1;

    struct addrinfo*& info = reinterpret_cast<struct addrinfo*&> (lastServerAddress);

    // getaddrinfo can be quite slow so cache the result of the address lookup
    if (info == nullptr || remoteHostname != lastServerHost || remotePortNumber != lastServerPort)
    {
        if (info != nullptr)
            freeaddrinfo (info);

        if ((info = SocketHelpers::getAddressInfo (true, remoteHostname, remotePortNumber)) == nullptr)
            return -1;

        lastServerHost = remoteHostname;
        lastServerPort = remotePortNumber;
    }

    return (int) ::sendto ((SocketHandle) handle.load(), (const char*) sourceBuffer,
                           (juce_recvsend_size_t) numBytesToWrite, 0,
                           info->ai_addr, (socklen_t) info->ai_addrlen);
}

bool UdpSocket::joinMulticast (const std::string& multicastIPAddress)
{
    if (handle < 0 || ! isBound)
        return false;

    return SocketHelpers::multicast (handle, multicastIPAddress, lastBindAddress, true);
}

bool UdpSocket::leaveMulticast (const std::string& multicastIPAddress)
{
    if (handle < 0 || ! isBound)
        return false;

    return SocketHelpers::multicast (handle, multicastIPAddress, lastBindAddress, false);
}

bool UdpSocket::setMulticastLoopbackEnabled (bool enable)
{
    if (handle < 0 || ! isBound)
        return false;

    return SocketHelpers::setOption<bool> ((SocketHandle) handle.load(), IPPROTO_IP, IP_MULTICAST_LOOP, enable);
}

bool UdpSocket::setEnablePortReuse (bool enabled)
{
   #if JUCE_ANDROID
    ignoreUnused (enabled);
   #else
    if (handle >= 0)
        return SocketHelpers::setOption ((SocketHandle) handle.load(),
                                        #if JUCE_WINDOWS || JUCE_LINUX
                                         SO_REUSEADDR,  // port re-use is implied by addr re-use on these platforms
                                        #else
                                         SO_REUSEPORT,
                                        #endif
                                         (int) (enabled ? 1 : 0));
   #endif

    return false;
}
