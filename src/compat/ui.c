// Compatibility UI functions

#include <libretro.h>
#include <externs.h>
#include <input.h>
#include <ui/ui.h>

enum keyb_states {
   CAPS_SHIFT_PRESSED = 0x01,
   SYMBOL_SHIFT_PRESSED = 0x02
};

static int64_t get_time_usec()
{
   return (int64_t)( total_time_ms * 1000.0 );
}

int ui_init(int *argc, char ***argv)
{
   (void)argc;
   (void)argv;
   return 0;
}

static input_key translate(unsigned index, int port, bool *keyboard_event)
{
   *keyboard_event = (index == RETRO_DEVICE_ID_JOYPAD_L || index == RETRO_DEVICE_ID_JOYPAD_R);
   // if there's a keyboard mapping override joystick input if player 1 only
   if (index < (sizeof(joymap) / sizeof(joymap[0])) && port == 0 && joymap[index] != INPUT_KEY_NONE) {
      *keyboard_event = true;
      return joymap[index];
   }

   switch (index)
   {
      case RETRO_DEVICE_ID_JOYPAD_UP:    return INPUT_JOYSTICK_UP;
      case RETRO_DEVICE_ID_JOYPAD_DOWN:  return INPUT_JOYSTICK_DOWN;
      case RETRO_DEVICE_ID_JOYPAD_LEFT:  return INPUT_JOYSTICK_LEFT;
      case RETRO_DEVICE_ID_JOYPAD_RIGHT: return INPUT_JOYSTICK_RIGHT;
      case RETRO_DEVICE_ID_JOYPAD_A:
      case RETRO_DEVICE_ID_JOYPAD_X:
      case RETRO_DEVICE_ID_JOYPAD_Y:     return INPUT_JOYSTICK_FIRE_1;
      case RETRO_DEVICE_ID_JOYPAD_B:     return INPUT_JOYSTICK_UP;
      case RETRO_DEVICE_ID_JOYPAD_L:     return INPUT_KEY_Return;
      case RETRO_DEVICE_ID_JOYPAD_R:     return INPUT_KEY_space;
   }

   return INPUT_KEY_NONE;
}

