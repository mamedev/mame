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

#include "emu.h"

#include "bus/sega8/sega8_slot.h"
#include "bus/sg1000_exp/sg1000exp.h"
#include "bus/sms_ctrl/controllers.h"
#include "bus/sms_ctrl/smsctrl.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"

#include "crsshair.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

#define Z80_TAG         "z80"
#define SN76489AN_TAG   "sn76489an"
#define TMS9918A_TAG    "tms9918a"

class sg1000_state_base : public driver_device
{
public:
	sg1000_state_base(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_vdp(*this, TMS9918A_TAG),
		m_psg(*this, SN76489AN_TAG),
		m_rom(*this, Z80_TAG),
		m_cart(*this, "slot"),
		m_sgexpslot(*this, "sgexp")
	{ }

protected:
	required_device<cpu_device> m_maincpu;
	required_device<tms9928a_device> m_vdp;
	required_device<sn76489a_device> m_psg;
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
	uint8_t sc3000_io_r(offs_t offset);
	void sc3000_io_w(offs_t offset, uint8_t data);

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
	void sc3000pal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void sc3000_base(machine_config &config);
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

uint8_t sg1000_state::sc3000_io_r(offs_t offset)
{
	uint8_t data = m_cart->read_io(offset);

	if (!BIT(offset, 6))
		data &= m_vdp->read(offset & 1);

	if (!BIT(offset, 5))
		data &= peripheral_r(offset & 3);

	return data;
}

void sg1000_state::sc3000_io_w(offs_t offset, uint8_t data)
{
	m_cart->write_io(offset, data);

	if (!BIT(offset, 7))
		m_psg->write(data);

	if (!BIT(offset, 6))
		m_vdp->write(offset & 1, data);

	if (!BIT(offset, 5))
		peripheral_w(offset & 3, data);
}

void sg1000_state::sc3000_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(sg1000_state::sc3000_io_r), FUNC(sg1000_state::sc3000_io_w));
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sg1000_state::trigger_nmi), 0)
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
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(omv2000_state::ctrl2_r<0>));

	PORT_MODIFY("C5")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(omv2000_state::ctrl2_r<2>));
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( sc3000 )
-------------------------------------------------*/

static INPUT_PORTS_START( sc3000 )
	// keyboard keys are added by the embedded sk1100 device

	PORT_START("NMI")
	// This is actually part of the main keyboard, though emulated separately here
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sg1000_state::trigger_nmi), 0)
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

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

	if (m_cart && (m_cart->dsram() || (m_cart->exists() && (m_cart->get_type() == SEGA8_BASIC_L3 || m_cart->get_type() == SEGA8_MUSIC_EDITOR
								|| m_cart->get_type() == SEGA8_DAHJEE_TYPEA || m_cart->get_type() == SEGA8_DAHJEE_TYPEB
								|| m_cart->get_type() == SEGA8_MULTICART || m_cart->get_type() == SEGA8_MEGACART))))
	{
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000, 0xffff, read8sm_delegate(*m_cart, FUNC(sega8_cart_slot_device::read_ram)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0xc000, 0xffff, write8sm_delegate(*m_cart, FUNC(sega8_cart_slot_device::write_ram)));
	}

	if (m_cart)
		m_cart->save_ram();

	m_ctrlports[0]->out_w(0x7f, 0x00);
	m_ctrlports[1]->out_w(0x7f, 0x00);
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void sg1000_state_base::sg1000_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 10.738635_MHz_XTAL / 3); // LH0080A

	/* video hardware */
	tms9918a_device &vdp(TMS9918A(config, TMS9918A_TAG, 10.738635_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SCREEN(config, "screen");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	SN76489A(config, m_psg, 10.738635_MHz_XTAL / 3);
	m_psg->ready_cb().set_inputline(m_maincpu, Z80_INPUT_LINE_WAIT).invert();
	m_psg->add_route(ALL_OUTPUTS, "mono", 1.00);

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
	SC3000_CART_SLOT(config, m_cart, sc3000_cart, nullptr);
}

void sc3000_state::sc3000pal(machine_config &config)
{
	sc3000(config);

	tms9929a_device &vdp(TMS9929A(config.replace(), TMS9918A_TAG, 10.738635_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// Z80A and SN76489AN are clocked by a separate ceramic oscillator since TMS9929A has no CPUCLK output
	m_maincpu->set_clock(3.58_MHz_XTAL);
	subdevice(SN76489AN_TAG)->set_clock(3.58_MHz_XTAL);

	m_sgexpslot->set_default_option("sk1100e");
}

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( sg1000 )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASE00 )
ROM_END

#define rom_sg1000m2 rom_sg1000

#define rom_sc3000 rom_sg1000

#define rom_sc3000pal rom_sg1000

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

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT    CLASS          INIT        COMPANY             FULLNAME                                    FLAGS
CONS( 1983, sg1000,    0,      0,      sg1000,    sg1000,  sg1000_state,  empty_init, "Sega",             "SG-1000",                                  MACHINE_SUPPORTS_SAVE )
CONS( 1984, sg1000m2,  sg1000, 0,      sc3000,    sc3000,  sc3000_state,  empty_init, "Sega",             "SG-1000 II",                               MACHINE_SUPPORTS_SAVE )
CONS( 1984, omv1000,   sg1000, 0,      omv1000,   omv1000, omv_state,     empty_init, "Tsukuda Original", "Othello Multivision FG-1000",              MACHINE_SUPPORTS_SAVE )
CONS( 1984, omv2000,   sg1000, 0,      omv2000,   omv2000, omv2000_state, empty_init, "Tsukuda Original", "Othello Multivision FG-2000",              MACHINE_SUPPORTS_SAVE )

// SC-3000 and SC-3000H are identical except for key mechanism.
// PAL/SECAM versions of SC-3000 were distributed by Yeno in France, by John Sands Electronics in Australia and by Grandstand Leisure in New Zealand.
COMP( 1983, sc3000,    0,      sg1000, sc3000,    sc3000,  sc3000_state,  empty_init, "Sega",             "SC-3000 (NTSC)",                           MACHINE_SUPPORTS_SAVE )
COMP( 1983, sc3000pal, sc3000, 0,      sc3000pal, sc3000,  sc3000_state,  empty_init, "Sega",             "SC-3000 (PAL)",                            MACHINE_SUPPORTS_SAVE )
