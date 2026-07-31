#include <libretro.h>
#include <streams/file_stream.h>
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
#include <peripherals/ay.h>
#include <peripherals/disk/beta.h>
#include <peripherals/disk/didaktik.h>
#include <peripherals/disk/plusd.h>
#include <peripherals/if1.h>
#include <peripherals/disk/opus.h>
#include <peripherals/disk/disciple.h>
#include <pokefinder/pokemem.h>
#include <periph.h>

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

/* coreopt() returns -1 when the key is absent from core_vars, when the
   frontend does not answer GET_VARIABLE, or when the stored value matches
   none of the declared options (e.g. a stale config naming an option value
   that no longer exists). Indexing spectrum_keys_map with that directly is
   an out-of-bounds read, so resolve unusable values to "no key" instead.
   The machine option already guards this case (see "option += option < 0"
   in update_variables). */
static int spectrum_key_from_option(int option)
{
   if (option < 0 ||
       option >= (int)(sizeof(spectrum_keys_map) / sizeof(spectrum_keys_map[0])))
      return INPUT_KEY_NONE;

   return spectrum_key_from_option(option);
}

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

static int machine_id_is_60hz(libspectrum_machine id)
{
   return id == LIBSPECTRUM_MACHINE_48_NTSC || id == LIBSPECTRUM_MACHINE_TS2068;
}

#define BGR16(color) rgb32_to_bgr16(color)
#define rgb32_to_bgr16(color) rgbc32_to_bgr16((color & 0xFF0000) >> 16, (color & 0x00FF00) >> 8, (color & 0x0000FF))
#define rgbc32_to_bgr16(red, green, blue) ((blue & 0b11111000) << 8) | ((green & 0b11111100) << 3) | (red >> 3)
#define RGB16(color) rgb32_to_rgb16(color)
#define rgb32_to_rgb16(color) rgbc32_to_rgb16((color & 0xFF0000) >> 16, (color & 0x00FF00) >> 8, (color & 0x0000FF))
#define rgbc32_to_rgb16(red, green, blue) ((red & 0b11111000) << 8) | ((green & 0b11111100) << 3) | (blue >> 3)
#define G3R3B2_TO_RGB565(color) \
    ((((color >> 5) & 0x7) << 13) & 0x7C00) | \
    ((((color >> 2) & 0x7) << 8) & 0x03E0) | \
    (((((color & 0x3) << 1) | (((color & 0x3) & 0x1) | (((color & 0x3) >> 1) & 0x1))) << 2) & 0x001F)


enum PALETTES {
   PALETTE_FUSE = 0,
   PALETTE_ZX_SPECTRUM_WIKIPEDIA,
   PALETTE_BLACK_AND_WHITE_TV,
   PALETTE_GREEN_MONOCHROME,
   PALETTE_AMBAR_MONOCHROME,
   PALETTE_C64,
   PALETTE_CGA_4,
   PALETTE_CGA_8,
   PALETTE_CGA_16,
   PALETTE_INVERTED,
   PALETTE_COUNT
};

static uint16_t palettes[PALETTE_COUNT][16] = {
   [PALETTE_FUSE] =  {
      0x0000, 0x0018, 0xc000, 0xc018,
      0x0600, 0x0618, 0xc600, 0xc618,
      0x0000, 0x001f, 0xf800, 0xf81f,
      0x07e0, 0x07ff, 0xffe0, 0xffff,
   },
   [PALETTE_ZX_SPECTRUM_WIKIPEDIA] = {
      RGB16(0x000000),RGB16(0x0000D7),RGB16(0xD70000),RGB16(0xD700D7),
      RGB16(0x00D700),RGB16(0x00D7D7),RGB16(0xD7D700),RGB16(0xD7D7D7),
      RGB16(0x000000),RGB16(0x0000FF),RGB16(0xFF0000),RGB16(0xFF00FF),
      RGB16(0x00FF00),RGB16(0x00FFFF),RGB16(0xFFFF00),RGB16(0xffffff),
   },
   [PALETTE_BLACK_AND_WHITE_TV] =  {
      0x0000, 0x10a2, 0x39c7, 0x4a69,
      0x738e, 0x8430, 0xad55, 0xbdf7,
      0x0000, 0x18e3, 0x4a69, 0x6b4d,
      0x94b2, 0xb596, 0xe71c, 0xffff,
   },
   [PALETTE_GREEN_MONOCHROME] = {  /* From ZX Spin */
      rgbc32_to_rgb16(0,0,0), /* black */
      rgbc32_to_rgb16(0,33,0), /* blue */
      rgbc32_to_rgb16(0,62,0), /* red */
      rgbc32_to_rgb16(0,85,0), /* magenta*/
      rgbc32_to_rgb16(0,115,0), /* green*/
      rgbc32_to_rgb16(0,136,0), /* cyan*/
      rgbc32_to_rgb16(0,168,0), /* yellow*/
      rgbc32_to_rgb16(0,181,0), /* white */
      rgbc32_to_rgb16(0,0,0), /* bright black */
      rgbc32_to_rgb16(0,52,0), /* bright blue */
      rgbc32_to_rgb16(0,81,0), /* bright red */
      rgbc32_to_rgb16(0,113,0), /* bright magenta*/
      rgbc32_to_rgb16(0,154,0), /* bright green*/
      rgbc32_to_rgb16(0,185,0), /* bright cyan*/
      rgbc32_to_rgb16(0,237,0), /* bright yellow*/
      rgbc32_to_rgb16(0,255,0)  /* bright white*/
   },
   [PALETTE_AMBAR_MONOCHROME] = {  /* From ZX Spin */
      rgbc32_to_rgb16(0,0,0), /* black */
      rgbc32_to_rgb16(34,24,0), /* blue */
      rgbc32_to_rgb16(62,44,0), /* red */
      rgbc32_to_rgb16(86,61,0), /* magenta*/
      rgbc32_to_rgb16(116,82,0), /* green*/
      rgbc32_to_rgb16(136,96,0), /* cyan*/
      rgbc32_to_rgb16(168,119,0), /* yellow*/
      rgbc32_to_rgb16(182,128,0), /* white */
      rgbc32_to_rgb16(0,0,0), /* bright black */
      rgbc32_to_rgb16(52,37,0), /* bright blue */
      rgbc32_to_rgb16(82,58,0), /* bright red */
      rgbc32_to_rgb16(114,80,0), /* bright magenta*/
      rgbc32_to_rgb16(154,109,0), /* bright green*/
      rgbc32_to_rgb16(186,131,0), /* bright cyan*/
      rgbc32_to_rgb16(238,168,0), /* bright yellow*/
      rgbc32_to_rgb16(255,180,1)  /* bright white*/
   },
   [PALETTE_C64] = {
      RGB16(0x000000), /* black */
      RGB16(0x40318D), /* blue */
      RGB16(0x883932), /* red */
      RGB16(0x8B5429), /* magenta*/
      RGB16(0x55A049), /* green*/
      RGB16(0x67B6BD), /* cyan*/
      RGB16(0x574200), /* yellow*/
      RGB16(0x9F9F9F), /* white */
      RGB16(0x000000), /* bright black */
      RGB16(0x7869C4), /* bright blue */
      RGB16(0xB86962), /* bright red */
      RGB16(0x8B5429), /* bright magenta*/
      RGB16(0x94E089), /* bright green*/
      RGB16(0x9F9F9F), /* bright cyan*/
      RGB16(0xBFCE72), /* bright yellow*/
      RGB16(0xFFFFFF), /* bright white*/
   },
   [PALETTE_CGA_4] = {
      RGB16(0x000000),RGB16(0x55ffff),RGB16(0xff55ff),RGB16(0xff55ff),
      RGB16(0x55ffff),RGB16(0x55ffff),RGB16(0xffffff),RGB16(0xffffff),
      RGB16(0x000000),RGB16(0x55ffff),RGB16(0xff55ff),RGB16(0xff55ff),
      RGB16(0x55ffff),RGB16(0x55ffff),RGB16(0xffffff),RGB16(0xffffff),
   },
   [PALETTE_CGA_8] = {
      RGB16(0x000000),  RGB16(0xAAAA), RGB16(0xAA00AA), RGB16(0xAA00AA),
      RGB16(0xAAAA),RGB16(0xAAAA),RGB16(0xAAAAAA),RGB16(0xAAAAAA),
      RGB16(0x000000),RGB16(0x55ffff),RGB16(0xff55ff),RGB16(0xff55ff),
      RGB16(0x55ffff),RGB16(0x55ffff),RGB16(0xffffff),RGB16(0xffffff),
   },
   [PALETTE_CGA_16] = {
      RGB16(0x000000),  RGB16(0x0000AA), RGB16(0xAA0000), RGB16(0xAA00AA),
      RGB16(0x00AA00),RGB16(0x00AAAA),RGB16(0xAA5500),RGB16(0xAAAAAA),
      RGB16(0x000000),RGB16(0x5555FF),RGB16(0xFF5555),RGB16(0xFF55FF),
      RGB16(0x55FF55),RGB16(0x55FFFF),RGB16(0xFFFF55),RGB16(0xffffff),
   },
   [PALETTE_INVERTED] = {
      RGB16(0xffffff),RGB16(0xFEFF31),RGB16(0x30FEFF),RGB16(0x30FE31),
      RGB16(0xFF30EA),RGB16(0xFE3030),RGB16(0x3030EA),RGB16(0x303030),
      RGB16(0xffffff),RGB16(0xFDFF02),RGB16(0x00FDFE),RGB16(0x00FD02),
      RGB16(0xFF00E3),RGB16(0xFD0000),RGB16(0xFD0000),RGB16(0x000000),
   },
};

