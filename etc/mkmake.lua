local template = [[
##############
# ${HEADERMSG}

#############
# User config

DEBUG = 0
LOG_PERFORMANCE = 1

####################################
# Variable setup for Makefile.common

CORE_DIR  ?= .
INCLUDES  = ${PLAT_INCDIR}

include $(CORE_DIR)/Makefile.common

#################
# Toolchain setup

CC  = ${CC}
CXX = ${CXX}
AS  = ${AS}
AR  = ${AR}

############
# Extensions

OBJEXT = .${EXT}.o
SOEXT  = .${EXT}.${SO}

################
# Platform setup

STATIC_LINKING = ${STATIC_LINKING}
platform       = ${PLATFORM}
PLATDEFS       = ${PLAT_DEFS}
PLATCFLAGS     = ${PLAT_CFLAGS}
PLATCXXFLAGS   = ${PLAT_CXXFLAGS}
PLATLDFLAGS    = ${PLAT_LDFLAGS}
PLATLDXFLAGS   = ${PLAT_LDXFLAGS}

################
# libretro setup

RETRODEFS     = -D__LIBRETRO__
RETROCFLAGS   =
RETROCXXFLAGS =
RETROLDFLAGS  =
RETROLDXFLAGS =

#################
# Final variables

DEFINES  = $(PLATDEFS) $(RETRODEFS)
CFLAGS   = $(PLATCFLAGS) $(RETROCFLAGS) $(DEFINES) $(INCLUDES)
CXXFLAGS = $(PLATCXXFLAGS) $(RETROCXXFLAGS) $(DEFINES) $(INCLUDES)
LDFLAGS  = $(PLATLDFLAGS) $(RETROLDFLAGS)
LDXFLAGS = $(PLATLDXFLAGS) $(RETROLDXFLAGS)

########
# Tuning

ifeq ($(DEBUG), 1)
  CFLAGS   += -O0 -g
  CXXFLAGS += -O0 -g
  LDFLAGS  += -g
  LDXFLAGS += -g
else
  CFLAGS   += -O3 -DNDEBUG
  CXXFLAGS += -O3 -DNDEBUG
endif

ifeq ($(LOG_PERFORMANCE), 1)
  CFLAGS   += -DLOG_PERFORMANCE
  CXXFLAGS += -DLOG_PERFORMANCE
endif

###############
# Include rules

include $(CORE_DIR)/Makefile.rules
]]

local host = 'linux-x86_64'
--local host = 'linux-x86'
--local host = 'windows-x86_64'

