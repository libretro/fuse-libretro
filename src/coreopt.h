#ifndef COREOPT_H
#define COREOPT_H

#include <libretro.h>

/* Returns the 0-based index of the user-selected option for the given
 * retro_variable. Returns -1 if the option wasn't found in the list or if
 * opt_name is not a valid option.
 */
int coreopt( retro_environment_t env_cb, const struct retro_variable* vars, const char* opt_name );

#endif /* COREOPT_H */
