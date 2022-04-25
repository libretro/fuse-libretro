#include <libretro.h>
#include <keyboverlay.h>

#include <coreopt.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

// Fuse includes
#include <libspectrum.h>
#include <externs.h>
#include <utils.h>
#include <spectrum.h>
#include <keyboard.h>
#include <machines/specplus3.h>
#include <peripherals/disk/beta.h>
#include <peripherals/disk/plusd.h>
#include <peripherals/if1.h>
#include <peripherals/disk/opus.h>
#include <peripherals/disk/disciple.h>
#include <pokefinder/pokemem.h>

#include "ui/uimedia.h"

static void dummy_log(enum retro_log_level level, const char *fmt, ...)
{
   (void)level;
   (void)fmt;
}

#define UPDATE_AV_INFO  1
#define UPDATE_GEOMETRY 2
#define UPDATE_MACHINE  4
#define SPECTRUMKEYS "<none>|0|1|2|3|4|5|6|7|8|9|a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|Enter|Caps|Symbol|Space"

typedef struct cheat_t cheat_t;

struct cheat_t
{
   cheat_t* next;
   poke_t poke;
};

static const int spectrum_keys_map[] = { INPUT_KEY_NONE, INPUT_KEY_0, INPUT_KEY_1, INPUT_KEY_2, INPUT_KEY_3, INPUT_KEY_4, INPUT_KEY_5, INPUT_KEY_6, INPUT_KEY_7, INPUT_KEY_8, INPUT_KEY_9,
      INPUT_KEY_a, INPUT_KEY_b, INPUT_KEY_c, INPUT_KEY_d, INPUT_KEY_e, INPUT_KEY_f, INPUT_KEY_g, INPUT_KEY_h, INPUT_KEY_i, INPUT_KEY_j,
      INPUT_KEY_k, INPUT_KEY_l, INPUT_KEY_m, INPUT_KEY_n, INPUT_KEY_o, INPUT_KEY_p, INPUT_KEY_q, INPUT_KEY_r, INPUT_KEY_s, INPUT_KEY_t,
      INPUT_KEY_u, INPUT_KEY_v, INPUT_KEY_w, INPUT_KEY_x, INPUT_KEY_y, INPUT_KEY_z,
      INPUT_KEY_Return, INPUT_KEY_Shift_L, INPUT_KEY_Control_R, INPUT_KEY_space, };

typedef struct
{
   libspectrum_machine id;
   const char*         fuse_id;
   char                is_timex;
}
machine_t;

static const machine_t machine_list[] =
{
   { LIBSPECTRUM_MACHINE_48,       "48",           0 },
   { LIBSPECTRUM_MACHINE_48_NTSC,  "48_ntsc",      0 },
   { LIBSPECTRUM_MACHINE_128,      "128",          0 },
   { LIBSPECTRUM_MACHINE_PLUS2,    "plus2",        0 },
   { LIBSPECTRUM_MACHINE_PLUS2A,   "plus2a",       0 },
   { LIBSPECTRUM_MACHINE_PLUS3,    "plus3",        0 },
   { LIBSPECTRUM_MACHINE_PLUS3E,   "plus3e",       0 },
   { LIBSPECTRUM_MACHINE_SE,       "se",           1 },
   { LIBSPECTRUM_MACHINE_TC2048,   "2048",         1 },
   { LIBSPECTRUM_MACHINE_TC2068,   "2068",         1 },
   { LIBSPECTRUM_MACHINE_TS2068,   "ts2068",       1 },
   { LIBSPECTRUM_MACHINE_16,       "16",           0 },
   /* these need additional roms in the system/fuse folder */
   { LIBSPECTRUM_MACHINE_PENT,     "pentagon",     0 },
   { LIBSPECTRUM_MACHINE_PENT512,  "pentagon512",  0 },
   { LIBSPECTRUM_MACHINE_PENT1024, "pentagon1024", 0 },
   { LIBSPECTRUM_MACHINE_SCORP,    "scorpion",     0 },
};

static int fuse_init_called = 0;

static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;

static uint16_t image_buffer_2[MAX_WIDTH * MAX_HEIGHT];
static unsigned first_pixel;
static unsigned soft_width, soft_height;
static int size_border;
static int keyb_transparent;
static const machine_t* machine;
static double frame_time;
static cheat_t* active_cheats;

// allow access to variables declared here
double total_time_ms;
retro_environment_t env_cb;
retro_log_printf_t log_cb = dummy_log;
retro_audio_sample_batch_t audio_cb;
retro_input_state_t input_state_cb;
uint16_t image_buffer[MAX_WIDTH * MAX_HEIGHT];
unsigned hard_width, hard_height;
int show_frame, some_audio;
unsigned input_devices[MAX_PADS];
int64_t keyb_send;
int64_t keyb_hold_time;
input_event_t keyb_event;
int8_t keyb_shift;
int select_pressed;
int keyb_overlay;
unsigned keyb_x;
unsigned keyb_y;
bool joypad_state[MAX_PADS][16];
bool keyb_state[RETROK_LAST];
void*  snapshot_buffer;
size_t snapshot_size;
void* tape_data;
size_t tape_size;
int joymap[16];

