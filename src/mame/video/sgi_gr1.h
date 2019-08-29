// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_VIDEO_SGI_GR1_H
#define MAME_VIDEO_SGI_GR1_H

#pragma once

#include "machine/bankdev.h"
#include "screen.h"
#include "video/sgi_ge5.h"
#include "video/sgi_re2.h"
#include "video/bt45x.h"
#include "video/bt431.h"

class sgi_gr1_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// configuration
	auto out_vblank() { return m_vblank_cb.bind(); }
	auto out_int_ge() { return subdevice<sgi_ge5_device>("ge5")->out_int(); }
	auto out_int_fifo() { return m_int_fifo_cb.bind(); }

	void reset_w(int state);

	virtual void map(address_map &map);

protected:
	sgi_gr1_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect) = 0;

	u8 dr0_r() { return m_dr0; }
	u8 dr1_r() { return m_dr1; }
	u8 dr2_r() { return m_dr2; }
	u8 dr3_r() { return m_dr3 | (m_fifo.empty() ? 0 : DR3_FIFOEMPTY); }
	u8 dr4_r() { return m_dr4 & DR4_RM; }
	void dr0_w(u8 data) { m_dr0 = (m_dr0 & ~DR0_WM) | (data & DR0_WM); }
	void dr1_w(u8 data) { m_dr1 = (m_dr1 & ~DR1_WM) | (data & DR1_WM); m_ge->cwen_w(BIT(data, 1)); }
	void dr2_w(u8 data) { m_dr2 = (m_dr2 & ~DR2_WM) | (data & DR2_WM); }
	void dr3_w(u8 data) { m_dr3 = (m_dr3 & ~DR3_WM) | (data & DR3_WM); }
	void dr4_w(u8 data) { m_dr4 = (m_dr4 & ~DR4_WM) | (data & DR4_WM); }

	void fifo_w(offs_t offset, u32 data, u32 mem_mask);

//private:
	required_device<screen_device> m_screen;
	required_device<sgi_re2_device> m_re;
	required_device<sgi_ge5_device> m_ge;
	required_device_array<bt431_device, 2> m_cursor;

	devcb_write_line m_vblank_cb;
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

		DR1_WM        = 0xf7, // write mask
	};
	enum dr2_mask : u8
	{
		DR2_SCREENON  = 0x01, // standby (rw)
		DR2_UNCOM2    = 0x02, // uncomitted bit to xilinx
		DR2_LEDOFF    = 0x04, // disable led
		DR2_BITPLANES = 0x08, // extra bitplanes installed (active low, ro)
		DR2_ZBUF      = 0x10, // z-buffer installed (active low, non-MGR, ro)

		DR2_WM        = 0xf1, // write mask
	};
	enum dr3_mask : u8
	{
		DR3_GENSTATEN    = 0x01, // enable genlock status out
		DR3_LSBBLUEOUT   = 0x01, // latch blue lsb out (VGR only)
		DR3_LCARESET     = 0x02, // reset xilix lca (active low, rw)
		DR3_MONITORRESET = 0x04, // monitor set (rw)
		DR3_FIFOEMPTY    = 0x08, // fifo empty (active low, ro)
		DR3_FIFOFULL     = 0x10, // fifo half full (active low, ro)

		DR3_WM           = 0xf7, // write mask
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
		DR4_WM          = 0xff, // write mask
	};

	u8 m_dr0;
	u8 m_dr1;
	u8 m_dr2;
	u8 m_dr3;
	u8 m_dr4;

	util::fifo<u64, 512> m_fifo;
	u32 m_ff[2];
};

class sgi_gr11_device : public sgi_gr1_device
{
public:
	sgi_gr11_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual void map(address_map &map) override;

protected:
	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect) override;

	template <unsigned Channel> u8 xpc1_r(offs_t offset)
	{
		if (offset)
		{
			unsigned const address = m_xpc1_addr[Channel];
			u16 const mode = m_xpc1_mode[Channel][address >> 1];

			return BIT(address, 0) ? (mode >> 8) : u8(mode);
		}
		else
			return m_xpc1_addr[Channel];
	}

	template <unsigned Channel> void xpc1_w(offs_t offset, u8 data)
	{
		if (offset)
		{
			unsigned const address = m_xpc1_addr[Channel];
			u16 &mode = m_xpc1_mode[Channel][address >> 1];

			if (BIT(address, 0))
				mode = (u16(data & 0x3f) << 8) | (mode & 0x00ff);
			else
				mode = (mode & 0x3f00) | data;
		}
		else
			m_xpc1_addr[Channel] = data & 7;
	}
	void xpc1_bc_w(offs_t offset, u8 data)
	{
		xpc1_w<0>(offset, data);
		xpc1_w<1>(offset, data);
		xpc1_w<2>(offset, data);
		xpc1_w<3>(offset, data);
		xpc1_w<4>(offset, data);
	}

private:
	required_device<bt458_device> m_ramdac;

	u8 m_xpc1_addr[5];
	u16 m_xpc1_mode[5][4];
};

class sgi_gr12_device : public sgi_gr1_device
{
public:
	sgi_gr12_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual void map(address_map &map) override;

protected:
	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect) override;

	template <unsigned Channel> u8 xmap2_r(offs_t offset);
	template <unsigned Channel> void xmap2_w(offs_t offset, u8 data);
	void xmap2_bc_w(offs_t offset, u8 data)
	{
		xmap2_w<0>(offset, data);
		xmap2_w<1>(offset, data);
		xmap2_w<2>(offset, data);
		xmap2_w<3>(offset, data);
		xmap2_w<4>(offset, data);
	}

private:
	required_device_array<bt457_device, 3> m_ramdac;

	std::unique_ptr<u32[]> m_vram;

	struct xmap2
	{
		u16 addr;
		rgb_t color[8192];
		rgb_t overlay[16];
		u16 mode[16];
		bool wid_aux;
	}
	m_xmap2[5];
};

DECLARE_DEVICE_TYPE(SGI_GR11, sgi_gr11_device)
DECLARE_DEVICE_TYPE(SGI_GR12, sgi_gr12_device)

#endif // MAME_VIDEO_SGI_GR1_H
