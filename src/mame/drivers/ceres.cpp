// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Eidgenössische Technische Hochschule (ETH) Zürich Ceres-1
 *
 * Sources:
 *   - http://bitsavers.org/pdf/eth/ceres/Eberle_Hardware_Description_of_the_Workstation_Ceres_198701.pdf
 *
 * TODO:
 *   - startup/reset memory map
 *   - i/o device mapping
 *   - interrupt acknowledge
 *   - WD1002-05 Winchester/Floppy Disk Controller (WFC)
 *   - keyboard
 *   - mouse
 */

#include "emu.h"

// cpus and memory
#include "cpu/ns32000/ns32000.h"
#include "machine/ram.h"

// various hardware
//#include "machine/ns32081.h"
//#include "machine/ns32082.h"
#include "machine/mc68681.h"
#include "machine/z80scc.h"
#include "machine/m3002.h"
#include "machine/am9519.h"
#include "machine/wd_fdc.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"

// video
#include "screen.h"

#define VERBOSE 0
#include "logmacro.h"

class ceres1_state : public driver_device
{
public:
	ceres1_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_uart(*this, "uart")
		, m_serial(*this, "v24")
		, m_scc(*this, "scc")
		, m_rtc(*this, "rtc")
		, m_icu(*this, "icu")
		, m_fdc(*this, "fdc")
		, m_fdd(*this, "fdc:0:35dd")
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map);

public:
	// machine config
	void ceres1(machine_config &config);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	void wfc_w(offs_t offset, u8 data);
	u8 wfc_r(offs_t offset);

protected:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	required_device<ns32032_device> m_cpu;
	required_device<ram_device> m_ram;

	required_device<scn2681_device> m_uart;
	required_device<rs232_port_device> m_serial;
	required_device<z80scc_device> m_scc;
	required_device<m3002_device> m_rtc;
	required_device<am9519_device> m_icu;

	required_device<wd2797_device> m_fdc;
	required_device<floppy_image_device> m_fdd;

	required_device<screen_device> m_screen;
	required_shared_ptr<u32> m_vram;

	u8 m_dcr;
};

void ceres1_state::machine_start()
{
	save_item(NAME(m_dcr));
}

void ceres1_state::machine_reset()
{
	m_dcr = 0;
}

void ceres1_state::wfc_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0: LOG("wfc_w data 0x%02x\n", offset, data); break;
	case 1: LOG("wfc_w write precomp 0x%02x\n", offset, data); break;
	case 2: LOG("wfc_w sector count 0x%02x\n", offset, data); break;
	case 3: LOG("wfc_w sector number 0x%02x\n", offset, data); break;
	case 4: LOG("wfc_w cylinder low 0x%02x\n", offset, data); break;
	case 5: LOG("wfc_w cylinder high 0x%02x\n", offset, data); break;
	case 6: LOG("wfc_w size/drive/head 0x%02x\n", offset, data); break;
	case 7: LOG("wfc_w command 0x%02x\n", offset, data); break;
	}
}

u8 ceres1_state::wfc_r(offs_t offset)
{
	u8 data = 0;

	switch (offset)
	{
	case 0: // data
	case 1: // error
	case 2: // sector count
	case 3: // sector number
	case 4: // cylinder low
	case 5: // cylinder high
	case 6: // size/drive/head
		break;

	case 7: // status
		//data = 0x40; // drive ready
		break;
	}

	LOG("wfc_r reg %d data 0x%02x\n", offset, data);

	return data;
}

template <unsigned ST> void ceres1_state::cpu_map(address_map &map)
{
	// FIXME: address lines 19-23 driven high until boot flipflop reset
	map(0x000000, 0x007fff).rom().region("eprom", 0);

	map(0xe00000, 0xe3ffff).ram().share("vram");
	map(0xf80000, 0xf87fff).rom().region("eprom", 0);
	map(0xfffa00, 0xfffa00).lw8(
		[this](u8 data)
		{
			m_dcr = data & 7;
		}, "dcr_w");
	map(0xfffdc0, 0xfffdc3).portr("SW1");
	map(0xfffdc0, 0xfffdc0).lw8(
		[this](u8 data)
		{
			m_cpu->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());
		}, "bt_ff_w");

	map(0xfffc00, 0xfffc1f).rw(FUNC(ceres1_state::wfc_r), FUNC(ceres1_state::wfc_w)).umask32(0xff);
}