static const struct { unsigned x; unsigned y; } keyb_positions[4] = {
   { 32, 40 }, { 40, 88 }, { 48, 136 }, { 32, 184 }
};

static struct retro_input_descriptor input_descriptors[] = {
   // See fuse-1.1.1\peripherals\joystick.c
   // Cursor
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "Up" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "Fire" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Fire" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "Enter" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "Space" },
   // Kempston
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "Up" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "Fire" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Fire" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "Enter" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "Space" },
   // Sinclair 1
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "Up" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "Fire" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Fire" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "Enter" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "Space" },
   // Sinclair 2
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "Up" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "Fire" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Fire" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "Enter" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "Space" },
   // Timex 1
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "Up" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "Fire" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Fire" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "Enter" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "Space" },
   // Timex 2
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "Up" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "Fire" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Fire" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "Enter" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "Space" },
   // Fuller
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "Up" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "Fire" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Fire" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "Enter" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "Space" },
   // Terminate
   { 255, 255, 255, 255, NULL }
};

// Must implement the keyboard
keysyms_map_t keysyms_map[] = {
   { RETROK_RETURN,    INPUT_KEY_Return      },
   { RETROK_SPACE,     INPUT_KEY_space       },
   { RETROK_BACKSPACE, INPUT_KEY_BackSpace   },
   { RETROK_0,         INPUT_KEY_0           },
   { RETROK_1,         INPUT_KEY_1           },
   { RETROK_2,         INPUT_KEY_2           },
   { RETROK_3,         INPUT_KEY_3           },
   { RETROK_4,         INPUT_KEY_4           },
   { RETROK_5,         INPUT_KEY_5           },
   { RETROK_6,         INPUT_KEY_6           },
   { RETROK_7,         INPUT_KEY_7           },
   { RETROK_8,         INPUT_KEY_8           },
   { RETROK_9,         INPUT_KEY_9           },
   { RETROK_a,         INPUT_KEY_a           },
   { RETROK_b,         INPUT_KEY_b           },
   { RETROK_c,         INPUT_KEY_c           },
   { RETROK_d,         INPUT_KEY_d           },
   { RETROK_e,         INPUT_KEY_e           },
   { RETROK_f,         INPUT_KEY_f           },
   { RETROK_g,         INPUT_KEY_g           },
   { RETROK_h,         INPUT_KEY_h           },
   { RETROK_i,         INPUT_KEY_i           },
   { RETROK_j,         INPUT_KEY_j           },
   { RETROK_k,         INPUT_KEY_k           },
   { RETROK_l,         INPUT_KEY_l           },
   { RETROK_m,         INPUT_KEY_m           },
   { RETROK_n,         INPUT_KEY_n           },
   { RETROK_o,         INPUT_KEY_o           },
   { RETROK_p,         INPUT_KEY_p           },
   { RETROK_q,         INPUT_KEY_q           },
   { RETROK_r,         INPUT_KEY_r           },
   { RETROK_s,         INPUT_KEY_s           },
   { RETROK_t,         INPUT_KEY_t           },
   { RETROK_u,         INPUT_KEY_u           },
   { RETROK_v,         INPUT_KEY_v           },
   { RETROK_w,         INPUT_KEY_w           },
   { RETROK_x,         INPUT_KEY_x           },
   { RETROK_y,         INPUT_KEY_y           },
   { RETROK_z,         INPUT_KEY_z           },
   { RETROK_LSHIFT,    INPUT_KEY_Shift_L     },
   { RETROK_RSHIFT,    INPUT_KEY_Shift_R     },
   { RETROK_LCTRL,     INPUT_KEY_Control_L   },
   { RETROK_RCTRL,     INPUT_KEY_Control_R   },
   { RETROK_LALT,      INPUT_KEY_Alt_L       },
   { RETROK_RALT,      INPUT_KEY_Alt_R       },
   { RETROK_LMETA,     INPUT_KEY_Meta_L      },
   { RETROK_RMETA,     INPUT_KEY_Meta_R      },
   { RETROK_LSUPER,    INPUT_KEY_Super_L     },
   { RETROK_RSUPER,    INPUT_KEY_Super_R     },
   { 0, 0 }    // End marker: DO NOT MOVE!
};

