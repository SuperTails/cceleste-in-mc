#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <stdbool.h>

#include <reent.h>

#include "data_font.h"
#include "data_gfx.h"

extern void turtle_x(int);
extern void turtle_y(int);
extern void turtle_z(int);
extern void turtle_set(int);

extern void mc_putc(uint8_t c);
extern void mc_sleep(void);

extern int main(int argc, char **argv);

void _start() {
  main(0, NULL);

  while (1) {
    mc_sleep();   
  }
}



/*uint32_t ticks_ms = 0;

void DG_DrawFrame()
{
	// RGB888

  turtle_z(-20);

  for (int y = 0; y < DOOMGENERIC_RESY; ++y) {
    turtle_y(DOOMGENERIC_RESY - y);
    for (int x = 0; x < DOOMGENERIC_RESX; ++x) {
      turtle_x(DOOMGENERIC_RESX - x);
      
      uint32_t rgb = DG_ScreenBuffer[DOOMGENERIC_RESX * y + x];
      turtle_set(rgb % 13);
    }
  }

  //ticks_ms += 16;
}

void DG_SleepMs(uint32_t ms)
{
  //SDL_Delay(ms);
}

uint32_t DG_GetTicksMs()
{
  ticks_ms += 1;
  return ticks_ms;
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
  return 0;

  if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex){
    //key queue is empty
    return 0;
  }else{
    unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
    s_KeyQueueReadIndex++;
    s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

    *pressed = keyData >> 8;
    *doomKey = keyData & 0xFF;

    return 1;
  }

  return 0;
}

void DG_SetWindowTitle(const char * title) {}*/

#define MAX_FDS 16

typedef struct {
  off_t offset;
  bool valid;

  uint8_t *data;
  off_t size;
} fd_info;

fd_info fd_table[MAX_FDS] = {};

int next_fd = 3;

#define STDOUT_FD 0
#define STDERR_FD 1
#define STDIN_FD 2

ssize_t _read(int fd, void *buf, size_t count) {
  uint8_t *buf_bytes = (uint8_t *)buf;

  if (fd < 0 || fd >= MAX_FDS) {
    errno = EBADF;
    return -1;
  }

  fd_info *info = &fd_table[fd];
  if (!info->valid) {
    errno = EBADF;
    return -1;
  }

  ssize_t num_read = 0;
  while (count > 0) {
    if (info->offset == info->size) {
      break;
    }

    *buf_bytes++ = info->data[info->offset++];
    --count;
    ++num_read;
  }

  return num_read;
}

ssize_t _write(int fd, const void *buf, size_t count) {
  if (fd == STDOUT_FD || fd == STDERR_FD) {
    uint8_t *buf_bytes = (uint8_t*)buf;
    for (size_t i = 0; i < count; ++i) {
      mc_putc(buf_bytes[i]);
    }
    return count;
  } else {
    errno = ENOSYS;
    return -1;
  }
}

#define MEMORY_SIZE (1 * 65536)

uint8_t memory[MEMORY_SIZE] = {};

uint8_t *next = memory;

void *sbrk(intptr_t increment) {
  uint8_t *result = next;

  intptr_t current_size = (intptr_t)next - (intptr_t)memory;

  if (current_size + increment >= MEMORY_SIZE) {
    errno = ENOMEM;
    return NULL;
  }

  next += increment;

  return result;
}

void _exit(int code) {
  const char msg[] = "exit\n";
  _write(STDOUT_FD, msg, sizeof(msg) - 1);

  // TODO:
  while (1) {}
}

int _open(const char *pathname, int flags, mode_t mode) {
  if (strcmp(pathname, "data/gfx.bmp") == 0 && next_fd < MAX_FDS) {
    int fd = next_fd++;
    fd_table[fd].valid = true;
    fd_table[fd].offset = 0;
    fd_table[fd].data = (uint8_t*)data_gfx_file;
    fd_table[fd].size = data_gfx_fsize;
    return fd;
  }

  if (strcmp(pathname, "data/font.bmp") == 0 && next_fd < MAX_FDS) {
    int fd = next_fd++;
    fd_table[fd].valid = true;
    fd_table[fd].offset = 0;
    fd_table[fd].data = (uint8_t*)data_font_file;
    fd_table[fd].size = data_font_fsize;
    return fd;
  }

  printf("ignoring open %s %d\n", pathname, next_fd);
  errno = ENOSYS;
  return -1;
}

