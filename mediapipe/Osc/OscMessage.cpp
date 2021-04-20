//
//  OscMessage.cpp
//  OSC++
//

#include "OscMessage.h"
#include <algorithm>
#include <iostream>
#include <assert.h>

const size_t OscMessage::MinOscMessageSize = sizeof ("/\0\0\0,\0\0");// just a /

//strnlen is non standard!
size_t stringLengthSafe (const char* source, size_t maxSize)
{
    const char *p = (const char *)memchr (source, 0, maxSize);
    return (p ? p - source : maxSize);
}

OscMessage::OscMessage()
{
    
}

OscMessage::OscMessage (const char* addressPattern)
{
    setAddressPattern (addressPattern);
}

bool OscMessage::isEmpty() const
{
    return addressPattern.empty() && arguments.size() == 0;
}

bool OscMessage::isValid() const
{
    return ! isEmpty() && addressPattern.length() >= sizeof ('/')  && addressPattern[0] == '/';
}

void OscMessage::clear()
{
    addressPattern.clear();
    arguments.clear();
}

const std::string& OscMessage::getAddressPattern() const
{
    return addressPattern;
}

OscError OscMessage::setAddressPattern (const char* addressPattern)
{
    return appendAddressPattern (addressPattern);
}

OscError OscMessage::appendAddressPattern (const char* appendedParts)
{
    if (*appendedParts != '/')
        return OscErrorNoSlashAtStartOfMessage;
    
    addressPattern.append (appendedParts);
    return OscErrorNone;
}

bool OscMessage::hasAddressPattern (const std::string& addressPatternToMatch) const
{
    return strcmp (addressPattern.c_str(), addressPatternToMatch.c_str()) == 0;
}

void OscMessage::addInt32 (int32_t value)
{
    arguments.push_back (OscArgument (value));
}

void OscMessage::addFloat32 (float value)
{
    arguments.push_back (OscArgument (value));
}

void OscMessage::addString (const std::string& stringValue)
{
    arguments.push_back (stringValue);
}

void OscMessage::addBlob (const char* source, size_t size)
{
    arguments.push_back (OscArgument (source, size));
}

void OscMessage::addInt64 (int64_t value)
{
    arguments.push_back (OscArgument (value));
}

void OscMessage::addTimeTag (OscTimeTag value)
{
    arguments.push_back (OscArgument (value));
}

void OscMessage::addFloat64 (Float64 value)
{
    arguments.push_back (OscArgument (value));
}

void OscMessage::addAlternateString (const std::string& alternateStringValue)
{
    OscArgument argument;
    argument.setAlternateString (alternateStringValue); //can't use string constructor
    arguments.push_back (argument);
}

void OscMessage::addCharacter (char asciiChar)
{
    arguments.push_back (OscArgument (asciiChar));
}

void OscMessage::addRgbaColour (RgbaColour rgbaColour)
{
    arguments.push_back (OscArgument (rgbaColour));
}

void OscMessage::addMidiMessage (MidiMessageData midiMessage)
{
    arguments.push_back (OscArgument (midiMessage));
}

void OscMessage::addBool (bool boolean)
{
    arguments.push_back (OscArgument (boolean));
}

void OscMessage::addNil()
{
    OscArgument argument;
    argument.setNil();
    arguments.push_back (argument);
}

void OscMessage::addInfinitum()
{
    OscArgument argument;
    argument.setInfinitum();
    arguments.push_back (argument);
}

void OscMessage::addBeginArray()
{
    OscArgument argument;
    argument.setBeginArray();
    arguments.push_back (argument);
}

void OscMessage::addEndArray()
{
    OscArgument argument;
    argument.setEndArray();
    arguments.push_back (argument);
}

OscMessage OscMessage::createFromEncodedData (const char* source, size_t sizeInBytes)
{
    OscMessage m;
    m.decode (source, sizeInBytes);
    return m;
}

bool OscMessage::operator== (const OscMessage& other) const
{
    if (addressPattern != other.getAddressPattern())
        return false;
    
    if (getNumberOfArguments() != other.getNumberOfArguments())
        return false;

    for (int i = 0; i < getNumberOfArguments(); i++)
        if ((*this)[i] != other[i])
            return false;
    
    return true;
}

