// license:BSD-3-Clause
// copyright-holders:R. Belmont, Andrew Gardner
#include "machine/eepromser.h"
#include "video/k053936.h"
#include "cpu/dsp56k/dsp56k.h"


static const UINT16 dsp56k_bank00_size = 0x1000;
static const UINT16 dsp56k_bank01_size = 0x1000;
static const UINT16 dsp56k_bank02_size = 0x4000;
static const UINT16 dsp56k_shared_ram_16_size = 0x2000;
static const UINT16 dsp56k_bank04_size = 0x1fc0;

class polygonet_state : public driver_device
{
public:
	polygonet_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dsp(*this, "dsp"),
		m_eeprom(*this, "eeprom"),
		m_k053936(*this, "k053936"),
		m_shared_ram(*this, "shared_ram"),
		m_dsp56k_p_mirror(*this, "dsp56k_p_mirror"),
		m_dsp56k_p_8000(*this, "dsp56k_p_8000"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dsp56k_device> m_dsp;
	required_device<eeprom_serial_er5911_device> m_eeprom;
	required_device<k053936_device> m_k053936;

	/* 68k-side shared ram */
	required_shared_ptr<UINT32> m_shared_ram;

	required_shared_ptr<UINT16> m_dsp56k_p_mirror;
	required_shared_ptr<UINT16> m_dsp56k_p_8000;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	ioport_port *m_inputs[4];
	UINT8 m_sys0;
	UINT8 m_sys1;

	/* TTL text plane stuff */
	int m_ttl_gfx_index;
	tilemap_t *m_ttl_tilemap;
	tilemap_t *m_roz_tilemap;
	UINT16 m_ttl_vram[0x800];
	UINT16 m_roz_vram[0x800];

	/* sound */
	UINT8 m_sound_ctrl;
	UINT8 m_sound_intck;

	/* memory buffers */
	UINT16 m_dsp56k_bank00_ram[2 * 8 * dsp56k_bank00_size]; /* 2 bank sets, 8 potential banks each */
	UINT16 m_dsp56k_bank01_ram[2 * 8 * dsp56k_bank01_size];
	UINT16 m_dsp56k_bank02_ram[2 * 8 * dsp56k_bank02_size];
	UINT16 m_dsp56k_shared_ram_16[2 * 8 * dsp56k_shared_ram_16_size];
	UINT16 m_dsp56k_bank04_ram[2 * 8 * dsp56k_bank04_size];

	DECLARE_WRITE8_MEMBER(polygonet_sys_w);
	DECLARE_READ8_MEMBER(polygonet_inputs_r);
	DECLARE_READ8_MEMBER(sound_comms_r);
	DECLARE_WRITE8_MEMBER(sound_comms_w);
	DECLARE_WRITE32_MEMBER(sound_irq_w);
	DECLARE_READ32_MEMBER(dsp_host_interface_r);
	DECLARE_WRITE32_MEMBER(shared_ram_write);
	DECLARE_WRITE32_MEMBER(dsp_w_lines);
	DECLARE_WRITE32_MEMBER(dsp_host_interface_w);
	DECLARE_READ32_MEMBER(network_r);
	DECLARE_READ16_MEMBER(dsp56k_bootload_r);
	DECLARE_READ16_MEMBER(dsp56k_ram_bank00_read);
	DECLARE_WRITE16_MEMBER(dsp56k_ram_bank00_write);
	DECLARE_READ16_MEMBER(dsp56k_ram_bank01_read);
	DECLARE_WRITE16_MEMBER(dsp56k_ram_bank01_write);
	DECLARE_READ16_MEMBER(dsp56k_ram_bank02_read);
	DECLARE_WRITE16_MEMBER(dsp56k_ram_bank02_write);
	DECLARE_READ16_MEMBER(dsp56k_shared_ram_read);
	DECLARE_WRITE16_MEMBER(dsp56k_shared_ram_write);
	DECLARE_READ16_MEMBER(dsp56k_ram_bank04_read);
	DECLARE_WRITE16_MEMBER(dsp56k_ram_bank04_write);
	DECLARE_WRITE8_MEMBER(sound_ctrl_w);
	DECLARE_READ32_MEMBER(polygonet_ttl_ram_r);
	DECLARE_WRITE32_MEMBER(polygonet_ttl_ram_w);
	DECLARE_READ32_MEMBER(polygonet_roz_ram_r);
	DECLARE_WRITE32_MEMBER(polygonet_roz_ram_w);
	DECLARE_DRIVER_INIT(polygonet);
	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	TILE_GET_INFO_MEMBER(roz_get_tile_info);
	TILEMAP_MAPPER_MEMBER(plygonet_scan);
	TILEMAP_MAPPER_MEMBER(plygonet_scan_cols);
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void video_start() override;
	UINT32 screen_update_polygonet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(polygonet_interrupt);
	DECLARE_WRITE_LINE_MEMBER(k054539_nmi_gen);
};
