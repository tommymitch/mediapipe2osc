//
//  OscArgument.h
//  OSC++
//
//  Created by Tom Mitchell on 14/02/2018.
//  Copyright © 2018 Tom Mitchell. All rights reserved.
//

#ifndef OscArgument_h
#define OscArgument_h

#include <array>
#include <string>
#include <vector>
#include "OscCommon.h"

// Definitions - 32-bit argument types
/**
 * @brief 32-bit RGBA colour.
 * @see http://en.wikipedia.org/wiki/RGBA_color_space
 */
#pragma pack(push,1)
struct RgbaColour
{
    RgbaColour() {red = green = blue = alpha = 0;}
    RgbaColour (unsigned char r, unsigned char g, unsigned char b, unsigned char a)
        {red = r; green = g; blue = b; alpha = a;}
    
    bool operator== (const RgbaColour& other)  const {return red   == other.red
                                                          && green == other.green
                                                          && blue  == other.blue
                                                          && alpha == other.alpha;}
    bool operator!= (const RgbaColour& other)  const {return red   != other.red
                                                          || green != other.green
                                                          || blue  != other.blue
                                                          || alpha != other.alpha;}
    
#ifdef LITTLE_ENDIAN_PLATFORM
    unsigned char alpha; // LSB
    unsigned char blue;
    unsigned char green;
    unsigned char red; // MSB
#else
    unsigned char red; // MSB
    unsigned char green;
    unsigned char blue;
    unsigned char alpha; // LSB
#endif
};
#pragma pack(pop)
/**
 * @brief 4 byte MIDI message as described in OSC 1.0 specification.
 */
#pragma pack(push,1)
struct MidiMessageData
{
    MidiMessageData() {portID = status = data1 = data2 = 0;}
    MidiMessageData (unsigned char p, unsigned char s, unsigned char d1, unsigned char d2)
        {portID = p; status = s; data1 = d1; data2 = d2;}
    
    bool operator== (const MidiMessageData& other)  const {return portID == other.portID
                                                               && status == other.status
                                                               && data1  == other.data1
                                                               && data2 == other.data2;}
    bool operator!= (const MidiMessageData& other)  const {return portID   != other.portID
                                                               || status != other.status
                                                               || data1  != other.data1
                                                               || data2 != other.data2;}

#ifdef LITTLE_ENDIAN_PLATFORM
    unsigned char data2; // LSB
    unsigned char data1;
    unsigned char status;
    unsigned char portID; // MSB
#else
    unsigned char portID; // MSB
    unsigned char status;
    unsigned char data1;
    unsigned char data2; // LSB
#endif
};
#pragma pack(pop)
/**
 * @brief Union of all 32-bit OSC argument types defined in OSC 1.0
 * specification.
 */
union OscArgument32
{
    OscArgument32() {int32 = 0;}
    OscArgument32 (int32_t value)           {int32 = value;}
    OscArgument32 (float value)             {float32 = value;}
    OscArgument32 (RgbaColour value)        {rgbaColour = value;}
    OscArgument32 (MidiMessageData value)   {midiMessage = value;}
    
    int32_t int32;
    float float32;
    RgbaColour rgbaColour;
    MidiMessageData midiMessage;
    
#pragma pack(push,1)
    struct
    {
#ifdef LITTLE_ENDIAN_PLATFORM
        char byte0; // LSB
        char byte1;
        char byte2;
        char byte3; // MSB
#else
        char byte3; // MSB
        char byte2;
        char byte1;
        char byte0; // LSB
#endif
    }byteStruct;
#pragma pack(pop)
};

// Definitions - 64-bit argument types
/**
 * @brief OSC time-tag.  Same representation used by NTP timestamps.
 */
union OscTimeTag
{
    OscTimeTag() {value = 0;}
    OscTimeTag (const OscTimeTag& other) {*this = other;}
    OscTimeTag (uint64_t t) {value = t;}
    OscTimeTag (uint32_t seconds, uint32_t fraction) {dwordStruct.seconds = seconds; dwordStruct.fraction = fraction;}
    bool operator== (const OscTimeTag& other)  const {return value == other.value;}
    bool operator!= (const OscTimeTag& other)  const {return value != other.value;}
    
