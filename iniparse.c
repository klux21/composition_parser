/*****************************************************************************\
*                                                                             *
*  FILENAME :    iniparse.c                                                   *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
*  DESCRIPTION : Contains diverse character buffer parsing functions          *
*                for a buffer that uses the data composition format.          *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
*  COPYRIGHT :   (c) 2026 Dipl.-Ing. Klaus Lux (Aachen, Germany)              *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
*  ORIGIN :      https://github.com/klux21/composition_parser                 *
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>

#pragma warning( disable : 4996)
#include <io.h>
#include <malloc.h>
#define strncasecmp strnicmp
#define stat  _stat64
#define fstat _fstat64
#define lseek _lseeki64

#else /* ! _WIN32 */

#include <unistd.h>

#ifndef _cdecl
#define _cdecl
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#endif /* #ifdef _WIN32 */

#include <stdint.h>

#include <iniparse.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#define  vPrintLog(...)

static const uint8_t ini_char_type[256] = { 0x08,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x02,0x02,0x02, 0x02,0x02,0x00,0x00,
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                                           /*       !    "    #     $    %    &    '       (    )    *    +     ,    -    .    /   */
                                            0x03,0x00,0x10,0x04, 0x00,0x00,0x00,0x10,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                                           /*  0    1    2    3     4    5    6    7       8    9    :    ;     <    =    >    ?   */
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x04, 0x00,0x01,0x00,0x00,
                                           /*  @    A    B    C     D    E    F    G       H    I    J    K     L    M    N    O   */
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                                           /*  P    Q    R    S     T    U    V    W       X    Y    Z    [     \    ]    ^    _   */
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x08, 0x10,0x08,0x00,0x00,
                                           /*  `    a    b    c     d    e    f    g       h    i    j    k     l    m    n    o   */
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                                           /*  p    q    r    s     t    u    v    w       x    y    z    {     |    }    ~        */
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x01, 0x00,0x08,0x00,0x00,

                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,

                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
                                            0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,   0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00};

#define IS_INI_CHAR(x)               (!ini_char_type[(uint8_t)(x)])        /* characters without any special function */
#define IS_INI_ARG(x)                (ini_char_type[(uint8_t) (x)] & 0x01) /* entry argument = { */
#define IS_INI_BLANK(x)              (ini_char_type[(uint8_t) (x)] & 0x02) /* white spaces */
#define IS_INI_COMMENT(x)            (ini_char_type[(uint8_t) (x)] & 0x04) /* comment start markers ; #   */
#define IS_INI_SECT_END(x)           (ini_char_type[(uint8_t) (x)] & 0x08) /* section end indicators [ ] } '\0' */
#define IS_INI_QUOTE                 (ini_char_type[(uint8_t) (x)] & 0x10) /* backslash \ or quote ' " */

#define IS_INI_SPECIAL(x)            (ini_char_type[(uint8_t) (x)] & 0x0f) /* terminates unquoted strings '\0',' ','\t','\n','\f','\v','\r','#',';','[',']','{','}','=' */
#define IS_INI_BLANK_OR_COMMENT(x)   (ini_char_type[(uint8_t) (x)] & 0x06) /* whitespace or start of a comment */


/* table for fast decoding ascii encoded numbers i.e. if (Digit[c] < 10) i = Digit[c]; */
static const uint8_t digit_value[256] = { 64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64,   64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64,
                                      /*      !  "  #  $   %  &  '   (   )  *  +  ,  -  .  /     0  1  2  3   4  5  6  7   8  9  :  ;   <  =  >  ?  */
                                          64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64,    0, 1, 2, 3,  4, 5, 6, 7,  8, 9,64,64, 64,64,64,64,
                                      /*   @  A  B  C   D  E  F  G   H  I  J  K   L  M  N  O     P  Q  R  S   T  U  V  W   X  Y  Z  [   \  ]  ^  _  */
                                          64,10,11,12, 13,14,15,16, 17,18,19,20, 21,22,23,24,   25,26,27,28, 29,30,31,32, 33,34,35,64, 64,64,64,64,
                                      /*   `  a  b  c   d  e  f  g   h  i  j  k   l  m  n  o     p  q  r  s   t  u  v  w   x  y  z  {   |  }  ~     */
                                          64,10,11,12, 13,14,15,16, 17,18,19,20, 21,22,23,24,   25,26,27,28, 29,30,31,32, 33,34,35,64, 64,64,64,64,

                                          64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64,   64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64,
                                          64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64,   64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64,
                                          64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64,   64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64,
                                          64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64,   64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64 };


