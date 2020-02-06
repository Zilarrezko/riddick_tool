#include "jec.h"

// #include <windows.h>
#include "jec_win.h"

#include "tool.c"

#include "win32.h"

global HCURSOR global_debug_cursor;

local void
resize_back_buffer(Back_Buffer *buffer, int width, int height)
{
   if(buffer->memory)
   {
      deallocate(buffer->memory);
   }
   
   int bytes_per_pixel = 4;
   buffer->width = width;
   buffer->height = height;

   buffer->bitmap_info.bmiHeader = (BITMAPINFOHEADER){0};
   buffer->bitmap_info.bmiHeader.biSize = sizeof(buffer->bitmap_info.bmiHeader);
   buffer->bitmap_info.bmiHeader.biWidth = width;
   buffer->bitmap_info.bmiHeader.biHeight = -height;
   buffer->bitmap_info.bmiHeader.biPlanes = 1;
   buffer->bitmap_info.bmiHeader.biBitCount = 32;
   buffer->bitmap_info.bmiHeader.biCompression = BI_RGB;

   buffer->memory = allocate(width*height*bytes_per_pixel);
}

local inline void
present(Back_Buffer *buffer, int client_width, int client_height)
{
   StretchDIBits(global_device_context,
                 0, 0, client_width, client_height,
                 0, 0, buffer->width, buffer->height,
                 buffer->memory, &buffer->bitmap_info,
                 DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
   LRESULT result = 0;

   Window_Proc_Passin *passin = 0;
   State *state = 0;
   Input *input = 0;

   if(message == WM_CREATE)
   {
      CREATESTRUCT *create_struct = (CREATESTRUCT *)l_param;
      passin = (Window_Proc_Passin *)create_struct ->lpCreateParams;
      SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)passin);
   }
   else
   {
      passin = (Window_Proc_Passin *)GetWindowLongPtr(window, GWLP_USERDATA);
   }

   if(passin)
   {
      state = passin->state;
      input = passin->input;
   }

   switch(message)
   {
      case WM_CREATE:
      {
         ShowWindow(window, SW_SHOW);
      } break;

      case WM_SIZE:
      {
         int width = (l_param & 0xFFFF);
         int height = ((l_param & 0xFFFF0000) >> 16);
         resize_back_buffer(&global_back_buffer, width, height);

         Bitmap back_buffer;
         back_buffer.memory = global_back_buffer.memory;
         back_buffer.width = global_back_buffer.width;
         back_buffer.height = global_back_buffer.height;

         update_and_render(state, input, &back_buffer);
         present(&global_back_buffer, global_back_buffer.width, global_back_buffer.height);
         ValidateRect(window, 0);
      } break;

      case WM_PAINT:
      {
         present(&global_back_buffer, global_back_buffer.width, global_back_buffer.height);
         ValidateRect(window, 0);
      } break;

      case WM_CHAR:
      {
         // Note: I believe if we have registered a raw input device, this doesn't get called?
         char code = (char)w_param;

         if(input->queue.base)
         {
#if 0
            u8 keyboard_state[256];
            GetKeyboardState(keyboard_state);

            bool shift_is_down = keyboard_state[VK_LSHIFT] & 0x80 || keyboard_state[VK_RSHIFT] & 0x80;
            bool control_is_down = keyboard_state[VK_LCONTROL] & 0x80 || keyboard_state[VK_RCONTROL] & 0x80;
            bool alt_is_down = keyboard_state[VK_LMENU] & 0x80 || keyboard_state[VK_RMENU] & 0x80;

            Key *element = queue_push(&input->queue);
            *element = (Key){0};
            element->code = code;
            element->modifiers |= shift_is_down|control_is_down|alt_is_down;
#endif
         }
      } break;

      case WM_INPUT:
      {
         if(input->queue.base)
         {
            u8 keyboard_state[256];
            GetKeyboardState(keyboard_state);

            bool shift_is_down = keyboard_state[VK_LSHIFT] & 0x80 || keyboard_state[VK_RSHIFT] & 0x80;
            bool control_is_down = keyboard_state[VK_LCONTROL] & 0x80 || keyboard_state[VK_RCONTROL] & 0x80;
            bool alt_is_down = keyboard_state[VK_LMENU] & 0x80 || keyboard_state[VK_RMENU] & 0x80;

            unsigned int data_size;
            GetRawInputData((HRAWINPUT)l_param, RID_INPUT, 0, &data_size, sizeof(RAWINPUTHEADER));
            data_size = 4096;
            u8 *input_data = push_size_aligned(&temp_mandala, data_size, 8);
            u32 bytes_copied = GetRawInputData((HRAWINPUT)l_param, RID_INPUT,
                                               input_data, &data_size, sizeof(RAWINPUTHEADER));
            int error = GetLastError();

            RAWINPUT *raw_input = (RAWINPUT *)input_data;
            switch(raw_input->header.dwType)
            {
               case RIM_TYPEKEYBOARD:
               {
                  RAWKEYBOARD *keyboard = &raw_input->data.keyboard;
                  if(keyboard->Flags == RI_KEY_MAKE &&
                     keyboard->Message == WM_KEYDOWN)
                  {
                     Key *element = queue_push(&input->queue);
                     *element = (Key){0};
                     element->code = keyboard->VKey;
                     element->modifiers |= shift_is_down|control_is_down|alt_is_down;
                  }
               } break;

               // invalid_default_case;
            }
         }
      } break;

      case WM_MOUSEMOVE:
      {
         int mousex = (int)((l_param & 0x0000ffff) >>  0);
         int mousey = (int)((l_param & 0xffff0000) >> 16);

         input->mouse = (v2i){mousex, mousey};
      } break;

      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      {
         input->mouse_down = (w_param & 0x0001) >> 0;
      } break;

      case WM_SETCURSOR:
      {
         result = DefWindowProcA(window, message, w_param, l_param);
         // Note: Will hide the cursor
         // SetCursor(0);
      } break;

      case WM_SETFOCUS:
      {
         state->has_focus = true;
      } break;

      case WM_KILLFOCUS:
      {
         state->has_focus = false;
      } break;

      case WM_CLOSE:
      case WM_DESTROY:
      case WM_QUIT:
      {
         state->running = false;
         free_code_list(state);
         if(registry.base)
         {
            deallocate_in_process(state->process, registry.base);
            registry.base = 0;
         }
         d_log("Closing program normally...\n");
         PostQuitMessage(0);
      } break;

      default:
      {
         result = DefWindowProcA(window, message, w_param, l_param);
      }
   }

   return result;
}

