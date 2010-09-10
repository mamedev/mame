/*************************************************************************

    Driver for Midway V-Unit games

**************************************************************************/

#include "midwunit.h"

#define MIDVUNIT_VIDEO_CLOCK	33000000

class midvunit_state : public driver_device
{
public:
	midvunit_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	optional_shared_ptr<UINT32>	m_nvram;
};



/*----------- defined in video/midvunit.c -----------*/

extern UINT16 *midvunit_videoram;
extern UINT32 *midvunit_textureram;

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
VIDEO_UPDATE( midvunit );
