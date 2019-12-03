#include "injector.h"

#include <string.h>
#include <psapi.h>

#include <stdio.h>

local Process
init_process()
{
   Process result = {0};

   result.modules = push_size(&perm_mandala, MAX_MODULE_COUNT*sizeof(Module));
   for(umm index = 0;
       index < MAX_MODULE_COUNT;
       ++index)
   {
      // Note: For module name string
      result.modules[index].name = push_size(&perm_mandala, 128);
   }

   return result;
}

local void
free_process(Process *process)
{
   CloseHandle(process->handle);
   process->handle = 0;
   process->id = 0;
   // Important/Note: The module array is pushed onto the permanent mandala, we won't be able to free it
}

local void
get_modules(Process *process)
{
   if(process->handle)
   {
      HMODULE modules[MAX_MODULE_COUNT];

      u32 needed;
      EnumProcessModulesEx(process->handle, modules, sizeof(modules), &needed, LIST_MODULES_ALL);

      umm count = needed/sizeof(HMODULE);
      char buffer[128] = "";
      process->module_count = count;

      for(umm index = 0;
          index < count;
          ++index)
      {
         HMODULE module = modules[index];
         Module *process_module = (process->modules + index);

         u32 length = GetModuleBaseName(process->handle, module, buffer, array_count(buffer));
         if(length > 0)
         {
            copy_string(process_module->name, buffer);

            MODULEINFO module_info;
            GetModuleInformation(process->handle, module, &module_info, sizeof(module_info));

            process_module->base = module_info.lpBaseOfDll;
            process_module->size = module_info.SizeOfImage;
            // Todo: Do we need the entry point?
         }
      }
   }
}

local bool
is_process_running(Process *process)
{
   bool result = false;

   u32 exit_code;
   GetExitCodeProcess(process->handle, &exit_code);
   result = exit_code == 259 ? true : false;

   return result;
}

local bool
attach_process(char *process_string, Process *process)
{
   bool result = false;

   u32 size = 4096;
   u32 *process_ids = push_size(&temp_mandala, size);
   u32 needed;
   EnumProcesses(process_ids, size, &needed);
   u32 process_count = needed/sizeof(u32);

   char buffer[1024] = "";

   for(umm index = 0;
       index < process_count;
       ++index)
   {
      u32 id = process_ids[index];
      HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS,
                                  FALSE, id);

      int error = GetLastError();

      if(handle)
      {
         HMODULE module;
         u32 needed;

         if(EnumProcessModules(handle, &module, sizeof(module), &needed))
         {
            GetModuleBaseName(handle, module, buffer, array_count(buffer));

            if(strcmp(process_string, buffer) == 0)
            {
               process->id = id;
               process->handle = handle;

               get_modules(process);

               result = true;
               break;
            }
         }
      }

      CloseHandle(handle);
   }

   // deallocate(process_ids);
   return result;
}

local Module
get_module_by_name(Process *process, char *name)
{
   Module result = {0};

   for(umm index = 0;
       index < MAX_MODULE_COUNT;
       ++index)
   {
      Module module = process->modules[index];

      if(strcmp(module.name, name) == 0)
      {
         result = module;
         break;
      }
   }

   return result;
}

local bool
read_memory(Process *process, void *address, void *buffer, umm length)
{
   // Todo: decide if we _actually_ ever need to read in at one page at a time
   SIZE_T bytes_read;
   ReadProcessMemory(process->handle, address, buffer, length, &bytes_read);

   int error = GetLastError();
   if(error &&
      bytes_read != length)
   {
      char log_buffer[256] = "";
      format_string(log_buffer, "Err: ReadProcessMemory in %s failed with error %d\n", __FUNCTION__, error);
      d_log(log_buffer);
      return false;
   }

   return true;
}

local bool
read_memory_pointer_path(Process *process, void *address, umm *offsets, umm offset_count, 
                         void *buffer, umm length)
{
   SIZE_T bytes_read;
   umm base = (umm)address;
   for(umm index = 0;
       index < offset_count;
       ++index)
   {
      umm offset = offsets[index];

      ReadProcessMemory(process->handle, (void *)base, &base, sizeof(base), &bytes_read);
      // Important: 32 bit addressing space for pointers
      //            if we do the 2004 version, depending on the exe chosen, this could be a 64 bit addressing space
      base &= 0xFFFFFFFF;
      base += offset;
      if(bytes_read != sizeof(void *))
      {
         return false;
      }
   }

   BOOL read_result = ReadProcessMemory(process->handle, (void *)base, buffer, length, &bytes_read);
   if(bytes_read != length)
   {
      return false;
   }

   return true;
}

