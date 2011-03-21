/***************************************************************************
        Twincobr/Flying Shark/Wardner  game hardware from 1986-1987
        -----------------------------------------------------------
****************************************************************************/


#include "video/mc6845.h"


class twincobr_state : public driver_device
{
public:
	twincobr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int toaplan_main_cpu;
	int wardner_membank;
	UINT8 *sharedram;
	INT32 fg_rom_bank;
	INT32 bg_ram_bank;
	INT32 wardner_sprite_hack;
	int intenable;
	int dsp_on;
	int dsp_BIO;
	int fsharkbt_8741;
	int dsp_execute;
	UINT32 dsp_addr_w;
	UINT32 main_ram_seg;
	UINT16 *bgvideoram16;
	UINT16 *fgvideoram16;
	UINT16 *txvideoram16;
	size_t bgvideoram_size;
	size_t fgvideoram_size;
	size_t txvideoram_size;
	INT32 txscrollx;
	INT32 txscrolly;
	INT32 fgscrollx;
	INT32 fgscrolly;
	INT32 bgscrollx;
	INT32 bgscrolly;
	INT32 txoffs;
	INT32 fgoffs;
	INT32 bgoffs;
	INT32 scroll_x;
	INT32 scroll_y;
	INT32 display_on;
	INT32 flip_screen;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *tx_tilemap;
};


/*----------- defined in drivers/wardner.c -----------*/

STATE_POSTLOAD( wardner_restore_bank );

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

extern void twincobr_driver_savestate(running_machine *machine);
extern void wardner_driver_savestate(running_machine *machine);




/*----------- defined in video/twincobr.c -----------*/
extern const mc6845_interface twincobr_mc6845_intf;

extern void twincobr_flipscreen(running_machine *machine, int flip);
extern void twincobr_display(running_machine *machine, int enable);

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
SCREEN_UPDATE( toaplan0 );
SCREEN_EOF( toaplan0 );
