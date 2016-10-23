// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Ernesto Corvi, Nicola Salmoria
#include "sound/namco.h"
#include "sound/samples.h"
#include "machine/namcoio.h"

#define MAX_STARS           250

struct star {
	float x,y;
	int col,set;
};


class gaplus_state : public driver_device
{
public:
	enum
	{
		TIMER_NAMCOIO_RUN
	};

	enum
	{
		GAME_GAPLUS = 0,
		GAME_GAPLUSD,
		GAME_GALAGA3
	};

	gaplus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_namco_15xx(*this, "namco"),
		m_samples(*this, "samples") ,
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_customio_3(*this,"customio_3"),
		m_videoram(*this,"videoram"),
		m_spriteram(*this,"spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	required_device<namco_15xx_device> m_namco_15xx;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	namco58xx_device *m_namco58xx;
	namco56xx_device *m_namco56xx;

	required_shared_ptr<uint8_t> m_customio_3;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_type;

	tilemap_t *m_bg_tilemap;
	uint8_t m_starfield_control[4];
	int m_total_stars;
	struct star m_stars[MAX_STARS];
	uint8_t m_main_irq_mask;
	uint8_t m_sub_irq_mask;
	uint8_t m_sub2_irq_mask;

	void irq_1_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_2_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_3_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sreset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void freset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void customio_3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t customio_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void starfield_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void out_lamps0(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void out_lamps1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_gaplus();
	void init_gaplusd();
	void init_galaga3();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(gaplus);

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);

	INTERRUPT_GEN_MEMBER(vblank_main_irq);
	INTERRUPT_GEN_MEMBER(gapluso_vblank_main_irq);
	INTERRUPT_GEN_MEMBER(vblank_sub_irq);
	INTERRUPT_GEN_MEMBER(vblank_sub2_irq);
	TIMER_CALLBACK_MEMBER(namcoio_run);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void starfield_init();
	void starfield_render(bitmap_ind16 &bitmap);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
