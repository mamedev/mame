// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    macpwrbkmsc.cpp
    68K Mac PowerBooks based on the MSC/MSC II system ASICs and the PG&E power manager
    By R. Belmont

    Supported machines:
    PowerBook Duo 210:  68030 @ 25 MHz, 640x400 passive-matrix 4bpp grayscale screen, 4 MiB RAM (24 max)
    PowerBook Duo 230:  68030 @ 33 MHz, 640x400 passive-matrix 4bpp grayscale screen, 4 MiB RAM (24 max)
    PowerBook Duo 250:  68030 @ 33 MHz, 640x400 active-matrix 4bpp grayscale screen, 4 MiB RAM (24 max)
    PowerBook Duo 270c: 68030 @ 33 MHz, FPU, 640x480 active-matrix 16bpp color screen, 4 MiB RAM (32 max)
    PowerBook Duo 280:  68040 @ 33 MHz, 640x480 active-matrix 4bpp grayscale screen, 4 MiB RAM (40 max)
    PowerBook Duo 280c: 68040 @ 33 MHz, 640x480 active-matrix 16bpp color screen, 4 MiB RAM (40 max)

    Future:
    PowerBook 150: '030 @ 33 MHz, 640x480 grayscale screen, 4 MiB RAM (40 max), IDE HDD, ADB trackpad, PG&E matrix keyboard

    ============================================================================
    Technical info

    Pseudo-VIA2 Port B bits 1 and 2 are /PMU_ACK and /PMU_REQ, respectively.
    Main PMU comms are through the VIA shifter, but using a hardware SPI block
    on the PG&E end instead of the 68HC05 losing cycles doing bit-banging.

    Brightness: PLM 1 7F (all the way down) to 26 (all the way up)
                PLM 2 01  "   "   "   "     to 5A
                Total of the 2 PLM timers is always 0x80.  Timer 1 is off period, timer 2 is on period.

    PWM A0 - charging current control
    PWM B0 - screen contrast, 0x33 to 0xc5 range

    Temperature sensors read on a non-linear scale, probably a commercial part.
    Here are selected points from the lookup table the 68HC05 uses to convert it.

    Sensor val  Temperature (Celsius)
    ---------------------------------
    0 - 9:      invalid (high)
    10          115
    20          92
    30          78
    40          68
    50          60
    60          54
    70          49
    80          44
    90          39
    100         35
    110         31
    120         28
    130         24
    140         21
    150         17
    160         14
    170         10
    180         6
    190         2
    195 & 196   0
    200         -2
    210         -7
    220         -12
    230         -19
    240         -28
    250         -48
    252         -55
    253+        invalid (low)
****************************************************************************/

#include "emu.h"

#include "csc.h"
#include "dfac.h"
#include "gsc.h"
#include "macscsi.h"
#include "mactoolbox.h"
#include "msc.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "cpu/m6805/m68hc05pge.h"
#include "cpu/m68000/m68030.h"
#include "cpu/m68000/m68040.h"
#include "machine/ds2401.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/devices.h"

#include "softlist_dev.h"
#include "speaker.h"
#include "utf8.h"

namespace {
class macpbmsc_state : public driver_device
{
public:
	macpbmsc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pmu(*this, "pge"),
		m_msc(*this, "msc"),
		m_dfac(*this, "dfac"),
		m_ncr5380(*this, "scsi:7:ncr5380"),
		m_scsihelp(*this, "scsihelp"),
		m_ram(*this, RAM_TAG),
		m_gsc(*this, "gsc"),
		m_csc(*this, "csc"),
		m_scc(*this, "scc"),
		m_battserial(*this, "ds2400"),
		m_mouse0(*this, "MOUSE0"),
		m_mouse1(*this, "MOUSE1"),
		m_mouse2(*this, "MOUSE2"),
		m_keys(*this, "Y%u", 0),
		m_kbspecial(*this, "keyb_special"),
		m_ca1_data(0),
		m_cb1_data(0),
		m_pmu_blank_display(true),
		m_portc(0),
		m_last_porte(0xff),
		m_last_portf(0xff),
		m_last_portg(0xff),
		m_last_porth(0x00), // bit 0 must start as 0 for the PG&E bootrom to configure the DFAC
		m_last_portl(0xff),
		m_lastmousex(0), m_lastmousey(0), m_lastbutton(0),
		m_mouseX(0), m_mouseY(0),
		m_matrix_row(0)
	{
	}

