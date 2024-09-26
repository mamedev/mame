// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_EDEVICES_FANTLAND_H
#define MAME_EDEVICES_FANTLAND_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "screen.h"

class fantland_state : public driver_device
{
public:
	fantland_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram", 0x2800, ENDIANNESS_LITTLE),
		m_spriteram2(*this, "spriteram2", 0x10000, ENDIANNESS_LITTLE),
		m_wheel(*this, "WHEEL%u", 0U)
	{ }

	void fantland(machine_config &config);
	void wheelrun(machine_config &config);
	void galaxygn(machine_config &config);

	template <int Player> ioport_value wheelrun_wheel_r();

protected:
	/* misc */
	uint8_t    m_nmi_enable = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	memory_share_creator<uint8_t> m_spriteram;
	memory_share_creator<uint8_t> m_spriteram2;

	optional_ioport_array<2> m_wheel;

	void nmi_enable_w(uint8_t data);
	void soundlatch_w(uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

private:
	uint8_t spriteram_r(offs_t offset);
	uint8_t spriteram2_r(offs_t offset);
	void spriteram_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	void spriteram2_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	void galaxygn_sound_irq(int state);
	INTERRUPT_GEN_MEMBER(fantland_sound_irq);
	void fantland_map(address_map &map) ATTR_COLD;
	void fantland_sound_iomap(address_map &map) ATTR_COLD;
	void fantland_sound_map(address_map &map) ATTR_COLD;
	void galaxygn_map(address_map &map) ATTR_COLD;
	void galaxygn_sound_iomap(address_map &map) ATTR_COLD;
	void wheelrun_map(address_map &map) ATTR_COLD;
	void wheelrun_sound_map(address_map &map) ATTR_COLD;
};

class borntofi_state : public fantland_state
{
public:
	borntofi_state(const machine_config &mconfig, device_type type, const char *tag) :
		fantland_state(mconfig, type, tag),
		m_msm(*this, "msm%u", 1U),
		m_adpcm_rom(*this, "adpcm")
	{
	}

	void borntofi(machine_config &config);

private:
	/* misc */
	int        m_old_x[2]{};
	int        m_old_y[2]{};
	int        m_old_f[2]{};
	uint8_t    m_input_ret[2]{};
	int        m_adpcm_playing[4]{};
	int        m_adpcm_addr[2][4]{};
	int        m_adpcm_nibble[4]{};

	/* devices */
	required_device_array<msm5205_device, 4> m_msm;
	required_region_ptr<uint8_t> m_adpcm_rom;

	uint8_t inputs_r(offs_t offset);
	void msm5205_w(offs_t offset, uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	template<int Voice> void adpcm_int(int state);
	void adpcm_start(int voice);
	void adpcm_stop(int voice);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_EDEVICES_FANTLAND_H
