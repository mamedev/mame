// license:BSD-3-Clause
// copyright-holders:David Haywood

// TV Word Processor, with printer

#include "emu.h"
#include "cpu/nec/v25.h"
#include "softlist_dev.h"

namespace {

class tvdear_state : public driver_device
{
public:
	tvdear_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "systemcpu")
	{
	}

	void tvdear(machine_config &config);

private:
	void output_500_w(u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<v25_device> m_maincpu;
};


void tvdear_state::output_500_w(u8 data)
{
	if (data != 0)
		logerror("%s: OUT 500h, %02Xh\n", machine().describe_context(), data);
}

void tvdear_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0x10000, 0x17fff).ram();
	map(0x80000, 0xfffff).rom().region("maincpu", 0x080000);
}

void tvdear_state::io_map(address_map &map)
{
	map(0x0500, 0x0500).w(FUNC(tvdear_state::output_500_w));
}

static INPUT_PORTS_START(tvdear)
INPUT_PORTS_END

void tvdear_state::tvdear(machine_config &config)
{
	V25(config, m_maincpu, 16000000); // NEC D70320DGJ-8; XTAL marked 16AKSS5HT
	m_maincpu->set_addrmap(AS_PROGRAM, &tvdear_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &tvdear_state::io_map);

	//MB90076(config, "tvvc", 14318181); // XTAL marked 14AKSS5JT

	SOFTWARE_LIST(config, "cart_list").set_original("tvdear");
}

ROM_START(tvdear)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD("d23c160000.u5", 0x00000, 0x200000, CRC(41ec9890) SHA1(20cfdfec7eeb39a9ce971f23fdc97b42a5d68301) )
ROM_END

} // anonymous namespace

CONS( 1995, tvdear,  0,          0,  tvdear,  tvdear, tvdear_state, empty_init, "Takara", "TV Dear Multi Word Processor", MACHINE_IS_SKELETON | MACHINE_NODEVICE_PRINTER ) // テレビディア マルチワープロ
