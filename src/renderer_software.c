
local f32
lerp(f32 val, f32 dest, f32 perc)
{
   f32 result = (1 - perc)*val + perc*dest;

   return result;
}

local void
render_clear(Bitmap *output)
{
   umm size = output->width*output->height;
   u32 *color = (u32 *)output->memory;
   while(size--)
   {
      *color++ = 0;
   }
}

local void
render_rect(Bitmap *output, v2 pos, v2 dim, v4 color_)
{
   int x = (int)round_f32(pos.x);
   int y = (int)round_f32(pos.y);

   u32 color = ((u32)round_f32(color_.a)&0xFF) << 24 |
               ((u32)round_f32(color_.r)&0xFF) << 16 |
               ((u32)round_f32(color_.g)&0xFF) << 8  |
               ((u32)round_f32(color_.b)&0xFF) << 0;

   u32 *dest_row = (u32 *)output->memory + y*output->width + x;
   for(umm y_scan = 0;
       y_scan < dim.y;
       ++y_scan)
   {
      u32 *dest = dest_row;
      for(umm x_scan = 0;
          x_scan < dim.x;
          ++x_scan)
      {
         f32 alpha = (f32)((color & 0xff000000) >> 24);
         f32 red =   (f32)((color & 0x00ff0000) >> 16);
         f32 green = (f32)((color & 0x0000ff00) >> 8);
         f32 blue =  (f32)((color & 0x000000ff) >> 0);

         f32 d_alpha = (f32)((*dest & 0xff000000) >> 24);
         f32 d_red =   (f32)((*dest & 0x00ff0000) >> 16);
         f32 d_green = (f32)((*dest & 0x0000ff00) >> 8);
         f32 d_blue =  (f32)((*dest & 0x000000ff) >> 0);

         f32 norm_alpha = alpha/255.0f;
         f32 norm_d_alpha = d_alpha/255.0f;
         red /= 255.0f;
         green /= 255.0f;
         blue /= 255.0f;
         d_red /= 255.0f;
         d_green /= 255.0f;
         d_blue /= 255.0f;

         // Note: Non-premultiplied Alpha blending
         // out_a = src_a + dst_a(1-src_a)
         // out_rgb = (src_rgb*src_a + dst_rgb*dst_a(1 - src_a))/out_a
         f32 out_alpha = norm_alpha + norm_d_alpha*(1.0f - norm_alpha);
         f32 inv_out_alpha = 1.0f/out_alpha;

         red = (red*norm_alpha + d_red*norm_d_alpha*(1.0f - norm_alpha))*inv_out_alpha;
         green = (green*norm_alpha + d_green*norm_d_alpha*(1.0f - norm_alpha))*inv_out_alpha;
         blue = (blue*norm_alpha + d_blue*norm_d_alpha*(1.0f - norm_alpha))*inv_out_alpha;

         out_alpha *= 255.0f;
         red *= 255.0f;
         green *= 255.0f;
         blue *= 255.0f;

         u32 final_color = (((u32)out_alpha & 0xff) << 24)|
                           (((u32)red   & 0xff) << 16)|
                           (((u32)green & 0xff) << 8)|
                           (((u32)blue  & 0xff) << 0);

         *dest++ = final_color;
      }
      dest_row += output->width;
   }
}

