// Compatibility timer functions

#include <libretro.h>
#include <externs.h>

double compat_timer_get_time( void )
{
   return total_time_ms / 1000.0;
}

void compat_timer_sleep(int ms)
{
   // retro_time_t t0 = perf_cb.get_time_usec();
   // retro_time_t t1;
  
   // /* Busy wait! */
   // do {
     // t1 = perf_cb.get_time_usec();
   // }
   // while ( ( t1 - t0 ) / 1000 < ms );
}
