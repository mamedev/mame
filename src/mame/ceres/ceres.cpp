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
 *   - WD1002-05 Winchester/Floppy Disk Controller (WFC)
 *   - keyboard
 */

#include "emu.h"

// cpus and memory
#include "cpu/ns32000/ns32000.h"
#include "machine/ram.h"

// various hardware
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/mc68681.h"
#include "machine/z80scc.h"
#include "machine/m3002.h"
#include "machine/am9519.h"
#include "machine/wd_fdc.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "imagedev/harddriv.h"
#include "imagedev/floppy.h"

// video
#include "screen.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class ceres1_state : public driver_device
{
public:
	ceres1_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		, m_mmu(*this, "mmu")
		, m_ram(*this, "ram")
		, m_uart(*this, "uart")
		, m_serial(*this, "v24")
		, m_scc(*this, "scc")
		, m_rtc(*this, "rtc")
		, m_icu(*this, "icu")
		, m_fdc(*this, "fdc")
		, m_fdd(*this, "fdc:%u:fdd", 0U)
		, m_hdd(*this, "hdd%u", 0U)
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;

public:
	// machine config
	void ceres1(machine_config &config);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	void wfc_w(offs_t offset, u8 data);
	u8 wfc_r(offs_t offset);
	void wfc_command(u8 command);
	int get_lbasector(harddisk_image_device *hdf);

	DECLARE_INPUT_CHANGED_MEMBER(mouse_x);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_y);

protected:
	required_device<ns32032_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ram_device> m_ram;

	required_device<scn2681_device> m_uart;
	required_device<rs232_port_device> m_serial;
	required_device<z80scc_device> m_scc;
	required_device<m3002_device> m_rtc;
	required_device<am9519_device> m_icu;

	required_device<wd2797_device> m_fdc;
	optional_device_array<floppy_image_device, 4> m_fdd;
	required_device_array<harddisk_image_device, 3> m_hdd;

	required_device<screen_device> m_screen;
	required_shared_ptr<u32> m_vram;

	u8 m_dcr = 0;
	u16 m_mouse_x = 0;
	u16 m_mouse_y = 0;

	enum wfc_status : u8
	{
		WFC_S_ERR = 0x01, // error
		WFC_S_CD  = 0x04, // corrected data
		WFC_S_DRQ = 0x08, // data request
		WFC_S_SC  = 0x10, // seek complete
		WFC_S_WF  = 0x20, // write fault
		WFC_S_RDY = 0x40, // drive ready
		WFC_S_BSY = 0x80, // busy
	};
	u8 m_wfc_sram[2048]{};
	unsigned m_wfc_offset = 0;
	u8 m_wfc_error = 0;
	u8 m_wfc_precomp = 0;
	u8 m_wfc_count = 0;
	u8 m_wfc_sector = 0;
	u16 m_wfc_cylinder = 0;
	u8 m_wfc_sdh = 0;
	u8 m_wfc_status = 0;
};

void ceres1_state::machine_start()
{
	save_item(NAME(m_dcr));
}

void ceres1_state::machine_reset()
{
	m_dcr = 0;
	m_mouse_x = 0;
	m_mouse_y = 0;

	m_wfc_offset = 0;
	m_wfc_status = WFC_S_RDY;
}

void ceres1_state::wfc_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
		m_wfc_sram[m_wfc_offset++] = data;
		m_wfc_offset &= 0x3ff;
		break;
	case 1: m_wfc_precomp = data; break;
	case 2: m_wfc_count = data; break;
	case 3: m_wfc_sector = data; break;
	case 4: m_wfc_cylinder = (m_wfc_cylinder & 0xff00) | data; break;
	case 5: m_wfc_cylinder = u16(data) << 8 | (m_wfc_cylinder & 0xff); break;
	case 6: m_wfc_sdh = data; break;
	case 7: wfc_command(data); break;
	}
}

u8 ceres1_state::wfc_r(offs_t offset)
{
	u8 data = 0;

	switch (offset)
	{
	case 0: // data
		data = m_wfc_sram[m_wfc_offset++];
		m_wfc_offset &= 0x3ff;
		break;
	case 1: data = m_wfc_error; break;
	case 2: data = m_wfc_count; break;
	case 3: data = m_wfc_sector; break;
	case 4: data = u8(m_wfc_cylinder); break;
	case 5: data = m_wfc_cylinder >> 8; break;
	case 6: data = m_wfc_sdh; break;
	case 7: data = m_wfc_status; m_icu->ireq3_w(0); break;
	}

	return data;
}

