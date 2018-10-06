// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Intel MCS-51 System Design Kit (SDK-51).

*******************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/mcs48/mcs48.h"
//#include "imagedev/cassette.h"
#include "machine/bankdev.h"
#include "machine/i8155.h"
#include "machine/i8243.h"
#include "machine/i8251.h"

class sdk51_state : public driver_device
{
public:
	sdk51_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_progmem(*this, "progmem")
		, m_datamem(*this, "datamem")
		, m_mem0(*this, "mem0")
		, m_usart(*this, "usart")
		, m_cycles(*this, "cycles")
	{
	}

	void sdk51(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void psen_map(address_map &map);
	void movx_map(address_map &map);
	void progmem_map(address_map &map);
	void datamem_map(address_map &map);
	void mem0_map(address_map &map);

	DECLARE_READ8_MEMBER(psen_r);
	DECLARE_READ8_MEMBER(datamem_r);
	DECLARE_WRITE8_MEMBER(datamem_w);

	u8 brkmem_r(offs_t offset);
	void brkmem_w(offs_t offset, u8 data);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_progmem;
	required_device<address_map_bank_device> m_datamem;
	required_device<address_map_bank_device> m_mem0;
	required_device<i8251_device> m_usart;
	required_region_ptr<u8> m_cycles;
};

READ8_MEMBER(sdk51_state::psen_r)
{
	return m_progmem->read8(space, offset);
}

READ8_MEMBER(sdk51_state::datamem_r)
{
	return m_datamem->read8(space, offset);
}

WRITE8_MEMBER(sdk51_state::datamem_w)
{
	m_datamem->write8(space, offset, data);
}

u8 sdk51_state::brkmem_r(offs_t offset)
{
	return 0;
}

void sdk51_state::brkmem_w(offs_t offset, u8 data)
{
	m_mem0->set_bank(0);
}

void sdk51_state::psen_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(sdk51_state::psen_r));
}

void sdk51_state::movx_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(sdk51_state::datamem_r), FUNC(sdk51_state::datamem_w));
}

void sdk51_state::progmem_map(address_map &map)
{
	map(0x0000, 0x1fff).m(m_mem0, FUNC(address_map_bank_device::amap8));
	map(0x2000, 0x3fff).ram();
	map(0xe000, 0xffff).rom().region("monitor", 0);
}

void sdk51_state::datamem_map(address_map &map)
{
	progmem_map(map);
	map(0xa000, 0xa001).mirror(0xffe).rw("upi", FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0xb000, 0xb0ff).mirror(0x700).rw("io", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0xb800, 0xb807).mirror(0x7f8).rw("io", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xc000, 0xdfff).rw(FUNC(sdk51_state::brkmem_r), FUNC(sdk51_state::brkmem_w));
}

void sdk51_state::mem0_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2fff).mirror(0x1000).rom().region("monitor", 0);
}

static INPUT_PORTS_START(sdk51)
INPUT_PORTS_END

void sdk51_state::machine_start()
{
}

void sdk51_state::machine_reset()
{
	m_mem0->set_bank(1);
}

void sdk51_state::sdk51(machine_config &config)
{
	I8031(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sdk51_state::psen_map);
	m_maincpu->set_addrmap(AS_IO, &sdk51_state::movx_map);

	ADDRESS_MAP_BANK(config, m_progmem);
	m_progmem->set_addrmap(0, &sdk51_state::progmem_map);
	m_progmem->set_data_width(8);
	m_progmem->set_addr_width(16);

	ADDRESS_MAP_BANK(config, m_datamem);
	m_datamem->set_addrmap(0, &sdk51_state::datamem_map);
	m_datamem->set_data_width(8);
	m_datamem->set_addr_width(16);

	ADDRESS_MAP_BANK(config, m_mem0);
	m_mem0->set_addrmap(0, &sdk51_state::mem0_map);
	m_mem0->set_data_width(8);
	m_mem0->set_addr_width(14);
	m_mem0->set_stride(0x2000);

	upi41_cpu_device &upi(I8041(config, "upi", 6_MHz_XTAL));
	upi.p2_in_cb().set("upiexp", FUNC(i8243_device::p2_r));
	upi.p2_out_cb().set("upiexp", FUNC(i8243_device::p2_w));
	upi.prog_out_cb().set("upiexp", FUNC(i8243_device::prog_w));
	upi.t1_in_cb().set(m_usart, FUNC(i8251_device::txrdy_r));

	I8243(config, "upiexp");

	i8155_device &io(I8155(config, "io", 6_MHz_XTAL / 3));
	io.out_to_callback().set(m_usart, FUNC(i8251_device::write_txc));
	io.out_to_callback().append(m_usart, FUNC(i8251_device::write_txc));

	I8251(config, m_usart, 6_MHz_XTAL / 3);
	m_usart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_usart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_usart, FUNC(i8251_device::write_cts));
}

ROM_START(sdk51)
	ROM_REGION(0x2000, "monitor", 0) // "SDK-51 MONITOR VER. 1.03"
	ROM_LOAD("u59-e000.bin", 0x0000, 0x1000, CRC(cc6c7b05) SHA1(ead75920347ff19487e730e90e1e1f7207d44601))
	ROM_LOAD("u60-f000.bin", 0x1000, 0x1000, CRC(da6e664d) SHA1(18416106307f37dba6dbb789f3d39fe6d5294755))

	ROM_REGION(0x0400, "upi", 0)
	ROM_LOAD("u41-8041a.bin", 0x000, 0x400, CRC(02f38b69) SHA1(ab2ac73b69b3297572583242ed5bd717eb116c37))

	ROM_REGION(0x0200, "cycles", 0)
	ROM_LOAD("u63-3622a.bin", 0x000, 0x200, CRC(85cbd498) SHA1(f0214b6d02d6d153b5fafd9adf5a23013373c9c4)) // pin 9 output stuck high but not used
ROM_END

COMP(1981, sdk51, 0, 0, sdk51, sdk51, sdk51_state, empty_init, "Intel", "MCS-51 System Design Kit", MACHINE_IS_SKELETON)
