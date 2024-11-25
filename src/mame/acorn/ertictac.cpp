// license:BSD-3-Clause
// copyright-holders:Angelo Salese, R. Belmont, Tomasz Slanina, Steve Ellenoff, Nicola Salmoria
/*******************************************************************************************

    Erotictac/Tactic (c) 1990 Sisteme
    Poizone (c) 1991 Eterna

    Actually an Acorn Archimedes-based Arcade system

    original driver by Tomasz Slanina, Steve Ellenoff, Nicola Salmoria
    rewrite to use AA functions by R. Belmont & Angelo Salese
    special thanks to Sarah Walker (author of the Acorn Archimedes Arculator emulator)

    TODO (specific issues only):
    - Sound is currently ugly in both games, recognizable but still nowhere near perfection
    - poizone: video timings are off, causing various glitches.
    - Does this Arcade conversion have I2C device? It seems unused afaik.
    - Need PCB for identify the exact model of AA, available RAM, what kind of i/o "podule"
      it has etc.

PCB has a single OSC at 24MHz

*******************************************************************************************/

#include "emu.h"
#include "cpu/arm/arm.h"
#include "machine/acorn_ioc.h"
#include "machine/acorn_memc.h"
#include "machine/acorn_vidc.h"
#include "machine/pcf8583.h"
#include "screen.h"


namespace {

class ertictac_state : public driver_device
{
public:
	ertictac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ioc(*this, "ioc")
		, m_memc(*this, "memc")
		, m_vidc10(*this, "vidc")
		{ }

	void ertictac(machine_config &config);

	void init_ertictac();

private:
	uint32_t ertictac_podule_r(offs_t offset);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	INTERRUPT_GEN_MEMBER(ertictac_podule_irq);
	void ertictac_arm_map(address_map &map) ATTR_COLD;
	void ertictac_map(address_map &map) ATTR_COLD;

	required_device<arm_cpu_device> m_maincpu;
	required_device<acorn_ioc_device> m_ioc;
	required_device<acorn_memc_device> m_memc;
	required_device<acorn_vidc10_device> m_vidc10;
};


uint32_t ertictac_state::ertictac_podule_r(offs_t offset)
{
	m_ioc->il5_w(CLEAR_LINE);

	switch(offset & 0x3fff)
	{
		case 0x04/4: return ioport("DSW1")->read() & 0xff;
		case 0x08/4: return ioport("DSW2")->read() & 0xff;
		case 0x10/4: return ioport("SYSTEM")->read() & 0xff;
		case 0x14/4: return ioport("P2")->read() & 0xff;
		case 0x18/4: return ioport("P1")->read() & 0xff;
	}

	return 0;
}


void ertictac_state::ertictac_arm_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(m_memc, FUNC(acorn_memc_device::logical_r), FUNC(acorn_memc_device::logical_w));
	map(0x02000000, 0x03ffffff).rw(m_memc, FUNC(acorn_memc_device::high_mem_r), FUNC(acorn_memc_device::high_mem_w));
}

void ertictac_state::ertictac_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(m_memc, FUNC(acorn_memc_device::logical_r), FUNC(acorn_memc_device::logical_w));
	map(0x02000000, 0x02ffffff).ram().share("physicalram"); /* physical RAM - 16 MB for now, should be 512k for the A310 */

	map(0x03000000, 0x033fffff).m(m_ioc, FUNC(acorn_ioc_device::map));
	map(0x03400000, 0x035fffff).w(m_vidc10, FUNC(acorn_vidc10_device::write));
	map(0x03600000, 0x037fffff).w(m_memc, FUNC(acorn_memc_device::registers_w));
	map(0x03800000, 0x03ffffff).rom().region("maincpu", 0).w(m_memc, FUNC(acorn_memc_device::page_w));
}

