// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for DECmate II & III business PCs.

    The DECmate II & III are the last and in some ways least PDP-8-like
    members of DEC's 12-bit computer line. Based on a Harris HD-6120 CMOS
    LSI CPU with 64K words (advertised as 96 KB) of RAM, they were designed
    mostly to run WPS on OS/278 and are not very compatible with older
    software.

    DECmate II (PC278) abandoned the system-in-a-terminal form factor of the
    VT78 DECstation and VT278 DECmate, adopting the BA25 chassis also used
    in the Rainbow 100. Instead of the custom RX01 or RX02 8-inch disk
    drives (whose use is supported only through the optional PC27X-BA
    adapter), the one or two internal 400 KB RX50 drives are basically
    industry-standard 5.25-inch floppy disk drives, with a CPU interface
    that simulates the old RX8-E using a 8051 MCU with 2 KB of buffer SRAM
    and an off-the-shelf FDC. The CRT9007 CRT controller (which may be
    configured for 80-column or 132-column modes) and LK201 keyboard are the
    same as used in the VT220 display terminal, which the monochrome (white,
    green or amber) VR201 monitor also physically resembles. (As with the
    DECmate's VT100 keyboard, the LK201 may have its PF1 key is painted gold
    to highlight its importance to DECmate software.) DECmate II also differs
    from its CMOS-8 predecessors in using standard 8-bit EPROMs rather than
    specialized 12-bit ROMs for its "control panel" program.

    DECmate II had three different types of expansion options: the
    aforementioned storage adapter, which also supported a 10 MB hard disk
    (RDC51-CA); a graphics board supporting monochrome or color graphics on
    a second monitor; and an Auxiliary Processor Unit. The basic APU
    (PC27X-AA or -AB) ran CP/M on a Z80 with 64K of its own RAM. The
    upgraded XPU (PC27X-AH or PC27X-AJ) could also run MS-DOS 2.11 using an
    additional 8086 CPU with 256K or 512K of RAM.

    DECmate III (PC238) was a repackaging of the DECmate II in a smaller but
    taller box that provides space for only one internal RX33 floppy drive.
    Much of the timing and support logic is compressed into two large PLCC
    gate arrays (DC381 and DC382), with a PLL and PAL16R8 to assist the FDC.
    The DECmate III's own APU and graphics options share a single and unique
    expansion connector; the PC23X-CA graphics card supported RGB color
    output on a VR241 monitor. DEC also later released an enhanced version
    called DECmate III Plus (PC24P), which added a 20 MB RD31 hard disk.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/pdp8/pdp8.h"
#include "cpu/mcs51/mcs51.h"
#include "imagedev/floppy.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/com8116.h"
#include "machine/dec_lk201.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"
#include "video/crt9007.h"
#include "screen.h"

class decmate2_state : public driver_device
{
public:
	decmate2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rxcpu(*this, "rxcpu")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, {"fdc:a0", "fdc:a1", "fdc:b0", "fdc:b1"})
		, m_kbduart(*this, "kbduart")
		, m_prtuart(*this, "prtuart")
		, m_mpscc(*this, "mpscc")
		, m_brg(*this, "brg%u", 0U)
		, m_crtc(*this, "crtc")
		, m_cprom(*this, "cprom")
		, m_chargen(*this, "chargen")
		, m_rxdata(0)
	{
	}

	void pc278(machine_config &config);
	void pc238(machine_config &config);
	void init_pc278();
	void init_pc238();

protected:
	virtual void machine_start() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 rx_ldata_r();
	void rx_ldata_w(u8 data);
	u8 rx_hdata_r();
	void rx_hdata_w(u8 data);
	void rx_control_w(u8 data);
	u8 rx_status_r();
	u8 rx_intr_r();
	void rx_sel_w(u8 data);
	u8 rx_rdy_r();

	void mem_map(address_map &map);
	void rx_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<i8051_device> m_rxcpu;
	required_device<fd1793_device> m_fdc;
	optional_device_array<floppy_connector, 4> m_floppy;
	required_device<ay31015_device> m_kbduart;
	required_device<ay31015_device> m_prtuart;
	required_device<upd7201_device> m_mpscc;
	optional_device_array<com8116_device, 2> m_brg;
	required_device<crt9007_device> m_crtc;
	required_region_ptr<u16> m_cprom;
	required_region_ptr<u8> m_chargen;

	u16 m_rxdata; // 74LS298, writable from both 6120 and 8051
};

