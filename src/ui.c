#include "ui.h"

#define U_MIN(x, y) ((x) > (y) ? y : x)
#define U_MAX(x, y) ((x) < (y) ? y : x)
#define U_CLAMP(val, low, high) (val < low ? low : (val > high ? high : val))

global U_Style u_global_style = 
{
   2, // Padding
   4, // Indent
   {22, 22, 22, 255},    // window color
   {16, 16, 16, 255},    // window title color
   {44, 44, 44, 255},    // button color
   {88, 88, 88, 255},    // button hover color
   {120, 120, 120, 255}, // button click color
   {0, 0, 0, 255},       // rect outline color
   {11, 11, 11, 255},    // checkbox unchecked color
   // {44, 44, 44, 255},    // checkbox hover color
   // {88, 88, 88, 255},    // checkbox checked color
   {44, 44, 44, 255},    // textbox color
   {16, 16, 16, 255},    // textbox focus color
   {122, 122, 122, 255}  // progress fill color
};

local U_Rect
u_rect(f32 x, f32 y, f32 w, f32 h)
{
   U_Rect result = {0};

   result.x = x;
   result.y = y;
   result.w = w;
   result.h = h;

   return result;
}

local U_Rect
u_shrink_rect(U_Rect rect, f32 val)
{
   U_Rect result = rect;

   result.x += val;
   result.y += val;
   result.w -= val*2.0f;
   result.h -= val*2.0f;

   return result;
}

local bool
u_test_rect_rect(U_Rect a, U_Rect b)
{
   bool result = true;

   if((b.x + b.w < a.x || b.x > a.x + a.w) &&
      (b.y + b.h < a.y || b.y > a.y + a.h))
   {
      result = false;
   }

   return result;
}

local f32
u_string_width(U_Context *u_context, char *string)
{
   f32 result = 0.0f;

   while(*string++)
   {
      result += u_context->font_width;
   }

   return result;
}

local int
u_string_length(char *string)
{
   int result = 0;

   while(*string++)
   {
      ++result;
   }

   return result;
}

local bool
u_rect_point_intersect(U_Rect rect, v2i point)
{
   bool result = false;

   if(point.x >= rect.x && point.x < rect.x + rect.w &&
      point.y >= rect.y && point.y < rect.y + rect.h)
   {
      result = true;
   }

   return result;
}

local f32
u_text_advance(U_Context *u_context)
{
   f32 result = 0.0f;

   U_Style *style = &u_context->style;
   result = style->padding*2 + u_context->font_height;

   return result;
}

local U_Layout *
u_push_layout(U_Context *u_context)
{
   U_Layout *result = 0;

   assert(u_context->layout_count < LAYOUT_COUNT);
   result = u_context->layout + u_context->layout_count++;
   zero_size(result, sizeof(U_Layout));

   return result;
}

local U_Layout *
u_get_layout(U_Context *u_context)
{
   U_Layout *result = 0;

   result = u_context->layout + (u_context->layout_count - 1);

   return result;
}

local U_Layout *
u_pop_layout(U_Context *u_context)
{
   U_Layout *result = 0;

   result = u_context->layout + (--u_context->layout_count);

   return result;
}

local U_Rect
u_get_container(U_Context *u_context)
{
   // Note: We can actually just have one container in layout.container and change that
   // with pretty much this function in the u_end_element, maybe that would be preferrable?
   U_Rect result = {0};

   U_Layout *layout = u_get_layout(u_context);
   U_Rect window_rect = u_context->window;
   result.x = window_rect.x + layout->container.x;
   result.y = window_rect.y + layout->container.y;

   s32 width = *(layout->widths + layout->curr_column);
   if(width < 0)
   {
      width = (s32)layout->container.w - (s32)layout->container.x + width;
   }

   s32 height = layout->row_height;
   result.w = (f32)width;
   result.h = (f32)height;

   return result;
}

local void
u_init(U_Context *u_context, f32 font_height, f32 font_width)
{
   u_context->command_mandala.base = u_context->commands;
   u_context->command_mandala.size = COMMAND_SIZE;

   u_context->style = u_global_style;
   u_context->font_height = font_height;
   u_context->font_width = font_width;
}

