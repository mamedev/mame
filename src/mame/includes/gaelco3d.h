/*************************************************************************

    Driver for Gaelco 3D games

    driver by Aaron Giles

**************************************************************************/

#include "sound/dmadac.h"
#include "video/polynew.h"

#define SOUND_CHANNELS	4

struct gaelco3d_object_data
{
	UINT32 tex, color;
	float ooz_dx, ooz_dy, ooz_base;
	float uoz_dx, uoz_dy, uoz_base;
	float voz_dx, voz_dy, voz_base;
	float z0;
};

class gaelco3d_state;

class gaelco3d_renderer : public poly_manager<float, gaelco3d_object_data, 1, 2000>
{
public:
	gaelco3d_renderer(gaelco3d_state &state);

	bitmap_ind16 &screenbits() { return m_screenbits; }
	UINT32 polygons() { UINT32 result = m_polygons; m_polygons = 0; return result; }

	void render_poly(screen_device &screen, UINT32 *polydata);

private:
	gaelco3d_state &m_state;
	bitmap_ind16 m_screenbits;
	bitmap_ind16 m_zbuffer;
	UINT32 m_polygons;
	offs_t m_texture_size;
	offs_t m_texmask_size;
	UINT8 *m_texture;
	UINT8 *m_texmask;

	void render_noz_noperspective(INT32 scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
	void render_normal(INT32 scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
	void render_alphablend(INT32 scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
};

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
	rgb_t *m_palette;
	UINT32 *m_polydata_buffer;
	UINT32 m_polydata_count;
	int m_lastscan;
	int m_video_changed;
	gaelco3d_renderer *m_poly;
	DECLARE_WRITE16_MEMBER(irq_ack_w);
	DECLARE_WRITE32_MEMBER(irq_ack32_w);
	DECLARE_WRITE16_MEMBER(sound_data_w);
	DECLARE_READ16_MEMBER(sound_data_r);
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_WRITE16_MEMBER(sound_status_w);
	DECLARE_WRITE16_MEMBER(analog_port_clock_w);
	DECLARE_WRITE16_MEMBER(analog_port_latch_w);
	DECLARE_READ32_MEMBER(tms_m68k_ram_r);
	DECLARE_WRITE32_MEMBER(tms_m68k_ram_w);
	DECLARE_WRITE16_MEMBER(tms_reset_w);
	DECLARE_WRITE16_MEMBER(tms_irq_w);
	DECLARE_WRITE16_MEMBER(tms_control3_w);
	DECLARE_WRITE16_MEMBER(tms_comm_w);
	DECLARE_WRITE16_MEMBER(adsp_control_w);
	DECLARE_WRITE16_MEMBER(adsp_rombank_w);
	DECLARE_WRITE32_MEMBER(radikalb_lamp_w);
	DECLARE_WRITE32_MEMBER(unknown_137_w);
	DECLARE_WRITE32_MEMBER(unknown_13a_w);
	DECLARE_WRITE32_MEMBER(gaelco3d_render_w);
	DECLARE_WRITE16_MEMBER(gaelco3d_paletteram_w);
	DECLARE_WRITE32_MEMBER(gaelco3d_paletteram_020_w);
};


/*----------- defined in video/gaelco3d.c -----------*/

void gaelco3d_render(screen_device &screen);


VIDEO_START( gaelco3d );
SCREEN_UPDATE_IND16( gaelco3d );