static const struct retro_variable core_vars[] =
{
   { "fuse_machine", "Model (needs content load); Spectrum 48K|Spectrum 48K (NTSC)|Spectrum 128K|Spectrum +2|Spectrum +2A|Spectrum +3|Spectrum +3e|Spectrum SE|Timex TC2048|Timex TC2068|Timex TS2068|Spectrum 16K|Pentagon 128K|Pentagon 512K|Pentagon 1024|Scorpion 256K" },
   { "fuse_size_border", "Size Video Border; full|medium|small|minimum|none" },
   { "fuse_auto_load", "Tape Auto Load; enabled|disabled" },
   { "fuse_fast_load", "Tape Fast Load; enabled|disabled" },
   { "fuse_load_sound", "Tape Load Sound; enabled|disabled" },
   { "fuse_speaker_type", "Speaker Type; tv speaker|beeper|unfiltered" },
   { "fuse_ay_stereo_separation", "AY Stereo Separation; none|acb|abc" },
   { "fuse_key_ovrlay_transp", "Transparent Keyboard Overlay; enabled|disabled" },
   { "fuse_key_hold_time", "Time to Release Key in ms; 500|1000|100|300" },
   { "fuse_joypad_left",    "Joypad Left mapping; " SPECTRUMKEYS },
   { "fuse_joypad_right",   "Joypad Right mapping; " SPECTRUMKEYS },
   { "fuse_joypad_up",      "Joypad Up mapping; " SPECTRUMKEYS },
   { "fuse_joypad_down",    "Joypad Down mapping; " SPECTRUMKEYS },
   { "fuse_joypad_start",   "Joypad Start mapping; " SPECTRUMKEYS },
   { "fuse_joypad_a",       "Joypad A button mapping; " SPECTRUMKEYS },
   { "fuse_joypad_b",       "Joypad B button mapping; " SPECTRUMKEYS },
   { "fuse_joypad_x",       "Joypad X button mapping; " SPECTRUMKEYS },
   { "fuse_joypad_y",       "Joypad Y button mapping; " SPECTRUMKEYS },
   { "fuse_joypad_l",       "Joypad L button mapping; " SPECTRUMKEYS },
   { "fuse_joypad_r",       "Joypad R button mapping; " SPECTRUMKEYS },
   { "fuse_joypad_l2",      "Joypad L2 button mapping; " SPECTRUMKEYS },
   { "fuse_joypad_r2",      "Joypad R2 button mapping; " SPECTRUMKEYS },
   { "fuse_joypad_l3",      "Joypad L3 button mapping; " SPECTRUMKEYS },
   { "fuse_joypad_r3",      "Joypad R3 button mapping; " SPECTRUMKEYS },
   { NULL, NULL },
};

