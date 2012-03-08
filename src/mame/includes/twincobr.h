/***************************************************************************
        Twincobr/Flying Shark/Wardner  game hardware from 1986-1987
        -----------------------------------------------------------
****************************************************************************/


#include "video/mc6845.h"
#include "video/bufsprite.h"


class twincobr_state : public driver_device
{
public:
	twincobr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	int m_toaplan_main_cpu;
	int m_wardner_membank;
	UINT8 *m_sharedram;
	INT32 m_fg_rom_bank;
	INT32 m_bg_ram_bank;
	INT32 m_wardner_sprite_hack;
	int m_intenable;
	int m_dsp_on;
	int m_dsp_BIO;
	int m_fsharkbt_8741;
	int m_dsp_execute;
	UINT32 m_dsp_addr_w;
	UINT32 m_main_ram_seg;
	UINT16 *m_bgvideoram16;
	UINT16 *m_fgvideoram16;
	UINT16 *m_txvideoram16;
	size_t m_bgvideoram_size;
	size_t m_fgvideoram_size;
	size_t m_txvideoram_size;
	INT32 m_txscrollx;
	INT32 m_txscrolly;
	INT32 m_fgscrollx;
	INT32 m_fgscrolly;
	INT32 m_bgscrollx;
	INT32 m_bgscrolly;
	INT32 m_txoffs;
	INT32 m_fgoffs;
	INT32 m_bgoffs;
	INT32 m_scroll_x;
	INT32 m_scroll_y;
	INT32 m_display_on;
	INT32 m_flip_screen;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	required_device<buffered_spriteram16_device> m_spriteram;
};


/*----------- defined in drivers/wardner.c -----------*/

void wardner_restore_bank(running_machine &machine);

/*----------- defined in machine/twincobr.c -----------*/

INTERRUPT_GEN( twincobr_interrupt );
INTERRUPT_GEN( wardner_interrupt );

WRITE16_HANDLER( twincobr_dsp_addrsel_w );
READ16_HANDLER(  twincobr_dsp_r );
WRITE16_HANDLER( twincobr_dsp_w );
WRITE16_HANDLER( twincobr_dsp_bio_w );
READ16_HANDLER ( twincobr_BIO_r );
WRITE16_HANDLER( twincobr_control_w );
READ16_HANDLER(  twincobr_sharedram_r );
WRITE16_HANDLER( twincobr_sharedram_w );
WRITE8_HANDLER(   twincobr_coin_w );
READ16_HANDLER(  fsharkbt_dsp_r );
WRITE16_HANDLER( fsharkbt_dsp_w );
WRITE16_HANDLER( fshark_coin_dsp_w );
WRITE16_HANDLER( wardner_dsp_addrsel_w );
READ16_HANDLER(  wardner_dsp_r );
WRITE16_HANDLER( wardner_dsp_w );
WRITE8_HANDLER(   wardner_control_w );
WRITE8_HANDLER(   wardner_coin_dsp_w );

MACHINE_RESET( twincobr );
MACHINE_RESET( fsharkbt );
MACHINE_RESET( wardner );

extern void twincobr_driver_savestate(running_machine &machine);
extern void wardner_driver_savestate(running_machine &machine);




/*----------- defined in video/twincobr.c -----------*/
extern const mc6845_interface twincobr_mc6845_intf;

extern void twincobr_flipscreen(running_machine &machine, int flip);
extern void twincobr_display(running_machine &machine, int enable);

READ16_HANDLER(  twincobr_txram_r );
READ16_HANDLER(  twincobr_bgram_r );
READ16_HANDLER(  twincobr_fgram_r );
WRITE16_HANDLER( twincobr_txram_w );
WRITE16_HANDLER( twincobr_bgram_w );
WRITE16_HANDLER( twincobr_fgram_w );
WRITE16_HANDLER( twincobr_txscroll_w );
WRITE16_HANDLER( twincobr_bgscroll_w );
WRITE16_HANDLER( twincobr_fgscroll_w );
WRITE16_HANDLER( twincobr_exscroll_w );
WRITE16_HANDLER( twincobr_txoffs_w );
WRITE16_HANDLER( twincobr_bgoffs_w );
WRITE16_HANDLER( twincobr_fgoffs_w );
WRITE8_HANDLER( wardner_videoram_w );
READ8_HANDLER(  wardner_videoram_r );
WRITE8_HANDLER( wardner_bglayer_w );
WRITE8_HANDLER( wardner_fglayer_w );
WRITE8_HANDLER( wardner_txlayer_w );
WRITE8_HANDLER( wardner_bgscroll_w );
WRITE8_HANDLER( wardner_fgscroll_w );
WRITE8_HANDLER( wardner_txscroll_w );
WRITE8_HANDLER( wardner_exscroll_w );
READ8_HANDLER(  wardner_sprite_r );
WRITE8_HANDLER( wardner_sprite_w );


VIDEO_START( toaplan0 );
SCREEN_UPDATE_IND16( toaplan0 );
