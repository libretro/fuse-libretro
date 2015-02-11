#include <libretro.h>
#include <keyboverlay.h>

#include <errno.h>

// Fuse includes
#include <libspectrum.h>
#include <externs.h>
#include <utils.h>
#include <keyboard.h>

static void dummy_log(enum retro_log_level level, const char *fmt, ...)
{
   (void)level;
   (void)fmt;
}

#define RETRO_DEVICE_CURSOR_JOYSTICK RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 0)
#define RETRO_DEVICE_KEMPSTON_JOYSTICK RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 1)
#define RETRO_DEVICE_SINCLAIR1_JOYSTICK RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 2)
#define RETRO_DEVICE_SINCLAIR2_JOYSTICK RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 3)
#define RETRO_DEVICE_TIMEX1_JOYSTICK RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 4)
#define RETRO_DEVICE_TIMEX2_JOYSTICK RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 5)
#define RETRO_DEVICE_FULLER_JOYSTICK RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 6)

static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static struct retro_frame_time_callback time_cb;

static unsigned input_devices[MAX_PADS];
static uint16_t image_buffer_2[MAX_WIDTH * MAX_HEIGHT];
static unsigned first_pixel;
static unsigned soft_width, soft_height;
static int hide_border, change_geometry;
static int keyb_transparent;

// allow access to variables declared here
retro_environment_t env_cb;
retro_log_printf_t log_cb = dummy_log;
retro_audio_sample_batch_t audio_cb;
retro_input_state_t input_state_cb;
struct retro_perf_callback perf_cb;
uint16_t image_buffer[MAX_WIDTH * MAX_HEIGHT];
unsigned hard_width, hard_height;
int show_frame, some_audio;
int64_t keyb_send;
int64_t keyb_hold_time;
input_event_t keyb_event;
int select_pressed;
int keyb_overlay;
unsigned keyb_x;
unsigned keyb_y;
int input_state[MAX_PADS][5];
void*  snapshot_buffer;
size_t snapshot_size;
const void* tape_data;
size_t tape_size;

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
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   // Kempston
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   // Sinclair 1
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   // Sinclair 2
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   // Timex 1
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   // Timex 2
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   // Fuller
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Fire" },
   { 6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Keyboard overlay" },
   // TODO: Is this needed?
   { 255, 255, 255, 255, "" }
};

// Must implement the keyboard
keysyms_map_t keysyms_map[] = {
   { RETROK_TAB,       INPUT_KEY_Tab         },
   { RETROK_RETURN,    INPUT_KEY_Return      },
   { RETROK_ESCAPE,    INPUT_KEY_Escape      },
   { RETROK_SPACE,     INPUT_KEY_space       },
   { RETROK_EXCLAIM,   INPUT_KEY_exclam      },
   { RETROK_HASH,      INPUT_KEY_numbersign  },
   { RETROK_DOLLAR,    INPUT_KEY_dollar      },
   { RETROK_AMPERSAND, INPUT_KEY_ampersand   },
   { RETROK_QUOTE,     INPUT_KEY_apostrophe  },
   { RETROK_LEFTPAREN, INPUT_KEY_parenleft   },
   { RETROK_RIGHTPAREN, INPUT_KEY_parenright  },
   { RETROK_ASTERISK,  INPUT_KEY_asterisk    },
   { RETROK_PLUS,      INPUT_KEY_plus        },
   { RETROK_COMMA,     INPUT_KEY_comma       },
   { RETROK_MINUS,     INPUT_KEY_minus       },
   { RETROK_PERIOD,    INPUT_KEY_period      },
   { RETROK_SLASH,     INPUT_KEY_slash       },
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
   { RETROK_COLON,     INPUT_KEY_colon       },
   { RETROK_SEMICOLON, INPUT_KEY_semicolon   },
   { RETROK_LESS,      INPUT_KEY_less        },
   { RETROK_EQUALS,    INPUT_KEY_equal       },
   { RETROK_GREATER,   INPUT_KEY_greater     },
   { RETROK_CARET,     INPUT_KEY_asciicircum },
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
   { RETROK_BACKSPACE, INPUT_KEY_BackSpace   },
   { RETROK_KP_ENTER,  INPUT_KEY_KP_Enter    },
   { RETROK_UP,        INPUT_KEY_Up          },
   { RETROK_DOWN,      INPUT_KEY_Down        },
   { RETROK_LEFT,      INPUT_KEY_Left        },
   { RETROK_RIGHT,     INPUT_KEY_Right       },
   { RETROK_INSERT,    INPUT_KEY_Insert      },
   { RETROK_DELETE,    INPUT_KEY_Delete      },
   { RETROK_HOME,      INPUT_KEY_Home        },
   { RETROK_END,       INPUT_KEY_End         },
   { RETROK_PAGEUP,    INPUT_KEY_Page_Up     },
   { RETROK_PAGEDOWN,  INPUT_KEY_Page_Down   },
   { RETROK_F1,        INPUT_KEY_F1          },
   { RETROK_F2,        INPUT_KEY_F2          },
   { RETROK_F3,        INPUT_KEY_F3          },
   { RETROK_F4,        INPUT_KEY_F4          },
   { RETROK_F5,        INPUT_KEY_F5          },
   { RETROK_F6,        INPUT_KEY_F6          },
   { RETROK_F7,        INPUT_KEY_F7          },
   { RETROK_F8,        INPUT_KEY_F8          },
   { RETROK_F9,        INPUT_KEY_F9          },
   { RETROK_F10,       INPUT_KEY_F10         },
   { RETROK_F11,       INPUT_KEY_F11         },
   { RETROK_F12,       INPUT_KEY_F12         },
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
   { RETROK_MENU,      INPUT_KEY_Mode_switch },
   { 0, 0 }	// End marker: DO NOT MOVE!
};