int ui_event(void)
{
   static const unsigned map[] = {
      RETRO_DEVICE_ID_JOYPAD_UP,
      RETRO_DEVICE_ID_JOYPAD_DOWN,
      RETRO_DEVICE_ID_JOYPAD_LEFT,
      RETRO_DEVICE_ID_JOYPAD_RIGHT,
      RETRO_DEVICE_ID_JOYPAD_A,
      RETRO_DEVICE_ID_JOYPAD_B,
      RETRO_DEVICE_ID_JOYPAD_X,
      RETRO_DEVICE_ID_JOYPAD_Y,
      RETRO_DEVICE_ID_JOYPAD_L,
      RETRO_DEVICE_ID_JOYPAD_R,
      RETRO_DEVICE_ID_JOYPAD_L2,
      RETRO_DEVICE_ID_JOYPAD_R2,
      RETRO_DEVICE_ID_JOYPAD_L3,
      RETRO_DEVICE_ID_JOYPAD_R3,
      RETRO_DEVICE_ID_JOYPAD_START
   };
   
   static const input_key keyb_layout[4][10] = {
      {
         INPUT_KEY_1, INPUT_KEY_2, INPUT_KEY_3, INPUT_KEY_4, INPUT_KEY_5,
         INPUT_KEY_6, INPUT_KEY_7, INPUT_KEY_8, INPUT_KEY_9, INPUT_KEY_0
      },
      {
         INPUT_KEY_q, INPUT_KEY_w, INPUT_KEY_e, INPUT_KEY_r, INPUT_KEY_t,
         INPUT_KEY_y, INPUT_KEY_u, INPUT_KEY_i, INPUT_KEY_o, INPUT_KEY_p
      },
      {
         INPUT_KEY_a, INPUT_KEY_s, INPUT_KEY_d, INPUT_KEY_f, INPUT_KEY_g,
         INPUT_KEY_h, INPUT_KEY_j, INPUT_KEY_k, INPUT_KEY_l, INPUT_KEY_Return
      },
      {
         INPUT_KEY_Shift_L, INPUT_KEY_z, INPUT_KEY_x, INPUT_KEY_c, INPUT_KEY_v,
         INPUT_KEY_b, INPUT_KEY_n, INPUT_KEY_m, INPUT_KEY_Control_R, INPUT_KEY_space
      }
   };

   if (keyb_send != 0 && get_time_usec() >= keyb_send)
   {
       keyb_event.type = INPUT_EVENT_KEYRELEASE;
       input_event(&keyb_event);
       // Also release shift keys
       input_event_t shift_event;
       shift_event.type = INPUT_EVENT_KEYRELEASE;
       shift_event.types.key.native_key = INPUT_KEY_Shift_L;
       shift_event.types.key.spectrum_key = INPUT_KEY_Shift_L;
       input_event(&shift_event);
       shift_event.types.key.native_key = INPUT_KEY_Control_R;
       shift_event.types.key.spectrum_key = INPUT_KEY_Control_R;
       input_event(&shift_event);
       keyb_send = 0;
   }

   unsigned port;
   int16_t is_down = 0;
   
   for (port = 0; port < MAX_PADS; port++)
   {
      unsigned device = input_devices[port];
      
      switch (device)
      {
         case RETRO_DEVICE_CURSOR_JOYSTICK:
         case RETRO_DEVICE_KEMPSTON_JOYSTICK:
         case RETRO_DEVICE_SINCLAIR1_JOYSTICK:
         case RETRO_DEVICE_SINCLAIR2_JOYSTICK:
         case RETRO_DEVICE_TIMEX1_JOYSTICK:
         case RETRO_DEVICE_TIMEX2_JOYSTICK:
         case RETRO_DEVICE_FULLER_JOYSTICK:
            is_down |= input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT);
      }
   }
   
   if (is_down)
   {
      if (!select_pressed)
      {
         select_pressed = true;
         keyb_overlay = !keyb_overlay;
      }
   }
   else
   {
      select_pressed = false;
   }

   // Kempston Mouse: a single peripheral, read from whichever port (if any)
   // is configured as RETRO_DEVICE_KEMPSTON_MOUSE. Runs regardless of
   // keyb_overlay state since it doesn't share the joypad with the overlay
   // navigation. input_state_cb() itself is still queried with the base
   // RETRO_DEVICE_MOUSE, not the subclass - same as joystick polling below
   // uses plain RETRO_DEVICE_JOYPAD, never one of the *_JOYSTICK subclasses.
   for (port = 0; port < MAX_PADS; port++)
   {
      if (input_devices[port] == RETRO_DEVICE_KEMPSTON_MOUSE)
      {
         int16_t dx = input_state_cb(port, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
         int16_t dy = input_state_cb(port, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);

         if (dx || dy)
            ui_mouse_motion(dx, dy);

         ui_mouse_button(1, input_state_cb(port, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT));
         ui_mouse_button(3, input_state_cb(port, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT));
      }
   }

   if (!keyb_overlay)
   {
      unsigned id;
      input_event_t fuse_event;
      input_key button;
      
      for (port = 0; port < MAX_PADS; port++)
      {
         unsigned device = input_devices[port];
         int is_joystick = 0;
         
         switch (device)
         {
            case RETRO_DEVICE_CURSOR_JOYSTICK:
            case RETRO_DEVICE_KEMPSTON_JOYSTICK:
            case RETRO_DEVICE_SINCLAIR1_JOYSTICK:
            case RETRO_DEVICE_SINCLAIR2_JOYSTICK:
            case RETRO_DEVICE_TIMEX1_JOYSTICK:
            case RETRO_DEVICE_TIMEX2_JOYSTICK:
            case RETRO_DEVICE_FULLER_JOYSTICK:
               is_joystick = 1;
         }
         
         if (is_joystick)
         {
            for (id = 0; id < sizeof(map) / sizeof(map[0]); id++)
            {
               is_down = input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, map[id]);
               
               bool keyboard_event;
               button = translate(map[id], port, &keyboard_event);

               if (is_down)
               {
                  if (!joyp_state[port][id])
                  {
                     joyp_state[port][id] = true;

                     if (keyboard_event)
                     {
                        fuse_event.type = INPUT_EVENT_KEYPRESS;
                        fuse_event.types.key.native_key = button;
                        fuse_event.types.key.spectrum_key = button;
                        
                        input_event(&fuse_event);
                     }
                     else if (button != INPUT_KEY_NONE)
                     {
                        fuse_event.type = INPUT_EVENT_JOYSTICK_PRESS;
                        fuse_event.types.joystick.which = port;
                        fuse_event.types.joystick.button = button;
                 
                        input_event(&fuse_event);
                     }
                  }
               }
               else
               {
                  if (joyp_state[port][id])
                  {
                     joyp_state[port][id] = false;
                 
                     if (keyboard_event)
                     {
                        fuse_event.type = INPUT_EVENT_KEYRELEASE;
                        fuse_event.types.key.native_key = button;
                        fuse_event.types.key.spectrum_key = button;
                        
                        input_event(&fuse_event);
                     }
                     else if (button != INPUT_KEY_NONE)
                     {
                        fuse_event.type = INPUT_EVENT_JOYSTICK_RELEASE;
                        fuse_event.types.joystick.which = port;
                        fuse_event.types.joystick.button = button;
                 
                        input_event(&fuse_event);
                     }
                  }
               }
            }
         }
      }
      
      // The keyboard belongs to the machine, not to a controller port, so
      // read it one time per frame from a single source.
      unsigned kb_port = 0;

      for (port = 0; port < MAX_PADS; port++)
      {
         if (input_devices[port] == RETRO_DEVICE_SPECTRUM_KEYBOARD)
         {
            kb_port = port;
            break;
         }
      }

      for (id = 0; keysyms_map[id].ui; id++)
      {
         unsigned ui = keysyms_map[id].ui;
         is_down = kb_cb_active ? kb_cb_state[ui]
                                : input_state_cb(kb_port, RETRO_DEVICE_KEYBOARD, 0, ui);

         if (is_down)
         {
            if (!keyb_state[ui])
            {
               keyb_state[ui] = true;

               fuse_event.type = INPUT_EVENT_KEYPRESS;
               fuse_event.types.key.native_key = keysyms_map[id].fuse;
               fuse_event.types.key.spectrum_key = keysyms_map[id].fuse;

               input_event(&fuse_event);
            }
         }
         else
         {
            if (keyb_state[ui])
            {
               keyb_state[ui] = false;

               fuse_event.type = INPUT_EVENT_KEYRELEASE;
               fuse_event.types.key.native_key = keysyms_map[id].fuse;
               fuse_event.types.key.spectrum_key = keysyms_map[id].fuse;

               input_event(&fuse_event);
            }
         }
      }
   }
   else
   {
      unsigned port, id;
      
      for (port = 0; port < MAX_PADS; port++)
      {
         unsigned device = input_devices[port] & RETRO_DEVICE_MASK;
         
         if (device == RETRO_DEVICE_JOYPAD)
         {
            for (id = 0; id < sizeof(map) / sizeof(map[0]); id++)
            {
               is_down = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, map[id]);
               
               if (is_down)
               {
                  if (!joyp_state[0][id])
                  {
                     joyp_state[0][id] = true;
                     
                     switch (map[id])
                     {
                        case RETRO_DEVICE_ID_JOYPAD_UP:    keyb_y = (keyb_y - 1) & 3; break;
                        case RETRO_DEVICE_ID_JOYPAD_DOWN:  keyb_y = (keyb_y + 1) & 3; break;
                        case RETRO_DEVICE_ID_JOYPAD_LEFT:  keyb_x = keyb_x == 0 ? 9 : keyb_x - 1; break;
                        case RETRO_DEVICE_ID_JOYPAD_RIGHT: keyb_x = keyb_x == 9 ? 0 : keyb_x + 1; break;
                        case RETRO_DEVICE_ID_JOYPAD_A:
                           if (keyb_send == 0)
                           {
                              keyb_event.type = INPUT_EVENT_KEYPRESS;
                              keyb_event.types.key.native_key = keyb_layout[keyb_y][keyb_x];
                              keyb_event.types.key.spectrum_key = keyb_layout[keyb_y][keyb_x];
                              input_event(&keyb_event);

                              switch (keyb_event.types.key.spectrum_key)
                              {
                              case INPUT_KEY_Shift_L:
                                 keyb_shift |= CAPS_SHIFT_PRESSED;
                                 break;
                              case INPUT_KEY_Control_R:
                                 keyb_shift |= SYMBOL_SHIFT_PRESSED;
                                 break;
                              default:
                                 keyb_shift = 0;
                                 break;
                              }

                              if ((keyb_shift == (CAPS_SHIFT_PRESSED | SYMBOL_SHIFT_PRESSED)) || !keyb_shift)
                              {
                                 keyb_send = get_time_usec() + keyb_hold_time;
                                 keyb_overlay = false;
                                 keyb_shift = 0;
                              }
                           }
                           return 0;
                     }
                  }
               }
               else
               {
                  if (joyp_state[0][id])
                  {
                     joyp_state[0][id] = false;
                  }
               }
            }
         }
      }
   }
   
   return 0;
}

