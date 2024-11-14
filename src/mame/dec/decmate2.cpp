// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Preliminary driver for DECmate II & III business PCs.

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
    from its CMOS-8 predecessors in using generic 8-bit EPROMs or mask ROMs
    rather than specialized 12-bit ROMs for its "control panel" program.

    DECmate II had three different types of expansion options: the
    aforementioned storage adapter, which also supported a 10 MB hard disk
    interface (RDC51-CA) with its own 8051; a graphics board supporting
    monochrome or color graphics on a second monitor; and an Auxiliary
    Processor Unit. The basic APU (PC27X-AA or -AB) ran CP/M on a Z80 with
    64K of its own RAM. The upgraded XPU (PC27X-AH or PC27X-AJ) could also
    run MS-DOS 2.11 using an additional 8086 CPU with 256K or 512K of RAM.

    DECmate III (PC238) was a repackaging of the DECmate II in a smaller but
    taller box that provides space for only one internal RX50 floppy drive.
    Much of the timing and support logic is compressed into two large PLCC
    gate arrays (DC381 and DC382), with a PLL and PAL16R8 to assist the FDC.
    (One consequence of this reduction is that the printer port is fixed to
    the same baud rate as the keyboard.) The DECmate III's own APU and
    graphics options share a single and unique expansion connector; the
    PC23X-CA graphics card supported RGB color output on a VR241 monitor.
    DEC also later released an enhanced version called DECmate III Plus
    (PC24P), which included a 20 MB RD31 hard disk underneath a half-height
    RX33 floppy drive.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/pdp8/hd6120.h"
#include "cpu/mcs51/mcs51.h"
#include "imagedev/floppy.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/com8116.h"
#include "lk201.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"
#include "video/crt9007.h"
#include "screen.h"


namespace {

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
		, m_ram(*this, "ram")
		, m_cprom(*this, "cprom")
		, m_chargen(*this, "chargen")
		, m_cprom_iview(*this, "cpromi")
		, m_cprom_dview(*this, "cpromd")
		, m_cpsel(false)
		, m_vint(false)
		, m_cpromsel(false)
		, m_eadd0(false)
		, m_com_control_read(false)
		, m_kbd_rflg(false)
		, m_kbd_tflg(false)
		, m_prt_rflg(false)
		, m_prt_tflg(false)
		, m_crtc_addr(0)
		, m_rxdata(0)
		, m_rx_control(0)
		, m_rx_status(0)
	{
	}

	void pc278(machine_config &config);
	void pc238(machine_config &config);
	void init_pc278();
	void init_pc238();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vint_w(int state);

	void lxmar_w(offs_t offset, u16 data);
	void lxpar_w(offs_t offset, u16 data);
	void lxdar_w(offs_t offset, u16 data);
	void wsr_w(u16 data);
	u16 cprom_switch_r(offs_t offset);
	void pc278_ioclr_w(int state);
	void pc238_ioclr_w(int state);

	void keyboard_dr_w(int state);
	void keyboard_dr_ff_w(int state);
	void keyboard_tbre_w(int state);
	void keyboard_tbre_ff_w(int state);
	void printer_dr_w(int state);
	void printer_dr_ff_w(int state);
	void printer_tbre_w(int state);
	void printer_tbre_ff_w(int state);
	u8 kbdrflg_devctl_r();
	void kbdrflg_set_w(u16 data);
	void kbdrflg_clear_w(u16 data);
	u16 kbduart_r();
	u8 kbdtflg_devctl_r();
	void kbdtflg_set_w(u16 data);
	void kbdtflg_clear_w(u16 data);
	u8 prtrflg_devctl_r();
	void prtrflg_set_w(u16 data);
	void prtrflg_clear_w(u16 data);
	u16 prtuart_r();
	void prttflg_set_w(u16 data);
	void prttflg_clear_w(u16 data);
	u8 prttflg_devctl_r();
	u8 apten_r();

	u8 comreg_devctl_r();
	u16 comreg_r();
	void comreg_w(u16 data);
	void cominit_w(u16 data);

	u8 vint_devctl_r();
	void lscreg_w(u16 data);
	void wrcrtc_w(u16 data);
	u16 rdcrtc_r();
	void video_mod_w(u16 data);

