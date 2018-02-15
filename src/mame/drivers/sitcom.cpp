// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    SITCOM (known as Sitcom, Sitcom85, Sitcom8085)

    25/09/2011 Driver [Robbbert]

    http://www.sitcom.tk/
    http://www.sbprojects.net/projects/izabella/html/sitcom_.html

    The display consists of a LED connected to SOD, and a pair of DL1414
    intelligent alphanumeric displays.

    The idea of this device is that you write a 8085 program with an
    assembler on your PC.  You then compile it, and then send it to the
    SITCOM via a serial cable. The program then (hopefully) runs on the
    SITCOM.  With the 8255 expansion, you could wire up input devices or
    other hardware for your program to use.

    The SOD LED blinks slowly while waiting; stays on while downloading;
    and blinks quickly if an error occurs.

    After a successful upload, hit the Reset button to mirror RAM into
    the low 32kB of the address space in place of ROM and run the
    program.

    The null modem bitbanger should be configured for 9600 8N1 to upload
    a program.  The data should be in Intel HEX format.

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"

#include "cpu/i8085/i8085.h"

#include "machine/clock.h"
#include "machine/bankdev.h"
#include "machine/i8255.h"

#include "video/dl1416.h"

#include "softlist_dev.h"

#include "sitcom.lh"
#include "sitcomtmr.lh"

#include <cmath>


namespace {

class sitcom_state : public driver_device
{
public:
	sitcom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_buttons(*this, "BUTTONS")
		, m_maincpu(*this, "maincpu")
		, m_bank(*this, "bank")
		, m_rxd(true)
	{
	}

	template <unsigned D> DECLARE_WRITE16_MEMBER(update_ds) { output().set_digit_value((D << 2) | offset, data); }
	DECLARE_WRITE_LINE_MEMBER(update_rxd)                   { m_rxd = bool(state); }
	DECLARE_WRITE_LINE_MEMBER(sod_led)                      { output().set_value("sod_led", state); }
	DECLARE_READ_LINE_MEMBER(sid_line)                      { return m_rxd ? 1 : 0; }

	virtual DECLARE_WRITE8_MEMBER(update_pia_pa);
	virtual DECLARE_WRITE8_MEMBER(update_pia_pb);

	DECLARE_INPUT_CHANGED_MEMBER(update_buttons);

	void sitcom(machine_config &config);
	void sitcom_bank(address_map &map);
	void sitcom_io(address_map &map);
	void sitcom_mem(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_ioport                          m_buttons;
	required_device<cpu_device>              m_maincpu;
	required_device<address_map_bank_device> m_bank;

	bool m_rxd;
};


class sitcom_timer_state : public sitcom_state
{
public:
	enum
	{
		TIMER_SHUTTER
	};

	sitcom_timer_state(const machine_config &mconfig, device_type type, const char *tag)
		: sitcom_state(mconfig, type, tag)
		, m_speed(*this, "SPEED")
		, m_pia(*this, "pia")
		, m_ds2(*this, "ds2")
		, m_shutter_timer(nullptr)
		, m_shutter(false)
		, m_dac_cs(true)
		, m_dac_wr(true)
	{
	}

	virtual DECLARE_WRITE8_MEMBER(update_pia_pa) override;
	virtual DECLARE_WRITE8_MEMBER(update_pia_pb) override;
	DECLARE_READ_LINE_MEMBER(shutter_r);

	DECLARE_INPUT_CHANGED_MEMBER(update_shutter);
	DECLARE_INPUT_CHANGED_MEMBER(update_speed);

	void sitcomtmr(machine_config &config);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void update_dac(uint8_t value);

	required_ioport                 m_speed;
	required_device<i8255_device>   m_pia;
	required_device<dl1414_device>  m_ds2;
	emu_timer                       *m_shutter_timer;

