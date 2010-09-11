/*************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

**************************************************************************/

#include "machine/nvram.h"

#define VIDEO_CLOCK		XTAL_8MHz			/* video (pixel) clock */
#define CPU_CLOCK		XTAL_12MHz			/* clock for 68000-based systems */
#define CPU020_CLOCK	XTAL_25MHz			/* clock for 68EC020-based systems */
#define SOUND_CLOCK		XTAL_16MHz			/* clock for sound board */
#define TMS_CLOCK		XTAL_40MHz			/* TMS320C31 clocks on drivedge */


class itech32_state : public driver_device
{
public:
	itech32_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	void nvram_init(nvram_device &nvram, void *base, size_t length);

	UINT16 *videoram;
};


/*----------- defined in drivers/itech32.c -----------*/

void itech32_update_interrupts(running_machine *machine, int vint, int xint, int qint);


/*----------- defined in video/itech32.c -----------*/

extern UINT16 *itech32_video;
extern UINT32 *drivedge_zbuf_control;
extern UINT8 itech32_planes;
extern UINT16 itech32_vram_height;

VIDEO_START( itech32 );

WRITE16_HANDLER( timekill_colora_w );
WRITE16_HANDLER( timekill_colorbc_w );
WRITE16_HANDLER( timekill_intensity_w );

WRITE16_HANDLER( bloodstm_color1_w );
WRITE16_HANDLER( bloodstm_color2_w );
WRITE16_HANDLER( bloodstm_plane_w );

WRITE32_HANDLER( drivedge_color0_w );

WRITE32_HANDLER( itech020_color1_w );
WRITE32_HANDLER( itech020_color2_w );
WRITE32_HANDLER( itech020_plane_w );

WRITE16_HANDLER( timekill_paletteram_w );
WRITE16_HANDLER( bloodstm_paletteram_w );
WRITE32_HANDLER( drivedge_paletteram_w );
WRITE32_HANDLER( itech020_paletteram_w );

WRITE16_HANDLER( itech32_video_w );
READ16_HANDLER( itech32_video_r );

WRITE16_HANDLER( bloodstm_video_w );
READ16_HANDLER( bloodstm_video_r );
WRITE32_HANDLER( itech020_video_w );
READ32_HANDLER( itech020_video_r );
WRITE32_HANDLER( drivedge_zbuf_control_w );

VIDEO_UPDATE( itech32 );
