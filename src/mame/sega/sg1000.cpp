// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

Sega SG-1000

PCB Layout
----------

171-5078 (C) SEGA 1983
171-5046 REV. A (C) SEGA 1983

|---------------------------|                              |----------------------------|
|   SW1     CN2             |   |------|---------------|   |    SW2     CN4             |
|                           |---|         CN3          |---|                            |
|  CN1                                                                              CN5 |
|                                                                                       |
|   10.738635MHz            |------------------------------|                7805        |
|   |---|                   |------------------------------|                            |
|   |   |                                 CN6                                           |
|   | 9 |                                                                               |
|   | 9 |                                                                       LS32    |
|   | 1 |       |---------|                                                             |
|   | 8 |       | TMM2009 |                                                     LS139   |
|   | A |       |---------|             |------------------|                            |
|   |   |                               |       Z80        |                            |
|   |---|                               |------------------|                            |
|                                                                                       |
|                                                                                       |
|       MB8118  MB8118  MB8118  MB8118              SN76489A            SW3             |
|           MB8118  MB8118  MB8118  MB8118                          LS257   LS257       |
|---------------------------------------------------------------------------------------|

Notes:
    All IC's shown.

    Z80     - NEC D780C-1 / Zilog Z8400A (REV.A) Z80A CPU @ 3.579545
    TMS9918A- Texas Instruments TMS9918ANL Video Display Processor @ 10.738635MHz
    MB8118  - Fujitsu MB8118-12 16K x 1 Dynamic RAM
    TMM2009 - Toshiba TMM2009P-A / TMM2009P-B (REV.A)
    SN76489A- Texas Instruments SN76489AN Digital Complex Sound Generator @ 3.579545
    CN1     - player 1 joystick connector
    CN2     - RF video connector
    CN3     - keyboard connector
    CN4     - power connector (+9VDC)
    CN5     - player 2 joystick connector
    CN6     - cartridge connector
    SW1     - TV channel select switch
    SW2     - power switch
    SW3     - hold switch

*/

/*

    TODO:

    - SC-3000 return instruction referenced by R when reading ports 60-7f,e0-ff
    - connect PSG /READY signal to Z80 WAIT
    - accurate video timing
    - SH-400 racing controller
    - SF-7000 serial comms

*/


#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "bus/sega8/sega8_slot.h"
#include "bus/sg1000_exp/sg1000exp.h"
#include "bus/sms_ctrl/controllers.h"
#include "bus/sms_ctrl/smsctrl.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"

#include "crsshair.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/sf7000_dsk.h"


namespace {

#define Z80_TAG         "z80"
#define SN76489AN_TAG   "sn76489an"
#define UPD765_TAG      "upd765"
#define UPD8251_TAG     "upd8251"
#define UPD9255_TAG     "upd9255"
#define UPD9255_1_TAG   "upd9255_1" // "upd9255_0" is being used by sk1100 device
#define TMS9918A_TAG    "tms9918a"
#define RS232_TAG       "rs232"

class sg1000_state_base : public driver_device
{
public:
	sg1000_state_base(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_ram(*this, RAM_TAG),
		m_rom(*this, Z80_TAG),
		m_cart(*this, "slot"),
		m_sgexpslot(*this, "sgexp")
	{ }

protected:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	optional_device<sega8_cart_slot_device> m_cart;
	required_device<sg1000_expansion_slot_device> m_sgexpslot;

	void sg1000_base(machine_config &config);
};

class sg1000_state : public sg1000_state_base
{
public:
	sg1000_state(const machine_config &mconfig, device_type type, const char *tag) :
		sg1000_state_base(mconfig, type, tag),
		m_ctrlports(*this, "ctrl%u", 1U)
	{ }

	void sg1000(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );

protected:
	required_device_array<sms_control_port_device, 2> m_ctrlports;

	virtual void machine_start() override ATTR_COLD;

	uint8_t peripheral_r(offs_t offset);
	void peripheral_w(offs_t offset, uint8_t data);

	void sg1000_map(address_map &map) ATTR_COLD;
	void sg1000_io_map(address_map &map) ATTR_COLD;
	void sc3000_map(address_map &map) ATTR_COLD;
	void sc3000_io_map(address_map &map) ATTR_COLD;
};

class omv_state : public sg1000_state_base
{
public:
	omv_state(const machine_config &mconfig, device_type type, const char *tag) :
		sg1000_state_base(mconfig, type, tag)
	{ }

