// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Crystal Castles hardware

*************************************************************************/

#include "cpu/m6502/m6502.h"
#include "machine/x2212.h"

class ccastles_state : public driver_device
{
public:
	ccastles_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_nvram_4b(*this, "nvram_4b"),
			m_nvram_4a(*this, "nvram_4a") ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"){ }

	/* devices */
	required_device<m6502_device> m_maincpu;
	required_device<x2212_device> m_nvram_4b;
	required_device<x2212_device> m_nvram_4a;
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	const uint8_t *m_syncprom;
	const uint8_t *m_wpprom;
	const uint8_t *m_priprom;
	bitmap_ind16 m_spritebitmap;
	double m_rweights[3];
	double m_gweights[3];
	double m_bweights[3];
	uint8_t m_video_control[8];
	uint8_t m_bitmode_addr[2];
	uint8_t m_hscroll;
	uint8_t m_vscroll;

	/* misc */
	int      m_vblank_start;
	int      m_vblank_end;
	emu_timer *m_irq_timer;
	uint8_t    m_irq_state;
	uint8_t    m_nvram_store[2];

	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ccounter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t leta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nvram_recall_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nvram_store_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nvram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nvram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ccastles_hscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ccastles_vscroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ccastles_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ccastles_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ccastles_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ccastles_bitmode_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ccastles_bitmode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ccastles_bitmode_addr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value get_vblank(ioport_field &field, void *param);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_ccastles(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void clock_irq(void *ptr, int32_t param);
	inline void ccastles_write_vram( uint16_t addr, uint8_t data, uint8_t bitmd, uint8_t pixba );
	inline void bitmode_autoinc(  );
	inline void schedule_next_irq( int curscanline );
};