int update_variables(int force)
{
   int flags = 0;

   if (force)
   {
      // Only change the machine when reloading content
      int option = coreopt(env_cb, core_vars, "fuse_machine", NULL);
      option += option < 0;
      const machine_t *new_machine = machine_list + option;

      if (new_machine != machine || force)
      {
         if (settings_current.start_machine)
         {
            libspectrum_free((void*)settings_current.start_machine);
         }

         settings_current.start_machine = utils_safe_strdup(new_machine->fuse_id);

         if (machine == NULL || new_machine->id == LIBSPECTRUM_MACHINE_48_NTSC || machine->id == LIBSPECTRUM_MACHINE_48_NTSC)
         {
            // region and fps change
            flags |= UPDATE_AV_INFO;
         }

         machine = new_machine;
         frame_time = 1000.0 / ( machine->id == LIBSPECTRUM_MACHINE_48_NTSC ? 60.0 : 50.0 );
         flags |= UPDATE_MACHINE;
      }

      unsigned width = machine->is_timex ? 640 : 320;
      unsigned height = machine->is_timex ? 480 : 240;

      if (width != hard_width || height != hard_height || force)
      {
         hard_width = width;
         hard_height = height;

         size_border = coreopt(env_cb, core_vars, "fuse_size_border", NULL);
         size_border += size_border < 0;

         if (size_border == 1)
         {
            soft_width = machine->is_timex ? 576 : 288;
            soft_height = machine->is_timex ? 432 : 216;
         }
         else if (size_border == 2)
         {
            soft_width = machine->is_timex ? 544 : 272;
            soft_height = machine->is_timex ? 408 : 204;
         }
         else if (size_border == 3)
         {
            soft_width = machine->is_timex ? 528 : 264;
            soft_height = machine->is_timex ? 396 : 198;
         }
         else if (size_border == 4)
         {
            soft_width = machine->is_timex ? 512 : 256;
            soft_height = machine->is_timex ? 384 : 192;
         }
         else
         {
            soft_width = hard_width;
            soft_height = hard_height;
         }

         first_pixel = (hard_height - soft_height) / 2 * hard_width + (hard_width - soft_width) / 2;
         flags |= UPDATE_AV_INFO | UPDATE_GEOMETRY;
      }
   }
   else
   {
      // When reloading content, this is already done as part of the machine change
      int option = coreopt(env_cb, core_vars, "fuse_size_border", NULL);
      option += option < 0;

      if (option != size_border || force)
      {
         size_border = option;

         if (size_border == 1)
         {
            soft_width = machine->is_timex ? 576 : 288;
            soft_height = machine->is_timex ? 432 : 216;
         }
         else if (size_border == 2)
         {
            soft_width = machine->is_timex ? 544 : 272;
            soft_height = machine->is_timex ? 408 : 204;
         }
         else if (size_border == 3)
         {
            soft_width = machine->is_timex ? 528 : 264;
            soft_height = machine->is_timex ? 396 : 198;
         }
         else if (size_border == 4)
         {
            soft_width = machine->is_timex ? 512 : 256;
            soft_height = machine->is_timex ? 384 : 192;
         }
         else
         {
            soft_width = hard_width;
            soft_height = hard_height;
         }

         first_pixel = (hard_height - soft_height) / 2 * hard_width + (hard_width - soft_width) / 2;
         flags |= UPDATE_GEOMETRY;
      }
   }

   settings_current.auto_load = coreopt(env_cb, core_vars, "fuse_auto_load", NULL) != 1;

   if (coreopt(env_cb, core_vars, "fuse_fast_load", NULL) == 0)
   {
      // Fastload enabled
      settings_current.fastload = 1;
      settings_current.accelerate_loader = 1;
      settings_current.tape_traps = 1;
      settings_current.slt_traps = 1;
   } else {
      // Fastload disabled
      settings_current.fastload = 0;
      settings_current.accelerate_loader = 0;
      settings_current.tape_traps = 0;
      settings_current.slt_traps = 0;
   }

   settings_current.sound_load = coreopt(env_cb, core_vars, "fuse_load_sound", NULL) != 1;

   {
      int option = coreopt(env_cb, core_vars, "fuse_speaker_type", NULL);

      if (settings_current.speaker_type)
      {
         libspectrum_free((void*)settings_current.speaker_type);
      }

      settings_current.speaker_type = utils_safe_strdup(option == 1 ? "Beeper" : option == 2 ? "Unfiltered" : "TV speaker");
   }

   {
      int option = coreopt(env_cb, core_vars, "fuse_ay_stereo_separation", NULL);

      if (settings_current.stereo_ay)
      {
         libspectrum_free((void*)settings_current.stereo_ay);
      }

      settings_current.stereo_ay = utils_safe_strdup(option == 1 ? "ACB" : option == 2 ? "ABC" : "None");
   }

   keyb_transparent = coreopt(env_cb, core_vars, "fuse_key_ovrlay_transp", NULL) != 1;

   {
      const char* value;
      int option = coreopt(env_cb, core_vars, "fuse_key_hold_time", &value);
      keyb_hold_time = option >= 0 ? strtoll(value, NULL, 10) * 1000LL : 500000LL;
   }

   const char* value;
   int option = coreopt(env_cb, core_vars, "fuse_joypad_up", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_UP ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_down", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_DOWN ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_left", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_LEFT ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_right", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_RIGHT ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_a", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_A ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_b", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_B ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_x", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_X ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_y", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_Y ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_l", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_L ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_r", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_R ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_l2", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_L2 ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_r2", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_R2 ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_l3", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_L3 ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_r3", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_R3 ] = spectrum_keys_map[option];

   option = coreopt(env_cb, core_vars, "fuse_joypad_start", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_START ] = spectrum_keys_map[option];

   return flags;
}

static int get_joystick(unsigned device)
{
   switch (device)
   {
      case RETRO_DEVICE_CURSOR_JOYSTICK:    return 1;
      case RETRO_DEVICE_KEMPSTON_JOYSTICK:  return 2;
      case RETRO_DEVICE_SINCLAIR1_JOYSTICK: return 3;
      case RETRO_DEVICE_SINCLAIR2_JOYSTICK: return 4;
      case RETRO_DEVICE_TIMEX1_JOYSTICK:    return 5;
      case RETRO_DEVICE_TIMEX2_JOYSTICK:    return 6;
      case RETRO_DEVICE_FULLER_JOYSTICK:    return 7;
   }

   return 0;
}

#ifdef GIT_VERSION
static char version[] = PACKAGE_VERSION " " GIT_VERSION;
#else
extern const char *fuse_githash;
static char version[] = PACKAGE_VERSION " .......";
#endif

void retro_get_system_info(struct retro_system_info *info)
{
#ifndef GIT_VERSION
   memcpy(version + sizeof(PACKAGE_VERSION), fuse_githash, 7);
#endif
   info->library_name = "Fuse"; // must be the same as in fuse_libretro.info
   info->library_version = version;
   info->need_fullpath = false;
   info->block_extract = false;
   info->valid_extensions = "tzx|tap|z80|rzx|scl|trd|dsk|zip";
}

void retro_set_environment(retro_environment_t cb)
{
   env_cb = cb;

   static const struct retro_controller_description controllers[] = {
      { "Cursor Joystick", RETRO_DEVICE_CURSOR_JOYSTICK },
      { "Kempston Joystick", RETRO_DEVICE_KEMPSTON_JOYSTICK },
      { "Sinclair 1 Joystick", RETRO_DEVICE_SINCLAIR1_JOYSTICK },
      { "Sinclair 2 Joystick", RETRO_DEVICE_SINCLAIR2_JOYSTICK },
      { "Timex 1 Joystick", RETRO_DEVICE_TIMEX1_JOYSTICK },
      { "Timex 2 Joystick", RETRO_DEVICE_TIMEX2_JOYSTICK },
      { "Fuller Joystick", RETRO_DEVICE_FULLER_JOYSTICK }
   };

   static const struct retro_controller_description keyboards[] = {
      { "Sinclair Keyboard", RETRO_DEVICE_SPECTRUM_KEYBOARD },
   };

   static const struct retro_controller_info ports[MAX_PADS + 1] = {
      { controllers, sizeof(controllers) / sizeof(controllers[0]) }, // port 1
      { controllers, sizeof(controllers) / sizeof(controllers[0]) }, // port 2
      { keyboards,   sizeof(keyboards)   / sizeof(keyboards[0])   }, // port 3
      { NULL, 0 }
   };

   // This seem to be broken right now, as info is NULL in retro_load_game
   // even when we load a game via the frontend after booting to BASIC.
   //bool yes = true;
   //cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, (void*)&yes);

   cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)core_vars);
   cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_init(void)
{
   struct retro_log_callback log;

   if (env_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
   {
      log_cb = log.log;
   }

   machine = machine_list;
   total_time_ms = 0.0;
   active_cheats = NULL;

   // Set default controllers
   retro_set_controller_port_device( 0, RETRO_DEVICE_CURSOR_JOYSTICK );
   retro_set_controller_port_device( 1, RETRO_DEVICE_KEMPSTON_JOYSTICK );
   retro_set_controller_port_device( 2, RETRO_DEVICE_SPECTRUM_KEYBOARD );
}

static libspectrum_id_t identify_file(const void* data, size_t size)
{
   libspectrum_id_t type;
   libspectrum_error error = libspectrum_identify_file(&type, NULL, (const unsigned char*)data, size);

   if (type != LIBSPECTRUM_ID_UNKNOWN)
   {
      return type;
   }

   libspectrum_snap* snap = libspectrum_snap_alloc();
   error = libspectrum_snap_read(snap, (const libspectrum_byte*)data, size, LIBSPECTRUM_ID_SNAPSHOT_Z80, NULL);
   libspectrum_snap_free(snap);

   if (error == LIBSPECTRUM_ERROR_NONE)
   {
      return LIBSPECTRUM_ID_SNAPSHOT_Z80;
   }

   // Default to TRD, we won't be able to load TRD files otherwise
   return LIBSPECTRUM_ID_DISK_TRD;
}

static libspectrum_id_t identify_file_get_ext(const void* data, size_t size, const char** ext)
{
   libspectrum_id_t type = identify_file(data, size);

   switch (type)
   {
      case LIBSPECTRUM_ID_RECORDING_RZX: *ext = ".rzx"; break;
      case LIBSPECTRUM_ID_SNAPSHOT_SNA:  *ext = ".sna"; break;
      case LIBSPECTRUM_ID_SNAPSHOT_Z80:  *ext = ".z80"; break;
      case LIBSPECTRUM_ID_TAPE_TAP:      // has same extension as LIBSPECTRUM_ID_TAPE_WARAJEVO
      case LIBSPECTRUM_ID_TAPE_WARAJEVO: *ext = ".tap"; break;
      case LIBSPECTRUM_ID_TAPE_TZX:      *ext = ".tzx"; break;
      case LIBSPECTRUM_ID_SNAPSHOT_SP:   *ext = ".sp";  break;
      case LIBSPECTRUM_ID_SNAPSHOT_SNP:  *ext = ".snp"; break;
      case LIBSPECTRUM_ID_SNAPSHOT_ZXS:  *ext = ".zxs"; break;
      case LIBSPECTRUM_ID_SNAPSHOT_SZX:  *ext = ".szx"; break;
      case LIBSPECTRUM_ID_TAPE_CSW:      *ext = ".csw"; break;
      case LIBSPECTRUM_ID_TAPE_Z80EM:    *ext = ".raw"; break;
      case LIBSPECTRUM_ID_TAPE_WAV:      *ext = ".wav"; break;
      case LIBSPECTRUM_ID_TAPE_SPC:      *ext = ".spc"; break;
      case LIBSPECTRUM_ID_TAPE_STA:      *ext = ".sta"; break;
      case LIBSPECTRUM_ID_TAPE_LTP:      *ext = ".ltp"; break;
      case LIBSPECTRUM_ID_TAPE_PZX:      *ext = ".pzx"; break;
      case LIBSPECTRUM_ID_DISK_SCL:      *ext = ".scl"; break;
      case LIBSPECTRUM_ID_DISK_TRD:      *ext = ".trd"; break;
      case LIBSPECTRUM_ID_DISK_DSK:
      case LIBSPECTRUM_ID_DISK_CPC:
      case LIBSPECTRUM_ID_DISK_ECPC:     *ext = ".dsk"; break;
      default:                           *ext = "";     break;
   }

   return type;
}

#ifndef GIT_VERSION
extern const char* fuse_gitstamp;
#endif

bool retro_load_game(const struct retro_game_info *info)
{
#ifndef GIT_VERSION
   log_cb( RETRO_LOG_INFO, "\n%s\n", fuse_gitstamp );
#endif

   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;

   if (!env_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      log_cb(RETRO_LOG_ERROR, "RGB565 is not supported\n");
      return false;
   }

   env_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, input_descriptors);
   memset(joypad_state, 0, sizeof(joypad_state));
   memset(keyb_state, 0, sizeof(keyb_state));
   hard_width = hard_height = soft_width = soft_height = 0;
   select_pressed = keyb_overlay = 0;
   keyb_x = keyb_y = 0;
   keyb_send = 0;
   snapshot_buffer = NULL;
   snapshot_size = 0;

   char *argv[] = {
      "fuse",
   };

   fuse_init_called = 1;

   if (fuse_init(sizeof(argv) / sizeof(argv[0]), argv) == 0)
   {
      if (info->size != 0)
      {
         tape_size = info->size;
         tape_data = malloc(tape_size);

         if (!tape_data)
         {
            log_cb(RETRO_LOG_ERROR, "Could not allocate memory for the tape\n");
            fuse_end();
            return false;
         }

         memcpy(tape_data, info->data, tape_size);

         const char* ext;
         libspectrum_id_t type = identify_file_get_ext(tape_data, tape_size, &ext);
         libspectrum_class_t class;
         libspectrum_identify_class(&class, type);

         char filename[32];
         snprintf(filename, sizeof(filename), "*%s", ext);
         filename[sizeof(filename) - 1] = 0;

         /*
         ** Deal with a number of special cases to make experience smoother
         */

         // autoload is on by default
         int autoload = settings_current.auto_load;

         // Disable autoload for tapes on Scorpion 256 (it doesn't work)
         if (!strcmp(settings_current.start_machine, machine_get_id(LIBSPECTRUM_MACHINE_SCORP)) &&
             class == LIBSPECTRUM_CLASS_TAPE)
         {
            autoload = 0;
         }

         // If we have a .dsk image, check if it has more than 40 tracks (e.g. a 720KB disk image)
         // .dsk file format: http://cpctech.cpc-live.com/docs/dsk.html
         if (class == LIBSPECTRUM_CLASS_DISK_PLUS3 && ((uint8_t *)tape_data)[0x30] > 40)
         {  
            // If yes, we need to change the drive type on the fly, as the default +3 drive only supports 40 tracks
            settings_current.drive_plus3a_type = utils_safe_strdup("Double-sided 80 track");
            specplus3_765_reset();
         }

         /*
         ** Load the file and launch the emulation
         */

         fuse_emulation_pause();
         utils_open_file(filename, autoload, &type);
         display_refresh_all();
         fuse_emulation_unpause();
      }
      else
      {
         // Load the _BASIC.z80 content to boot to BASIC
         tape_data = NULL;
         tape_size = 0;
      }

      // Enable read/write on all disk drives
      int i;

      for (i = 0; i < 2; i++)
      {
         ui_media_drive_writeprotect(UI_MEDIA_CONTROLLER_PLUS3, i, 0);
         ui_media_drive_writeprotect(UI_MEDIA_CONTROLLER_PLUSD, i, 0);
         ui_media_drive_writeprotect(UI_MEDIA_CONTROLLER_OPUS, i, 0);
         ui_media_drive_writeprotect(UI_MEDIA_CONTROLLER_DISCIPLE, i, 0);
      }

      for (i = 0; i < 4; i++)
      {
         ui_media_drive_writeprotect(UI_MEDIA_CONTROLLER_BETA, i, 0);
      }

      for (i = 0; i < 8; i++)
      {
         if1_mdr_writeprotect( i, 0 );
      }

      // Set up memory map interface
      struct retro_memory_descriptor desc[MEMORY_PAGES_IN_64K];
      memset(desc, 0, sizeof(desc));

      for (i = 0; i < MEMORY_PAGES_IN_64K; i++)
      {
         desc[i].start  = i * MEMORY_PAGE_SIZE;
         desc[i].len    = MEMORY_PAGE_SIZE;
         desc[i].select = 0;
         desc[i].ptr    = memory_map_read[i].page;
      }
      struct retro_memory_map memory_map = {desc, MEMORY_PAGES_IN_64K};

      env_cb(RETRO_ENVIRONMENT_SET_MEMORY_MAPS, &memory_map);

      return true;
   }

   return false;
}

