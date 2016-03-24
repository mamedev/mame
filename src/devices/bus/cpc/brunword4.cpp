// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Brunword MK4 - Word processor ROM / expansion

    Software is provided as an expansion device, which uses it own ROM mapping

    The ROM select port will be handled by this device, calling back to the standard driver when necessary.
    Not enabled for the CPC Plus, as ROM selection wraps after 63, making it impossible to see cartridge
    banks in the upper ROM area (0x80-0xff)
*/

#include "brunword4.h"

const device_type CPC_BRUNWORD_MK4 = &device_creator<cpc_brunword4_device>;


ROM_START( cpc_brunword4 )
	ROM_REGION( 0xc000, "exp_rom", 0 )
	ROM_LOAD( "brunw-c1.rom",   0x0000, 0x4000, CRC(3200299b) SHA1(d7d5fcacf3c6707a6629b0c65564ac44267d2b49) )
	ROM_LOAD( "brunw-c2.rom",   0x4000, 0x4000, CRC(aa19aff1) SHA1(5aa4e87ae6ad2063540e3f5179298657bbd82bfb) )
	ROM_LOAD( "brunw-c3.rom",   0x8000, 0x4000, CRC(eabe60fe) SHA1(41f605f1e1b5e2bc7dcbd702f2d202ab4d2f44ec) )

	ROM_REGION( 0x80000, "mk4_roms", 0 )
	ROM_LOAD( "brunw-c0.rom",   0x0000, 0x4000, CRC(45493337) SHA1(a971e2e63adb004c605cf642edde828e8b3ab897) )
	ROM_LOAD( "brunw-c1.rom",   0x4000, 0x4000, CRC(3200299b) SHA1(d7d5fcacf3c6707a6629b0c65564ac44267d2b49) )
	ROM_LOAD( "brunw-c2.rom",   0x8000, 0x4000, CRC(aa19aff1) SHA1(5aa4e87ae6ad2063540e3f5179298657bbd82bfb) )
	ROM_LOAD( "brunw-c3.rom",   0xc000, 0x4000, CRC(eabe60fe) SHA1(41f605f1e1b5e2bc7dcbd702f2d202ab4d2f44ec) )
	ROM_LOAD( "brunw-c8.rom",  0x10000, 0x4000, CRC(5522b3ee) SHA1(0b7a97134b9d093f668350206e181f1dfc919540) )
	ROM_LOAD( "brunw-c9.rom",  0x14000, 0x4000, CRC(8ba85101) SHA1(85dec683b5e55f1bba0e5f5ab87380b22743909c) )
	ROM_LOAD( "brunw-ca.rom",  0x18000, 0x4000, CRC(2c81fe99) SHA1(221370c021dad839884c2fc4f5fbdbc964db99d5) )
	ROM_LOAD( "brunw-cb.rom",  0x1c000, 0x4000, CRC(e011d6c4) SHA1(bdfb7ee08291bebd30443d4f6bb5b52a4d6468e7) )
	ROM_LOAD( "brunw-d0.rom",  0x20000, 0x4000, CRC(e2aacb30) SHA1(e75e53991c88e9e586e97be2e6aba8b58c79b7e7) )
	ROM_LOAD( "brunw-d1.rom",  0x24000, 0x4000, CRC(b166a6bd) SHA1(f070ca29b2046f3b87c3eafdc59ac1e0c6f39755) )
	ROM_LOAD( "brunw-d2.rom",  0x28000, 0x4000, CRC(5919f1c3) SHA1(227e74fecab244aa2a8baa483679719fa4939cc0) )
	ROM_LOAD( "brunw-d3.rom",  0x2c000, 0x4000, CRC(d29b6a50) SHA1(de83771d6641c16199412efef4aa706f3da5a1e5) )
	ROM_LOAD( "brunw-d8.rom",  0x30000, 0x4000, CRC(28cb6163) SHA1(0d9a05d1c7eeaaf8b94ba551df3b2a6fbabcdcc9) )
	ROM_LOAD( "brunw-d9.rom",  0x34000, 0x4000, CRC(6d528a78) SHA1(fa8659f07ea4b69c0c699fe03c5068a490326ed8) )
	ROM_LOAD( "brunw-da.rom",  0x38000, 0x4000, CRC(c47589c4) SHA1(6b66d1a24388d95310f536710c5d718472800af0) )
	ROM_LOAD( "brunw-db.rom",  0x3c000, 0x4000, CRC(af69c4b9) SHA1(a6849e774de05acb5ba3cf76ded132ef4ea7c271) )
	ROM_LOAD( "brunw-e0.rom",  0x40000, 0x4000, CRC(aef8fdbb) SHA1(425b863df65f106753ce258ed445b8edb3428ad7) )
	ROM_LOAD( "brunw-e1.rom",  0x44000, 0x4000, CRC(7508b568) SHA1(1d7581fafd0f1119a60b9fc108e60a4274d182ab) )
	ROM_LOAD( "brunw-e2.rom",  0x48000, 0x4000, CRC(c7f9a0f6) SHA1(2f89841ab4431a400491c49b20a484375ddebda9) )
	ROM_LOAD( "brunw-e3.rom",  0x4c000, 0x4000, CRC(1adab4cd) SHA1(82e0a30f16b7b6ca5f65da691c5a84ff0aff9dcf) )
	ROM_LOAD( "brunw-e8.rom",  0x50000, 0x4000, CRC(2450e2b3) SHA1(a85f67ef5f683634ef82f4eee6306be44b6d5f9b) )
	ROM_LOAD( "brunw-e9.rom",  0x54000, 0x4000, CRC(f2d6084c) SHA1(8f9b24478dfb4df691d8f7a13bb7e676766f9154) )
	ROM_LOAD( "brunw-ea.rom",  0x58000, 0x4000, CRC(213176b8) SHA1(61d60a10f12d09045801de013346c889ae194985) )
	ROM_LOAD( "brunw-eb.rom",  0x5c000, 0x4000, CRC(ae811538) SHA1(c92169b385599e0c2ceead17d81689adea9dd164) )
	ROM_LOAD( "brunw-f0.rom",  0x60000, 0x4000, CRC(b3ebc6d3) SHA1(b01e6d23eca4cafca7a7102669fd77a158796159) )
	ROM_LOAD( "brunw-f1.rom",  0x64000, 0x4000, CRC(8faae309) SHA1(1caa585a2c4e1f0ffe6923aa9aaf66e4b0275e17) )
	ROM_LOAD( "brunw-f2.rom",  0x68000, 0x4000, CRC(e6afde30) SHA1(3f1e06ebd8319822de25ed731d9e95ec22568c3d) )
	ROM_LOAD( "brunw-f3.rom",  0x6c000, 0x4000, CRC(637d4c20) SHA1(233ae9507bfbea0ae1e8a3b068e1b19df0151a72) )
	ROM_LOAD( "brunw-f8.rom",  0x70000, 0x4000, CRC(2be3aa82) SHA1(b1896b4f78869632fa5a08a82e126a7ba041e00f) )
	ROM_LOAD( "brunw-f9.rom",  0x74000, 0x4000, CRC(0645f1d0) SHA1(83634937142def1e96306d4be53b491da9f5a25d) )
	ROM_LOAD( "brunw-fa.rom",  0x78000, 0x4000, CRC(f99b181e) SHA1(0533e96a71d2a694efcc7677a446af7fee1eb463) )
	ROM_LOAD( "brunw-fb.rom",  0x7c000, 0x4000, CRC(88383953) SHA1(50c6417b26134b68a80912bdb91c8578eb00c8a2) )
