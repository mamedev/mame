// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for MC88100-based NCD X terminals.

****************************************************************************/
/*
 * WIP notes
 *
 * There are two basic machines, NCD88k and Modular Color X (MCX). They share
 * the same firmware and have very similar hardware, however the system memory
 * maps differ. Marketing material for the MCX systems describes 3D and audio
 * hardware as enhancements over the prior NCD88k, and the latter system board
 * includes an additional custom ASIC. Various models of terminal differ only
 * by the type of monitor supplied.
 *
 * 88k "base" (19c, 19g, 17cr, 17g, 19cp)
 *  - no on-board memory
 *  - J6 and J7 are code memory SIMM sockets (up to 64M)
 *  - J8, J9 and J10 are data memory SIMM sockets (up to 96M)
 *  - 19c shipped with 2M code memory (1991)
 *  - 17g 1280x1024 gray-scale (1992)
 *  - 17cr 1280x1024 color (1992)
 *
 * MCX
 *  - 2MB code and 4MB data memory on motherboard
 *  - J10 SIMM slot for code, J11/J12 for data
 *  - Code memory can be added as 256Kx32, 512Kx32, 1Mx32, 2Mx32, 4Mx32,
 *    or 8Mx32 SIMMs. Data memory can be added as 1Mx32, 2Mx32, 4Mx32,
 *    or 8Mx32 SIMMs.
 *  - 256Kx32 and 512Kx32 SIMMs are only supported as code memory
 *  - ports: ethernet (TP & AUI), monitor, aux, mouse, keyboard, audio in, audio out
 *  - 50kHz 16-bit sound
 *
 * Model    Type, Colors/Grayscales           Res.      Mem.  CPU/GCC     List US$
 * -------------------------------------------------------------------------------
 * MCX-L   base only, VGA color, 100dpi       1152x900  6.0   88100-20       2,295
 * MCX14   14" color, 103dpi/70Hz     640x480,1024x768  6.0   88100-20       3,295
 * MCX15   15" color, 100dpi/70Hz     640x480,1152x900  6.0   88100-20       3,495
 * MCX17   17" color, 87dpi/75Hz     1024x768,1152x900  6.0   88100-20       4,295
 * MCX-L19 19" color, 84dpi/72Hz              1152x900  6.0   88100-20       4,695
 *
 * Sources:
 *  - https://web-docs.gsi.de/~kraemer/COLLECTION/ftp.ncd.com/pub/ncd/Archive/NCD-Articles/NCD_X_Terminals/Memory_specs/NCD_88k_family_memory_specs
 *  - http://books.google.com/books?id=ujsEAAAAMBAJ&lpg=PA24&pg=PA24#v=onepage&q&f=false
 *
 * TODO:
 *  - 19c nvram panic
 *  - nvram defaults
 *  - mcx ramdac issues (red background at reset)
 *  - video clocks and timing
 *  - mouse
 *  - audio
 */

#include "emu.h"

#include "ncd88k_ram.h"

#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/rs232/rs232.h"
#include "cpu/m88000/m88000.h"
#include "machine/am79c90.h"
#include "machine/at_ssrt.h"
#include "machine/eepromser.h"
#include "machine/icd2061a.h"
#include "machine/mc68681.h"
#include "video/bt45x.h"
#include "video/bt47x.h"