local void
render_bitmap(Bitmap *output, Bitmap bitmap, int x, int y)
{
   u32 *src = (u32 *)bitmap.memory;
   u32 *dest_row = (u32 *)output->memory + y*output->width + x;
   for(umm y_scan = 0;
       y_scan < bitmap.height;
       ++y_scan)
   {
      u32 *dest = dest_row;
      for(umm x_scan = 0;
          x_scan < bitmap.width;
          ++x_scan)
      {
         f32 alpha = (f32)((*src & 0xFF000000) >> 24);
         f32 red   = (f32)((*src & 0x00FF0000) >> 16);
         f32 green = (f32)((*src & 0x0000FF00) >> 8);
         f32 blue  = (f32)((*src & 0x000000FF) >> 0);

         f32 d_alpha = (f32)((*dest & 0xFF000000) >> 24);
         f32 d_red =   (f32)((*dest & 0x00FF0000) >> 16);
         f32 d_green = (f32)((*dest & 0x0000FF00) >> 8);
         f32 d_blue =  (f32)((*dest & 0x000000FF) >> 0);

         f32 norm_alpha = alpha/255.0f;

         red   /= 255.0f;
         green /= 255.0f;
         blue  /= 255.0f;
         d_red   /= 255.0f;
         d_green /= 255.0f;
         d_blue  /= 255.0f;

         // Note: premultiplied alpha, alpha blending
         // out_a   = src_a + dst_a(1 - src_a)
         // out_rgb = src_rgb + dst_rgb(1 - src_a)

         f32 out_alpha = alpha + d_alpha*(1 - norm_alpha);

         red =   (red + d_red*(1 - norm_alpha));
         green = (green + d_green*(1 - norm_alpha));
         blue =  (blue + d_blue*(1 - norm_alpha));

         red   *= 255.0f;
         green *= 255.0f;
         blue  *= 255.0f;

         u32 color = ((u32)d_alpha << 24) |
                     ((u32)red   << 16) |
                     ((u32)green << 8)  |
                     ((u32)blue  << 0);

         *dest++ = color;
         ++src;
      }
      dest_row += output->width;
   }
}

local void
render_bitmap_atlas(Bitmap *output, Rect rect, Atlas *atlas, int x, int y, f32 scale)
{
   u32 left =   (u32)round_f32(rect.x*atlas->width);
   u32 right =  (u32)round_f32((rect.x + rect.w)*atlas->width);
   u32 top =    (u32)round_f32(rect.y*atlas->height);
   u32 bottom = (u32)round_f32((rect.y + rect.h)*atlas->height);

   u32 bitmap_width = right - left;
   u32 bitmap_height = bottom - top;

   f32 u = 0;
   f32 v = 0;

   u32 *src = (u32 *)atlas->memory + top*atlas->width + left;
   u32 *dest_row = (u32 *)output->memory + y*output->width + x;
   for(u32 y_scan = 0;
       y_scan < bitmap_height*scale;
       ++y_scan)
   {
      // u32 *src = src_row;
      u32 *dest = dest_row;
      f32 v = (f32)y_scan/((f32)bitmap_height*scale);
      for(u32 x_scan = 0;
          x_scan < bitmap_width*scale;
          ++x_scan)
      {
         f32 u = (f32)x_scan/((f32)bitmap_width*scale);

         umm u_index = (umm)(u*bitmap_width);
         umm v_index = (umm)(v*bitmap_height);
         u32 src_color = src[v_index*atlas->width + u_index];
         
         f32 alpha = (f32)((src_color & 0xFF000000) >> 24);
         f32 red   = (f32)((src_color & 0x00FF0000) >> 16);
         f32 green = (f32)((src_color & 0x0000FF00) >> 8);
         f32 blue  = (f32)((src_color & 0x000000FF) >> 0);

         f32 d_alpha = (f32)((*dest & 0xFF000000) >> 24);
         f32 d_red   = (f32)((*dest & 0x00FF0000) >> 16);
         f32 d_green = (f32)((*dest & 0x0000FF00) >> 8);
         f32 d_blue  = (f32)((*dest & 0x000000FF) >> 0);

         f32 norm_alpha = alpha/255.0f;

         red   /= 255.0f;
         green /= 255.0f;
         blue  /= 255.0f;
         d_red   /= 255.0f;
         d_green /= 255.0f;
         d_blue  /= 255.0f;

         // Note: premultiplied alpha, alpha blending
         // out_a   = src_a + dst_a(1 - src_a)
         // out_rgb = src_rgb + dst_rgb(1 - src_a)

         f32 out_alpha = alpha + d_alpha*(1 - norm_alpha);
         
         red =   (red + d_red*(1 - norm_alpha));
         green = (green + d_green*(1 - norm_alpha));
         blue =  (blue + d_blue*(1 - norm_alpha));

         red   *= 255.0f;
         green *= 255.0f;
         blue  *= 255.0f;

         u32 color = ((u32)out_alpha << 24) |
                     ((u32)red   << 16) |
                     ((u32)green << 8)  |
                     ((u32)blue  << 0);

         *dest++ = color;
         // ++src;
      }
      // src_row += atlas->width;
      dest_row += output->width;
   }
}

