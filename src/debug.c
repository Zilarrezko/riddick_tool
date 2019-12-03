
global LARGE_INTEGER perf_freq;

// Perf Frequency
// (1 seconds)/(x cycles)

#define TIMING_START(_label)\
u64 _start##_label;\
{\
_start##_label = get_cycle_stamp();\
}

#define TIMING_END(_label, _out)\
{\
u64 _end##_label = get_cycle_stamp();\
double _label##_timing = get_timing(_start##_label, _end##_label);\
if(_out)\
{\
   *_out = _label##_timing;\
}\
}

local double
get_timing(u64 start, u64 end)
{
   double result = 0.0;

   result = (double)(end - start)/(double)perf_freq.QuadPart;

   return result;
}

local void
d_clear_log()
{
   write_entire_file("log.txt", 0, 0);
}

local void
d_log(char *text)
{
   if(text && measure_string(text) > 0)
   {
      write_append_file("log.txt", text, measure_string(text));
   }
}