    uint64_t value;
#pragma pack(push,1)
    struct 
    {
        uint32_t fraction;
        uint32_t seconds;
    }dwordStruct;
#pragma pack(pop)  

#pragma pack(push,1)
    struct 
    {
#ifdef LITTLE_ENDIAN_PLATFORM
        char byte0; // LSB
        char byte1;
        char byte2;
        char byte3;
        char byte4;
        char byte5;
        char byte6;
        char byte7; // MSB
#else
        char byte7; // MSB
        char byte6;
        char byte5;
        char byte4;
        char byte3;
        char byte2;
        char byte1;
        char byte0; // LSB
#endif
    }byteStruct;
#pragma pack(pop)
};

/**
 * @brief Union of all 64-bit OSC argument types defined in OSC 1.0
 * specification.
 */
union OscArgument64
{
    OscArgument64() {int64 = 0;}
    OscArgument64 (const OscArgument64& other) {*this = other;}
    OscArgument64 (int64_t i)    {int64 = i;}
    OscArgument64 (OscTimeTag t) {oscTimeTag = t;}
    OscArgument64 (Float64 f)    {float64 = f;}
    
    int64_t int64;
    OscTimeTag oscTimeTag;
    Float64 float64;
    
#pragma pack(push,1)
    struct
    {
#ifdef LITTLE_ENDIAN_PLATFORM
        char byte0; // LSB
        char byte1;
        char byte2;
        char byte3;
        char byte4;
        char byte5;
        char byte6;
        char byte7; // MSB
#else
        char byte7; // MSB
        char byte6;
        char byte5;
        char byte4;
        char byte3;
        char byte2;
        char byte1;
        char byte0; // LSB
#endif
    }
    byteStruct;
#pragma pack(pop)
};
/**
 * Class for OSC arguments
 * 
 * Contains a tagged union with accessor and mutators for all OSC types along with some codec helper
 * functions
 */

class OscArgument
{
public:
    /**
     * @brief OSC type tag string characters indicating argument type.
     */
    
    enum TypeTag
    {
        Int32TypeTag = 'i',
        Float32TypeTag = 'f',
        StringTypeTag = 's',
        BlobTypeTag = 'b',
        Int64TypeTag = 'h',
        Float64TypeTag = 'd',
        TimeTagTypeTag = 't',
        AlternateStringTypeTag = 'S',
        CharacterTypeTag = 'c',
        RgbaColourTypeTag = 'r',
        MidiMessageTypeTag = 'm',
        TrueTypeTag = 'T',
        FalseTypeTag = 'F',
        NilTypeTag = 'N',
        InfinitumTypeTag = 'I',
        BeginArrayTypeTag = '[',
        EndArrayTypeTag = ']',
    };
    
    OscArgument();
    OscArgument (int32_t value)                     {setInt32 (value);}
    OscArgument (float value)                       {setFloat32 (value);}
    OscArgument (const std::string& value)          {setString (value);}
    OscArgument (const char* bytes, size_t size)    {setBlob (bytes, size);}
    OscArgument (int64_t value)                     {setInt64 (value);}
    OscArgument (Float64 value)                     {setFloat64 (value);}
    OscArgument (OscTimeTag value)                  {setTimeTag (value);}
    OscArgument (char value)                        {setCharacter (value);}
    OscArgument (RgbaColour value)                  {setRgbaColour (value);}
    OscArgument (MidiMessageData value)             {setMidiMessage (value);}
    OscArgument (bool value)                        {setBool (value);}
    
    OscArgument (const OscArgument& other);
    
    char getType() const;
    size_t getEncodedSize() const;
    size_t encode (char* destination, size_t destinationSize) const;
    std::string asString() const;
    
    void setInt32 (int32_t newInt32);
    bool isInt32() const;
    int32_t getInt32() const;
    
    void setFloat32 (float newFloat32);
    bool isFloat32() const;
    float getFloat32() const;
    
    void setString (const std::string& newString);
    bool isString() const;
    const std::string& getString() const;
    
    void setBlob (const char* bytes, size_t numberOfBytes);
    bool isBlob() const;
    size_t getBlobSize() const;
    void getBlobData (char* destination) const;
    const std::vector<char>& getBlob() const;
    
    void setInt64 (int64_t newInt64);
    bool isInt64() const;
    int64_t getInt64() const;
    
