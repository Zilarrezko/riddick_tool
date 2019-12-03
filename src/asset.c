#include "asset.h"

local u32
get_mask_shift(u32 mask)
{
   u32 result;

   switch(mask)
   {
      case 0xFF000000:
      {
         result = 24;
      } break; 
      case 0x00FF0000:
      {
         result = 16;
      } break; 
      case 0x0000FF00:
      {
         result = 8;
      } break; 
      case 0x000000FF:
      {
         result = 0;
      } break; 
      invalid_default_case;
   }

   return result;
}

local Bitmap
load_bitmap_from_bmp(Mandala *mandala, char *filename)
{
   Bitmap result = {0};

   Read_File_Result read_result = read_entire_file(filename);
   assert(((char *)read_result.memory)[0] == 'B');
   assert(((char *)read_result.memory)[1] == 'M');

   Bitmap_Header *bitmap_header = (Bitmap_Header *)read_result.memory;
   result.width = (u16)bitmap_header->width;
   result.height = (u16)bitmap_header->height;

   result.memory = push_size(mandala, bitmap_header->size);

   u32 red_mask =   bitmap_header->red_mask;
   u32 green_mask = bitmap_header->green_mask;
   u32 blue_mask =  bitmap_header->blue_mask;
   u32 alpha_mask = bitmap_header->alpha_mask;

   u32 red_shift = get_mask_shift(red_mask);
   u32 green_shift = get_mask_shift(green_mask);
   u32 blue_shift = get_mask_shift(blue_mask);
   u32 alpha_shift = get_mask_shift(alpha_mask);

   u32 *src_row = (u32 *)((u8 *)read_result.memory + bitmap_header->offset_data + bitmap_header->size) - bitmap_header->width;
   u32 *dest = (u32 *)((u8 *)result.memory);
   for(umm y = 0;
       y < result.height;
       ++y)
   {
      u32 *src = src_row;
      for(umm x = 0;
          x < result.width;
          ++x)
      {
         // Right now we don't know where the red green blue alpha channels are,
         // we are going to make the color format RGBA

         f32 red =   (f32)((*src & red_mask  ) >> red_shift  );
         f32 green = (f32)((*src & green_mask) >> green_shift);
         f32 blue =  (f32)((*src & blue_mask ) >> blue_shift );
         f32 alpha = (f32)((*src & alpha_mask) >> alpha_shift);

         f32 norm_alpha = alpha/255.0f;

         red   = round_f32(red*norm_alpha);
         green = round_f32(green*norm_alpha);
         blue  = round_f32(blue*norm_alpha);

         u32 color = ((s32)alpha << 24) & 0xFF000000|
                     ((s32)red   << 16) & 0x00FF0000|
                     ((s32)green <<  8) & 0x0000FF00|
                     ((s32)blue  <<  0) & 0x000000FF;
         ++src;

         *dest++ = color;
      }
      src_row -= bitmap_header->width;
   }

   free_file_result(&read_result);
   return result;
}

#if 1
local Bitmap
load_bitmap_from_rta(char *filename)
{
   Bitmap result = {0};

   Read_File_Result read_result = read_entire_file(filename);

   result.width = *(u32 *)read_result.memory;
   result.height = (u32)(read_result.size - sizeof(result.width))/(result.width*4);

   result.memory = push_size(&temp_mandala, read_result.size);
   memcpy(result.memory, (u8 *)read_result.memory + sizeof(result.width), read_result.size);

   free_file_result(&read_result);
   return result;
}

static void
lz_compress(void *buffer, umm size, void *out, umm *compress_size)
{
   u8 literal_buffer[255];
   u8 literal_length = 0;

   u8 *src = (u8 *)buffer;
   u8 *dest = (u8 *)out;

#define look_back_window 0xff
#define literal_length_max 0xff

   umm scan = 0;
   while(scan <= size)
   {
      umm look_back = scan;
      if(look_back > look_back_window)
      {
         look_back = look_back_window;
      }

      umm best_run = 0;
      umm best_pos = 0;
      for(u8 *back_scan = src + scan - look_back;
          back_scan < src + scan;
          ++back_scan)
      {
         umm back_scan_size = src + size - back_scan;
         if(back_scan_size > look_back_window)
         {
            back_scan_size = look_back_window;
         }

         u8 *back_scan_end = back_scan + back_scan_size;
         u8 *test_src = src + scan;
         u8 *back_scan_src = back_scan;
         umm test_run = 0;
         while(back_scan_src < back_scan_end &&
               *test_src++ == *back_scan_src++)
         {
            ++test_run;
         }

         if(best_run < test_run)
         {
            best_run = test_run;
            best_pos = src + scan - back_scan;
         }
      }

      bool run_condition = false;
      if(literal_length)
      {
         run_condition = best_run > 4;
      }
      else
      {
         run_condition = best_run > 2;
      }

      if(run_condition ||
         literal_length == literal_length_max)
      {
         if(literal_length)
         {
            u8 *writer = dest;
            *writer++ = (u8)literal_length;
            *writer++ = 0;
            dest = writer;

            for(umm index = 0;
                index < literal_length;
                ++index)
            {
               *dest++ = literal_buffer[index];
            }
            literal_length = 0;
         }

         if(run_condition)
         {
            assert(best_run <= look_back_window && best_pos <= look_back_window);
            u8 *writer = (u8 *)dest;
            *writer++ = (u8)best_run;
            *writer++ = (u8)best_pos;
            dest = writer;

            scan += best_run;
         }
      }
      else
      {
         literal_buffer[literal_length++] = src[scan];

         ++scan;
      }

      if(scan == size)
      {
         break;
      }
   }

   if(compress_size)
   {
      *compress_size = dest - (u8 *)out;
   }


#undef look_back_window
#undef literal_length_max
}

