#include "renderer_software.c"

#include "renderer.h"

local R_Context
r_init_context(Mandala *mandala, umm size)
{
   R_Context result = {0};

   result.mandala = sub_mandala(mandala, size);

   return result;
}

local void *
r_push_element(R_Context *r_context, R_Render_Type type)
{
   void *result = 0;

   R_Header *header = push_struct(&r_context->mandala, R_Header);
   header->type = type;
   result = (u8 *)header + offsetof(R_Header, rect);

   return result;
}

local void
r_clear(R_Context *r_context)
{
   R_Clear *element = (R_Clear *)r_push_element(r_context, R_Type_Clear);
}

local void
r_rectangle(R_Context *r_context, v2 pos, v2 dim, v4 color)
{
    R_Rect *element = (R_Rect *)r_push_element(r_context, R_Type_Rect);
    if(element)
    {
       element->pos = pos;
       element->dim = dim;
       element->color = color;
    }
}

local void
r_bitmap(R_Context *r_context, Bitmap *bitmap, v2 pos)
{
   R_Bitmap *element = (R_Bitmap *)r_push_element(r_context, R_Type_Bitmap);
   if(element)
   {
      element->bitmap = *bitmap;
      element->pos = pos;
   }
}

local void
r_glyph(R_Context *r_context, Font *font, char glyph, v2 pos, f32 scale)
{
   R_Glyph *element = (R_Glyph *)r_push_element(r_context, R_Type_Glyph);
   if(element)
   {
      element->glyph = a_get_atlas_glyph(font, glyph);
      element->atlas = &font->atlas;
      element->pos = pos;
      element->scale = scale;
   }
}

local u32
r_text(R_Context *r_context, Font *font, char *string, v2 pos, f32 scale)
{
   u32 advance = 0;
   for(char *scan = string;
       *scan;
       ++scan)
   {
      r_glyph(r_context, font, *scan, (v2){pos.x + (f32)advance, pos.y}, scale);
      advance += (u32)((f32)font->advance_x*scale);
   }

   return advance;
}

local void
r_blur(R_Context *r_context, Rect rect, u32 kernel_size)
{
   R_Blur *element = (R_Blur *)r_push_element(r_context, R_Type_Blur);
   if(element)
   {
      element->rect = rect;
      element->kernel_size = kernel_size;
   }
}

local void
r_render(Bitmap *output, R_Context *r_context)
{
   umm cursor = 0;
   Mandala mandala = r_context->mandala;
   while(cursor < r_context->mandala.used)
   {
      R_Header *header = (R_Header *)((u8 *)mandala.base + cursor);

      switch(header->type)
      {
         case R_Type_Clear:
         {
            R_Clear element = header->clear;

            render_clear(output);
         } break;
         case R_Type_Glyph:
         {
            R_Glyph element = header->glyph;

            int x = (int)round_f32(element.pos.x);
            int y = (int)round_f32(element.pos.y);

            render_bitmap_atlas(output, element.glyph->rect, element.atlas, x, y, element.scale);
         } break;
         case R_Type_Bitmap:
         {
            R_Bitmap element = header->bitmap;

            int x = (int)round_f32(element.pos.x);
            int y = (int)round_f32(element.pos.y);

            render_bitmap(output, element.bitmap, x, y);
         } break;
         case R_Type_Rect:
         {
            // R_Rect element = header->rect;
            R_Rect element = header->rect;

            render_rect(output, element.pos, element.dim, element.color);
         } break;
         case R_Type_Blur:
         {
            R_Blur element = header->blur;

#if 1
            Bitmap intermediate = render_blur(output, element.rect, element.kernel_size, false);
            render_bitmap(output, intermediate, (int)element.rect.x, (int)element.rect.y);
            intermediate = render_blur(output, element.rect, element.kernel_size, true);
            render_bitmap(output, intermediate, (int)element.rect.x, (int)element.rect.y);
#else
            render_blur_2(output, element.rect, element.kernel_size);
#endif
         } break;
         invalid_default_case;
      }

      cursor += sizeof(R_Header);
   }
}