    void setFloat64 (Float64 newFloat64);
    bool isFloat64() const;
    Float64 getFloat64() const;

    void setTimeTag (OscTimeTag newTimeTag);
    bool isTimeTag() const;
    OscTimeTag getTimeTag() const;
    
    void setAlternateString (const std::string& newString);
    void setAlternateString (const char* bytes);
    bool isAlternateString() const;
    const std::string& getAlternateString() const;

    void setCharacter (char newCharacter);
    bool isCharacter() const;
    char getCharacter() const;

    void setRgbaColour (RgbaColour newRgbaColour);
    void setRgbaColour (unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    bool isRgbaColour() const;
    RgbaColour getRgbaColour() const;

    void setMidiMessage (MidiMessageData newMidiMessageData);
    void setMidiMessage (unsigned char p, unsigned char s, unsigned char d1, unsigned char d2);
    bool isMidiMessage() const;
    MidiMessageData getMidiMessage() const;

    void setBool (bool newBool);
    bool isBool() const;
    bool getBool() const;
    
    void setNil();
    bool isNil() const;
    
    void setInfinitum();
    bool isInfinitum() const;
    
    void setBeginArray();
    bool isBeginArray() const;
    
    void setEndArray();
    bool isEndArray() const;
    
    static inline OscArgument32 decodeArgument32 (const char* source)
    {
        OscArgument32 argument32;
        argument32.byteStruct.byte3 = source[0];
        argument32.byteStruct.byte2 = source[1];
        argument32.byteStruct.byte1 = source[2];
        argument32.byteStruct.byte0 = source[3];
        return argument32;
    }
    
    static inline OscArgument64 decodeArgument64 (const char* source)
    {
        OscArgument64 argument64;
        argument64.byteStruct.byte7 = source[0];
        argument64.byteStruct.byte6 = source[1];
        argument64.byteStruct.byte5 = source[2];
        argument64.byteStruct.byte4 = source[3];
        argument64.byteStruct.byte3 = source[4];
        argument64.byteStruct.byte2 = source[5];
        argument64.byteStruct.byte1 = source[6];
        argument64.byteStruct.byte0 = source[7];
        return argument64;
    }
    
    static inline size_t encodeArgument32 (const OscArgument32 argument, char* destination)
    {
        destination[0] = argument.byteStruct.byte3;
        destination[1] = argument.byteStruct.byte2;
        destination[2] = argument.byteStruct.byte1;
        destination[3] = argument.byteStruct.byte0;
        return sizeof (OscArgument32);
    }
    
    static inline size_t encodeArgument64 (const OscArgument64 argument, char* destination)
    {
        destination[0] = argument.byteStruct.byte7;
        destination[1] = argument.byteStruct.byte6;
        destination[2] = argument.byteStruct.byte5;
        destination[3] = argument.byteStruct.byte4;
        destination[4] = argument.byteStruct.byte3;
        destination[5] = argument.byteStruct.byte2;
        destination[6] = argument.byteStruct.byte1;
        destination[7] = argument.byteStruct.byte0;
        return sizeof (OscArgument64);
    }
    
    bool operator== (const OscArgument& other) const;
    bool operator!= (const OscArgument& other) const;
private:
    char typeTag;
    std::string stringValue;
    std::vector<char> blobValue;
    union ArgumentValue
    {
        ArgumentValue() {}
        ArgumentValue (int32_t value)                         {int32Value = value;}
        ArgumentValue (float value)                           {float32Value = value;}
        ArgumentValue (int64_t value)                         {int64Value = value;}
        ArgumentValue (Float64 value)                         {float64Value = value;}
        ArgumentValue (OscTimeTag value)                      {timeTagValue = value;}
        ArgumentValue (char value)                            {charValue = value;}
        ArgumentValue (RgbaColour value)                      {colourValue = value;}
        ArgumentValue (MidiMessageData value)                 {midiMessageValue = value;}
        
        char charValue;
        int32_t int32Value;
        float float32Value;
        RgbaColour colourValue;
        MidiMessageData midiMessageValue;
        int64_t int64Value;
        Float64 float64Value;
        OscTimeTag timeTagValue;
        OscArgument32 argument32;
        OscArgument64 argument64;
    } value;
};

#endif /* OscArgument_h */
