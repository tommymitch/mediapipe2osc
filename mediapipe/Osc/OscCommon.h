//
//  OscCommon.h
//  OSC++
//
//  Created by Tom Mitchell on 14/02/2018.
//  Copyright © 2018 Tom Mitchell. All rights reserved.
//

#ifndef OscCommon_h
#define OscCommon_h

//------------------------------------------------------------------------------
// Includes

#include <stdlib.h>
#include <float.h> // DBL_MANT_DIG
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//------------------------------------------------------------------------------
// Definitions - Application/platform specific

/**
 * @brief Comment out this definition if the platform is big-endian.  For
 * example: Arduino, Atmel AVR, Microchip PIC, Intel x86-64 are little-endian.
 * @see http://en.wikipedia.org/wiki/Endianness
 */
#define LITTLE_ENDIAN_PLATFORM

/**
 * @brief Maximum packet size permitted by the transport layer.  Reducing this
 * value will reduce the amount of memory required.
 */
#define MAX_TRANSPORT_SIZE (1472)

/**
 * @brief Comment out this definition to prevent the OscErrorGetMessage function
 * from providing detailed error messages.  This will reduce the amount of
 * memory required.
 */
#define OSC_ERROR_MESSAGES_ENABLED

/**
 * @brief 64-bit double precision floating point number.  
 * Defined as double or long double depending on platform.
 */
#if (DBL_MANT_DIG == 53)
using Float64 = double;
#else
using Float64 = long double; // use long double if double is not 64-bit
#endif

/**
 * @brief terminates and pads an osc string with '\0' characters.
 *
 * @param oscContents OSC packet, OSC bundle, or OSC message.
 * @param oscStringSize OSC packet, OSC bundle, or OSC message.
 * @param maxOscStringSize OSC packet, OSC bundle, or OSC message.
 * @return True if there are no errors.
 */

inline bool terminateOscString (char* oscContents, size_t& oscStringSize, size_t maxOscStringSize)
{
    do
    {
        if (oscStringSize >= maxOscStringSize)
            return true; // error: string exceeds maximum size
        oscContents[oscStringSize++] = '\0';
    }
    while (oscStringSize % 4 != 0);
    return false;
}

/**
 * @brief useful for working out how much a data pointer needs to be packed to make it a multiple of
 * four bytes.
 *
 * @return value of argument raised to the nearest multiple of 4.
 */
inline size_t getPaddedSize (size_t unpaddedSize)
{
    return unpaddedSize % 4 == 0 ? unpaddedSize : unpaddedSize + (4 - (unpaddedSize % 4));
}

#endif /* OscCommon_h */
