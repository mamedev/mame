// license:BSD-3-Clause
// copyright-holders: tim lindner
/***************************************************************************

    Elanco AgVision and Radio Shack VideoTex terminals

    Dynamic RAM (16 or 4k) starts at $0000.
    ROM (2K) starts at $A000 and mirrors up to $BFFF
    Static RAM starts at $C000 for 128 bytes
    The PIA data port a is at $FF1C
         control port a is at $FF1D
            data port b is at $FF1E
         control port b is at $FF1F

    The Control Register (flip flop) is at $FF20
    The SAM starts at $FFC0

    PIA:
        Port A and B make up the keyboard matrix
        PA7  - RS-232 receive.
        CA1  - The horizontal sync from the VDG.
        CA2  - Unconnected, not verified.
        CB1  - Modem status, carrier detect.
        CB2  - Modem control (unverified).
        IRQA - Connected to the 6809 FIRQ.
        IRQB - Unconnected, not verified.

    Control Register:
        Bit 7 - RS-232 transmit.
        Bit 6 - RS-232 DTR, or activity LED (unsure which).
        Bit 5 - unconnected (not verified).
        Bit 3 - VDG GM0.
        Bit 2 - VDG GM1.
        Bit 1 - VDG GM2.
        Bit 0 - VDG A* / G.

    Reverse engineering used with permission of George Phillips of the
    trs80gp emulator.

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/6883sam.h"
#include "machine/ram.h"
#include "video/mc6847.h"
#include "screen.h"

//-------------------------------------------------
//  TYPE DEFINITIONS
//-------------------------------------------------

namespace
{

class agvision_state : public driver_device
{
public:
	agvision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_ram(*this, "ram")
		, m_pia_0(*this, "pia0")
		, m_sam(*this, "sam")
		, m_vdg(*this, "vdg")
		, m_rs232(*this, "rs232")
		, m_keyboard(*this, "row%u", 0)
	{}

	void agvision(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(hang_up);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	static constexpr int CD_DELAY = 250;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ram_device> m_ram;
	required_device<pia6821_device> m_pia_0;
	required_device<sam6883_device> m_sam;
	required_device<mc6847_base_device> m_vdg;
	required_device<rs232_port_device> m_rs232;
	required_ioport_array<7> m_keyboard;

	emu_timer *m_timer; // Carrier Detect Timer
	uint8_t sam_read(offs_t offset);
	void mem_map(address_map &map) ATTR_COLD;
	void rom_map(address_map &map) ATTR_COLD;
	void static_ram_map(address_map &map) ATTR_COLD;
	void io0_map(address_map &map) ATTR_COLD;
	void io1_map(address_map &map) ATTR_COLD;
	void boot_map(address_map &map) ATTR_COLD;
	void ff20_write(offs_t offset, uint8_t data);
	void configure_sam();
	uint8_t pia0_pa_r();
	void pia0_cb2_w(int state);
	TIMER_CALLBACK_MEMBER(timer_elapsed);

	int m_cd;
};

//-------------------------------------------------
//  INPUT_PORTS( agvision )
//-------------------------------------------------

static INPUT_PORTS_START( agvision )
	PORT_START("row0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN), 10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 8) PORT_CHAR('[')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 9) PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12, UCHAR_SHIFT_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x78, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("hup")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HUP") PORT_CODE(KEYCODE_TAB) PORT_CHANGED_MEMBER(DEVICE_SELF, agvision_state, agvision_state::hang_up, 0)
INPUT_PORTS_END

//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS_START( ag_modem )
//-------------------------------------------------

static DEVICE_INPUT_DEFAULTS_START( ag_modem )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

//-------------------------------------------------
//  machine_config
//-------------------------------------------------

void agvision_state::agvision(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, XTAL(14'318'181) / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &agvision_state::mem_map);

	PIA6821(config, m_pia_0);
	m_pia_0->readpa_handler().set(FUNC(agvision_state::pia0_pa_r));
	m_pia_0->irqa_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	m_pia_0->cb2_handler().set(FUNC(agvision_state::pia0_cb2_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	MC6847_NTSC(config, m_vdg, XTAL(14'318'181) / 4); // VClk output from MC6883
	m_vdg->set_screen(m_screen);
	m_vdg->hsync_wr_callback().set(m_pia_0, FUNC(pia6821_device::ca1_w));
	m_vdg->input_callback().set(FUNC(agvision_state::sam_read));

	// memory controller
	SAM6883(config, m_sam, XTAL(14'318'181), m_maincpu);
	m_sam->set_addrmap(2, &agvision_state::rom_map);            // ROM at $A000
	m_sam->set_addrmap(3, &agvision_state::static_ram_map);     // RAM at $C000
	m_sam->set_addrmap(4, &agvision_state::io0_map);            //  IO at $FF00
	m_sam->set_addrmap(5, &agvision_state::io1_map);            //  IO at $FF20
	m_sam->set_addrmap(7, &agvision_state::boot_map);           //  IO at $FF60

	RS232_PORT(config, m_rs232, default_rs232_devices, "null_modem");
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(ag_modem));

	// internal ram
	RAM(config, m_ram).set_default_size("16K").set_extra_options("4K");
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void agvision_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(m_sam, FUNC(sam6883_device::read), FUNC(sam6883_device::write));
}

void agvision_state::rom_map(address_map &map)
{
	// $A000-$BFFF
	map(0x0000, 0x07ff).rom().region("maincpu", 0x0000).nopw().mirror(0x1800);
}

void agvision_state::static_ram_map(address_map &map)
{
	// $C000-$FEFF
	map(0x0000, 0x0080).ram();
}

void agvision_state::io0_map(address_map &map)
{
	// $FF00-$FF1F
	map(0x1c, 0x1f).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}

void agvision_state::io1_map(address_map &map)
{
	// $FF20-$FF3F
	map(0x00, 0x00).w(FUNC(agvision_state::ff20_write));
}

void agvision_state::boot_map(address_map &map)
{
	// $FF60-$FFEF
	map(0x60, 0x7f).nopw(); // SAM Registers
}

//-------------------------------------------------
//  device_start
//-------------------------------------------------

void agvision_state::machine_start()
{
	configure_sam();

	save_item(NAME(m_cd));
	m_pia_0->cb1_w(0);
	m_timer = timer_alloc(FUNC(agvision_state::timer_elapsed), this);
	m_cd = 0;
}

//-------------------------------------------------
//  sam_read
//-------------------------------------------------

uint8_t agvision_state::sam_read(offs_t offset)
{
	uint8_t data = m_sam->display_read(offset);
	m_vdg->as_w(BIT(data,7));
	m_vdg->inv_w(BIT(data,6));
	return data;
}

//-------------------------------------------------
//  ff20_write
//-------------------------------------------------

void agvision_state::ff20_write(offs_t offset, uint8_t data)
{
	m_rs232->write_txd(BIT(data,7));
	m_vdg->gm0_w(BIT(data,3));
	m_vdg->gm1_w(BIT(data,2));
	m_vdg->gm2_w(BIT(data,1));
	m_vdg->ag_w(BIT(data,0));
}

//-------------------------------------------------
//  configure_sam
//-------------------------------------------------

void agvision_state::configure_sam()
{
	offs_t ramsize = m_ram->size();
	m_sam->space(0).install_ram(0, ramsize - 1, m_ram->pointer());
	if (ramsize < 65536)
		m_sam->space(0).nop_readwrite(ramsize, 0xffff);
}

//-------------------------------------------------
//  timer_elapsed
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(agvision_state::timer_elapsed)
{
	m_timer->adjust(attotime::from_usec(CD_DELAY));
	m_pia_0->cb1_w(m_cd);
	m_cd = !m_cd;
}

//-------------------------------------------------
//  pia0_pa_r - keyboard handler
//-------------------------------------------------

uint8_t agvision_state::pia0_pa_r()
{
	uint8_t pia0_pb = m_pia_0->b_output();

	uint8_t pia0_pa = 0x7f;

	/* poll the keyboard, and update PA6-PA0 accordingly*/
	for (unsigned i = 0; i < m_keyboard.size(); i++)
	{
		int value = m_keyboard[i]->read();
		if ((value | pia0_pb) != 0xff)
		{
			pia0_pa &= ~(0x01 << i);
		}
	}

	pia0_pa |= (m_rs232->rxd_r() ? 0x80 : 0);

	return pia0_pa;
}

