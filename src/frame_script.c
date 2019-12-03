#include <windows.h>
#include <stdio.h>

#include <psapi.h>

#define global static

#define assert(expr) if(!(expr)){*(int *)0=0;}

typedef int bool;

#define true 1
#define false 0

typedef BOOL Swap_Buffers(void *);

global LARGE_INTEGER start;
global bool initialized;
global float wanted_mspf = 0.0f;

__declspec(dllexport) void
frame_limit()
{
   LARGE_INTEGER perf;
   LARGE_INTEGER end;
   float elapsed_time;
   QueryPerformanceFrequency(&perf);
   if(start.QuadPart)
   {
      QueryPerformanceCounter(&end);
      elapsed_time = (float)(end.QuadPart - start.QuadPart)/(float)perf.QuadPart;
      while(elapsed_time < wanted_mspf)
      {
         QueryPerformanceCounter(&end);
         elapsed_time = (float)(end.QuadPart - start.QuadPart)/(float)perf.QuadPart;
      }
   }
   QueryPerformanceCounter(&start);
}

BOOL WINAPI
DllMain(HINSTANCE instance, DWORD reason, void *reserved)
{
   BOOL result = TRUE;

   if(reason == DLL_PROCESS_DETACH)
   {
      // MessageBoxA(0, "Oh boy, here I go dejecting again", "Frame Limit Script", 0);
   }
   else if(reason == DLL_PROCESS_ATTACH)
   {
      if(!initialized)
      {
         // MessageBoxA(0, "Oh boy, here I go injectin again", "Frame Limit Script", 0);
      }
      initialized = true;
   }

   return result;
}
