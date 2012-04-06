/*************************************************************************

    Namco PuckMan

**************************************************************************/

class pacman_state : public driver_device
{
public:
	pacman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram2(*this, "spriteram2"),
		  m_s2650_spriteram(*this, "s2650_spriteram") { }

	UINT8 m_cannonb_bit_to_read;
	int m_mystery;
	UINT8 m_counter;
	int m_bigbucks_bank;
	UINT8 *m_rocktrv2_prot_data;
	UINT8 m_rocktrv2_question_bank;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_s2650games_tileram;
	tilemap_t *m_bg_tilemap;
	UINT8 m_charbank;
	UINT8 m_spritebank;
	UINT8 m_palettebank;
	UINT8 m_colortablebank;
	UINT8 m_flipscreen;
	UINT8 m_bgpriority;
	int m_xoffsethack;
	UINT8 m_inv_spr;
	UINT8 m_irq_mask;
	optional_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_spriteram2;
	optional_shared_ptr<UINT8> m_s2650_spriteram;
	DECLARE_WRITE8_MEMBER(pacman_interrupt_vector_w);
	DECLARE_WRITE8_MEMBER(piranha_interrupt_vector_w);
	DECLARE_WRITE8_MEMBER(nmouse_interrupt_vector_w);
	DECLARE_WRITE8_MEMBER(pacman_leds_w);
	DECLARE_WRITE8_MEMBER(pacman_coin_counter_w);
	DECLARE_WRITE8_MEMBER(pacman_coin_lockout_global_w);
	DECLARE_WRITE8_MEMBER(alibaba_sound_w);
	DECLARE_READ8_MEMBER(alibaba_mystery_1_r);
	DECLARE_READ8_MEMBER(alibaba_mystery_2_r);
	DECLARE_READ8_MEMBER(maketrax_special_port2_r);
	DECLARE_READ8_MEMBER(maketrax_special_port3_r);
	DECLARE_READ8_MEMBER(korosuke_special_port2_r);
	DECLARE_READ8_MEMBER(korosuke_special_port3_r);
	DECLARE_READ8_MEMBER(mschamp_kludge_r);
	DECLARE_WRITE8_MEMBER(bigbucks_bank_w);
	DECLARE_READ8_MEMBER(bigbucks_question_r);
	DECLARE_WRITE8_MEMBER(porky_banking_w);
	DECLARE_READ8_MEMBER(drivfrcp_port1_r);
	DECLARE_READ8_MEMBER(_8bpm_port1_r);
	DECLARE_READ8_MEMBER(porky_port1_r);
	DECLARE_READ8_MEMBER(rocktrv2_prot1_data_r);
	DECLARE_READ8_MEMBER(rocktrv2_prot2_data_r);
	DECLARE_READ8_MEMBER(rocktrv2_prot3_data_r);
	DECLARE_READ8_MEMBER(rocktrv2_prot4_data_r);
	DECLARE_WRITE8_MEMBER(rocktrv2_prot_data_w);
	DECLARE_WRITE8_MEMBER(rocktrv2_question_bank_w);
	DECLARE_READ8_MEMBER(rocktrv2_question_r);
	DECLARE_READ8_MEMBER(pacman_read_nop);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x0038);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x03b0);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x1600);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x2120);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x3ff0);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x8000);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x97f0);
	DECLARE_WRITE8_MEMBER(mspacman_disable_decode_w);
	DECLARE_READ8_MEMBER(mspacman_enable_decode_r_0x3ff8);
	DECLARE_WRITE8_MEMBER(mspacman_enable_decode_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_READ8_MEMBER(cannonbp_protection_r);
	DECLARE_WRITE8_MEMBER(pacman_videoram_w);
	DECLARE_WRITE8_MEMBER(pacman_colorram_w);
	DECLARE_WRITE8_MEMBER(pacman_flipscreen_w);
	DECLARE_WRITE8_MEMBER(pengo_palettebank_w);
	DECLARE_WRITE8_MEMBER(pengo_colortablebank_w);
	DECLARE_WRITE8_MEMBER(pengo_gfxbank_w);
	DECLARE_WRITE8_MEMBER(s2650games_videoram_w);
	DECLARE_WRITE8_MEMBER(s2650games_colorram_w);
	DECLARE_WRITE8_MEMBER(s2650games_scroll_w);
	DECLARE_WRITE8_MEMBER(s2650games_tilesbank_w);
	DECLARE_WRITE8_MEMBER(jrpacman_videoram_w);
	DECLARE_WRITE8_MEMBER(jrpacman_charbank_w);
	DECLARE_WRITE8_MEMBER(jrpacman_spritebank_w);
	DECLARE_WRITE8_MEMBER(jrpacman_scroll_w);
	DECLARE_WRITE8_MEMBER(jrpacman_bgpriority_w);
};


/*----------- defined in video/pacman.c -----------*/

PALETTE_INIT( pacman );
VIDEO_START( pacman );
SCREEN_UPDATE_IND16( pacman );



VIDEO_START( pengo );



VIDEO_START( s2650games );
SCREEN_UPDATE_IND16( s2650games );




VIDEO_START( jrpacman );


VIDEO_START( birdiy );


/*----------- defined in machine/pacplus.c -----------*/

void pacplus_decode(running_machine &machine);


/*----------- defined in machine/jumpshot.c -----------*/

void jumpshot_decode(running_machine &machine);


/*----------- defined in machine/theglobp.c -----------*/

MACHINE_START( theglobp );
MACHINE_RESET( theglobp );
READ8_HANDLER( theglobp_decrypt_rom );


/*----------- defined in machine/acitya.c -------------*/

MACHINE_START( acitya );
MACHINE_RESET( acitya );
READ8_HANDLER( acitya_decrypt_rom );
