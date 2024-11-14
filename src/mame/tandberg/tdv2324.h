// license:BSD-3-Clause
// copyright-holders:Curt Coder,Jonathan Gevaryahu
#ifndef MAME_TANDBERG_TDV2324_H
#define MAME_TANDBERG_TDV2324_H

#pragma once


#include "cpu/i8085/i8085.h"
#include "cpu/m6800/m6800.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"
#include "video/tms9927.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define P8085AH_0_TAG       "17f"
#define P8259A_TAG          "17d"
#define P8253_5_0_TAG       "17c"
#define P8253_5_1_TAG       "18c"
#define ER3400_TAG          "12a"

#define P8085AH_1_TAG       "6c"
#define TMS9937NL_TAG       "7e"
#define MK3887N4_TAG        "15d"

#define MC68B02P_TAG        "12b"
#define FD1797PL02_TAG      "fd1797"

#define SCREEN_TAG          "screen"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tdv2324_state : public driver_device
{
public:
	tdv2324_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, P8085AH_0_TAG)
		, m_subcpu(*this, P8085AH_1_TAG)
		, m_fdccpu(*this, MC68B02P_TAG)
		, m_sio(*this, MK3887N4_TAG)
		, m_pic(*this, P8259A_TAG)
		, m_pit0(*this, P8253_5_0_TAG)
		, m_pit1(*this, P8253_5_1_TAG)
		, m_tms(*this, TMS9937NL_TAG)
		, m_video_ram(*this, "video_ram")
	{ }

	void tdv2324(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<m6802_cpu_device> m_fdccpu;
	required_device<z80sio_device> m_sio;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit0;
	required_device<pit8253_device> m_pit1;
	required_device<tms9927_device> m_tms;

	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t tdv2324_main_io_30();
	uint8_t tdv2324_main_io_e6();
	void tdv2324_main_io_e2(uint8_t data);

	// video state
	required_shared_ptr<uint8_t> m_video_ram;
	void tdv2324_fdc_mem(address_map &map) ATTR_COLD;
	void tdv2324_io(address_map &map) ATTR_COLD;
	void tdv2324_mem(address_map &map) ATTR_COLD;
	void tdv2324_sub_io(address_map &map) ATTR_COLD;
	void tdv2324_sub_mem(address_map &map) ATTR_COLD;
};


#endif // MAME_TANDBERG_TDV2324_H