local void
render_bitmap_scale(Bitmap *output, Bitmap bitmap, int x, int y, f32 scale)
{
   u32 *src = (u32 *)bitmap.memory;
   u32 *dest_row = (u32 *)output->memory + y*output->width + x;
   for(umm y_scan = 0;
       y_scan < bitmap.height*scale;
       ++y_scan)
   {
      u32 *dest = dest_row;
      f32 v = (f32)y_scan/((f32)bitmap.height*scale);
      for(umm x_scan = 0;
          x_scan < bitmap.width*scale;
          ++x_scan)
      {
         f32 u = (f32)x_scan/((f32)bitmap.width*scale);

         umm u_index = (umm)(u*bitmap.width);
         umm v_index = (umm)(v*bitmap.height);
         u32 src_color = src[v_index*bitmap.width + u_index];

         f32 alpha = (f32)((src_color & 0xFF000000) >> 24);
         f32 red   = (f32)((src_color & 0x00FF0000) >> 16);
         f32 green = (f32)((src_color & 0x0000FF00) >> 8);
         f32 blue  = (f32)((src_color & 0x000000FF) >> 0);

         f32 d_alpha = (f32)((*dest & 0xFF000000) >> 24);
         f32 d_red   = (f32)((*dest & 0x00FF0000) >> 16);
         f32 d_green = (f32)((*dest & 0x0000FF00) >> 8);
         f32 d_blue  = (f32)((*dest & 0x000000FF) >> 0);

         f32 norm_alpha = alpha/255.0f;

         red   /= 255.0f;
         green /= 255.0f;
         blue  /= 255.0f;
         d_red   /= 255.0f;
         d_green /= 255.0f;
         d_blue  /= 255.0f;

         // Note: premultiplied alpha, alpha blending
         // out_a   = src_a + dst_a(1 - src_a)
         // out_rgb = src_rgb + dst_rgb(1 - src_a)

         f32 out_alpha = alpha + d_alpha*(1 - norm_alpha);

         red =   (red + d_red*(1 - norm_alpha));
         green = (green + d_green*(1 - norm_alpha));
         blue =  (blue + d_blue*(1 - norm_alpha));

         red   *= 255.0f;
         green *= 255.0f;
         blue  *= 255.0f;

         u32 color = ((u32)out_alpha << 24) |
                     ((u32)red   << 16) |
                     ((u32)green << 8)  |
                     ((u32)blue  << 0);

         *dest++ = color;
         // ++src;
      }
      dest_row += output->width;
   }
}

local Bitmap
render_blur(Bitmap *output, Rect rect, u32 kernel_size, bool on_vertical)
{
   Bitmap result = {0};
   result.width =  (u32)rect.w;
   result.height = (u32)rect.h;
   result.memory = push_size(&temp_mandala, (umm)(rect.w*rect.h*4));

   // Todo: Clip Rect to output dimensions

   // u32 end_x = min((u32)(rect.x + rect.w), output->width);
   // u32 end_y = min((u32)(rect.y + rect.h), output->height);

   // Note: 1.0f/(square_root(2*PI32)*sigma)*power32(E32, (-(x*x))/(2*(sigma*sigma)))

   f32 sigma = 5.0f;
   f32 kernel[127] = {0};
   for(s32 index = 0;
       index < 127;
       ++index)
   {
      s32 x = index - kernel_size;
      kernel[index] = power32(E32, (f32)(-(x*x))/(2.0f*(sigma*sigma)));
   }

   u32 *src = (u32 *)output->memory + (u32)rect.x + (u32)rect.y*output->width;
   u32 *dest = result.memory;
   for(s32 scan_y = (umm)rect.y;
       scan_y < (umm)(rect.y + rect.h);
       ++scan_y)
   {
      src = (u32 *)output->memory + scan_y*output->width + (umm)rect.x;
      dest = (u32 *)result.memory + result.width*(scan_y - (s32)rect.y);
      for(s32 scan_x = (umm)rect.x;
          scan_x < (umm)(rect.x + rect.w);
          ++scan_x)
      {
         // Note: Yeaaaah I don't know what I'm doing
         f32 sum_r = 0.0f;
         f32 sum_g = 0.0f;
         f32 sum_b = 0.0f;
         u32 amount = 0;
         for(s32 index = -((s32)kernel_size/2);
             index <= ((s32)kernel_size/2);
             ++index)
         {
            f32 perc = kernel[index];
            if(on_vertical)
            {
               if(scan_y + index >= rect.y &&
                  scan_y + index < rect.y + rect.h)
               {
                  u32 color = *(src + index*(s32)output->width);
                  sum_r += perc*((color&0x00FF0000) >> 16);
                  sum_g += perc*((color&0x0000FF00) >>  8);
                  sum_b += perc*((color&0x000000FF) >>  0);
                  ++amount;
               }
            }
            else
            {
               if(scan_x + index >= rect.x &&
                  scan_x + index < rect.x + rect.w)
               {
                  u32 color = *(src + index);
                  sum_r += perc*((color&0x00FF0000) >> 16);
                  sum_g += perc*((color&0x0000FF00) >>  8);
                  sum_b += perc*((color&0x000000FF) >>  0);
                  ++amount;
               }
            }
         }
         f32 final_r = sum_r;
         f32 final_g = sum_g;
         f32 final_b = sum_b;

         *dest++ = (0xFF         << 24)|
                   ((u32)final_r << 16) |
                   ((u32)final_g << 8)|
                   ((u32)final_b << 0);
         ++src;
      }
   }

   return result;
}

