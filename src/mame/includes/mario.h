#ifndef MARIO_H_
#define MARIO_H_

/*
 * From the schematics:
 *
 * Video generation like dkong/dkongjr. However, clock is 24MHZ
 * 7C -> 100 => 256 - 124 = 132 ==> 264 Scanlines
 */

#define MASTER_CLOCK			XTAL_24MHz
#define PIXEL_CLOCK				(MASTER_CLOCK / 4)
#define CLOCK_1H				(MASTER_CLOCK / 8)
#define CLOCK_16H				(CLOCK_1H / 16)
#define CLOCK_1VF				((CLOCK_16H) / 12 / 2)
#define CLOCK_2VF				((CLOCK_1VF) / 2)

#define HTOTAL					(384)
#define HBSTART					(256)
#define HBEND					(0)
#define VTOTAL					(264)
#define VBSTART					(240)
#define VBEND					(16)

#define Z80_MASTER_CLOCK		XTAL_8MHz
#define Z80_CLOCK				(Z80_MASTER_CLOCK / 2) /* verified on pcb */

#define I8035_MASTER_CLOCK		XTAL_11MHz /* verified on pcb: 730Khz */
#define I8035_CLOCK				(I8035_MASTER_CLOCK)

#define MARIO_PALETTE_LENGTH	(256)

class mario_state : public driver_device
{
public:
	mario_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */

	/* machine states */

	/* sound state */
	UINT8	m_last;
	UINT8	m_portT;
	const char *m_eabank;

	/* video state */
	UINT8	m_gfx_bank;
	UINT8	m_palette_bank;
	UINT16	m_gfx_scroll;
	UINT8	m_flip;

	/* driver general */

	UINT8	*m_spriteram;
	UINT8	*m_videoram;
	size_t	m_spriteram_size;
	tilemap_t *m_bg_tilemap;
	int m_monitor;

	UINT8   m_nmi_mask;
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(mario_videoram_w);
	DECLARE_WRITE8_MEMBER(mario_gfxbank_w);
	DECLARE_WRITE8_MEMBER(mario_palettebank_w);
	DECLARE_WRITE8_MEMBER(mario_scroll_w);
	DECLARE_WRITE8_MEMBER(mario_flip_w);
};

/*----------- defined in video/mario.c -----------*/


PALETTE_INIT( mario );
VIDEO_START( mario );
SCREEN_UPDATE_IND16( mario );


/*----------- defined in audio/mario.c -----------*/

WRITE8_DEVICE_HANDLER( mario_sh1_w );
WRITE8_DEVICE_HANDLER( mario_sh2_w );
WRITE8_HANDLER( mario_sh3_w );

WRITE8_HANDLER( mario_sh_tuneselect_w );
WRITE8_HANDLER( masao_sh_irqtrigger_w );

MACHINE_CONFIG_EXTERN( mario_audio );
MACHINE_CONFIG_EXTERN( masao_audio );

#endif /*MARIO_H_*/
