// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Iskra-1030 and -1031 XX-key keyboard emulation

*********************************************************************/

#include "emu.h"
#include "iskr1030.h"


//#define LOG_GENERAL (1U <<  0) //defined in logmacro.h already
#define LOG_KEYBOARD  (1U <<  1)
#define LOG_DEBUG     (1U <<  2)

//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGKBD(...) LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "i8048"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PC_KBD_ISKR_1030, iskr_1030_keyboard_device, "kb_iskr1030", "Iskra-1030 Keyboard")


//-------------------------------------------------
//  ROM( iskr_1030_keyboard )
//-------------------------------------------------

ROM_START( iskr_1030_keyboard )
	ROM_REGION( 0x800, I8048_TAG, 0 )
	ROM_LOAD( "i1030.bin", 0x000, 0x800, CRC(7cac9c4b) SHA1(03959d3350e012ebfe61cee9c062b6c1fdd8766e) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *iskr_1030_keyboard_device::device_rom_region() const
{
	return ROM_NAME( iskr_1030_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------

void iskr_1030_keyboard_device::iskr_1030_keyboard_io(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(iskr_1030_keyboard_device::ram_r), FUNC(iskr_1030_keyboard_device::ram_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void iskr_1030_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_maincpu, XTAL(5'000'000));
	m_maincpu->set_addrmap(AS_IO, &iskr_1030_keyboard_device::iskr_1030_keyboard_io);
	m_maincpu->p1_in_cb().set(FUNC(iskr_1030_keyboard_device::p1_r));
	m_maincpu->p1_out_cb().set(FUNC(iskr_1030_keyboard_device::p1_w));
	m_maincpu->p2_out_cb().set(FUNC(iskr_1030_keyboard_device::p2_w));
	m_maincpu->t1_in_cb().set(FUNC(iskr_1030_keyboard_device::t1_r));
}


//-------------------------------------------------
//  INPUT_PORTS( iskr_1030_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( iskr_1030_keyboard )
	PORT_START("MD00")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD01")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD02")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD03")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD04")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD05")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD06")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD07")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD08")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD09")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD16")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD17")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD18")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD19")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD20")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD21")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD22")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("MD23")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor iskr_1030_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( iskr_1030_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iskr_1030_keyboard_device - constructor
//-------------------------------------------------

iskr_1030_keyboard_device::iskr_1030_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC_KBD_ISKR_1030, tag, owner, clock),
		device_pc_kbd_interface(mconfig, *this),
		m_maincpu(*this, I8048_TAG),
		m_md00(*this, "MD00"),
		m_md01(*this, "MD01"),
		m_md02(*this, "MD02"),
		m_md03(*this, "MD03"),
		m_md04(*this, "MD04"),
		m_md05(*this, "MD05"),
		m_md06(*this, "MD06"),
		m_md07(*this, "MD07"),
		m_md08(*this, "MD08"),
		m_md09(*this, "MD09"),
		m_md10(*this, "MD10"),
		m_md11(*this, "MD11"),
		m_md12(*this, "MD12"),
		m_md13(*this, "MD13"),
		m_md14(*this, "MD14"),
		m_md15(*this, "MD15"),
		m_md16(*this, "MD16"),
		m_md17(*this, "MD17"),
		m_md18(*this, "MD18"),
		m_md19(*this, "MD19"),
		m_md20(*this, "MD20"),
		m_md21(*this, "MD21"),
		m_md22(*this, "MD22"),
		m_md23(*this, "MD23"), m_bus(0),
		m_p1(0),
		m_p2(0),
		m_q(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iskr_1030_keyboard_device::device_start()
{
	set_pc_kbdc_device();

	m_ram.resize(0x100);
	save_item(NAME(m_ram));

	save_item(NAME(m_bus));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_q));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void iskr_1030_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  clock_write -
//-------------------------------------------------