#ifdef LOG_PERFORMANCE
#define RETRO_PERFORMANCE_INIT(name)  static struct retro_perf_counter name = {#name}; if (!name.registered) perf_cb.perf_register(&(name))
#define RETRO_PERFORMANCE_START(name) perf_cb.perf_start(&(name))
#define RETRO_PERFORMANCE_STOP(name)  perf_cb.perf_stop(&(name))
#else
#define RETRO_PERFORMANCE_INIT(name)
#define RETRO_PERFORMANCE_START(name)
#define RETRO_PERFORMANCE_STOP(name)
#endif

static void update_bool(const char* key, int* value, int def)
{
   struct retro_variable var;
  
   var.key = key;
   var.value = NULL;
   int x = def;
   
   if (env_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         x = 1;
      else if (!strcmp(var.value, "disabled"))
         x = 0;
      else
         log_cb(RETRO_LOG_ERROR, "Invalid value for %s: %s\n", key, var.value);
   }
   
   *value = x;
   log_cb(RETRO_LOG_INFO, "%s set to %d\n", key, x);
}

static void update_string_list(const char* key, const char** value, const char* options[], const char* values[], unsigned count)
{
   struct retro_variable var;
  
   var.key = key;
   var.value = NULL;
   const char* x = values[0];
   
   if (env_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      unsigned i;
      bool found = false;
      
      for (i = 0; i < count; i++)
      {
         if (!strcmp(var.value, options[i]))
         {
            x = values[i];
            found = true;
            break;
         }
      }
      
      if (!found)
      {
         log_cb(RETRO_LOG_ERROR, "Invalid value for %s: \"%s\"\n", key, var.value);
      }
   }
   
   if (*value)
   {
      libspectrum_free(*value);
   }
   
   *value = utils_safe_strdup(x);
   log_cb(RETRO_LOG_INFO, "%s set to \"%s\"\n", key, x);
}

static void update_long_list(const char* key, long* value, long values[], unsigned count)
{
   struct retro_variable var;
  
   var.key = key;
   var.value = NULL;
   long x = values[0];
   
   if (env_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      char *endptr;
      long y = strtol(var.value, &endptr, 10);
      
      if (*endptr != 0 || errno == ERANGE)
      {
         log_cb(RETRO_LOG_ERROR, "Invalid value for %s: %s\n", key, var.value);
      }
      else
      {
         unsigned i;
         bool found = false;
         
         for (i = 0; i < count; i++)
         {
            if (y == values[i])
            {
               x = y;
               found = true;
               break;
            }
         }
         
         if (!found)
         {
            log_cb(RETRO_LOG_ERROR, "Invalid value for %s: %s\n", key, var.value);
         }
      }
   }
   
   *value = x;
   log_cb(RETRO_LOG_INFO, "%s set to %ld\n", key, x);
}

