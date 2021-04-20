/**
 * @file OscBundle.c
 * @author Tom Mitchell
 * @brief Functions and structures for constructing and deconstructing OSC
 * bundles.
 * @see http://opensoundcontrol.org/spec-1_0
 */

//------------------------------------------------------------------------------
// Includes

#include "OscBundle.h"
#include "OscArgument.h"

const std::string OscBundle::BundleHeader = "#bundle";
const size_t OscBundle::MinimumBundleSize = BundleHeader.length() + sizeof ('\0') + sizeof (OscArgument64);

OscBundle::OscBundle()
{

}

OscBundle::OscBundle (OscTimeTag t)
{
    timeTag = t;
}

void OscBundle::addContent (const OscContent& contentToAdd)
{
    if (contentToAdd.isMessage())
        addMessage (getAsMessage());
    else if (contentToAdd.isBundle())
        addBundle (getAsBundle());
}
void OscBundle::addMessage (const OscMessage& messageToAdd)
{
    if (messageToAdd.isValid())
        elements.push_back (std::make_unique<OscMessage> (messageToAdd));
}

void OscBundle::addBundle (const OscBundle& bundleToAdd)
{
    if (bundleToAdd.isValid())
        elements.push_back (std::make_unique<OscBundle> (bundleToAdd));
}

void OscBundle::clear()
{
    timeTag = OscTimeTag();
    elements.clear();
}

OscBundle OscBundle::createFromEncodedData (const char* source, size_t sizeInBytes)
{
    OscBundle b;
    b.decode (source, sizeInBytes);
    return b;
}

bool OscBundle::operator== (const OscBundle& other) const
{
    if (getTimeTag() != other.getTimeTag())
        return false;
    
    if (size() != other.size())
        return false;
    
    for (int i = 0; i < size(); i++)
    {
        const OscContent& c = (*this)[i];
        const OscContent& cother = other[i];
        
        if (c.getContentType() != cother.getContentType())
            return false;
        
        if (c.isBundle())
        {
            if (c.getAsBundle() != cother.getAsBundle())
                return false;
        }
        else
        {
            if (c.getAsMessage() != cother.getAsMessage())
                return false;
        }
    }
    return true;
}

bool OscBundle::operator!= (const OscBundle& other) const
{
    if (getTimeTag() != other.getTimeTag())
        return true;
    
    if (size() != other.size())
        return true;
    
    for (int i = 0; i < size(); i++)
    {
        const OscContent& c = (*this)[i];
        const OscContent& cother = other[i];
        
        if (c.getContentType() != cother.getContentType())
            return true;
        
        if (c.isBundle())
        {
            if (c.getAsBundle() != cother.getAsBundle())
                return true;
        }
        else
        {
            if (c.getAsMessage() != cother.getAsMessage())
                return true;
        }
    }
    return false;
}

bool OscBundle::isValid() const
{
    return ! header.empty() && header[0] == '#';
}

size_t OscBundle::getEncodedSize() const
{
    size_t bundleSize = 0;
    bundleSize +=  header.size() + sizeof ('\0');
    bundleSize += sizeof (OscTimeTag);
    bundleSize += getEncodedElementsSize();
    return bundleSize;
}

size_t OscBundle::encode (char* destination, size_t destinationSize) const
{
    size_t destinationIndex = 0;
    
    //header
    if (header.length() > destinationSize)
        return destinationIndex;
    
    for (auto& c : header)
        destination[destinationIndex++] = c;

    if (terminateOscString (destination, destinationIndex, destinationSize) != 0)
        return destinationIndex;
    
    //time tag
    if (destinationIndex + sizeof (OscTimeTag) > destinationSize)
        return destinationIndex;
    
    OscArgument64 timeTag = {getTimeTag()};
    destinationIndex += OscArgument::encodeArgument64 (timeTag, &destination[destinationIndex]);
    
    //add bundle elements
    for (auto& p : elements)
    {
        if (destinationIndex + sizeof (OscArgument32) > destinationSize)
            return destinationIndex;
        
        OscArgument32 size ((int32_t)p->getEncodedSize());
        destinationIndex += OscArgument::encodeArgument32 (size, &destination[destinationIndex]);
        
        destinationIndex += p->encode (&destination[destinationIndex], destinationSize - destinationIndex);
    }
    return destinationIndex;
}

size_t OscBundle::getEncodedElementsSize() const
{
    size_t elementsSize = 0;
    for (auto& c : elements)
    {
        elementsSize += sizeof (OscArgument32); //int32 containing the size of the content
        elementsSize += c->getEncodedSize();    //the content size
    }
    return elementsSize;
}

OscError OscBundle::error (OscError error)
{
    elements.clear();
    header[0] = '\0'; //null terminate the header to indicate an error;
    return error;
}

OscError OscBundle::decode (const char* source, size_t sizeInBytes)
{
    size_t sourceIndex = 0;
    
    // Return error if not valid bundle
    if (sizeInBytes % 4 != 0)
        return error (OscErrorSizeIsNotMultipleOfFour); // error: size not multiple of 4

    if (sizeInBytes < MinimumBundleSize)
        return error (OscErrorBundleSizeTooSmall); // error: too few bytes to contain bundle
    
    if (source[sourceIndex] != '#')
        return error (OscErrorNoHashAtStartOfBundle); // error: first byte is not '#'
    
    // Header
    if (header != source)
        return error (OscErrorMalformedBundleHeader);
    
    sourceIndex += header.size() + sizeof ('\0');
    
    // decode OSC time tag
    setTimeTag (OscArgument::decodeArgument64 (&source[sourceIndex]).oscTimeTag);
    sourceIndex += sizeof (OscTimeTag);
    
    while (sourceIndex < sizeInBytes)
    {
        if (sourceIndex + sizeof (OscArgument32) > sizeInBytes)
            return error (OscErrorSourceEndsBeforeBundleElementSize);
        OscArgument32 elementSize = OscArgument::decodeArgument32 (&source[sourceIndex]);
        
        //advance to content
        sourceIndex += sizeof (OscArgument32);
        
        if (sourceIndex + elementSize.int32 > sizeInBytes)
            return error (OscErrorUnexpectedEndOfSource);
        
        std::unique_ptr<OscContent> p;
        
        if (OscContent::encodedContentIsMessage (&source[sourceIndex]))
            p = (std::make_unique<OscMessage> (OscMessage::createFromEncodedData (&source[sourceIndex], elementSize.int32)));
        else if (OscContent::encodedContentIsBundle (&source[sourceIndex]))
            p = (std::make_unique<OscBundle> (OscBundle::createFromEncodedData (&source[sourceIndex], elementSize.int32)));
        else
            return error (OscErrorMalformedElement);
        
        if (! p->isValid())
            return error (OscErrorMalformedElement);
        
        p->setTimeTag (timeTag);
        elements.push_back (std::move (p));
        sourceIndex += elementSize.int32;
    }
    return OscErrorNone;
}
