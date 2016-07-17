// Compatibility timer functions

#include <libretro.h>
#include <externs.h>

double compat_timer_get_time( void )
{
   return total_time_ms / 1000.0;
}

void compat_timer_sleep(int ms)
{
  (void)ms;
}
