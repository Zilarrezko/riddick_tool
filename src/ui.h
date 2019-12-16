#if !defined(ui_h)

#include "renderer.h"
#include "asset.h"

typedef struct
{
   f32 x, y, w, h;
} U_Rect;

typedef struct
{
   f32 padding;
   f32 indent;
   v4 window_color;
   v4 window_title_color;
   v4 button_color;
   v4 button_hover_color;
   v4 button_click_color;
   v4 rect_outline_color;
   v4 checkbox_unchecked_color;
   // v4 checkbox_hover_color;
   // v4 checkbox_checked_color;
   v4 progress_fill_color;
} U_Style;

typedef struct
{
   s32 row_height;
   s32 max_row_height;
   u8 columns;
   u8 curr_column;
   s32 *widths;
   U_Rect container;
} U_Layout;

typedef enum
{
   UCommand_text,
   UCommand_rect,
   UCommand_clip
} U_Command_Type;

typedef struct
{
   v2 pos;
} U_Command_Text;
typedef struct
{
   U_Rect rect;
   v4 color;
} U_Command_Rect;
typedef struct
{
   U_Rect rect;
} U_Command_Clip;

typedef struct
{
   U_Command_Type type;
   union
   {
      U_Command_Text text;
      U_Command_Rect rect;
      U_Command_Clip clip;
   };
} U_Command;

typedef enum
{
   UMouseState_up,
   UMouseState_pressed,
   UMouseState_down,
   UMouseState_released
} U_Mouse_State;

typedef enum
{
   UKeyState_up,
   UKeyState_pressed,
   UKeyState_down,
   UKeyState_released
} U_Key_State;

typedef enum
{
   UKey_backspace,
   UKey_return,
   UKey_count
} U_Key;

#define LAYOUT_COUNT 16
#define COMMAND_SIZE 1024*64

typedef struct
{
   void *base;
   u32 used;
   u32 size;
} U_Mandala;

typedef struct
{
   void *r_elements;
   umm r_element_count;

   u16 id;
   u16 focus;
   bool changed_focus;
   u16 hover;

   U_Rect window;
   U_Layout layout[LAYOUT_COUNT];
   u32 layout_count;

   U_Style style;
   f32 font_height;
   f32 font_width;

   v2i mouse;
   U_Mouse_State mouse_state;

   U_Key_State keys[UKey_count];
   char text_input[128];

   U_Mandala command_mandala;
   u32 command_cursor;
   u8 commands[COMMAND_SIZE];
} U_Context;

#define ui_h
#endif