local platforms = {
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  android_arm64_v8a = {
    MAKEFILE      = 'Makefile.android_arm64_v8a',
    HEADERMSG     = 'Download android-ndk-r10d-linux-x86_64.bin from https://developer.android.com/tools/sdk/ndk/index.html, unpack somewhere, and set NDK_ROOT_DIR to it',
    CC            = '$(NDK_ROOT_DIR)/toolchains/aarch64-linux-android-4.9/prebuilt/' .. host .. '/bin/aarch64-linux-android-gcc',
    CXX           = '$(NDK_ROOT_DIR)/toolchains/aarch64-linux-android-4.9/prebuilt/' .. host .. '/bin/aarch64-linux-android-g++',
    AS            = '$(NDK_ROOT_DIR)/toolchains/aarch64-linux-android-4.9/prebuilt/' .. host .. '/bin/aarch64-linux-android-as',
    AR            = '$(NDK_ROOT_DIR)/toolchains/aarch64-linux-android-4.9/prebuilt/' .. host .. '/bin/aarch64-linux-android-ar',
    EXT           = 'android_arm64_v8a',
    SO            = 'so',
    PLATFORM      = 'android',
    PLAT_INCDIR   = '-I$(NDK_ROOT_DIR)/platforms/android-21/arch-arm64/usr/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/libs/arm64-v8a/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/include/backward',
    PLAT_DEFS     = '-DANDROID -DINLINE=inline -DHAVE_STDINT_H -DBSPF_UNIX -DHAVE_INTTYPES -DLSB_FIRST',
    PLAT_CFLAGS   = '-fpic -ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -fomit-frame-pointer -fstrict-aliasing -funswitch-loops -finline-limit=300 -Wa,--noexecstack -Wformat -Werror=format-security',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS} -fno-exceptions -fno-rtti',
    PLAT_LDFLAGS  = '-shared --sysroot=$(NDK_ROOT_DIR)/platforms/android-21/arch-arm64 -lgcc -no-canonical-prefixes -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -lc -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS} $(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/libs/arm64-v8a/libgnustl_static.a',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  android_x86_64 = {
    MAKEFILE      = 'Makefile.android_x86_64',
    HEADERMSG     = 'Download android-ndk-r10d-linux-x86_64.bin from https://developer.android.com/tools/sdk/ndk/index.html, unpack somewhere, and set NDK_ROOT_DIR to it',
    CC            = '$(NDK_ROOT_DIR)/toolchains/x86_64-4.9/prebuilt/' .. host .. '/bin/x86_64-linux-android-gcc',
    CXX           = '$(NDK_ROOT_DIR)/toolchains/x86_64-4.9/prebuilt/' .. host .. '/bin/x86_64-linux-android-g++',
    AS            = '$(NDK_ROOT_DIR)/toolchains/x86_64-4.9/prebuilt/' .. host .. '/bin/x86_64-linux-android-as',
    AR            = '$(NDK_ROOT_DIR)/toolchains/x86_64-4.9/prebuilt/' .. host .. '/bin/x86_64-linux-android-ar',
    EXT           = 'android_x86_64',
    SO            = 'so',
    PLATFORM      = 'android',
    PLAT_INCDIR   = '-I$(NDK_ROOT_DIR)/platforms/android-21/arch-x86_64/usr/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/libs/x86_64/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/include/backward',
    PLAT_DEFS     = '-DANDROID -DINLINE=inline -DHAVE_STDINT_H -DBSPF_UNIX -DHAVE_INTTYPES -DLSB_FIRST',
    PLAT_CFLAGS   = '-ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -fomit-frame-pointer -fstrict-aliasing -funswitch-loops -finline-limit=300 -Wa,--noexecstack -Wformat -Werror=format-security',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS} -fno-exceptions -fno-rtti',
    PLAT_LDFLAGS  = '-shared --sysroot=$(NDK_ROOT_DIR)/platforms/android-21/arch-x86_64 -lgcc -no-canonical-prefixes -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -lc -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS} $(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/libs/x86_64/libgnustl_static.a',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  android_mips64 = {
    MAKEFILE      = 'Makefile.android_mips64',
    HEADERMSG     = 'Download android-ndk-r10d-linux-x86_64.bin from https://developer.android.com/tools/sdk/ndk/index.html, unpack somewhere, and set NDK_ROOT_DIR to it',
    CC            = '$(NDK_ROOT_DIR)/toolchains/mips64el-linux-android-4.9/prebuilt/' .. host .. '/bin/mips64el-linux-android-gcc',
    CXX           = '$(NDK_ROOT_DIR)/toolchains/mips64el-linux-android-4.9/prebuilt/' .. host .. '/bin/mips64el-linux-android-g++',
    AS            = '$(NDK_ROOT_DIR)/toolchains/mips64el-linux-android-4.9/prebuilt/' .. host .. '/bin/mips64el-linux-android-as',
    AR            = '$(NDK_ROOT_DIR)/toolchains/mips64el-linux-android-4.9/prebuilt/' .. host .. '/bin/mips64el-linux-android-ar',
    EXT           = 'android_mips64',
    SO            = 'so',
    PLATFORM      = 'android',
    PLAT_INCDIR   = '-I$(NDK_ROOT_DIR)/platforms/android-21/arch-mips64/usr/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/libs/mips64/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/include/backward',
    PLAT_DEFS     = '-DANDROID -DINLINE=inline -DHAVE_STDINT_H -DBSPF_UNIX -DHAVE_INTTYPES -DLSB_FIRST',
    PLAT_CFLAGS   = '-fpic -fno-strict-aliasing -finline-functions -ffunction-sections -funwind-tables -fmessage-length=0 -fno-inline-functions-called-once -fgcse-after-reload -frerun-cse-after-loop -frename-registers -no-canonical-prefixes -fomit-frame-pointer -funswitch-loops -finline-limit=300 -Wa,--noexecstack -Wformat -Werror=format-security',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS} -fno-exceptions -fno-rtti',
    PLAT_LDFLAGS  = '-shared --sysroot=$(NDK_ROOT_DIR)/platforms/android-21/arch-mips64 -lgcc -no-canonical-prefixes -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -lc -lm',
    PLAT_LDXFLAGS  = '${PLAT_LDFLAGS} $(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.9/libs/mips64/libgnustl_static.a',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  android_arm_v7a = {
    MAKEFILE      = 'Makefile.armeabi-v7a',
    HEADERMSG     = 'Download android-ndk-r10d-linux-x86_64.bin from https://developer.android.com/tools/sdk/ndk/index.html, unpack somewhere, and set NDK_ROOT_DIR to it',
    CC            = '$(NDK_ROOT_DIR)/toolchains/arm-linux-androideabi-4.8/prebuilt/' .. host .. '/bin/arm-linux-androideabi-gcc',
    CXX           = '$(NDK_ROOT_DIR)/toolchains/arm-linux-androideabi-4.8/prebuilt/' .. host .. '/bin/arm-linux-androideabi-g++',
    AS            = '$(NDK_ROOT_DIR)/toolchains/arm-linux-androideabi-4.8/prebuilt/' .. host .. '/bin/arm-linux-androideabi-as',
    AR            = '$(NDK_ROOT_DIR)/toolchains/arm-linux-androideabi-4.8/prebuilt/' .. host .. '/bin/arm-linux-androideabi-ar',
    EXT           = 'armeabi-v7a',
    SO            = 'so',
    PLATFORM      = 'android',
    PLAT_INCDIR   = '-I$(NDK_ROOT_DIR)/platforms/android-3/arch-arm/usr/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/include/backward',
    PLAT_DEFS     = '-DANDROID -DINLINE=inline -DHAVE_STDINT_H -DBSPF_UNIX -DHAVE_INTTYPES -DLSB_FIRST',
    PLAT_CFLAGS   = '-fpic -ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp -fomit-frame-pointer -fstrict-aliasing -funswitch-loops -finline-limit=300 -Wa,--noexecstack -Wformat -Werror=format-security',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS} -fno-exceptions -fno-rtti',
    PLAT_LDFLAGS  = '-shared --sysroot=$(NDK_ROOT_DIR)/platforms/android-3/arch-arm -lgcc -no-canonical-prefixes -march=armv7-a -Wl,--fix-cortex-a8 -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -lc -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS} $(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/thumb/libgnustl_static.a',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  android_arm_v5te = {
    MAKEFILE      = 'Makefile.armeabi',
    HEADERMSG     = 'Download android-ndk-r10d-linux-x86_64.bin from https://developer.android.com/tools/sdk/ndk/index.html, unpack somewhere, and set NDK_ROOT_DIR to it',
    CC            = '$(NDK_ROOT_DIR)/toolchains/arm-linux-androideabi-4.8/prebuilt/' .. host .. '/bin/arm-linux-androideabi-gcc',
    CXX           = '$(NDK_ROOT_DIR)/toolchains/arm-linux-androideabi-4.8/prebuilt/' .. host .. '/bin/arm-linux-androideabi-g++',
    AS            = '$(NDK_ROOT_DIR)/toolchains/arm-linux-androideabi-4.8/prebuilt/' .. host .. '/bin/arm-linux-androideabi-as',
    AR            = '$(NDK_ROOT_DIR)/toolchains/arm-linux-androideabi-4.8/prebuilt/' .. host .. '/bin/arm-linux-androideabi-ar',
    EXT           = 'armeabi',
    SO            = 'so',
    PLATFORM      = 'android',
    PLAT_INCDIR   = '-I$(NDK_ROOT_DIR)/platforms/android-3/arch-arm/usr/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/include/backward',
    PLAT_DEFS     = '-DANDROID -DINLINE=inline -DHAVE_STDINT_H -DBSPF_UNIX -DHAVE_INTTYPES -DLSB_FIRST',
    PLAT_CFLAGS   = '-fpic -ffunction-sections -funwind-tables -fstack-protector -no-canonical-prefixes -march=armv5te -mtune=xscale -msoft-float -fomit-frame-pointer -fstrict-aliasing -funswitch-loops -finline-limit=300 -Wa,--noexecstack -Wformat -Werror=format-security',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS} -fno-exceptions -fno-rtti',
    PLAT_LDFLAGS  = '-shared --sysroot=$(NDK_ROOT_DIR)/platforms/android-3/arch-arm -lgcc -no-canonical-prefixes -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -lc -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS} $(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi/thumb/libgnustl_static.a',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  android_x86 = {
    MAKEFILE      = 'Makefile.x86',
    HEADERMSG     = 'Download android-ndk-r10d-linux-x86_64.bin from https://developer.android.com/tools/sdk/ndk/index.html, unpack somewhere, and set NDK_ROOT_DIR to it',
    CC            = '$(NDK_ROOT_DIR)/toolchains/x86-4.8/prebuilt/' .. host .. '/bin/i686-linux-android-gcc',
    CXX           = '$(NDK_ROOT_DIR)/toolchains/x86-4.8/prebuilt/' .. host .. '/bin/i686-linux-android-g++',
    AS            = '$(NDK_ROOT_DIR)/toolchains/x86-4.8/prebuilt/' .. host .. '/bin/i686-linux-android-as',
    AR            = '$(NDK_ROOT_DIR)/toolchains/x86-4.8/prebuilt/' .. host .. '/bin/i686-linux-android-ar',
    EXT           = 'x86',
    SO            = 'so',
    PLATFORM      = 'android',
    PLAT_INCDIR   = '-I$(NDK_ROOT_DIR)/platforms/android-9/arch-x86/usr/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/include $(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/libs/x86/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/include/backward',
    PLAT_DEFS     = '-DANDROID -DINLINE=inline -DHAVE_STDINT_H -DBSPF_UNIX -DHAVE_INTTYPES -DLSB_FIRST',
    PLAT_CFLAGS   = '-ffunction-sections -funwind-tables -no-canonical-prefixes -fstack-protector -fomit-frame-pointer -fstrict-aliasing -funswitch-loops -finline-limit=300 -Wa,--noexecstack -Wformat -Werror=format-security',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS} -fno-exceptions -fno-rtti',
    PLAT_LDFLAGS  = '-shared --sysroot=$(NDK_ROOT_DIR)/platforms/android-9/arch-x86 -lgcc -no-canonical-prefixes -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -lc -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS} $(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/libs/x86/libgnustl_static.a',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  android_mips = {
    MAKEFILE      = 'Makefile.android_mips',
    HEADERMSG     = 'Download android-ndk-r10d-linux-x86_64.bin from https://developer.android.com/tools/sdk/ndk/index.html, unpack somewhere, and set NDK_ROOT_DIR to it',
    CC            = '$(NDK_ROOT_DIR)/toolchains/mipsel-linux-android-4.8/prebuilt/' .. host .. '/bin/mipsel-linux-android-gcc',
    CXX           = '$(NDK_ROOT_DIR)/toolchains/mipsel-linux-android-4.8/prebuilt/' .. host .. '/bin/mipsel-linux-android-g++',
    AS            = '$(NDK_ROOT_DIR)/toolchains/mipsel-linux-android-4.8/prebuilt/' .. host .. '/bin/mipsel-linux-android-as',
    AR            = '$(NDK_ROOT_DIR)/toolchains/mipsel-linux-android-4.8/prebuilt/' .. host .. '/bin/mipsel-linux-android-ar',
    EXT           = 'android_mips',
    SO            = 'so',
    PLATFORM      = 'android',
    PLAT_INCDIR   = '-I$(NDK_ROOT_DIR)/platforms/android-9/arch-mips/usr/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/libs/mips/include -I$(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/include/backward',
    PLAT_DEFS     = '-DANDROID -DINLINE=inline -DHAVE_STDINT_H -DBSPF_UNIX -DHAVE_INTTYPES -DLSB_FIRST',
    PLAT_CFLAGS   = '-fpic -fno-strict-aliasing -finline-functions -ffunction-sections -funwind-tables -fmessage-length=0 -fno-inline-functions-called-once -fgcse-after-reload -frerun-cse-after-loop -frename-registers -no-canonical-prefixes -fomit-frame-pointer -funswitch-loops -finline-limit=300 -Wa,--noexecstack -Wformat -Werror=format-security',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS} -fno-exceptions -fno-rtti',
    PLAT_LDFLAGS  = '-shared --sysroot=$(NDK_ROOT_DIR)/platforms/android-9/arch-mips -lgcc -no-canonical-prefixes -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -lc -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS} $(NDK_ROOT_DIR)/sources/cxx-stl/gnu-libstdc++/4.8/libs/mips/libgnustl_static.a',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  linux_x86 = {
    MAKEFILE      = 'Makefile.linux_x86',
    HEADERMSG     = 'apt-get install g++-multilib libc6-dev-i386',
    CC            = 'gcc',
    CXX           = 'g++',
    AS            = 'as',
    AR            = 'ar',
    EXT           = 'linux_x86',
    SO            = 'so',
    PLATFORM      = 'unix',
    PLAT_INCDIR   = '',
    PLAT_DEFS     = '',
    PLAT_CFLAGS   = '-m32 -fpic -fstrict-aliasing',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS}',
    PLAT_LDFLAGS  = '-m32 -shared -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS}',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  linux_x86_64 = {
    MAKEFILE      = 'Makefile.linux_x86_64',
    HEADERMSG     = '',
    CC            = 'gcc',
    CXX           = 'g++',
    AS            = 'as',
    AR            = 'ar',
    EXT           = 'linux_x86_64',
    SO            = 'so',
    PLATFORM      = 'unix',
    PLAT_INCDIR   = '',
    PLAT_DEFS     = '',
    PLAT_CFLAGS   = '-m64 -fpic -fstrict-aliasing',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS}',
    PLAT_LDFLAGS  = '-m64 -shared -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS}',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  windows_x86 = {
    MAKEFILE      = 'Makefile.windows_x86',
    HEADERMSG     = 'apt-get install mingw-w64',
    CC            = 'i686-w64-mingw32-gcc',
    CXX           = 'i686-w64-mingw32-g++',
    AS            = 'i686-w64-mingw32-as',
    AR            = 'i686-w64-mingw32-ar',
    EXT           = 'windows_x86',
    SO            = 'dll',
    PLATFORM      = 'win',
    PLAT_INCDIR   = '',
    PLAT_DEFS     = '',
    PLAT_CFLAGS   = '-fstrict-aliasing',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS}',
    PLAT_LDFLAGS  = '-shared -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS}',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  windows_x86_64 = {
    MAKEFILE      = 'Makefile.windows_x86_64',
    HEADERMSG     = 'apt-get install mingw-w64',
    CC            = 'x86_64-w64-mingw32-gcc',
    CXX           = 'x86_64-w64-mingw32-g++',
    AS            = 'x86_64-w64-mingw32-as',
    AR            = 'x86_64-w64-mingw32-ar',
    EXT           = 'windows_x86_64',
    SO            = 'dll',
    PLATFORM      = 'win',
    PLAT_INCDIR   = '',
    PLAT_DEFS     = '',
    PLAT_CFLAGS   = '-fpic -fstrict-aliasing',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS}',
    PLAT_LDFLAGS  = '-shared -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS}',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  mingw32 = {
    MAKEFILE      = 'Makefile.mingw32',
    HEADERMSG     = 'Install MSYS2',
    CC            = 'gcc',
    CXX           = 'g++',
    AS            = 'as',
    AR            = 'ar',
    EXT           = 'mingw32',
    SO            = 'dll',
    PLATFORM      = 'win',
    PLAT_INCDIR   = '',
    PLAT_DEFS     = '',
    PLAT_CFLAGS   = '-m32 -fpic -fstrict-aliasing',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS}',
    PLAT_LDFLAGS  = '-shared -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS}',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  mingw64 = {
    MAKEFILE      = 'Makefile.mingw64',
    HEADERMSG     = 'Install MSYS2',
    CC            = 'gcc',
    CXX           = 'g++',
    AS            = 'as',
    AR            = 'ar',
    EXT           = 'mingw64',
    SO            = 'dll',
    PLATFORM      = 'win',
    PLAT_INCDIR   = '',
    PLAT_DEFS     = '',
    PLAT_CFLAGS   = '-m64 -fpic -fstrict-aliasing',
    PLAT_CXXFLAGS = '${PLAT_CFLAGS}',
    PLAT_LDFLAGS  = '-shared -lm',
    PLAT_LDXFLAGS = '${PLAT_LDFLAGS}',
  },
  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  wii = {
    MAKEFILE       = 'Makefile.wii',
    HEADERMSG      = 'Install devkitppc and libogc',
    CC             = '$(DEVKITPPC_ROOT_DIR)/bin/powerpc-eabi-gcc',
    CXX            = '$(DEVKITPPC_ROOT_DIR)/bin/powerpc-eabi-g++',
    AS             = '$(DEVKITPPC_ROOT_DIR)/bin/powerpc-eabi-as',
    AR             = '$(DEVKITPPC_ROOT_DIR)/bin/powerpc-eabi-ar',
    EXT            = 'wii',
    SO             = 'so',
    PLATFORM       = 'wii',
    PLAT_INCDIR    = '',
    PLAT_DEFS      = '-DGEKKO -DHW_RVL',
    PLAT_CFLAGS    = '-m32 -fstrict-aliasing -mrvl -mcpu=750 -meabi -mhard-float',
    PLAT_CXXFLAGS  = '${PLAT_CFLAGS}',
    PLAT_LDFLAGS   = '-shared -lm',
    PLAT_LDXFLAGS  = '${PLAT_LDFLAGS}',
    STATIC_LINKING = '1'
  },
}

for plat, defs in pairs( platforms ) do
  local templ = template
  local equal

  defs.STATIC_LINKING = defs.STATIC_LINKING or '0'
  
  repeat
    equal = true
    
    for def, value in pairs( defs ) do
      local templ2 = templ:gsub( '%${' .. def .. '}', ( value:gsub( '%%', '%%%%' ) ) )
      equal = equal and templ == templ2
      templ = templ2
    end
  until equal
  
  local file, err = io.open( defs.MAKEFILE, 'wb' )
  if not file then error( err ) end
  
  file:write( templ )
  file:close()
end