size_t retro_get_memory_size(unsigned id)
{
   return 0;
}

void *retro_get_memory_data(unsigned id)
{
   return NULL;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   (void)cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   // Here we always use the "hard" resolution to accomodate output with *and*
   // without the video border
   info->geometry.base_width = hard_width;
   info->geometry.base_height = hard_height;

   info->geometry.max_width = MAX_WIDTH;
   info->geometry.max_height = MAX_HEIGHT;
   info->geometry.aspect_ratio = 0.0f;
   info->timing.fps = machine->id == LIBSPECTRUM_MACHINE_48_NTSC ? 60.0 : 50.0;
   info->timing.sample_rate = 44100.0;
}

static void render_video(void)
{
   if (!keyb_overlay)
   {
      video_cb(show_frame ? image_buffer + first_pixel : NULL, soft_width, soft_height, hard_width * sizeof(uint16_t));
   }
   else
   {
      if (show_frame)
      {
         if (machine->is_timex)
         {
            const uint16_t* src1 = keyboard_overlay;
            const uint16_t* src2 = image_buffer;
            uint16_t* dest = image_buffer_2;
            int x, y;

            if (keyb_transparent)
            {
               for (y = 0; y < 240; y++)
               {
                  for (x = 0; x < 320; x++)
                  {
                     uint32_t src1_pixel = (*src1++ & 0xe79c) * 3;

                     dest[ 0 ] = (src1_pixel + ( src2[ 0 ] & 0xe79c)) >> 2;
                     dest[ 1 ] = (src1_pixel + ( src2[ 1 ] & 0xe79c)) >> 2;
                     dest[ 640 ] = (src1_pixel + ( src2[ 640 ] & 0xe79c)) >> 2;
                     dest[ 641 ] = (src1_pixel + ( src2[ 641 ] & 0xe79c)) >> 2;

                     src2 += 2;
                     dest += 2;
                  }

                  src2 += 640;
                  dest += 640;
               }
            }
            else
            {
               for (y = 0; y < 240; y++)
               {
                  for (x = 0; x < 320; x++)
                  {
                     uint32_t src1_pixel = *src1++;

                     dest[ 0 ] = src1_pixel;
                     dest[ 1 ] = src1_pixel;
                     dest[ 640 ] = src1_pixel;
                     dest[ 641 ] = src1_pixel;

                     src2 += 2;
                     dest += 2;
                  }

                  src2 += 640;
                  dest += 640;
               }
             }
         }
         else
         {
            if (keyb_transparent)
            {
               const uint16_t* src1 = keyboard_overlay;
               const uint16_t* src2 = image_buffer;
               const uint16_t* end = src1 + sizeof(keyboard_overlay) / sizeof(keyboard_overlay[0]);
               uint16_t* dest = image_buffer_2;

               do
               {
                  uint32_t src1_pixel = *src1++ & 0xe79c;
                  uint32_t src2_pixel = *src2++ & 0xe79c;

                  *dest++ = (src1_pixel * 3 + src2_pixel) >> 2;
               }
               while (src1 < end);
            }
            else
            {
               memcpy(image_buffer_2, keyboard_overlay, sizeof(keyboard_overlay));
            }
         }

         unsigned x = keyb_positions[keyb_y].x + keyb_x * 24;
         unsigned y = keyb_positions[keyb_y].y;
         unsigned width = 23;

         if (keyb_y == 3)
         {
            if (keyb_x == 8)
            {
               width = 24;
            }
            else if (keyb_x == 9)
            {
               x++;
               width = 30;
            }
         }

         unsigned mult = machine->is_timex ? 2 : 1;
         uint16_t* pixel = image_buffer_2 + (y * hard_width + x + 1) * mult;
         unsigned i, j;

         for (j = mult; j > 0; --j )
         {
            for (i = (width - 2) * mult; i > 0; --i)
            {
               *pixel = ~*pixel;
               pixel++;
            }

            pixel += hard_width - (width - 2) * mult;
         }

         pixel -= mult;

         for (j = 22 * mult; j > 0; --j)
         {
            for (i = width * mult; i > 0; --i)
            {
               *pixel = ~*pixel;
               pixel++;
            }

            pixel += hard_width - width * mult;
         }

         pixel += mult;

         for (j = mult; j > 0; --j)
         {
            for (i = (width - 2) * mult; i > 0; --i)
            {
               *pixel = ~*pixel;
               pixel++;
            }

            pixel += hard_width - (width - 2) * mult;
         }

         video_cb(image_buffer_2 + first_pixel, soft_width, soft_height, hard_width * sizeof(uint16_t));
      }
      else
      {
         video_cb(NULL, soft_width, soft_height, hard_width * sizeof(uint16_t));
      }
   }
}

