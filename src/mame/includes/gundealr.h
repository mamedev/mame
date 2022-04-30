// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Gun Dealer

*************************************************************************/

#include "machine/timer.h"
#include "emupal.h"
#include "tilemap.h"

class gundealr_state : public driver_device
{
public:
	gundealr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_paletteram(*this, "paletteram")
		, m_bg_videoram(*this, "bg_videoram")
		, m_fg_videoram(*this, "fg_videoram")
		, m_rambase(*this, "rambase")
		, m_mainbank(*this, "mainbank")
		, m_port_in(*this, "IN%u", 0)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void gundealr(machine_config &config);
	void gundealrbl(machine_config &config);
	void yamyam(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_rambase;

	required_memory_bank m_mainbank;
	optional_ioport_array<3> m_port_in;

	/* video-related */
	tilemap_t    *m_bg_tilemap = nullptr;
	tilemap_t    *m_fg_tilemap = nullptr;
	uint8_t      m_scroll[4]{};

	/* misc */
	int        m_input_ports_hack;
	void bankswitch_w(uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);
	template<int Xor> void fg_scroll_w(offs_t offset, uint8_t data);
	template<int Bit> void flipscreen_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(pagescan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(yamyam_mcu_sim);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void base_map(address_map &map);
	void gundealr_main_map(address_map &map);
	void main_portmap(address_map &map);
	void yamyam_main_map(address_map &map);
};
