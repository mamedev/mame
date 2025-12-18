// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Elektronika MS 1201 series of processor boards, used in DVK series
    desktops.  Bus interface is MPI (clone of Q-Bus).

    Firmware's CLI is similar to DEC LSI-11 ODT.

    Available bootstrap routines vary by board and ROM revision:

    MS 1201.01 board:

    ROM 000 - paper tape (bootstrap format only), 'DX' floppy
    ROM 031, 054 - same, plus 'DY' and 'MX' floppies

    Paper tape: "177550L" (loads absolute loader from tape), followed by
	"P" (loads the rest of tape).
    DX floppy: "173000G" or "D0" (drive 0), or "D1" (drive 1)
    MX floppy: "X0" (drive 0) or "X1" (drive 1)

    MS 1201.02 board:

    ROM 055 - 'LA' paper tape, 'DX' and 'MX' floppies, 'MT' magnetic tape,
	'RK' fixed disk, 'RM' ROM disk
    ROM 279 - same, plus 'MY' floppy and 'DW' fixed disk

    Paper tape: "B LA" (copies absolute loader from ROM), followed by
	"P" (loads tape).
    All other devices: "B" followed by device type and optional unit number.

****************************************************************************/

#include "emu.h"

#include "1801vp033.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/qbus/qbus.h"
#include "bus/rs232/rs232.h"
#include "cpu/t11/t11.h"
#include "machine/timer.h"
#include "machine/dl11.h"
#include "machine/ram.h"
#include "vm1timer.h"

#include "softlist_dev.h"


class ms1201_base_state : public driver_device
{
public:
	ms1201_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bpic(*this, "bpic")
		, m_dl11(*this, "dl11")
		, m_rs232(*this, "rs232")
		, m_qbus(*this, "qbus")
		, m_sa0(*this, "SA0")
		, m_sa1(*this, "SA1")
		, m_view(*this, "view")
	{ }

	void ms1201_base(machine_config &config);

	uint16_t trap_r(offs_t offset);
	void trap_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

private:

protected:
	required_device<t11_device> m_maincpu;
	required_device<k1801vp033_device> m_bpic;
	required_device<k1801vp065_device> m_dl11;
	required_device<rs232_port_device> m_rs232;
	required_device<qbus_device> m_qbus;
	required_ioport m_sa0;
	required_ioport m_sa1;

	memory_view m_view;

	TIMER_DEVICE_CALLBACK_MEMBER(pclk_timer);
	int m_odt_map;
};


class ms1201_01_state : public ms1201_base_state
{
public:
	ms1201_01_state(const machine_config &mconfig, device_type type, const char *tag)
		: ms1201_base_state(mconfig, type, tag)
		, m_extrom(*this, "extrom")
		, m_rombank(*this, "rombank")
	{ }

	void ms120101(machine_config &config);
	void dvk1(machine_config &config);
	void dvk2(machine_config &config);

	virtual void machine_reset() override ATTR_COLD;

	void ms1201_01_mem(address_map &map) ATTR_COLD;

	uint16_t sel_r(offs_t offset);
	void sel_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	required_device<generic_slot_device> m_extrom;

	memory_view m_rombank;

private:
	void reset_w(int state);
	uint16_t m_sel[2];
};

class ms1201_02_state : public ms1201_base_state
{
public:
	ms1201_02_state(const machine_config &mconfig, device_type type, const char *tag)
		: ms1201_base_state(mconfig, type, tag)
	{ }

	void ms120102(machine_config &config);
	void dvk3(machine_config &config);
	void dvk3m(machine_config &config);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void ms1201_02_mem(address_map &map) ATTR_COLD;

private:
	void reset_w(int state);
};


static INPUT_PORTS_START(ms1201)
	PORT_START("SA0")
	PORT_DIPNAME(0x01, 0x00, "ODT (HALT) mode")
	PORT_DIPSETTING(0x01, DEF_STR(Yes) )
	PORT_DIPSETTING(0x00, DEF_STR(No) )
	PORT_DIPNAME(0x02, 0x00, "Timer interrupt")
	PORT_DIPSETTING(0x02, DEF_STR(Yes) )
	PORT_DIPSETTING(0x00, DEF_STR(No) )

	PORT_START("SA1")
	PORT_DIPNAME(0x0f, 0x01, "Boot mode")
	PORT_DIPSETTING(0x00, "0 - start at powerfail vector 024" )
	PORT_DIPSETTING(0x01, "1 - start in ODT" )
	PORT_DIPSETTING(0x02, "2 - boot from disk" )
	PORT_DIPSETTING(0x03, "3 - start in user ROM at 0140000" )
