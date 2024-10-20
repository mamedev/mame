// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SGI_SGI_GR1_H
#define MAME_SGI_SGI_GR1_H

#pragma once

#include "sgi_ge5.h"
#include "sgi_re2.h"
#include "sgi_xmap2.h"

#include "machine/bankdev.h"
#include "video/bt45x.h"
#include "video/bt431.h"

#include "screen.h"


class sgi_gr1_device : public device_t
{
public:
	sgi_gr1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// configuration
	auto out_vblank() { return m_screen.lookup()->screen_vblank(); }
	auto out_int() { return m_ge.lookup()->out_int(); }
	auto out_int_fifo() { return m_int_fifo_cb.bind(); }

	u32 dma_r() { return m_ge.lookup()->buffer_r(0); }
	void dma_w(u32 data) { fifo_w(0, data, 0xffffffffU); }

	void reset_w(int state);

	void map(address_map &map) ATTR_COLD;

protected:
	void map_bank(address_map &map) ATTR_COLD;

	// device_t overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// display registers
	u8 dr0_r();
	u8 dr1_r();
	u8 dr2_r();
	u8 dr3_r();
	u8 dr4_r();
	void dr0_w(u8 data);
	void dr1_w(u8 data);
	void dr2_w(u8 data);
	void dr3_w(u8 data);
	void dr4_w(u8 data);

	u64 fifo_r();
	void fifo_w(offs_t offset, u32 data, u32 mem_mask);

private:
	required_device<address_map_bank_device> m_bank;
	required_device<screen_device> m_screen;
	required_device<sgi_ge5_device> m_ge;
	required_device<sgi_re2_device> m_re;
	required_device_array<sgi_xmap2_device, 5> m_xmap;
	required_device_array<bt431_device, 2> m_cursor;
	required_device_array<bt457_device, 3> m_ramdac;

	devcb_write_line m_int_fifo_cb;

	enum dr0_mask : u8
	{
		DR0_GRF1EN    = 0x01, // grf1 board enable (active low, disable for RE2)
		DR0_PGRINBIT  = 0x02, // reflects PGROUTBIT (PGR)
		DR0_PGROUTBIT = 0x04, // routed to PGRINBIT (PGR)
		DR0_ZBUF0     = 0x08, // mzb1 card is installed (active low, ro, MGR)
		DR0_SMALLMON0 = 0x08, // small monitor installed (active low, non-MGR)

		DR0_WM        = 0xf7, // write mask
	};
	enum dr1_mask : u8
	{
		DR1_SE        = 0x01, // sync on green enable (active low, rw)
		DR1_CWEN      = 0x02, // wtl3132 cwen-
		DR1_VRLS      = 0x04, // vertical retrace latency select
		DR1_MTYPE     = 0x06, // monitor type msb (rw)
		DR1_TURBO     = 0x08, // turbo option installed (active low, ro)
		DR1_OVERLAY0A = 0x10, // dac overlay bit 0 bank a (ro)

		DR1_WM        = 0xe7, // write mask
	};
	enum dr2_mask : u8
	{
		DR2_SCREENON  = 0x01, // standby (rw)
		DR2_UNCOM2    = 0x02, // uncomitted bit to xilinx
		DR2_LEDOFF    = 0x04, // disable led
		DR2_BITPLANES = 0x08, // extra bitplanes installed (active low, ro)
		DR2_ZBUF      = 0x10, // z-buffer installed (active low, non-MGR, ro)

		DR2_WM        = 0xe7, // write mask
	};
	enum dr3_mask : u8
	{
		DR3_GENSTATEN    = 0x01, // enable genlock status out
		DR3_LSBBLUEOUT   = 0x01, // latch blue lsb out (VGR only)
		DR3_LCARESET     = 0x02, // reset xilinx lca (active low, rw)
		DR3_MONITORRESET = 0x04, // reset monitor type (rw)
		DR3_FIFOEMPTY    = 0x08, // fifo empty (active low, ro)
		DR3_FIFOFULL     = 0x10, // fifo half full (active low, ro)

		DR3_WM           = 0xe7, // write mask
	};
	enum dr4_mask : u8
	{
		DR4_MONITORMASK = 0x03, // monitor type lsb (rw)
		DR4_EXTCLKSEL   = 0x04, // select external pixel clock (rw)
		DR4_MEGOPT      = 0x08, // 1M video rams installed (ro)
		DR4_GESTALL     = 0x10, // ge stalled (active low, ro)
		DR4_ACLKEN      = 0x20, // asynchronous clock enabled (wo)
		DR4_SCLKEN      = 0x40, // synchronous clock enabled (wo)
		DR4_MS          = 0x80, // select upper 4K color map (rw)

		DR4_RM          = 0x9f, // read mask
		DR4_WM          = 0xe7, // write mask
	};

	u8 m_dr0;
	u8 m_dr1;
	u8 m_dr2;
	u8 m_dr3;
	u8 m_dr4;

	util::fifo<u64, 512> m_fifo;

	bool m_reset;
};

DECLARE_DEVICE_TYPE(SGI_GR1, sgi_gr1_device)

#endif // MAME_SGI_SGI_GR1_H