void retro_run(void)
{
   bool updated = false;

   if (env_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
   {
      int flags = update_variables(0);

      if (flags & UPDATE_AV_INFO)
      {
         struct retro_system_av_info info;
         retro_get_system_av_info(&info);
         env_cb(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &info);
      }

      if (flags & UPDATE_GEOMETRY)
      {
         struct retro_game_geometry geometry;

         // Here we use the "soft" resolution that is changed according to the
         // fuse_size_border variable
         geometry.base_width = soft_width;
         geometry.base_height = soft_height;

         geometry.max_width = MAX_WIDTH;
         geometry.max_height = MAX_HEIGHT;
         geometry.aspect_ratio = 0.0f;

         env_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry);
      }

      if (flags & UPDATE_MACHINE)
      {
         machine_select( machine->id );
      }
   }

   total_time_ms += frame_time;
   show_frame = some_audio = 0;

   /*
   After playing Sabre Wulf's initial title music, fuse starts generating
   audio only for every other frame. RetroArch computes the FPS based on
   this and tries to call retro_run at double the rate to compensate for
   the audio. When vsync is on, FPS will cap at some value and the game
   will run slower.

   This solution guarantees that every call to retro_run generates audio, so
   emulation runs steadly at 50 FPS. Ideally, we should investigate why fuse
   is doing that and fix it, but this solutions seems to work just fine.
   */
   do {
      input_poll_cb();
      z80_do_opcodes();
      event_do_events();
   }
   while (!some_audio);

   render_video();
}

