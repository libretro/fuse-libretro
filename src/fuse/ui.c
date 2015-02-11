// Rename print_error_to_stderr so we can mute it

#define print_error_to_stderr fuse_print_error_to_stderr
#include <fuse/ui.c>
#undef print_error_to_stderr

static int print_error_to_stderr(ui_error_level severity, const char *message)
{
   (void)severity;
   (void)message;
   return 0;
}