	void omv1000(machine_config &config);

private:
	uint8_t omv_r(offs_t offset);
	void omv_w(offs_t offset, uint8_t data);

	void omv_map(address_map &map) ATTR_COLD;
	void omv_io_map(address_map &map) ATTR_COLD;
};

class omv2000_state : public omv_state
{
public:
	omv2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		omv_state(mconfig, type, tag),
		m_ctrl2(*this, "ctrl2")
	{ }

	void omv2000(machine_config &config);

	template <unsigned Shift> ioport_value ctrl2_r();

private:
	required_device<sms_control_port_device> m_ctrl2;

	virtual void machine_start() override ATTR_COLD;
};

class sc3000_state : public sg1000_state
{
public:
	sc3000_state(const machine_config &mconfig, device_type type, const char *tag) :
		sg1000_state(mconfig, type, tag)
	{ }

	void sc3000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void sc3000_base(machine_config &config);
};

class sf7000_state : public sc3000_state
{
public:
	sf7000_state(const machine_config &mconfig, device_type type, const char *tag) :
		sc3000_state(mconfig, type, tag),
		m_fdc(*this, UPD765_TAG),
		m_centronics(*this, "centronics"),
		m_floppy0(*this, UPD765_TAG ":0:3ssdd")
	{ }

	void sf7000(machine_config &config);

private:
	required_device<upd765a_device> m_fdc;
	required_device<centronics_device> m_centronics;
	required_device<floppy_image_device> m_floppy0;

	int m_centronics_busy = 0;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void write_centronics_busy(int state);
	uint8_t ppi_pa_r();
	void ppi_pc_w(uint8_t data);

	void sf7000_io_map(address_map &map) ATTR_COLD;
	void sf7000_map(address_map &map) ATTR_COLD;

	static void floppy_formats(format_registration &fr);
};

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

// TODO: not sure if the OMV bios actually detects the presence of a cart,
// or if the cart data simply overwrites the internal BIOS...
// for the moment let assume the latter!
uint8_t omv_state::omv_r(offs_t offset)
{
	if (m_cart && m_cart->exists())
		return m_cart->read_cart(offset);
	else
		return m_rom->base()[offset];
}

void omv_state::omv_w(offs_t offset, uint8_t data)
{
	if (m_cart && m_cart->exists())
		m_cart->write_cart(offset, data);
}

uint8_t sg1000_state::peripheral_r(offs_t offset)
{
	bool joy_ports_disabled = m_sgexpslot->is_readable(offset);

	if (joy_ports_disabled)
		return m_sgexpslot->read(offset);
	else if (offset & 0x01)
		return BIT(m_ctrlports[1]->in_r(), 2, 4) | (0xf0 & m_sgexpslot->read(offset));
	else
		return BIT(m_ctrlports[0]->in_r(), 0, 6) | (BIT(m_ctrlports[1]->in_r(), 0, 2) << 6);
}

void sg1000_state::peripheral_w(offs_t offset, uint8_t data)
{
	bool joy_ports_disabled = m_sgexpslot->is_writeable(offset);

	if (joy_ports_disabled)
	{
		m_sgexpslot->write(offset, data);
	}
}

/*-------------------------------------------------
    ADDRESS_MAP( sg1000_map )
-------------------------------------------------*/

void sg1000_state::sg1000_map(address_map &map)
{
	map(0x0000, 0xbfff).rw(m_cart, FUNC(sega8_cart_slot_device::read_cart), FUNC(sega8_cart_slot_device::write_cart));
	map(0xc000, 0xc3ff).mirror(0x3c00).ram();
}

/*-------------------------------------------------
    ADDRESS_MAP( sg1000_io_map )
-------------------------------------------------*/

void sg1000_state::sg1000_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).mirror(0x3f).w(SN76489AN_TAG, FUNC(sn76489a_device::write));
	map(0x80, 0x81).mirror(0x3e).rw(TMS9918A_TAG, FUNC(tms9918a_device::read), FUNC(tms9918a_device::write));
	map(0xdc, 0xdf).rw(FUNC(sg1000_state::peripheral_r), FUNC(sg1000_state::peripheral_w));
}

/*-------------------------------------------------
    ADDRESS_MAP( omv_map )
-------------------------------------------------*/

void omv_state::omv_map(address_map &map)
{
	map(0x0000, 0xbfff).rw(FUNC(omv_state::omv_r), FUNC(omv_state::omv_w));
	map(0xc000, 0xc7ff).mirror(0x3800).ram();
}

