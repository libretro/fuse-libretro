// Compatibility UI functions

#include <libretro.h>
#include <externs.h>
#include <input.h>
#include <ui/ui.h>

extern struct retro_perf_callback perf_cb;
extern int64_t keyb_send;
extern input_event_t keyb_event;
extern int select_pressed;
extern int keyb_overlay;

int ui_init(int *argc, char ***argv)
{
   (void)argc;
   (void)argv;
   return 0;
}

static input_key translate(unsigned index)
{
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
      RETRO_DEVICE_ID_JOYPAD_R
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
         INPUT_KEY_b, INPUT_KEY_n, INPUT_KEY_m, INPUT_KEY_Shift_R, INPUT_KEY_space
      }
   };

   //log_cb( RETRO_LOG_DEBUG, "%s\n", __FUNCTION__ );
   
   if (keyb_send != 0 && perf_cb.get_time_usec() >= keyb_send)
   {
       keyb_event.type = INPUT_EVENT_KEYRELEASE;
       input_event(&keyb_event);
       keyb_send = 0;
   }

   int16_t is_down = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT);
   
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
   
   if (!keyb_overlay)
   {
      unsigned port, id;
      input_event_t fuse_event;
      
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
               
               if (is_down)
               {
                  if (!joypad_state[port][id])
                  {
                     joypad_state[port][id] = true;
                     input_key button = translate(map[id]);
                 
                     if (button == INPUT_KEY_Return || button == INPUT_KEY_space)
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
                  if (joypad_state[port][id])
                  {
                     joypad_state[port][id] = false;
                     input_key button = translate(map[id]);
                 
                     if (button == INPUT_KEY_Return || button == INPUT_KEY_space)
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
      
      for (port = 0; port < MAX_PADS; port++)
      {
         unsigned device = input_devices[port];
         
         if (device == RETRO_DEVICE_SPECTRUM_KEYBOARD)
         {
            for (id = 0; keysyms_map[id].ui; id++)
            {
               unsigned ui = keysyms_map[id].ui;
               is_down = input_state_cb(port, RETRO_DEVICE_KEYBOARD, 0, ui);
               
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
                  if (!joypad_state[0][id])
                  {
                     joypad_state[0][id] = true;
                     
                     switch (map[id])
                     {
                        case RETRO_DEVICE_ID_JOYPAD_UP:    keyb_y = (keyb_y - 1) & 3; break;
                        case RETRO_DEVICE_ID_JOYPAD_DOWN:  keyb_y = (keyb_y + 1) & 3; break;
                        case RETRO_DEVICE_ID_JOYPAD_LEFT:  keyb_x = keyb_x == 0 ? 9 : keyb_x - 1; break;
                        case RETRO_DEVICE_ID_JOYPAD_RIGHT: keyb_x = keyb_x == 9 ? 0 : keyb_x + 1; break;
                        case RETRO_DEVICE_ID_JOYPAD_A:
                           if (keyb_send == 0)
                           {
                              keyb_overlay = false;
                              
                              keyb_event.type = INPUT_EVENT_KEYPRESS;
                              keyb_event.types.key.native_key = keyb_layout[keyb_y][keyb_x];
                              keyb_event.types.key.spectrum_key = keyb_layout[keyb_y][keyb_x];
                              input_event(&keyb_event);
                              
                              keyb_send = perf_cb.get_time_usec() + keyb_hold_time;
                           }
                           return 0;
                     }
                  }
               }
               else
               {
                  if (joypad_state[0][id])
                  {
                     joypad_state[0][id] = false;
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
  
   return fuse_ui_error_specific(severity, message);
}

int ui_end(void)
{
   return 0;
}