void ceres1_state::wfc_command(u8 command)
{
	m_wfc_status |= WFC_S_BSY;
	m_wfc_status &= ~WFC_S_ERR;
	m_wfc_error = 0;

	if (((m_wfc_sdh >> 3) & 3) == 3)
		return;

	harddisk_image_device *hdf = m_hdd[(m_wfc_sdh >> 3) & 3];

	switch (command >> 4)
	{
	case 1:
		// restore
		break;
	case 2:
		LOG("read sector drive %d chs %d,%d,%d count %d\n",
			(m_wfc_sdh >> 3) & 3, m_wfc_cylinder & 0x3ff, (m_wfc_sdh >> 0) & 7, m_wfc_sector, m_wfc_count);
		if (hdf->exists())
			hdf->read(get_lbasector(hdf), m_wfc_sram);
		m_wfc_offset = 0;
		break;
	case 3:
		LOG("write sector drive %d chs %d,%d,%d count %d\n",
			(m_wfc_sdh >> 3) & 3, m_wfc_cylinder & 0x3ff, (m_wfc_sdh >> 0) & 7, m_wfc_sector, m_wfc_count);
		if (hdf->exists())
			hdf->write(get_lbasector(hdf), m_wfc_sram);
		m_wfc_offset = 0;
		break;
	case 4:
		// scan id
		break;
	case 5:
		// write format id
		break;
	case 7:
		// seek
		break;
	}

	m_wfc_status &= ~WFC_S_BSY;
	m_icu->ireq3_w(1);
}

int ceres1_state::get_lbasector(harddisk_image_device *hdf)
{
	const auto &info = hdf->get_info();

	int lbasector = m_wfc_cylinder & 0x3ff;
	lbasector *= info.heads;
	lbasector += (m_wfc_sdh >> 0) & 7;
	lbasector *= info.sectors;
	lbasector += m_wfc_sector;

	return lbasector;
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
	map(0xfffc00, 0xfffc1f).rw(FUNC(ceres1_state::wfc_r), FUNC(ceres1_state::wfc_w)).umask32(0xff);
	map(0xfffc40, 0xfffc7f).lrw32(
		[this]() { m_cpu->set_input_line(INPUT_LINE_NMI, 0); return 0; }, "parity_clear",
		[this](u32 data) { m_cpu->set_input_line(INPUT_LINE_NMI, 0); }, "parity_clear");
	map(0xfffc80, 0xfffc80).rw(m_rtc, FUNC(m3002_device::read), FUNC(m3002_device::write));
	map(0xfffd00, 0xfffd01).lr16([this]() { return m_mouse_x; }, "mouse_x");
	map(0xfffd04, 0xfffd05).lr16([this]() { return m_mouse_y; }, "mouse_y");
	map(0xfffd08, 0xfffd0b).portr("mouse_buttons");
	map(0xfffd40, 0xfffd7f).rw(m_uart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0xff);
	map(0xfffd80, 0xfffd8f).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff);
	map(0xfffdc0, 0xfffdc3).portr("SW1");
	map(0xfffdc0, 0xfffdc0).lw8(
		[this](u8 data)
		{
			m_cpu->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());
		}, "bt_ff_w");
	if (ST == 4)
		map(0xfffe00, 0xfffe00).lr8([this]() { return m_icu->acknowledge(); }, "iack");
	map(0xfffe08, 0xfffe08).rw(m_icu, FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0xfffe0c, 0xfffe0c).rw(m_icu, FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));
}

INPUT_CHANGED_MEMBER(ceres1_state::mouse_x)
{
	// compute x delta
	int delta = newval - oldval;
	if (delta > 0x80)
		delta -= 0x100;
	else if (delta < -0x80)
		delta += 0x100;

	m_mouse_x = std::clamp(m_mouse_x + delta, 0, 1023);
}

INPUT_CHANGED_MEMBER(ceres1_state::mouse_y)
{
	// compute y delta
	int delta = newval - oldval;
	if (delta > 0x80)
		delta -= 0x100;
	else if (delta < -0x80)
		delta += 0x100;

	m_mouse_y = std::clamp(m_mouse_y - delta, 0, 799);
}

