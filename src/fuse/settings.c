#include <externs.h>

// Rename settings_init so we can extend it

#define settings_init fuse_settings_init
#include <fuse/settings.c>
#undef settings_init

// Replacement function for settings_init so we can change the settings before
// the emulation starts without having to build an extensive argv.
int settings_init(int *first_arg, int argc, char **argv)
{
   int res = fuse_settings_init(first_arg, argc, argv);
  
   settings_current.auto_load = 1;
   settings_current.detect_loader = 1;
   
   settings_current.printer = 0;
   
   settings_current.bw_tv = 0;

   settings_current.sound = 1;
   settings_current.sound_force_8bit = 0;
   settings_current.sound_freq = 44100;
   settings_current.sound_load = 1;
   
   settings_current.joy_kempston = 1;
   settings_current.fuller = 1;
   settings_current.joystick_1_output = 1;
   settings_current.joystick_2_output = 2;
   
   update_variables();
   return res;
}
