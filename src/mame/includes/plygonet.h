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
		m_k054321(*this, "k054321"),
		m_shared_ram(*this, "shared_ram"),
		m_dsp_p_mirror(*this, "dsp_p_mirror"),
		m_dsp_p_8000(*this, "dsp_p_8000"),
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
	required_device<k054321_device> m_k054321;

	/* 68k-side shared ram */
	required_shared_ptr<uint32_t> m_shared_ram;

	required_shared_ptr<uint16_t> m_dsp_p_mirror;
	required_shared_ptr<uint16_t> m_dsp_p_8000;

	required_ioport_array<4> m_inputs;
	required_ioport m_eepromout;

	uint8_t m_sys0 = 0;
	uint8_t m_sys1 = 0;

	/* TTL text plane stuff */
	int m_ttl_gfx_index = 0;
	tilemap_t *m_ttl_tilemap = nullptr;
	tilemap_t *m_roz_tilemap = nullptr;
	uint16_t m_ttl_vram[0x800];
	uint16_t m_roz_vram[0x800];

	/* sound */
	uint8_t m_sound_ctrl = 0;
	uint8_t m_sound_intck = 0;

	/* memory buffers */
	std::unique_ptr<uint16_t[]> m_dsp_bank00_ram;
	std::unique_ptr<uint16_t[]> m_dsp_bank01_ram;
	std::unique_ptr<uint16_t[]> m_dsp_bank02_ram;
	std::unique_ptr<uint16_t[]> m_dsp_shared_ram_16;
	std::unique_ptr<uint16_t[]> m_dsp_bank04_ram;

	/* DSP information */
	uint16_t m_dsp_portc;
	uint8_t m_dsp_bank_group;
	uint8_t m_dsp_bank_num;
	uint32_t m_dsp_bank_offsets[5];

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
	uint8_t dsp_bank_group();
	uint8_t dsp_bank_num(uint8_t bank_group);
	uint16_t dsp_ram_bank00_read(offs_t offset);
	void dsp_ram_bank00_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_bank01_read(offs_t offset);
	void dsp_ram_bank01_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_bank02_read(offs_t offset);
	void dsp_ram_bank02_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_shared_ram_read(offs_t offset);
	void dsp_shared_ram_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_bank04_read(offs_t offset);
	void dsp_ram_bank04_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_ctrl_w(uint8_t data);
	uint32_t ttl_ram_r(offs_t offset);
	void ttl_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t roz_ram_r(offs_t offset);
	void roz_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

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

	static const uint16_t dsp_bank00_size;
	static const uint16_t dsp_bank01_size;
	static const uint16_t dsp_bank02_size;
	static const uint16_t dsp_shared_ram_16_size;
	static const uint16_t dsp_bank04_size;
};

#endif // MAME_INCLUDES_PLYGONET_H
