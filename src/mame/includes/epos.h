// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Epos games

**************************************************************************/

class epos_state : public driver_device
{
public:
	epos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	uint8_t    m_palette;

	/* misc */
	int      m_counter;
	void dealer_decrypt_rom(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_prtc(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_dealer();
	virtual void machine_reset() override;
	void machine_start_epos();
	void machine_start_dealer();
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void get_pens( pen_t *pens );
	required_device<cpu_device> m_maincpu;
};
