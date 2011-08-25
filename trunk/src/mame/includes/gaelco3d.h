/*************************************************************************

    Driver for Gaelco 3D games

    driver by Aaron Giles

**************************************************************************/

#include "sound/dmadac.h"
#include "video/poly.h"

#define SOUND_CHANNELS	4

class gaelco3d_state : public driver_device
{
public:
	gaelco3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 *m_adsp_ram_base;
	UINT16 *m_m68k_ram_base;
	UINT16 *m_tms_comm_base;
	UINT16 m_sound_data;
	UINT8 m_sound_status;
	offs_t m_tms_offset_xor;
	UINT8 m_analog_ports[4];
	UINT8 m_framenum;
	timer_device *m_adsp_autobuffer_timer;
	UINT16 *m_adsp_control_regs;
	UINT16 *m_adsp_fastram_base;
	UINT8 m_adsp_ireg;
	offs_t m_adsp_ireg_base;
	offs_t m_adsp_incs;
	offs_t m_adsp_size;
	dmadac_sound_device *m_dmadac[SOUND_CHANNELS];
	UINT8 *m_texture;
	UINT8 *m_texmask;
	offs_t m_texture_size;
	offs_t m_texmask_size;
	bitmap_t *m_screenbits;
	bitmap_t *m_zbuffer;
	rgb_t *m_palette;
	UINT32 *m_polydata_buffer;
	UINT32 m_polydata_count;
	int m_polygons;
	int m_lastscan;
	int m_video_changed;
	poly_manager *m_poly;
};


/*----------- defined in video/gaelco3d.c -----------*/

void gaelco3d_render(screen_device &screen);
WRITE32_HANDLER( gaelco3d_render_w );

WRITE16_HANDLER( gaelco3d_paletteram_w );
WRITE32_HANDLER( gaelco3d_paletteram_020_w );

VIDEO_START( gaelco3d );
SCREEN_UPDATE( gaelco3d );
