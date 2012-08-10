/*************************************************************************

    Driver for Midway T-unit games.

**************************************************************************/

#include "audio/williams.h"
#include "audio/dcs.h"

class midtunit_state : public driver_device
{
public:
	midtunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram"),
		  m_cvsd_sound(*this, "cvsd"),
		  m_adpcm_sound(*this, "adpcm") { }

	required_shared_ptr<UINT16>	m_nvram;
	optional_device<williams_cvsd_sound_device> m_cvsd_sound;
	optional_device<williams_adpcm_sound_device> m_adpcm_sound;

	DECLARE_WRITE16_MEMBER(midtunit_cmos_enable_w);
	DECLARE_WRITE16_MEMBER(midtunit_cmos_w);
	DECLARE_READ16_MEMBER(midtunit_cmos_r);
	DECLARE_READ16_MEMBER(midtunit_input_r);
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
	DECLARE_READ16_MEMBER(jdredd_hack_r);
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
	DECLARE_WRITE16_MEMBER(midtunit_paletteram_w);
	DECLARE_WRITE16_MEMBER(midxunit_paletteram_w);
	DECLARE_READ16_MEMBER(midxunit_paletteram_r);
	DECLARE_READ16_MEMBER(midtunit_dma_r);
	DECLARE_WRITE16_MEMBER(midtunit_dma_w);
	DECLARE_DRIVER_INIT(mktunit);
	DECLARE_DRIVER_INIT(mkturbo);
	DECLARE_DRIVER_INIT(nbajamte);
	DECLARE_DRIVER_INIT(nbajam);
	DECLARE_DRIVER_INIT(jdreddp);
	DECLARE_DRIVER_INIT(mk2);
};

/*----------- defined in machine/midtunit.c -----------*/




MACHINE_RESET( midtunit );



/*----------- defined in video/midtunit.c -----------*/

extern UINT8 *	midtunit_gfx_rom;
extern size_t	midtunit_gfx_rom_size;
extern UINT8 midtunit_gfx_rom_large;

VIDEO_START( midtunit );
VIDEO_START( midwunit );
VIDEO_START( midxunit );




void midtunit_to_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg);
void midtunit_from_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg);




void midtunit_scanline_update(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params);
void midxunit_scanline_update(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params);
