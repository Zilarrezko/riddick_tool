// Todo:
// [-] UI
// [-] Better Input?
//     Probably not going to ever finish this to something satisfying
// [ ] Better Software Rendering
//     Multithreading
//     SIMD
// [+] Enumerating Dll function names
//     IMAGE_DOS_HEADER is at the start of the dll, at the end of the header is an offset for the PE header, link below
//     https://en.wikipedia.org/wiki/Portable_Executable
// [-] Create a better font, that is more sizeable and better aligned
// [-] Logging
// [ ] Make a custom assembler

#include "memory.c"
#include "platform.c"
#include "debug.c"
#include "asset.c"
#include "renderer.c"
#include "injector.c"
#include "ui.c"
// #include "assembler.c"

#include "tool.h"

// Note: the returned index isn't used, it will be if I make this into a full code injector
local u32
add_to_code_list(State *state, Code_Info code_info)
{
   u32 result = 0;
   assert(state->code_list_count + 1 <= state->code_list_max);

   result = state->code_list_count;
   state->code_list[state->code_list_count++] = code_info;

   return result;
}

local void
free_code_list(State *state)
{
   if(state->process.handle)
   {
      char out[256];
      format_string(out, "freeing %u injected code sites\n", state->code_list_count);
      d_log(out);
      for(u32 index = 0;
          index < state->code_list_count;
          ++index)
      {
         Code_Info *code_info = state->code_list + index;
         free_code(&state->process, code_info);
      }
      state->code_list_count = 0;
   }
}

// Note: Pretty much for later use, in case I want to make a full blown code injector
local void
free_code_list_index(State *state, u32 index)
{
   if(state->process.handle)
   {
      free_code(&state->process, state->code_list + index);
   }
}