	bool                            m_shutter;
	bool                            m_dac_cs, m_dac_wr;
};


ADDRESS_MAP_START(sitcom_state::sitcom_bank)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("bootstrap", 0)
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

ADDRESS_MAP_START(sitcom_state::sitcom_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_DEVICE("bank", address_map_bank_device, amap8)
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

ADDRESS_MAP_START(sitcom_state::sitcom_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x1c) AM_DEVREADWRITE("pia", i8255_device, read, write)
	AM_RANGE(0xc0, 0xc3) AM_MIRROR(0x1c) AM_DEVWRITE("ds0", dl1414_device, bus_w)
	AM_RANGE(0xe0, 0xe3) AM_MIRROR(0x1c) AM_DEVWRITE("ds1", dl1414_device, bus_w)
ADDRESS_MAP_END


INPUT_PORTS_START( sitcom )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Boot")  PORT_CHANGED_MEMBER(DEVICE_SELF, sitcom_state, update_buttons, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, sitcom_state, update_buttons, 0)

	PORT_START("PORTC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("7") PORT_CODE(KEYCODE_7)
INPUT_PORTS_END

INPUT_PORTS_START( sitcomtmr )
	PORT_INCLUDE(sitcom)

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Shutter") PORT_CHANGED_MEMBER(DEVICE_SELF, sitcom_timer_state, update_shutter, 0)

	PORT_MODIFY("PORTC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Grey")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Blue")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, sitcom_timer_state, shutter_r)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPEED")
	PORT_CONFNAME(0xff, 0x1e, "Shutter Speed") PORT_CHANGED_MEMBER(DEVICE_SELF, sitcom_timer_state, update_speed, 0)
	PORT_CONFSETTING(0x00, "B")
	PORT_CONFSETTING(0x01, "1")
	PORT_CONFSETTING(0x02, "1/2")
	PORT_CONFSETTING(0x04, "1/4")
	PORT_CONFSETTING(0x08, "1/8")
	PORT_CONFSETTING(0x0f, "1/15")
	PORT_CONFSETTING(0x1e, "1/30")
	PORT_CONFSETTING(0x3c, "1/60")
	PORT_CONFSETTING(0x7d, "1/125")
INPUT_PORTS_END


void sitcom_state::machine_start()
{
	save_item(NAME(m_rxd));

	m_rxd = true;
}

void sitcom_state::machine_reset()
{
	m_bank->set_bank(0);
}

WRITE8_MEMBER( sitcom_state::update_pia_pa )
{
	for (int i = 0; 8 > i; ++i)
		output().set_indexed_value("pa", i, BIT(data, i));
}

WRITE8_MEMBER( sitcom_state::update_pia_pb )
{
	for (int i = 0; 8 > i; ++i)
		output().set_indexed_value("pb", i, BIT(data, i));
}

INPUT_CHANGED_MEMBER( sitcom_state::update_buttons )
{
	bool const boot(BIT(m_buttons->read(), 0));
	bool const reset(BIT(m_buttons->read(), 1));

	m_maincpu->set_input_line(INPUT_LINE_RESET, (boot || reset) ? ASSERT_LINE : CLEAR_LINE);

	if (boot)
		m_bank->set_bank(0);
	else if (reset)
		m_bank->set_bank(1);
}


WRITE8_MEMBER( sitcom_timer_state::update_pia_pa )
{
	if (!m_dac_cs && !m_dac_wr)
		update_dac(data);

	m_ds2->data_w(data & 0x7f);
}

WRITE8_MEMBER( sitcom_timer_state::update_pia_pb )
{
	if (!m_dac_cs && !BIT(data, 0))
		update_dac(m_pia->pa_r());
	m_dac_wr = BIT(data, 0);
	m_dac_cs = BIT(data, 1);

	m_ds2->wr_w(BIT(data, 2));
	m_ds2->addr_w(bitswap<2>(data, 3, 4));
	output().set_value("test_led", BIT(data, 5));
}

READ_LINE_MEMBER( sitcom_timer_state::shutter_r )
{
	return m_shutter ? 0 : 1;
}

INPUT_CHANGED_MEMBER( sitcom_timer_state::update_shutter )
{
	ioport_value const speed(m_speed->read());
	if (!speed)
	{
		m_shutter = bool(newval);
	}
	else if (!m_shutter && newval)
	{
		m_shutter = true;
		m_shutter_timer->adjust(attotime::from_hz(speed));
	}
}

INPUT_CHANGED_MEMBER( sitcom_timer_state::update_speed )
{
	if (!newval)
	{
		m_shutter = bool(BIT(m_buttons->read(), 2));
	}
	else if (!oldval)
	{
		m_shutter = false;
	}
}

void sitcom_timer_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_SHUTTER:
		m_shutter = false;
		break;
	default:
		sitcom_state::device_timer(timer, id, param, ptr);
	}
}

void sitcom_timer_state::machine_start()
{
	sitcom_state::machine_start();

	m_shutter_timer = timer_alloc(TIMER_SHUTTER);

	save_item(NAME(m_shutter));
	save_item(NAME(m_dac_cs));
	save_item(NAME(m_dac_wr));

	m_shutter = false;
	m_dac_cs = true;
	m_dac_wr = true;
}

void sitcom_timer_state::machine_reset()
{
	sitcom_state::machine_reset();

	m_ds2->ce_w(0);
}

void sitcom_timer_state::update_dac(uint8_t value)
{
	// supposed to be a DAC and an analog meter, but that's hard to do with internal layouts
	constexpr u8 s_7seg[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
	output().set_digit_value(12, s_7seg[value % 10]);
	value /= 10;
	output().set_digit_value(13, s_7seg[value % 10]);
	value /= 10;
	output().set_digit_value(14, s_7seg[value % 10] | 0x80);
}


MACHINE_CONFIG_START(sitcom_state::sitcom)
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I8085A, 6.144_MHz_XTAL) // 3.072MHz can be used for an old slow 8085
	MCFG_CPU_PROGRAM_MAP(sitcom_mem)
	MCFG_CPU_IO_MAP(sitcom_io)
	MCFG_I8085A_SID(READLINE(sitcom_state, sid_line))
	MCFG_I8085A_SOD(WRITELINE(sitcom_state, sod_led))

