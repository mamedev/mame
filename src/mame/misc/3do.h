// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Wilbert Pol
/*****************************************************************************
 *
 * includes/3do.h
 *
 ****************************************************************************/

#ifndef MAME_MISC_3DO_H
#define MAME_MISC_3DO_H

#include "machine/bankdev.h"
#include "machine/cr560b.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "screen.h"

#include "3do_amy.h"
#include "3do_clio.h"
#include "3do_madam.h"
#include "3do_portfolio.h"

class _3do_state : public driver_device
{
public:
	_3do_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dram(*this, "dram", 0x200000, ENDIANNESS_BIG),
		m_vram(*this, "vram"),
		m_nvram(*this, "nvram"),
		m_madam(*this, "madam"),
		m_clio(*this, "clio"),
		m_amy(*this, "amy"),
		m_cdrom(*this, "cdrom"),
		m_screen(*this, "screen"),
		m_dac(*this, "dac%u", 0U),
		m_overlay_view(*this, "overlay_view"),
		m_bankdev(*this, "bankdev"),
		m_p1_r(*this, "P1.%u", 0)
	{ }

	void _3do(machine_config &config);
	void _3do_pal(machine_config &config);
	void arcade_ntsc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void green_config(machine_config &config);

private:
	struct SLOW2 {
		/* 03180000 - 0318003f - configuration group */
		/* 03180040 - 0318007f - diagnostic UART */

		uint8_t   cg_r_count = 0;
		uint8_t   cg_w_count = 0;
		uint32_t  cg_input = 0;
		uint32_t  cg_output = 0;
	};

	struct UNCLE {
		uint32_t  rev = 0;       /* 0340c000 */
		uint32_t  soft_rev = 0; /* 0340c004 */
		uint32_t  addr = 0;     /* 0340c008 */
	};

	void uncle_map(address_map &map);

	struct SVF {
		uint32_t  sport[512]{};
		uint32_t  color = 0;
	};

	required_device<cpu_device> m_maincpu;
	memory_share_creator<uint32_t> m_dram;
	required_shared_ptr<uint32_t> m_vram;
	required_device<nvram_device> m_nvram;
	// HACK: protected for adapting with Arcade systems
	// The only thing required being protected will eventually be the Player Bus only
protected:
	required_device<madam_device> m_madam;
private:
	required_device<clio_device> m_clio;
	required_device<amy_device> m_amy;
	required_device<cr560b_device> m_cdrom;
	required_device<screen_device> m_screen;
	required_device_array<dac_16bit_r2r_twos_complement_device, 2> m_dac;
	memory_view m_overlay_view;
	required_device<address_map_bank_device> m_bankdev;
protected:
	required_ioport_array<2> m_p1_r;
private:

	SLOW2 m_slow2;
	UNCLE m_uncle;
	SVF m_svf;
	uint8_t m_nvmem[0x8000]{};

//  uint8_t m_video_bits[512];
	uint8_t nvarea_r(offs_t offset);
	void nvarea_w(offs_t offset, uint8_t data);
	uint32_t slow2_r(offs_t offset);
	void slow2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t svf_r(offs_t offset);
	void svf_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void main_mem(address_map &map) ATTR_COLD;
	void bios_mem(address_map &map) ATTR_COLD;

	void m_slow2_init( void );

	void soft_reset_w(int state);
	TIMER_CALLBACK_MEMBER(soft_reset_cb);
};

class orbatak_state : public _3do_state
{
public:
	orbatak_state(const machine_config &mconfig, device_type type, const char *tag)
		: _3do_state(mconfig, type, tag)
		, m_track_p1_r(*this, "TRACK1.%u", 0)
		, m_track_p2_r(*this, "TRACK2.%u", 0)
		, m_raw_analog(*this, "RAW_ANALOG.%u", 0)
	{ }

	void orbatak(machine_config &config);

	template <unsigned P> ioport_value analog_0_r()
	{
		return (m_track_delta[P * 2] >> 6) & 0xf;
	}

	template <unsigned P> ioport_value analog_1_r()
	{
		const u8 track_y = (m_track_delta[P * 2] & 0x3f) << 2;
		const u8 track_x = (m_track_delta[P * 2 + 1] & 0x300) >> 8;
		return track_x | track_y;
	}

	template <unsigned P> ioport_value analog_2_r()
	{
		return m_track_delta[P * 2 + 1] & 0xff;
	}

private:
	required_ioport_array<3> m_track_p1_r;
	required_ioport_array<3> m_track_p2_r;
	required_ioport_array<4> m_raw_analog;
	u16 m_track_previous[4];
	u16 m_track_delta[4];
};

class alg_gun_state : public _3do_state
{
public:
	alg_gun_state(const machine_config &mconfig, device_type type, const char *tag)
		: _3do_state(mconfig, type, tag)
	{ }

	void alg_gun(machine_config &config);
};

#endif // MAME_MISC_3DO_H
