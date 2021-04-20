/**
 * @file OscAddress.c
 * @author Seb Madgwick
 * @brief Functions for matching and manipulating OSC address patterns and OSC
 * addresses.
 * @see http://opensoundcontrol.org/spec-1_0
 */

//------------------------------------------------------------------------------
// Includes

#include "OscAddress.h"
#include <string.h>

//------------------------------------------------------------------------------
// Function prototypes

static bool MatchLiteral(const char * oscAddressPattern, const char * oscAddress, const bool isPartial);
static bool MatchExpression(const char * * const oscAddressPattern, const char * * const oscAddress, const bool isPartial);
static bool MatchStar(const char * * const oscAddressPattern, const char * * const oscAddress, const bool isPartial);
static bool MatchCharacter(const char * * const oscAddressPattern, const char * * const oscAddress, const bool isPartial);
static bool MatchBrackets(const char * * const oscAddressPattern, const char * * const oscAddress);
static bool MatchCurlyBraces(const char * * const oscAddressPattern, const char * * const oscAddress, const bool isPartial);

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Matches an OSC address pattern with a target OSC address.
 *
 * Returns true if the OSC address pattern matches the target OSC address.  The
 * target OSC address cannot contain any special characters: '?', ' *', '[]', or
 * '{}'.
 *
 * Example use:
 * @code
 * OscMessage oscMessage;
 * OscMessageInitialise(&oscMessage, "/example/oscAddress/pattern");
 * if(OscAddressMatch(oscMessage.oscAddressPattern, "/example/oscAddress/pattern") == true) {
 *     printf("Match!");
 * }
 * @endcode
 *
 * @param oscAddressPattern OSC address pattern.
 * @param oscAddress Target OSC address.
 * @return True if the OSC address pattern and target oscAddress match.
 */
bool OscAddressMatch(const char * oscAddressPattern, const char * const oscAddress) {
    return MatchLiteral(oscAddressPattern, oscAddress, false);
}

/**
 * @brief Matches an OSC address pattern with a partial target OSC address that
 * has been truncated.
 *
 * Returns true if the OSC address pattern matches the partial target OSC
 * address.  The target OSC address cannot contain any special characters:
 * '?', ' *', '[]', or '{}'.
 *
 * Matching to a partial OSC address can simplify the process of filtering
 * through multiple similar OSC address.  For example, matching to the following
 * list would be quicker if an incoming message was first matched to either
 * "/inputs" and "/outputs".
 * - "/inputs/digital"
 * - "/inputs/analogue"
 * - "/inputs/serial"
 * - "/outputs/digital"
 * - "/outputs/pwm"
 * - "/outputs/serial"
 *
 * Example use:
 * @code
 * OscMessage oscMessage;
 * OscMessageInitialise(&oscMessage, "/example/oscAddress/pattern");
 * if(OscAddressMatch(oscMessage.oscAddressPattern, "/example") == true) {
 *     printf("Match!");
 * }
 * @endcode
 *
 * @param oscAddressPattern OSC address pattern.
 * @param oscAddress Partial target OSC address.
 * @return True if the OSC address pattern matches the partial target OSC
 * oscAddress.
 */
bool OscAddressMatchPartial(const char * oscAddressPattern, const char * const oscAddress) {
    return MatchLiteral(oscAddressPattern, oscAddress, true);
}

/**
 * @brief Matches literal OSC address pattern with target OSC address.
 *
 * The OSC address pattern is initially assumed to be literal and not to contain
 * any special characters: '?', ' *', '[]', or '{}'.  If a special character is
 * found then the result of MatchExpression is returned.  Matching literal OSC
 * address patterns is faster than matching OSC address patterns that contain
 * special characters.
 *
 * This is an internal function and cannot be called by the user application.
 *
 * @param oscAddressPattern First character of OSC address pattern.
 * @param oscAddress First character of target OSC address.
 * @param isPartial Flag indicating if a partial match is acceptable.
 * @return True if OSC address pattern and target OSC address match.
 */
