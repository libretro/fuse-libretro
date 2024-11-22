// Compatibility display funcions

#include <libretro.h>
#include <externs.h>
#include <machine.h>

int uidisplay_init(int width, int height)
{
   log_cb(RETRO_LOG_DEBUG, "%s(%d, %d)\n", __FUNCTION__, width, height);
   
   if (width != 320)
   {
      log_cb(RETRO_LOG_ERROR, "Invalid value for the display width: %d\n", width);
      width = 320;
   }
   
   if (height != 240)
   {
      log_cb(RETRO_LOG_ERROR, "Invalid value for the display height: %d\n", height);
      height = 240;
   }
   
   //soft_width = (unsigned)width;
   //soft_height = (unsigned)height;
   //change_geometry = true;
   //log_cb(RETRO_LOG_INFO, "Display set to %dx%d\n", width, height);
   
   return 0;
}

void uidisplay_area(int x, int y, int w, int h)
{
   (void)x;
   (void)y;
   (void)w;
   (void)h;
}

void uidisplay_frame_end(void)
{
   show_frame = 1;
}

int uidisplay_hotswap_gfx_mode(void)
{
   return 0;
}

int uidisplay_end(void)
{
   return 0;
}

void uidisplay_putpixel(int x, int y, int color)
{
   uint16_t palette_color = palette[color];
   
   if (machine_current->timex)
   {
      x <<= 1; y <<= 1;
      uint16_t* image_buffer_pos = image_buffer + (y * hard_width + x);
      
      *image_buffer_pos++ = palette_color;
      *image_buffer_pos   = palette_color;
      
      image_buffer_pos += hard_width - 1;
      
      *image_buffer_pos++ = palette_color;
      *image_buffer_pos   = palette_color;
   }
   else
   {
      uint16_t* image_buffer_pos = image_buffer + (y * hard_width + x);
      *image_buffer_pos = palette_color;
   }
}

void uidisplay_plot8(int x, int y, libspectrum_byte data, libspectrum_byte ink, libspectrum_byte paper)
{
   // TODO: SSE versions maybe?
   uint16_t palette_ink = palette[ink];
   uint16_t palette_paper = palette[paper];
   
   x <<= 3;

   if (machine_current->timex)
   {
      x <<= 1; y <<= 1;
      uint16_t* image_buffer_pos = image_buffer + (y * hard_width + x);
      
      uint16_t data_80 = data & 0x80 ? palette_ink : palette_paper;
      uint16_t data_40 = data & 0x40 ? palette_ink : palette_paper;
      uint16_t data_20 = data & 0x20 ? palette_ink : palette_paper;
      uint16_t data_10 = data & 0x10 ? palette_ink : palette_paper;
      uint16_t data_08 = data & 0x08 ? palette_ink : palette_paper;
      uint16_t data_04 = data & 0x04 ? palette_ink : palette_paper;
      uint16_t data_02 = data & 0x02 ? palette_ink : palette_paper;
      uint16_t data_01 = data & 0x01 ? palette_ink : palette_paper;
    
      *image_buffer_pos++ = data_80;
      *image_buffer_pos++ = data_80;
      *image_buffer_pos++ = data_40;
      *image_buffer_pos++ = data_40;
      *image_buffer_pos++ = data_20;
      *image_buffer_pos++ = data_20;
      *image_buffer_pos++ = data_10;
      *image_buffer_pos++ = data_10;
      *image_buffer_pos++ = data_08;
      *image_buffer_pos++ = data_08;
      *image_buffer_pos++ = data_04;
      *image_buffer_pos++ = data_04;
      *image_buffer_pos++ = data_02;
      *image_buffer_pos++ = data_02;
      *image_buffer_pos++ = data_01;
      *image_buffer_pos   = data_01;
    
      image_buffer_pos += hard_width - 15;

      *image_buffer_pos++ = data_80;
      *image_buffer_pos++ = data_80;
      *image_buffer_pos++ = data_40;
      *image_buffer_pos++ = data_40;
      *image_buffer_pos++ = data_20;
      *image_buffer_pos++ = data_20;
      *image_buffer_pos++ = data_10;
      *image_buffer_pos++ = data_10;
      *image_buffer_pos++ = data_08;
      *image_buffer_pos++ = data_08;
      *image_buffer_pos++ = data_04;
      *image_buffer_pos++ = data_04;
      *image_buffer_pos++ = data_02;
      *image_buffer_pos++ = data_02;
      *image_buffer_pos++ = data_01;
      *image_buffer_pos   = data_01;
   }
   else
   {
      uint16_t* image_buffer_pos = image_buffer + (y * hard_width + x);
      
      *image_buffer_pos++ = data & 0x80 ? palette_ink : palette_paper;
      *image_buffer_pos++ = data & 0x40 ? palette_ink : palette_paper;
      *image_buffer_pos++ = data & 0x20 ? palette_ink : palette_paper;
      *image_buffer_pos++ = data & 0x10 ? palette_ink : palette_paper;
      *image_buffer_pos++ = data & 0x08 ? palette_ink : palette_paper;
      *image_buffer_pos++ = data & 0x04 ? palette_ink : palette_paper;
      *image_buffer_pos++ = data & 0x02 ? palette_ink : palette_paper;
      *image_buffer_pos   = data & 0x01 ? palette_ink : palette_paper;
   }
}

