#if !defined(memory_h)

typedef struct
{
   void *base;
   umm used;
   umm size;
} Mandala;

typedef struct
{
   void *base;
   umm read;
   umm write;
   umm type_size;
   umm count;
   umm capacity;
} Queue;

typedef struct
{
   void *base;
   umm type_size;
   umm count;
   umm capacity;
} Stack;

#define memory_h
#endif