	void modem_w(u16 data);

	void sel_w(u16 data);
	void lcd_w(u16 data);
	u16 xdr_r();
	void xdr_w(u16 data);
	u8 xdr_devctl_r();
	u8 str_devctl_r();
	u8 ser_devctl_r();
	u8 sdn_devctl_r();
	void intr_w(u16 data);
	void rxinit_w(u16 data);

	u8 rx_ldata_r();
	void rx_ldata_w(u8 data);
	u8 rx_hdata_r();
	void rx_hdata_w(u8 data);
	void rx_control_w(u8 data);
	u8 rx_status_r();
	u8 rx_intr_r();
	void rx_sel_w(u8 data);
	u8 rx_rdy_r();

	void inst_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
	void pc278_io_map(address_map &map) ATTR_COLD;
	void pc238_io_map(address_map &map) ATTR_COLD;
	void devctl_map(address_map &map) ATTR_COLD;
	void rx_map(address_map &map) ATTR_COLD;

	required_device<hd6120_device> m_maincpu;
	required_device<i8051_device> m_rxcpu;
	required_device<fd1793_device> m_fdc;
	optional_device_array<floppy_connector, 4> m_floppy;
	required_device<ay31015_device> m_kbduart;
	required_device<ay31015_device> m_prtuart;
	required_device<upd7201_device> m_mpscc;
	optional_device_array<com8116_device, 2> m_brg;
	required_device<crt9007_device> m_crtc;
	required_shared_ptr<u16> m_ram;
	required_region_ptr<u16> m_cprom;
	required_region_ptr<u8> m_chargen;
	memory_view m_cprom_iview;
	memory_view m_cprom_dview;

	bool m_cpsel;
	bool m_vint;
	bool m_cpromsel;
	bool m_eadd0;
	bool m_com_control_read;

	bool m_kbd_rflg;
	bool m_kbd_tflg;
	bool m_prt_rflg;
	bool m_prt_tflg;

	u8 m_crtc_addr;
	u16 m_rxdata; // 74LS298, writable from both 6120 and 8051
	u8 m_rx_control;
	u8 m_rx_status;
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

	save_item(NAME(m_cpsel));
	save_item(NAME(m_vint));
	save_item(NAME(m_cpromsel));
	save_item(NAME(m_eadd0));
	save_item(NAME(m_com_control_read));
	save_item(NAME(m_kbd_rflg));
	save_item(NAME(m_kbd_tflg));
	save_item(NAME(m_prt_rflg));
	save_item(NAME(m_prt_tflg));
	save_item(NAME(m_crtc_addr));
	save_item(NAME(m_rxdata));
	save_item(NAME(m_rx_control));
	save_item(NAME(m_rx_status));
}

void decmate2_state::machine_reset()
{
	m_cprom_iview.select(0);
	m_cprom_dview.select(0);
	m_cpromsel = false;
}

u32 decmate2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// TODO
	return 0;
}

void decmate2_state::vint_w(int state)
{
	// TODO: synchronize
	m_vint = state;
}

void decmate2_state::lxmar_w(offs_t offset, u16 data)
{
	if (offset == hd6120_device::IFETCH)
		m_cpsel = false;
}

void decmate2_state::lxpar_w(offs_t offset, u16 data)
{
	if (offset == hd6120_device::IFETCH)
		m_cpsel = true;
}

void decmate2_state::lxdar_w(offs_t offset, u16 data)
{
	m_eadd0 = BIT(data, 12); // latched from EMA2
}

void decmate2_state::wsr_w(u16 data)
{
	if (m_cpsel)
	{
		if (BIT(data, 0) != m_cpromsel) // DX11
		{
			m_cprom_iview.select(1);
			m_cpromsel = BIT(data, 0);
		}

		if (BIT(data, 11)) // DX0
			m_cprom_dview.disable();
		else
			m_cprom_dview.select(0);
	}
}

// This is not a precise implementation of the actual RAMDIS control circuit, but is close enough to work just as well
u16 decmate2_state::cprom_switch_r(offs_t offset)
{
	if (m_cpromsel)
	{
		if (!machine().side_effects_disabled())
			m_cprom_iview.disable();
		return m_cprom[offset & 07777];
	}
	else
	{
		if (!machine().side_effects_disabled())
			m_cprom_iview.select(0);
		return m_ram[offset];
	}
}