/* ------------------------------------------------------------------------- *\
   strmemcmp compares a string content with a buffer content of a given size
   and returns like strcmp if any differences are found.
   (A strncmp() succeeds if the string is longer than the size.)
\* ------------------------------------------------------------------------- */

int strmemcmp(const char * pStr, const void * pBuffer, size_t BufferSize)
{
   const char * pb = (const char*) pBuffer;
   const char * ps = pStr ? pStr : "";

   while(BufferSize && *ps && (*ps == *pb))
   {
      --BufferSize;
      ++ps;
      ++pb;
   }

   if(!BufferSize)
       pb = "";

   return (((int) (unsigned char) * ps) - (unsigned char) * pb);
}/* int strmemcmp(...) */


/* ------------------------------------------------------------------------- *\
   Extracts a C style character from a string and moves the string pointer to
   the end of the C style character definition. It doesn't moves the pointer
   behind the terminating '\0' of the string.
\* ------------------------------------------------------------------------- */

static char get_C_char(const char ** ppSrc)
{
   const uint8_t * ps;
   uint8_t c = 0;
   uint8_t d;

   if(!ppSrc || !*ppSrc)
     goto Exit;

   ps = *(uint8_t **) ppSrc;
   c  = *ps;

   if(c == '\\')
   {
      ps++;
      c = (uint8_t) *ps++;
      switch(c) /* check character case insensitive */
      {
         case '\0': --ps;     break; /* string terminator */
         case 'a' : c = '\a'; break; /* alert */
         case 'b' : c = '\b'; break; /* back space */
         case 'd' : /* decimal byte value */
                    d = digit_value[*ps];
                    if(d < 10)
                    {
                       c = d;
                       d = digit_value[*(++ps)];
                       if(d < 10)
                       {
                          c = (c * 10) + d;
                          ++ps;
                          if(c <= 25)
                          {
                             d = digit_value[*ps];
                             if(d <= 5)
                             {
                                c = (c * 10) + d;
                                ++ps;
                             }
                          }
                       }
                    }
                    break;
         case 'e' : c = 0x1b; break; /* escape */
         case 'f' : c = '\f'; break; /* formfeed */
         case 'n' : c = '\n'; break; /* new line */
         case 'r' : c = '\r'; break; /* carriage return */
         case 't' : c = '\t'; break; /* horizontal tab */
         case 'u' : --ps; c = '\\'; break; /* \u must be kept */
         case 'v' : c = '\v'; break; /* vertical tab */
         case 'x' : /* hexadecimal byte value */
                    d = digit_value[*ps];
                    if(d < 16)
                    {
                       c = d;
                       d = digit_value[*(++ps)];
                       if(d < 16)
                       {
                          c = (c << 4) + d;
                          ++ps;
                       }
                    }
                    break;
         default  : if((uint8_t) (c - '0') < 8)
                    { /* octal byte value */
                       c -= '0';
                       d = digit_value[*ps];
                       if(d < 8)
                       {
                          ++ps;
                          c = (c << 3) + d;
                          if(c < 32)
                          {
                             d = digit_value[*ps];
                             if(d < 8)
                             {
                                ++ps;
                                c = (c << 3) + d;
                             }
                          }
                       }
                    }
                    break; /* return the character */
      }
   }
   else if(c) /* prevent moving the pointer behind the end of string */
   {
      ++ps;
   }

   *ppSrc = (char *) ps;
   Exit:;
   return (c);
}/* get_C_char(char ** pSrc) */



/* ------------------------------------------------------------------------- *\
   pIniFindCommentEnd returns a pointer to first character the comment.
\* ------------------------------------------------------------------------- */

