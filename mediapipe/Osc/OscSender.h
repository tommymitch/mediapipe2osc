/*
  ==============================================================================

    OscSender.h
    Created: 15 May 2018 5:48:23pm
    Author:  Tom Mitchell

  ==============================================================================
*/

#pragma once

#include "../Osc/OscMessage.h"
#include "../Osc/OscBundle.h"
#include "../Osc/OscContent.h"
#include "UdpSocket.h"

class OscSender
{
public:
    
    /** Constructor */
    OscSender();
    
    /** Destructor */
    ~OscSender();
    
    /** Opens a connection to the speicifed hostname and port number */
    void connect (const std::string& sendHostname, unsigned short sendPort);
    
    /** Writes the provided OscMessage to the output socket */
    bool send (const OscMessage& message);
    
    /** Writes the provided OscMessage to the output socket */
    bool send (const OscMessage& message, const std::string& ipToSendTo, int portToSendTo);
    
    /** Writes the provided OscBundle to the output socket */
    bool send (const OscBundle& bundle);
    
    /** Writes the provided OscBundle to the output socket */
    bool send (const OscBundle& bundle, const std::string& ipToSendTo, int portToSendTo);
    
    /** Writes the provided OscContent to the output socket */
    bool send (const OscContent* content, const std::string& ipToSendTo, int portToSendTo);
    
    /** gets the current send port number */
    unsigned short getSendPort() const;
    
    /** gets the current send port number */
    const std::string& getSendHostname() const;
    
    /** sets the current send port number */
    void setSendPort (unsigned short newPortNumber);
    
    /** sets the current send port number */
    void setSendHostname (const std::string& newHostname);
    
private:
    unsigned short sendPort;     // port for outgoing messages
    std::string sendHostname; // name for remote host
    
    UdpSocket sendSocket;
};
