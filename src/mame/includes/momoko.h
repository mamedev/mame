// license:BSD-3-Clause
// copyright-holders:Uki
/*************************************************************************

    Momoko 120%

*************************************************************************/

class momoko_state : public driver_device
{
public:
	momoko_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_bg_scrolly(*this, "bg_scrolly"),
		m_bg_scrollx(*this, "bg_scrollx"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_bg_scrolly;
	required_shared_ptr<uint8_t> m_bg_scrollx;

	/* video-related */
	uint8_t          m_fg_scrollx;
	uint8_t          m_fg_scrolly;
	uint8_t          m_fg_select;
	uint8_t          m_text_scrolly;
	uint8_t          m_text_mode;
	uint8_t          m_bg_select;
	uint8_t          m_bg_priority;
	uint8_t          m_bg_mask;
	uint8_t          m_fg_mask;
	uint8_t          m_flipscreen;
	void momoko_bg_read_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void momoko_fg_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void momoko_fg_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void momoko_fg_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void momoko_text_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void momoko_text_mode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void momoko_bg_scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void momoko_bg_scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void momoko_bg_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void momoko_bg_priority_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void momoko_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_momoko(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void momoko_draw_bg_pri( bitmap_ind16 &bitmap, int chr, int col, int flipx, int flipy, int x, int y, int pri );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
