#if !defined(jec_h)

#include <stdarg.h>
#include <math.h>

#define local static
#define global static
#define persistent static

typedef char s8;
typedef short s16;
typedef long s32;
typedef long long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

typedef size_t umm;
typedef s64 smm;

#define true 1
#define false 0

typedef u8 bool;

typedef s32 b32;

#define PI32 3.1415926535897932384626433f
#define E32 2.718281828459f

#define U32_MAX 0xFFFFFFFF
#define U64_MAX 0xFFFFFFFFFFFFFFFFULL

#define kilo(val) (val*1024)
#define mega(val) (kilo(val)*1024ULL)
#define giga(val) (mega(val)*1024ULL)

#define array_count(arr) ((sizeof(arr))/(sizeof((arr)[0])))

#define assert(expr) if(!(expr)){*(int *)0 = 0;}
#define invalid_default_case default:{assert(0)}

typedef union
{
   f32 e[2];
   struct
   {
      f32 x, y;
   };
} v2;

typedef union
{
   s32 e[2];
   struct
   {
      s32 x, y;
   };
} v2i;

typedef union
{
   f32 e[4];
   struct
   {
      f32 x, y, z, w;
   };
   struct
   {
      f32 r, g, b, a;
   };
} v4;

typedef union
{
   struct
   {
      f32 x, y, w, h;
   };
} Rect;

local f32
round_f32(f32 value)
{
   f32 result = (f32)((s64)(value + 0.5f));

   return result;
}

local f32
power32(f32 value, f32 exp)
{
   // Todo: Write our own pow function
   f32 result = powf(value, exp);
   return result;
}

local f32
square_root(f32 value)
{
   // Todo: Write our own sqrt function
   f32 result = sqrtf(value);
   return result;
}

local inline umm
measure_string(char *string)
{
   umm result = 0;

   while(*string++)
   {
      ++result;
   }

   return result;
}

#define MAX_KMP_ARRAY_SIZE 1024
local umm
substring(u8 *src, umm src_size, u8 *pattern)
{
   // Note: Longer startup, but also potential search optimizations... but mostly for ascii characters like words
   umm result = 0;

   u32 pattern_size = (u32)measure_string((char *)pattern);

   assert(pattern_size <= src_size);
   assert(pattern_size <= MAX_KMP_ARRAY_SIZE);
   u32 failure_function[MAX_KMP_ARRAY_SIZE];
   failure_function[0] = 0;
   u32 i = 1;
   u32 j = 0;
   while(i < pattern_size)
   {
      if(pattern[j] == pattern[i])
      {
         failure_function[i] = j + 1;
         ++j;
      }
      else
      {
         if(j == 0)
         {
            failure_function[i] = 0;
         }
         else
         {
            j = failure_function[j - 1];
         }
      }

      ++i;
   }

   umm pattern_index = 0;
   for(umm index = 0;
       index < src_size;
       ++index)
   {
      if(pattern[pattern_index] == src[index])
      {
         ++pattern_index;
         if(pattern_index == pattern_size)
         {
            result = (index + 1) - pattern_size;
            break;
         }
      }
      else
      {
         pattern_index = (pattern_index > 0) ? failure_function[pattern_index - 1] : 0;
      }
   }

   return result;
}

local umm
substring_count(u8 *src, umm src_size, u8 *pattern, umm pattern_size)
{
   umm result = 0;

   assert(pattern_size <= src_size);
   assert(pattern_size <= MAX_KMP_ARRAY_SIZE);
   u16 failure_function[MAX_KMP_ARRAY_SIZE] = {0};
   failure_function[0] = 0;
   u16 i = 1;
   u16 j = 0;
   while(i < pattern_size)
   {
      if(pattern[j] == pattern[i])
      {
         failure_function[i] = j + 1;
         ++j;
      }
      else
      {
         if(j == 0)
         {
            failure_function[i] = 0;
         }
         else
         {
            j = failure_function[j - 1];
         }
      }

      ++i;
   }

   umm pattern_index = 0;
   for(umm index = 0;
       index < src_size;
       ++index)
   {
      if(pattern[pattern_index] == src[index])
      {
         ++pattern_index;
         if(pattern_index == pattern_size)
         {
            ++result;
            pattern_index = 0;
         }
      }
      else
      {
         pattern_index = (pattern_index > 0) ? failure_function[pattern_index - 1] : 0;
      }
   }

   return result;
}

