/*************************************************************************

    Driver for Midway V-Unit games

**************************************************************************/

#include "video/polynew.h"

#define MIDVUNIT_VIDEO_CLOCK	33000000

struct midvunit_object_data
{
	UINT16 *	destbase;
	UINT8 *		texbase;
	UINT16		pixdata;
	UINT8		dither;
};

class midvunit_state;

class midvunit_renderer : public poly_manager<float, midvunit_object_data, 2, 4000>
{
public:
	midvunit_renderer(midvunit_state &state);
	void process_dma_queue();
	void make_vertices_inclusive(vertex_t *vert);

private:
	midvunit_state &m_state;

	void render_flat(INT32 scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_tex(INT32 scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_textrans(INT32 scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
	void render_textransmask(INT32 scanline, const extent_t &extent, const midvunit_object_data &extradata, int threadid);
};

class midvunit_state : public driver_device
{
public:
	midvunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") { }

	optional_shared_ptr<UINT32>	m_nvram;
	UINT32 *m_ram_base;
	UINT32 *m_fastram_base;
	UINT8 m_cmos_protected;
	UINT16 m_control_data;
	UINT8 m_adc_data;
	UINT8 m_adc_shift;
	UINT16 m_last_port0;
	UINT8 m_shifter_state;
	timer_device *m_timer[2];
	double m_timer_rate;
	UINT32 *m_tms32031_control;
	UINT32 *m_midvplus_misc;
	UINT16 m_bit_index;
	int m_lastval;
	UINT32 *m_generic_speedup;
	UINT16 *m_videoram;
	UINT32 *m_textureram;
	UINT16 m_video_regs[16];
	UINT16 m_dma_data[16];
	UINT8 m_dma_data_index;
	UINT16 m_page_control;
	UINT8 m_video_changed;
	emu_timer *m_scanline_timer;
	midvunit_renderer *m_poly;
};



/*----------- defined in video/midvunit.c -----------*/

WRITE32_HANDLER( midvunit_dma_queue_w );
READ32_HANDLER( midvunit_dma_queue_entries_r );
READ32_HANDLER( midvunit_dma_trigger_r );

WRITE32_HANDLER( midvunit_page_control_w );
READ32_HANDLER( midvunit_page_control_r );

WRITE32_HANDLER( midvunit_video_control_w );
READ32_HANDLER( midvunit_scanline_r );

WRITE32_HANDLER( midvunit_videoram_w );
READ32_HANDLER( midvunit_videoram_r );

WRITE32_HANDLER( midvunit_paletteram_w );

WRITE32_HANDLER( midvunit_textureram_w );
READ32_HANDLER( midvunit_textureram_r );

VIDEO_START( midvunit );
SCREEN_UPDATE_IND16( midvunit );
