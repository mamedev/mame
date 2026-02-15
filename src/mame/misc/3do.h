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
	required_device<madam_device> m_madam;
	required_device<clio_device> m_clio;
	required_device<amy_device> m_amy;
	required_device<cr560b_device> m_cdrom;
	required_device<screen_device> m_screen;
	required_device_array<dac_16bit_r2r_twos_complement_device, 2> m_dac;
	memory_view m_overlay_view;
	required_device<address_map_bank_device> m_bankdev;
	required_ioport_array<2> m_p1_r;

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
};


#endif // MAME_MISC_3DO_H
