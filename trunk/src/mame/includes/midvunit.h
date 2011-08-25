/*************************************************************************

    Driver for Midway V-Unit games

**************************************************************************/

#include "video/poly.h"

#define MIDVUNIT_VIDEO_CLOCK	33000000

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
	poly_manager *m_poly;
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
SCREEN_UPDATE( midvunit );