static bool MatchLiteral(const char * oscAddressPattern, const char * oscAddress, const bool isPartial) {
    while (*oscAddressPattern != '\0') {
        if (*oscAddress == '\0') {
            if (isPartial == true) {
                return true;
            } else {
                return MatchExpression(&oscAddressPattern, &oscAddress, isPartial); // handle trailing 'zero character' expressions
            }
            return false; // fail: OSC address pattern too short
        }
        switch (*oscAddressPattern) {
            case '?':
            case '*':
            case '[':
            case '{':
                return MatchExpression(&oscAddressPattern, &oscAddress, isPartial);
            default:
                if (*oscAddressPattern != *oscAddress) {
                    return false; // fail: character mismatch
                }
                break;
        }
        oscAddressPattern++;
        oscAddress++;
    }
    if (*oscAddress != '\0') {
        return false; // fail: OSC address pattern too long
    }
    return true;
}

/**
 * @brief Matches an OSC address pattern expression with a target OSC address.
 *
 * The OSC address pattern expression may contain any combination of special
 * characters: '?', ' *', '[]', or '{}'.
 *
 * This is an internal function and cannot be called by the user application.
 *
 * @param oscAddressPattern First character of OSC address pattern.
 * @param oscAddress First character of target OSC address.
 * @param isPartial Flag indicating if a partial match is acceptable.
 * @return True if OSC address pattern and target OSC address match.
 */
static bool MatchExpression(const char * * const oscAddressPattern, const char * * const oscAddress, const bool isPartial) {
    while (**oscAddressPattern != '\0') {
        if (**oscAddress == '\0') {
            if (isPartial == true) {
                return true;
            }
        }
        if (**oscAddressPattern == '*') {
            if (MatchStar(oscAddressPattern, oscAddress, isPartial) == false) {
                return false; // fail: unable to match star sequence
            }
        } else {
            if (MatchCharacter(oscAddressPattern, oscAddress, isPartial) == false) {
                return false; // fail: unable to match single character, bracketed list or curly braced list
            }
        }
    }
    if (**oscAddress != '\0') {
        return false; // fail: OSC address pattern too long
    }
    return true;
}

/**
 * @brief Matches an OSC address pattern expression starting with a star with
 * the next character(s) in the target OSC address.
 *
 * The OSC address pattern must start with a ' *' character.  A ' *' character
 * will be matched to any sequence of zero or more characters in the OSC address
 * up to the next '/' character or to the end of the OSC address.  For example,
 * the OSC address pattern "/colour/b *" would match the OSC addresses
 * "/colour/blue", "/colour/black" and "/colour/brown".
 *
 * The oscAddressPattern and oscAddress pointers are advanced to the character
 * proceeding the matched sequence. This function calls MatchCharacter and so
 * becomes recursive if the expression contains multiple stars.
 *
 * This is an internal function and cannot be called by the user application.
 *
 * @param oscAddressPattern First character of OSC address pattern.
 * @param oscAddress First character of target OSC address.
 * @param isPartial Flag indicating if a partial match is acceptable.
 * @return True if OSC address pattern and target OSC address match.
 */
static bool MatchStar(const char * * const oscAddressPattern, const char * * const oscAddress, const bool isPartial) {

    // Advance OSC address pattern pointer to character proceeding star(s)
    while (**oscAddressPattern == '*') {
        (*oscAddressPattern)++;
    }

    // Advance OSC address pattern pointer to end of part if star is last character
    if ((**oscAddressPattern == '/') || (**oscAddressPattern == '\0')) {
        while ((**oscAddress != '/') && (**oscAddress != '\0')) {
            (*oscAddress)++;
        }
        return true;
    }

    // Attempt to match remainder of expression for each possible star match
    do {
        const char * oscAddressPatternCache = *oscAddressPattern; // cache character oscAddress proceeding star

        // Advance OSC address pattern to next match of character proceeding star
        while (MatchCharacter(oscAddressPattern, oscAddress, isPartial) == false) {
            (*oscAddress)++;
            if ((**oscAddress == '/') || (**oscAddress == '\0')) {
                if ((isPartial == true) && (**oscAddress == '\0')) {
                    return true;
                }
                return false; // fail: OSC address pattern part ended before match
            }
        }
        const char * oscAddressCache = (*oscAddress); // cache character oscAddress proceeding current star match

        // Attempt to match remainder of expression
        if (MatchExpression(oscAddressPattern, oscAddress, isPartial) == true) { // potentially recursive
            return true;
        } else {
            *oscAddressPattern = oscAddressPatternCache;
            *oscAddress = oscAddressCache;
        }
    } while (true);
}