local inline void *
copy_memory(void *dest, void *source, size_t size)
{
   void *result = dest;

   u8 *dest_ = (u8 *)dest;
   u8 *source_ = (u8 *)source;
   while(size--)
   {
      *dest_++ = *source_++;
   }

   return result;
}

#define zero_struct(structure) zero_size(structure, sizeof(structure))

local inline void
zero_size(void *base, umm size)
{
   u8 *dest = (u8 *)base;
   while(size--)
   {
      *dest++ = 0;
   }
}

local inline s32
compare_string(char *a, char *b)
{
   s32 result = true;

   while(*a && *b)
   {
      if(*a++ != *b++)
      {
         result = false;
         break;
      }
   }

   return result;
}

local inline umm
copy_string(char *dest, char *source)
{
   umm result = 0;

   while(*source)
   {
      *dest++ = *source++;
      ++result;
   }
   *dest = 0;

   return result;
}

local inline umm
append_string(char *dest, char *annex)
{
   umm result = 0;

   while(*dest)
   {
      ++dest;
      ++result;
   }

   while(*annex)
   {
      *dest++ = *annex++;
      ++result;
   }
   *dest = 0;

   return result;
}

local inline void
lower_string(char *src)
{
   while(*src)
   {
      // Note: This bit says if it's lowercase or not, 1 if lowercase
      //   V
      // 0010 0000
      // 0x20, or 32 is the value
      if(*src >= 'A' && *src <= 'Z')
      {
         *src |= 0x20;
      }
      ++src;
   }
}

local inline void
upper_string(char *src)
{
   while(*src)
   {
      // Note: This bit says if it's lowercase or not, 1 if lowercase
      //   V
      // 0010 0000
      // 0x20, or 32 is the value
      if(*src >= 'a' && *src <= 'z')
      {
         *src &= 0xDF;
      }
      ++src;
   }
}

local inline bool
is_nan(f64 value)
{
   bool result = false;

   u64 casted_float = 0;
   copy_memory(&casted_float, &value, sizeof(f64));
   if((casted_float & 0x7FF0000000000000) == 0x7FF0000000000000 &&
      (casted_float & 0x000FFFFFFFFFF) !=    0x0000000000000000)
   {
      result = true;
   }

   return result;
}

local inline bool
is_infinite(f64 value)
{
   bool result = false;

   u64 casted_float = 0;
   copy_memory(&casted_float, &value, sizeof(f64));
   if((casted_float & 0x7FF0000000000000) == 0x7FF0000000000000 &&
      (casted_float & 0x000FFFFFFFFFF) ==    0x0000000000000000)
   {
      result = true;
   }

   return result;
}

local inline bool
is_numeric(char character)
{
   bool result = false;

   if(character >= '0' &&
      character <= '9')
   {
      result = true;
   }

   return result;
}

local u64
read_uint_arg(va_list *arg_list, umm size)
{
   u64 result = 0;

   switch(size)
   {
      case 1: result = va_arg(*arg_list, u8);  break;
      case 2: result = va_arg(*arg_list, u16); break;
      case 4: result = va_arg(*arg_list, u32); break;
      case 8: result = va_arg(*arg_list, u64); break;
         invalid_default_case;
   }

   return(result);
}

local s64
read_int_arg(va_list *arg_list, umm size)
{
   s64 result = 0;

   switch(size)
   {
      case 1: result = va_arg(*arg_list, s8);  break;
      case 2: result = va_arg(*arg_list, s16); break;
      case 4: result = va_arg(*arg_list, s32); break;
      case 8: result = va_arg(*arg_list, s64); break;
         invalid_default_case;
   }

   return(result);
}