void decmate2_state::init_pc278()
{
	for (u16 addr = 0; addr < 04000; addr++)
	{
		// Shift E113 and E114 ROMs into their proper places and separate even and odd bits of E115
		m_cprom[addr | 04000] = (m_cprom[addr | 04000] & 0xff00) >> 4 | bitswap<4>(m_cprom[addr], 6, 4, 2, 0);
		m_cprom[addr] = (m_cprom[addr] & 0xff00) >> 4 | bitswap<4>(m_cprom[addr], 7, 5, 3, 1);
	}
}

void decmate2_state::init_pc238()
{
	for (u16 addr = 0; addr < 010000; addr++)
		m_cprom[addr] &= 07777;
}

void decmate2_state::machine_start()
{
	if (m_brg[1].found())
	{
		m_brg[1]->str_w(12);
		m_brg[1]->stt_w(0);
	}

	m_kbduart->write_np(1);
	m_kbduart->write_nb2(1);
	m_kbduart->write_nb1(1);
	m_kbduart->write_eps(1);
	m_kbduart->write_tsb(0);
	m_kbduart->write_cs(1);
	m_kbduart->write_swe(0);

	m_prtuart->write_np(1);
	m_prtuart->write_nb2(1);
	m_prtuart->write_nb1(1);
	m_prtuart->write_eps(1);
	m_prtuart->write_tsb(0);
	m_prtuart->write_cs(1);
	m_prtuart->write_swe(0);

	m_fdc->dden_w(0);

	// 7201 modem signal inputs are either pulled up or grounded
	m_mpscc->ctsa_w(0);
	m_mpscc->dcda_w(0);
	m_mpscc->dcdb_w(1);
	m_mpscc->rxb_w(0);
	m_mpscc->synca_w(1);
	m_mpscc->syncb_w(1);

	save_item(NAME(m_rxdata));
}

u32 decmate2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// TODO
	return 0;
}

u8 decmate2_state::rx_ldata_r()
{
	return m_rxdata & 0xff;
}

void decmate2_state::rx_ldata_w(u8 data)
{
	m_rxdata = (m_rxdata & 07400) | data;
}

u8 decmate2_state::rx_hdata_r()
{
	return m_rxdata >> 8;
}

void decmate2_state::rx_hdata_w(u8 data)
{
	m_rxdata = (m_rxdata & 0377) | (data & 0x0f) << 8;
}

void decmate2_state::rx_control_w(u8 data)
{
	// TODO
}

u8 decmate2_state::rx_status_r()
{
	// TODO
	return 0;
}

u8 decmate2_state::rx_intr_r()
{
	return m_fdc->intrq_r() << 5;
}

void decmate2_state::rx_sel_w(u8 data)
{
	const int n = BIT(~data, 1, 2);
	if (BIT(data, 3) && m_floppy[n].found())
	{
		floppy_image_device *const img = m_floppy[n]->get_device();
		m_fdc->set_floppy(img);
		if (img != nullptr)
			img->ss_w(BIT(data, 0)); // or inverted?
	}
	else
		m_fdc->set_floppy(nullptr);

	for (int i = 0; i < 4 && m_floppy[i].found(); i++)
	{
		floppy_image_device *img = m_floppy[i]->get_device();
		if (img != nullptr)
			img->mon_w((i & 2) == (~data & 0x14) >> 1 ? 0 : 1);
	}
}

u8 decmate2_state::rx_rdy_r()
{
	return m_fdc->drq_r() << 6 /*| m_fdc->ready_r() << 7*/;
}

void decmate2_state::mem_map(address_map &map)
{
	map(00000, 07777).rom().region("cprom", 0);
}

// TODO: all I/O devices
// 03/04/07: Control panel interrupt
// 05: Keyboard input
// 06: Video interrupt
// 11: Keyboard output
// 12: CRTC
// 13: Clock
// 14: APU option
// 30: Communications receive
// 31: Communications transmit
// 32: Printer input
// 33: Printer output and baud rate
// 36: Modem control and baud rate
// 75: RX Simulator

void decmate2_state::rx_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x000, 0x000).mirror(0x3f0).rw(FUNC(decmate2_state::rx_ldata_r), FUNC(decmate2_state::rx_ldata_w));
	map(0x001, 0x001).mirror(0x3f0).rw(FUNC(decmate2_state::rx_hdata_r), FUNC(decmate2_state::rx_hdata_w));
	map(0x002, 0x002).mirror(0x3f0).w(FUNC(decmate2_state::rx_control_w));
	map(0x003, 0x003).mirror(0x3f0).r(FUNC(decmate2_state::rx_status_r));
	map(0x008, 0x00b).mirror(0x3f4).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x400, 0x7ff).ram();
}

