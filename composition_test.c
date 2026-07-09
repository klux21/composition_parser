/*****************************************************************************\
*                                                                             *
*  FILENAME:      composition_test.c                                          *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
*  DESCRIPTION:   tests and benchmark of composition format parser functions  *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
*  COPYRIGHT:     (c) 2026 Dipl.-Ing. Klaus Lux (Aachen, Germany)             *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
*  ORIGIN:        https://github.com/klux21/composition_parser                *
*                                                                             *
* --------------------------------------------------------------------------- *
*                                                                             *
* Civil Usage Public License, Version 1.2, June 2026                          *
*                                                                             *
* Redistribution and use in source and binary forms, with or without          *
* modification, are permitted provided that the following conditions are met: *
*                                                                             *
* 1. Redistributions of source code must retain the above copyright           *
*    notice, this list of conditions, the explanation of terms                *
*    and the following disclaimer.                                            *
*                                                                             *
* 2. Redistributions in binary form must reproduce the above copyright        *
*    notice, this list of conditions and the following disclaimer in the      *
*    documentation or other materials provided with the distribution.         *
*                                                                             *
* 3. All modified files must carry prominent notices stating that the         *
*    files have been changed.                                                 *
*                                                                             *
* 4. The source code and binary forms and any derivative works are not        *
*    stored or executed in systems or devices which are designed or           *
*    intended to harm, to kill or to forcibly immobilize people.              *
*                                                                             *
* 5. The source code and binary forms and any derivative works are not        *
*    stored or executed in systems or devices which are intended to           *
*    monitor, to track, to change or to control the behavior, the             *
*    constitution, the location or the communication of any people or         *
*    their property without the explicit and prior agreement of those         *
*    people except those devices and systems are solely designed for          *
*    saving or protecting peoples life or health.                             *
*                                                                             *
* 6. The source code and binary forms and any derivative works are not        *
*    stored or executed in any systems or devices that are intended           *
*    for the production of any of the systems or devices that                 *
*    have been stated before except the ones for saving or protecting         *
*    peoples life or health only.                                             *
*                                                                             *
* The term 'systems' in all clauses shall include all types and combinations  *
* of physical, virtualized or simulated hardware and software and any kind    *
* of data storage.                                                            *
*                                                                             *
* The term 'devices' shall include any kind of local or non-local control     *
* system of the stated devices as part of that device as well. Any assembly   *
* of more than one device is one and the same device regarding this license.  *
*                                                                             *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  *
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   *
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        *
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    *
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     *
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     *
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  *
* POSSIBILITY OF SUCH DAMAGE.                                                 *
*                                                                             *
\*****************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <errno.h>
#if defined (_WIN32) || defined (__CYGWIN__)

#include <windows.h>

#if defined (_WIN32)
#include <io.h>
#include <fcntl.h>

#define strtoll     _strtoi64
#define strtoull    _strtoui64
#endif

#else
#include <time.h>
#include <sys/time.h>
#endif
#include <inttypes.h>

#if 1
#include <sfprintf.h>
#else
#define sfprintf fprintf
#endif

#include <str2num.h>

#include <iniparse.h> /* header of the data composition parser */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

#if defined (_WIN32)

static UINT (WINAPI * pGetConsoleOutputCP)()        = NULL;
static BOOL (WINAPI * pSetConsoleOutputCP)(UINT CP) = NULL;

int bInitCPFunctions()
{
   int bRet = 0;
   static HMODULE hmKernel32Dll = 0;

   if(!hmKernel32Dll)
   {
      hmKernel32Dll = LoadLibrary(TEXT("Kernel32.dll"));
      if(hmKernel32Dll)
      {
         pGetConsoleOutputCP = (UINT (WINAPI * )())    GetProcAddress(hmKernel32Dll, "GetConsoleOutputCP");
         pSetConsoleOutputCP = (BOOL (WINAPI *)(UINT)) GetProcAddress(hmKernel32Dll, "SetConsoleOutputCP");
         bRet = 1;
      }
   }
   return (bRet);
} /* bInitCPfuntiontions() */

static UINT GetWindowsConsoleOutputCP()
{
   UINT uRet = 0;

   bInitCPFunctions();

   if(pGetConsoleOutputCP)
      uRet = pGetConsoleOutputCP();

   return (uRet);
} /* UINT GetWindowsConsoleOutputCP() */

static UINT SetWindowsConsoleOutputCP(UINT CP)
{
   UINT uRet = 0;

   bInitCPFunctions();

   if(pSetConsoleOutputCP)
      uRet = pSetConsoleOutputCP(CP);

   return (uRet);
} /* UINT SetWindowsConsoleOutputCP(UINT CP) */
#endif

