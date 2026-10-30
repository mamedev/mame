// license:BSD-3-Clause
// copyright-holders:David Haywood, MetalliC
// Multifish


#include "sound/ay8910.h"
#include "cpu/z80/z80.h"
#include "machine/timekpr.h"
#include "machine/watchdog.h"
#include "machine/ticket.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class igrosoft_gamble_state : public driver_device
{
public:
	igrosoft_gamble_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_m48t35(*this, "m48t35" ),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_hopper(*this, "hopper"),
		m_mainbank(*this, "mainbank"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void rollfr(machine_config &config) ATTR_COLD;
	void igrosoft_gamble(machine_config &config) ATTR_COLD;

	void init_customl() ATTR_COLD;
	void init_island2l() ATTR_COLD;
	void init_keksl() ATTR_COLD;
	void init_pirate2l() ATTR_COLD;
	void init_fcockt2l() ATTR_COLD;
	void init_sweetl2l() ATTR_COLD;
	void init_gnomel() ATTR_COLD;
	void init_crzmonent() ATTR_COLD;
	void init_fcocktent() ATTR_COLD;
	void init_garageent() ATTR_COLD;
	void init_rclimbent() ATTR_COLD;
	void init_sweetl2ent() ATTR_COLD;
	void init_resdntent() ATTR_COLD;
	void init_island2ent() ATTR_COLD;
	void init_pirate2ent() ATTR_COLD;
	void init_keksent() ATTR_COLD;
	void init_gnomeent() ATTR_COLD;
	void init_lhauntent() ATTR_COLD;
	void init_fcockt2ent() ATTR_COLD;
	void init_crzmon2() ATTR_COLD;
	void init_crzmon2lot() ATTR_COLD;
	void init_crzmon2ent() ATTR_COLD;
	void init_islandent() ATTR_COLD;
	void init_pirateent() ATTR_COLD;
	void init_sweetlent() ATTR_COLD;
	void init_rollfruit() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	static constexpr unsigned ROM_SIZE = 0x80000;

	void vid_w(offs_t offset, uint8_t data);
	void rombank_w(uint8_t data);
	uint8_t bankedram_r(offs_t offset);
	void bankedram_w(offs_t offset, uint8_t data);
	void rambank_w(uint8_t data);
	uint8_t ray_r();
	void hopper_w(uint8_t data);
	void rollfr_hopper_w(uint8_t data);
	void lamps1_w(uint8_t data);
	void lamps2_w(uint8_t data);
	void lamps3_w(uint8_t data);
	void counters_w(uint8_t data);
	void f3_w(uint8_t data);
	void dispenable_w(uint8_t data);
	uint8_t timekeeper_r(offs_t offset);
	void timekeeper_w(offs_t offset, uint8_t data);

	void rom_decodel(uint8_t *romptr, uint8_t *tmprom, uint8_t xor_data, uint32_t xor_add) ATTR_COLD;
	void rom_decodeh(uint8_t *romptr, uint8_t *tmprom, uint8_t xor_data, uint32_t xor_add) ATTR_COLD;
	void lottery_decode(uint8_t xor12, uint8_t xor34, uint8_t xor56, uint8_t xor78, uint32_t xor_addr) ATTR_COLD;
	void roment_decodel(uint8_t *romptr, uint8_t *tmprom, uint8_t xor_data, uint32_t xor_add) ATTR_COLD;
	void roment_decodeh(uint8_t *romptr, uint8_t *tmprom, uint8_t xor_data, uint32_t xor_add) ATTR_COLD;
	void ent_decode(uint8_t xor12, uint8_t xor34, uint8_t xor56, uint8_t xor78, uint32_t xor_addr) ATTR_COLD;
	void newer_gfx_decode(uint8_t *gfx, uint32_t const (&addr_xor_plain)[2], uint8_t const (&pen_xor)[2]) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_static_tile_info);
	TILE_GET_INFO_MEMBER(get_reel_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void base_portmap(address_map &map) ATTR_COLD;
	void base_prgmap(address_map &map) ATTR_COLD;
	void rollfr_portmap(address_map &map) ATTR_COLD;

	// Video related

	uint8_t m_disp_enable = 0;
	int m_xor_paltype = 0;
	int m_xor_palette = 0;
	uint8_t m_xor_palindex = 0;
	bool m_cm2_priority = false;

	tilemap_t *m_tilemap = nullptr;
	tilemap_t *m_reel_tilemap = nullptr;

	// Misc related

	uint8_t m_rambk = 0;

	std::unique_ptr<uint8_t[]> m_vid;
	required_device<cpu_device> m_maincpu;
	required_device<timekeeper_device> m_m48t35;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ticket_dispenser_device> m_hopper;

	required_memory_bank m_mainbank;

	output_finder<13> m_lamps;
};


INPUT_PORTS_EXTERN( igrosoft_gamble );
