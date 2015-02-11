// Compatibility mouse functions

int ui_mouse_grab(int startup)
{
   (void)startup;
   return 1;
}

int ui_mouse_release(int suspend)
{
   return !suspend;
}
