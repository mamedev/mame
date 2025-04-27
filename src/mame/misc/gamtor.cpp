// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Novomatic CoolFire I */
/* Austrian gaming system - used late 2002 - early 2008? */

/*

  interrupt vectors get put in ram at 0x08000000


  preliminary video based on llcharm / g4u5 set ONLY
  there is clearly a way to set the base address for the tiles, or they should get
  DMAd elsewhere before use.

  ends up making some accesses to the rom area because the pointer it gets from RAM is 0
  and dies during what it calls 'EEPROM test' (investigate - is it hooked up in the coldfire
  peripheral area?)

  llcharm ->
    'maincpu' (014902D2): unmapped program memory write to 000000D8 = 00000000 & FFFFFFFF
    'maincpu' (01490312): unmapped program memory write to 000000B8 = 0BB19C00 & FFFFFFFF
    'maincpu' (0149031C): unmapped program memory write to 000000D8 = 00000001 & FFFFFFFF
    'maincpu' (01490312): unmapped program memory write to 000000B8 = 038B9C00 & FFFFFFFF
    'maincpu' (0149031C): unmapped program memory write to 000000D8 = 00000001 & FFFFFFFF

*/


#include "emu.h"
#include "cpu/m68000/mcf5206e.h"
#include "machine/mcf5206e.h"
#include "video/pc_vga.h"
#include "speaker.h"


// TODO: Chips & Technologies 65550 with swapped address lines?
// Needs to be moved to own family file
// 65550 is used by Apple PowerBook 2400c
// 65535 is used by IBM PC-110

class gamtor_vga_device : public svga_device
{
public:
	// construction/destruction
	gamtor_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

//  virtual uint8_t mem_r(offs_t offset) override;
//  virtual void mem_w(offs_t offset, uint8_t data) override;
	virtual uint8_t mem_linear_r(offs_t offset) override;
	virtual void mem_linear_w(offs_t offset,uint8_t data) override;

protected:
	virtual void io_3cx_map(address_map &map) override ATTR_COLD;
	virtual void io_3bx_3dx_map(address_map &map) override ATTR_COLD;

	virtual uint16_t offset() override;
};

DEFINE_DEVICE_TYPE(GAMTOR_VGA, gamtor_vga_device, "gamtor_vga", "CT-65550 SVGA")

gamtor_vga_device::gamtor_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, GAMTOR_VGA, tag, owner, clock)
{
	m_main_if_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(gamtor_vga_device::io_3bx_3dx_map), this));
}

// A0/A1 are inverted (protection or ct55550 feature?)
void gamtor_vga_device::io_3bx_3dx_map(address_map &map)
{
	map(0x04 ^ 3, 0x04 ^ 3).rw(FUNC(gamtor_vga_device::crtc_address_r), FUNC(gamtor_vga_device::crtc_address_w));
	map(0x05 ^ 3, 0x05 ^ 3).rw(FUNC(gamtor_vga_device::crtc_data_r), FUNC(gamtor_vga_device::crtc_data_w));
	map(0x0a ^ 3, 0x0a ^ 3).rw(FUNC(gamtor_vga_device::input_status_1_r), FUNC(gamtor_vga_device::feature_control_w));
}

void gamtor_vga_device::io_3cx_map(address_map &map)
{
	map(0x00 ^ 3, 0x00 ^ 3).rw(FUNC(gamtor_vga_device::atc_address_r), FUNC(gamtor_vga_device::atc_address_data_w));
	map(0x01 ^ 3, 0x01 ^ 3).r(FUNC(gamtor_vga_device::atc_data_r));
	map(0x02 ^ 3, 0x02 ^ 3).rw(FUNC(gamtor_vga_device::input_status_0_r), FUNC(gamtor_vga_device::miscellaneous_output_w));
	map(0x04 ^ 3, 0x04 ^ 3).rw(FUNC(gamtor_vga_device::sequencer_address_r), FUNC(gamtor_vga_device::sequencer_address_w));
	map(0x05 ^ 3, 0x05 ^ 3).rw(FUNC(gamtor_vga_device::sequencer_data_r), FUNC(gamtor_vga_device::sequencer_data_w));
	map(0x06 ^ 3, 0x06 ^ 3).rw(FUNC(gamtor_vga_device::ramdac_mask_r), FUNC(gamtor_vga_device::ramdac_mask_w));
	map(0x07 ^ 3, 0x07 ^ 3).rw(FUNC(gamtor_vga_device::ramdac_state_r), FUNC(gamtor_vga_device::ramdac_read_index_w));
	map(0x08 ^ 3, 0x08 ^ 3).rw(FUNC(gamtor_vga_device::ramdac_write_index_r), FUNC(gamtor_vga_device::ramdac_write_index_w));
	map(0x09 ^ 3, 0x09 ^ 3).rw(FUNC(gamtor_vga_device::ramdac_data_r), FUNC(gamtor_vga_device::ramdac_data_w));
	map(0x0a ^ 3, 0x0a ^ 3).r(FUNC(gamtor_vga_device::feature_control_r));
	map(0x0c ^ 3, 0x0c ^ 3).r(FUNC(gamtor_vga_device::miscellaneous_output_r));
	map(0x0e ^ 3, 0x0e ^ 3).rw(FUNC(gamtor_vga_device::gc_address_r), FUNC(gamtor_vga_device::gc_address_w));
	map(0x0f ^ 3, 0x0f ^ 3).rw(FUNC(gamtor_vga_device::gc_data_r), FUNC(gamtor_vga_device::gc_data_w));
}

uint8_t gamtor_vga_device::mem_linear_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		logerror("Reading gamtor SVGA memory %08x\n", offset);
	return vga.memory[offset];
}

void gamtor_vga_device::mem_linear_w(offs_t offset, uint8_t data)
{
	if (offset & 2)
		vga.memory[(offset >> 2) + 0x20000] = data;
	else
		vga.memory[(offset & 1) | (offset >> 1)] = data;
}

uint16_t gamtor_vga_device::offset()
{
	// TODO: pinpoint whatever extra register that wants this shifted by 1
	return vga_device::offset() << 1;
}

namespace {

class gaminator_state : public driver_device
{
public:
	gaminator_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void init_gaminator();
	void gaminator(machine_config &config);

private:
	void gaminator_map(address_map &map) ATTR_COLD;

	void gamtor_unk_w(uint32_t data);

	required_device<cpu_device> m_maincpu;
};

void gaminator_state::gamtor_unk_w(uint32_t data)
{
}



void gaminator_state::gaminator_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).rom();
	map(0x08000000, 0x0bffffff).ram();
	map(0x1e040008, 0x1e04000b).w(FUNC(gaminator_state::gamtor_unk_w));

	map(0x20000000, 0x2003ffff).ram();

	/* standard VGA */
	//map(0x40000000, 0x40000fff).ram(); // regs
	map(0x400003b0, 0x400003df).m("vga", FUNC(gamtor_vga_device::io_map));

	// TODO: revisit mapping in VGA core once it enters non-text mode
	map(0x44000000, 0x4401ffff).rw("vga", FUNC(gamtor_vga_device::mem_linear_r), FUNC(gamtor_vga_device::mem_linear_w));

	// beetlem accesses here, standard layout unlike later games
	map(0x440a0000, 0x440bffff).rw("vga", FUNC(gamtor_vga_device::mem_r), FUNC(gamtor_vga_device::mem_w));

	map(0xe0000000, 0xe00001ff).ram(); // nvram?
	map(0xf0000000, 0xf00003ff).rw("maincpu_onboard", FUNC(mcf5206e_peripheral_device::dev_r), FUNC(mcf5206e_peripheral_device::dev_w)); // technically this can be moved with MBAR
}

static INPUT_PORTS_START(  gaminator )
INPUT_PORTS_END



void gaminator_state::gaminator(machine_config &config)
{
	MCF5206E(config, m_maincpu, 40000000); /* definitely Coldfire, model / clock uncertain */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaminator_state::gaminator_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaminator_state::irq6_line_hold)); // irq6 seems to be needed to get past the ROM checking
	MCF5206E_PERIPHERAL(config, "maincpu_onboard", 0, m_maincpu);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800),900,0,640,526,0,480);
	screen.set_screen_update("vga", FUNC(gamtor_vga_device::screen_update));

	gamtor_vga_device &vga(GAMTOR_VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x100000);

	SPEAKER(config, "speaker", 2).front();
	/* unknown sound */
}




ROM_START( g4u5 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "games4y_594v5.6-5@1.bin", 0x0000000, 0x4000000, CRC(3eb0a6fe) SHA1(2c8a1f3850a1377c2e2430d527787d2586d7dbea) ) /* GAMES4Y5 V5.6-5 May 16 2007 */
	ROM_LOAD( "games4y_594v5.6-5@2.bin", 0x4000000, 0x4000000, CRC(b26afd49) SHA1(28ae48467581517be12922c7920afc0f54e4fe4f) )
ROM_END


ROM_START( g4u6 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "games4y_694v5.6-5@1.bin", 0x0000000, 0x4000000, CRC(b353bb0f) SHA1(6f9895ec42601ee82b31274c9f8cc54c3929717f) ) /* GAMES4Y6 V5.6-5 Apr 02 2007 */
	ROM_LOAD( "games4y_694v5.6-5@2.bin", 0x4000000, 0x4000000, CRC(fcfe8b6f) SHA1(0b77557f95c1fd315434dfafeb4d13f71a0198e1) )
ROM_END

ROM_START( g4u7 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "games4y_794v5.6-5a@1.bin", 0x0000000, 0x4000000, CRC(ea05445d) SHA1(b667a7c2529304354ccce86e9ccdfc9e8235fea0) ) /* GAMES4Y7 V5.6-5A May 16 2007 */
	ROM_LOAD( "games4y_794v5.6-5a@2.bin", 0x4000000, 0x4000000, CRC(b6f9be81) SHA1(ba296d8e9adf8bcbb783a581b39e645da9a7f0b0) )
ROM_END

ROM_START( g4u2 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "games4y_294@1_v5.6-0.rom", 0x0000000, 0x4000000, CRC(36fed1f6) SHA1(fbe7e8b99c90336075831ceb47f345c82068f719) ) /* GAMES4Y2 V5.6-0 Aug 08 2006 */
	ROM_LOAD( "games4y_294@2_v5.6-0.rom", 0x4000000, 0x4000000, CRC(7c29b788) SHA1(bc6fbcc5a36e464d10d4061a7ba83b29703d4839) )
ROM_END

ROM_START( g4u3 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "games4y_394@1_v5.6-4.rom", 0x0000000, 0x4000000, CRC(2228b20a) SHA1(a01eb039e0102145af4bc0ed4714d20799da6a0c) ) /* GAMES4Y3 V5.6-4 Jan 09 2007 */
	ROM_LOAD( "games4y_394@2_v5.6-4.rom", 0x4000000, 0x4000000, CRC(68cd6b12) SHA1(75b83c0fcde36bc250546b519783c3470bf2044b) )
ROM_END

ROM_START( g4u3a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "4y3@1 v5.6-5.rom", 0x0000000, 0x4000000, CRC(3915aeb9) SHA1(7e3fae019cefd55a1645f547017ab28f6c884b86) ) /* GAMES4Y3 V5.6-5 Apr 02 2007 */
	ROM_LOAD( "4y3@2 v5.6-5.rom", 0x4000000, 0x4000000, CRC(29faae20) SHA1(00929d5b600f5461bf1ca53888b23db1d52dc320) )
ROM_END

ROM_START( g4u4 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "games4y_494v5.6-5@1.bin", 0x0000000, 0x4000000, CRC(ece8734d) SHA1(706459cd275b74f888930f08ac167dbb85b85385) ) /* GAMES4Y4 V5.6-5 Apr 02 2007 */
	ROM_LOAD( "games4y_494v5.6-5@2.bin", 0x4000000, 0x4000000, CRC(f690e6a5) SHA1(4286ccaad92cbad28480323ae8bd072a4629685c) )
ROM_END

/* Gaminator 4 */

ROM_START( gamt1 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "1_94_5418_1", 0x0000000, 0x4000000, CRC(b81f6395) SHA1(36df76fcae23097cc3f45685636e6328e2cabed1) ) /* V5.4-18 May 04 2004 */
	ROM_LOAD( "1_94_5418_2", 0x4000000, 0x4000000, CRC(86267501) SHA1(2d90708f34818a2b6949f62fda6d6e66471dea79) )
ROM_END