static int fuse_init_called = 0;
static int forced_machine_at_init = 0;
static int forced_machine_idx = 0;
static int auto_size_savestate = 1;

static unsigned msg_interface_version = 0;
static int show_joystick_type_at_startup;
static int show_emulation_speed_at_startup;
static int display_joystick_type;
static int display_emulation_speed;
static int kempston_mouse_needs_periph_update = 0;
static void sync_kempston_mouse_from_ports(void);

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
static int current_palette = PALETTE_FUSE;

// Multi-disk (M3U) support: a single virtual tray for drive A, swappable at
// runtime via the libretro disk control interface. See retro_load_game()
// (M3U parsing), disk_control_insert_current() and the
// retro_disk_control_ext_callback implementation below.
#define MAX_DISK_IMAGES 16
#define MAX_DISK_PATH_LEN 1024

static char disk_image_paths[MAX_DISK_IMAGES][MAX_DISK_PATH_LEN];
static unsigned num_disk_images = 0;
static unsigned current_disk_index = 0;
static bool disk_tray_ejected = false;
static int content_is_m3u = 0;

// Cached by set_initial_image(), called by the frontend *before*
// retro_load_game() - see the libretro.h doc comment on that callback.
static int initial_disk_index_hint = -1;
static char initial_disk_path_hint[MAX_DISK_PATH_LEN];

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
bool joyp_state[MAX_PADS][16];
bool keyb_state[RETROK_LAST];
void*  snapshot_buffer;
size_t snapshot_size;
void* tape_data;
size_t tape_size;
int joymap[16];
uint16_t *palette = palettes[PALETTE_FUSE];

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
   { RETROK_UP,        INPUT_KEY_Up          },
   { RETROK_DOWN,      INPUT_KEY_Down        },
   { RETROK_RIGHT,     INPUT_KEY_Right       },
   { RETROK_LEFT,      INPUT_KEY_Left        },
   { 0, 0 }    // End marker: DO NOT MOVE!
};

#define CORE_OPTION_VALUE_LIST_ENABLED_DISABLED \
   { "enabled", NULL }, \
   { "disabled", NULL }, \
   { NULL, NULL }

#define CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS \
   { "<none>", NULL }, \
   { "0", NULL }, \
   { "1", NULL }, \
   { "2", NULL }, \
   { "3", NULL }, \
   { "4", NULL }, \
   { "5", NULL }, \
   { "6", NULL }, \
   { "7", NULL }, \
   { "8", NULL }, \
   { "9", NULL }, \
   { "a", NULL }, \
   { "b", NULL }, \
   { "c", NULL }, \
   { "d", NULL }, \
   { "e", NULL }, \
   { "f", NULL }, \
   { "g", NULL }, \
   { "h", NULL }, \
   { "i", NULL }, \
   { "j", NULL }, \
   { "k", NULL }, \
   { "l", NULL }, \
   { "m", NULL }, \
   { "n", NULL }, \
   { "o", NULL }, \
   { "p", NULL }, \
   { "q", NULL }, \
   { "r", NULL }, \
   { "s", NULL }, \
   { "t", NULL }, \
   { "u", NULL }, \
   { "v", NULL }, \
   { "w", NULL }, \
   { "x", NULL }, \
   { "y", NULL }, \
   { "z", NULL }, \
   { "Enter", NULL }, \
   { "Caps", NULL }, \
   { "Symbol", NULL }, \
   { "Space", NULL }, \
   { NULL, NULL }

static struct retro_core_option_v2_category core_option_categories[] = {
   { "system",   "System",   NULL },
   { "video",    "Video",    NULL },
   { "audio",    "Audio",    NULL },
   { "input",    "Input",    NULL },
   { "advanced", "Advanced", NULL },
   { NULL, NULL, NULL },
};