local void
write_memory(Process *process, void *address, void *buffer, umm size)
{
   size_t bytes_written;
   WriteProcessMemory(process->handle, address, buffer, size, &bytes_written);
   int error = GetLastError();
   if(error &&
      bytes_written != size)
   {
      char log_buffer[256] = "";
      format_string(log_buffer, "Err: WriteProcessMemory in %s failed with error %d\n", __FUNCTION__, error);
      d_log(log_buffer);
      format_string(log_buffer, "Trying to write %llu bytes at 0x%x: \n", size, address);
      d_log(log_buffer);
      format_string(log_buffer, "\t");
      for(u32 index = 0;
          index < size;
          ++index)
      {
         format_string(log_buffer, "%s0x%02x, ", log_buffer, ((u8 *)buffer)[index]);
      }
      format_string(log_buffer, "%s\n", log_buffer);
      d_log(log_buffer);
   }
}

local Code_Info
write_code(Process *process, void *address, u8 *code, umm code_size)
{
   Code_Info result = {0};
   result.address = address;

   result.replaced_code = allocate(code_size);
   result.replaced_code_size = code_size;

   read_memory(process, address, result.replaced_code, code_size);
   write_memory(process, address, code, code_size);

   return result;
}

local void
free_code(Process *process, Code_Info *info)
{
   if(info)
   {
      assert(info->replaced_code && info->replaced_code_size && info->address);
      write_memory(process, info->address, info->replaced_code, info->replaced_code_size);
      info->address = 0;
      deallocate(info->replaced_code);
      info->replaced_code = 0;
      info->replaced_code_size = 0;
   }
}

local umm
aob_scan_count(Process *process, Module module, u8 *bytes, umm size)
{
   umm result = 0;

   u8 *src = (u8 *)push_size(&temp_mandala, module.size);
   read_memory(process, module.base, src, module.size);

   for(umm scan = 0;
       scan < module.size;
       ++scan)
   {
      if(memcmp(src, bytes, size) == 0)
      {
         ++result;
      }

      ++src;
   }

   return result;
}

local void *
aob_scan(Process *process, Module module, u8 *bytes, umm size)
{
   void *result = 0;

#if DEBUG
   umm occurences = aob_scan_count(process, module, bytes, size);
   if(occurences > 1)
   {
      char buffer[256] = "";
      format_string(buffer, "More than one occurence looking for ");
      for(u32 index = 0;
          index < size;
          ++index)
      {
         format_string(buffer, "%s %hhu", buffer, bytes[index]);
      }
      format_string(buffer, "%s\n", buffer);
      d_log(buffer);
      assert(false);
   }
#endif

   u8 *src = (u8 *)push_size(&temp_mandala, module.size);
   result = module.base;
   read_memory(process, result, src, module.size);

   for(umm scan = 0;
       scan < module.size;
       ++scan)
   {
      if(memcmp(src, bytes, size) == 0)
      {
         result = (void *)scan;
      }

      ++src;
   }

   return result;
}

// Note: https://docs.microsoft.com/en-us/windows/win32/debug/pe-format
//       https://docs.microsoft.com/en-us/previous-versions/ms809762(v=msdn.10)?redirectedfrom=MSDN
typedef struct
{
   u16 e_magic;
   u16 e_cblp;
   u16 e_cp;
   u16 e_crlc;
   u16 e_cparhdr;
   u16 e_minalloc;
   u16 e_maxalloc;
   u16 e_ss;
   u16 e_sp;
   u16 e_sum;
   u16 e_ip;
   u16 e_cs;
   u16 e_lfarlc;
   u16 e_ovno;
   u16 e_resl[4];
   u16 e_oemid;
   u16 e_oeminfo;
   u16 e_res2[10];
   u32 e_lfanew;
} IMAGE_DOS_HEADER;

typedef struct
{
   u32 signature;
   u16 machine;
   u16 section_count;
   u32 time_stamp;
   u32 symbol_table;
   u32 symbol_count;
   u16 optional_header_size;
   u16 flags; // Note: also called Characteristics
} PE_Header;

typedef struct
{
   u16 magic;
   u8 major_linker_version;
   u8 minor_linker_version;
   u32 code_size;
   u32 init_data_size;
   u32 uninit_data_size;
   u32 entry_point_address;
   u32 code_base;
   u32 data_base;
} PE_Field_Header;

