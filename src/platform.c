
typedef struct
{
   u8 *memory;
   umm size;
} Read_File_Result;

typedef struct
{
   char *name;
   void *base;
   umm size;
} Module;

typedef struct
{
   u32 id;
   HANDLE handle;
   Module *modules;
   umm module_count;
} Process;

global HWND global_window;

local Read_File_Result
read_entire_file(char *filename)
{
   Read_File_Result result = {0};

   HANDLE handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
                               0,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               0);

   if(handle != INVALID_HANDLE_VALUE)
   {
      LARGE_INTEGER filesize;
      GetFileSizeEx(handle, &filesize);

      result.size = (umm)filesize.QuadPart;
      result.memory = allocate(result.size);

      u32 bytes_read;
      ReadFile(handle, result.memory, (u32)filesize.QuadPart, &bytes_read, 0);
      assert((umm)bytes_read == result.size);

      CloseHandle(handle);
   }

   return result;
}

local void
free_file_result(Read_File_Result *read_result)
{
   assert(read_result);

   deallocate(read_result->memory);
   read_result->size = 0;
}

local void
write_entire_file(char *filename, void *buffer, umm size)
{
   HANDLE handle = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
                               0,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               0);

   if(handle != INVALID_HANDLE_VALUE)
   {
      u32 bytes_written;
      WriteFile(handle, buffer, (u32)size, &bytes_written, 0);
      assert((umm)bytes_written == size);

      CloseHandle(handle);
   }
}

local void
write_append_file(char *filename, void *buffer, umm size)
{
   Read_File_Result read_result = read_entire_file(filename);

   if(read_result.memory)
   {
      void *new_buffer = allocate(read_result.size + size);
      memcpy(new_buffer, read_result.memory, read_result.size);
      memcpy((u8 *)new_buffer + read_result.size, buffer, size);

      write_entire_file(filename, new_buffer, read_result.size + size);

      deallocate(new_buffer);
      free_file_result(&read_result);
   }
   else
   { // Note: File doesn't exit probably?
      write_entire_file(filename, buffer, size);
   }
}

local void *
allocate_in_process(Process process, void *address, umm size)
{
   void *result = 0;

   result = VirtualAllocEx(process.handle, address, size, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
   if(!result)
   {
      int error = GetLastError();
      assert(result);
   }

   return result;
}

local void
deallocate_in_process(Process process, void *address)
{
   VirtualFreeEx(process.handle, address, 0, MEM_RELEASE);
}

local inline u64
get_cycle_stamp()
{
   u64 result = 0;

   LARGE_INTEGER counter;
   QueryPerformanceCounter(&counter);
   result = (u64)counter.QuadPart;

   return result;
}

local inline void
get_cwd(char *buffer, s32 buffer_length)
{
   GetCurrentDirectory(buffer_length, buffer);
}

local inline umm
get_full_path(char *partial, char *buffer, s32 buffer_length)
{
   umm result = GetFullPathName(partial, buffer_length, buffer, 0);
   return result;
}

local void
create_remote_thread(Process process, LPTHREAD_START_ROUTINE start_routine_address, void *param)
{
   HANDLE load_thread = CreateRemoteThread(process.handle, 0, 0, start_routine_address, param, 0, 0);

   int error = GetLastError();

   WaitForSingleObject(load_thread, INFINITE);
}

local b32
file_exists(char *filepath)
{
   b32 result = true;

   HANDLE file_handle = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_DELETE,
                                    0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
   if(GetLastError() == 0x02)
   {
      result = false;
   }

   return result;
}

local void
register_global_keys()
{
   HWND window = global_window;

   RAWINPUTDEVICE input_device = {0};
   input_device.usUsagePage = 0x01;
   input_device.usUsage = 0x06;
   input_device.dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY; // | RIDEV_DEVNOTIFY
   input_device.hwndTarget = window;

   u32 input_device_size = sizeof(input_device);
   if(!RegisterRawInputDevices(&input_device, 1, input_device_size))
   {
      int error = GetLastError();
      assert(0);
   }
}

local void
unregister_global_keys()
{

}
