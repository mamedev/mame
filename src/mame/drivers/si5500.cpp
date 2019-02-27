// license:BSD-3-Clause
// copyright-holders:AJR
/*****************************************************************************************

    Skeleton driver for Scientific Instruments Model 5500 temperature controller.

*****************************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9980a.h"
#include "machine/74259.h"
//#include "machine/icl7104.h"
#include "machine/tms9901.h"
#include "machine/tms9902.h"
#include "machine/tms9914.h"
#include "machine/x2201.h"

class si5500_state : public driver_device
{
public:
	si5500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainpsi(*this, "mainpsi")
		, m_gpibpsi(*this, "gpibpsi")
		, m_gpibc(*this, "gpibc")
		, m_keyplatch(*this, "keyplatch")
		, m_keypad(*this, "KEYPAD%u", 0U)
	{
	}

	void si5500(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void mainic_w(offs_t offset, u8 data);
	DECLARE_WRITE_LINE_MEMBER(gpib_int_w);
	DECLARE_WRITE_LINE_MEMBER(acc_int_w);
	u8 gpibpsi_input_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(gpibc_we_w);
	DECLARE_WRITE_LINE_MEMBER(gpibc_dbin_w);
	u8 keypad_r();

	void mem_map(address_map &map);
	void cru_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<tms9901_device> m_mainpsi;
	required_device<tms9901_device> m_gpibpsi;
	required_device<tms9914_device> m_gpibc;
	required_device<ls259_device> m_keyplatch;
	required_ioport_array<4> m_keypad;

	u8 m_gpib_data;
};

void si5500_state::machine_start()
{
	m_gpib_data = 0;
	save_item(NAME(m_gpib_data));
}

void si5500_state::mainic_w(offs_t offset, u8 data)
{
	if (data)
		m_maincpu->set_input_line(offset & 7, ASSERT_LINE);
	else
		m_maincpu->set_input_line(INT_9980A_CLEAR, CLEAR_LINE);
}

WRITE_LINE_MEMBER(si5500_state::gpib_int_w)
{
	m_mainpsi->set_single_int(4, state);
}

WRITE_LINE_MEMBER(si5500_state::acc_int_w)
{
	m_mainpsi->set_single_int(5, state);
}

u8 si5500_state::gpibpsi_input_r(offs_t offset)
{
	if (offset == tms9901_device::P0_P7)
		return m_gpib_data;

	return 0xff;
}

WRITE_LINE_MEMBER(si5500_state::gpibc_we_w)
{
	if (!state)
	{
		u16 pio = m_gpibpsi->pio_outputs();
		m_gpibc->write((pio >> 8) & 7, pio & 0xff);
	}
}

WRITE_LINE_MEMBER(si5500_state::gpibc_dbin_w)
{
	if (state)
	{
		u16 pio = m_gpibpsi->pio_outputs();
		m_gpib_data = m_gpibc->read((pio >> 8) & 7);
	}
}

u8 si5500_state::keypad_r()
{
	u8 result = 0xff;

	for (int n = 0; n < 4; n++)
		if (!BIT(m_keyplatch->output_state(), n))
			result &= m_keypad[n]->read();

	return result;
}

void si5500_state::mem_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("program", 0);
	map(0x3800, 0x3fff).ram();
}

void si5500_state::cru_map(address_map &map)
{
	// MAME currently has incompatible addressing for CRU reads and writes
	map(0x00, 0x03).r(m_mainpsi, FUNC(tms9901_device::read));
	map(0x08, 0x0b).r("acc", FUNC(tms9902_device::cruread));
	map(0x0c, 0x0f).r("adpsi", FUNC(tms9901_device::read));
	map(0x16, 0x16).nopr();
	map(0x17, 0x17).r(FUNC(si5500_state::keypad_r));
	map(0x40, 0x43).r("nvrpsi", FUNC(tms9901_device::read));
	map(0x44, 0x47).r(m_gpibpsi, FUNC(tms9901_device::read));
	map(0x80, 0xff).r("novram", FUNC(x2201_device::read_byte));

	map(0x000, 0x01f).w(m_mainpsi, FUNC(tms9901_device::write));
	map(0x040, 0x05f).w("acc", FUNC(tms9902_device::cruwrite));
	map(0x060, 0x07f).w("adpsi", FUNC(tms9901_device::write));
	map(0x080, 0x087).w("outlatch1", FUNC(ls259_device::write_d0));
	map(0x088, 0x08f).w("outlatch2", FUNC(ls259_device::write_d0));
	map(0x090, 0x097).w("outlatch3", FUNC(ls259_device::write_d0));
	map(0x0a0, 0x0a7).w("outlatch4", FUNC(ls259_device::write_d0));
	map(0x0a8, 0x0af).w("outlatch5", FUNC(ls259_device::write_d0));
	map(0x0b0, 0x0bf).w(m_keyplatch, FUNC(ls259_device::write_d0));
	map(0x200, 0x21f).w("nvrpsi", FUNC(tms9901_device::write));
	map(0x220, 0x23f).w(m_gpibpsi, FUNC(tms9901_device::write));
	map(0x400, 0x7ff).w("novram", FUNC(x2201_device::write));
}

static INPUT_PORTS_START(si5500)
	PORT_START("KEYPAD0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7  Edit  Prop") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4  Select  Heater") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1  Diag  Sensor") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYPAD1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8  Keep  Int") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5  Auto  Time") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2  Cal  Sensor") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYPAD2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9  Load  Der") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6  Print  Rate") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3  I/O  Set Pt") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("#  -") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYPAD3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Stop") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Man") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Run") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("*") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void si5500_state::si5500(machine_config &config)
{
	TMS9981(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &si5500_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &si5500_state::cru_map);

	TMS9901(config, m_mainpsi, 10_MHz_XTAL / 4);
	m_mainpsi->intlevel_cb().set(FUNC(si5500_state::mainic_w));

	tms9902_device &acc(TMS9902(config, "acc", 10_MHz_XTAL / 4));
	acc.int_cb().set(FUNC(si5500_state::acc_int_w));

	TMS9901(config, "adpsi", 10_MHz_XTAL / 4);

	X2201(config, "novram");

	tms9901_device &nvrpsi(TMS9901(config, "nvrpsi", 10_MHz_XTAL / 4));
	// P0-11 = inputs for DAC?
	nvrpsi.p_out_cb(13).set("novram", FUNC(x2201_device::array_recall_w));
	nvrpsi.p_out_cb(14).set("novram", FUNC(x2201_device::store_w));
	nvrpsi.p_out_cb(15).set("novram", FUNC(x2201_device::cs_w));

	TMS9901(config, m_gpibpsi, 10_MHz_XTAL / 4);
	m_gpibpsi->p_out_cb(11).set(FUNC(si5500_state::gpibc_we_w));
	m_gpibpsi->p_out_cb(12).set(FUNC(si5500_state::gpibc_dbin_w));
	m_gpibpsi->read_cb().set(FUNC(si5500_state::gpibpsi_input_r));

	TMS9914(config, m_gpibc, 10_MHz_XTAL / 4);
	m_gpibc->int_write_cb().set(FUNC(si5500_state::gpib_int_w));

	LS259(config, "outlatch1");
	LS259(config, "outlatch2");
	LS259(config, "outlatch3");
	LS259(config, "outlatch4");
	LS259(config, "outlatch5");
	LS259(config, m_keyplatch);
}

// Board #1 (power): no digital ICs
// Board #2 (analog): TMS9901NL, ICL7104-16CPL, ICL8052ACPD, AD524AD, LF13509D
// Board #3 (output): X2201AD, 2x TMS9901NL, DAC1222LCN, TMS9914ANL, SN75162BN, DS75160AN
// Board #4: HM6116P-4, 3x TMS2532A-25JL, XTAL (10 MHz), TMS9981JDL, TMS9902ANL, TMS9901NL
// Back of front panel: 6x HD74LS259, HD74LS251
ROM_START(si5500)
	ROM_REGION(0x3000, "program", 0)
	ROM_LOAD("m5500dl_7-9-86.u4", 0x0000, 0x1000, CRC(cfcff0fe) SHA1(94173b3b7513954221ce3402ea5b5c36dfa5a8da))
	ROM_LOAD("m5500dl_7-9-86.u5", 0x1000, 0x1000, CRC(a932e85a) SHA1(f7152ae78bad79b457bb739e7ecc4556ca33cbbc))
	ROM_LOAD("m5500dl_7-9-86.u6", 0x2000, 0x1000, CRC(3161347d) SHA1(fab6c228a21ef3ecce255079a48ef1697f2c7ccb))
ROM_END

COMP(1986, si5500, 0, 0, si5500, si5500, si5500_state, empty_init, "Scientific Instruments", "Model 5500 Temperature Controller", MACHINE_IS_SKELETON)