static struct retro_core_option_v2_definition core_option_definitions[] = {
   {
      "fuse_machine",
      "Model (needs content load)",
      NULL,
      NULL,
      NULL,
      "system",
      {
         { "Spectrum 48K", NULL },
         { "Spectrum 48K (NTSC)", NULL },
         { "Spectrum 128K", NULL },
         { "Spectrum +2", NULL },
         { "Spectrum +2A", NULL },
         { "Spectrum +3", NULL },
         { "Spectrum +3e", NULL },
         { "Spectrum SE", NULL },
         { "Timex TC2048", NULL },
         { "Timex TC2068", NULL },
         { "Timex TS2068", NULL },
         { "Spectrum 16K", NULL },
         { "Pentagon 128K", NULL },
         { "Pentagon 512K", NULL },
         { "Pentagon 1024", NULL },
         { "Scorpion 256K", NULL },
         { NULL, NULL }
      },
      "Spectrum 48K"
   },
   {
      "fuse_emulation_speed",
      "Emulation speed percentage (needs content load)",
      NULL,
      NULL,
      NULL,
      "system",
      {
         { "50", NULL },
         { "100", NULL },
         { "150", NULL },
         { "200", NULL },
         { "300", NULL },
         { NULL, NULL }
      },
      "100"
   },
   {
      "fuse_size_border",
      "Size Video Border",
      NULL,
      NULL,
      NULL,
      "video",
      {
         { "full", NULL },
         { "medium", NULL },
         { "small", NULL },
         { "minimum", NULL },
         { "none", NULL },
         { NULL, NULL }
      },
      "full"
   },
   {
      "fuse_palette",
      "Colour Palette",
      NULL,
      NULL,
      NULL,
      "video",
      {
         { "Fuse Standard", NULL },
         { "ZX Standard", NULL },
         { "B&W TV", NULL },
         { "Green Monochrome", NULL },
         { "Ambar Monochrome", NULL },
         { "C64", NULL },
         { "CGA 4 colours", NULL },
         { "CGA 8 colours", NULL },
         { "CGA 16 colours", NULL },
         { "Inverted colours", NULL },
         { NULL, NULL }
      },
      "Fuse Standard"
   },
   {
      "fuse_auto_load",
      "Tape Auto Load",
      NULL,
      NULL,
      NULL,
      "system",
      { CORE_OPTION_VALUE_LIST_ENABLED_DISABLED },
      "enabled"
   },
   {
      "fuse_fast_load",
      "Tape Fast Load",
      NULL,
      NULL,
      NULL,
      "advanced",
      { CORE_OPTION_VALUE_LIST_ENABLED_DISABLED },
      "enabled"
   },
   {
      "fuse_load_sound",
      "Tape Load Sound",
      NULL,
      NULL,
      NULL,
      "audio",
      { CORE_OPTION_VALUE_LIST_ENABLED_DISABLED },
      "enabled"
   },
   {
      "fuse_speaker_type",
      "Speaker Type",
      NULL,
      NULL,
      NULL,
      "audio",
      {
         { "tv speaker", NULL },
         { "beeper", NULL },
         { "unfiltered", NULL },
         { NULL, NULL }
      },
      "tv speaker"
   },
   {
      "fuse_ay_stereo_separation",
      "AY Stereo Separation",
      NULL,
      NULL,
      NULL,
      "audio",
      {
         { "none", NULL },
         { "acb", NULL },
         { "abc", NULL },
         { NULL, NULL }
      },
      "none"
   },
   {
      "fuse_turbosound",
      "TurboSound (2x AY-8910)",
      NULL,
      NULL,
      NULL,
      "audio",
      { CORE_OPTION_VALUE_LIST_ENABLED_DISABLED },
      "disabled"
   },
   {
      "fuse_key_ovrlay_transp",
      "Transparent Keyboard Overlay",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_ENABLED_DISABLED },
      "enabled"
   },
   {
      "fuse_key_hold_time",
      "Time to Release Key in ms",
      NULL,
      NULL,
      NULL,
      "input",
      {
         { "100", NULL },
         { "300", NULL },
         { "500", NULL },
         { "1000", NULL },
         { NULL, NULL }
      },
      "500"
   },
   {
      "fuse_mouse_swap_buttons",
      "Kempston Mouse Swap Buttons",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_ENABLED_DISABLED },
      "disabled"
   },
   {
      "fuse_display_joystick_type",
      "Display joystick type at startup",
      NULL,
      NULL,
      NULL,
      "advanced",
      { CORE_OPTION_VALUE_LIST_ENABLED_DISABLED },
      "disabled"
   },
   {
      "fuse_display_emulation_speed",
      "Display emulation speed at startup",
      NULL,
      NULL,
      NULL,
      "advanced",
      { CORE_OPTION_VALUE_LIST_ENABLED_DISABLED },
      "disabled"
   },
   {
      "fuse_auto_size_savestate",
      "Use Auto Size for Savestates. For Netplay 'Off' is recommended",
      NULL,
      NULL,
      NULL,
      "advanced",
      { CORE_OPTION_VALUE_LIST_ENABLED_DISABLED },
      "enabled"
   },
   {
      "fuse_joypad_left",
      "Joypad Left mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_right",
      "Joypad Right mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_up",
      "Joypad Up mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_down",
      "Joypad Down mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_start",
      "Joypad Start mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_a",
      "Joypad A button mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_b",
      "Joypad B button mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_x",
      "Joypad X button mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_y",
      "Joypad Y button mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_l",
      "Joypad L button mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_r",
      "Joypad R button mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_l2",
      "Joypad L2 button mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_r2",
      "Joypad R2 button mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_l3",
      "Joypad L3 button mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   {
      "fuse_joypad_r3",
      "Joypad R3 button mapping",
      NULL,
      NULL,
      NULL,
      "input",
      { CORE_OPTION_VALUE_LIST_SPECTRUM_KEYS },
      "<none>"
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, { { NULL, NULL } }, NULL },
};

static struct retro_core_options_v2 core_options_v2 = {
   core_option_categories,
   core_option_definitions
};

static const struct retro_variable core_vars[] =
{
   { "fuse_machine", "Model (needs content load); Spectrum 48K|Spectrum 48K (NTSC)|Spectrum 128K|Spectrum +2|Spectrum +2A|Spectrum +3|Spectrum +3e|Spectrum SE|Timex TC2048|Timex TC2068|Timex TS2068|Spectrum 16K|Pentagon 128K|Pentagon 512K|Pentagon 1024|Scorpion 256K" },
   { "fuse_emulation_speed", "Emulation speed percentage (needs content load); 100|150|200|300|50"},
   { "fuse_size_border", "Size Video Border; full|medium|small|minimum|none" },
   { "fuse_palette", "Colour Palette; Fuse Standard|ZX Standard|B&W TV|Green Monochrome|Ambar Monochrome|C64|CGA 4 colours|CGA 8 colours|CGA 16 colours|Inverted colours"},
   { "fuse_auto_load", "Tape Auto Load; enabled|disabled" },
   { "fuse_fast_load", "Tape Fast Load; enabled|disabled" },
   { "fuse_load_sound", "Tape Load Sound; enabled|disabled" },
   { "fuse_speaker_type", "Speaker Type; tv speaker|beeper|unfiltered" },
   { "fuse_ay_stereo_separation", "AY Stereo Separation; none|acb|abc" },
   { "fuse_turbosound", "TurboSound (2x AY-8910); disabled|enabled" },
   { "fuse_key_ovrlay_transp", "Transparent Keyboard Overlay; enabled|disabled" },
   { "fuse_key_hold_time", "Time to Release Key in ms; 500|1000|100|300" },
   { "fuse_display_joystick_type", "Display joystick type at startup; disabled|enabled" },
   { "fuse_display_emulation_speed", "Display emulation speed at startup; disabled|enabled" },
   { "fuse_auto_size_savestate", "Use Auto Size for Savestates. For Netplay 'Off' is recommended; enabled|disabled" },
   { "fuse_mouse_swap_buttons", "Kempston Mouse Swap Buttons; disabled|enabled" },
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

void Retro_Msg(const char * msg_str)
{
   if (msg_interface_version >= 1)
   {
      struct retro_message_ext msg = {
         msg_str,
         3000,
         3,
         RETRO_LOG_WARN,
         RETRO_MESSAGE_TARGET_ALL,
         RETRO_MESSAGE_TYPE_NOTIFICATION,
         -1
      };
      env_cb(RETRO_ENVIRONMENT_SET_MESSAGE_EXT, &msg);
   }
   else
   {
      struct retro_message msg = {
         msg_str,
         180
      };
      env_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &msg);
   }
}