static INPUT_PORTS_START( ertictac )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00c4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00c4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x02, 0x02, "Demo Sound" )    PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Test Mode" )     PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Music" )     PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, "Game Timing" )   PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x30, "Normal Game" )
	PORT_DIPSETTING(    0x20, "3:00" )
	PORT_DIPSETTING(    0x10, "2:30" )
	PORT_DIPSETTING(    0x00, "2:00" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x05, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_DIPNAME( 0x0a, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_4C ) )

	PORT_DIPNAME( 0x10, 0x00, "Sexy Views" )    PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( poizone )
	PORT_INCLUDE( ertictac )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	// TODO: default settings
	PORT_MODIFY("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW1:3" )
	PORT_DIPNAME( 0x30, 0x40, "Coinage 1" ) PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xC0, 0x00, "Coinage 2" ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xC0, DEF_STR( 1C_4C ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Setting 1" ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x01, "Manual" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x1A, 0x1A, "Setting 2" )  PORT_DIPLOCATION("DSW2:3,4,5")
	PORT_DIPSETTING(    0x00, "Extremely Easy - 2:00")  PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, "Very Easy - 1:30")   PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x08, "Easy - 2:00")    PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x0A, "Normal 1 - 1:30")    PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, "Normal 2 - 1:45")    PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x12, "Difficult - 2:00")   PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x18, "Very Difficult - 2:00")  PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x1A, "Extremely Difficult - 1:30") PORT_CONDITION("DSW2", 0x01, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "Clear 20% - 1:00")   PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x02, "Clear 30% - 1:00")   PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x08, "Clear 40% - 1:00")   PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x0A, "Clear 50% - 1:00")   PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x10, "Clear 60% - 1:00")   PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x12, "Clear 70% - 1:00")   PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x18, "Clear 80% - 1:00")   PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPSETTING(    0x1A, "Clear 90% - 1:00")   PORT_CONDITION("DSW2", 0x01, EQUALS, 0x01)
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW2:6" )
INPUT_PORTS_END

void ertictac_state::init_ertictac()
{
}

void ertictac_state::machine_start()
{
}

void ertictac_state::machine_reset()
{
}

INTERRUPT_GEN_MEMBER(ertictac_state::ertictac_podule_irq)
{
	m_ioc->il5_w(ASSERT_LINE);
}

void ertictac_state::ertictac(machine_config &config)
{
	ARM(config, m_maincpu, 24_MHz_XTAL/3); /* guess, 12MHz 8MHz or 6MHz, what's the correct divider 2, 3 or 4? */
	m_maincpu->set_addrmap(AS_PROGRAM, &ertictac_state::ertictac_arm_map);
	m_maincpu->set_periodic_int(FUNC(ertictac_state::ertictac_podule_irq), attotime::from_hz(60)); // FIXME: timing of this

	PCF8583(config, "i2cmem", 32.768_kHz_XTAL); // TODO: Are we sure that this HW have I2C device?

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.screen_vblank().set(m_ioc, FUNC(acorn_ioc_device::ir_w));
	screen.screen_vblank().append(m_memc, FUNC(acorn_memc_device::vidrq_w));

	ACORN_MEMC(config, m_memc, 24_MHz_XTAL/3, m_vidc10);
	m_memc->set_addrmap(0, &ertictac_state::ertictac_map);
	m_memc->sirq_w().set(m_ioc, FUNC(acorn_ioc_device::il1_w));

	ACORN_IOC(config, m_ioc, 24_MHz_XTAL/3);
	m_ioc->fiq_w().set_inputline(m_maincpu, ARM_FIRQ_LINE);
	m_ioc->irq_w().set_inputline(m_maincpu, ARM_IRQ_LINE);
	m_ioc->peripheral_r<4>().set(FUNC(ertictac_state::ertictac_podule_r));
	m_ioc->gpio_r<0>().set("i2cmem", FUNC(pcf8583_device::sda_r));
	m_ioc->gpio_w<0>().set("i2cmem", FUNC(pcf8583_device::sda_w));
	m_ioc->gpio_w<1>().set("i2cmem", FUNC(pcf8583_device::scl_w));

	ACORN_VIDC1A(config, m_vidc10, 24_MHz_XTAL);
	m_vidc10->set_screen("screen");
	m_vidc10->sound_drq().set(m_memc, FUNC(acorn_memc_device::sndrq_w));
}