#include "screen.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class ncd88k_base : public driver_device
{
public:
	ncd88k_base(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_eeprom(*this, "eeprom")
		, m_lance(*this, "lance")
		, m_kbds(*this, "kbds")
		, m_kbdc(*this, "kbdc")
		, m_duart(*this, "duart")
		, m_serial(*this, "serial%u", 0U)
		, m_screen(*this, "screen")
		, m_gclk(*this, "gclk")
		, m_vram(*this, "vram")
	{
	}

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void common(machine_config &config, u32 clock);

	virtual void code_map(address_map &map) ATTR_COLD;
	virtual void data_map(address_map &map) ATTR_COLD;

	template <unsigned N> void irq_w(int state);

	required_device<cpu_device> m_cpu;
	required_device<ncd88k_ram_device> m_ram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	required_device<am7990_device> m_lance;
	required_device<at_ssrt_device> m_kbds;
	required_device<pc_kbdc_device> m_kbdc;
	required_device<scn2681_device> m_duart;
	required_device_array<rs232_port_device, 2> m_serial;

	required_device<screen_device> m_screen;
	required_device<icd2061a_device> m_gclk;

	required_shared_ptr<u32> m_vram;

	u8 m_int;
	u8 m_msk;
	u8 m_kbd;
	u8 m_clk;

	u32 m_vreg[3];

private:
	void vreg1_w(u32 data);

	void timer_update();
	void timer_tick(s32 param);
	emu_timer *m_timer;

	u16 m_tmr_ctrl;
	u16 m_tmr_period;
};

class ncd88k_state : public ncd88k_base
{
public:
	ncd88k_state(machine_config const &mconfig, device_type type, char const *tag)
		: ncd88k_base(mconfig, type, tag)
		, m_ramdac(*this, "ramdac")
	{
	}

	void ncd19c(machine_config &config);

protected:
	virtual void data_map(address_map &map) override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	void kbd_irq();

	required_device<bt458_device> m_ramdac;
};

class ncdmcx_state : public ncd88k_base
{
public:
	ncdmcx_state(machine_config const &mconfig, device_type type, char const *tag)
		: ncd88k_base(mconfig, type, tag)
		, m_ramdac(*this, "ramdac")
		, m_monitor(*this, "MONITOR")
		, m_ramdac_view(*this, "ramdac_view")
	{
	}

	void ncdmcx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void data_map(address_map &map) override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	void kbd_irq();
	void kbd_ctrl_w(u8 data);
	void vreg0_w(u32 data);
	void vreg2_w(u32 data);

	required_device<bt477_device> m_ramdac;
	required_ioport m_monitor;
	memory_view m_ramdac_view;

	u8 m_msk2;
	u8 m_ack;
	u8 m_vctrl;
	u8 m_mid;
};

void ncd88k_base::machine_start()
{
	save_item(NAME(m_int));
	save_item(NAME(m_msk));
	save_item(NAME(m_kbd));
	save_item(NAME(m_clk));

	save_item(NAME(m_tmr_ctrl));
	save_item(NAME(m_tmr_period));

	save_item(NAME(m_vreg));

	m_timer = timer_alloc(FUNC(ncd88k_base::timer_tick), this);
}

void ncdmcx_state::machine_start()
{
	ncd88k_base::machine_start();

	save_item(NAME(m_msk2));
	save_item(NAME(m_ack));
	save_item(NAME(m_vctrl));
	save_item(NAME(m_mid));
}

void ncd88k_base::machine_reset()
{
	m_int = 0;
	m_msk = 0;
	m_kbd = 0;
	m_clk = 0;

	m_tmr_ctrl = 0;
	m_tmr_period = 0;
	m_timer->reset();

	std::ranges::fill(m_vreg, 0);
}

void ncdmcx_state::machine_reset()
{
	ncd88k_base::machine_reset();

	m_msk2 = 0;
	m_ack = 0;
	m_vctrl = 0;
	m_mid = 0;

	m_duart->ip2_w(0);
	m_ramdac_view.select(0);
}

void ncd88k_base::common(machine_config &config, u32 clock)
{
	MC88100(config, m_cpu, clock);
	m_cpu->set_addrmap(AS_PROGRAM, &ncd88k_base::code_map);
	m_cpu->set_addrmap(AS_DATA, &ncd88k_base::data_map);

	NCD88K_RAM(config, m_ram);
	m_ram->set_code_space(m_cpu, AS_PROGRAM);
	m_ram->set_data_space(m_cpu, AS_DATA);

	EEPROM_93C66_16BIT(config, m_eeprom); // CAT35C104P
	m_eeprom->do_callback().set(m_duart, FUNC(scn2681_device::ip2_w));

	AM7990(config, m_lance, 20_MHz_XTAL / 2); // 4100004
	m_lance->intr_out().set(FUNC(ncd88k_state::irq_w<2>)).invert();
	m_lance->dma_in().set([this](offs_t offset) { return m_cpu->space(AS_DATA).read_word(ncd88k_ram_device::DATA_BASE + offset); });
	m_lance->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { m_cpu->space(AS_DATA).write_word(ncd88k_ram_device::DATA_BASE + offset, data, mem_mask); });

	SCN2681(config, m_duart, 3'686'400);
	m_duart->irq_cb().set(FUNC(ncd88k_base::irq_w<3>));
	m_duart->outport_cb().set(
		[this](u8 data)
		{
			m_eeprom->cs_write(BIT(data, 5));
			m_eeprom->di_write(BIT(data, 4));
			m_eeprom->clk_write(BIT(data, 6));
		});

	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr); // mouse?
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr); // aux?

	m_duart->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_serial[0]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	m_serial[1]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));

	// ICD2061ASC-1
	ICD2061A(config, m_gclk, 14'318'180);
	m_gclk->mclkout_changed().set(
		[this](u32 data)
		{
			LOG("mclk %d\n", data);
		});
	m_gclk->vclkout_changed().set(
		[this](u32 data)
		{
			LOG("vclk %d\n", data);
		});

	SCREEN(config, m_screen);
	m_screen->set_raw(125'000'000, 1680, 0, 1280, 1063, 0, 1024); // 74.4 kHz horizontal, 70 Hz vertical
	m_screen->screen_vblank().set(FUNC(ncd88k_base::irq_w<4>));

	AT_SSRT(config, m_kbds);
	m_kbds->clk().set(m_kbdc, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_kbds->txd().set(m_kbdc, FUNC(pc_kbdc_device::data_write_from_mb));

	PC_KBDC(config, m_kbdc, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_kbdc->out_clock_cb().set(m_kbds, FUNC(at_ssrt_device::clk_w));
	m_kbdc->out_data_cb().set(m_kbds, FUNC(at_ssrt_device::rxd_w));
}

void ncd88k_state::ncd19c(machine_config &config)
{
	common(config, 15'000'000);

	m_ram->slot<0>().set_ioport("J8");
	m_ram->slot<1>().set_ioport("J9");
	m_ram->slot<2>().set_ioport("J10");
	m_ram->slot<3>().set_ioport("J6");
	m_ram->slot<4>().set_ioport("J7");

	m_screen->set_screen_update(FUNC(ncd88k_state::screen_update));

	BT458(config, m_ramdac, 125'000'000);

	m_kbds->rx().set(
		[this](int state)
		{
			if (state)
				m_kbd |= 0x10;
			else
				m_kbd &= ~0x10;

			kbd_irq();
		});
	m_kbds->tx().set(
		[this](int state)
		{
			if (state)
				m_kbd |= 0x20;
			else
				m_kbd &= ~0x20;

			kbd_irq();
		});
}

void ncdmcx_state::ncdmcx(machine_config &config)
{
	common(config, (80_MHz_XTAL / 4).value());

	m_ram->slot<0>().set_ioport("J11");
	m_ram->slot<1>().set_ioport("J12");
	m_ram->slot<2>().set_constant(0x40'0000); // onboard data memory: KM44C1000CLJ-7 x8 (1024kx4 x8)
	m_ram->slot<3>().set_ioport("J10");
	m_ram->slot<4>().set_constant(0x20'0000); // onboard code memory: KM416C256AJ-7 x4 (256kx16 x4)

	m_duart->outport_cb().append(
		[this](u8 data)
		{
			// used for machine identification?
			m_duart->ip6_w(BIT(data, 0));
			m_duart->ip3_w(BIT(data, 1));
		});

	m_screen->set_screen_update(FUNC(ncdmcx_state::screen_update));

	BT477(config, m_ramdac, 125'000'000); // ATT20C497-11

	m_kbds->rx().set(
		[this](int state)
		{
			if (state)
			{
				m_kbd |= 0x40;

				kbd_irq();
			}
		});
	m_kbds->tx().set(
		[this](int state)
		{
			if (state)
			{
				m_kbd |= 0x80;

				kbd_irq();
			}
		});
}

void ncd88k_base::code_map(address_map &map)
{
	map(0x0000'0000, 0x0003'ffff).rom().region("prom", 0);

	//map(0x0400'0000, 0x07ff'ffff); // code ram: dynamically mapped
}

void ncd88k_base::data_map(address_map &map)
{
	map(0x0000'0000, 0x0003'ffff).rom().region("prom", 0);

	map(0x00c0'0000, 0x00c0'0003).rw(m_lance, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w)).mirror(0x000fff04);
	map(0x0100'0000, 0x0100'003f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0xff00'0000);

	map(0x0158'0001, 0x0158'0001).lw8(
		[this](u8 data)
		{
			if (BIT(m_clk ^ data, 1))
				m_gclk->data_w(BIT(data, 1));

			if (BIT(m_clk ^ data, 0))
				m_gclk->clk_w(BIT(data, 0));

			m_clk = data;
		}, "gclk_w");

	map(0x0180'0000, 0x0180'0003).lw32([this](u32 data) { LOG("kbd en 0x%x\n", data); }, "kbd_enable"); // kbd enable?

	//map(0x01cc'0000, 0x01cc'0003).w(FUNC(ncd88k_base::vreg0_w));
	map(0x01d0'0000, 0x01d0'0003).w(FUNC(ncd88k_base::vreg1_w));
	//map(0x01d4'0000, 0x01d4'0003).w(FUNC(ncd88k_base::vreg2_w));

	map(0x01d8'0000, 0x01d8'0000).lrw8(
		[this]()
		{
			return m_int;
		}, "int_r",
		[this](u8 data)
		{
			LOG("%s: msk_w 0x%02x\n", machine().describe_context(), data);

			m_msk = data;

			m_cpu->set_input_line(INPUT_LINE_IRQ0, bool(m_int & m_msk));
		}, "msk_w");
	map(0x01d8'0001, 0x01d8'0001).lrw8(
		[this]()
		{
			LOG("%s: kbd_r 0x%02x\n", machine().describe_context(), m_kbd ^ 0x10);

			return m_kbd ^ 0x10;
		}, "kbd_r",
		[this](u8 data)
		{
			// soft interrupt set/clear
			if (data & 1)
				m_int |= 0x01;
			else
				m_int &= ~0x01;

			// timer interrupt clear
			if (data & 2)
				m_int &= ~0x40;

			// vsync interrupt clear
			if (data & 4)
				m_int &= ~0x10;

			m_cpu->set_input_line(INPUT_LINE_IRQ0, bool(m_int & m_msk));
		}, "ctl_w");

	// different keyboard controllers based on byte at 0x800'0188 (0=NCD88K, 1=MCX)
	// cmd1 r status  1d8'0001  148'0001                             read status
	// cmd2 w ctrl    1d8'0002  148'0001  (store 0x01, store 0x80)   ack tx
	// cmd3 w         1e0'0000  148'0001  (store 0x2001, store 0x40) ack rx
	// cmd4           1d8'0003  144'0001  (store r11^1, store r11)   send byte
	// cmd5           1d8'0003  144'0001                             read byte
	// cmd6                               return r11&0x20, return r11&0x80  tx done?
	// cmd7                               test r11&0x10, test r11&0x40      rx done?
	//
	// machine type at 0x0800'0188 and 0x0800'0189: set sat 0x120f8
	// r19 & r20 evaluated starting at 0x1a00 ramdac_test()
	//
	//

	map(0x01dc'0000, 0x01dc'0001).lw16(
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			m_tmr_ctrl = data;

			timer_update();
		}, "tmr_ctrl_w");
	map(0x01dc'0002, 0x01dc'0003).lw16(
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			m_tmr_period = data;

			timer_update();
		}, "tmr_w");
	map(0x01e0'0002, 0x01e0'0003).lw16(
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			timer_update();
		}, "tmr_start_w");

	map(0x0200'0000, 0x03ff'ffff).w(m_ram, FUNC(ncd88k_ram_device::ctrl_w));

	//map(0x0400'0000, 0x07ff'ffff); // code ram: dynamically mapped
	//map(0x0800'0000, 0x0dff'ffff); // data ram: dynamically mapped
}

