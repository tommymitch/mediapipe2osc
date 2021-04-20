//
//  OscArgument.cpp
//  OSC++
//
//  Created by Adam STARK on 14/02/2018.
//  Copyright © 2018 Adam Stark. All rights reserved.
//

#include "OscArgument.h"
#include <assert.h>
#include <vector>
#include <sstream>
#include <iomanip>

OscArgument::OscArgument()
{

}

OscArgument::OscArgument (const OscArgument& other)
{
    typeTag = other.typeTag;
    switch (typeTag)
    {
        case Int32TypeTag:
        case Float32TypeTag:
        case Int64TypeTag:
        case Float64TypeTag:
        case TimeTagTypeTag:
        case CharacterTypeTag:
        case RgbaColourTypeTag:
        case MidiMessageTypeTag:
            value = other.value;
            break;
        
        case StringTypeTag:
        case AlternateStringTypeTag:
            stringValue = other.stringValue;
            break;
            
        case BlobTypeTag:
            blobValue = other.blobValue;
            break;
            
        default: break;
    }
}

char OscArgument::getType() const
{
    return typeTag;
}

size_t OscArgument::getEncodedSize() const
{
    size_t encodedSize = 0;
    switch (typeTag)
    {
        case Int32TypeTag:
        case Float32TypeTag:
        case CharacterTypeTag:
        case RgbaColourTypeTag:
        case MidiMessageTypeTag:
            encodedSize = sizeof (OscArgument32);
            break;
        
        case StringTypeTag:
        case AlternateStringTypeTag:
            encodedSize = getPaddedSize (stringValue.length() + sizeof ('\0'));
            break;
            
        case BlobTypeTag:
            encodedSize = sizeof (OscArgument32) + getPaddedSize (blobValue.size());
            break;
        
        case Int64TypeTag:
        case Float64TypeTag:
        case TimeTagTypeTag:
            encodedSize = sizeof (OscArgument64);
            break;
        
        case TrueTypeTag:
        case FalseTypeTag:
        case NilTypeTag:
        case InfinitumTypeTag:
        case BeginArrayTypeTag:
        case EndArrayTypeTag:
            //do nothing
            break;
        
        default: //this should never happen
            assert (false);
            break;
    }
    return encodedSize;
}

size_t OscArgument::encode (char* destination, size_t destinationSize) const
{
    size_t encodedSize = getEncodedSize();
    if (encodedSize > destinationSize) //make sure the destination is big enough
        return 0;
    
    switch (typeTag)
    {
        case Int32TypeTag:
        case Float32TypeTag:
        case RgbaColourTypeTag:
        case MidiMessageTypeTag:
        {
            encodeArgument32 (value.argument32, destination);
            break;
        }
            
        case Int64TypeTag:
        case Float64TypeTag:
        case TimeTagTypeTag:
        {
            encodeArgument64 (value.argument64, destination);
            break;
        }
            
        // ------------------------------------------------------
        case CharacterTypeTag:
        {
            destination[0] = 0;
            destination[1] = 0;
            destination[2] = 0;
            destination[3] = value.charValue;
            break;
        }
          
        // ------------------------------------------------------
        case StringTypeTag:
        case AlternateStringTypeTag:
        {
            for (int i = 0; i < getEncodedSize(); i++)
            {
                if (i < stringValue.length())
                    destination[i] = stringValue[i];
                else
                    destination[i] = '\0';
            }
            break;
        }
            
        // ------------------------------------------------------
        case BlobTypeTag:
        {
            int index = 0;
            OscArgument32 blobSize ((int32_t)blobValue.size());
            destination[index++] = blobSize.byteStruct.byte3;
            destination[index++] = blobSize.byteStruct.byte2;
            destination[index++] = blobSize.byteStruct.byte1;
            destination[index++] = blobSize.byteStruct.byte0;
            
            for (char c : blobValue)
            {
                if (index >= getEncodedSize())
                    return 0;
                destination[index++] = c;
            }
            
            while (index % 4 != 0)
                destination[index++] = 0;
            break;
        }
           
        // ------------------------------------------------------
        default:
            break;
    }
    
    return getEncodedSize();
}

