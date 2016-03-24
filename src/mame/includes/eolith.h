// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina,Pierpaolo Prazzoli

#include "cpu/mcs51/mcs51.h"
#include "sound/qs1000.h"

class eolith_state : public driver_device
{
public:
	eolith_state(const machine_config &mconfig, device_type type, const char *tag)
		:   driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_soundcpu(*this, "soundcpu"),
			m_qs1000(*this, "qs1000"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_in0(*this, "IN0"),
			m_eepromoutport(*this, "EEPROMOUT"),
			m_penx1port(*this, "PEN_X_P1"),
			m_peny1port(*this, "PEN_Y_P1"),
			m_penx2port(*this, "PEN_X_P2"),
			m_peny2port(*this, "PEN_Y_P2"),
			m_sndbank(*this, "sound_bank")
		{ }


	required_device<cpu_device> m_maincpu;
	optional_device<i8032_device> m_soundcpu;
	optional_device<qs1000_device> m_qs1000;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_ioport m_in0; // klondkp doesn't have it
	optional_ioport m_eepromoutport;
	optional_ioport m_penx1port;
	optional_ioport m_peny1port;
	optional_ioport m_penx2port;
	optional_ioport m_peny2port;

	optional_memory_bank m_sndbank;

	int m_coin_counter_bit;
	int m_buffer;
	std::unique_ptr<UINT32[]> m_vram;

	UINT8 m_sound_data;

	// speedups - see machine/eolithsp.c
	int m_speedup_address;
	int m_speedup_address2;
	int m_speedup_resume_scanline;
	int m_speedup_vblank;
	int m_speedup_scanline;
	void speedup_read();
	void init_speedup();
	DECLARE_CUSTOM_INPUT_MEMBER(eolith_speedup_getvblank);
	DECLARE_CUSTOM_INPUT_MEMBER(stealsee_speedup_getvblank);

	DECLARE_READ32_MEMBER(eolith_custom_r);
	DECLARE_WRITE32_MEMBER(systemcontrol_w);
	DECLARE_WRITE32_MEMBER(sound_w);
	DECLARE_READ32_MEMBER(hidctch3_pen1_r);
	DECLARE_READ32_MEMBER(hidctch3_pen2_r);
	DECLARE_WRITE32_MEMBER(eolith_vram_w);
	DECLARE_READ32_MEMBER(eolith_vram_r);
	DECLARE_READ8_MEMBER(sound_cmd_r);
	DECLARE_WRITE8_MEMBER(sound_p1_w);
	DECLARE_READ8_MEMBER(qs1000_p1_r);
	DECLARE_WRITE8_MEMBER(qs1000_p1_w);
	DECLARE_WRITE8_MEMBER(soundcpu_to_qs1000);

	DECLARE_DRIVER_INIT(eolith);
	DECLARE_DRIVER_INIT(landbrk);
	DECLARE_DRIVER_INIT(hidctch3);
	DECLARE_DRIVER_INIT(hidctch2);
	DECLARE_DRIVER_INIT(hidnc2k);
	DECLARE_DRIVER_INIT(landbrka);

	DECLARE_MACHINE_RESET(eolith);
	DECLARE_VIDEO_START(eolith);

	UINT32 screen_update_eolith(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(eolith_speedup);
};
