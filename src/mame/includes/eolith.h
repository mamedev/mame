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
	std::unique_ptr<uint32_t[]> m_vram;

	uint8_t m_sound_data;

	// speedups - see machine/eolithsp.c
	int m_speedup_address;
	int m_speedup_address2;
	int m_speedup_resume_scanline;
	int m_speedup_vblank;
	int m_speedup_scanline;
	void speedup_read();
	void init_speedup();
	ioport_value eolith_speedup_getvblank(ioport_field &field, void *param);
	ioport_value stealsee_speedup_getvblank(ioport_field &field, void *param);

	uint32_t eolith_custom_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void systemcontrol_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void sound_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t hidctch3_pen1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t hidctch3_pen2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void eolith_vram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t eolith_vram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint8_t sound_cmd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_p1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t qs1000_p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void qs1000_p1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void soundcpu_to_qs1000(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_eolith();
	void init_landbrk();
	void init_hidctch3();
	void init_hidctch2();
	void init_hidnc2k();
	void init_landbrka();

	void machine_reset_eolith();
	void video_start_eolith();

	uint32_t screen_update_eolith(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void eolith_speedup(timer_device &timer, void *ptr, int32_t param);
};