void ncd88k_state::data_map(address_map &map)
{
	ncd88k_base::data_map(map);

	map(0x0140'0000, 0x0140'001f).m(m_ramdac, FUNC(bt458_device::map)).umask32(0xff00'0000);

	map(0x01d8'0002, 0x01d8'0002).lw8(
		[this](u8 data)
		{
			if (data == 0x01)
			{
				m_kbd &= ~0x20;

				kbd_irq();
			}
		}, "kbd_tx_ack");
	map(0x01d8'0003, 0x01d8'0003).r(m_kbds, FUNC(at_ssrt_device::data_r));
	map(0x01d8'0003, 0x01d8'0003).lw8([this](u8 data) { m_kbds->data_w(data ^ 1); }, "kbd_data_w");

	map(0x01e0'0000, 0x01e0'0001).lw16(
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			if (data == 0x2001)
			{
				m_kbd &= ~0x10;

				kbd_irq();
			}
		}, "kbd_rx_ack");

	map(0x0e00'0000, 0x0e1f'ffff).ram().share("vram");
}

void ncdmcx_state::data_map(address_map &map)
{
	ncd88k_base::data_map(map);

	map(0x0140'0000, 0x0140'001f).view(m_ramdac_view);
	m_ramdac_view[0](0x0140'0000, 0x0140'001f).rw(m_ramdac, FUNC(bt477_device::read), FUNC(bt477_device::write)).umask32(0x0000'00ff);

	map(0x0144'0000, 0x0144'0000).lrw8(
		[this]()
		{
			return BIT(m_mid, 0, 3) << 4;
		}, "mid_210_r",
		[this](u8 data)
		{
			LOG("vctrl_w 0x%02x (%s)\n", data, machine().describe_context());

			if (BIT(data, 7))
				m_ramdac_view.disable();
			else
				m_ramdac_view.select(0);

			m_vctrl = data;
		}, "vctrl_w");

	map(0x0144'0001, 0x0144'0001).rw(m_kbds, FUNC(at_ssrt_device::data_r), FUNC(at_ssrt_device::data_w));

	map(0x0148'0000, 0x0148'0000).lr8([this]() { return (m_kbd & 0xc0) ? 0x01 : 0x00; }, "kbd_int_r");

	map(0x0148'0001, 0x0148'0001).lr8([this]() { return m_kbd; }, "kbd_stat_r");
	map(0x0148'0001, 0x0148'0001).w(FUNC(ncdmcx_state::kbd_ctrl_w));

	// second interrupt mask: gates the sources reported in the summary
	// byte at 0x01480000 (bit 0 = keyboard) onto interrupt 7
	map(0x014c'0000, 0x014c'0000).lrw8(
		[this]()
		{
			return m_msk2;
		}, "msk2_r",
		[this](u8 data)
		{
			m_msk2 = data;

			kbd_irq();
		}, "msk2_w");

	// 0x0168'0000: write 0x0003, wait vsync interrupt?

	map(0x01cc'0000, 0x01cc'0003).w(FUNC(ncdmcx_state::vreg0_w));
	map(0x01d4'0000, 0x01d4'0003).w(FUNC(ncdmcx_state::vreg2_w));

	map(0x01d8'0001, 0x01d8'0001).lrw8(
		[this]()
		{
			return m_ack;
		}, "ack_r",
		[this](u8 data)
		{
			// soft interrupt set/clear
			if (data & 1)
				m_int |= 0x01;
			else
				m_int &= ~0x01;

			// timer interrupt clear
			if (data & 2)
				m_int &= ~0x40;

			// vsync interrupt clear
			if (data & 4)
				m_int &= ~0x10;

			m_cpu->set_input_line(INPUT_LINE_IRQ0, bool(m_int & m_msk));

			m_ack = data;
		}, "ctl_w");
	map(0x01d8'0002, 0x01d8'0002).lr8([this]() { return BIT(m_mid, 3) << 6; }, "mid_3_r");

	map(0x0e00'0000, 0x0e3f'ffff).ram().share("vram"); // M5M482256J x4 (256kx8 x4)
}

u32 ncd88k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u32 const *pixel_pointer = m_vram;

	for (int y = screen.visible_area().top(); y <= screen.visible_area().bottom(); y++)
	{
		for (int x = screen.visible_area().left(); x <= screen.visible_area().right(); x += 4)
		{
			u32 const pixel_data = *pixel_pointer++;

			bitmap.pix(y, x + 0) = m_ramdac->pen_color(BIT(pixel_data, 24, 8));
			bitmap.pix(y, x + 1) = m_ramdac->pen_color(BIT(pixel_data, 16, 8));
			bitmap.pix(y, x + 2) = m_ramdac->pen_color(BIT(pixel_data, 8, 8));
			bitmap.pix(y, x + 3) = m_ramdac->pen_color(BIT(pixel_data, 0, 8));
		}

		// compensate by 2048 - 1280 pixels per line
		pixel_pointer += 0xc0;
	}

	return 0;
}