static char * pIniFindCommentEnd(const char * pb)
{
   char s = *pb; /* comment start character */

   if(!IS_INI_COMMENT(s))
      goto Exit;

   if(*(++pb) == '*')
   { /* a block comment starts with #* (or ;*) ends with *# (or *;) or at the end of the data */
      do
      {
         if(*(++pb) == '*')
         {  /* end of comment? */
            if(*(++pb) == s)
            { /* end of comment found */
               ++pb;
               break;
            }
         }
      } while(*pb);
   }
   else
   {  /* line comment starts with #* (or ;*) and ends at the end of the current line */
      while (*pb && (*pb++ != '\n'))
      {};
   }

   Exit:;

   return ((char *) pb);
}/* const char * pIniFindCommentEnd(const char * pb) */



/* ------------------------------------------------------------------------- *\
   pSkipBlanksAndComments: helper for ignoring blanks and comments
\* ------------------------------------------------------------------------- */

static char * pSkipBlanksAndComments(const char * pb)
{
   if(!pb)
      goto Exit;

   /* ignore tabs and spaces and other separating blanks */
   while(IS_INI_BLANK(*pb)) /* " \t\n\f\v\r" are treated as entry separating whitespaces */
      ++pb;

   while(IS_INI_COMMENT(*pb))
   {
      pb = pIniFindCommentEnd(pb);

      if(!*pb)
         goto Exit; /* end of document */

      while(IS_INI_BLANK(*pb)) /* " \t\n\f\v\r" are treated as entry separating whitespaces */
        ++pb;
   }

   Exit:;

   return ((char *) pb);
}/* const char * pSkipBlanksAndComments(const char * pb) */



/* ------------------------------------------------------------------------- *\
   pFindBlockEnd returns the pointer the character that terminates the block 
\* ------------------------------------------------------------------------- */

static char * pIniFindBlockEnd (const char * pb)
{
   size_t BlockDepth = 1;

   if(!pb)
      goto Exit;

   while (1)
   {
      while (IS_INI_CHAR(*pb))
         ++pb;  /* ignore normal characters */

      if(IS_INI_BLANK_OR_COMMENT(*pb))
      {
         pb = pSkipBlanksAndComments(pb);
         continue;
      }

      if(*pb == '\\')
      {  /* ignore characters that are escaped except '\0' */
         if(! *(++pb))
            goto Exit; /* end of document */
      }
      else if((*pb == '\'')  || (*pb == '\"'))
      {/* ignore  characters inside of quoted strings */
         char c = *pb;

         while(*(++pb) != c)
         {
            if(*pb == '\\')
               ++pb;

            if(!*pb)
               goto Exit; /* end of document */
         }
      }
      else if(*pb == '}')
      {
         if(!-- BlockDepth)
         {
            ++pb;
            goto Exit;
         }
      }
      else if(*pb == '{')
      {
         ++BlockDepth;
      }
      else if(*pb == '[')
      {  /* Let's skip sections headers. */
         if(IS_INI_BLANK_OR_COMMENT(*(++pb)))
             pb = pSkipBlanksAndComments(pb);

         /* let's find the end of the section header */
         while(*pb != ']')
         {
            if(*pb == '\\')
            { /* ignore all characters that are escaped except '\0' */
               if(!*(++pb))
                  goto Exit; /* unexpected end of document */
            }
            else if((*pb == '\'')  || (*pb == '\"'))
            {/* a part of the section name is quoted and must be ignored */
               char c = *pb;

               while(*(++pb) != c)
               {
                  if(*pb == '\\')
                     ++pb;

                  if(!*pb)
                     goto Exit; /* unexpected end of document */
               }
           }
           else if(!*pb)
              goto Exit; /* unexpected end of document */

           ++pb;

           if(IS_INI_BLANK_OR_COMMENT(*pb))
              pb = pSkipBlanksAndComments(pb);
         }
      }
      else if(!*pb)
         goto Exit; /* end of document */

      ++pb;
   }

   Exit:;
   return((char *) pb);
} /* char * pIniFindBlockEnd (const char * pBlock) */


