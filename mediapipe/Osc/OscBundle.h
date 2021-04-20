/**
 * @file OscBundle.h
 * @author Tom Mitchell
 * @brief Functions and structures for constructing and deconstructing OSC
 * bundles.
 * @see http://opensoundcontrol.org/spec-1_0
 */


#ifndef OSC_BUNDLE_H
#define OSC_BUNDLE_H

//------------------------------------------------------------------------------
// Includes
#include <stddef.h>

#include "OscCommon.h"
#include "OscError.h"
#include "OscMessage.h"
#include "OscContent.h"

/**
 * @brief OSC bundle class.
 */
class OscBundle : public OscContent
{
public:
    /**
     * @brief OSC bundle header.  These are the first 8 bytes (including the
     * terminating null character) that appear at the start of every bundle.
     */
    static const std::string BundleHeader;
    
    /**
     * @brief Minimum size (number of bytes) of an OSC bundle as per the OSC
     * specification.
     */
    static const size_t MinimumBundleSize;
    
    /**
     * @brief Constructs an OSC bundle
     *
     * The oscTimeTag argument will be oscTimeTagZero.  Use this if the OSC time tag value 
     * is irrelevant to the user application, if the contained OSC messages should be invoke 
     * immediately, or if the OSC time-tag value is intended to be overwritten after 
     * initialisation of the OSC bundle.
     */
    OscBundle();
    
    /**
     * @brief Initialises an OSC bundle with a specified OSC time tag.
     *
     * The oscTimeTag argument may be specified as oscTimeTagZero for an OSC time-tag value of  
     * zero.  This may be of use if the the contained OSC messages should be invoke immediately, or 
     * if the OSC time-tag value is intended to be overwritten after initialisation of the OSC 
     * bundle.
     *
     * @param time OSC time tag.
     */
    OscBundle (OscTimeTag time);
    
    /**
     * @brief Copy constructor
     */
    OscBundle (const OscBundle& other) : header (other.header)
    {
        timeTag = other.timeTag;
        for (auto& e : other.elements)
            if (e->isBundle())
                elements.push_back (std::make_unique<OscBundle> (e->getAsBundle()));
            else
                elements.push_back (std::make_unique<OscMessage> (e->getAsMessage()));
    }
    
    /**
     * @brief Move constructor
     */
    OscBundle (OscBundle&& other) :   header (std::move (other.header)),
                                      elements (std::move (other.elements))
    {
        timeTag = other.timeTag;
    }

    /**
     * @brief Adds an OSC message or OSC bundle to this OSC bundle.
     *
     * This function may be called multiple times to add multiple  OSC
     * messages or OSC bundles to this OSC bundle.
     *
     * Example use:
     * @code
     * OscBundle bundle;
     * OscMessage oscMessageToAdd ("/example/address/pattern");
     * bundle.addContent (oscMessageToAdd);
     *
     * OscBundle oscBundleToAdd;
     * bundle.addContent (oscMessageToAdd);
     * @endcode
     *
     * @param contentToAdd OSC message or bundle to be added to this OSC bundle.
     */
    void addContent (const OscContent& contentToAdd);
    
    /**
     * @brief Adds an OSC message to this OSC bundle.
     *
     * This function may be called multiple times to add multiple  OSC
     * messages to this OSC bundle.
     *
     * @param messageToAdd OSC message or bundle to be added to this OSC bundle.
     */
    void addMessage (const OscMessage& messageToAdd);
    
    /**
     * @brief Adds an OSC bundle to this OSC bundle.
     *
     * This function may be called multiple times to add multiple  OSC
     * bundles to this OSC bundle.
     *
     * @param bundleToAdd OSC message or bundle to be added to this OSC bundle.
     */
    void addBundle (const OscBundle& bundleToAdd);

    /**
     * @brief Returns the number of elements in this bundle.
     *
     * Elements can be an OSC message or OSC bundle (which may contain further messages/bundles).
     *
     * @return number of elements in this bundle.
     */
    size_t size() const {return elements.size();}
    
    /**
     * @brief Provides read only access to specified OSC element
     *
     * OSC bundle elements are references to OSC Content which is the base class of an OSC message
     * or bundle. You can probe what type of OSC Content the return reference is using the 
     * isMessage()/isBundle() helpers and then use getAsMessage()/getAsBundle to call type specific
     * functions.
     *
     * @param contentIndex of the argument be returned
     * @return Read only reference to the specified OSC content
     */
    const OscContent& get (int contentIndex) const noexcept
    {
        assert (contentIndex < size());
        return *elements[contentIndex];
    }
    
    /**
     * @brief Provides access to arguments with array notation. 
     *
     * @see get() for a more detailed explanation
     *
     * @param contentIndex of the argument be returned
     * @return Reference to the specified message
     */
    OscContent& operator[] (const int contentIndex) noexcept
    {
        assert (contentIndex < size());
        return *elements[contentIndex];
    }
    
    /**
     * @brief Provides read only access to arguments with array notation
     *
     * @param contentIndex of the argument be returned
     * @return Read only reference to the specified message
     */
    const OscContent& operator[] (const int contentIndex) const noexcept
    {
        return get (contentIndex);
    }
    
    /**
     * @brief Creates an OSC bundle from a byte array containing a encoded OSC bundle. If the 
     * returned OscBundle is empty then it means that the data was malformed.
     *
     * @param source Source byte array.
     * @param sizeInBytes Number of bytes within the source byte array.
     */
    static OscBundle createFromEncodedData (const char* source, size_t sizeInBytes);
    
    /**
     * @brief Returns in iterator pointing to the first element of the bundle.
     * This is provided for compatibility with standard C++ iteration mechanisms.
     *
     * @return iterator pointing to the first item in the bundle elements vector
     */
    std::vector<std::unique_ptr<OscContent>>::const_iterator begin() const
    {
        return elements.begin();
    }
    
    /**
     * @brief Returns in iterator pointing to past-the-end argument of the message
     * This is provided for compatibility with standard C++ iteration mechanisms.
     *
     * @return iterator pointing to the past-the-end element of the arguments vector
     */
    std::vector<std::unique_ptr<OscContent>>::const_iterator end() const
    {
        return elements.end();
    }
    
    bool operator== (const OscBundle& other) const;
    bool operator!= (const OscBundle& other) const;
    
    //OscContent
    ContentType getContentType() const override {return OscContent::BundleContent;}
    bool isEmpty() const override  {return elements.size() == 0;}
    bool isValid() const override;
    void clear() override;
    size_t getEncodedSize() const override;
    size_t encode (char* destination, size_t destinationSize) const override;
    
private:
    /**
     * goes through each element of this bundle calculating the sum of all elements. Note that this 
     * does not give the total size of the encoded bundle as this excludes bundle header and 
     * time tag. See @getEncodedSize() for that.
     */
    size_t getEncodedElementsSize() const;
    OscError error (OscError error);
    OscError decode (const char* source, size_t sizeInBytes) override;
    
    std::string header = BundleHeader;
    std::vector<std::unique_ptr<OscContent>> elements;
};

#endif