	void macpd2xx_base_map(address_map &map) ATTR_COLD;
	void macpd210(machine_config &config);
	void macpd210_map(address_map &map) ATTR_COLD;
	void macpd230(machine_config &config);
	void macpd230_map(address_map &map) ATTR_COLD;
	void macpd250(machine_config &config);
	void macpd250_map(address_map &map) ATTR_COLD;
	void macpd270c(machine_config &config);
	void macpd270c_map(address_map &map) ATTR_COLD;
	void macpd280(machine_config &config);
	void macpd280c(machine_config &config);
	void macpd280_map(address_map &map) ATTR_COLD;

private:
	required_device<m68000_musashi_device> m_maincpu;
	required_device<m68hc05pge_device> m_pmu;
	required_device<msc_device> m_msc;
	required_device<dfac_device> m_dfac;
	required_device<ncr53c80_device> m_ncr5380;
	required_device<mac_scsi_helper_device> m_scsihelp;
	required_device<ram_device> m_ram;
	optional_device<gsc_device> m_gsc;
	optional_device<csc_device> m_csc;
	required_device<z80scc_device> m_scc;
	required_device<ds2401_device> m_battserial;
	required_ioport m_mouse0, m_mouse1, m_mouse2;
	required_ioport_array<8> m_keys;
	required_ioport m_kbspecial;
	int m_ca1_data;
	int m_cb1_data;

	bool m_pmu_blank_display;

	u8 m_portc, m_last_porte, m_last_portf, m_last_portg, m_last_porth, m_last_portl;

	s32 m_lastmousex, m_lastmousey, m_lastbutton;
	u8 m_mouseX, m_mouseY;
	u8 m_matrix_row;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void scc_irq_w(int state);
	void via_irq_w(int state);

	u16 scsi_r(offs_t offset, u16 mem_mask);
	void scsi_w(offs_t offset, u16 data, u16 mem_mask);
	u32 scsi_drq_r(offs_t offset, u32 mem_mask = ~0);
	void scsi_drq_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void scsi_berr_w(u8 data);
	u16 scc_r(offs_t offset);
	void scc_w(offs_t offset, u16 data);
	void vbl_w(int state);

	u8 pmu_porta_r();
	u8 pmu_portb_r();
	u8 pmu_portc_r();
	void pmu_portc_w(u8 data);
	u8 pmu_portd_r();
	u8 pmu_porte_r();
	void pmu_porte_w(u8 data);
	u8 pmu_portf_r();
	void pmu_portf_w(u8 data);
	u8 pmu_portg_r();
	void pmu_portg_w(u8 data);
	u8 pmu_porth_r();
	void pmu_porth_w(u8 data);
	void pmu_portj_w(u8 data);
	void pmu_portl_w(u8 data);
	u8 pmu_read_mouseX();
	u8 pmu_read_mouseY();
	int pmu_read_mouseButton();
	u8 pmu_bat_low();
	u8 pmu_bat_high();
	u8 pmu_bat_current();
	u8 pmu_bat_temp();
	u8 pmu_ambient_temp();
};

void macpbmsc_state::machine_start()
{
	m_msc->set_ram_info((u32 *)m_ram->pointer(), m_ram->size());

	m_ca1_data = 0;

	save_item(NAME(m_ca1_data));
	save_item(NAME(m_cb1_data));
	save_item(NAME(m_pmu_blank_display));
	save_item(NAME(m_portc));
	save_item(NAME(m_last_porte));
	save_item(NAME(m_last_portf));
	save_item(NAME(m_last_portg));
	save_item(NAME(m_last_porth));
	save_item(NAME(m_last_portl));
	save_item(NAME(m_lastmousex));
	save_item(NAME(m_lastmousey));
	save_item(NAME(m_lastbutton));
	save_item(NAME(m_mouseX));
	save_item(NAME(m_mouseY));
	save_item(NAME(m_matrix_row));
}

void macpbmsc_state::machine_reset()
{
	m_ca1_data = 0;
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

u16 macpbmsc_state::scc_r(offs_t offset)
{
	m_msc->via_sync();
	const u16 result = m_scc->dc_ab_r(offset);
	return (result << 8) | result;
}

void macpbmsc_state::scc_w(offs_t offset, u16 data)
{
	m_scc->dc_ab_w(offset, data >> 8);
}

void macpbmsc_state::vbl_w(int state)
{
	int MouseCountX = 0, MouseCountY = 0;
	int NewX, NewY;

	NewX = m_mouse1->read();
	NewY = m_mouse2->read();

	//  printf("pollmouse: X %d Y %d\n", NewX, NewY);

	/* see if it moved in the x coord */
	if (NewX != m_lastmousex)
	{
		int diff = (NewX - m_lastmousex);

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100 - diff;
		if (diff < -0x80)
			diff = -0x100 - diff;

		MouseCountX += diff;
		m_lastmousex = NewX;
	}

	/* see if it moved in the y coord */
	if (NewY != m_lastmousey)
	{
		int diff = (NewY - m_lastmousey);

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100 - diff;
		if (diff < -0x80)
			diff = -0x100 - diff;

		MouseCountY += diff;
		m_lastmousey = NewY;
	}

	m_lastbutton = m_mouse0->read() & 0x01;
	m_mouseX = MouseCountX;
	m_mouseY = MouseCountY;
//  printf("X %02x Y %02x\n", m_mouseX, m_mouseY);
}

u16 macpbmsc_state::scsi_r(offs_t offset, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 6) && (offset >= 0x130);

	return m_scsihelp->read_wrapper(pseudo_dma, reg) << 8;
}

void macpbmsc_state::scsi_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 0) && (offset >= 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data>>8);
}

