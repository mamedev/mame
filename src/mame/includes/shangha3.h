// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#include "machine/gen_latch.h"
#include "sound/okim6295.h"

class shangha3_state : public driver_device
{
public:
	shangha3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_ram(*this, "ram") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_ram;

	// driver init configuration
	int m_do_shadows;
	uint8_t m_drawmode_table[16];

	int m_prot_count;
	uint16_t m_gfxlist_addr;
	bitmap_ind16 m_rawbitmap;

	// common
	void flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gfxlist_addr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_go_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// shangha3 specific
	uint16_t shangha3_prot_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void shangha3_prot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void shangha3_coinctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// heberpop specific
	void heberpop_coinctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void heberpop_sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff); // used by blocken too

	// blocken specific
	void blocken_coinctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_shangha3();
	void init_heberpop();
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