bool OscMessage::operator!= (const OscMessage& other) const
{
    if (addressPattern != other.getAddressPattern())
        return true;
    
    if (getNumberOfArguments() != other.getNumberOfArguments())
        return true;
    
    for (int i = 0; i < getNumberOfArguments(); i++)
        if ((*this)[i] != other[i])
            return true;
    
    return false;
}

size_t OscMessage::getEncodedSize() const
{
    size_t messageSize = 0;
    messageSize +=  getPaddedSize (addressPattern.size() + sizeof ('\0'));
    messageSize += getPaddedSize (arguments.size() + sizeof (',') + sizeof ('\0')); // include comma and null character
    messageSize += getEncodedArgumentsSize();
    return messageSize;
}

size_t OscMessage::encode (char* destination, size_t destinationSize) const
{
    size_t destinationIndex = 0;
    
    // Address pattern
    if (addressPattern.size() > destinationSize)
        return destinationIndex;
    
    for (auto& c : addressPattern)
        destination[destinationIndex++] = c;
    
    if (terminateOscString (destination, destinationIndex, destinationSize) != 0)
        return destinationIndex;
    
    // Type tag string
    if (getPaddedSize (destinationIndex + arguments.size() + sizeof (',') + sizeof ('\0')) > destinationSize)
        return destinationIndex;
    
    destination[destinationIndex++] = ',';
    
    for (int i = 0; i < arguments.size(); i++)
        destination[destinationIndex++] = arguments[i].getType();
    
    if (terminateOscString (destination, destinationIndex, destinationSize) != false)
        return destinationIndex;
    
    // Arguments
    if ((destinationIndex + getEncodedArgumentsSize()) > destinationSize)
        return destinationIndex;
    
    for (auto argument : arguments)
    {
        size_t bytesWritten = argument.encode (&destination[destinationIndex], destinationSize - destinationIndex);
        if (bytesWritten != argument.getEncodedSize())
            return destinationIndex;
        destinationIndex += bytesWritten;
    }
    
    return destinationIndex;
}

