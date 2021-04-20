//
//  OscContent.cpp
//  OSC++
//
//  Created by Tom Mitchell on 25/04/2018.
//

#include <assert.h>
#include <vector>
#include <memory>

#include "OscContent.h"
#include "OscArgument.h"
#include "OscMessage.h"
#include "OscBundle.h"

static const char SlipEnd    = 0xC0;
static const char SlipEsc    = 0xDB;
static const char SlipEscEnd = 0xDC;
static const char SlipEscEsc = 0xDD;

OscContent::OscContent()
{
    
}

size_t OscContent::getSlipEncodedSize (const char* oscEncodedSource, size_t sourceSizeInBytes)
{
    size_t slipEncodedSize = 0;
    for (size_t oscDataIndex = 0; oscDataIndex < sourceSizeInBytes; oscDataIndex++)
    {
        switch (oscEncodedSource[oscDataIndex])
        {
            case SlipEnd:
            case SlipEsc: slipEncodedSize += 2; break;
            default: slipEncodedSize++; break;
        }
    }
    slipEncodedSize++;
    return slipEncodedSize;
}

size_t OscContent::slipEncode (const char* oscEncodedSource, size_t sourceSize, char* slipEncodedDestination)
{
    size_t slipEncodedIndex = 0;
    
    for (size_t oscDataIndex = 0; oscDataIndex < sourceSize; oscDataIndex++)
    {
        switch (oscEncodedSource[oscDataIndex])
        {
            case SlipEnd:
                slipEncodedDestination[slipEncodedIndex++] = SlipEsc;
                slipEncodedDestination[slipEncodedIndex++] = SlipEscEnd;
                break;
            case SlipEsc:
                slipEncodedDestination[slipEncodedIndex++] = SlipEsc;
                slipEncodedDestination[slipEncodedIndex++] = SlipEscEsc;
                break;
            default:
                slipEncodedDestination[slipEncodedIndex++] = oscEncodedSource[oscDataIndex];
        }
    }
    
    slipEncodedDestination[slipEncodedIndex++] = SlipEnd;
    return slipEncodedIndex;
}

size_t OscContent::getSlipDecodedSize (const char* slipEncodedSource, size_t sourceSize)
{
    size_t oscEncodedIndex = 0;
    
    // Return if last byte is not END byte
    if (slipEncodedSource[sourceSize - 1] != SlipEnd)
        return oscEncodedIndex;
    
    size_t sourceIndex = 0;
    while (sourceIndex < sourceSize && slipEncodedSource[sourceIndex] != SlipEnd)
    {
        if (slipEncodedSource[sourceIndex] == SlipEsc)
        {
            switch (slipEncodedSource[++sourceIndex])
            {
                case SlipEscEnd:
                case SlipEscEsc: oscEncodedIndex++; break;
                default:
                    return oscEncodedIndex = 0; // error: unexpected byte value
            }
        }
        else
        {
            oscEncodedIndex++;
        }
        sourceIndex++;
    }
    
    return oscEncodedIndex;
}

size_t OscContent::slipDecode (char* slipEncodedSource, size_t sourceSize, char* oscEncodedDestination)
{
    size_t oscEncodedIndex = 0;
    
    // Return if last byte is not END byte
    if (slipEncodedSource[sourceSize - 1] != SlipEnd)
        return oscEncodedIndex;
    
    size_t sourceIndex = 0;
    while (sourceIndex < sourceSize && slipEncodedSource[sourceIndex] != SlipEnd)
    {
        if (slipEncodedSource[sourceIndex] == SlipEsc)
        {
            switch (slipEncodedSource[++sourceIndex])
            {
                case SlipEscEnd:
                    oscEncodedDestination[oscEncodedIndex++] = SlipEnd;
                    break;
                case SlipEscEsc:
                    oscEncodedDestination[oscEncodedIndex++] = SlipEsc;
                    break;
                default:
                    return oscEncodedIndex = 0; // error: unexpected byte value
            }
        }
        else
        {
            oscEncodedDestination[oscEncodedIndex++] = slipEncodedSource[sourceIndex];
        }
        sourceIndex++;
    }
    
    return oscEncodedIndex;
}

bool OscContent::isSlipEndCharacter (char character)
{
    return character == SlipEnd;
}

OscMessage& OscContent::getAsMessage()
{
    assert (isMessage());
    return *dynamic_cast<OscMessage*> (this);
}

OscBundle& OscContent::getAsBundle()
{
    assert (isBundle());
    return *dynamic_cast<OscBundle*> (this);
}

const OscMessage& OscContent::getAsMessage() const
{
    assert (isMessage());
    return *dynamic_cast<const OscMessage*> (this);
}

const OscBundle& OscContent::getAsBundle() const
{
    assert (isBundle());
    return *dynamic_cast<const OscBundle*> (this);
}