void update_variables(void)
{
   {
      // Change this when we're emulating Timex. The MAX_[WIDTH|HEIGHT] macros must be changed also.
      hard_width = 320;
      hard_height = 240;
      
      soft_width = hard_width;
      soft_height = hard_height;
      first_pixel = 0;
      
      change_geometry = true;
      
      log_cb(RETRO_LOG_INFO, "Hard resolution set to %ux%u\n", hard_width, hard_height);
      log_cb(RETRO_LOG_INFO, "Soft resolution set to %ux%u\n", soft_width, soft_height);
   }
   
   {
      int value;
      update_bool("fuse_hide_border", &value, 0);
      
      if (value != hide_border)
      {
         hide_border = value;
         
         soft_width = hide_border ? 256 : 320;
         soft_height = hide_border ? 192 : 240;
         first_pixel = ( hard_height - soft_height ) / 2 * hard_width + ( hard_width - soft_width ) / 2;
         
         change_geometry = true;
         
         log_cb(RETRO_LOG_INFO, "Soft resolution set to %ux%u\n", soft_width, soft_height);
      }
   }
   
   update_bool("fuse_fast_load", &settings_current.accelerate_loader, 1);
   settings_current.fastload = settings_current.accelerate_loader;

   update_bool("fuse_load_sound", &settings_current.sound_load, 1);

   {
      static const char* options[] = { "tv speaker", "beeper", "unfiltered" };
      static const char* values[]  = { "TV speaker", "Beeper", "Unfiltered" };
      update_string_list("fuse_speaker_type", &settings_current.speaker_type, options, values, sizeof(options) / sizeof(options[0]));
   }

   {
      static const char* options[] = { "none", "acb", "abc" };
      static const char* values[]  = { "None", "ACB", "ABC" };
      update_string_list("fuse_ay_stereo_separation", &settings_current.stereo_ay, options, values, sizeof(options) / sizeof(options[0]));
   }

   update_bool("fuse_key_ovrlay_transp", &keyb_transparent, 1);

   {
      static long values[] = { 500, 1000, 100, 300 };
      long value;
      update_long_list("fuse_key_hold_time", &value, values, sizeof(values) / sizeof(values[0]));
      keyb_hold_time = value * 1000LL;
   }
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
   
   log_cb(RETRO_LOG_ERROR, "Unknown device 0x%04x\n", device);
   return 0;
}

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name = PACKAGE_NAME;
   info->library_version = PACKAGE_VERSION;
   info->need_fullpath = false;
   info->block_extract = false;
   info->valid_extensions = "tzx|tap|z80";
}

void retro_set_environment(retro_environment_t cb)
{
   env_cb = cb;

   static const struct retro_variable vars[] = {
      { "fuse_hide_border", "Hide Video Border; disabled|enabled" },
      { "fuse_fast_load", "Tape Fast Load; enabled|disabled" },
      { "fuse_load_sound", "Tape Load Sound; enabled|disabled" },
      { "fuse_speaker_type", "Speaker Type; tv speaker|beeper|unfiltered" },
      { "fuse_ay_stereo_separation", "AY Stereo Separation; none|acb|abc" },
      { "fuse_key_ovrlay_transp", "Transparent Keyboard Overlay; enabled|disabled" },
      { "fuse_key_hold_time", "Time to Release Key in ms; 500|1000|100|300" },
      { NULL, NULL },
   };
   
   static const struct retro_controller_description controllers[] = {
      { "Cursor Joystick", RETRO_DEVICE_CURSOR_JOYSTICK },
      { "Kempston Joystick", RETRO_DEVICE_KEMPSTON_JOYSTICK },
      { "Sinclair 1 Joystick", RETRO_DEVICE_SINCLAIR1_JOYSTICK },
      { "Sinclair 2 Joystick", RETRO_DEVICE_SINCLAIR2_JOYSTICK },
      { "Timex 1 Joystick", RETRO_DEVICE_TIMEX1_JOYSTICK },
      { "Timex 2 Joystick", RETRO_DEVICE_TIMEX2_JOYSTICK },
      { "Fuller Joystick", RETRO_DEVICE_FULLER_JOYSTICK }
   };
   
   static const struct retro_controller_info ports[] = {
      { controllers, 7 },
      { controllers, 7 },
      { controllers, 7 },
      { controllers, 7 },
      { controllers, 7 },
      { controllers, 7 },
      { controllers, 7 },
      { 0 }
   };

   // This seem to be broken right now, as info is NULL in retro_load_game
   // even when we load a game via the frontend after booting to BASIC.
   //bool _true = true;
   //cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, (void*)&_true);
   
   cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
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

   // Always get the perf interface because we need it in compat_timer_get_time
   // and compat_timer_sleep.
   if (!env_cb(RETRO_ENVIRONMENT_GET_PERF_INTERFACE, &perf_cb))
   {
      perf_cb.get_time_usec = NULL;
   }
}