INPUT_PORTS_END


void ms1201_01_state::ms1201_01_mem(address_map &map)
{
	map(0000000, 0177777).rw(FUNC(ms1201_01_state::trap_r), FUNC(ms1201_01_state::trap_w));

	map(0000000, 0137777).ram();
	map(0140000, 0157777).view(m_rombank);
	m_rombank[0](0140000, 0157777).ram();
	m_rombank[1](0140000, 0157777).r(m_extrom, FUNC(generic_slot_device::read_rom));
	map(0160000, 0172777).view(m_view);
	m_view[0](0160000, 0172777).rw(FUNC(ms1201_01_state::trap_r), FUNC(ms1201_01_state::trap_w));
	m_view[1](0160000, 0163777).rom().region("maincpu", 0);
	m_view[1](0164000, 0172777).rw(FUNC(ms1201_01_state::trap_r), FUNC(ms1201_01_state::trap_w));
	m_view[2](0160000, 0172777).rom().region("maincpu", 0);
	m_view[3](0160000, 0172777).rom().region("maincpu", 0);
	map(0173000, 0173777).rom().region("maincpu", 013000);
	map(0177510, 0177517).rw(m_bpic, FUNC(k1801vp033_device::bpic_read), FUNC(k1801vp033_device::bpic_write));
	map(0177560, 0177567).rw(m_dl11, FUNC(k1801vp065_device::read), FUNC(k1801vp065_device::write));
	map(0177600, 0177677).ram();
	map(0177706, 0177713).rw("vm1timer", FUNC(k1801vm1_timer_device::read), FUNC(k1801vm1_timer_device::write));
	map(0177714, 0177717).rw(FUNC(ms1201_01_state::sel_r), FUNC(ms1201_01_state::sel_w));
}

void ms1201_02_state::ms1201_02_mem(address_map &map)
{
	map(0000000, 0177777).view(m_view);

// USER mode
	m_view[0](0000000, 0177777).rw(FUNC(ms1201_02_state::trap_r), FUNC(ms1201_02_state::trap_w));
	m_view[0](0000000, 0157777).ram();
	m_view[0](0177510, 0177517).rw(m_bpic, FUNC(k1801vp033_device::bpic_read), FUNC(k1801vp033_device::bpic_write));
	m_view[0](0177560, 0177567).rw(m_dl11, FUNC(k1801vp065_device::read), FUNC(k1801vp065_device::write));

// HALT mode
	m_view[1](0000000, 0177777).rw(FUNC(ms1201_02_state::trap_r), FUNC(ms1201_02_state::trap_w));
	m_view[1](0140000, 0157777).rom().region("maincpu", 0);
	m_view[1](0170000, 0177777).ram();
}


uint16_t ms1201_01_state::sel_r(offs_t offset)
{
	if (offset == 0)
	{
		return m_sel[1];
	}

	return 0160000 | (m_sa1->read() & 3) | (m_sel[0] & 0374);
}

void ms1201_01_state::sel_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_sel[1]);
		return;
	}

	COMBINE_DATA(&m_sel[0]);

	if (m_odt_map != ((m_sel[0] >> 2) & 3))
	{
		m_odt_map = (m_sel[0] >> 2) & 3;
		m_view.select(m_odt_map);
	}
}

uint16_t ms1201_base_state::trap_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero);

	return 0xffff;
}

void ms1201_base_state::trap_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero);
}


TIMER_DEVICE_CALLBACK_MEMBER(ms1201_base_state::pclk_timer)
{
	m_maincpu->set_input_line(t11_device::HLT_LINE, BIT(m_sa0->read(), 0));
	if (BIT(m_sa0->read(), 1))
		m_maincpu->pulse_input_line(t11_device::CP2_LINE, m_maincpu->minimum_quantum_time());
}


static const z80_daisy_config daisy_chain[] =
{
	{ "dl11" },
//	{ "fdic" },
	{ "bpic" },
	{ "qbus" },
	{ nullptr }
};