static INPUT_PORTS_START(decmate2)
INPUT_PORTS_END

static void rx50_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD); // MFM, 2 sides, 80 tracks
}

// XTALs: 5.0688 MHz, 15.741 MHz, 22.896 MHz, 16.000 MHz
void decmate2_state::pc278(machine_config &config)
{
	PDP8(config, m_maincpu, 16_MHz_XTAL / 2); // FIXME: HD-6120 with dynamic divider
	m_maincpu->set_addrmap(AS_PROGRAM, &decmate2_state::mem_map);

	I8051(config, m_rxcpu, 16_MHz_XTAL / 2);
	m_rxcpu->set_addrmap(AS_IO, &decmate2_state::rx_map);
	m_rxcpu->port_in_cb<1>().set(FUNC(decmate2_state::rx_intr_r));
	m_rxcpu->port_out_cb<1>().set(FUNC(decmate2_state::rx_sel_w));
	m_rxcpu->port_in_cb<2>().set(FUNC(decmate2_state::rx_rdy_r));

	FD1793(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->set_force_ready(false);

	FLOPPY_CONNECTOR(config, m_floppy[0], rx50_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], rx50_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], rx50_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], rx50_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	AY31015(config, m_kbduart); // 6402
	m_kbduart->write_so_callback().set("keyboard", FUNC(lk201_device::rx_w));
	m_kbduart->set_auto_rdav(true);

	AY31015(config, m_prtuart); // 6402
	m_prtuart->write_so_callback().set("printer", FUNC(rs232_port_device::write_txd));
	m_prtuart->write_fe_callback().set_nop(); // TODO: output on connector pin 9
	m_prtuart->set_auto_rdav(true);

	UPD7201(config, m_mpscc, 16_MHz_XTAL / 8);
	m_mpscc->out_txda_callback().set("com", FUNC(rs232_port_device::write_txd));

	COM8116(config, m_brg[0], 5.0688_MHz_XTAL); // 5016T
	m_brg[0]->fr_handler().set(m_prtuart, FUNC(ay31015_device::write_rcp));
	m_brg[0]->fr_handler().append(m_prtuart, FUNC(ay31015_device::write_tcp));
	m_brg[0]->ft_handler().set(m_mpscc, FUNC(upd7201_device::txca_w));
	m_brg[0]->ft_handler().append(m_mpscc, FUNC(upd7201_device::rxca_w));

	COM8116(config, m_brg[1], 5.0688_MHz_XTAL); // 5016T
	m_brg[1]->fr_handler().set(m_kbduart, FUNC(ay31015_device::write_rcp));
	m_brg[1]->fr_handler().append(m_kbduart, FUNC(ay31015_device::write_tcp));
	// TODO: other output is divided down to 100 Hz

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(15.741_MHz_XTAL, 990, 0, 800, 265, 0, 240); // 24x80, 10x10 character cell
	//screen.set_raw(22.896_MHz_XTAL, 1440, 0, 1188, 265, 0, 240); // 24x132, 9x10 character cell?
	screen.set_screen_update(FUNC(decmate2_state::screen_update));

	CRT9007(config, m_crtc, 15.741_MHz_XTAL / 10);
	m_crtc->set_character_width(10); // 9 in 132-column mode
	m_crtc->set_screen("screen");

	LK201(config, "keyboard").tx_handler().set(m_kbduart, FUNC(ay31015_device::write_si));

	rs232_port_device &com(RS232_PORT(config, "com", default_rs232_devices, nullptr));
	com.rxd_handler().set(m_mpscc, FUNC(upd7201_device::rxa_w));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.rxd_handler().set(m_prtuart, FUNC(ay31015_device::write_si)); // TODO: loopback control
}