void decmate2_state::pc278_ioclr_w(int state)
{
	if (!state)
	{
		m_kbd_rflg = false;
		m_kbd_tflg = false;
		m_prt_rflg = false;
		m_prt_tflg = false;
		rxinit_w(0);
		cominit_w(0);
	}
}

void decmate2_state::pc238_ioclr_w(int state)
{
	if (!state)
	{
		rxinit_w(0);
		cominit_w(0);
	}
}

void decmate2_state::keyboard_dr_w(int state)
{
	m_kbd_rflg = state;
}

void decmate2_state::keyboard_dr_ff_w(int state)
{
	// TODO: edge trigger
	if (state)
		m_kbd_rflg = true;
}

void decmate2_state::keyboard_tbre_w(int state)
{
	m_kbd_tflg = state;
}

void decmate2_state::keyboard_tbre_ff_w(int state)
{
	// TODO: edge trigger
	if (state)
		m_kbd_tflg = true;
}

void decmate2_state::printer_dr_w(int state)
{
	m_prt_rflg = state;
}

void decmate2_state::printer_dr_ff_w(int state)
{
	// TODO: edge trigger
	if (state)
		m_prt_rflg = true;
}

void decmate2_state::printer_tbre_w(int state)
{
	m_prt_tflg = state;
}

void decmate2_state::printer_tbre_ff_w(int state)
{
	// TODO: edge trigger
	if (state)
		m_prt_tflg = true;
}

u8 decmate2_state::kbdrflg_devctl_r()
{
	return m_kbd_rflg ? hd6120_device::SKIP : 0;
}

void decmate2_state::kbdrflg_set_w(u16 data)
{
	m_kbd_rflg = true;
}

void decmate2_state::kbdrflg_clear_w(u16 data)
{
	m_kbd_rflg = false;
}

u16 decmate2_state::kbduart_r()
{
	return m_kbduart->receive();
}

u8 decmate2_state::kbdtflg_devctl_r()
{
	return m_kbd_tflg ? hd6120_device::SKIP : 0;
}

void decmate2_state::kbdtflg_set_w(u16 data)
{
	m_kbd_tflg = true;
}

void decmate2_state::kbdtflg_clear_w(u16 data)
{
	m_kbd_tflg = false;
}

u8 decmate2_state::prtrflg_devctl_r()
{
	return m_prt_rflg ? hd6120_device::SKIP : 0;
}

void decmate2_state::prtrflg_set_w(u16 data)
{
	m_prt_rflg = true;
}

void decmate2_state::prtrflg_clear_w(u16 data)
{
	m_prt_rflg = false;
}

u16 decmate2_state::prtuart_r()
{
	return m_prtuart->receive();
}

u8 decmate2_state::prttflg_devctl_r()
{
	return m_prt_tflg ? hd6120_device::SKIP : 0;
}

void decmate2_state::prttflg_set_w(u16 data)
{
	m_prt_tflg = true;
}

void decmate2_state::prttflg_clear_w(u16 data)
{
	m_prt_tflg = false;
}

u8 decmate2_state::apten_r()
{
	// TODO: from pin 4 of printer connector J2
	return 0;
}

u8 decmate2_state::comreg_devctl_r()
{
	if (m_com_control_read)
		return hd6120_device::C1;
	else
		return 0;
}

u16 decmate2_state::comreg_r()
{
	if (m_eadd0)
		return m_mpscc->cb_r();
	else
		return m_mpscc->ca_r();
}

void decmate2_state::comreg_w(u16 data)
{
	if (!m_com_control_read)
	{
		if (m_eadd0)
			m_mpscc->cb_w(data & 0377);
		else
			m_mpscc->ca_w(data & 0377);
	}
	m_com_control_read = BIT(data, 11);
}

void decmate2_state::cominit_w(u16 data)
{
	m_mpscc->reset();
	m_com_control_read = false;
}

u8 decmate2_state::vint_devctl_r()
{
	return m_vint ? hd6120_device::SKIP : 0;
}

