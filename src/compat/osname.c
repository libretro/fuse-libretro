// Compatibility osname function

#include <stddef.h>

int compat_osname(char *buffer, size_t length)
{
   strncpy(buffer, "libretro", length);
   buffer[length - 1] = 0;
   return 0;
}