u32 ncdmcx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	u32 const *pixel_pointer = m_vram;

	for (int y = screen.visible_area().top(); y <= screen.visible_area().bottom(); y++)
	{
		for (int x = screen.visible_area().left(); x <= screen.visible_area().right(); x += 4)
		{
			u32 const pixel_data = *pixel_pointer++;

			bitmap.pix(y, x + 0) = m_ramdac->pen_color(BIT(pixel_data, 24, 8));
			bitmap.pix(y, x + 1) = m_ramdac->pen_color(BIT(pixel_data, 16, 8));
			bitmap.pix(y, x + 2) = m_ramdac->pen_color(BIT(pixel_data, 8, 8));
			bitmap.pix(y, x + 3) = m_ramdac->pen_color(BIT(pixel_data, 0, 8));
		}
	}

	return 0;
}

void ncd88k_base::timer_update()
{
	if ((m_tmr_ctrl & 0x0100) && m_tmr_period)
	{
		attotime const period = attotime::from_ticks(m_tmr_period, 20_MHz_XTAL / 64);
		m_timer->adjust(period, 0, period);
	}
	else
		m_timer->reset();
}

void ncd88k_base::timer_tick(s32 param)
{
	irq_w<6>(1);
}

void ncdmcx_state::vreg0_w(u32 data)
{
	LOG("%s: vreg0_w 0x%08x\n", machine().describe_context(), data);
}