#if defined (_WIN32) || defined (__CYGWIN__)

/* ------------------------------------------------------------------------- *\
   nsTimeStamp() returns the Unix time in nanoseconds since 01/01/1970
\* ------------------------------------------------------------------------- */

int64_t nsTimeStamp()
{
   static void (WINAPI * vGetSystemTimePreciseAsFileTime)(LPFILETIME lpSystemTimeAsFileTime);
   static HMODULE hmKernel32Dll = (HMODULE) -1;

   int64_t iRet;
   FILETIME CurrentTime;

   if(vGetSystemTimePreciseAsFileTime)
   {
      vGetSystemTimePreciseAsFileTime(&CurrentTime);
   }
   else if(hmKernel32Dll == (HMODULE) -1)
   { /* 1st call */
      hmKernel32Dll = LoadLibrary("Kernel32.dll");
      if(hmKernel32Dll)
         vGetSystemTimePreciseAsFileTime = (void (WINAPI * )(LPFILETIME)) GetProcAddress(hmKernel32Dll, "GetSystemTimePreciseAsFileTime");

      if(vGetSystemTimePreciseAsFileTime)
         vGetSystemTimePreciseAsFileTime(&CurrentTime);
      else
         GetSystemTimeAsFileTime(&CurrentTime);
   }
   else
   {
      GetSystemTimeAsFileTime(&CurrentTime);
   }

   iRet  = ((int64_t) CurrentTime.dwHighDateTime << 32);
   iRet += (int64_t)  CurrentTime.dwLowDateTime;
   iRet -= (int64_t)  116444736 * 1000000 * 1000; /* offset of Windows FileTime to start of Unix time */
   return (iRet * 100);
}/* int64_t nsTimeStamp() */

#else

int64_t nsTimeStamp()
{
   int64_t tm;
   struct timespec ts;

   if(0 >= clock_gettime(CLOCK_REALTIME, &ts))
   {
      tm = ts.tv_sec * 1000000000ul + ts.tv_nsec;
   }
   else
   {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      tm = (int64_t) tv.tv_sec;
      if((sizeof(time_t) <= 4) && (tv.tv_sec < 0))
         tm += (int64_t) 0x80000000ul + (int64_t) 0x80000000ul; /* handle year 2038 problem by making it a year 2106 problem */
      tm *= 1000000000ul;
      tm += tv.tv_sec * 1000;
   }
   return (tm);
}/* int64_t nsTimeStamp() */
#endif




/* ------------------------------------------------------------------------- *\
  Here we iterate the top level entries. bIniEntryFind returns the pointers
  to the raw data in memory. In case of data composition type subentries
  the data contains the opening curly braces of the block only is the
  option bFindBlockEnd is set to 0 otherwise the block termination is
  searched and pArg contains the whole block including the terminating
  character. If the closing brace is missing and the document is terminated
  by a '\0' character then the terminating '\0' is returned instead of the
  closing brace.
\*------------------------------------------------------------------------- */

