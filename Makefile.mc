all: ccleste-mc.wasm ccleste-mc.wat

# ------------- Makefile from doomgeneric --------------- #

LIBC_DIR = /home/salix/Programming/newlib-cygwin/newlib/libc
LIBM_DIR = /home/salix/Programming/newlib-cygwin/newlib/libm

CC=clang  # gcc or g++
CXX=clang++
CFLAGS+=-O3 -g -nostdlib -target wasm32 -D__IEEE_LITTLE_ENDIAN -DMINECRAFT=1
CFLAGS+=-I$(LIBC_DIR)/include
CFLAGS+=-I$(LIBM_DIR)/include
CFLAGS+=-DHAVE_MMAP=0 -Dmalloc_getpagesize="(65536)" -D_GNU_SOURCE=1 -DNO_FLOATING_POINT=1
LDFLAGS+=-Wl,--lto-O3,--gc-sections,--import-undefined
CFLAGS+=-Wall
LIBS+=

# subdirectory for objects
OBJDIR2=newlib_build

OBJS_NEWLIB+=errno.o sysfstat.o

include Makefile.string.inc
include Makefile.stdlib.inc
include Makefile.stdio.inc
include Makefile.reent.inc
include Makefile.ctype.inc
include Makefile.posix.inc
include Makefile.locale.inc
include Makefile.libmcommon.inc

OBJS2 += $(addprefix $(OBJDIR2)/, $(OBJS_NEWLIB))

SRC_NEWLIB_RAW = stdio/printf.c stdio/fprintf.c
SRC_NEWLIB += $(addprefix ../../newlib-cygwin/newlib/libc/, $(SRC_NEWLIB_RAW))

$(OBJS2): | $(OBJDIR2)

$(OBJDIR2):
	mkdir -p $(OBJDIR2)

$(OBJDIR2)/%.o: $(LIBC_DIR)/stdio/%.c
	@echo [Compiling newlib $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR2)/%.o: $(LIBC_DIR)/string/%.c
	@echo [Compiling newlib $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR2)/%.o: $(LIBC_DIR)/stdlib/%.c
	@echo [Compiling newlib $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR2)/%.o: $(LIBC_DIR)/reent/%.c
	@echo [Compiling newlib $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR2)/%.o: $(LIBC_DIR)/ctype/%.c
	@echo [Compiling newlib $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR2)/%.o: $(LIBC_DIR)/posix/%.c
	@echo [Compiling newlib $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR2)/%.o: $(LIBC_DIR)/locale/%.c
	@echo [Compiling newlib $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR2)/%.o: $(LIBC_DIR)/errno/%.c
	@echo [Compiling newlib $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR2)/%.o: $(LIBC_DIR)/syscalls/%.c
	@echo [Compiling newlib $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR2)/%.o: $(LIBM_DIR)/common/%.c
	@echo [Compiling newlib $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

print:
	@echo OBJS: $(OBJS)

# ----------------------- 'Normal' makefile -------------- #

CELESTE_CC=$(CC)

OUTPUT=ccleste-mc.wasm

CELESTE_OBJ=celeste-mc.o
CFLAGS+=-DCELESTE_P8_FIXEDP

ifneq ($(HACKED_BALLOONS),)
	CFLAGS+=-DCELESTE_P8_HACKED_BALLOONS
endif

ccleste-mc.wat: ccleste-mc.wasm
	wasm2wat ccleste-mc.wasm -o ccleste-mc.wat

$(OUTPUT): mcmain.o newlib_stubs.o $(CELESTE_OBJ) $(OBJS2)
	@echo [Linking $@]
	$(VB)$(CC) $(CFLAGS) $(LDFLAGS) mcmain.o newlib_stubs.o $(CELESTE_OBJ) $(OBJS2) \
	-o $(OUTPUT) $(LIBS)

#	@echo [Size]
#	-$(CROSS_COMPILE)size $(OUTPUT)

$(CELESTE_OBJ): celeste2.c celeste.h
	$(CXX) $(CFLAGS) -c -o $(CELESTE_OBJ) celeste2.c

mcmain.o: mcmain.c mc_sdl_compat.h mc_sdl_bmp.h
	@echo [cflags are $(CFLAGS)]
	$(CELESTE_CC) $(CFLAGS) -c -o mcmain.o mcmain.c

newlib_stubs.o: newlib_stubs.c
	$(CELESTE_CC) $(CFLAGS) -c -o newlib_stubs.o newlib_stubs.c

clean:
	$(RM) ccleste ccleste-fixedp celeste.o celeste-fixedp.o newlib_stubs.o
	make -f Makefile.3ds clean

#	rm -rf $(OBJDIR)
#	rm -rf $(OBJDIR2)
#	rm -f $(OUTPUT)
#	rm -f $(OUTPUT).gdb
#	rm -f $(OUTPUT).map