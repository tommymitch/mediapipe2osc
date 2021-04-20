//
//  OscError.h
//  OSC++
//
//  Created by Tom Mitchell on 14/02/2018.
//  Copyright © 2018 Tom Mitchell. All rights reserved.
//

#ifndef OscError_h
#define OscError_h

//------------------------------------------------------------------------------
// Definitions

/**
 * @brief Enumerated error codes for debugging and user feedback.
 */
enum OscError
{
    OscErrorNone = 0,
    
    /* Common errors  */
    OscErrorDestinationTooSmall,
    OscErrorSizeIsNotMultipleOfFour,
    OscErrorCallbackFunctionUndefined,
    
    /* OscAddress errors  */
    OscErrorNotEnoughPartsInAddressPattern,
    
    /* OscMessage errors  */
    OscErrorMessageToShort,
    OscErrorNoSlashAtStartOfMessage,
    OscErrorAddressPatternUnterminated,
    OscErrorTooManyArguments,
    OscErrorArgumentsSizeTooLarge,
    OscErrorUndefinedAddressPattern,
    OscErrorMessageSizeTooSmall,
    OscErrorMessageSizeTooLarge,
    OscErrorSourceEndsBeforeEndOfAddressPattern,
    OscErrorSourceEndsBeforeStartOfTypeTagString,
    OscErrorTypeTagStringToLong,
    OscErrorSourceEndsBeforeEndOfTypeTagString,
    OscErrorUnexpectedEndOfSource,
    OscErrorNoArgumentsAvailable,
    OscErrorUnexpectedArgumentType,
    OscErrorMessageTooShortForArgumentType,
    
    /* OscBundle errors  */
    OscErrorBundleFull,
    OscErrorBundleSizeTooSmall,
    OscErrorBundleSizeTooLarge,
    OscErrorNoHashAtStartOfBundle,
    OscErrorMalformedBundleHeader,
    OscErrorMalformedElement,
    OscErrorSourceEndsBeforeBundleElementSize,
    OscErrorBundleElementNotAvailable,
    OscErrorBundleCouldNotAddBundleElement,
    OscErrorNegativeBundleElementSize,
    OscErrorInvalidElementSize,
    
    /* OscPacket errors  */
    OscErrorInvalidContents,
    OscErrorPacketSizeTooLarge,
    OscErrorContentsEmpty,
    
    /* OscSlip errors  */
    OscErrorEncodedSlipPacketTooLong,
    OscErrorUnexpectedByteAfterSlipEsc,
    OscErrorDecodedSlipPacketTooLong,
    
};

//------------------------------------------------------------------------------
// Function prototypes

char * OscErrorGetMessage(const OscError oscError);

#endif /* OscError_h */

//------------------------------------------------------------------------------
// End of file