ROM_START( gamt1a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator194___1_v 5.4-23.rom", 0x0000000, 0x4000000, CRC(ec211fed) SHA1(b1aa7fa5974008d239cc5500a72033c38fa5fff2) ) /* V5.4-23 Oct 21 2004 */
	ROM_LOAD( "gaminator194___2_v 5.4-23.rom", 0x4000000, 0x4000000, CRC(f19e43a9) SHA1(418f1c3d98cf615f2941be0cf87b3eced9a601c2) )
ROM_END

ROM_START( gamt1b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator194 @1 v5.6-0.rom", 0x0000000, 0x4000000, CRC(fb3e9209) SHA1(4a68db08b37880913041434a82ee3721052c9713) ) /* V5.6-0 Aug 09 2006 */
	ROM_LOAD( "gaminator194 @2 v5.6-0.rom", 0x4000000, 0x4000000, CRC(201b9510) SHA1(acaf13c76d3e78b6dc967a1c2300ef30dc863694) )
ROM_END

/* Gaminator 4 */

ROM_START( gamt4 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator496___1_v 5.4-19.rom", 0x0000000, 0x4000000, CRC(dc5aea1b) SHA1(4c4ec1d92357141eafdf5586dc3b9c638b6d586d) ) /* V5.4-19 May 25 2004 */
	ROM_LOAD( "gaminator496___2_v 5.4-19.rom", 0x4000000, 0x4000000, CRC(cfb77ee8) SHA1(ccec49dbc5e369e3253cb526e438bae8b8a90be8) )
ROM_END

ROM_START( gamt4a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator494___1_v 5.4-19.rom", 0x0000000, 0x4000000, CRC(0d8a4096) SHA1(d633c4dd4204c36b39701e5cc1b4be1f640e946e) ) /* V5.4-19 May 25 2004 */
	ROM_LOAD( "gaminator494___2_v 5.4-19.rom", 0x4000000, 0x4000000, CRC(cfb77ee8) SHA1(ccec49dbc5e369e3253cb526e438bae8b8a90be8) )
ROM_END

ROM_START( gamt4b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator494___1_v5.4-27.rom", 0x0000000, 0x4000000, CRC(714b0964) SHA1(93b5f9c56f593b66f1a092d510eb333eb6c85ad4) ) /* V5.4-27 Mar 07 2005 */
	ROM_LOAD( "gaminator494___2_v5.4-27.rom", 0x4000000, 0x4000000, CRC(af46a281) SHA1(5bd99c1e136aa7d9075dae0d943ced316aa62479) )
ROM_END

ROM_START( gamt4c )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "4_94-5.5-5_1", 0x0000000, 0x4000000, CRC(c6edbaab) SHA1(b8e4beeccc311650168248a88032420393cae64e) ) /*V5.5-5 Jul 26 2005 */
	ROM_LOAD( "4_94-5.5-5_2", 0x4000000, 0x4000000, CRC(bec89778) SHA1(a91c05ca0d75eaed7cd5be3a2a91a2b93209ff86) )
ROM_END

ROM_START( gamt4d )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator494___1_v 5.5-6.rom", 0x0000000, 0x4000000, CRC(67bd128a) SHA1(4452891fe58ec3df0610c02b24d26e48f41c97ee) ) /* V5.5-6 Sep 27 2005 */
	ROM_LOAD( "gaminator494___2_v 5.5-6.rom", 0x4000000, 0x4000000, CRC(452d4f41) SHA1(db45af7399eff8bbbf32199705abc56a78134a1b) )
ROM_END

ROM_START( gamt4dbag )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator494___1_v 5.5-6.rom", 0x0000000, 0x4000000, CRC(67bd128a) SHA1(4452891fe58ec3df0610c02b24d26e48f41c97ee) ) /* V5.5-6 Sep 27 2005 */
	ROM_LOAD( "g4_5.5-6bag,lady.bin", 0x4000000, 0x4000000, CRC(d4a299ac) SHA1(a9984f46868905e38d5458aed69e9be8e7a76818) )
ROM_END


ROM_START( gamt4e )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "4_94-5.5-8_1", 0x0000000, 0x4000000, CRC(3a8d890e) SHA1(9883d55201ce1f7f5c1d1298e7dd2b487c13d976) ) /* V5.5-8 Dec 07 2005 */
	ROM_LOAD( "4_94-5.5-8_2", 0x4000000, 0x4000000, CRC(79ba6d61) SHA1(17a92020b18ea6292f82a0810f02ebe9afee322e) )
ROM_END

ROM_START( gamt4f )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "4_94_5-5-9_1", 0x0000000, 0x4000000, CRC(3a365dbe) SHA1(974ff7b4090f90e219b47d48b3e3a516c678ed7d) ) /* V5.5-9 Feb 02 2006 */
	ROM_LOAD( "4_94-5.5-9_2", 0x4000000, 0x4000000, CRC(11b698e7) SHA1(d2fe7e15fdc0e9bd7498f65200224addc434cba7) )
ROM_END

ROM_START( gamt4fbag )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "4_94_5-5-9_1", 0x0000000, 0x4000000, CRC(3a365dbe) SHA1(974ff7b4090f90e219b47d48b3e3a516c678ed7d) ) /* V5.5-9 Feb 02 2006 */
	ROM_LOAD( "g4_5.5-9bag,lady.bin", 0x4000000, 0x4000000, CRC(e394ddf8) SHA1(3de6309d11bea0aa800f23c7044e5519f8328df6) )
ROM_END



ROM_START( gamt4g )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator494___1_v 5.5-10a.rom", 0x0000000, 0x4000000, CRC(61cae731) SHA1(4324a30f9b04efd6b48e21070e31eb8bda0c607f) ) /* V5.5-10A Jul 19 2006 */
	ROM_LOAD( "gaminator494___2_v 5.5-10a.rom", 0x4000000, 0x4000000, CRC(3878a4f2) SHA1(c7f63f19e99368fd9b47abcfd17526dc360ddecd) )
ROM_END





ROM_START( gamt4h )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "4_94-5.6-0_1", 0x0000000, 0x4000000, CRC(f3f4b553) SHA1(9d3108c7a5e702d8b336744835aeb308ec36057e) ) /* V5.6-0 Aug 07 2006 */
	ROM_LOAD( "4_94-5.6-0_2", 0x4000000, 0x4000000, CRC(0f278eff) SHA1(1ff4a6c7b2b6d68c8e5dacb7268b79228ead9c01) )
ROM_END

ROM_START( gamt4hmult )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "4_94-5.6-0_1", 0x0000000, 0x4000000, CRC(f3f4b553) SHA1(9d3108c7a5e702d8b336744835aeb308ec36057e) ) /* V5.6-0 Aug 07 2006 */
	ROM_LOAD( "g494_v5.6-0_2", 0x4000000, 0x4000000, CRC(c17fa3f9) SHA1(a698f94cc218b266c69f5e5e3ea347906d7a6344) )
ROM_END

ROM_START( gamt4hbag )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "4_94-5.6-0_1", 0x0000000, 0x4000000, CRC(f3f4b553) SHA1(9d3108c7a5e702d8b336744835aeb308ec36057e) ) /* V5.6-0 Aug 07 2006 */
	ROM_LOAD( "gaminator4@2 v.5.6-0.rom", 0x4000000, 0x4000000, CRC(e5177c37) SHA1(b4006401ecd4840aa8a5fb5349e6da6746cfe2f3) )
ROM_END




ROM_START( gamt4i )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator494_v5.6-5@1.rom", 0x0000000, 0x4000000, CRC(652cbd62) SHA1(a420bc61434ff56b38fa088becf4cbf43a998f9b) ) /* V5.6-5 Feb 26 2007 */
	ROM_LOAD( "gaminator494_v5.6-5@2.rom", 0x4000000, 0x4000000, CRC(7be81ea9) SHA1(a166312c69d5084d96f7f92f0a0bfd73322a1084) )
ROM_END

ROM_START( gamt4ibag )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator494_v5.6-5@1.rom", 0x0000000, 0x4000000, CRC(652cbd62) SHA1(a420bc61434ff56b38fa088becf4cbf43a998f9b) ) /* V5.6-5 Feb 26 2007 */
	ROM_LOAD( "gaminator4@2 v.5.6-5.rom", 0x4000000, 0x4000000, CRC(66b1fba0) SHA1(3fb798a19a6b44f6993787bbe2e6747c6c826674) )
ROM_END


ROM_START( gamt4j )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt4j_read@1.bin", 0x0000000, 0x4000000, CRC(9203dfd0) SHA1(02ba813ca3c77e57d8a2622da57b5f4b0c3d80fd) ) /* V5.6-11 Jan 09 2008 */
	ROM_LOAD( "gamt4j_read@2.bin", 0x4000000, 0x4000000, CRC(7fb46ab0) SHA1(b2c8be0874072a1fb0788741e406f334173dec4c) )
ROM_END

/* Gaminator 5 */

