// Compatibility path functions

#include <libretro.h>
#include <externs.h>
#include <ui/ui.h>
#include <compat.h>

const char *compat_get_temp_path(void)
{
   return "";
}

const char *compat_get_config_path(void)
{
#ifdef __CELLOS_LV2__
   return "/dev_hdd0/game/SSNE10000/USRDIR/cores/system";
#else
   return "";
#endif
}

int compat_is_absolute_path(const char *path)
{
   return 0;
}

int compat_get_next_path(path_context *ctx)
{
   switch (ctx->state++)
   {
      case 0:
         ctx->path[0] = 0;
         return 1;

      case 1:
         return 0;
   }

   ui_error(UI_ERROR_ERROR, "unknown path_context state %d", ctx->state);
   return 0;
}
