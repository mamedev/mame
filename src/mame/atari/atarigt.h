// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari GT hardware

*************************************************************************/

#include "cage.h"
#include "machine/adc0808.h"
#include "atarigen.h"
#include "atarixga.h"
#include "machine/timer.h"
#include "atarirle.h"
#include "emupal.h"
#include "tilemap.h"


class atarigt_state : public atarigen_state
{
public:
	atarigt_state(const machine_config &mconfig, device_type type, const char *tag) :
		atarigen_state(mconfig, type, tag),
		m_palette(*this, "palette"),
		m_colorram(*this, "colorram", 0x80000, ENDIANNESS_BIG),
		m_adc(*this, "adc"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_rle(*this, "rle"),
		m_mo_command(*this, "mo_command"),
		m_cage(*this, "cage"),
		m_xga(*this, "xga"),
		m_service_io(*this, "SERVICE"),
		m_coin_io(*this, "COIN"),
		m_fake_io(*this, "FAKE")
	{ }

	void atarigt(machine_config &config) ATTR_COLD;
	void atarigt_stereo(machine_config &config) ATTR_COLD;
	void tmek(machine_config &config) ATTR_COLD;
	void tmek20(machine_config &config) ATTR_COLD;
	void primrage20(machine_config &config) ATTR_COLD;
	void primrage(machine_config &config) ATTR_COLD;

	void init_primrage() ATTR_COLD;
	void init_tmek() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<palette_device> m_palette;
	memory_share_creator<uint16_t> m_colorram;

	optional_device<adc0808_device> m_adc;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;

	required_shared_ptr<uint32_t> m_mo_command;
	required_device<atari_cage_device> m_cage;
	optional_device<atari_gt_xga_device> m_xga;

	optional_ioport m_service_io;
	optional_ioport m_coin_io;
	optional_ioport m_fake_io;

	bool            m_is_primrage = false;

	bool            m_scanline_int_state = false;
	bool            m_video_int_state = false;

	bitmap_ind16    m_pf_bitmap;
	bitmap_ind16    m_an_bitmap;

	uint8_t         m_playfield_tile_bank = 0;
	uint8_t         m_playfield_color_bank = 0;
	uint16_t        m_playfield_xscroll = 0;
	uint16_t        m_playfield_yscroll = 0;

	uint32_t        m_tram_checksum = 0;

	bool            m_ignore_writes = false;

	INTERRUPT_GEN_MEMBER(scanline_int_gen);
	void video_int_write_line(int state);
	void scanline_int_ack_w(uint32_t data = 0);
	void video_int_ack_w(uint32_t data = 0);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);
	uint32_t special_port2_r();
	uint32_t special_port3_r();
	uint8_t analog_port_r(offs_t offset);
	void latch_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void mo_command_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void led_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sound_data_r(offs_t offset, uint32_t mem_mask = ~0);
	void sound_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t colorram_protection_r(offs_t offset, uint32_t mem_mask = ~0);
	void colorram_protection_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void cage_irq_callback(uint8_t data);

	void colorram_w(offs_t address, uint16_t data, uint16_t mem_mask);
	uint16_t colorram_r(offs_t address);

	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILEMAP_MAPPER_MEMBER(playfield_scan);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void compute_fake_pots(int *pots);
};