void decmate2_state::lscreg_w(u16 data)
{
	m_crtc_addr = data & 077;
}

void decmate2_state::wrcrtc_w(u16 data)
{
	m_crtc->write(m_crtc_addr, data & 0377);
}

u16 decmate2_state::rdcrtc_r()
{
	return m_crtc->read(m_crtc_addr);
}

void decmate2_state::video_mod_w(u16 data)
{
	logerror("%s: Loading %04o into video mod register (loopback %s)\n", machine().describe_context(), data, BIT(data, 4) ? "enabled" : "disabled");
}

void decmate2_state::modem_w(u16 data)
{
	logerror("%s: Writing %04o to modem control register\n", machine().describe_context(), data);
}

void decmate2_state::sel_w(u16 data)
{
	logerror("%s: RX drive pair %c selected\n", machine().describe_context(), BIT(data, 0) ? 'B' : 'A');
	if (BIT(data, 0))
		m_rx_status |= 0x80;
	else
		m_rx_status &= 0x7f;
}

void decmate2_state::lcd_w(u16 data)
{
	logerror("%s: RX %d-bit command %d, drive %d\n", machine().describe_context(), BIT(data, 6) ? 8 : 12, BIT(data, 1, 3), BIT(data, 4));
	m_rxdata = data;
	m_rx_status &= 0xfc;
	if (!BIT(data, 6))
		m_rx_status |= 0x01;
	m_rxcpu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}

u16 decmate2_state::xdr_r()
{
	if (BIT(m_rx_status, 1))
	{
		if (!machine().side_effects_disabled())
			logerror("%s: XDR received %04o from 8051\n", machine().describe_context(), m_rxdata);
		return m_rxdata;
	}
	else
		return 0;
}

void decmate2_state::xdr_w(u16 data)
{
	if (!BIT(m_rx_status, 1))
	{
		logerror("%s: XDR transmit %04o to 8051\n", machine().describe_context(), data);
		m_rxdata = data;
	}
	m_rxcpu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}

u8 decmate2_state::xdr_devctl_r()
{
	if (BIT(m_rx_status, 1))
		return hd6120_device::C1 | (BIT(m_rx_status, 0) ? hd6120_device::C0 : 0);
	else
		return 0;
}

u8 decmate2_state::str_devctl_r()
{
	if (BIT(m_rx_control, 3))
	{
		if (!machine().side_effects_disabled())
			m_rx_control &= 0x07;
		return hd6120_device::SKIP;
	}
	else
		return 0;
}

u8 decmate2_state::ser_devctl_r()
{
	if (BIT(m_rx_control, 2))
	{
		if (!machine().side_effects_disabled())
			m_rx_control &= 0x0b;
		return hd6120_device::SKIP;
	}
	else
		return 0;
}

u8 decmate2_state::sdn_devctl_r()
{
	if (BIT(m_rx_control, 0))
	{
		if (!machine().side_effects_disabled())
			m_rx_control &= 0x0e;
		return hd6120_device::SKIP;
	}
	else
		return 0;
}

void decmate2_state::intr_w(u16 data)
{
	logerror("%s: RX interrupt enable %s\n", machine().describe_context(), BIT(data, 0) ? "set" : "cleared");
}

void decmate2_state::rxinit_w(u16 data)
{
	m_rx_control = 0;
	m_rx_status = (m_rx_status & 0x02) | 0x01;
	m_rxcpu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
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
	m_rx_control = data & 0x0d;
	m_rx_status = (m_rx_status & 0xfd) | (data & 0x02);
	m_rxcpu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);
	m_rxcpu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
}

u8 decmate2_state::rx_status_r()
{
	return m_rx_status;
}

u8 decmate2_state::rx_intr_r()
{
	return m_fdc->intrq_r() << 5 | 0xdf;
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
	return m_fdc->drq_r() << 6 | 1 /*m_fdc->ready_r()*/ << 7 | 0x3f;
}

void decmate2_state::inst_map(address_map &map)
{
	map(0000000, 0177777).ram().share("ram");
	map(0000000, 0177777).view(m_cprom_iview);
	m_cprom_iview[0](0000000, 0007777).mirror(0170000).rom().region("cprom", 0);
	m_cprom_iview[1](0000000, 0177777).r(FUNC(decmate2_state::cprom_switch_r));
}