void ncd88k_base::vreg1_w(u32 data)
{
	LOG("%s: vreg1_w 0x%08x\n", machine().describe_context(), data);

	unsigned const width = (BIT(data, 24, 8) + 1) * 8;
	unsigned const height = BIT(data, 4, 12) + 1;
	// TODO: low 4 bits: 3==640x480, 4==800x600, 5==1024x768, 7=1152x900

	if (data ^ m_vreg[1])
	{
		LOG("video mode %ux%u\n", width, height);
		// TODO: figure out video timing parameters from ICD and other registers
		m_screen->configure(width + 400, height + 40, rectangle(0, width - 1, 0, height - 1), attotime::from_hz(60));

		m_vreg[1] = data;
	}
}

void ncdmcx_state::vreg2_w(u32 data)
{
	/*
	 * The value read from the video connector ID0..ID3 pins depends upon the
	 * monitor itself and the upper two bits of this register. Monitor ID bits
	 * are encoded in ioport_value using 2 bits each, and when read back with
	 * different values in the upper 2 bits of this register, are decoded by
	 * the firmware as follows:
	 *
	 *  IDn  Readback    Decoding
	 *   0   constant 0  '0'
	 *   1   constant 1  '1'
	 *   2   == bit 30   'H' (hsync?)
	 *   3   == bit 31   'V' (vsync?)
	 *
	 * The decoded characters are appended into a 32 bit integer and compared
	 * against various monitor types by the firmware. Tested combinations and
	 * the corresponding display modes presented are:
	 *
	 *   ID   Default        Others
	 *  1H0H  1024x768@70Hz  640x480@60/72/75Hz, 800x600@60/72/75Hz, 1024x768@60/75NCD/75Hz, 1152x900@66/72Hz
	 *  1H00  1024x768@70Hz  1024x768@75NCD/75Hz, 1152x900@66/72Hz
	 *  110H  1024x768 70Hz  640x480@60/72/75Hz, 800x600@60/72/75Hz, 1024x768@60/75NCD/75Hz, 1152x900@66/72Hz
	 *  1H01  1024x746 75Hz  1024x768@75NCD, 1152x900@66/72Hz
	 *  1H1H  1152x900 66Hz  1152x900@66Hz Greyscale
	 *  1HH1  1152x900 76Hz  1152x900@76Hz Greyscale
	 *  1H10  1152x900 66Hz  1152x900@66Hz Greyscale
	 *  1H11  1152x900 66Hz  1152x900@66Hz Greyscale
	 */

	LOG("%s: vreg2_w 0x%08x\n", machine().describe_context(), data);

	// refresh monitor identification bits
	u32 const monitor = m_monitor->read();
	m_mid = 0;

	for (unsigned bit = 0; bit < 4; bit++)
	{
		switch (BIT(monitor, bit * 2, 2))
		{
		case 0:
			// constant 0
			break;
		case 1:
			// constant 1
			m_mid |= 1U << bit;
			break;
		case 2:
			// == bit 30 (hsync?)
			if (BIT(data, 30))
				m_mid |= 1U << bit;
			break;
		case 3:
			// == bit 31 (vsync?)
			if (BIT(data, 31))
				m_mid |= 1U << bit;
			break;
		}
	}

	m_vreg[2] = data;
}

