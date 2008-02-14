#ifndef __VECTOR__
#define __VECTOR__

#define VECTOR_COLOR111(c) \
	MAKE_RGB(pal1bit((c) >> 2), pal1bit((c) >> 1), pal1bit((c) >> 0))

#define VECTOR_COLOR222(c) \
	MAKE_RGB(pal2bit((c) >> 4), pal2bit((c) >> 2), pal2bit((c) >> 0))

#define VECTOR_COLOR444(c) \
	MAKE_RGB(pal4bit((c) >> 8), pal4bit((c) >> 4), pal4bit((c) >> 0))

extern UINT8 *vectorram;
extern size_t vectorram_size;

VIDEO_START( vector );
VIDEO_UPDATE( vector );

void vector_clear_list(void);
void vector_add_point(int x, int y, rgb_t color, int intensity);
void vector_add_clip(int minx, int miny, int maxx, int maxy);

void vector_set_flicker(float _flicker);
float vector_get_flicker(void);

void vector_set_beam(float _beam);
float vector_get_beam(void);

#endif

