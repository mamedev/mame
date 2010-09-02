

/* Define clocks based on actual OSC on the PCB */

#define CPU_CLOCK		(XTAL_40MHz / 2)		/* clock for 68020 */
#define SOUND_CPU_CLOCK		(XTAL_12MHz / 2)		/* clock for Z80 sound CPU */
#define FM_SOUND_CLOCK		(XTAL_33_8688MHz / 2)		/* FM clock */

/* NOTE: YMF278B_STD_CLOCK is defined in /src/emu/sound/ymf278b.h */


class fuuki32_state : public driver_device
{
public:
	fuuki32_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT32 *    vram_0;
	UINT32 *    vram_1;
	UINT32 *    vram_2;
	UINT32 *    vram_3;
	UINT32 *    vregs;
	UINT32 *    priority;
	UINT32 *    tilebank;
	UINT32 *    spriteram;
	UINT32 *    buf_spriteram;
	UINT32 *    buf_spriteram2;
	UINT32 *    paletteram;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *tilemap_0, *tilemap_1, *tilemap_2, *tilemap_3;
	UINT32      spr_buffered_tilebank[2];

	/* misc */
	emu_timer   *raster_interrupt_timer;
	UINT8       shared_ram[16];

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/fuuki32.c -----------*/

WRITE32_HANDLER( fuuki32_vram_0_w );
WRITE32_HANDLER( fuuki32_vram_1_w );
WRITE32_HANDLER( fuuki32_vram_2_w );
WRITE32_HANDLER( fuuki32_vram_3_w );

VIDEO_START( fuuki32 );
VIDEO_UPDATE( fuuki32 );
VIDEO_EOF( fuuki32 );