int run_zero_copy_tests()
{
   int    iRet = 0;

   char * pName;       /* returned pointer to entry name */
   size_t NameSize;    /* will be set to the length of the name */
   char * pArg;        /* pointer to the entries parameter string */
   size_t ArgSize;     /* length of parameter string */

   char * pd = (char*) "testname = zero_copy_tests \n"
                       "inttests = { ib=0b1111 io=0o1234567 id=000056789 ix=0xabcd987 } \n"
                       "floattests = { fb=0b11.11e100 fo=0o1234.56e10 fd=1.2345e64 fx=0xabc.defp10 } \n";
   char * ps  = pd;

   char * pe  = NULL;
   int    err = 0;

   int64_t t0 =  nsTimeStamp();
   int64_t t1 =  nsTimeStamp();

   size_t  count;
   double  db  = 0.0;
   int64_t i64 = 0;

   sfprintf(stdout, "%4d: zero_copy parsing tests of\n\"\n%s\n\"\n", __LINE__, pd);

   while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize, 0))
   {
      if(ArgSize && (*pArg == '{'))
      { /* subblock found */
         ArgSize = pIniFindBlockEnd (ps) - pArg; /* required for subsequent block size check */

         if(!strmemcmp("inttests", pName, NameSize))
         {
            sfprintf(stdout, "%4d: zero_copy: found '%.*s = %.*s'\n", __LINE__, (int) NameSize, pName, (int)  ArgSize, pArg);

            if (ArgSize != 52)
            {
               sfprintf(stderr, "%4d: zero_copy: unexpected ArgSize of %zu instead of %u bytes!\n", __LINE__, ArgSize, (int) 53);
               goto Exit;
            }

            while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize, 0))
            {
               sfprintf(stdout, "%4d: zero_copy: found    '%.*s = %.*s' (%lld)\n",
                        __LINE__, (int) NameSize, pName, (int) ArgSize, pArg, (long long) str2i64_r(pArg, &pe, 1, &err));
            }
         }
         else if(!strmemcmp("floattests", pName, NameSize))
         {
            sfprintf(stdout, "%4d: zero_copy: found '%.*s = %.*s'\n", __LINE__, (int)  NameSize, pName, (int) ArgSize, pArg);

            if (ArgSize != 63)
            {
               sfprintf(stderr, "%4d: zero_copy: unexpected ArgSize of %zu instead of %u bytes!\n", __LINE__, ArgSize, (int)  53);
               goto Exit;
            }

            while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize, 0))
            {
               sfprintf(stdout, "%4d: zero_copy: found    '%.*s = %.*s' (%e)\n", __LINE__,
                        (int) NameSize, pName, (int)  ArgSize, pArg, str2d_r(pArg, &pe, 1, &err));
            }
         }
         else
         {
            sfprintf(stderr, "%4d: zero_copy: unexpected subblock '%.*s' found!\n", __LINE__, (int) NameSize, pName);
            goto Exit;
         }

         if(*ps == '}')
             ++ps; /* continue after the end of the subblock */
      }
      else if(!strmemcmp("testname", pName, NameSize))
      {
         sfprintf(stdout, "%4d: zero_copy: found '%.*s = %.*s'\n", __LINE__, (int) NameSize, pName, (int) ArgSize, pArg);

         if (strmemcmp ("zero_copy_tests",  pArg, ArgSize))
         {
            sfprintf(stderr, "%4d: zero_copy: unexpected argument!\n", __LINE__);
            goto Exit;
         }
      }
      else
      {
         sfprintf(stderr, "%4d: zero_copy: unexpected entry '%.*s = %.*s' found!\n", __LINE__, (int) NameSize, pName, (int) ArgSize, pArg);
         goto Exit;
      }
   }

   if(ps != (pd + strlen(pd)))
   {
      sfprintf(stderr, "\n%4d: basic zero_copy tests failed because the did't stop at buffer end!\n",  __LINE__);
      goto Exit;
   }

   sfprintf(stdout, "\n%4d: basic zero_copy tests succeeded, testing performance ...\n",  __LINE__);

   count = 1000;
   i64 = 0;
   t0 = nsTimeStamp();

   while (count--)
   {
      ps = pd;
      while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize, 0))
      {
         if(ArgSize && (*pArg == '{'))
         { /* subblock found */
            if(!strmemcmp("inttests", pName, NameSize))
            {
               while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize, 0))
               {
                  ++i64;
               }
            }
            else if(!strmemcmp("floattests", pName, NameSize))
            {
               while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize, 0))
               {
                  ++i64;
               }
            }
            else
            {
               sfprintf(stderr, "%4d: zero_copy: unexpected subblock '%.*s' found!\n", __LINE__, (int) NameSize, pName);
               goto Exit;
            }

            if(*ps == '}')
                ++ps; /* continue after the end of the subblock */
         }
         else if(!strmemcmp("testname", pName, NameSize))
         {
            if (strmemcmp ("zero_copy_tests",  pArg, ArgSize))
            {
            }
         }
         else
         {
            sfprintf(stderr, "%4d: zero_copy: unexpected entry '%.*s = %.*s' found!\n", __LINE__, (int) NameSize, pName, (int) ArgSize, pArg);
            goto Exit;
         }
      }
   }
   t1 = nsTimeStamp();

   t1 = (t1 - t0) / 1000;
   i64 /= 1000;

   sfprintf(stdout, "%4d: parsing without reading the %lld values took %lld nanoseconds.\n", __LINE__, (long long) i64, (long long) t1);

   count = 1000;
   t0 = nsTimeStamp();

   while (count--)
   {
      ps = pd;
      while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize, 0))
      {
         if(ArgSize && (*pArg == '{'))
         { /* subblock found */
            if(!strmemcmp("inttests", pName, NameSize))
            {
               while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize, 0))
               {
                  i64 = str2i64_r(pArg, &pe, 1, &err);
                  if(err || ((ptrdiff_t) ArgSize != (pe - pArg)))
                  {
                     sfprintf(stdout, "%4d: zero_copy: failed to read '%.*s = %.*s'! (read='%.*s', ret=%lld, err=%d (%s))\n",
                              __LINE__, (int) NameSize, pName, (int) ArgSize, pArg,
                              (int) (pe - pArg), pArg, (long long)i64, err, strerror(err));
                     goto Exit;
                  }
               }
            }
            else if(!strmemcmp("floattests", pName, NameSize))
            {
               while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize, 0))
               {
                  db = str2d_r(pArg, &pe, 1, &err);
                  if(err || ((ptrdiff_t) ArgSize != (pe - pArg)))
                  {
                     sfprintf(stdout, "%4d: zero_copy: failed to read '%.*s = %.*s'! (read='%.*s', ret=%e, err=%d, %s)\n",
                              __LINE__, (int) NameSize, pName, (int) ArgSize, pArg,
                              (int) (pe - pArg), pArg, db, err, strerror(err));
                     goto Exit;
                  }
               }
            }
            else
            {
               sfprintf(stderr, "%4d: zero_copy: unexpected subblock '%.*s' found!\n", __LINE__, (int) NameSize, pName);
               goto Exit;
            }

            if(*ps == '}')
                ++ps; /* continue after the end of the subblock */
         }
         else if(!strmemcmp("testname", pName, NameSize))
         {
            if (strmemcmp ("zero_copy_tests",  pArg, ArgSize))
            {
            }
         }
         else
         {
            sfprintf(stderr, "%4d: zero_copy: unexpected entry '%.*s = %.*s' found!\n", __LINE__, (int) NameSize, pName, (int) ArgSize, pArg);
            goto Exit;
         }
      }
   }

   t1 = nsTimeStamp();

   t1 = (t1 - t0) / 1000;
   sfprintf(stdout, "%4d: parsing with reading 4 int64_t and 4 double values into variables took %lld nanoseconds.\n\n", __LINE__, (long long) t1);

   iRet = 1;