/**
 * @brief Matches an OSC address pattern starting with literal character,
 * bracketed list or curly braced list with the next character(s) in the target
 * OSC address.
 *
 * The oscAddressPattern and oscAddress pointers are advanced to the character
 * proceeding the matched sequence only if a match is found.  The pointers are
 * not modified if a match cannot be found.
 *
 * This is an internal function and cannot be called by the user application.
 *
 * @param oscAddressPattern First character of OSC address pattern.
 * @param oscAddress First character of target OSC address.
 * @param isPartial Flag indicating if a partial match is acceptable.
 * @return True if OSC address pattern and target OSC address match.
 */
static bool MatchCharacter(const char * * const oscAddressPattern, const char * * const oscAddress, const bool isPartial) {
    const char * oscAddressPatternCache = *oscAddressPattern;
    const char * oscAddressCache = *oscAddress;
    switch (**oscAddressPattern) {
        case '[':
            if (MatchBrackets(oscAddressPattern, oscAddress) == true) {
                return true;
            }
            break;
        case ']':
            break; // fail: unbalanced brackets
        case '{':
            if (MatchCurlyBraces(oscAddressPattern, oscAddress, isPartial) == true) {
                return true;
            }
            break;
        case '}':
            break; // fail: unbalanced curly braces
        default:
            if ((**oscAddressPattern == **oscAddress) || (**oscAddressPattern == '?')) {
                (*oscAddressPattern)++;
                (*oscAddress)++;
                return true;
            }
            break;
    }
    *oscAddressPattern = oscAddressPatternCache;
    *oscAddress = oscAddressCache;
    return false;
}

/**
 * @brief Matches an OSC address pattern starting with a bracketed list of
 * characters with the next character in the target OSC address.
 *
 * The OSC address pattern must start with a '[' character.  A match will occur
 * if any of the characters in the bracketed list match the next character in
 * the OSC address.  If the first character in the list is '!' then the list
 * will be negated (i.e. a match will occur if the next character in the OSC
 * address does not appear in the bracketed list).  Any two characters separated
 * by a '-' character will represent the inclusive range of characters between
 * the two characters.  The two characters may be in acceding or descending
 * order. A bracketed list may contain both '!' and '-' characters and may also
 * include multiple character ranges.  For example, the OSC address pattern
 * "/abc[!d-hijkp-l]qrst" will match the OSC address "/abcXqrst" provided that
 * character X is not within the range 'd' to 'p'.
 *
 * The oscAddressPattern and oscAddress pointers are advanced to the character
 * proceeding the matched sequence if a match is found.  This function should
 * only be called internally by MatchCharacter.
 *
 * This is an internal function and cannot be called by the user application.
 *
 * @param oscAddressPattern First character of OSC address pattern.
 * @param oscAddress First character of target OSC address.
 * @return True if OSC address pattern and target OSC address match.
 */
static bool MatchBrackets(const char * * const oscAddressPattern, const char * * const oscAddress) {
    (*oscAddressPattern)++; // increment past opening bracket

    // Check if list is negated
    bool negatedList = false;
    if (**oscAddressPattern == '!') {
        negatedList = true;
        (*oscAddressPattern)++; // increment past '!'
    }

    // Match each character in list
    bool match = negatedList;
    while (**oscAddressPattern != ']') {
        if ((**oscAddressPattern == '/') || (**oscAddressPattern == '\0')) {
            return false; // fail: unbalanced brackets
        }

        // If character is part of hyphenated range
        if ((*(*oscAddressPattern + 1) == '-') && (*(*oscAddressPattern + 2) != ']')) {
            if ((*(*oscAddressPattern + 2) == '/') || (*(*oscAddressPattern + 2) == '\0')) {
                return false; // fail: unbalanced brackets
            }

            // Handle acceding/descending range
            char lowerChar = **oscAddressPattern;
            char upperChar = *(*oscAddressPattern + 2);
            if (lowerChar > upperChar) {
                lowerChar = *(*oscAddressPattern + 2);
                upperChar = **oscAddressPattern;
            }

            // Check if target character in range
            if ((**oscAddress >= lowerChar) && (**oscAddress <= upperChar)) {
                if (negatedList == true) {
                    match = false; // fail: character matched in negated list
                } else {
                    match = true;
                }
            }
            (*oscAddressPattern) += 3; // increment past hyphenated characters
        } else {

            // Else match single character
            if (**oscAddressPattern == **oscAddress) {
                if (negatedList == true) {
                    match = false; // fail: character matched in negated list
                } else {
                    match = true;
                }
            }
            (*oscAddressPattern)++;
        }
    }
    (*oscAddressPattern)++; // increment past closing bracket
    (*oscAddress)++; // increment past matched character
    return match;
}