//-------------------------------------------------
//  hang_up
//-------------------------------------------------

INPUT_CHANGED_MEMBER(agvision_state::hang_up)
{
	if (newval == 1)
	{
		m_timer->adjust(attotime::never);
	}
}

//-------------------------------------------------
//  pia0_cb2_w
//-------------------------------------------------

void agvision_state::pia0_cb2_w(int state)
{
	if (state == 1)
	{
		m_timer->adjust(attotime::from_usec(CD_DELAY*8000));
	}
	else
	{
		m_timer->adjust(attotime::never);
	}
}

//**************************************************************************
//  ROMS
//**************************************************************************

ROM_START(agvision)
	ROM_REGION(0xa000,"maincpu",0)
	ROM_LOAD("8041716-1.1-agvision.u13", 0x0000, 0x0800, CRC(5b11b13e) SHA1(896e68f90ed57e0921887e717fc92eda067a210d))
ROM_END

ROM_START(trsvidtx)
	ROM_REGION(0xa000,"maincpu",0)
	ROM_LOAD("8041716-1.1-videotex.u13", 0x0000, 0x0800, CRC(821a59bb) SHA1(e3643f27fcf8287c0bc0f66b21554dc988ded9c1))
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT COMPAT MACHINE   INPUT     CLASS           INIT        COMPANY              FULLNAME    FLAGS
COMP( 1979, agvision, 0,     0,     agvision, agvision, agvision_state, empty_init, "Elanco",            "AgVision", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, trsvidtx, 0,     0,     agvision, agvision, agvision_state, empty_init, "Tandy Radio Shack", "Videotex", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