u32 macpbmsc_state::scsi_drq_r(offs_t offset, u32 mem_mask)
{
	switch (mem_mask)
	{
		case 0xff000000:
			return m_scsihelp->read_wrapper(true, 6)<<24;

		case 0xffff0000:
			return (m_scsihelp->read_wrapper(true, 6)<<24) | (m_scsihelp->read_wrapper(true, 6)<<16);

		case 0xffffffff:
			return (m_scsihelp->read_wrapper(true, 6)<<24) | (m_scsihelp->read_wrapper(true, 6)<<16) | (m_scsihelp->read_wrapper(true, 6)<<8) | m_scsihelp->read_wrapper(true, 6);

		default:
			logerror("scsi_drq_r: unknown mem_mask %08x\n", mem_mask);
	}

	return 0;
}

void macpbmsc_state::scsi_drq_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (mem_mask)
	{
		case 0xff000000:
			m_scsihelp->write_wrapper(true, 0, data>>24);
			break;

		case 0xffff0000:
			m_scsihelp->write_wrapper(true, 0, data>>24);
			m_scsihelp->write_wrapper(true, 0, data>>16);
			break;

		case 0xffffffff:
			m_scsihelp->write_wrapper(true, 0, data>>24);
			m_scsihelp->write_wrapper(true, 0, data>>16);
			m_scsihelp->write_wrapper(true, 0, data>>8);
			m_scsihelp->write_wrapper(true, 0, data&0xff);
			break;

		default:
			logerror("scsi_drq_w: unknown mem_mask %08x\n", mem_mask);
			break;
	}
}

void macpbmsc_state::scsi_berr_w(u8 data)
{
	m_maincpu->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
}

u8 macpbmsc_state::pmu_porta_r()
{
	if (m_portc == 0)   // power key
	{
		return 0xdf | ((m_kbspecial->read() & 1) << 5);
	}

	// matrix X0-X7 (bits 0-7)
	return m_keys[m_matrix_row]->read() & 0xff;
}

u8 macpbmsc_state::pmu_portb_r()
{
	// matrix X8-X10 (bits 0-2), modifiers (bits 3-7)
	return (m_kbspecial->read() & 0xf8) | ((m_keys[m_matrix_row]->read() >> 8) & 7);
}

u8 macpbmsc_state::pmu_portc_r()
{
	return m_portc ^ 0xff;
}

void macpbmsc_state::pmu_portc_w(u8 data)
{
	m_portc = data ^ 0xff;

	// matrix row select
	m_matrix_row = 0;
	for (u8 i = 0; i < 8; i++)
	{
		if (BIT(m_portc, i))
		{
			m_matrix_row = i;
			return;
		}
	}
}