int update_variables(int force)
{
   int flags = 0;

   if (force)
   {
      // Only change the machine when reloading content
      int option = coreopt(env_cb, core_vars, "fuse_machine", NULL);
      option += option < 0;

      if (forced_machine_at_init)
         option = forced_machine_idx;

      const machine_t *new_machine = machine_list + option;

      if (new_machine != machine || force)
      {
         if (settings_current.start_machine)
         {
            libspectrum_free((void*)settings_current.start_machine);
         }

         settings_current.start_machine = utils_safe_strdup(new_machine->fuse_id);

         if (machine == NULL || machine_id_is_60hz(new_machine->id) != machine_id_is_60hz(machine->id))
         {
            // region and fps change
            flags |= UPDATE_AV_INFO;
         }

         machine = new_machine;
         frame_time = 1000.0 / (machine_id_is_60hz(machine->id) ? 60.0 : 50.0);
         flags |= UPDATE_MACHINE;
      }

      // Fuse always renders the full PAL-sized canvas (DISPLAY_SCREEN_HEIGHT
      // is a compile-time constant); 60Hz machines get a smaller window
      // carved out of it below, when the border size is applied
      unsigned width = machine->is_timex ? 640 : 320;
      unsigned height = machine->is_timex ? 576 : 288;

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
            // 60Hz machines only have ~24 border lines above and below the
            // paper area; show a 240 (480 for Timex) line window centred on
            // the canvas so the paper stays centred with real-sized borders
            soft_height = machine_id_is_60hz(machine->id) ? (machine->is_timex ? 480 : 240) : hard_height;
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
            // Same 60Hz window as above
            soft_height = machine_id_is_60hz(machine->id) ? (machine->is_timex ? 480 : 240) : hard_height;
         }

         first_pixel = (hard_height - soft_height) / 2 * hard_width + (hard_width - soft_width) / 2;
         flags |= UPDATE_GEOMETRY;
      }
   }

   {
      const char* value;
      int option = coreopt(env_cb, core_vars, "fuse_emulation_speed", &value);
      settings_current.emulation_speed = option >= 0 ? atoi(value) : 100;
   }

   {
      int option = coreopt(env_cb, core_vars, "fuse_palette", NULL);
      option += option < 0;
      if (option>=0 && option<=PALETTE_COUNT-1 && current_palette!=option)
      {
         current_palette = option;
         palette = palettes[current_palette];
         display_refresh_all();
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


   {
      int joystick_option = coreopt(env_cb, core_vars, "fuse_display_joystick_type", NULL);
      show_joystick_type_at_startup = joystick_option == 1;
   }

   {
      int speed_option = coreopt(env_cb, core_vars, "fuse_display_emulation_speed", NULL);
      show_emulation_speed_at_startup = speed_option == 1;
   }

   display_joystick_type = show_joystick_type_at_startup;
   display_emulation_speed = show_emulation_speed_at_startup;

   if (coreopt(env_cb, core_vars, "fuse_auto_size_savestate", NULL) == 0)
      auto_size_savestate = TRUE;
   else
      auto_size_savestate = FALSE;

   ay_turbosound_enabled = coreopt(env_cb, core_vars, "fuse_turbosound", NULL) == 1;

   settings_current.mouse_swap_buttons = coreopt(env_cb, core_vars, "fuse_mouse_swap_buttons", NULL) == 1;

   const char* value;
   int option = coreopt(env_cb, core_vars, "fuse_joypad_up", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_UP ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_down", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_DOWN ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_left", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_LEFT ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_right", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_RIGHT ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_a", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_A ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_b", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_B ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_x", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_X ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_y", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_Y ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_l", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_L ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_r", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_R ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_l2", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_L2 ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_r2", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_R2 ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_l3", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_L3 ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_r3", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_R3 ] = spectrum_key_from_option(option);

   option = coreopt(env_cb, core_vars, "fuse_joypad_start", &value );
   joymap[ RETRO_DEVICE_ID_JOYPAD_START ] = spectrum_key_from_option(option);

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
   info->library_name = PACKAGE_NAME;
   info->library_version = version;
   info->need_fullpath = false;
   info->block_extract = false;
   info->valid_extensions = "tzx|tap|z80|rzx|scl|trd|dsk|dck|sna|szx|zip|m3u";
}

// Disk control interface (see the implementations and disk_control_ext_cb
// definition further down, near retro_load_game).
static bool RETRO_CALLCONV disk_set_eject_state(bool ejected);
static bool RETRO_CALLCONV disk_get_eject_state(void);
static unsigned RETRO_CALLCONV disk_get_image_index(void);
static bool RETRO_CALLCONV disk_set_image_index(unsigned index);
static unsigned RETRO_CALLCONV disk_get_num_images(void);
static bool RETRO_CALLCONV disk_replace_image_index(unsigned index, const struct retro_game_info *info);
static bool RETRO_CALLCONV disk_add_image_index(void);
static struct retro_disk_control_ext_callback disk_control_ext_cb;

void retro_set_environment(retro_environment_t cb)
{
   struct retro_vfs_interface_info vfs_iface_info;

   env_cb = cb;

   /* Hand the frontend's VFS to libretro-common's filestream layer, which
      every file access in this core goes through. Ask for the newest
      interface revision we use and fall back, since a frontend that only
      implements an older one must still work. If the frontend offers no
      VFS at all, filestream keeps using libretro's own vfs_implementation,
      so the call path through the core is identical either way. */
   {
      unsigned v;

      for (v = 3; v >= 1; v--)
      {
         vfs_iface_info.required_interface_version = v;
         vfs_iface_info.iface                      = NULL;

         if (cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_iface_info))
         {
            filestream_vfs_init(&vfs_iface_info);
            break;
         }
      }
   }

   static const struct retro_controller_description controllers_and_kb[] = {
      { "Core defined Input",  RETRO_DEVICE_AUTO_CFG           },
      { "Cursor Joystick",     RETRO_DEVICE_CURSOR_JOYSTICK    },
      { "Kempston Joystick",   RETRO_DEVICE_KEMPSTON_JOYSTICK  },
      { "Sinclair 1 Joystick", RETRO_DEVICE_SINCLAIR1_JOYSTICK },
      { "Sinclair 2 Joystick", RETRO_DEVICE_SINCLAIR2_JOYSTICK },
      { "Timex 1 Joystick",    RETRO_DEVICE_TIMEX1_JOYSTICK    },
      { "Timex 2 Joystick",    RETRO_DEVICE_TIMEX2_JOYSTICK    },
      { "Fuller Joystick",     RETRO_DEVICE_FULLER_JOYSTICK    },
      { "Kempston Mouse",      RETRO_DEVICE_KEMPSTON_MOUSE     },
      { "Sinclair Keyboard",   RETRO_DEVICE_SPECTRUM_KEYBOARD  }
   };

   static const struct retro_controller_info ports[MAX_PADS + 1] = {
      { controllers_and_kb, sizeof(controllers_and_kb) / sizeof(controllers_and_kb[0]) }, // port 1
      { controllers_and_kb, sizeof(controllers_and_kb) / sizeof(controllers_and_kb[0]) }, // port 2
      { controllers_and_kb, sizeof(controllers_and_kb) / sizeof(controllers_and_kb[0]) }, // port 3
      { NULL, 0 }
   };

   bool yes = true;
   unsigned core_options_version = 0;
   cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &yes);

   if (cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &core_options_version) &&
       core_options_version >= 2)
   {
      cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2, (void*)&core_options_v2);
   }
   else
   {
      cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)core_vars);
   }

   cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);

   unsigned dc_version = 0;
   if (cb(RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION, &dc_version) && dc_version >= 1)
   {
      cb(RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE, &disk_control_ext_cb);
   }
   else
   {
      static const struct retro_disk_control_callback disk_control_cb = {
         disk_set_eject_state,
         disk_get_eject_state,
         disk_get_image_index,
         disk_set_image_index,
         disk_get_num_images,
         disk_replace_image_index,
         disk_add_image_index,
      };
      cb(RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE, (void*)&disk_control_cb);
   }
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

   msg_interface_version = 0;
   env_cb(RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION, &msg_interface_version);

   machine = machine_list;
   total_time_ms = 0.0;
   active_cheats = NULL;

   // Always report a mouse as available so Fuse auto-grabs it at startup
   // (see fuse_init() -> ui_mouse_grab()); our ui_mouse_grab() stub in
   // src/compat/mouse.c always succeeds, so this keeps ui_mouse_grabbed set
   // and ui_mouse_button()/ui_mouse_motion() active whenever a port is
   // configured as RETRO_DEVICE_KEMPSTON_MOUSE.
   ui_mouse_present = 1;

   // Set default controllers
   retro_set_controller_port_device( 0, RETRO_DEVICE_CURSOR_JOYSTICK   );
   retro_set_controller_port_device( 1, RETRO_DEVICE_KEMPSTON_JOYSTICK );
   retro_set_controller_port_device( 2, RETRO_DEVICE_SPECTRUM_KEYBOARD );

   show_joystick_type_at_startup = FALSE;
   show_emulation_speed_at_startup = FALSE;
   display_joystick_type = FALSE;
   display_emulation_speed = FALSE;
}

static libspectrum_id_t identify_file(const char* filename, const void* data, size_t size)
{
   libspectrum_id_t type;
   libspectrum_error error = libspectrum_identify_file(&type, filename, (const unsigned char*)data, size);

   if (error == LIBSPECTRUM_ERROR_NONE && type != LIBSPECTRUM_ID_UNKNOWN)
      return type;

   // Default to TRD, we won't be able to load TRD files otherwise
   return LIBSPECTRUM_ID_DISK_TRD;
}

