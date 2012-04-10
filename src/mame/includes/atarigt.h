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

	bitmap_ind16 *		m_pf_bitmap;
	bitmap_ind16 *		m_an_bitmap;

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
	DECLARE_READ32_MEMBER(special_port2_r);
	DECLARE_READ32_MEMBER(special_port3_r);
	DECLARE_READ32_MEMBER(analog_port0_r);
	DECLARE_READ32_MEMBER(analog_port1_r);
	DECLARE_WRITE32_MEMBER(latch_w);
	DECLARE_WRITE32_MEMBER(mo_command_w);
	DECLARE_WRITE32_MEMBER(led_w);
	DECLARE_READ32_MEMBER(sound_data_r);
	DECLARE_WRITE32_MEMBER(sound_data_w);
	DECLARE_READ32_MEMBER(colorram_protection_r);
	DECLARE_WRITE32_MEMBER(colorram_protection_w);
	DECLARE_WRITE32_MEMBER(tmek_pf_w);
	void atarigt_colorram_w(offs_t address, UINT16 data, UINT16 mem_mask);
	UINT16 atarigt_colorram_r(offs_t address);
};


/*----------- defined in video/atarigt.c -----------*/



VIDEO_START( atarigt );
SCREEN_VBLANK( atarigt );
SCREEN_UPDATE_RGB32( atarigt );

void atarigt_scanline_update(screen_device &screen, int scanline);
