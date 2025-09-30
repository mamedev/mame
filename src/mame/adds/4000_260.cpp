// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADDS 4000/260

    ASCII/ANSI terminal

    Hardware:
    - P-80C32-16
    - KM622560LP-7L (32k RAM)
    - CY6225LL-70 (32k RAM)
    - LSI Victor 006-9802760 REV B
    - KM622560LP-7L x2 (32k RAM x2)
    - 16 MHz XTAL, 44.976 MHz XTAL

    TODO:
    - Almost everything

    Notes:
    - It takes a while for it to populate the chargen RAM
    - Other models in this line: 4000/260C, 4000/260LF, 4000/260LFC
    - Sold under the ADDS brand, but ADDS was part of SunRiver Data Systems
      by then, which became Boundless Technologies.

***************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"

#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class _4000_260_state : public driver_device
{
public:
	_4000_260_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_rombank(*this, "rombank"),
		m_rombase(*this, "rombase"),
		m_ramview(*this, "ramview")
	{ }

	void _4000_260(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i80c32_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_memory_bank m_rombank;
	memory_view m_rombase;
	memory_view m_ramview;

	void mem_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	// uint8_t unk_08_r();
	void unk_09_w(uint8_t data);
	// uint8_t unk_16_r();
	void unk_1a_w(uint8_t data);
	void unk_23_w(uint8_t data);
	void rombank_w(uint8_t data);

	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);

	uint8_t m_unk_23 = 0;

	uint16_t m_ram_offset = 0;

	std::unique_ptr<uint8_t[]> m_chargen;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void _4000_260_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).view(m_rombase);
	m_rombase[0](0x0000, 0x7fff).rom().region("maincpu", 0x00000);
	m_rombase[0](0x8000, 0xffff).bankr(m_rombank);
	m_rombase[1](0x0000, 0x7fff).rom().region("maincpu", 0x20000);
	m_rombase[1](0x8000, 0xffff).bankr(m_rombank);
}

void _4000_260_state::ext_map(address_map &map)
{
	map(0x0000, 0x00ff).view(m_ramview);
	m_ramview[0](0x0008, 0x0008).portr("08"); // r(FUNC(_4000_260_state::unk_08_r));
	m_ramview[0](0x0009, 0x0009).w(FUNC(_4000_260_state::unk_09_w));
	m_ramview[0](0x0010, 0x0010).unmaprw(); // keyboard data?
	m_ramview[0](0x0013, 0x0013).unmaprw(); // keyboard related?
	m_ramview[0](0x0016, 0x0016).portr("16"); // r(FUNC(_4000_260_state::unk_16_r));
	m_ramview[0](0x001a, 0x001a).w(FUNC(_4000_260_state::unk_1a_w));
	m_ramview[0](0x0023, 0x0023).w(FUNC(_4000_260_state::unk_23_w));
	m_ramview[0](0x0026, 0x0026).w(FUNC(_4000_260_state::rombank_w));
	m_ramview[1](0x0000, 0x00ff).rw(FUNC(_4000_260_state::ram_r), FUNC(_4000_260_state::ram_w));
	map(0x8000, 0xffff).ram();
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( _4000_260 )
	PORT_START("08")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "08:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "08:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "08:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "08:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "08:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "08:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "08:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "08:8")

	PORT_START("16")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "16:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "16:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "16:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "16:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "16:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "16:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "16:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "16:8")
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

static const gfx_layout char_layout_8x16 =
{
	8, 16,
	512,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	8*16
};

static GFXDECODE_START( chars )
	GFXDECODE_RAM(nullptr, 0, char_layout_8x16, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

#if 0
uint8_t _4000_260_state::unk_08_r()
{
	logerror("unk_08_r\n");
	return 0xc0;
}
#endif

void _4000_260_state::unk_09_w(uint8_t data)
{
	// 7-------  beeper?
	// -6543210  unknown (volume?)

	logerror("unk_09_w: %02x\n", data);
}

#if 0
uint8_t _4000_260_state::unk_16_r()
{
	logerror("unk_16_r\n");
	return 0x80;
}
#endif

void _4000_260_state::unk_1a_w(uint8_t data)
{
	// 7-------  ram write enable?
	// -6543210  ram address high bits

	logerror("unk_1a_w: %02x\n", data);

	m_ram_offset = (data & 0x7f) << 8;

	if (BIT(data, 7))
	{
		// switch to ram when bit 7 is set?
		m_ramview.select(1);
	}
}

void _4000_260_state::unk_23_w(uint8_t data)
{
	logerror("unk_23_w: %02x\n", data);
	m_unk_23 = data;
}

void _4000_260_state::rombank_w(uint8_t data)
{
	// 7-------  rom address line a17
	// 765-----  rom bank select
	// ---4----  always 1?
	// ----3210  always 0?

	m_rombase.select(BIT(data, 7));
	m_rombank->set_entry(data >> 5);

	logerror("bank select: %d\n", data >> 5);
}

uint8_t _4000_260_state::ram_r(offs_t offset)
{
	offs_t addr = m_ram_offset | offset;
	uint8_t data = m_chargen[addr];

	logerror("ram_r %04x = %02x\n", addr, data);

	// after each read, switch back to the asic?
	m_ramview.select(0);

	return data;
}

void _4000_260_state::ram_w(offs_t offset, uint8_t data)
{
	offs_t addr = m_ram_offset | offset;
	m_chargen[addr] = data;

	logerror("ram_w: %04x = %02x\n", addr, data);

	m_gfxdecode->gfx(0)->mark_dirty(addr / 16);
}

void _4000_260_state::machine_start()
{
	m_rombank->configure_entries(0, 7, memregion("maincpu")->base() + 0x8000, 0x8000);
	m_chargen = make_unique_clear<uint8_t[]>(0x8000);

	m_gfxdecode->gfx(0)->set_source(&m_chargen[0]);

	// register for save states
	save_item(NAME(m_unk_23));
	save_item(NAME(m_ram_offset));
	save_pointer(NAME(m_chargen), 0x8000);
}

void _4000_260_state::machine_reset()
{
	m_rombase.select(0);
	m_rombank->set_entry(0);
	m_ramview.select(0);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void _4000_260_state::_4000_260(machine_config &config)
{
	I80C32(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &_4000_260_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &_4000_260_state::ext_map);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GFXDECODE(config, m_gfxdecode, "palette", chars);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( 4000_260 )
	ROM_REGION(0x40000, "maincpu", 0)
	// 598-0010669 3.16 SunRiver Data Systems 1995
	ROM_LOAD("4000_260.bin", 0x00000, 0x40000, CRC(b957cd1d) SHA1(1b1185174ba95dca004169e4e1b51b05c8991c43))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY  FULLNAME    FLAGS
COMP( 1995, 4000_260, 0,      0,      _4000_260, _4000_260, _4000_260_state, empty_init, "ADDS",  "4000/260", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