static libspectrum_id_t identify_file_get_ext(const char* filename, const void* data, size_t size, const char** ext)
{
   libspectrum_id_t type = identify_file(filename, data, size);

   switch (type)
   {
      case LIBSPECTRUM_ID_RECORDING_RZX:  *ext = ".rzx"; break;
      case LIBSPECTRUM_ID_SNAPSHOT_SNA:   *ext = ".sna"; break;
      case LIBSPECTRUM_ID_SNAPSHOT_Z80:   *ext = ".z80"; break;
      case LIBSPECTRUM_ID_TAPE_TAP:       // has same extension as LIBSPECTRUM_ID_TAPE_WARAJEVO
      case LIBSPECTRUM_ID_TAPE_WARAJEVO:  *ext = ".tap"; break;
      case LIBSPECTRUM_ID_TAPE_TZX:       *ext = ".tzx"; break;
      case LIBSPECTRUM_ID_SNAPSHOT_SP:    *ext = ".sp";  break;
      case LIBSPECTRUM_ID_SNAPSHOT_SNP:   *ext = ".snp"; break;
      case LIBSPECTRUM_ID_SNAPSHOT_ZXS:   *ext = ".zxs"; break;
      case LIBSPECTRUM_ID_SNAPSHOT_SZX:   *ext = ".szx"; break;
      case LIBSPECTRUM_ID_TAPE_CSW:       *ext = ".csw"; break;
      case LIBSPECTRUM_ID_TAPE_Z80EM:     *ext = ".raw"; break;
      case LIBSPECTRUM_ID_TAPE_WAV:       *ext = ".wav"; break;
      case LIBSPECTRUM_ID_TAPE_SPC:       *ext = ".spc"; break;
      case LIBSPECTRUM_ID_TAPE_STA:       *ext = ".sta"; break;
      case LIBSPECTRUM_ID_TAPE_LTP:       *ext = ".ltp"; break;
      case LIBSPECTRUM_ID_TAPE_PZX:       *ext = ".pzx"; break;
      case LIBSPECTRUM_ID_DISK_SCL:       *ext = ".scl"; break;
      case LIBSPECTRUM_ID_DISK_TRD:       *ext = ".trd"; break;
      case LIBSPECTRUM_ID_DISK_DSK:
      case LIBSPECTRUM_ID_DISK_CPC:
      case LIBSPECTRUM_ID_DISK_ECPC:      *ext = ".dsk"; break;
      case LIBSPECTRUM_ID_CARTRIDGE_DCK:  *ext = ".dck"; break;
      default:                            *ext = "";     break;
   }

   return type;
}

// Parses an M3U playlist (one disk image path per line; blank lines and
// lines starting with '#' are skipped) into disk_image_paths[]. Relative
// entries are resolved against the M3U file's own directory, since that's
// the standard convention - RetroArch doesn't parse M3U content for the
// core, so any subsequent load must be able to find these paths on the
// real filesystem (see the direct-fopen fallback in compat_file_open()).
static int content_is_ipf(const void* data, size_t size)
{
   // IPF images open with the CAPS chunk id. libspectrum has no signature
   // for them, so libspectrum_identify_file() returns UNKNOWN and the TRD
   // fallback below claims them.
   return size >= 4 && memcmp(data, "CAPS", 4) == 0;
}

static void parse_m3u(const char* m3u_path, const char* data, size_t size)
{
   char base_dir[MAX_DISK_PATH_LEN];
   const char* slash1 = strrchr(m3u_path, '/');
   const char* slash2 = strrchr(m3u_path, '\\');
   const char* slash = slash1 > slash2 ? slash1 : slash2;
   size_t base_len = slash ? (size_t)(slash - m3u_path + 1) : 0;
   const char* p = data;
   const char* end = data + size;

   if (base_len >= sizeof(base_dir))
      base_len = sizeof(base_dir) - 1;
   memcpy(base_dir, m3u_path, base_len);
   base_dir[base_len] = 0;

   num_disk_images = 0;

   while (p < end && num_disk_images < MAX_DISK_IMAGES)
   {
      const char* line_start = p;
      const char* newline = memchr(p, '\n', end - p);
      size_t line_len = newline ? (size_t)(newline - line_start) : (size_t)(end - line_start);
      char entry[MAX_DISK_PATH_LEN];
      size_t copy_len;
      int is_absolute;

      p = newline ? newline + 1 : end;

      while (line_len > 0 && (line_start[line_len - 1] == '\r' ||
             line_start[line_len - 1] == ' ' || line_start[line_len - 1] == '\t'))
         line_len--;
      while (line_len > 0 && (*line_start == ' ' || *line_start == '\t'))
      {
         line_start++;
         line_len--;
      }

      if (line_len == 0 || line_start[0] == '#')
         continue;

      // A path that would truncate can never open - skip it outright
      // rather than storing a mangled path that fails later.
      if (line_len > sizeof(entry) - 1)
         continue;

      copy_len = line_len;
      memcpy(entry, line_start, copy_len);
      entry[copy_len] = 0;

      // "/foo", "\foo" or "C:\foo" style paths are already absolute
      is_absolute = entry[0] == '/' || entry[0] == '\\' ||
                    (copy_len > 1 && entry[1] == ':');

      if (is_absolute || base_len == 0)
      {
         memcpy(disk_image_paths[num_disk_images], entry, copy_len + 1);
      }
      else
      {
         if (base_len + copy_len > MAX_DISK_PATH_LEN - 1)
            continue;
         memcpy(disk_image_paths[num_disk_images], base_dir, base_len);
         memcpy(disk_image_paths[num_disk_images] + base_len, entry, copy_len + 1);
      }

      // Drop entries the core cannot load. utils_open_file() would fail on
      // an IPF anyway, but only after it had been counted as a disk and
      // registered with the frontend's disk control, leaving a playlist
      // slot that can never be inserted.
      {
         const char* path = disk_image_paths[num_disk_images];
         RFILE* fp = filestream_open(path, RETRO_VFS_FILE_ACCESS_READ,
                                     RETRO_VFS_FILE_ACCESS_HINT_NONE);
         if (fp)
         {
            char magic[4];
            int64_t got = filestream_read(fp, magic, sizeof(magic));
            filestream_close(fp);

            if (got == (int64_t)sizeof(magic) && content_is_ipf(magic, sizeof(magic)))
            {
               log_cb(RETRO_LOG_ERROR,
                      "Skipping unsupported IPF disk image: %s\n", path);
               continue;
            }
         }
      }

      num_disk_images++;
   }
}

// Mounts disk_image_paths[current_disk_index] into whichever disk drive is
// active for the current machine, without resetting it - used for runtime
// disk swaps (as opposed to the initial content load, which does respect
// the Tape Auto Load option and machine boot semantics).
static bool disk_control_insert_current(void)
{
   libspectrum_id_t type;
   int error;

   if (num_disk_images == 0 || current_disk_index >= num_disk_images)
      return true; // "no disk" is a valid state, not a failure

   fuse_emulation_pause();
   error = utils_open_file(disk_image_paths[current_disk_index], 0, &type);
   fuse_emulation_unpause();
   display_refresh_all();

   return error == 0;
}

// Ejects whatever's currently in the active disk drive. Mirrors the
// machine/peripheral dispatch utils_open_file() uses for its
// LIBSPECTRUM_CLASS_DISK_GENERIC case, since at eject time there's no file
// to identify a class from - only "whichever drive is live right now".
static void disk_control_eject_current(void)
{
   if (machine_current->machine == LIBSPECTRUM_MACHINE_PLUS3 ||
       machine_current->machine == LIBSPECTRUM_MACHINE_PLUS2A)
      ui_media_drive_eject(UI_MEDIA_CONTROLLER_PLUS3, SPECPLUS3_DRIVE_A);
   else if (machine_current->machine == LIBSPECTRUM_MACHINE_PENT    ||
            machine_current->machine == LIBSPECTRUM_MACHINE_PENT512 ||
            machine_current->machine == LIBSPECTRUM_MACHINE_PENT1024 ||
            machine_current->machine == LIBSPECTRUM_MACHINE_SCORP   ||
            periph_is_active(PERIPH_TYPE_BETA128))
      ui_media_drive_eject(UI_MEDIA_CONTROLLER_BETA, BETA_DRIVE_A);
   else if (periph_is_active(PERIPH_TYPE_DISCIPLE))
      ui_media_drive_eject(UI_MEDIA_CONTROLLER_DISCIPLE, DISCIPLE_DRIVE_1);
   else if (periph_is_active(PERIPH_TYPE_PLUSD))
      ui_media_drive_eject(UI_MEDIA_CONTROLLER_PLUSD, PLUSD_DRIVE_1);
   else if (periph_is_active(PERIPH_TYPE_OPUS))
      ui_media_drive_eject(UI_MEDIA_CONTROLLER_OPUS, OPUS_DRIVE_1);
   else if (periph_is_active(PERIPH_TYPE_DIDAKTIK80))
      ui_media_drive_eject(UI_MEDIA_CONTROLLER_DIDAKTIK, DIDAKTIK80_DRIVE_A);
}