typedef struct
{
   u16 magic;
   u8 major_linker_version;
   u8 minor_linker_version;
   u32 code_size;
   u32 init_data_size;
   u32 uninit_data_size;
   u32 entry_point_address;
   u32 code_base;
} PE_Field_Plus_Header;

typedef struct
{
   u32 address;
   u32 size;
} Data_Directory;

typedef struct
{
   u32 image_base;
   u32 section_alignment;
   u32 file_alignment;
   u16 major_os_version;
   u16 minor_os_version;
   u16 major_image_version;
   u16 minor_image_version;
   u16 major_subsys_version;
   u16 minor_subsys_version;
   u32 win32_version; // Note: Zero filled
   u32 image_size;
   u32 header_size; // Note: from MS-DOS stub to PE Header and section headers, rounded to a multiple of file_alignment
   u32 checksum;
   u16 subsys;
   u16 dll_characteristics;
   u32 stack_reserve_size;
   u32 stack_commit_size;
   u32 heap_reserve_size;
   u32 heap_commit_size;
   u32 loader_flags; // Note: Zero filled
   u32 rva_and_size_count;
   Data_Directory data_directory;
} PE_Windows_Header; // Note: Windows Specific Fields

typedef struct
{
   u64 image_base;
   u32 section_alignment;
   u32 file_alignment;
   u16 major_os_version;
   u16 minor_os_version;
   u16 major_image_version;
   u16 minor_image_version;
   u16 major_subsys_version;
   u16 minor_subsys_version;
   u32 win32_version; // Note: Zero filled
   u32 image_size;
   u32 header_size; // Note: from MS-DOS stub to PE Header and section headers, rounded to a multiple of file_alignment
   u32 checksum;
   u16 subsys;
   u16 dll_characteristics;
   u64 stack_reserve_size;
   u64 stack_commit_size;
   u64 heap_reserve_size;
   u64 heap_commit_size;
   u32 loader_flags; // Note: Zero filled
   u32 rva_and_size_count;
   Data_Directory data_directory;
} PE_Windows_Plus_Header; // Note: Windows Specific Fields

#if 0
typedef struct
{
   Data_Directory export_table;
   Data_Directory import_table;
   Data_Directory resource_table;
   Data_Directory exception_table;
   Data_Directory certificate_table;
   Data_Directory base_relocation_table;
   Data_Directory debug;
   Data_Directory architecture_reserved;
   Data_Directory global_ptr;
   Data_Directory tls_table;
   Data_Directory load_config_table;
   Data_Directory bound_import;
   Data_Directory import_address_table;
   Data_Directory delay_import_descriptor;
   Data_Directory clr_runtime_header;
   Data_Directory reserved;
} Data_Directory_Header;
#endif

typedef struct
{
   char name[8]; // Don't pay attention too much to this
   u32 virtual_size;
   u32 virtual_address;
   u32 raw_data_size;
   u32 raw_data_pointer;
   u32 relocation_pointer;
   u32 line_number_pointer;
   u16 relocation_count;
   u16 line_number_count;
   u32 characteristics;
} Section_Table;

typedef struct
{
   u32 characteristics;
   u32 time_stamp;
   u16 major_version;
   u16 minor_version;
   u32 name_unused;
   u32 base;
   u32 export_count;
   u32 name_count;
   u32 export_address;
   u32 name_address;
   u32 ordinal_address;
} Export_Directory;

typedef struct
{
   u32 address;
   u32 name_address;
} Export_Element;

local inline u32
rva_resolve(Section_Table *section, u32 address)
{
   u32 result = 0;

   // assert(address >= section->virtual_address);
   
   // Note: Used for when the dll is a file
   result = (section->raw_data_pointer + (address - section->virtual_address));

   // Note: Used for when the dll is loaded in another process
   // result = section->raw_data_pointer + address;

   return result;
}

typedef struct
{
   u8 *memory;
   char name[256];
   u32 bit_size;

   bool is_dll;

   u32 function_count;
   u32 *function_names;
   u32 *function_offsets;
   u16 *ordinals;
} Dll_Info;

