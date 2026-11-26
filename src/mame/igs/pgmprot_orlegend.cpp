// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, iq_132
/***********************************************************************
 PGM ASIC3 PGM protection emulation

 The game calls it ASIC3, the protocol and the protection are the same
 as the IGS003 on other IGS boards.

 The cartridge (NO-0134-2) shorts some IGS003 outputs to the adjacent
 inputs on the solder side (the game tests them) and supplies the region
 on two inputs (probably from JP6 and JP7, with the 10K resistors R2 and
 R3 next to the chip).

 Used by:

 Oriental Legend

 ***********************************************************************/

#include "emu.h"
#include "pgm.h"
#include "pgmprot_orlegend.h"

u16 pgm_asic3_state::igs003_r()
{
	return m_igs003->data_r();
}

void pgm_asic3_state::igs003_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u8 val = ACCESSING_BITS_0_7 ? (data & 0xff) : (data >> 8);

	if (offset == 0)
		m_igs003->index_w(val);
	else
		m_igs003->data_w(val);
}

void pgm_asic3_state::machine_start()
{
	pgm_state::machine_start();

	save_item(NAME(m_igs003_out));
}

void pgm_asic3_state::machine_reset()
{
	pgm_state::machine_reset();

	// the IGS003 protection parameters differ between the regions' chips (World and Taiwan
	// supply the same levels on the known region inputs, so they can't select them)
	switch (m_region->read())
	{
		case 0:
		case 1:
			m_igs003->set_hold_taps(igs003_device::hold_taps::A);
			m_igs003->set_hold_xpos(1, 6, 10, 14);
			break;

		case 2:
			m_igs003->set_hold_taps(igs003_device::hold_taps::A);
			m_igs003->set_hold_xpos(4, 6, 10, 12);
			break;

		case 3:
			m_igs003->set_hold_taps(igs003_device::hold_taps::B);
			m_igs003->set_hold_xpos(4, 6, 10, 12);
			break;

		case 4: // orlegend111t
			m_igs003->set_hold_taps(igs003_device::hold_taps::B);
			m_igs003->set_hold_xpos(3, 8, 10, 14);
			break;
	}
}

/* Oriental Legend INIT */

void pgm_asic3_state::init_orlegend()
{
	pgm_basic_init();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc04000, 0xc0400f, read16smo_delegate(*this, FUNC(pgm_asic3_state::igs003_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xc04000, 0xc0400f, write16s_delegate(*this, FUNC(pgm_asic3_state::igs003_w)));
}


INPUT_PORTS_START( orlegend )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_BIT(     0xfffc, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0001, "World (duplicate)" ) // again?
	PORT_CONFSETTING(      0x0002, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( China ) )
INPUT_PORTS_END

INPUT_PORTS_START( orlegendt )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_BIT(     0xfff8, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0007, 0x0004, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, "Invalid 00?" )
	PORT_CONFSETTING(      0x0001, "Invalid 01?" )
	PORT_CONFSETTING(      0x0002, "Invalid 02?" )
	PORT_CONFSETTING(      0x0003, "Invalid 03?" )
	PORT_CONFSETTING(      0x0004, DEF_STR( Taiwan ) )
INPUT_PORTS_END


INPUT_PORTS_START( orlegendk )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_BIT(     0xfff8, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0007, 0x0002, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, "Invalid 00?" )
	PORT_CONFSETTING(      0x0001, "Invalid 01?" )
	PORT_CONFSETTING(      0x0002, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0003, "Invalid 03?" )
	PORT_CONFSETTING(      0x0004, "Invalid 04?" )
INPUT_PORTS_END


void pgm_asic3_state::pgm_asic3(machine_config &config)
{
	pgmbase(config);

	IGS003(config, m_igs003);
	m_igs003->out_pa_callback().set([this] (u8 data) { m_igs003_out[0] = data; });
	m_igs003->out_pb_callback().set([this] (u8 data) { m_igs003_out[1] = data; });
	m_igs003->out_pc_callback().set([this] (u8 data) { m_igs003_out[2] = data; });
	// outputs looped back: A6 -> A7, B1 -> B2, B6 -> B7, C4 -> C5, region on A3 and C7
	m_igs003->in_pa_callback().set([this] () -> u8 { return ((m_igs003_out[0] << 1) & 0x80) | ((m_region->read() << 3) & 0x08); });
	m_igs003->in_pb_callback().set([this] () -> u8 { return (m_igs003_out[1] << 1) & 0x84; });
	m_igs003->in_pc_callback().set([this] () -> u8 { return ((m_igs003_out[2] << 1) & 0x20) | ((m_region->read() << 6) & 0x80); });
}