/* ------------------------------------------------------------------------- *\
   bIniEntryFind searches the next entry in a section and returns nonzero if
   an entry was found. * ppData will be set to to the begin of the next
   section entry or to the character that terminates the section.
   Argument strings to entries which contain spaces have to be enclosed in
   "" or '' in the section.
   Name or argument embracing quotes or braces will be not removed.
\* ------------------------------------------------------------------------- */

int bIniEntryFind(char **  ppData,      /* section data pointer */
                  char **  ppName,      /* returned pointer to entry name */
                  size_t * pNameSize,   /* will be set to the length of the name */
                  char **  ppArg,       /* pointer to the entries parameter string */
                  size_t * pArgSize)    /* length of parameter string */
{
   int bRet       = 0;
   char * pd      = NULL; /* data pointer for temporary usage */
   char c         = 0;    /* char for temporary usage */
   char * pName   = NULL; /* pointer to name */
   char * pArg    = NULL; /* pointer to argument string */
   size_t NameLen = 0;    /* length of name */
   size_t ArgLen  = 0;    /* argument length */

   if(!ppData || !*ppData)
      goto Exit; /* invalid argument pointer */

   /* let's find begin of next entry */
   pd = pSkipBlanksAndComments(*ppData);

   if(IS_INI_SECT_END(*pd)) /* [ ] } '\0' */
      goto Exit; /* end of section found */

   pName = pd; /* begin of entry name found, remember pointer */

   /* find end of the name */
   while(!IS_INI_SPECIAL(*pd))
   {/* find end of entry name */
      if(*pd == '\\')
      {
         if(!*(++pd)) /* ignore c style characters */
            break;
      }
      else if((*pd == '\'')  || (*pd == '\"'))
      {/* entry name given in quotes */
         c = *pd;

         while(*(++pd) != c)
         {
            if(*pd == '\\')
               ++pd;

            if(!*pd)
               break; /* end of document */
         }
      }

      ++pd; /* add character to string */
   }

   NameLen = (size_t)(pd - pName); /* calculate name length */

   /* end of name, find argument */
   pd = pSkipBlanksAndComments(pd);

   if(*pd == '=')
   {/* argument string found */
      pd = pSkipBlanksAndComments(pd + 1); /* skip '=' and subsequent blanks */

      if (*pd == '{')
      { /* subblock found */
         pArg   = pd;
         pd     = pIniFindBlockEnd(pd + 1);

         ArgLen = (size_t)(pd - pArg); /* calculate length but treat the block terminating character as part of the argument */

         if(*pd)
            pd  = pSkipBlanksAndComments(pd); /* skip subsequent blanks */
         else
            ++ArgLen;
      }
      else
      {
         pArg = pd;

         while(!IS_INI_SPECIAL(*pd))
         {/* find end of entries argument string */
            if(*pd == '\\')
            {
               if(!*(++pd)) /* ignore c style characters */
                  break;
            }
            else if((*pd == '\'')  || (*pd == '\"'))
            {/* entry name given in quotes */
               c = *pd;

               while(*(++pd) != c)
               {
                  if(*pd == '\\')
                     ++pd;

                  if(!*pd)
                     break; /* end of document */
               }

               if(!*pd)
                  break; /* end of document */
            }

            ++pd;
         }

         ArgLen = (size_t)(pd - pArg);    /* calculate length of argument string */
         pd = pSkipBlanksAndComments(pd); /* skip subsequent blanks */
      }
   }/* argument string found */
   else if ((*pd == '{') && !NameLen)
   { /* subsequent subblock found */
      pArg   = pd;
      pd     = pIniFindBlockEnd(pd + 1);
      ArgLen = (size_t)(pd - pArg); /* calculate length but treat the block terminating character as part of the argument */

      if(*pd)
         pd  = pSkipBlanksAndComments(pd); /* skip subsequent blanks */
      else
         ++ArgLen;
   }

   if(NameLen || ArgLen)
   {
      bRet = 1;

      if(ppName)
         *ppName = pName;

      if(pNameSize)
         *pNameSize = NameLen;

      if(ppArg)
         *ppArg = pArg;

      if(pArgSize)
         *pArgSize = ArgLen;

      if(*ppData) /* let's find begin of next entry or next section and store it into pData */
         *ppData = pd;

      vPrintLog(DFL_DBG,"Found entry: name[%lu] = \'%.*s\'   arg[%lu] = \'%.*s\'",
                (unsigned long) NameLen, (int) NameLen, pName, (unsigned long) ArgLen, (int) ArgLen, pArg);
   }

Exit:;

   if(!bRet && pd)
   { /* move the iterating data pointer to the terminating character */
      if(*ppData)
         *ppData = pd;
   }

   return (bRet);
}/* int bIniEntryFind(...) */