// bit 4 = 1 for US keyboard, 0 for ISO
// bit 5 = 1 for sound power off
// bit 6 = 1 for docking station NOT present
// bit 7 = 1 for second mouse button NOT pressed
u8 macpbmsc_state::pmu_portd_r()
{
	return (1 << 7) | (1 << 6) | (1 << 4);   // no docking station, US keyboard
}

// bit 1 = screen power on/off
// bit 2 = MSC /reset
// bit 7 = data line for 1-wire Dallas comms with the battery
u8 macpbmsc_state::pmu_porte_r()
{
	if (!machine().side_effects_disabled())
	{
		return (m_last_porte & 0x7f) | (m_battserial->read() << 7);
	}

	return m_last_porte;
}

void macpbmsc_state::pmu_porte_w(u8 data)
{
	if (BIT(data, 2) != BIT(m_last_porte, 2))
	{
		if (BIT(data, 2))
		{
			m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		}
		m_msc->pmu_reset_w(BIT(data, 2) ^ 1);
	}
	m_maincpu->set_input_line(INPUT_LINE_RESET, BIT(data, 2) ? CLEAR_LINE : ASSERT_LINE);
	if (BIT(data, 1) != BIT(m_last_porte, 1))
	{
		m_pmu_blank_display = BIT(data, 1);
		if (m_gsc)
		{
			m_gsc->set_pmu_blank(m_pmu_blank_display);
		}
		else if (m_gsc)
		{
			m_csc->set_pmu_blank(m_pmu_blank_display);
		}
	}
	if (BIT(data, 7) != BIT(m_last_porte, 7))
	{
		m_battserial->write(BIT(data, 7));
	}
	m_last_porte = data;
}

// bit 0 = Power (1 = off, 0 = on)
// bit 2 = 1 for +5V present when input, cause level 1 interrupt when output (VIA CB2?)
// bit 3 = clamshell open (1) or closed (0)
// bit 6 = /PMREQ
u8 macpbmsc_state::pmu_portf_r()
{
	u8 retval = (1 << 2);       // indicate +5V present
	retval |= (1 << 3);         // indicate clamshell open
	retval |= (m_msc->get_pmu_req() << 6);
	return retval;
}

u8 macpbmsc_state::pmu_bat_low()
{
	return 0xff;
}

u8 macpbmsc_state::pmu_bat_high()
{
	return 0x7f;
}

u8 macpbmsc_state::pmu_bat_current()
{
	return 0x40;
}

u8 macpbmsc_state::pmu_bat_temp()
{
	return 131; // ~24 degrees C
}

u8 macpbmsc_state::pmu_ambient_temp()
{
	return 131; // ~24 degrees C
}

void macpbmsc_state::pmu_portf_w(u8 data)
{
	if (!BIT(data, 2) && BIT(m_last_portf, 2))
	{
		m_msc->cb1_int_hack(ASSERT_LINE);
	}
	else if (BIT(data, 2) && !BIT(m_last_portf, 2))
	{
		m_msc->cb1_int_hack(CLEAR_LINE);
	}

	m_last_portf = data;
}

// bit 3 = 1 for docking station powered up
// bit 4 = caps lock LED
// bit 5 = sleep LED
// bit 6 = charger present (1 = present)
u8 macpbmsc_state::pmu_portg_r()
{
	return (1 << 6); // indicate we're on a charger
}

// bit 1 set turns on the main battery power
// bit 5 is sleep: 0 = normal operation, 1 = turn off 31.whatever MHz master clock
void macpbmsc_state::pmu_portg_w(u8 data)
{
	if (BIT(data, 5) != BIT(m_last_portg, 5))
	{
		if (!BIT(data, 5))
		{
			m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_msc->pmu_reset_w(ASSERT_LINE);
			m_msc->pmu_reset_w(CLEAR_LINE);
		}
	}

	m_last_portg = data;
}

// bit 0 = DFAC reset
// bit 1 = sleep LED (270c, maybe 280/280c also?)
// bit 2 = NMI
// bit 5 = DFAC latch
// bit 6 = /PMACK
u8 macpbmsc_state::pmu_porth_r()
{
	return m_last_porth;
}

void macpbmsc_state::pmu_porth_w(u8 data)
{
	m_dfac->latch_write(BIT(data, 5));
	m_msc->pmu_ack_w(BIT(data, 6));
	m_last_porth = data;
}