static const char* get_extension(const void* data, size_t size)
{
   libspectrum_id_t type;
   libspectrum_error error = libspectrum_identify_file(&type, NULL, (const unsigned char*)data, size);
   
   if (type != LIBSPECTRUM_ID_UNKNOWN)
   {
return_ext:
      switch (type)
      {
         case LIBSPECTRUM_ID_RECORDING_RZX: return ".rzx";
         case LIBSPECTRUM_ID_SNAPSHOT_SNA:  return ".sna";
         case LIBSPECTRUM_ID_SNAPSHOT_Z80:  return ".z80";
         case LIBSPECTRUM_ID_TAPE_TAP:      // has same extension as LIBSPECTRUM_ID_TAPE_WARAJEVO
         case LIBSPECTRUM_ID_TAPE_WARAJEVO: return ".tap";
         case LIBSPECTRUM_ID_TAPE_TZX:      return ".tzx";
         case LIBSPECTRUM_ID_SNAPSHOT_SP:   return ".sp";
         case LIBSPECTRUM_ID_SNAPSHOT_SNP:  return ".snp";
         case LIBSPECTRUM_ID_SNAPSHOT_ZXS:  return ".zxs";
         case LIBSPECTRUM_ID_SNAPSHOT_SZX:  return ".szx";
         case LIBSPECTRUM_ID_TAPE_CSW:      return ".csw";
         case LIBSPECTRUM_ID_TAPE_Z80EM:    return ".raw";
         case LIBSPECTRUM_ID_TAPE_WAV:      return ".wav";
         case LIBSPECTRUM_ID_TAPE_SPC:      return ".spc";
         case LIBSPECTRUM_ID_TAPE_STA:      return ".sta";
         case LIBSPECTRUM_ID_TAPE_LTP:      return ".ltp";
         case LIBSPECTRUM_ID_TAPE_PZX:      return ".pzx";
         default:                           return NULL;
      }
   }
   
   libspectrum_snap* snap = libspectrum_snap_alloc();
   error = libspectrum_snap_read(snap, (const libspectrum_byte*)data, size, LIBSPECTRUM_ID_SNAPSHOT_Z80, NULL);
   libspectrum_snap_free(snap);
   
   if (error == LIBSPECTRUM_ERROR_NONE)
   {
      type = LIBSPECTRUM_ID_SNAPSHOT_Z80;
      goto return_ext;
   }
   
   return NULL;
}

