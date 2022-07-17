#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

extern int _open(const char *pathname, int flags, mode_t mode);
extern off_t _lseek(int fd, off_t offset, int whence);
extern int _close(int fd);
extern ssize_t _read(int fd, void *buf, size_t count);

extern void mc_sleep(void);

typedef uint32_t DWORD;
typedef uint16_t WORD;

typedef int32_t LONG;

typedef int32_t FXPT2DOT30;

typedef struct tagCIEXYZ {
  FXPT2DOT30 ciexyzX;
  FXPT2DOT30 ciexyzY;
  FXPT2DOT30 ciexyzZ;
} CIEXYZ;

typedef struct tagICEXYZTRIPLE {
  CIEXYZ ciexyzRed;
  CIEXYZ ciexyzGreen;
  CIEXYZ ciexyzBlue;
} CIEXYZTRIPLE;

typedef struct {
  DWORD        bV4Size;
  LONG         bV4Width;
  LONG         bV4Height;
  WORD         bV4Planes;
  WORD         bV4BitCount;
  DWORD        bV4V4Compression;
  DWORD        bV4SizeImage;
  LONG         bV4XPelsPerMeter;
  LONG         bV4YPelsPerMeter;
  DWORD        bV4ClrUsed;
  DWORD        bV4ClrImportant;
  DWORD        bV4RedMask;
  DWORD        bV4GreenMask;
  DWORD        bV4BlueMask;
  DWORD        bV4AlphaMask;
  DWORD        bV4CSType;
  CIEXYZTRIPLE bV4Endpoints;
  DWORD        bV4GammaRed;
  DWORD        bV4GammaGreen;
  DWORD        bV4GammaBlue;
} BITMAPV4HEADER;

SDL_Surface *SDL_LoadBMP(const char *path) {
	int f = _open(path, 0, O_RDONLY);
	if (f < 0) {
		printf("error opening file\n");
		return 0;
	}

	uint16_t magic;
	if (_read(f, &magic, sizeof(magic)) != sizeof(magic)) {
		printf("couldn't read enough for magic\n");
		return 0;
	}
	// magic should be 0x4D42

	uint32_t size;
	if (_read(f, &size, sizeof(size)) != sizeof(size)) {
		printf("couldn't read enough for size\n");
		return 0;
	}

	uint32_t reserved;
	if (_read(f, &reserved, sizeof(reserved)) != sizeof(reserved)) {
		printf("couldn't read enough for reserved\n");
		return 0;
	}

	uint32_t offset;
	if (_read(f, &offset, sizeof(offset)) != sizeof(offset)) {
		printf("couldn't read enough for offset\n");
		return 0;
	}

	uint32_t infohdrsize;
	if (_read(f, &infohdrsize, sizeof(infohdrsize)) != sizeof(infohdrsize)) {
		printf("couldn't read enough for infohdrsize\n");
		return 0;
	}

	if (infohdrsize != 108) {
		printf("wrong infohdrsize %d\n", infohdrsize);
		return 0;
	}

	BITMAPV4HEADER bmp_hdr;

	void *ptr = ((char*)&bmp_hdr) + 4;

	if (_read(f, ptr, 104) != 104) {
		printf("Couldn't read enough for bmp_hdr\n");
		return 0;
	}

	mc_sleep();

	int num_bytes;
	if (bmp_hdr.bV4BitCount == 4) {
		num_bytes = (bmp_hdr.bV4Width * bmp_hdr.bV4Height) / 2;
	} else if (bmp_hdr.bV4BitCount == 1) {
		num_bytes = (bmp_hdr.bV4Width * bmp_hdr.bV4Height) / 8;
	}

	uint8_t *buf = malloc(num_bytes);
	if (!buf) {
		printf("couldn't allocate buffer\n");
		return 0;
	}

	if (_lseek(f, offset, SEEK_SET) < 0) {
		printf("lseek error\n");
		return 0;
	}

	if (_read(f, buf, num_bytes) != num_bytes) {
		printf("couldn't read pixel data\n");
		return 0;
	}

	if (_close(f) < 0) {
		printf("couldn't close file\n");
		return 0;
	}

	mc_sleep();

	uint8_t *pixels = malloc(bmp_hdr.bV4Width * bmp_hdr.bV4Height);

	if (bmp_hdr.bV4BitCount == 4) {
		for (int y = 0; y < bmp_hdr.bV4Height; ++y) {
			mc_sleep();

			for (int x = 0; x < bmp_hdr.bV4Width; ++x) {
				int offset = (x + bmp_hdr.bV4Width * y) / 2;
				int need_lo = x % 2;

				uint8_t *dst = &pixels[(bmp_hdr.bV4Height - 1 - y) * bmp_hdr.bV4Width + x];

				uint8_t src = buf[offset];

				if (need_lo) {
					*dst = src & 0xF;
				} else {
					*dst = src >> 4;
				}
			}
		}
	} else if (bmp_hdr.bV4BitCount == 1) {
		for (int y = 0; y < bmp_hdr.bV4Height; ++y) {
			mc_sleep();

			for (int x = 0; x < bmp_hdr.bV4Width; ++x) {
				int offset = (x + bmp_hdr.bV4Width * y) / 8;
				int need_lo = x % 8;

				uint8_t *dst = &pixels[(bmp_hdr.bV4Height - 1 - y) * bmp_hdr.bV4Width + x];

				uint8_t src = buf[offset];

				*dst = ((src >> (7 - need_lo)) & 0x1);
			}
		}
	} else {
		printf("invalid bit count\n");
		return 0;
	}

	free(buf);

	mc_sleep();

	SDL_Surface *surf = (SDL_Surface*)SDL_malloc(sizeof(SDL_Surface));
	if (surf == 0) {
		free(buf);
		SDL_free(pixels);
		return 0;
	}

	SDL_PixelFormat *format = (SDL_PixelFormat*)SDL_malloc(sizeof(SDL_PixelFormat));
	if (format == 0) {
		free(buf);
		SDL_free(pixels);
		SDL_free(surf);
		return 0;
	}

	format->BitsPerPixel = 8;
	format->BytesPerPixel = 1;

	surf->w = bmp_hdr.bV4Width;
	surf->h = bmp_hdr.bV4Height;

	surf->clip_rect.x = 0;
	surf->clip_rect.y = 0;
	surf->clip_rect.w = surf->w;
	surf->clip_rect.h = surf->h;

	surf->locked = 0;
	surf->pixels = pixels;
	surf->format = format;
	surf->pitch = surf->w * surf->format->BytesPerPixel;

	return surf;
}