#include "memory.h"

#include <string.h>

global Mandala perm_mandala;
global Mandala temp_mandala;

local void *
allocate(size_t size)
{
   void *result = VirtualAlloc(0, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
   return result;
}

local void
deallocate(void *base)
{
   VirtualFree(base, 0, MEM_RELEASE);
}

local Mandala
init_mandala(umm size)
{
   Mandala result = {0};

   result.size = size;
   result.base = allocate(size);

   return result;
}

local void
reset_mandala(Mandala *mandala)
{
   mandala->used = 0;
}

#define push_struct(mandala, struct)   (struct *)push_size(mandala, sizeof(struct))
#define push_array(mandala, type, count) (type *)push_size(mandala, sizeof(type)*count)

local void *
push_size(Mandala *mandala, umm size)
{
   void *result = 0;

#if 0 // Note: Unaligned
   assert(mandala->used + size <= mandala->size);

   result = (void *)((u8 *)mandala->base + mandala->used);
   mandala->used += size;
#else // Note: Aligned
   umm padding = 8 - (mandala->used & (8 - 1)) & (8 - 1);
   umm aligned_used = padding + mandala->used;

   assert(aligned_used + size <= mandala->size);

   result = (void *)((u8 *)mandala->base + aligned_used);
   mandala->used += size + padding;
#endif

   return result;
}

local void *
push_size_aligned(Mandala *mandala, umm size, umm alignment)
{
   void *result = 0;

   umm padding = alignment - (mandala->used & (alignment - 1)) & (alignment - 1);
   umm aligned_used = padding + mandala->used;

   assert(aligned_used + size <= mandala->size);

   result = (void *)((u8 *)mandala->base + aligned_used);
   mandala->used += size + padding;

   return result;
}

local char *
push_string(Mandala *mandala, char *string)
{
   char *result = 0;

   umm length = measure_string(string);
   result = (char *)push_size(mandala, length + 1);
   copy_string(result, string);

   return result;
}

local Mandala
sub_mandala(Mandala *mandala, umm size)
{
   Mandala result = {0};

   result.size = size;
   result.base = push_size(mandala, size);

   return result;
}

#define queue_init(mandala, type, count) queue_init_((mandala), sizeof(type), (count))
#define queue_push(queue) queue_push_((queue))
#define queue_pop(queue)  queue_pop_((queue))
#define queue_peek(queue)  queue_peek_((queue))

local Queue
queue_init_(Mandala *mandala, umm type_size, umm count)
{
   Queue result = {0};

   result.count = 0;
   result.capacity = count;
   result.type_size = type_size;
   result.base = push_size(mandala, count*type_size);

   return result;
}

local void *
queue_push_(Queue *queue)
{
   void *result = 0;

   result = (u8 *)queue->base + queue->write;
   queue->write += queue->type_size;
   if(queue->write == queue->type_size*queue->capacity)
   {
      queue->write = 0;
   }
   ++queue->count;

   return result;
}

local void *
queue_pop_(Queue *queue)
{
   void *result = 0;

   if(queue->read == queue->type_size*queue->capacity)
   {
      queue->read = 0;
   }

   result = (u8 *)queue->base + queue->read;
   queue->read += queue->type_size;
   --queue->count;

   return result;
}

local void *
queue_peek_(Queue *queue)
{
   void *result = 0;

   result = (u8 *)queue->base + queue->read;

   return result;
}

#define stack_init(mandala, type, count) stack_init_((mandala), sizeof(type), count)

local Stack
stack_init_(Mandala *mandala, umm type_size, umm count)
{
   Stack result = {0};

   result.base = push_size(mandala, type_size*count);
   result.capacity = count;
   result.type_size = type_size;
   
   return result;
}

local void *
stack_push(Stack *stack)
{
   void *result = 0;

   assert(stack->count + 1 <= stack->capacity);

   result = (u8 *)stack->base + stack->type_size*stack->count;
   ++stack->count;

   return result;
}

local void *
stack_pop(Stack *stack)
{
   void *result = 0;

   assert(stack->count > 0);

   --stack->count;
   result = (u8 *)stack->base + stack->type_size*stack->count;

   return result;
}

local void *
stack_top(Stack *stack)
{
   void *result = 0;

   assert(stack->count > 0);

   result = (u8 *)stack->base + stack->type_size*(stack->count - 1);

   return result;
}

local void
stack_void(Stack *stack)
{
   stack->count = 0;
}
