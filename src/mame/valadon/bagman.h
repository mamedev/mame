// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VALADON_BAGMAN_H
#define MAME_VALADON_BAGMAN_H

#pragma once


#include "machine/74259.h"
#include "sound/ay8910.h"
#include "sound/tms5110.h"
#include "emupal.h"
#include "tilemap.h"

class bagman_state : public driver_device
{
public:
	bagman_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_tmsprom(*this, "tmsprom"),
		m_tmslatch(*this, "tmslatch"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram")
	{ }

	void sbagman(machine_config &config);
	void bagman(machine_config &config);
	void sbagmani(machine_config &config);

	void init_bagmans3();
	void init_botanic2();

protected:
	// common
	void coin_counter_w(int state);
	void irq_mask_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void video_enable_w(int state);

	// bagman
	void ls259_w(offs_t offset, uint8_t data);
	void tmsprom_bit_w(int state);
	void tmsprom_csq0_w(int state);
	void tmsprom_csq1_w(int state);
	void pal16r6_w(offs_t offset, uint8_t data);
	uint8_t pal16r6_r();

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void bagman_palette(palette_device &palette) const;

	void vblank_irq(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_pal();
	void bagman_base(machine_config &config);
	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_mainlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<tmsprom_device> m_tmsprom;
	optional_device<ls259_device> m_tmslatch;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	bool m_irq_mask = false;
	bool m_video_enable = false;

	/*table holds outputs of all ANDs (after AND map)*/
	uint8_t m_andmap[64]{};

	/*table holds inputs (ie. not x, x, not q, q) to the AND map*/
	uint8_t m_columnvalue[32]{};

	/*8 output pins (actually 6 output and 2 input/output)*/
	uint8_t m_outvalue[8]{};

	tilemap_t *m_bg_tilemap = nullptr;
};


class pickin_state : public bagman_state
{
public:
	pickin_state(const machine_config &mconfig, device_type type, const char *tag) :
		bagman_state(mconfig, type, tag),
		m_aysnd(*this, {"aysnd", "ay2"})
	{ }

	void pickin(machine_config &config);
	void botanic(machine_config &config);

private:
	uint8_t aysnd_r();
	void aysnd_w(offs_t offset, uint8_t data);

	void pickin_map(address_map &map) ATTR_COLD;
	void pickin_portmap(address_map &map) ATTR_COLD;

	required_device_array<ay8910_device, 2> m_aysnd;
};


class squaitsa_state : public pickin_state
{
public:
	squaitsa_state(const machine_config &mconfig, device_type type, const char *tag) :
		pickin_state(mconfig, type, tag),
		m_dial(*this, "DIAL_P%u", 1),
		m_res{ 0, 0 },
		m_old_val{ 0, 0 }
	{ }

	template <unsigned N> ioport_value dial_input_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_ioport_array<2> m_dial;
	uint8_t m_res[2];
	uint8_t m_old_val[2];
};


/*----------- timings -----------*/

#define BAGMAN_MAIN_CLOCK   XTAL(18'432'000)
#define BAGMAN_HCLK         (BAGMAN_MAIN_CLOCK / 3)
#define BAGMAN_H0           (BAGMAN_HCLK / 2)
#define BAGMAN_H1           (BAGMAN_H0   / 2)
#define HTOTAL              ((0x100-0x40)*2)
#define HBEND               (0x00)
#define HBSTART             (0x100)
#define VTOTAL              ((0x100-0x7c)*2)

/* the following VBEND/VBSTART are used for compsync
 * #define VBEND                (0x08)
 * #define VBSTART              (0x100)
 *
 * However VBSYQ (and INTQ) is generated using the following values:
 */
#define VBEND               (0x10)
#define VBSTART             (0xf0)

#endif // MAME_VALADON_BAGMAN_H
