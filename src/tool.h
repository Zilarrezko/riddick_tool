#if !defined(tool_h)

#define KeyMod_Shift   0x01
#define KeyMod_Control 0x02
#define KeyMod_Alt     0x04

typedef struct
{
   u32 code;
   u8 modifiers;
} Key;

typedef struct
{
   Queue queue;
   v2i mouse;
   bool mouse_down;
} Input;

typedef struct
{
   Assets assets;
   R_Context r_context;
   U_Context u_context;
   
   Process process;

   Code_Info *code_list;
   u32 code_list_count;
   u32 code_list_max;

   Code_Info infinite_ammo;

   void *dll_mspf_address;
   char mspf_toggle_key;
   bool mspf_set;
   s32 fps_inject;
   bool dll_injected;

   bool attached;

   umm frame;
   f32 frame_time;

   Bitmap test_bitmap;

   bool game_starting;
   bool running;
   bool has_focus;
   bool initialized;
} State;

typedef struct
{
   void *script_base;
   void *inject_base;
} Frame_Limit_Registry;

global Frame_Limit_Registry frame_limit_registry;

global unsigned char fl_jump_script[] = 
{
   /*0x00*/ 0x00,0x00,0x00,0x00,             // frame_limit address
   /*0x04*/ 0x60,                            // pushad
   /*0x05*/ 0xff, 0x15, 0x00,0x00,0x00,0x00, // call dword ptr [frame_limit]
   /*0x0b*/ 0x61,                            // popad
   /*0x0c*/ 0xba, 0x00,0x00,0x00,0x00,       // mov edx, gdi32full.dll+????????
   /*0x11*/ 0xe9, 0x00,0x00,0x00,0x00        // jmp inject+inject_size
};

global unsigned char fl_jump_inject[] =
{
   /*0x00*/   0xe9, 0x00,0x00,0x00,0x00    // jmp [script_base]
};


#define tool_h
#endif