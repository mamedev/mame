// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Williams/Midway Y/Z-unit system

**************************************************************************/

#include "cpu/tms34010/tms34010.h"
#include "audio/williams.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"

/* protection data types */
struct protection_data
{
	UINT16  reset_sequence[3];
	UINT16  data_sequence[100];
};

struct dma_state_t
{
	UINT32      offset;         /* source offset, in bits */
	INT32       rowbytes;       /* source bytes to skip each row */
	INT32       xpos;           /* x position, clipped */
	INT32       ypos;           /* y position, clipped */
	INT32       width;          /* horizontal pixel count */
	INT32       height;         /* vertical pixel count */
	UINT16      palette;        /* palette base */
	UINT16      color;          /* current foreground color with palette */
};


class midyunit_state : public driver_device
{
public:
	enum
	{
		TIMER_DMA,
		TIMER_AUTOERASE_LINE
	};

	midyunit_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_oki(*this, "oki"),
			m_palette(*this, "palette"),
			m_narc_sound(*this, "narcsnd"),
			m_cvsd_sound(*this, "cvsd"),
			m_adpcm_sound(*this, "adpcm"),
			m_generic_paletteram_16(*this, "paletteram"),
			m_gfx_rom(*this, "gfx_rom", 16),
			m_ports(*this, ports) { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	required_device<palette_device> m_palette;
	optional_device<williams_narc_sound_device> m_narc_sound;
	optional_device<williams_cvsd_sound_device> m_cvsd_sound;
	optional_device<williams_adpcm_sound_device> m_adpcm_sound;

	required_shared_ptr<UINT16> m_generic_paletteram_16;
	optional_shared_ptr<UINT8> m_gfx_rom;

	optional_ioport_array<6> m_ports;

	DECLARE_IOPORT_ARRAY(ports);

	std::unique_ptr<UINT16[]> m_cmos_ram;
	UINT32 m_cmos_page;
	UINT16 m_prot_result;
	UINT16 m_prot_sequence[3];
	UINT8 m_prot_index;
	UINT8 m_term2_analog_select;
	const struct protection_data *m_prot_data;
	UINT8 m_cmos_w_enable;
	UINT8 m_chip_type;
	UINT16 *m_t2_hack_mem;
	UINT8 *m_cvsd_protection_base;
	UINT8 m_autoerase_enable;
	UINT32 m_palette_mask;
	std::unique_ptr<pen_t[]> m_pen_map;
	std::unique_ptr<UINT16[]>   m_local_videoram;
	UINT8 m_videobank_select;
	UINT8 m_yawdim_dma;
	UINT16 m_dma_register[16];
	dma_state_t m_dma_state;
	DECLARE_WRITE16_MEMBER(midyunit_cmos_w);
	DECLARE_READ16_MEMBER(midyunit_cmos_r);
	DECLARE_WRITE16_MEMBER(midyunit_cmos_enable_w);
	DECLARE_READ16_MEMBER(midyunit_protection_r);
	DECLARE_READ16_MEMBER(midyunit_input_r);
	DECLARE_WRITE16_MEMBER(midyunit_sound_w);
	DECLARE_READ16_MEMBER(term2_input_r);
	DECLARE_WRITE16_MEMBER(term2_sound_w);
	DECLARE_WRITE16_MEMBER(term2_hack_w);
	DECLARE_WRITE16_MEMBER(term2la3_hack_w);
	DECLARE_WRITE16_MEMBER(term2la2_hack_w);
	DECLARE_WRITE16_MEMBER(term2la1_hack_w);
	DECLARE_WRITE8_MEMBER(cvsd_protection_w);
	DECLARE_READ16_MEMBER(mkturbo_prot_r);
	DECLARE_READ16_MEMBER(midyunit_gfxrom_r);
	DECLARE_WRITE16_MEMBER(midyunit_vram_w);
	DECLARE_READ16_MEMBER(midyunit_vram_r);
	DECLARE_WRITE16_MEMBER(midyunit_control_w);
	DECLARE_WRITE16_MEMBER(midyunit_paletteram_w);
	DECLARE_READ16_MEMBER(midyunit_dma_r);
	DECLARE_WRITE16_MEMBER(midyunit_dma_w);
	DECLARE_CUSTOM_INPUT_MEMBER(narc_talkback_strobe_r);
	DECLARE_CUSTOM_INPUT_MEMBER(narc_talkback_data_r);
	DECLARE_CUSTOM_INPUT_MEMBER(adpcm_irq_state_r);
	DECLARE_WRITE8_MEMBER(yawdim_oki_bank_w);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);
	DECLARE_DRIVER_INIT(smashtv);
	DECLARE_DRIVER_INIT(strkforc);
	DECLARE_DRIVER_INIT(narc);
	DECLARE_DRIVER_INIT(term2);
	DECLARE_DRIVER_INIT(term2la1);
	DECLARE_DRIVER_INIT(term2la3);
	DECLARE_DRIVER_INIT(mkyunit);
	DECLARE_DRIVER_INIT(trog);
	DECLARE_DRIVER_INIT(totcarn);
	DECLARE_DRIVER_INIT(mkyawdim);
	DECLARE_DRIVER_INIT(shimpact);
	DECLARE_DRIVER_INIT(hiimpact);
	DECLARE_DRIVER_INIT(mkyturbo);
	DECLARE_DRIVER_INIT(term2la2);
	DECLARE_MACHINE_RESET(midyunit);
	DECLARE_VIDEO_START(midzunit);
	DECLARE_VIDEO_START(midyunit_4bit);
	DECLARE_VIDEO_START(midyunit_6bit);
	DECLARE_VIDEO_START(mkyawdim);
	DECLARE_VIDEO_START(common);
	TIMER_CALLBACK_MEMBER(dma_callback);
	TIMER_CALLBACK_MEMBER(autoerase_line);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void dma_draw(UINT16 command);
	void init_generic(int bpp, int sound, int prot_start, int prot_end);
	void term2_init_common(write16_delegate hack_w);
};