int _close(int fd) {
  if (0 <= fd && fd < MAX_FDS) {
    if (fd_table[fd].valid) {
      fd_table[fd].valid = false;
      return 0;
    } else {
      errno = EBADF;
      return -1;
    }
  }

  const char msg[] = "ignoring close\n";
  _write(STDOUT_FD, msg, sizeof(msg) - 1);


  errno = ENOSYS;
  return -1;
}

int _link(const char *oldpath, const char *newpath) {
  const char msg[] = "ignoring link\n";
  _write(STDOUT_FD, msg, sizeof(msg) - 1);

  errno = ENOSYS;
  return -1;
}

int _unlink(const char *pathname) {
  const char msg[] = "ignoring unlink\n";
  _write(STDOUT_FD, msg, sizeof(msg) - 1);

  errno = ENOSYS;
  return -1;
}

off_t _lseek(int fd, off_t offset, int whence) {
  if (fd < 0 || fd >= MAX_FDS) {
    errno = EBADF;
    return (off_t)-1;
  }

  fd_info *info = &fd_table[fd];
  if (!info->valid) {
    errno = EBADF;
    return (off_t)-1;
  }

  off_t new_offset;

  switch (whence) {
  case SEEK_SET:
    new_offset = offset;
    break;
  case SEEK_CUR:
    new_offset = info->offset + offset;
    break;
  case SEEK_END:
    new_offset = info->size + offset;
    break;
  default:
    errno = EINVAL;
    return (off_t)-1;
  }

  if (new_offset < 0 || new_offset > info->size) {
    errno = EINVAL;
    return (off_t)-1;
  }

  info->offset = new_offset;
  return info->offset;
}

void *_calloc_r(struct _reent* r, size_t num, size_t size) {
  const char msg[] = "ignoring calloc\n";
  _write(STDOUT_FD, msg, sizeof(msg) - 1);


  return calloc(num, size);
}

void *_malloc_r(struct _reent* r, size_t size) {
  const char msg[] = "malloc of size ";
  _write(STDOUT_FD, msg, sizeof(msg) - 1);
  for (int i = 0; i < sizeof(size_t) * 2; ++i) {
    int off = sizeof(size_t) * 2 - 1 - i;
    const char m1 = (size >> (off * 4)) & 0xF;
    char m2;
    if (m1 < 0xA) {
      m2 = m1 + '0';
    } else {
      m2 = m1 - 0xA + 'A';
    }
    _write(STDOUT_FD, &m2, 1);
  }
  _write(STDOUT_FD, "\n", 1);

  return malloc(size);
}

void _free_r(struct _reent* r, void *ptr) {
  free(ptr);
}

void *_realloc_r(struct _reent* r, void *ptr, size_t new_size) {
  return realloc(ptr, new_size);
}

long double __extenddftf2(double a) {
  return 0.0;
}

double __trunctfdf2(long double a) {
  return 0.0;
}

int mkdir(const char *pathname, mode_t mode) {
  const char msg[] = "ignoring mkdir\n";
  _write(STDOUT_FD, msg, sizeof(msg) - 1);

  errno = ENOSYS;
  return -1;
}

int _fstat(int fd, struct stat *statbuf) {
  if (fd < 0 || fd >= MAX_FDS) {
    errno = EBADF;
    return -1;
  }
  
  fd_info *info = &fd_table[fd];
  if (!info->valid) {
    errno = EBADF;
    return -1;
  }

  //statbuf->st_mode = ???
  statbuf->st_nlink = 1;
  statbuf->st_size = info->size;
  statbuf->st_blksize = 512;
  statbuf->st_blocks = info->size / (512 * 1024);

  return 0;
}

int raise(int sig) {
  const char msg[] = "signal raised\n";
  _write(STDOUT_FD, msg, sizeof(msg) - 1);

  // TODO:
  while (1) {}
}

#ifdef __cplusplus
}
#endif