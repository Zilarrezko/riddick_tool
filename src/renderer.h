#if !defined(renderer_h)

typedef enum
{
   R_Type_Clear,
   R_Type_Rect,
   R_Type_Bitmap,
   R_Type_Glyph,
   R_Type_Blur
} R_Render_Type;

typedef struct
{
   v4 color;
} R_Clear;

typedef struct
{
   v4 color;
   v2 pos;
   v2 dim;
} R_Rect;

typedef struct
{
   Bitmap bitmap;
   v2 pos;
} R_Bitmap;

typedef struct
{
   Glyph *glyph;
   Atlas *atlas;
   v2 pos;
   f32 scale;
} R_Glyph;

typedef struct
{
   Rect rect;
   u32 kernel_size;
} R_Blur;

typedef struct
{
   R_Render_Type type;
   union
   {
      R_Clear clear;
      R_Rect rect;
      R_Bitmap bitmap;
      R_Glyph glyph;
      R_Blur blur;
   };
} R_Header;

typedef struct
{
   Mandala mandala;
} R_Context;

#define renderer_h
#endif