ROM_START( ertictac )
	ROM_REGION(0x800000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "01", 0x00000, 0x10000, CRC(8dce677c) SHA1(9f12b1febe796038caa1ecad1d17864dc546cfd8) ) /* Unknown version / revision */
	ROM_LOAD32_BYTE( "02", 0x00001, 0x10000, CRC(7c5c916c) SHA1(d4ed5fc3a7b27253551e7d9176ed9e6513092e60) ) /* Unknown version / revision */
	ROM_LOAD32_BYTE( "03", 0x00002, 0x10000, CRC(edca5ac6) SHA1(f6c4b8030f3c1c93922c5f7232f2159e0471b93a) ) /* Unknown version / revision */
	ROM_LOAD32_BYTE( "04", 0x00003, 0x10000, CRC(959be81c) SHA1(3e8eaacf8809863fd712ad605d23fdda4e904aee) ) /* Unknown version / revision */
	ROM_LOAD32_BYTE( "eroti_ver01_-05-", 0x40000, 0x10000, CRC(d08a6c89) SHA1(17b0f5fb2094126146b09d69c91bf413737b0c9e) )
	ROM_LOAD32_BYTE( "eroti_ver01_-06-", 0x40001, 0x10000, CRC(d727bcd8) SHA1(4627f8d4d6e6f323445b3ffcfc0d9c699a9a4f89) )
	ROM_LOAD32_BYTE( "eroti_ver01_-07-", 0x40002, 0x10000, CRC(23b75de2) SHA1(e18f5339ca2dd362298784ff3e5479d780d823f8) )
	ROM_LOAD32_BYTE( "eroti_ver01_-08-", 0x40003, 0x10000, CRC(9448ccdf) SHA1(75fe3c31625f8ba1eedd806b52993e92b1f585b6) )
	ROM_LOAD32_BYTE( "eroti_ver01_-09-", 0x80000, 0x10000, CRC(2bfb312e) SHA1(af7a4a1926c9c3d0b3ad41a4729a17383581c070) )
	ROM_LOAD32_BYTE( "eroti_ver01_-10-", 0x80001, 0x10000, CRC(e7a05477) SHA1(0ec6f94a35b87e1e4529dea194fce1fde9a5b0ad) )
	ROM_LOAD32_BYTE( "eroti_ver01_-11-", 0x80002, 0x10000, CRC(3722591c) SHA1(e0c4075bc4b1c90a6decba3005a1f8298bd61bd1) )
	ROM_LOAD32_BYTE( "eroti_ver01_-12-", 0x80003, 0x10000, CRC(fe022b7e) SHA1(056f7283bc54eff555fd1db91410c020fd905063) )
	ROM_LOAD32_BYTE( "eroti_ver01_-13-", 0xc0000, 0x10000, CRC(83550842) SHA1(0fee78dbf13ba970e0544c48f8939550f9347822) )
	ROM_LOAD32_BYTE( "eroti_ver01_-14-", 0xc0001, 0x10000, CRC(3029567c) SHA1(6d49bea3a3f6f11f4182a602d37b53f1f896c154) )
	ROM_LOAD32_BYTE( "eroti_ver01_-15-", 0xc0002, 0x10000, CRC(500997ab) SHA1(028c7b3ca03141e5b596ab1e2ab98d0ccd9bf93a) )
	ROM_LOAD32_BYTE( "eroti_ver01_-16-", 0xc0003, 0x10000, CRC(70a8d136) SHA1(50b11f5701ed5b79a5d59c9a3c7d5b7528e66a4d) )
ROM_END


ROM_START( ertictaca ) /* PCB had sticker printed "092121 EROTICTAC" */
	ROM_REGION(0x800000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "eroti_ver01_-01-", 0x00000, 0x10000, CRC(7e5b5b0b) SHA1(a9a814ca99a4cb5ee1372c0258a5b93ec90fde5c) )
	ROM_LOAD32_BYTE( "eroti_ver01_-02-", 0x00001, 0x10000, CRC(e4116f89) SHA1(79bab10f7c49e47e6692b4211c0445886b005275) )
	ROM_LOAD32_BYTE( "eroti_ver01_-03-", 0x00002, 0x10000, CRC(2d14a239) SHA1(980926ea188f96d0453571fe1afbdb3492d4cf7a) )
	ROM_LOAD32_BYTE( "eroti_ver01_-04-", 0x00003, 0x10000, CRC(e862b0d2) SHA1(fd66e55ea8fe6d65db6e61b90af690a81efee6dd) )
	ROM_LOAD32_BYTE( "eroti_ver01_-05-", 0x40000, 0x10000, CRC(d08a6c89) SHA1(17b0f5fb2094126146b09d69c91bf413737b0c9e) )
	ROM_LOAD32_BYTE( "eroti_ver01_-06-", 0x40001, 0x10000, CRC(d727bcd8) SHA1(4627f8d4d6e6f323445b3ffcfc0d9c699a9a4f89) )
	ROM_LOAD32_BYTE( "eroti_ver01_-07-", 0x40002, 0x10000, CRC(23b75de2) SHA1(e18f5339ca2dd362298784ff3e5479d780d823f8) )
	ROM_LOAD32_BYTE( "eroti_ver01_-08-", 0x40003, 0x10000, CRC(9448ccdf) SHA1(75fe3c31625f8ba1eedd806b52993e92b1f585b6) )
	ROM_LOAD32_BYTE( "eroti_ver01_-09-", 0x80000, 0x10000, CRC(2bfb312e) SHA1(af7a4a1926c9c3d0b3ad41a4729a17383581c070) )
	ROM_LOAD32_BYTE( "eroti_ver01_-10-", 0x80001, 0x10000, CRC(e7a05477) SHA1(0ec6f94a35b87e1e4529dea194fce1fde9a5b0ad) )
	ROM_LOAD32_BYTE( "eroti_ver01_-11-", 0x80002, 0x10000, CRC(3722591c) SHA1(e0c4075bc4b1c90a6decba3005a1f8298bd61bd1) )
	ROM_LOAD32_BYTE( "eroti_ver01_-12-", 0x80003, 0x10000, CRC(fe022b7e) SHA1(056f7283bc54eff555fd1db91410c020fd905063) )
	ROM_LOAD32_BYTE( "eroti_ver01_-13-", 0xc0000, 0x10000, CRC(83550842) SHA1(0fee78dbf13ba970e0544c48f8939550f9347822) )
	ROM_LOAD32_BYTE( "eroti_ver01_-14-", 0xc0001, 0x10000, CRC(3029567c) SHA1(6d49bea3a3f6f11f4182a602d37b53f1f896c154) )
	ROM_LOAD32_BYTE( "eroti_ver01_-15-", 0xc0002, 0x10000, CRC(500997ab) SHA1(028c7b3ca03141e5b596ab1e2ab98d0ccd9bf93a) )
	ROM_LOAD32_BYTE( "eroti_ver01_-16-", 0xc0003, 0x10000, CRC(70a8d136) SHA1(50b11f5701ed5b79a5d59c9a3c7d5b7528e66a4d) )