static INPUT_PORTS_START(ceres1)
	PORT_START("SW1")
	PORT_DIPNAME(0x80, 0x00, "Diagnostic") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x80, DEF_STR(Off))

	PORT_DIPNAME(0x40, 0x40, "FPU") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x40, DEF_STR(Off))

	PORT_DIPNAME(0x20, 0x00, "MMU") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x20, DEF_STR(Off))

	PORT_DIPUNUSED_DIPLOC(0x10, 0x00, "SW1:4")

	// TODO: configuration is guesswork
	PORT_DIPNAME(0x0f, 0x08, "Memory Size") PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(0x01, "2M")
	PORT_DIPSETTING(0x03, "4M")
	PORT_DIPSETTING(0x07, "6M")
	PORT_DIPSETTING(0x0f, "8M")
INPUT_PORTS_END

void ceres1_state::ceres1(machine_config &config)
{
	NS32032(config, m_cpu, 10_MHz_XTAL);
	m_cpu->set_addrmap(0, &ceres1_state::cpu_map<0>);
	m_cpu->set_addrmap(6, &ceres1_state::cpu_map<6>);

	RAM(config, m_ram);
	m_ram->set_default_size("2MiB");
	m_ram->set_default_value(0);
	m_ram->set_extra_options("4MiB,6MiB,8MiB");

	// TODO: port A txd/rxd connect to keyboard
	SCN2681(config, m_uart, 3.6864_MHz_XTAL);
	m_uart->irq_cb().set(m_icu, FUNC(am9519_device::ireq2_w));
	m_uart->outport_cb().set(
		[this](u8 data)
		{
			m_serial->write_dtr(BIT(data, 0));
			m_serial->write_rts(BIT(data, 1));
			m_icu->ireq0_w(BIT(data, 3));
			m_icu->ireq4_w(BIT(data, 4));
		});
	m_uart->b_tx_cb().set(m_serial, FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_serial, default_rs232_devices, nullptr);
	m_serial->rxd_handler().set(m_uart, FUNC(scn2681_device::rx_b_w));
	m_serial->cts_handler().set(m_uart, FUNC(scn2681_device::ip1_w));
	m_serial->dcd_handler().set(m_uart, FUNC(scn2681_device::ip0_w));
	m_serial->dsr_handler().set(m_uart, FUNC(scn2681_device::ip2_w));

	// TODO: RS-485 ports "na" and "nb"
	SCC8530N(config, m_scc, 6_MHz_XTAL);
	m_scc->configure_channels(m_uart->clock(), 0, m_uart->clock(), 0);
	m_scc->out_int_callback().set(m_icu, FUNC(am9519_device::ireq1_w));

	M3002(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq_out().set(m_icu, FUNC(am9519_device::ireq5_w));

	AM9519(config, m_icu);
	m_icu->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	// TODO: WD1002-05 Winchester/Floppy Disk Controller (WFC)
	WD2797(config, m_fdc, 0);
	FLOPPY_CONNECTOR(config, "fdc:0", "35dd", FLOPPY_35_DD, true, floppy_formats).enable_sound(false);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(70'000'000, 1344, 0, 1024, 838, 0, 800);
	m_screen->set_screen_update(FUNC(ceres1_state::screen_update));
}

u32 ceres1_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	if (!BIT(m_dcr, 0))
	{
		offs_t offset = BIT(m_dcr, 1) ? 0x20000 : 0;
		u32 const invert = BIT(m_dcr, 2) ? 0xffffffffU : 0;

		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		{
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 32)
			{
				u32 const data = m_vram[offset++] ^ invert;

				for (unsigned i = 0; i < 32; i++)
					bitmap.pix(y, x + i) = BIT(data, 31 - i) ? rgb_t::white() : rgb_t::black();
			}
		}
	}

	return 0;
}

FLOPPY_FORMATS_MEMBER(ceres1_state::floppy_formats)
	{}
FLOPPY_FORMATS_END

ROM_START(ceres1)
	ROM_REGION32_LE(0x8000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ceres1", "30.6.89")
	ROMX_LOAD("by0_u44__30_6_89.u44", 0x0000, 0x2000, CRC(8e1559fe) SHA1(2e11c144fd698b9576816f5cfb003c46ab9ce43b), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("by1_u43__30_6_89.u43", 0x0001, 0x2000, CRC(18625b0c) SHA1(3319043507b7eeb488ecd839708032f968e6e53e), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("by2_u42__30_6_89.u42", 0x0002, 0x2000, CRC(a56497c5) SHA1(85858bea9fd27031ee0191b1d508c7469a384f10), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("by3_u41__30_6_89.u41", 0x0003, 0x2000, CRC(6b628912) SHA1(f76ea089e8bf82f572f27def344b5174e577ebbb), ROM_BIOS(0) | ROM_SKIP(3))
ROM_END

/*   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                                        FULLNAME   FLAGS */
COMP(1986, ceres1, 0,      0,      ceres1,  ceres1, ceres1_state, empty_init, "Eidgenössische Technische Hochschule Zürich", "Ceres-1", MACHINE_NOT_WORKING)
