//
//  OscMessage.h
//  OSC++
//
//  Created by Tom Mitchell on 14/02/2018.
//  Copyright © 2018 Tom Mitchell. All rights reserved.
//

#ifndef OscMessage_h
#define OscMessage_h

#include "OscContent.h"
#include "OscCommon.h"
#include "OscError.h"
#include "OscArgument.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <assert.h>
#include <iostream>

class OscMessage : public OscContent
{
public:
    
    static const size_t MinOscMessageSize;
    
    /**
     * @brief Constructor - creates a new empty OscMessage
     */
    OscMessage();
    
    /**
     * @brief Constructor - creates a new OscMessage with a given address pattern
     *
     * A message may be initialised without an address pattern by parsing an
     * oscAddressPattern value of "".  This may be of use if the address pattern is
     * undetermined at the time of initialisation.  In which case, the address
     * pattern may be set later using setAddressPattern().
     *
     * @param oscAddressPattern OSC address pattern as null terminated string.
     */
    OscMessage (const char* oscAddressPattern);
    
    /**
     * @brief Destructor
     */
    ~OscMessage() {}
    
    /**
     * @brief Constructor - creates a new OscMessage with a given address pattern and
     * a variable number of arguments
     *
     * A message may be initialised without an address pattern by parsing an
     * oscAddressPattern value of "".  This may be of use if the address pattern is
     * undetermined at the time of initialisation.  In which case, the address
     * pattern may be set later using setAddressPattern().
     *
     * @param oscAddressPattern OSC address pattern as null terminated string.
     */
    template <typename FirstArgument, typename... RemainingArguments>
    OscMessage (const char* oscAddressPattern, FirstArgument firstArgument, RemainingArguments... remainingArguments)
    {
        setAddressPattern (oscAddressPattern);
        addArguments (firstArgument, remainingArguments...);
    }
    
    /**
     * @brief Get the full address pattern
     *
     * @return String containing the full address pattern
     */
    const std::string& getAddressPattern() const;
    
    /**
     * @brief Sets the OSC address pattern of an OSC message.
     *
     * The oscAddressPattern argument must be a null terminated string of zero of
     * more characters.  The existing OSC address pattern will be overwritten.
     *
     * Example use:
     * @code
     * OscMessage oscMessage;
     * oscMessage.setAddressPattern ("/example/address/pattern");
     * @endcode
     *
     * @param oscAddressPattern OSC address pattern as null terminated string.
     * @return Error code (0 if successful).
     */
    OscError setAddressPattern (const char* oscAddressPattern);
    
    /**
     * @brief Appends OSC address pattern parts to the OSC address pattern of an
     * OSC message.
     *
     * The appendedParts argument must be a null terminated string.
     *
     * Example use:
     * @code
     * OscMessage oscMessage ("");
     * oscMessage.appendAddressPattern ("/example");
     * oscMessage.appendAddressPattern ("/address");
     * oscMessage.appendAddressPattern ("/pattern");
     * @endcode
     *
     * @param appendedParts OSC pattern parts to be appended.
     * @return Error code (0 if successful).
     */
    OscError appendAddressPattern (const char* appendedParts);
    
    /**
     * @brief Returns true if the message has the address pattern provided in the argument
     *
     * @return true if the address pattern passed as an argument matches the one for this message
     */
    bool hasAddressPattern (const std::string& addressPatternToMatch) const; 
    
    /**
     * @brief Adds a 32-bit integer argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addInt32 (123);
     * @endcode
     *
     * @param value 32-bit integer to be added as argument to the OSC message.
     */
    void addInt32 (int32_t value);

    /**
     * @brief Adds a 32-bit float argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addFloat32 (3.14f);
     * @endcode
     *
     * @param value 32-bit float to be added as argument to the OSC message.
     */
    void addFloat32 (float value);
    
    /**
     * @brief Adds a string argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addString ("Hello World!");
     * @endcode
     *
     * @param stringValue String to be added as argument to the OSC message.
     */
    void addString (const std::string& stringValue);
    
    /**
     * @brief Adds a blob (byte array) argument to an OSC message.
     *
     * Example use:
     * @code
     * const char source[] = { 0x00, 0x01, 0x02, 0x03, 0x04 };
     * oscMessage.addBlob (source, sizeof(source));
     * @endcode
     *
     * @param source Byte array to be added as argument.
     * @param size bytes in byte array to be added as argument.
     */
    void addBlob (const char* source, size_t size);
    