local inline f64
read_f64_arg(va_list *arg_list, umm size)
{
   f64 result = 0;

   if(size == 4)
   {
      result = va_arg(*arg_list, float);
   }
   else
   {
      result = va_arg(*arg_list, double);
   }

   return(result);
}

char decimal[] = "0123456789";
char lower_hex[] = "0123456789abcdef";
char upper_hex[] = "0123456789ABCDEF";
local inline umm
ascii_from_u64(char *dest_, u64 integer, s32 base, char *digits)
{
   umm result = 0;

   char *dest = dest_;
   do
   {
      *dest++ = *(digits + (integer%base));
      integer /= base;
   } while(integer > 0);

   result = dest - dest_;

   while(dest > dest_)
   {
      --dest;
      char Temp = *dest;
      *dest = *dest_;
      *dest_ = Temp;
      ++dest_;
   }

   return(result);
}

local inline umm
ascii_from_s64(char *dest_, s64 integer, s32 base, char *digits)
{
   umm result = 0;

   char *dest = dest_;

   if(integer < 0)
   {
      *dest++ = '-';
      integer = -integer;
   }

   do
   {
      *dest++ = *(digits + (integer%base));
      integer /= 10;
   } while(integer > 0);

   result = dest - dest_;

   while(dest > dest_)
   {
      --dest;
      char Temp = *dest;
      *dest = *dest_;
      *dest_ = Temp;
      ++dest_;
   }

   return(result);
}

local inline umm
ascii_from_f64(char *dest_, f64 val, s32 precision)
{
   umm result = 0;

   char *dest = dest_;

   if(val < 0)
   {
      *dest++ = '-';
      val = -val;
   }

   if(is_nan(val))
   {
      copy_memory(dest, "NaN", 3);
      dest += 3;
   }
   else if(is_infinite(val))
   {
      copy_memory(dest, "Inf.", 4);
      dest += 4;
   }
   else
   {
      u64 int_part = (u64)val;
      val -= (f64)int_part;
      dest += ascii_from_u64(dest, int_part, 10, decimal);

      if(precision > 0)
      {
         *dest++ = '.';
      }

      for(s32 index = 0;
          index < precision;
          ++index)
      {
         val *= 10;
         int_part = (u64)val;
         val -= (f64)int_part;
         *dest++ = *(decimal + int_part);
      }
   }

   result = dest - dest_;
   return(result);
}

