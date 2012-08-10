/*************************************************************************

    Atari GX2 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarigx2_state : public atarigen_state
{
public:
	atarigx2_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
		  m_mo_command(*this, "mo_command"),
		  m_protection_base(*this, "protection_base") { }

	UINT16			m_playfield_base;

	required_shared_ptr<UINT32> m_mo_command;
	required_shared_ptr<UINT32> m_protection_base;

	UINT16			m_current_control;
	UINT8			m_playfield_tile_bank;
	UINT8			m_playfield_color_bank;
	UINT16			m_playfield_xscroll;
	UINT16			m_playfield_yscroll;

	UINT16			m_last_write;
	UINT16			m_last_write_offset;

	device_t *		m_rle;
	DECLARE_READ32_MEMBER(special_port2_r);
	DECLARE_READ32_MEMBER(special_port3_r);
	DECLARE_READ32_MEMBER(a2d_data_r);
	DECLARE_WRITE32_MEMBER(latch_w);
	DECLARE_WRITE32_MEMBER(mo_command_w);
	DECLARE_WRITE32_MEMBER(atarigx2_protection_w);
	DECLARE_READ32_MEMBER(atarigx2_protection_r);
	DECLARE_READ32_MEMBER(rrreveng_prot_r);
	DECLARE_DRIVER_INIT(spclords);
	DECLARE_DRIVER_INIT(rrreveng);
	DECLARE_DRIVER_INIT(motofren);
};


/*----------- defined in video/atarigx2.c -----------*/

VIDEO_START( atarigx2 );
SCREEN_VBLANK( atarigx2 );
SCREEN_UPDATE_IND16( atarigx2 );

WRITE16_HANDLER( atarigx2_mo_control_w );

void atarigx2_scanline_update(screen_device &screen, int scanline);