void decmate2_state::data_map(address_map &map)
{
	map(0000000, 0177777).ram().share("ram");
	map(0000000, 0177777).view(m_cprom_dview);
	m_cprom_dview[0](0000000, 0007777).mirror(0170000).rom().region("cprom", 0);
}

// TODO: almost all I/O devices
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

void decmate2_state::pc278_io_map(address_map &map)
{
	map(0050, 0050).w(FUNC(decmate2_state::kbdtflg_set_w));
	map(0051, 0051).w(FUNC(decmate2_state::kbdtflg_clear_w));
	map(0054, 0054).mirror(2).w(m_kbduart, FUNC(ay31015_device::transmit)).umask16(0377);
	map(0061, 0061).nopw();
	map(0110, 0110).w(FUNC(decmate2_state::kbdrflg_set_w));
	map(0111, 0111).w(FUNC(decmate2_state::kbdrflg_clear_w));
	map(0112, 0112).nopw();
	map(0114, 0114).mirror(2).r(FUNC(decmate2_state::kbduart_r)).nopw();
	map(0121, 0121).nopw();
	map(0122, 0122).w(FUNC(decmate2_state::lscreg_w));
	map(0124, 0124).w(FUNC(decmate2_state::wrcrtc_w));
	map(0126, 0126).w(FUNC(decmate2_state::video_mod_w));
	map(0127, 0127).r(FUNC(decmate2_state::rdcrtc_r)).nopw();
	map(0320, 0320).w(FUNC(decmate2_state::prtrflg_set_w));
	map(0321, 0321).w(FUNC(decmate2_state::prtrflg_clear_w));
	map(0322, 0322).nopw();
	map(0324, 0324).mirror(2).r(FUNC(decmate2_state::prtuart_r)).nopw();
	map(0330, 0330).w(FUNC(decmate2_state::prttflg_set_w));
	map(0331, 0331).w(FUNC(decmate2_state::prttflg_clear_w));
	map(0334, 0334).mirror(2).w(m_prtuart, FUNC(ay31015_device::transmit)).umask16(0377);
	map(0362, 0362).w(FUNC(decmate2_state::modem_w));
	map(0366, 0366).rw(FUNC(decmate2_state::comreg_r), FUNC(decmate2_state::comreg_w));
	map(0367, 0367).w(FUNC(decmate2_state::cominit_w));
	map(0750, 0750).w(FUNC(decmate2_state::sel_w));
	map(0751, 0751).w(FUNC(decmate2_state::lcd_w));
	map(0752, 0752).rw(FUNC(decmate2_state::xdr_r), FUNC(decmate2_state::xdr_w));
	map(0753, 0755).nopw();
	map(0756, 0756).w(FUNC(decmate2_state::intr_w));
	map(0757, 0757).w(FUNC(decmate2_state::rxinit_w));
}

void decmate2_state::pc238_io_map(address_map &map)
{
	pc278_io_map(map);
	map(0050, 0051).nopw();
	map(0110, 0111).nopw();
	map(0320, 0321).nopw();
	map(0330, 0331).nopw();
}

