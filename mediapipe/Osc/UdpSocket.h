//
//  UdpSocket.hpp
//  OscppTest
//
//  Created by Tom Mitchell on 16/04/2021.
//  Copyright © 2021 Tom Mitchell. All rights reserved.
//

#pragma once

#include <string>
#include <mutex>

class  UdpSocket
{
public:
    //==============================================================================
    /** Creates a datagram socket.

        You first need to bind this socket to a port with bindToPort if you intend to read
        from this socket.

        If enableBroadcasting is true, the socket will be allowed to send broadcast messages
        (may require extra privileges on linux)
    */
    UdpSocket (bool enableBroadcasting = false);


    /** Destructor. */
    ~UdpSocket();

    //==============================================================================
    /** Binds the socket to the specified local port.

        The localPortNumber is the port on which to bind this socket. If this value is 0,
        the port number is assigned by the operating system.

        @returns  true on success; false may indicate that another socket is already bound
                  on the same port
    */
    bool bindToPort (int localPortNumber);

    /** Binds the socket to the specified local port and local address.

        If localAddress is not an empty string then the socket will be bound to localAddress
        as well. This is useful if you would like to bind your socket to a specific network
        adapter. Note that localAddress must be an IP address assigned to one of your
        network address otherwise this function will fail.

        @returns  true on success; false may indicate that another socket is already bound
                  on the same port
        @see bindToPort(int localPortNumber), IPAddress::getAllAddresses
    */
    bool bindToPort (int localPortNumber, const std::string& localAddress);

    /** Returns the local port number to which this socket is currently bound.

        This is useful if you need to know to which port the OS has actually bound your
        socket when bindToPort was called with zero.

        @returns  -1 if the socket didn't bind to any port yet or an error occurred
    */
    int getBoundPort() const noexcept;

    /** Returns the OS's socket handle that's currently open. */
    int getRawSocketHandle() const noexcept                     { return handle; }

    //==============================================================================
    /** Waits until the socket is ready for reading or writing.

        If readyForReading is true, it will wait until the socket is ready for
        reading; if false, it will wait until it's ready for writing.

        If the timeout is < 0, it will wait forever, or else will give up after
        the specified time.

        @returns  1 if the socket is ready on return, 0 if it times-out before the
                  socket becomes ready, or -1 if an error occurs
    */
    int waitUntilReady (bool readyForReading, int timeoutMsecs);

    /** Reads bytes from the socket.

        If blockUntilSpecifiedAmountHasArrived is true, the method will block until
        maxBytesToRead bytes have been read, (or until an error occurs). If this
        flag is false, the method will return as much data as is currently available
        without blocking.

        @returns  the number of bytes read, or -1 if there was an error
        @see waitUntilReady
    */
    int read (void* destBuffer, int maxBytesToRead,
              bool blockUntilSpecifiedAmountHasArrived);

    /** Reads bytes from the socket and return the IP address of the sender.

        If blockUntilSpecifiedAmountHasArrived is true, the method will block until
        maxBytesToRead bytes have been read, (or until an error occurs). If this
        flag is false, the method will return as much data as is currently available
        without blocking.

        @returns  the number of bytes read, or -1 if there was an error. On a successful
                  result, the senderIPAddress value will be set to the IP of the sender
        @see waitUntilReady
    */
    int read (void* destBuffer, int maxBytesToRead,
              bool blockUntilSpecifiedAmountHasArrived,
              std::string& senderIPAddress, int& senderPortNumber);

    /** Writes bytes to the socket from a buffer.

        Note that this method will block unless you have checked the socket is ready
        for writing before calling it (see the waitUntilReady() method).

        @returns  the number of bytes written, or -1 if there was an error
    */
    int write (const std::string& remoteHostname, int remotePortNumber,
               const void* sourceBuffer, int numBytesToWrite);

    /** Closes the underlying socket object.

        Closes the underlying socket object and aborts any read or write operations.
        Note that all other methods will return an error after this call and the object
        cannot be re-used.

        This method is useful if another thread is blocking in a read/write call and you
        would like to abort the read/write thread. Simply deleting the socket
        object without calling shutdown may cause a race-condition where the read/write
        returns just before the socket is deleted and the subsequent read/write would
        try to read from an invalid pointer. By calling shutdown first, the socket
        object remains valid but all current and subsequent calls to read/write will
        return immediately.
    */
    void shutdown();

    //==============================================================================
    /** Join a multicast group.

        @returns  true if it succeeds
    */
    bool joinMulticast (const std::string& multicastIPAddress);

    /** Leave a multicast group.

        @returns  true if it succeeds
    */
    bool leaveMulticast (const std::string& multicastIPAddress);

    /** Enables or disables multicast loopback.

        @returns  true if it succeeds
    */
    bool setMulticastLoopbackEnabled (bool enableLoopback);

    //==============================================================================
    /** Allow other applications to re-use the port.

        Allow any other application currently running to bind to the same port.
        Do not use this if your socket handles sensitive data as it could be
        read by any, possibly malicious, third-party apps.

        @returns  true on success
    */
    bool setEnablePortReuse (bool enabled);

private:
    //==============================================================================
    std::atomic<int> handle { -1 };
    bool isBound = false;
    std::string lastBindAddress, lastServerHost;
    int lastServerPort = -1;
    void* lastServerAddress = nullptr;
    mutable std::mutex readLock;

};
