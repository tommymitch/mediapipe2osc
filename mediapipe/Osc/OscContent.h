//
//  OscContent.hpp
//  OSC++
//
//  Created by Tom Mitchell on 25/04/2018.
//  Copyright © 2018 Tom Mitchell. All rights reserved.
//

#ifndef OscContent_hpp
#define OscContent_hpp

#include "OscArgument.h"
#include "OscError.h"


class OscMessage;
class OscBundle;

/**
 * Base class for OSC content (i.e. a message or a bundle)
 *
 *
 */

class OscContent
{
public:
    OscContent();
    
    virtual ~OscContent() {}
    enum ContentType
    {
        MessageContent = '/',
        BundleContent = '#'
    };
    
    /**
     * @brief returns an @ContentType indicating whether this is a message or bundle
     *
     * @return ContentType indicating that this is a message or bundle
     */
    virtual ContentType getContentType() const = 0;
    
    /**
     * @brief Returns true if this OscContent is empty.
     *
     * When initialised contents shold be empty.
     *
     * @return true if empty
     */
    virtual bool isEmpty() const = 0;
    
    /**
     * @brief Returns true if this content is valid and theirfore sendable as OscContent.
     *
     * Derivatives will need to check internal state to see whether the messaeg is sendable. How
     * this is achieved varies between types because, for example, an empty OscBundle (i.e. with
     * header '#bundle' and timeTag 0) is valid and sendable, but an empty OscMessage is not. This
     * function can be used to firstly see if an OscContent is really for sending or to check 
     * encodedRaw Osc data was valid and passed correctly.
     *
     * @return true if empty
     */
    virtual bool isValid() const = 0;
    
    /**
     * @brief Clears the content, @isEmpty will return true after this is called.
     *
     * Clears the message emptying all internal containers for the message.
     *
     */
    virtual void clear() = 0;
    
    /**
     * @brief Returns the size (number of bytes) that this message will take up when encoded.
     *
     * An example use of this function would be to check whether the OSC message size
     * exceeds the remaining capacity of a containing OSC bundle.
     *
     * Example use:
     * @code
     * if (oscMessage.getEncodedDataSize() > OscBundleGetRemainingCapacity (&oscBundle))
     *     std::cout ("oscMessage is too large to be contained within oscBundle");
     * @endcode
     *
     * @return Size (number of bytes) of the OSC bundle.
     */
    virtual size_t getEncodedSize() const = 0;
    
    /**
     * @brief encodes the OSC content into a byte array.
     *
     * @param destination Destination byte array.
     * @param destinationSize Destination size that cannot exceed.
     * @return number of bytes written to the destination if this is less than getEncodedDataSize()
     * then probably @destination was not big enough.
     */
    virtual size_t encode (char* destination, size_t destinationSize) const = 0;
    
    /**
     * @brief returns the number of bytes that the SLIP encoded version of oscEncodedSource will
     * take up. oscEncodedSource can be created with the encode function.
     *
     * This is usually called before slipEncode() to to work out how big the destination array needs
     * to be before SLIP encoding the content.
     *
     * @param oscEncodedSource source byte array.
     * @param sourceSize size of the source oscContent in bytes.
     * @return size of the slip encoded version of the content
     */
    static size_t getSlipEncodedSize (const char* oscEncodedSource, size_t sourceSize);
    
    /**
     * @brief encodes the OSC content using SLIP, should be called after encode.
     *
     * This encodes in place so even though the oscEncodedSource is sourceSize the underlying array 
     * should be large enough to contain the slip encoded version of the source content. You can get
     * this size with getSlipEncodedSize
     *
     * @param oscEncodedSource source byte array.
     * @param sourceSize size of the source oscContent in bytes.
     * @param slipEncodedDestination pointer to an array that is large enough for the slip encoded 
     *        data to be coppied.
     * @return size of the slip encoded content, if this not equal to what getSlipEncodedSize() 
     *         previouly returned then something went wrong.
     * @see http://en.wikipedia.org/wiki/Serial_Line_Internet_Protocol
     */
    static size_t slipEncode (const char* oscEncodedSource, size_t sourceSize, char* slipEncodedDestination);
    
    /**
     * @brief returns the number of bytes that that the SLIP decoded version of slipEncodedSource will
     * take up.
     *
     * This is usually called before slipDecode() to to work out how big the oscEncodedDestination 
     * array needs to be for in place encoding.
     *
     * @param slipEncodedSource source byte array.
     * @param sourceSize size of the source oscContent in bytes.
     * @return size of the slip encoded version of the content
     */
    static size_t getSlipDecodedSize (const char* slipEncodedSource, size_t sourceSize);
    
    /**
     * @brief decodes slip encoded OSC into an OSC byte array that can then be passed to the 
     * OscMessage or OscBundle 'createFromEncodedData' functions
     *
     * @param slipEncodedSource source byte array.
     * @param sourceSize size of the source slip encoded data.
     * @param oscEncodedDestination pointer to an array that is large enough for the decoded osc
     *        content to be coppied.
     * @return new size of the slip decoded content
     */
    static size_t slipDecode (char* slipEncodedSource, size_t sourceSize, char* oscEncodedDestination);
    
    /**
     * Checks the argument to see if it is the SLIP end of frame character
     * @param character the character to test
     * @return true if the argument is an end of frame character
     */
    static bool isSlipEndCharacter (char character);
    
    bool isMessage() const {return getContentType() == MessageContent;}
    bool isBundle() const {return getContentType() == BundleContent;}
    
    OscMessage& getAsMessage();
    OscBundle& getAsBundle();
    
    const OscMessage& getAsMessage() const;
    const OscBundle& getAsBundle() const;
    
    inline static bool encodedContentIsMessage (const char* content)
    {
        return *content == (char)MessageContent;
    };
    
    inline static bool encodedContentIsBundle (const char* content)
    {
        return *content == (char)BundleContent;
    };
    
    OscTimeTag getTimeTag() const { return timeTag; }
    void setTimeTag (OscTimeTag newTimeTag) {timeTag = newTimeTag;}

protected:
    virtual OscError decode (const char* source, size_t sizeInBytes) = 0;
    OscTimeTag timeTag;
private:
    
};

#endif /* OscContent_hpp */