ROM_END

const rom_entry *cpc_brunword4_device::device_rom_region() const
{
	return ROM_NAME( cpc_brunword4 );
}

cpc_brunword4_device::cpc_brunword4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CPC_BRUNWORD_MK4, "Brunword Elite MK4", tag, owner, clock, "cpc_brunword4", __FILE__),
	device_cpc_expansion_card_interface(mconfig, *this), m_slot(nullptr), m_rombank_active(false), m_bank_sel(0)
{
}

void cpc_brunword4_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_IO);
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());

	space.install_write_handler(0xdf00,0xdfff,0,0,write8_delegate(FUNC(cpc_brunword4_device::rombank_w),this));
}

void cpc_brunword4_device::device_reset()
{
	m_rombank_active = false;
}

WRITE8_MEMBER(cpc_brunword4_device::rombank_w)
{
	if((data & 0xc0) == 0xc0 && (data & 0x04) == 0)
	{
		m_bank_sel = data;
		m_rombank_active = true;
		return;
	}
	if((data & 0xc0) == 0x40)  // disable ROM?
	{
		m_rombank_active = false;
		return;
	}
	m_slot->rom_select(space,0,data & 0x3f);  // repeats every 64 ROMs, this breaks upper cart ROM selection on the Plus
}

void cpc_brunword4_device::set_mapping(UINT8 type)
{
	if(type != MAP_OTHER)
		return;
	if(m_rombank_active)
	{
		UINT8* ROM = memregion("mk4_roms")->base();
		UINT8 bank = ((m_bank_sel & 0x38) >> 1) | (m_bank_sel & 0x03);
		membank(":bank3")->set_base(ROM+(bank*0x4000));
		membank(":bank4")->set_base(ROM+((bank*0x4000) + 0x2000));
	}
}
