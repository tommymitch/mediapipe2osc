/*
  ==============================================================================

    OscSender.cpp
    Created: 15 May 2018 5:48:23pm
    Author:  Tom Mitchell

  ==============================================================================
*/

#include "OscSender.h"

OscSender::OscSender() : sendSocket (true)
{

}

void OscSender::connect (const std::string& sendHostname, unsigned short sendPort)
{
    setSendHostname (sendHostname);
    setSendPort (sendPort);
}

OscSender::~OscSender()
{

}


bool OscSender::send (const OscMessage& message)
{
    return send (&message, sendHostname, sendPort);
}

bool OscSender::send (const OscMessage& message, const std::string& ipToSendTo, int portToSendTo)
{
    return send (&message, ipToSendTo, portToSendTo);
}

bool OscSender::send (const OscBundle& bundle)
{
    return send (&bundle, sendHostname, sendPort);
}

bool OscSender::send (const OscBundle& bundle, const std::string& ipToSendTo, int portToSendTo)
{
    return send (&bundle, ipToSendTo, portToSendTo);
}

bool OscSender::send (const OscContent* content, const std::string& ipToSendTo, int portToSendTo)
{
    if (content == nullptr)
        return false;
    
    int encodedDataSize = static_cast<int> (content->getEncodedSize());
    std::vector<char> encodedData (encodedDataSize);

    content->encode (encodedData.data(), encodedDataSize);
    
    if (encodedDataSize != sendSocket.write (ipToSendTo, portToSendTo, encodedData.data(), encodedDataSize))
        return false;
    
    return true;
}

unsigned short OscSender::getSendPort() const
{
    return sendPort;
}


void OscSender::setSendPort (unsigned short newPortNumber)
{
    sendPort = newPortNumber;
}


const std::string& OscSender::getSendHostname() const
{
    return sendHostname;
}


void OscSender::setSendHostname (const std::string& newHostname)
{
    sendHostname = newHostname;
}
