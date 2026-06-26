/*****************************************************************************\
*                                                                             *
*  FILENAME :     iniparse.h                                                  *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
*  DESCRIPTION :  Contains diverse inifile character buffer parsing functions *
*                 for a buffer in composition format.                         *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
*  COPYRIGHT :   (c) 2026 Dipl.-Ing. Klaus Lux (Aachen, Germany)              *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
*  ORIGIN :      https://github/klux21/composition_parser                     *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
* This software is provided 'as-is', without any express or implied           *
* warranty. In no event will the authors be held liable for any damages       *
* arising from the use of this software.                                      *
*                                                                             *
* Permission is granted to anyone to use this software for any purpose,       *
* including commercial applications, and to alter it and redistribute it      *
* freely, subject to the following restrictions:                              *
*                                                                             *
* 1. The origin of this software must not be misrepresented; you must not     *
*    claim that you wrote the original software. If you use this software     *
*    in a product, an acknowledgment in the product documentation would be    *
*    appreciated but is not required.                                         *
* 2. Altered source versions must be plainly marked as such, and must not be  *
*    misrepresented as being the original software.                           *
* 3. This notice may not be removed or altered from any source distribution.  *
*                                                                             *
\*****************************************************************************/

#ifndef INIPARSE_H
#define INIPARSE_H

#ifdef _WIN32
#include <windows.h>   /* required for HMODULE */
#elif defined __CYGWIN__
#include <windows.h>   /* required for HMODULE */
#else

#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- *\
   The struct INI_ENTRY holds the data of an inifile entry
\* ------------------------------------------------------------------------- */

typedef struct INI_ENTRY_S INI_ENTRY;
struct INI_ENTRY_S
{
   char * pName;      /* entry name */
   size_t NameSize;   /* length of name string (without trailing '\0') */
   char * pArg;       /* argument value of the entry */
   size_t ArgSize;    /* size of the argument data (without trailing '\0') */
};

/* ------------------------------------------------------------------------- *\
   pIniFileRead reads the content of an inifile into an allocated buffer.
   The returned buffer is terminated by a '\0' character and must be released
   using free() after usage.
\* ------------------------------------------------------------------------- */
char * pIniFileRead(const char * IniFileName);

/* ------------------------------------------------------------------------- *\
   pIniFindSection searches a section in an inifile buffer. It returns a
   pointer to the begin of the sections content or nonzero if the section
   doesn't exist.
\* ------------------------------------------------------------------------- */
char * pIniFindSection(const char * pData,  /* pointer to an inifile data buffer to read */
                       const char * pName); /* inifile section name */

/* ------------------------------------------------------------------------- *\
  pIniFindNextSection searches the next section in a given inifile buffer
  It returns a pointer to the begin of the sections data or NULL if there
  doesn't exist any next section within the data.
\* ------------------------------------------------------------------------- */
char * pIniFindNextSection(const char * pData,             /* inifile data buffer to scan */
                           char **      ppSectionName,     /* in success case: section name */
                           size_t *     pSectionNameSize); /* length of the section name */

/* ------------------------------------------------------------------------- *\
   bIniEntryRead reads the next entry within an inifile section and returns
   in success case an pointer to an allocated PINI_ENTRY structure
   containing the data of an found entry. It fails, if no entry can be found
   before begin of the next section or if a terminating '\0' character was
   found or if the allocation of the returned data buffer has failed.
   If bUnescape is nonzero then the name and the arguments are converted
   from a C like escape sequence format to binary format.
   ppData becomes set to the data following the entry within the inifile.
   The returned pointer must be released using free() after usage.
\* ------------------------------------------------------------------------- */

int bIniEntryRead(char **      ppData,     /* pointer to inifile data */
                  INI_ENTRY ** ppEntry,    /* storage for the allocated pointer */
                  int          bUnescape); /* wether or not replace C style format escapes */

/* ------------------------------------------------------------------------- *\
   bIniEntryFind searces the next entry in a section and returns nonzero if
   an entry was found. * ppData will be set to to the begin of the next
   section entry or to the character that terminates the section.
   Argument strings to entries which contain spaces have to be enclosed in
   "" or '' in the section.
   Name or argument embracing quotes or braces will be not removed.
\* ------------------------------------------------------------------------- */

int bIniEntryFind(char **  ppData,    /* section data pointer */
                  char **  ppName,    /* returned pointer to entry name */
                  size_t * pNameSize, /* will be set to the length of the name */
                  char **  ppArg,     /* pointer to the entries parameter string */
                  size_t * pArgSize); /* length of parameter string */

/* ------------------------------------------------------------------------- *\
   lIniGetStringValue converts an unterminated C style escaped string from
   inifile buffer to it's related value. DstLen has to be at maximum as large
   as SrcLen + 1 for a terminating '\0' character. The function returns the
   number of written bytes. Source and Destination buffer may overlap.
   If the destination is NULL than the required buffer size is returned only.
\* ------------------------------------------------------------------------- */

size_t lIniGetStringValue(char *       pDst,    /* pointer to destination buffer */
                          const char * pSrc,    /* pointer to source buffer */
                          size_t       SrcLen); /* length of the source buffer */

/* ------------------------------------------------------------------------- *\
   lIniRemoveQuotes removes the quotes from a value read from inifile without
   replacing C like escaped characters. The destination buffer length has to
   be at least as large as SrcLen + 1 (for a terminating '\0' character).
   The function returns the  number of bytes written to the destination string
   including the terminating '\0'. Source and destination buffer may overlap.
   If the destination is NULL than the required buffer size is returned only.
\* ------------------------------------------------------------------------- */

size_t lIniRemoveQuotes(char *       pDst,    /* pointer to destination buffer */
                        const char * pSrc,    /* pointer to source buffer */
                        size_t       SrcLen); /* length of source buffer */

/* ------------------------------------------------------------------------- *\
   strmemcmp compares a string content with a buffer content of a given size
   and returns like strcmp if any differences are found.
   (A strncmp() succeeds if the string is longer than the size.)
\* ------------------------------------------------------------------------- */

int strmemcmp(const char * pStr, const void * pBuffer, size_t BufferSize);

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif/* INIPARSE_H */


/* ========================================================================= *\
   E N D   O F   F I L E
\* ========================================================================= */