    /**
     * @brief Adds a 64-bit integer argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addInt64 (123);
     * @endcode
     *
     * @param value 64-bit integer to be added as argument to the OSC message.
     */
    void addInt64 (int64_t value);
    
    /**
     * @brief Adds an OSC time tag argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addTimeTag (oscTimeTagZero);
     * @endcode
     *
     * @param value OSC time tag to be added as argument to the OSC message.
     */
    void addTimeTag (OscTimeTag value);

    /**
     * @brief Adds a 64-bit float argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addDouble (3.14);
     * @endcode
     *
     * @param value 64-bit double to be added as argument to the OSC message.
     */
    void addFloat64 (Float64 value);
    
    /**
     * @brief Adds an alternate string argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addAlternateString ("Hello World!");
     * @endcode
     *
     * @param alternateStringValue String to be added as argument to the OSC message.
     */
    void addAlternateString (const std::string& alternateStringValue);
    
    /**
     * @brief Adds a char argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addCharacter ('a');
     * @endcode
     *
     * @param value character to be added as argument to the OSC message.
     */
    void addCharacter (char value);
    
    /**
     * @brief Adds a 32-bit RGBA colour argument to an OSC message.
     *
     * Example use:
     * @code
     * const RgbaColour rgbaColour = { 0x00, 0x00, 0x00, 0x00 };
     * oscMessage.addRgbaColour (rgbaColour);
     * @endcode
     *
     * @param value 32-bit RGBA colour to be added as argument to the OSC
     * message.
     */
    void addRgbaColour (RgbaColour value);
    
    /**
     * @brief Adds a 4 byte MIDI message argument to an OSC message.
     *
     * Example use:
     * @code
     * const MidiMessage midiMessage = { 0x00, 0x00, 0x00, 0x00 };
     * oscMessage.addMidiMessage (midiMessage);
     * @endcode
     *
     * @param value 4 byte MIDI message to be added as argument to the OSC
     * message.
     */
    void addMidiMessage (MidiMessageData value);
    
    /**
     * @brief Adds a boolean argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addBool (true);
     * @endcode
     *
     * @param value Boolean to be added as argument to the OSC message.
     */
    void addBool (bool value);
    
    /**
     * @brief Adds a nil argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addNil (&oscMessage);
     * @endcode
     */
    void addNil();
    
    /**
     * @brief Adds an infinitum argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addInfinitum();
     * @endcode
     */
    void addInfinitum();
    
    /**
     * @brief Adds a 'begin array' argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addBeginArray();
     * @endcode
     */
    void addBeginArray();
    
    /**
     * @brief Adds a 'end array' argument to an OSC message.
     *
     * Example use:
     * @code
     * oscMessage.addEndArray();
     * @endcode
     */
    void addEndArray();
    
    //overloaded addArgument functions
    void addArgument (int32_t value)                     {addInt32       (value);}
    void addArgument (float value)                       {addFloat32     (value);}
    void addArgument (const std::string& value)          {addString      (value);}
    void addArgument (const char* bytes, size_t size)    {addBlob        (bytes, size);}
    void addArgument (int64_t value)                     {addInt64       (value);}
    void addArgument (Float64 value)                     {addFloat64     (value);}
    void addArgument (OscTimeTag value)                  {addTimeTag     (value);}
    void addArgument (char value)                        {addCharacter   (value);}
    void addArgument (RgbaColour value)                  {addRgbaColour  (value);}
    void addArgument (MidiMessageData value)             {addMidiMessage (value);}
    void addArgument (bool value)                        {addBool        (value);}
    void addArgument (const char* value)                 {addString      (value);}
    
    /**
     * @brief Returns OSC type tag of the next argument available within an OSC
     * message indicated by the current oscTypeTagStringIndex value.
     *
     * A null character (value zero) will be returned if no arguments are available.
     *
     * Example use:
     * @code
     * const OscTypeTag oscTypeTag = oscMessage.getArgumentType();
     * @endcode
     *
     * @param argumentIndex index of the argument whose type is returned
     * @return Next type tag in type tag string.
     */
    const OscArgument::TypeTag getArgumentType (int argumentIndex) const;
    