void decmate2_state::devctl_map(address_map &map)
{
	map(0050, 0050).lr8(NAME([]() { return 0; }));
	map(0051, 0051).r(FUNC(decmate2_state::kbdtflg_devctl_r));
	map(0054, 0054).lr8(NAME([]() { return 0; }));
	map(0056, 0056).lr8(NAME([]() { return hd6120_device::C0; }));
	map(0061, 0061).r(FUNC(decmate2_state::vint_devctl_r));
	map(0110, 0110).lr8(NAME([]() { return 0; }));
	map(0111, 0111).r(FUNC(decmate2_state::kbdrflg_devctl_r));
	map(0112, 0112).lr8(NAME([]() { return hd6120_device::C0; }));
	map(0114, 0114).lr8(NAME([]() { return hd6120_device::C1; }));
	map(0116, 0116).lr8(NAME([]() { return hd6120_device::C0 | hd6120_device::C1; }));
	map(0121, 0121).r(FUNC(decmate2_state::apten_r));
	map(0122, 0126).lr8(NAME([]() { return 0; }));
	map(0127, 0127).lr8(NAME([]() { return hd6120_device::C1; }));
	map(0302, 0302).lr8(NAME([]() { return hd6120_device::C0; }));
	map(0304, 0304).lr8(NAME([]() { return hd6120_device::C1; }));
	map(0306, 0306).lr8(NAME([]() { return hd6120_device::C0 | hd6120_device::C1; }));
	map(0314, 0314).lr8(NAME([]() { return 0; }));
	map(0316, 0316).lr8(NAME([]() { return hd6120_device::C0; }));
	map(0320, 0320).lr8(NAME([]() { return 0; }));
	map(0321, 0321).r(FUNC(decmate2_state::prtrflg_devctl_r));
	map(0322, 0322).lr8(NAME([]() { return hd6120_device::C0; }));
	map(0324, 0324).lr8(NAME([]() { return hd6120_device::C1; }));
	map(0326, 0326).lr8(NAME([]() { return hd6120_device::C0 | hd6120_device::C1; }));
	map(0330, 0330).lr8(NAME([]() { return 0; }));
	map(0331, 0331).r(FUNC(decmate2_state::prttflg_devctl_r));
	map(0334, 0334).lr8(NAME([]() { return 0; }));
	map(0336, 0336).lr8(NAME([]() { return hd6120_device::C0; }));
	map(0362, 0363).lr8(NAME([]() { return 0; }));
	map(0366, 0366).r(FUNC(decmate2_state::comreg_devctl_r));
	map(0367, 0367).lr8(NAME([]() { return 0; }));
	map(0750, 0750).lr8(NAME([]() { return 0; }));
	map(0751, 0751).lr8(NAME([]() { return hd6120_device::C0; }));
	map(0752, 0752).r(FUNC(decmate2_state::xdr_devctl_r));
	map(0753, 0753).r(FUNC(decmate2_state::str_devctl_r));
	map(0754, 0754).r(FUNC(decmate2_state::ser_devctl_r));
	map(0755, 0755).r(FUNC(decmate2_state::sdn_devctl_r));
	map(0756, 0757).lr8(NAME([]() { return 0; }));
}

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
	PORT_START("LAS")
	PORT_BIT(00010, IP_ACTIVE_LOW, IPT_UNKNOWN) // APU present
	PORT_BIT(00004, IP_ACTIVE_LOW, IPT_UNKNOWN) // storage adapter present
	PORT_BIT(00002, IP_ACTIVE_LOW, IPT_UNKNOWN) // graphics controller present
	PORT_BIT(07761, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static void rx50_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD); // MFM, 2 sides, 80 tracks
}

