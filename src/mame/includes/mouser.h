// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/*************************************************************************

    Mouser

*************************************************************************/

class mouser_state : public driver_device
{
public:
	mouser_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* misc */
	uint8_t      m_sound_byte;
	uint8_t      m_nmi_enable;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	void mouser_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mouser_sound_interrupt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mouser_sound_byte_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mouser_sound_nmi_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mouser_flip_screen_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mouser_flip_screen_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_mouser();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void palette_init_mouser(palette_device &palette);
	uint32_t screen_update_mouser(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mouser_nmi_interrupt(device_t &device);
	void mouser_sound_nmi_assert(device_t &device);
};
