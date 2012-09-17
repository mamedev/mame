/*************************************************************************

    Atari Cyberball hardware

*************************************************************************/

#include "machine/atarigen.h"

class cyberbal_state : public atarigen_state
{
public:
	cyberbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
		  m_paletteram_0(*this, "paletteram_0"),
		  m_paletteram_1(*this, "paletteram_1") { }

	optional_shared_ptr<UINT16> m_paletteram_0;
	optional_shared_ptr<UINT16> m_paletteram_1;
	UINT16			m_current_slip[2];
	UINT8			m_playfield_palette_bank[2];
	UINT16			m_playfield_xscroll[2];
	UINT16			m_playfield_yscroll[2];

	UINT8 *			m_bank_base;
	UINT8			m_fast_68k_int;
	UINT8			m_io_68k_int;
	UINT8			m_sound_data_from_68k;
	UINT8			m_sound_data_from_6502;
	UINT8			m_sound_data_from_68k_ready;
	UINT8			m_sound_data_from_6502_ready;
	DECLARE_READ16_MEMBER(special_port0_r);
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_READ16_MEMBER(sound_state_r);
	DECLARE_WRITE16_MEMBER(p2_reset_w);
	DECLARE_READ8_MEMBER(cyberbal_special_port3_r);
	DECLARE_READ8_MEMBER(cyberbal_sound_6502_stat_r);
	DECLARE_WRITE8_MEMBER(cyberbal_sound_bank_select_w);
	DECLARE_READ8_MEMBER(cyberbal_sound_68k_6502_r);
	DECLARE_WRITE8_MEMBER(cyberbal_sound_68k_6502_w);
	DECLARE_WRITE16_MEMBER(cyberbal_io_68k_irq_ack_w);
	DECLARE_READ16_MEMBER(cyberbal_sound_68k_r);
	DECLARE_WRITE16_MEMBER(cyberbal_sound_68k_w);
	DECLARE_WRITE16_MEMBER(cyberbal_sound_68k_dac_w);
	DECLARE_DRIVER_INIT(cyberbalt);
	DECLARE_DRIVER_INIT(cyberbal2p);
	DECLARE_DRIVER_INIT(cyberbal);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_alpha2_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	DECLARE_MACHINE_START(cyberbal);
	DECLARE_MACHINE_RESET(cyberbal);
	DECLARE_VIDEO_START(cyberbal);
	DECLARE_MACHINE_RESET(cyberbal2p);
	DECLARE_VIDEO_START(cyberbal2p);
};



/*----------- defined in audio/cyberbal.c -----------*/

void cyberbal_sound_reset(running_machine &machine);

INTERRUPT_GEN( cyberbal_sound_68k_irq_gen );




/*----------- defined in video/cyberbal.c -----------*/

DECLARE_READ16_HANDLER( cyberbal_paletteram_0_r );
DECLARE_READ16_HANDLER( cyberbal_paletteram_1_r );
DECLARE_WRITE16_HANDLER( cyberbal_paletteram_0_w );
DECLARE_WRITE16_HANDLER( cyberbal_paletteram_1_w );



SCREEN_UPDATE_IND16( cyberbal_left );
SCREEN_UPDATE_IND16( cyberbal_right );
SCREEN_UPDATE_IND16( cyberbal2p );

void cyberbal_scanline_update(screen_device &screen, int scanline);