Exit:;

   sfprintf(stdout, "%4d: zero_copy tests %s!\n\n", __LINE__, iRet ? "succeeded" : "failed");
   return(iRet);
}/* run_zero_copy_tests() */



/* ------------------------------------------------------------------------- *\
   Here we iterate the top level entries using bIniEntryRead.
   bIniEntryRead returns a copy of the optionally unescaped string data or
   the raw content of a block if the entries argument is a block.
   In that case the data does not include the enclosing curly braces nor any
   other block terminating character except a string terminatin '\0'.
\* ------------------------------------------------------------------------- */

int run_string_copy_tests()
{
   int    iRet = 0;

   INI_ENTRY * pEntry  = NULL;
   INI_ENTRY * pEntry2 = NULL;

   char * pd = (char*) "testname = string_copy_tests \n"
                       "inttests = { ib=0b1111 io=0o1234567 id=000056789 ix=0xabcd987 } \n"
                       "floattests = { fb=0b11.11e100 fo=0o1234.56e10 fd=1.2345e64 fx=0xabc.defp10 } \n";

   char * ps  = pd;
   char * pa  = NULL;
   char * pe  = NULL;
   int    err = 0;

   int64_t t0 =  nsTimeStamp();
   int64_t t1 =  nsTimeStamp();

   uint32_t top_entries = 0;
   uint32_t sub_entries = 0;

   size_t  count;
   double  db  = 0.0;
   int64_t i64 = 0;

   #define found(e, x)  (e) += 1 + (1l << (16 + (x)))

   sfprintf(stdout, "%4d: string_copy parsing tests of\n\"\n%s\n\"\n", __LINE__, pd);

   while(bIniEntryRead(&ps, &pEntry, 0 /* bUnescape can be set to 0 or 1 but we don't use any escapes here */))
   {
      if(!strmemcmp("testname", pEntry->pName, pEntry->NameSize))
      {
         found(top_entries, 0);
         sfprintf(stdout, "%4d: string_copy: found '%.*s = %.*s'\n", __LINE__,
                  (int) pEntry->NameSize, pEntry->pName, (int) pEntry->ArgSize, pEntry->pArg);

         if (strmemcmp ("string_copy_tests", pEntry->pArg, pEntry->ArgSize))
         {
            sfprintf(stderr, "%4d: string_copy: unexpected argument!\n", __LINE__);
            goto Exit;
         }
      }
      else if(!strmemcmp("inttests", pEntry->pName, pEntry->NameSize))
      {
         found(top_entries, 1);
         sfprintf(stdout, "%4d: string_copy: found    '%.*s = %.*s'\n", __LINE__,
                  (int) pEntry->NameSize, pEntry->pName, (int) pEntry->ArgSize, pEntry->pArg);

         if (pEntry->ArgSize != 50)
         {
            sfprintf(stderr, "%4d: string_copy: unexpected ArgSize of %zu instead of %u bytes!\n", __LINE__, pEntry->ArgSize, (int) 53);
            goto Exit;
         }

         pa = pEntry->pArg;
         while(bIniEntryRead(&pa, &pEntry2, 0 /* bUnescape */))
         {
            found(sub_entries, 1);
            sfprintf(stdout, "%4d: string_copy: found    '%.*s = %.*s' (%lld)\n", __LINE__,
                     (int) pEntry2->NameSize, pEntry2->pName, (int) pEntry2->ArgSize, pEntry2->pArg,
                     (long long) str2i64_r(pEntry2->pArg, &pe, 1, &err));
            free(pEntry2);
         }
      }
      else if(!strmemcmp("floattests", pEntry->pName, pEntry->NameSize))
      {
         found(top_entries, 2);
         sfprintf(stdout, "%4d: string_copy: found '%.*s = %.*s'\n", __LINE__,
                  (int)  pEntry->NameSize, pEntry->pName, (int) pEntry->ArgSize, pEntry->pArg);

         if (pEntry->ArgSize != 61)
         {
            sfprintf(stderr, "%4d: string_copy: unexpected ArgSize of %zu instead of %u bytes!\n", __LINE__, pEntry->ArgSize, (int)  53);
            goto Exit;
         }

         pa = pEntry->pArg;
         while(bIniEntryRead(&pa, &pEntry2, 0 /* bUnescape */))
         {
            found(sub_entries, 1);
            sfprintf(stdout, "%4d: string_copy: found    '%.*s = %.*s' (%e)\n", __LINE__, 
                     (int) pEntry2->NameSize, pEntry2->pName, (int) pEntry2->ArgSize, pEntry2->pArg, str2d_r(pEntry2->pArg, &pe, 1, &err));
            free(pEntry2);
         }
      }
      else
      {
         sfprintf(stderr, "%4d: string_copy: unexpected entry '%.*s = %.*s' found!\n", __LINE__,
                  (int) pEntry->NameSize, pEntry->pName, (int) pEntry->ArgSize, pEntry->pArg);
         goto Exit;
      }

      free(pEntry);
   }

   sfprintf(stdout, "\n%4d: basic string_copy tests succedeed, testing performance ...\n",  __LINE__);

   count = 1000;
   i64 = 0;
   t0 = nsTimeStamp();

   while (count--)
   {
      ps = pd;
      while(bIniEntryRead(&ps, &pEntry, 0 /* bUnescape */))
      {
         if(!strmemcmp("testname", pEntry->pName, pEntry->NameSize))
         {
            if (strmemcmp ("string_copy_tests", pEntry->pArg, pEntry->ArgSize))
            {
            }
         }
         else if(!strmemcmp("inttests", pEntry->pName, pEntry->NameSize))
         {
            pa = pEntry->pArg;
            while(bIniEntryRead(&pa, &pEntry2, 0 /* bUnescape */))
            {
               ++i64;
               free(pEntry2);
            }
         }
         else if(!strmemcmp("floattests", pEntry->pName, pEntry->NameSize))
         {
            pa = pEntry->pArg;
            while(bIniEntryRead(&pa, &pEntry2, 0 /* bUnescape */))
            {
               ++i64;
               free(pEntry2);
            }
         }
         else
         {
            sfprintf(stderr, "%4d: string_copy: unexpected entry '%.*s = %.*s' found!\n", __LINE__,
                     (int) pEntry->NameSize, pEntry->pName, (int) pEntry->ArgSize, pEntry->pArg);
            goto Exit;
         }

         free(pEntry);
      }
   }
   t1 = nsTimeStamp();

   t1 = (t1 - t0) / 1000;
   i64 /= 1000;

   sfprintf(stdout, "%4d: parsing without reading the %lld values took %lld nanoseconds.\n", __LINE__, (long long) i64, (long long) t1);

   count = 1000;
   t0 = nsTimeStamp();

   while (count--)
   {
      ps = pd;
      while(bIniEntryRead(&ps, &pEntry, 0 /* bUnescape */))
      {
         if(!strmemcmp("testname", pEntry->pName, pEntry->NameSize))
         {
            if (strmemcmp ("string_copy_tests", pEntry->pArg, pEntry->ArgSize))
            {
            }
         }
         else if(!strmemcmp("inttests", pEntry->pName, pEntry->NameSize))
         {
            pa = pEntry->pArg;
            while(bIniEntryRead(&pa, &pEntry2, 0 /* bUnescape */))
            {
               i64 = str2i64_r(pEntry2->pArg, &pe, 1, &err);
               if(err || ((ptrdiff_t) pEntry2->ArgSize != (pe - pEntry2->pArg)))
               {
                  sfprintf(stdout, "%4d: string_copy: failed to read '%.*s = %.*s'! (read='%.*s', ret=%lld, err=%d (%s))\n",
                           __LINE__, (int) pEntry2->NameSize, pEntry2->pName, (int) pEntry2->ArgSize, pEntry2->pArg,
                           (int) (pe - pEntry2->pArg), pEntry2->pArg, (long long)i64, err, strerror(err));
                  goto Exit;
               }
               free(pEntry2);
            }
         }
         else if(!strmemcmp("floattests", pEntry->pName, pEntry->NameSize))
         {
            pa = pEntry->pArg;
            while(bIniEntryRead(&pa, &pEntry2, 0 /* bUnescape */))
            {
               db = str2d_r(pEntry2->pArg, &pe, 1, &err);
               if(err || ((ptrdiff_t) pEntry2->ArgSize != (pe - pEntry2->pArg)))
               {
                  sfprintf(stdout, "%4d: string_copy: failed to read '%.*s = %.*s'! (read='%.*s', ret=%e, err=%d, %s)\n",
                           __LINE__, (int) pEntry2->NameSize, pEntry2->pName, (int) pEntry2->ArgSize, pEntry2->pArg,
                           (int) (pe - pEntry2->pArg), pEntry2->pArg, db, err, strerror(err));
                  goto Exit;
               }
               free(pEntry2);
            }
         }
         else
         {
            sfprintf(stderr, "%4d: string_copy: unexpected entry '%.*s = %.*s' found!\n", __LINE__,
                     (int) pEntry->NameSize, pEntry->pName, (int) pEntry->ArgSize, pEntry->pArg);
            goto Exit;
         }

         free(pEntry);
      }
   }

   t1 = nsTimeStamp();

   t1 = (t1 - t0) / 1000;
   sfprintf(stdout, "%4d: parsing with reading 4 int64_t and 4 double values into variables took %lld nanoseconds.\n\n", __LINE__, (long long) t1);

   iRet = 1;
Exit:;

   sfprintf(stdout, "%4d: string_copy tests %s!\n\n", __LINE__, iRet ? "succeeded" : "failed");
   return(iRet);
}/* run_string_copy_tests() */