void ms1201_01_state::machine_reset()
{
	// move to machine_start? see https://github.com/1801BM1/k1801/tree/master/030
	m_odt_map = 2;
	m_view.select(m_odt_map);
	m_rombank.select(m_extrom->get_rom_size() == 0 ? 0 : 1);
	m_sel[0] = m_sel[1] = 0;
}

void ms1201_01_state::reset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_dl11->reset();
		// FDIC
		m_bpic->reset();
		m_qbus->init_w();
	}
}

void ms1201_02_state::machine_start()
{
	m_qbus->set_view(m_view[0]);
}

void ms1201_02_state::machine_reset()
{
	m_odt_map = 1;
	m_view.select(m_odt_map);
}

void ms1201_02_state::reset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_dl11->reset();
		// FDIC
		m_bpic->reset();
		m_qbus->init_w();
	}
}

void ms1201_base_state::ms1201_base(machine_config &config)
{
	TIMER(config, "pclk").configure_periodic(FUNC(ms1201_base_state::pclk_timer), attotime::from_hz(50));

	QBUS(config, m_qbus, 0);
	m_qbus->set_space(m_maincpu, AS_PROGRAM);
	m_qbus->birq4().set_inputline(m_maincpu, t11_device::VEC_LINE);
	QBUS_SLOT(config, "qbus" ":1", qbus_cards, "pc11"); // actually BPIC in bidirectional mode
	QBUS_SLOT(config, "qbus" ":2", qbus_cards, nullptr); // actually FDIC
	QBUS_SLOT(config, "qbus" ":3", qbus_cards, nullptr);
	QBUS_SLOT(config, "qbus" ":4", qbus_cards, nullptr);

	K1801VP065(config, m_dl11, XTAL(4'608'000));
	m_dl11->set_rxc(9600);
	m_dl11->set_txc(9600);
	m_dl11->set_rxvec(060);
	m_dl11->set_txvec(064);
	m_dl11->txd_wr_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_dl11->txrdy_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);
	m_dl11->rxrdy_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set(m_dl11, FUNC(k1801vp065_device::rx_w));

	K1801VP033(config, m_bpic, 0);
	m_bpic->set_txvec(0200);
	m_bpic->bpic_txrdy_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);
	m_bpic->bpic_reset_wr_callback().set("centronics", FUNC(centronics_device::write_init));
	m_bpic->bpic_strobe_wr_callback().set("centronics", FUNC(centronics_device::write_strobe));
	m_bpic->bpic_pd_wr_callback().set("printdata", FUNC(output_latch_device::write));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.set_output_latch(OUTPUT_LATCH(config, "printdata"));
	centronics.busy_handler().set(m_bpic, FUNC(k1801vp033_device::bpic_write_drq)).invert();
	centronics.perror_handler().set(m_bpic, FUNC(k1801vp033_device::bpic_write_err));
}

void ms1201_01_state::ms120101(machine_config &config)
{
	ms1201_base(config);

	K1801VM1(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ms1201_01_state::ms1201_01_mem);
	downcast<k1801vm1_device *>(m_maincpu.target())->set_daisy_config(daisy_chain);
	m_maincpu->out_reset().set(FUNC(ms1201_01_state::reset_w));

	K1801VM1_TIMER(config, "vm1timer", XTAL(8'000'000) / 2);

	GENERIC_CARTSLOT(config, m_extrom, generic_plain_slot, "ms120101_rom");
	SOFTWARE_LIST(config, "cart_list").set_original("ms120101_rom");
}

void ms1201_01_state::dvk1(machine_config &config)
{
	ms120101(config);

	// FIXME add basic
	subdevice<qbus_slot_device>("qbus:2")->set_default_option(nullptr);
	subdevice<rs232_port_device>("rs232")->set_default_option("ie15");
}

void ms1201_01_state::dvk2(machine_config &config)
{
	ms120101(config);

	subdevice<rs232_port_device>("rs232")->set_default_option("ie15");
}

void ms1201_02_state::ms120102(machine_config &config)
{
	ms1201_base(config);

	K1801VM2(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ms1201_02_state::ms1201_02_mem);
	m_maincpu->set_initial_mode(0140000);
	downcast<k1801vm2_device *>(m_maincpu.target())->set_daisy_config(daisy_chain);
	m_maincpu->out_reset().set(FUNC(ms1201_02_state::reset_w));
	m_maincpu->out_bankswitch().set([this] (int state) {
		if (m_odt_map != state) { m_view.select(state); m_odt_map = state; }
	});
	m_maincpu->in_sel1().set_ioport("SA1");
}