ROM_END

ROM_START( ertictacb )
	ROM_REGION(0x800000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "1.bin", 0x00000, 0x10000, CRC(b8eee693) SHA1(12a7c50040ccbc14bac0beb2938d79322aa01a28) )
	ROM_LOAD32_BYTE( "2.bin", 0x00001, 0x10000, CRC(e22618ef) SHA1(cf6a6ba37400a2b3f4235a02d70cfb6258d52a16) )
	ROM_LOAD32_BYTE( "3.bin", 0x00002, 0x10000, CRC(15683de7) SHA1(b9d478437fe927e05632c6e03b65e5e953fce3a3) )
	ROM_LOAD32_BYTE( "4.bin", 0x00003, 0x10000, CRC(7949e19c) SHA1(af5bf745d4266368b129ca25d623b724f0a23603) )
	ROM_LOAD32_BYTE( "eroti_ver01_-05-", 0x40000, 0x10000, CRC(d08a6c89) SHA1(17b0f5fb2094126146b09d69c91bf413737b0c9e) )
	ROM_LOAD32_BYTE( "eroti_ver01_-06-", 0x40001, 0x10000, CRC(d727bcd8) SHA1(4627f8d4d6e6f323445b3ffcfc0d9c699a9a4f89) )
	ROM_LOAD32_BYTE( "eroti_ver01_-07-", 0x40002, 0x10000, CRC(23b75de2) SHA1(e18f5339ca2dd362298784ff3e5479d780d823f8) )
	ROM_LOAD32_BYTE( "eroti_ver01_-08-", 0x40003, 0x10000, CRC(9448ccdf) SHA1(75fe3c31625f8ba1eedd806b52993e92b1f585b6) )
	ROM_LOAD32_BYTE( "eroti_ver01_-09-", 0x80000, 0x10000, CRC(2bfb312e) SHA1(af7a4a1926c9c3d0b3ad41a4729a17383581c070) )
	ROM_LOAD32_BYTE( "eroti_ver01_-10-", 0x80001, 0x10000, CRC(e7a05477) SHA1(0ec6f94a35b87e1e4529dea194fce1fde9a5b0ad) )
	ROM_LOAD32_BYTE( "eroti_ver01_-11-", 0x80002, 0x10000, CRC(3722591c) SHA1(e0c4075bc4b1c90a6decba3005a1f8298bd61bd1) )
	ROM_LOAD32_BYTE( "eroti_ver01_-12-", 0x80003, 0x10000, CRC(fe022b7e) SHA1(056f7283bc54eff555fd1db91410c020fd905063) )
	ROM_LOAD32_BYTE( "eroti_ver01_-13-", 0xc0000, 0x10000, CRC(83550842) SHA1(0fee78dbf13ba970e0544c48f8939550f9347822) )
	ROM_LOAD32_BYTE( "eroti_ver01_-14-", 0xc0001, 0x10000, CRC(3029567c) SHA1(6d49bea3a3f6f11f4182a602d37b53f1f896c154) )
	ROM_LOAD32_BYTE( "eroti_ver01_-15-", 0xc0002, 0x10000, CRC(500997ab) SHA1(028c7b3ca03141e5b596ab1e2ab98d0ccd9bf93a) )
	ROM_LOAD32_BYTE( "eroti_ver01_-16-", 0xc0003, 0x10000, CRC(70a8d136) SHA1(50b11f5701ed5b79a5d59c9a3c7d5b7528e66a4d) )