static INPUT_PORTS_START(ceres1)
	PORT_START("mouse_x")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, ceres1_state, mouse_x, 0)

	PORT_START("mouse_y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, ceres1_state, mouse_y, 0)

	PORT_START("mouse_buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Left Button")   PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mouse Middle Button") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Mouse Right Button")  PORT_CODE(MOUSECODE_BUTTON2)

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
	m_cpu->set_addrmap(4, &ceres1_state::cpu_map<4>);

	NS32081(config, m_fpu, 10_MHz_XTAL);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 10_MHz_XTAL);
	m_cpu->set_mmu(m_mmu);

	RAM(config, m_ram);
	m_ram->set_default_size("4MiB");
	m_ram->set_default_value(0);
	m_ram->set_extra_options("2MiB,6MiB,8MiB");

	// TODO: port A txd/rxd connect to keyboard
	SCN2681(config, m_uart, 3.6864_MHz_XTAL);
	m_uart->irq_cb().set(m_icu, FUNC(am9519_device::ireq2_w)).invert();
	m_uart->b_tx_cb().set(m_serial, FUNC(rs232_port_device::write_txd)).invert();
	m_uart->outport_cb().set(m_serial, FUNC(rs232_port_device::write_dtr)).bit(0).invert();
	m_uart->outport_cb().set(m_serial, FUNC(rs232_port_device::write_rts)).bit(1).invert();
	m_uart->outport_cb().set(m_icu, FUNC(am9519_device::ireq0_w)).bit(3);
	m_uart->outport_cb().set(m_icu, FUNC(am9519_device::ireq4_w)).bit(4);

	RS232_PORT(config, m_serial, default_rs232_devices, nullptr);
	m_serial->rxd_handler().set(m_uart, FUNC(scn2681_device::rx_b_w));
	m_serial->cts_handler().set(m_uart, FUNC(scn2681_device::ip1_w));
	m_serial->dcd_handler().set(m_uart, FUNC(scn2681_device::ip0_w));
	m_serial->dsr_handler().set(m_uart, FUNC(scn2681_device::ip2_w));

	// TODO: RS-485 ports "na" and "nb"
	SCC8530N(config, m_scc, 6_MHz_XTAL);
	m_scc->configure_channels(m_uart->clock(), 0, m_uart->clock(), 0);
	m_scc->out_int_callback().set(m_icu, FUNC(am9519_device::ireq1_w)).invert();

	M3002(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq_out().set(m_icu, FUNC(am9519_device::ireq5_w));

	AM9519(config, m_icu);
	m_icu->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	// TODO: WD1002-05 Winchester/Floppy Disk Controller (WFC)
	HARDDISK(config, m_hdd[0], 0);
	HARDDISK(config, m_hdd[1], 0);
	HARDDISK(config, m_hdd[2], 0);

	WD2797(config, m_fdc, 20_MHz_XTAL / 20);
	FLOPPY_CONNECTOR(config, "fdc:0", "fdd", FLOPPY_35_DD, true, floppy_image_device::default_mfm_floppy_formats);

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
					bitmap.pix(y, x + i) = BIT(data, i) ? rgb_t::white() : rgb_t::black();
			}
		}
	}

	return 0;
}

ROM_START(ceres1)
	ROM_REGION32_LE(0x8000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "300689", "30.6.89")
	ROMX_LOAD("by0_u44__30_6_89.u44", 0x0000, 0x2000, CRC(8e1559fe) SHA1(2e11c144fd698b9576816f5cfb003c46ab9ce43b), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("by1_u43__30_6_89.u43", 0x0001, 0x2000, CRC(18625b0c) SHA1(3319043507b7eeb488ecd839708032f968e6e53e), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("by2_u42__30_6_89.u42", 0x0002, 0x2000, CRC(a56497c5) SHA1(85858bea9fd27031ee0191b1d508c7469a384f10), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("by3_u41__30_6_89.u41", 0x0003, 0x2000, CRC(6b628912) SHA1(f76ea089e8bf82f572f27def344b5174e577ebbb), ROM_BIOS(0) | ROM_SKIP(3))

	ROM_SYSTEM_BIOS(1, "231087", "CBUG-Ceres32 V231087 Release 3.2")
	ROMX_LOAD("cbug32__by0__23_10_87.u44", 0x0000, 0x2000, CRC(177bab38) SHA1(ce8c9edbcd142b638a4e906b1d88b6770ac41702), ROM_BIOS(1) | ROM_SKIP(3))
	ROMX_LOAD("cbug32__by1__23_10_87.u43", 0x0001, 0x2000, CRC(4ed51850) SHA1(66b2ee07c88bfa3adac1d75eab463e7f280220a3), ROM_BIOS(1) | ROM_SKIP(3))
	ROMX_LOAD("cbug32__by2__23_10_87.u42", 0x0002, 0x2000, CRC(a05cdf91) SHA1(509b97bfb9e4631a3c6e9af27458d1099903a0ff), ROM_BIOS(1) | ROM_SKIP(3))
	ROMX_LOAD("cbug32__by3__23_10_87.u41", 0x0003, 0x2000, CRC(7e3fd512) SHA1(77a9590490f8fd2212b79cd6c4f81b8f46d6b486), ROM_BIOS(1) | ROM_SKIP(3))
ROM_END

}

/*   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                                        FULLNAME   FLAGS */
COMP(1986, ceres1, 0,      0,      ceres1,  ceres1, ceres1_state, empty_init, "Eidgenössische Technische Hochschule Zürich", "Ceres-1", MACHINE_NOT_WORKING)