std::string OscArgument::asString() const
{
    std::string argumentString;
    
    switch (typeTag)
    {
        case Int32TypeTag:
        {
            argumentString = std::to_string (getInt32());
            break;
        }
        case Float32TypeTag:
        {
            argumentString = std::to_string (getFloat32());
            break;
        }
        case StringTypeTag:
        {
            argumentString = getString();
            break;
        }
        case BlobTypeTag:
        {
            argumentString = "blob[";
            for (auto c : blobValue)
            {
                std::ostringstream stream;
                stream << "0x" << std::setfill ('0') << std::setw (sizeof (c) * 2) << std::hex << (int)c << ",";
                argumentString.append (stream.str());
            }
            argumentString.back() = ']'; //replace trailing comma with square bracket
            break;
        }
        case Int64TypeTag:
        {
            argumentString = std::to_string (getInt64());
            break;
        }
        case Float64TypeTag:
        {
            argumentString = std::to_string (getFloat64());
            break;
        }
        case TimeTagTypeTag:
        {
//#warning Convert to human readable date when time permits
            argumentString = std::to_string (getTimeTag().value);
            break;
        }
        case AlternateStringTypeTag:
        {
            argumentString = getAlternateString();
            break;
        }
        case CharacterTypeTag:
        {
            std::ostringstream stream;
            stream << "'" << getCharacter() << "'";
            argumentString = stream.str();
            break;
        }
        case RgbaColourTypeTag:
        {
            argumentString.append ("rgba[" + std::to_string (getRgbaColour().red)   + ",");
            argumentString.append (std::to_string (getRgbaColour().green) + ",");
            argumentString.append (std::to_string (getRgbaColour().blue)  + ",");
            argumentString.append (std::to_string (getRgbaColour().alpha) + "]");
            break;
        }
        case MidiMessageTypeTag:
        {
            argumentString.append ("midi[portID=" + std::to_string ((int)getMidiMessage().portID)   + ",");
            argumentString.append ("status=" + std::to_string ((int)getMidiMessage().status) + ",");
            argumentString.append ("data1=" + std::to_string ((int)getMidiMessage().data1)  + ",");
            argumentString.append ("data2=" + std::to_string ((int)getMidiMessage().data2)  + "]");
            break;
        }
        case TrueTypeTag:
        case FalseTypeTag:
        {
            argumentString = getBool() ? "true" : "false";
            break;
        }

        case NilTypeTag:
        {
            argumentString = "Nil";
            break;
        }
        case InfinitumTypeTag:
        {
            argumentString = "inf";
            break;
        }
        case BeginArrayTypeTag:
        case EndArrayTypeTag:
        {
            argumentString = typeTag;
            break;
        }

        default:
            break;
    }
    
    return argumentString;
}

void OscArgument::setInt32 (int32_t newInt)
{
    typeTag = Int32TypeTag;
    value.int32Value = newInt;
}

bool OscArgument::isInt32() const
{
    return typeTag == Int32TypeTag;
}

int32_t OscArgument::getInt32() const
{
    // You are trying to get a int32 value from an argument that is not an int32
    assert (typeTag == Int32TypeTag);
    return value.int32Value;
}

void OscArgument::setFloat32 (float newFloat32)
{
    typeTag = Float32TypeTag;
    value.float32Value = newFloat32;
}

bool OscArgument::isFloat32() const
{
    return typeTag == Float32TypeTag;
}

float OscArgument::getFloat32() const
{
    // You are trying to get a float32 value from an argument that is not a float
    assert (typeTag == Float32TypeTag);
    return value.float32Value;
}

void OscArgument::setString (const std::string& newString)
{
    typeTag = StringTypeTag;
    stringValue = newString;
}

bool OscArgument::isString() const
{
    return typeTag == StringTypeTag;
}

const std::string& OscArgument::getString() const
{
    // this isn't a string
    assert (typeTag == StringTypeTag || typeTag == AlternateStringTypeTag);
    return stringValue;
}