DWORD WINAPI 
thread_proc(void *parameter)
{
   Thread_Package *thread_package = (Thread_Package *)parameter;
   State *state = thread_package->state;
   HINSTANCE instance = thread_package->instance;

   while(!state->initialized)
   {
      // Note: waiting for state to be running
   }

   MSG message;
   while(state->running)
   {
      while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
      {
         TranslateMessage(&message);
         DispatchMessageA(&message);
      }

      Sleep(33); // Note: Honestly probably just good enough... >.>
   }

   return 0;
}

int WINAPI
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_command)
{
   QueryPerformanceFrequency(&perf_freq);
   timeBeginPeriod(1);

   LARGE_INTEGER start = {0};

   global_debug_cursor = LoadCursor(0, IDC_ARROW);

   WNDCLASSEXA window_class = {0};
   window_class.cbSize = sizeof(WNDCLASSEX);
   window_class.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
   window_class.lpfnWndProc = window_proc;
   window_class.cbClsExtra = 0;
   window_class.cbWndExtra = 0;
   window_class.hInstance = instance;
   // window_class.hIcon = ;
   window_class.hCursor = global_debug_cursor;
   // window_class.hbrBackground = ;
   // window_class.lpszMenuName = ;
   window_class.lpszClassName = "riddick_tool_class";
   // window_class.hIconSm = ;

   if(!RegisterClassExA(&window_class))
   {
      return -1;
   }

   State state = {0};
   Input input = {0};

   Window_Proc_Passin passin = {0};
   passin.state = &state;
   passin.input = &input;

#if 0
   Thread_Package thread_package = {0};
   thread_package.state = &state;
   thread_package.input= &input;
   thread_package.instance = instance;

   DWORD thread_id;
   HANDLE thread_handle = CreateThread(0, 0, thread_proc, &thread_package, 0, &thread_id);
#endif

   perm_mandala = init_mandala(mega(2));
   temp_mandala = init_mandala(giga(2));

   global_window = CreateWindowA(window_class.lpszClassName, "riddick tool",
                                 WS_OVERLAPPEDWINDOW,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 540, 540,
                                 0, 0,
                                 instance, &passin);

   if(!global_window)
   {
      d_log("ERR: Window couldn't be created\n");
   }

   global_device_context = GetDC(global_window);
   // resize_back_buffer(&global_back_buffer, 540, 540);

   register_global_keys();

   state.running = true;
   MSG message;
   while(state.running)
   {
      while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
      {
         TranslateMessage(&message);
         DispatchMessageA(&message);
      }

      Bitmap back_buffer;
      back_buffer.memory = global_back_buffer.memory;
      back_buffer.width = global_back_buffer.width;
      back_buffer.height = global_back_buffer.height;

      update_and_render(&state, &input, &back_buffer);

      // double wanted_mspf = 0.03333333333333;
      f32 wanted_mspf = 1.0/60.0;
      LARGE_INTEGER end;
      f32 sleep_time = 0;
      if(start.QuadPart)
      {
         QueryPerformanceCounter(&end);
         f32 elapsed_time = ((f32)(end.QuadPart - start.QuadPart)/(f32)perf_freq.QuadPart);
#if 1
         sleep_time = (wanted_mspf - elapsed_time)*1000.0f;
         if(sleep_time > 0.0f)
         {
            Sleep((DWORD)sleep_time);
         }

         QueryPerformanceCounter(&end);
         elapsed_time = ((f32)(end.QuadPart - start.QuadPart)/(f32)perf_freq.QuadPart);
         while(elapsed_time < wanted_mspf)
         {
            QueryPerformanceCounter(&end);
            elapsed_time = ((f32)(end.QuadPart - start.QuadPart)/(f32)perf_freq.QuadPart);
         }
#endif

         state.frame_time = elapsed_time;
      }
      QueryPerformanceCounter(&start);

      present(&global_back_buffer, global_back_buffer.width, global_back_buffer.height);
   }

   return 0;
}
