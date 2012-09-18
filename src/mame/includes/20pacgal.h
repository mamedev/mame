/***************************************************************************

    Ms.Pac-Man/Galaga - 20 Year Reunion hardware

    driver by Nicola Salmoria

***************************************************************************/


class _20pacgal_state : public driver_device
{
public:
	_20pacgal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_video_ram(*this, "video_ram"),
		m_char_gfx_ram(*this, "char_gfx_ram"),
		m_stars_seed(*this, "stars_seed"),
		m_stars_ctrl(*this, "stars_ctrl"),
		m_flip(*this, "flip"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_video_ram;
	required_shared_ptr<UINT8> m_char_gfx_ram;
	required_shared_ptr<UINT8> m_stars_seed;
	required_shared_ptr<UINT8> m_stars_ctrl;
	required_shared_ptr<UINT8> m_flip;

	/* machine state */
	UINT8 m_game_selected;	/* 0 = Ms. Pac-Man, 1 = Galaga */

	/* devices */
	cpu_device *m_maincpu;
	device_t *m_eeprom;

	/* memory */
	UINT8 m_sprite_gfx_ram[0x2000];
	UINT8 m_sprite_ram[0x180];
	UINT8 m_sprite_color_lookup[0x100];
	UINT8 m_ram_48000[0x2000];

	/* 25pacman and 20pacgal store the sprite palette at a different address, this is a hardware difference and confirmed NOT to be a register */
	UINT8 m_sprite_pal_base;

	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(irqack_w);
	DECLARE_WRITE8_MEMBER(timer_pulse_w);
	DECLARE_WRITE8_MEMBER(_20pacgal_coin_counter_w);
	DECLARE_WRITE8_MEMBER(ram_bank_select_w);
	DECLARE_WRITE8_MEMBER(ram_48000_w);
	DECLARE_WRITE8_MEMBER(sprite_gfx_w);
	DECLARE_WRITE8_MEMBER(sprite_ram_w);
	DECLARE_WRITE8_MEMBER(sprite_lookup_w);
	DECLARE_DRIVER_INIT(25pacman);
	DECLARE_DRIVER_INIT(20pacgal);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_20pacgal(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/20pacgal.c -----------*/
MACHINE_CONFIG_EXTERN( 20pacgal_video );