void decmate2_state::pc238(machine_config &config)
{
	pc278(config);
	config.device_remove("brg0");
	config.device_remove("brg1");
	config.device_remove("fdc:b0");
	config.device_remove("fdc:b1");

	clock_device &pclk(CLOCK(config, "pclk", 16_MHz_XTAL / 208)); // Generated on pin 65 of DC382
	pclk.signal_handler().set(m_kbduart, FUNC(ay31015_device::write_rcp));
	pclk.signal_handler().append(m_kbduart, FUNC(ay31015_device::write_tcp));
	pclk.signal_handler().append(m_prtuart, FUNC(ay31015_device::write_rcp));
	pclk.signal_handler().append(m_prtuart, FUNC(ay31015_device::write_tcp));

	// TODO: COMCLK (generated on pin 39 of DC382, rate presumably programmable)

	// DECmate III has no 15.741 MHz XTAL
	m_crtc->set_clock(22.896_MHz_XTAL / 15);
	subdevice<screen_device>("screen")->set_raw(22.896_MHz_XTAL * 2 / 3, 960, 0, 800, 265, 0, 240);
}

ROM_START(decmate2)
	ROM_REGION16_BE(0x2000, "cprom", 0)
	//ROM_LOAD16_BYTE("23-399e2.e113", 0x0000, 0x0800, NO_DUMP) // 0000-3777, bits 0:7 (to be shifted)
	//ROM_LOAD16_BYTE("23-400e2.e114", 0x1000, 0x0800, NO_DUMP) // 4000-7777, bits 0:7 (to be shifted)
	//ROM_LOAD16_BYTE("23-401e2.e115", 0x0001, 0x0800, NO_DUMP) // 0000-3777 & 4000-7777, bits 8:11 (to be split up)
	ROM_SYSTEM_BIOS(0, "19h", "19H (2310)")
	ROMX_LOAD("23-390e2.e113", 0x0000, 0x0800, CRC(9b19451a) SHA1(ed26557f17f59ce05ca08b34a05a24a97388dfe0), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("23-391e2.e114", 0x1000, 0x0800, CRC(3a09ada1) SHA1(3093bd926d49c2fd62a773e8019e3755aa165ae9), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("23-392e2.e115", 0x0001, 0x0800, CRC(19901cb6) SHA1(82a642d2b5b56250611f69321d0251e27fa639fc), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(1, "31z", "31Z (3732)") // Regenerated from source code listing
	ROMX_LOAD("23-358e2.e113", 0x0000, 0x0800, BAD_DUMP CRC(e6b9ab4d) SHA1(12533bab586b7bc753fae3ce1959bc5ced3905f5), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("23-359e2.e114", 0x1000, 0x0800, BAD_DUMP CRC(3d1825b7) SHA1(753af9657eef1a284801c9765f4899563d3d9a20), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("23-360e2.e115", 0x0001, 0x0800, BAD_DUMP CRC(ed8162e6) SHA1(4ab993c00a5afc6465153ca4f52faa76e02a0ef3), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_REGION(0x1000, "rxcpu", 0)
	ROM_LOAD("23-008m1.e27", 0x0000, 0x1000, CRC(fae4026b) SHA1(388e093d952ce1f6fcf2dcdab5b5099a6aafad0f))

	ROM_REGION(0x100, "precomp", 0) // 256x4 bipolar ROM (only one bit used)
	ROM_LOAD("23-640a2.e13", 0x000, 0x100, NO_DUMP)

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("23-114e4.e173", 0x0000, 0x2000, NO_DUMP)
ROM_END

ROM_START(decmate3)
	ROM_REGION16_BE(0x2000, "cprom", 0)
	ROM_LOAD16_BYTE("23-330e4.e33", 0x0000, 0x1000, CRC(4258e0d0) SHA1(09a2f5f25620b491aed87b3c6465fb0a4c4211ff))
	ROM_CONTINUE(0x0001, 0x1000)

	ROM_REGION(0x1000, "rxcpu", 0)
	ROM_LOAD("23-008m1.e4", 0x0000, 0x1000, CRC(fae4026b) SHA1(388e093d952ce1f6fcf2dcdab5b5099a6aafad0f))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("23-331e4.e42", 0x0000, 0x2000, CRC(dca00fae) SHA1(5beff80611149cbae3e91c813d302f09a82fc3dd))

	// TODO: add NO_DUMP entry for PAL (23-097K5)
ROM_END

COMP(1982, decmate2, 0, 0, pc278, decmate2, decmate2_state, init_pc278, "Digital Equipment Corporation", "DECmate II (PC278)", MACHINE_IS_SKELETON)
COMP(1984, decmate3, 0, 0, pc238, decmate2, decmate2_state, init_pc238, "Digital Equipment Corporation", "DECmate III (PC238)", MACHINE_IS_SKELETON)