// bit 6 = DFAC clock
// bit 7 = DFAC data
void macpbmsc_state::pmu_portj_w(u8 data)
{
	m_dfac->clock_write(BIT(data, 6));
	m_dfac->data_write(BIT(data, 7));
}

// bit 1 = main power to the CPU (1 = off, 0 = on)
void macpbmsc_state::pmu_portl_w(u8 data)
{
	m_last_portl = data;
}

u8 macpbmsc_state::pmu_read_mouseX()
{
	return m_mouseX;
}

u8 macpbmsc_state::pmu_read_mouseY()
{
	return m_mouseY;
}

int macpbmsc_state::pmu_read_mouseButton()
{
	return m_lastbutton;
}

/***************************************************************************
    ADDRESS MAPS
****************************************************************************/

void macpbmsc_state::macpd2xx_base_map(address_map &map)
{
	map(0x40000000, 0x600fffff).m(m_msc, FUNC(msc_device::map));

	map(0x50f04000, 0x50f05fff).rw(FUNC(macpbmsc_state::scc_r), FUNC(macpbmsc_state::scc_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(macpbmsc_state::scsi_drq_r), FUNC(macpbmsc_state::scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(macpbmsc_state::scsi_r), FUNC(macpbmsc_state::scsi_w));
	map(0x50f12000, 0x50f13fff).rw(FUNC(macpbmsc_state::scsi_drq_r), FUNC(macpbmsc_state::scsi_drq_w));
}

void macpbmsc_state::macpd210_map(address_map &map)
{
	macpd2xx_base_map(map);

	map(0x50000000, 0x6fffffff).m(m_gsc, FUNC(gsc_device::map));

	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a1004; }));
}

void macpbmsc_state::macpd230_map(address_map &map)
{
	macpd210_map(map);

	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a1005; }));
}

void macpbmsc_state::macpd250_map(address_map &map)
{
	macpd210_map(map);

	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a1006; }));
}

void macpbmsc_state::macpd270c_map(address_map &map)
{
	macpd2xx_base_map(map);

	map(0x50000000, 0x6fffffff).m(m_csc, FUNC(csc_device::map));

	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a1002; }));
}

void macpbmsc_state::macpd280_map(address_map &map)
{
	macpd270c_map(map);

	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a1000; }));
}

