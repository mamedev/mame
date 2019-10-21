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


static const uint16_t dsp56156_bank00_size = 0x1000;
static const uint16_t dsp56156_bank01_size = 0x1000;
static const uint16_t dsp56156_bank02_size = 0x4000;
static const uint16_t dsp56156_shared_ram_16_size = 0x2000;
static const uint16_t dsp56156_bank04_size = 0x1fc0;

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
		m_dsp56156_p_mirror(*this, "dsp56156_p_mirror"),
		m_dsp56156_p_8000(*this, "dsp56156_p_8000")
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

	required_shared_ptr<uint16_t> m_dsp56156_p_mirror;
	required_shared_ptr<uint16_t> m_dsp56156_p_8000;

	ioport_port *m_inputs[4];
	uint8_t m_sys0;
	uint8_t m_sys1;

	/* TTL text plane stuff */
	int m_ttl_gfx_index;
	tilemap_t *m_ttl_tilemap;
	tilemap_t *m_roz_tilemap;
	uint16_t m_ttl_vram[0x800];
	uint16_t m_roz_vram[0x800];

	/* sound */
	uint8_t m_sound_ctrl;
	uint8_t m_sound_intck;

	/* memory buffers */
	uint16_t m_dsp56156_bank00_ram[2 * 8 * dsp56156_bank00_size]; /* 2 bank sets, 8 potential banks each */
	uint16_t m_dsp56156_bank01_ram[2 * 8 * dsp56156_bank01_size];
	uint16_t m_dsp56156_bank02_ram[2 * 8 * dsp56156_bank02_size];
	uint16_t m_dsp56156_shared_ram_16[2 * 8 * dsp56156_shared_ram_16_size];
	uint16_t m_dsp56156_bank04_ram[2 * 8 * dsp56156_bank04_size];

	DECLARE_WRITE8_MEMBER(polygonet_sys_w);
	DECLARE_READ8_MEMBER(polygonet_inputs_r);
	DECLARE_WRITE32_MEMBER(sound_irq_w);
	DECLARE_READ32_MEMBER(dsp_host_interface_r);
	DECLARE_WRITE32_MEMBER(shared_ram_write);
	DECLARE_WRITE32_MEMBER(dsp_w_lines);
	DECLARE_WRITE32_MEMBER(dsp_host_interface_w);
	DECLARE_READ32_MEMBER(network_r);
	DECLARE_READ16_MEMBER(dsp56156_bootload_r);
	DECLARE_READ16_MEMBER(dsp56156_ram_bank00_read);
	DECLARE_WRITE16_MEMBER(dsp56156_ram_bank00_write);
	DECLARE_READ16_MEMBER(dsp56156_ram_bank01_read);
	DECLARE_WRITE16_MEMBER(dsp56156_ram_bank01_write);
	DECLARE_READ16_MEMBER(dsp56156_ram_bank02_read);
	DECLARE_WRITE16_MEMBER(dsp56156_ram_bank02_write);
	DECLARE_READ16_MEMBER(dsp56156_shared_ram_read);
	DECLARE_WRITE16_MEMBER(dsp56156_shared_ram_write);
	DECLARE_READ16_MEMBER(dsp56156_ram_bank04_read);
	DECLARE_WRITE16_MEMBER(dsp56156_ram_bank04_write);
	DECLARE_WRITE8_MEMBER(sound_ctrl_w);
	DECLARE_READ32_MEMBER(polygonet_ttl_ram_r);
	DECLARE_WRITE32_MEMBER(polygonet_ttl_ram_w);
	DECLARE_READ32_MEMBER(polygonet_roz_ram_r);
	DECLARE_WRITE32_MEMBER(polygonet_roz_ram_w);

	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	TILE_GET_INFO_MEMBER(roz_get_tile_info);
	TILEMAP_MAPPER_MEMBER(plygonet_scan);
	TILEMAP_MAPPER_MEMBER(plygonet_scan_cols);
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_polygonet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(polygonet_interrupt);
	DECLARE_WRITE_LINE_MEMBER(k054539_nmi_gen);

	void dsp_data_map(address_map &map);
	void dsp_program_map(address_map &map);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_PLYGONET_H
