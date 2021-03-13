// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Azkoyen "Design"

    Tobacco vending machines

    Hardware:
    - Intel P8051
    - 27C256 EPROM
    - NEC D446C-2 SRAM
    - OKI M62X428 RTC
    - Rockwell 10937P-50 A8201-17 display controller

    TODO:
    - The coin mech isn't emulated. It's an Azkoyen L66S coin selector,
      which uses a PIC16C76/PIC16F76 (undumped).

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/74259.h"
#include "machine/bankdev.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/roc10937.h"

#include "design6.lh"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class design6_state : public driver_device
{
public:
	design6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_iobank(*this, "iobank"),
		m_vfd(*this, "vfd"),
		m_buttons(*this, "in2_%u", 1U),
		m_input_sel(0)
	{ }

	void design6(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i8051_device> m_maincpu;
	required_device<address_map_bank_device> m_iobank;
	required_device<roc10937_device> m_vfd;
	required_ioport_array<4> m_buttons;

	void mem_map(address_map &map);
	void io_map(address_map &map);
	void iobanked_map(address_map &map);

	void port1_w(uint8_t data);
	uint8_t in2_r();

	uint8_t m_input_sel;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void design6_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}

void design6_state::io_map(address_map &map)
{
	map(0x0000, 0xffff).rw(m_iobank, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
}

void design6_state::iobanked_map(address_map &map)
{
	map(0x00000, 0x00000).portr("in0");
	map(0x00000, 0x00007).w("outlatch0", FUNC(cd4099_device::write_d0));
	map(0x00010, 0x00010).portr("in1");
	map(0x00010, 0x00017).w("outlatch1", FUNC(cd4099_device::write_d0));
	map(0x00020, 0x00020).r(FUNC(design6_state::in2_r));
	map(0x00020, 0x00027).w("outlatch2", FUNC(cd4099_device::write_d0));
	map(0x00030, 0x00030).portr("in3");
	map(0x00030, 0x00037).w("outlatch3", FUNC(cd4099_device::write_d0));
	map(0x00040, 0x00047).w("outlatch4", FUNC(cd4099_device::write_d0));
	map(0x00060, 0x0006f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0x10000, 0x107ff).ram().share("nvram");
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( design6 )
	PORT_START("in0")
	PORT_DIPUNKNOWN(0x01, IP_ACTIVE_HIGH) PORT_NAME("in0:0")
	PORT_DIPUNKNOWN(0x02, IP_ACTIVE_HIGH) PORT_NAME("in0:1")
	PORT_DIPUNKNOWN(0x04, IP_ACTIVE_HIGH) PORT_NAME("in0:2")
	PORT_DIPUNKNOWN(0x08, IP_ACTIVE_HIGH) PORT_NAME("in0:3")
	PORT_DIPUNKNOWN(0x10, IP_ACTIVE_HIGH) PORT_NAME("in0:4")
	PORT_DIPUNKNOWN(0x20, IP_ACTIVE_HIGH) PORT_NAME("in0:5")
	PORT_SERVICE(0x40, IP_ACTIVE_HIGH)
	PORT_DIPUNKNOWN(0x80, IP_ACTIVE_HIGH) PORT_NAME("in0:7")

	PORT_START("in1")
	PORT_DIPNAME(0x01, 0x00, "Coin Return Pulse 3")
	PORT_DIPSETTING(   0x01, DEF_STR( On ))
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPNAME(0x02, 0x00, "Coin Return Pulse 2")
	PORT_DIPSETTING(   0x02, DEF_STR( On ))
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPNAME(0x04, 0x00, "Coin Return Pulse 1")
	PORT_DIPSETTING(   0x04, DEF_STR( On ))
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPNAME(0x08, 0x08, "Empty Return 3")
	PORT_DIPSETTING(   0x08, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPNAME(0x10, 0x10, "Empty Return 2")
	PORT_DIPSETTING(   0x10, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPNAME(0x20, 0x20, "Empty Return 1")
	PORT_DIPSETTING(   0x20, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPNAME(0x40, 0x00, "Averia Tramp. 1")
	PORT_DIPSETTING(   0x40, DEF_STR( On ))
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPUNKNOWN(0x80, IP_ACTIVE_HIGH) PORT_NAME("in1:7")

	PORT_START("in2_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT) PORT_NAME("Coin Recovery")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Q) PORT_NAME("Canal 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_W) PORT_NAME("Canal 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_E) PORT_NAME("Canal 3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_R) PORT_NAME("Canal 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_T) PORT_NAME("Canal 5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Y) PORT_NAME("Canal 6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("in2_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_DIPUNKNOWN(0x04, IP_ACTIVE_HIGH) PORT_NAME("in2_2:2")
	PORT_DIPUNKNOWN(0x08, IP_ACTIVE_HIGH) PORT_NAME("in2_2:3")
	PORT_DIPUNKNOWN(0x10, IP_ACTIVE_HIGH) PORT_NAME("in2_2:4")
	PORT_DIPUNKNOWN(0x20, IP_ACTIVE_HIGH) PORT_NAME("in2_2:5")
	PORT_DIPUNKNOWN(0x40, IP_ACTIVE_HIGH) PORT_NAME("in2_2:6")
	PORT_DIPUNKNOWN(0x80, IP_ACTIVE_HIGH) PORT_NAME("in2_2:7")

	PORT_START("in2_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("in2_4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_DIPUNKNOWN(0x04, IP_ACTIVE_HIGH) PORT_NAME("in2_4:2")
	PORT_DIPUNKNOWN(0x08, IP_ACTIVE_HIGH) PORT_NAME("in2_4:3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_A) PORT_NAME("Remote Control A")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_B) PORT_NAME("Remote Control B")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_C) PORT_NAME("Remote Control C")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_D) PORT_NAME("Remote Control D")

	PORT_START("in3")
	PORT_DIPUNKNOWN(0x01, IP_ACTIVE_HIGH) PORT_NAME("in3:0")
	PORT_DIPUNKNOWN(0x02, IP_ACTIVE_HIGH) PORT_NAME("in3:1")
	PORT_DIPUNKNOWN(0x04, IP_ACTIVE_HIGH) PORT_NAME("in3:2")
	PORT_DIPUNKNOWN(0x08, IP_ACTIVE_HIGH) PORT_NAME("in3:3")
	PORT_DIPUNKNOWN(0x10, IP_ACTIVE_HIGH) PORT_NAME("in3:4")
	PORT_DIPUNKNOWN(0x20, IP_ACTIVE_HIGH) PORT_NAME("in3:5")
	PORT_DIPUNKNOWN(0x40, IP_ACTIVE_HIGH) PORT_NAME("in3:6")
	PORT_DIPUNKNOWN(0x80, IP_ACTIVE_HIGH) PORT_NAME("in3:7")
INPUT_PORTS_END

static INPUT_PORTS_START( designe )
	PORT_INCLUDE(design6)

	PORT_MODIFY("in2_1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_U) PORT_NAME("Canal 7")

	PORT_MODIFY("in2_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F) PORT_NAME("Canal 8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_G) PORT_NAME("Canal 9")

	PORT_MODIFY("in2_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_H) PORT_NAME("Canal 10")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_J) PORT_NAME("Canal 11")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_K) PORT_NAME("Canal 12")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_L) PORT_NAME("Canal 13")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Y) PORT_NAME("Canal 14")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_X) PORT_NAME("Canal 15")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_C) PORT_NAME("Canal 16")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_V) PORT_NAME("Canal 17")

	PORT_MODIFY("in2_4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_N) PORT_NAME("Canal 18")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_M) PORT_NAME("Canal 19")
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void design6_state::port1_w(uint8_t data)
{
	// 7-------  watchdog?
	// -6------  ram write protect?
	// --5-----  ram enable?
	// ---4----  not used?
	// ----3210  input select

	m_iobank->set_bank(BIT(data, 5));
	m_input_sel = data & 0x0f;
}

uint8_t design6_state::in2_r()
{
	uint8_t data = 0;

	if (BIT(m_input_sel, 0)) data |= m_buttons[0]->read();
	if (BIT(m_input_sel, 1)) data |= m_buttons[1]->read();
	if (BIT(m_input_sel, 2)) data |= m_buttons[2]->read();
	if (BIT(m_input_sel, 3)) data |= m_buttons[3]->read();

	return data;
}

void design6_state::machine_start()
{
	// register for save states
	save_item(NAME(m_input_sel));
}

void design6_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void design6_state::design6(machine_config &config)
{
	I8051(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &design6_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &design6_state::io_map);
	m_maincpu->port_out_cb<1>().set(FUNC(design6_state::port1_w));

	ADDRESS_MAP_BANK(config, m_iobank, 0);
	m_iobank->set_map(&design6_state::iobanked_map);
	m_iobank->set_addr_width(17);
	m_iobank->set_data_width(8);
	m_iobank->set_stride(0x10000);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	CD4099(config, "outlatch0");
//  outlatch0.q_out_cb<0>() // enable coin return motor 1
//  outlatch0.q_out_cb<1>() // enable coin return motor 2
//  outlatch0.q_out_cb<2>() // enable coin return motor 3
//  outlatch0.q_out_cb<3>() // master enable coin return motor?
//  outlatch0.q_out_cb<6>() // ?

	cd4099_device &outlatch1(CD4099(config, "outlatch1"));
	outlatch1.q_out_cb<5>().set("vfd", FUNC(roc10937_device::data));
	outlatch1.q_out_cb<6>().set("vfd", FUNC(roc10937_device::sclk));
	outlatch1.q_out_cb<7>().set("vfd", FUNC(roc10937_device::por));

	CD4099(config, "outlatch2");
	CD4099(config, "outlatch3");
	CD4099(config, "outlatch4");

	ROC10937(config, m_vfd);

	MSM6242(config, "rtc", 32.768_kHz_XTAL);

	config.set_default_layout(layout_design6);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( design6 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x8000, CRC(1155999c) SHA1(2896af89011c496f905ed0e57d7035a3b612c718))

	ROM_REGION(0x4000, "coinsel", 0)
	ROM_LOAD("pic16x76_l56s-l66s.bin", 0x0000, 0x4000, NO_DUMP)
ROM_END

ROM_START( designe )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("designe.bin", 0x0000, 0x8000, CRC(693d40bd) SHA1(9596bbf9c367bc919393923460da15563d9447ca))

	ROM_REGION(0x4000, "coinsel", 0)
	ROM_LOAD("pic16x76_l56s-l66s.bin", 0x0000, 0x4000, NO_DUMP)
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR   NAME     PARENT  MACHINE  INPUT    CLASS          INIT        ROTATION  COMPANY    FULLNAME         FLAGS
GAME( 1995?, design6, 0,      design6, design6, design6_state, empty_init, ROT0,     "Azkoyen", "Design D6",     MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1995?, designe, 0,      design6, designe, design6_state, empty_init, ROT0,     "Azkoyen", "Design (Euro)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