/**
 * @brief Matches an OSC address pattern starting with a curly braced list of
 * substrings with the next character(s) in the target OSC address.
 *
 * The OSC address pattern must start with a '{' character.  A match will occur
 * if a substring with the curly braced list matches the next substring in the
 * target OSC address. If multiple substrings are matched then the longest
 * matching substring will be used.  A substring may contain zero characters.
 * For example, the OSC address pattern "/{in,out,,}puts/enable" would match OSC
 * addresses "/inputs/enable" and "/outputs/enable" and "/puts/enable".
 *
 * The oscAddressPattern and oscAddress pointers are advanced to the character
 * proceeding the matched sequence if a match is found.  This function should
 * only be called internally by MatchCharacter.
 *
 * This is an internal function and cannot be called by the user application.
 *
 * @param oscAddressPattern First character of OSC address pattern.
 * @param oscAddress First character of target OSC address.
 * @param isPartial Flag indicating if a partial match is acceptable.
 * @return True if OSC address pattern and target OSC address match.
 */
static bool MatchCurlyBraces(const char * * const oscAddressPattern, const char * * const oscAddress, const bool isPartial) {
    const char * endOfSubstring = *oscAddressPattern;
    size_t matchedSubStringLength = 0;
    bool match = false;
    while (**oscAddressPattern != '}') {
        if ((**oscAddressPattern == '/') || (**oscAddressPattern == '\0')) {
            return false; // fail: unbalanced curly braces
        }

        // Advance to end of substring
        while ((*endOfSubstring != ',') && (*endOfSubstring != '}')) {
            if ((*endOfSubstring == '/') || (*endOfSubstring == '\0')) {
                return false; // fail: unbalanced curly braces
            }
            endOfSubstring++;
        }

        // Determine substring length
        (*oscAddressPattern)++; // increment past '{' or ','
        size_t subStringLength = endOfSubstring - *oscAddressPattern;
        if (isPartial == true) {
            size_t oscAddressLength = strlen(*oscAddress);
            if (subStringLength > oscAddressLength) {
                subStringLength = oscAddressLength; // limit length to not exceed partial target
            }
        }

        // Match substring
        if (strncmp(*oscAddressPattern, *oscAddress, subStringLength) == 0) {
            match = true;
            if (subStringLength > matchedSubStringLength) {
                matchedSubStringLength = subStringLength;
            }
        }
        *oscAddressPattern = endOfSubstring; // advance to next ',' or '}'
        endOfSubstring++; // increment past ',' or '}'
    }
    (*oscAddressPattern)++; // increment past '{' or ','
    *oscAddress += matchedSubStringLength; // increment past matched substring
    return match;
}

/**
 * @brief Returns true if the OSC address pattern is literal.
 *
 * A literal OSC address pattern cannot contain any special characters: '?',
 * ' *', '[]', or '{}'.  In some applications it is desirable to reject OSC
 * address patterns that contain special characters because the use of special
 * characters risks invoking critical methods unintentionally.  For example,
 * critical methods such as "/shutdown" or "/selfdestruct" risk being invoked
 * unintentionally the OSC address pattern "/s *".
 *
 * Example use:
 * @code
 * OscMessage oscMessage;
 * OscMessageInitialise(&oscMessage, "/example/oscAddress/pattern");
 * if(OscAddressIsLiteral(oscMessage.oscAddressPattern)) {
 *     printf("Is literal");
 * }
 * @endcode
 *
 * @param oscAddressPattern First character of OSC address pattern.
 * @return True if the OSC address pattern is literal.
 */