/*-------------------------------------------------
    ADDRESS_MAP( omv_io_map )
-------------------------------------------------*/

void omv_state::omv_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).mirror(0x3f).w(SN76489AN_TAG, FUNC(sn76489a_device::write));
	map(0x80, 0x81).mirror(0x3e).rw(TMS9918A_TAG, FUNC(tms9918a_device::read), FUNC(tms9918a_device::write));
	map(0xc0, 0xc0).mirror(0x38).portr("C0");
	map(0xc1, 0xc1).mirror(0x38).portr("C1");
	map(0xc2, 0xc2).mirror(0x38).portr("C2");
	map(0xc3, 0xc3).mirror(0x38).portr("C3");
	map(0xc4, 0xc4).mirror(0x3a).portr("C4");
	map(0xc5, 0xc5).mirror(0x3a).portr("C5");
}

/*-------------------------------------------------
    ADDRESS_MAP( sc3000_map )
-------------------------------------------------*/

void sg1000_state::sc3000_map(address_map &map)
{
	map(0x0000, 0xbfff).rw(m_cart, FUNC(sega8_cart_slot_device::read_cart), FUNC(sega8_cart_slot_device::write_cart));
	map(0xc000, 0xc7ff).mirror(0x3800).ram();
}

/*-------------------------------------------------
    ADDRESS_MAP( sc3000_io_map )
-------------------------------------------------*/

void sg1000_state::sc3000_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(m_cart, FUNC(sega8_cart_slot_device::read_io), FUNC(sega8_cart_slot_device::write_io));
	map(0x7f, 0x7f).w(SN76489AN_TAG, FUNC(sn76489a_device::write));
	map(0xbe, 0xbf).rw(TMS9918A_TAG, FUNC(tms9918a_device::read), FUNC(tms9918a_device::write));
	map(0xdc, 0xdf).rw(FUNC(sg1000_state::peripheral_r), FUNC(sg1000_state::peripheral_w));
}

/* This is how the I/O ports are really mapped, but MAME does not support overlapping ranges
void sg1000_state::sc3000_io_map(address_map &map)
{
    map.global_mask(0xff);
    map(0x00, 0x00).mirror(0xdf).rw(UPD9255_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
    map(0x00, 0x00).mirror(0x7f).w(SN76489AN_TAG, FUNC(sn76489a_device::write));
    map(0x00, 0x01).mirror(0xae).rw(TMS9918A_TAG, FUNC(tms9918a_device::read), FUNC(tms9918a_device::write));
    map(0x60, 0x60).mirror(0x9f).r(FUNC(sg1000_state::sc3000_r_r));
}
*/

/*-------------------------------------------------
    ADDRESS_MAP( sf7000_map )
-------------------------------------------------*/

void sf7000_state::sf7000_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bank1").bankw("bank2");
	map(0x4000, 0xffff).ram();
}

/*-------------------------------------------------
    ADDRESS_MAP( sf7000_io_map )
-------------------------------------------------*/

void sf7000_state::sf7000_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x7f, 0x7f).w(SN76489AN_TAG, FUNC(sn76489a_device::write));
	map(0xbe, 0xbf).rw(TMS9918A_TAG, FUNC(tms9918a_device::read), FUNC(tms9918a_device::write));
	map(0xdc, 0xdf).rw(FUNC(sf7000_state::peripheral_r), FUNC(sf7000_state::peripheral_w));
	map(0xe0, 0xe1).m(m_fdc, FUNC(upd765a_device::map));
	map(0xe4, 0xe7).rw(UPD9255_1_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe8, 0xe9).rw(UPD8251_TAG, FUNC(i8251_device::read), FUNC(i8251_device::write));
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_CHANGED_MEMBER( trigger_nmi )
-------------------------------------------------*/

INPUT_CHANGED_MEMBER( sg1000_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

/*-------------------------------------------------
    ioport_value ctrl2_r()
-------------------------------------------------*/

template <unsigned Shift>
ioport_value omv2000_state::ctrl2_r()
{
	return m_ctrl2->in_r() >> Shift;
}

/*-------------------------------------------------
    INPUT_PORTS( sg1000 )
-------------------------------------------------*/

static INPUT_PORTS_START( sg1000 )
	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, sg1000_state, trigger_nmi, 0)
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( omv )
-------------------------------------------------*/

