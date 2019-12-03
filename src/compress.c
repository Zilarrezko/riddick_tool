
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

typedef unsigned char u8;
typedef unsigned short u16;

typedef size_t umm;

typedef u8 bool;
#define true  1
#define false 0

#define assert(expr) if(!(expr)){*(int *)0=0;}

#define kilo(val) ((val)*1024)
#define mega(val) (kilo(val)*1024)
#define giga(val) (mega(val)*1024)

typedef struct
{
   void *memory;
   umm size;
} Read_File_Result;

static Read_File_Result
read_entire_file_w(char *filename)
{
   Read_File_Result result = {0};

   printf("opening...\n");

   HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
   if(!file)
   {
      fprintf(stderr, "Error: could not find/open file for reading %s\n", filename);
      fprintf(stderr, "error number: %d\n", errno);
      assert(file);
   }

   printf("sizing...\n");

   LARGE_INTEGER file_size;
   GetFileSizeEx(file, &file_size);
   result.memory = malloc(file_size.QuadPart);
   result.size = file_size.QuadPart;

   printf("reading...\n");

   unsigned long long total_bytes_read = 0;
   while(file_size.QuadPart)
   {
      DWORD bytes_read;
      ReadFile(file, (u8 *)result.memory + total_bytes_read, (DWORD)(result.size - total_bytes_read), &bytes_read, 0);
      total_bytes_read += bytes_read;
      file_size.QuadPart -= bytes_read;
   }

   CloseHandle(file);
   return result;
}

static Read_File_Result
read_entire_file(char *filename)
{
   Read_File_Result result = {0};

   printf("opening...\n");

   FILE *file = fopen(filename, "rb");
   int error = errno;
   if(!file)
   {
      fprintf(stderr, "Error: could not find/open file for reading %s\n", filename);
      fprintf(stderr, "error number: %d\n", errno);
      assert(file);
   }

   printf("sizing...\n");

   fseek(file, 0, SEEK_END);
   error = errno;
   umm size = ftell(file);
   if(size == (ULLONG_MAX))
   {
      error = errno;
   }
   rewind(file);
   result.size = size;
   result.memory = malloc(size);

   printf("reading...\n");

   umm bytes_read = fread(result.memory, 1, size, file);
   if(bytes_read != size)
   {
      fprintf(stderr, "Error: file read error, bytes read was not the file's size\n");
      fprintf(stderr, "error number: %d\n", errno);
      assert(bytes_read == size);
   }

   fclose(file);
   return result;
}

static void
free_entire_file(Read_File_Result *read_result)
{
   if(read_result)
   {
      free(read_result->memory);
      read_result->size = 0;
   }
}

static void
write_entire_file(char *filename, void *buffer, umm size)
{
   FILE *file = fopen(filename, "wb");
   if(!file)
   {
      fprintf(stderr, "Error: could not find/open file for writting %s\n", filename);
      fprintf(stderr, "error number: %d\n", errno);
      assert(file);
   }

   umm bytes_written = fwrite(buffer, 1, size, file);
   if(bytes_written != size)
   {
      fprintf(stderr, "Error: file write error, bytes written was not the buffer's size");
      fprintf(stderr, "error number: %d\n", errno);
      assert(bytes_written == size);
   }

   fclose(file);
}

