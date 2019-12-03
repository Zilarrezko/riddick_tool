#if !defined(win32_h)

typedef struct
{
   void *memory;
   int width;
   int height;

   BITMAPINFO bitmap_info;
} Back_Buffer;

typedef struct
{
   State *state;
   Input *input;
} Window_Proc_Passin;

typedef struct
{
   State *state;
   Input *input;
   HINSTANCE instance;
} Thread_Package;

global HDC global_device_context;
global Back_Buffer global_back_buffer;

#define win32_h
#endif