// Compatibility path functions

#include <libretro.h>
#include <externs.h>
#include <ui/ui.h>
#include <compat.h>

const char *compat_get_temp_path(void)
{
   return ".";
}

const char *compat_get_home_path(void)
{
   return ".";
}

int compat_is_absolute_path(const char *path)
{
   // TODO how to handle other architectures?
#ifdef WIN32
   if( path[0] == '\\' ) return 1;
   if( path[0] && path[1] == ':' ) return 1;
#else
   if ( path[0] == '/' ) return 1;
#endif

   return 0;
}

int compat_get_next_path(path_context *ctx)
{
   const char *content, *path_segment;
   
   if (!env_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &content) || !content)
   {
      ui_error(UI_ERROR_ERROR, "Could not get libretro's system directory" );
      return 0;
   }

   switch (ctx->type)
   {
      case UTILS_AUXILIARY_LIB:    path_segment = "lib"; break;
      case UTILS_AUXILIARY_ROM:    path_segment = "roms"; break;
      case UTILS_AUXILIARY_WIDGET: path_segment = "ui/widget"; break;
      case UTILS_AUXILIARY_GTK:    path_segment = "ui/gtk"; break;
      
      default:
         ui_error(UI_ERROR_ERROR, "Unknown auxiliary file type %d", ctx->type);
         return 0;
   }

   switch (ctx->state++)
   {
      case 0:
         snprintf(ctx->path, PATH_MAX, "%s" FUSE_DIR_SEP_STR "fuse" FUSE_DIR_SEP_STR "%s", content, path_segment);
         ctx->path[PATH_MAX - 1] = 0;
         log_cb(RETRO_LOG_INFO, "Returning \"%s\" as path for %s\n", ctx->path, ctx->type == UTILS_AUXILIARY_LIB ? "lib" : ctx->type == UTILS_AUXILIARY_ROM ? "rom" : ctx->type == UTILS_AUXILIARY_WIDGET ? "widget" : "gtk" );
         return 1;

      case 1:
         return 0;
   }

   ui_error(UI_ERROR_ERROR, "unknown path_context state %d", ctx->state);
   return 0;
}