void retro_deinit(void)
{
   cheat_t* cheat = active_cheats;
   cheat_t* next = NULL;

   while (cheat != NULL)
   {
      next = cheat->next;
      free((void*)cheat);
      cheat = next;
   }

   active_cheats = NULL;

   if ( fuse_init_called )
   {
      fuse_init_called = 0;
      fuse_end();
   }
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   log_cb(RETRO_LOG_INFO, "port %u device %08x\n", port, device);
   switch (device)
   {
      case RETRO_DEVICE_CURSOR_JOYSTICK:
      case RETRO_DEVICE_KEMPSTON_JOYSTICK:
      case RETRO_DEVICE_SINCLAIR1_JOYSTICK:
      case RETRO_DEVICE_SINCLAIR2_JOYSTICK:
      case RETRO_DEVICE_TIMEX1_JOYSTICK:
      case RETRO_DEVICE_TIMEX2_JOYSTICK:
      case RETRO_DEVICE_FULLER_JOYSTICK:
         if (port == 0)
         {
            settings_current.joystick_1_output = get_joystick(device);
         }
         else if (port == 1)
         {
            settings_current.joystick_2_output = get_joystick(device);
         }

         /* fallthrough */

      default:
         if (port < MAX_PADS)
         {
            //check is required, without it port >= MAX_PADS will cause a buffer overflow
            input_devices[port] = device;
         }
         break;
   }
}

