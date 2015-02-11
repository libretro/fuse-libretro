// Compatibility file functions

#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <libretro.h>
#include <externs.h>
#include <compat.h>
#include <ui/ui.h>
#include <utils.h>

#include <src/compat/48.rom.h>
#include <src/compat/tape_48.szx.h>

typedef struct
{
   const char *name;
   const unsigned char* ptr;
   size_t size;
}
entry_t;

static const entry_t mem_entries[] = {
   {
      FUSE_DIR_SEP_STR "fuse" FUSE_DIR_SEP_STR "roms" FUSE_DIR_SEP_STR "48.rom",
      fuse_roms_48_rom, sizeof(fuse_roms_48_rom)
   },
   {
      FUSE_DIR_SEP_STR "fuse" FUSE_DIR_SEP_STR "lib" FUSE_DIR_SEP_STR "tape_48.szx",
      fuse_lib_compressed_tape_48_szx, sizeof(fuse_lib_compressed_tape_48_szx)
   }
};

static const entry_t* find_entry(const char *path)
{
   int i;
   size_t len = strlen(path);
   
   for (i = 0; i < sizeof(mem_entries) / sizeof(mem_entries[0]); i++)
   {
      size_t len2 = strlen(mem_entries[i].name);
      
      if (!strcmp(path + len - len2, mem_entries[i].name))
      {
         return mem_entries + i;
      }
   }
   
   return NULL;
}

typedef struct
{
   int type; // 0=FILE 1=memory
   
   union {
      FILE* file;
      
      struct {
         const char* ptr;
         size_t length, remain;
      };
   };
}
compat_fd_internal;

const compat_fd COMPAT_FILE_OPEN_FAILED = NULL;

compat_fd compat_file_open(const char *path, int write)
{
   compat_fd_internal *fd = (compat_fd_internal*)malloc(sizeof(compat_fd_internal));
   
   if (fd)
   {
      if (!write)
      {
         const entry_t* entry = find_entry(path);
         
         if (entry != NULL)
         {
            fd->type = 1;
            fd->ptr = entry->ptr;
            fd->length = fd->remain = entry->size;
            
            log_cb(RETRO_LOG_INFO, "Opened \"%s\" from memory\n", path);
            return (compat_fd)fd;
         }
      }
      
      fd->type = 0;
      fd->file = fopen( path, write ? "wb" : "rb" );
      
      if (fd->file)
      {
         log_cb(RETRO_LOG_INFO, "Opened \"%s\" from the file system\n", path);
         return (compat_fd)fd;
      }
      
      free(fd);
   }
   
   return COMPAT_FILE_OPEN_FAILED;
}

off_t compat_file_get_length(compat_fd cfd)
{
   struct stat file_info;
   compat_fd_internal *fd = (compat_fd_internal*)cfd;
   
   if (fd->type == 1)
   {
      return (off_t)fd->length;
   }
 
   if (fstat(fileno(fd->file), &file_info))
   {
      ui_error(UI_ERROR_ERROR, "couldn't stat file: %s", strerror(errno));
      return -1;
   }
 
   return file_info.st_size;
}

int compat_file_read(compat_fd cfd, utils_file *file)
{
   compat_fd_internal *fd = (compat_fd_internal*)cfd;
   size_t numread;
   
   if (fd->type == 1)
   {
      numread = file->length < fd->remain ? file->length : fd->remain;
      memcpy(file->buffer, fd->ptr, numread);
      fd->ptr += numread;
      fd->remain -= numread;
   }
   else
   {
      numread = fread(file->buffer, 1, file->length, fd->file);
   }
   
   if (numread == file->length)
   {
      return 0;
   }
   
   ui_error( UI_ERROR_ERROR,
             "error reading file: expected %lu bytes, but read only %lu",
             (unsigned long)file->length, (unsigned long)numread );
   return 1;
}

int compat_file_write(compat_fd cfd, const unsigned char *buffer, size_t length)
{
   compat_fd_internal *fd = (compat_fd_internal*)cfd;
   
   size_t numwritten = fwrite(buffer, 1, length, fd->file);
   
   if (numwritten == length)
   {
      return 0;
   }
   
   ui_error( UI_ERROR_ERROR,
             "error writing file: expected %lu bytes, but wrote only %lu",
             (unsigned long)length, (unsigned long)numwritten );
   return 1;
}

int compat_file_close(compat_fd cfd)
{
   compat_fd_internal *fd = (compat_fd_internal*)cfd;
   
   if (fd->type == 0)
   {
      int res = fclose(fd->file);
      free(fd);
      return res;
   }
   
   free(fd);
   return 0;
}

int compat_file_exists(const char *path)
{
   log_cb(RETRO_LOG_INFO, "Checking if \"%s\" exists\n", path);
   return find_entry(path) != NULL || access( path, R_OK ) != -1;
}