static bool RETRO_CALLCONV disk_set_eject_state(bool ejected)
{
   if (ejected == disk_tray_ejected)
      return true;

   if (ejected)
   {
      disk_control_eject_current();
      disk_tray_ejected = true;
      return true;
   }

   disk_tray_ejected = false;
   return disk_control_insert_current();
}

static bool RETRO_CALLCONV disk_get_eject_state(void)
{
   return disk_tray_ejected;
}

static unsigned RETRO_CALLCONV disk_get_image_index(void)
{
   return current_disk_index;
}

static bool RETRO_CALLCONV disk_set_image_index(unsigned index)
{
   if (!disk_tray_ejected)
      return false;

   // An index >= num_disk_images means "no disk", which is valid.
   current_disk_index = index;
   return true;
}

static unsigned RETRO_CALLCONV disk_get_num_images(void)
{
   return num_disk_images;
}

static bool RETRO_CALLCONV disk_replace_image_index(unsigned index, const struct retro_game_info *info)
{
   if (!disk_tray_ejected || index >= num_disk_images)
      return false;

   if (!info)
   {
      // Remove this index, shifting later entries down.
      unsigned i;
      for (i = index; i + 1 < num_disk_images; i++)
         strncpy(disk_image_paths[i], disk_image_paths[i + 1], MAX_DISK_PATH_LEN);
      num_disk_images--;
      if (current_disk_index > index || current_disk_index >= num_disk_images)
         current_disk_index = current_disk_index > 0 ? current_disk_index - 1 : 0;
      return true;
   }

   if (!info->path)
      return false;

   strncpy(disk_image_paths[index], info->path, MAX_DISK_PATH_LEN - 1);
   disk_image_paths[index][MAX_DISK_PATH_LEN - 1] = 0;
   return true;
}

static bool RETRO_CALLCONV disk_add_image_index(void)
{
   if (num_disk_images >= MAX_DISK_IMAGES)
      return false;

   disk_image_paths[num_disk_images][0] = 0;
   num_disk_images++;
   return true;
}

static bool RETRO_CALLCONV disk_set_initial_image(unsigned index, const char *path)
{
   if (!path)
      return false;

   initial_disk_index_hint = (int)index;
   strncpy(initial_disk_path_hint, path, MAX_DISK_PATH_LEN - 1);
   initial_disk_path_hint[MAX_DISK_PATH_LEN - 1] = 0;
   return true;
}

static bool RETRO_CALLCONV disk_get_image_path(unsigned index, char *path, size_t len)
{
   if (index >= num_disk_images || disk_image_paths[index][0] == 0)
      return false;

   strncpy(path, disk_image_paths[index], len - 1);
   path[len - 1] = 0;
   return true;
}

static bool RETRO_CALLCONV disk_get_image_label(unsigned index, char *label, size_t len)
{
   const char *slash1, *slash2, *base;

   if (index >= num_disk_images || disk_image_paths[index][0] == 0)
      return false;

   slash1 = strrchr(disk_image_paths[index], '/');
   slash2 = strrchr(disk_image_paths[index], '\\');
   base = slash1 > slash2 ? slash1 + 1 : (slash2 ? slash2 + 1 : disk_image_paths[index]);

   strncpy(label, base, len - 1);
   label[len - 1] = 0;
   return true;
}