template <unsigned N> void ncd88k_base::irq_w(int state)
{
	if (state)
		m_int |= 1U << N;
	else
		m_int &= ~(1U << N);

	m_cpu->set_input_line(INPUT_LINE_IRQ0, bool(m_int & m_msk));
}

void ncd88k_state::kbd_irq()
{
	irq_w<1>(m_kbd ? 1 : 0);
}

void ncdmcx_state::kbd_irq()
{
	// the keyboard summary is gated by bit 0 of the second interrupt mask
	irq_w<7>((m_kbd & 0xc0) && (m_msk2 & 0x01));
}

void ncdmcx_state::kbd_ctrl_w(u8 data)
{
	LOG("kbd_ctrl_w 0x%02x (%s)\n", data, machine().describe_context());

	m_kbd &= ~(data & 0xc0);

	kbd_irq();
}

ROM_START(ncd19c)
	ROM_REGION32_BE(0x40000, "prom", 0)
	// These dumps have very strange lengths. The actual ROMs should be standard EEPROM types.
	//ROM_LOAD16_BYTE("ncd19c-e.rom", 0x0000, 0xb000, CRC(01e31b42) SHA1(28da6e4465415d00a739742ded7937a144129aad) BAD_DUMP)
	//ROM_LOAD16_BYTE("ncd19c-o.rom", 0x0001, 0xb000, CRC(dfd9be7c) SHA1(2e99a325b039f8c3bb89833cd1940e6737b64d79) BAD_DUMP)

	ROM_SYSTEM_BIOS(0, "v2.7.3", "v2.7.3")
	ROMX_LOAD("ncd88k_mcx_bm__v2.7.3_b0e.u3",  0x0000, 0x20000, CRC(70305680) SHA1(b10b250fe319e823cff28ba7b449b0a40755f5a2), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("ncd88k_mcx_bm__v2.7.3_b0o.u14", 0x0001, 0x20000, CRC(fc066464) SHA1(fa894de56b77bd4bc619040a2cf3a0d260914727), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(1, "v2.6.0", "v2.6.0")
	ROMX_LOAD("ncd88k_mcx_bm__v2.6.0_b0e.u3",  0x0000, 0x20000, CRC(99644196) SHA1(d5091fd4f096000de4970ae778112ff3c01ac340), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("ncd88k_mcx_bm__v2.6.0_b0o.u14", 0x0001, 0x20000, CRC(db2ed336) SHA1(8be4e08bf097d2b85be84da62b2a24c6e55661d9), ROM_BIOS(1) | ROM_SKIP(1))
ROM_END

ROM_START(ncdmcx)
	ROM_REGION32_BE(0x40000, "prom", 0)
	ROM_SYSTEM_BIOS(0, "v2.7.3", "v2.7.3")
	ROMX_LOAD("ncd88k_mcx_bm__v2.7.3_b0e.u3",  0x0000, 0x20000, CRC(70305680) SHA1(b10b250fe319e823cff28ba7b449b0a40755f5a2), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("ncd88k_mcx_bm__v2.7.3_b0o.u14", 0x0001, 0x20000, CRC(fc066464) SHA1(fa894de56b77bd4bc619040a2cf3a0d260914727), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(1, "v2.6.0", "v2.6.0")
	ROMX_LOAD("ncd88k_mcx_bm__v2.6.0_b0e.u3",  0x0000, 0x20000, CRC(99644196) SHA1(d5091fd4f096000de4970ae778112ff3c01ac340), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("ncd88k_mcx_bm__v2.6.0_b0o.u14", 0x0001, 0x20000, CRC(db2ed336) SHA1(8be4e08bf097d2b85be84da62b2a24c6e55661d9), ROM_BIOS(1) | ROM_SKIP(1))
ROM_END

static INPUT_PORTS_START(ncd19c)
	PORT_START("J6")
	PORT_CONFNAME(0x03f0'0000, 0x0020'0000, "Code SIMM (J6)")
	PORT_CONFSETTING(0x0000'0000, "Empty")
	PORT_CONFSETTING(0x0010'0000, "1MiB (256Kx32)")
	PORT_CONFSETTING(0x0020'0000, "2MiB (512Kx32)")
	PORT_CONFSETTING(0x0040'0000, "4MiB (1Mx32)")
	PORT_CONFSETTING(0x0080'0000, "8MiB (2Mx32)")
	PORT_CONFSETTING(0x0100'0000, "16MiB (4Mx32)")
	PORT_CONFSETTING(0x0200'0000, "32MiB (8Mx32)")

	PORT_START("J7")
	PORT_CONFNAME(0x03f0'0000, 0x0000'0000, "Code SIMM (J7)")
	PORT_CONFSETTING(0x0000'0000, "Empty")
	PORT_CONFSETTING(0x0010'0000, "1MiB (256Kx32)")
	PORT_CONFSETTING(0x0020'0000, "2MiB (512Kx32)")
	PORT_CONFSETTING(0x0040'0000, "4MiB (1Mx32)")
	PORT_CONFSETTING(0x0080'0000, "8MiB (2Mx32)")
	PORT_CONFSETTING(0x0100'0000, "16MiB (4Mx32)")
	PORT_CONFSETTING(0x0200'0000, "32MiB (8Mx32)")

	PORT_START("J8")
	PORT_CONFNAME(0x03f0'0000, 0x0040'0000, "Data SIMM (J8)")
	PORT_CONFSETTING(0x0000'0000, "Empty")
	PORT_CONFSETTING(0x0010'0000, "1MiB (256Kx32)")
	PORT_CONFSETTING(0x0020'0000, "2MiB (512Kx32)")
	PORT_CONFSETTING(0x0040'0000, "4MiB (1Mx32)")
	PORT_CONFSETTING(0x0080'0000, "8MiB (2Mx32)")
	PORT_CONFSETTING(0x0100'0000, "16MiB (4Mx32)")
	PORT_CONFSETTING(0x0200'0000, "32MiB (8Mx32)")

	PORT_START("J9")
	PORT_CONFNAME(0x03f0'0000, 0x0000'0000, "Data SIMM (J9)")
	PORT_CONFSETTING(0x0000'0000, "Empty")
	PORT_CONFSETTING(0x0010'0000, "1MiB (256Kx32)")
	PORT_CONFSETTING(0x0020'0000, "2MiB (512Kx32)")
	PORT_CONFSETTING(0x0040'0000, "4MiB (1Mx32)")
	PORT_CONFSETTING(0x0080'0000, "8MiB (2Mx32)")
	PORT_CONFSETTING(0x0100'0000, "16MiB (4Mx32)")
	PORT_CONFSETTING(0x0200'0000, "32MiB (8Mx32)")

	PORT_START("J10")
	PORT_CONFNAME(0x03f0'0000, 0x0000'0000, "Data SIMM (J10)")
	PORT_CONFSETTING(0x0000'0000, "Empty")
	PORT_CONFSETTING(0x0010'0000, "1MiB (256Kx32)")
	PORT_CONFSETTING(0x0020'0000, "2MiB (512Kx32)")
	PORT_CONFSETTING(0x0040'0000, "4MiB (1Mx32)")
	PORT_CONFSETTING(0x0080'0000, "8MiB (2Mx32)")
	PORT_CONFSETTING(0x0100'0000, "16MiB (4Mx32)")
	PORT_CONFSETTING(0x0200'0000, "32MiB (8Mx32)")
INPUT_PORTS_END

static INPUT_PORTS_START(ncdmcx)
	PORT_START("J10")
	PORT_CONFNAME(0x03f0'0000, 0x0000'0000, "Code SIMM (J10)")
	PORT_CONFSETTING(0x0000'0000, "Empty")
	PORT_CONFSETTING(0x0010'0000, "1MiB (256Kx32)")
	PORT_CONFSETTING(0x0020'0000, "2MiB (512Kx32)")
	PORT_CONFSETTING(0x0040'0000, "4MiB (1Mx32)")
	PORT_CONFSETTING(0x0080'0000, "8MiB (2Mx32)")
	PORT_CONFSETTING(0x0100'0000, "16MiB (4Mx32)")
	PORT_CONFSETTING(0x0200'0000, "32MiB (8Mx32)")

	PORT_START("J11")
	PORT_CONFNAME(0x03c0'0000, 0x0000'0000, "Data SIMM (J11)")
	PORT_CONFSETTING(0x0000'0000, "Empty")
	PORT_CONFSETTING(0x0040'0000, "4MiB (1Mx32)")
	PORT_CONFSETTING(0x0080'0000, "8MiB (2Mx32)")
	PORT_CONFSETTING(0x0100'0000, "16MiB (4Mx32)")
	PORT_CONFSETTING(0x0200'0000, "32MiB (8Mx32)")

	PORT_START("J12")
	PORT_CONFNAME(0x03c0'0000, 0x0000'0000, "Data SIMM (J12)")
	PORT_CONFSETTING(0x0000'0000, "Empty")
	PORT_CONFSETTING(0x0040'0000, "4MiB (1Mx32)")
	PORT_CONFSETTING(0x0080'0000, "8MiB (2Mx32)")
	PORT_CONFSETTING(0x0100'0000, "16MiB (4Mx32)")
	PORT_CONFSETTING(0x0200'0000, "32MiB (8Mx32)")

	// 0 == constant 0
	// 1 == constant 1
	// H == follow bit 30
	// V == follow bit 31
	//
	// 1H0H == 1024x768 70Hz
	//
	// 1H00 == 1024x768 70Hz
	// 110H == 1024x768 70Hz
	// 1H01 == 1024x768 75Hz
	//
	// 1H1H == 1152x900 66Hz
	// 1HH1 == 1152x900 76Hz
	// 1H10 == 1152x900 66Hz
	// 1H11 == 1152x900 66Hz (same as 1H10)

	// vga
	// id2  id1  id0
	// n/c  n/c  n/c  none
	// n/c  gnd  n/c  <1024x768 mono
	// n/c  n/c  gnd  <1024x768 color
	// gnd  n/c  gnd  >=1024x768 color

	// id0->color, id2->hires

	PORT_START("MONITOR")
	PORT_CONFNAME(0xff, 0x62, "Monitor Type")
	PORT_CONFSETTING(0b0101'0101, "None (1111)")

	PORT_CONFSETTING(0b0110'0010, "1024x768 70Hz (1H0H)") // 12 modes
	PORT_CONFSETTING(0b0110'0000, "1024x768 70Hz (1H00)")
	PORT_CONFSETTING(0b0101'0010, "1024x768 70Hz (110H)") // 12 modes
	PORT_CONFSETTING(0b0110'0001, "1024x768 75Hz (1H01)")

	PORT_CONFSETTING(0b0110'0110, "1152x900 66Hz (1H1H)")
	PORT_CONFSETTING(0b0110'1001, "1152x900 76Hz (1HH1)")
	PORT_CONFSETTING(0b0110'0100, "1152x900 66Hz (1H10)")
	PORT_CONFSETTING(0b0110'0101, "1152x900 66Hz (1H11)")
INPUT_PORTS_END

} // anonymous namespace

COMP(1991, ncd19c, 0, 0, ncd19c, ncd19c, ncd88k_state, empty_init, "Network Computing Devices", "19c", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
COMP(1993, ncdmcx, 0, 0, ncdmcx, ncdmcx, ncdmcx_state, empty_init, "Network Computing Devices", "MCX", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
