// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, Uki
#ifndef MAME_VSYSTEM_FROMANC2_H
#define MAME_VSYSTEM_FROMANC2_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/eepromser.h"
#include "machine/ins8250.h"
#include "emupal.h"
#include "tilemap.h"


class fromanc2_base_state : public driver_device
{
public:
	int sndcpu_nmi_r();

protected:
	fromanc2_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode")
		, m_lpalette(*this, "lpalette")
		, m_rpalette(*this, "rpalette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch2(*this, "soundlatch2")
		, m_in_key(*this, "KEY%u", 0U)
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void sndcmd_w(uint16_t data);
	void portselect_w(uint16_t data);
	uint16_t keymatrix_r();
	uint8_t sndcpu_nmi_clr();

	template <int VRAM, int Layer> TILE_GET_INFO_MEMBER(fromancr_get_tile_info);
	uint32_t screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sound_io_map(address_map &map);
	void sound_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_lpalette;
	required_device<palette_device> m_rpalette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	required_ioport_array<4> m_in_key;

	// memory pointers
	std::unique_ptr<uint16_t[]> m_videoram[2][4]{};

	// video-related
	tilemap_t  *m_tilemap[2][4]{};
	int32_t    m_scrollx[2][4]{};
	int32_t    m_scrolly[2][4]{};
	uint32_t   m_gfxbank[2][4]{};

	// misc
	uint16_t m_portselect = 0U;
	uint8_t  m_sndcpu_nmi_flag = 0U;
};


class fromanc2_state : public fromanc2_base_state
{
public:
	fromanc2_state(const machine_config &mconfig, device_type type, const char *tag)
		: fromanc2_base_state(mconfig, type, tag)
		, m_subcpu(*this, "sub")
		, m_subrombank(*this, "subrombank")
		, m_subrambank(*this, "subrambank")
	{ }

	void fromanc2(machine_config &config);
	void fromancr(machine_config &config);

	void init_fromanc2();

	int subcpu_int_r();
	int subcpu_nmi_r();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void subcpu_w(uint16_t data);
	uint16_t subcpu_r();
	uint8_t maincpu_r_l();
	uint8_t maincpu_r_h();
	void maincpu_w_l(uint8_t data);
	void maincpu_w_h(uint8_t data);
	void subcpu_nmi_clr(uint8_t data);
	void subcpu_rombank_w(uint8_t data);
	void fromancr_gfxbank_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromanc2_videoram_0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromanc2_videoram_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromanc2_videoram_2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromanc2_videoram_3_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromanc2_gfxreg_0_w(offs_t offset, uint16_t data);
	void fromanc2_gfxreg_1_w(offs_t offset, uint16_t data);
	void fromanc2_gfxreg_2_w(offs_t offset, uint16_t data);
	void fromanc2_gfxreg_3_w(offs_t offset, uint16_t data);
	void fromanc2_gfxbank_0_w(uint16_t data);
	void fromanc2_gfxbank_1_w(uint16_t data);
	void fromancr_videoram_0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromancr_videoram_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromancr_videoram_2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromancr_gfxreg_0_w(offs_t offset, uint16_t data);
	void fromancr_gfxreg_1_w(offs_t offset, uint16_t data);

	template<int VRAM, int Layer> TILE_GET_INFO_MEMBER(fromanc2_get_tile_info);
	DECLARE_VIDEO_START(fromanc2);
	DECLARE_VIDEO_START(fromancr);

	void fromanc2_dispvram_w(offs_t offset, uint16_t data, uint16_t mem_mask, int vram, int layer);
	void fromancr_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask, int layer);
	void fromancr_gfxbank_w(int data);

	void fromanc2_main_map(address_map &map);
	void fromanc2_sub_io_map(address_map &map);
	void fromanc2_sub_map(address_map &map);
	void fromancr_main_map(address_map &map);

	// devices
	required_device<cpu_device> m_subcpu;
	required_memory_bank m_subrombank;
	required_memory_bank m_subrambank;

	// memory pointers
	std::unique_ptr<uint8_t[]> m_bankedram{};

	// misc
	uint8_t    m_subcpu_int_flag = 0U;
	uint8_t    m_subcpu_nmi_flag = 0U;
	uint16_t   m_datalatch1 = 0U;
	uint8_t    m_datalatch_2h = 0U;
	uint8_t    m_datalatch_2l = 0U;
};


class fromanc4_state : public fromanc2_base_state
{
public:
	fromanc4_state(const machine_config &mconfig, device_type type, const char *tag)
		: fromanc2_base_state(mconfig, type, tag)
		, m_uart(*this, "uart")
	{ }

	void fromanc4(machine_config &config);

	void init_fromanc4();

protected:
	virtual void video_start() override;

	void fromanc4_videoram_0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromanc4_videoram_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromanc4_videoram_2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fromanc4_gfxreg_0_w(offs_t offset, uint16_t data);
	void fromanc4_gfxreg_1_w(offs_t offset, uint16_t data);
	void fromanc4_gfxreg_2_w(offs_t offset, uint16_t data);

	void fromanc4_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask, int layer);

	void fromanc4_main_map(address_map &map);

	// devices
	required_device<ns16550_device> m_uart;
};

#endif // MAME_VSYSTEM_FROMANC2_H
