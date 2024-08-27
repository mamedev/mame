// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Countertop Champ II? (c) 1993 U.S. Games

Dump contains a MS-DOS 3.3 ROM disk, presumably for a bartop style multigame/quiz/fortune teller.

TODO:
- Host MB unknown, may be an higher CPU socket;
- Jumps in unpopulated RAM area as soon as it banks (PC=ca029 -> 0000:36AF), area may be
multibanked instead;

**************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/bankdev.h"
#include "machine/genpc.h"


namespace {

class champ2_state : public driver_device
{
public:
	champ2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mb(*this, "mb")
		, m_bank(*this, "bank")
	{ }

	void champ2(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint8_t m_port_b_data;

//	uint8_t port_a_r();
//	uint8_t port_b_r();
//	uint8_t port_c_r();
//	void port_b_w(uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<pc_noppi_mb_device> m_mb;
	required_device<address_map_bank_device> m_bank;
	void bank_map(address_map &map);
	void main_io(address_map &map);
	void main_map(address_map &map);
};

void champ2_state::main_map(address_map &map)
{
	// Selectable thru jumpers (0xc800, 0xd000, 0xd800, 0xe000)
	map(0xc8000, 0xcffff).m(m_bank, FUNC(address_map_bank_device::amap8));
	// writes to $+2000 then $+0000, same value (0x03)
	map(0xca000, 0xca000).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_bank->set_bank(data & 0x1f);
		})
	);
	map(0xfe000, 0xfffff).rom().region("bios", 0);
}

void champ2_state::main_io(address_map &map)
{
	map.global_mask(0x3ff);
	map(0x0000, 0x00ff).m(m_mb, FUNC(pc_noppi_mb_device::map));
	map(0x0060, 0x0060).lr8(NAME([] (offs_t offset) { return 0xaa; }));
//  map(0x0061, 0x0061).rw(FUNC(champ2_state::port_b_r), FUNC(champ2_state::port_b_w));
//  map(0x0062, 0x0062).r(FUNC(champ2_state::port_c_r));
//  map(0x0201, 0x0201).portr("COIN"); // game port
//  map(0x0310, 0x0311).rw(FUNC(champ2_state::disk_iobank_r), FUNC(champ2_state::disk_iobank_w)); //Prototyping card
//  map(0x0312, 0x0312).portr("IN0"); // Prototyping card, r/o
//  map(0x0313, 0x0313).w(FUNC(champ2_state::voice_start_w));
//  map(0x03f2, 0x03f2).w(FUNC(champ2_state::fdc_dor_w));
//  map(0x03f4, 0x03f4).r(FUNC(champ2_state::fdc765_status_r)); //765 Floppy Disk Controller (FDC) Status
//  map(0x03f5, 0x03f5).rw(FUNC(champ2_state::fdc765_data_r), FUNC(champ2_state::fdc765_data_w));//FDC Data
}


void champ2_state::bank_map(address_map &map)
{
	map(0x00000, 0xfffff).rom().region("game_prg", 0);
}

static INPUT_PORTS_START( champ2 )
INPUT_PORTS_END

void champ2_state::machine_start()
{
}

void champ2_state::machine_reset()
{
	m_bank->set_bank(0);
}


void champ2_state::champ2(machine_config &config)
{
	I8088(config, m_maincpu, XTAL(14'318'181)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &champ2_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &champ2_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	PCNOPPI_MOTHERBOARD(config, m_mb, 0).set_cputag(m_maincpu);
	m_mb->int_callback().set_inputline(m_maincpu, 0);
	m_mb->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa1", 0, "mb:isa", pc_isa8_cards, "vga", false);
//  ISA8_SLOT(config, "isa2", 0, "mb:isa", pc_isa8_cards, "com", false);
//  ISA8_SLOT(config, "isa3", 0, "mb:isa", pc_isa8_cards, "fdc_xt", false);
//  ISA8_SLOT(config, "isa4", 0, "mb:isa", pc_isa8_cards, "hdc", false);
//  ISA8_SLOT(config, "isa5", 0, "mb:isa", pc_isa8_cards, "adlib", false);
//  ISA8_SLOT(config, "isa6", 0, "mb:isa", pc_isa8_cards, nullptr, false);

	RAM(config, RAM_TAG).set_default_size("640K");

	ADDRESS_MAP_BANK(config, m_bank).set_map(&champ2_state::bank_map).set_options(ENDIANNESS_LITTLE, 8, 20, 0x8000);
}


ROM_START( champ2 )
	ROM_REGION( 0x2000, "bios", 0 )
	// borrowed from pc/genpc.cpp
	ROM_LOAD( "pcxt.rom",       0x0000, 0x02000, BAD_DUMP CRC(031aafad) SHA1(a641b505bbac97b8775f91fe9b83d9afdf4d038f) )

	ROM_REGION( 0x100000, "game_prg", 0 )
	ROM_LOAD( "champ2.u2", 0x00000, 0x80000, CRC(058bd1a4) SHA1(e4c0db329cda0cdcab7c7b4d130f1c38fa32385f) )
	ROM_LOAD( "champ2.u3", 0x80000, 0x80000, CRC(3bb1951f) SHA1(8057327285f57787cc7da678427767ee7f979a64) )

	ROM_REGION( 0x1caf, "pal", 0 )
	ROM_LOAD( "champ2.u7",  0x00000, 0x1caf, BAD_DUMP CRC(bd70e89b) SHA1(3567d22057e366a439ffce2dd35180f5df80d47c) )
ROM_END


} // anonymous namespace


// Title assumed by "Champ II" ROM labels
GAME( 1994, champ2,  0, champ2,  champ2, champ2_state, empty_init, ROT0,  "U.S. Games", "Countertop Champion 2 (ver 2.11)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
//GAME( 1993, champ2,  0, champ2,  champ2, champ2_state, empty_init, ROT0,  "U.S. Games", "Countertop Champion", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

