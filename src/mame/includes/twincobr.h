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
	DECLARE_WRITE16_MEMBER(twincobr_dsp_addrsel_w);
	DECLARE_READ16_MEMBER(twincobr_dsp_r);
	DECLARE_WRITE16_MEMBER(twincobr_dsp_w);
	DECLARE_WRITE16_MEMBER(wardner_dsp_addrsel_w);
	DECLARE_READ16_MEMBER(wardner_dsp_r);
	DECLARE_WRITE16_MEMBER(wardner_dsp_w);
	DECLARE_WRITE16_MEMBER(twincobr_dsp_bio_w);
	DECLARE_READ16_MEMBER(fsharkbt_dsp_r);
	DECLARE_WRITE16_MEMBER(fsharkbt_dsp_w);
	DECLARE_READ16_MEMBER(twincobr_BIO_r);
	DECLARE_WRITE16_MEMBER(twincobr_control_w);
	DECLARE_WRITE8_MEMBER(wardner_control_w);
	DECLARE_READ16_MEMBER(twincobr_sharedram_r);
	DECLARE_WRITE16_MEMBER(twincobr_sharedram_w);
	DECLARE_WRITE16_MEMBER(fshark_coin_dsp_w);
	DECLARE_WRITE8_MEMBER(twincobr_coin_w);
	DECLARE_WRITE8_MEMBER(wardner_coin_dsp_w);
	DECLARE_WRITE16_MEMBER(twincobr_txoffs_w);
	DECLARE_READ16_MEMBER(twincobr_txram_r);
	DECLARE_WRITE16_MEMBER(twincobr_txram_w);
	DECLARE_WRITE16_MEMBER(twincobr_bgoffs_w);
	DECLARE_READ16_MEMBER(twincobr_bgram_r);
	DECLARE_WRITE16_MEMBER(twincobr_bgram_w);
	DECLARE_WRITE16_MEMBER(twincobr_fgoffs_w);
	DECLARE_READ16_MEMBER(twincobr_fgram_r);
	DECLARE_WRITE16_MEMBER(twincobr_fgram_w);
	DECLARE_WRITE16_MEMBER(twincobr_txscroll_w);
	DECLARE_WRITE16_MEMBER(twincobr_bgscroll_w);
	DECLARE_WRITE16_MEMBER(twincobr_fgscroll_w);
	DECLARE_WRITE16_MEMBER(twincobr_exscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_txlayer_w);
	DECLARE_WRITE8_MEMBER(wardner_bglayer_w);
	DECLARE_WRITE8_MEMBER(wardner_fglayer_w);
	DECLARE_WRITE8_MEMBER(wardner_txscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_bgscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_fgscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_exscroll_w);
	DECLARE_READ8_MEMBER(wardner_videoram_r);
	DECLARE_WRITE8_MEMBER(wardner_videoram_w);
	DECLARE_READ8_MEMBER(wardner_sprite_r);
	DECLARE_WRITE8_MEMBER(wardner_sprite_w);
};


/*----------- defined in drivers/wardner.c -----------*/

void wardner_restore_bank(running_machine &machine);

/*----------- defined in machine/twincobr.c -----------*/

INTERRUPT_GEN( twincobr_interrupt );
INTERRUPT_GEN( wardner_interrupt );


MACHINE_RESET( twincobr );
MACHINE_RESET( fsharkbt );
MACHINE_RESET( wardner );

extern void twincobr_driver_savestate(running_machine &machine);
extern void wardner_driver_savestate(running_machine &machine);




/*----------- defined in video/twincobr.c -----------*/
extern const mc6845_interface twincobr_mc6845_intf;

extern void twincobr_flipscreen(running_machine &machine, int flip);
extern void twincobr_display(running_machine &machine, int enable);



VIDEO_START( toaplan0 );
SCREEN_UPDATE_IND16( toaplan0 );
