LOCAL_PATH := $(call my-dir)

CORE_DIR := $(LOCAL_PATH)/../..

INCLUDES      :=
SOURCES_C     :=
SOURCES_LUA_C :=

include $(CORE_DIR)/build/Makefile.common
SOURCES_C := $(filter-out %/version.c, $(SOURCES_C))

COREFLAGS := $(RETRODEFS) $(INCLUDES)

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

$(shell cp $(CORE_DIR)/src/config_fuse.h $(CORE_DIR)/fuse/config.h)
$(shell cp $(CORE_DIR)/src/config_libspectrum.h $(CORE_DIR)/libspectrum/config.h)

include $(CLEAR_VARS)
LOCAL_MODULE    := retro
LOCAL_SRC_FILES := $(SOURCES_C)
LOCAL_CFLAGS    := $(COREFLAGS) -D__LIBRETRO__
LOCAL_LDFLAGS   := -Wl,-version-script=$(CORE_DIR)/build/link.T
include $(BUILD_SHARED_LIBRARY)
