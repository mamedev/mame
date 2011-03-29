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
	UINT8 vint_state;
	UINT8 xint_state;
	UINT8 qint_state;
	UINT8 sound_data;
	UINT8 sound_return;
	UINT8 sound_int_state;
	UINT16 *main_rom;
	UINT16 *main_ram;
	offs_t itech020_prot_address;
	UINT32 *tms1_ram;
	UINT32 *tms2_ram;
	UINT32 *tms1_boot;
	UINT8 tms_spinning[2];
	int special_result;
	int p1_effx;
	int p1_effy;
	int p1_lastresult;
	attotime p1_lasttime;
	int p2_effx;
	int p2_effy;
	int p2_lastresult;
	attotime p2_lasttime;
	UINT8 written[0x8000];
	int is_drivedge;
	UINT16 *video;
	UINT32 *drivedge_zbuf_control;
	UINT8 planes;
	UINT16 vram_height;
	UINT16 xfer_xcount;
	UINT16 xfer_ycount;
	UINT16 xfer_xcur;
	UINT16 xfer_ycur;
	rectangle clip_rect;
	rectangle scaled_clip_rect;
	rectangle clip_save;
	emu_timer *scanline_timer;
	UINT8 *grom_base;
	UINT32 grom_size;
	UINT32 grom_bank;
	UINT32 grom_bank_mask;
	UINT16 color_latch[2];
	UINT8 enable_latch[2];
	UINT16 *videoplane[2];
	UINT32 vram_mask;
	UINT32 vram_xmask;
	UINT32 vram_ymask;
};


/*----------- defined in drivers/itech32.c -----------*/

void itech32_update_interrupts(running_machine &machine, int vint, int xint, int qint);


/*----------- defined in video/itech32.c -----------*/

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

SCREEN_UPDATE( itech32 );