static INPUT_PORTS_START( dblite )
	PORT_START("MOUSE0") /* Mouse - button */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START("MOUSE1") /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSE2") /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("Y0")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("Y1")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN) PORT_CHAR(10)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("Y2")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")             PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Brightness Up")   PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Brightness Down") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("Y3")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Contrast Down") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)       PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Contrast Up")   PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)       PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)       PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)   PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("Y4")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)      PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)      PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)      PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)      PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)      PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)  PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("Y5")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)      PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)      PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)      PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)      PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)      PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("Y6")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('#') PORT_CHAR(U'^') // (actually to the left of the return key on the ASDF row)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("Y7")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)      PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)      PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)      PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)      PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Power")         PORT_CODE(KEYCODE_F12)

	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Command")       PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control")       PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift")         PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Option")        PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void macpbmsc_state::macpd210(machine_config &config)
{
	M68030(config, m_maincpu, 25_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpbmsc_state::macpd210_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");
	m_maincpu->set_fpu_enable(false);

	M68HC05PGE(config, m_pmu, 4.194304_MHz_XTAL);
	m_pmu->read_p<m68hc05pge_device::PGE_PORTA>().set(FUNC(macpbmsc_state::pmu_porta_r));
	m_pmu->read_p<m68hc05pge_device::PGE_PORTB>().set(FUNC(macpbmsc_state::pmu_portb_r));
	m_pmu->read_p<m68hc05pge_device::PGE_PORTC>().set(FUNC(macpbmsc_state::pmu_portc_r));
	m_pmu->write_p<m68hc05pge_device::PGE_PORTC>().set(FUNC(macpbmsc_state::pmu_portc_w));
	m_pmu->read_p<m68hc05pge_device::PGE_PORTD>().set(FUNC(macpbmsc_state::pmu_portd_r));
	m_pmu->read_p<m68hc05pge_device::PGE_PORTE>().set(FUNC(macpbmsc_state::pmu_porte_r));
	m_pmu->write_p<m68hc05pge_device::PGE_PORTE>().set(FUNC(macpbmsc_state::pmu_porte_w));
	m_pmu->read_p<m68hc05pge_device::PGE_PORTF>().set(FUNC(macpbmsc_state::pmu_portf_r));
	m_pmu->write_p<m68hc05pge_device::PGE_PORTF>().set(FUNC(macpbmsc_state::pmu_portf_w));
	m_pmu->read_p<m68hc05pge_device::PGE_PORTG>().set(FUNC(macpbmsc_state::pmu_portg_r));
	m_pmu->write_p<m68hc05pge_device::PGE_PORTG>().set(FUNC(macpbmsc_state::pmu_portg_w));
	m_pmu->read_p<m68hc05pge_device::PGE_PORTH>().set(FUNC(macpbmsc_state::pmu_porth_r));
	m_pmu->write_p<m68hc05pge_device::PGE_PORTH>().set(FUNC(macpbmsc_state::pmu_porth_w));
	m_pmu->write_p<m68hc05pge_device::PGE_PORTJ>().set(FUNC(macpbmsc_state::pmu_portj_w));
	m_pmu->write_p<m68hc05pge_device::PGE_PORTL>().set(FUNC(macpbmsc_state::pmu_portl_w));
	m_pmu->set_pullups<m68hc05pge_device::PGE_PORTC>(0xff);
	m_pmu->set_pullups<m68hc05pge_device::PGE_PORTE>(0x80);     // bit 7 of port E is the 1-Wire bus
	m_pmu->ad_in<0>().set(FUNC(macpbmsc_state::pmu_bat_low));
	m_pmu->ad_in<1>().set(FUNC(macpbmsc_state::pmu_bat_high));
	m_pmu->ad_in<2>().set(FUNC(macpbmsc_state::pmu_bat_current));
	m_pmu->ad_in<3>().set(FUNC(macpbmsc_state::pmu_bat_temp));
	m_pmu->ad_in<4>().set(FUNC(macpbmsc_state::pmu_ambient_temp));
	m_pmu->spi_clock_callback().set(m_msc, FUNC(msc_device::cb1_w));
	m_pmu->spi_mosi_callback().set(m_msc, FUNC(msc_device::cb2_w));
	m_pmu->read_tbB().set(FUNC(macpbmsc_state::pmu_read_mouseButton));
	m_pmu->read_tbX().set(FUNC(macpbmsc_state::pmu_read_mouseX));
	m_pmu->read_tbY().set(FUNC(macpbmsc_state::pmu_read_mouseY));

	MSC(config, m_msc, 31.3344_MHz_XTAL);
	m_msc->set_maincpu_tag("maincpu");
	m_msc->set_pmu_tag("pge");
	m_msc->set_rom_tag("bootrom");
	m_msc->set_cpu_clock(25_MHz_XTAL);
	m_msc->add_route(0, m_dfac, 1.0);
	m_msc->add_route(1, m_dfac, 1.0);
	m_msc->cb2_callback().set(m_pmu, FUNC(m68hc05pge_device::spi_miso_w));
	m_msc->vbl_callback().set(FUNC(macpbmsc_state::vbl_w));

	APPLE_DFAC(config, m_dfac, 22257);
	m_dfac->add_route(0, "lspeaker", 1.0);
	m_dfac->add_route(1, "rspeaker", 1.0);

	GSC(config, m_gsc, 31.3344_MHz_XTAL);
	m_gsc->set_panel_id(6);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^lspeaker", 1.0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^rspeaker", 1.0);
		});
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR53C80).machine_config([this](device_t *device) {
		ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
		adapter.irq_handler().set(m_msc, FUNC(msc_device::scsi_irq_w));
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w));
	});

	MAC_SCSI_HELPER(config, m_scsihelp);
	m_scsihelp->scsi_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::read));
	m_scsihelp->scsi_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::write));
	m_scsihelp->scsi_dma_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_r));
	m_scsihelp->scsi_dma_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_w));
	m_scsihelp->cpu_halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_scsihelp->timeout_error_callback().set(FUNC(macpbmsc_state::scsi_berr_w));

	SCC85C30(config, m_scc, 31.3344_MHz_XTAL / 4);
	m_scc->out_int_callback().set(m_msc, FUNC(msc_device::scc_irq_w));

	DS2401(config, m_battserial, 0); // actually DS2400, but 2400/2401 are compatible

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,12M,16M,24M");

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68030,MC68030_32");
}

