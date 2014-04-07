#include "sound/dac.h"
#include "sound/namco.h"

#define NAMCOS1_MAX_BANK 0x400

/* Bank handler definitions */
struct bankhandler
{
	read8_delegate bank_handler_r;
	write8_delegate bank_handler_w;
	int           bank_offset;
	UINT8 *bank_pointer;
};

class namcos1_state : public driver_device
{
public:
	namcos1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_cus30(*this, "namco"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_device<dac_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

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
	DECLARE_READ8_MEMBER(quester_paddle_r);
	DECLARE_READ8_MEMBER(berabohm_buttons_r);
	DECLARE_READ8_MEMBER(faceoff_inputs_r);
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
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_namcos1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_namcos1(screen_device &screen, bool state);
	void namcos1_update_DACs();
	void namcos1_init_DACs();
	DECLARE_READ8_MEMBER( namcos1_videoram_r );
	DECLARE_WRITE8_MEMBER( namcos1_videoram_w );
	DECLARE_WRITE8_MEMBER( namcos1_paletteram_w );
	DECLARE_READ8_MEMBER( namcos1_spriteram_r );
	DECLARE_WRITE8_MEMBER( namcos1_spriteram_w );
	inline UINT8 bank_r(address_space &space, offs_t offset, int bank);
	READ8_MEMBER( bank1_r );
	READ8_MEMBER( bank2_r );
	READ8_MEMBER( bank3_r );
	READ8_MEMBER( bank4_r );
	READ8_MEMBER( bank5_r );
	READ8_MEMBER( bank6_r );
	READ8_MEMBER( bank7_r );
	READ8_MEMBER( bank8_r );
	READ8_MEMBER( bank9_r );
	READ8_MEMBER( bank10_r );
	READ8_MEMBER( bank11_r );
	READ8_MEMBER( bank12_r );
	READ8_MEMBER( bank13_r );
	READ8_MEMBER( bank14_r );
	READ8_MEMBER( bank15_r );
	READ8_MEMBER( bank16_r );
	inline void bank_w(address_space &space, offs_t offset, UINT8 data, int bank);
	WRITE8_MEMBER( bank1_w );
	WRITE8_MEMBER( bank2_w );
	WRITE8_MEMBER( bank3_w );
	WRITE8_MEMBER( bank4_w );
	WRITE8_MEMBER( bank5_w );
	WRITE8_MEMBER( bank6_w );
	WRITE8_MEMBER( bank7_w );
	WRITE8_MEMBER( bank8_w );
	WRITE8_MEMBER( bank9_w );
	WRITE8_MEMBER( bank10_w );
	WRITE8_MEMBER( bank11_w );
	WRITE8_MEMBER( bank12_w );
	WRITE8_MEMBER( bank13_w );
	WRITE8_MEMBER( bank14_w );
	WRITE8_MEMBER( bank15_w );
	WRITE8_MEMBER( bank16_w );
	WRITE8_MEMBER( namcos1_3dcs_w );
	READ8_MEMBER( no_key_r );
	WRITE8_MEMBER( no_key_w );
	READ8_MEMBER( key_type1_r );
	WRITE8_MEMBER( key_type1_w );
	READ8_MEMBER( key_type2_r );
	WRITE8_MEMBER( key_type2_w );
	READ8_MEMBER( key_type3_r );
	WRITE8_MEMBER( key_type3_w );
	READ8_MEMBER( soundram_r );
	WRITE8_MEMBER( soundram_w );
	WRITE8_MEMBER( rom_w );
	READ8_MEMBER( unknown_r );
	WRITE8_MEMBER( unknown_w );
	void set_bank(int banknum, const bankhandler *handler);
	void namcos1_bankswitch(int cpu, offs_t offset, UINT8 data);
	void namcos1_install_bank(int start,int end,read8_delegate hr,write8_delegate hw,int offset,UINT8 *pointer);
	void namcos1_build_banks(read8_delegate key_r,write8_delegate key_w);
	struct namcos1_specific
	{
		/* keychip */
		read8_delegate key_r;
		write8_delegate key_w;
		int key_id;
		int key_reg1;
		int key_reg2;
		int key_reg3;
		int key_reg4;
		int key_reg5;
		int key_reg6;
	};

	void namcos1_driver_init(const struct namcos1_specific *specific );
private:
	inline void bg_get_info(tile_data &tileinfo,int tile_index,UINT8 *info_vram);
	inline void fg_get_info(tile_data &tileinfo,int tile_index,UINT8 *info_vram);
};