/* ------------------------------------------------------------------------- *\
  pIniFindNextSection searches the next section in given INI file buffer.
  It returns a pointer to the begin of the sections data or NULL if there
  doesn't exist any section within the data.
\* ------------------------------------------------------------------------- */

char * pIniFindNextSection(const char * pData,             /* INI file data buffer */
                           char **      ppSectionName,     /* pointer to section name */
                           size_t *     pSectionNameSize)  /* pointer to section name length */
{
   const char * pcRet           = NULL;
   char *       pd              = (char *) pData;
   char *       pSectionName    = NULL;
   size_t       SectionNameSize = 0;

   /* vPrintLog(DFL_ERR,"Searching section \"%s\"!", Name); */

   if(!pd)
      goto Exit;

   while(bIniEntryFind(&pd, NULL, NULL, NULL, NULL))
   { /* skip all INI file entries */
   }

   if(*pd == '[')
   {  /* Found a section header. Let's find the name and the end of it... */
      if(IS_INI_BLANK_OR_COMMENT(*(++pd)))
          pd = pSkipBlanksAndComments(pd);

      pSectionName = pd;

      /* let's find the end of the section header */
      while(*pd != ']')
      {
         if(*pd == '\\')
         { /* we must ignore nonzero characters after a backslash */
            if(!*(++pd)) 
               goto Exit; /* unexpected end of document */
         }
         else if((*pd == '\'')  || (*pd == '\"'))
         {/* a part of the section name was given in quotes */
            char c = *pd;

            while(*(++pd) != c)
            {
               if(*pd == '\\')
                  ++pd;

               if(!*pd)
                  goto Exit; /* unexpected end of document */
            }
         }
         else if(!*pd)
            goto Exit; /* unexpected end of document */

         ++pd;

         SectionNameSize = pd - pSectionName;

         if(IS_INI_BLANK_OR_COMMENT(*pd))
            pd = pSkipBlanksAndComments(pd);
      }

      /* Skip section header terminating ']' found. Skip all blanks and comments at begin of that section. */
      /* pcRet points to the begin of the first entry or end of section afterwards. */
      pcRet = pSkipBlanksAndComments(pd + 1);

      if(ppSectionName)
         *ppSectionName = pSectionName;

      if(pSectionNameSize)
         *pSectionNameSize = SectionNameSize;
   }

Exit:;
   return ((char *) pcRet);
}/* pIniFindNextSection(char * pData, char** ppSectionName, size_t * pSectionNameSize) */


/* ------------------------------------------------------------------------- *\
  pIniFindSection searches a section in a given string. It returns a pointer
  to the begin of the sections data or NULL is the section doesn't exist.
\* ------------------------------------------------------------------------- */