void retro_reset(void)
{
   const char* ext;
   libspectrum_id_t type = identify_file_get_ext(tape_data, tape_size, &ext);

   char filename[32];
   snprintf(filename, sizeof(filename), "*%s", ext);
   filename[sizeof(filename) - 1] = 0;

   fuse_emulation_pause();
   utils_open_file(filename, 1, &type);
   display_refresh_all();
   fuse_emulation_unpause();
}

size_t retro_serialize_size(void)
{
   fuse_emulation_pause();
   snapshot_write("dummy.szx"); // filename is only used to get the snapshot type
   fuse_emulation_unpause();
   return snapshot_size;
}

bool retro_serialize(void *data, size_t size)
{
   bool res = false;

   if (size <= snapshot_size)
   {
      memcpy(data, snapshot_buffer, snapshot_size);
      res = true;
   }

   return res;
}

bool retro_unserialize(const void *data, size_t size)
{
   return snapshot_read_buffer(data, size, LIBSPECTRUM_ID_SNAPSHOT_SZX) == 0;
}

void retro_cheat_reset(void)
{
   cheat_t* cheat = active_cheats;
   cheat_t* next = NULL;

   while (cheat != NULL)
   {
      if (cheat->poke.bank == 8)
         writebyte_internal(cheat->poke.address, cheat->poke.restore);
      else
         RAM[cheat->poke.bank][cheat->poke.address & 0x3fff] = cheat->poke.restore;
      
      next = cheat->next;
      free((void*)cheat);
      cheat = next;
   }

   active_cheats = NULL;
}

static void skip_spaces(const char** c)
{
   while (isspace(**c))
      (*c)++;
}

static unsigned parse_unsigned(const char** c)
{
   char* end;
   unsigned value = (unsigned)strtol(*c, &end, 10);
   *c = end;
   return value;
}

void retro_cheat_set(unsigned index, bool b, const char* code)
{
   cheat_t* cheat = NULL;
   unsigned bank = 0, address = 0, value = 0, original = 0;
   const char* saved_code = code;

   (void)b;

   do
   {
      if (*code == 'M' || *code == 'Z')
      {
         code++;
         skip_spaces(&code);
         bank = parse_unsigned(&code);
         skip_spaces(&code);
         address = parse_unsigned(&code);
         skip_spaces(&code);
         value = parse_unsigned(&code);
         skip_spaces(&code);
         original = parse_unsigned(&code);

         if (value > 255)
         {
            /* We don't support user-provided values for now */
            continue;
         }

         cheat = (cheat_t*)calloc(1, sizeof(*cheat));

         if (cheat == NULL)
            return;
         
         cheat->next = active_cheats;
         active_cheats = cheat;

         if (bank == 8)
         {
            if (original == 0)
               original = readbyte_internal(address);

            writebyte_internal(address, value);
         }
         else
         {
            if (original == 0)
               original = RAM[ bank ][ address ];
            
            RAM[bank][address & 0x3fff] = value;
         }

         cheat->poke.bank = bank;
         cheat->poke.address = address;
         cheat->poke.value = value;
         cheat->poke.restore = original;

         log_cb(RETRO_LOG_INFO, "Enabled cheat #%u: %s\n", index, saved_code);
      }
      else
         break;

      if (*code != '\\' || code[1] != 'n')
         break;

      code += 2;
   }
   while (1);
}

bool retro_load_game_special(unsigned a, const struct retro_game_info *b, size_t c)
{
   (void)a;
   (void)b;
   (void)c;
   return false;
}

void retro_unload_game(void)
{
   free(snapshot_buffer);
   snapshot_buffer = NULL;
   snapshot_size = 0;
   
   free(tape_data);
}

unsigned retro_get_region(void)
{
   return machine->id == LIBSPECTRUM_MACHINE_48_NTSC ? RETRO_REGION_NTSC : RETRO_REGION_PAL;
}

// Dummy callbacks for the UI

/*
#define MENU_CALLBACK( name ) void name( int action )
MENU_CALLBACK(menu_file_savescreenaspng) {}
MENU_CALLBACK(menu_file_loadbinarydata)  {}
MENU_CALLBACK(menu_file_savebinarydata)  {}
MENU_CALLBACK(menu_machine_pause)        {}
*/

// Dummy mkstemp

int mkstemp(char *template)
{
  return -1;
}
