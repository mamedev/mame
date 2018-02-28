// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "screen.h"

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
	DECLARE_WRITE16_MEMBER(flipscreen_w);
	DECLARE_WRITE16_MEMBER(gfxlist_addr_w);
	DECLARE_WRITE16_MEMBER(blitter_go_w);
	DECLARE_WRITE16_MEMBER(irq_ack_w);

	// shangha3 specific
	DECLARE_READ16_MEMBER(shangha3_prot_r);
	DECLARE_WRITE16_MEMBER(shangha3_prot_w);
	DECLARE_WRITE16_MEMBER(shangha3_coinctrl_w);

	// heberpop specific
	DECLARE_WRITE16_MEMBER(heberpop_coinctrl_w);
	DECLARE_WRITE16_MEMBER(heberpop_sound_command_w); // used by blocken too

	// blocken specific
	DECLARE_WRITE16_MEMBER(blocken_coinctrl_w);

	DECLARE_DRIVER_INIT(shangha3);
	DECLARE_DRIVER_INIT(heberpop);
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void shangha3(machine_config &config);
	void heberpop(machine_config &config);
	void blocken(machine_config &config);
	void blocken_map(address_map &map);
	void heberpop_map(address_map &map);
	void heberpop_sound_io_map(address_map &map);
	void heberpop_sound_map(address_map &map);
	void shangha3_map(address_map &map);
};