static INPUT_PORTS_START( omv1000 )
	PORT_START("C0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("9 #") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0 *") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("S-1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("S-2")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( omv2000 )
	PORT_INCLUDE( omv1000 )

	PORT_MODIFY("C4")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(omv2000_state, ctrl2_r<0>);

	PORT_MODIFY("C5")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(omv2000_state, ctrl2_r<2>);
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( sc3000 )
-------------------------------------------------*/

static INPUT_PORTS_START( sc3000 )
	// keyboard keys are added by the embedded sk1100 device

	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, sg1000_state, trigger_nmi, 0)
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( sf7000 )
-------------------------------------------------*/

static INPUT_PORTS_START( sf7000 )
	PORT_INCLUDE( sc3000 )

	PORT_START("BAUD")
	PORT_CONFNAME( 0x07, 0x05, "Baud rate")
	PORT_CONFSETTING( 0x00, "9600 baud" )
	PORT_CONFSETTING( 0x01, "4800 baud" )
	PORT_CONFSETTING( 0x02, "2400 baud" )
	PORT_CONFSETTING( 0x03, "1200 baud" )
	PORT_CONFSETTING( 0x04, "600 baud" )
	PORT_CONFSETTING( 0x05, "300 baud" )
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    I8255 INTERFACE
-------------------------------------------------*/

void sf7000_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

uint8_t sf7000_state::ppi_pa_r()
{
	/*
	    Signal  Description

	    PA0     INT from FDC
	    PA1     BUSY from Centronics printer
	    PA2     INDEX from FDD
	    PA3
	    PA4
	    PA5
	    PA6
	    PA7
	*/

	uint8_t data = 0;

	data |= m_fdc->get_irq() ? 0x01 : 0x00;
	data |= m_centronics_busy << 1;
	data |= m_floppy0->idx_r() << 2;

	return data;
}

void sf7000_state::ppi_pc_w(uint8_t data)
{
	/*
	    Signal  Description

	    PC0     /INUSE signal to FDD
	    PC1     /MOTOR ON signal to FDD
	    PC2     TC signal to FDC
	    PC3     RESET signal to FDC
	    PC4     not connected
	    PC5     not connected
	    PC6     /ROM SEL (switch between IPL ROM and RAM)
	    PC7     /STROBE to Centronics printer
	*/

	if (!BIT(data, 0))
		m_fdc->set_floppy(m_floppy0);
	else
		m_fdc->set_floppy(nullptr);

	/* floppy motor */
	m_floppy0->mon_w(BIT(data, 1));

	/* FDC terminal count */
	m_fdc->tc_w(BIT(data, 2));

	/* FDC reset */
	if (BIT(data, 3))
	{
		m_fdc->reset();
	}

	/* ROM selection */
	membank("bank1")->set_entry(BIT(data, 6));

	/* printer strobe */
	m_centronics->write_strobe(BIT(data, 7));
}

/*-------------------------------------------------
    upd765_interface sf7000_upd765_interface
-------------------------------------------------*/

void sf7000_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_SF7000_FORMAT);
}

/*-------------------------------------------------
    floppy_interface sf7000_floppy_interface
-------------------------------------------------*/

static void sf7000_floppies(device_slot_interface &device)
{
	device.option_add("3ssdd", FLOPPY_3_SSDD);
}

/*-------------------------------------------------
    MACHINE_START( sg1000 )
-------------------------------------------------*/

void sg1000_state::machine_start()
{
	sg1000_state_base::machine_start();

	if (m_cart->get_type() == SEGA8_DAHJEE_TYPEA || m_cart->get_type() == SEGA8_DAHJEE_TYPEB)
	{
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000, 0xffff, read8sm_delegate(*m_cart, FUNC(sega8_cart_slot_device::read_ram)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0xc000, 0xffff, write8sm_delegate(*m_cart, FUNC(sega8_cart_slot_device::write_ram)));
	}

	if (m_cart)
		m_cart->save_ram();

	m_ctrlports[0]->out_w(0x3f, 0x40);
	m_ctrlports[1]->out_w(0x3f, 0x40);
}

/*-------------------------------------------------
    MACHINE_START( omv2000 )
-------------------------------------------------*/

void omv2000_state::machine_start()
{
	omv_state::machine_start();

	// TODO: confirm wiring of pin 7
	m_ctrl2->out_w(0x3f, 0x40);
}

/*-------------------------------------------------
    MACHINE_START( sc3000 )
-------------------------------------------------*/

