#define NAMCOS1_MAX_BANK 0x400

/* Bank handler definitions */
struct bankhandler
{
	read8_space_func bank_handler_r;
	write8_space_func bank_handler_w;
	int           bank_offset;
	UINT8 *bank_pointer;
};

class namcos1_state : public driver_device
{
public:
	namcos1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_dac0_value;
	int m_dac1_value;
	int m_dac0_gain;
	int m_dac1_gain;
	UINT8 *m_paletteram;
	UINT8 *m_triram;
	UINT8 *m_s1ram;
	bankhandler m_bank_element[NAMCOS1_MAX_BANK];
	bankhandler m_active_bank[16];
	int m_key_id;
	int m_key_reg;
	int m_key_rng;
	int m_key_swap4_arg;
	int m_key_swap4;
	int m_key_bottom4;
	int m_key_top4;
	unsigned int m_key_quotient;
	unsigned int m_key_reminder;
	unsigned int m_key_numerator_high_word;
	UINT8 m_key[8];
	int m_mcu_patch_data;
	int m_reset;
	int m_wdog;
	int m_chip[16];
	UINT8 *m_videoram;
	UINT8 m_cus116[0x10];
	UINT8 *m_spriteram;
	UINT8 m_playfield_control[0x20];
	tilemap_t *m_bg_tilemap[6];
	UINT8 *m_tilemap_maskdata;
	int m_copy_sprites;
	UINT8 m_drawmode_table[16];
	DECLARE_WRITE8_MEMBER(namcos1_sub_firq_w);
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_WRITE8_MEMBER(firq_ack_w);
	DECLARE_READ8_MEMBER(dsw_r);
	DECLARE_WRITE8_MEMBER(namcos1_coin_w);
	DECLARE_WRITE8_MEMBER(namcos1_dac_gain_w);
	DECLARE_WRITE8_MEMBER(namcos1_dac0_w);
	DECLARE_WRITE8_MEMBER(namcos1_dac1_w);
	DECLARE_WRITE8_MEMBER(namcos1_sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(namcos1_cpu_control_w);
	DECLARE_WRITE8_MEMBER(namcos1_watchdog_w);
	DECLARE_WRITE8_MEMBER(namcos1_bankswitch_w);
	DECLARE_WRITE8_MEMBER(namcos1_subcpu_bank_w);
	DECLARE_WRITE8_MEMBER(namcos1_mcu_bankswitch_w);
	DECLARE_WRITE8_MEMBER(namcos1_mcu_patch_w);
	DECLARE_DRIVER_INIT(pacmania);
	DECLARE_DRIVER_INIT(ws);
	DECLARE_DRIVER_INIT(wldcourt);
	DECLARE_DRIVER_INIT(tankfrc4);
	DECLARE_DRIVER_INIT(blazer);
	DECLARE_DRIVER_INIT(dangseed);
	DECLARE_DRIVER_INIT(splatter);
	DECLARE_DRIVER_INIT(alice);
	DECLARE_DRIVER_INIT(faceoff);
	DECLARE_DRIVER_INIT(puzlclub);
	DECLARE_DRIVER_INIT(bakutotu);
	DECLARE_DRIVER_INIT(rompers);
	DECLARE_DRIVER_INIT(ws90);
	DECLARE_DRIVER_INIT(tankfrce);
	DECLARE_DRIVER_INIT(soukobdx);
	DECLARE_DRIVER_INIT(shadowld);
	DECLARE_DRIVER_INIT(berabohm);
	DECLARE_DRIVER_INIT(galaga88);
	DECLARE_DRIVER_INIT(blastoff);
	DECLARE_DRIVER_INIT(quester);
	DECLARE_DRIVER_INIT(ws89);
	DECLARE_DRIVER_INIT(dspirit);
	DECLARE_DRIVER_INIT(pistoldm);
	TILE_GET_INFO_MEMBER(bg_get_info0);
	TILE_GET_INFO_MEMBER(bg_get_info1);
	TILE_GET_INFO_MEMBER(bg_get_info2);
	TILE_GET_INFO_MEMBER(bg_get_info3);
	TILE_GET_INFO_MEMBER(fg_get_info4);
	TILE_GET_INFO_MEMBER(fg_get_info5);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_namcos1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_namcos1(screen_device &screen, bool state);
};

/*----------- defined in drivers/namcos1.c -----------*/

void namcos1_init_DACs(running_machine &machine);

/*----------- defined in video/namcos1.c -----------*/

DECLARE_READ8_HANDLER( namcos1_videoram_r );
DECLARE_WRITE8_HANDLER( namcos1_videoram_w );
DECLARE_WRITE8_HANDLER( namcos1_paletteram_w );
DECLARE_READ8_HANDLER( namcos1_spriteram_r );
DECLARE_WRITE8_HANDLER( namcos1_spriteram_w );
