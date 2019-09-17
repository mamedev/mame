// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina,Pierpaolo Prazzoli

#include "cpu/mcs51/mcs51.h"
#include "machine/timer.h"
#include "sound/qs1000.h"
#include "emupal.h"
#include "screen.h"

class eolith_state : public driver_device
{
public:
	eolith_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_qs1000(*this, "qs1000")
		, m_eepromoutport(*this, "EEPROMOUT")
		, m_soundcpu(*this, "soundcpu")
		, m_in0(*this, "IN0")
		, m_penxport(*this, "PEN_X_P%u", 1)
		, m_penyport(*this, "PEN_Y_P%u", 1)
		, m_led(*this, "led0")
		, m_sndbank(*this, "sound_bank")
	{
	}

	void ironfort(machine_config &config);
	void eolith50(machine_config &config);
	void eolith45(machine_config &config);
	void hidctch3(machine_config &config);

	void init_eolith();
	void init_landbrk();
	void init_hidctch2();
	void init_hidnc2k();
	void init_landbrka();
	void init_landbrkb();

	DECLARE_READ_LINE_MEMBER(speedup_vblank_r);
	DECLARE_READ_LINE_MEMBER(stealsee_speedup_vblank_r);

	void speedup_read();
	void init_speedup();

protected:

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<qs1000_device> m_qs1000;
	optional_ioport m_eepromoutport;
	TIMER_DEVICE_CALLBACK_MEMBER(eolith_speedup);

private:

	DECLARE_READ32_MEMBER(eolith_custom_r);
	DECLARE_WRITE32_MEMBER(systemcontrol_w);
	template<int Player> DECLARE_READ32_MEMBER(hidctch3_pen_r);
	DECLARE_WRITE16_MEMBER(eolith_vram_w);
	DECLARE_READ16_MEMBER(eolith_vram_r);
	DECLARE_WRITE8_MEMBER(sound_p1_w);
	DECLARE_READ8_MEMBER(qs1000_p1_r);
	DECLARE_WRITE8_MEMBER(qs1000_p1_w);
	DECLARE_WRITE8_MEMBER(soundcpu_to_qs1000);

	DECLARE_MACHINE_RESET(eolith);
	DECLARE_VIDEO_START(eolith);


	uint32_t screen_update_eolith(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void eolith_map(address_map &map);
	void hidctch3_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_prg_map(address_map &map);

	virtual void machine_start() override;
	// shared with eolith16.cpp, vegaeo.cpp

	void patch_mcu_protection(uint32_t address);

	optional_device<i8032_device> m_soundcpu;

	optional_ioport m_in0; // klondkp doesn't have it
	optional_ioport_array<2> m_penxport;
	optional_ioport_array<2> m_penyport;
	output_finder<> m_led;

	optional_memory_bank m_sndbank;

	int m_coin_counter_bit;
	std::unique_ptr<uint16_t[]> m_vram;
	int m_buffer;

	// speedups - see machine/eolithsp.c
	int m_speedup_address;
	int m_speedup_address2;
	int m_speedup_resume_scanline;
	int m_speedup_vblank;
	int m_speedup_scanline;
};
