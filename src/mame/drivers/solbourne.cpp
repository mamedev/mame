// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-28 Skeleton

Solbourne computer workstation. This looks like the Series 5E which uses the Cypress CY7C601 CPU @ 40MHz.

************************************************************************************************************************************/

#include "emu.h"

class solbourne_state : public driver_device
{
public:
	solbourne_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		//      , m_maincpu(*this, "maincpu")
	{ }

	void solbourne(machine_config &config);
private:
	//  required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( solbourne )
INPUT_PORTS_END

void solbourne_state::solbourne(machine_config &config)
{
}

ROM_START( sols5e )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "s5e_3.4_8e36_00.bin", 0x000000, 0x020000, CRC(bc44765a) SHA1(3727af2e683d7cdd4c65865268c689533758c6bb) )
	ROM_LOAD( "s5e_3.4_6d2e_01.bin", 0x000000, 0x020000, CRC(9c1f95ab) SHA1(73d416ba50c2b448b6e69cd30909d6dfe46f9265) )
	ROM_LOAD( "s5e_3.4_eaa8_02.bin", 0x000000, 0x020000, CRC(4b6ee28a) SHA1(c7d5d588a0bec7fc996daef6a76151bf763b5f1c) )
	ROM_LOAD( "s5e_3.4_3a44_03.bin", 0x000000, 0x020000, CRC(cb5dc0ca) SHA1(7fd94f26a9aa605b8ca6a444a131e8011b681d1b) )
	ROM_LOAD( "s5e_3.5_8e0a_00.bin", 0x000000, 0x020000, CRC(60e7c977) SHA1(ac9e3219b6e190cfe912b751cbfe835adf045ab9) )
	ROM_LOAD( "s5e_3.5_2306_01.bin", 0x000000, 0x020000, CRC(935f2d7f) SHA1(5b302701c6bfd7d0156120d474ba1e6df74bdee4) )
	ROM_LOAD( "s5e_3.5_a5aa_02.bin", 0x000000, 0x020000, CRC(b381cfdd) SHA1(90e91aaaff76f8ffcca692aa20750c2afd7c3c93) )
	ROM_LOAD( "s5e_3.5_93bc_03.bin", 0x000000, 0x020000, CRC(ec92be58) SHA1(e645d1573bc9e5b34707d1556df4edf43632b09e) )
	ROM_LOAD( "s5e_3.6b_45aa_00.bin", 0x000000, 0x020000, CRC(52315d71) SHA1(4e6cdfadd1c8a1dd39f6e1b9e09a90f1d9e6b54c) )
	ROM_LOAD( "s5e_3.6b_fdd0_01.bin", 0x000000, 0x020000, CRC(454e8e62) SHA1(dba27acd4abd5e9873dacfaff982c59175de34f6) )
	ROM_LOAD( "s5e_3.6b_623a_02.bin", 0x000000, 0x020000, CRC(56c582fb) SHA1(367b2a9bedf5e5fc7a2aa1f7a05e8c1101b09dd7) )
	ROM_LOAD( "s5e_3.6b_d732_03.bin", 0x000000, 0x020000, CRC(bb6ce8d6) SHA1(6d83e28f9ebac794ad05f47bdd578ef9c1e07de8) )
	ROM_LOAD( "channel_exp_2816.bin", 0x000000, 0x000800, CRC(f9ac0bc6) SHA1(685c2425f58e3c861df4f9f37f39bae343567e13) )
	ROM_LOAD( "30341id.bin",  0x000000, 0x000800, CRC(cdf8cffe) SHA1(725279d995100a37c8a01edfdb3af71332a55c89) )
	ROM_LOAD( "10812id.bin",  0x000000, 0x000800, CRC(07645046) SHA1(673b0be54daf0b890580c8244926835ed323a270) )
	ROM_LOAD( "10454id.bin",  0x000000, 0x000800, CRC(c94b3371) SHA1(9ef9792ffe26302965023041a969d57749d101f7) )
ROM_END

COMP( 198?, sols5e, 0, 0, solbourne, solbourne, solbourne_state, empty_init, "Solbourne Computer Inc", "Series 5E Computer Workstation", MACHINE_IS_SKELETON )