int ui_error_specific(ui_error_level severity, const char *message)
{
   switch (severity)
   {
   case UI_ERROR_INFO:    log_cb(RETRO_LOG_INFO, "%s\n", message); break;
   case UI_ERROR_WARNING: log_cb(RETRO_LOG_WARN, "%s\n", message); break;
   case UI_ERROR_ERROR:   log_cb(RETRO_LOG_ERROR, "%s\n", message); break;
   }
  
   return 0;
}

/* The rest of Fuse's ui_* surface, stubbed. Fuse's emulation core calls
   these to keep a desktop frontend's chrome in sync - statusbar icons,
   menu item enable/disable, the tape browser, file dialogs, the
   debugger's windows. This core has none of that chrome: the frontend
   owns the UI and the core options own the configuration, so every one
   of these is a report into the void or a question with only one
   answer. They lived in fuse/ui/widget/ before the widget UI was
   removed - where they were equally inert, since widget_do() bailed
   out on display_ui_initialised, which no code in this build ever
   sets. */

int ui_statusbar_update(ui_statusbar_item item, ui_statusbar_state state)
{
   (void)item;
   (void)state;
   return 0;
}

int ui_statusbar_update_speed(float speed)
{
   (void)speed;
   return 0;
}

int ui_menu_item_set_active(const char *path, int active)
{
   (void)path;
   (void)active;
   return 0;
}