WRITE_LINE_MEMBER( iskr_1030_keyboard_device::clock_write )
{
	LOG("clock write %d\n", state);
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  data_write -
//-------------------------------------------------

WRITE_LINE_MEMBER( iskr_1030_keyboard_device::data_write )
{
	LOG("data write %d\n", state);
}


//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

READ_LINE_MEMBER( iskr_1030_keyboard_device::t1_r )
{
	uint8_t data = data_signal();
	uint8_t bias = m_p1 & 15;

	if (!BIT(m_p1, 7)) {
		LOGDBG("t1_r (l) %d\n", data);
		return data;
	}

	if (bias) {
		LOGDBG("t1_r (b) %d\n", bias);
		return 1;
	}

	data = 0;
	switch (m_bus >> 2)
	{
	case  0<<1: data = m_md00->read(); break;
	case  1<<1: data = m_md01->read(); break;
	case  2<<1: data = m_md02->read(); break;
	case  3<<1: data = m_md03->read(); break;
	case  4<<1: data = m_md04->read(); break;
	case  5<<1: data = m_md05->read(); break;
	case  6<<1: data = m_md06->read(); break;
	case  7<<1: data = m_md07->read(); break;
	case  8<<1: data = m_md08->read(); break;
	case  9<<1: data = m_md09->read(); break;
	case 10<<1: data = m_md10->read(); break;
	case 11<<1: data = m_md11->read(); break;
	case  (0<<1)+1: data = m_md12->read(); break;
	case  (1<<1)+1: data = m_md13->read(); break;
	case  (2<<1)+1: data = m_md14->read(); break;
	case  (3<<1)+1: data = m_md15->read(); break;
	case  (4<<1)+1: data = m_md16->read(); break;
	case  (5<<1)+1: data = m_md17->read(); break;
	case  (6<<1)+1: data = m_md18->read(); break;
	case  (7<<1)+1: data = m_md19->read(); break;
	case  (8<<1)+1: data = m_md20->read(); break;
	case  (9<<1)+1: data = m_md21->read(); break;
	case (10<<1)+1: data = m_md22->read(); break;
	case (11<<1)+1: data = m_md23->read(); break;
	}
	data = BIT(data, m_bus&3);

	LOGDBG("t1_r (k r%d c%d) %d\n", m_bus&3, m_bus>>2, data);
	return data;
}


//-------------------------------------------------
//  ram_w -
//-------------------------------------------------

WRITE8_MEMBER( iskr_1030_keyboard_device::ram_w )
{
	LOGDBG("ram_w[%02x] <- %02x\n", offset, data);

	m_bus = offset;
	m_ram[offset] = data;
}


//-------------------------------------------------
//  ram_r -
//-------------------------------------------------

READ8_MEMBER( iskr_1030_keyboard_device::ram_r )
{
	LOGDBG("ram_r[%02x] = %02x\n", offset, m_ram[offset]);

	return m_ram[offset];
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

READ8_MEMBER( iskr_1030_keyboard_device::p1_r )
{
	/*
	    bit     description

	    0       -REQ IN
	    1       DATA IN
	    2
	    3
	    4
	    5
	    6
	    7
	*/

	uint8_t data = 0;

	LOG("p1_r %02x\n", data);

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( iskr_1030_keyboard_device::p2_w )
{
	/*
	    bit     description

	    0       ...
	    1       ...
	    2       ...
	    3       (not connected)
	    4       SPEAKER
	    5       LED RUS/LAT
	    6       LED NLK
	    7       LED CLK
	*/
	LOG("p2_w %02x\n", data);

	m_p2 = data;
}


//-------------------------------------------------
//  p1_w - OK
//-------------------------------------------------

WRITE8_MEMBER( iskr_1030_keyboard_device::p1_w )
{
	/*
	    bit     description

	    0       XXX
	    1       XXX
	    2       XXX
	    3       XXX
	    4       CLOCK out
	    5       DATA out
	    6       XXX
	    7       POLL GATE
	*/

	m_p1 = data;

	LOG("p1_w %02x (c %d d %d bias %d)\n", data, BIT(data, 4), BIT(data, 5), data&15);

	m_pc_kbdc->data_write_from_kb(BIT(data, 5));
	m_pc_kbdc->clock_write_from_kb(BIT(data, 4));
}