bool retro_load_game(const struct retro_game_info *info)
{
   if (!perf_cb.get_time_usec)
   {
      log_cb(RETRO_LOG_ERROR, "Fuse needs the perf interface");
      return false;
   }
   
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;

   if (!env_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      log_cb(RETRO_LOG_ERROR, "RGB565 is not supported\n");
      return false;
   }
   
   env_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, input_descriptors);
   memset(input_state, 0, sizeof(input_state));
   select_pressed = keyb_overlay = 0;
   keyb_x = keyb_y = 0;
   keyb_send = 0;
   snapshot_buffer = NULL;
   
   if (info != NULL)
   {
      // Try to identify the file type
      char filename_option[32];
      snprintf(filename_option, sizeof(filename_option), "-t*%s", get_extension(info->data, info->size));
      filename_option[sizeof(filename_option) - 1] = 0;
      
      log_cb(RETRO_LOG_INFO, "Named the input file as %s\n", filename_option + 2);
      
      char *argv[] = {
         "fuse",
         filename_option
      };
      
      tape_data = info->data;
      tape_size = info->size;
      
      log_cb(RETRO_LOG_INFO, "Loading content %s\n", info->path);
      return fuse_init(sizeof(argv) / sizeof(argv[0]), argv) == 0;
   }
   
   // Boots the emulation into BASIC
   char *argv[] = {
      "fuse"
   };
   
   log_cb(RETRO_LOG_INFO, "Booting into BASIC\n");
   return fuse_init(sizeof(argv) / sizeof(argv[0]), argv) == 0;
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
   log_cb(RETRO_LOG_DEBUG, "%s(%p)\n", __FUNCTION__, info);
   
   // Here we always use the "hard" resolution to accomodate output with *and*
   // without the video border
   info->geometry.base_width = hard_width;
   info->geometry.base_height = hard_height;
   
   info->geometry.max_width = MAX_WIDTH;
   info->geometry.max_height = MAX_HEIGHT;
   info->geometry.aspect_ratio = 0.0f;
   info->timing.fps = 50.0;
   info->timing.sample_rate = 44100.0;
   
   log_cb(RETRO_LOG_INFO, "Set retro_system_av_info to:\n");
   log_cb(RETRO_LOG_INFO, "  base_width   = %u\n", info->geometry.base_width);
   log_cb(RETRO_LOG_INFO, "  base_height  = %u\n", info->geometry.base_height);
   log_cb(RETRO_LOG_INFO, "  max_width    = %u\n", info->geometry.max_width);
   log_cb(RETRO_LOG_INFO, "  max_height   = %u\n", info->geometry.max_height);
   log_cb(RETRO_LOG_INFO, "  aspect_ratio = %f\n", info->geometry.max_height);
   log_cb(RETRO_LOG_INFO, "  fps          = %f\n", info->timing.fps);
   log_cb(RETRO_LOG_INFO, "  sample_rate  = %f\n", info->timing.sample_rate);
}