// XTALs: 5.0688 MHz, 15.741 MHz, 22.896 MHz, 16.000 MHz
void decmate2_state::pc278(machine_config &config)
{
	HD6120(config, m_maincpu, 16_MHz_XTAL / 2);
	// TODO: CPU clock is throttled from 8 MHz to 4 MHz while accessing I/O devices or when CPROM is enabled
	m_maincpu->set_addrmap(AS_PROGRAM, &decmate2_state::inst_map);
	m_maincpu->set_addrmap(AS_DATA, &decmate2_state::data_map);
	m_maincpu->set_addrmap(AS_IO, &decmate2_state::pc278_io_map);
	m_maincpu->set_addrmap(hd6120_device::AS_DEVCTL, &decmate2_state::devctl_map);
	m_maincpu->lxmar_callback().set(FUNC(decmate2_state::lxmar_w));
	m_maincpu->lxpar_callback().set(FUNC(decmate2_state::lxpar_w));
	m_maincpu->lxdar_callback().set(FUNC(decmate2_state::lxdar_w));
	m_maincpu->rsr_callback().set_ioport("LAS");
	m_maincpu->wsr_callback().set(FUNC(decmate2_state::wsr_w));
	m_maincpu->ioclr_callback().set(FUNC(decmate2_state::pc278_ioclr_w));
	m_maincpu->strtup_callback().set_constant(0);

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
	m_kbduart->write_dav_callback().set(FUNC(decmate2_state::keyboard_dr_ff_w));
	m_kbduart->write_tbmt_callback().set(FUNC(decmate2_state::keyboard_tbre_ff_w));
	m_kbduart->set_auto_rdav(true);

	AY31015(config, m_prtuart); // 6402
	m_prtuart->write_so_callback().set("printer", FUNC(rs232_port_device::write_txd));
	m_prtuart->write_fe_callback().set_nop(); // TODO: output on connector pin 9
	m_prtuart->write_dav_callback().set(FUNC(decmate2_state::printer_dr_ff_w));
	m_prtuart->write_tbmt_callback().set(FUNC(decmate2_state::printer_tbre_ff_w));
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
	m_crtc->int_callback().set(FUNC(decmate2_state::vint_w));

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

	m_maincpu->set_addrmap(AS_IO, &decmate2_state::pc238_io_map);
	m_maincpu->ioclr_callback().set(FUNC(decmate2_state::pc238_ioclr_w));

	m_kbduart->write_dav_callback().set(FUNC(decmate2_state::keyboard_dr_w));
	m_kbduart->write_tbmt_callback().set(FUNC(decmate2_state::keyboard_tbre_w));
	m_prtuart->write_dav_callback().set(FUNC(decmate2_state::printer_dr_w));
	m_prtuart->write_tbmt_callback().set(FUNC(decmate2_state::printer_tbre_w));

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
	ROM_SYSTEM_BIOS(0, "19n", "19N (2316)")
	ROMX_LOAD("23-399e2.e113", 0x0000, 0x0800, CRC(2881252e) SHA1(5825bc00833dd33f294f79df11200dce76e78740), ROM_BIOS(0) | ROM_SKIP(1)) // 0000-3777, bits 0:7 (to be shifted)
	ROMX_LOAD("23-400e2.e114", 0x1000, 0x0800, CRC(b6f79f77) SHA1(131a1c28c7d1c90dc1b6c85194f84f120cc60bd9), ROM_BIOS(0) | ROM_SKIP(1)) // 4000-7777, bits 0:7 (to be shifted)
	ROMX_LOAD("23-401e2.e115", 0x0001, 0x0800, CRC(1076b630) SHA1(7be4c30d512b4f86f645cb7a3a20122ee4187759), ROM_BIOS(0) | ROM_SKIP(1)) // 0000-3777 & 4000-7777, bits 8:11 (to be split up)
	ROM_SYSTEM_BIOS(1, "19h", "19H (2310)")
	ROMX_LOAD("23-390e2.e113", 0x0000, 0x0800, CRC(9b19451a) SHA1(ed26557f17f59ce05ca08b34a05a24a97388dfe0), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("23-391e2.e114", 0x1000, 0x0800, CRC(3a09ada1) SHA1(3093bd926d49c2fd62a773e8019e3755aa165ae9), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("23-392e2.e115", 0x0001, 0x0800, CRC(19901cb6) SHA1(82a642d2b5b56250611f69321d0251e27fa639fc), ROM_BIOS(1) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(2, "31z", "31Z (3732)") // Regenerated from source code listing
	ROMX_LOAD("23-358e2.e113", 0x0000, 0x0800, BAD_DUMP CRC(459231ea) SHA1(608342b7129d54a2a0f4a8e0645dad1ae26fccf4), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("23-359e2.e114", 0x1000, 0x0800, BAD_DUMP CRC(3d1825b7) SHA1(753af9657eef1a284801c9765f4899563d3d9a20), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("23-360e2.e115", 0x0001, 0x0800, BAD_DUMP CRC(ed8162e6) SHA1(4ab993c00a5afc6465153ca4f52faa76e02a0ef3), ROM_BIOS(2) | ROM_SKIP(1))

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

} // anonymous namespace


COMP(1982, decmate2, 0, 0, pc278, decmate2, decmate2_state, init_pc278, "Digital Equipment Corporation", "DECmate II (PC278)", MACHINE_IS_SKELETON)
COMP(1984, decmate3, 0, 0, pc238, decmate2, decmate2_state, init_pc238, "Digital Equipment Corporation", "DECmate III (PC238)", MACHINE_IS_SKELETON)