local Dll_Info
get_dll_info(u8 *memory)
{
   Dll_Info result = {0};
   result.memory = memory;

   IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)memory;
   u16 dos_magic_number = 0x5A4D; // Note: "MZ"
   assert(dos_header->e_magic == dos_magic_number);

   PE_Header *pe_header = (PE_Header *)(memory + dos_header->e_lfanew);
   u32 pe_magic_number = 0x4550; // Note: "PE\0\0"
   assert(pe_header->signature == pe_magic_number);

   switch(pe_header->machine)
   {
      case 0x14c:{result.bit_size = 32;} break;
      case 0x8664:{result.bit_size = 64;} break;
      default:{result.bit_size = 0;}
   }

   result.is_dll = (pe_header->flags&0x2000) ? true : false;

   u16 *field_magic = (u16 *)(pe_header + 1);
   assert(*field_magic == 0x10b ||
          *field_magic == 0x20b);
   u32 export_rva;
   if(*field_magic == 0x10b)
   {
      PE_Field_Header *pe_field_header = (PE_Field_Header *)(pe_header + 1);
      PE_Windows_Header *pe_win_header = (PE_Windows_Header *)(pe_field_header + 1);
      // Data_Directory_Header *data_header = (Data_Directory_Header *)(pe_win_header + 1);

      Data_Directory *export_directory = &pe_win_header->data_directory;
      // Todo: The address here might need some calculation to get the proper thing?
      export_rva = export_directory->address;
   }
   else
   {
      PE_Field_Plus_Header *pe_field_header = (PE_Field_Plus_Header *)(pe_header + 1);
      PE_Windows_Plus_Header *pe_win_header = (PE_Windows_Plus_Header *)(pe_field_header + 1);
      // Data_Directory_Header *data_header = (Data_Directory_Header *)(pe_win_header + 1);

      Data_Directory *export_directory = &pe_win_header->data_directory;
      export_rva = export_directory->address;
   }

   // Important: This will get the export directory if the export is _loaded_ in another process
   Export_Directory *export_directory = (Export_Directory *)(memory + export_rva);
   u32 *functions = (u32 *)(memory + export_directory->export_address);
   u32 *names =     (u32 *)(memory + export_directory->name_address);
   u16 *ordinals =  (u16 *)(memory + export_directory->ordinal_address);

   result.function_count = export_directory->export_count;
   result.function_names = names;
   result.function_offsets = functions;
   result.ordinals = ordinals;

   for(u32 index = 0;
       index < export_directory->export_count;
       ++index)
   {
      u32 function = functions[index];
      char *name = (char *)(memory + names[index]);

      int b = 0xb;
   }

   // Important: This is what's used when you're reading a
   /*
   Section_Table *section = (Section_Table *)((u8 *)(pe_header + 1) + pe_header->optional_header_size);
   for(s32 index = 0;
       index < pe_header->section_count;
       ++index, section += 1)
   {
      if(export_rva >= section->virtual_address && export_rva <= section->virtual_address + section->raw_data_size)
      {
         Export_Directory *export_table = (Export_Directory *)(memory + rva_resolve(section, export_rva));
         Export_Element *export_list = (Export_Element *)(memory + rva_resolve(section, export_table->export_address));
         for(u32 element_index = 0;
             element_index < export_table->export_count;
             ++element_index)
         {
            // Todo: Alright, It seems that a good bit of elements tend to have garbanzo beans names that don't tell us crap.
            // Not sure why they're there but maybe there's something that tells us whether or not we want to even look at those ones?
            Export_Element *export_element = export_list + element_index;
            u32 element_address = export_element->address;
            char *element_name = (char *)(memory + rva_resolve(section, export_element->name_address));

            if(element_address && element_name)
            {
               int b = 0xb;
            }

         }
      }
   }
   */

   return result;
}

local Dll_Info
get_dll_info_from_file(char *filepath)
{
   Dll_Info result = {0};
   Read_File_Result read_result = read_entire_file(filepath);
   get_file_from_path(result.name, filepath);
   
   result = get_dll_info(read_result.memory);

   free_file_result(&read_result);
   return result;
}

local u32
dll_function_offset(Dll_Info *info, char *function_name)
{
   u32 result = 0;

   for(u32 index = 0;
       index < info->function_count;
       ++index)
   {
      char *name = (char *)(info->memory + info->function_names[index]);
      if(info->function_offsets[index] == 923712)
      {
         int b = 0xb;
      }
      if(compare_string(function_name, name))
      {
         // Todo: this isn't getting the right value
         // Not really sure why though
         result = info->function_offsets[info->ordinals[index]];
         break; // Todo: uncomment this out of course when we figure out how to get the correct offset
      }
   }

   return result;
}
