/*************************************************************************

    Atari G42 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarig42_state : public atarigen_state
{
public:
	atarig42_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
		  m_mo_command(*this, "mo_command") { }

	UINT16			m_playfield_base;

	UINT16			m_current_control;
	UINT8			m_playfield_tile_bank;
	UINT8			m_playfield_color_bank;
	UINT16			m_playfield_xscroll;
	UINT16			m_playfield_yscroll;

	UINT8			m_analog_data;
	required_shared_ptr<UINT16> m_mo_command;

	int 			m_sloop_bank;
	int 			m_sloop_next_bank;
	int 			m_sloop_offset;
	int 			m_sloop_state;
	UINT16 *		m_sloop_base;

	device_t *		m_rle;
	UINT32			m_last_accesses[8];
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_WRITE16_MEMBER(a2d_select_w);
	DECLARE_READ16_MEMBER(a2d_data_r);
	DECLARE_WRITE16_MEMBER(io_latch_w);
	DECLARE_WRITE16_MEMBER(mo_command_w);
	DECLARE_READ16_MEMBER(roadriot_sloop_data_r);
	DECLARE_WRITE16_MEMBER(roadriot_sloop_data_w);
	DECLARE_READ16_MEMBER(guardians_sloop_data_r);
	DECLARE_WRITE16_MEMBER(guardians_sloop_data_w);
	void roadriot_sloop_tweak(int offset);
	void guardians_sloop_tweak(int offset);
	DECLARE_DIRECT_UPDATE_MEMBER(atarig42_sloop_direct_handler);
};


/*----------- defined in video/atarig42.c -----------*/

VIDEO_START( atarig42 );
SCREEN_VBLANK( atarig42 );
SCREEN_UPDATE_IND16( atarig42 );

WRITE16_HANDLER( atarig42_mo_control_w );

void atarig42_scanline_update(screen_device &screen, int scanline);