void ms1201_02_state::dvk3(machine_config &config)
{
	ms120102(config);

	// FIXME replace with ksm
	subdevice<rs232_port_device>("rs232")->set_default_option("ie15");
	subdevice<qbus_slot_device>("qbus:1")->set_default_option("kgd");
	subdevice<qbus_slot_device>("qbus:2")->set_default_option("mx");
}

void ms1201_02_state::dvk3m(machine_config &config)
{
	ms120102(config);

	// FIXME replace with kcgd
	subdevice<rs232_port_device>("rs232")->set_default_option("ie15");
	subdevice<qbus_slot_device>("qbus:1")->set_default_option("dw");
	subdevice<qbus_slot_device>("qbus:2")->set_default_option("my");
}


ROM_START(ms120101)
	ROM_REGION16_LE(020000, "maincpu", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("054")
	ROM_SYSTEM_BIOS(0, "000", "mask 000 (1981)")
	ROMX_LOAD("000.dat", 0, 020000, CRC(7c8d149b) SHA1(8603c99d99237d3dec4a022118f4ad0af358e899), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "031", "mask 031 (198x)")
	ROMX_LOAD("031.dat", 0, 020000, CRC(4631ffe6) SHA1(1584c674ca9728be51040fd04b0a22c938fe57b1), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "054", "mask 054 (198x)")
	ROMX_LOAD("054.bin", 0, 020000, CRC(89254082) SHA1(9aa3ad780d881915d27fea7621d474c4dda2a6d2), ROM_BIOS(2))
ROM_END

ROM_START(ms120102)
	ROM_REGION16_LE(020000, "maincpu", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("279")
	ROM_SYSTEM_BIOS(0, "055", "mask 055 (1985)")
	ROMX_LOAD("055.dat", 0, 020000, CRC(11c28e48) SHA1(a328ca6de54630cd81659977fd2a0e7f0ad168fd), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "279", "mask 279 (1991)")
	ROMX_LOAD("279.dat", 0, 020000, CRC(26932af0) SHA1(e2d863085fc818ebcb08dcccbc5470106dd8eb33), ROM_BIOS(1))
ROM_END

#define rom_dvk1  rom_ms120101
#define rom_dvk2  rom_ms120101
#define rom_dvk3  rom_ms120102
#define rom_dvk3m rom_ms120102

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT MACHINE   INPUT   INIT                         COMPANY  FULLNAME       FLAGS */
COMP( 1983, ms120101, 0,      0,     ms120101, ms1201, ms1201_01_state, empty_init, "USSR",  "MS 1201.01",  MACHINE_NOT_WORKING)
COMP( 1985, ms120102, 0,      0,     ms120102, ms1201, ms1201_02_state, empty_init, "USSR",  "MS 1201.02",  MACHINE_NOT_WORKING)
// ie15, rom 013 (basic), no storage
COMP( 1983, dvk1,   ms120101, 0,     dvk1,     ms1201, ms1201_01_state, empty_init, "USSR",  "DVK-1",       MACHINE_NOT_WORKING)
// ie15, kgd, rom 093? (focal), no storage, lan
//MP( 1983, dvk1msh, ms120101, 0,    ms120101, ms1201, ms1201_01_state, empty_init, "USSR",  "DVK-1MSH",    MACHINE_NOT_WORKING)
// ie15, dx floppy
COMP( 1983, dvk2,   ms120101, 0,     dvk2,     ms1201, ms1201_01_state, empty_init, "USSR",  "DVK-2",       MACHINE_NOT_WORKING)
// ksm, kgd, mx floppy
COMP( 1983, dvk3,   ms120102, 0,     dvk3,     ms1201, ms1201_02_state, empty_init, "USSR",  "DVK-3",       MACHINE_NOT_WORKING)
// kcgd, my floppy, dw disk
COMP( 1983, dvk3m,  ms120102, 0,     dvk3m,    ms1201, ms1201_02_state, empty_init, "USSR",  "DVK-3M",      MACHINE_NOT_WORKING)