bool OscAddressIsLiteral(const char * oscAddressPattern) {
    while (*oscAddressPattern != '\0') {
        switch (*oscAddressPattern) {
            case '?':
            case '*':
            case '[':
            case '{':
                return false;
            default:
                break;
        }
        oscAddressPattern++;
    }
    return true;
}

/**
 * @brief Returns the number of parts that make up an OSC address or an OSC
 * address pattern.
 *
 * The parts of an OSC address or an OSC address pattern are the substrings
 * between adjacent pairs of forward slash characters and the substring after
 * the last forward slash character.  This function may be called in conjunction
 * with OscAddressGetPartAtIndex to copy a given part.
 *
 * Example use:
 * @code
 * OscMessage oscMessage;
 * OscMessageInitialise(&oscMessage, "/example/oscAddress/pattern");
 * const unsigned int numberOfParts = OscAddressGetNumberOfParts(oscMessage.oscAddressPattern);
 * printf("Number of parts = %d", numberOfParts); // should be 3
 * @endcode
 *
 * @param oscAddressPattern OSC address or an OSC address pattern.
 * @return Number of parts that make up the message OSC address or an OSC
 * oscAddress pattern.
 */
unsigned int OscAddressGetNumberOfParts(const char * oscAddressPattern) {
    unsigned int numberOfParts = 0;
    while (*oscAddressPattern != '\0') {
        if (*oscAddressPattern == '/') {
            numberOfParts++;
        }
        oscAddressPattern++;
    }
    return numberOfParts;
}

/**
 * @brief Copies the OSC address or OSC address pattern part at the specified
 * index.
 *
 * The parts of an OSC Address or an OSC address pattern are the substrings
 * between adjacent pairs of forward slash characters and the substring after
 * the last forward slash character.  This functions copies the part at the
 * specified index (starting from zero) to a destination address.  The part is
 * provided as a null-terminated string. OscAddressGetNumberOfParts may be
 * called first to determine the number of parts available.
 *
 * Example use:
 * @code
 * OscMessage oscMessage;
 * OscMessageInitialise(&oscMessage, "/example/oscAddress/pattern");
 * const unsigned int numberOfParts = OscAddressGetNumberOfParts(oscMessage.oscAddressPattern);
 * unsigned int index;
 * for (index = 0; index < numberOfParts; index++) {
 *     char string[16];
 *     OscAddressGetPartAtIndex(oscMessage.oscAddressPattern, index, string, sizeof (string));
 *     printf("%s ", string);
 * }
 * @endcode
 *
 * @param oscAddressPattern OSC address or an OSC address pattern.
 * @param index Index (starting from zero) of part to be copied.
 * @param destination Destination oscAddress for part string.
 * @param destinationSize Destination size that cannot be exceeded.
 * @return Error code (0 if successful).
 */
OscError OscAddressGetPartAtIndex(const char * oscAddressPattern, const unsigned int index, char * const destination, const size_t destinationSize) {

    // Advance oscAddressPattern oscAddress to start of part
    unsigned int partCount = 0;
    while (partCount < (index + 1)) {
        while (*oscAddressPattern != '\0') {
            if (*oscAddressPattern == '/') {
                partCount++;
                break;
            }
            oscAddressPattern++;
        }
        if (*oscAddressPattern == '\0') {
            return OscErrorNotEnoughPartsInAddressPattern; // error: not enough parts in message oscAddress
        }
    }

    // Copy part to destination as string
    unsigned int destinationIndex = 0;
    while (*oscAddressPattern != '\0') {
        destination[destinationIndex] = *oscAddressPattern;
        if (++destinationIndex >= destinationSize) {
            return OscErrorDestinationTooSmall; // error: destination too small
        }
        oscAddressPattern++;
        if (*oscAddressPattern == '/') {
            break;
        }
    }
    destination[destinationIndex] = '\0'; // terminate as string
    return OscErrorNone;
}

//------------------------------------------------------------------------------
// End of file