static void
rle_compress(void *buffer, umm size, void *out, umm *compress_size)
{
   u8 literal_buffer[256];
   u8 literal_length = 0;

   u8 *src = (u8 *)buffer;
   u8 *dest = (u8 *)out;
   umm scan = 0;
   while(scan <= size)
   {
      u8 run_length = 0;
      u8 run_byte = (scan == size) ? 0 : src[scan];
      while(src[scan + run_length] == run_byte &&
            run_length + scan < size)
      {
         if(run_length == 0xff)
         {
            break;
         }
         ++run_length;
      }

      if(run_length > 3 ||
         literal_length == 0xff)
      {
         *dest++ = literal_length;
         for(umm index = 0;
             index < literal_length;
             ++index)
         {
            *dest++ = literal_buffer[index];
         }
         literal_length = 0;

         *dest++ = run_length;
         *dest++ = run_byte;
         scan += run_length;
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
}

static void
rle_decompress(void *buffer, umm size, void *out, umm *decompress_size)
{
   u8 *dest = (u8 *)out;
   u8 *src = (u8 *)buffer;
   u8 *end = (u8 *)buffer + size;
   while(src < end)
   {
      u8 literal_count = *src++;
      while(literal_count--)
      {
         *dest++ = *src++;
      }

      u8 run_count = *src++;
      u8 run_byte = *src++;
      while(run_count--)
      {
         *dest++ = run_byte;
      }
   }

   if(decompress_size)
   {
      *decompress_size = dest - (u8 *)out;
   }
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

   printf("progress...  00.000%%");

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

      if(scan%1024 == 0)
      {
         printf("\rprogress...  %.3f%% ", 100.0*((double)scan/(double)size));
      }
   }
   printf("\rprogress... 100.000%%");

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
      u8 *reader = src;
      u8 count  = *reader++;
      u8 offset = *reader++;
      src = reader;

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

static void
print_usage()
{
   fprintf(stderr, "\n");
   fprintf(stderr, "Usage:\n");
   fprintf(stderr, "   compress [-c|-d] input output\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "   -c will compress\n");
   fprintf(stderr, "   -d will decompress\n");
   fprintf(stderr, "\n");
}

int
main(int argc, char **args)
{
   int result = 0;

   if(argc != 4)
   {
      print_usage();
      return -1;
   }

   if(args[1][1] == 'c' || args[1][1] == 'd' || args[1][1] == 't')
   {
      char *input_file = args[2];
      char *output_file = args[3];

      Read_File_Result read_result = read_entire_file_w(input_file);

      switch(args[1][1])
      {
         case 'c':
         {
#if 0
            if(strcmp(args[1], "rle") == 0)
            {
               umm compress_size;
               void *compress_buffer = malloc(read_result.size*4);
               rle_compress(read_result.memory, read_result.size, compress_buffer, &compress_size);

               write_entire_file(output_file, compress_buffer, compress_size);
               printf("decompressed size %llu\n", read_result.size);
               printf("compressed size   %llu\n", compress_size);
            }
            else if(strcmp(args[1], "lz8") == 0)
            {
               umm compress_size;
               void *compress_buffer = malloc(read_result.size*4);
               lz_compress(read_result.memory, read_result.size, compress_buffer, &compress_size);

               write_entire_file(output_file, compress_buffer, compress_size);
               printf("decompressed size %llu\n", read_result.size);
               printf("compressed size   %llu\n", compress_size);
            }
            else if(strcmp(args[1], "lz16") == 0)
            {
               umm compress_size;
               void *compress_buffer = malloc(read_result.size*4);
               lz_compress(read_result.memory, read_result.size, compress_buffer, &compress_size);

               write_entire_file(output_file, compress_buffer, compress_size);
               printf("decompressed size %llu\n", read_result.size);
               printf("compressed size   %llu\n", compress_size);
            }
            else
            {
               fprintf(stderr, "Unrecognized format specifier %s\n", args[1]);
            }
#else
            umm compress_size;
            void *compress_buffer = malloc(read_result.size*4);
            lz_compress(read_result.memory, read_result.size, compress_buffer, &compress_size);

            write_entire_file(output_file, compress_buffer, compress_size);
            printf("uncompressed size %llu\n", read_result.size);
            printf("compressed size   %llu\n", compress_size);
#endif
         } break;
         case 'd':
         {
#if 0
            if(strcmp(args[1], "rle") == 0)
            {
               umm decompress_size;
               void *decompress_buffer = malloc(read_result.size*4);
               rle_decompress(read_result.memory, read_result.size, decompress_buffer, &decompress_size);

               write_entire_file(output_file, decompress_buffer, decompress_size);
               printf("compressed size   %llu\n", read_result.size);
               printf("decompressed size %llu\n", decompress_size);
            }
            else if(strcmp(args[1], "lz8") == 0)
            {
               umm decompress_size;
               void *decompress_buffer = malloc(read_result.size*4);
               lz_decompress(read_result.memory, read_result.size, decompress_buffer, &decompress_size);

               write_entire_file(output_file, decompress_buffer, decompress_size);
               printf("compressed size   %llu\n", read_result.size);
               printf("decompressed size %llu\n", decompress_size);
            }
            else if(strcmp(args[1], "lz16") == 0)
            {
               umm decompress_size;
               void *decompress_buffer = malloc(read_result.size*4);
               lz_decompress(read_result.memory, read_result.size, decompress_buffer, &decompress_size);

               write_entire_file(output_file, decompress_buffer, decompress_size);
               printf("compressed size   %llu\n", read_result.size);
               printf("decompressed size %llu\n", decompress_size);
            }
            else
            {
               fprintf(stderr, "Unrecognized format specifier %s\n", args[1]);
            }
#else
            umm decompress_size;
            void *decompress_buffer = malloc(read_result.size*4);
            lz_decompress(read_result.memory, read_result.size, decompress_buffer, &decompress_size);

            write_entire_file(output_file, decompress_buffer, decompress_size);
            printf("compressed size   %llu\n", read_result.size);
            printf("decompressed size %llu\n", decompress_size);
#endif
         } break;
         case 't':
         {
#if 0
            if(strcmp(args[1], "rle") == 0)
            {
               // Todo: On some files the rle test seems to fail, we'll have to look into it
               umm compress_size;
               void *compress_buffer = malloc(read_result.size*4);
               rle_compress(read_result.memory, read_result.size, compress_buffer, &compress_size);

               umm decompress_size;
               void *decompress_buffer = malloc(read_result.size*4);
               rle_decompress(compress_buffer, compress_size, decompress_buffer, &decompress_size);

               bool the_same = true;
               if(memcmp(decompress_buffer, read_result.memory, read_result.size) != 0 ||
                  read_result.size != decompress_size)
               {
                  the_same = false;
               }

               printf("file size         %llu\n", read_result.size);
               printf("compressed size   %llu\n", compress_size);
               printf("decompressed size %llu\n", decompress_size);
               printf("%s\n", the_same ? "SUCCESS" : "FAILURE");
            }
            else if(strcmp(args[1], "lz8") == 0)
            {
               umm compress_size;
               void *compress_buffer = malloc(read_result.size*4);
               lz_compress(read_result.memory, read_result.size, compress_buffer, &compress_size);

               umm decompress_size;
               void *decompress_buffer = malloc(read_result.size*4);
               lz_decompress(compress_buffer, compress_size, decompress_buffer, &decompress_size);

               bool the_same = true;
               if(memcmp(decompress_buffer, read_result.memory, read_result.size) != 0 ||
                  read_result.size != decompress_size)
               {
                  the_same = false;
               }

               printf("file size         %llu\n", read_result.size);
               printf("compressed size   %llu\n", compress_size);
               printf("decompressed size %llu\n", decompress_size);
               printf("%s\n", the_same ? "SUCCESS" : "FAILURE");
            }
            else if(strcmp(args[1], "lz16") == 0)
            {
               umm compress_size;
               void *compress_buffer = malloc(read_result.size*4);
               lz_compress(read_result.memory, read_result.size, compress_buffer, &compress_size);

               umm decompress_size;
               void *decompress_buffer = malloc(read_result.size*4);
               lz_decompress(compress_buffer, compress_size, decompress_buffer, &decompress_size);

               bool the_same = true;
               if(memcmp(decompress_buffer, read_result.memory, read_result.size) != 0 ||
                  read_result.size != decompress_size)
               {
                  the_same = false;
               }

               printf("file size         %llu\n", read_result.size);
               printf("compressed size   %llu\n", compress_size);
               printf("decompressed size %llu\n", decompress_size);
               printf("%s\n", the_same ? "SUCCESS" : "FAILURE");
            }
            else
            {
               fprintf(stderr, "Unrecognized format specifier %s\n", args[1]);
            }
#else
            umm compress_size;
            void *compress_buffer = malloc(read_result.size*4);
            lz_compress(read_result.memory, read_result.size, compress_buffer, &compress_size);

            umm decompress_size;
            void *decompress_buffer = malloc(read_result.size*4);
            lz_decompress(compress_buffer, compress_size, decompress_buffer, &decompress_size);

            bool the_same = true;
            if(memcmp(decompress_buffer, read_result.memory, read_result.size) != 0 ||
               read_result.size != decompress_size)
            {
               the_same = false;
            }

            printf("file size         %llu\n", read_result.size);
            printf("compressed size   %llu\n", compress_size);
            printf("decompressed size %llu\n", decompress_size);
            printf("%s\n", the_same ? "SUCCESS" : "FAILURE");
#endif
         } break;
         default:
         {
            fprintf(stderr, "unknown command %s\n", args[2]);
            assert(!"invalid default case");
         }
      }

      free_entire_file(&read_result);
      printf("I'm done, bro\n");
   }
   else
   {
      print_usage();
   }

   return result;
}