static void render_video(void)
{
   if (change_geometry)
   {
      struct retro_game_geometry geometry;
      
      // Here we use the "soft" resolution that is changed according to the
      // fuse_hide_border variable
      geometry.base_width = soft_width;
      geometry.base_height = soft_height;
      
      geometry.max_width = MAX_WIDTH;
      geometry.max_height = MAX_HEIGHT;
      geometry.aspect_ratio = 0.0f;
      
      env_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry);
      change_geometry = false;
      
      log_cb(RETRO_LOG_INFO, "Set retro_game_geometry to:\n");
      log_cb(RETRO_LOG_INFO, "  base_width   = %u\n", geometry.base_width);
      log_cb(RETRO_LOG_INFO, "  base_height  = %u\n", geometry.base_height);
      log_cb(RETRO_LOG_INFO, "  max_width    = %u\n", geometry.max_width);
      log_cb(RETRO_LOG_INFO, "  max_height   = %u\n", geometry.max_height);
      log_cb(RETRO_LOG_INFO, "  aspect_ratio = %f\n", geometry.max_height);
   }
   
   if (!keyb_overlay)
   {
      video_cb(show_frame ? image_buffer + first_pixel : NULL, soft_width, soft_height, hard_width * sizeof(uint16_t));
   }
   else
   {
      if (show_frame)
      {
         if (keyb_transparent)
         {
            uint16_t* src1 = keyboard_overlay;
            uint16_t* src2 = image_buffer;
            uint16_t* end = src1 + sizeof(keyboard_overlay) / sizeof(keyboard_overlay[0]);
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
         
         uint16_t* pixel = image_buffer_2 + y * 320 + x + 1;
         unsigned i, j;
         
         for (i = 0; i < width - 2; i++)
         {
            *pixel = ~*pixel;
            pixel++;
         }
         
         pixel += 320 - width + 1;
         
         for (j = 0; j < 22; j++)
         {
            for (i = 0; i < width; i++)
            {
               *pixel = ~*pixel;
               pixel++;
            }
            
            pixel += 320 - width;
         }
         
         for (i = 0; i < width - 2; i++)
         {
            pixel++;
            *pixel = ~*pixel;
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
      update_variables();
   }

   show_frame = some_audio = 0;
   
   if (settings_current.fastload && tape_is_playing())
   {
      // Repeat the loop while the tape is playing if we're fast loading.
      do {
         input_poll_cb();
         z80_do_opcodes();
         event_do_events();
      }
      while (tape_is_playing());
   }
   else
   {
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
   }
   
   render_video();
}

void retro_deinit(void)
{
   fuse_end();

#ifdef LOG_PERFORMANCE
   perf_cb.perf_log();
#endif
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   switch (device)
   {
      case RETRO_DEVICE_CURSOR_JOYSTICK:
      case RETRO_DEVICE_KEMPSTON_JOYSTICK:
      case RETRO_DEVICE_SINCLAIR1_JOYSTICK:
      case RETRO_DEVICE_SINCLAIR2_JOYSTICK:
      case RETRO_DEVICE_TIMEX1_JOYSTICK:
      case RETRO_DEVICE_TIMEX2_JOYSTICK:
      case RETRO_DEVICE_FULLER_JOYSTICK:
         input_devices[port] = device;
         
         if (port == 0)
         {
            settings_current.joystick_1_output = get_joystick(device);
            log_cb(RETRO_LOG_INFO, "Joystick 1 set to 0x%04x\n", device);
         }
         else if (port == 1)
         {
            settings_current.joystick_2_output = get_joystick(device);
            log_cb(RETRO_LOG_INFO, "Joystick 2 set to 0x%04x\n", device);
         }
         
         break;
         
      default:
         log_cb(RETRO_LOG_ERROR, "Unknown device 0x%04x, setting type to RETRO_DEVICE_KEYBOARD\n", device);
         input_devices[port] = RETRO_DEVICE_KEYBOARD;
         break;
   }
}

void retro_reset(void)
{
}

size_t retro_serialize_size(void)
{
   log_cb(RETRO_LOG_DEBUG, "%s()\n", __FUNCTION__);
   fuse_emulation_pause();
   snapshot_write("dummy.szx"); // filename is only used to get the snapshot type
   fuse_emulation_unpause();
   return snapshot_size;
}

bool retro_serialize(void *data, size_t size)
{
   log_cb(RETRO_LOG_DEBUG, "%s(%p, %lu)\n", __FUNCTION__, data, size);
   bool res = false;
   
   if (size >= snapshot_size)
   {
      memcpy(data, snapshot_buffer, snapshot_size);
      res = true;
   }
   else
   {
      log_cb(RETRO_LOG_ERROR, "Provided buffer size of %lu is less than the required size of %lu\n", size, snapshot_size);
   }

   return res;
}

bool retro_unserialize(const void *data, size_t size)
{
   log_cb(RETRO_LOG_DEBUG, "%s(%p, %lu)\n", __FUNCTION__, data, size);
   return snapshot_read_buffer(data, size, LIBSPECTRUM_ID_SNAPSHOT_SZX) == 0;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned a, bool b, const char *c)
{
   (void)a;
   (void)b;
   (void)c;
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
   if (snapshot_buffer)
   {
      free(snapshot_buffer);
   }
}

unsigned retro_get_region(void)
{
   // TODO set this accordingly to the machine being emulated
   return RETRO_REGION_PAL;
}

// Dummy callbacks for the UI

#define MENU_CALLBACK( name ) void name( int action )
MENU_CALLBACK(menu_file_savescreenaspng) {}
MENU_CALLBACK(menu_file_loadbinarydata)  {}
MENU_CALLBACK(menu_file_savebinarydata)  {}
MENU_CALLBACK(menu_machine_pause)        {}

// Dummy mkstemp

int mkstemp(char *template)
{
  return -1;
}