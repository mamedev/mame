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

	required_shared_ptr<UINT16> m_nvram;

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
	DECLARE_READ16_MEMBER(midtunit_gfxrom_r);
	DECLARE_READ16_MEMBER(midwunit_gfxrom_r);
	DECLARE_WRITE16_MEMBER(midtunit_vram_w);
	DECLARE_WRITE16_MEMBER(midtunit_vram_data_w);
	DECLARE_WRITE16_MEMBER(midtunit_vram_color_w);
	DECLARE_READ16_MEMBER(midtunit_vram_r);
	DECLARE_READ16_MEMBER(midtunit_vram_data_r);
	DECLARE_READ16_MEMBER(midtunit_vram_color_r);
	DECLARE_WRITE16_MEMBER(midtunit_control_w);
	DECLARE_WRITE16_MEMBER(midwunit_control_w);
	DECLARE_READ16_MEMBER(midwunit_control_r);
	DECLARE_WRITE16_MEMBER(midxunit_paletteram_w);
	DECLARE_READ16_MEMBER(midxunit_paletteram_r);
	DECLARE_READ16_MEMBER(midtunit_dma_r);
	DECLARE_WRITE16_MEMBER(midtunit_dma_w);

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	DECLARE_DRIVER_INIT(mktunit);
	DECLARE_DRIVER_INIT(mkturbo);
	DECLARE_DRIVER_INIT(nbajamte);
	DECLARE_DRIVER_INIT(nbajam);
	DECLARE_DRIVER_INIT(jdreddp);
	DECLARE_DRIVER_INIT(mk2);

	DECLARE_MACHINE_RESET(midtunit);
	DECLARE_VIDEO_START(midtunit);

	void register_state_saving();
	void init_tunit_generic(int sound);
	void init_nbajam_common(int te_protection);

	/* CMOS-related variables */
	UINT8    m_cmos_write_enable;

	/* sound-related variables */
	UINT8    m_chip_type;
	UINT8    m_fake_sound_state;

	/* protection */
	UINT8    m_mk_prot_index;
	UINT16   m_mk2_prot_data;

	const UINT32 *m_nbajam_prot_table;
	UINT16   m_nbajam_prot_queue[5];
	UINT8    m_nbajam_prot_index;

	const UINT8 *m_jdredd_prot_table;
	UINT8    m_jdredd_prot_index;
	UINT8    m_jdredd_prot_max;

	UINT8 m_gfx_rom_large;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