void uidisplay_plot16(int x, int y, libspectrum_word data, libspectrum_byte ink, libspectrum_byte paper)
{
   // TODO: SSE versions maybe?
   uint16_t palette_ink = palette[ink];
   uint16_t palette_paper = palette[paper];
   x <<= 4; y <<= 1;
   uint16_t* image_buffer_pos = image_buffer + (y * hard_width + x);
   
   uint16_t data_8000 = data & 0x8000 ? palette_ink : palette_paper;
   uint16_t data_4000 = data & 0x4000 ? palette_ink : palette_paper;
   uint16_t data_2000 = data & 0x2000 ? palette_ink : palette_paper;
   uint16_t data_1000 = data & 0x1000 ? palette_ink : palette_paper;
   uint16_t data_0800 = data & 0x0800 ? palette_ink : palette_paper;
   uint16_t data_0400 = data & 0x0400 ? palette_ink : palette_paper;
   uint16_t data_0200 = data & 0x0200 ? palette_ink : palette_paper;
   uint16_t data_0100 = data & 0x0100 ? palette_ink : palette_paper;
   uint16_t data_0080 = data & 0x0080 ? palette_ink : palette_paper;
   uint16_t data_0040 = data & 0x0040 ? palette_ink : palette_paper;
   uint16_t data_0020 = data & 0x0020 ? palette_ink : palette_paper;
   uint16_t data_0010 = data & 0x0010 ? palette_ink : palette_paper;
   uint16_t data_0008 = data & 0x0008 ? palette_ink : palette_paper;
   uint16_t data_0004 = data & 0x0004 ? palette_ink : palette_paper;
   uint16_t data_0002 = data & 0x0002 ? palette_ink : palette_paper;
   uint16_t data_0001 = data & 0x0001 ? palette_ink : palette_paper;

   *image_buffer_pos++ = data_8000;
   *image_buffer_pos++ = data_4000;
   *image_buffer_pos++ = data_2000;
   *image_buffer_pos++ = data_1000;
   *image_buffer_pos++ = data_0800;
   *image_buffer_pos++ = data_0400;
   *image_buffer_pos++ = data_0200;
   *image_buffer_pos++ = data_0100;
   *image_buffer_pos++ = data_0080;
   *image_buffer_pos++ = data_0040;
   *image_buffer_pos++ = data_0020;
   *image_buffer_pos++ = data_0010;
   *image_buffer_pos++ = data_0008;
   *image_buffer_pos++ = data_0004;
   *image_buffer_pos++ = data_0002;
   *image_buffer_pos   = data_0001;
  
   image_buffer_pos += hard_width - 15;

   *image_buffer_pos++ = data_8000;
   *image_buffer_pos++ = data_4000;
   *image_buffer_pos++ = data_2000;
   *image_buffer_pos++ = data_1000;
   *image_buffer_pos++ = data_0800;
   *image_buffer_pos++ = data_0400;
   *image_buffer_pos++ = data_0200;
   *image_buffer_pos++ = data_0100;
   *image_buffer_pos++ = data_0080;
   *image_buffer_pos++ = data_0040;
   *image_buffer_pos++ = data_0020;
   *image_buffer_pos++ = data_0010;
   *image_buffer_pos++ = data_0008;
   *image_buffer_pos++ = data_0004;
   *image_buffer_pos++ = data_0002;
   *image_buffer_pos   = data_0001;
}

void uidisplay_frame_save( void )
{
   /* FIXME: Save current framebuffer state as the widget UI wants to scribble
      in here */
}

void uidisplay_frame_restore( void )
{
   /* FIXME: Restore saved framebuffer state as the widget UI wants to draw a
      new menu */
}
