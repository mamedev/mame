// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
 sp.ACE system by ACE
 based roughly on the Mach2000 hardware used by Castle

 Skeleton driver!

 Some games have OKI sample ROMs, others just use the AYs,
 this depends on the additional plug-in boards.
 We need to verify which is which as some samples could be missing

 Some manufacturers, such as PCP, also used this hardware with
 different reel controllers etc.

 based on internal accesses it seems to use a 6303Y (like Mach2000)
 which does NOT have the same internal map as a 6303R

 some ROMsets here contain a single larger ROM instead of 2 smaller
 ones, these need verifying to make sure they contain unique data
 and removing if they do not.

*/


#include "emu.h"
#include "cpu/m6800/m6801.h"
#include "machine/6821pia.h"
#include "machine/timer.h"

#include "ace_sp_reelctrl.h"

#include "ace_sp_dmd.lh"


namespace {

class ace_sp_state : public driver_device
{
public:
	ace_sp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_dmdram(*this, "dmdram")
		, m_sevensegram(*this, "sevensegram")
		, m_dmd(*this, "dotmatrix%u", 0U)
		, m_maincpu(*this, "maincpu")
		, m_reelctrl(*this, "reelctrl")
	{ }

	void ace_sp(machine_config &config);
	void ace_sp_pcp(machine_config &config);

	void init_ace_sp();
	void init_ace_cr();

private:
	void machine_start() override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(gen_fixfreq);

	void ace_sp_map(address_map &map) ATTR_COLD;

	void dmd_w(offs_t offset, uint8_t data);
	void sevenseg_w(offs_t offset, uint8_t data);

	uint8_t serial_r(offs_t offset);
	void serial_w(offs_t offset, uint8_t data);

	required_shared_ptr<uint8_t> m_dmdram;
	required_shared_ptr<uint8_t> m_sevensegram;

	output_finder<1536> m_dmd;

	// devices
	required_device<hd6303y_cpu_device> m_maincpu;
	required_device<ace_sp_reelctrl_base_device> m_reelctrl;
};



void ace_sp_state::machine_start()
{
	m_dmd.resolve();
}

void ace_sp_state::dmd_w(offs_t offset, uint8_t data)
{
	m_dmdram[offset] = data;

	for (int i = 0; i < 8; i++)
		m_dmd[(offset * 8) + i] = (data >> i) & 1;
}

void ace_sp_state::sevenseg_w(offs_t offset, uint8_t data)
{
	m_sevensegram[offset] = data;
}

uint8_t ace_sp_state::serial_r(offs_t offset)
{
	logerror("%s: serial_r\n", machine().describe_context());
	return machine().rand();
}

void ace_sp_state::serial_w(offs_t offset, uint8_t data)
{
	logerror("%s: serial_w %02x\n", machine().describe_context(), data);
}

void ace_sp_state::ace_sp_map(address_map &map)
{
	/**** 6303Y internal area ****/
	//----- 0x0000 - 0x0027 is internal registers -----

	//----- 0x0028 - 0x003f is external access -----
	// 0x30 - to/from reel MCU
	// 0x31 - lamp high
	// 0x32 - lamp low
	// 0x33 - lamp stb
	// 0x34 - shift stb
	// 0x35 - shift clk
	map(0x36, 0x36).ram().rw(FUNC(ace_sp_state::serial_r), FUNC(ace_sp_state::serial_w));   // 0x36 - sio

	// 0x37 - watchdog?
	map(0x0038, 0x003b).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	/* 0x3c */
	/* 0x3d */
	/* 0x3e */
	/* 0x3f */
	//----- 0x0040 - 0x013f is internal RAM (256 bytes) -----

	/**** regular map ****/
	map(0x0140, 0x1eff).ram();
	map(0x1f00, 0x1fbf).ram().w(FUNC(ace_sp_state::dmd_w)).share("dmdram");  // DMD controller shares the RAM? or does this get uploaded somewhere?
	map(0x1fc0, 0x1fff).ram().w(FUNC(ace_sp_state::sevenseg_w)).share("sevensegram");

	map(0x2000, 0xffff).rom();
}


#if 0
void ace_sp_state::ace_sp_portmap(address_map &map)
{
	//map(0x02, 0x02) // misc
	//map(0x05, 0x06) // AYs
}
#endif


static INPUT_PORTS_START( ace_sp )
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(ace_sp_state::gen_fixfreq)
{
	// 6303Y must take vector 0xffea periodically, as amongst other things it clears a counter
	// in RAM which is increased in one of the other interrupts, with a time-out check which
	// will cause the game to jump back to the reset vector if it fails
	m_maincpu->set_input_line(HD6301_IRQ2_LINE, HOLD_LINE);
}

void ace_sp_state::ace_sp(machine_config &config)
{
	HD6303Y(config, m_maincpu, 2000000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &ace_sp_state::ace_sp_map);

	PIA6821(config, "pia0");

	// unknown frequency
	TIMER(config, "fixedfreq").configure_periodic(FUNC(ace_sp_state::gen_fixfreq), attotime::from_hz(10));

	ACE_SP_REELCTRL(config, m_reelctrl, 2000000); // unknown clock

	config.set_default_layout(layout_ace_sp_dmd);
}

void ace_sp_state::ace_sp_pcp(machine_config &config)
{
	ace_sp(config);

	ACE_SP_REELCTRL_PCP(config.replace(), m_reelctrl, 2000000);
}



#define SP_CBOWL_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	ROM_LOAD( "cashbowlsnd1.bin", 0x00000, 0x80000, CRC(44e67cef) SHA1(3cfe48122da527e82f9058e0c5b81b5096bf4181) ) \
	ROM_LOAD( "cashbowlsnd2.bin", 0x80000, 0x80000, CRC(a28291a2) SHA1(c07b585cee89bc35c880d24eb6124796d6df423c) )
ROM_START( sp_cbowl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cashbowlgam2.bin", 0x0000, 0x8000, CRC(b20fa6f3) SHA1(63ded9527650e7810d6432fce762fe4691b87c1b) )
	ROM_LOAD( "cashbowlgam1.bin", 0x8000, 0x8000, CRC(3193cf8c) SHA1(f5a2fa260261adbdcc39a002b6fde11c3c350d84) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowla )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9392.bin", 0x0000, 0x8000, CRC(9345973d) SHA1(2a4349ea3c115fbf8ce465eae2dcfacb13ada310) )
	ROM_LOAD( "9391.bin", 0x8000, 0x8000, CRC(7f7e3253) SHA1(c42346651095db78ab85cc69da775307643fe4ce) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9393.bin", 0x0000, 0x8000, CRC(971596f0) SHA1(4a4e4a505df0b60a2be19e87ec9dd04e65d14082) )
	ROM_LOAD( "9391.bin", 0x8000, 0x8000, CRC(7f7e3253) SHA1(c42346651095db78ab85cc69da775307643fe4ce) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cb.bin", 0x00000, 0x10000, CRC(dc454e67) SHA1(57f470fbb44fe50c9c8068bbcdc9b41c617b0d82) ) // just a merged rom?
	SP_CBOWL_SOUND
ROM_END


ROM_START( sp_cbowld )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p4-5b.bin", 0x0000, 0x8000, CRC(07eae791) SHA1(9edd66f50a137063565e4152e3d0ea223a467e52) )
	ROM_LOAD( "022p4-5a.bin", 0x8000, 0x8000, CRC(ca31a175) SHA1(e25b5cc23276cf2be20e0724cfc8517ade5bb1f8) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowle )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p4-5f.bin", 0x0000, 0x8000, CRC(4bcd2648) SHA1(d55a777f3c948738b2ee29388e481e3918e8f418) )
	ROM_LOAD( "022p4-5a.bin", 0x8000, 0x8000, CRC(ca31a175) SHA1(e25b5cc23276cf2be20e0724cfc8517ade5bb1f8) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p4-5g.bin", 0x0000, 0x8000, CRC(c689e24e) SHA1(f79173f1628af0bf33de4c676f519dd901d7c867) )
	ROM_LOAD( "022p4-5a.bin", 0x8000, 0x8000, CRC(ca31a175) SHA1(e25b5cc23276cf2be20e0724cfc8517ade5bb1f8) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p4-5h.bin", 0x0000, 0x8000, CRC(4dd8fdc7) SHA1(203759247112ad3424282a7d4c75d0ed79da9738) )
	ROM_LOAD( "022p4-5a.bin", 0x8000, 0x8000, CRC(ca31a175) SHA1(e25b5cc23276cf2be20e0724cfc8517ade5bb1f8) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p4-5i.bin", 0x0000, 0x8000, CRC(c09c39c1) SHA1(ce6cd27bec22d806a9bbd654b09a853d49f2e0cd) )
	ROM_LOAD( "022p4-5a.bin", 0x8000, 0x8000, CRC(ca31a175) SHA1(e25b5cc23276cf2be20e0724cfc8517ade5bb1f8) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowli )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p5-2b.bin", 0x0000, 0x8000, CRC(dd8ea703) SHA1(a213f2010a4b525dfb73c98558b704e55aad888c) )
	ROM_LOAD( "022p5-2a.bin", 0x8000, 0x8000, CRC(7f5c9ba7) SHA1(9b06664d734f7902cd2146d0c97aaebdb5f08e5f) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p5-2c.bin", 0x0000, 0x8000, CRC(50ca6305) SHA1(8b089eb234fe0ed5bd502ff878ef0de71e440d83) )
	ROM_LOAD( "022p5-2a.bin", 0x8000, 0x8000, CRC(7f5c9ba7) SHA1(9b06664d734f7902cd2146d0c97aaebdb5f08e5f) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p5-2f.bin", 0x0000, 0x8000, CRC(91a966da) SHA1(4ce0eaa2c4f0dbcd9dafaa19526d90b25c753227) )
	ROM_LOAD( "022p5-2a.bin", 0x8000, 0x8000, CRC(7f5c9ba7) SHA1(9b06664d734f7902cd2146d0c97aaebdb5f08e5f) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowll )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p5-2g.bin", 0x0000, 0x8000, CRC(1ceda2dc) SHA1(70da06cd910ab6ae9cada17133aa5c7278394988) )
	ROM_LOAD( "022p5-2a.bin", 0x8000, 0x8000, CRC(7f5c9ba7) SHA1(9b06664d734f7902cd2146d0c97aaebdb5f08e5f) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p5-2h.bin", 0x0000, 0x8000, CRC(97bcbd55) SHA1(3bfad6beef82dac501b8ce3439c2c81e565b8b55) )
	ROM_LOAD( "022p5-2a.bin", 0x8000, 0x8000, CRC(7f5c9ba7) SHA1(9b06664d734f7902cd2146d0c97aaebdb5f08e5f) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowln )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "022p5-2i.bin", 0x0000, 0x8000, CRC(1af87953) SHA1(62d21cd63c45cee69af6fc5b83f5cc5336db2e73) )
	ROM_LOAD( "022p5-2a.bin", 0x8000, 0x8000, CRC(7f5c9ba7) SHA1(9b06664d734f7902cd2146d0c97aaebdb5f08e5f) )
	SP_CBOWL_SOUND
ROM_END

// sets below were found in 'Nudge Explosion' but appear to be Cash Bowl
ROM_START( sp_cbowlo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "033p3-2b.bin", 0x0000, 0x8000, CRC(ffeadfa8) SHA1(37d2f75fa86f170afdc70c94d57458011f3fd851) )
	ROM_LOAD( "033p3-2a.bin", 0x8000, 0x8000, CRC(a2eaf15e) SHA1(e6ce57675c7f0219394cee3653584351121f370f) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "033p3-2h.bin", 0x0000, 0x8000, CRC(b5d8c5fe) SHA1(ffd748da8457bfa89b6646e10d36895cf7cdbd6f) )
	ROM_LOAD( "033p3-2a.bin", 0x8000, 0x8000, CRC(a2eaf15e) SHA1(e6ce57675c7f0219394cee3653584351121f370f) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlq )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "033p3-2i.bin", 0x0000, 0x8000, CRC(389c01f8) SHA1(1ef7ec3f84d009157120736e13ae60baa4b9bfcf) )
	ROM_LOAD( "033p3-2a.bin", 0x8000, 0x8000, CRC(a2eaf15e) SHA1(e6ce57675c7f0219394cee3653584351121f370f) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "033p3-2w.bin", 0x0000, 0x8000, CRC(064047aa) SHA1(6927117c31293d638e5116f87b453321d089568b) )
	ROM_LOAD( "033p3-2a.bin", 0x8000, 0x8000, CRC(a2eaf15e) SHA1(e6ce57675c7f0219394cee3653584351121f370f) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowls )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "033p4-0b.bin", 0x0000, 0x8000, CRC(ca5fb438) SHA1(5f0a40c17ac4746200f4f92f28546ef2de23c13b) )
	ROM_LOAD( "033p4-0a.bin", 0x0000, 0x8000, CRC(64b09a0a) SHA1(a9a6bf4dbc6ffe7febd36d1d522747fb0755708c) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "033p4-0h.bin", 0x0000, 0x8000, CRC(6763ccbd) SHA1(b3d20079128aa12bf8ad85bee61729fe4f36eda8) )
	ROM_LOAD( "033p4-0a.bin", 0x0000, 0x8000, CRC(64b09a0a) SHA1(a9a6bf4dbc6ffe7febd36d1d522747fb0755708c) )
	SP_CBOWL_SOUND
ROM_END

ROM_START( sp_cbowlu )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "033p4-0i.bin", 0x0000, 0x8000, CRC(ea2708bb) SHA1(74e5bc75a6d6ea235a5fe59e29a54e14d8807931) )
	ROM_LOAD( "033p4-0a.bin", 0x0000, 0x8000, CRC(64b09a0a) SHA1(a9a6bf4dbc6ffe7febd36d1d522747fb0755708c) )
	SP_CBOWL_SOUND
ROM_END




#define SP_CRIME_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_crime )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "crimwatchnew2.bin", 0x0000, 0x8000, CRC(b799fa39) SHA1(7d701d9368c3db26d4f6dae9a68f2833e2d48a40) )
	ROM_LOAD( "crimwatchnew1.bin", 0x8000, 0x8000, CRC(733312bd) SHA1(bb449babd3b3eb9d23efc532b2a3ad6e6fac7837) )
	SP_CRIME_SOUND
ROM_END


ROM_START( sp_crimea )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cw591d5b", 0x0000, 0x8000, CRC(f0da19e0) SHA1(17605d6462a8feeec338a637fbc8d3d840e00431) )
	ROM_LOAD( "cw591d5a", 0x8000, 0x8000, CRC(b97effd9) SHA1(203171d993f81cc11169cf8daaa26b1639511d40) )
	SP_CRIME_SOUND
ROM_END

ROM_START( sp_crimeb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cw591d5c", 0x0000, 0x8000, CRC(7d9edde6) SHA1(af5ffc4c4b7a02ae8ce8683741c5586b21f0e971) )
	ROM_LOAD( "cw591d5a", 0x8000, 0x8000, CRC(b97effd9) SHA1(203171d993f81cc11169cf8daaa26b1639511d40) )
	SP_CRIME_SOUND
ROM_END

ROM_START( sp_crimec )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cw591d5d", 0x0000, 0x8000, CRC(f6cfc26f) SHA1(a2f6118831b29ab264d772446e73fa1e4f2ee35a) )
	ROM_LOAD( "cw591d5a", 0x8000, 0x8000, CRC(b97effd9) SHA1(203171d993f81cc11169cf8daaa26b1639511d40) )
	SP_CRIME_SOUND
ROM_END

ROM_START( sp_crimed )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cw591d5e", 0x0000, 0x8000, CRC(7b8b0669) SHA1(73aa51676a296c1e2e2184cb816e86be7204617a) )
	ROM_LOAD( "cw591d5a", 0x8000, 0x8000, CRC(b97effd9) SHA1(203171d993f81cc11169cf8daaa26b1639511d40) )
	SP_CRIME_SOUND
ROM_END

ROM_START( sp_crimee )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cw591d5f", 0x0000, 0x8000, CRC(bcfdd839) SHA1(d8f055308045c423da90e82e6b6d0cf3011f9e5a) )
	ROM_LOAD( "cw591d5a", 0x8000, 0x8000, CRC(b97effd9) SHA1(203171d993f81cc11169cf8daaa26b1639511d40) )
	SP_CRIME_SOUND
ROM_END

ROM_START( sp_crimef )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cw591d5g", 0x0000, 0x8000, CRC(31b91c3f) SHA1(c5aefaab25d127d6a4b48d3131f86fd64383c61b) )
	ROM_LOAD( "cw591d5a", 0x8000, 0x8000, CRC(b97effd9) SHA1(203171d993f81cc11169cf8daaa26b1639511d40) )
	SP_CRIME_SOUND
ROM_END

ROM_START( sp_crimeg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cw591d5h", 0x0000, 0x8000, CRC(bae803b6) SHA1(b2cc751e6b66099757dc32a5c74378843429f179) )
	ROM_LOAD( "cw591d5a", 0x8000, 0x8000, CRC(b97effd9) SHA1(203171d993f81cc11169cf8daaa26b1639511d40) )
	SP_CRIME_SOUND
ROM_END

ROM_START( sp_crimeh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cw591d5i", 0x0000, 0x8000, CRC(37acc7b0) SHA1(a4e765e67bd282c024bec6f523e553edc95d63c6) )
	ROM_LOAD( "cw591d5a", 0x8000, 0x8000, CRC(b97effd9) SHA1(203171d993f81cc11169cf8daaa26b1639511d40) )
	SP_CRIME_SOUND
ROM_END

#define SP_EMMRD_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	ROM_LOAD( "edsnd1.bin", 0x00000, 0x80000, CRC(e91382d7) SHA1(499a0606e9bbabcf207c8778323899b7b81ae372) ) \
	ROM_LOAD( "edsnd2.bin", 0x80000, 0x80000, CRC(0e103080) SHA1(2dcfcb35d04f34e4bc6da32f2d23bd8685654f8e) )


ROM_START( sp_emmrd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "015p5-0b.bin", 0x0000, 0x8000, CRC(7673ecda) SHA1(d4cf2e0c5ee4d0d3a26033f8192a01dec7df3649) )
	ROM_LOAD( "015p5-0a.bin", 0x8000, 0x8000, CRC(b4986b03) SHA1(baba6da695b0b75360ebb091ead8aa2a25c1177a) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrda )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "015p5-0c.bin", 0x0000, 0x8000, CRC(fb3728dc) SHA1(f892f8716b28e70f188babca7bf91462a61128fb) )
	ROM_LOAD( "015p5-0a.bin", 0x8000, 0x8000, CRC(b4986b03) SHA1(baba6da695b0b75360ebb091ead8aa2a25c1177a) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "015p5-0f.bin", 0x0000, 0x8000, CRC(3a542d03) SHA1(22c167924a1f58a297b99f30d379187c033aec35) )
	ROM_LOAD( "015p5-0a.bin", 0x8000, 0x8000, CRC(b4986b03) SHA1(baba6da695b0b75360ebb091ead8aa2a25c1177a) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "015p5-0g.bin", 0x0000, 0x8000, CRC(b710e905) SHA1(c5b562665df7e3302b05875bdc393501cff2bb06) )
	ROM_LOAD( "015p5-0a.bin", 0x8000, 0x8000, CRC(b4986b03) SHA1(baba6da695b0b75360ebb091ead8aa2a25c1177a) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "015p5-0h.bin", 0x0000, 0x8000, CRC(3c41f68c) SHA1(7a93bfe90718a13e0ddcefe4f2622ebea5590700) )
	ROM_LOAD( "015p5-0a.bin", 0x8000, 0x8000, CRC(b4986b03) SHA1(baba6da695b0b75360ebb091ead8aa2a25c1177a) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrde )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "015p5-0i.bin", 0x0000, 0x8000, CRC(b105328a) SHA1(00de9eefd19c96e6a821bb9e0156bfd13d1d0ca3) )
	ROM_LOAD( "015p5-0a.bin", 0x8000, 0x8000, CRC(b4986b03) SHA1(baba6da695b0b75360ebb091ead8aa2a25c1177a) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "708p6-0b.bin", 0x0000, 0x8000, CRC(b3dda1f9) SHA1(aadf8e9b85efbc8d4335cb47c399e0d936ba35cf) )
	ROM_LOAD( "708p6-0a.bin", 0x8000, 0x8000, CRC(dda4f3ff) SHA1(a7049aaa07b401482c42639161b1c5b59d9076c6) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "708p6-0c.bin", 0x0000, 0x8000, CRC(3e9965ff) SHA1(ec30f5d4c231a2f0836f22519b234a19db990497) )
	ROM_LOAD( "708p6-0a.bin", 0x8000, 0x8000, CRC(dda4f3ff) SHA1(a7049aaa07b401482c42639161b1c5b59d9076c6) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "708p6-0f.bin", 0x0000, 0x8000, CRC(fffa6020) SHA1(8d9ca0f55edae56a41514b03394139d4a736b850) )
	ROM_LOAD( "708p6-0a.bin", 0x8000, 0x8000, CRC(dda4f3ff) SHA1(a7049aaa07b401482c42639161b1c5b59d9076c6) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "708p6-0g.bin", 0x0000, 0x8000, CRC(72bea426) SHA1(01f11cf43867f8238407acd1b6d28749a619feec) )
	ROM_LOAD( "708p6-0a.bin", 0x8000, 0x8000, CRC(dda4f3ff) SHA1(a7049aaa07b401482c42639161b1c5b59d9076c6) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "708p6-0h.bin", 0x0000, 0x8000, CRC(f9efbbaf) SHA1(d2ec92424ddfb5021e5e0869222e30d8fcbd7d9d) )
	ROM_LOAD( "708p6-0a.bin", 0x8000, 0x8000, CRC(dda4f3ff) SHA1(a7049aaa07b401482c42639161b1c5b59d9076c6) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "708p6-0i.bin", 0x0000, 0x8000, CRC(74ab7fa9) SHA1(398df07fb6c085ea96105148ecc3f562d8fea394) )
	ROM_LOAD( "708p6-0a.bin", 0x8000, 0x8000, CRC(dda4f3ff) SHA1(a7049aaa07b401482c42639161b1c5b59d9076c6) )
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0015rp50.bin", 0x00000, 0x10000, CRC(e37fa624) SHA1(3d7e09a259ed53a88cd4c9dc2e39b1aadb7049c7) ) // just a merged rom?
	SP_EMMRD_SOUND
ROM_END

ROM_START( sp_emmrdo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0015rp60.bin", 0x00000, 0x10000, CRC(b27378c8) SHA1(dd8dfc587d0c051d1144f4b0205bd8d4a28ceaaf) ) // just a merged rom?
	SP_EMMRD_SOUND
ROM_END

#define SP_WOOLP_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_woolp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "711p9-0b.bin", 0x0000, 0x8000, CRC(411a5b82) SHA1(e22f52fb18395b3ca746a76ed78328847b5bef19) )
	ROM_LOAD( "711p9-0a.bin", 0x8000, 0x8000, CRC(5c9a56f7) SHA1(58c720114e29f8ccea676b0026daef9309d650bf) )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "711p9-0c.bin", 0x0000, 0x8000, CRC(cc5e9f84) SHA1(5191ff7b81993f503b9632607065e6647b75d084) )
	ROM_LOAD( "711p9-0a.bin", 0x8000, 0x8000, CRC(5c9a56f7) SHA1(58c720114e29f8ccea676b0026daef9309d650bf) )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "711p9-0f.bin", 0x0000, 0x8000, CRC(0d3d9a5b) SHA1(1e59e62c9a0265d120d1ee137dfda5d670530656) )
	ROM_LOAD( "711p9-0a.bin", 0x8000, 0x8000, CRC(5c9a56f7) SHA1(58c720114e29f8ccea676b0026daef9309d650bf) )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "711p9-0g.bin", 0x0000, 0x8000, CRC(80795e5d) SHA1(c9048363b77ce52bd53966eebae19ed42f252350) )
	ROM_LOAD( "711p9-0a.bin", 0x8000, 0x8000, CRC(5c9a56f7) SHA1(58c720114e29f8ccea676b0026daef9309d650bf) )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "711p9-0h.bin", 0x0000, 0x8000, CRC(0b2841d4) SHA1(d46ed7e1241f83260197cb7725192c27d553da83) )
	ROM_LOAD( "711p9-0a.bin", 0x8000, 0x8000, CRC(5c9a56f7) SHA1(58c720114e29f8ccea676b0026daef9309d650bf) )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "711p9-0i.bin", 0x0000, 0x8000, CRC(866c85d2) SHA1(26eb44d03cc3286c619fb9632bc4f47f035a8b3b) )
	ROM_LOAD( "711p9-0a.bin", 0x8000, 0x8000, CRC(5c9a56f7) SHA1(58c720114e29f8ccea676b0026daef9309d650bf) )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wp71p2xb", 0x0000, 0x8000, CRC(89b78194) SHA1(83968dfb5ea8e351343fd53eab74de75832b9e7e) )
	ROM_LOAD( "wp71p2xa", 0x8000, 0x8000, NO_DUMP )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wp71p2xg", 0x0000, 0x8000, CRC(48d4844b) SHA1(b307d0a830ba479a12c8f2656a34a2537d33d1dc) )
	ROM_LOAD( "wp71p2xa", 0x8000, 0x8000, NO_DUMP )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolph )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wp71p2xh", 0x0000, 0x8000, CRC(c3859bc2) SHA1(0c7c2595f75386eae10fcbc64b9cb63be30200d8) )
	ROM_LOAD( "wp71p2xa", 0x8000, 0x8000, NO_DUMP )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wp71p2xi", 0x0000, 0x8000, CRC(4ec15fc4) SHA1(67ef1ae50c059bc3b03d7f5c8a4a80dcef5d6dd3) )
	ROM_LOAD( "wp71p2xa", 0x8000, 0x8000, NO_DUMP )
	SP_WOOLP_SOUND
ROM_END


ROM_START( sp_woolpj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "014p8-0b.bin", 0x0000, 0x8000, CRC(70a0aaf6) SHA1(d8fa564739784512ff982e3c809a92fcffdef645) )
	ROM_LOAD( "014p8-0a.bin", 0x8000, 0x8000, CRC(7ecfcdbd) SHA1(e86083ac22eec4ce1f977b5c03d12b7b051bbf1e) )
	SP_WOOLP_SOUND
ROM_END


ROM_START( sp_woolpk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "014p8-0c.bin", 0x0000, 0x8000, CRC(fde46ef0) SHA1(544d71a228285162f4e6e5a0de9a380e927bfbfa) )
	ROM_LOAD( "014p8-0a.bin", 0x8000, 0x8000, CRC(7ecfcdbd) SHA1(e86083ac22eec4ce1f977b5c03d12b7b051bbf1e) )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "014p8-0f.bin", 0x0000, 0x8000, CRC(3c876b2f) SHA1(a3af4ceb6f7c32b4dd27ab47cdfb10f44fa5aec1) )
	ROM_LOAD( "014p8-0a.bin", 0x8000, 0x8000, CRC(7ecfcdbd) SHA1(e86083ac22eec4ce1f977b5c03d12b7b051bbf1e) )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "014p8-0g.bin", 0x0000, 0x8000, CRC(b1c3af29) SHA1(f584bd51f37c1e13bf6510b454352d3ccbf92202) )
	ROM_LOAD( "014p8-0a.bin", 0x8000, 0x8000, CRC(7ecfcdbd) SHA1(e86083ac22eec4ce1f977b5c03d12b7b051bbf1e) )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "014p8-0h.bin", 0x0000, 0x8000, CRC(3a92b0a0) SHA1(d1665938afd5884ff77c717a18d89be6b0a86303) )
	ROM_LOAD( "014p8-0a.bin", 0x8000, 0x8000, CRC(7ecfcdbd) SHA1(e86083ac22eec4ce1f977b5c03d12b7b051bbf1e) )
	SP_WOOLP_SOUND