ROM_END


ROM_START( poizone )
	ROM_REGION(0x800000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "p_son01.bin", 0x00000, 0x10000, CRC(28793c9f) SHA1(2d9f7d667203e745b47cd2cc97501ae961ae1a66) )
	ROM_LOAD32_BYTE( "p_son02.bin", 0x00001, 0x10000, CRC(2d4b6f4b) SHA1(8df2680d6e5dc41787b3a72e594f01f5e732d0ec) )
	ROM_LOAD32_BYTE( "p_son03.bin", 0x00002, 0x10000, CRC(0834d46e) SHA1(bf1cc9b47759ef39ed8fd8f334ed8f2902be3bf8) )
	ROM_LOAD32_BYTE( "p_son04.bin", 0x00003, 0x10000, CRC(9e9b1f6e) SHA1(d95067f3ecca3c079a67bd0b80e3b45c5b42151e) )
	ROM_LOAD32_BYTE( "p_son05.bin", 0x40000, 0x10000, CRC(be62ad42) SHA1(5eb51ad277ec7b7f1b5995bcdea35114f805baae) )
	ROM_LOAD32_BYTE( "p_son06.bin", 0x40001, 0x10000, CRC(c2f9141c) SHA1(e910fefcd6f0b99ab299b3a5f099b9ef84e1cc23) )
	ROM_LOAD32_BYTE( "p_son07.bin", 0x40002, 0x10000, CRC(8929c748) SHA1(35c108170590fbe97fdd4a1db7d660b4ee0adac8) )
	ROM_LOAD32_BYTE( "p_son08.bin", 0x40003, 0x10000, CRC(0ef5b14f) SHA1(425f130b2a94a4152fab763e0734e71f2913b25f) )
	ROM_LOAD32_BYTE( "p_son09.bin", 0x80000, 0x10000, CRC(e8cd75a6) SHA1(386a4ff576574e49711e72640dd3f33c8b7e04b3) )
	ROM_LOAD32_BYTE( "p_son10.bin", 0x80001, 0x10000, CRC(1dc01da7) SHA1(d37456e3407cab5eff5bbd9735c3a54e73b27545) )
	ROM_LOAD32_BYTE( "p_son11.bin", 0x80002, 0x10000, CRC(85e973ad) SHA1(850cd0dbda42eab78625038c6ea1f5b31674018a) )
	ROM_LOAD32_BYTE( "p_son12.bin", 0x80003, 0x10000, CRC(b89376d1) SHA1(cff29c2a8db88d4d104bae19a90de034158fe9e7) )

	ROM_LOAD32_BYTE( "p_son21.bin", 0x140000, 0x10000, CRC(a0c06c1e) SHA1(8d065117788e96ecd147d3d7ffdd273d4b69bb7a) )
	ROM_LOAD32_BYTE( "p_son22.bin", 0x140001, 0x10000, CRC(16f0bb52) SHA1(893ab1e72b84de7a38f88f9d713769968ebd4553) )
	ROM_LOAD32_BYTE( "p_son23.bin", 0x140002, 0x10000, CRC(e9c118b2) SHA1(110d9a204e701b9b54d89f027f8892c3f3a819c7) )
	ROM_LOAD32_BYTE( "p_son24.bin", 0x140003, 0x10000, CRC(a09d7f55) SHA1(e0d562c655c16034b40db93de801b98b7948beb2) )
ROM_END

} // anonymous namespace


GAME( 1990, ertictac,         0, ertictac, ertictac, ertictac_state, init_ertictac, ROT0, "Sisteme", "Erotictac/Tactic",          MACHINE_IMPERFECT_SOUND)
GAME( 1990, ertictaca, ertictac, ertictac, ertictac, ertictac_state, init_ertictac, ROT0, "Sisteme", "Erotictac/Tactic (ver 01)", MACHINE_IMPERFECT_SOUND)
GAME( 1990, ertictacb, ertictac, ertictac, ertictac, ertictac_state, init_ertictac, ROT0, "Sisteme", "Erotictac/Tactic (set 2)",  MACHINE_IMPERFECT_SOUND)
GAME( 1991, poizone,          0, ertictac, poizone,  ertictac_state, init_ertictac, ROT0, "Eterna",  "Poizone",                   MACHINE_IMPERFECT_SOUND|MACHINE_IMPERFECT_GRAPHICS)