void macpbmsc_state::macpd230(machine_config &config)
{
	macpd210(config);
	m_maincpu->set_clock(33_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpbmsc_state::macpd230_map);
	m_msc->set_cpu_clock(33_MHz_XTAL);
}

void macpbmsc_state::macpd250(machine_config &config)
{
	macpd230(config);
	m_maincpu->set_fpu_enable(true);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpbmsc_state::macpd250_map);
}

void macpbmsc_state::macpd270c(machine_config &config)
{
	macpd230(config);
	config.device_remove("gsc");

	CSC(config, m_csc, 31.3344_MHz_XTAL);
	m_csc->write_irq().set(m_msc, FUNC(msc_device::lcd_irq_w));
	m_csc->set_panel_id(0);

	m_maincpu->set_fpu_enable(true);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpbmsc_state::macpd270c_map);
}

void macpbmsc_state::macpd280(machine_config &config)
{
	macpd270c(config);

	m_csc->set_panel_id(4);

	M68040(config.replace(), m_maincpu, 33_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpbmsc_state::macpd280_map);
}

void macpbmsc_state::macpd280c(machine_config &config)
{
	macpd280(config);

	m_csc->set_panel_id(0);
}

ROM_START(macpd210)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("ecfa989b.rom", 0x000000, 0x100000, CRC(b86ed854) SHA1(ed1371c97117a5884da4a6605ecfc5abed48ae5a))

	// battery serial number, read from an embedded Dallas DS2400
	ROM_REGION(0x8, "ds2400", ROMREGION_ERASE00)
	ROM_LOAD( "duobatid.bin", 0x000000, 0x000008, CRC(7545c341) SHA1(61b094ee5b398077f70eaa1887921c8366f7abfe) )
ROM_END

ROM_START(macpd270c)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "0024d346.rom", 0x000000, 0x100000, CRC(94c4d04a) SHA1(be7bd9637203e4513b896146ddfc85c37817d131) )

	ROM_REGION(0x8, "ds2400", ROMREGION_ERASE00)
	ROM_LOAD( "duobatid.bin", 0x000000, 0x000008, CRC(7545c341) SHA1(61b094ee5b398077f70eaa1887921c8366f7abfe) )
ROM_END

ROM_START(macpd280)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("015621d7.rom", 0x000000, 0x100000, CRC(568d28eb) SHA1(d49dd69cf038784b8849793ad3c0e62c2d11f653))

	ROM_REGION(0x8, "ds2400", ROMREGION_ERASE00)
	ROM_LOAD( "duobatid.bin", 0x000000, 0x000008, CRC(7545c341) SHA1(61b094ee5b398077f70eaa1887921c8366f7abfe) )
ROM_END

#define rom_macpd230 rom_macpd210
#define rom_macpd250 rom_macpd210
#define rom_macpd280c rom_macpd280

} // anonymous namespace

COMP(1992, macpd210, 0, 0, macpd210, dblite, macpbmsc_state, empty_init, "Apple Computer", "Macintosh PowerBook Duo 210", MACHINE_SUPPORTS_SAVE)
COMP(1992, macpd230, macpd210, 0, macpd230, dblite, macpbmsc_state, empty_init, "Apple Computer", "Macintosh PowerBook Duo 230", MACHINE_SUPPORTS_SAVE)
COMP(1993, macpd250, macpd210, 0, macpd250, dblite, macpbmsc_state, empty_init, "Apple Computer", "Macintosh PowerBook Duo 250", MACHINE_SUPPORTS_SAVE)
COMP(1993, macpd270c, 0, 0, macpd270c, dblite, macpbmsc_state, empty_init, "Apple Computer", "Macintosh PowerBook Duo 270c", MACHINE_SUPPORTS_SAVE)
COMP(1994, macpd280, 0, 0, macpd280, dblite, macpbmsc_state, empty_init, "Apple Computer", "Macintosh PowerBook Duo 280", MACHINE_SUPPORTS_SAVE)
COMP(1994, macpd280c, macpd280, 0, macpd280c, dblite, macpbmsc_state, empty_init, "Apple Computer", "Macintosh PowerBook Duo 280c", MACHINE_SUPPORTS_SAVE)
