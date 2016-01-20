// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Ms.Pac-Man/Galaga - 20 Year Reunion hardware

    driver by Nicola Salmoria

***************************************************************************/
#include "machine/eepromser.h"
#include "sound/namco.h"
#include "machine/intelfsh.h"

class _20pacgal_state : public driver_device
{
public:
	_20pacgal_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_char_gfx_ram(*this, "char_gfx_ram"),
		m_stars_seed(*this, "stars_seed"),
		m_stars_ctrl(*this, "stars_ctrl"),
		m_flip(*this, "flip"),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_video_ram;
	required_shared_ptr<UINT8> m_char_gfx_ram;
	required_shared_ptr<UINT8> m_stars_seed;
	required_shared_ptr<UINT8> m_stars_ctrl;
	required_shared_ptr<UINT8> m_flip;

	/* machine state */
	UINT8 m_game_selected;  /* 0 = Ms. Pac-Man, 1 = Galaga */

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

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
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_20pacgal(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void set_bankptr();
	void get_pens(pen_t *pens);
	void do_pen_lookup(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_rgb32 &bitmap);
	void draw_chars(bitmap_rgb32 &bitmap);
	void draw_stars(bitmap_rgb32 &bitmap, const rectangle &cliprect );
	void draw_sprite(bitmap_rgb32 &bitmap, int y, int x,
						UINT8 code, UINT8 color, int flip_y, int flip_x);
	void common_save_state();
};


class _25pacman_state : public _20pacgal_state
{
public:
	_25pacman_state(const machine_config &mconfig, device_type type, std::string tag)
		: _20pacgal_state(mconfig, type, tag)
	{ }

	DECLARE_READ8_MEMBER( _25pacman_io_87_r );
	virtual void machine_start() override;
};

/*----------- defined in video/20pacgal.c -----------*/
MACHINE_CONFIG_EXTERN( 20pacgal_video );
