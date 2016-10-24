// license:BSD-3-Clause
// copyright-holders:R. Belmont, Andrew Gardner

#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "video/k053936.h"
#include "cpu/dsp56k/dsp56k.h"


static const uint16_t dsp56k_bank00_size = 0x1000;
static const uint16_t dsp56k_bank01_size = 0x1000;
static const uint16_t dsp56k_bank02_size = 0x4000;
static const uint16_t dsp56k_shared_ram_16_size = 0x2000;
static const uint16_t dsp56k_bank04_size = 0x1fc0;

class polygonet_state : public driver_device
{
public:
	polygonet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dsp(*this, "dsp"),
		m_eeprom(*this, "eeprom"),
		m_k053936(*this, "k053936"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundlatch3(*this, "soundlatch3"),
		m_shared_ram(*this, "shared_ram"),
		m_dsp56k_p_mirror(*this, "dsp56k_p_mirror"),
		m_dsp56k_p_8000(*this, "dsp56k_p_8000")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dsp56k_device> m_dsp;
	required_device<eeprom_serial_er5911_device> m_eeprom;
	required_device<k053936_device> m_k053936;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<generic_latch_8_device> m_soundlatch3;

	/* 68k-side shared ram */
	required_shared_ptr<uint32_t> m_shared_ram;

	required_shared_ptr<uint16_t> m_dsp56k_p_mirror;
	required_shared_ptr<uint16_t> m_dsp56k_p_8000;

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
	uint16_t m_dsp56k_bank00_ram[2 * 8 * dsp56k_bank00_size]; /* 2 bank sets, 8 potential banks each */
	uint16_t m_dsp56k_bank01_ram[2 * 8 * dsp56k_bank01_size];
	uint16_t m_dsp56k_bank02_ram[2 * 8 * dsp56k_bank02_size];
	uint16_t m_dsp56k_shared_ram_16[2 * 8 * dsp56k_shared_ram_16_size];
	uint16_t m_dsp56k_bank04_ram[2 * 8 * dsp56k_bank04_size];

	void polygonet_sys_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t polygonet_inputs_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_comms_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_comms_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_irq_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t dsp_host_interface_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void shared_ram_write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void dsp_w_lines(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void dsp_host_interface_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t network_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint16_t dsp56k_bootload_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dsp56k_ram_bank00_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp56k_ram_bank00_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp56k_ram_bank01_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp56k_ram_bank01_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp56k_ram_bank02_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp56k_ram_bank02_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp56k_shared_ram_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp56k_shared_ram_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp56k_ram_bank04_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp56k_ram_bank04_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t polygonet_ttl_ram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void polygonet_ttl_ram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t polygonet_roz_ram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void polygonet_roz_ram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void init_polygonet();
	void ttl_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void roz_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index plygonet_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index plygonet_scan_cols(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_polygonet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void polygonet_interrupt(device_t &device);
	void k054539_nmi_gen(int state);
};
