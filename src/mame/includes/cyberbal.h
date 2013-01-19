/*************************************************************************

    Atari Cyberball hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "sound/dac.h"

class cyberbal_state : public atarigen_state
{
public:
	cyberbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_extracpu(*this, "extra"),
			m_daccpu(*this, "dac"),
			m_dac1(*this, "dac1"),
			m_dac2(*this, "dac2"),
			m_paletteram_0(*this, "paletteram_0"),
			m_paletteram_1(*this, "paletteram_1") { }

	required_device<m68000_device> m_maincpu;
	optional_device<m6502_device> m_audiocpu;
	optional_device<m68000_device> m_extracpu;
	optional_device<m68000_device> m_daccpu;
	optional_device<dac_device> m_dac1;
	optional_device<dac_device> m_dac2;
	optional_shared_ptr<UINT16> m_paletteram_0;
	optional_shared_ptr<UINT16> m_paletteram_1;
	UINT16          m_current_slip[2];
	UINT8           m_playfield_palette_bank[2];
	UINT16          m_playfield_xscroll[2];
	UINT16          m_playfield_yscroll[2];

	UINT8 *         m_bank_base;
	UINT8           m_fast_68k_int;
	UINT8           m_io_68k_int;
	UINT8           m_sound_data_from_68k;
	UINT8           m_sound_data_from_6502;
	UINT8           m_sound_data_from_68k_ready;
	UINT8           m_sound_data_from_6502_ready;
	virtual void update_interrupts();
	virtual void scanline_update(screen_device &screen, int scanline);
	DECLARE_READ16_MEMBER(special_port0_r);
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_READ16_MEMBER(sound_state_r);
	DECLARE_WRITE16_MEMBER(p2_reset_w);
	DECLARE_READ8_MEMBER(special_port3_r);
	DECLARE_READ8_MEMBER(sound_6502_stat_r);
	DECLARE_WRITE8_MEMBER(sound_bank_select_w);
	DECLARE_READ8_MEMBER(sound_68k_6502_r);
	DECLARE_WRITE8_MEMBER(sound_68k_6502_w);
	DECLARE_WRITE16_MEMBER(io_68k_irq_ack_w);
	DECLARE_READ16_MEMBER(sound_68k_r);
	DECLARE_WRITE16_MEMBER(sound_68k_w);
	DECLARE_WRITE16_MEMBER(sound_68k_dac_w);
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
	UINT32 screen_update_cyberbal_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cyberbal_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cyberbal2p(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sound_68k_irq_gen);
	DECLARE_READ16_MEMBER(paletteram_0_r);
	DECLARE_READ16_MEMBER(paletteram_1_r);
	DECLARE_WRITE16_MEMBER(paletteram_0_w);
	DECLARE_WRITE16_MEMBER(paletteram_1_w);
private:
	void video_start_common(int screens);
	void cyberbal_sound_reset();
	UINT32 update_one_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index);
	void update_sound_68k_interrupts();
};