char * pIniFindSection(const char * pData, /* INI file data buffer */
                       const char * pName) /* section name */
{
   const char * pRet      = NULL;
   size_t       NameLen   = 0;
   char *       FoundName = NULL;
   size_t       FoundLen  = 0;

   if(!pData || !*pData || !pName)
      goto Exit;

   /* vPrintLog(DFL_ERR,"Searching section \"%s\"!", Name); */

   if(!*pName)
   {
      pRet = pData;
      goto Exit;
   }

   NameLen = strlen(pName);

   while(pData && *pData)
   {
      pData = pIniFindNextSection(pData, &FoundName, &FoundLen);
      /* vPrintLog(DFL_DBG,"### section [%.*s]: '%.20s ...'",
      (int)FoundLen, FoundName, pData);  */

      if(NameLen == FoundLen)
      {
         if(!strncasecmp(pName, FoundName, FoundLen))
         {
            pRet = pData;
            goto Exit; /* section found */
         }
      }
      else if(FoundLen > NameLen)
      { /* convert special characters and ignore quotation marks in the found string */
         const char * pn = pName;
         const char * ps = FoundName;
         char quote = '\0';

         while((size_t)(ps - FoundName) < FoundLen)
         {
            if(quote && (*ps == quote))
            {/* end of quotation found, remember it and ignore the character */
               ++ps;
               quote = '\0';
            }
            else if (!quote && ((*ps == '\'') || (*ps == '\"')))
            {/* begin of quotation found, remember it and ignore the character */
               quote = *ps++;
            }
            else
            {
               if((size_t)(pn - pName) >= NameLen)
                  break; /* Searched name is shorter then the found one */

               if(*pn++ != get_C_char(&ps))
                  break;
            }
         }

         if((pn == (pName + NameLen)) &&
            (ps == (FoundName + FoundLen)))
         { /* searched section found */
            pRet = pData;
            break;
         }

         /* vPrintLog(DFL_DBG,"!!! Section [%.*s|%.*s] != [%.*s|%.*s]",
                      pn - pName, pName,(int)(NameLen - (pn - pName)), pn,
                      ps - FoundName, FoundName, (int)(FoundLen - (ps - FoundName)), ps); */
      }/* if(FoundLen > NameLen) */
   }/* while(*pData) */

Exit:;

   if(pRet)
      vPrintLog(DFL_DBG,"Found section [%s]", pName ? pName : "");
   else
      vPrintLog(DFL_DBG,"Section [%s] not found!", pName ? pName : "");

   return ((char *) pRet);
}/* pIniFindSection(char * pData, char * pName) */


/* ------------------------------------------------------------------------- *\
   lIniGetStringValue converts an unterminated C style escaped string from
   INI file buffer to it's related value. DstLen has to be at maximum as large
   as SrcLen + 1 for a terminating '\0' character. The function returns the
   number of written bytes. Source and Destination buffer may overlap.
   If the destination is NULL than the required buffer size is returned only.
\* ------------------------------------------------------------------------- */

size_t lIniGetStringValue(char *       pDst,   /* pointer to destination buffer */
                          const char * pSrc,   /* pointer to source buffer */
                          size_t       SrcLen) /* length of the source buffer */
{
   const char * ps = pSrc;
   char       * pd = pDst;
   char         q  = '\0';

   if(!pSrc)
   {
      if(pDst)
         *pd++='\0';
      else
         ++pd;
   }

   else if(!pDst)
   {
      while(*ps && ((size_t)(ps - pSrc) < SrcLen))
      {
         if(*ps == '\\')
         {
            get_C_char(&ps); /* get C style char */
            ++pd;
         }
         else if ((*ps == '\'') || (*ps == '\"'))
         { /* remove quotes */
            if(q)
            {/* trailing quote? */
               if(q == *ps)
               {
                  q = '\0';
                  ++ps;
               }
               else
               { /* it's the other type of quote */
                  ++ps;
                  ++pd;
               }
            }
            else
            {/* leading quote found */
               q = *ps++;
            }
         }
         else
         {
            ++ps;
            ++pd;
         }
      }

      ++pd;
   }
   else
   {
      while(*ps && ((size_t)(ps - pSrc) < SrcLen))
      {
         if(*ps == '\\')
         {
            *pd++ = get_C_char(&ps); /* get C style char */
         }
         else if ((*ps == '\'') || (*ps == '\"'))
         { /* remove quotes */
            if(q)
            {/* trailing quote? */
               if(q == *ps)
               {
                  q = '\0';
                  ++ps;
               }
               else
               { /* it's the other type of quote */
                  *pd++ = *ps++;
               }
            }
            else
            {/* leading quote found */
               q = *ps++;
            }
         }
         else
         {
            *pd++ = *ps++;
         }
      }

      *pd++ = '\0';
   }

   return (size_t)(pd - pDst); /* written size */
}/* lIniGetStringValue */