local void
u_start(U_Context *u_context, s32 mousex, s32 mousey, bool mouse_down, bool key_down[UKey_count])
{
   u_context->mouse.x = mousex;
   u_context->mouse.y = mousey;

   U_Mouse_State mouse_state = u_context->mouse_state;
   if(mouse_down)
   {
      if(mouse_state == UMouseState_pressed || 
         mouse_state == UMouseState_down)
      {
         u_context->mouse_state = UMouseState_down;
      }
      else
      {
         u_context->mouse_state = UMouseState_pressed;
      }
   }
   else
   {
      if(mouse_state == UMouseState_released ||
         mouse_state == UMouseState_up)
      {
         u_context->mouse_state = UMouseState_up;
      }
      else
      {
         u_context->mouse_state = UMouseState_released;
      }
   }

   for(umm key_index = 0;
       key_index < UKey_count;
       ++key_index)
   {
      U_Key_State key_state = u_context->keys[key_index];
      if(key_down[key_index])
      {
         if(key_state == UKeyState_pressed ||
            key_state == UKeyState_down)
         {
            u_context->keys[key_index] = UKeyState_down;
         }
         else
         {
            u_context->keys[key_index] = UKeyState_pressed;
         }
      }
      else
      {
         if(key_state == UKeyState_released ||
            key_state == UKeyState_up)
         {  
            u_context->keys[key_index] = UKeyState_up;
         }
         else
         {
            u_context->keys[key_index] = UKeyState_released;
         }
      }
   }

   u_context->layout_count = 0;
   u_context->command_mandala.used = 0;
   u_context->id = 1;
}

local void
u_end(U_Context *u_context)
{
   if(!u_context->changed_focus)
   {
      u_context->focus = 0;
   }
   u_context->changed_focus = false;

   *u_context->text_input = 0;
}

local void *
u_push_command(U_Context *u_context, U_Command_Type type)
{
   void *result = 0;

   assert(u_context->command_mandala.used + sizeof(U_Command) < u_context->command_mandala.size);
   U_Command *command = (U_Command *)((u8 *)u_context->command_mandala.base + u_context->command_mandala.used);
   command->type = type;
   result = &command->text;
   u_context->command_mandala.used += sizeof(U_Command);

   return result;
}

local char *
u_push_text(U_Context *u_context, char *string)
{
   char *result = 0;

   int string_length = u_string_length(string);
   char *dest = (char *)((u8 *)u_context->command_mandala.base + u_context->command_mandala.used);
   assert(u_context->command_mandala.used + string_length + 1 <= u_context->command_mandala.size);
   u_context->command_mandala.used += string_length + 1;
   while(string_length--)
   {
      *dest++ = *string++;
   }
   *dest = 0;

   return result;
}

local void
u_start_row(U_Context *u_context, s32 columns, s32 *widths, s32 height)
{
   // Todo: Need to push? then copy the last one I think
   U_Layout *layout = u_get_layout(u_context);
   layout->container.x = 0;
   layout->container.y += layout->row_height;
   layout->columns = (u8)columns;
   layout->curr_column = 0;
   layout->row_height = height;
   layout->max_row_height = 0;

   // Todo: Will a row ever extend outside of a scope?
   layout->widths = widths;
   // layout->container.w = layout->container.w/columns; 

}

local void
u_start_column(U_Context *u_context)
{
   U_Layout *layout = u_get_layout(u_context);

   U_Layout *curr_layout = u_push_layout(u_context);
   *curr_layout = *layout;
   curr_layout->container.w = (1.0f/(f32)layout->columns)*layout->container.w;
}

local void
u_end_column(U_Context *u_context)
{
   u_pop_layout(u_context);
   U_Layout *layout = u_get_layout(u_context);
   ++layout->curr_column;
}

local void
u_start_element(U_Context *u_context)
{
   U_Layout *layout = u_get_layout(u_context);

   U_Rect container = u_get_container(u_context);
   
   assert(layout->curr_column <= layout->columns);
   if(layout->curr_column == layout->columns)
   {
      layout->curr_column = 0;
      layout->container.x = 0;
      layout->container.y += U_MAX(layout->row_height, layout->max_row_height);
      layout->max_row_height = 0;
   }
}

