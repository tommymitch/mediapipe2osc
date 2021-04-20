//
//  OscError.cpp
//  OSC++
//
//  Created by Tom Mitchell on 14/02/2018.
//  Copyright © 2018 Tom Mitchell. All rights reserved.
//

//------------------------------------------------------------------------------
// Includes

#include "OscCommon.h"
#include "OscError.h"

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Returns the error message associated with the error code.
 *
 * Example use:
 * @code
 * OscMessage oscMessage;
 * const OscError oscError = OscMessageInitialise(&oscMessage, "/example/address/pattern");
 * printf(OscErrorGetMessage(oscError));
 * @endcode
 *
 * @param oscError Error code returned by library function.
 * @return Error message string.
 */
char * OscErrorGetMessage(const OscError oscError)
{
#ifdef OSC_ERROR_MESSAGES_ENABLED
    switch (oscError)
    {
        case OscErrorNone:
            return (char *) &"No error.";
            
            /* Common errors  */
        case OscErrorDestinationTooSmall:
            return (char *) &"Destination size too small to contain the number of bytes available.";
        case OscErrorSizeIsNotMultipleOfFour:
            return (char *) &"OSC packet or OSC contents size must be a multiple of four.";
        case OscErrorCallbackFunctionUndefined:
            return (char *) &"Callback function undefined.";
            
            /* OscAddress errors  */
        case OscErrorNotEnoughPartsInAddressPattern:
            return (char *) &"Not enough parts in OSC address pattern to get part at specified index.";
            
            /* OscMessage errors  */
        case OscErrorMessageToShort:
            return (char *) &"OSC message data is too short.";
        case OscErrorNoSlashAtStartOfMessage:
            return (char *) &"OSC address pattern does not start with a slash character.";
        case OscErrorAddressPatternUnterminated:
            return (char *) &"OSC address pattern unterminated.";
        case OscErrorTooManyArguments:
            return (char *) &"Number of arguments cannot exceed MAX_NUMBER_OF_ARGUMENTS.";
        case OscErrorArgumentsSizeTooLarge:
            return (char *) &"Total arguments size cannot exceed MAX_ARGUMENTS_SIZE.";
        case OscErrorUndefinedAddressPattern:
            return (char *) &"Undefined OSC address pattern.";
        case OscErrorMessageSizeTooSmall:
            return (char *) &"OSC message size too small to be a valid OSC message.";
        case OscErrorMessageSizeTooLarge:
            return (char *) &"OSC message size cannot exceed MAX_OSC_MESSAGE_SIZE.";
        case OscErrorSourceEndsBeforeEndOfAddressPattern:
            return (char *) &"Source data ends before the end of address pattern.";
        case OscErrorSourceEndsBeforeStartOfTypeTagString:
            return (char *) &"Source data ends before the start of type tag string.";
        case OscErrorTypeTagStringToLong:
            return (char *) &"Type tag string length cannot exceed MAX_OSC_TYPE_TAG_STRING_LENGTH.";
        case OscErrorSourceEndsBeforeEndOfTypeTagString:
            return (char *) &"Source data ends before the end of type tag string.";
        case OscErrorUnexpectedEndOfSource:
            return (char *) &"Unexpected end of source data.";
        case OscErrorNoArgumentsAvailable:
            return (char *) &"No arguments available.";
        case OscErrorUnexpectedArgumentType:
            return (char *) &"Unexpected argument type.";
        case OscErrorMessageTooShortForArgumentType:
            return (char *) &"OSC message is too short to contain argument type.";
            
            /* OscBundle errors  */
        case OscErrorBundleFull:
            return (char *) &"Not enough space available in OSC bundle to contain contents.";
        case OscErrorBundleSizeTooSmall:
            return (char *) &"OSC bundle size too small to be a valid OSC bundle.";
        case OscErrorBundleSizeTooLarge:
            return (char *) &"OSC bundle size cannot exceed MAX_OSC_BUNDLE_SIZE.";
        case OscErrorNoHashAtStartOfBundle:
            return (char *) &"OSC bundle does not start with a hash character.";
        case OscErrorMalformedBundleHeader:
            return (char *) &"OSC bundle header is not \"#bundle\".";
        case OscErrorMalformedElement:
            return (char *) &"OSC bundle element is corrupted";
        case OscErrorSourceEndsBeforeBundleElementSize:
            return (char *) &"Data source ends before bundle element size";
        case OscErrorBundleElementNotAvailable:
            return (char *) &"OSC bundle element not available.";
        case OscErrorBundleCouldNotAddBundleElement:
            return (char *) &"Could not add bundle element.";
        case OscErrorNegativeBundleElementSize:
            return (char *) &"OSC bundle element size cannot be negative.";
        case OscErrorInvalidElementSize:
            return (char *) &"OSC bundle too short to contain the OSC bundle element size.";
            
            /* OscPacket errors  */
        case OscErrorInvalidContents:
            return (char *) &"OSC contents is not an OSC bundle or OSC message.";
        case OscErrorPacketSizeTooLarge:
            return (char *) &"OSC packet size cannot exceed MAX_OSC_PACKET_SIZE.";
        case OscErrorContentsEmpty:
            return (char *) &"OSC contents size cannot be zero.";
            
            /* OscSlip errors  */
        case OscErrorEncodedSlipPacketTooLong:
            return (char *) &"Encoded SLIP packet size cannot exceed OSC_SLIP_DECODER_BUFFER_SIZE.";
        case OscErrorUnexpectedByteAfterSlipEsc:
            return (char *) &"Unexpected byte after SLIP ESC byte.";
        case OscErrorDecodedSlipPacketTooLong:
            return (char *) &"Decoded SLIP packet size cannot exceed MAX_OSC_PACKET_SIZE.";
    }
    return (char *) &"Unknown error.";
#else
    return (char *) &"OSC error.";
#endif
}

//------------------------------------------------------------------------------
// End of file