static struct retro_disk_control_ext_callback disk_control_ext_cb = {
   disk_set_eject_state,
   disk_get_eject_state,
   disk_get_image_index,
   disk_set_image_index,
   disk_get_num_images,
   disk_replace_image_index,
   disk_add_image_index,
   disk_set_initial_image,
   disk_get_image_path,
   disk_get_image_label,
};

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
   memset(joyp_state, 0, sizeof(joyp_state));
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

   /* Special case TS2068 .dck */
   /* We force TC2068 */
   if (info && info->size != 0)
   {
      const char *ext_f = strrchr(info->path, '.');
      if (ext_f != NULL && strcmp(ext_f, ".dck") == 0)
      {
         forced_machine_at_init = 1;
         /* LIBSPECTRUM_MACHINE_TS2068 position in machine_list */
         forced_machine_idx = 10;
      }
   }
   
   // A frontend may load new content without an intervening
   // retro_deinit(); retro_unload_game() releases the content buffers but
   // does not bring Fuse down. Do it here, or fuse_init() runs a second
   // time over a live machine and abandons everything the first one
   // allocated - a little over a megabyte per reload.
   if (fuse_init_called)
   {
      fuse_init_called = 0;
      fuse_end();
   }

   // Arm the teardown flag only once fuse_init() has reported success.
   // fuse_init() can bail out at any point - including before
   // run_startup_manager() has called startup_manager_init(), i.e. before
   // there is anything at all to shut down - and the frontend still calls
   // retro_deinit() after a failed retro_load_game(). Setting the flag
   // up front made that deinit run fuse_end() over a Fuse that was never
   // brought up.
   if (fuse_init(sizeof(argv) / sizeof(argv[0]), argv) == 0)
   {
      fuse_init_called = 1;

      if (info && info->size != 0)
      {
         tape_size = info->size;
         tape_data = malloc(tape_size);

         if (!tape_data)
         {
            log_cb(RETRO_LOG_ERROR, "Could not allocate memory for the tape\n");
            // Disarm before ending, or retro_deinit() ends Fuse a second
            // time.
            fuse_init_called = 0;
            fuse_end();
            return false;
         }

         memcpy(tape_data, info->data, tape_size);

         // The TRD fallback in identify_file() claims anything libspectrum
         // cannot name, so an IPF was being loaded as a raw TRD: the core
         // reported success, switched to a Scorpion, failed to find its
         // ROMs and left the user at a 48K BASIC prompt. Say so instead.
         if (content_is_ipf(tape_data, tape_size))
         {
            log_cb(RETRO_LOG_ERROR,
                   "IPF disk images are not supported by this core\n");
            free(tape_data);
            tape_data = NULL;
            tape_size = 0;
            fuse_init_called = 0;
            fuse_end();
            return false;
         }

         const char* filename_load_game = info->path;
         int is_m3u = 0;

         // info->path may legitimately be NULL with need_fullpath=false;
         // M3U content (and single-disk registration below) needs a real
         // path, everything else falls back to content-based identify.
         if (filename_load_game != NULL)
         {
            const char* m3u_ext = strrchr(filename_load_game, '.');
            // Case-insensitive: frontends and users pass ".M3U" too, and
            // falling through to identify would misread the playlist text
            // as a raw disk image.
            is_m3u = m3u_ext != NULL &&
                     (m3u_ext[1] == 'm' || m3u_ext[1] == 'M') &&
                      m3u_ext[2] == '3' &&
                     (m3u_ext[3] == 'u' || m3u_ext[3] == 'U') &&
                      m3u_ext[4] == 0;
         }

         num_disk_images = 0;
         current_disk_index = 0;
         disk_tray_ejected = false;
         content_is_m3u = is_m3u;

         if (is_m3u)
         {
            // An M3U's own bytes were never a disk image - parse it into
            // the disk list instead of running it through the normal
            // identify/load path (which would otherwise mistake the M3U
            // text for a raw TRD image, see identify_file()'s fallback).
            parse_m3u(filename_load_game, (const char*)tape_data, tape_size);

            if (initial_disk_index_hint >= 0 &&
                (unsigned)initial_disk_index_hint < num_disk_images &&
                strcmp(disk_image_paths[initial_disk_index_hint], initial_disk_path_hint) == 0)
            {
               current_disk_index = (unsigned)initial_disk_index_hint;
            }

            if (num_disk_images > 0)
            {
               libspectrum_id_t type;

               fuse_emulation_pause();
               utils_open_file(disk_image_paths[current_disk_index], settings_current.auto_load, &type);
               display_refresh_all();
               fuse_emulation_unpause();
            }
         }
         else
         {
            const char* ext;
            libspectrum_id_t type;
            libspectrum_class_t class;

            if (forced_machine_at_init && forced_machine_idx == 10)  /* LIBSPECTRUM_MACHINE_TS2068 position in machine_list */
            {
               type = LIBSPECTRUM_ID_CARTRIDGE_DCK;
               class = LIBSPECTRUM_CLASS_CARTRIDGE_TIMEX;
               ext = ".dck";
            }
            else
            {
               type = identify_file_get_ext(filename_load_game, tape_data, tape_size, &ext);
               libspectrum_identify_class(&class, type);
            }

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

            // Single disk image: still expose it through Disk Control, so
            // a second disk can be added manually from the frontend's UI
            // even without an M3U.
            if (filename_load_game != NULL &&
               (class == LIBSPECTRUM_CLASS_DISK_PLUS3   || class == LIBSPECTRUM_CLASS_DISK_DIDAKTIK ||
                class == LIBSPECTRUM_CLASS_DISK_PLUSD   || class == LIBSPECTRUM_CLASS_DISK_TRDOS     ||
                class == LIBSPECTRUM_CLASS_DISK_OPUS    || class == LIBSPECTRUM_CLASS_DISK_GENERIC))
            {
               strncpy(disk_image_paths[0], filename_load_game, MAX_DISK_PATH_LEN - 1);
               disk_image_paths[0][MAX_DISK_PATH_LEN - 1] = 0;
               num_disk_images = 1;
            }
         }
      }
      else
      {
         // Load the _BASIC.z80 content to boot to BASIC
         tape_data = NULL;
         tape_size = 0;
         num_disk_images = 0;
         current_disk_index = 0;
         disk_tray_ejected = false;
         content_is_m3u = 0;
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

      // Re-apply the live controller-port wiring: if a frontend assigned
      // RETRO_DEVICE_KEMPSTON_MOUSE before retro_load_game(), fuse_init()'s
      // settings_defaults() has just wiped settings_current.kempston_mouse
      // (and the deferral flag was never armed because fuse_init_called was
      // still 0); likewise, loading snapshot content re-derives the flag
      // from the file via kempmouse_snapshot_enabled(). Port wiring is
      // frontend state, not machine state - see retro_unserialize().
      sync_kempston_mouse_from_ports();

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
   // Report the currently visible output geometry.
   info->geometry.base_width = soft_width;
   info->geometry.base_height = soft_height;

   info->geometry.max_width = MAX_WIDTH;
   info->geometry.max_height = MAX_HEIGHT;
   info->geometry.aspect_ratio = 0.0f;
   info->timing.fps = machine_id_is_60hz(machine->id) ? 60.0 : 50.0;
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
            const uint16_t* src2 = image_buffer + (48 * hard_width); // Centre doubled 480px overlay in 576px canvas
            uint16_t* dest = image_buffer_2 + (48 * hard_width);     // Centre doubled 480px overlay in 576px canvas
            int x, y;

            if (keyb_transparent)
            {
               for (y = 0; y < 240; y++) // Process only 240px height
               {
                  for (x = 0; x < 320; x++)
                  {
                     uint32_t src1_pixel = (*src1++ & 0xe79c) * 3;

                     dest[0] = (src1_pixel + (src2[0] & 0xe79c)) >> 2;
                     dest[1] = (src1_pixel + (src2[1] & 0xe79c)) >> 2;
                     dest[640] = (src1_pixel + (src2[640] & 0xe79c)) >> 2;
                     dest[641] = (src1_pixel + (src2[641] & 0xe79c)) >> 2;

                     src2 += 2;
                     dest += 2;
                  }

                  src2 += 640;
                  dest += 640;
               }
            }
            else
            {
               for (y = 0; y < 240; y++) // Process only 240px height
               {
                  for (x = 0; x < 320; x++)
                  {
                     uint32_t src1_pixel = *src1++;

                     dest[0] = src1_pixel;
                     dest[1] = src1_pixel;
                     dest[640] = src1_pixel;
                     dest[641] = src1_pixel;

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
               const uint16_t* src2 = image_buffer + (24 * hard_width); // Offset by 24px
               const uint16_t* end = src1 + (240 * 320);                // Limit to 240px height
               uint16_t* dest = image_buffer_2 + (24 * hard_width);    // Offset by 24px

               while (src1 < end)
               {
                  uint32_t src1_pixel = *src1++ & 0xe79c;
                  uint32_t src2_pixel = *src2++ & 0xe79c;

                  *dest++ = (src1_pixel * 3 + src2_pixel) >> 2;
               }
            }
            else
            {
               memcpy(image_buffer_2 + (24 * hard_width), keyboard_overlay, 240 * 320 * sizeof(uint16_t)); // Copy to offset position
            }
         }

         // Render virtual keyboard highlighting
         unsigned x = keyb_positions[keyb_y].x + keyb_x * 24;
         unsigned y = keyb_positions[keyb_y].y + 24; // Offset highlighting by 24px
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
         uint16_t* pixel = image_buffer_2 + ((y * hard_width) + x + 1) * mult;
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

   if (kempston_mouse_needs_periph_update)
   {
      kempston_mouse_needs_periph_update = 0;
      periph_update();
   }

   if (display_joystick_type == TRUE)
   {
      int port;
      for (port = 0; port < MAX_PADS; port++) {
         int joystick_type = get_joystick(input_devices[port]);
         if (joystick_type != 0) {
            char title[80];
            snprintf(title, sizeof(title), "Port %d set as %s joystick", port + 1,
               libspectrum_joystick_name(joystick_type));
            Retro_Msg(title);
         }
      }
      display_joystick_type = FALSE;
   }

   if (display_emulation_speed == TRUE) {
      char title[80];
      snprintf(title, sizeof(title), "Emulation speed set to %d%%", settings_current.emulation_speed);
      Retro_Msg(title);
      display_emulation_speed = FALSE;
   }

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
   /* Poll exactly once per retro_run(), before stepping the machine.
      The loop below runs until this frame's audio has been emitted, which
      takes several event batches, so polling inside it called
      input_poll_cb() ~4 times per frame (and 15+ on some frames). The
      libretro contract is one poll per frame - the frontend latches input
      state on poll and serves it through input_state_cb() - and frontends
      that record, replay or re-run frames (netplay, runahead, BSV replay)
      consume one input sample per poll, so extra polls desync them.
      Reading state inside the loop via input_state_cb() remains fine. */
   input_poll_cb();

   /* Bounded wait. some_audio is set by sound_lowlevel_frame(), reached only
      via sound_frame(), which returns early whenever sound_enabled is clear
      - sound_init() failed, the emulation speed sits outside the supported
      range, or the machine was left paused. An unbounded wait then freezes
      the frontend with no way out short of killing it, which this core has
      already hit twice (a machine_reset() during content load leaves the
      frame interrupt unscheduled). Typical frames need ~4 iterations and
      the worst observed is ~15, so this ceiling is several orders of
      magnitude of headroom; reaching it means something is wrong, and
      emitting a frame without audio degrades far better than hanging. */
   {
      int guard = 10000;

      do {
         z80_do_opcodes();
         event_do_events();
      }
      while (!some_audio && --guard > 0);

      if (!some_audio)
      {
         static int warned_no_audio = 0;

         if (!warned_no_audio)
         {
            warned_no_audio = 1;
            log_cb(RETRO_LOG_WARN,
                   "retro_run: no audio produced within %d iterations; "
                   "continuing without it (sound_enabled is probably clear)\n",
                   10000);
         }
      }
   }

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

// Kempston Mouse is a single peripheral, not per-port; enable it in Fuse
// whenever any port is currently configured as a mouse, disable it
// otherwise. This reflects live frontend port wiring, not emulated
// machine state, so it must be re-applied after a snapshot/rewind restore
// too - the snapshot's own kempston_mouse_active flag only reflects
// whatever was true at the moment that particular state was captured
// (e.g. still off, if rewound back to before the mouse was ever
// connected), which can otherwise leave the peripheral disabled even
// though the frontend still has a mouse wired to a port right now.
static void sync_kempston_mouse_from_ports(void)
{
   unsigned p;
   int kempston_mouse = 0;

   for (p = 0; p < MAX_PADS; p++)
   {
      if (input_devices[p] == RETRO_DEVICE_KEMPSTON_MOUSE)
      {
         kempston_mouse = 1;
         break;
      }
   }

   if (settings_current.kempston_mouse != kempston_mouse)
   {
      settings_current.kempston_mouse = kempston_mouse;

      // See the comment in retro_set_controller_port_device(): defer the
      // actual periph_update() to the top of retro_run() rather than
      // calling it here synchronously.
      if (fuse_init_called)
         kempston_mouse_needs_periph_update = 1;
   }
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   log_cb(RETRO_LOG_INFO, "port %u device %08x\n", port, device);

   if (device == RETRO_DEVICE_AUTO_CFG)
   {
      if (port == 0 && show_joystick_type_at_startup == TRUE)
         display_joystick_type = TRUE;
      return;
   }

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

   // Flipping settings_current.kempston_mouse alone does nothing: Fuse
   // only (de)registers a peripheral's I/O ports when periph_update() runs
   // (normally done by each machine's own init(), on snapshot load, etc;
   // there is no automatic hook for "an option changed while already
   // running"). Before fuse_init() has run there is no machine/peripheral
   // table yet to update - retro_init()'s default controller setup would
   // crash here.
   //
   // Do NOT call periph_update()/machine_reset() synchronously here:
   // frontends (RetroArch included) apply saved port/remap config right
   // after retro_load_game() returns, before the first retro_run().
   // Resetting the machine at that point - before Fuse's event queue has
   // processed a single frame - leaves the next frame interrupt event
   // unscheduled, so retro_run()'s "wait until some_audio" loop spins
   // forever and the frontend hangs. Defer to the top of retro_run()
   // instead, exactly like UPDATE_MACHINE (model changes) already does.
   sync_kempston_mouse_from_ports();
}

void retro_reset(void)
{
   const char* ext;
   libspectrum_id_t type;
   char filename[32];

   if (content_is_m3u)
   {
      // tape_data holds the M3U's playlist text, not an image -
      // re-identifying it here would misread it (possibly as a raw TRD).
      // Reset means reboot with the currently selected disk inserted,
      // mirroring the initial load; autoload forced like the normal
      // reset path below.
      if (num_disk_images > 0 && current_disk_index < num_disk_images)
      {
         fuse_emulation_pause();
         utils_open_file(disk_image_paths[current_disk_index], 1, &type);
         display_refresh_all();
         fuse_emulation_unpause();
         disk_tray_ejected = false;
      }
      else
      {
         machine_reset(1);
      }

      sync_kempston_mouse_from_ports();
      return;
   }

   type = identify_file_get_ext(NULL, tape_data, tape_size, &ext);

   snprintf(filename, sizeof(filename), "*%s", ext);
   filename[sizeof(filename) - 1] = 0;

   fuse_emulation_pause();
   utils_open_file(filename, 1, &type);
   display_refresh_all();
   fuse_emulation_unpause();

   // If the content is a snapshot, utils_open_file() went through
   // snapshot_copy_from() and re-derived settings_current.kempston_mouse
   // from the file, exactly like retro_unserialize() - re-apply the live
   // controller-port wiring here too.
   sync_kempston_mouse_from_ports();
}

size_t retro_serialize_size(void)
{
   if (auto_size_savestate) {
      if (snapshot_update())
      {
         log_cb(RETRO_LOG_ERROR, "Could not build snapshot\n");
         return 0;
      }
      return snapshot_size;
   }
   else
   {
      /* Hack to keep all snapshots with a fixed size (double of normal snapshot size)*/
      if (machine->id == LIBSPECTRUM_MACHINE_48       ||
         machine->id == LIBSPECTRUM_MACHINE_48_NTSC   ||
         machine->id == LIBSPECTRUM_MACHINE_TC2048    ||
         machine->id == LIBSPECTRUM_MACHINE_16)
         return 2 * 64 * 1024;
      else if (machine->id == LIBSPECTRUM_MACHINE_128 ||
         machine->id == LIBSPECTRUM_MACHINE_PLUS2     ||
         machine->id == LIBSPECTRUM_MACHINE_PLUS2A    ||
         machine->id == LIBSPECTRUM_MACHINE_PLUS3     ||
         machine->id == LIBSPECTRUM_MACHINE_PLUS3E    ||
         machine->id == LIBSPECTRUM_MACHINE_TC2068    ||
         machine->id == LIBSPECTRUM_MACHINE_TS2068    ||
         machine->id == LIBSPECTRUM_MACHINE_TS2068    ||
         machine->id == LIBSPECTRUM_MACHINE_SE        ||
         machine->id == LIBSPECTRUM_MACHINE_PENT)
         return 2 * 128 * 1024;
      else if (machine->id == LIBSPECTRUM_MACHINE_SCORP)
         return 2 * 256 * 1024;
      else if (machine->id == LIBSPECTRUM_MACHINE_PENT512)
         return 2 * 512 * 1024;
      else
         return 2 * 1024 * 1024;
   }
}

bool retro_serialize(void *data, size_t size)
{
   // snapshot_update() frees the buffer and zeroes the size when it cannot
   // allocate or the SZX write fails. Ignoring that produced a state that
   // was nothing but 0xFF padding and still reported success.
   if (snapshot_update())
   {
      log_cb(RETRO_LOG_ERROR, "Could not build snapshot\n");
      return false;
   }

   if (auto_size_savestate)
   {
      if (size < snapshot_size)
      {
         log_cb(RETRO_LOG_WARN, "Data size is not enough for snapshot\n");
         return false;
      }
      memcpy(data, snapshot_buffer, snapshot_size);
      // Frontends (rewind in particular) keep passing the size reported by
      // the first retro_serialize_size() call even after the SZX shrinks;
      // pad like the fixed-size path below so the tail is well-formed 0xFF
      // instead of stale ring-buffer bytes that would parse as garbage
      // chunks on unserialize.
      memset(data + snapshot_size, 0xFF, size - snapshot_size);
      return true;
   }

   if (size < snapshot_size)
   {
      log_cb(RETRO_LOG_WARN, "Snapshot size is larger than fixed size\n");
      return false;
   }
   memcpy(data, snapshot_buffer, snapshot_size);
   memset(data + snapshot_size, 0xFF, size - snapshot_size);
   return true;
}

bool retro_unserialize(const void *data, size_t size)
{
   bool ok = snapshot_read_buffer(data, size, LIBSPECTRUM_ID_SNAPSHOT_SZX) == 0;

   // Loading a snapshot re-derives settings_current.kempston_mouse from
   // whatever was true when that particular state was captured (see
   // kempmouse_snapshot_enabled() in fuse/peripherals/kempmouse.c) - e.g.
   // still off, if rewound back to before the mouse was ever connected.
   // Port wiring is live frontend state, not emulated machine state, so
   // re-sync it from the actual current controller assignment right after
   // every restore (rewind or manual load) rather than leaving whatever
   // the snapshot happened to say.
   if (ok)
      sync_kempston_mouse_from_ports();

   return ok;
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
   tape_data = NULL;
   tape_size = 0;
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
