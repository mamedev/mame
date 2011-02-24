/*************************************************************************

    Driver for Midway V-Unit games

**************************************************************************/

#include "video/poly.h"

#define MIDVUNIT_VIDEO_CLOCK	33000000

class midvunit_state : public driver_device
{
public:
	midvunit_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	optional_shared_ptr<UINT32>	m_nvram;
	UINT32 *ram_base;
	UINT32 *fastram_base;
	UINT8 cmos_protected;
	UINT16 control_data;
	UINT8 adc_data;
	UINT8 adc_shift;
	UINT16 last_port0;
	UINT8 shifter_state;
	timer_device *timer[2];
	double timer_rate;
	UINT32 *tms32031_control;
	UINT32 *midvplus_misc;
	UINT16 bit_index;
	int lastval;
	UINT32 *generic_speedup;
	UINT16 *videoram;
	UINT32 *textureram;
	UINT16 video_regs[16];
	UINT16 dma_data[16];
	UINT8 dma_data_index;
	UINT16 page_control;
	UINT8 video_changed;
	emu_timer *scanline_timer;
	poly_manager *poly;
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
