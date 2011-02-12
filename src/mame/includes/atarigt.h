/*************************************************************************

    Atari GT hardware

*************************************************************************/

#include "machine/atarigen.h"


#define CRAM_ENTRIES		0x4000
#define TRAM_ENTRIES		0x4000
#define MRAM_ENTRIES		0x8000

#define ADDRSEQ_COUNT	4

class atarigt_state : public atarigen_state
{
public:
	atarigt_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT8			is_primrage;
	UINT16 *		colorram;

	bitmap_t *		pf_bitmap;
	bitmap_t *		an_bitmap;

	UINT8			playfield_tile_bank;
	UINT8			playfield_color_bank;
	UINT16			playfield_xscroll;
	UINT16			playfield_yscroll;

	UINT32			tram_checksum;

	UINT32			expanded_mram[MRAM_ENTRIES * 3];

	UINT32 *		mo_command;

	void			(*protection_w)(address_space *space, offs_t offset, UINT16 data);
	void			(*protection_r)(address_space *space, offs_t offset, UINT16 *data);

	UINT8			ignore_writes;
	offs_t			protaddr[ADDRSEQ_COUNT];
	UINT8			protmode;
	UINT16			protresult;
	UINT8			protdata[0x800];

	device_t *		rle;
};


/*----------- defined in video/atarigt.c -----------*/

void atarigt_colorram_w(atarigt_state *state, offs_t address, UINT16 data, UINT16 mem_mask);
UINT16 atarigt_colorram_r(atarigt_state *state, offs_t address);

VIDEO_START( atarigt );
VIDEO_EOF( atarigt );
VIDEO_UPDATE( atarigt );

void atarigt_scanline_update(screen_device &screen, int scanline);