#ifdef SFPRINTF_H
/* ------------------------------------------------------------------------- *\
   pvc_unescape is a helper for printing unquoted and unescaped strings with
   the help of a %.*v option of callback_printf that is used below.

   That way we don't need to modify the buffer using an inplace unescaping
   nor to make copy of the data using a strndup for replacing the escapes
   sequences.

   This function can be used for printing the unescaped and unquoted
   configuration entries to a FILE pointer, a file descriptor or a text buffer
   in a fprintf or snprintf like way and even do that using a minimum width
   and left justified or right justified output.
   A %v in the format string expects a callback function pointer like
   &pvc_unescape and a pointer to the data that it prints as arguments.
   However the zero-copy strings are unterminated, so the data length must
   be provided too and the precision argument is used for that.
   Some simple usage samples of that

   _sfprintf(stderr, "%.*v", (int) NameSize, pvc_unescape, pName);
   _sfdprintf(fileno(stderr), "%-8.*v", (int) NameSize, pvc_unescape, pName);
   _ssnprintf(buffer, 16, "%15.*v", (int) NameSize, pvc_unescape, pName);
\* ------------------------------------------------------------------------- */

size_t pvc_unescape (void *            pUserData,
                     PRINTF_CALLBACK * pCB,
                     void *            pvdata,
                     size_t            precision,
                     size_t            minimum_width,
                     uint8_t           left_justified,
                     uint8_t           prefixing)
{
   size_t       sz         = 0;
   size_t       max_length = precision; /* pvc_unescape handles the data like %.*s */
   const char * pv         = (const char *) pvdata;
   const char * pb         = pv;
   const char * ps         = pv;
   char         q          = '\0'; /* enclosing quote if within a string */
   char         c;

   if(!left_justified && minimum_width)
   {
      while(max_length-- && *ps)
      {
         if(*ps == '\\')
         {  /* escape sequence found */
            if(pb != ps)
               sz += ps - pb;

            pb = ps;

            if(!max_length)
                break;

            get_C_char(&ps);
            ++sz;

            if(max_length <= (size_t)(ps - pb - 1)) /* -1 because max_length was decremented already */
               break;

            max_length -= (size_t)(ps - pb - 1);
            pb = ps;
         }
         else if ((*ps == '\"') || (*ps == '\''))
         { /* remove quotes */
            if(q)
            {/* trailing quote? */
               if(q == *ps)
               {
                  if(pb != ps)
                     sz += ps - pb;

                  q = '\0';
                  pb = ++ps;
               }
               else
               { /* it's the other type of quote */
                  ++ps;
               }
            }
            else
            {/* leading quote found */
               if(pb != ps)
                  sz += ps - pb;

               q = *ps++;
               pb = ps;
            }
         }
         else
             ++ps;
      }

      if(pb != ps)
         sz += ps - pb;

      if(sz < minimum_width)
         sz = cbk_print_string(pUserData, pCB, "", 0, minimum_width - sz, 0); /* fill in the missing blanks and remember that length  */
      else
         sz = 0; /* no real output done until here */

      max_length = precision;
      pb = pv;
      ps = pv;
   }

   while(max_length-- && *ps)
   {
      if(*ps == '\\')
      {  /* escape sequence found */
         if(pb != ps)
            sz += cbk_print_string(pUserData, pCB, pb,(size_t) (ps - pb), 0, 0);

         pb = ps;

         if(!max_length)
            break;

         c = get_C_char(&ps);

         sz += cbk_print_string(pUserData, pCB, &c, 1, 0, 0);

         if(max_length <= (size_t)(ps - pb - 1)) /* -1 because max_length was decremented already */
         {
            pb = ps;
            break;
         }
         max_length -= (size_t)(ps - pb - 1);
         pb = ps;
      }
      else if ((*ps == '\"') || (*ps == '\''))
      { /* remove quotes */
         if(q)
         {/* trailing quote? */
            if(q == *ps)
            {
               if(pb != ps)
                  sz += cbk_print_string(pUserData, pCB, pb,(size_t) (ps - pb), 0, 0);

               q = '\0';
               pb = ++ps;
            }
            else
            { /* it's the other type of quote */
               ++ps;
            }
         }
         else
         {/* leading quote found */
            if(pb != ps)
               sz += cbk_print_string(pUserData, pCB, pb,(size_t) (ps - pb), 0, 0);

            q = *ps;
            pb = ++ps;
         }
      }
      else
          ++ps;
   }

   if(pb != ps)
      sz += cbk_print_string(pUserData, pCB, pb,(size_t) (ps - pb), 0, 0);

   return (sz);
} /* size_t pvc_unescape (...) */
#endif