	MCFG_DEVICE_ADD("bank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(sitcom_bank)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x8000)

	MCFG_CLOCK_ADD("100hz", 100)
	MCFG_CLOCK_SIGNAL_HANDLER(INPUTLINE("maincpu", I8085_RST75_LINE))

	MCFG_DEVICE_ADD("pia", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(sitcom_state, update_pia_pa))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(sitcom_state, update_pia_pb))
	MCFG_I8255_IN_PORTC_CB(IOPORT("PORTC"))

	// video hardware
	MCFG_DEVICE_ADD("ds0", DL1414T, 0) // left display
	MCFG_DL1414_UPDATE_HANDLER(WRITE16(sitcom_state, update_ds<0>))
	MCFG_DEVICE_ADD("ds1", DL1414T, 0) // right display
	MCFG_DL1414_UPDATE_HANDLER(WRITE16(sitcom_state, update_ds<1>))

	// host interface
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "null_modem")
	MCFG_RS232_RXD_HANDLER(WRITELINE(sitcom_state, update_rxd))

	MCFG_SOFTWARE_LIST_ADD("bitb_list", "sitcom")
	MCFG_DEFAULT_LAYOUT(layout_sitcom)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(sitcom_timer_state::sitcomtmr)
	sitcom(config);
	MCFG_DEVICE_ADD("ds2", DL1414T, 0) // remote display
	MCFG_DL1414_UPDATE_HANDLER(WRITE16(sitcom_state, update_ds<2>))

	MCFG_DEFAULT_LAYOUT(layout_sitcomtmr)
MACHINE_CONFIG_END


ROM_START( sitcom )
	ROM_REGION( 0x8000, "bootstrap", ROMREGION_ERASEFF )
	ROM_LOAD( "boot8085.bin", 0x0000, 0x06b8, CRC(1b5e3310) SHA1(3323b65f0c10b7ab6bb75ec824e6d5fb643693a8) )
ROM_END

ROM_START( sitcomtmr )
	ROM_REGION( 0x8000, "bootstrap", ROMREGION_ERASEFF )
	ROM_LOAD( "boot8085.bin", 0x0000, 0x06b8, CRC(1b5e3310) SHA1(3323b65f0c10b7ab6bb75ec824e6d5fb643693a8) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME       PARENT  COMPAT  MACHINE     INPUT      STATE               INIT  COMPANY                            FULLNAME        FLAGS */
COMP( 2002, sitcom,    0,      0,      sitcom,     sitcom,    sitcom_state,       0,    "San Bergmans & Izabella Malcolm", "SITCOM",       MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW)
COMP( 2002, sitcomtmr, sitcom, 0,      sitcomtmr,  sitcomtmr, sitcom_timer_state, 0,    "San Bergmans & Izabella Malcolm", "SITCOM Timer", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW)
