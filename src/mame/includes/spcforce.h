// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#include "sound/sn76496.h"

class spcforce_state : public driver_device
{
public:
	spcforce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sn1(*this, "sn1"),
		m_sn2(*this, "sn2"),
		m_sn3(*this, "sn3"),
		m_scrollram(*this, "scrollram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<sn76496_device> m_sn1;
	required_device<sn76496_device> m_sn2;
	required_device<sn76496_device> m_sn3;

	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	int m_sn76496_latch;
	int m_sn76496_select;
	int m_sn1_ready;
	int m_sn2_ready;
	int m_sn3_ready;
	uint8_t m_irq_mask;

	void SN76496_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t SN76496_select_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void SN76496_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_sn1_ready(int state);
	void write_sn2_ready(int state);
	void write_sn3_ready(int state);
	uint8_t t0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void soundtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	virtual void machine_start() override;
	void palette_init_spcforce(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(device_t &device);
};