local void
u_end_element(U_Context *u_context)
{
   U_Layout *layout = u_get_layout(u_context);

   U_Rect container = u_get_container(u_context);
   layout->container.x += container.w;

   ++layout->curr_column;
   
   ++u_context->id;
}

enum
{
   UText_halign_left = 1 << 0,  // Note: This is just default, why would you not do this
   UText_halign_center = 1 << 1,
   UText_halign_right = 1 << 2,
   UText_valign_top = 1 << 4,   // Again this is default, what are you doing
   UText_valign_center = 1 << 5,
   UText_valign_bottom = 1 << 6
};

local void
u_text_rect_opt(U_Context *u_context, U_Rect rect, char *string, u32 flags)
{
   U_Style style = u_context->style;
   U_Layout *layout = u_get_layout(u_context);

   v2 position = {0};
   position.x = rect.x + style.indent + style.padding;
   position.y = rect.y + style.padding + rect.h/2.0f - u_context->font_height/2.0f;
   if(flags & UText_halign_center)
   {
      position.x = rect.x + rect.w/2.0f;
   }
   if(flags & UText_valign_center)
   {
      position.y = rect.y + rect.h/2.0f - u_context->font_height/2.0f;
   }

   f32 at_y = 0;
   f32 at_x = 0;

   char buffer[512] = "";
   char *dest = buffer;
   char *src = string;
   while(true)
   {
      if(*src == '\n' ||
         *src == 0 ||
         (rect.x + at_x + u_context->font_width + style.padding > rect.x + rect.w))
      {
         *dest = 0;

         U_Command_Text *command = u_push_command(u_context, UCommand_text);
         u_push_text(u_context, buffer);

         f32 x_adjustment = (flags & UText_halign_center) ? -(u_string_width(u_context, buffer)/2.0f) : 0;

         command->pos.x = position.x + x_adjustment;
         command->pos.y = position.y + at_y;
         at_y += u_context->font_height;
         at_x = 0;

         dest = buffer;
         if(!*src)
         {
            break;
         }
      }
      *dest = *src;
      ++dest;
      ++src;
      at_x += u_context->font_width;
   }

   if(u_context->layout_count)
   {
      layout->max_row_height = U_MAX(layout->max_row_height, (s32)(at_y + u_context->font_height));
   }
}

local void
u_text_rect(U_Context *u_context, U_Rect rect, char *string)
{
   u_text_rect_opt(u_context, rect, string, 0);
}

typedef enum
{
   UWindowOption_no_title = 0x01,
} U_Window_Options;

local bool
u_window_opt(U_Context *u_context, U_Rect rect, char *label, int opt)
{
   bool result = false;

   f32 window_title_height = 22;

   U_Command_Rect *command = u_push_command(u_context, UCommand_rect);
   command->rect = rect;
   command->color = u_context->style.window_color;

   u_context->window = rect;

   if(!(opt & UWindowOption_no_title))
   {
      command = u_push_command(u_context, UCommand_rect);
      command->rect = rect;
      command->rect.h = window_title_height;
      command->color = u_context->style.window_title_color;

      u_text_rect(u_context, command->rect, label);

      rect.y += window_title_height;
      rect.h -= window_title_height;
   }

   U_Layout *layout = u_push_layout(u_context);
   layout->container = rect;

   return result;
}

local bool
u_window(U_Context *u_context, U_Rect rect, char *label)
{
   return u_window_opt(u_context, rect, label, 0);
}

local void
u_end_window(U_Context *u_context)
{
   u_pop_layout(u_context);
}

typedef enum
{
   UControllerOption_persist = 0x01,
} U_Controller_Options;

local void
u_update_controller(U_Context *u_context, U_Rect rect, int opt)
{
   bool hovered = u_rect_point_intersect(rect, u_context->mouse);
   if(hovered)
   {
      u_context->hover = u_context->id;

      if(u_context->mouse_state == UMouseState_pressed)
      {
         u_context->changed_focus = true;
         u_context->focus = u_context->id;
      }
   }

   if(u_context->focus == u_context->id)
   {
      u_context->changed_focus = true;

      if(u_context->mouse_state == UMouseState_pressed &&
         !hovered)
      {
         u_context->focus = 0;
      }
      if(!(u_context->mouse_state == UMouseState_down || u_context->mouse_state == UMouseState_pressed) &&
         (~opt & UControllerOption_persist))
      {
         u_context->focus = 0;
      }
   }

   if(u_context->hover == u_context->id)
   {
      if(!hovered)
      {
         u_context->hover = 0;
      }
   }
}