    /**
     * @brief Provides read only access to arguments with array notation
     *
     * @param argumentIndex of the argument be returned
     * @return Read only reference to the specified argument
     */
    const OscArgument& getArgument (int argumentIndex) const noexcept
    {
        assert (argumentIndex < getNumberOfArguments());
        return arguments[argumentIndex];
    }
    
    /**
     * @brief Provides access to arguments with array notation
     *
     * @param argumentIndex of the argument be returned
     * @return Reference to the specified argument
     */
    OscArgument& operator[] (const int argumentIndex) noexcept
    {
        assert (argumentIndex < getNumberOfArguments());
        return arguments[argumentIndex];
    }
    
    /**
     * @brief Provides read only access to arguments with array notation
     *
     * @param argumentIndex of the argument be returned
     * @return Read only reference to the specified argument
     */
    const OscArgument& operator[] (const int argumentIndex) const noexcept
    {
        assert (argumentIndex < getNumberOfArguments());
        return arguments[argumentIndex];
    }
    
    /**
     * @brief Returns the number of arguments in this osc message.
     *
     * Example use:
     * @code
     * const size_t numAtguments = oscMessage.getNumberOfArguments();
     * @endcode
     *
     * @return number of arguments.
     */
    const int getNumberOfArguments() const
    {
        return (int)arguments.size();
    }
    
    /**
     * @brief Returns a string containing a text conversion of the message and all arguments.
     *
     * @return string containing the message and all arguments.
     */
    std::string toString() const
    {
        std::string string (getAddressPattern() + " ");
        
        for (auto a : arguments)
            string += a.asString() + " ";
        
        return string;
    }
    
    /**
     * @brief Returns the argument converted to string format.
     *
     * Example use:
     * @code
     * const std::strong argumentStrong = oscMessage.getArgumentAsString (0);
     * @endcode
     * 
     * @param argumentIndex index of the argument to be returned in string format
     * @return string containing the specified argument converted to text
     */
    const std::string getArgumentAsString (int argumentIndex) const
    {
        assert (argumentIndex < getNumberOfArguments());
        return arguments[argumentIndex].asString();
    }
    
    /**
     * @brief Returns in iterator pointing to the first argument of the message.
     * This is provided for compatibility with standard C++ iteration mechanisms.
     *
     * @return iterator pointing to the first element of the arguments vector
     */
    std::vector<OscArgument>::const_iterator begin() const
    {
        return arguments.begin();
    }
    
    /**
     * @brief Returns in iterator pointing to past-the-end argument of the message
     * This is provided for compatibility with standard C++ iteration mechanisms.
     *
     * @return iterator pointing to the past-the-end element of the arguments vector
     */
    std::vector<OscArgument>::const_iterator end() const
    {
        return arguments.end();
    }
    
    /**
     * @brief Constructor - creates an OscMessage from a byte array contaning an encoded OSC 
     * message. If the returned OscMessage is empty then it means that the data was malformed.
     *
     * @param source Source byte array.
     * @param sizeInBytes Number of bytes within the source byte array.
     */
    static OscMessage createFromEncodedData (const char* source, size_t sizeInBytes);
    
    bool operator== (const OscMessage& other) const;
    bool operator!= (const OscMessage& other) const;
    
    //OscElement
    ContentType getContentType() const override {return OscContent::MessageContent;}
    bool isEmpty() const override;
    bool isValid() const override;
    void clear() override;
    size_t getEncodedSize() const override;
    size_t encode (char* destination, size_t destinationSize) const override;
    
private:
    OscError decode (const char* source, size_t sizeInBytes) override;
    size_t getEncodedArgumentsSize() const;
    OscError error (OscError error);
    
    template <typename FirstArgument, typename... RemainingArguments>
    void addArguments (FirstArgument&& firstArgument, RemainingArguments&&... remainingArguments)
    {
        addArgument (firstArgument);
        addArguments (std::forward<RemainingArguments> (remainingArguments)...);
    }
    void addArguments() {} // zero argument case for variadic template above
    
    std::string addressPattern; // must be first member so that first byte of structure is equal to '/'.  Null terminated. TM - this will be a problem now it's a string
    std::vector<OscArgument> arguments;
};
 
 
#endif /* OscMessage_h */
