// Rename ui_error_specific so we can extend it

#define ui_error_specific fuse_ui_error_specific
#include <fuse/ui/widget/error.c>
#undef ui_error_specific