local void
render_blur_2(Bitmap *output, Rect rect, u32 kernel_size)
{
   // Todo: make this into a gaussian blur

   // Todo: Clip Rect to output dimensions

   // u32 end_x = min((u32)(rect.x + rect.w), output->width);
   // u32 end_y = min((u32)(rect.y + rect.h), output->height);

   f32 kernel[128] = {0};
   for(u32 index = 0;
       index < 128;
       ++index)
   {
      kernel[index] = 1.0f/(f32)kernel_size;
   }

   u32 *src = (u32 *)output->memory + (u32)rect.x + (u32)rect.y*output->width;
   u32 *dest = (u32 *)output->memory + (u32)rect.x + (u32)rect.y*output->width;
   for(int times = 0;
       times < 2;
       ++times)
   {
      bool on_vertical = times == 0 ? false : true;
      for(s32 scan_y = (umm)rect.y;
          scan_y < (umm)(rect.y + rect.h);
          ++scan_y)
      {
         src = (u32 *)output->memory + scan_y*output->width + (umm)rect.x;
         dest = (u32 *)output->memory + output->width*scan_y + (u32)rect.x;
         for(s32 scan_x = (umm)rect.x;
             scan_x < (umm)(rect.x + rect.w);
             ++scan_x)
         {
            f32 sum_r = 0.0f;
            f32 sum_g = 0.0f;
            f32 sum_b = 0.0f;
            f32 amount = 0;
            for(s32 index = -((s32)kernel_size/2);
                index <= ((s32)kernel_size/2);
                ++index)
            {
               if(on_vertical)
               {
                  if(scan_y + index >= rect.y &&
                     scan_y + index < rect.y + rect.h)
                  {
                     u32 color = *(src + index*(s32)output->width);
                     sum_r += (color&0x00FF0000) >> 16;
                     sum_g += (color&0x0000FF00) >>  8;
                     sum_b += (color&0x000000FF) >>  0;
                     ++amount;
                  }
               }
               else
               {
                  if(scan_x + index >= rect.x &&
                     scan_x + index < rect.x + rect.w)
                  {
                     u32 color = *(src + index);
                     sum_r += (color&0x00FF0000) >> 16;
                     sum_g += (color&0x0000FF00) >>  8;
                     sum_b += (color&0x000000FF) >>  0;
                     ++amount;
                  }
               }
            }
            f32 avg_r = sum_r/amount;
            f32 avg_g = sum_g/amount;
            f32 avg_b = sum_b/amount;

            *dest++ = (0xFF       << 24)|
                      ((u32)avg_r << 16)|
                      ((u32)avg_g << 8)|
                      ((u32)avg_b << 0);
            ++src;
         }
      }

      src = (u32 *)output->memory + (u32)rect.x + (u32)rect.y*output->width;
      dest = (u32 *)output->memory + (u32)rect.x + (u32)rect.y*output->width;
   }
}