local bool
u_is_hovered(U_Context *u_context)
{
   bool result = u_context->hover == u_context->id;
   return result;
}

local bool
u_is_focused(U_Context *u_context)
{
   bool result = u_context->focus == u_context->id;
   return result;
}

local void
u_rectangle_outline(U_Context *u_context, U_Rect rect)
{
   U_Rect container = u_get_container(u_context);
   U_Command_Rect *command = u_push_command(u_context, UCommand_rect);
   command->rect.x = rect.x;
   command->rect.y = rect.y;
   command->rect.w = rect.w;
   command->rect.h = 1;
   command->color = u_context->style.rect_outline_color;

   command = u_push_command(u_context, UCommand_rect);
   command->rect.x = rect.x;
   command->rect.y = rect.y + rect.h - 1;
   command->rect.w = rect.w;
   command->rect.h = 1;
   command->color = u_context->style.rect_outline_color;

   command = u_push_command(u_context, UCommand_rect);
   command->rect.x = rect.x;
   command->rect.y = rect.y + 1;
   command->rect.w = 1;
   command->rect.h = rect.h - 2;
   command->color = u_context->style.rect_outline_color;

   command = u_push_command(u_context, UCommand_rect);
   command->rect.x = rect.x + rect.w - 1;
   command->rect.y = rect.y + 1;
   command->rect.w = 1;
   command->rect.h = rect.h - 2;
   command->color = u_context->style.rect_outline_color;
}

local void
u_text_opt(U_Context *u_context, char *string, u32 flags)
{
   u_start_element(u_context);

   U_Rect container = u_get_container(u_context);
   container = u_shrink_rect(container, u_context->style.padding);
   u_text_rect_opt(u_context, container, string, flags);

   u_end_element(u_context);
}

local void
u_text(U_Context *u_context, char *string)
{
   u_text_opt(u_context, string, 0);
}

local bool
u_button(U_Context *u_context, char *label)
{
   bool result = false;
   u_start_element(u_context);

   U_Style style = u_context->style;
   U_Layout *layout = u_get_layout(u_context);

   U_Rect container = u_get_container(u_context);
   container = u_shrink_rect(container, style.padding);
   u_update_controller(u_context, container, 0);

   U_Command_Rect *command = u_push_command(u_context, UCommand_rect);
   command->rect = container;
   u_rectangle_outline(u_context, container);

   if(u_is_focused(u_context) && u_is_hovered(u_context) &&
      u_context->mouse_state == UMouseState_down)
   {
      command->color = u_context->style.button_click_color;
   }
   else if(u_is_hovered(u_context))
   {
      command->color = u_context->style.button_hover_color;
      
      if(u_context->mouse_state == UMouseState_released)
      {
         result = true;
      }
   }
   else
   {
      command->color = u_context->style.button_color;
   }

   u_text_rect_opt(u_context, container, label, UText_halign_center|UText_valign_center);

   u_end_element(u_context);
   return result;
}

local bool
u_checkbox(U_Context *u_context, bool *checked, char *label)
{
   bool result = false;
   u_start_element(u_context);

   U_Style style = u_context->style;
   U_Layout *layout = u_get_layout(u_context);

   U_Rect container = u_get_container(u_context);
   container = u_shrink_rect(container, style.padding);
   u_update_controller(u_context, container, 0);

   U_Command_Rect *command = u_push_command(u_context, UCommand_rect);

   command->rect = container;
   command->rect.w = command->rect.h;

   if(u_is_focused(u_context) && 
      u_context->mouse_state == UMouseState_pressed)
   {
      result = true;
      *checked = !*checked;
   }

   if(u_is_focused(u_context) && u_is_hovered(u_context) &&
      u_context->mouse_state == UMouseState_down)
   {
      command->color = style.button_click_color;
   }
   else if(u_is_hovered(u_context))
   {
      command->color = style.button_hover_color;
   }
   else if(checked && *checked)
   {
      command->color = style.button_hover_color;
   }
   else if(checked && !*checked)
   {
      command->color = style.checkbox_unchecked_color;
   }
   
   u_rectangle_outline(u_context, command->rect);

   container.x += container.h;
   container.w -= container.h;
   u_text_rect(u_context, container, label);

   u_end_element(u_context);
   return result;
}