void sc3000_state::machine_start()
{
	sg1000_state_base::machine_start();

	if (m_cart && m_cart->exists() && (m_cart->get_type() == SEGA8_BASIC_L3 || m_cart->get_type() == SEGA8_MUSIC_EDITOR
								|| m_cart->get_type() == SEGA8_DAHJEE_TYPEA || m_cart->get_type() == SEGA8_DAHJEE_TYPEB
								|| m_cart->get_type() == SEGA8_MULTICART || m_cart->get_type() == SEGA8_MEGACART))
	{
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000, 0xffff, read8sm_delegate(*m_cart, FUNC(sega8_cart_slot_device::read_ram)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0xc000, 0xffff, write8sm_delegate(*m_cart, FUNC(sega8_cart_slot_device::write_ram)));
	}

	if (m_cart)
		m_cart->save_ram();

	m_ctrlports[0]->out_w(0x7f, 0x00);
	m_ctrlports[1]->out_w(0x7f, 0x00);
}


/*-------------------------------------------------
    MACHINE_START( sf7000 )
-------------------------------------------------*/

void sf7000_state::machine_start()
{
	sc3000_state::machine_start();

	save_item(NAME(m_centronics_busy));

	/* configure memory banking */
	membank("bank1")->configure_entry(0, m_rom->base());
	membank("bank1")->configure_entry(1, m_ram->pointer());
	membank("bank2")->configure_entry(0, m_ram->pointer());
}

void sf7000_state::machine_reset()
{
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void sg1000_state_base::sg1000_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'738'635) / 3); // LH0080A

	/* video hardware */
	tms9918a_device &vdp(TMS9918A(config, TMS9918A_TAG, XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	SN76489A(config, SN76489AN_TAG, XTAL(10'738'635) / 3).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("sg1000");
}

void sc3000_state::sc3000_base(machine_config &config)
{
	sg1000_base(config);

	/* controller ports */
	SMS_CONTROL_PORT(config, m_ctrlports[0], sms_control_port_passive_devices, SMS_CTRL_OPTION_JOYPAD);
	SMS_CONTROL_PORT(config, m_ctrlports[1], sms_control_port_passive_devices, SMS_CTRL_OPTION_JOYPAD);

	/* sc3000 has all sk1100 features built-in, so add it as a fixed slot */
	SG1000_EXPANSION_SLOT(config, m_sgexpslot, sg1000_expansion_devices, "sk1100", true);

	/* the sk1100 device will add sc3000 cart and cass lists */
}

/*-------------------------------------------------
    machine_config( sg1000 )
-------------------------------------------------*/

void sg1000_state::sg1000(machine_config &config)
{
	sg1000_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sg1000_state::sg1000_map);
	m_maincpu->set_addrmap(AS_IO, &sg1000_state::sg1000_io_map);

	/* controller ports */
	SMS_CONTROL_PORT(config, m_ctrlports[0], sms_control_port_passive_devices, SMS_CTRL_OPTION_JOYPAD).set_fixed(true);
	SMS_CONTROL_PORT(config, m_ctrlports[1], sms_control_port_passive_devices, SMS_CTRL_OPTION_JOYPAD);

	/* expansion slot */
	SG1000_EXPANSION_SLOT(config, m_sgexpslot, sg1000_expansion_devices, nullptr, false);

	/* cartridge */
	SG1000_CART_SLOT(config, m_cart, sg1000_cart, nullptr).set_must_be_loaded(true);

	/* internal ram */
	RAM(config, m_ram).set_default_size("1K");
}

/*-------------------------------------------------
    machine_config( omv1000 )
-------------------------------------------------*/

void omv_state::omv1000(machine_config &config)
{
	sg1000_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &omv_state::omv_map);
	m_maincpu->set_addrmap(AS_IO, &omv_state::omv_io_map);

	/* expansion slot */
	SG1000_EXPANSION_SLOT(config, m_sgexpslot, sg1000_expansion_devices, nullptr, false);

	/* cartridge */
	OMV_CART_SLOT(config, m_cart, sg1000_cart, nullptr);

	/* internal ram */
	RAM(config, m_ram).set_default_size("2K");
}

/*-------------------------------------------------
    machine_config( omv2000 )
-------------------------------------------------*/

void omv2000_state::omv2000(machine_config &config)
{
	omv1000(config);

	/* controller ports */
	SMS_CONTROL_PORT(config, m_ctrl2, sms_control_port_passive_devices, SMS_CTRL_OPTION_JOYPAD);
}

/*-------------------------------------------------
    machine_config( sc3000 )
-------------------------------------------------*/

