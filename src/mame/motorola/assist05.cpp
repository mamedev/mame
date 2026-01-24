// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Motorola ASSIST05 RS-232 debug monitor

    At present, the only hardware hooked up not described in the M6805
    HMOS/M146805 CMOS Family User's Manual is a block of RAM. This makes
    the breakpointing (B) and tracing (T) commands more useful.

    Usage note: only uppercase input is accepted.

    Known bugs:
    — Tracing doesn't work (timer)

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6805/m68705.h"
#include "machine/6850acia.h"
#include "machine/mc14411.h"

namespace {

class assist05_state : public driver_device
{
public:
	assist05_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_acia(*this, "acia")
		, m_brg(*this, "brg")
	{
	}

	void assist05(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(baud_rate_changed);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<acia6850_device> m_acia;
	required_device<mc14411_device> m_brg;
};

void assist05_state::machine_start()
{
	m_acia->write_cts(0);
	m_acia->write_dcd(0);

	m_brg->rate_select_w(mc14411_device::RSB);
}

INPUT_CHANGED_MEMBER(assist05_state::baud_rate_changed)
{
	m_brg->timer_disable_all();
	if (!BIT(newval, 7))
		m_brg->timer_enable(mc14411_device::TIMER_F1, true);
	else if (!BIT(newval, 6))
		m_brg->timer_enable(mc14411_device::TIMER_F3, true);
	else if (!BIT(newval, 5))
		m_brg->timer_enable(mc14411_device::TIMER_F5, true);
	else if (!BIT(newval, 4))
		m_brg->timer_enable(mc14411_device::TIMER_F7, true);
	else if (!BIT(newval, 3))
		m_brg->timer_enable(mc14411_device::TIMER_F8, true);
	else if (!BIT(newval, 2))
		m_brg->timer_enable(mc14411_device::TIMER_F9, true);
	else if (!BIT(newval, 1))
		m_brg->timer_enable(mc14411_device::TIMER_F11, true);
	else if (!BIT(newval, 0))
		m_brg->timer_enable(mc14411_device::TIMER_F13, true);
}

static INPUT_PORTS_START(assist05)
	PORT_START("BAUD")
	PORT_DIPNAME(0xff, 0x7f, "Baud Rate") PORT_DIPLOCATION("SW:1,2,3,4,5,6,7,8") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(assist05_state::baud_rate_changed), 0)
	PORT_DIPSETTING(0xfe, "110")
	PORT_DIPSETTING(0xfd, "150")
	PORT_DIPSETTING(0xfb, "300")
	PORT_DIPSETTING(0xf7, "600")
	PORT_DIPSETTING(0xef, "1200")
	PORT_DIPSETTING(0xdf, "2400")
	PORT_DIPSETTING(0xbf, "4800")
	PORT_DIPSETTING(0x7f, "9600")
INPUT_PORTS_END

void assist05_state::mem_map(address_map &map)
{
	map(0x0080, 0x17f7).ram();
	map(0x17f8, 0x17f9).mirror(6).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x1800, 0x1fff).rom().region("monitor", 0);
}

void assist05_state::assist05(machine_config &config)
{
	m146805e2_device &maincpu(M146805E2(config, "maincpu", 5_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &assist05_state::mem_map);

	ACIA6850(config, m_acia);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	MC14411(config, m_brg, 1.8432_MHz_XTAL);
	m_brg->out_f<1>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_brg->out_f<1>().append(m_acia, FUNC(acia6850_device::write_rxc));
	m_brg->out_f<3>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_brg->out_f<3>().append(m_acia, FUNC(acia6850_device::write_rxc));
	m_brg->out_f<5>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_brg->out_f<5>().append(m_acia, FUNC(acia6850_device::write_rxc));
	m_brg->out_f<7>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_brg->out_f<7>().append(m_acia, FUNC(acia6850_device::write_rxc));
	m_brg->out_f<8>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_brg->out_f<8>().append(m_acia, FUNC(acia6850_device::write_rxc));
	m_brg->out_f<9>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_brg->out_f<9>().append(m_acia, FUNC(acia6850_device::write_rxc));
	m_brg->out_f<11>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_brg->out_f<11>().append(m_acia, FUNC(acia6850_device::write_rxc));
	m_brg->out_f<13>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_brg->out_f<13>().append(m_acia, FUNC(acia6850_device::write_rxc));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
}

ROM_START(assist05)
	ROM_REGION(0x800, "monitor", 0)
	ROM_LOAD("assist05.bin", 0x000, 0x800, CRC(5e89509e) SHA1(cfdbbf499740fed87fdd8b8056d2668dd99dd540)) // from listing in M6805/M146805 Family User's Manual
ROM_END

} // anonymous namespace

SYST(1980, assist05, 0, 0, assist05, assist05, assist05_state, empty_init, "Motorola", "ASSIST05", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