ROM_START( gamt5 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator-5-94=1 v5-6-0", 0x0000000, 0x4000000, CRC(b4f67432) SHA1(094e044b82bd7ae0d0a8c3d4d9b66304939670dc) ) /* V5.6-0 Oct 09 2006 */
	ROM_LOAD( "gaminator-5-94=2 v5-6-0", 0x4000000, 0x4000000, CRC(825c8994) SHA1(2b63181a3da9b07b89483b0d48a595b3827972b3) )
ROM_END

/* Gaminator 6 */

ROM_START( gamt6 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator694___1_v5.4-29.rom", 0x0000000, 0x4000000, CRC(19cadd12) SHA1(f13884bd601c7c17e99bf14dfd26d81b26208dec) ) /* V5.4-29 May 10 2005 */
	ROM_LOAD( "gaminator694___2_v5.4-29.rom", 0x4000000, 0x4000000, CRC(5a2b558f) SHA1(b58b85a8d10a57977fbd58ff6223e76c5a5eeeeb) )
ROM_END

ROM_START( gamt6a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamin_6_96_@1,5.4-29.rom", 0x0000000, 0x4000000, CRC(c81a779f) SHA1(a4ac66bc099fa9a1321bcd97cf1f0026dd02fb12) ) /* V5.4.29 May 10 2005 */
	ROM_LOAD( "gamin_6_96_@2,5.4-29.rom", 0x4000000, 0x4000000, CRC(5a2b558f) SHA1(b58b85a8d10a57977fbd58ff6223e76c5a5eeeeb) )
ROM_END

ROM_START( gamt6b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator6_94_v5.5-9_1.rom", 0x0000000, 0x4000000, CRC(7a7030b9) SHA1(e6901d0ec6001895e229de87ec062977c6e959c1) ) /* V5.5-9 Feb 02 2006 */
	ROM_LOAD( "gaminator6_94_v5.5-9_2.rom", 0x4000000, 0x4000000, CRC(befac2d4) SHA1(7ac1c24e4d13155c4f4624ed86fb11999c01020a) )
ROM_END

ROM_START( gamt6c )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator6_96_1.rom", 0x0000000, 0x4000000, CRC(75f2381b) SHA1(c4bc4ecdb04596599e03c5aa0cc93725d6581d60) ) /* V5.5-9 Feb 02 2006 */
	ROM_LOAD( "gaminator6_96_2.rom", 0x4000000, 0x4000000, CRC(befac2d4) SHA1(7ac1c24e4d13155c4f4624ed86fb11999c01020a) )
ROM_END

ROM_START( gamt6d )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator694___1_v5.5-10.rom", 0x0000000, 0x4000000, CRC(b602fda8) SHA1(a180370c03b817261f7d4464965fb9c23daeb685) ) /* V5.5-10 May 11 2006 */
	ROM_LOAD( "gaminator694___2_v5.5-10.rom", 0x4000000, 0x4000000, CRC(9e66fac8) SHA1(9f851fc29fea50d9d1a8f63a8abdf48a7a6f0fc7) )
ROM_END

ROM_START( gamt6e )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator6-94=1 v5-6-0", 0x0000000, 0x4000000, CRC(c72076e3) SHA1(84f60a0418685732ac72d39272bfa8c493b92a13) ) /* V5.6-0 Sep 06 2006 */
	ROM_LOAD( "gaminator6-94=2 v5-6-0", 0x4000000, 0x4000000, CRC(253a4806) SHA1(61b89f18851152b031bd589bec1cfb5f9767b6f7) )
ROM_END

ROM_START( gamt6f )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "565-1.bin", 0x0000000, 0x4000000, CRC(ff5d2799) SHA1(c006aa55beccb07dde970c382353ba9e3a9f534c) ) /* V5.6-5 Feb 27 2007 */
	ROM_LOAD( "565-2.bin", 0x4000000, 0x4000000, CRC(6d34cddd) SHA1(b39a0fff39efc8b5cc03912a239955656984e947) )
ROM_END

/* Gaminator 7 */

ROM_START( gamt7 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "7_92-5.5-0_1", 0x0000000, 0x4000000, CRC(a01f5e01) SHA1(358b715bbd59f7520a0e63c988a24a478ce1e629) ) /* V5.5-0 Apr 13 2005 */
	ROM_LOAD( "7_92-5.5-0_2", 0x4000000, 0x4000000, CRC(079cb130) SHA1(71c26de127676ae512851c4c5436cad81819e4ec) )
ROM_END

ROM_START( gamt7a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "7_92-5.5-8_1", 0x0000000, 0x4000000, CRC(f88e2586) SHA1(6920988dcc199ab9bf9a04d52c19290e18b94898) ) /* V5.5-6 Sep 21 2005 */
	ROM_LOAD( "7_92-5.5-8_2", 0x4000000, 0x4000000, CRC(969bcff5) SHA1(fc7fafe0177237998b93f03a528053c2f626d3df) )
ROM_END

ROM_START( gamt7b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator 7,423d,94________1.rom.rom", 0x0000000, 0x4000000, CRC(a03484f0) SHA1(019fcb410768b981b939d0ce4b2bfeea3c8d85c9) ) /* V5.5-8 Nov 30 2005 */
	ROM_LOAD( "gaminator 7,423d,94________2.rom.rom", 0x4000000, 0x4000000, CRC(c2cf8fd2) SHA1(544b6dfb7f184bb818cedb9600f43915094ad7e1) )
ROM_END

ROM_START( gamt7c )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator794@1-v5.5-10.rom", 0x0000000, 0x4000000, CRC(4b7f3a3b) SHA1(40cdb1d94c087efadcb672339dfde722b8e3e4bc) ) /* V5.5-10A Jul 24 2006 */
	ROM_LOAD( "gaminator794@2-v5.5-10.rom", 0x4000000, 0x4000000, CRC(4936b216) SHA1(7a929260f52d94a0445380fcfee05d3e2e2d5abf) )
ROM_END

ROM_START( gamt7d )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator792@1_v5.6-4.rom", 0x0000000, 0x4000000, CRC(b436b4e0) SHA1(085c6000f44c609ff3aa2ebcbbe56eb6c40fdfdf) ) /* V5.6-4 Dec 04 2006 */
	ROM_LOAD( "gaminator792@2_v5.6-4.rom", 0x4000000, 0x4000000, CRC(5cff1dee) SHA1(12958e587253ce8bb9f04ee5289a0c77b4c74c10) )
ROM_END

ROM_START( gamt7e )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator792@1-v5.6-5.rom", 0x0000000, 0x4000000, CRC(5dfa30ae) SHA1(778a80de202944be4635cae484f2084bb35e5a95) ) /* V5.6-5 Mar 15 2007 */
	ROM_LOAD( "gaminator792@2-v5.6-5.rom", 0x4000000, 0x4000000, CRC(2c005f45) SHA1(91b8e53ac85665247bdd0164da5d5dbc0d2c903d) )
ROM_END

ROM_START( gamt7f )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator794@1-v5.6-5.rom", 0x0000000, 0x4000000, CRC(b589b09c) SHA1(66a7c04d4d727f8c89adfdd2203920fac9913051) ) /* V5.6-5 Mar 15 2007 */
	ROM_LOAD( "gaminator794@2-v5.6-5.rom", 0x4000000, 0x4000000, CRC(2c005f45) SHA1(91b8e53ac85665247bdd0164da5d5dbc0d2c903d) )
ROM_END

ROM_START( gamt7g )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator7 92_1,v.5.6-10.bin", 0x0000000, 0x4000000, CRC(66dbb196) SHA1(af2f660a16c2b2a2690a532a727af98e19e14520) )  /* V5.6-10 Nov 06 2007 */
	ROM_LOAD( "gaminator7 92_2,v.5.6-10.bin", 0x4000000, 0x4000000, CRC(1492cabc) SHA1(c556a3be375fd83fd1ef3c642a83090c433c3f5c) )
ROM_END

ROM_START( gamt7h )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator7 94_1,v.5.6-10.bin", 0x0000000, 0x4000000, CRC(8ea831a4) SHA1(84d162da2142f0426db55bbf2e2d6c54889c7bef) ) /* V5.6-10 Nov 06 2007 */
	ROM_LOAD( "gaminator7 94_2,v.5.6-10.bin", 0x4000000, 0x4000000, CRC(1492cabc) SHA1(c556a3be375fd83fd1ef3c642a83090c433c3f5c) )
ROM_END

/* Gaminator 8 */

ROM_START( gamt8 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator894___1_v5.5-0.rom", 0x0000000, 0x4000000, CRC(68ee1de5) SHA1(f43efb4562a35daff49acb7a48bba398c868f65b) ) /* V5.5-0 Apr 05 2005 */
	ROM_LOAD( "gaminator894___2_v5.5-0.rom", 0x4000000, 0x4000000, CRC(845c8c7e) SHA1(e7922b8458b6c42f0b5f2357c28091467faf98a2) )
ROM_END

ROM_START( gamt8a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "8_94-5.5-2_1", 0x0000000, 0x4000000, CRC(ed1ea273) SHA1(7b2d3df79468b17e99149f0031b1a257b9292d0d) ) /* V5.5-2 May 30 2005 */
	ROM_LOAD( "8_94-5.5-2_2", 0x4000000, 0x4000000, CRC(ef9169da) SHA1(23a95ad9627eec5a27726dc15d6aa5d027ddd158) )
ROM_END

ROM_START( gamt8b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator8_94_1.rom", 0x0000000, 0x4000000, CRC(feee74b5) SHA1(2ef9fb28a62bde32fc13f5ecbac87bcbcb225af3) ) /* V5.5-9 Dec 13 2005 */
	ROM_LOAD( "gaminator8_94_2.rom", 0x4000000, 0x4000000, CRC(168d616e) SHA1(cd9506c75c225eb67fa5468f2aea73372f339edf) )
ROM_END

ROM_START( gamt8c )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator894___1_v5.5-10.rom", 0x0000000, 0x4000000, CRC(adaba76d) SHA1(83a95444c96f8b34b257a05d346f6308d72c9cc4) ) /* V5.5-10 May 11 2006 */
	ROM_LOAD( "gaminator894___2_v5.5-10.rom", 0x4000000, 0x4000000, CRC(be5ff4ae) SHA1(7cd4b5559ad423f2d21d9d98ef03ac24b4311288) )
ROM_END

ROM_START( gamt8d )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator892v5.6_5@1.bin", 0x0000000, 0x4000000, CRC(72c28e3f) SHA1(495e5a673776125697d0e6f07a40c9953c19b92c) ) /* V5.6-5 Feb 27 2007 */
	ROM_LOAD( "gaminator892v5.6_5@2.bin", 0x4000000, 0x4000000, CRC(9fc3467b) SHA1(a3dc484de82f9094d9c77d569e6673df8a54f1c8) )
ROM_END

/* Gaminator 9 */

ROM_START( gamt9 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator994v5.5-1_@1.bin", 0x0000000, 0x4000000, CRC(8bba5902) SHA1(f96885815b9de955a5f9b3ffd36ec5f8bd606836) ) /* V5.5-1 May 10 2005 */
	ROM_LOAD( "gaminator994v5.5-1_@2.bin", 0x4000000, 0x4000000, CRC(445d67d1) SHA1(081f10f519f5fa0a71e3b0395dbf969b930677ef) )
ROM_END

ROM_START( gamt9a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gaminator994@1_5.6-0.rom", 0x0000000, 0x4000000, CRC(74556573) SHA1(ddf2bd7f9845a2b3174b40f3ab82a03b28b467e8) ) /* V5.6-0 Aug 03 2006 */
	ROM_LOAD( "gaminator994@2_5.6-0.rom", 0x4000000, 0x4000000, CRC(5af4be44) SHA1(61933b2948bb55403e2e90bc1d34245bc1930670) )
ROM_END

/* Gaminator 10 */

ROM_START( gamt10 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-94-5.5-5.rom1", 0x0000000, 0x4000000, CRC(4c2286cb) SHA1(226e729b348d02fdcb2e8db2ef11c903d0659cba) ) /* V5.5-5 Aug 24 2005 */
	ROM_LOAD( "gamt10-9x-5.5-5.rom2", 0x4000000, 0x4000000, CRC(ae1acaf2) SHA1(1cf40608e19f2e94e9c49c6a046940e74023c43c) )
ROM_END

ROM_START( gamt10bag )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-94-5.5-5.rom1", 0x0000000, 0x4000000, CRC(4c2286cb) SHA1(226e729b348d02fdcb2e8db2ef11c903d0659cba) ) /* V5.5-5 Aug 24 2005 */
	ROM_LOAD( "g10_5.5-5bag,money.bin", 0x4000000, 0x4000000, CRC(6318d6f8) SHA1(4b4fdb31da8d1a058053326b77c1d206b13e3734) )
ROM_END



ROM_START( gamt10a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-94-5.5-9.rom1", 0x0000000, 0x4000000, CRC(7c8588ab) SHA1(c69b2218893fb09bb13ecfdfc2a45a1fa67e10d0) ) /* V5.5-9 Dec 13 2005 */
	ROM_LOAD( "gamt10-9x-5.5-9.rom2", 0x4000000, 0x4000000, CRC(479ef3e3) SHA1(082841f0af8eca3728a940cd284b00e77b7b7546) )
ROM_END

ROM_START( gamt10b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-96-5.5-10.rom1", 0x0000000, 0x4000000, CRC(f5eec6c2) SHA1(49069055fdcbeef9e43f71a42eba0ef8f3a4c4e6) ) /* V5.5-10 Mar 29 2006 */
	ROM_LOAD( "gamt10-9x-5.5-10.rom2", 0x4000000, 0x4000000, CRC(2712f24e) SHA1(776c422d8a08edb2e9cfc30a69d3cc87eae50e7d) )
ROM_END

ROM_START( gamt10c )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-94-5.5-10a.rom1", 0x0000000, 0x4000000, CRC(5281d489) SHA1(971d8fed420a0750212ac216c6724b4b4d76c391) ) /* V5.5-10A Jul 19 2006 */
	ROM_LOAD( "gamt10-9x-5.5-10a.rom2", 0x4000000, 0x4000000, CRC(d78101fd) SHA1(b6ebbb52766296f76a7ef47db11d553a1922ba3d) )
ROM_END

ROM_START( gamt10d )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-92-5.6-1.rom1", 0x0000000, 0x4000000, CRC(33095a74) SHA1(a6e760627ec9ba83799fe01ac0d3102a0e76e8e1) ) /* V5.6-1 Nov 20 2006 */
	ROM_LOAD( "gamt10-9x-5.6-1.rom2", 0x4000000, 0x4000000, CRC(b249c8d4) SHA1(1f8de4d1504de7897dc0240f47ac398f03749004) )
ROM_END

ROM_START( gamt10e )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-90-5.6-4.rom1", 0x0000000, 0x4000000, CRC(e5a2723a) SHA1(25d0a1d14ec2cf68c1927b7ebfac58aced74fdb7) ) /* V5.6-4 Dec 04 2006 */
	ROM_LOAD( "gamt10-9x-5.6-4.rom2", 0x4000000, 0x4000000, CRC(62adda8d) SHA1(e4fa94b2aa321f7d27666f6a55aaa871c8a0ed85) )
ROM_END

ROM_START( gamt10f )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-92-5.6-4.rom1", 0x0000000, 0x4000000, CRC(6079b346) SHA1(9fc90c47c394cc86771bce365c9abe98bc1b70d9) ) /* V5.6-4 Dec 04 2006 */
	ROM_LOAD( "gamt10-9x-5.6-4.rom2", 0x4000000, 0x4000000, CRC(62adda8d) SHA1(e4fa94b2aa321f7d27666f6a55aaa871c8a0ed85) )
ROM_END

ROM_START( gamt10g )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-94-5.6-4.rom1", 0x0000000, 0x4000000, CRC(880a3374) SHA1(defbd9273b3b84cc2a6a4b784dada582b9d8a399) ) /* V5.6-4 Dec 04 2006 */
	ROM_LOAD( "gamt10-9x-5.6-4.rom2", 0x4000000, 0x4000000, CRC(62adda8d) SHA1(e4fa94b2aa321f7d27666f6a55aaa871c8a0ed85) )
ROM_END

ROM_START( gamt10gmult )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-94-5.6-4.rom1", 0x0000000, 0x4000000, CRC(880a3374) SHA1(defbd9273b3b84cc2a6a4b784dada582b9d8a399) ) /* V5.6-4 Dec 04 2006 */
	ROM_LOAD( "g1094_v5.6-4_2", 0x4000000, 0x4000000, CRC(5ad52530) SHA1(1aa300d65e790a8e83e35068f0b7656c0afaaa67) )
ROM_END



ROM_START( gamt10h )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-90-5.6-5.rom1", 0x0000000, 0x4000000, CRC(d76c962e) SHA1(d2441e447ff3a723692a9ddc2d40feaa45138581) ) /* V5.6-5 Feb 27 2007 */
	ROM_LOAD( "gamt10-9x-5.6-5.rom2", 0x4000000, 0x4000000, CRC(85c674ae) SHA1(b60253b881ad043c15f60d1f309172ec85a974e7) )
ROM_END

ROM_START( gamt10i )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-xx-5.6-5.rom1", 0x0000000, 0x4000000, CRC(52b75752) SHA1(96291da84650c91b5d08a4c18e8ff5a3c12723fa) ) /* V5.6-5 Feb 27 2007 */
	ROM_LOAD( "gamt10-9x-5.6-5.rom2", 0x4000000, 0x4000000, CRC(85c674ae) SHA1(b60253b881ad043c15f60d1f309172ec85a974e7) )
ROM_END

ROM_START( gamt10j )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-90-5.6-8.rom1", 0x0000000, 0x4000000, CRC(ed65a443) SHA1(786154b0e62432e8e3b5394cef57d983588e6632) ) /* V5.6-8 Jul 04 2007 */
	ROM_LOAD( "gamt10-9x-5.6-8.rom2", 0x4000000, 0x4000000, CRC(63943c1b) SHA1(649f1b2da380c996d2aa2f04fca4ad17b686e79f) )
ROM_END

ROM_START( gamt10k )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-92-5.6-8.rom1", 0x0000000, 0x4000000, CRC(68be653f) SHA1(a850fb7913bd5b60e4f4742edc43530a5f9835fc) ) /* V5.6-8 Jul 04 2007 */
	ROM_LOAD( "gamt10-9x-5.6-8.rom2", 0x4000000, 0x4000000, CRC(63943c1b) SHA1(649f1b2da380c996d2aa2f04fca4ad17b686e79f) )
ROM_END

ROM_START( gamt10l )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-94-5.6-8.rom1", 0x0000000, 0x4000000, CRC(80cde50d) SHA1(a582f3d1e2a187492a131f84ddd8cc8b79a54973) ) /* V5.6-8 Jul 04 2007 */
	ROM_LOAD( "gamt10-9x-5.6-8.rom2", 0x4000000, 0x4000000, CRC(63943c1b) SHA1(649f1b2da380c996d2aa2f04fca4ad17b686e79f) )
ROM_END

ROM_START( gamt10m )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-90-5.6-10.rom1", 0x0000000, 0x4000000, CRC(4a3b02ed) SHA1(70d30221b44d5a24f415fb5c12cd0cf43d479127) ) /* V5.6-10 Oct 25 2007 */
	ROM_LOAD( "gamt10-9x-5.6-10.rom2", 0x4000000, 0x4000000, CRC(caad5ae9) SHA1(2812c3c3e73a7d82c77030e6c327e56f8f934891) )
ROM_END

ROM_START( gamt10n )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-92-5.6-10.rom1", 0x0000000, 0x4000000, CRC(cfe0c391) SHA1(898d3f657c755e9ab574f1b31593e9a7173c4026) ) /* V5.6-10 Oct 25 2007 */
	ROM_LOAD( "gamt10-9x-5.6-10.rom2", 0x4000000, 0x4000000, CRC(caad5ae9) SHA1(2812c3c3e73a7d82c77030e6c327e56f8f934891) )
ROM_END

ROM_START( gamt10o )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt10-94-5.6-10.rom1", 0x0000000, 0x4000000, CRC(279343a3) SHA1(2fe7116317051d48ed95485ca210cc3a4e60a639) ) /* V5.6-10 Oct 25 2007 */
	ROM_LOAD( "gamt10-9x-5.6-10.rom2", 0x4000000, 0x4000000, CRC(caad5ae9) SHA1(2812c3c3e73a7d82c77030e6c327e56f8f934891) )
ROM_END

/* Gaminator 11 */

ROM_START( gamt11 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt11-94-5.5-8.rom1", 0x0000000, 0x4000000, CRC(aecd4412) SHA1(758c46fc8fcdb491e5f3746d54d0c3fd07eb5205) ) /* V5.5-8 Nov 17 2005 */
	ROM_LOAD( "gamt11-9x-5.5-8.rom2", 0x4000000, 0x4000000, CRC(8df8c5d2) SHA1(edd004f0c1b8403187e370cae901cf6c9a7e4e30) )
ROM_END

ROM_START( gamt11a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt11-96-5.5-8.rom1", 0x0000000, 0x4000000, CRC(f9949aed) SHA1(8b589747e5ed3d27292ab57e9c052c58523b73fb) ) /* V5.5-8 Nov 17 2005 */
	ROM_LOAD( "gamt11-9x-5.5-8.rom2", 0x4000000, 0x4000000, CRC(8df8c5d2) SHA1(edd004f0c1b8403187e370cae901cf6c9a7e4e30) )
ROM_END

ROM_START( gamt11b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt11-94-5.6-0.rom1", 0x0000000, 0x4000000, CRC(d591cde6) SHA1(5769962209e7364426d2c24149229e45e5531e73) ) /* V5.6-0 Sep 11 2006 */
	ROM_LOAD( "gamt11-9x-5.6-0.rom2", 0x4000000, 0x4000000, CRC(98889261) SHA1(aad17a810a47b254023301740ca9d05c1060e4b4) )
ROM_END

ROM_START( gamt11bmult )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt11-94-5.6-0.rom1", 0x0000000, 0x4000000, CRC(d591cde6) SHA1(5769962209e7364426d2c24149229e45e5531e73) ) /* V5.6-0 Sep 11 2006 */
	ROM_LOAD( "gam_11_94_5.6-0_2.rom", 0x4000000, 0x4000000, CRC(e1e476c7) SHA1(c513df590d0a8e9d26fc58a066cf9d3a1a77cc7a) )
ROM_END



ROM_START( gamt11c )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt11-xx-5.6-5.rom1", 0x0000000, 0x4000000, CRC(e1ae549f) SHA1(2bf4aa23dd67de526676d4a56da1fed2f4316047) ) /* V5.6-5 Mar 21 2007 */
	ROM_LOAD( "gamt11-9x-5.6-5.rom2", 0x4000000, 0x4000000, CRC(c69ada8e) SHA1(7519ccf4279dae6a2e661fa2dee874e48f3c7b42) )
ROM_END

/* Gaminator 12 */

ROM_START( gamt12 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt12-96-5.5-6.rom1", 0x0000000, 0x4000000, CRC(875d9879) SHA1(467c8f3ea6bbfb2527cfac00b73991ff185eccf7) ) /* V5.5-6 Oct 17 2005 */
	ROM_LOAD( "gamt12-9x-5.5-6.rom2", 0x4000000, 0x4000000, CRC(a5a08cb1) SHA1(73de85869c7762c34658b8619120879e29eeed8c) )
ROM_END

ROM_START( gamt12a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt12-96-5.6-0.rom1", 0x0000000, 0x4000000, CRC(286dd471) SHA1(6f8cdb9efa488450d41348887403c5100fd9a70e) ) /* V5.6-0 Sep 07 2006 */
	ROM_LOAD( "gamt12-9x-5.6-0.rom2", 0x4000000, 0x4000000, CRC(4d9a8dc4) SHA1(5605bd3dc10bccecb4ec17641a8b63dc3bf4c07e) )
ROM_END

ROM_START( gamt12b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt12-96-5.6-5.rom1", 0x0000000, 0x4000000, CRC(641458af) SHA1(a96ea356996b0c7e41e420655d602bf1122dcac4) ) /* V5.6-5 Apr 12 2007 */
	ROM_LOAD( "gamt12-9x-5.6-5.rom2", 0x4000000, 0x4000000, CRC(078aba43) SHA1(f23a119bea5d97b9e478fe05af2da3328db064ac) )
ROM_END

/* Gaminator 16 */

ROM_START( gamt16 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt16-94-5.5-10.rom1", 0x0000000, 0x4000000, CRC(36dc53ae) SHA1(014a2605f465fc8b073d04343dddb5a665a9f9bc) ) /* V5.5-10 Apr 19 2006 */
	ROM_LOAD( "gamt16-9x-5.5-10.rom2", 0x4000000, 0x4000000, CRC(86ecdc45) SHA1(f14c1c8914bd881cb095d5667685f00b8fd28743) )
ROM_END

ROM_START( gamt16a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt16-92-5.5-10.rom1", 0x0000000, 0x4000000, CRC(70f304a3) SHA1(d4161a72e24f29c53d5f89abf5b68ad3348c09b2) ) /* V5.5-10 Apr 19 2006 */
	ROM_LOAD( "gamt16-9x-5.5-10.rom2", 0x4000000, 0x4000000, CRC(86ecdc45) SHA1(f14c1c8914bd881cb095d5667685f00b8fd28743) )
ROM_END

ROM_START( gamt16b ) /* not sure of the version number, was marked 5.5-10 but rom 2 doesn't match above either, could be completely mislabeled */
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt16-96-5.5-10x.rom1", 0x0000000, 0x4000000, NO_DUMP ) /* was identical to file below! */
	ROM_LOAD( "gamt16-9x-5.5-10x.rom2", 0x4000000, 0x4000000, CRC(00e5d642) SHA1(c52e5e3b8c6e132e1b06c2ddffc8538ea9e53822) )
ROM_END

ROM_START( gamt16c )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor1692_@1_5.6-1.rom", 0x0000000, 0x4000000, CRC(b56420dc) SHA1(389c717eaf51a21f286c526c36fbc3f39929e00f) ) /* V5.6-1 Sep 20 2006 */
	ROM_LOAD( "gamt16-9x-5.6-1.rom2", 0x4000000, 0x4000000, CRC(b20d5d5d) SHA1(8ae296f846488bf334f7785bc584a4b88a0bc504) )
ROM_END

ROM_START( gamt16d )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor1696_@1_5.6-1.rom", 0x0000000, 0x4000000, CRC(a412a92e) SHA1(748e6a2bb33d8d32004d95c9075630a24859d2b8) ) /* V5.6-1 Sep 20 2006 */
	ROM_LOAD( "gamt16-9x-5.6-1.rom2", 0x4000000, 0x4000000, CRC(b20d5d5d) SHA1(8ae296f846488bf334f7785bc584a4b88a0bc504) )
ROM_END

ROM_START( gamt16e )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "16_94-5.6-1_1", 0x0000000, 0x4000000, CRC(f34b77d1) SHA1(4aad3285b294e522875db62b046eaa37f90652c7) ) /* V5.6-1 Sep 20 2006 */
	ROM_LOAD( "gamt16-9x-5.6-1.rom2", 0x4000000, 0x4000000, CRC(b20d5d5d) SHA1(8ae296f846488bf334f7785bc584a4b88a0bc504) )
ROM_END

ROM_START( gamt16f )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor1692v5.6-5_@1.bin", 0x0000000, 0x4000000, CRC(3530bdc1) SHA1(82a72fe38100fb88b679fe50dab14c1035501579) ) /* V5.6-5 Feb 28 2007 */
	ROM_LOAD( "gamt16-9x-5.6-5.rom2", 0x4000000, 0x4000000, CRC(9484ddec) SHA1(03a21d231496d364f48173064027d6871a3f3fc0) )
ROM_END

ROM_START( gamt16fmult )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor1692v5.6-5_@1.bin", 0x0000000, 0x4000000, CRC(3530bdc1) SHA1(82a72fe38100fb88b679fe50dab14c1035501579) ) /* V5.6-5 Feb 28 2007 */
	ROM_LOAD( "g16 92_v5.6-5_2", 0x4000000, 0x4000000, CRC(bfad3326) SHA1(b94ac12130563295f2b99d77205a211118f95f37) )
ROM_END



ROM_START( gamt16g )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor1694_@1_5.6-5.rom", 0x0000000, 0x4000000, CRC(dd433df3) SHA1(b4a89cf4e190c1d32900fa5425a6b1326c722e6d) ) /* V5.6-5 Feb 28 2007 */
	ROM_LOAD( "gamt16-9x-5.6-5.rom2", 0x4000000, 0x4000000, CRC(9484ddec) SHA1(03a21d231496d364f48173064027d6871a3f3fc0) )
ROM_END

ROM_START( gamt16h )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g16_92_5.6-8_@1", 0x0000000, 0x4000000, CRC(9ad16d5e) SHA1(373e5c49553cb4a4c2f9a9ab2dd15fa45df11ebe) ) /* V5.6-8 Jul 04 2007 */
	ROM_LOAD( "gamt16-9x-5.6-8.rom2", 0x4000000, 0x4000000, CRC(53079651) SHA1(4f4a5332dd7a144bcb3b96540f2f124876f03abc) )
ROM_END

ROM_START( gamt16i )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g16_94_5.6-8_@1", 0x0000000, 0x4000000, CRC(72a2ed6c) SHA1(16969260e960f3a62dccc9a1e3763abcdaa1fa99) ) /* V5.6-8 Jul 04 2007 */
	ROM_LOAD( "gamt16-9x-5.6-8.rom2", 0x4000000, 0x4000000, CRC(53079651) SHA1(4f4a5332dd7a144bcb3b96540f2f124876f03abc) )
ROM_END

ROM_START( gamt16j )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g16_92_5.6-10_@1", 0x0000000, 0x4000000, CRC(ca4bf3da) SHA1(5fdef7b46c5d51bf16228c9541b18603bce72b51) ) /* V5.6-10 Oct 25 2007 */
	ROM_LOAD( "gamt16-9x-5.6-10.rom2", 0x4000000, 0x4000000, CRC(e6fc0478) SHA1(2bf998471ef86bccc45332551738b5fc3f70d188) )
ROM_END

ROM_START( gamt16k )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g16_94_5.6-10_@1", 0x0000000, 0x4000000, CRC(223873e8) SHA1(f57c80ba710529f011ee505fa8e1e6a0af70bc4e) ) /* V5.6-10 Oct 25 2007 */
	ROM_LOAD( "gamt16-9x-5.6-10.rom2", 0x4000000, 0x4000000, CRC(e6fc0478) SHA1(2bf998471ef86bccc45332551738b5fc3f70d188) )
ROM_END

/* Gaminator 17 */

ROM_START( gamt17 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor1792_@1_5.5-10.rom", 0x0000000, 0x4000000, CRC(8da74797) SHA1(f7a654682e870169d1ce2901ac61d2f60ae2d568) ) /*V5.5-10 May 29 2006 */
	ROM_LOAD( "gamtor1792_@2_5.5-10.rom", 0x4000000, 0x4000000, CRC(9348cb69) SHA1(604060ad6fb2d37cf3d449390e5cfb685d74b11d) )
ROM_END

ROM_START( gamt17a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor1794_@1_5.5-10.rom", 0x0000000, 0x4000000, CRC(cb88109a) SHA1(c059553c7ce19766007a669b21994b7c81cf320d) ) /*V5.5-10 May 29 2006 */
	ROM_LOAD( "gamtor1794_@2_5.5-10.rom", 0x4000000, 0x4000000, CRC(9348cb69) SHA1(604060ad6fb2d37cf3d449390e5cfb685d74b11d) )
ROM_END

ROM_START( gamt17b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor1794_@1_5.6-0", 0x0000000, 0x4000000, CRC(ce6bc732) SHA1(b745938bbcb2dcb6a8ac3d7f46f9919124d9e7ed) ) /* V5.6-0 Sep 04 2006 */
	ROM_LOAD( "gamtor1794_@2_5.6-0", 0x4000000, 0x4000000, CRC(120f4c40) SHA1(f2450dd1ed2127a4005510baedf5781100556e26) )
ROM_END

/* Gaminator 18 */

ROM_START( gamt18 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "18_94_5-5-10_1", 0x0000000, 0x4000000, CRC(c054009f) SHA1(b75b339ae06b005fcd0c78302aceaf64069a6345) ) /* V5.5-10 Jun 19 2006 */
	ROM_LOAD( "18_94_5-5-10_2", 0x4000000, 0x4000000, CRC(2d798567) SHA1(8e6d36d1d5ae04d7c1bb48d816a865fa074fc24e) )
ROM_END

ROM_START( gamt18a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g18_96_5.6-0_1.bin", 0x0000000, 0x4000000, CRC(ba25423d) SHA1(455e187c9da7d9c38517794069aa43841af39b96) ) /* V5.6.0 Sep 20 2006 */
	ROM_LOAD( "g18_96_5.6-0_2.bin", 0x4000000, 0x4000000, CRC(e82c1bd5) SHA1(1dfde1185fc9a4a717529fcbd841f6a9bed3e8cc) )
ROM_END

ROM_START( gamt18b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g18_94_5-6-0_1", 0x0000000, 0x4000000, CRC(ed7c9cc2) SHA1(558385c03f575b217ca8ba7cbc37603bd7b70fb5) ) /* V5.6-0 Sep 20 2006 */
	ROM_LOAD( "g18_94_5-6-0_2", 0x4000000, 0x4000000, CRC(e82c1bd5) SHA1(1dfde1185fc9a4a717529fcbd841f6a9bed3e8cc) )
ROM_END

ROM_START( gamt18bmult )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g18_94_5-6-0_1", 0x0000000, 0x4000000, CRC(ed7c9cc2) SHA1(558385c03f575b217ca8ba7cbc37603bd7b70fb5) ) /* V5.6-0 Sep 20 2006 */
	ROM_LOAD( "g18_94_5.6-0_2.rom", 0x4000000, 0x4000000, CRC(1d9e7456) SHA1(3c9b56816dfa7b6f782f27510f69a2bf40b3415f) )
ROM_END




ROM_START( gamt18c )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor18_94_5.6-5@1.rom", 0x0000000, 0x4000000, CRC(2095957f) SHA1(a0ae6275875cd77999e105a88c0a8334efc2dd53) ) /* 5.6-5 Mar 16 2007 */
	ROM_LOAD( "gamtor18_94_5.6-5@2.rom", 0x4000000, 0x4000000, CRC(892bbcb3) SHA1(7dd6a37ce1cb4371c00915ef5a61eef96385ccd1) )
ROM_END

ROM_START( gamt18d )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor18_92_5.6-5@1.rom", 0x0000000, 0x4000000, CRC(c8e6154d) SHA1(60c46196f0f9192717a01acab87304af3600e37c) ) /* V5.6-5 Mar 16 2007 */
	ROM_LOAD( "gamtor18_92_5.6-5@2.rom", 0x4000000, 0x4000000, CRC(892bbcb3) SHA1(7dd6a37ce1cb4371c00915ef5a61eef96385ccd1) )
ROM_END

/* Gaminator 19 */

ROM_START( gamt19 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor1994_5.6-0@1.rom", 0x0000000, 0x4000000, CRC(a3bc17c4) SHA1(ce1e0efb578c5de9e4f27f60a30cd291071568aa) ) /* V5.6-0 Aug 02 2006 */
	ROM_LOAD( "gamtor1994_5.6-0@2.rom", 0x4000000, 0x4000000, CRC(91093b37) SHA1(2fa01f2a88534afd55dcc96357ab388e58dd297e) )
ROM_END

ROM_START( gamt19mult )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor1994_5.6-0@1.rom", 0x0000000, 0x4000000, CRC(a3bc17c4) SHA1(ce1e0efb578c5de9e4f27f60a30cd291071568aa) ) /* V5.6-0 Aug 02 2006 */
	ROM_LOAD( "19_94_5.6-1_2.rom", 0x4000000, 0x4000000, CRC(518c7400) SHA1(1b9ed91bdfc677bb0f7ead331369d40e85452838) )
ROM_END



ROM_START( gamt19a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor19_94_5.6-5@1.rom", 0x0000000, 0x4000000, CRC(84f480ad) SHA1(aafd1264bed9f97ccc359343a202c6a9783dca27) ) /* V5.6-5 Mar 19 2007 */
	ROM_LOAD( "gamtor19_94_5.6-5@2.rom", 0x4000000, 0x4000000, CRC(33ca96d6) SHA1(a2129be5a352a92e2ee38ca535616506a07dd73b) )
ROM_END

/* Gaminator 20 */

ROM_START( gamt20 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g20-560-1", 0x0000000, 0x4000000, CRC(c9108625) SHA1(169282339216408930f6c43e9e0002e3a26f27b9) ) /* V5.6-0 Oct 09 2006 */
	ROM_LOAD( "g20-560-2", 0x4000000, 0x4000000, CRC(bc5ba9aa) SHA1(ef37163192dcf26b03469d0f2579c817c2a3bc54) )
ROM_END

ROM_START( gamt20a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g20_94_5.6-5_@1", 0x0000000, 0x4000000, CRC(efb1d6ad) SHA1(0dbbed1f4561c85fa6388dde5e7cc8dcaad80e2b) ) /* V5.6-5 Apr 02 2007 */
	ROM_LOAD( "g20_94_5.6-5_@2", 0x4000000, 0x4000000, CRC(560fe75b) SHA1(721d677140eec884571f629aa07538cd3183cb40) )
ROM_END

ROM_START( gamt20b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor2092v5.6-5_@1.bin", 0x0000000, 0x4000000, CRC(07c2569f) SHA1(b335f6b7ff89ebfdee0cbb21fffedfa04336a85f) ) /* v5.6-5 Apr 02 2007 */
	ROM_LOAD( "gamtor2092v5.6-5_@2.bin", 0x4000000, 0x4000000, CRC(560fe75b) SHA1(721d677140eec884571f629aa07538cd3183cb40) )
ROM_END

/* Gaminator 21 */

ROM_START( gamt21 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor21_92_5.6-5.rom1", 0x0000000, 0x4000000, CRC(e0de022f) SHA1(a29107ee906883cabe0a31be843c20b29541846a) ) /* V5.6-5 Feb 26 2007 */
	ROM_LOAD( "gamtor21_9x_5.6-5.rom2", 0x4000000, 0x4000000, CRC(c8e6674f) SHA1(0d7fd9489d32faf831f16da79151c7ad0a123287) )
ROM_END

ROM_START( gamt21a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor21_94_5.6-5.rom1", 0x0000000, 0x4000000, CRC(08ad821d) SHA1(23907398ea83f1ce5c705582f2138e6e8e08f011) ) /* V5.6-5 Feb 26 2007 */
	ROM_LOAD( "gamtor21_9x_5.6-5.rom2", 0x4000000, 0x4000000, CRC(c8e6674f) SHA1(0d7fd9489d32faf831f16da79151c7ad0a123287) )
ROM_END

ROM_START( gamt21amult )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor21_94_5.6-5.rom1", 0x0000000, 0x4000000, CRC(08ad821d) SHA1(23907398ea83f1ce5c705582f2138e6e8e08f011) ) /* V5.6-5 Feb 26 2007 */
	ROM_LOAD( "21_94_5.6-5_2.rom", 0x4000000, 0x4000000, CRC(92b36a02) SHA1(e13fd3b122a87251e3735dce6a38d5f966997a7e) )
ROM_END



/* Gaminator 22 */

ROM_START( gamt22 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor2290_@1_5.6-5.rom", 0x0000000, 0x4000000, CRC(3b1a6e9d) SHA1(f7c5225195427acd350c5c281708f60a7f0e20cb) ) /* V5.6-5 Mar 06 2007 */
	ROM_LOAD( "gamtor2290_@2_5.6-5.rom", 0x4000000, 0x4000000, CRC(d64140fc) SHA1(13324082b13beb5f72485fdf5f66ed4323ce94aa) )
ROM_END

ROM_START( gamt22a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor2292_@1_5.6-5.rom", 0x0000000, 0x4000000, CRC(bec1afe1) SHA1(2304aac2aacd78358c12ed3a797c7cda36990324) ) /* V5.6-5 Mar 06 2007 */
	ROM_LOAD( "gamtor2292_@2_5.6-5.rom", 0x4000000, 0x4000000, CRC(d64140fc) SHA1(13324082b13beb5f72485fdf5f66ed4323ce94aa) )
ROM_END

ROM_START( gamt22amult )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor2292_@1_5.6-5.rom", 0x0000000, 0x4000000, CRC(bec1afe1) SHA1(2304aac2aacd78358c12ed3a797c7cda36990324) ) /* V5.6-5 Mar 06 2007 */
	ROM_LOAD( "g22_2", 0x4000000, 0x4000000, CRC(91fd9d7b) SHA1(786f54b8ecc607f9a99db068590e7dbc718fb66a) )
ROM_END


ROM_START( gamt22b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor2294_@1_5.6-5.rom", 0x0000000, 0x4000000, CRC(56b22fd3) SHA1(aac67a6d20889ad69a567aa7f0ea71a89e2b5591) ) /* V5.6-5 Mar 06 2007 */
	ROM_LOAD( "gamtor2294_@2_5.6-5.rom", 0x4000000, 0x4000000, CRC(d64140fc) SHA1(13324082b13beb5f72485fdf5f66ed4323ce94aa) )
ROM_END

/* Gaminator 23 */

ROM_START( gamt23 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor2390_@1_5.6-5.rom", 0x0000000, 0x4000000, CRC(45008770) SHA1(67e49d4055c1b79adef052d97fa0e59cc1031330) ) /* V5.6-5 Apr 24 2007 */
	ROM_LOAD( "gamtor2390_@2_5.6-5.rom", 0x4000000, 0x4000000, CRC(be9c9b96) SHA1(9e6b2d97b14fa5bf94cacf2b658b5d85f6a3dd5f) )
ROM_END

ROM_START( gamt23a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor2392_@1_5.6-5.rom", 0x0000000, 0x4000000, CRC(c0db460c) SHA1(8835f7fb7397a2712481ee1701283a97e3a785e6) ) /* V5.6-5 Apr 24 2007 */
	ROM_LOAD( "gamtor2392_@2_5.6-5.rom", 0x4000000, 0x4000000, CRC(be9c9b96) SHA1(9e6b2d97b14fa5bf94cacf2b658b5d85f6a3dd5f) )
ROM_END

ROM_START( gamt23b )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamtor2394_@1_5.6-5.rom", 0x0000000, 0x4000000, CRC(28a8c63e) SHA1(1970ac0bf6f63c495c77d758e2ca9fb919b5b80e) ) /* V5.6-5 Apr 24 2007 */
	ROM_LOAD( "gamtor2394_@2_5.6-5.rom", 0x4000000, 0x4000000, CRC(be9c9b96) SHA1(9e6b2d97b14fa5bf94cacf2b658b5d85f6a3dd5f) )
ROM_END

/* Gaminator 29 */

ROM_START( gamt29 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g29-5-6-8-92-1", 0x0000000, 0x4000000, CRC(d64043ce) SHA1(ea00e85206b49842436453d323384f6b42981c96) ) /* V5.6-8 Jun 29 2007 */
	ROM_LOAD( "g29-5-6-8-92-2", 0x4000000, 0x4000000, CRC(6a441fb3) SHA1(a986873b378e51b698812e3f5a6c23075e061d49) )
ROM_END

ROM_START( gamt29a )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g29-5-6-8-94-1", 0x0000000, 0x4000000, CRC(3e33c3fc) SHA1(c536b3df0e673ed7eacaf9ac57dbaa54af22257e) ) /* V5.6-8 Jun 29 2007 */
	ROM_LOAD( "g29-5-6-8-94-2", 0x4000000, 0x4000000, CRC(6a441fb3) SHA1(a986873b378e51b698812e3f5a6c23075e061d49) )
ROM_END

/* Gaminator 30 */

ROM_START( gamt30 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g30v5.6-10@1.rom", 0x0000000, 0x4000000, CRC(e4d5dcce) SHA1(417169de53c7d9110968d087cfe5d611f6875fcc) ) /* V5.6-10 Dec 14 2007 */
	ROM_LOAD( "g30v5.6-10@2.rom", 0x4000000, 0x4000000, CRC(f5d15593) SHA1(4268115dbbbb710dcfb511d4ed4eccc6b382a9c8) )
ROM_END

/* Gaminator 31 */

ROM_START( gamt31 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g31v5.6-10@1.rom", 0x0000000, 0x4000000, CRC(770a3c19) SHA1(082a5e4d4d11803cde1981c3201a6b174b4d86ea) ) /* V5.6-10 Dec 13 2007 */
	ROM_LOAD( "g31v5.6-10@2.rom", 0x4000000, 0x4000000, CRC(88107247) SHA1(d9c2eabc0d92b7e8b93923fe053c7b0a93c28e76) )
ROM_END

ROM_START( gamt31mult )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g3194_v5.6-12_1", 0x0000000, 0x4000000, CRC(0b873052) SHA1(7b072346ae14bfe475547bdcad946be2a7e6cf2f) )
	ROM_LOAD( "g3194_v5.6-12_2", 0x4000000, 0x4000000, CRC(4cd4ba07) SHA1(b146db07134b4fa28377f45cdd22ef19bb4b2262) )
ROM_END



/* Hot Spot 2 */

ROM_START( hspot2 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "hotspot494v5.6-5@1", 0x0000000, 0x4000000, CRC(04f482a5) SHA1(84ab552cd7c776b70889249a9809008584c6de6c) )
	ROM_LOAD( "hotspot494v5.6-5@2", 0x4000000, 0x4000000, CRC(99e58027) SHA1(c78f019b843d39f45c58c77295e19d07ff6f251d) )
ROM_END

/* Hot Spot 3 */

ROM_START( hspot3 )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "hs3_5425_94_1", 0x0000000, 0x4000000, CRC(5265a518) SHA1(fc60aa91716c583a63984f7f68fe9a1c1834834e) )
	ROM_LOAD( "hs3_5425_94_2", 0x4000000, 0x4000000, CRC(b499c667) SHA1(58dd6cf2a4564cf7867ac72308c84310f918ca88) )
ROM_END

/* Mega Katok 2 */

ROM_START( megakat )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "megakatok2_94_5.6-0_@1.rom", 0x0000000, 0x4000000, CRC(39ef31ec) SHA1(7a04c15d37b2921a475294dd9e474be4c2a1a0db) )
	ROM_LOAD( "megakatok2_94_5.6-0_@2.rom", 0x4000000, 0x4000000, CRC(d2024952) SHA1(fcc1c7574eeef0ae26ba80d30fac3e74b8a9dbf2) )
ROM_END


ROM_START( gamt1lotc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g1.1.bin", 0x0000000, 0x4000000, CRC(ce2a8ae7) SHA1(61b81257659271f1686624824b7e13c76e2cc182) )
	ROM_LOAD( "g1.2.bin", 0x4000000, 0x4000000, CRC(1c8732ba) SHA1(ff3d71ec4650dbad0785ddda135f8726b5cfb493) )
ROM_END

ROM_START( gamt4lotc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g4_@1", 0x0000000, 0x4000000, CRC(5c36b5dd) SHA1(e89e4e88ba230380d738b2a0b7ce49131387e0ee) )
	ROM_LOAD( "g4_@2", 0x4000000, 0x4000000, CRC(0222514a) SHA1(a18e1ba91de12742c332866734ba45d88ad23179) )
ROM_END

ROM_START( gamt6lotc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g6_@1", 0x0000000, 0x4000000, CRC(380f71f2) SHA1(0a96a06774d242ea86c8eb578dcd39b2df0d1452) )
	ROM_LOAD( "g6_@2", 0x4000000, 0x4000000, CRC(6ec0b445) SHA1(b0ea41775c7355ed704943363fd8fe79179f321d) )
ROM_END

ROM_START( gamt8lotc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g8_@1", 0x0000000, 0x4000000, CRC(e98e2154) SHA1(1a7be1d8c5a4ce71cf7fa5a6dfd36841aeb14eb0) )
	ROM_LOAD( "g8_@2", 0x4000000, 0x4000000, CRC(23d8339d) SHA1(e477a15bd8fc08c4f083cc2207d2a616f15ec06d) )
ROM_END

ROM_START( gamt9lotc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g9.1.bin", 0x0000000, 0x4000000, CRC(d108aa10) SHA1(347d71c094d31eb8022430e6eec0b309041a1606) )
	ROM_LOAD( "g9.2.bin", 0x4000000, 0x4000000, CRC(ccc0475c) SHA1(7f6acfb0c851814b4d0187ce66b6294ea617d6f4) )
ROM_END

ROM_START( gamt10lotc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g10_@1", 0x0000000, 0x4000000, CRC(52590121) SHA1(5dce141833552ecc5a2b32f309ec1356a14f1019) )
	ROM_LOAD( "g10_@2", 0x4000000, 0x4000000, CRC(5af9da32) SHA1(fe9ab8d5c8d785f61c7ed8a5fca63be07db29611) )
ROM_END

ROM_START( gamt16lotc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g16_@1", 0x0000000, 0x4000000, CRC(40171bf5) SHA1(f576f7ca72d5972096ed98963d809838e34d6fb0) )
	ROM_LOAD( "g16_@2", 0x4000000, 0x4000000, CRC(6a00b2f1) SHA1(ed7a533b94520050ae6a9be9344d3a7587fa3ebb) )
ROM_END

ROM_START( gamt18lotc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "res01", 0x0000000, 0x4000000, CRC(54875320) SHA1(5876c70f4f22a576bdccbe641ea5e78f4b2d3f73) )
	ROM_LOAD( "res02", 0x4000000, 0x4000000, CRC(41af4a09) SHA1(cde677935ff454137f5b02c9d22bcec4759bb6fe) )
ROM_END

ROM_START( gamt19lotc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g_19_1.bin", 0x0000000, 0x4000000, CRC(4ea23b2b) SHA1(6ef7c82510fbc3ad871567314fec476077d393c7) )
	ROM_LOAD( "g_19_2.bin", 0x4000000, 0x4000000, CRC(b37c18b7) SHA1(3ee0e19f3f1671aa51b73436a990dc618a18642b) )
ROM_END

ROM_START( gamt20lotc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g 20.1.bin", 0x0000000, 0x4000000, CRC(a7f892b2) SHA1(eab445a24e04df6ea1c59941b6bbd30c7164970c) )
	ROM_LOAD( "g 20.2.bin", 0x4000000, 0x4000000, CRC(3053d4f8) SHA1(ba09a3580aef5fb13470ae5ab20c829eabba5ced) )
ROM_END

ROM_START( gamt4lotca ) // zip was 'mk4.zip', might be something else (mulitkarot?) looks scrambled or bad
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gamt4lotca_read@1.bin", 0x0000000, 0x4000000, CRC(bb6f730d) SHA1(f4bd96b8b2ea8bd06961747f63584180f41f6a13) )
	ROM_LOAD( "gamt4lotca_read@2.bin", 0x4000000, 0x4000000, CRC(d33f55b4) SHA1(f3f822998977e485f0f7c2e7335602490fc13ba7) )
ROM_END


ROM_START( gamt4lotm )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g_4_1.bin", 0x0000000, 0x4000000, CRC(bec5e028) SHA1(091fd37883225373f65fcaafcc9e3be465a4fca0) )
	ROM_LOAD( "g_4_2.bin", 0x4000000, 0x4000000, CRC(83d03258) SHA1(8b6bdd8a112cd3e9245317b4311922ae9017899f) )
ROM_END

ROM_START( gamt10lotm )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g_10_1.bin", 0x0000000, 0x4000000, CRC(8d48577f) SHA1(2dc2aeb5063bcf2f10ae4d08cd59eab649f6f3c0) )
	ROM_LOAD( "g_10_2.bin", 0x4000000, 0x4000000, CRC(986300e7) SHA1(b64e31375446bd0bdaab0f983d94a0e55a1c3e69) )
ROM_END

ROM_START( gamt20lotm )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "g_20_1.bin", 0x0000000, 0x4000000, CRC(a914c063) SHA1(8a1b9cc69241eb2f533697e4e92e7bb8aee5724b) )
	ROM_LOAD( "g_20_2.bin", 0x4000000, 0x4000000, CRC(17ccd29c) SHA1(7e3f1ca89f660e8a018a7e8a9c240cc2bab36760) )
ROM_END



ROM_START( gamt1ent )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gr01.5_6-0.01.rom", 0x0000000, 0x4000000, CRC(d9a48096) SHA1(a636e1c59c3286e5b3f09c719fb3f80101c01c8e) )
	ROM_LOAD( "gr01.5_6-0.02.rom", 0x4000000, 0x4000000, CRC(fcdb6726) SHA1(1f5d079fa9456276ff45477b870e30c421b5200d) )
ROM_END

ROM_START( gamt4ent )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "xg04.5_6-0.01.rom", 0x0000000, 0x4000000, CRC(802e908b) SHA1(1da8ff8c3b1afa996b5115fa83ed22f0f10432c9) )
	ROM_LOAD( "xg04.5_6-0.02.rom", 0x4000000, 0x4000000, CRC(30c4efc4) SHA1(919dc461ed75ccea4d33061044befb2caab0a8ed) )
ROM_END

ROM_START( gamt6ent )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gr06.5_6-0.01.rom", 0x0000000, 0x4000000, CRC(58026e26) SHA1(b015ed2037d278167f23c1bb8676a0fe74f2face) )
	ROM_LOAD( "gr06.5_6-0.02.rom", 0x4000000, 0x4000000, CRC(c6b55386) SHA1(adc3a5771480cd515378355722ccb913863c61db) )
ROM_END

ROM_START( gamt10ent )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gr10.5_6-5.01.rom", 0x0000000, 0x4000000, CRC(97e4085c) SHA1(20e5a9ce0c69609c04335f2b52ce79e6b13981c5) )
	ROM_LOAD( "gr10.5_6-5.02.rom", 0x4000000, 0x4000000, CRC(456cfc0b) SHA1(ecf30c602dc590ae75d2996b94d10bf7280a4423) )
ROM_END

ROM_START( gamt18ent )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gr18.5_6-5.01.rom", 0x0000000, 0x4000000, CRC(59f3abba) SHA1(01c3edfa4f3fffb1919a74d50aa38a6ca46e7443) )
	ROM_LOAD( "gr18.5_6-5.02.rom", 0x4000000, 0x4000000, CRC(53fcb2c7) SHA1(217ffcc87c8d9c902c964b72bd09da5631abdab3) )
ROM_END

ROM_START( gamt19ent )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gr19.5_6-5.01.rom", 0x0000000, 0x4000000, CRC(1276fb51) SHA1(5b4746929bc8703bb263d72c8fcaf82f51ca100f) )
	ROM_LOAD( "gr19.5_6-5.02.rom", 0x4000000, 0x4000000, CRC(298391c9) SHA1(5211f088869c61627c23f590871dd98385d9444b) )
ROM_END

ROM_START( gamt20ent )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gr20.5_6-0.01.rom", 0x0000000, 0x4000000, CRC(c391c53d) SHA1(84cbec4e491e7661ebcd48959b9c5f6bbdfb8599) )
	ROM_LOAD( "gr20.5_6-0.02.rom", 0x4000000, 0x4000000, CRC(0dd0fea8) SHA1(39ed9083b374f1af13cc04671fad520e4e7223be) )
ROM_END


// Ancient Atlantis

ROM_START( ancienta )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "atlantis92_5.3-15", 0x0000, 0x2000000, CRC(f26a6465) SHA1(6586b5f2c5c36369371963259bd60abb6867fe2f) )
ROM_END

ROM_START( ancientaa )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "atlantis92_v5.4-25.rom", 0x0000, 0x2000000, CRC(c42388f5) SHA1(631e7d1498dc92a54e2979f3fdac936b4f9dc5b4) )
ROM_END

ROM_START( ancientab )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "atlantis94_v5.3-17.rom", 0x0000, 0x2000000, CRC(dd602a91) SHA1(8c33aa21336464b9192a680b0e6fb71bcc434a74) )
ROM_END

ROM_START( ancientac )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "atlantis96_5.3-17.rom", 0x0000, 0x2000000, CRC(b40e6228) SHA1(884f2b589da1091a5036fe82b884afbb2d1e1e21) )
ROM_END

ROM_START( ancientad )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "atlantis96-v5.4-16.rom", 0x0000, 0x2000000, CRC(cd9d4e3c) SHA1(8304fce99bc0787f0596cb8d4a80f9e737d37379) )
ROM_END

// Bananas Go Bahamas
ROM_START( bananas )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "banana92.rom", 0x0000, 0x2000000, CRC(52d3a923) SHA1(0aa0a2d3cafaa19a4672ac2d34d5442ce238e179) )
ROM_END

ROM_START( bananasa )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "banana94.rom", 0x0000, 0x2000000, CRC(ecdac445) SHA1(a50225a4d3208b622847ec22daaaff392c064f4f) )
ROM_END


// Bee Bop


ROM_START( beebop )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bbop92_5-3-15.rom", 0x0000, 0x2000000, CRC(6f631553) SHA1(77198b400c05cdb935243dc5efa9258f4bd84bea) )
ROM_END

ROM_START( beebopa )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bbop92_v5.3-17.rom", 0x0000, 0x2000000, CRC(e9dd757d) SHA1(c360fa091675e0381d811d29d4437a31bd427341) )
ROM_END

ROM_START( beebopb )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bbop92_5.4-17.rom", 0x0000, 0x2000000, CRC(cf664e8c) SHA1(679ce902be99776b6a82fb40d7e3ea27d9fb576c) )
ROM_END

ROM_START( beebopc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bbop94_5.3-17.rom", 0x0000, 0x2000000, CRC(989441e5) SHA1(b4a5cd4ff7daba87f94c0a22730ad84540d64678) )
ROM_END

ROM_START( beebopd )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bbop96_5.3-17.rom", 0x0000, 0x2000000, CRC(f1fa095c) SHA1(fcc0be64a9b2258c8ee7f6521c49dcc75a9cbf63) )
ROM_END

ROM_START( beebope )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bbop96-v5.4-20.rom", 0x0000, 0x2000000, CRC(cef0508a) SHA1(61603cd9ed4ef3196d8558a4c9b0af837b6e2f78) )
ROM_END


// Beetlemania
ROM_START( beetlem )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "beetle90_5_0_21.rom", 0x0000, 0x1000000, CRC(fe6e8595) SHA1(ec590fd981ae8cb59a65cbfdf53dff6387abde94) )
ROM_END

ROM_START( beetlema )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "beetle92_5.3-17.rom", 0x0000, 0x1000000, CRC(a446abb0) SHA1(f57be7716df8af073de2b1e92278be7b3ee58141) )
ROM_END

ROM_START( beetlemb )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "beetle94_5.3-17.rom", 0x0000, 0x1000000, CRC(bf6df189) SHA1(a60becf235bf0f0dc5336cfc616e1f684e597a7e) )
ROM_END

ROM_START( beetlemc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "beetle96_5.2-12.rom", 0x0000, 0x1000000, CRC(201eb20d) SHA1(d65db2bbad2546b6a15cb950bbc55bba9ed02688) )
ROM_END

ROM_START( beetlemd )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "beetle96_5.3-17.rom", 0x0000, 0x1000000, CRC(5f9433e0) SHA1(0b6d53a6feb31a28f30029cad3626a71c2d46c1f) )
ROM_END



// Bungee Monkey
ROM_START( bungeem )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bm92_1", 0x000000, 0x2000000, CRC(b7beae4d) SHA1(42f93a50daf8cf0e4a31ba5e72732d8c0fd611c9) )
	ROM_LOAD( "bm92_2", 0x2000000, 0x1000000, CRC(3598cdc8) SHA1(5cf8e8d6df031dac0f3894d2a9964640d65612e8) )
ROM_END

ROM_START( bungeema )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bmonkey96@1_5.4-18.rom", 0x000000, 0x2000000, CRC(db3aeef4) SHA1(3edf6d46c92631e87922b7d8b36ffb7dd66b7068) )
	ROM_LOAD( "bmonkey96@2_5.4-18.rom", 0x2000000, 0x1000000, CRC(3598cdc8) SHA1(5cf8e8d6df031dac0f3894d2a9964640d65612e8) )
ROM_END


// Book of Ra
ROM_START( bookra )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bookra94_v5.5-6_@1.bin", 0x0000000, 0x4000000, CRC(a1576b33) SHA1(6820759f11e1946cb2b3d89f3d7c72835e487ad1) )
ROM_END



// Banana Splash
ROM_START( bsplash )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bsplash92.rom", 0x0000, 0x2000000, CRC(5111ab61) SHA1(22b44c92ab7239c1f7c18b47aea2d832c1282b1b) )
ROM_END



// Chilli Con Cash

ROM_START( chillicc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "chilli94_5.4-25.rom", 0x0000, 0x2000000, CRC(888bfc9f) SHA1(e5549229c5d324980bfb1970b67763a705793763) )
ROM_END


// Columbus

ROM_START( columbus )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "columbus90_5_3_15.rom", 0x0000, 0x2000000, CRC(05cfb9d7) SHA1(1702bdce819f577925a51e9427753142d4a1eaa1) )
ROM_END

ROM_START( columbusa )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "columbus92_5.3-17.rom", 0x0000, 0x2000000, CRC(ee67e20b) SHA1(5e86f475497321f43757d54bcbec744221163cbd) )
ROM_END

ROM_START( columbusb ) // bad or scrambled
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "columbus92_5.4-16bad.rom", 0x0000, 0x2000000, CRC(9bda2044) SHA1(c6d974dba387a9a50b51b51942179276b807cc03) )
ROM_END

ROM_START( columbusc )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "columbus94_5.3-17.rom", 0x0000, 0x2000000, CRC(9f2ed693) SHA1(ecc3e9ad207fc883efeb40e87f7de81a5d8db390) )
ROM_END

ROM_START( columbusd )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "columbus94_5.5.-9.rom", 0x0000, 0x2000000, CRC(e05dff9c) SHA1(260eaf580786d6186668bdf2267d3bc253158dbe) )
ROM_END

ROM_START( columbuse )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "columbus96_5.3-17.rom", 0x0000, 0x2000000, CRC(f6409e2a) SHA1(1766b8cc2662929e0af3781a6ec8571e628be7cd) )
ROM_END

ROM_START( columbusf )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "columbus96-v5.4-18.demo.rom", 0x0000, 0x2000000, CRC(cf3b305d) SHA1(3f9816dde3857fa6f8f50644b17ec2b45cbb878c) )
ROM_END



// Diamond Trio

ROM_START( ditrio )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "dtrio_96_5.5-10.rom", 0x0000, 0x2000000, CRC(689b5220) SHA1(1daefb9b7d597d6e35afd598fe3e5737afc53639) )
ROM_END


// Dolphin's Pearl
ROM_START( dolphinp )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "dpearl96_v5.4-16.rom", 0x0000, 0x1000000, CRC(93a015ec) SHA1(6bbf87c8762e04d20710337c1b3a4378360c1d68) )
ROM_END


// The Euro Game

ROM_START( eurogame )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "euro96v5.5-10.rom", 0x0000, 0x2000000, CRC(f08cb518) SHA1(b3f2f454c993a04aedf563cbf75fd96df68318ba) )
ROM_END

ROM_START( eurogamea )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "32mb_eurogame94_", 0x0000, 0x2000000, CRC(f55bcd7f) SHA1(04171cfbd2e2557436ed1485ff389e4b359b1161) )
ROM_END


// First Class Traveller
ROM_START( firstcl )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "fclass_96_5.5-10.rom", 0x0000, 0x2000000, CRC(7729061e) SHA1(1e0e3e7c4478edc5cdbf0d13a974e5b6eeef2d67) )
ROM_END



// Lucky Lady's Charm

ROM_START( llcharm )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "llady94,v5.4-23 7733.bin", 0x0000, 0x2000000, CRC(45fbeb06) SHA1(7ff1df04620bd9e9afd965736c3eff60ac11f409) )
ROM_END

ROM_START( llcharma )
	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "llc_92_5.6-0", 0x0000, 0x2000000, CRC(c8c2a5d3) SHA1(ec23eff63871cc515ec58a894446d4d639d864e4) )
ROM_END

void gaminator_state::init_gaminator()
{
}

} // anonymous namespace


#define GAME_FLAGS MACHINE_NOT_WORKING|MACHINE_NO_SOUND

GAME( 2002?, g4u2,        0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Games 4 U 2 (94 5.6-0)", GAME_FLAGS )

GAME( 2002?, g4u3,        0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Games 4 U 3 (94 5.6-4)", GAME_FLAGS )
GAME( 2002?, g4u3a,       g4u3,     gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Games 4 U 3 (94 5.6-5)", GAME_FLAGS )

GAME( 2002?, g4u4,        0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Games 4 U 4 (94 5.6-5)", GAME_FLAGS )

GAME( 2002?, g4u5,        0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Games 4 U 5 (94 5.6-5)", GAME_FLAGS )

GAME( 2002?, g4u6,        0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Games 4 U 6 (94 5.6-5)", GAME_FLAGS )

GAME( 2002?, g4u7,        0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Games 4 U 7 (94 5.6-5a)", GAME_FLAGS )


GAME( 2002?, gamt1,       0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 1 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt1a,      gamt1,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 1 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt1b,      gamt1,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 1 (set 3)", GAME_FLAGS )

GAME( 2002?, gamt1lotc,   gamt1,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 1 (bootleg, C-Loto)", GAME_FLAGS )
GAME( 2002?, gamt1ent,    gamt1,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 1 (bootleg, Ent)", GAME_FLAGS )


GAME( 2002?, gamt4,       0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt4a,      gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt4b,      gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 3)", GAME_FLAGS )
GAME( 2002?, gamt4c,      gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 4)", GAME_FLAGS )
GAME( 2002?, gamt4d,      gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 5)", GAME_FLAGS )
GAME( 2002?, gamt4e,      gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 6)", GAME_FLAGS )
GAME( 2002?, gamt4f,      gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 7)", GAME_FLAGS )
GAME( 2002?, gamt4g,      gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 8)", GAME_FLAGS )
GAME( 2002?, gamt4h,      gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 9)", GAME_FLAGS )
GAME( 2002?, gamt4i,      gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 10)", GAME_FLAGS )
GAME( 2002?, gamt4j,      gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 4 (set 11)", GAME_FLAGS )

GAME( 2002?, gamt4lotc,   gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 4 (bootleg, C-Loto)", GAME_FLAGS )
GAME( 2002?, gamt4lotca,  gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 4 (C-Loto, MK4)", GAME_FLAGS )
GAME( 2002?, gamt4lotm,   gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 4 (bootleg, Lotomatic)", GAME_FLAGS )
GAME( 2002?, gamt4hmult,  gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 4 (bootleg, Multiloto)", GAME_FLAGS )
GAME( 2002?, gamt4ent,    gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 4 (bootleg, Ent)", GAME_FLAGS )
GAME( 2002?, gamt4dbag,   gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 4 (bootleg, Bag, set 1)", GAME_FLAGS )
GAME( 2002?, gamt4fbag,   gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 4 (bootleg, Bag, set 2)", GAME_FLAGS )
GAME( 2002?, gamt4hbag,   gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 4 (bootleg, Bag, set 3)", GAME_FLAGS )
GAME( 2002?, gamt4ibag,   gamt4,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 4 (bootleg, Bag, set 4)", GAME_FLAGS )


GAME( 2002?, gamt5,       0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 5 (set 1)", GAME_FLAGS )

GAME( 2002?, gamt6,       0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 6 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt6a,      gamt6,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 6 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt6b,      gamt6,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 6 (set 3)", GAME_FLAGS )
GAME( 2002?, gamt6c,      gamt6,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 6 (set 4)", GAME_FLAGS )
GAME( 2002?, gamt6d,      gamt6,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 6 (set 5)", GAME_FLAGS )
GAME( 2002?, gamt6e,      gamt6,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 6 (set 6)", GAME_FLAGS )
GAME( 2002?, gamt6f,      gamt6,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 6 (set 7)", GAME_FLAGS )

GAME( 2002?, gamt6lotc,   gamt6,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 6 (bootleg, C-Loto)", GAME_FLAGS )
GAME( 2002?, gamt6ent,    gamt6,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 6 (bootleg, Ent)", GAME_FLAGS )


GAME( 2002?, gamt7,       0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 7 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt7a,      gamt7,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 7 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt7b,      gamt7,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 7 (set 3)", GAME_FLAGS )
GAME( 2002?, gamt7c,      gamt7,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 7 (set 4)", GAME_FLAGS )
GAME( 2002?, gamt7d,      gamt7,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 7 (set 5)", GAME_FLAGS )
GAME( 2002?, gamt7e,      gamt7,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 7 (set 6)", GAME_FLAGS )
GAME( 2002?, gamt7f,      gamt7,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 7 (set 7)", GAME_FLAGS )
GAME( 2002?, gamt7g,      gamt7,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 7 (set 8)", GAME_FLAGS )
GAME( 2002?, gamt7h,      gamt7,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 7 (set 9)", GAME_FLAGS )

GAME( 2002?, gamt8,       0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 8 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt8a,      gamt8,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 8 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt8b,      gamt8,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 8 (set 3)", GAME_FLAGS )
GAME( 2002?, gamt8c,      gamt8,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 8 (set 4)", GAME_FLAGS )
GAME( 2002?, gamt8d,      gamt8,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 8 (set 5)", GAME_FLAGS )

GAME( 2002?, gamt8lotc,   gamt8,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 8 (bootleg, C-Loto)", GAME_FLAGS )


GAME( 2002?, gamt9,       0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 9 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt9a,      gamt9,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 9 (set 2)", GAME_FLAGS )

GAME( 2002?, gamt9lotc,   gamt9,    gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 9 (bootleg, C-Loto)", GAME_FLAGS )


GAME( 2002?, gamt10,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt10a,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt10b,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 3)", GAME_FLAGS )
GAME( 2002?, gamt10c,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 4)", GAME_FLAGS )
GAME( 2002?, gamt10d,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 5)", GAME_FLAGS )
GAME( 2002?, gamt10e,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 6)", GAME_FLAGS )
GAME( 2002?, gamt10f,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 7)", GAME_FLAGS )
GAME( 2002?, gamt10g,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 8)", GAME_FLAGS )
GAME( 2002?, gamt10h,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 9)", GAME_FLAGS )
GAME( 2002?, gamt10i,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 10)", GAME_FLAGS )
GAME( 2002?, gamt10j,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 11)", GAME_FLAGS )
GAME( 2002?, gamt10k,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 12)", GAME_FLAGS )
GAME( 2002?, gamt10l,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 13)", GAME_FLAGS )
GAME( 2002?, gamt10m,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 14)", GAME_FLAGS )
GAME( 2002?, gamt10n,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 15)", GAME_FLAGS )
GAME( 2002?, gamt10o,     gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 10 (set 16)", GAME_FLAGS )

GAME( 2002?, gamt10lotc,  gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 10 (bootleg, C-Loto)", GAME_FLAGS )
GAME( 2002?, gamt10lotm,  gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 10 (bootleg, Lotomatic)", GAME_FLAGS )
GAME( 2002?, gamt10gmult, gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 10 (bootleg, Multiloto)", GAME_FLAGS )
GAME( 2002?, gamt10ent,   gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 10 (bootleg, Ent)", GAME_FLAGS )
GAME( 2002?, gamt10bag,   gamt10,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 10 (bootleg, Bag)", GAME_FLAGS )

GAME( 2002?, gamt11,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 11 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt11a,     gamt11,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 11 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt11b,     gamt11,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 11 (set 3)", GAME_FLAGS )
GAME( 2002?, gamt11c,     gamt11,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 11 (set 4)", GAME_FLAGS )

GAME( 2002?, gamt11bmult, gamt11,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 11 (bootleg, Multiloto)", GAME_FLAGS )


GAME( 2002?, gamt12,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 12 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt12a,     gamt12,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 12 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt12b,     gamt12,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 12 (set 3)", GAME_FLAGS )

GAME( 2002?, gamt16 ,     0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt16a,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt16b,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 3)", GAME_FLAGS )
GAME( 2002?, gamt16c,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 4)", GAME_FLAGS )
GAME( 2002?, gamt16d,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 5)", GAME_FLAGS )
GAME( 2002?, gamt16e,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 6)", GAME_FLAGS )
GAME( 2002?, gamt16f,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 7)", GAME_FLAGS )
GAME( 2002?, gamt16g,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 8)", GAME_FLAGS )
GAME( 2002?, gamt16h,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 9)", GAME_FLAGS )
GAME( 2002?, gamt16i,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 10)", GAME_FLAGS )
GAME( 2002?, gamt16j,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 11)", GAME_FLAGS )
GAME( 2002?, gamt16k,     gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 16 (set 12)", GAME_FLAGS )

GAME( 2002?, gamt16lotc,  gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 16 (bootleg, C-Loto)", GAME_FLAGS )
GAME( 2002?, gamt16fmult, gamt16,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 16 (bootleg, Multiloto)", GAME_FLAGS )


GAME( 2002?, gamt17,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 17 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt17a,     gamt17,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 17 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt17b,     gamt17,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 17 (set 3)", GAME_FLAGS )

GAME( 2002?, gamt18,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 18 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt18a,     gamt18,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 18 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt18b,     gamt18,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 18 (set 3)", GAME_FLAGS )
GAME( 2002?, gamt18c,     gamt18,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 18 (set 4)", GAME_FLAGS )
GAME( 2002?, gamt18d,     gamt18,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 18 (set 5)", GAME_FLAGS )

GAME( 2002?, gamt18lotc,  gamt18,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 18 (bootleg, C-Loto)", GAME_FLAGS )
GAME( 2002?, gamt18bmult, gamt18,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 18 (bootleg, Multiloto)", GAME_FLAGS )
GAME( 2002?, gamt18ent,   gamt18,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 18 (bootleg, Ent)", GAME_FLAGS )


GAME( 2002?, gamt19,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 19 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt19a,     gamt19,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 19 (set 2)", GAME_FLAGS )

GAME( 2002?, gamt19lotc,  gamt19,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 19 (bootleg, C-Loto)", GAME_FLAGS )
GAME( 2002?, gamt19mult,  gamt19,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 19 (bootleg, Multiloto)", GAME_FLAGS )
GAME( 2002?, gamt19ent,   gamt19,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 19 (bootleg, Ent)", GAME_FLAGS )


GAME( 2002?, gamt20,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 20 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt20a,     gamt20,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 20 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt20b,     gamt20,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 20 (set 3)", GAME_FLAGS )

GAME( 2002?, gamt20lotc,  gamt20,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 20 (bootleg, C-Loto)", GAME_FLAGS )
GAME( 2002?, gamt20lotm,  gamt20,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 20 (bootleg, Lotomatic)", GAME_FLAGS )
GAME( 2002?, gamt20ent,   gamt20,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 20 (bootleg, Ent)", GAME_FLAGS )


GAME( 2002?, gamt21,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 21 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt21a,     gamt21,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 21 (set 2)", GAME_FLAGS )

GAME( 2002?, gamt21amult, gamt21,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 21 (bootleg, Multiloto)", GAME_FLAGS )

GAME( 2002?, gamt22,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 22 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt22a,     gamt22,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 22 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt22b,     gamt22,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 22 (set 3)", GAME_FLAGS )

GAME( 2002?, gamt22amult, gamt22,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 22 (bootleg, Multiloto)", GAME_FLAGS )


GAME( 2002?, gamt23,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 23 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt23a,     gamt23,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 23 (set 2)", GAME_FLAGS )
GAME( 2002?, gamt23b,     gamt23,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 23 (set 3)", GAME_FLAGS )

GAME( 2002?, gamt29,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 29 (set 1)", GAME_FLAGS )
GAME( 2002?, gamt29a,     gamt29,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 29 (set 2)", GAME_FLAGS )

GAME( 2002?, gamt30,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 30 (set 1)", GAME_FLAGS )

GAME( 2002?, gamt31,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Gaminator 31 (set 1)", GAME_FLAGS )

GAME( 2002?, gamt31mult,  gamt31,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "bootleg","Gaminator 31 (bootleg, Multiloto)", GAME_FLAGS )

GAME( 2002?, megakat,     0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Mega Katok 2", GAME_FLAGS )
GAME( 2002?, hspot2,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Hot Spot 2", GAME_FLAGS )
GAME( 2002?, hspot3,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Hot Spot 3", GAME_FLAGS )


GAME( 2002?, ancienta,    0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Ancient Atlantis (set 1)", GAME_FLAGS )
GAME( 2002?, ancientaa,   ancienta, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Ancient Atlantis (set 2)", GAME_FLAGS )
GAME( 2002?, ancientab,   ancienta, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Ancient Atlantis (set 3)", GAME_FLAGS )
GAME( 2002?, ancientac,   ancienta, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Ancient Atlantis (set 4)", GAME_FLAGS )
GAME( 2002?, ancientad,   ancienta, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Ancient Atlantis (set 5)", GAME_FLAGS )
GAME( 2002?, bananas,     0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Bananas Go Bahamas (set 1)", GAME_FLAGS )
GAME( 2002?, bananasa,    bananas,  gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Bananas Go Bahamas (set 2)", GAME_FLAGS )
GAME( 2002?, beebop,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Bee Bop (set 1)", GAME_FLAGS )
GAME( 2002?, beebopa,     beebop,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Bee Bop (set 2)", GAME_FLAGS )
GAME( 2002?, beebopb,     beebop,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Bee Bop (set 3)", GAME_FLAGS )
GAME( 2002?, beebopc,     beebop,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Bee Bop (set 4)", GAME_FLAGS )
GAME( 2002?, beebopd,     beebop,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Bee Bop (set 5)", GAME_FLAGS )
GAME( 2002?, beebope,     beebop,   gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Bee Bop (set 6)", GAME_FLAGS )
GAME( 2002?, beetlem,     0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Beetlemania (set 1)", GAME_FLAGS )
GAME( 2002?, beetlema,    beetlem,  gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Beetlemania (set 2)", GAME_FLAGS )
GAME( 2002?, beetlemb,    beetlem,  gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Beetlemania (set 3)", GAME_FLAGS )
GAME( 2002?, beetlemc,    beetlem,  gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Beetlemania (set 4)", GAME_FLAGS )
GAME( 2002?, beetlemd,    beetlem,  gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Beetlemania (set 5)", GAME_FLAGS )
GAME( 2002?, bungeem,     0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Bungee Monkey (set 1)", GAME_FLAGS )
GAME( 2002?, bungeema,    bungeem,  gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Bungee Monkey (set 2)", GAME_FLAGS )
GAME( 2002?, bookra,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Book Of Ra (set 1)", GAME_FLAGS )
GAME( 2002?, bsplash,     0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Banana Splash (set 1)", GAME_FLAGS )
GAME( 2002?, chillicc,    0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Chilli Con Cash (set 1)", GAME_FLAGS )
GAME( 2002?, columbus,    0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Columbus (set 1)", GAME_FLAGS )
GAME( 2002?, columbusa,   columbus, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Columbus (set 2)", GAME_FLAGS )
GAME( 2002?, columbusb,   columbus, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Columbus (set 3)", GAME_FLAGS )
GAME( 2002?, columbusc,   columbus, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Columbus (set 4)", GAME_FLAGS )
GAME( 2002?, columbusd,   columbus, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Columbus (set 5)", GAME_FLAGS )
GAME( 2002?, columbuse,   columbus, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Columbus (set 6)", GAME_FLAGS )
GAME( 2002?, columbusf,   columbus, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Columbus (set 7)", GAME_FLAGS )
GAME( 2002?, ditrio,      0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Diamond Trio (set 1)", GAME_FLAGS )
GAME( 2002?, dolphinp,    0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Dolphin's Pearl (set 1)", GAME_FLAGS )
GAME( 2002?, eurogame,    0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","The Euro Game (set 1)", GAME_FLAGS )
GAME( 2002?, eurogamea,   eurogame, gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","The Euro Game (set 2)", GAME_FLAGS )
GAME( 2002?, firstcl,     0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","First Class Traveller (set 1)", GAME_FLAGS )
GAME( 2002?, llcharm,     0,        gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Lucky Lady's Charm (set 1)", GAME_FLAGS )
GAME( 2002?, llcharma,    llcharm,  gaminator, gaminator, gaminator_state, init_gaminator, ROT0, "Novotech","Lucky Lady's Charm (set 2)", GAME_FLAGS )
