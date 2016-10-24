// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

    Mr. Flea

*************************************************************************/

class mrflea_state : public driver_device
{
public:
	mrflea_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	int     m_gfx_bank;

	/* misc */
	int m_io;
	int m_main;
	int m_status;
	int m_select1;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void mrflea_main_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrflea_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mrflea_main_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mrflea_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mrflea_main_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mrflea_io_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mrflea_interrupt_type_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mrflea_select1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mrflea_input1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mrflea_data1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrflea_gfx_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrflea_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrflea_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_mrflea(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mrflea_slave_interrupt(timer_device &timer, void *ptr, int32_t param);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