void OscArgument::setBlob (const char* bytes, size_t numberOfBytes)
{
    typeTag = BlobTypeTag;
    
    unsigned int sourceIndex;
    for (sourceIndex = 0; sourceIndex < numberOfBytes; sourceIndex++)
        blobValue.push_back (bytes[sourceIndex]);
}

bool OscArgument::isBlob() const
{
    return typeTag == BlobTypeTag;
}

size_t OscArgument::getBlobSize() const
{
    assert (typeTag == BlobTypeTag);
    return blobValue.size();
}

void OscArgument::getBlobData (char* destination) const
{
    assert (typeTag == BlobTypeTag);
    
    const size_t blobDataSize = getBlobSize();
    for (int i = 0; i < blobDataSize; i++)
        destination[i] = blobValue[i];
}

const std::vector<char>& OscArgument::getBlob() const
{
    assert (typeTag == BlobTypeTag);
    return blobValue;
}

void OscArgument::setInt64 (int64_t newInt64)
{
    typeTag = Int64TypeTag;
    value.int64Value = newInt64;
}

bool OscArgument::isInt64() const
{
    return typeTag == Int64TypeTag;
}

int64_t OscArgument::getInt64() const
{
    // You are trying to get a int64 value from an argument that is not an int64
    assert (typeTag == Int64TypeTag);
    return value.int64Value;
}

void OscArgument::setFloat64 (Float64 newFloat64)
{
    typeTag = Float64TypeTag;
    value.float64Value = newFloat64;
}

bool OscArgument::isFloat64() const
{
    return typeTag == Float64TypeTag;
}

Float64 OscArgument::getFloat64() const
{
    // You are trying to get a double value from an argument that is not a double
    assert (typeTag == Float64TypeTag);
    return value.float64Value;
}

void OscArgument::setTimeTag (const OscTimeTag newTimeTag)
{
    typeTag = TimeTagTypeTag;
    value.timeTagValue = newTimeTag;
}

bool OscArgument::isTimeTag() const
{
    return typeTag == TimeTagTypeTag;
}

OscTimeTag OscArgument::getTimeTag() const
{
    // You are trying to get a time tag from an argument that is not a time tag
    assert (typeTag == TimeTagTypeTag);
    return value.timeTagValue;
}

void OscArgument::setAlternateString (const std::string& newString)
{
    typeTag = AlternateStringTypeTag;
    stringValue = newString;
}

void OscArgument::setAlternateString (const char* bytes)
{
    typeTag = AlternateStringTypeTag;
    
    stringValue = "";
    
    int i = 0;
    while (bytes[i] != '\0')
    {
        stringValue += bytes[i];
        i++;
    }
}

bool OscArgument::isAlternateString() const
{
    return typeTag == AlternateStringTypeTag;
}

const std::string& OscArgument::getAlternateString() const
{
    assert (typeTag == AlternateStringTypeTag);
    return getString();
}

void OscArgument::setCharacter (char newCharacter)
{
    typeTag = CharacterTypeTag;
    value.charValue = newCharacter;
}

bool OscArgument::isCharacter() const
{
    return typeTag == CharacterTypeTag;
}

char OscArgument::getCharacter() const
{
    // You are trying to get a time char from an argument that is not a char
    assert (typeTag == CharacterTypeTag);
    return value.charValue;
}

void OscArgument::setRgbaColour (RgbaColour newRgbaColour)
{
    typeTag = RgbaColourTypeTag;
    value.colourValue.red   = newRgbaColour.red;
    value.colourValue.green = newRgbaColour.green;
    value.colourValue.blue  = newRgbaColour.blue;
    value.colourValue.alpha = newRgbaColour.alpha;
}

void OscArgument::setRgbaColour (unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    setRgbaColour ({r, g, b, a});
}

bool OscArgument::isRgbaColour() const
{
    return typeTag == RgbaColourTypeTag;
}

RgbaColour OscArgument::getRgbaColour() const
{
    // You are trying to get a colour from an argument that is not a colour
    assert (typeTag == RgbaColourTypeTag);
    return value.colourValue;
}

void OscArgument::setMidiMessage (MidiMessageData newMidiMessage)
{
    typeTag = MidiMessageTypeTag;
    value.midiMessageValue = newMidiMessage;
}

