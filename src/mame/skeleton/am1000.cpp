// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Alpha Micro AM-1000 system.

    This system was sold in various configurations beginning in 1982.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
//#include "machine/6840ptm.h"
//#include "machine/6850acia.h"
//#include "machine/com8116.h"
//#include "machine/msm5832.h"


namespace {

class am1000_state : public driver_device
{
public:
	am1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_mainprom(*this, "mainprom")
		, m_mainram(*this, "mainram")
	{
	}

	void am1000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u16 rom_ram_r(offs_t offset, u16 mem_mask);
	void control_w(u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	required_device<m68000_device> m_maincpu;
	required_device<z80_device> m_subcpu;
	required_region_ptr<u16> m_mainprom;
	required_shared_ptr<u16> m_mainram;

	bool m_ram_enabled = false;
};


void am1000_state::machine_start()
{
	// assumes it can make an address mask from m_mainprom.length() - 1
	assert(!(m_mainprom.length() & (m_mainprom.length() - 1)));

	save_item(NAME(m_ram_enabled));
}

void am1000_state::machine_reset()
{
	m_ram_enabled = false;
}


u16 am1000_state::rom_ram_r(offs_t offset, u16 mem_mask)
{
	if (m_ram_enabled)
		return m_mainram[offset];
	else
		return m_mainprom[offset & (m_mainprom.length() - 1)];
}

void am1000_state::control_w(u8 data)
{
	m_ram_enabled = true;
}


void am1000_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).r(FUNC(am1000_state::rom_ram_r)).writeonly().share(m_mainram); // TMM41256P-12 x16
	map(0x800000, 0x801fff).rom().region("mainprom", 0);
	map(0xfffe00, 0xfffe00).w(FUNC(am1000_state::control_w));
}

void am1000_state::sub_map(address_map &map)
{
	map(0x0000, 0x07ff).rom().region("subprom", 0);
	map(0x4000, 0x4fff).ram(); // NEC D4016C-2 x2
}


static INPUT_PORTS_START(am1000)
INPUT_PORTS_END


void am1000_state::am1000(machine_config &config)
{
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // MK68000P-8B
	m_maincpu->set_addrmap(AS_PROGRAM, &am1000_state::main_map);

	Z80(config, m_subcpu, 16_MHz_XTAL / 4); // NEC D780C-1
	m_subcpu->set_addrmap(AS_PROGRAM, &am1000_state::sub_map);
}


ROM_START(am1000)
	ROM_REGION16_BE(0x2000, "mainprom", 0) // "© COPYRIGHT 1988 ALPHA MICRO" on labels; "COPR. 1985 ALMI" in byte-swapped data
	ROM_LOAD16_BYTE("169-13_c00.u83", 0x0000, 0x1000, CRC(1d9cc8c0) SHA1(fced7bde6dbee4b54050bc7a91fc8eb74a659823))
	ROM_LOAD16_BYTE("169-12_c00.u82", 0x0001, 0x1000, CRC(72bca8ae) SHA1(67d7a13ea2d630af8e6aefeb1a30b2de8cf6012a))
	// Adjacent 24-pin sockets at U100 and U101 are both unpopulated

	ROM_REGION(0x800, "subprom", 0)
	ROM_LOAD("169-05_b00.u162", 0x000, 0x800, CRC(e0f8bb40) SHA1(ac2531994eb90447d320401948af07723e51c8f9))
ROM_END

} // anonymous namespace


COMP(1988, am1000, 0, 0, am1000, am1000, am1000_state, empty_init, "Alpha Micro", "AM-1000", MACHINE_IS_SKELETON)