/* ------------------------------------------------------------------------- *\
   The file tests are reading and iterating a test file
\* ------------------------------------------------------------------------- */

int run_file_iteration_tests()
{
   int iRet = 0;

   char * pName;       /* returned pointer to entry name */
   size_t NameSize;    /* will be set to the length of the name */
   char * pArg;        /* pointer to the entries parameter string */
   size_t ArgSize;     /* length of parameter string */

   char * pSectionName    = NULL;
   size_t SectionNameSize = 0;

   const char * pIniFile   = "./composition_test.ini";
   char *       pIniData   = pIniFileRead(pIniFile);
   char *       ps         = pIniData;

   size_t Indent = 0;

   if(!pIniData)
   {
      sfprintf(stderr, "%4d: failed to open the file '%s'! (err=%d, %s)\n", __LINE__, pIniFile, errno, strerror(errno));  
      goto Exit;
   }

   sfprintf(stdout, "%4d: file '%s' successfully opened.\n", __LINE__,  pIniFile);  

   /* Let's iterate the file content */
   while (*ps)
   {
      while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize, 0))
      {
         if(ArgSize && (*pArg == '{'))
         {
#ifndef SFPRINTF_H
            if(NameSize != lIniGetStringValue(NULL, pName, NameSize) - 1)
               NameSize = lIniGetStringValue(pName, pName, NameSize) - 1; /* -> this terminates and invalidates our source buffer */

            sfprintf(stdout, "\n%4d: found block    %*s%.*s%s{ #* %zu bytes *#\n",
                     __LINE__, (int)(Indent*3),"", (int) NameSize, pName, NameSize ? " = " : "", pIniFindBlockEnd (pArg + 1) - pArg + 2);
#else
            /* Using _sfprintf we can leave the buffer unchanged and print the the data using %v and pvc_unescape */
            _sfprintf(stdout, "\n%4d: found block    %*s%.*v%s{ #* %zu bytes *#\n",
                      __LINE__, (int)(Indent*3),"", (int) NameSize, &pvc_unescape, pName, NameSize ? " = " : "", pIniFindBlockEnd (pArg + 1) - pArg);
#endif
            ps = pArg + 1;
            ++Indent;
         }
         else
         {
#ifndef SFPRINTF_H
            /* Removing quotes or replacing escapes always shortens a string.
               For this we test whether the length changes before letting lIniGetStringValue terminating the string.
               This way we can work zero-copy and remove quotes and escapes in place. */

            if(NameSize != lIniGetStringValue(NULL, pName, NameSize) - 1)
               NameSize = lIniGetStringValue(pName, pName, NameSize) - 1; /* -> this terminates and invalidates our source buffer */

            if(ArgSize != lIniGetStringValue(NULL, pArg, ArgSize) - 1)
               ArgSize = lIniGetStringValue(pArg, pArg, ArgSize) - 1; /* -> this terminates and invalidates our source buffer */

            sfprintf(stdout, "%4d: found entry    %*s%.*s%s%.*s\n",
                     __LINE__, (int)(Indent*3), "", (int) NameSize, pName ? pName : "\"\"",
                     ArgSize ? " = " : "", (int) ArgSize, pArg);
#else
            /* Using _sfprintf we can leave the buffer unchanged and print the the data using %v and pvc_unescape */
            _sfprintf(stdout, "%4d: found entry    %*s%.*v%s%.*v\n",
                      __LINE__, (int)(Indent*3), "", (int) NameSize, &pvc_unescape, pName ? pName : "\"\"",
                     ArgSize ? " = " : "", (int) ArgSize, &pvc_unescape, pArg);
#endif
         }
      }

      if(pIniFindNextSection(&ps, &pSectionName, &SectionNameSize))
      {
#ifndef SFPRINTF_H
         if(SectionNameSize != lIniGetStringValue(NULL, pSectionName, SectionNameSize) - 1)
            SectionNameSize = lIniGetStringValue(pSectionName, pSectionName, SectionNameSize) - 1; /* -> this terminates and invalidates our source buffer */

         sfprintf(stdout, "\n%4d: found section  %*s[%.*s]\n", __LINE__, (int)(Indent*3), "", (int) SectionNameSize, pSectionName ? pSectionName : "");
#else
         /* Using _sfprintf we can leave the buffer unchanged and print the the data using %v and pvc_unescape */
         _sfprintf(stdout, "\n%4d: found section  %*s[%.*v]\n", __LINE__, (int)(Indent*3), "", (int) SectionNameSize, &pvc_unescape, pSectionName ? pSectionName : "");
#endif
      }
      else
      {
         if(Indent)
         {
            --Indent;
            sfprintf(stdout, "%4d:   block end    %*s%.1s\n", __LINE__, (int)(Indent*3), "", ps);

            if(*ps != '}')
               sfprintf(stdout, "%4d: unexpected char! ('%.1s' 0x%x)!\n", __LINE__, ps, (unsigned int) *ps);
            else
               ++ps;
         }
         else if(*ps)
         {
            sfprintf(stdout, "%4d: unexpected file end ('%.1s' 0x%x)!\n", __LINE__, ps, (unsigned int) *ps);
            break;
         }
      }
   }

   iRet = 1;
   Exit:;

   if(pIniData)
      free(pIniData);

   sfprintf(stdout, "%4d: file iteration test %s!\n\n", __LINE__, iRet ? "succeeded" : "failed");

   return (iRet);
} /* run_file_iteration_tests() */



/* ------------------------------------------------------------------------- *\
   main function
\* ------------------------------------------------------------------------- */

int main(int argc, char * argv[])
{
    int iRet = 1;

#if defined(_WIN32) && !defined(__CYGWIN__)
    UINT OldCP = GetWindowsConsoleOutputCP(); /* default console input codepage usually 850 */
    _setmode(_fileno(stdin), _O_U8TEXT);
    SetWindowsConsoleOutputCP(65001);
#endif

    if(!run_file_iteration_tests())
        goto Exit;

    if(!run_string_copy_tests())
        goto Exit;

    if(!run_zero_copy_tests())
        goto Exit;

    iRet = 0;

    Exit:;

#if defined(_WIN32) && !defined(__CYGWIN__)
    SetWindowsConsoleOutputCP(OldCP);
    _setmode(_fileno(stdin), _O_TEXT);
#endif

    if(!iRet)
        sfprintf(stdout, "All tests passed!\n");
    else
        sfprintf(stderr, "Tests failed!\n");

    return (iRet);
}/* main() */


/* ========================================================================= *\
   E N D   O F   F I L E
\* ========================================================================= */
