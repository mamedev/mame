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
	gaelco3d_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *adsp_ram_base;
	UINT16 *m68k_ram_base;
	UINT16 *tms_comm_base;
	UINT16 sound_data;
	UINT8 sound_status;
	offs_t tms_offset_xor;
	UINT8 analog_ports[4];
	UINT8 framenum;
	timer_device *adsp_autobuffer_timer;
	UINT16 *adsp_control_regs;
	UINT16 *adsp_fastram_base;
	UINT8 adsp_ireg;
	offs_t adsp_ireg_base;
	offs_t adsp_incs;
	offs_t adsp_size;
	dmadac_sound_device *dmadac[SOUND_CHANNELS];
	UINT8 *texture;
	UINT8 *texmask;
	offs_t texture_size;
	offs_t texmask_size;
	bitmap_t *screenbits;
	bitmap_t *zbuffer;
	rgb_t *palette;
	UINT32 *polydata_buffer;
	UINT32 polydata_count;
	int polygons;
	int lastscan;
	int video_changed;
	poly_manager *poly;
};


/*----------- defined in video/gaelco3d.c -----------*/

void gaelco3d_render(screen_device &screen);
WRITE32_HANDLER( gaelco3d_render_w );

WRITE16_HANDLER( gaelco3d_paletteram_w );
WRITE32_HANDLER( gaelco3d_paletteram_020_w );

VIDEO_START( gaelco3d );
SCREEN_UPDATE( gaelco3d );
