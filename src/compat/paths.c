// Compatibility path functions

#include <string.h>

#include <libretro.h>
#include <file/file_path.h>
#include <externs.h>
#include <ui/ui.h>
#include <compat.h>

const char *compat_get_temp_path(void)
{
   return "";
}

const char *compat_get_config_path(void)
{
#ifdef __PS3__
   return "/dev_hdd0/game/RETROARCH/USRDIR/system";
#else
   return "";
#endif
}

int compat_is_absolute_path(const char *path)
{
   /* This used to answer "never", which made utils_find_auxiliary_file()
      and utils_find_file_path() treat an absolute name as a relative one
      and glue a search directory in front of it. Nothing exercised that
      while every asset was baked into the binary, but a ROM path pointing
      outside the system directory has to resolve to itself. Delegate to
      libretro-common, which knows about drive letters and UNC prefixes as
      well as a leading separator. */
   return path && path_is_absolute(path);
}

int compat_get_next_path(path_context *ctx)
{
   switch (ctx->state++)
   {
      /* The empty path first, which resolves to the bare filename and so
         hits the assets baked into the binary (see find_entry() in
         compat/file.c). Keeping it first means a build-in ROM still wins
         over a same-named file in the system directory, so no existing
         content changes behaviour. */
      case 0:
         ctx->path[0] = 0;
         return 1;

      /* Then the frontend's system directory, for files the core cannot
         ship - the Currah uSpeech interface and SP0256-AL2 allophone ROMs
         are both copyrighted and have to be supplied by the user. Skipped
         entirely when the frontend offered no system directory. */
      case 1:
         if (!system_dir[0])
            return 0;

         strncpy(ctx->path, system_dir, sizeof(ctx->path) - 1);
         ctx->path[sizeof(ctx->path) - 1] = 0;
         return 1;

      case 2:
         return 0;
   }

   ui_error(UI_ERROR_ERROR, "unknown path_context state %d", ctx->state);
   return 0;
}