void sc3000_state::sc3000(machine_config &config)
{
	sc3000_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sc3000_state::sc3000_map);
	m_maincpu->set_addrmap(AS_IO, &sc3000_state::sc3000_io_map);

	/* cartridge */
	SC3000_CART_SLOT(config, m_cart, sg1000_cart, nullptr).set_must_be_loaded(true);

	/* internal ram */
	RAM(config, m_ram).set_default_size("2K");
}

/*-------------------------------------------------
    machine_config( sf7000 )
-------------------------------------------------*/

void sf7000_state::sf7000(machine_config &config)
{
	// FIXME: this thing is actually a cartridge slot peripheral for the SC-3000 and shouldn't be a separate machine

	sc3000_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sf7000_state::sf7000_map);
	m_maincpu->set_addrmap(AS_IO, &sf7000_state::sf7000_io_map);

	/* devices */
	i8255_device &ppi(I8255(config, UPD9255_1_TAG));
	ppi.in_pa_callback().set(FUNC(sf7000_state::ppi_pa_r));
	ppi.out_pb_callback().set("cent_data_out", FUNC(output_latch_device::write));
	ppi.out_pc_callback().set(FUNC(sf7000_state::ppi_pc_w));

	i8251_device &upd8251(I8251(config, UPD8251_TAG, 0));
	upd8251.txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	upd8251.dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	upd8251.rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(UPD8251_TAG, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(UPD8251_TAG, FUNC(i8251_device::write_dsr));

	UPD765A(config, m_fdc, 8'000'000, false, false);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", sf7000_floppies, "3ssdd", sf7000_state::floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(sf7000_state::write_centronics_busy));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	/* software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("sf7000");

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K");
}

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( sg1000 )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASE00 )
ROM_END

#define rom_sg1000m2 rom_sg1000

#define rom_sc3000 rom_sg1000

#define rom_sc3000h rom_sg1000

ROM_START( sf7000 )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASE00 )
	ROM_LOAD( "ipl.rom", 0x0000, 0x2000, CRC(d76810b8) SHA1(77339a6db2593aadc638bed77b8e9bed5d9d87e3) )
ROM_END

ROM_START( omv1000 )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASE00 )
	ROM_LOAD( "omvbios.bin", 0x0000, 0x4000, BAD_DUMP CRC(c5a67b95) SHA1(6d7c64dd60dee4a33061d3d3a7c2ed190d895cdb) )    // The BIOS comes from a Multivision FG-2000. It is still unknown if the FG-1000 BIOS differs
ROM_END

ROM_START( omv2000 )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASE00 )
	ROM_LOAD( "omvbios.bin", 0x0000, 0x4000, CRC(c5a67b95) SHA1(6d7c64dd60dee4a33061d3d3a7c2ed190d895cdb) )
ROM_END

} // anonymous namespace


/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY             FULLNAME                                    FLAGS
CONS( 1983, sg1000,   0,      0,      sg1000,  sg1000,  sg1000_state,  empty_init, "Sega",             "SG-1000",                                  MACHINE_SUPPORTS_SAVE )
CONS( 1984, sg1000m2, sg1000, 0,      sc3000,  sc3000,  sc3000_state,  empty_init, "Sega",             "SG-1000 II",                               MACHINE_SUPPORTS_SAVE )
COMP( 1983, sc3000,   0,      sg1000, sc3000,  sc3000,  sc3000_state,  empty_init, "Sega",             "SC-3000",                                  MACHINE_SUPPORTS_SAVE )
COMP( 1983, sc3000h,  sc3000, 0,      sc3000,  sc3000,  sc3000_state,  empty_init, "Sega",             "SC-3000H",                                 MACHINE_SUPPORTS_SAVE )
COMP( 1983, sf7000,   sc3000, 0,      sf7000,  sf7000,  sf7000_state,  empty_init, "Sega",             "SC-3000/Super Control Station SF-7000",    MACHINE_SUPPORTS_SAVE )
CONS( 1984, omv1000,  sg1000, 0,      omv1000, omv1000, omv_state,     empty_init, "Tsukuda Original", "Othello Multivision FG-1000",              MACHINE_SUPPORTS_SAVE )
CONS( 1984, omv2000,  sg1000, 0,      omv2000, omv2000, omv2000_state, empty_init, "Tsukuda Original", "Othello Multivision FG-2000",              MACHINE_SUPPORTS_SAVE )
