// license:BSD-3-Clause
// copyright-holders:R. Belmont, Andrew Gardner
#ifndef MAME_INCLUDES_PLYGONET_H
#define MAME_INCLUDES_PLYGONET_H

#pragma once

#include "machine/eepromser.h"
#include "machine/k054321.h"
#include "video/k053936.h"
#include "cpu/dsp56156/dsp56156.h"
#include "emupal.h"
#include "tilemap.h"


class polygonet_state : public driver_device
{
public:
	polygonet_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dsp(*this, "dsp"),
		m_eeprom(*this, "eeprom"),
		m_k053936(*this, "k053936"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ttl_vram(*this, "ttl_vram"),
		m_roz_vram(*this, "roz_vram"),
		m_k054321(*this, "k054321"),
		m_dsp_bank_a_8(*this, "dsp_bank_a_8"),
		m_dsp_bank_b_6(*this, "dsp_bank_b_6"),
		m_dsp_bank_b_7(*this, "dsp_bank_b_7"),
		m_dsp_bank_b_8(*this, "dsp_bank_b_8"),
		m_dsp_common(*this, "dsp_common"),
		m_dsp_share(*this, "dsp_share"),
		m_dsp_ab_0(*this, "dsp_ab_0"),
		m_dsp_a_6(*this, "dsp_a_6"),
		m_dsp_a_7(*this, "dsp_a_7"),
		m_dsp_a_e(*this, "dsp_a_e"),
		m_dsp_ram_a_8(*this, "dsp_ram_a_8", 8 * 0x4000U * 2, ENDIANNESS_BIG),
		m_dsp_ram_b_6(*this, "dsp_ram_b_6", 4 * 0x1000U * 2, ENDIANNESS_BIG),
		m_dsp_ram_b_7(*this, "dsp_ram_b_7", 4 * 0x1000U * 2, ENDIANNESS_BIG),
		m_dsp_ram_b_8(*this, "dsp_ram_b_8", 8 * 0x8000U * 2, ENDIANNESS_BIG),
		m_dsp_data_view(*this, "dspdata"),
		m_inputs(*this, "IN%u", 0U),
		m_eepromout(*this, "EEPROMOUT")
	{ }

	void plygonet(machine_config &config);

	void init_polygonet();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dsp56156_device> m_dsp;
	required_device<eeprom_serial_er5911_device> m_eeprom;
	required_device<k053936_device> m_k053936;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint32_t> m_ttl_vram;
	required_shared_ptr<uint32_t> m_roz_vram;
	required_device<k054321_device> m_k054321;

	memory_bank_creator m_dsp_bank_a_8;
	memory_bank_creator m_dsp_bank_b_6;
	memory_bank_creator m_dsp_bank_b_7;
	memory_bank_creator m_dsp_bank_b_8;
	required_shared_ptr<uint16_t> m_dsp_common;
	required_shared_ptr<uint16_t> m_dsp_share;
	required_shared_ptr<uint16_t> m_dsp_ab_0;
	required_shared_ptr<uint16_t> m_dsp_a_6;
	required_shared_ptr<uint16_t> m_dsp_a_7;
	required_shared_ptr<uint16_t> m_dsp_a_e;
	memory_share_creator<uint16_t> m_dsp_ram_a_8;
	memory_share_creator<uint16_t> m_dsp_ram_b_6;
	memory_share_creator<uint16_t> m_dsp_ram_b_7;
	memory_share_creator<uint16_t> m_dsp_ram_b_8;
	memory_view m_dsp_data_view;

	required_ioport_array<4> m_inputs;
	required_ioport m_eepromout;

	uint8_t m_sys0 = 0;
	uint8_t m_sys1 = 0;

	/* TTL text plane stuff */
	int m_ttl_gfx_index = 0;
	tilemap_t *m_ttl_tilemap = nullptr;
	tilemap_t *m_roz_tilemap = nullptr;

	/* sound */
	uint8_t m_sound_ctrl = 0;
	uint8_t m_sound_intck = 0;

	/* DSP information */
	uint16_t m_dsp_portc;

	void sys_w(offs_t offset, uint8_t data);
	uint8_t inputs_r(offs_t offset);
	void sound_irq_w(uint32_t data);
	uint32_t dsp_host_interface_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t shared_ram_read(offs_t offset, uint32_t mem_mask = ~0);
	void shared_ram_write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void dsp_w_lines(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void dsp_host_interface_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t network_r();
	uint16_t dsp_bootload_r();
	void dsp_portc_write(uint16_t data);
	uint16_t dsp_ram_ab_0_read(offs_t offset);
	void dsp_ram_ab_0_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_a_6_read(offs_t offset);
	void dsp_ram_a_6_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_b_6_read(offs_t offset);
	void dsp_ram_b_6_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_a_7_read(offs_t offset);
	void dsp_ram_a_7_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_b_7_read(offs_t offset);
	void dsp_ram_b_7_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_a_8_read(offs_t offset);
	void dsp_ram_a_8_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_b_8_read(offs_t offset);
	void dsp_ram_b_8_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_a_c_read(offs_t offset);
	void dsp_ram_a_c_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_a_e_read(offs_t offset);
	void dsp_ram_a_e_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_ctrl_w(uint8_t data);
	void ttl_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void roz_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	TILE_GET_INFO_MEMBER(roz_get_tile_info);
	TILEMAP_MAPPER_MEMBER(scan_rows);
	TILEMAP_MAPPER_MEMBER(scan_cols);
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_interrupt);
	DECLARE_WRITE_LINE_MEMBER(k054539_nmi_gen);

	void dsp_data_map(address_map &map);
	void dsp_program_map(address_map &map);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_PLYGONET_H