/* ------------------------------------------------------------------------- *\
   lIniRemoveQuotes removes the quotes from a value read from INI file without
   replacing C like escaped characters. The destination buffer length has to
   be at least as large as SrcLen + 1 (for a terminating '\0' character).
   The function returns the  number of bytes written to the destination string
   including the terminating '\0'. Source and destination buffer may overlap.
   If the destination is NULL than the required buffer size is returned only.
\* ------------------------------------------------------------------------- */

size_t lIniRemoveQuotes(char *       pDst,   /* pointer to destination buffer */
                        const char * pSrc,   /* pointer to source buffer */
                        size_t       SrcLen) /* length of source buffer */
{
   const char * ps = pSrc;
   char *       pd = pDst;
   char         q  = '\0';

   if(!pSrc)
   {
      if(pDst)
         *pd++='\0';
      else
         ++pd;
   }
   else if(!pDst)
   {
      while(*ps && SrcLen--)
      {
         if ((*ps == '\'') || (*ps == '\"'))
         { /* remove quotes */
            if(q)
            {/* trailing quote? */
               if(q == *ps)
               {
                  q = '\0';
                  ++ps;
               }
               else
               { /* it's the other type of quote */
                  ++ps;
                  ++pd;
               }
            }
            else
            {/* leading quote found */
               q = *ps++;
            }
         }
         else
         {
            ++ps;
            ++pd;
         }
      }
      ++pd;
   }
   else
   {
      while(*ps && SrcLen--)
      {
         if ((*ps == '\'') || (*ps == '\"'))
         { /* remove quotes */
            if(q)
            {/* trailing quote? */
               if(q == *ps)
               {
                  q = '\0';
                  ++ps;
               }
               else
               { /* it's the other type of quote */
                  *pd++ = *ps++;
               }
            }
            else
            {/* leading quote found */
               q = *ps++;
            }
         }
         else
         {
            *pd++ = *ps++;
         }
      }
      *pd++ = '\0';
   }

   return ((size_t)(pd - pDst)); /* written size */
}/* lIniRemoveQuotes */


/* ------------------------------------------------------------------------- *\
   bIniEntryRead reads the next entry within an INI file section. It returns
   a pointer to an allocated PINI_ENTRY structure that contains the data of
   the first found entry. It fails, if no entry could be found before begin
   of the next section, if a terminating '\0' character was found or if the
   allocation of the return buffer has failed.
   If bUnescape is nonzero, then the entry name and its argument data
   are converted from a C like escape sequence format to binary format.
   ppData is set to the data that follows this entry withing the INI file.
   The returned pointer must be released using free() after usage.
\* ------------------------------------------------------------------------- */

int bIniEntryRead(char **      ppData,    /* pointer to INI file data */
                  INI_ENTRY ** ppEntry,   /* storage for the allocated pointer */
                  int          bUnescape) /* wether or not replace C style format escapes */
{
   INI_ENTRY * pEntry   = NULL;
   char *      pd       = NULL;
   char *      pName    = NULL;
   size_t      NameSize = 0;
   char *      pArg     = NULL;
   size_t      ArgSize  = 0;

   if(!ppData || !*ppData || !ppEntry) /* parameter check */
      goto Exit;

   pd = *ppData;

   if(!bIniEntryFind(&pd,       /* section data to be read (In+Out) */
                     &pName,    /* returned pointer to entry name */
                     &NameSize, /* returned length of the name */
                     &pArg,     /* returned pointer to the value string */
                     &ArgSize)) /* returned length of the value string */
   {
      goto Exit;  /* nothing left to be read */
   }

   pEntry = (INI_ENTRY *) malloc(sizeof(*pEntry) + NameSize + ArgSize + 2);
   if(!pEntry)
   {
      vPrintLog(DFL_ERR,"Out of memory while reading infile parameter! (%s)",
                strerror(errno));
      goto Exit;
   }

   memset(pEntry, 0, sizeof(*pEntry));

   pEntry->pName = (char*) pEntry + sizeof(*pEntry);
   pEntry->pArg  = pEntry->pName + NameSize + 1;

   if(bUnescape)
   {
      pEntry->NameSize = lIniGetStringValue(pEntry->pName, pName, NameSize) - 1; /* ignore the terminating '/0' in returned size */

      if((ArgSize) && (*pArg == '{'))
      { /* keep unchanged content of subblock */
         ArgSize -= 2;

         memcpy(pEntry->pArg, pArg+1, ArgSize);
         pEntry->pArg[ArgSize] = '\0';
         pEntry->ArgSize = ArgSize;
      }
      else
      {
         pEntry->ArgSize = lIniGetStringValue(pEntry->pArg, pArg, ArgSize) - 1; /* ignore the the terminating '/0' in returned size */
      }
   }
   else
   {/* Let's simply copy the data from the INI file */
      pEntry->NameSize = lIniRemoveQuotes(pEntry->pName, pName, NameSize) - 1; /* ignore the terminating '/0' in returned size */

      if((ArgSize) && (*pArg == '{'))
      { /* keep unchanged content of subblock */
         ArgSize -= 2;

         memcpy(pEntry->pArg, pArg+1, ArgSize);
         pEntry->pArg[ArgSize] = '\0';
         pEntry->ArgSize = ArgSize;
      }
      else
      {
         pEntry->ArgSize = lIniRemoveQuotes(pEntry->pArg, pArg, ArgSize) - 1; /* ignore the the terminating '/0' in returned size */
      }
   }

   *ppData  = pd;
   *ppEntry = pEntry;

   Exit:;

   return (pEntry != NULL);
}/* int bIniEntryRead(char ** ppData, INI_ENTRY ** ppEntry, int bUnescape) */



