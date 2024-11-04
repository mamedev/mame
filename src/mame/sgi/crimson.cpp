// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI Crimson deskside skeleton driver

    To Do: Everything

    Memory map:
    1fc00000 - 1fc7ffff      Boot ROM

**********************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/mips/mips3.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"

#define LOG_UNKNOWN     (1U << 1)
#define LOG_ALL         (LOG_UNKNOWN)

#define VERBOSE         (0)
#include "logmacro.h"


namespace {

class crimson_state : public driver_device
{
public:
	crimson_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_duart(*this, "duart")
	{
	}

	void crimson(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	u8 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u8 data);
	u8 duart_r(offs_t offset);
	void duart_w(offs_t offset, u8 data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<r4000be_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<scn2681_device> m_duart;

	std::unique_ptr<u8[]> m_nvram_data;
};

void crimson_state::machine_start()
{
	m_nvram_data = std::make_unique<u8[]>(0x800);
	m_nvram->set_base(m_nvram_data.get(), 0x800);
	save_pointer(NAME(m_nvram_data), 0x800);
}

u8 crimson_state::nvram_r(offs_t offset)
{
	return m_nvram_data[offset];
}

void crimson_state::nvram_w(offs_t offset, u8 data)
{
	m_nvram_data[offset] = data;
}

u8 crimson_state::duart_r(offs_t offset)
{
	return m_duart->read(offset >> 1);
}

void crimson_state::duart_w(offs_t offset, u8 data)
{
	m_duart->write(offset >> 1, data);
}

void crimson_state::mem_map(address_map &map)
{
	map(0x00000000, 0x00002fff).ram();
	map(0x00500000, 0x009fffff).ram();
	map(0x17f10000, 0x17f13fff).rw(FUNC(crimson_state::nvram_r), FUNC(crimson_state::nvram_w)).umask64(0xff00000000000000LLU);
	map(0x1f600000, 0x1f60000f).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask64(0xff000000ff000000LLU);
	map(0x1fa00000, 0x1fa000ff).rw(FUNC(crimson_state::duart_r), FUNC(crimson_state::duart_w)).umask64(0xff00000000000000LLU);
	map(0x1fc00000, 0x1fc7ffff).rom().region("user1", 0);
}

static INPUT_PORTS_START( crimson )
INPUT_PORTS_END

void crimson_state::crimson(machine_config &config)
{
	R4000BE(config, m_maincpu, 50000000*2);
	m_maincpu->set_icache_size(32768);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &crimson_state::mem_map);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0); // CXK5816PN-15 + battery?

	SCN2681(config, m_duart, 3.6864_MHz_XTAL); // TODO: there are two more of these
	m_duart->a_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));

	PIT8254(config, "pit");

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
}

ROM_START( crimson )
	ROM_REGION64_BE( 0x80000, "user1", 0 )
	ROMX_LOAD( "ip17prom.070-081x-005.bin", 0x000000, 0x080000, CRC(d62e8c8e) SHA1(b335213ecfd02ca3185b6ba1874a8b76f908c68b), ROM_GROUPDWORD )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY                 FULLNAME                               FLAGS
COMP( 1992, crimson,  0,      0,      crimson,  crimson, crimson_state, empty_init, "Silicon Graphics Inc", "Crimson (R4000, 100MHz, Ver. 4.0.3)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