void OscArgument::setMidiMessage (unsigned char p, unsigned char s,
                                  unsigned char d1, unsigned char d2)
{
    setMidiMessage ({p, s, d1, d2});
}

bool OscArgument::isMidiMessage() const
{
    return typeTag == MidiMessageTypeTag;
}

MidiMessageData OscArgument::getMidiMessage() const
{
    // You are trying to get a Midi message from an argument that is not a Midi message
    assert (typeTag == MidiMessageTypeTag);
    return value.midiMessageValue;
}

void OscArgument::setBool (bool newBool)
{
    typeTag = newBool ? TrueTypeTag : FalseTypeTag;
}

bool OscArgument::isBool() const
{
    return typeTag == TrueTypeTag || typeTag == FalseTypeTag;
}

bool OscArgument::getBool() const
{
    assert (typeTag == TrueTypeTag || FalseTypeTag);
    return typeTag == TrueTypeTag;
}

void OscArgument::setNil()
{
    typeTag = NilTypeTag;
}

bool OscArgument::isNil() const
{
    return typeTag == NilTypeTag;
}

void OscArgument::setInfinitum()
{
    typeTag = InfinitumTypeTag;
}

bool OscArgument::isInfinitum() const
{
    return typeTag == InfinitumTypeTag;
}

void OscArgument::setBeginArray()
{
    typeTag = BeginArrayTypeTag;
}

bool OscArgument::isBeginArray() const
{
    return typeTag == BeginArrayTypeTag;
}

void OscArgument::setEndArray()
{
    typeTag = EndArrayTypeTag;
}

bool OscArgument::isEndArray() const
{
    return typeTag == EndArrayTypeTag;
}

bool OscArgument::operator== (const OscArgument& other) const
{
    if (getType() != other.getType())
        return false;
    
    switch (getType())
    {
        case Int32TypeTag:           return getInt32() == other.getInt32();
        case Float32TypeTag:         return getFloat32() == other.getFloat32();
        case StringTypeTag:          return getString() == other.getString();
        case BlobTypeTag:            return getBlob() == other.getBlob();
        case Int64TypeTag:           return getInt64() == other.getInt64();
        case Float64TypeTag:         return getFloat64() == other.getFloat64();
        case TimeTagTypeTag:         return getTimeTag() == other.getTimeTag();
        case AlternateStringTypeTag: return getAlternateString() == other.getAlternateString();
        case CharacterTypeTag:       return getCharacter() == other.getCharacter();
        case RgbaColourTypeTag:      return getRgbaColour() == other.getRgbaColour();
        case MidiMessageTypeTag:     return getMidiMessage() == other.getMidiMessage();
            
        case TrueTypeTag:
        case FalseTypeTag:
        case NilTypeTag:
        case InfinitumTypeTag:
        case BeginArrayTypeTag:
        case EndArrayTypeTag:
            return true;
            
        default: return false;
    }
}

bool OscArgument::operator!= (const OscArgument& other) const
{
    if (getType() != other.getType())
        return true;
    
    switch (getType())
    {
        case Int32TypeTag:           return getInt32() != other.getInt32();
        case Float32TypeTag:         return getFloat32() != other.getFloat32();
        case StringTypeTag:          return getString() != other.getString();
        case BlobTypeTag:            return getBlob() != other.getBlob();
        case Int64TypeTag:           return getInt64() != other.getInt64();
        case Float64TypeTag:         return getFloat64() != other.getFloat64();
        case TimeTagTypeTag:         return getTimeTag() != other.getTimeTag();
        case AlternateStringTypeTag: return getAlternateString() != other.getAlternateString();
        case CharacterTypeTag:       return getCharacter() != other.getCharacter();
        case RgbaColourTypeTag:      return getRgbaColour() != other.getRgbaColour();
        case MidiMessageTypeTag:     return getMidiMessage() != other.getMidiMessage();
            
        case TrueTypeTag:
        case FalseTypeTag:
        case NilTypeTag:
        case InfinitumTypeTag:
        case BeginArrayTypeTag:
        case EndArrayTypeTag:
            return false;
            
        default: return false;
    }
}