local umm
format_string(char *dest_, char *Format, ...)
{
   umm result = 0;

   va_list arg_list;
   va_start(arg_list, Format);

   char *dest = dest_;
   char *scan = Format;
   while(*scan)
   {
      if(*scan == '%')
      {
         ++scan;
         // Note: %[flags][width][.precision][length]specifier

         // Note: Flags
         bool left_justify = false;
         bool force_sign = false;
         bool insert_space = false;
         bool add_grammar = false;
         bool pad_with_zeroes = false;
         if(*scan == '-')
         {
            left_justify = true;
            ++scan;
         }
         if(*scan == '+')
         {
            force_sign = true;
            ++scan;
         }
         if(*scan == ' ')
         {
            insert_space = true;
            ++scan;
         }
         if(*scan == '#')
         {
            add_grammar = true;
            ++scan;
         }
         if(*scan == '0')
         {
            pad_with_zeroes = true;
            ++scan;
         }

         // Note: Width
         s32 width = 0;
         if(*scan == '*')
         {

            ++scan;
         }
         else
         {
            while(is_numeric(*scan))
            {
               width *= 10;
               width += *scan - '0';
               ++scan;
            }
         }

         // Note: Precision
         s32 precision = 3;
         if(*scan == '.')
         {
            ++scan;
            precision = 0;
            while(is_numeric(*scan))
            {
               precision *= 10;
               precision += (s32)(*scan - '0');
               ++scan;
            }
         }

         // Note: Length
         umm int_size = 4; // Todo/Research: It feels like everything is defaulting to 8 bit?
         switch(*scan)
         {
            case 'h':
            {
               ++scan;
               if(*scan == 'h')
               {
                  int_size = 1;
                  ++scan;
               }
               else
               {
                  int_size = 2;
               }
            } break;
            case 'l':
            {
               ++scan;
               if(*scan == 'l')
               {
                  int_size = 8;
                  ++scan;
               }
               else
               {
                  int_size = 4;
               }
            } break;
         }

         // Note: Specifier
         switch(*scan)
         {
            case '%':
            {
               *dest++ = *scan++;
            } break;
            case 'u':
            { // Note: Unsigned integer
               u64 integer = read_uint_arg(&arg_list, int_size);

               dest += ascii_from_u64(dest, integer, 10, decimal);
            } break;
            case 'd':
            case 'i':
            { // Note: Signed integer
               s64 integer = read_int_arg(&arg_list, int_size);

               dest += ascii_from_s64(dest, integer, 10, decimal);
            } break;
            case 'x':
            case 'X':
            { // Note: Unsigned Hex
               char *digits = upper_hex;
               if(*scan == 'x')
               {
                  digits = lower_hex;
               }
               u64 integer = read_uint_arg(&arg_list, int_size);

               dest += ascii_from_u64(dest, integer, 16, digits);
            } break;
            case 'f':
            case 'F':
            {
               f64 val = read_f64_arg(&arg_list, 8); // Research: Why the heck does this work for both 32bit floating points and doubles?

               dest += ascii_from_f64(dest, val, precision);
            } break;
            case 's':
            {
               char *Buffer = va_arg(arg_list, char *);

               copy_string(dest, Buffer);
               dest += measure_string(Buffer);
            } break;
            case 'c':
            {
               char character = va_arg(arg_list, char);

               *dest++ = character;
            } break;
            invalid_default_case;
         }
         ++scan;
      }
      else
      {
#if 1
         *dest++ = *scan++;
#else
         if(*scan != '\n')
         {
            *dest++ = *scan++;
         }
         else
         {
            ++scan;
         }
#endif
      }
   }
   va_end(arg_list);

   *dest = 0;
   result = dest - dest_;
   return(result);
}

local s64
s64_from_ascii(char *buffer)
{
   s64 result = 0;

   bool is_negative = false;
   while(*buffer)
   {
      if(*buffer == '-' ||
         (*buffer >= '0' && *buffer <= '9'))
      {
         if(*buffer == '-')
         {
            if(is_negative)
            {
               break;
            }
            is_negative = !is_negative;
         }

         result *= 10;
         result += *buffer - '0';
      }
      else
      {
         break;
      }

      ++buffer;
   }

   if(is_negative)
   {
      result = -result;
   }

   return result;
}

local void
get_file_from_path(char *dest_, char *src)
{
   char *dest = dest_;
   while(*src)
   {
      if(*src == '/' || *src == '\\')
      {
         dest = dest_;
         ++src;
      }
      else
      {
         *dest++ = *src++;
      }
   }
   *dest = 0;
}

/////////////////////////////////////////////
//////     pseudo random number generation //
/////////////////////////////////////////////

typedef struct
{
   u64 state;

   u32 d_count;
} Entropy;


void
seed_entropy(Entropy *entropy, u64 seed)
{
   if(seed > 0)
   {
      entropy->state = seed;
   }
   else
   {
      seed = __rdtsc();
      entropy->state = (seed*U64_MAX);
   }
}

u64
xor_shift(Entropy *entropy)
{
   u64 result = 0;

   result = entropy->state;
   result ^= result << 23;
   result ^= result >> 43;
   result ^= result << 11;
   entropy->state = result;
   ++entropy->d_count;

   return result;
}

f32
random_unilateral(Entropy *entropy)
{
   f32 result = (f32)((f64)xor_shift(entropy)/(f64)U64_MAX);
   return result;
}

f32
random_bilateral(Entropy *entropy)
{
   f32 result = -1.0f + 2.0f*random_unilateral(entropy);
   return result;
}

#define jec_h
#endif
