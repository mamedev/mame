// license:BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Qume QVT-70 terminal

    Hardware:
    - Z80 (Z8040008VSC)
    - Z80 DART (Z0847006PSC)
    - QUME 303489-01 QFP144
    - DTC 801000-02 QFP100
    - ROM 128k + 64k
    - CXK5864CM-70LL (8k, next to ROMs)
    - W242575-70LL (32k) + 5x CXK5864CM-70LL (8k)
    - DS1231
    - Beeper + Battery
    - XTAL unreadable

    Features:
    - 65 hz with 16x16 characters
    - 78 hz with 16x13 characters
    - 64 background/foreground colors
    - 80/132 columns

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/z80dart.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"

class qvt70_state : public driver_device
{
public:
	qvt70_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bank(*this, "bank")
	{ }

	void qvt70(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bank;

	void mem_map(address_map &map);
	void bank_map(address_map &map);
	void io_map(address_map &map);

	DECLARE_WRITE8_MEMBER(bankswitch_w);
};

void qvt70_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).m(m_bank, FUNC(address_map_bank_device::amap8));
	map(0x8000, 0x8000).w(FUNC(qvt70_state::bankswitch_w));
	map(0xa000, 0xbfff).ram();
//  map(0xc000, 0xffff).ram();
}

void qvt70_state::bank_map(address_map &map)
{
	map(0x00000, 0x07fff).rom().region("maincpu", 0x00000);
	map(0x08000, 0x0ffff).rom().region("maincpu", 0x08000);
	map(0x10000, 0x17fff).rom().region("maincpu", 0x10000);
	map(0x18000, 0x1ffff).rom().region("maincpu", 0x18000);
	map(0x20000, 0x27fff).rom().region("maincpu", 0x20000);
	map(0x28000, 0x2ffff).rom().region("maincpu", 0x28000);
	map(0x30000, 0x37fff).ram();
//  map(0x38000, 0x3ffff)
}

void qvt70_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	// 07-54
	// 60-6f
	map(0x80, 0x83).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
}

static INPUT_PORTS_START( qvt70 )
INPUT_PORTS_END

WRITE8_MEMBER(qvt70_state::bankswitch_w)
{
	logerror("bankswitch_w: %02x\n", data);

	// 765----- unknown
	// ---43--- bankswitching?
	// -----21- unknown
	// -------0 bankswitching?

	// likely wrong
	int bank = ((data >> 2) & 0x06) | BIT(data, 0);
	logerror("bank = %02x\n", bank);

	if (bank < 6)
		m_bank->set_bank(bank);
}

void qvt70_state::machine_start()
{
}

void qvt70_state::machine_reset()
{
}

void qvt70_state::qvt70(machine_config &config)
{
	Z80(config, m_maincpu, 8'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &qvt70_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &qvt70_state::io_map);

	ADDRESS_MAP_BANK(config, m_bank);
	m_bank->set_map(&qvt70_state::bank_map);
	m_bank->set_data_width(8);
	m_bank->set_addr_width(18);
	m_bank->set_stride(0x8000);

	Z80DART(config, "dart", 4'000'000);

	// 25-pin (dcd, rxd, txd, dtr, dsr, rts, cts and current loop)
	RS232_PORT(config, "serial1", default_rs232_devices, nullptr);

	// 9-pin (dcd, rxd, txd, dtr, dsr, rts, cts)
	RS232_PORT(config, "serial2", default_rs232_devices, nullptr);

	CENTRONICS(config, "parallel", centronics_devices, nullptr);
}

ROM_START( qvt70 )
	ROM_REGION(0x30000, "maincpu", 0)
	ROM_LOAD( "251513-03_revj.u11", 0x00000, 0x20000, CRC(c56796fe) SHA1(afe024ff93d5e75dc18041219d61e1a22fc6d883) ) // 251513-03  C/S:D1DA  95' REV.J (checksum matches)
	ROM_LOAD( "251513-04_revj.u12", 0x20000, 0x10000, CRC(3960bbd5) SHA1(9db306cef09be21ff43c081ebe11e9b46f617861) ) // 251513-04  C/S:18D0  95' REV.J (checksum matches)
ROM_END

COMP( 1992, qvt70, 0, 0, qvt70, qvt70, qvt70_state, empty_init, "Qume", "QVT-70", MACHINE_IS_SKELETON )
