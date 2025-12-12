// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "cpu/mcs51/i80c52.h"

#define LOG_SERIAL (1U << 1)
#define VERBOSE (LOG_SERIAL)
#include "logmacro.h"

namespace {

class keyfox10_state : public driver_device
{
public:
    keyfox10_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag)
        , m_maincpu(*this, "maincpu")
    { }

    void keyfox10(machine_config &config);

private:
    void program_map(address_map &map) ATTR_COLD;
    void data_map(address_map &map) ATTR_COLD;

    void port0_w(u8 data);
    void port1_w(u8 data);
    void port2_w(u8 data);
    void port3_w(u8 data);
    u8 port0_r();
    u8 port1_r();
    u8 port2_r();
    u8 port3_r();

    required_device<i80c32_device> m_maincpu;
    u8 m_port0 = 0xff;
    u8 m_port1 = 0xff;
    u8 m_port2 = 0xff;
    u8 m_port3 = 0xff;
};

void keyfox10_state::program_map(address_map &map)
{
    // 80C32 has 16-bit address space; ROM banking for 128KB ROM TBD
    map(0x0000, 0xffff).rom().region("program", 0);
}

void keyfox10_state::data_map(address_map &map)
{
    map(0x0000, 0x1fff).ram();  // 8KB RAM
}

u8 keyfox10_state::port0_r()
{
    LOGMASKED(LOG_SERIAL, "P0 read: 0x%02X\n", m_port0);
    return m_port0;
}

u8 keyfox10_state::port1_r()
{
    LOGMASKED(LOG_SERIAL, "P1 read: 0x%02X\n", m_port1);
    return m_port1;
}

u8 keyfox10_state::port2_r()
{
    LOGMASKED(LOG_SERIAL, "P2 read: 0x%02X\n", m_port2);
    return m_port2;
}

u8 keyfox10_state::port3_r()
{
    LOGMASKED(LOG_SERIAL, "P3 read: 0x%02X\n", m_port3);
    return m_port3;
}

void keyfox10_state::port0_w(u8 data)
{
    if (m_port0 != data)
        LOGMASKED(LOG_SERIAL, "P0 write: 0x%02X\n", data);
    m_port0 = data;
}

void keyfox10_state::port1_w(u8 data)
{
    if (m_port1 != data)
        LOGMASKED(LOG_SERIAL, "P1 write: 0x%02X\n", data);
    m_port1 = data;
}

void keyfox10_state::port2_w(u8 data)
{
    if (m_port2 != data)
        LOGMASKED(LOG_SERIAL, "P2 write: 0x%02X\n", data);
    m_port2 = data;
}

void keyfox10_state::port3_w(u8 data)
{
    if (m_port3 != data)
        LOGMASKED(LOG_SERIAL, "P3 write: 0x%02X\n", data);
    m_port3 = data;
}

static INPUT_PORTS_START(keyfox10)
INPUT_PORTS_END

void keyfox10_state::keyfox10(machine_config &config)
{
    I80C32(config, m_maincpu, 16_MHz_XTAL);
    m_maincpu->set_addrmap(AS_PROGRAM, &keyfox10_state::program_map);
    m_maincpu->set_addrmap(AS_DATA, &keyfox10_state::data_map);
    m_maincpu->port_in_cb<0>().set(FUNC(keyfox10_state::port0_r));
    m_maincpu->port_in_cb<1>().set(FUNC(keyfox10_state::port1_r));
    m_maincpu->port_in_cb<2>().set(FUNC(keyfox10_state::port2_r));
    m_maincpu->port_in_cb<3>().set(FUNC(keyfox10_state::port3_r));
    m_maincpu->port_out_cb<0>().set(FUNC(keyfox10_state::port0_w));
    m_maincpu->port_out_cb<1>().set(FUNC(keyfox10_state::port1_w));
    m_maincpu->port_out_cb<2>().set(FUNC(keyfox10_state::port2_w));
    m_maincpu->port_out_cb<3>().set(FUNC(keyfox10_state::port3_w));
}

ROM_START(keyfox10)
    ROM_REGION(0x20000, "program", 0)
    ROM_LOAD("kf10_ic27_v2.bin", 0x0000, 0x20000, CRC(c04e40a9) SHA1(668d40761658f1863ab028ed317141839d1075ac))
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME      FLAGS
SYST( 1990, keyfox10, 0,      0,      keyfox10, keyfox10, keyfox10_state, empty_init, "Wersi", "Keyfox 10",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