int ui_tape_browser_update(ui_tape_browser_update_type change,
                           libspectrum_tape_block *block)
{
   (void)change;
   (void)block;
   return 0;
}

/* NULL is the "user cancelled" answer for both file dialogs: the
   callers (tape write, +3 disk save, ui_mdr_write) all treat it as
   "give up quietly", which is right for a core that cannot prompt. */
char *ui_get_open_filename(const char *title)
{
   (void)title;
   return NULL;
}

char *ui_get_save_filename(const char *title)
{
   (void)title;
   return NULL;
}

int ui_query(const char *message)
{
   (void)message;
   return 0;
}

/* Media with unsaved changes is ejected without prompting, matching what
   the previous widget implementation resolved to with confirm_actions
   off. Data loss on eject is the frontend's disk-control contract. */
ui_confirm_save_t ui_confirm_save_specific(const char *message)
{
   (void)message;
   return UI_CONFIRM_SAVE_DONTSAVE;
}

/* Identical decision to the widget implementation with joy_prompt off,
   which is this core's only configuration: a snapshot recording a
   joystick maps it onto joystick 1 and everything else is left alone. */
ui_confirm_joystick_t ui_confirm_joystick(libspectrum_joystick libspectrum_type,
                                          int inputs)
{
   (void)libspectrum_type;

   if (inputs & LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_1)
      return UI_CONFIRM_JOYSTICK_JOYSTICK_1;

   return UI_CONFIRM_JOYSTICK_NONE;
}

int ui_widgets_reset(void)
{
   return 0;
}

void ui_pokemem_selector(const char *filename)
{
   (void)filename;
}

/* -1 is "no rollback point chosen"; rzx.c abandons the rollback. */
int ui_get_rollback_point(GSList *points)
{
   (void)points;
   return -1;
}

void ui_breakpoints_updated(void)
{
}

int ui_debugger_activate(void)
{
   return 0;
}

int ui_debugger_deactivate(int interruptable)
{
   (void)interruptable;
   return 0;
}

int ui_debugger_update(void)
{
   return 0;
}

int ui_debugger_disassemble(libspectrum_word address)
{
   (void)address;
   return 0;
}

int ui_end(void)
{
   return 0;
}