ROM_END

ROM_START( sp_woolpo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "014p8-0i.bin", 0x0000, 0x8000, CRC(b7d674a6) SHA1(6833fe55189282fdbeab478d5d258cf8832ceab7) )
	ROM_LOAD( "014p8-0a.bin", 0x8000, 0x8000, CRC(7ecfcdbd) SHA1(e86083ac22eec4ce1f977b5c03d12b7b051bbf1e) )
	SP_WOOLP_SOUND
ROM_END

#define SP_ZIGZAG_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */


ROM_START( sp_zigzg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "707p6-0b.bin", 0x0000, 0x8000, CRC(087d44df) SHA1(bdce3a095c4f885968012720200b611f613ef1b5) )
	ROM_LOAD( "707p6-0a.bin", 0x8000, 0x8000, CRC(a658574b) SHA1(8e0322466e6c012a59ba0be5d008e5d7b97c04c6) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzga )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "707p6-0c.bin", 0x0000, 0x8000, CRC(853980d9) SHA1(9d19f2efc1b8188dcc90ee4617d29d882f9b5ba3) )
	ROM_LOAD( "707p6-0a.bin", 0x8000, 0x8000, CRC(a658574b) SHA1(8e0322466e6c012a59ba0be5d008e5d7b97c04c6) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzgb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "707p6-0f.bin", 0x0000, 0x8000, CRC(a89d734e) SHA1(e7473b375afa5fe23d8e1935c83ca38b0fbd326e) )
	ROM_LOAD( "707p6-0a.bin", 0x8000, 0x8000, CRC(a658574b) SHA1(8e0322466e6c012a59ba0be5d008e5d7b97c04c6) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzgc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "707p6-0g.bin", 0x0000, 0x8000, CRC(25d9b748) SHA1(43b8385bea3ae420179d9b09f65466662e7eaf9c) )
	ROM_LOAD( "707p6-0a.bin", 0x8000, 0x8000, CRC(a658574b) SHA1(8e0322466e6c012a59ba0be5d008e5d7b97c04c6) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzgd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "707p6-0h.bin", 0x0000, 0x8000, CRC(dbdaa6e9) SHA1(3b30307402ad1a536727b2b98ef1226537bb129f) )
	ROM_LOAD( "707p6-0a.bin", 0x8000, 0x8000, CRC(a658574b) SHA1(8e0322466e6c012a59ba0be5d008e5d7b97c04c6) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzge )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "707p6-0i.bin", 0x0000, 0x8000, CRC(569e62ef) SHA1(e08edd9f06c7140a6fcee41ebea6bb805a7b15aa) )
	ROM_LOAD( "707p6-0a.bin", 0x8000, 0x8000, CRC(a658574b) SHA1(8e0322466e6c012a59ba0be5d008e5d7b97c04c6) )
	SP_ZIGZAG_SOUND
ROM_END


ROM_START( sp_zigzgf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "013p5-0b.bin", 0x0000, 0x8000, CRC(f6230a91) SHA1(748a1fa356b1748f38f62089e61863b96a2a62e7) )
	ROM_LOAD( "013p5-0a.bin", 0x8000, 0x8000, CRC(2856abac) SHA1(a3ecd165189bb8ebbd0cb74c3594b19eb27183e7) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzgg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "013p5-0c.bin", 0x0000, 0x8000, CRC(7b67ce97) SHA1(0430d0331e696c1c812754341f55e5aad8e284a7) )
	ROM_LOAD( "013p5-0a.bin", 0x8000, 0x8000, CRC(2856abac) SHA1(a3ecd165189bb8ebbd0cb74c3594b19eb27183e7) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzgh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "013p5-0f.bin", 0x0000, 0x8000, CRC(56c33d00) SHA1(ff1afbca404a9dca096b3660dc01ba0e213c26f0) )
	ROM_LOAD( "013p5-0a.bin", 0x8000, 0x8000, CRC(2856abac) SHA1(a3ecd165189bb8ebbd0cb74c3594b19eb27183e7) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzgi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "013p5-0g.bin", 0x0000, 0x8000, CRC(db87f906) SHA1(9e3a32e29d7f622bfc38d0f8a48f86a551c23e78) )
	ROM_LOAD( "013p5-0a.bin", 0x8000, 0x8000, CRC(2856abac) SHA1(a3ecd165189bb8ebbd0cb74c3594b19eb27183e7) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzgj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "013p5-0h.bin", 0x0000, 0x8000, CRC(2584e8a7) SHA1(027ca1d9861824dc9235f030e73e569278b61113) )
	ROM_LOAD( "013p5-0a.bin", 0x8000, 0x8000, CRC(2856abac) SHA1(a3ecd165189bb8ebbd0cb74c3594b19eb27183e7) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzgk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "013p5-0i.bin", 0x0000, 0x8000, CRC(a8c02ca1) SHA1(ed776dc8aca3d820170ee5dd4ae5d0c440209ee9) )
	ROM_LOAD( "013p5-0a.bin", 0x8000, 0x8000, CRC(2856abac) SHA1(a3ecd165189bb8ebbd0cb74c3594b19eb27183e7) )
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzgl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0013rp50.bin", 0x00000, 0x10000, CRC(5e7ba11a) SHA1(30477ff930e9f23bf32a5bdf8573fc47ed26773d) ) // just a merged rom?
	SP_ZIGZAG_SOUND
ROM_END

ROM_START( sp_zigzgm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0013rp60.bin", 0x00000, 0x10000, CRC(fb439994) SHA1(0820cf663ed5f9600d4e0313a7b3c6f8b28db471) ) // just a merged rom?
	SP_ZIGZAG_SOUND
ROM_END

#define SP_GOLDM_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	ROM_LOAD( "gmilesnd1.bin", 0x0000, 0x80000, CRC(cb1d49a2) SHA1(ed212041017cf1305821a5d99c48723d06c16f0f) ) \
	ROM_LOAD( "gmilesnd2.bin", 0x80000, 0x80000, CRC(a58e01a9) SHA1(5cdeb10c451eaf93cc4d6a0208408b00f134f8f4) )

ROM_START( sp_goldm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm664d7h", 0x0000, 0x8000, CRC(c99e8551) SHA1(5a0f606b8b8449b864764506ca148a89bf50c3ee) )
	ROM_LOAD( "gm664d7a", 0x8000, 0x8000, CRC(a577eaca) SHA1(f0e440db685b339515eba18970361c1fb51c3769) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldma )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm664d7i", 0x0000, 0x8000, CRC(44da4157) SHA1(402f7e43083043836a5afd8ada33c6e02d6692ba) )
	ROM_LOAD( "gm664d7a", 0x8000, 0x8000, CRC(a577eaca) SHA1(f0e440db685b339515eba18970361c1fb51c3769) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "017p5-0b.bin", 0x0000, 0x8000, CRC(c63338b2) SHA1(0d4be6cce02ee8e5937395826a6b0103e2660832) )
	ROM_LOAD( "017p5-0a.bin", 0x8000, 0x8000, CRC(2b53b222) SHA1(a3c72b67404a1bb8f189c6c54f1f17aa843a01ee) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "017p5-0c.bin", 0x0000, 0x8000, CRC(4b77fcb4) SHA1(6d36482904e681544f01cb6ebd9d9c18d61d886a) )
	ROM_LOAD( "017p5-0a.bin", 0x8000, 0x8000, CRC(2b53b222) SHA1(a3c72b67404a1bb8f189c6c54f1f17aa843a01ee) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "017p5-0f.bin", 0x0000, 0x8000, CRC(8a14f96b) SHA1(b4fc17a7e56e1a50b5acf53c83b96e0b142f21e7) )
	ROM_LOAD( "017p5-0a.bin", 0x8000, 0x8000, CRC(2b53b222) SHA1(a3c72b67404a1bb8f189c6c54f1f17aa843a01ee) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldme )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "017p5-0g.bin", 0x0000, 0x8000, CRC(07503d6d) SHA1(02d24539b7c2392d212e64366c27129e073c6b72) )
	ROM_LOAD( "017p5-0a.bin", 0x8000, 0x8000, CRC(2b53b222) SHA1(a3c72b67404a1bb8f189c6c54f1f17aa843a01ee) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "017p5-0h.bin", 0x0000, 0x8000, CRC(8c0122e4) SHA1(6d4360712083bc7b9f1ed3e5de527b316276c6e0) )
	ROM_LOAD( "017p5-0a.bin", 0x8000, 0x8000, CRC(2b53b222) SHA1(a3c72b67404a1bb8f189c6c54f1f17aa843a01ee) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "017p5-0i.bin", 0x0000, 0x8000, CRC(0145e6e2) SHA1(4652d14c6ace9be085164aaee39278c024fcfedc) )
	ROM_LOAD( "017p5-0a.bin", 0x8000, 0x8000, CRC(2b53b222) SHA1(a3c72b67404a1bb8f189c6c54f1f17aa843a01ee) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "713p4-0b.bin", 0x0000, 0x8000, CRC(97d88823) SHA1(b864588c083804d994178deb6a07439058df4b24) )
	ROM_LOAD( "713p4-0a.bin", 0x8000, 0x8000, CRC(07d031bf) SHA1(bfda6f29666d4e68b93b068919d3af2c1b82f224) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "713p4-0f.bin", 0x0000, 0x8000, CRC(dbff49fa) SHA1(d57bf217ba6a21e72c1cc4c2b7799e70ec04f0c8) )
	ROM_LOAD( "713p4-0a.bin", 0x8000, 0x8000, CRC(07d031bf) SHA1(bfda6f29666d4e68b93b068919d3af2c1b82f224) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "713p4-0h.bin", 0x0000, 0x8000, CRC(ddea9275) SHA1(266fbd6a7dc336f77d9422c02a202fec28ea340e) )
	ROM_LOAD( "713p4-0a.bin", 0x8000, 0x8000, CRC(07d031bf) SHA1(bfda6f29666d4e68b93b068919d3af2c1b82f224) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "713p4-0i.bin", 0x0000, 0x8000, CRC(50ae5673) SHA1(c9503e5dfdbe3488e267b7c2fea25625515463f3) )
	ROM_LOAD( "713p4-0a.bin", 0x8000, 0x8000, CRC(07d031bf) SHA1(bfda6f29666d4e68b93b068919d3af2c1b82f224) )
	SP_GOLDM_SOUND
ROM_END


ROM_START( sp_goldml )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "723 gmsv all cash p1.2.bin", 0x0000, 0x8000, CRC(fe8c99dc) SHA1(79e124e2e1e300e78955d7764e52c5e4d47eb07d) )
	ROM_LOAD( "723 gm all cash p1.1.bin", 0x8000, 0x8000, CRC(990236ac) SHA1(2c02a478e71858d7109dd2558e4ccf885d93f2e2) )
	SP_GOLDM_SOUND
ROM_END