local void
update_and_render(State *state, Input *input, Bitmap *back_buffer)
{
   reset_mandala(&temp_mandala);

   if(!state->initialized)
   {
      d_clear_log();

      input->queue = queue_init(&perm_mandala, Key, 128);

      state->r_context = r_init_context(&perm_mandala, mega(1));

      state->code_list = push_array(&perm_mandala, Code_Info, 64);
      state->code_list_max = 64;
      
      state->process = init_process();

#if 1
      Bitmap bitmap = load_bitmap_from_bmp(&perm_mandala, "..\\font.bmp");
      font_atlas_to_file(bitmap);
      // Bitmap other_bitmap = load_bitmap_from_rta("..\\font.rta");
#endif

      state->assets.font = a_init_font_atlas_from_global();

      registry.loading_flag_base = 0x00;
      registry.loading_flag_pointer = 0x09;
      registry.loading_flag_jump = 0x0E;

      registry.map_name_base = 0x13;
      registry.map_name_pointer = 0x18;
      registry.map_name_jump = 0x21;

      u8 loading_aob_temp[] = {0x83, 0x85, 0xB4, 0x12, 0x00, 0x00, 0x01};
      memcpy(registry.loading_flag_aob, loading_aob_temp, array_count(loading_aob_temp));
      u8 map_aob_temp[] =     {0x8D, 0x56, 0x04, 0x8B, 0xC8, 0x2B, 0xD1};
      memcpy(registry.map_name_aob, map_aob_temp, array_count(map_aob_temp));
      u8 player_aob_temp[] =  {0x8B, 0x8F, 0xE0, 0x02, 0x00, 0x00};
      memcpy(registry.player_inject_aob, player_aob_temp, array_count(player_aob_temp));

      registry.player_inject_base = 0x26;
      registry.player_base_pointer = 0x8D;
      registry.player_inject_jump = 0x98;

      registry.player_base =  0x9D;
      registry.loading_flag = 0xA1;
      registry.map_name =     0xA5;

      state->mspf_set = false;
      state->mspf_toggle_key = '0';

      state->test_bitmap = load_bitmap_from_bmp(&perm_mandala, "..\\thing.bmp");

      u_init(&state->u_context, state->assets.font.glyph_height, state->assets.font.glyph_width);

      state->initialized = true;
   }

   Assets *assets = &state->assets;

   R_Context *r_context = &state->r_context;
   reset_mandala(&r_context->mandala);

   { // Note: Drawing background
      v2 dim;
      dim.e[0] = (f32)back_buffer->width;
      dim.e[1] = (f32)back_buffer->height;
      // r_clear(r_context);
   }

   U_Context *u_context = &state->u_context;
   
   bool u_keys[UKey_count] = {0};

   Queue *queue = &input->queue;
   while(queue->count)
   {
      Key *key = queue_pop(queue);

      bool shift = key->modifiers & KeyMod_Shift;
      bool control = key->modifiers & KeyMod_Control;
      bool alt = key->modifiers & KeyMod_Alt;

      if(state->has_focus)
      {
         // Note: UI input handling
         if(key->code >= ' ' && key->code <= '~')
         {
            format_string(u_context->text_input, "%s%c", u_context->text_input, key->code);
         }
         else if(key->code == '\b')
         {
            u_keys[UKey_backspace] = true;
         }
         else if(key->code == '\r' || key->code == '\n')
         {
            u_keys[UKey_return] = true;
         }
      }
      else
      {
         if(key->code == (u8)state->mspf_toggle_key)
         {
#if 0
            if(state->infinite_ammo.address)
            {
               free_code(&state->process, &state->infinite_ammo);
            }
            else
            {
               Module module = get_module_by_name(&state->process, "GameClasses_Win32_x86.dll");
               u8 bytes[] = {0x66, 0x89, 0x86, 0xA0, 0x01, 0x00, 0x00, 0x80};
               void *address = aob_scan(&state->process, module, bytes, array_count(bytes));
               u8 code[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
               state->infinite_ammo = write_code(&state->process, address, code, array_count(code));
            }
#else
            if(state->dll_injected)
            {
               state->mspf_set = !state->mspf_set;
               float mspf;
               if(state->mspf_set)
               {
                  mspf = (1.0f/(f32)state->fps_inject);
               }
               else
               {
                  mspf = 0.0f;
               }
               write_memory(&state->process, state->dll_mspf_address, &mspf, 4);
            }
#endif
         }
      }

   }

   u_start(u_context, input->mouse.x, input->mouse.y, input->mouse_down, u_keys);
   u_window_opt(u_context, u_rect(0, 0, (f32)back_buffer->width, (f32)back_buffer->height), "", UWindowOption_no_title);

   char print_buffer[256] = "";
   
   if(state->attached)
   {
      if(!is_process_running(&state->process))
      {
         free_process(&state->process);
         state->attached = false;
      }
      else
      {
         Process *process = &state->process;
#if 0
         f64 *buffer = (f64 *)push_size(&temp_mandala, sizeof(f64));
         module_address = build_module_address(process, "GameClasses_Win32_x86.dll", 0x849E68);
         if(read_memory(process, module_address, (void *)buffer, sizeof(f64)))
         {
            format_string(print_buffer, "GAME_TIME: %f", *buffer);
            r_text(r_context, &assets->font, print_buffer, (v2){0, 0}, 2);
         }
#endif

#if 0
         umm offsets[3] = {0xC4, 0x3C, 0x40};
         char menu_state[25] = "";
         module_address = build_module_address(process, "GameClasses_Win32_x86.dll", 0x004E88B4);
         if(read_memory_pointer_path(process, module_address, offsets, array_count(offsets),
                                     menu_state, 24))
         {
            format_string(print_buffer, "MENU_STATE: %s", menu_state);
            r_text(r_context, &assets->font, print_buffer, (v2){0, 16}, 2);
         }
#endif

#if 0
         Module module = get_module_from_name(&state->process, "GameClasses_Win32_x86.dll");
         u8 bytes[] = {0x66, 0x89, 0x86, 0xA0, 0x01, 0x00, 0x00, 0x80};
         umm ammo_decrement_offset = aob_scan(&state->process, module, bytes, 8);

         module_address.module = module;
         module_address.offset = ammo_decrement_offset;

         u8 code[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
         Code_Info code_info = write_code(&state->process, module_address, 
                                          code, array_count(code));
         if(code_info.replaced_code)
         {
            free_code(&state->process, &code_info);
         }
#endif

#if 0
         module_address = (Module_Address){0};
         module_address.offset = (smm)registry.base + registry.player_base;
         u32 player_base;
         if(read_memory(process, module_address, &player_base, sizeof(u32)))
         {
            format_string(print_buffer, "PLAYER_BASE: %X", player_base);
            r_text(r_context, &assets->font, print_buffer, (v2){0, 32}, 2);
         }
#endif

#if 0
         module_address = (Module_Address){0};
         module_address.offset = (smm)registry.base + registry.map_name;
         void *map_pointer = 0;
         char map_name[25] = "";
         if(read_memory(process, module_address, &map_pointer, 4))
         {
            if(map_pointer)
            {
               module_address.offset = (umm)map_pointer;
               if(read_memory(process, module_address, &map_name, 24))
               {
                  format_string(print_buffer, "MAP_NAME: %s", map_name);
                  r_text(r_context, &assets->font, print_buffer, (v2){0, 48}, 2);
               }
            }
            else
            {
               format_string(print_buffer, "MAP_NAME:");
               r_text(r_context, &assets->font, print_buffer, (v2){0, 48}, 2);
            }
         }
#endif

#if 0
         module_address = (Module_Address){0};
         module_address.offset = (smm)registry.base + registry.loading_flag;
         u8 loading_flag;
         umm offsets_loading[1] = {0x12B4};
         if(read_memory_pointer_path(process, module_address, offsets_loading, array_count(offsets_loading), &loading_flag, sizeof(1)))
         {
            format_string(print_buffer, "LOADING_FLAG: %hhu", loading_flag);
            r_text(r_context, &assets->font, print_buffer, (v2){0, 64}, 2);
         }
#endif
         char text_buffer[256] = "";
         u_start_row(u_context, 3, (s32[]){150, 200, -1}, 28);
         u_text(u_context, "fps limiter: fps:");
#if 0
         persistent char fps_buffer[3] = "30";
         u_textbox(u_context, fps_buffer, array_count(fps_buffer));
         state->fps_inject = (s32)s64_from_ascii(fps_buffer);
#else
         persistent f32 fps_buffer = 30.0f;
         format_string(text_buffer, "%.0f", round_f32(fps_buffer));
         bool changed_2 = u_slider(u_context, &fps_buffer, 1, 144, text_buffer);
         state->fps_inject = (s32)round_f32(fps_buffer);
#endif
         format_string(text_buffer, "mspf: %f", 1.0f/round_f32(fps_buffer));
         u_text(u_context, text_buffer);

         bool changed_1 = u_checkbox(u_context, &state->mspf_set, "limiter toggle");

         if((changed_2 || changed_1))
         {
            // Todo: Is it possible for the dll to not be injected here?
            //       Yeah I don't think so either
            float mspf;
            if(state->mspf_set)
            {
               mspf = (1.0f/(f32)state->fps_inject);
            }
            else
            {
               mspf = 0.0f;
            }
            write_memory(&state->process, state->dll_mspf_address, &mspf, 4);
         }

         u_start_row(u_context, 1, (s32[]){-1}, 40);
         format_string(text_buffer, "press %c to toggle the frame limiter", state->mspf_toggle_key);
         u_text(u_context, text_buffer);

         u_start_row(u_context, 1, (s32[]){150}, 32);
         if(u_button(u_context, "DETACH"))
         {
            state->force_detach = true;
            state->attached = false;
            d_log("Force detach");
            free_code_list(state);
            if(registry.base)
            {
               deallocate_in_process(state->process, registry.base);
               registry.base = 0;
            }
         }

#if DEBUG
         Module gdi32full_module = get_module_by_name(process, "gdi32full.dll");
         void *dll_buffer = push_size(&temp_mandala, gdi32full_module.size);
         read_memory(process, gdi32full_module.base, dll_buffer, gdi32full_module.size);
         Dll_Info dll_info = get_dll_info(dll_buffer);

         u_start_row(u_context, 1, (s32[]){-1}, 28);
         u_text(u_context, "DEBUG:");
         u_start_row(u_context, 3, (s32[]){150, 150, -1}, 28);
         format_string(text_buffer, "mspf_set: %hhu", state->mspf_set);
         u_text(u_context, text_buffer);
         // format_string(text_buffer, "mspf_set: %hhu", state->mspf_set);
#endif
      }
   }
   else
   {
      persistent bool attach_now = false;
      if(state->game_starting)
      {
         bool process_is_up = is_process_running(&state->process);
         if(process_is_up)
         {
            u_start_row(u_context, 1, (s32[]){-1}, 28);
            u_text(u_context, "Riddick Found...");
            u_text(u_context, "Calling Johns");

            get_modules(&state->process);
            Module test_module = get_module_by_name(&state->process, "GameClasses_Win32_x86.dll");
            Module test_module_2 = get_module_by_name(&state->process, "gdi32full.dll");
            state->game_starting = (test_module.size == 0 || test_module_2.size == 0) ? true : false;
            attach_now = (state->game_starting) ? false : true;
         }
         else
         {
            d_log("Game was closed before modules were loaded and we could attach\n");
            free_process(&state->process);
            state->game_starting = false;
         }
      }
      else
      {
         if(!state->force_detach && (attach_now || state->frame%120 == 0) && attach_process("DarkAthena.exe", &state->process))
         {
            attach_now = false;

            Module test_module = get_module_by_name(&state->process, "GameClasses_Win32_x86.dll");
            Module test_module_2 = get_module_by_name(&state->process, "gdi32full.dll");
            if(test_module.size == 0 || test_module_2.size == 0)
            {
               state->game_starting = true;

               // Todo: Do we really need to set the process id and handle to 0 here?
               // free_process(&state->process);
            }
            else
            {
               d_log("Game attached\n");
               state->game_starting = false;
               state->attached = true;

               Process *process = &state->process;

#if 0 // Note: uuuh need to figure out how to communicate between what livesplit needs, and what I need
               // Todo: wait until the executable actually has all the modules loaded... might take a second or two?
               Process *process = &state->process;
               // Note: Setting up the main script stuff for debug stuff... map name, loading flag and player struct base
               registry.base = allocate_in_process(state->process, 0, 4096);

               Module_Address module_address = {0};
               module_address.offset = (umm)registry.base;
               write_memory(&state->process, module_address, global_inject_code, array_count(global_inject_code));

               // Note: Player code site
               {
                  Module_Address player_code_site = aob_scan(process, get_module_by_name(process, "GameClasses_Win32_x86.dll"),
                                                             registry.player_inject_aob, array_count(registry.player_inject_aob));
                  assert(player_code_site.offset); // Todo: Replace with a check for an internal error

                  smm player_code_site_address = (smm)compute_module_address(player_code_site);
                  // Note: -5 from I believe the length of the bytes for the jmp instruction?
                  smm jump_position_relative = (smm)((u8 *)registry.base + registry.player_inject_base) - player_code_site_address - 5;
                  smm jump_back_position_relative = player_code_site_address + 6 - (smm)((u8 *)registry.base + registry.player_inject_jump) - 5;

                  { // Note: Jump to original code write
                     u8 inject_jump[] = {0x00, 0x00, 0x00, 0x00};
                     assert((s32)jump_back_position_relative == jump_back_position_relative);
                     memcpy(inject_jump, &jump_back_position_relative, 4);
                     Module_Address write_address = {0};
                     write_address.offset = (umm)registry.base + registry.player_inject_jump;
                     write_memory(process, write_address, inject_jump, array_count(inject_jump));
                  }
                  { // Note: Write the location of where we're storing the player base
                     smm player_base = (smm)registry.base + registry.player_base;
                     Module_Address write_address = {0};
                     write_address.offset = (umm)registry.base + registry.player_base_pointer;
                     write_memory(process, write_address, &player_base, sizeof(s32));
                  }
                  { // Note: Jump to script write
                     u8 inject_jump[] = {0xE9, 0x00, 0x00, 0x00, 0x00, 0x90};
                     assert((s32)jump_position_relative == jump_position_relative);
                     memcpy(inject_jump + 1, &jump_position_relative, 4);
                     Code_Info code_info = write_code(process, player_code_site, inject_jump, array_count(inject_jump));
                     add_to_code_list(state, code_info);
                  }
               }

               // Note: Map name code
               {
                  Module_Address map_code_site = aob_scan(process, get_module_by_name(process, "GameWorld_Win32_x86.dll"),
                                                          registry.map_name_aob, array_count(registry.map_name_aob));
                  assert(map_code_site.offset); // Todo: Replace with a check for an internal error

                  smm map_code_site_address = (smm)compute_module_address(map_code_site);
                  // Note: -5 from I believe the length of the bytes for the jmp instruction?
                  smm jump_position_relative = (smm)((u8 *)registry.base + registry.map_name_base) - map_code_site_address - 5;
                  smm jump_back_position_relative = map_code_site_address + 6 - (smm)((u8 *)registry.base + registry.map_name_jump) - 5;

                  { // Note: Jump to original code write
                     u8 inject_jump[] = {0x00, 0x00, 0x00, 0x00};
                     assert((s32)jump_back_position_relative == jump_back_position_relative);
                     memcpy(inject_jump, &jump_back_position_relative, 4);
                     Module_Address write_address = {0};
                     write_address.offset = (umm)registry.base + registry.map_name_jump;
                     write_memory(process, write_address, inject_jump, array_count(inject_jump));
                  }
                  { // Note: Write the location of where we're storing the player base
                     smm map_name = (smm)registry.base + registry.map_name;
                     Module_Address write_address = {0};
                     write_address.offset = (umm)registry.base + registry.map_name_pointer;
                     write_memory(process, write_address, &map_name, sizeof(s32));
                  }
                  { // Note: Jump to script write
                     u8 inject_jump[] = {0xE9, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90};
                     assert((s32)jump_position_relative == jump_position_relative);
                     memcpy(inject_jump + 1, &jump_position_relative, 4);
                     Code_Info code_info = write_code(process, map_code_site, inject_jump, array_count(inject_jump));
                     add_to_code_list(state, code_info);
                  }
               }

               // Note: Loading Flag
               {
                  Module_Address loading_code_site = aob_scan(process, get_module_by_name(process, "DarkAthena.exe"),
                                                              registry.loading_flag_aob, array_count(registry.loading_flag_aob));
                  assert(loading_code_site.offset); // Todo: Replace with a check for an internal error

                  smm loading_code_site_address = (smm)compute_module_address(loading_code_site);
                  // Note: -5 from I believe the length of the bytes for the jmp instruction?
                  smm jump_position_relative = (smm)((u8 *)registry.base + registry.loading_flag_base) - loading_code_site_address - 5;
                  smm jump_back_position_relative = loading_code_site_address + 6 - (smm)((u8 *)registry.base + registry.loading_flag_jump) - 5;

                  { // Note: Jump to original code write
                     u8 inject_jump[] = {0x00, 0x00, 0x00, 0x00};
                     assert((s32)jump_back_position_relative == jump_back_position_relative);
                     memcpy(inject_jump, &jump_back_position_relative, 4);
                     Module_Address write_address = {0};
                     write_address.offset = (umm)registry.base + registry.loading_flag_jump;
                     write_memory(process, write_address, inject_jump, array_count(inject_jump));
                  }
                  { // Note: Write the location of where we're storing the player base
                     smm map_name = (smm)registry.base + registry.loading_flag;
                     Module_Address write_address = {0};
                     write_address.offset = (umm)registry.base + registry.loading_flag_pointer;
                     write_memory(process, write_address, &map_name, sizeof(s32));
                  }
                  { // Note: Jump to script write
                     u8 inject_jump[] = {0xE9, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90};
                     assert((s32)jump_position_relative == jump_position_relative);
                     memcpy(inject_jump + 1, &jump_position_relative, 4);
                     Code_Info code_info = write_code(process, loading_code_site, inject_jump, array_count(inject_jump));
                     add_to_code_list(state, code_info);
                  }
               }
#endif
               { // Note: Injecting Dll

                  // Note: Check to see if it already _has_ the dll injected

                  Module temp_module = get_module_by_name(process, "frame_script.dll");
                  if(!temp_module.base)
                  {
                     d_log("Frame script dll isn't injected, doing the inject now.\n");

                     char dll_path[256] = "";

                     umm path_length = get_full_path("frame_script.dll", dll_path, 256);
                     assert(file_exists(dll_path));

                     void *frame_limit_base = allocate_in_process(state->process, 0, 4096);
                     void *address = frame_limit_base;
                     write_memory(&state->process, address, dll_path, path_length + 1);

                     Module kernel_module = get_module_by_name(process, "KERNEL32.DLL");

                     // Note: really hoping the offset is static
                     // Todo: scan for this offset
                     LPTHREAD_START_ROUTINE load_library_address = (LPTHREAD_START_ROUTINE)((u8 *)kernel_module.base + 0x22990);

                     create_remote_thread(state->process,
                                          load_library_address,
                                          frame_limit_base);
                     deallocate_in_process(state->process, frame_limit_base);

                     get_modules(&state->process);
                     d_log("Inject done.\n");
                  }
                  else
                  {
                     d_log("Frame script is already injected.\n");
                  }

                  u8 some_address[4] = {0};
                  {
                     Module gdi32full_module = get_module_by_name(process, "gdi32full.dll");
                     void *gdi32full_dll = push_size(&temp_mandala, gdi32full_module.size);
                     read_memory(process, gdi32full_module.base, gdi32full_dll, gdi32full_module.size);
                     Dll_Info dll_info = get_dll_info(gdi32full_dll);
                     u32 swapbuffer_offset = dll_function_offset(&dll_info, "SwapBuffers");
                     frame_limit_registry.inject_base = (u8 *)gdi32full_module.base + swapbuffer_offset + 0xb;
                     
                     u8 replacement_bytes[5] = {0};
                     read_memory(process, frame_limit_registry.inject_base, replacement_bytes, 5);
                     if(replacement_bytes[0] != 0xba)
                     {
                        d_log("gdi32full inject site isn't as expected:\n");
                        char output[256] = "";
                        format_string(output, "gdi32full.SwapBuffers: 0x%x\n", swapbuffer_offset);
                        d_log(output);
                        d_log("offset\tbyte\n");
                        for(u32 line = 0;
                            line < 0xa;
                            ++line)
                        {
                           format_string(output, "0x%x\t", line*0x10);
                           for(u32 byte = 0;
                               byte < 0x10;
                               ++byte)
                           {
                              format_string(output, "%s0x%02x,", output, *((u8 *)gdi32full_dll + swapbuffer_offset + line*0x10 + byte));
                           }
                           format_string(output, "%s\n", output);
                           d_log(output);
                        }
                        d_log("ERR: Severe, Exiting...");
#if 0
                        state->running = false;
                        return;
#else
                        assert(false);
#endif
                     }
                     memcpy(some_address, replacement_bytes + 1, 4);
                  }
                  
                  Module script_module = temp_module.base ? temp_module : get_module_by_name(process, "frame_script.dll");
                  
                  state->dll_mspf_address = (u8 *)script_module.base + 0x1285C;
                  
                  void *dll_buffer = push_size(&temp_mandala, script_module.size);
                  read_memory(process, script_module.base, dll_buffer, script_module.size);
                  Dll_Info dll_info = get_dll_info(dll_buffer);
                  u32 function_offset = dll_function_offset(&dll_info, "frame_limit");
                  void *frame_limit_address = (u8 *)script_module.base + function_offset;

                  // frame_limit_registry.gdi32full_swapbuffers = (u8 *)frame_limit_registry.inject_base + 2;
                  void *address = (void *)((umm)frame_limit_registry.inject_base);
                  // read_memory(&state->process, address, &frame_limit_registry.gdi32full_swapbuffers, 4);

                  { // Note: script setup
                     frame_limit_registry.script_base = allocate_in_process(state->process, 0, 4096);
                     // smm diff_to_limiter = (smm)frame_limit_address - (smm)frame_limit_registry.base - 5;
                     // s32 diff_to_swapbuffers = (s32)frame_limit_registry.gdi32full_swapbuffers - ((s32)frame_limit_registry.base + 0x06) - 5;
                     s32 diff_to_inject = ((s32)frame_limit_registry.inject_base + 0x04) - ((s32)frame_limit_registry.script_base) - 0x15;
                     memcpy(fl_jump_script + 0x00, &frame_limit_address, 4);
                     memcpy(fl_jump_script + 0x07, &frame_limit_registry.script_base, 4);
                     memcpy(fl_jump_script + 0x0d, &some_address, 4);
                     memcpy(fl_jump_script + 0x12, &diff_to_inject, 4);

                     address = frame_limit_registry.script_base;
                     write_memory(&state->process, address, fl_jump_script, array_count(fl_jump_script));
                  }

                  { // Note: jump inject setup
                     smm diff_to_script = ((smm)frame_limit_registry.script_base + 0x04) - (smm)frame_limit_registry.inject_base - 0x05;
                     memcpy(fl_jump_inject + 0x01, &diff_to_script, 4);

                     address = frame_limit_registry.inject_base;
                     Code_Info code_info = write_code(&state->process, address, fl_jump_inject, array_count(fl_jump_inject));
                     add_to_code_list(state, code_info);
                  }

                  state->dll_injected = true;
               }

               f32 mspf;
               read_memory(process, state->dll_mspf_address, &mspf, sizeof(f32));
               state->mspf_set = mspf == 0.0f ? false : true;
            }
         }
         else
         {
            u_start_row(u_context, 2, (s32[]){256, -1}, 28);
            // u_start_column(u_context);
            u_text(u_context, "Looking for Riddick");
            u_text(u_context, "Please Start Riddick");
            // u_button(u_context, "Here's a button!");
            // u_end_column(u_context);

            // u_start_column(u_context);
            char text_buffer[256] = "";
            if(state->force_detach)
            {
               u_text(u_context, "Tool won't auto-attach");
            }
            else
            {
               u_text(u_context, "Tool will auto-attach");
            }
            u_text(u_context, "Assault on Dark Athena 2009");

            u_start_row(u_context, 1, (s32[]){150}, 32);

            if(state->force_detach && u_button(u_context, "ATTACH"))
            {
               state->force_detach = false;
               attach_now = true;
               d_log("allowing reattaching");
            }

#if 0
            if(u_button(u_context, "EXIT"))
            {
               state->running = false;
            }
#endif
         }
      }
   }

   u_end_window(u_context);
   u_end(u_context);

   umm cursor = 0;
   while(cursor < u_context->command_mandala.used)
   {
      U_Command *command = (U_Command *)((u8 *)u_context->command_mandala.base + cursor);
      cursor += sizeof(U_Command);
      switch(command->type)
      {
         case UCommand_rect:
         {
            U_Command_Rect *rect = &command->rect;

            v4 color = rect->color;
            v2 pos = {rect->rect.x, rect->rect.y};
            v2 dim = {rect->rect.w, rect->rect.h};
            r_rectangle(r_context, pos, dim, color);
         } break;
         case UCommand_text:
         {
            U_Command_Text *text = &command->text;
            char *string = (char *)((u8 *)u_context->command_mandala.base + cursor);
            cursor += measure_string(string) + 1;

            r_text(r_context, &assets->font, string, text->pos, 1);
         } break;

         invalid_default_case;
      }
   }

#if 0
   // r_bitmap(r_context, &state->test_bitmap, (v2){250, 250});
   r_blur(r_context, (Rect){50, 50, 300, 300}, 3);
#endif

#if 0
   Rect a = {100, 100, 75, 75};
   r_rectangle_rect(r_context, a, (v4){0, 100, 100, 255});
   Rect b = {(f32)input->mouse.x, (f32)input->mouse.y, 40, 40};
   r_rectangle_rect(r_context, b, (v4){100, 100, 0, 255});
   r_rectangle_rect(r_context, r_intersect_rect(b, a), (v4){100, 0, 0, 255});
#else
   r_bitmap(r_context, &state->test_bitmap, (v2){(f32)input->mouse.x - state->test_bitmap.width/2, (f32)input->mouse.y - state->test_bitmap.height/2});
#endif

   r_render(back_buffer, r_context);
   ++state->frame;
}
