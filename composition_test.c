#if 0
rm -f ./_composition_test
cc -Wall -s -O3 -o _composition_test -I . -I ../str2num -I ../callback_printf ../str2num/str2num.c ../callback_printf/callback_printf.c  ../callback_printf/sfprintf.c iniparse.c composition_test.c
./_composition_test
exit $?
#endif

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

#if defined (_WIN32) || defined (__CYGWIN__)

static void (WINAPI * vGetSystemTimePreciseAsFileTime)(LPFILETIME lpSystemTimeAsFileTime);
static HMODULE hmKernel32Dll = (HMODULE) -1;

/* ------------------------------------------------------------------------- *\
   nsTimeStamp() returns the Unix time in nanoseconds since 01/01/1970
\* ------------------------------------------------------------------------- */
int64_t nsTimeStamp()
{
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





/* Here we iterate the top level entries. bIniEntryFind returns the pointers to the raw data in memory.
   In case of data composition subentries the data contains the enclosing curly braces.
   If the closing brace is missing and the document is terminated by a '\0' character the terminating '\0'
   is returned instead of the closing brace. */

int run_zero_copy_tests()
{
   int    iRet = 0;

   char * pName;       /* returned pointer to entry name */
   size_t NameSize;    /* will be set to the length of the name */
   char * pArg;        /* pointer to the entries parameter string */
   size_t ArgSize;     /* length of parameter string */

   char * pName2;      /* returned pointer to entry name */
   size_t NameSize2;   /* will be set to the length of the name */
   char * pArg2;       /* pointer to the entries parameter string */
   size_t ArgSize2;    /* length of parameter string */

   char * pd = (char*) "testname = zero_copy_tests \n"
                       "inttests = { ib=0b1111 io=0o1234567 id=000056789 ix=0xabcd987 } \n"
                       "floattests { fb=0b11.11e100 fo=0o1234.56e10 fd=1.2345e64 fx=0xabc.defp10 } \n";
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

   sfprintf(stdout, "%4d: zero_copy parsing tests of\n\"\n%s\n\"\n", __LINE__, pd);

   while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize))
   {
      if(!strmemcmp("testname", pName, NameSize))
      {
         found(top_entries, 0);
         sfprintf(stdout, "%4d: zero_copy: found '%.*s = %.*s'\n", __LINE__, (int) NameSize, pName, (int) ArgSize, pArg);

         if (strmemcmp ("zero_copy_tests",  pArg, ArgSize))
         {
            sfprintf(stderr, "%4d: zero_copy: unexpected argument!\n", __LINE__);
            goto Exit;
         }
      }
      else if(!strmemcmp("inttests", pName, NameSize))
      {
         found(top_entries, 1);
         sfprintf(stdout, "%4d: zero_copy: found '%.*s = %.*s'\n", __LINE__, (int) NameSize, pName, (int)  ArgSize, pArg);

         if (ArgSize != 52)
         {
            sfprintf(stderr, "%4d: zero_copy: unexpected ArgSize of %zu instead of %u bytes!\n", __LINE__, ArgSize, (int) 53);
            goto Exit;
         }

         pa = pArg+1;
         while(bIniEntryFind(&pa, &pName2, &NameSize2, &pArg2, &ArgSize2))
         {
            found(sub_entries, 1);
            sfprintf(stdout, "%4d: zero_copy: found    '%.*s = %.*s' (%lld)\n",
                     __LINE__, (int) NameSize2, pName2, (int) ArgSize2, pArg2, (long long) str2i64_r(pArg2, &pe, 1, &err));
    
         }
      }
      else if(!strmemcmp("floattests", pName, NameSize))
      {
         found(top_entries, 2);
         sfprintf(stdout, "%4d: zero_copy: found '%.*s = %.*s'\n", __LINE__, (int)  NameSize, pName, (int) ArgSize, pArg);

         if (ArgSize != 63)
         {
            sfprintf(stderr, "%4d: zero_copy: unexpected ArgSize of %zu instead of %u bytes!\n", __LINE__, ArgSize, (int)  53);
            goto Exit;
         }

         pa = pArg+1;
         while(bIniEntryFind(&pa, &pName2, &NameSize2, &pArg2, &ArgSize2))
         {
            found(sub_entries, 1);
            sfprintf(stdout, "%4d: zero_copy: found    '%.*s = %.*s' (%e)\n", __LINE__,
                     (int) NameSize2, pName2, (int)  ArgSize2, pArg2, str2d_r(pArg2, &pe, 1, &err));
    
         }
      }
      else
      {
         sfprintf(stderr, "%4d: zero_copy: unexpected entry '%.*s = %.*s' found!\n", __LINE__, (int) NameSize, pName, (int) ArgSize, pArg);
         goto Exit;
      }
   }

   sfprintf(stdout, "\n%4d: basic zero_copy tests succeeded, testing performance ...\n",  __LINE__);

   count = 1000;
   i64 = 0;
   t0 = nsTimeStamp();

   while (count--)
   {
      ps = pd;
      while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize))
      {
         if(!strmemcmp("testname", pName, NameSize))
         {
            if (strmemcmp ("zero_copy_tests",  pArg, ArgSize))
            {
            }
         }
         else if(!strmemcmp("inttests", pName, NameSize))
         {
            pa = pArg+1;
            while(bIniEntryFind(&pa, &pName2, &NameSize2, &pArg2, &ArgSize2))
            {
               ++i64;
            }
         }
         else if(!strmemcmp("floattests", pName, NameSize))
         {
            pa = pArg+1;
            while(bIniEntryFind(&pa, &pName2, &NameSize2, &pArg2, &ArgSize2))
            {
               ++i64;
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
      while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize))
      {
         if(!strmemcmp("testname", pName, NameSize))
         {
            if (strmemcmp ("zero_copy_tests",  pArg, ArgSize))
            {
            }
         }
         else if(!strmemcmp("inttests", pName, NameSize))
         {
            pa = pArg + 1;
            while(bIniEntryFind(&pa, &pName2, &NameSize2, &pArg2, &ArgSize2))
            {
               i64 = str2i64_r(pArg2, &pe, 1, &err);
               if(err || ((ptrdiff_t) ArgSize2 != (pe - pArg2)))
               {
                  sfprintf(stdout, "%4d: zero_copy: failed to read '%.*s = %.*s'! (read='%.*s', ret=%lld, err=%d (%s))\n",
                           __LINE__, (int) NameSize2, pName2, (int) ArgSize2, pArg2,
                           (int) (pe - pArg2), pArg2, (long long)i64, err, strerror(err));
                  goto Exit;
               }
            }
         }
         else if(!strmemcmp("floattests", pName, NameSize))
         {
            pa = pArg + 1;
            while(bIniEntryFind(&pa, &pName2, &NameSize2, &pArg2, &ArgSize2))
            {
               db = str2d_r(pArg2, &pe, 1, &err);
               if(err || ((ptrdiff_t) ArgSize2 != (pe - pArg2)))
               {
                  sfprintf(stdout, "%4d: zero_copy: failed to read '%.*s = %.*s'! (read='%.*s', ret=%e, err=%d, %s)\n",
                           __LINE__, (int) NameSize2, pName2, (int) ArgSize2, pArg2,
                           (int) (pe - pArg2), pArg2, db, err, strerror(err));
                  goto Exit;
               }
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




/* Here we iterate the top level entries. bIniEntryFind returns the pointers to the raw data in memory.
   In case of data composition subentries the data contains the enclosing curly braces.
   If the closing brace is missing and the document is terminated by a '\0' character the terminating '\0'
   is returned instead of the closing brace. */

int run_string_copy_tests()
{
   int    iRet = 0;

   INI_ENTRY * pEntry  = NULL;
   INI_ENTRY * pEntry2 = NULL;

   char * pd = (char*) "testname = string_copy_tests \n"
                       "inttests = { ib=0b1111 io=0o1234567 id=000056789 ix=0xabcd987 } \n"
                       "floattests { fb=0b11.11e100 fo=0o1234.56e10 fd=1.2345e64 fx=0xabc.defp10 } \n";

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

   while(bIniEntryRead(&ps, &pEntry, 0 /* bUnescape */))
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

/* ------------------------------------------------------------------------- *\
   The file tests are reading and iterating a test file
\* ------------------------------------------------------------------------- */

int run_file_tests()
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
   char *       pe;

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
      while(bIniEntryFind(&ps, &pName, &NameSize, &pArg, &ArgSize))
      {
         if(ArgSize && (*pArg == '{'))
         {
            sfprintf(stdout, "\n%4d: found block    %*s%.*s%s{ #* %zu bytes *#\n",
                     __LINE__, (int)(Indent*3),"", (int) NameSize, pName, NameSize ? " " : "", ArgSize);
            ps = pArg + 1;
            ++Indent;
         }
         else
         {
            sfprintf(stdout, "%4d: found entry    %*s%.*s%s%.*s\n",
                     __LINE__, (int)(Indent*3), "", (int) NameSize, pName ? pName : "\"\"",
                     ArgSize ? " = " : "", (int) ArgSize, pArg);
         }
      }

      pe = ps;

      ps = pIniFindNextSection(ps, &pSectionName, &SectionNameSize);
      if(ps)
      {
         sfprintf(stdout, "\n%4d: found section  %*s[%.*s]\n", __LINE__, (int)(Indent*3), "", (int) SectionNameSize, pSectionName ? pSectionName : "");
      }
      else
      {
         ps = pe;

         if(Indent)
         {
            --Indent;
            sfprintf(stdout, "%4d:                %*s%.1s\n", __LINE__, (int)(Indent*3), "", ps);

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

   sfprintf(stdout, "%4d: file tests %s!\n\n", __LINE__, iRet ? "succeeded" : "failed");

   return (iRet);
} /* run_file_tests() */



/* ------------------------------------------------------------------------- *\
   main function
\* ------------------------------------------------------------------------- */

int main(int argc, char * argv[])
{
    int iRet = 1;

    if(!run_file_tests())
        goto Exit;

    if(!run_string_copy_tests())
        goto Exit;

    if(!run_zero_copy_tests())
        goto Exit;

    iRet = 0;

    Exit:;

    if(!iRet)
        sfprintf(stdout, "All tests passed!\n");
    else
        sfprintf(stderr, "Tests failed!\n");

    return (iRet);
}/* main() */


/* ========================================================================= *\
   E N D   O F   F I L E
\* ========================================================================= */