/* ------------------------------------------------------------------------- *\
   pIniFileRead reads the content of an INI file into an allocated buffer.
   The returned buffer is terminated by a '\0' character and must be released
   using free() after usage.
\* ------------------------------------------------------------------------- */

char * pIniFileRead(const char * pIniFileName)
{
   char *  pRet     = NULL;
   struct  stat st;
   int64_t Size     = 0;
   int64_t FileSize = 0;
   int     fd       = -1;
   char *  pb       = NULL;

   if(!pIniFileName || !*pIniFileName)
      goto Exit; /* we can't handle this */

   do
   {
      fd = open(pIniFileName, O_RDONLY | O_BINARY | O_CLOEXEC, 0);
   } while((fd == -1) && (errno == EINTR)); /* retry if interrupted by a signal */

   if(fd == -1)
   {
      vPrintLog(DFL_WRN,"Failed to open INI file '%s'! (%s)", pIniFileName, strerror(errno));
      goto Exit;
   }

   if(!fstat(fd, &st))
      FileSize = st.st_size;

   if(FileSize <= 0)
   {
      vPrintLog(DFL_WRN,"Inifile '%s' is empty! (%s)", pIniFileName, strerror(errno));
      goto Exit;
   }

   pRet = (char*) malloc((size_t) FileSize + 1);
   if(!pRet)
   {
      vPrintLog(DFL_ERR,"Out of memory while reading the INI file '%s'! (%s)", pIniFileName, strerror(errno));
      goto Exit;
   }

   pb = pRet;
   do
   {
      Size = read(fd, pb, (int) ( FileSize > 0x100000 ? 0x100000 : FileSize ));
      if((Size == -1) && (errno == EINTR))
         continue;

      if(Size < 0)
         break;

      FileSize -= Size;
      pb       += Size;
   } while (FileSize > 0);

   if(FileSize)
   {
      vPrintLog(DFL_ERR,"Error while reading the INI file '%s'! (%s)", pIniFileName, strerror(errno));

      free(pRet);
      pRet = NULL;

      goto Exit;
   }

   *pb = '\0'; /* terminate the buffer */

   Size = (size_t) (pb - pRet);

   pb = pRet;

   while(Size --)
   { /* Ensure that there are no buffer terminating '\0' bytes within the file content */
      if(!*pb)
         *pb = ' ';

      ++pb;
   }

Exit:;

   if(fd != -1)
   {
      while(close(fd) && (errno == EINTR))
      { /* just retry because interrupted by signal */
      }
      fd = -1;
   }

   return (pRet);
}/* char * pIniFileRead(char * pIniFileName) */


/* ========================================================================= *\
   E N D   O F   F I L E
\* ========================================================================= */
