#include "video/poly.h"

struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class galastrm_state : public driver_device
{
public:
	galastrm_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 coin_word;
	UINT16 frame_counter;
	UINT32 *ram;
	int tc0110pcr_addr;
	int tc0610_0_addr;
	int tc0610_1_addr;
	UINT32 mem[2];
	INT16 tc0610_ctrl_reg[2][8];
	struct tempsprite *spritelist;
	struct tempsprite *sprite_ptr_pre;
	bitmap_t *tmpbitmaps;
	bitmap_t *polybitmap;
	poly_manager *poly;
	int rsxb;
	int rsyb;
	int rsxoffs;
	int rsyoffs;
};


/*----------- defined in video/galastrm.c -----------*/

VIDEO_START( galastrm );
VIDEO_UPDATE( galastrm );