static void
lz_decompress(void *buffer, umm size, void *out, umm *decompress_size)
{
   u8 *dest = (u8 *)out;
   u8 *src = (u8 *)buffer;
   u8 *end = (u8 *)buffer + size;
   while(src < end)
   {
      u8 count  = *src++;
      u8 offset = *src++;

      u8 *copy = (dest - offset);
      if(offset == 0)
      {
         copy = src;
         src += count;
      }

      while(count--)
      {
         *dest++ = *copy++;
      }
   }

   if(decompress_size)
   {
      *decompress_size = dest - (u8 *)out;
   }
}

void
font_atlas_to_file(Bitmap bitmap)
{
   umm bitmap_size = bitmap.width*bitmap.height*4;

#if 1
   umm encoded_size;
   void *encoded_buffer = push_size(&temp_mandala, bitmap_size*2);
   lz_compress(bitmap.memory, bitmap_size, encoded_buffer, &encoded_size);

   // Note: Array literal allocation
   // 2 bytes for character prefixes "0x", then 2 more for the hex byte and finally one more byte for a comma
   umm write_size = encoded_size*(2 + 2 + 1);
   void *buffer = push_size(&temp_mandala, write_size);
   char *dest = (char *)buffer;
   u8 *byte;
   for(umm index = 0;
       index < encoded_size;
       ++index)
   {
      byte = (u8 *)encoded_buffer + index;

      *dest++ = '0';
      *dest++ = 'x';

      u8 upper = (*byte&0xF0) >> 4;
      u8 lower = (*byte&0x0F) >> 0;

      *dest++ = (upper > 9) ? upper - 10 + 'a' : upper + '0';
      *dest++ = (lower > 9) ? lower - 10 + 'a' : lower + '0';
      *dest++ = ',';
   }
   write_entire_file("..\\font.rta", buffer, write_size);
#else
   write_entire_file("..\\font.rta", bitmap.memory, bitmap_size);
#endif
}

#endif

local Font
a_init_font_atlas_from_global()
{
   Font result = {0};

   umm atlas_size = global_atlas_width*global_atlas_height*4;
   umm decompress_size;
   void *buffer = push_size(&perm_mandala, atlas_size);
   lz_decompress(global_font_atlas, sizeof(global_font_atlas), buffer, &decompress_size);

   result.atlas.memory = buffer;
   result.atlas.width = global_atlas_width;
   result.atlas.height = global_atlas_height;

   result.glyph_width = 8;
   result.glyph_height = 14;

   result.advance_x = result.glyph_width;
   result.advance_y = result.glyph_height;

   f32 glyph_width_norm = (f32)result.glyph_width/global_atlas_width;
   f32 glyph_height_norm = (f32)result.glyph_height/global_atlas_height;

   u32 glyph_count = 0;
   Glyph *dest = result.glyphs;
   for(umm y = 0;
       y < global_atlas_height/result.glyph_height;
       ++y)
   {
      for(umm x = 0;
          x < global_atlas_width/result.glyph_width;
          ++x)
      {
         ++glyph_count;
         if(glyph_count == array_count(result.glyphs) + 1)
         {
            return result;
         }

         dest->rect.x = (f32)x*result.glyph_width/global_atlas_width;
         dest->rect.y = (f32)y*result.glyph_height/global_atlas_height;
         dest->rect.w = glyph_width_norm;
         dest->rect.h = glyph_height_norm;

         ++dest;
      }
   }

   return result;
}

local char
a_get_closest_ascii(char glyph)
{
   char result = 0;

   u8 byte = (u8)glyph;
   result = glyph;

   return result;
}

local Glyph *
a_get_atlas_glyph(Font *font, char glyph)
{
   Glyph *result = 0;

#if 0
   // char close_glyph = a_get_closest_ascii(glyph);
#else
   char close_glyph = glyph;
#endif
   result = font->glyphs + close_glyph;

   return result;
}