local bool
u_textbox(U_Context *u_context, char *buffer, s32 buffer_size)
{
   bool result = false;
   u_start_element(u_context);

   U_Style style = u_context->style;
   U_Layout *layout = u_get_layout(u_context);

   U_Rect container = u_get_container(u_context);
   container = u_shrink_rect(container, style.padding);

   U_Command_Rect *command = u_push_command(u_context, UCommand_rect);
   command->rect = container;

   u_update_controller(u_context, container, UControllerOption_persist);

   if(u_context->id == u_context->focus)
   {
      umm length = u_string_length(buffer);

      if(length + 1 < buffer_size)
      {
         append_string(buffer, u_context->text_input);
      }

      if(u_context->keys[UKey_backspace] == UKeyState_pressed &&
         length > 0)
      {
         *(buffer + length - 1) = 0;
      }

      command->color = style.textbox_focus_color;
   }
   else
   {
      command->color = style.textbox_color;
   }

   u_rectangle_outline(u_context, container);

   u_text_rect(u_context, container, buffer);

   u_end_element(u_context);
   return result;
}

local bool
u_dropdown(U_Context *u_context, s32 option_count, char **options, s32 *index)
{
   bool result = false;
   u_start_element(u_context);

   U_Style style = u_context->style;

   U_Rect container = u_get_container(u_context);
   container = u_shrink_rect(container, style.padding);



   u_rectangle_outline(u_context, container);

   u_end_element(u_context);
   return result;
}

local bool
u_slider(U_Context *u_context, f32 *val, f32 low, f32 high, char *label)
{
   assert(val);
   assert(high > low);

   bool result = false;
   u_start_element(u_context);

   U_Style style = u_context->style;

   U_Rect container = u_get_container(u_context);
   container = u_shrink_rect(container, style.padding);

   U_Command_Rect *track_rect = u_push_command(u_context, UCommand_rect);
   track_rect->rect = container;
   track_rect->color = style.window_title_color;

   U_Command_Rect *tab_rect = u_push_command(u_context, UCommand_rect);
   tab_rect->rect = container;
   tab_rect->rect.w = U_MAX(container.w/(high - low), 15);
   tab_rect->rect.x = container.x + (container.w - tab_rect->rect.w)*(*val - low)/(high - low);
   tab_rect->color = style.button_color;

   u_update_controller(u_context, container, 0);
   if(u_is_hovered(u_context))
   {
      tab_rect->color = style.button_hover_color;
   }
   if(u_is_focused(u_context))
   {
      f32 temp = *val;
      *val = (u_context->mouse.x - container.x)/(container.w - tab_rect->rect.w)*(high - low) - tab_rect->rect.w/2.0f + 1.0f;
#if 1
      *val = U_CLAMP(*val, low, high);
#else
      *val = U_MAX(*val, low);
      *val = U_MIN(*val, high);
#endif
      tab_rect->rect.x = container.x + (container.w - tab_rect->rect.w)*(*val - low)/(high - low);
      result = temp != *val ? true : false;
   }

   if(label)
   {
      u_text_rect_opt(u_context, container, label, UText_halign_center|UText_valign_center);
   }

   u_rectangle_outline(u_context, container);

   u_end_element(u_context);
   return result;
}

local bool
u_progress_bar(U_Context *u_context, f32 perc, char *label)
{
   bool result = false;
   u_start_element(u_context);

   U_Style style = u_context->style;

   U_Rect container = u_get_container(u_context);
   container = u_shrink_rect(container, style.padding);

   U_Command_Rect *command = u_push_command(u_context, UCommand_rect);
   command->rect = container;
   command->rect.w *= perc;
   command->color = style.progress_fill_color;

   if(label)
   {
      u_text_rect_opt(u_context, container, label, UText_halign_center|UText_valign_center);
   }

   u_rectangle_outline(u_context, container);

   u_end_element(u_context);
   return result;
}
