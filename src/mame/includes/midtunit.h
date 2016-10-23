// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Driver for Midway T-unit games.

**************************************************************************/

#include "audio/williams.h"
#include "audio/dcs.h"

class midtunit_state : public driver_device
{
public:
	enum
	{
		TIMER_DMA
	};

	midtunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_dcs(*this, "dcs"),
		m_cvsd_sound(*this, "cvsd"),
		m_adpcm_sound(*this, "adpcm") ,
		m_nvram(*this, "nvram"),
		m_gfxrom(*this, "gfxrom") { }

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	optional_device<dcs_audio_device> m_dcs;
	optional_device<williams_cvsd_sound_device> m_cvsd_sound;
	optional_device<williams_adpcm_sound_device> m_adpcm_sound;

	required_shared_ptr<uint16_t> m_nvram;

	required_memory_region m_gfxrom;

	void midtunit_cmos_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midtunit_cmos_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midtunit_cmos_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midtunit_sound_state_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midtunit_sound_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midtunit_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mk_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mk_prot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mkturbo_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mk2_prot_const_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mk2_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mk2_prot_shift_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mk2_prot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t nbajam_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void nbajam_prot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void jdredd_prot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t jdredd_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midtunit_gfxrom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midwunit_gfxrom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midtunit_vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midtunit_vram_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midtunit_vram_color_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midtunit_vram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midtunit_vram_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midtunit_vram_color_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midtunit_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midwunit_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midwunit_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midxunit_paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t midxunit_paletteram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midtunit_dma_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void midtunit_dma_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	void init_mktunit();
	void init_mkturbo();
	void init_nbajamte();
	void init_nbajam();
	void init_jdreddp();
	void init_mk2();

	void machine_reset_midtunit();
	void video_start_midtunit();

	void register_state_saving();
	void init_tunit_generic(int sound);
	void init_nbajam_common(int te_protection);

	/* CMOS-related variables */
	uint8_t    m_cmos_write_enable;

	/* sound-related variables */
	uint8_t    m_chip_type;
	uint8_t    m_fake_sound_state;

	/* protection */
	uint8_t    m_mk_prot_index;
	uint16_t   m_mk2_prot_data;

	const uint32_t *m_nbajam_prot_table;
	uint16_t   m_nbajam_prot_queue[5];
	uint8_t    m_nbajam_prot_index;

	const uint8_t *m_jdredd_prot_table;
	uint8_t    m_jdredd_prot_index;
	uint8_t    m_jdredd_prot_max;

	uint8_t m_gfx_rom_large;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
