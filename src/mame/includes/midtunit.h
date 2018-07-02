// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Driver for Midway T-unit games.

**************************************************************************/

#include "audio/dcs.h"
#include "audio/williams.h"

#include "cpu/tms34010/tms34010.h"
#include "emupal.h"


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
		m_dcs(*this, "dcs"),
		m_palette(*this, "palette"),
		m_cvsd_sound(*this, "cvsd"),
		m_adpcm_sound(*this, "adpcm"),
		m_nvram(*this, "nvram"),
		m_gfxrom(*this, "gfxrom") { }

	void tunit_core(machine_config &config);
	void tunit_adpcm(machine_config &config);
	void tunit_dcs(machine_config &config);

	void init_mktunit();
	void init_mkturbo();
	void init_nbajamte();
	void init_nbajam();
	void init_jdreddp();
	void init_mk2();

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<dcs_audio_device> m_dcs;
	required_device<palette_device> m_palette;

	DECLARE_READ16_MEMBER(midtunit_vram_r);
	DECLARE_WRITE16_MEMBER(midtunit_vram_w);
	DECLARE_READ16_MEMBER(midtunit_gfxrom_r);
	DECLARE_WRITE16_MEMBER(midtunit_vram_data_w);
	DECLARE_WRITE16_MEMBER(midtunit_vram_color_w);
	DECLARE_READ16_MEMBER(midtunit_vram_data_r);
	DECLARE_READ16_MEMBER(midtunit_vram_color_r);

	DECLARE_WRITE16_MEMBER(midxunit_paletteram_w);
	DECLARE_READ16_MEMBER(midxunit_paletteram_r);

	DECLARE_READ16_MEMBER(midtunit_dma_r);
	DECLARE_WRITE16_MEMBER(midtunit_dma_w);

	DECLARE_READ16_MEMBER(midwunit_gfxrom_r);

	DECLARE_WRITE16_MEMBER(midwunit_control_w);
	DECLARE_READ16_MEMBER(midwunit_control_r);

	DECLARE_VIDEO_START(midtunit);

	uint8_t m_gfx_rom_large;

private:
	optional_device<williams_cvsd_sound_device> m_cvsd_sound;
	optional_device<williams_adpcm_sound_device> m_adpcm_sound;

	required_shared_ptr<uint16_t> m_nvram;

	required_memory_region m_gfxrom;

	DECLARE_WRITE16_MEMBER(midtunit_cmos_enable_w);
	DECLARE_WRITE16_MEMBER(midtunit_cmos_w);
	DECLARE_READ16_MEMBER(midtunit_cmos_r);
	DECLARE_READ16_MEMBER(midtunit_sound_state_r);
	DECLARE_READ16_MEMBER(midtunit_sound_r);
	DECLARE_WRITE16_MEMBER(midtunit_sound_w);
	DECLARE_READ16_MEMBER(mk_prot_r);
	DECLARE_WRITE16_MEMBER(mk_prot_w);
	DECLARE_READ16_MEMBER(mkturbo_prot_r);
	DECLARE_READ16_MEMBER(mk2_prot_const_r);
	DECLARE_READ16_MEMBER(mk2_prot_r);
	DECLARE_READ16_MEMBER(mk2_prot_shift_r);
	DECLARE_WRITE16_MEMBER(mk2_prot_w);
	DECLARE_READ16_MEMBER(nbajam_prot_r);
	DECLARE_WRITE16_MEMBER(nbajam_prot_w);
	DECLARE_WRITE16_MEMBER(jdredd_prot_w);
	DECLARE_READ16_MEMBER(jdredd_prot_r);
	DECLARE_WRITE16_MEMBER(midtunit_control_w);

	DECLARE_MACHINE_RESET(midtunit);

	void register_state_saving();
	void init_tunit_generic(int sound);
	void init_nbajam_common(int te_protection);

	emu_timer *m_dma_timer;

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

	void main_map(address_map &map);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
