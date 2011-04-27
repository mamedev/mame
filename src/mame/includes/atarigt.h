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
	atarigt_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT8			m_is_primrage;
	UINT16 *		m_colorram;

	bitmap_t *		m_pf_bitmap;
	bitmap_t *		m_an_bitmap;

	UINT8			m_playfield_tile_bank;
	UINT8			m_playfield_color_bank;
	UINT16			m_playfield_xscroll;
	UINT16			m_playfield_yscroll;

	UINT32			m_tram_checksum;

	UINT32			m_expanded_mram[MRAM_ENTRIES * 3];

	UINT32 *		m_mo_command;

	void			(*m_protection_w)(address_space *space, offs_t offset, UINT16 data);
	void			(*m_protection_r)(address_space *space, offs_t offset, UINT16 *data);

	UINT8			m_ignore_writes;
	offs_t			m_protaddr[ADDRSEQ_COUNT];
	UINT8			m_protmode;
	UINT16			m_protresult;
	UINT8			m_protdata[0x800];

	device_t *		m_rle;
};


/*----------- defined in video/atarigt.c -----------*/

void atarigt_colorram_w(atarigt_state *state, offs_t address, UINT16 data, UINT16 mem_mask);
UINT16 atarigt_colorram_r(atarigt_state *state, offs_t address);

VIDEO_START( atarigt );
SCREEN_EOF( atarigt );
SCREEN_UPDATE( atarigt );

void atarigt_scanline_update(screen_device &screen, int scanline);
