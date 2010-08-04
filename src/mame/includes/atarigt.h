/*************************************************************************

    Atari GT hardware

*************************************************************************/

#include "machine/atarigen.h"


#define CRAM_ENTRIES		0x4000
#define TRAM_ENTRIES		0x4000
#define MRAM_ENTRIES		0x8000

class atarigt_state : public atarigen_state
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, atarigt_state(machine)); }

	atarigt_state(running_machine &machine)
		: atarigen_state(machine) { }

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

	void			(*protection_w)(const address_space *space, offs_t offset, UINT16 data);
	void			(*protection_r)(const address_space *space, offs_t offset, UINT16 *data);
};


/*----------- defined in video/atarigt.c -----------*/

void atarigt_colorram_w(atarigt_state *state, offs_t address, UINT16 data, UINT16 mem_mask);
UINT16 atarigt_colorram_r(atarigt_state *state, offs_t address);

VIDEO_START( atarigt );
VIDEO_UPDATE( atarigt );

void atarigt_scanline_update(screen_device &screen, int scanline);