OscError OscMessage::decode (const char* source, const size_t sizeInBytes)
{
    if (sizeInBytes == 0)
        return OscErrorMessageToShort;
    
    if ((sizeInBytes % 4) != 0)
        return OscErrorSizeIsNotMultipleOfFour;
    
    if (sizeInBytes < MinOscMessageSize)
        return OscErrorMessageSizeTooSmall;

    if (source[0] != '/')
        return OscErrorNoSlashAtStartOfMessage;
    
    // Read address pattern
    unsigned int sourceIndex = 0;
    {//scope
        const size_t addressLength = stringLengthSafe (source, sizeInBytes);
        
        if (addressLength > sizeInBytes)
            return OscErrorAddressPatternUnterminated;
        
        addressPattern.reserve (addressLength);
        while (source[sourceIndex] != '\0')
        {
            addressPattern.push_back (source[sourceIndex]);
            if (++sourceIndex >= sizeInBytes)
                return error (OscErrorSourceEndsBeforeEndOfAddressPattern);
        }
    }
    
    // Advance to type tag string
    while (source[sourceIndex - 1] != ',')
    { // skip index past comma
        if (++sourceIndex >= sizeInBytes)
            return error (OscErrorSourceEndsBeforeStartOfTypeTagString);
    }
    
    // Read type tag string
    std::string typetagString;
    {//scope
        size_t typeTagLength = stringLengthSafe (&source[sourceIndex], sizeInBytes - sourceIndex);
        typetagString.reserve (typeTagLength);
        while (source[sourceIndex] != '\0')
        {
            typetagString += source[sourceIndex];            
            if (++sourceIndex >= sizeInBytes)
                return error (OscErrorSourceEndsBeforeEndOfTypeTagString);
        }
    }
    
    // Advance to arguments
    do
    {
        if (++sourceIndex > sizeInBytes)
            return error (OscErrorUnexpectedEndOfSource);
    }
    while (sourceIndex % 4 != 0);
    
    for (auto type : typetagString)
    {
        OscArgument argument;
        switch (type)
        {
            case OscArgument::Int32TypeTag:
            case OscArgument::Float32TypeTag:
            case OscArgument::CharacterTypeTag:
            case OscArgument::RgbaColourTypeTag:
            case OscArgument::MidiMessageTypeTag:
            {
                if (sourceIndex + sizeof (OscArgument32) > sizeInBytes)
                    return error (OscErrorUnexpectedEndOfSource);
                
                OscArgument32 argument32 (OscArgument::decodeArgument32 (&source[sourceIndex]));
                switch (type)
                {
                    case OscArgument::Int32TypeTag:       argument.setInt32 (argument32.int32);             break;
                    case OscArgument::Float32TypeTag:     argument.setFloat32 (argument32.float32);         break;
                    case OscArgument::CharacterTypeTag:   argument.setCharacter (source[sourceIndex + 3]);  break;
                    case OscArgument::RgbaColourTypeTag:  argument.setRgbaColour (argument32.rgbaColour);   break;
                    case OscArgument::MidiMessageTypeTag: argument.setMidiMessage (argument32.midiMessage); break;
                    default: assert (false); break;
                }
                break;
            }
            case OscArgument::StringTypeTag:
            case OscArgument::AlternateStringTypeTag:
            {
                //safely check the stringlen
                size_t length = stringLengthSafe (&source[sourceIndex], sizeInBytes - sourceIndex);

                if (length == 0 || getPaddedSize (sourceIndex + length + sizeof ('\0')) > sizeInBytes)
                    return error (OscErrorUnexpectedEndOfSource);
                
                std::string string;
                string.reserve (length);
                for (int i = 0; source[sourceIndex + i] != '\0';)
                    string += source[sourceIndex + i++];
                
                if (type == OscArgument::StringTypeTag)
                    argument.setString (string);
                else
                    argument.setAlternateString (string);
                break;
            }
            case OscArgument::BlobTypeTag:
            {
                //check sizeCount is present
                if (sourceIndex + sizeof (int32_t) > sizeInBytes)
                    return error (OscErrorUnexpectedEndOfSource);
                
                const int32_t sizeCount = OscArgument::decodeArgument32 (&source[sourceIndex]).int32;
            
                //check entire blob is present
                if (sourceIndex + sizeof (sizeCount) + getPaddedSize (sizeCount) > sizeInBytes)
                    return error (OscErrorUnexpectedEndOfSource);
                
                argument.setBlob (&source[sourceIndex + sizeof (OscArgument32)], sizeCount);
                break;
            }
            case OscArgument::Int64TypeTag:
            case OscArgument::Float64TypeTag:
            case OscArgument::TimeTagTypeTag:
            {
                if (sourceIndex + sizeof (OscArgument64) > sizeInBytes)
                    return error (OscErrorUnexpectedEndOfSource);
                
                OscArgument64 argument64 (OscArgument::decodeArgument64 (&source[sourceIndex]));
                switch (type)
                {
                    case OscArgument::Int64TypeTag:   argument.setInt64 (argument64.int64);        break;
                    case OscArgument::Float64TypeTag: argument.setFloat64 (argument64.float64);    break;
                    case OscArgument::TimeTagTypeTag: argument.setTimeTag (argument64.oscTimeTag); break;
                    default: assert (false); break;
                }
                break;
            }
            //arguments with typeTag data only
            case OscArgument::TrueTypeTag:        argument.setBool (true);    break;
            case OscArgument::FalseTypeTag:       argument.setBool (false);   break;
            case OscArgument::NilTypeTag:         argument.setNil();          break;
            case OscArgument::InfinitumTypeTag:   argument.setInfinitum();    break;
            case OscArgument::BeginArrayTypeTag:  argument.setBeginArray();   break;
            case OscArgument::EndArrayTypeTag:    argument.setEndArray();     break;
            
            default: std::cout << "Unrecognised Type Tag!" << std::endl; break;
        }
        
        arguments.push_back (argument);
        sourceIndex += argument.getEncodedSize();
    }
    
    return OscErrorNone;
}

const OscArgument::TypeTag OscMessage::getArgumentType (int argumentIndex) const
{
    assert (argumentIndex < arguments.size());
    return (OscArgument::TypeTag)arguments[argumentIndex].getType();
}

size_t OscMessage::getEncodedArgumentsSize() const
{
    size_t totalSize = 0;
    
    for (auto a : arguments)
        totalSize += a.getEncodedSize();
    
    return totalSize;
}

OscError OscMessage::error (OscError error)
{
    clear();
    return error;
}