ROM_START( sp_goldmm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm723p1b", 0x0000, 0x8000, CRC(f21c8b9d) SHA1(033918574caaa5cf333992965152b70260c9818d) )
	ROM_LOAD( "gm723p1a", 0x8000, 0x8000, CRC(990236ac) SHA1(2c02a478e71858d7109dd2558e4ccf885d93f2e2) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm723p1c", 0x0000, 0x8000, CRC(7f584f9b) SHA1(4f48313b03654a856be9ab16b386508027901054) )
	ROM_LOAD( "gm723p1a", 0x8000, 0x8000, CRC(990236ac) SHA1(2c02a478e71858d7109dd2558e4ccf885d93f2e2) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm723p1h", 0x0000, 0x8000, CRC(73a6816e) SHA1(b11ca13a554600803051d13848f27344015c7528) )
	ROM_LOAD( "gm723p1a", 0x8000, 0x8000, CRC(990236ac) SHA1(2c02a478e71858d7109dd2558e4ccf885d93f2e2) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm723p1i", 0x0000, 0x8000, CRC(fee24568) SHA1(71e67297a4117d17e323f030fb9b853fda3244b4) )
	ROM_LOAD( "gm723p1a", 0x8000, 0x8000, CRC(990236ac) SHA1(2c02a478e71858d7109dd2558e4ccf885d93f2e2) )
	SP_GOLDM_SOUND
ROM_END


ROM_START( sp_goldmq )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gmcash2.bin", 0x0000, 0x8000, CRC(6b4e6856) SHA1(7efe009e2b293789588fbb5e15b04037d9f9c7ea) )
	ROM_LOAD( "gmcash1.bin", 0x8000, 0x8000, CRC(65453292) SHA1(614d4a0c6adf13e0a1eff149cad68cc248b95541) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm686p2b", 0x0000, 0x8000, CRC(cbae5fc7) SHA1(c28545a54f37870856fb12e4a495d0c374f25c30) )
	ROM_LOAD( "gm686p2a", 0x8000, 0x8000, CRC(65453292) SHA1(614d4a0c6adf13e0a1eff149cad68cc248b95541) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldms )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm686p2c", 0x0000, 0x8000, CRC(46ea9bc1) SHA1(6b76ce437e956af457255cb154a0a79225ca4d54) )
	ROM_LOAD( "gm686p2a", 0x8000, 0x8000, CRC(65453292) SHA1(614d4a0c6adf13e0a1eff149cad68cc248b95541) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm686p2h", 0x0000, 0x8000, CRC(4a145534) SHA1(09ee11cdc378c9cc9c59a197da5bc64310877a61) )
	ROM_LOAD( "gm686p2a", 0x8000, 0x8000, CRC(65453292) SHA1(614d4a0c6adf13e0a1eff149cad68cc248b95541) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmu )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm686p2i", 0x0000, 0x8000, CRC(c7509132) SHA1(295ec7b8c63bd53ed9ad49ed10a274f1de66ba76) )
	ROM_LOAD( "gm686p2a", 0x8000, 0x8000, CRC(65453292) SHA1(614d4a0c6adf13e0a1eff149cad68cc248b95541) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmv )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm672d5h", 0x0000, 0x8000, CRC(19d8ec08) SHA1(3d2a4c3ac0c502c54e9a9aa0e17acae4e71aece6) )
	ROM_LOAD( "gm672d5a", 0x8000, 0x8000, CRC(a9c5f26b) SHA1(183017ae1a79b2896ba0272c7dc310e0c0684f72) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmw )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm672d5i", 0x0000, 0x8000, CRC(949c280e) SHA1(15140f829f9c7661192ca6835d05f9c7d952c6c1) )
	ROM_LOAD( "gm672d5a", 0x8000, 0x8000, CRC(a9c5f26b) SHA1(183017ae1a79b2896ba0272c7dc310e0c0684f72) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmx )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm721p1f", 0x0000, 0x8000, CRC(1bb9c530) SHA1(000867d3982cd17ce1d2d01d7380946985bad560) )
	ROM_LOAD( "gm721p1a", 0x8000, 0x8000, CRC(19815a05) SHA1(01dd1c4c34aed138cc115ceac8977337ef692d53) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmy )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm721p1g", 0x0000, 0x8000, CRC(96fd0136) SHA1(93c67c98301acb8b4d769188bf4f8916fc424a7b) )
	ROM_LOAD( "gm721p1a", 0x8000, 0x8000, CRC(19815a05) SHA1(01dd1c4c34aed138cc115ceac8977337ef692d53) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldmz )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm721p1h", 0x0000, 0x8000, CRC(4feb88e4) SHA1(71708bf5c02b6fb804c5c1286fe2fc03a01b8fe5) )
	ROM_LOAD( "gm721p1a", 0x8000, 0x8000, CRC(19815a05) SHA1(01dd1c4c34aed138cc115ceac8977337ef692d53) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldm0 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gm721p1i", 0x0000, 0x8000, CRC(c2af4ce2) SHA1(e1f86a5992f8af5080b0aae7281964895e5b5af3) )
	ROM_LOAD( "gm721p1a", 0x8000, 0x8000, CRC(19815a05) SHA1(01dd1c4c34aed138cc115ceac8977337ef692d53) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldm1 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	// Hex format - convert and remove if duplicate
	ROM_LOAD( "golden_mile_p2", 0x0000, 0x01681e, CRC(dcfacf24) SHA1(982169f1ef2ca9fa48fab0aca900426651469c83) )
	ROM_LOAD( "golden_mile_p1", 0x0000, 0x01681e, CRC(047b1bf3) SHA1(31bf8581af382cf4f55e67f72ca775854b124c84) )
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldm2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0017rp40.bin", 0x00000, 0x10000, CRC(6cdd6c14) SHA1(ef57586a770c0693eb16b381356a90040ab000a6) ) // merged rom?
	SP_GOLDM_SOUND
ROM_END

ROM_START( sp_goldm3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0017rp50.bin", 0x00000, 0x10000, CRC(56e77a5d) SHA1(5eae450637f5e57a08e67306d394a03ad18093aa) )
	SP_GOLDM_SOUND
ROM_END



#define SP_GNAT_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_gnat )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn569p6b", 0x0000, 0x8000, CRC(4cb4b855) SHA1(c695b943af15aeabc897d926be9b927d44b1e3da) )
	ROM_LOAD( "gn569p6a", 0x8000, 0x8000, CRC(1ac4906b) SHA1(65025b119de034d6908f367783fe5c7617fb6a2f) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnata )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn569p6c", 0x0000, 0x8000, CRC(c1f07c53) SHA1(97a5d90f39d5e40c85bed46b4395ad49d6eed1ad) )
	ROM_LOAD( "gn569p6a", 0x8000, 0x8000, CRC(1ac4906b) SHA1(65025b119de034d6908f367783fe5c7617fb6a2f) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnatb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn569p6d", 0x0000, 0x8000, CRC(4aa163da) SHA1(556b98ff8040f52ac45dac48dc833897b7ae11be) )
	ROM_LOAD( "gn569p6a", 0x8000, 0x8000, CRC(1ac4906b) SHA1(65025b119de034d6908f367783fe5c7617fb6a2f) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnatc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn569p6e", 0x0000, 0x8000, CRC(c7e5a7dc) SHA1(b99cc0300c43e4f48515ecdc6aaea0889c6bdb3a) )
	ROM_LOAD( "gn569p6a", 0x8000, 0x8000, CRC(1ac4906b) SHA1(65025b119de034d6908f367783fe5c7617fb6a2f) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnatd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn569p6f", 0x0000, 0x8000, CRC(0093798c) SHA1(aac5ad4e230a97596875f75821c2f5a4160d61fd) )
	ROM_LOAD( "gn569p6a", 0x8000, 0x8000, CRC(1ac4906b) SHA1(65025b119de034d6908f367783fe5c7617fb6a2f) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnate )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn569p6g", 0x0000, 0x8000, CRC(8dd7bd8a) SHA1(1f4335fc7279855433848ecbd2b2c8663058bc07) )
	ROM_LOAD( "gn569p6a", 0x8000, 0x8000, CRC(1ac4906b) SHA1(65025b119de034d6908f367783fe5c7617fb6a2f) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnatf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn569p6h", 0x0000, 0x8000, CRC(0686a203) SHA1(eaa2eae39939eac9e40a63e88a2b7defa7f663a9) )
	ROM_LOAD( "gn569p6a", 0x8000, 0x8000, CRC(1ac4906b) SHA1(65025b119de034d6908f367783fe5c7617fb6a2f) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnatg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn569p6i", 0x0000, 0x8000, CRC(8bc26605) SHA1(a2a1bfd87abe9eadd7a6d3bc24655dcfbbaaab9f) )
	ROM_LOAD( "gn569p6a", 0x8000, 0x8000, CRC(1ac4906b) SHA1(65025b119de034d6908f367783fe5c7617fb6a2f) )
	SP_GNAT_SOUND
ROM_END


ROM_START( sp_gnath )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn597p6b", 0x0000, 0x8000, CRC(3e4d3189) SHA1(af42034f7f18e92f86cba27299ec2fda2cd7d07a) )
	ROM_LOAD( "gn597p6a", 0x8000, 0x8000, CRC(77a45042) SHA1(b2718b33274b570cce034d84ce9eb1afe4b38ef0) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnati )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn597p6f", 0x0000, 0x8000, CRC(726af050) SHA1(7bc1fcb85aba389895f24f9a2af0a5cda79101a0) )
	ROM_LOAD( "gn597p6a", 0x8000, 0x8000, CRC(77a45042) SHA1(b2718b33274b570cce034d84ce9eb1afe4b38ef0) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnatj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn597p6g", 0x0000, 0x8000, CRC(ff2e3456) SHA1(b5e6b8b299a87e247cbe888ae5057065ca10cdaa) )
	ROM_LOAD( "gn597p6a", 0x8000, 0x8000, CRC(77a45042) SHA1(b2718b33274b570cce034d84ce9eb1afe4b38ef0) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnatk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn597p6h", 0x0000, 0x8000, CRC(747f2bdf) SHA1(4b4e1f53f98d0eea8b7a2da2b2c070e628e5e798) )
	ROM_LOAD( "gn597p6a", 0x8000, 0x8000, CRC(77a45042) SHA1(b2718b33274b570cce034d84ce9eb1afe4b38ef0) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnatl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gn597p6i", 0x0000, 0x8000, CRC(f93befd9) SHA1(0c499435ddb9d7aa2df2e4eb454acdbfd2ba962f) )
	ROM_LOAD( "gn597p6a", 0x8000, 0x8000, CRC(77a45042) SHA1(b2718b33274b570cce034d84ce9eb1afe4b38ef0) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnatm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gncash2.bin", 0x0000, 0x8000, CRC(131a5a4c) SHA1(15da21be15655f3779671ab5948cf6f850df0d05) )
	ROM_LOAD( "gn569p6a", 0x8000, 0x8000, CRC(1ac4906b) SHA1(65025b119de034d6908f367783fe5c7617fb6a2f) ) // appears to pair with this
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnatn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "597 gnsk all cash p6.2.bin", 0x0000, 0x8000, CRC(2bd1c9c6) SHA1(4b1249fc7aca548e8acd4ee75bf2260dd512ac72) )
	ROM_LOAD( "gn597p6a", 0x8000, 0x8000, CRC(77a45042) SHA1(b2718b33274b570cce034d84ce9eb1afe4b38ef0) )
	SP_GNAT_SOUND
ROM_END

ROM_START( sp_gnato )
	ROM_REGION( 0x80000, "maincpu", 0 )
	// hex format, convert and remove if duplicate
	ROM_LOAD( "grand_national", 0x0000, 0x01681e, CRC(3390b136) SHA1(a7ebbd260ac2e36cb5ad52efc43ac88058e4cd55) )
	SP_GNAT_SOUND
ROM_END


#define SP_GPRIX_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_gprix )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gp567p6b", 0x0000, 0x8000, CRC(e0ca19a5) SHA1(8b4eec4223d29f0f4098d0539fded5adca22070b) )
	ROM_LOAD( "gp567p6a", 0x8000, 0x8000, CRC(40067f1f) SHA1(e4508c31a3d69e84304915184634bb007793cb11) )
	SP_GPRIX_SOUND
ROM_END

ROM_START( sp_gprixa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gp567p6c", 0x0000, 0x8000, CRC(6d8edda3) SHA1(5520f57dcf2a456f9020ba8338a1674e910d9992) )
	ROM_LOAD( "gp567p6a", 0x8000, 0x8000, CRC(40067f1f) SHA1(e4508c31a3d69e84304915184634bb007793cb11) )
	SP_GPRIX_SOUND
ROM_END

ROM_START( sp_gprixb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gp567p6d", 0x0000, 0x8000, CRC(e6dfc22a) SHA1(ceb6894e147bb9c3a9468f86238c439850f702cc) )
	ROM_LOAD( "gp567p6a", 0x8000, 0x8000, CRC(40067f1f) SHA1(e4508c31a3d69e84304915184634bb007793cb11) )
	SP_GPRIX_SOUND
ROM_END

ROM_START( sp_gprixc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gp567p6e", 0x0000, 0x8000, CRC(6b9b062c) SHA1(e505b71bfbefb9609663ae46c162d6e1efefdfe9) )
	ROM_LOAD( "gp567p6a", 0x8000, 0x8000, CRC(40067f1f) SHA1(e4508c31a3d69e84304915184634bb007793cb11) )
	SP_GPRIX_SOUND
ROM_END

ROM_START( sp_gprixd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gp567p6f", 0x0000, 0x8000, CRC(acedd87c) SHA1(0933472f7d25b35f514460241e4986d77cc6a006) )
	ROM_LOAD( "gp567p6a", 0x8000, 0x8000, CRC(40067f1f) SHA1(e4508c31a3d69e84304915184634bb007793cb11) )
	SP_GPRIX_SOUND
ROM_END

ROM_START( sp_gprixe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gp567p6g", 0x0000, 0x8000, CRC(21a91c7a) SHA1(0b7a6d0a7dcb1cdaecc42418540a0cfb12ff7aaf) )
	ROM_LOAD( "gp567p6a", 0x8000, 0x8000, CRC(40067f1f) SHA1(e4508c31a3d69e84304915184634bb007793cb11) )
	SP_GPRIX_SOUND
ROM_END

ROM_START( sp_gprixf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gp567p6h", 0x0000, 0x8000, CRC(aaf803f3) SHA1(d2e7f74c438cd601064c606d98d7b6bb20ba36c4) )
	ROM_LOAD( "gp567p6a", 0x8000, 0x8000, CRC(40067f1f) SHA1(e4508c31a3d69e84304915184634bb007793cb11) )
	SP_GPRIX_SOUND
ROM_END

ROM_START( sp_gprixg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gp567p6i", 0x0000, 0x8000, CRC(27bcc7f5) SHA1(f2b274d322f0d3ea786080f113d0926aff0bc0dc) )
	ROM_LOAD( "gp567p6a", 0x8000, 0x8000, CRC(40067f1f) SHA1(e4508c31a3d69e84304915184634bb007793cb11) )
	SP_GPRIX_SOUND
ROM_END

ROM_START( sp_gprixh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "567 gpsk all cash p6.2.bin", 0x0000, 0x8000, CRC(f556e1ea) SHA1(512189c051863f79a1e1b3c256e62cca8840429f) )
	ROM_LOAD( "gp567p6a", 0x8000, 0x8000, CRC(40067f1f) SHA1(e4508c31a3d69e84304915184634bb007793cb11) ) // aka 567 gp p6.1.bin
	SP_GPRIX_SOUND
ROM_END

#define SP_HIDEH_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_hideh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh525p9b", 0x0000, 0x8000, CRC(5af888b1) SHA1(189ae178728773308492a25bc68da51e93b6eb43) )
	ROM_LOAD( "hh525p9a", 0x8000, 0x8000, CRC(2b13ead3) SHA1(c7b6c649b907403c74d5849ffd8697a0e534133f) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hideha )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh525p9c", 0x0000, 0x8000, CRC(af0dfbab) SHA1(66a339f7380ba70f28e115a7a836aa194b7d36fe) )
	ROM_LOAD( "hh525p9a", 0x8000, 0x8000, CRC(2b13ead3) SHA1(c7b6c649b907403c74d5849ffd8697a0e534133f) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh525p9d", 0x0000, 0x8000, CRC(5ced533e) SHA1(0561de24116f670a99dc125b1c03e7a70e6795d1) )
	ROM_LOAD( "hh525p9a", 0x8000, 0x8000, CRC(2b13ead3) SHA1(c7b6c649b907403c74d5849ffd8697a0e534133f) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh525p9e", 0x0000, 0x8000, CRC(a9182024) SHA1(149f84805c53fc83dbe4749f22371f60d319e81d) )
	ROM_LOAD( "hh525p9a", 0x8000, 0x8000, CRC(2b13ead3) SHA1(c7b6c649b907403c74d5849ffd8697a0e534133f) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh525p9f", 0x0000, 0x8000, CRC(16df4968) SHA1(8bc8c0e47377006b545e48631d04371ed2bd1472) )
	ROM_LOAD( "hh525p9a", 0x8000, 0x8000, CRC(2b13ead3) SHA1(c7b6c649b907403c74d5849ffd8697a0e534133f) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh525p9g", 0x0000, 0x8000, CRC(e32a3a72) SHA1(464b6de8209f8ca3c2126b13aa0daf4f41e65d8c) )
	ROM_LOAD( "hh525p9a", 0x8000, 0x8000, CRC(2b13ead3) SHA1(c7b6c649b907403c74d5849ffd8697a0e534133f) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh525p9h", 0x0000, 0x8000, CRC(10ca92e7) SHA1(311e8a2feea31495a3ff7deaf2236312cf734f30) )
	ROM_LOAD( "hh525p9a", 0x8000, 0x8000, CRC(2b13ead3) SHA1(c7b6c649b907403c74d5849ffd8697a0e534133f) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh525p9i", 0x0000, 0x8000, CRC(e53fe1fd) SHA1(b73fb084bdc5c8f08e950fc5cd00a77ad2eb36c3) )
	ROM_LOAD( "hh525p9a", 0x8000, 0x8000, CRC(2b13ead3) SHA1(c7b6c649b907403c74d5849ffd8697a0e534133f) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh577p6b", 0x0000, 0x8000, CRC(4af30605) SHA1(500326a5ad3268cd51e7b238d7ca641e668421ec) )
	ROM_LOAD( "hh577p6a", 0x8000, 0x8000, CRC(1923ed77) SHA1(50325d55c904e1b6ecc7dc311978377bfdaa3351) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh577p6c", 0x0000, 0x8000, CRC(bf06751f) SHA1(429cc9e1a2e0c72c6eb16bc31d7907ea9fc6d4ac) )
	ROM_LOAD( "hh577p6a", 0x8000, 0x8000, CRC(1923ed77) SHA1(50325d55c904e1b6ecc7dc311978377bfdaa3351) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh577p6d", 0x0000, 0x8000, CRC(4ce6dd8a) SHA1(4081667745ffa33e45b2963e3210b081b7bb540a) )
	ROM_LOAD( "hh577p6a", 0x8000, 0x8000, CRC(1923ed77) SHA1(50325d55c904e1b6ecc7dc311978377bfdaa3351) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh577p6e", 0x0000, 0x8000, CRC(b913ae90) SHA1(c7188f1245628371ad9fa3f59b52f4ecb553b03d) )
	ROM_LOAD( "hh577p6a", 0x8000, 0x8000, CRC(1923ed77) SHA1(50325d55c904e1b6ecc7dc311978377bfdaa3351) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh577p6f", 0x0000, 0x8000, CRC(06d4c7dc) SHA1(78d3a0fb52c6384b31bf8a689e3179b6efbd81e4) )
	ROM_LOAD( "hh577p6a", 0x8000, 0x8000, CRC(1923ed77) SHA1(50325d55c904e1b6ecc7dc311978377bfdaa3351) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh577p6g", 0x0000, 0x8000, CRC(f321b4c6) SHA1(08d5c92f4c50d74d0274fceddbc2c005e0e32521) )
	ROM_LOAD( "hh577p6a", 0x8000, 0x8000, CRC(1923ed77) SHA1(50325d55c904e1b6ecc7dc311978377bfdaa3351) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh577p6h", 0x0000, 0x8000, CRC(00c11c53) SHA1(cb5634b07904097af6bcab9b54bfb6717ef815dd) )
	ROM_LOAD( "hh577p6a", 0x8000, 0x8000, CRC(1923ed77) SHA1(50325d55c904e1b6ecc7dc311978377bfdaa3351) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hideho )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hh577p6i", 0x0000, 0x8000, CRC(f5346f49) SHA1(dfbfef58811c724a32da38c4479cacbe24c7d447) )
	ROM_LOAD( "hh577p6a", 0x8000, 0x8000, CRC(1923ed77) SHA1(50325d55c904e1b6ecc7dc311978377bfdaa3351) )
	SP_HIDEH_SOUND
ROM_END

ROM_START( sp_hidehp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "577 hhsk p6.2.bin", 0x0000, 0x8000, CRC(5f6ffe4a) SHA1(e1817342a2c58225a98d71bff51de925de9e097b) )
	ROM_LOAD( "577 hh p6.1.bin", 0x8000, 0x8000, CRC(780515ae) SHA1(8e350aa248a6fb5bf272961472966249643208fe) ) // hacked maybe? doesn't match the other 577 'a' rom
	SP_HIDEH_SOUND
ROM_END



#define SP_HIFLY_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_hifly )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf32p10b", 0x0000, 0x8000, CRC(197f7993) SHA1(d6c60288aae0bf90831b741941992315b3362ba0) )
	ROM_LOAD( "hf32p10a", 0x8000, 0x8000, CRC(ebd53c1a) SHA1(a5176b0f85c0e18cb7e7d8dc6ca3bf7a1e4b7535) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflya )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf32p10c", 0x0000, 0x8000, CRC(943bbd95) SHA1(a1716bd89137dace0d7377a95e929f7c9e0c6461) )
	ROM_LOAD( "hf32p10a", 0x8000, 0x8000, CRC(ebd53c1a) SHA1(a5176b0f85c0e18cb7e7d8dc6ca3bf7a1e4b7535) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf32p10d", 0x0000, 0x8000, CRC(1f6aa21c) SHA1(24339000c7c87f25dc0e91dfbbd81e3397787f1e) )
	ROM_LOAD( "hf32p10a", 0x8000, 0x8000, CRC(ebd53c1a) SHA1(a5176b0f85c0e18cb7e7d8dc6ca3bf7a1e4b7535) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf32p10e", 0x0000, 0x8000, CRC(922e661a) SHA1(145e8f6529265d34bfaab039d04165c947da125a) )
	ROM_LOAD( "hf32p10a", 0x8000, 0x8000, CRC(ebd53c1a) SHA1(a5176b0f85c0e18cb7e7d8dc6ca3bf7a1e4b7535) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf32p10f", 0x0000, 0x8000, CRC(5558b84a) SHA1(08bcf9ea70bc48f557a9d1a3d5534c3e90986941) )
	ROM_LOAD( "hf32p10a", 0x8000, 0x8000, CRC(ebd53c1a) SHA1(a5176b0f85c0e18cb7e7d8dc6ca3bf7a1e4b7535) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflye )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf32p10g", 0x0000, 0x8000, CRC(d81c7c4c) SHA1(61c3e091612134006ebfab0d5a050831b5135f58) )
	ROM_LOAD( "hf32p10a", 0x8000, 0x8000, CRC(ebd53c1a) SHA1(a5176b0f85c0e18cb7e7d8dc6ca3bf7a1e4b7535) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf32p10h", 0x0000, 0x8000, CRC(534d63c5) SHA1(1db7fa29f728b1613b1c069d44a18f777b6c9263) )
	ROM_LOAD( "hf32p10a", 0x8000, 0x8000, CRC(ebd53c1a) SHA1(a5176b0f85c0e18cb7e7d8dc6ca3bf7a1e4b7535) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf32p10i", 0x0000, 0x8000, CRC(de09a7c3) SHA1(65492a63e8f52447c861d55c555db81d62b63736) )
	ROM_LOAD( "hf32p10a", 0x8000, 0x8000, CRC(ebd53c1a) SHA1(a5176b0f85c0e18cb7e7d8dc6ca3bf7a1e4b7535) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf572p6b", 0x0000, 0x8000, CRC(f99bb045) SHA1(825611e3b0e0b90534918f730375b8550d6a3be6) )
	ROM_LOAD( "hf572p6a", 0x8000, 0x8000, CRC(e6efe3ea) SHA1(6e30df4659945008defdd2f2069fb4eff45929e2) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf572p6c", 0x0000, 0x8000, CRC(74df7443) SHA1(a5309ad92217e4a181012a51ef6fd1dc4fe7c2bf) )
	ROM_LOAD( "hf572p6a", 0x8000, 0x8000, CRC(e6efe3ea) SHA1(6e30df4659945008defdd2f2069fb4eff45929e2) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf572p6d", 0x0000, 0x8000, CRC(ff8e6bca) SHA1(7f8042e43cc77d7b52fc52d9eea6493d34ad592c) )
	ROM_LOAD( "hf572p6a", 0x8000, 0x8000, CRC(e6efe3ea) SHA1(6e30df4659945008defdd2f2069fb4eff45929e2) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf572p6e", 0x0000, 0x8000, CRC(72caafcc) SHA1(c62cabe7149ccea53488ac3f080c6129561eee65) )
	ROM_LOAD( "hf572p6a", 0x8000, 0x8000, CRC(e6efe3ea) SHA1(6e30df4659945008defdd2f2069fb4eff45929e2) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf572p6f", 0x0000, 0x8000, CRC(b5bc719c) SHA1(a2ec2d38d91edd24955e790f42bcee2075c1353f) )
	ROM_LOAD( "hf572p6a", 0x8000, 0x8000, CRC(e6efe3ea) SHA1(6e30df4659945008defdd2f2069fb4eff45929e2) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflym )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf572p6g", 0x0000, 0x8000, CRC(38f8b59a) SHA1(961eb63f135ca558e3cf12d5579d0b6f5907dfb2) )
	ROM_LOAD( "hf572p6a", 0x8000, 0x8000, CRC(e6efe3ea) SHA1(6e30df4659945008defdd2f2069fb4eff45929e2) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hf572p6i", 0x0000, 0x8000, CRC(3eed6e15) SHA1(a9ca69ee4d2276aa2c0e38c957b4d6589ff8c40f) )
	ROM_LOAD( "hf572p6a", 0x8000, 0x8000, CRC(e6efe3ea) SHA1(6e30df4659945008defdd2f2069fb4eff45929e2) )
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "572 hfsv all cash p6.2.bin", 0x0000, 0x8000, CRC(ea129385) SHA1(0cbc62a8cf511553388fb11cf810b1a8a5bb9623) )
	ROM_LOAD( "hf572p6a", 0x8000, 0x8000, CRC(e6efe3ea) SHA1(6e30df4659945008defdd2f2069fb4eff45929e2) ) // aka 572 hf all cash p6.1.bin
	SP_HIFLY_SOUND
ROM_END

ROM_START( sp_hiflyp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hifly2.bin", 0x0000, 0x8000, CRC(43f88b77) SHA1(76190af4558ead2294c43c09d7bc5169cccdfeec) )
	ROM_LOAD( "hifly1.bin", 0x8000, 0x8000, CRC(5ffe7a65) SHA1(f9710e3a40e36b119d2a6a1b2bf7008a9642ac51) )
	SP_HIFLY_SOUND
ROM_END

#define SP_JURAS_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	ROM_LOAD( "jt8_snd1.bin", 0x00000, 0x80000, CRC(54f02e21) SHA1(1f2142e3cad828f3f07b729ad8394392c3a5ef46) ) \
	ROM_LOAD( "jt8_snd2.bin", 0x80000, 0x80000, CRC(6ae75d87) SHA1(f6a73c26f7715b2a2d69b05d7729571b05b2fdaa) )

ROM_START( sp_juras )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jt608d2b", 0x0000, 0x8000, CRC(4acd3e3f) SHA1(67b448ef3289c23626100c7f068360d44e9a6e96) )
	ROM_LOAD( "jt608d2a", 0x8000, 0x8000, CRC(8487a7c3) SHA1(752ad5f2e6b360d9a80db4b0277daccbd03d804f) )
	SP_JURAS_SOUND
ROM_END

#define SP_OPENB_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_openb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ob568p5b", 0x0000, 0x8000, CRC(a9d794be) SHA1(c2d706980d067f730a313332adcbf0f56c996365) )
	ROM_LOAD( "ob568p5a", 0x8000, 0x8000, CRC(29d57828) SHA1(62122bc44ce36c40afa3053b92b846e5d525eb99) )
	SP_OPENB_SOUND
ROM_END

ROM_START( sp_openba )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ob568p5c", 0x0000, 0x8000, CRC(249350b8) SHA1(42a8604b2bfacffb50a568f0a2b34d37e4a7d8bb) )
	ROM_LOAD( "ob568p5a", 0x8000, 0x8000, CRC(29d57828) SHA1(62122bc44ce36c40afa3053b92b846e5d525eb99) )
	SP_OPENB_SOUND
ROM_END

ROM_START( sp_openbb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ob568p5d", 0x0000, 0x8000, CRC(afc24f31) SHA1(4e044dd1ea3be80c04e3bc1acdba798be50ef358) )
	ROM_LOAD( "ob568p5a", 0x8000, 0x8000, CRC(29d57828) SHA1(62122bc44ce36c40afa3053b92b846e5d525eb99) )
	SP_OPENB_SOUND
ROM_END

ROM_START( sp_openbc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ob568p5e", 0x0000, 0x8000, CRC(22868b37) SHA1(57b8a7c8c7c97992d534118bdced9cce14faecb6) )
	ROM_LOAD( "ob568p5a", 0x8000, 0x8000, CRC(29d57828) SHA1(62122bc44ce36c40afa3053b92b846e5d525eb99) )
	SP_OPENB_SOUND
ROM_END

ROM_START( sp_openbd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ob568p5f", 0x0000, 0x8000, CRC(e5f05567) SHA1(1c3193804d3d08e60d60193e5e8787930e35da2f) )
	ROM_LOAD( "ob568p5a", 0x8000, 0x8000, CRC(29d57828) SHA1(62122bc44ce36c40afa3053b92b846e5d525eb99) )
	SP_OPENB_SOUND
ROM_END

ROM_START( sp_openbe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ob568p5g", 0x0000, 0x8000, CRC(68b49161) SHA1(7ebb8235a42b672809074452ec6c3120caa1d1ce) )
	ROM_LOAD( "ob568p5a", 0x8000, 0x8000, CRC(29d57828) SHA1(62122bc44ce36c40afa3053b92b846e5d525eb99) )
	SP_OPENB_SOUND
ROM_END

ROM_START( sp_openbf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ob568p5h", 0x0000, 0x8000, CRC(e3e58ee8) SHA1(f1fb6f9abaaa87a857e453f0c3a98772a7b6050e) )
	ROM_LOAD( "ob568p5a", 0x8000, 0x8000, CRC(29d57828) SHA1(62122bc44ce36c40afa3053b92b846e5d525eb99) )
	SP_OPENB_SOUND
ROM_END

ROM_START( sp_openbg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ob568p5i", 0x0000, 0x8000, CRC(6ea14aee) SHA1(f8682c1b150cd60b7878864ef2f0bf53fcf9f063) )
	ROM_LOAD( "ob568p5a", 0x8000, 0x8000, CRC(29d57828) SHA1(62122bc44ce36c40afa3053b92b846e5d525eb99) )
	SP_OPENB_SOUND
ROM_END

ROM_START( sp_openbh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "578 obsk p5.2.bin", 0x0000, 0x8000, CRC(bc4b6cf1) SHA1(99dd18004b6699f92b2cfb7ae2cff45a458239a0) )
	ROM_LOAD( "ob568p5a", 0x8000, 0x8000, CRC(29d57828) SHA1(62122bc44ce36c40afa3053b92b846e5d525eb99) ) // aka 578 ob p5.1.bin
	SP_OPENB_SOUND
ROM_END

ROM_START( sp_openbi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "otb2.bin", 0x0000, 0x8000, CRC(9169e5c1) SHA1(60cb2acfed08bb661e43216594a16aec87a59273) )
	ROM_LOAD( "otb1.bin", 0x8000, 0x8000, CRC(4dad5b26) SHA1(858803355edc3fa1c7e1f5279c26557c7cac526e) )
	SP_OPENB_SOUND
ROM_END

#define SP_PAYRS_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_payrs )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pr570p6b", 0x0000, 0x8000, CRC(7fe9b3b8) SHA1(0c629ab93c1acbe5e1b031295c6cfe0989f2d0be) )
	ROM_LOAD( "pr570p6a", 0x8000, 0x8000, CRC(215c77ff) SHA1(46b70735e2dc2deef90e21f9659a9ba4bdb30729) )
	SP_PAYRS_SOUND
ROM_END


ROM_START( sp_payrsa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pr570p6c", 0x0000, 0x8000, CRC(f2ad77be) SHA1(9da8283e7974cae09981f06df065a3c1610b3dda) )
	ROM_LOAD( "pr570p6a", 0x8000, 0x8000, CRC(215c77ff) SHA1(46b70735e2dc2deef90e21f9659a9ba4bdb30729) )
	SP_PAYRS_SOUND
ROM_END

ROM_START( sp_payrsb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pr570p6d", 0x0000, 0x8000, CRC(79fc6837) SHA1(c7587b4d00ab65e45a1d1c4ea8fab844640b5f63) )
	ROM_LOAD( "pr570p6a", 0x8000, 0x8000, CRC(215c77ff) SHA1(46b70735e2dc2deef90e21f9659a9ba4bdb30729) )
	SP_PAYRS_SOUND
ROM_END

ROM_START( sp_payrsc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pr570p6e", 0x0000, 0x8000, CRC(f4b8ac31) SHA1(8c6aab2e1ccd05efe6a1fb0a150b9ff6a6c25f0f) )
	ROM_LOAD( "pr570p6a", 0x8000, 0x8000, CRC(215c77ff) SHA1(46b70735e2dc2deef90e21f9659a9ba4bdb30729) )
	SP_PAYRS_SOUND
ROM_END


ROM_START( sp_payrsd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pr570p6f", 0x0000, 0x8000, CRC(33ce7261) SHA1(353f3d8bdd901a69bcff641639a055475a153f25) )
	ROM_LOAD( "pr570p6a", 0x8000, 0x8000, CRC(215c77ff) SHA1(46b70735e2dc2deef90e21f9659a9ba4bdb30729) )
	SP_PAYRS_SOUND
ROM_END

ROM_START( sp_payrse )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pr570p6g", 0x0000, 0x8000, CRC(be8ab667) SHA1(aafc6ff69e7196543306b5e4c99bc66294fda795) )
	ROM_LOAD( "pr570p6a", 0x8000, 0x8000, CRC(215c77ff) SHA1(46b70735e2dc2deef90e21f9659a9ba4bdb30729) )
	SP_PAYRS_SOUND
ROM_END

ROM_START( sp_payrsf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pr570p6h", 0x0000, 0x8000, CRC(35dba9ee) SHA1(cdb5b5b3dc4b999912a04c3ee8252096c64a64be) )
	ROM_LOAD( "pr570p6a", 0x8000, 0x8000, CRC(215c77ff) SHA1(46b70735e2dc2deef90e21f9659a9ba4bdb30729) )
	SP_PAYRS_SOUND
ROM_END

ROM_START( sp_payrsg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pr570p6i", 0x0000, 0x8000, CRC(b89f6de8) SHA1(9e1cf24a2cbe93ab4d438980f7d73eeeddfa5ebe) )
	ROM_LOAD( "pr570p6a", 0x8000, 0x8000, CRC(215c77ff) SHA1(46b70735e2dc2deef90e21f9659a9ba4bdb30729) )
	SP_PAYRS_SOUND
ROM_END

ROM_START( sp_payrsh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "570 prsk p6.2.bin", 0x0000, 0x8000, CRC(6a754bf7) SHA1(1aa2eb32500a9c7fd0201cabb76b4eb77edfb1ae) )
	ROM_LOAD( "570 pr p6.1.bin", 0x8000, 0x8000, CRC(c365090f) SHA1(fa596444696bf5f19058ba36ba285b49457778db) ) // doesn't match the other 570 roms, hacked?
	SP_PAYRS_SOUND
ROM_END



#define SP_PLAYA_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_playa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "playitagain-v1-6pound2.bin", 0x0000, 0x8000, CRC(7bab5c33) SHA1(46bc6fe7d5cdd998fc1e4e9a4b1a6a95cd160cf0) )
	ROM_LOAD( "playitagain-v1-6pound1.bin", 0x8000, 0x8000, CRC(e377e7af) SHA1(4ca7c8ddd15791f4d45bebe861fd3c193c7227e0) )
	SP_PLAYA_SOUND
ROM_END

ROM_START( sp_playaa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "575 pask p3.2.bin", 0x0000, 0x8000, CRC(f7d2d40d) SHA1(83e4e83217fef8d92bcba3edf1250d09243f9f79) )
	ROM_LOAD( "575 pa p3.1.bin", 0x8000, 0x8000, CRC(9e51ff86) SHA1(a2da9eee6b5f7211296e8633e5ec8eeec8ec77fd) )
	SP_PLAYA_SOUND
ROM_END

ROM_START( sp_playab )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "20p payout pia05 1.2-2.bin", 0x0000, 0x8000, CRC(8ad38f97) SHA1(51af3c3987137dc9ed16fe69e0685e08c4370611) ) ROM_IGNORE(0x8000) // overdump
	ROM_LOAD( "20p payout pia05 1.2-1.bin", 0x8000, 0x8000, NO_DUMP )
	SP_PLAYA_SOUND
ROM_END

ROM_START( sp_playac )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pia p2 master 10_10p.bin", 0x0000, 0x8000, CRC(79f5be36) SHA1(8af5f4eb5207afdec53a107d44ba63409e6f78ef) )
	ROM_LOAD( "pia p1 master 10_10p.bin", 0x8000, 0x8000, NO_DUMP )
	SP_PLAYA_SOUND
ROM_END

ROM_START( sp_playad )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pia05a_a.3_2", 0x0000, 0x8000, CRC(44c7116b) SHA1(09aaa635c1985f5d5d83a969760b12502b751daa) ) ROM_IGNORE(0x8000) // overdump
	ROM_LOAD( "pia_____.3_1", 0x8000, 0x8000, CRC(6040d0fe) SHA1(220eb8bac7ecb47036e9009fa5b3b2b28884daf1) ) ROM_IGNORE(0x8000) // overdump - might be wrong for this set
	SP_PLAYA_SOUND
ROM_END

ROM_START( sp_playae )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pia05a_b.3_2", 0x0000, 0x8000, CRC(3f4a2750) SHA1(4125521c2bf09b2d6ae4165946ff47d5d01aa16c) ) ROM_IGNORE(0x8000) // overdump
	ROM_LOAD( "pia_____.3_1", 0x8000, 0x8000, CRC(6040d0fe) SHA1(220eb8bac7ecb47036e9009fa5b3b2b28884daf1) ) ROM_IGNORE(0x8000) // overdump - might be wrong for this set
	SP_PLAYA_SOUND
ROM_END

ROM_START( sp_playaf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pia05s_a.3_2", 0x0000, 0x8000, CRC(a86bc433) SHA1(32d816dd5ec6ed09dddd3b5207ec9e795e35331a) ) ROM_IGNORE(0x8000) // overdump
	ROM_LOAD( "pia_____.3_1", 0x8000, 0x8000, CRC(6040d0fe) SHA1(220eb8bac7ecb47036e9009fa5b3b2b28884daf1) ) ROM_IGNORE(0x8000) // overdump - might be wrong for this set
	SP_PLAYA_SOUND
ROM_END

ROM_START( sp_playag )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pia05s_b.3_2", 0x0000, 0x8000, CRC(d3e6f208) SHA1(81ee113810a549176fb887fb29cc2d878431c3d0) ) ROM_IGNORE(0x8000) // overdump
	ROM_LOAD( "pia_____.3_1", 0x8000, 0x8000, CRC(6040d0fe) SHA1(220eb8bac7ecb47036e9009fa5b3b2b28884daf1) ) ROM_IGNORE(0x8000) // overdump - might be wrong for this set
	SP_PLAYA_SOUND
ROM_END

ROM_START( sp_playah )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pia10a_a.3_2", 0x0000, 0x8000, CRC(ecfa1687) SHA1(20490e9d9bfd19f8e0f26d5fa30dbc83e2f34d46) ) ROM_IGNORE(0x8000) // overdump
	ROM_LOAD( "pia_____.3_1", 0x8000, 0x8000, CRC(6040d0fe) SHA1(220eb8bac7ecb47036e9009fa5b3b2b28884daf1) ) ROM_IGNORE(0x8000) // overdump - might be wrong for this set
	SP_PLAYA_SOUND
ROM_END


ROM_START( sp_playai )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pia10a_b.3_2", 0x0000, 0x8000, CRC(977720bc) SHA1(a8199a661f3bb0c5f7a2e048a70227a62252b83b) ) ROM_IGNORE(0x8000) // overdump
	ROM_LOAD( "pia_____.3_1", 0x8000, 0x8000, CRC(6040d0fe) SHA1(220eb8bac7ecb47036e9009fa5b3b2b28884daf1) ) ROM_IGNORE(0x8000) // overdump - might be wrong for this set
	SP_PLAYA_SOUND
ROM_END

#define SP_SPELL_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	ROM_LOAD( "spellboundsnd1.bin", 0x00000, 0x80000, CRC(ab462981) SHA1(a88728eb8c5dbf114007551c7b5d4eb06cc7eb0b) ) \
	ROM_LOAD( "spellboundsnd2.bin", 0x80000, 0x80000, CRC(9ada4413) SHA1(2dc9b42cdd3a64b5e5d3eab0d68b109258d12eda) )

ROM_START( sp_spell )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "730p5f.bin", 0x0000, 0x8000, CRC(439bcf8b) SHA1(d84be299744eeb4c3040e5ed64fb64f614fe0a1c) )
	ROM_LOAD( "730p5a.bin", 0x8000, 0x8000, CRC(285b4774) SHA1(0d7816cf0a47dabdb45eb5d904e45b44a170f87f) )
	SP_SPELL_SOUND
ROM_END

ROM_START( sp_spella )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "730p5g.bin", 0x0000, 0x8000, CRC(cedf0b8d) SHA1(b7e290cf92e43e73353a95c9dce938fe6bdf26d0) )
	ROM_LOAD( "730p5a.bin", 0x8000, 0x8000, CRC(285b4774) SHA1(0d7816cf0a47dabdb45eb5d904e45b44a170f87f) )
	SP_SPELL_SOUND
ROM_END

ROM_START( sp_spellb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "730p5z.bin", 0x0000, 0x8000, CRC(1c352d92) SHA1(60f0970223504258c8fc1ce1e425e3d6a7abf575) )
	ROM_LOAD( "730p5a.bin", 0x8000, 0x8000, CRC(285b4774) SHA1(0d7816cf0a47dabdb45eb5d904e45b44a170f87f) )
	SP_SPELL_SOUND
ROM_END

ROM_START( sp_spellc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9404.bin", 0x0000, 0x8000, CRC(e4a24d14) SHA1(04086c0baf8b0eecc84a3ffd04c4e745fdc5b4a9) )
	ROM_LOAD( "9403.bin", 0x8000, 0x8000, CRC(896853bb) SHA1(508ca2d8c498c9798be65f043bf2969b7e3aa836) )
	SP_SPELL_SOUND
ROM_END

ROM_START( sp_spelld )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "spba0dsk.2", 0x0000, 0x8000, CRC(a85add92) SHA1(0193eacea983be0b62ef62d4a3cfedc2fff6ec5f) )
	ROM_LOAD( "spba0.1", 0x8000, 0x8000, CRC(48401b92) SHA1(0254a4345964bdda8aff03c4cb4198488e577c06) )
	SP_SPELL_SOUND
ROM_END

ROM_START( sp_spelle )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "spba0nav.2", 0x0000, 0x8000, CRC(6f2c03c2) SHA1(66298a3c8af2df432ef6f52b9cd736517bdb054d) )
	ROM_LOAD( "spba0.1", 0x8000, 0x8000, CRC(48401b92) SHA1(0254a4345964bdda8aff03c4cb4198488e577c06) )
	SP_SPELL_SOUND
ROM_END

ROM_START( sp_spellf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "spba0nsk.2", 0x0000, 0x8000, CRC(251e1994) SHA1(8d75b734b76c760f51c32cb645a989f3be217645) )
	ROM_LOAD( "spba0.1", 0x8000, 0x8000, CRC(48401b92) SHA1(0254a4345964bdda8aff03c4cb4198488e577c06) )
	SP_SPELL_SOUND
ROM_END

ROM_START( sp_spellg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "spba0nsv.2", 0x0000, 0x8000, CRC(230bc21b) SHA1(9492312d45c250ef3a16b933c3020a06780927ec) )
	ROM_LOAD( "spba0.1", 0x8000, 0x8000, CRC(48401b92) SHA1(0254a4345964bdda8aff03c4cb4198488e577c06) )
	SP_SPELL_SOUND
ROM_END

ROM_START( sp_spelli )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0009p5.bin", 0x00000, 0x10000, CRC(5d94418e) SHA1(92a9b6deed307b99bb57193d9974a0b4d76ee569) ) // merged rom
	SP_SPELL_SOUND
ROM_END

ROM_START( sp_spellj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0009p6.bin", 0x00000, 0x10000, CRC(1b55acee) SHA1(8364cf5b1c1d50f10e1e80031e9b8a587ec0bd39) ) // merged rom
	SP_SPELL_SOUND
ROM_END


#define SP_SWOP_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_swop )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ss579p3b", 0x0000, 0x8000, CRC(43ecddee) SHA1(18d2144f270d7017bac45338bbcd9e47d45779a3) )
	ROM_LOAD( "ss579p3a", 0x8000, 0x8000, CRC(4eb5b401) SHA1(78fa0ccc1996ba835043ff5fdd280938440a0b52) )
	SP_SWOP_SOUND
ROM_END


ROM_START( sp_swopa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ss579p3c", 0x0000, 0x8000, CRC(cea819e8) SHA1(86e4b263b42348d30bde88b8bf90502386bd7b85) )
	ROM_LOAD( "ss579p3a", 0x8000, 0x8000, CRC(4eb5b401) SHA1(78fa0ccc1996ba835043ff5fdd280938440a0b52) )
	SP_SWOP_SOUND
ROM_END

ROM_START( sp_swopb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ss579p3d", 0x0000, 0x8000, CRC(45f90661) SHA1(ef0e47c49cf33c70f3a877bff62ecf51ffe9bc1b) )
	ROM_LOAD( "ss579p3a", 0x8000, 0x8000, CRC(4eb5b401) SHA1(78fa0ccc1996ba835043ff5fdd280938440a0b52) )
	SP_SWOP_SOUND
ROM_END

ROM_START( sp_swopc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ss579p3e", 0x0000, 0x8000, CRC(c8bdc267) SHA1(b9c230289b5cea3ed13969cbf3f72a7deec46871) )
	ROM_LOAD( "ss579p3a", 0x8000, 0x8000, CRC(4eb5b401) SHA1(78fa0ccc1996ba835043ff5fdd280938440a0b52) )
	SP_SWOP_SOUND
ROM_END

ROM_START( sp_swopd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ss579p3f", 0x0000, 0x8000, CRC(0fcb1c37) SHA1(9f0f0dcd3105d35b088ee7534d45ef66605f3552) )
	ROM_LOAD( "ss579p3a", 0x8000, 0x8000, CRC(4eb5b401) SHA1(78fa0ccc1996ba835043ff5fdd280938440a0b52) )
	SP_SWOP_SOUND
ROM_END

ROM_START( sp_swope )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ss579p3g", 0x0000, 0x8000, CRC(828fd831) SHA1(33e4c11a381ea0d3c20ab7b164d5893317d503b5) )
	ROM_LOAD( "ss579p3a", 0x8000, 0x8000, CRC(4eb5b401) SHA1(78fa0ccc1996ba835043ff5fdd280938440a0b52) )
	SP_SWOP_SOUND
ROM_END

ROM_START( sp_swopf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ss579p3h", 0x0000, 0x8000, CRC(09dec7b8) SHA1(8955d48aa1a3901c96e1b9f1ca171d50591dfff8) )
	ROM_LOAD( "ss579p3a", 0x8000, 0x8000, CRC(4eb5b401) SHA1(78fa0ccc1996ba835043ff5fdd280938440a0b52) )
	SP_SWOP_SOUND
ROM_END

ROM_START( sp_swopg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ss579p3i", 0x0000, 0x8000, CRC(849a03be) SHA1(d64f529eb91dce8a5e0dfc5f3384ed13a952ff27) )
	ROM_LOAD( "ss579p3a", 0x8000, 0x8000, CRC(4eb5b401) SHA1(78fa0ccc1996ba835043ff5fdd280938440a0b52) )
	SP_SWOP_SOUND
ROM_END


#define SP_TIMEM_SOUND \
	ROM_REGION( 0x100000, "oki", 0 ) \
	ROM_LOAD( "002rs1a.bin", 0x0000, 0x80000, CRC(b7f7dcc4) SHA1(5c4f991c25c56e837502c395eeb62e2adc4dd089) ) \
	ROM_LOAD( "002rs1b.bin", 0x80000, 0x80000, CRC(a7261ad8) SHA1(cdef00bf6db78309cbf9a49117d82bda2496c0a8) )

ROM_START( sp_timem )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002p9-3b.bin", 0x0000, 0x8000, CRC(e171da03) SHA1(581e5edc9cb09b730db49e8b468b4a2bbd53147c) )
	ROM_LOAD( "002p9-3a.bin", 0x8000, 0x8000, CRC(b5ab1101) SHA1(bd4384ff1ab30a684bcf639c5677e67fa92bfc30) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timema )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002p9-3c.bin", 0x0000, 0x8000, CRC(6c351e05) SHA1(360711b9711bfb4303cd15a3c6d55c78e03cff64) )
	ROM_LOAD( "002p9-3a.bin", 0x8000, 0x8000, CRC(b5ab1101) SHA1(bd4384ff1ab30a684bcf639c5677e67fa92bfc30) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timemb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002p9-3f.bin", 0x0000, 0x8000, CRC(4191ed92) SHA1(d5c221fc5ec56c00a7218d8ac2db88b3901b662b) )
	ROM_LOAD( "002p9-3a.bin", 0x8000, 0x8000, CRC(b5ab1101) SHA1(bd4384ff1ab30a684bcf639c5677e67fa92bfc30) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timemc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002p9-3g.bin", 0x0000, 0x8000, CRC(ccd52994) SHA1(460b28f0d33083bc82d0ade7c0b92a28188073e5) )
	ROM_LOAD( "002p9-3a.bin", 0x8000, 0x8000, CRC(b5ab1101) SHA1(bd4384ff1ab30a684bcf639c5677e67fa92bfc30) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timemd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002p9-3h.bin", 0x0000, 0x8000, CRC(32d63835) SHA1(3d3e5b2a739d55982b432198ed9954de30159e62) )
	ROM_LOAD( "002p9-3a.bin", 0x8000, 0x8000, CRC(b5ab1101) SHA1(bd4384ff1ab30a684bcf639c5677e67fa92bfc30) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timeme )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002p9-3i.bin", 0x0000, 0x8000, CRC(bf92fc33) SHA1(a5cc8e2815b6de10cb7d129debfcc96c5fec5420) )
	ROM_LOAD( "002p9-3a.bin", 0x8000, 0x8000, CRC(b5ab1101) SHA1(bd4384ff1ab30a684bcf639c5677e67fa92bfc30) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timemf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002pa-0b.bin", 0x0000, 0x8000, CRC(a39fc696) SHA1(50e7b3b6084bd3a93b87ff351cde9045f5d354c6) )
	ROM_LOAD( "002pa-0a.bin", 0x8000, 0x8000, CRC(4e86fbdb) SHA1(951c2f4bda7bf0c54de89f57d96d8f6d378fae91) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timemg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002pa-0c.bin", 0x0000, 0x8000, CRC(2edb0290) SHA1(a1b6e4c4fb37dae6e11b31ece9253984bf4c49eb) )
	ROM_LOAD( "002pa-0a.bin", 0x8000, 0x8000, CRC(4e86fbdb) SHA1(951c2f4bda7bf0c54de89f57d96d8f6d378fae91) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timemh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002pa-0f.bin", 0x0000, 0x8000, CRC(037ff107) SHA1(437f784e422f07c5751f7798a1b96ab2d7520472) )
	ROM_LOAD( "002pa-0a.bin", 0x8000, 0x8000, CRC(4e86fbdb) SHA1(951c2f4bda7bf0c54de89f57d96d8f6d378fae91) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timemi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002pa-0g.bin", 0x0000, 0x8000, CRC(8e3b3501) SHA1(2d94414cfb8cbe175c81c46868ace990867361a9) )
	ROM_LOAD( "002pa-0a.bin", 0x8000, 0x8000, CRC(4e86fbdb) SHA1(951c2f4bda7bf0c54de89f57d96d8f6d378fae91) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timemj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002pa-0h.bin", 0x0000, 0x8000, CRC(703824a0) SHA1(827c1242cd13915fe121940c6a81f4e718479165) )
	ROM_LOAD( "002pa-0a.bin", 0x8000, 0x8000, CRC(4e86fbdb) SHA1(951c2f4bda7bf0c54de89f57d96d8f6d378fae91) )
	SP_TIMEM_SOUND
ROM_END

ROM_START( sp_timemk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "002pa-0i.bin", 0x0000, 0x8000, CRC(fd7ce0a6) SHA1(56c58ca94f42c67e8feaf86bf9ac4395db3fe364) )
	ROM_LOAD( "002pa-0a.bin", 0x8000, 0x8000, CRC(4e86fbdb) SHA1(951c2f4bda7bf0c54de89f57d96d8f6d378fae91) )
	SP_TIMEM_SOUND
ROM_END

#define SP_TZ_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_tz )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz576p4b", 0x0000, 0x8000, CRC(6d2fc12a) SHA1(f42e32e6da7a61542d4f43590af0336b8dece039) )
	ROM_LOAD( "tz576p4a", 0x8000, 0x8000, CRC(89247bc3) SHA1(3da13069111169367fc734729ba45eb097e2bd03) )
	SP_TZ_SOUND
ROM_END

ROM_START( sp_tza )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz576p4c", 0x0000, 0x8000, CRC(e06b052c) SHA1(ebdf847580afaad4e4ad926bc40c6005709b398f) )
	ROM_LOAD( "tz576p4a", 0x8000, 0x8000, CRC(89247bc3) SHA1(3da13069111169367fc734729ba45eb097e2bd03) )
	SP_TZ_SOUND
ROM_END

ROM_START( sp_tzb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz576p4d", 0x0000, 0x8000, CRC(6b3a1aa5) SHA1(7a4ab4c71b794542fcba0c2fd40e70af08c3a3b5) )
	ROM_LOAD( "tz576p4a", 0x8000, 0x8000, CRC(89247bc3) SHA1(3da13069111169367fc734729ba45eb097e2bd03) )
	SP_TZ_SOUND
ROM_END

ROM_START( sp_tzc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz576p4e", 0x0000, 0x8000, CRC(e67edea3) SHA1(9b5d9c679d303cdb166d946b7a29651376cf7a59) )
	ROM_LOAD( "tz576p4a", 0x8000, 0x8000, CRC(89247bc3) SHA1(3da13069111169367fc734729ba45eb097e2bd03) )
	SP_TZ_SOUND
ROM_END

ROM_START( sp_tzd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz576p4f", 0x0000, 0x8000, CRC(210800f3) SHA1(448cd63c3291f178a700cb9476d01867fe071cdd) )
	ROM_LOAD( "tz576p4a", 0x8000, 0x8000, CRC(89247bc3) SHA1(3da13069111169367fc734729ba45eb097e2bd03) )
	SP_TZ_SOUND
ROM_END

ROM_START( sp_tze )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz576p4g", 0x0000, 0x8000, CRC(ac4cc4f5) SHA1(a31ade733e3c0b5a205908fcca0d8539fdae2c7b) )
	ROM_LOAD( "tz576p4a", 0x8000, 0x8000, CRC(89247bc3) SHA1(3da13069111169367fc734729ba45eb097e2bd03) )
	SP_TZ_SOUND
ROM_END

ROM_START( sp_tzf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz576p4h", 0x0000, 0x8000, CRC(271ddb7c) SHA1(8aee4b662e642ee4ea64ea10800a2d1269650160) )
	ROM_LOAD( "tz576p4a", 0x8000, 0x8000, CRC(89247bc3) SHA1(3da13069111169367fc734729ba45eb097e2bd03) )
	SP_TZ_SOUND
ROM_END

ROM_START( sp_tzg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz576p4i", 0x0000, 0x8000, CRC(aa591f7a) SHA1(3f491677d6725e7bbb76d735a7e7ca4c157edfd7) )
	ROM_LOAD( "tz576p4a", 0x8000, 0x8000, CRC(89247bc3) SHA1(3da13069111169367fc734729ba45eb097e2bd03) )
	SP_TZ_SOUND
ROM_END

ROM_START( sp_tzh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "576 tzsk p4.2.bin", 0x0000, 0x8000, CRC(78b33965) SHA1(7e865126113b81af7013a8e0a8a6ed72fd6ae743) )
	ROM_LOAD( "tz576p4a", 0x8000, 0x8000, CRC(89247bc3) SHA1(3da13069111169367fc734729ba45eb097e2bd03) ) // aka 576 tz p4.1.bin
	SP_TZ_SOUND
ROM_END

ROM_START( sp_tzbwb ) // has BWBTWIZN ident string where others have TWILIGHT
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "tz_x6s__.2_2", 0x0000, 0x8000, CRC(303e40ac) SHA1(fa8f6f33c142909b8d407533768d79157fbe67cd) ) ROM_IGNORE(0x8000) // overdump
	ROM_LOAD( "tz______.2_1", 0x8000, 0x8000, CRC(8f6f5895) SHA1(8caedfd10b20cc7885893d07aeef8ff317bd7e40) ) ROM_IGNORE(0x8000) // overdump
	SP_TZ_SOUND
ROM_END

#define SP_TZFE_SOUND \
	ROM_REGION( 0x100000, "oki", 0 ) /* from the filename and size I'm going to guess this isn't genuine */ \
	ROM_LOAD( "tzfe_hacksound.bin", 0x0000, 0x05ea7c, BAD_DUMP CRC(e333e740) SHA1(332106987943d3043902887a0f4b2aea75d3fb04) )
ROM_START( sp_tzfe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz679d8b", 0x0000, 0x8000, CRC(b5f66994) SHA1(869a6a4c784a320b17d6552032e45d170af0e95f) )
	ROM_LOAD( "tz679d8a", 0x8000, 0x8000, CRC(3486b0d1) SHA1(9950168f6aaaecffeaa621d22a570ba1a3d82f59) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfea )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz679d8c", 0x0000, 0x8000, CRC(38b2ad92) SHA1(435989fa29346e9ff5355f7da3db7416787e1cd7) )
	ROM_LOAD( "tz679d8a", 0x8000, 0x8000, CRC(3486b0d1) SHA1(9950168f6aaaecffeaa621d22a570ba1a3d82f59) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfeb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz679d8d", 0x0000, 0x8000, CRC(b3e3b21b) SHA1(4567e3bd28af76d0c3d144506097ae407cd997fc) )
	ROM_LOAD( "tz679d8a", 0x8000, 0x8000, CRC(3486b0d1) SHA1(9950168f6aaaecffeaa621d22a570ba1a3d82f59) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfec )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz679d8e", 0x0000, 0x8000, CRC(3ea7761d) SHA1(cfe509443e7dc40a8b4ce1e15f7412481d61b1ad) )
	ROM_LOAD( "tz679d8a", 0x8000, 0x8000, CRC(3486b0d1) SHA1(9950168f6aaaecffeaa621d22a570ba1a3d82f59) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfed )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz679d8f", 0x0000, 0x8000, CRC(3ed88bac) SHA1(bd8f076aa2aebe2241e1ee51c62726bddd7286fe) )
	ROM_LOAD( "tz679d8a", 0x8000, 0x8000, CRC(3486b0d1) SHA1(9950168f6aaaecffeaa621d22a570ba1a3d82f59) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfee )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz679d8g", 0x0000, 0x8000, CRC(b39c4faa) SHA1(ec6e384b79588f9faebe66ef8cb4207ba3e3edc7) )
	ROM_LOAD( "tz679d8a", 0x8000, 0x8000, CRC(3486b0d1) SHA1(9950168f6aaaecffeaa621d22a570ba1a3d82f59) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfef )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz679d8h", 0x0000, 0x8000, CRC(38cd5023) SHA1(56e84d8caa3c6bfd917ab647ad233de05040b0f2) )
	ROM_LOAD( "tz679d8a", 0x8000, 0x8000, CRC(3486b0d1) SHA1(9950168f6aaaecffeaa621d22a570ba1a3d82f59) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfeg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz679d8i", 0x0000, 0x8000, CRC(b5899425) SHA1(da37d736268dfafd171e74ce9045d70374e0b716) )
	ROM_LOAD( "tz679d8a", 0x8000, 0x8000, CRC(3486b0d1) SHA1(9950168f6aaaecffeaa621d22a570ba1a3d82f59) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfeh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "fetzcash.2", 0x0000, 0x8000, CRC(ea588b8d) SHA1(1476d3f41080f256d98e0d7044fd9b20e07196c1) )
	ROM_LOAD( "tz679d8a", 0x8000, 0x8000, CRC(3486b0d1) SHA1(9950168f6aaaecffeaa621d22a570ba1a3d82f59) ) // seems to pair with the above
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfei )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz689p1b", 0x0000, 0x8000, CRC(427b8f7b) SHA1(0894c7924604ff2733ca56aad613ceb3b8edc004) )
	ROM_LOAD( "tz689p1a", 0x8000, 0x8000, CRC(da39c4cd) SHA1(05ba35ac6a19121884ce11baf65d8892a64f7b18) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfej )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz689p1c", 0x0000, 0x8000, CRC(cf3f4b7d) SHA1(998cb344d3b09508211ee31ece616777679291bf) )
	ROM_LOAD( "tz689p1a", 0x8000, 0x8000, CRC(da39c4cd) SHA1(05ba35ac6a19121884ce11baf65d8892a64f7b18) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfek )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz715p1b", 0x0000, 0x8000, CRC(67a2cd3b) SHA1(0ccc6d5f281a8494cadfc6d749a794feb9c0e9c8) )
	ROM_LOAD( "tz715p1a", 0x8000, 0x8000, CRC(b8ace271) SHA1(769d36155b9c090a5fcb04732dbb117d8d764597) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfel )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz715p1c", 0x0000, 0x8000, CRC(eae6093d) SHA1(cdec80a4b03643b847546d5e492ac911291ea8e9) )
	ROM_LOAD( "tz715p1a", 0x8000, 0x8000, CRC(b8ace271) SHA1(769d36155b9c090a5fcb04732dbb117d8d764597) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfem )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz715p1f", 0x0000, 0x8000, CRC(2b850ce2) SHA1(1f9418e73d073122a05f124fc8cb804716c401a2) )
	ROM_LOAD( "tz715p1a", 0x8000, 0x8000, CRC(b8ace271) SHA1(769d36155b9c090a5fcb04732dbb117d8d764597) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfen )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz715p1g", 0x0000, 0x8000, CRC(a6c1c8e4) SHA1(e5a4946a42a267ae18d906bedeb04f6031d2b8ac) )
	ROM_LOAD( "tz715p1a", 0x8000, 0x8000, CRC(b8ace271) SHA1(769d36155b9c090a5fcb04732dbb117d8d764597) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfeo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz715p1h", 0x0000, 0x8000, CRC(2d90d76d) SHA1(4d30d9c55ac68d62d8091735014f2420216af3ee) )
	ROM_LOAD( "tz715p1a", 0x8000, 0x8000, CRC(b8ace271) SHA1(769d36155b9c090a5fcb04732dbb117d8d764597) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfep )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tz715p1i", 0x0000, 0x8000, CRC(a0d4136b) SHA1(8514e0a6f01e2b6944003d183b2afd27ef1355d0) )
	ROM_LOAD( "tz715p1a", 0x8000, 0x8000, CRC(b8ace271) SHA1(769d36155b9c090a5fcb04732dbb117d8d764597) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfeq )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "011p5-0b.bin", 0x0000, 0x8000, CRC(c999ceb0) SHA1(62f3654af2e7936665157f074fe10142c5bc5ee2) )
	ROM_LOAD( "011p5-0a.bin", 0x8000, 0x8000, CRC(9887117d) SHA1(124622e2508fdf7dec5b0c747e4245f4fea3ddc9) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfer )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "011p5-0h.bin", 0x0000, 0x8000, CRC(83abd4e6) SHA1(125673138c0303280efad9e8bb132d5e07d45715) )
	ROM_LOAD( "011p5-0a.bin", 0x8000, 0x8000, CRC(9887117d) SHA1(124622e2508fdf7dec5b0c747e4245f4fea3ddc9) )
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfes )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "011p5-0i.bin", 0x0000, 0x8000, CRC(0eef10e0) SHA1(8eecbb6486c20c46e82cd5a3257f6fcefeb7cd4e) )
	ROM_LOAD( "011p5-0a.bin", 0x8000, 0x8000, CRC(9887117d) SHA1(124622e2508fdf7dec5b0c747e4245f4fea3ddc9) )
	SP_TZFE_SOUND
ROM_END


ROM_START( sp_tzfet )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0011rp40.bin", 0x00000, 0x10000, CRC(80600391) SHA1(d45e909bfe26e18047ece19bb8004f14a3388427) ) // merged rom?
	SP_TZFE_SOUND
ROM_END

ROM_START( sp_tzfeu )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0011rp50.bin", 0x00000, 0x10000, CRC(0ad67cf0) SHA1(b4b436f1f0f0b9a0b50013f85c21f203bf8528d0) ) // merged rom
	SP_TZFE_SOUND
ROM_END



#define SP_BEAU_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */


ROM_START( sp_beau )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp619d3b", 0x0000, 0x8000, CRC(3c6b35c3) SHA1(50e099d80397fea3ab9c65784c0bf8c4ca9a4b90) )
	ROM_LOAD( "bp619d3a", 0x8000, 0x8000, CRC(6ccd693a) SHA1(97d281509883623615b1d634e736d16814963cb8) )
	SP_BEAU_SOUND
ROM_END

ROM_START( sp_beaua )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp619d3c", 0x0000, 0x8000, CRC(b12ff1c5) SHA1(670dbb4339bebd7f1ed2d97f13aa814efa132660) )
	ROM_LOAD( "bp619d3a", 0x8000, 0x8000, CRC(6ccd693a) SHA1(97d281509883623615b1d634e736d16814963cb8) )
	SP_BEAU_SOUND
ROM_END

ROM_START( sp_beaub )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp619d3d", 0x0000, 0x8000, CRC(3a7eee4c) SHA1(259d0942a3bb65f1ef3643b76bb6539f715545a2) )
	ROM_LOAD( "bp619d3a", 0x8000, 0x8000, CRC(6ccd693a) SHA1(97d281509883623615b1d634e736d16814963cb8) )
	SP_BEAU_SOUND
ROM_END

ROM_START( sp_beauc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp619d3e", 0x0000, 0x8000, CRC(b73a2a4a) SHA1(83c3b0c377a2d3a326e98f80cd029cb96761e035) )
	ROM_LOAD( "bp619d3a", 0x8000, 0x8000, CRC(6ccd693a) SHA1(97d281509883623615b1d634e736d16814963cb8) )
	SP_BEAU_SOUND
ROM_END

ROM_START( sp_beaud )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp619d3f", 0x0000, 0x8000, CRC(704cf41a) SHA1(0a76b1bf93ede9b3e6af6cdef754165a69fff197) )
	ROM_LOAD( "bp619d3a", 0x8000, 0x8000, CRC(6ccd693a) SHA1(97d281509883623615b1d634e736d16814963cb8) )
	SP_BEAU_SOUND
ROM_END

ROM_START( sp_beaue )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp619d3g", 0x0000, 0x8000, CRC(fd08301c) SHA1(09d4a62a9a00f5c8b72cc28d6a91cdfba57a554f) )
	ROM_LOAD( "bp619d3a", 0x8000, 0x8000, CRC(6ccd693a) SHA1(97d281509883623615b1d634e736d16814963cb8) )
	SP_BEAU_SOUND
ROM_END

ROM_START( sp_beauf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp619d3h", 0x0000, 0x8000, CRC(76592f95) SHA1(8c633582787b65fc4c52a7fcec25d38c5e62dc26) )
	ROM_LOAD( "bp619d3a", 0x8000, 0x8000, CRC(6ccd693a) SHA1(97d281509883623615b1d634e736d16814963cb8) )
	SP_BEAU_SOUND
ROM_END

ROM_START( sp_beaug )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp619d3i", 0x0000, 0x8000, CRC(fb1deb93) SHA1(f1cc0b874488120416543f75057af5a4356540ea) )
	ROM_LOAD( "bp619d3a", 0x8000, 0x8000, CRC(6ccd693a) SHA1(97d281509883623615b1d634e736d16814963cb8) )
	SP_BEAU_SOUND
ROM_END

ROM_START( sp_beauh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "619 bpsv all cash d3.2.bin", 0x0000, 0x8000, CRC(2fe21603) SHA1(6f22fc33e52c003f0734cc4773c4c0e0a1711b40) )
	ROM_LOAD( "bp619d3a", 0x8000, 0x8000, CRC(6ccd693a) SHA1(97d281509883623615b1d634e736d16814963cb8) ) // aka 619 bp all cash d3.1.bin
	SP_BEAU_SOUND
ROM_END


#define SP_BIGBD_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_bigbd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bb10d13b", 0x0000, 0x8000, CRC(011bbbd8) SHA1(9fc0fff292badcf636d9d22a8a16f5e832e498a5) )
	ROM_LOAD( "bb10d13a", 0x8000, 0x8000, CRC(c5ca7121) SHA1(5a7e9ff79a9ce6c21f07b04a31ec22ebb80d61e1) )
	SP_BIGBD_SOUND
ROM_END

ROM_START( sp_bigbda )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bb10d13c", 0x0000, 0x8000, CRC(8c5f7fde) SHA1(f4e070041b2a06d2ed61d8a9ebe0c503036c93e4) )
	ROM_LOAD( "bb10d13a", 0x8000, 0x8000, CRC(c5ca7121) SHA1(5a7e9ff79a9ce6c21f07b04a31ec22ebb80d61e1) )
	SP_BIGBD_SOUND
ROM_END



#define SP_BRKBK_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_brkbk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "btb80dsk.2", 0x0000, 0x8000, CRC(a13b8dc7) SHA1(eca89375e02a15b0031e3d1c0acc053eb7906b1a) )
	ROM_LOAD( "btb80.1", 0x8000, 0x8000, CRC(9a828e00) SHA1(06784d0b3586930bbbbb880f088ba4b417d71587) )
	SP_BRKBK_SOUND
ROM_END

ROM_START( sp_brkbka )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "btb80nav.2", 0x0000, 0x8000, CRC(664d5397) SHA1(ce48d52139dc14542186d60975435a22c3b1006c) )
	ROM_LOAD( "btb80.1", 0x8000, 0x8000, CRC(9a828e00) SHA1(06784d0b3586930bbbbb880f088ba4b417d71587) )
	SP_BRKBK_SOUND
ROM_END

ROM_START( sp_brkbkb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "btb80nsk.2", 0x0000, 0x8000, CRC(2c7f49c1) SHA1(9e55bec6710b763e0dc300c45a01b81a26074853) )
	ROM_LOAD( "btb80.1", 0x8000, 0x8000, CRC(9a828e00) SHA1(06784d0b3586930bbbbb880f088ba4b417d71587) )
	SP_BRKBK_SOUND
ROM_END

ROM_START( sp_brkbkc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "btb80nsv.2", 0x0000, 0x8000, CRC(2a6a924e) SHA1(0ab0404b8b8e651f6c40f8d8a4166ccfbd2df3b2) )
	ROM_LOAD( "btb80.1", 0x8000, 0x8000, CRC(9a828e00) SHA1(06784d0b3586930bbbbb880f088ba4b417d71587) )
	SP_BRKBK_SOUND
ROM_END

ROM_START( sp_brkbkd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0008p6.bin", 0x00000, 0x10000, CRC(4fc20e58) SHA1(4530998a60731283430801028a59b7a4fbd3f1bc) ) // merged rom
	SP_BRKBK_SOUND
ROM_END



#define SP_CAMEL_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */


ROM_START( sp_camel )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca552p7b", 0x0000, 0x8000, CRC(96046b89) SHA1(e26a0de1ec735b5302f27ee8c3df2314c717eaf7) )
	ROM_LOAD( "ca552p7a", 0x8000, 0x8000, CRC(40f32602) SHA1(ff7067db5cbf3ff997032a21e8d9c493e9192ec1) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camela )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca552p7c", 0x0000, 0x8000, CRC(1b40af8f) SHA1(705896b8fd0741d65ba78711999ad8f7a0d97796) )
	ROM_LOAD( "ca552p7a", 0x8000, 0x8000, CRC(40f32602) SHA1(ff7067db5cbf3ff997032a21e8d9c493e9192ec1) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camelb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca552p7d", 0x0000, 0x8000, CRC(9011b006) SHA1(8ed8ea62a735e86a8ff90564c5ccd2893a886c9a) )
	ROM_LOAD( "ca552p7a", 0x8000, 0x8000, CRC(40f32602) SHA1(ff7067db5cbf3ff997032a21e8d9c493e9192ec1) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camelc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca552p7e", 0x0000, 0x8000, CRC(1d557400) SHA1(eb707f20b90cbaec6e1d4a3209aac44e41bd5c2e) )
	ROM_LOAD( "ca552p7a", 0x8000, 0x8000, CRC(40f32602) SHA1(ff7067db5cbf3ff997032a21e8d9c493e9192ec1) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_cameld )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca552p7f", 0x0000, 0x8000, CRC(da23aa50) SHA1(94181fb1edacd4c49aaa9047f4ab1f39248d3382) )
	ROM_LOAD( "ca552p7a", 0x8000, 0x8000, CRC(40f32602) SHA1(ff7067db5cbf3ff997032a21e8d9c493e9192ec1) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camele )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca552p7g", 0x0000, 0x8000, CRC(57676e56) SHA1(a1fb17e230d528f262e1248d28870f16116b1559) )
	ROM_LOAD( "ca552p7a", 0x8000, 0x8000, CRC(40f32602) SHA1(ff7067db5cbf3ff997032a21e8d9c493e9192ec1) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camelf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca552p7h", 0x0000, 0x8000, CRC(dc3671df) SHA1(a41acac50f94a7ca6b80c56c9e6d7469c920ba37) )
	ROM_LOAD( "ca552p7a", 0x8000, 0x8000, CRC(40f32602) SHA1(ff7067db5cbf3ff997032a21e8d9c493e9192ec1) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camelg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca552p7i", 0x0000, 0x8000, CRC(5172b5d9) SHA1(ec286a283178d33c966d718f8a65073e4d3d73ea) )
	ROM_LOAD( "ca552p7a", 0x8000, 0x8000, CRC(40f32602) SHA1(ff7067db5cbf3ff997032a21e8d9c493e9192ec1) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camelh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca562p5b", 0x0000, 0x8000, CRC(30c82da8) SHA1(ee20c6960f726163cb9d77777b02594789633850) )
	ROM_LOAD( "ca562p5a", 0x8000, 0x8000, CRC(61ccd154) SHA1(07345e0d8d90ef36737b30a00a7f8d252aaaf596) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_cameli )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca562p5c", 0x0000, 0x8000, CRC(bd8ce9ae) SHA1(0a2553dc9a189e940e176a40df766a752d882d87) )
	ROM_LOAD( "ca562p5a", 0x8000, 0x8000, CRC(61ccd154) SHA1(07345e0d8d90ef36737b30a00a7f8d252aaaf596) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camelj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca562p5d", 0x0000, 0x8000, CRC(36ddf627) SHA1(319c68600a729cd249fb7d86a1f94067f0ba90de) )
	ROM_LOAD( "ca562p5a", 0x8000, 0x8000, CRC(61ccd154) SHA1(07345e0d8d90ef36737b30a00a7f8d252aaaf596) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camelk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca562p5e", 0x0000, 0x8000, CRC(bb993221) SHA1(c71e923cdf2338a37a50c09ce3c4ab2c1f932624) )
	ROM_LOAD( "ca562p5a", 0x8000, 0x8000, CRC(61ccd154) SHA1(07345e0d8d90ef36737b30a00a7f8d252aaaf596) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camell )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca562p5f", 0x0000, 0x8000, CRC(7cefec71) SHA1(581f4b9b8bcde27860361b1ef1d79e91f8a81308) )
	ROM_LOAD( "ca562p5a", 0x8000, 0x8000, CRC(61ccd154) SHA1(07345e0d8d90ef36737b30a00a7f8d252aaaf596) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camelm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca562p5g", 0x0000, 0x8000, CRC(f1ab2877) SHA1(ad3f66a8915d4cc640947c74a099084eca0fb01e) )
	ROM_LOAD( "ca562p5a", 0x8000, 0x8000, CRC(61ccd154) SHA1(07345e0d8d90ef36737b30a00a7f8d252aaaf596) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_cameln )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca562p5h", 0x0000, 0x8000, CRC(7afa37fe) SHA1(d01a079ac8b11cb5a248b74253376913b93f7796) )
	ROM_LOAD( "ca562p5a", 0x8000, 0x8000, CRC(61ccd154) SHA1(07345e0d8d90ef36737b30a00a7f8d252aaaf596) )
	SP_CAMEL_SOUND
ROM_END

ROM_START( sp_camelo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ca562p5i", 0x0000, 0x8000, CRC(f7bef3f8) SHA1(ee9e4adc4f918e34012a0527e722402dec33f6fc) )
	ROM_LOAD( "ca562p5a", 0x8000, 0x8000, CRC(61ccd154) SHA1(07345e0d8d90ef36737b30a00a7f8d252aaaf596) )
	SP_CAMEL_SOUND
ROM_END



#define SP_CLBNA_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_clbna )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cn620d3b", 0x0000, 0x8000, CRC(7253bff1) SHA1(f2f148669b60438950eefa45f7f6d9fdeed13f4b) )
	ROM_LOAD( "cn620d3a", 0x8000, 0x8000, CRC(abe0cb02) SHA1(983702166303d796aa805f9a7daad696bbcf0c7f) )
	SP_CLBNA_SOUND
ROM_END


ROM_START( sp_clbnaa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cn620d3c", 0x0000, 0x8000, CRC(ff177bf7) SHA1(7ad64041a9ebeb4e3a60445987121f3db724bcaa) )
	ROM_LOAD( "cn620d3a", 0x8000, 0x8000, CRC(abe0cb02) SHA1(983702166303d796aa805f9a7daad696bbcf0c7f) )
	SP_CLBNA_SOUND
ROM_END



#define SP_CODER_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_coder )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cr655d3b", 0x0000, 0x8000, CRC(a41a1de4) SHA1(868b3e8740e875e2a9109c92c8df3de3e32c7265) )
	ROM_LOAD( "cr655d3a", 0x8000, 0x8000, CRC(9df65a1b) SHA1(c2a536bac290e79203dcdb26f3b88dd2740a3518) )
	SP_CODER_SOUND
ROM_END


ROM_START( sp_codera )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cr655d3c", 0x0000, 0x8000, CRC(295ed9e2) SHA1(4d733f3710dff413554f591d746f3081f90fc782) )
	ROM_LOAD( "cr655d3a", 0x8000, 0x8000, CRC(9df65a1b) SHA1(c2a536bac290e79203dcdb26f3b88dd2740a3518) )
	SP_CODER_SOUND
ROM_END


ROM_START( sp_coderb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cr655d3d", 0x0000, 0x8000, CRC(a20fc66b) SHA1(df67d0fe655e218afc92342609411e76be4dd47b) )
	ROM_LOAD( "cr655d3a", 0x8000, 0x8000, CRC(9df65a1b) SHA1(c2a536bac290e79203dcdb26f3b88dd2740a3518) )
	SP_CODER_SOUND
ROM_END

ROM_START( sp_coderc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cr655d3e", 0x0000, 0x8000, CRC(2f4b026d) SHA1(40cf7d9a181bf67a9ef13f5525a036c8f3c561e8) )
	ROM_LOAD( "cr655d3a", 0x8000, 0x8000, CRC(9df65a1b) SHA1(c2a536bac290e79203dcdb26f3b88dd2740a3518) )
	SP_CODER_SOUND
ROM_END

ROM_START( sp_coderd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cr655d3f", 0x0000, 0x8000, CRC(e83ddc3d) SHA1(ed5cde1828dc61510e8675829e78984d20da29f3) )
	ROM_LOAD( "cr655d3a", 0x8000, 0x8000, CRC(9df65a1b) SHA1(c2a536bac290e79203dcdb26f3b88dd2740a3518) )
	SP_CODER_SOUND
ROM_END

ROM_START( sp_codere )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cr655d3g", 0x0000, 0x8000, CRC(6579183b) SHA1(9a3be8153116bc638a3d846d12f93ec89beb5423) )
	ROM_LOAD( "cr655d3a", 0x8000, 0x8000, CRC(9df65a1b) SHA1(c2a536bac290e79203dcdb26f3b88dd2740a3518) )
	SP_CODER_SOUND
ROM_END

ROM_START( sp_coderf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cr655d3h", 0x0000, 0x8000, CRC(ee2807b2) SHA1(73fb60ab33d7a8ccba68ba49d29ac6340b52853d) )
	ROM_LOAD( "cr655d3a", 0x8000, 0x8000, CRC(9df65a1b) SHA1(c2a536bac290e79203dcdb26f3b88dd2740a3518) )
	SP_CODER_SOUND
ROM_END

ROM_START( sp_coderg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cr655d3i", 0x0000, 0x8000, CRC(636cc3b4) SHA1(4a5b786df56afe6bb0802f6246e6e77ae13ba391) )
	ROM_LOAD( "cr655d3a", 0x8000, 0x8000, CRC(9df65a1b) SHA1(c2a536bac290e79203dcdb26f3b88dd2740a3518) )
	SP_CODER_SOUND
ROM_END


#define SP_CRISS_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_criss )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc590d2b", 0x0000, 0x8000, CRC(05c01b41) SHA1(cee6c4ed440e378e53cb11ae9d100b2be5c99134) )
	ROM_LOAD( "cc590d2a", 0x8000, 0x8000, CRC(8519dd4c) SHA1(14446d9cc8dc4c3120ec8a1a6d151bc094733a00) )
	SP_CRISS_SOUND
ROM_END

ROM_START( sp_crissa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc590d2c", 0x0000, 0x8000, CRC(8884df47) SHA1(2339cf16a30597e37420eaee195a358a74cc7a8c) )
	ROM_LOAD( "cc590d2a", 0x8000, 0x8000, CRC(8519dd4c) SHA1(14446d9cc8dc4c3120ec8a1a6d151bc094733a00) )
	SP_CRISS_SOUND
ROM_END

ROM_START( sp_crissb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc590d2d", 0x0000, 0x8000, CRC(03d5c0ce) SHA1(ec5024136648c3099fa5830fb165f4a4d7f18d43) )
	ROM_LOAD( "cc590d2a", 0x8000, 0x8000, CRC(8519dd4c) SHA1(14446d9cc8dc4c3120ec8a1a6d151bc094733a00) )
	SP_CRISS_SOUND
ROM_END

ROM_START( sp_crissc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc590d2e", 0x0000, 0x8000, CRC(8e9104c8) SHA1(58ea715512ba4399445fadc5e1e6bcd5e0feb0b2) )
	ROM_LOAD( "cc590d2a", 0x8000, 0x8000, CRC(8519dd4c) SHA1(14446d9cc8dc4c3120ec8a1a6d151bc094733a00) )
	SP_CRISS_SOUND
ROM_END

ROM_START( sp_crissd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc590d2f", 0x0000, 0x8000, CRC(49e7da98) SHA1(6aa68fc5ea2801a539446131da2a2f9965e366a4) )
	ROM_LOAD( "cc590d2a", 0x8000, 0x8000, CRC(8519dd4c) SHA1(14446d9cc8dc4c3120ec8a1a6d151bc094733a00) )
	SP_CRISS_SOUND
ROM_END

ROM_START( sp_crisse )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc590d2g", 0x0000, 0x8000, CRC(c4a31e9e) SHA1(691f5ad7380cdf072ad71df0ed664e26fde1602d) )
	ROM_LOAD( "cc590d2a", 0x8000, 0x8000, CRC(8519dd4c) SHA1(14446d9cc8dc4c3120ec8a1a6d151bc094733a00) )
	SP_CRISS_SOUND
ROM_END

ROM_START( sp_crissf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc590d2h", 0x0000, 0x8000, CRC(4ff20117) SHA1(a94350de04f797e69ede93e1f64a686be2be8e46) )
	ROM_LOAD( "cc590d2a", 0x8000, 0x8000, CRC(8519dd4c) SHA1(14446d9cc8dc4c3120ec8a1a6d151bc094733a00) )
	SP_CRISS_SOUND
ROM_END

ROM_START( sp_crissg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cc590d2i", 0x0000, 0x8000, CRC(c2b6c511) SHA1(1960aad3dbe0ea2d1cf142b7d27a1a7c7c7a5049) )
	ROM_LOAD( "cc590d2a", 0x8000, 0x8000, CRC(8519dd4c) SHA1(14446d9cc8dc4c3120ec8a1a6d151bc094733a00) )
	SP_CRISS_SOUND
ROM_END



#define SP_DAYTR_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */


ROM_START( sp_daytr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dt656p5b", 0x0000, 0x8000, CRC(925884b2) SHA1(da9284b9b23720a20b794c0a9122872d372b6cd2) )
	ROM_LOAD( "dt656p5a", 0x8000, 0x8000, CRC(272a7e08) SHA1(b8a7c31fba74adaea270c2a394636d9bdbf3d443) )
	SP_DAYTR_SOUND
ROM_END

ROM_START( sp_daytra )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dt656p5c", 0x0000, 0x8000, CRC(1f1c40b4) SHA1(e0b9842c34370fbdcfa6f1aa3121403d61487f0f) )
	ROM_LOAD( "dt656p5a", 0x8000, 0x8000, CRC(272a7e08) SHA1(b8a7c31fba74adaea270c2a394636d9bdbf3d443) )
	SP_DAYTR_SOUND
ROM_END

ROM_START( sp_daytrb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dt656p5d", 0x0000, 0x8000, CRC(944d5f3d) SHA1(67279ae14eea9bc93465e72f20b0e04f804cdea4) )
	ROM_LOAD( "dt656p5a", 0x8000, 0x8000, CRC(272a7e08) SHA1(b8a7c31fba74adaea270c2a394636d9bdbf3d443) )
	SP_DAYTR_SOUND
ROM_END

ROM_START( sp_daytrc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dt656p5e", 0x0000, 0x8000, CRC(19099b3b) SHA1(77f681681b5489c730bbc2585332f6bb2bc06d0b) )
	ROM_LOAD( "dt656p5a", 0x8000, 0x8000, CRC(272a7e08) SHA1(b8a7c31fba74adaea270c2a394636d9bdbf3d443) )
	SP_DAYTR_SOUND
ROM_END



#define SP_DONKY_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_donky )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dd663p2b", 0x0000, 0x8000, CRC(49c60006) SHA1(9a4964df1238f267cdf05fa063f7de8b5716da10) )
	ROM_LOAD( "dd663p2a", 0x8000, 0x8000, CRC(9a477e7e) SHA1(d6206495c1a75ac2c1ce51f24ca18898916b6e11) )
	SP_DONKY_SOUND
ROM_END

ROM_START( sp_donkya )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dd663p2c", 0x0000, 0x8000, CRC(c482c400) SHA1(dc5087260772807725ce08e7fd89ee5c19406fe5) )
	ROM_LOAD( "dd663p2a", 0x8000, 0x8000, CRC(9a477e7e) SHA1(d6206495c1a75ac2c1ce51f24ca18898916b6e11) )
	SP_DONKY_SOUND
ROM_END

ROM_START( sp_donkyb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dd663p2d", 0x0000, 0x8000, CRC(4fd3db89) SHA1(f1233adfbfd95c87ebf7706bb242b7297947699c) )
	ROM_LOAD( "dd663p2a", 0x8000, 0x8000, CRC(9a477e7e) SHA1(d6206495c1a75ac2c1ce51f24ca18898916b6e11) )
	SP_DONKY_SOUND
ROM_END

ROM_START( sp_donkyc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dd663p2e", 0x0000, 0x8000, CRC(c2971f8f) SHA1(c14db5294c397148293c496d52ac0b41067952b2) )
	ROM_LOAD( "dd663p2a", 0x8000, 0x8000, CRC(9a477e7e) SHA1(d6206495c1a75ac2c1ce51f24ca18898916b6e11) )
	SP_DONKY_SOUND
ROM_END

ROM_START( sp_donkyd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dd663p2f", 0x0000, 0x8000, CRC(05e1c1df) SHA1(c00fe8553eac50229dc6bf2a5016c353fcb61c82) )
	ROM_LOAD( "dd663p2a", 0x8000, 0x8000, CRC(9a477e7e) SHA1(d6206495c1a75ac2c1ce51f24ca18898916b6e11) )
	SP_DONKY_SOUND
ROM_END

ROM_START( sp_donkye )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dd663p2g", 0x0000, 0x8000, CRC(88a505d9) SHA1(0d10f8e102daddde840b582cb962270f6180a399) )
	ROM_LOAD( "dd663p2a", 0x8000, 0x8000, CRC(9a477e7e) SHA1(d6206495c1a75ac2c1ce51f24ca18898916b6e11) )
	SP_DONKY_SOUND
ROM_END

ROM_START( sp_donkyf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dd663p2h", 0x0000, 0x8000, CRC(03f41a50) SHA1(907a947449a7dccfff00dca2536e0bc230b5771d) )
	ROM_LOAD( "dd663p2a", 0x8000, 0x8000, CRC(9a477e7e) SHA1(d6206495c1a75ac2c1ce51f24ca18898916b6e11) )
	SP_DONKY_SOUND
ROM_END

ROM_START( sp_donkyg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dd663p2i", 0x0000, 0x8000, CRC(8eb0de56) SHA1(6411e9008b09f9387622911e09d954dc7e89c6cf) )
	ROM_LOAD( "dd663p2a", 0x8000, 0x8000, CRC(9a477e7e) SHA1(d6206495c1a75ac2c1ce51f24ca18898916b6e11) )
	SP_DONKY_SOUND
ROM_END



#define SP_DYOUR_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_dyour )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "dym467p2.2.bin", 0x0000, 0x8000, CRC(c85dede8) SHA1(c6cc3bb343983aa95f3c95992949ba49d5076a32) )
	ROM_LOAD( "dym467p2.1.bin", 0x8000, 0x8000, CRC(fd9999c9) SHA1(929aa9bcf92d264d700d629ad3f1c0417d679651) )
	SP_DYOUR_SOUND
ROM_END


#define SP_FESTI_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_festi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0001rp10.bin", 0x00000, 0x10000, CRC(e584c26e) SHA1(dc48e27895c5c0b2004f6bc74ec0bdfa24af9613) ) // merged rom
	SP_FESTI_SOUND
ROM_END

#define SP_BEAU2_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_beau2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp618p1b", 0x0000, 0x8000, CRC(834501f9) SHA1(7552506d187ebc3322e2b5e913853eaff1615ef2) )
	ROM_LOAD( "bp618p1a", 0x8000, 0x8000, CRC(8d484a66) SHA1(a919b4dc49a0103a2a2f6ed25f4d4c9a57c60fe1) )
	SP_BEAU2_SOUND
ROM_END

ROM_START( sp_beau2a )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp618p1c", 0x0000, 0x8000, CRC(0e01c5ff) SHA1(9c703a641763429214a62a3c60a18ea07f516173) )
	ROM_LOAD( "bp618p1a", 0x8000, 0x8000, CRC(8d484a66) SHA1(a919b4dc49a0103a2a2f6ed25f4d4c9a57c60fe1) )
	SP_BEAU2_SOUND
ROM_END

ROM_START( sp_beau2b )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp618p1d", 0x0000, 0x8000, CRC(8550da76) SHA1(da88af51e2408e0717ee2a053efe47da483c825e) )
	ROM_LOAD( "bp618p1a", 0x8000, 0x8000, CRC(8d484a66) SHA1(a919b4dc49a0103a2a2f6ed25f4d4c9a57c60fe1) )
	SP_BEAU2_SOUND
ROM_END

ROM_START( sp_beau2c )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp618p1e", 0x0000, 0x8000, CRC(08141e70) SHA1(1d9223cff34263d6bfcd24ce89d189d96cb3261e) )
	ROM_LOAD( "bp618p1a", 0x8000, 0x8000, CRC(8d484a66) SHA1(a919b4dc49a0103a2a2f6ed25f4d4c9a57c60fe1) )
	SP_BEAU2_SOUND
ROM_END

ROM_START( sp_beau2d )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp618p1f", 0x0000, 0x8000, CRC(cf62c020) SHA1(18ee3ea51dca5724fa7e307dcc25a348eee64cec) )
	ROM_LOAD( "bp618p1a", 0x8000, 0x8000, CRC(8d484a66) SHA1(a919b4dc49a0103a2a2f6ed25f4d4c9a57c60fe1) )
	SP_BEAU2_SOUND
ROM_END

ROM_START( sp_beau2e )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp618p1g", 0x0000, 0x8000, CRC(42260426) SHA1(890204f06c0103a60a5ece55e22739bd40004a84) )
	ROM_LOAD( "bp618p1a", 0x8000, 0x8000, CRC(8d484a66) SHA1(a919b4dc49a0103a2a2f6ed25f4d4c9a57c60fe1) )
	SP_BEAU2_SOUND
ROM_END

ROM_START( sp_beau2f )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "bp618p1i", 0x0000, 0x8000, CRC(4433dfa9) SHA1(8ccdcb0a638f28b5772dc1a5865be6573b8816aa) )
	ROM_LOAD( "bp618p1a", 0x8000, 0x8000, CRC(8d484a66) SHA1(a919b4dc49a0103a2a2f6ed25f4d4c9a57c60fe1) )
	SP_BEAU2_SOUND
ROM_END



#define SP_GHOST_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_ghost )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt668p3b", 0x0000, 0x8000, CRC(94c96f58) SHA1(65b1e4874f7d34289df0ed50bb8553d8c56c1f52) )
	ROM_LOAD( "gt668p3a", 0x8000, 0x8000, CRC(81c4163d) SHA1(f533bddfd862df9413128742cfb0a7d2b6b7a122) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghosta )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt668p3c", 0x0000, 0x8000, CRC(198dab5e) SHA1(9c90cd7581badfac64662dbf0f6d3b3f300aba63) )
	ROM_LOAD( "gt668p3a", 0x8000, 0x8000, CRC(81c4163d) SHA1(f533bddfd862df9413128742cfb0a7d2b6b7a122) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt668p3d", 0x0000, 0x8000, CRC(92dcb4d7) SHA1(bc9b15a8f3cf2e3fea1407557d68d3bc8f0558e9) )
	ROM_LOAD( "gt668p3a", 0x8000, 0x8000, CRC(81c4163d) SHA1(f533bddfd862df9413128742cfb0a7d2b6b7a122) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt668p3e", 0x0000, 0x8000, CRC(1f9870d1) SHA1(5e20a165800871bdb68fb71e65a4fea3fb6312d4) )
	ROM_LOAD( "gt668p3a", 0x8000, 0x8000, CRC(81c4163d) SHA1(f533bddfd862df9413128742cfb0a7d2b6b7a122) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt635p2f", 0x0000, 0x8000, CRC(b7962429) SHA1(1b2f75e99462433ea96b3128ee196b7a0d651099) )
	ROM_LOAD( "gt635p2a", 0x8000, 0x8000, CRC(8dfc342e) SHA1(528f0a6efa77bff359c14fce6bcf566c0475087b) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghoste )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt635p2g", 0x0000, 0x8000, CRC(3ad2e02f) SHA1(6a199df69be07fa45c71830c8b226a0a3d7505c6) )
	ROM_LOAD( "gt635p2a", 0x8000, 0x8000, CRC(8dfc342e) SHA1(528f0a6efa77bff359c14fce6bcf566c0475087b) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt635p2h", 0x0000, 0x8000, CRC(b183ffa6) SHA1(555ed997f9999220ec7cbd9b9634ae2bd8474c85) )
	ROM_LOAD( "gt635p2a", 0x8000, 0x8000, CRC(8dfc342e) SHA1(528f0a6efa77bff359c14fce6bcf566c0475087b) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt635p2i", 0x0000, 0x8000, CRC(3cc73ba0) SHA1(a05db4b42f479e4059547f5e38f06f28169d25a8) )
	ROM_LOAD( "gt635p2a", 0x8000, 0x8000, CRC(8dfc342e) SHA1(528f0a6efa77bff359c14fce6bcf566c0475087b) )
	SP_GHOST_SOUND
ROM_END


ROM_START( sp_ghosth )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1b", 0x0000, 0x8000, CRC(9acce844) SHA1(5dd96dcc68b0b19710e099504c6d853d6bbcbbe6) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghosti )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1c", 0x0000, 0x8000, CRC(17882c42) SHA1(cc817665e7c167a7851d2495b1c24ded1ce687f3) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1f", 0x0000, 0x8000, CRC(d6eb299d) SHA1(95d160bff6c3f61652d045bb8e88f26edd20f722) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1g", 0x0000, 0x8000, CRC(5bafed9b) SHA1(2b05dc21c27e0bb9dc95442dafa7808d05c059df) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1h", 0x0000, 0x8000, CRC(d0fef212) SHA1(86e10dcb2bc15732008b9d29ddea0b3449b26dbb) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1i", 0x0000, 0x8000, CRC(5dba3614) SHA1(6eb34d4ac94c90190101bcc785df75c52da6059e) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1j", 0x0000, 0x8000, CRC(42b15d01) SHA1(624bcce8ee77467b246c403e4d6a35d61afcb643) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghosto )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1k", 0x0000, 0x8000, CRC(cff59907) SHA1(4e76374bfa3f766d4e920a751a834ed4c1b4ca91) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1n", 0x0000, 0x8000, CRC(0e969cd8) SHA1(1374fcf0bbc76edec7a5931f4b35c09067d0e61e) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostq )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1p", 0x0000, 0x8000, CRC(83d258de) SHA1(2f20c4b1e9c0cecf5d1abe8eaf7924665203cc2d) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghostr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1q", 0x0000, 0x8000, CRC(08834757) SHA1(7de63d0afd3407a7163eed34fbb10bc585ea7a25) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END

ROM_START( sp_ghosts )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt670p1r", 0x0000, 0x8000, CRC(85c78351) SHA1(0bc3e9759684983ff700c21c510f86c5c1b42782) )
	ROM_LOAD( "gt670p1a", 0x8000, 0x8000, CRC(4916a2b1) SHA1(da58289e1f46468e42822f2fa4a9f3af9223d278) )
	SP_GHOST_SOUND
ROM_END


#define SP_GLOBE_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_globe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt585p3b", 0x0000, 0x8000, CRC(dee01228) SHA1(a8994c73bad667c8eace40db802e57e839b2d769) )
	ROM_LOAD( "gt585p3a", 0x8000, 0x8000, CRC(5e6547a9) SHA1(79ba6b576cc4c5ada276041cfa8e3b7fa0baf6f3) )
	SP_GLOBE_SOUND
ROM_END

ROM_START( sp_globea )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt585p3c", 0x0000, 0x8000, CRC(53a4d62e) SHA1(1d347a4661f17e1602023f9877f6c9ac3d82be0e) )
	ROM_LOAD( "gt585p3a", 0x8000, 0x8000, CRC(5e6547a9) SHA1(79ba6b576cc4c5ada276041cfa8e3b7fa0baf6f3) )
	SP_GLOBE_SOUND
ROM_END

ROM_START( sp_globeb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt585p3d", 0x0000, 0x8000, CRC(d8f5c9a7) SHA1(2b783a373175c9e72ccd1081c97f0463fb677ddd) )
	ROM_LOAD( "gt585p3a", 0x8000, 0x8000, CRC(5e6547a9) SHA1(79ba6b576cc4c5ada276041cfa8e3b7fa0baf6f3) )
	SP_GLOBE_SOUND
ROM_END

ROM_START( sp_globec )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt585p3e", 0x0000, 0x8000, CRC(55b10da1) SHA1(0715d2a3b32ccdfaf77d37d45ee4bc5702703142) )
	ROM_LOAD( "gt585p3a", 0x8000, 0x8000, CRC(5e6547a9) SHA1(79ba6b576cc4c5ada276041cfa8e3b7fa0baf6f3) )
	SP_GLOBE_SOUND
ROM_END

ROM_START( sp_globed )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt585p3f", 0x0000, 0x8000, CRC(92c7d3f1) SHA1(11c3d511ef164a8ce115e9bd6d35d4711c155977) )
	ROM_LOAD( "gt585p3a", 0x8000, 0x8000, CRC(5e6547a9) SHA1(79ba6b576cc4c5ada276041cfa8e3b7fa0baf6f3) )
	SP_GLOBE_SOUND
ROM_END

ROM_START( sp_globee )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt585p3g", 0x0000, 0x8000, CRC(1f8317f7) SHA1(e8a6ada2f9d37524edd4db64fcde328f73c93030) )
	ROM_LOAD( "gt585p3a", 0x8000, 0x8000, CRC(5e6547a9) SHA1(79ba6b576cc4c5ada276041cfa8e3b7fa0baf6f3) )
	SP_GLOBE_SOUND
ROM_END

ROM_START( sp_globef )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt585p3h", 0x0000, 0x8000, CRC(94d2087e) SHA1(3b0a1294ef4c5f6ff9e8b9fb10940bae7b54e753) )
	ROM_LOAD( "gt585p3a", 0x8000, 0x8000, CRC(5e6547a9) SHA1(79ba6b576cc4c5ada276041cfa8e3b7fa0baf6f3) )
	SP_GLOBE_SOUND
ROM_END

ROM_START( sp_globeg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gt585p3i", 0x0000, 0x8000, CRC(1996cc78) SHA1(01c1ee2d26a670a0847adf36f0c2200336a772cb) )
	ROM_LOAD( "gt585p3a", 0x8000, 0x8000, CRC(5e6547a9) SHA1(79ba6b576cc4c5ada276041cfa8e3b7fa0baf6f3) )
	SP_GLOBE_SOUND
ROM_END


#define SP_GOL_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_gol )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gl706p04.bin", 0x00000, 0x10000, CRC(01a48714) SHA1(0a48cfad05905450aa2a7c9dc22f937377894ff0) ) // merged rom
	SP_GOL_SOUND
ROM_END


#define SP_GOLDA_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_golda )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ga701p8b", 0x0000, 0x8000, CRC(68701724) SHA1(7499ac703d03b6a028eb026dffdf6bfc6197f2e7) )
	ROM_LOAD( "ga701p8a", 0x8000, 0x8000, CRC(3ec1730a) SHA1(d097eee7d9cef0af174ebc89e244295fabc83612) )
	SP_GOLDA_SOUND
ROM_END

ROM_START( sp_goldaa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ga701p8c", 0x0000, 0x8000, CRC(e534d322) SHA1(d362bb48894d8a34126bff5890b83cadf436b9f8) )
	ROM_LOAD( "ga701p8a", 0x8000, 0x8000, CRC(3ec1730a) SHA1(d097eee7d9cef0af174ebc89e244295fabc83612) )
	SP_GOLDA_SOUND
ROM_END




#define SP_GOLDS_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_golds )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "020p3-0b.bin", 0x0000, 0x8000, CRC(12868efb) SHA1(1e7129127bb30d10e817a7906963a80ab986972a) )
	ROM_LOAD( "020p3-0a.bin", 0x8000, 0x8000, CRC(276b62d4) SHA1(0d733b38130d2191fdeb1ae8b0f489005ce4ec4e) )
	SP_GOLDS_SOUND
ROM_END


ROM_START( sp_goldsa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gs20nav5.2", 0x0000, 0x8000, CRC(f3702ab5) SHA1(ce46318b1ab83b4be3deb141058b14c7a1b9b411) )
	ROM_LOAD( "gs20.1", 0x8000, 0x8000, CRC(5b2911d3) SHA1(9bac67b8da16c241e5960cda21751049eb293362) )
	SP_GOLDS_SOUND
ROM_END

ROM_START( sp_goldsb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "gs20nav8.2", 0x0000, 0x8000, CRC(99fe1947) SHA1(81003bef2fc4e9d6b63b0d2839985ffb69a10d5f) )
	ROM_LOAD( "gs20.1", 0x8000, 0x8000, CRC(5b2911d3) SHA1(9bac67b8da16c241e5960cda21751049eb293362) )
	SP_GOLDS_SOUND
ROM_END

ROM_START( sp_goldsc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "021p3-0b.bin", 0x0000, 0x8000, CRC(4c86f4bc) SHA1(d03d1c938ffc9ca140a997ca45956036632b9afb) )
	ROM_LOAD( "021p3-0a.bin", 0x8000, 0x8000, CRC(dc792e10) SHA1(ecbd66fe481785b0b6125c4b8d379f8710ed4320) )
	SP_GOLDS_SOUND
ROM_END

ROM_START( sp_goldsd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "021p3-0f.bin", 0x0000, 0x8000, CRC(30ae8972) SHA1(aa85e174e93e97b0d23ae6cba031726a5e99547d) )
	ROM_LOAD( "021p3-0a.bin", 0x8000, 0x8000, CRC(dc792e10) SHA1(ecbd66fe481785b0b6125c4b8d379f8710ed4320) )
	SP_GOLDS_SOUND
ROM_END

ROM_START( sp_goldse )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "724p2-0b.bin", 0x0000, 0x8000, CRC(39c3a458) SHA1(6fe06283e8d942c3fa851a29c069dbcf4a9f07a5) )
	ROM_LOAD( "724p2-0a.bin", 0x8000, 0x8000, CRC(26c9e283) SHA1(7b817b89ff751c068de4c76ee1ddb68183bca5c8) )
	SP_GOLDS_SOUND
ROM_END

ROM_START( sp_goldsf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "724p2-0z.bin", 0x0000, 0x8000, CRC(1219607f) SHA1(c5b4b92a237884df5cc4ca5d5563c9b66b183fa5) )
	ROM_LOAD( "724p2-0a.bin", 0x8000, 0x8000, CRC(26c9e283) SHA1(7b817b89ff751c068de4c76ee1ddb68183bca5c8) )
	SP_GOLDS_SOUND
ROM_END

ROM_START( sp_goldsg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0021rp30.bin", 0x00000, 0x10000, CRC(3228013a) SHA1(dcf41ddd3003804062cde928bcb5bc409fa66c75) )
	SP_GOLDS_SOUND
ROM_END

ROM_START( sp_goldsh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0022rp30.bin", 0x00000, 0x10000, CRC(28b80866) SHA1(9f8af3d5cb6a03cba5ba37f2177390eef2950eb4) )
	SP_GOLDS_SOUND
ROM_END



#define SP_GOLDT_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_goldt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "729p2-0b.bin", 0x0000, 0x8000, CRC(a29a7084) SHA1(2a5ac792fd5a7514b74063475ca9419d03f17959) )
	ROM_LOAD( "729p2-0a.bin", 0x8000, 0x8000, CRC(ab402f5c) SHA1(88c92faffa4892b7e5c386d50c2eda4507710707) )
	SP_GOLDT_SOUND
ROM_END


#define SP_HERE_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_here )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hw674d8b", 0x0000, 0x8000, CRC(f3f83196) SHA1(4bdca9380632ae3be6b842e24df7597a9f6d073c) )
	ROM_LOAD( "hw674d8a", 0x8000, 0x8000, CRC(dd706eec) SHA1(7c7749a9879421348f4dd8bd574d7922dfd2a3a7) )
	SP_HERE_SOUND
ROM_END

ROM_START( sp_herea )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hw674d8c", 0x0000, 0x8000, CRC(7ebcf590) SHA1(0516e596028503870f3095b28e0fbe08bf8fde23) )
	ROM_LOAD( "hw674d8a", 0x8000, 0x8000, CRC(dd706eec) SHA1(7c7749a9879421348f4dd8bd574d7922dfd2a3a7) )
	SP_HERE_SOUND
ROM_END

ROM_START( sp_hereb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hw674d8d", 0x0000, 0x8000, CRC(f5edea19) SHA1(2423e1ed49da5fea39e581d4e6318b8357bf75b2) )
	ROM_LOAD( "hw674d8a", 0x8000, 0x8000, CRC(dd706eec) SHA1(7c7749a9879421348f4dd8bd574d7922dfd2a3a7) )
	SP_HERE_SOUND
ROM_END

ROM_START( sp_herec )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hw674d8e", 0x0000, 0x8000, CRC(78a92e1f) SHA1(986086f631b0cfdec204e98fa56a106b37669fc6) )
	ROM_LOAD( "hw674d8a", 0x8000, 0x8000, CRC(dd706eec) SHA1(7c7749a9879421348f4dd8bd574d7922dfd2a3a7) )
	SP_HERE_SOUND
ROM_END

ROM_START( sp_hered )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hw674d8f", 0x0000, 0x8000, CRC(bfdff04f) SHA1(a00e440bcbffcacd40e2119b6c0e5a403bd3a20b) )
	ROM_LOAD( "hw674d8a", 0x8000, 0x8000, CRC(dd706eec) SHA1(7c7749a9879421348f4dd8bd574d7922dfd2a3a7) )
	SP_HERE_SOUND
ROM_END

ROM_START( sp_heree )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hw674d8g", 0x0000, 0x8000, CRC(329b3449) SHA1(b292363422660cd18d785935e9bb0dcb4f057af9) )
	ROM_LOAD( "hw674d8a", 0x8000, 0x8000, CRC(dd706eec) SHA1(7c7749a9879421348f4dd8bd574d7922dfd2a3a7) )
	SP_HERE_SOUND
ROM_END

ROM_START( sp_heref )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hw674d8h", 0x0000, 0x8000, CRC(b9ca2bc0) SHA1(0887cd3d842fecac3730e0fdebca9fcc6a7d3246) )
	ROM_LOAD( "hw674d8a", 0x8000, 0x8000, CRC(dd706eec) SHA1(7c7749a9879421348f4dd8bd574d7922dfd2a3a7) )
	SP_HERE_SOUND
ROM_END

ROM_START( sp_hereg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hw674d8i", 0x0000, 0x8000, CRC(348eefc6) SHA1(2e61b6dc31c7ec739fc994d7389e274337a1f340) )
	ROM_LOAD( "hw674d8a", 0x8000, 0x8000, CRC(dd706eec) SHA1(7c7749a9879421348f4dd8bd574d7922dfd2a3a7) )
	SP_HERE_SOUND
ROM_END


#define SP_HOLID_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_holid )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hc669p3b", 0x0000, 0x8000, CRC(c703c949) SHA1(6a97a5a93f811054887feb3bc822f88ddd3d217b) )
	ROM_LOAD( "hc669p3a", 0x8000, 0x8000, CRC(2603d446) SHA1(2f00b9c8a085bf914a7df1c87d593ad6338f4236) )
	SP_HOLID_SOUND
ROM_END

ROM_START( sp_holida )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hc669p3c", 0x0000, 0x8000, CRC(4a470d4f) SHA1(b9566b617579a9deaa9b64f3b72b522ccbd8a2d3) )
	ROM_LOAD( "hc669p3a", 0x8000, 0x8000, CRC(2603d446) SHA1(2f00b9c8a085bf914a7df1c87d593ad6338f4236) )
	SP_HOLID_SOUND
ROM_END


#define SP_LOTTO_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_lotto )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sp705p14.bin", 0x00000, 0x10000, CRC(46a8a503) SHA1(39d40c6d34ec1879b037d13b47e648ae09f345d8) ) // merged rom
	SP_LOTTO_SOUND
ROM_END


#define SP_MAGMO_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_magmo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "032p0-3b.bin", 0x0000, 0x8000, CRC(8d22c9c5) SHA1(0126de26e91bbbd80a9c799231b57b1252d862a0) )
	ROM_LOAD( "032p0-3a.bin", 0x8000, 0x8000, CRC(1143184c) SHA1(5cdabdb3e3b67338c6c847993cb5dc78c2bf3ffe) )
	SP_MAGMO_SOUND
ROM_END

ROM_START( sp_magmoa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "032p0-3f.bin", 0x0000, 0x8000, CRC(c105081c) SHA1(ca559c6fb1eafcf84e35e23bfebc2d09a2b5cc3f) )
	ROM_LOAD( "032p0-3a.bin", 0x8000, 0x8000, CRC(1143184c) SHA1(5cdabdb3e3b67338c6c847993cb5dc78c2bf3ffe) )
	SP_MAGMO_SOUND
ROM_END

ROM_START( sp_magmob )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "032p0-3g.bin", 0x0000, 0x8000, CRC(4c41cc1a) SHA1(79c97b41578f2d4b8909fd43aa8741c41ef51b43) )
	ROM_LOAD( "032p0-3a.bin", 0x8000, 0x8000, CRC(1143184c) SHA1(5cdabdb3e3b67338c6c847993cb5dc78c2bf3ffe) )
	SP_MAGMO_SOUND
ROM_END

ROM_START( sp_magmoc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "032p0-3h.bin", 0x0000, 0x8000, CRC(c710d393) SHA1(8cbcd45aaa55e4d613d61950790462976d33690a) )
	ROM_LOAD( "032p0-3a.bin", 0x8000, 0x8000, CRC(1143184c) SHA1(5cdabdb3e3b67338c6c847993cb5dc78c2bf3ffe) )
	SP_MAGMO_SOUND
ROM_END

ROM_START( sp_magmod )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "032p0-3i.bin", 0x0000, 0x8000, CRC(4a541795) SHA1(2d6258e3f0e18fdc07c484ff1c2f596b6c6a0377) )
	ROM_LOAD( "032p0-3a.bin", 0x8000, 0x8000, CRC(1143184c) SHA1(5cdabdb3e3b67338c6c847993cb5dc78c2bf3ffe) )
	SP_MAGMO_SOUND
ROM_END


#define SP_MEGMO_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */


ROM_START( sp_megmo )
	ROM_REGION( 0x80000, "maincpu", 0 ) // obtained by splitting a merged rom
	ROM_LOAD( "0004p5_.bin", 0x0000, 0x8000, CRC(348fc072) SHA1(56086caf0b4900d170af26b8165e2df309048d58) )
	ROM_LOAD( "0004p5a.bin", 0x8000, 0x8000, CRC(48554d64) SHA1(308efa885bad564c098406e9d71a4031ceaaf369) )
	SP_MEGMO_SOUND
ROM_END

ROM_START( sp_megmoa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0004p5b.bin", 0x0000, 0x8000, CRC(7ebdda24) SHA1(ba6ba5079cab1fd27cd40c3bd200df79b2c868b2) )
	ROM_LOAD( "0004p5a.bin", 0x8000, 0x8000, CRC(48554d64) SHA1(308efa885bad564c098406e9d71a4031ceaaf369) )
	SP_MEGMO_SOUND
ROM_END

ROM_START( sp_megmob )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0004p5c.bin", 0x0000, 0x8000, CRC(4c995e81) SHA1(34001cb985f35d7e27f385aa7b829e454a1d5b0c) )
	ROM_LOAD( "0004p5a.bin", 0x8000, 0x8000, CRC(48554d64) SHA1(308efa885bad564c098406e9d71a4031ceaaf369) )
	SP_MEGMO_SOUND
ROM_END

ROM_START( sp_megmoc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0004p5f.bin", 0x0000, 0x8000, CRC(8dfa5b5e) SHA1(f0cf8b6e5047a3ed7f5f88c49ee97aff9c6c0e44) )
	ROM_LOAD( "0004p5a.bin", 0x8000, 0x8000, CRC(48554d64) SHA1(308efa885bad564c098406e9d71a4031ceaaf369) )
	SP_MEGMO_SOUND
ROM_END

ROM_START( sp_megmod )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0004p5g.bin", 0x0000, 0x8000, CRC(00be9f58) SHA1(fe7f32e065d642c197c791f84d8b1b4ee6bf874d) )
	ROM_LOAD( "0004p5a.bin", 0x8000, 0x8000, CRC(48554d64) SHA1(308efa885bad564c098406e9d71a4031ceaaf369) )
	SP_MEGMO_SOUND
ROM_END

ROM_START( sp_megmoe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0004p5h.bin", 0x0000, 0x8000, CRC(8bef80d1) SHA1(d283cc9288769de0fa461cd598a00d5a298a5079) )
	ROM_LOAD( "0004p5a.bin", 0x8000, 0x8000, CRC(48554d64) SHA1(308efa885bad564c098406e9d71a4031ceaaf369) )
	SP_MEGMO_SOUND
ROM_END

ROM_START( sp_megmof )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0004p5i.bin", 0x0000, 0x8000, CRC(06ab44d7) SHA1(5c4e6654dc3c61bb1aedf939b863dbe03bb6c3aa) )
	ROM_LOAD( "0004p5a.bin", 0x8000, 0x8000, CRC(48554d64) SHA1(308efa885bad564c098406e9d71a4031ceaaf369) )
	SP_MEGMO_SOUND
ROM_END

ROM_START( sp_megmog )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0004p6.bin", 0x00000, 0x10000, CRC(fcfa212d) SHA1(968cc358450d85e6fd4e3086347be7d34d68e012) ) // merged rom
	SP_MEGMO_SOUND
ROM_END



#define SP_MONMA_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_monma )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "006p7b.bin", 0x0000, 0x8000, CRC(81ed5a8e) SHA1(8165f8a40a1fa7ca2afb3d2def37ea88f219e231) )
	ROM_LOAD( "006p7a.bin", 0x8000, 0x8000, CRC(0f44d014) SHA1(1bf4d594226581f5fd9cd749eae92abc49571cdf) )
	SP_MONMA_SOUND
ROM_END

ROM_START( sp_monmaa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "006p7f.bin", 0x0000, 0x8000, CRC(cdca9b57) SHA1(f703d765c3fd561646b1e03c4ebc06f3ea2d2f3e) )
	ROM_LOAD( "006p7a.bin", 0x8000, 0x8000, CRC(0f44d014) SHA1(1bf4d594226581f5fd9cd749eae92abc49571cdf) )
	SP_MONMA_SOUND
ROM_END

ROM_START( sp_monmab )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "006p7g.bin", 0x0000, 0x8000, CRC(408e5f51) SHA1(d4d81a83f640600650b52501a86e54b7eb3990b3) )
	ROM_LOAD( "006p7a.bin", 0x8000, 0x8000, CRC(0f44d014) SHA1(1bf4d594226581f5fd9cd749eae92abc49571cdf) )
	SP_MONMA_SOUND
ROM_END

ROM_START( sp_monmac )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "006p7h.bin", 0x0000, 0x8000, CRC(cbdf40d8) SHA1(37914d61fdb13df0252c0ef35a5bcdc4a2954293) )
	ROM_LOAD( "006p7a.bin", 0x8000, 0x8000, CRC(0f44d014) SHA1(1bf4d594226581f5fd9cd749eae92abc49571cdf) )
	SP_MONMA_SOUND
ROM_END

ROM_START( sp_monmad )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "006p7i.bin", 0x0000, 0x8000, CRC(469b84de) SHA1(ea9aef5c98d2a4251cd74df5d15b05d48bf677ab) )
	ROM_LOAD( "006p7a.bin", 0x8000, 0x8000, CRC(0f44d014) SHA1(1bf4d594226581f5fd9cd749eae92abc49571cdf) )
	SP_MONMA_SOUND
ROM_END




#define SP_MONMO_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */


ROM_START( sp_monmo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mm98d17b", 0x0000, 0x8000, CRC(2aa32ecd) SHA1(0f96c9bc362a68afa3e2d2bc082ec402bea9f027) )
	ROM_LOAD( "mm98d17a", 0x8000, 0x8000, CRC(a55a337a) SHA1(add902b2b3ab1d8e96f6d4855f8ea37e45b547c6) )
	SP_MONMO_SOUND
ROM_END

ROM_START( sp_monmoa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mm98d17c", 0x0000, 0x8000, CRC(a7e7eacb) SHA1(779a28bf03058b0290a74f7965541b665f6488b1) )
	ROM_LOAD( "mm98d17a", 0x8000, 0x8000, CRC(a55a337a) SHA1(add902b2b3ab1d8e96f6d4855f8ea37e45b547c6) )
	SP_MONMO_SOUND
ROM_END

ROM_START( sp_monmob )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mm98d17d", 0x0000, 0x8000, CRC(2cb6f542) SHA1(2386c79d5fda1b7048da8335bb03a64592972f96) )
	ROM_LOAD( "mm98d17a", 0x8000, 0x8000, CRC(a55a337a) SHA1(add902b2b3ab1d8e96f6d4855f8ea37e45b547c6) )
	SP_MONMO_SOUND
ROM_END

ROM_START( sp_monmoc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mm98d17e", 0x0000, 0x8000, CRC(a1f23144) SHA1(a8671971191f68c270f07592c85271fdd6e4d7f4) )
	ROM_LOAD( "mm98d17a", 0x8000, 0x8000, CRC(a55a337a) SHA1(add902b2b3ab1d8e96f6d4855f8ea37e45b547c6) )
	SP_MONMO_SOUND
ROM_END

ROM_START( sp_monmod )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mm98d17f", 0x0000, 0x8000, CRC(6684ef14) SHA1(0e19bcae57811aa221e3170cc321201732ba2547) )
	ROM_LOAD( "mm98d17a", 0x8000, 0x8000, CRC(a55a337a) SHA1(add902b2b3ab1d8e96f6d4855f8ea37e45b547c6) )
	SP_MONMO_SOUND
ROM_END

ROM_START( sp_monmoe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mm98d17g", 0x0000, 0x8000, CRC(ebc02b12) SHA1(dfd9703d56ddedeccacdb292fc518103a7781829) )
	ROM_LOAD( "mm98d17a", 0x8000, 0x8000, CRC(a55a337a) SHA1(add902b2b3ab1d8e96f6d4855f8ea37e45b547c6) )
	SP_MONMO_SOUND
ROM_END

ROM_START( sp_monmof )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mm98d17h", 0x0000, 0x8000, CRC(6091349b) SHA1(070281fe81518933e3c91dd97c64c5fd6dff4349) )
	ROM_LOAD( "mm98d17a", 0x8000, 0x8000, CRC(a55a337a) SHA1(add902b2b3ab1d8e96f6d4855f8ea37e45b547c6) )
	SP_MONMO_SOUND
ROM_END

ROM_START( sp_monmog )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "mm98d17i", 0x0000, 0x8000, CRC(edd5f09d) SHA1(5ff68fdd09fca890376c086c526915c8a0d9221a) )
	ROM_LOAD( "mm98d17a", 0x8000, 0x8000, CRC(a55a337a) SHA1(add902b2b3ab1d8e96f6d4855f8ea37e45b547c6) )
	SP_MONMO_SOUND
ROM_END


#define SP_NUDEX_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_nudex )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9398.bin", 0x0000, 0x8000, CRC(76f26a46) SHA1(a94c083db401ba732cb4a97cc276f296b495354d) )
	ROM_LOAD( "9397.bin", 0x8000, 0x8000, CRC(1214fa3b) SHA1(a446d184b32978c35c5d7635d5ac6b8b4afbcf33) )
	SP_NUDEX_SOUND
ROM_END

ROM_START( sp_nudexa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "9399.bin", 0x0000, 0x8000, CRC(72a26b8b) SHA1(ad3296ecc35f6deb28978cd045aaa02313a101c8) )
	ROM_LOAD( "9397.bin", 0x8000, 0x8000, CRC(1214fa3b) SHA1(a446d184b32978c35c5d7635d5ac6b8b4afbcf33) )
	SP_NUDEX_SOUND
ROM_END



#define SP_ONBOX_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_onbox )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "016p4-0b.bin", 0x0000, 0x8000, CRC(95376cf9) SHA1(bf60f45c3314147ca6b608ee46cfdf2e92d7073d) )
	ROM_LOAD( "016p4-0a.bin", 0x8000, 0x8000, CRC(3b17be64) SHA1(8a8338a8271f1245050c3dd21d0b9c197c85d0b0) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "016p4-0c.bin", 0x0000, 0x8000, CRC(1873a8ff) SHA1(c429334855456231c0afe14f3a1e230835591a4f) )
	ROM_LOAD( "016p4-0a.bin", 0x8000, 0x8000, CRC(3b17be64) SHA1(8a8338a8271f1245050c3dd21d0b9c197c85d0b0) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "016p4-0f.bin", 0x0000, 0x8000, CRC(d910ad20) SHA1(1c02cd267d4e2e60567a6beebacc0a577830b682) )
	ROM_LOAD( "016p4-0a.bin", 0x8000, 0x8000, CRC(3b17be64) SHA1(8a8338a8271f1245050c3dd21d0b9c197c85d0b0) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "016p4-0g.bin", 0x0000, 0x8000, CRC(54546926) SHA1(216aee054bcd5db3fd93cf1e300b860b84bd49e9) )
	ROM_LOAD( "016p4-0a.bin", 0x8000, 0x8000, CRC(3b17be64) SHA1(8a8338a8271f1245050c3dd21d0b9c197c85d0b0) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "016p4-0h.bin", 0x0000, 0x8000, CRC(df0576af) SHA1(16bf9edbd2afffb0d34365f4e3077b56df52796a) )
	ROM_LOAD( "016p4-0a.bin", 0x8000, 0x8000, CRC(3b17be64) SHA1(8a8338a8271f1245050c3dd21d0b9c197c85d0b0) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "016p4-0i.bin", 0x0000, 0x8000, CRC(5241b2a9) SHA1(7b8437d3a5d8ea7c3b5260ff0be9284f9c02b414) )
	ROM_LOAD( "016p4-0a.bin", 0x8000, 0x8000, CRC(3b17be64) SHA1(8a8338a8271f1245050c3dd21d0b9c197c85d0b0) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "709p5-0b.bin", 0x0000, 0x8000, CRC(0b87a83d) SHA1(4b519977bab90143cd8cd890120fba36bfc20f47) )
	ROM_LOAD( "709p5-0a.bin", 0x8000, 0x8000, CRC(a9204087) SHA1(99e2e6ba716369f69526b82ad88b22698ed6e2cc) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "709p5-0c.bin", 0x0000, 0x8000, CRC(86c36c3b) SHA1(7f21177bb3e244aaa66752cc398262a86cdf41a4) )
	ROM_LOAD( "709p5-0a.bin", 0x8000, 0x8000, CRC(a9204087) SHA1(99e2e6ba716369f69526b82ad88b22698ed6e2cc) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "709p5-0f.bin", 0x0000, 0x8000, CRC(47a069e4) SHA1(b99c4b740fea418be7ef51ec7c06373bf9bd2cc8) )
	ROM_LOAD( "709p5-0a.bin", 0x8000, 0x8000, CRC(a9204087) SHA1(99e2e6ba716369f69526b82ad88b22698ed6e2cc) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "709p5-0g.bin", 0x0000, 0x8000, CRC(cae4ade2) SHA1(48786c08a6e8040ee90e340eba9c3e4b62a96e7c) )
	ROM_LOAD( "709p5-0a.bin", 0x8000, 0x8000, CRC(a9204087) SHA1(99e2e6ba716369f69526b82ad88b22698ed6e2cc) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "709p5-0h.bin", 0x0000, 0x8000, CRC(41b5b26b) SHA1(ec7ba225ffc77417c6375d840e9bec40ce6387e9) )
	ROM_LOAD( "709p5-0a.bin", 0x8000, 0x8000, CRC(a9204087) SHA1(99e2e6ba716369f69526b82ad88b22698ed6e2cc) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "709p5-0i.bin", 0x0000, 0x8000, CRC(ccf1766d) SHA1(9557387944e27415cca67e138a9606e626498fa0) )
	ROM_LOAD( "709p5-0a.bin", 0x8000, 0x8000, CRC(a9204087) SHA1(99e2e6ba716369f69526b82ad88b22698ed6e2cc) )
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "prom2.bin", 0x0000, 0x8000, CRC(54294a24) SHA1(6ad55fc6d40cb9cb2ef785f4bd99a6c91899cdd5) )
	ROM_LOAD( "709p5-0a.bin", 0x8000, 0x8000, CRC(a9204087) SHA1(99e2e6ba716369f69526b82ad88b22698ed6e2cc) ) // aka prom1.bin
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0016rp40.bin", 0x00000, 0x10000, CRC(591e3e24) SHA1(b147bfc19aa46eb3237b270d771031caaf978850) ) // merged rom
	SP_ONBOX_SOUND
ROM_END

ROM_START( sp_onboxn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0016rp50.bin", 0x00000, 0x10000, CRC(73c5c449) SHA1(453969ec5c4fe1ce8f118110106e31126a358cda) ) // merged rom
	SP_ONBOX_SOUND
ROM_END



#define SP_PISTE_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_piste )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0010p5b.bin", 0x0000, 0x8000, CRC(423edc62) SHA1(f0ab4b412d95475b3552c91c7e46bf328a51261c) )
	ROM_LOAD( "0010p5a.bin", 0x8000, 0x8000, CRC(4e4ef499) SHA1(2969e39f126571182decf8506775149b81741df3) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pistea )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0010p5c.bin", 0x0000, 0x8000, CRC(cf7a1864) SHA1(476b29e41792a22de32fcfc02c9a7e6aa2ed9008) )
	ROM_LOAD( "0010p5a.bin", 0x8000, 0x8000, CRC(4e4ef499) SHA1(2969e39f126571182decf8506775149b81741df3) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pisteb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0010p5f.bin", 0x0000, 0x8000, CRC(0e191dbb) SHA1(e19d261078992587475d08f5b1252efab34a7255) )
	ROM_LOAD( "0010p5a.bin", 0x8000, 0x8000, CRC(4e4ef499) SHA1(2969e39f126571182decf8506775149b81741df3) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pistec )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0010p5g.bin", 0x0000, 0x8000, CRC(835dd9bd) SHA1(0d9b8c7375720376ff13c3fe68d7af1fae7eae3b) )
	ROM_LOAD( "0010p5a.bin", 0x8000, 0x8000, CRC(4e4ef499) SHA1(2969e39f126571182decf8506775149b81741df3) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pisted )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0010p5h.bin", 0x0000, 0x8000, CRC(080cc634) SHA1(239837d932f9206bb0bd61510268d801ace11661) )
	ROM_LOAD( "0010p5a.bin", 0x8000, 0x8000, CRC(4e4ef499) SHA1(2969e39f126571182decf8506775149b81741df3) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pistee )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "0010p5i.bin", 0x0000, 0x8000, CRC(85480232) SHA1(6aeb9d7587909362400f2c628fdb49ad279f9579) )
	ROM_LOAD( "0010p5a.bin", 0x8000, 0x8000, CRC(4e4ef499) SHA1(2969e39f126571182decf8506775149b81741df3) )
	SP_PISTE_SOUND
ROM_END


ROM_START( sp_pistef )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "op710p1b", 0x0000, 0x8000, CRC(4710faa8) SHA1(8bfe50793d3cc79154cd90055d456d7617944de4) )
	ROM_LOAD( "op710p1a", 0x8000, 0x8000, CRC(64add8c6) SHA1(46cfa6f9e06a1c74f0230d4b9cc6c3f9d5167cfe) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pisteg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "op710p1c", 0x0000, 0x8000, CRC(ca543eae) SHA1(1e7fd9b1f3d8a55387dc885fe1011f50286ecaa3) )
	ROM_LOAD( "op710p1a", 0x8000, 0x8000, CRC(64add8c6) SHA1(46cfa6f9e06a1c74f0230d4b9cc6c3f9d5167cfe) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pisteh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "op710p1f", 0x0000, 0x8000, CRC(0b373b71) SHA1(c71c93d6cae52c37458f011f96972bfe38fba418) )
	ROM_LOAD( "op710p1a", 0x8000, 0x8000, CRC(64add8c6) SHA1(46cfa6f9e06a1c74f0230d4b9cc6c3f9d5167cfe) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pistei )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "op710p1g", 0x0000, 0x8000, CRC(8673ff77) SHA1(f5e27bed47c09f56a1a99b811681d8c1250ff10e) )
	ROM_LOAD( "op710p1a", 0x8000, 0x8000, CRC(64add8c6) SHA1(46cfa6f9e06a1c74f0230d4b9cc6c3f9d5167cfe) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pistej )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "op710p1h", 0x0000, 0x8000, CRC(0d22e0fe) SHA1(714b4d775022b173df8ae9c7f21def86aa007776) )
	ROM_LOAD( "op710p1a", 0x8000, 0x8000, CRC(64add8c6) SHA1(46cfa6f9e06a1c74f0230d4b9cc6c3f9d5167cfe) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pistek )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "op710p1i", 0x0000, 0x8000, CRC(806624f8) SHA1(39ed94f493bba316232375b6425376d31c922d8f) )
	ROM_LOAD( "op710p1a", 0x8000, 0x8000, CRC(64add8c6) SHA1(46cfa6f9e06a1c74f0230d4b9cc6c3f9d5167cfe) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pistel )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "op722p1f", 0x0000, 0x8000, CRC(e30f4c49) SHA1(fb842e5cab78b046fcf7a7098a720977a66d3674) )
	ROM_LOAD( "op722p1a", 0x8000, 0x8000, CRC(26cfaaa8) SHA1(6e5c3a4c22bccab0e84737f4e0c8d4fef9a189aa) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pistem )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "op722p1g", 0x0000, 0x8000, CRC(6e4b884f) SHA1(f2de0ab5aa57f0d0928d1f02f55a802637b878d9) )
	ROM_LOAD( "op722p1a", 0x8000, 0x8000, CRC(26cfaaa8) SHA1(6e5c3a4c22bccab0e84737f4e0c8d4fef9a189aa) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pisten )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "op722p1h", 0x0000, 0x8000, CRC(e51a97c6) SHA1(b54fb615daa003144ab57e1ea31a53baa20d607a) )
	ROM_LOAD( "op722p1a", 0x8000, 0x8000, CRC(26cfaaa8) SHA1(6e5c3a4c22bccab0e84737f4e0c8d4fef9a189aa) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pisteo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "op722p1i", 0x0000, 0x8000, CRC(685e53c0) SHA1(a876188e93d69b1953a52fa14c951cc082d13853) )
	ROM_LOAD( "op722p1a", 0x8000, 0x8000, CRC(26cfaaa8) SHA1(6e5c3a4c22bccab0e84737f4e0c8d4fef9a189aa) )
	SP_PISTE_SOUND
ROM_END

ROM_START( sp_pistep )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "otp10.bin", 0x00000, 0x10000, CRC(8a9e40df) SHA1(be1c8c5733d65dbdfefcaeb35050d82d59c75450) ) // merged rom
	SP_PISTE_SOUND
ROM_END



#define SP_POUND_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */


ROM_START( sp_pound )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp546p7b", 0x0000, 0x8000, CRC(277ade43) SHA1(bdaa665184a667165a3668dcb50e2d5bb82b57ee) )
	ROM_LOAD( "pp546p7a", 0x8000, 0x8000, CRC(bec729b0) SHA1(0a3e965a65d5d9df179d374b1397ce8c1ac65e2d) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_pounda )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp546p7c", 0x0000, 0x8000, CRC(aa3e1a45) SHA1(f26e138bb546219abd33595f4064b4a3e856a805) )
	ROM_LOAD( "pp546p7a", 0x8000, 0x8000, CRC(bec729b0) SHA1(0a3e965a65d5d9df179d374b1397ce8c1ac65e2d) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp546p7d", 0x0000, 0x8000, CRC(216f05cc) SHA1(4eb5e4408c2bd1d1742a955c66b1379cf639f5c4) )
	ROM_LOAD( "pp546p7a", 0x8000, 0x8000, CRC(bec729b0) SHA1(0a3e965a65d5d9df179d374b1397ce8c1ac65e2d) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp546p7e", 0x0000, 0x8000, CRC(ac2bc1ca) SHA1(baa7aba351d630f9808f70cc2e10f4b01abe6ec7) )
	ROM_LOAD( "pp546p7a", 0x8000, 0x8000, CRC(bec729b0) SHA1(0a3e965a65d5d9df179d374b1397ce8c1ac65e2d) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp546p7f", 0x0000, 0x8000, CRC(6b5d1f9a) SHA1(9b40d6f528d6148320df70a4ffd14a9518f34d2e) )
	ROM_LOAD( "pp546p7a", 0x8000, 0x8000, CRC(bec729b0) SHA1(0a3e965a65d5d9df179d374b1397ce8c1ac65e2d) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_pounde )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp546p7g", 0x0000, 0x8000, CRC(e619db9c) SHA1(ab4c76e9bc0adaa6f896bfcc9332947ce890a933) )
	ROM_LOAD( "pp546p7a", 0x8000, 0x8000, CRC(bec729b0) SHA1(0a3e965a65d5d9df179d374b1397ce8c1ac65e2d) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp546p7h", 0x0000, 0x8000, CRC(6d48c415) SHA1(82cfeec5ef58a16c2f77e64c21bd8c5db57c5983) )
	ROM_LOAD( "pp546p7a", 0x8000, 0x8000, CRC(bec729b0) SHA1(0a3e965a65d5d9df179d374b1397ce8c1ac65e2d) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp546p7i", 0x0000, 0x8000, CRC(e00c0013) SHA1(47ac1f49e6442ab205c40881448ed095aa6eee0e) )
	ROM_LOAD( "pp546p7a", 0x8000, 0x8000, CRC(bec729b0) SHA1(0a3e965a65d5d9df179d374b1397ce8c1ac65e2d) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp571p5b", 0x0000, 0x8000, CRC(08e95ad4) SHA1(12ee9a1d0047fcd6406f57bd9de15d523bf0b0cb) )
	ROM_LOAD( "pp571p5a", 0x8000, 0x8000, CRC(f6e92348) SHA1(749b22f4cfffe05824f1284dcdde246a232be4bd) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundi )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp571p5c", 0x0000, 0x8000, CRC(85ad9ed2) SHA1(b1e24e394055c973a68a74288c162162ec18cfdc) )
	ROM_LOAD( "pp571p5a", 0x8000, 0x8000, CRC(f6e92348) SHA1(749b22f4cfffe05824f1284dcdde246a232be4bd) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp571p5d", 0x0000, 0x8000, CRC(0efc815b) SHA1(e9f4931b4b1b674ecd7b60413eac732c0d5a13b6) )
	ROM_LOAD( "pp571p5a", 0x8000, 0x8000, CRC(f6e92348) SHA1(749b22f4cfffe05824f1284dcdde246a232be4bd) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp571p5e", 0x0000, 0x8000, CRC(83b8455d) SHA1(15781a3ba52f5e13f000d766fbd7d52285955658) )
	ROM_LOAD( "pp571p5a", 0x8000, 0x8000, CRC(f6e92348) SHA1(749b22f4cfffe05824f1284dcdde246a232be4bd) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp571p5f", 0x0000, 0x8000, CRC(44ce9b0d) SHA1(521689757d48d1dda5bf02351fb4c4335cca7c0f) )
	ROM_LOAD( "pp571p5a", 0x8000, 0x8000, CRC(f6e92348) SHA1(749b22f4cfffe05824f1284dcdde246a232be4bd) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp571p5g", 0x0000, 0x8000, CRC(c98a5f0b) SHA1(b6af772227b3847f08c8d602e00da78e9a2a11b4) )
	ROM_LOAD( "pp571p5a", 0x8000, 0x8000, CRC(f6e92348) SHA1(749b22f4cfffe05824f1284dcdde246a232be4bd) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp571p5h", 0x0000, 0x8000, CRC(42db4082) SHA1(0e3b5b46d76a2865e5b8a44169b1472500ad8312) )
	ROM_LOAD( "pp571p5a", 0x8000, 0x8000, CRC(f6e92348) SHA1(749b22f4cfffe05824f1284dcdde246a232be4bd) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp571p5i", 0x0000, 0x8000, CRC(cf9f8484) SHA1(4b92a11c710550ee1830f1392ad45f0221806724) )
	ROM_LOAD( "pp571p5a", 0x8000, 0x8000, CRC(f6e92348) SHA1(749b22f4cfffe05824f1284dcdde246a232be4bd) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp571p5z", 0x0000, 0x8000, CRC(1b607914) SHA1(d549c833610e9b49aebf4e20121ce72dfc62cd95) ) // aka 571 ppav all cash p5.2.bin
	ROM_LOAD( "pp571p5a", 0x8000, 0x8000, CRC(f6e92348) SHA1(749b22f4cfffe05824f1284dcdde246a232be4bd) ) // aka 571 pp p5.1.bin
ROM_END

ROM_START( sp_poundbwb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp624p9b", 0x0000, 0x8000, CRC(de9f5318) SHA1(a8c26de17be684ee8817587575af130002f04da5) )
	ROM_LOAD( "pp624p9a", 0x8000, 0x8000, CRC(3e8dd5af) SHA1(5ba1c5887d366403448e5b220eaf72cc6de7caf5) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundbwba )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp624p9c", 0x0000, 0x8000, CRC(53db971e) SHA1(a5e5235fc99a0f9916d3658a78fbef7c489336f0) )
	ROM_LOAD( "pp624p9a", 0x8000, 0x8000, CRC(3e8dd5af) SHA1(5ba1c5887d366403448e5b220eaf72cc6de7caf5) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundbwbb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp624p9d", 0x0000, 0x8000, CRC(522fff50) SHA1(0acf30ad1c4cd21ec2dfcd17d9961c3c31a5a4e5) )
	ROM_LOAD( "pp624p9a", 0x8000, 0x8000, CRC(3e8dd5af) SHA1(5ba1c5887d366403448e5b220eaf72cc6de7caf5) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundbwbc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp624p9e", 0x0000, 0x8000, CRC(df6b3b56) SHA1(87d73b548cd46044c430fdf9c40cd13c873bd8ad) )
	ROM_LOAD( "pp624p9a", 0x8000, 0x8000, CRC(3e8dd5af) SHA1(5ba1c5887d366403448e5b220eaf72cc6de7caf5) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundbwbd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp624p9f", 0x0000, 0x8000, CRC(ea8a476f) SHA1(bc6f79fd2bcc4765846c6c54563ae0c2a926178a) )
	ROM_LOAD( "pp624p9a", 0x8000, 0x8000, CRC(3e8dd5af) SHA1(5ba1c5887d366403448e5b220eaf72cc6de7caf5) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundbwbe )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp624p9g", 0x0000, 0x8000, CRC(67ce8369) SHA1(cb7d67df1d116fc0b9f28303f5897d0156015293) )
	ROM_LOAD( "pp624p9a", 0x8000, 0x8000, CRC(3e8dd5af) SHA1(5ba1c5887d366403448e5b220eaf72cc6de7caf5) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundbwbf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pp624p9h", 0x0000, 0x8000, CRC(663aeb27) SHA1(6703ceeb41b9670319caf74de2ce175b86303623) )
	ROM_LOAD( "pp624p9a", 0x8000, 0x8000, CRC(3e8dd5af) SHA1(5ba1c5887d366403448e5b220eaf72cc6de7caf5) )
	SP_POUND_SOUND
ROM_END

ROM_START( sp_poundbwbg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "p4p_1.bin", 0x0000, 0x8000, CRC(b2c1625e) SHA1(c992e54280bfe42a77686802faccfebb4db300a8) )
	ROM_LOAD( "p4p_2.bin", 0x8000, 0x8000, CRC(30f09a7d) SHA1(e00eeba15b8ed1f47a5d8f7ae1e59fcadbcbd838) )
	SP_POUND_SOUND
ROM_END

#define SP_PRZNA_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_przna )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pn614p1b", 0x0000, 0x8000, CRC(414a4c56) SHA1(506e143a3b9c57ee6301e9c69e5659c18bd2be4c) )
	ROM_LOAD( "pn614p1a", 0x8000, 0x8000, CRC(e47239d6) SHA1(5ce52fe4793f7e8092b97add33f51ecfd705f6a8) )
	SP_PRZNA_SOUND
ROM_END

ROM_START( sp_prznaa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pn614p1c", 0x0000, 0x8000, CRC(cc0e8850) SHA1(a347ebb92c8cc476b849dfb39b3829ed1418dc19) )
	ROM_LOAD( "pn614p1a", 0x8000, 0x8000, CRC(e47239d6) SHA1(5ce52fe4793f7e8092b97add33f51ecfd705f6a8) )
	SP_PRZNA_SOUND
ROM_END

ROM_START( sp_prznab )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pn614p1d", 0x0000, 0x8000, CRC(475f97d9) SHA1(77e0145ea6e9c3b4cc659daa653d6ce2b32193de) )
	ROM_LOAD( "pn614p1a", 0x8000, 0x8000, CRC(e47239d6) SHA1(5ce52fe4793f7e8092b97add33f51ecfd705f6a8) )
	SP_PRZNA_SOUND
ROM_END

ROM_START( sp_prznac )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pn614p1e", 0x0000, 0x8000, CRC(ca1b53df) SHA1(8e5f479e1c1932182c6f845e472fbd67c5d4ec49) )
	ROM_LOAD( "pn614p1a", 0x8000, 0x8000, CRC(e47239d6) SHA1(5ce52fe4793f7e8092b97add33f51ecfd705f6a8) )
	SP_PRZNA_SOUND
ROM_END

ROM_START( sp_prznad )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pn614p1f", 0x0000, 0x8000, CRC(0d6d8d8f) SHA1(3d0eb176f4fb9b44e7d213f3944353db78b82ba2) )
	ROM_LOAD( "pn614p1a", 0x8000, 0x8000, CRC(e47239d6) SHA1(5ce52fe4793f7e8092b97add33f51ecfd705f6a8) )
	SP_PRZNA_SOUND
ROM_END

ROM_START( sp_prznae )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pn614p1g", 0x0000, 0x8000, CRC(80294989) SHA1(60c40b2f7b45bd8e9d0ade69607b4f9fd8c449fa) )
	ROM_LOAD( "pn614p1a", 0x8000, 0x8000, CRC(e47239d6) SHA1(5ce52fe4793f7e8092b97add33f51ecfd705f6a8) )
	SP_PRZNA_SOUND
ROM_END

ROM_START( sp_prznaf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pn614p1h", 0x0000, 0x8000, CRC(0b785600) SHA1(3d72b4ed6965fc73e26b30a9b9b66a95c87b6cd5) )
	ROM_LOAD( "pn614p1a", 0x8000, 0x8000, CRC(e47239d6) SHA1(5ce52fe4793f7e8092b97add33f51ecfd705f6a8) )
	SP_PRZNA_SOUND
ROM_END

ROM_START( sp_prznag )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pn614p1i", 0x0000, 0x8000, CRC(863c9206) SHA1(f06152166bc1cc0fe4ee882dbb1c325e8a449915) )
	ROM_LOAD( "pn614p1a", 0x8000, 0x8000, CRC(e47239d6) SHA1(5ce52fe4793f7e8092b97add33f51ecfd705f6a8) )
	SP_PRZNA_SOUND
ROM_END


#define SP_ROAD_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_road )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "rh654p2b", 0x0000, 0x8000, CRC(05e052eb) SHA1(a69dd44bad0d83de3ba4f284e1cbff49ec0bbe3d) )
	ROM_LOAD( "rh654p2a", 0x8000, 0x8000, NO_DUMP )
	SP_ROAD_SOUND
ROM_END

#define SP_SKYLM_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_skylm )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sk617d7d.bin", 0x0000, 0x8000, CRC(6eb99fb3) SHA1(fe1decff1ed5a68117673bf4834eda9d7975c9c3) ) // was in a merged rom as 'sk617d7b' ?!
	ROM_LOAD( "sk617d7a.bin", 0x8000, 0x8000, CRC(68ce3a07) SHA1(1b0562b1273ac98ccf55a8679b796763d9ab66f9) ) // was named sk617d7c ?!
	SP_SKYLM_SOUND
ROM_END

ROM_START( sp_skylma )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sk617d7e.bin", 0x0000, 0x8000, CRC(e3fd5bb5) SHA1(0b35132423924029f571ae720ab3a5680d11b4c8) )
	ROM_LOAD( "sk617d7a.bin", 0x8000, 0x8000, CRC(68ce3a07) SHA1(1b0562b1273ac98ccf55a8679b796763d9ab66f9) ) // was named sk617d7c ?!
	SP_SKYLM_SOUND
ROM_END


#define SP_TKPIK_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_tkpik )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tp04d11b", 0x0000, 0x8000, CRC(20ebbeaa) SHA1(2e7d8ff60eb3a89990ec4f432eb625520bb793f2) )
	ROM_LOAD( "tp04d11a", 0x8000, 0x8000, CRC(a3a3a155) SHA1(b05005b324560b64bcba3ab53a66586650cae6ad) )
	SP_TKPIK_SOUND
ROM_END

ROM_START( sp_tkpika )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tp04d11c", 0x0000, 0x8000, CRC(adaf7aac) SHA1(b497dbfc4acec6c287e9dabef254dfb625534f18) )
	ROM_LOAD( "tp04d11a", 0x8000, 0x8000, CRC(a3a3a155) SHA1(b05005b324560b64bcba3ab53a66586650cae6ad) )
	SP_TKPIK_SOUND
ROM_END

ROM_START( sp_tkpikb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tp04d11d", 0x0000, 0x8000, CRC(26fe6525) SHA1(e087b8f9ba4b9d5984c381bd67aad2074027e9c4) )
	ROM_LOAD( "tp04d11a", 0x8000, 0x8000, CRC(a3a3a155) SHA1(b05005b324560b64bcba3ab53a66586650cae6ad) )
	SP_TKPIK_SOUND
ROM_END

ROM_START( sp_tkpikc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tp04d11e", 0x0000, 0x8000, CRC(abbaa123) SHA1(c1b9cbf833d80adf21c186c3fad64f090bc50b3a) )
	ROM_LOAD( "tp04d11a", 0x8000, 0x8000, CRC(a3a3a155) SHA1(b05005b324560b64bcba3ab53a66586650cae6ad) )
	SP_TKPIK_SOUND
ROM_END

ROM_START( sp_tkpikd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tp04d11f", 0x0000, 0x8000, CRC(6ccc7f73) SHA1(cdc193d099e95efed72ce2ff554ec482f15e42a6) )
	ROM_LOAD( "tp04d11a", 0x8000, 0x8000, CRC(a3a3a155) SHA1(b05005b324560b64bcba3ab53a66586650cae6ad) )
	SP_TKPIK_SOUND
ROM_END

ROM_START( sp_tkpike )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tp04d11g", 0x0000, 0x8000, CRC(e188bb75) SHA1(e70bdbf55a2571e41384c8c0dcf892c6d14a15e2) )
	ROM_LOAD( "tp04d11a", 0x8000, 0x8000, CRC(a3a3a155) SHA1(b05005b324560b64bcba3ab53a66586650cae6ad) )
	SP_TKPIK_SOUND
ROM_END

ROM_START( sp_tkpikf )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tp04d11h", 0x0000, 0x8000, CRC(6ad9a4fc) SHA1(9b378cec57f0f042a44c11e4f756f7f748fee739) )
	ROM_LOAD( "tp04d11a", 0x8000, 0x8000, CRC(a3a3a155) SHA1(b05005b324560b64bcba3ab53a66586650cae6ad) )
	SP_TKPIK_SOUND
ROM_END

#define SP_CARRY_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_carry )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "carry on 10p payout p2.bin", 0x0000, 0x8000, CRC(1db4062c) SHA1(92098a145c9cdf04758b92998dd4fca7945cf991) )
	ROM_LOAD( "carry on p1.bin", 0x8000, 0x8000, CRC(9a989d88) SHA1(e4cb4107c30b22fe9d952888285eb23de3005d2c) ) // aka carry on v6 p1.bin
	SP_CARRY_SOUND
ROM_END

ROM_START( sp_carrya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "carry on 20p payout p2.bin", 0x0000, 0x8000, CRC(ae1b2c66) SHA1(47a17c5178520d7ed6bc3aaaef111e8188d9a6b0) )
	ROM_LOAD( "carry on p1.bin", 0x8000, 0x8000, CRC(9a989d88) SHA1(e4cb4107c30b22fe9d952888285eb23de3005d2c) ) // aka carry on v6 p1.bin
	SP_CARRY_SOUND
ROM_END



#define SP_FRONT_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_front )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bwb_final_frontier_p2.bin", 0x0000, 0x8000, CRC(f5a963d8) SHA1(433ffe6e45ef92dd15f3fd7942567b21ec064c78) )
	ROM_LOAD( "bwb_final_frontier_p1.bin", 0x8000, 0x8000, CRC(2e835304) SHA1(ebc8286c70c286a8f4aa196e996540790742917e) )
	SP_FRONT_SOUND
ROM_END

/* Crystal games below, these have Crystal encryption, and appear to be running on sp.ace HW, possible the sound systems are different too. */

#define SP_ATW_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* actually I think this rom is just a bad dump, there is a rom which is the same in the JPM HW set, */ \
	/* but twice the size.  Also this isn't an oki rom!                                                  */ \
	ROM_LOAD( "atw80snd.bin", 0x00000, 0x20000, CRC(b002e11c) SHA1(f7133f4bb8c31feaad0a7b9ee88749f9b7877575) )
ROM_START( sp_atw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "artwld80", 0x00000, 0x10000, CRC(3ff314c3) SHA1(345df80243953b35916449b0aa6ffaf9d3501d2b) ) // pre-decrypted? bootleg?
	SP_ATW_SOUND
ROM_END

#define SP_FIVE_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_five )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fn19.bin", 0x00000, 0x10000, CRC(4721ccf8) SHA1(a6b7b238df7e7cf45c049b4fb16bf0c05fb95b41) )
	SP_FIVE_SOUND
ROM_END

ROM_START( sp_fivea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fn19p.bin", 0x00000, 0x10000, CRC(ac2de72f) SHA1(61263944fe29b5f5c79c987989a784b32700c902) )
	SP_FIVE_SOUND
ROM_END


#define SP_CRUN_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_crun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crun411.bin", 0x00000, 0x10000, CRC(74a69327) SHA1(087d791b8e5c43a9c614f5f4344ce2524a8c445d) )
	SP_CRUN_SOUND
ROM_END

ROM_START( sp_cruna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "em111.bin", 0x00000, 0x10000, CRC(c1e9d4ec) SHA1(49eadcf7880d68c1559e94f4389eca739a3b04d7) )
	SP_CRUN_SOUND
ROM_END

ROM_START( sp_crunb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "em111p.bin", 0x00000, 0x10000, CRC(bbe90c64) SHA1(5e65c318f14f7aa160f3d0daeb1f3038df162b65) )
	SP_CRUN_SOUND
ROM_END

#define SP_ROOF_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */
ROM_START( sp_roof )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "034p1-2h.bin", 0x0000, 0x8000, CRC(2b0353fa) SHA1(5c9f06fdda33c4a4a09c69f1e969ae4041513fd9) )
	ROM_LOAD( "034p1-2a.bin", 0x8000, 0x8000, NO_DUMP )
	SP_ROOF_SOUND
ROM_END

ROM_START( sp_roofa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "034p1-2i.bin", 0x0000, 0x8000, CRC(a64797fc) SHA1(7437dc2e203efc525aab251da5196d31b95d159a) )
	ROM_LOAD( "034p1-2a.bin", 0x8000, 0x8000, NO_DUMP )
	SP_ROOF_SOUND
ROM_END

#define SP_CPAL_SOUND \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 ) \
	/* not used, or missing? */

ROM_START( sp_cpal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fm519d11.bin", 0x0000, 0x010000, CRC(0272325e) SHA1(2f632ea7246c2afd485b11a03afeef4c9e30f5cf) )
	SP_CPAL_SOUND
ROM_END

static void descramble_crystal( uint8_t* region, int start, int end, uint8_t extra_xor)
{
	for (int i=start;i<end;i++)
	{
		uint8_t x = region[i];
		switch (i & 0x58)
		{
		case 0x00: // same as 0x08
		case 0x08: x = bitswap<8>( x^0xca , 3,2,1,0,7,4,6,5 ); break;
		case 0x10: x = bitswap<8>( x^0x30 , 3,0,4,6,1,5,7,2 ); break;
		case 0x18: x = bitswap<8>( x^0x89 , 4,1,2,5,7,0,6,3 ); break;
		case 0x40: x = bitswap<8>( x^0x14 , 6,1,4,3,2,5,0,7 ); break;
		case 0x48: x = bitswap<8>( x^0x40 , 1,0,3,2,5,4,7,6 ); break;
		case 0x50: x = bitswap<8>( x^0xcb , 3,2,1,0,7,6,5,4 ); break;
		case 0x58: x = bitswap<8>( x^0xc0 , 2,3,6,0,5,1,7,4 ); break;
		}
		region[i] = x ^ extra_xor;
	}
}
void ace_sp_state::init_ace_cr()
{
	descramble_crystal(memregion( "maincpu" )->base(), 0x0000, 0x10000, 0x00);
}

void ace_sp_state::init_ace_sp()
{
}

} // anonymous namespace


GAME( 199?, sp_cbowl,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowla,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlb,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlc,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowld,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowle,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlf,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlg,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlh,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowli,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlj,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlk,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowll,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlm,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowln,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlo,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlp,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 17)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlq,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 18)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlr,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 19)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowls,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 20)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlt,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 21)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cbowlu,    sp_cbowl, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Cash Bowl (Ace) (sp.ACE) (set 22)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_crime,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Crime Watch (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crimea,    sp_crime, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Crime Watch (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crimeb,    sp_crime, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Crime Watch (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crimec,    sp_crime, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Crime Watch (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crimed,    sp_crime, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Crime Watch (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crimee,    sp_crime, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Crime Watch (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crimef,    sp_crime, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Crime Watch (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crimeg,    sp_crime, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Crime Watch (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crimeh,    sp_crime, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Crime Watch (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )

// seems to be very closely related to Woolpack, looks like the same basic game with different strings, these have "GO TO EMMERDALE"
GAME( 1995, sp_emmrd,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrda,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdb,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdc,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdd,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrde,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdf,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdg,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdh,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdi,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdj,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdk,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdn,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_emmrdo,    sp_emmrd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Emmerdale (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
// seems to be very closely related to Emmerdale, looks like the same basic game with different strings, these have "GO T' WOOLPACK"
GAME( 1995, sp_woolp,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpa,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpb,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpc,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpd,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpe,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpf,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL ) // incomplete
GAME( 1995, sp_woolpg,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL ) // incomplete
GAME( 1995, sp_woolph,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL ) // incomplete
GAME( 1995, sp_woolpi,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL ) // incomplete
GAME( 1995, sp_woolpj,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpk,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpl,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpm,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpn,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1995, sp_woolpo,    sp_woolp, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Woolpack (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
// this has 'Emmerdale' padding, but seems to be a unique game, contains "ZIGZAG" strings
GAME( 199?, sp_zigzg,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzga,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgb,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgc,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgd,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzge,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgf,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgg,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgh,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgi,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgj,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgk,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgl,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_zigzgm,    sp_zigzg, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Zig Zag (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
// some of these have additional Whitbread copyright, many appear to be 'Golden Mile Showcase', several have a 'Golden Mile Prize' padding instead of the regular
GAME( 199?, sp_goldm,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldma,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmb,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmc,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmd,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldme,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmf,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmg,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmh,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmi,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmj,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmk,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldml,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmm,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmn,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmo,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmp,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 17)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmq,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 18)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmr,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 19)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldms,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 20)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmt,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 21)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmu,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 22)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmv,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 23)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmw,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 24)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmx,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 25)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmy,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 26)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldmz,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 27)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldm0,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 28)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldm1,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 29)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldm2,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 30)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldm3,    sp_goldm, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Mile (Ace) (sp.ACE) (set 31)",MACHINE_IS_SKELETON_MECHANICAL )
// one of the types here have blanked out padding, possibly a BWB re-release?
GAME( 199?, sp_gnat,      0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnata,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnatb,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnatc,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnatd,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnate,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnatf,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnatg,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnath,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnati,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnatj,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnatk,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnatl,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnatm,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnatn,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gnato,     sp_gnat , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand National (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
// seems to be a variation on Grand National (prize cabinet?)
GAME( 199?, sp_przna,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Prize National (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_prznaa,    sp_przna, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Prize National (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_prznab,    sp_przna, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Prize National (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_prznac,    sp_przna, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Prize National (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_prznad,    sp_przna, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Prize National (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_prznae,    sp_przna, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Prize National (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_prznaf,    sp_przna, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Prize National (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_prznag,    sp_przna, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Prize National (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_gprix,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand Prix (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gprixa,    sp_gprix, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand Prix (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gprixb,    sp_gprix, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand Prix (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gprixc,    sp_gprix, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand Prix (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gprixd,    sp_gprix, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand Prix (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gprixe,    sp_gprix, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand Prix (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gprixf,    sp_gprix, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand Prix (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gprixg,    sp_gprix, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand Prix (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_gprixh,    sp_gprix, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Grand Prix (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_hideh,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hideha,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehb,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehc,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehd,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehe,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehf,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehg,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehh,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehi,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehj,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehk,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehl,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehm,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehn,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hideho,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hidehp,    sp_hideh, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi De Hi (Ace) (sp.ACE) (set 17)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_hifly,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflya,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyb,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyc,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyd,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflye,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyf,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyg,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyh,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyi,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyj,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyk,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyl,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflym,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyn,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyo,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hiflyp,    sp_hifly, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Hi Flyer (Ace) (sp.ACE) (set 17)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_juras,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Jurassic Trail (Ace) (sp.ACE)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_openb,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Open The Box (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_openba,    sp_openb, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Open The Box (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_openbb,    sp_openb, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Open The Box (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_openbc,    sp_openb, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Open The Box (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_openbd,    sp_openb, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Open The Box (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_openbe,    sp_openb, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Open The Box (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_openbf,    sp_openb, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Open The Box (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_openbg,    sp_openb, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Open The Box (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_openbh,    sp_openb, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Open The Box (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_openbi,    sp_openb, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Open The Box (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_payrs,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Payrise (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_payrsa,    sp_payrs, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Payrise (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_payrsb,    sp_payrs, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Payrise (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_payrsc,    sp_payrs, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Payrise (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_payrsd,    sp_payrs, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Payrise (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_payrse,    sp_payrs, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Payrise (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_payrsf,    sp_payrs, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Payrise (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_payrsg,    sp_payrs, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Payrise (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_payrsh,    sp_payrs, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Payrise (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_playa,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Play It Again (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_playaa,    sp_playa, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Play It Again (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_playab,    sp_playa, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Play It Again (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL ) // incomplete
GAME( 199?, sp_playac,    sp_playa, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Play It Again (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL ) // incomplete
GAME( 199?, sp_playad,    sp_playa, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Play It Again (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL ) // possible bad pairing
GAME( 199?, sp_playae,    sp_playa, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Play It Again (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL ) // possible bad pairing
GAME( 199?, sp_playaf,    sp_playa, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Play It Again (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL ) // possible bad pairing
GAME( 199?, sp_playag,    sp_playa, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Play It Again (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL ) // possible bad pairing
GAME( 199?, sp_playah,    sp_playa, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Play It Again (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL ) // possible bad pairing
GAME( 199?, sp_playai,    sp_playa, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Play It Again (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL ) // possible bad pairing

GAME( 199?, sp_spell,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Spellbound (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_spella,    sp_spell, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Spellbound (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_spellb,    sp_spell, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Spellbound (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_spellc,    sp_spell, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Spellbound (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_spelld,    sp_spell, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Spellbound (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_spelle,    sp_spell, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Spellbound (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_spellf,    sp_spell, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Spellbound (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_spellg,    sp_spell, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Spellbound (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_spelli,    sp_spell, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Spellbound (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_spellj,    sp_spell, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Spellbound (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_swop,      0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Swop Shop (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_swopa,     sp_swop , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Swop Shop (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_swopb,     sp_swop , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Swop Shop (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_swopc,     sp_swop , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Swop Shop (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_swopd,     sp_swop , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Swop Shop (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_swope,     sp_swop , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Swop Shop (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_swopf,     sp_swop , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Swop Shop (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_swopg,     sp_swop , ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Swop Shop (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_timem,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timema,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timemb,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timemc,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timemd,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timeme,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timemf,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timemg,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timemh,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timemi,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timemj,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_timemk,    sp_timem, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Time Machine (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_tz,        0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace",       "Twilight Zone (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tza,       sp_tz,    ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace",       "Twilight Zone (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzb,       sp_tz,    ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace",       "Twilight Zone (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzc,       sp_tz,    ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace",       "Twilight Zone (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzd,       sp_tz,    ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace",       "Twilight Zone (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tze,       sp_tz,    ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace",       "Twilight Zone (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzf,       sp_tz,    ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace",       "Twilight Zone (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzg,       sp_tz,    ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace",       "Twilight Zone (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzh,       sp_tz,    ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace",       "Twilight Zone (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzbwb,     sp_tz,    ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace / Bwb", "Twilight Zone (Ace/Bwb) (sp.ACE)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_tzfe,      0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfea,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfeb,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfec,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfed,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfee,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfef,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfeg,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfeh,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfei,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfej,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfek,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfel,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfem,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfen,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfeo,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfep,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 17)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfeq,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 18)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfer,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 19)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfes,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 20)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfet,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 21)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tzfeu,     sp_tzfe,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Twilight Zone - Further Encounters (Ace) (sp.ACE) (set 22)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_beau,      0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Beau Peep (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beaua,     sp_beau,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Beau Peep (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beaub,     sp_beau,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Beau Peep (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beauc,     sp_beau,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Beau Peep (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beaud,     sp_beau,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Beau Peep (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beaue,     sp_beau,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Beau Peep (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beauf,     sp_beau,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Beau Peep (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beaug,     sp_beau,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Beau Peep (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beauh,     sp_beau,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Beau Peep (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_bigbd,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Big Break Deluxe Club (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_bigbda,    sp_bigbd, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Big Break Deluxe Club (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_brkbk,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Break The Bank (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_brkbka,    sp_brkbk, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Break The Bank (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_brkbkb,    sp_brkbk, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Break The Bank (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_brkbkc,    sp_brkbk, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Break The Bank (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_brkbkd,    sp_brkbk, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Break The Bank (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_camel,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camela,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camelb,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camelc,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cameld,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camele,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camelf,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camelg,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camelh,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cameli,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camelj,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camelk,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camell,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camelm,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cameln,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_camelo,    sp_camel, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Camelot (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_clbna,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Club National (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_clbnaa,    sp_clbna, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Club National (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_coder,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Code Red (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_codera,    sp_coder, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Code Red (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_coderb,    sp_coder, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Code Red (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_coderc,    sp_coder, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Code Red (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_coderd,    sp_coder, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Code Red (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_codere,    sp_coder, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Code Red (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_coderf,    sp_coder, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Code Red (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_coderg,    sp_coder, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Code Red (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_criss,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Criss Cross Cash (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crissa,    sp_criss, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Criss Cross Cash (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crissb,    sp_criss, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Criss Cross Cash (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crissc,    sp_criss, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Criss Cross Cash (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crissd,    sp_criss, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Criss Cross Cash (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crisse,    sp_criss, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Criss Cross Cash (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crissf,    sp_criss, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Criss Cross Cash (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crissg,    sp_criss, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Criss Cross Cash (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_daytr,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Daytripper (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_daytra,    sp_daytr, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Daytripper (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_daytrb,    sp_daytr, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Daytripper (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_daytrc,    sp_daytr, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Daytripper (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_donky,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Donkey Derby (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_donkya,    sp_donky, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Donkey Derby (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_donkyb,    sp_donky, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Donkey Derby (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_donkyc,    sp_donky, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Donkey Derby (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_donkyd,    sp_donky, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Donkey Derby (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_donkye,    sp_donky, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Donkey Derby (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_donkyf,    sp_donky, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Donkey Derby (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_donkyg,    sp_donky, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Donkey Derby (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_dyour,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Double Your Money (Ace) (sp.ACE)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_festi,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Festival (Spanish) (Ace) (sp.ACE)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_beau2,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Further Adventures Of Beau Peep (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beau2a,    sp_beau2, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Further Adventures Of Beau Peep (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beau2b,    sp_beau2, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Further Adventures Of Beau Peep (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beau2c,    sp_beau2, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Further Adventures Of Beau Peep (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beau2d,    sp_beau2, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Further Adventures Of Beau Peep (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beau2e,    sp_beau2, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Further Adventures Of Beau Peep (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_beau2f,    sp_beau2, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Further Adventures Of Beau Peep (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_ghost,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghosta,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostb,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostc,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostd,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghoste,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostf,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostg,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghosth,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghosti,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostj,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostk,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostl,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostm,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostn,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghosto,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostp,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 17)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostq,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 18)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghostr,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 19)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_ghosts,    sp_ghost, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Ghost Trapper (Ace) (sp.ACE) (set 20)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_globe,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Globe Trotter (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_globea,    sp_globe, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Globe Trotter (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_globeb,    sp_globe, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Globe Trotter (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_globec,    sp_globe, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Globe Trotter (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_globed,    sp_globe, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Globe Trotter (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_globee,    sp_globe, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Globe Trotter (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_globef,    sp_globe, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Globe Trotter (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_globeg,    sp_globe, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Globe Trotter (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_gol,       0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Gol (Spanish) (Ace) (sp.ACE)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_golda,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Arrow Club (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldaa,    sp_golda, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Arrow Club (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )

// These contain lots of 'Golden Streak' strings, as well as 'Have you got the Golden Touch?' strings
GAME( 199?, sp_golds,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Streak (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldsa,    sp_golds, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Streak (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldsb,    sp_golds, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Streak (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldsc,    sp_golds, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Streak (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldsd,    sp_golds, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Streak (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldse,    sp_golds, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Streak (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldsf,    sp_golds, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Streak (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldsg,    sp_golds, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Streak (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_goldsh,    sp_golds, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Streak (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
// Very similar to above, but many of the 'Golden Streak' strings have been changed to Golden Touch, header still says Golden Streak tho
GAME( 199?, sp_goldt,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Golden Streak (Golden Touch) (Ace) (sp.ACE)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_here,      0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Here We Go (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_herea,     sp_here,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Here We Go (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hereb,     sp_here,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Here We Go (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_herec,     sp_here,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Here We Go (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hered,     sp_here,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Here We Go (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_heree,     sp_here,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Here We Go (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_heref,     sp_here,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Here We Go (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_hereg,     sp_here,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Here We Go (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_holid,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Holiday Club (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_holida,    sp_holid, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Holiday Club (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_lotto,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Lotto (Spanish) (Ace) (sp.ACE)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_magmo,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Magic Money (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_magmoa,    sp_magmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Magic Money (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_magmob,    sp_magmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Magic Money (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_magmoc,    sp_magmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Magic Money (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_magmod,    sp_magmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Magic Money (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_megmo,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Mega Money (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_megmoa,    sp_megmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Mega Money (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_megmob,    sp_megmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Mega Money (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_megmoc,    sp_megmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Mega Money (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_megmod,    sp_megmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Mega Money (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_megmoe,    sp_megmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Mega Money (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_megmof,    sp_megmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Mega Money (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_megmog,    sp_megmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Mega Money (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_monma,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Magic (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmaa,    sp_monma, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Magic (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmab,    sp_monma, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Magic (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmac,    sp_monma, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Magic (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmad,    sp_monma, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Magic (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_monmo,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Mountain (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmoa,    sp_monmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Mountain (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmob,    sp_monmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Mountain (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmoc,    sp_monmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Mountain (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmod,    sp_monmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Mountain (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmoe,    sp_monmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Mountain (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmof,    sp_monmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Mountain (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_monmog,    sp_monmo, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Money Mountain (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_nudex,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Nudge Explosion (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_nudexa,    sp_nudex, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Nudge Explosion (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_onbox,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxa,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxb,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxc,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxd,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxe,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxf,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxg,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxh,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxi,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxj,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxk,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxl,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxm,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_onboxn,    sp_onbox, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Box (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_piste,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pistea,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pisteb,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pistec,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pisted,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pistee,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pistef,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pisteg,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pisteh,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pistei,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pistej,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pistek,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pistel,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pistem,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pisten,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pisteo,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pistep,    sp_piste, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "On The Piste (Ace) (sp.ACE) (set 17)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_pound,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pounda,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundb,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundc,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundd,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_pounde,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundf,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundg,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundh,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 9)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundi,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 10)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundj,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 11)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundk,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 12)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundl,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 13)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundm,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 14)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundn,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 15)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundo,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 16)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundp,    sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Pound For Pound (Ace) (sp.ACE) (set 17)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundbwb,  sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace / Bwb", "Pound For Pound (Ace/Bwb) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundbwba, sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace / Bwb", "Pound For Pound (Ace/Bwb) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundbwbb, sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace / Bwb", "Pound For Pound (Ace/Bwb) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundbwbc, sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace / Bwb", "Pound For Pound (Ace/Bwb) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundbwbd, sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace / Bwb", "Pound For Pound (Ace/Bwb) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundbwbe, sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace / Bwb", "Pound For Pound (Ace/Bwb) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundbwbf, sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace / Bwb", "Pound For Pound (Ace/Bwb) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_poundbwbg, sp_pound, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace / Bwb", "Pound For Pound (Ace/Bwb) (sp.ACE) (set 8)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_road,      0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Road To Hell (Ace) (sp.ACE)",MACHINE_IS_SKELETON_MECHANICAL ) // incomplete program

GAME( 199?, sp_skylm,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "The Sky's The Limit Club (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_skylma,    sp_skylm, ace_sp ,ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "The Sky's The Limit Club (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_tkpik,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Take Your Pick (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tkpika,    sp_tkpik, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Take Your Pick (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tkpikb,    sp_tkpik, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Take Your Pick (Ace) (sp.ACE) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tkpikc,    sp_tkpik, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Take Your Pick (Ace) (sp.ACE) (set 4)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tkpikd,    sp_tkpik, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Take Your Pick (Ace) (sp.ACE) (set 5)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tkpike,    sp_tkpik, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Take Your Pick (Ace) (sp.ACE) (set 6)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_tkpikf,    sp_tkpik, ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Take Your Pick (Ace) (sp.ACE) (set 7)",MACHINE_IS_SKELETON_MECHANICAL )
// not sure.. looks like 6303 code to me
GAME( 199?, sp_atw,       0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Crystal","Around The World In Eighty Days (Crystal) (sp.ACE?)",MACHINE_IS_SKELETON_MECHANICAL )
// not sure.. looks like 6303 code to me
GAME( 199?, sp_five,      0,        ace_sp, ace_sp, ace_sp_state, init_ace_cr, ROT0, "Crystal","Fiver Fever (Crystal) (sp.ACE?) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_fivea,     sp_five,  ace_sp, ace_sp, ace_sp_state, init_ace_cr, ROT0, "Crystal","Fiver Fever (Crystal) (sp.ACE?) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
// not sure.. looks like 6303 code to me
GAME( 199?, sp_crun,      0,        ace_sp, ace_sp, ace_sp_state, init_ace_cr, ROT0, "Crystal","Cash Run (Crystal) (sp.ACE?) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_cruna,     sp_crun,  ace_sp, ace_sp, ace_sp_state, init_ace_cr, ROT0, "Crystal","Cash Run (Crystal) (sp.ACE?) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_crunb,     sp_crun,  ace_sp, ace_sp, ace_sp_state, init_ace_cr, ROT0, "Crystal","Cash Run (Crystal) (sp.ACE?) (set 3)",MACHINE_IS_SKELETON_MECHANICAL )

// incomplete dump (was mixed with the IMPACT rebuild)
GAME( 199?, sp_roof,      0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Thru' The Roof (Ace) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_roofa,     sp_roof,  ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Thru' The Roof (Ace) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )

GAME( 199?, sp_cpal,      0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Ace", "Caesars Palace (Ace) (sp.ACE?)",MACHINE_IS_SKELETON_MECHANICAL ) // was in an IMPACT set, might be a different game but CPU seems correct for here

// these show something

GAME( 199?, sp_carry,     0,        ace_sp_pcp, ace_sp, ace_sp_state, init_ace_sp, ROT0,"Pcp", "Carry On (Pcp) (sp.ACE) (set 1)",MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, sp_carrya,    sp_carry, ace_sp_pcp, ace_sp, ace_sp_state, init_ace_sp, ROT0,"Pcp", "Carry On (Pcp) (sp.ACE) (set 2)",MACHINE_IS_SKELETON_MECHANICAL )
// boots to 'Fire Crak V1' (maybe DMD wasn't visible on this machine or this is mislabaled?)
GAME( 199?, sp_front,     0,        ace_sp, ace_sp, ace_sp_state, init_ace_sp, ROT0, "Bwb", "Final Frontier (Bwb) (sp.ACE)",MACHINE_IS_SKELETON_MECHANICAL )

