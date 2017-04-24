PLATFORM_SYS_ROOT       := /android-ndk-r10d/platforms/android-17/arch-arm 

PLATFORM_INCLUDE_DIRECTORYS     := $(PLATFORM_INCLUDE_DIRECTORYS) /android-ndk-r10d/sources/cxx-stl/gnu-libstdc++/4.9/include
PLATFORM_INCLUDE_DIRECTORYS     := $(PLATFORM_INCLUDE_DIRECTORYS) /android-ndk-r10d/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi/include
PLATFORM_LIBRARY_DIRECTORYS := $(PLATFORM_LIBRARY_DIRECTORYS) /android-ndk-r10d/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi

PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -fpic
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -mthumb-interwork
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -ffunction-sections
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -funwind-tables
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -fstack-protector
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -fno-short-enums 
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -Wno-psabi
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -march=armv7-a
#PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -mtune=xscale
#PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -msoft-float
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -mthumb
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -fomit-frame-pointer
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -fno-strict-aliasing
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -finline-limit=64
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -Wa,--noexecstack
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -DANDROID=1 

PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -D__ANDROID__=1

#PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -nostdlib
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Bdynamic
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Wl,-dynamic-linker,/system/bin/linker
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Wl,--gc-sections
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Wl,-z,nocopyreloc
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Wl,--no-undefined
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Wl,-z,noexecstack

PLATFORM_CRTBEGIN_STATICBIN	:= crtbegin_static.o
PLATFORM_CRTEND_STATICBIN	:= crtend_android.o
PLATFORM_CRTBEGIN_DYNAMICBIN	:= crtbegin_dynamic.o
PLATFORM_CRTEND_DYNAMICBIN	:= crtend_android.o

PLATFORM_DEPEND_LIBRARYS    := $(PLATFORM_DEPEND_LIBRARYS) gnustl_static
# 暂不用
#PLATFORM_CRTBEGIN_DYNAMIC	:= crtbegin_so.o
#PLATFORM_CRTEND_DYNAMIC		:= crtend_so.o
	
PLATFORM_DEPEND_LIBRARYS	:= $(PLATFORM_DEPEND_LIBRARYS) stdc++
PLATFORM_DEPEND_LIBRARYS	:= $(PLATFORM_DEPEND_LIBRARYS) m rt pthread c dl gcc

PLATFORM_DISABLE_FLAGS		:= -pthread -lpthread -lrt

PLATFORM_DEPEND_LIBRARYS    := $(PLATFORM_DEPEND_LIBRARYS) ssl crypto
