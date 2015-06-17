// Compatibility timer functions

#include <libretro.h>
#include <externs.h>

double compat_timer_get_time( void )
{
   static int setup = 1;
   static int64_t t0;
   
   if (!setup)
   {
      return ( t0 + perf_cb.get_time_usec() ) / 1000000.0;
   }
   else
   {
      setup = 0;
      t0 = -perf_cb.get_time_usec();
      return 0.0;
   }
}

void compat_timer_sleep(int ms)
{
   retro_time_t t0 = perf_cb.get_time_usec();
   retro_time_t t1;
  
   /* Busy wait! */
   do {
     t1 = perf_cb.get_time_usec();
   }
   while ( ( t1 - t0 ) / 1000 < ms );
}
