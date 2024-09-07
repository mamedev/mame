// license:BSD-3-Clause
// copyright-holders: Xing Xing, David Haywood

/*

IGS ARM7 (IGS027A) based Mahjong / Gambling platform(s) with XA sub-cpu
These games use the IGS027A processor.

*/

#include "emu.h"

#include "igs017_igs031.h"
#include "pgmcrypt.h"

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "cpu/xa/xa.h"

#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/timer.h"

#include "screen.h"

namespace {

class igs_m027xa_state : public driver_device
{
public:
	igs_m027xa_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_igs_mainram(*this, "igs_mainram"),
		m_maincpu(*this, "maincpu"),
		m_xa(*this, "xa"),
		m_ppi(*this, "ppi8255"),
		m_igs017_igs031(*this, "igs017_igs031"),
		m_screen(*this, "screen")
	{ }

	void igs_mahjong_xa(machine_config &config);

	void init_crzybugs();
	void init_crzybugsj();
	void init_hauntedh();
	void init_tripfev();
	void init_wldfruit();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	optional_shared_ptr<u32> m_igs_mainram;
	required_device<cpu_device> m_maincpu;
	required_device<mx10exa_cpu_device> m_xa;
	required_device<i8255_device> m_ppi;
	required_device<igs017_igs031_device> m_igs017_igs031;
	required_device<screen_device> m_screen;

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void pgm_create_dummy_internal_arm_region();
	void igs_mahjong_map(address_map &map);

	void igs_70000100_w(u32 data);
	u32 m_igs_70000100 = 0;

	u32 rnd_r()
	{
		return machine().rand();
	}
};



void igs_m027xa_state::machine_start()
{
	save_item(NAME(m_igs_70000100));
}

void igs_m027xa_state::video_start()
{
	m_igs017_igs031->video_start();
}

void igs_m027xa_state::igs_70000100_w(u32 data)
{
	m_igs_70000100 = data;
}


/***************************************************************************

    Memory Maps

***************************************************************************/

void igs_m027xa_state::igs_mahjong_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom(); // Internal ROM
	map(0x08000000, 0x0807ffff).rom().region("user1", 0); // Game ROM
	map(0x10000000, 0x100003ff).ram().share("igs_mainram"); // main RAM for ASIC?
	map(0x18000000, 0x18007fff).ram();

	map(0x38000000, 0x38007fff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write));
	map(0x4000000c, 0x4000000f).r(FUNC(igs_m027xa_state::rnd_r));

	map(0x58000000, 0x580000ff).ram(); // XA?

	map(0x70000100, 0x70000103).w(FUNC(igs_m027xa_state::igs_70000100_w));

	map(0x70000200, 0x70000203).ram();     //??????????????
	map(0x50000000, 0x500003ff).nopw(); // uploads XOR table to external ROM here
	map(0xf0000000, 0xf000000f).nopw(); // magic registers
}

static INPUT_PORTS_START( base )
	PORT_START("TEST0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TEST1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TEST2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(igs_m027xa_state::interrupt)
{
	int scanline = param;

	if (scanline == 240 && m_igs017_igs031->get_irq_enable())
		m_maincpu->pulse_input_line(ARM7_IRQ_LINE, m_maincpu->minimum_quantum_time()); // source? (can the XA trigger this?)

	if (scanline == 0 && m_igs017_igs031->get_nmi_enable())
		m_maincpu->pulse_input_line(ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time()); // vbl?
}


void igs_m027xa_state::igs_mahjong_xa(machine_config &config)
{
	ARM7(config, m_maincpu, 22000000); // Crazy Bugs has a 22Mhz Xtal, what about the others?
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_m027xa_state::igs_mahjong_map);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MX10EXA(config, m_xa, 10000000); // MX10EXAQC (Philips 80C51 XA) unknown frequency

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 240-1);
	m_screen->set_screen_update("igs017_igs031", FUNC(igs017_igs031_device::screen_update));
	m_screen->set_palette("igs017_igs031:palette");

	TIMER(config, "scantimer").configure_scanline(FUNC(igs_m027xa_state::interrupt), "screen", 0, 1);

	I8255A(config, m_ppi);
	m_ppi->in_pa_callback().set_ioport("TEST0");
	m_ppi->in_pb_callback().set_ioport("TEST1");
	m_ppi->in_pc_callback().set_ioport("TEST2");

	IGS017_IGS031(config, m_igs017_igs031, 0);
	m_igs017_igs031->set_text_reverse_bits(true);
	m_igs017_igs031->set_i8255_tag("ppi8255");

	// sound hardware
	// OK6295
}

// prg at u34
// text at u15
// cg at u32 / u12
// samples at u3

ROM_START( haunthig )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "haunthig_igs027a", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "hauntedhouse_ver-109us.u34", 0x000000, 0x80000, CRC(300fed78) SHA1(afa4c8855cd780c57d4f92ea6131ed4e77063268) )

	ROM_REGION( 0x10000, "xa", 0 )
	ROM_LOAD( "hauntedhouse.u17", 0x000000, 0x10000, BAD_DUMP CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) ) // not dumped for this set

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "haunted-h_text.u15", 0x000000, 0x80000, CRC(c23f48c8) SHA1(0cb1b6c61611a081ae4a3c0be51812045ff632fe) )

	// are these PGM-like sprites?
	ROM_REGION( 0x800000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "haunted-h_cg.u32",  0x000000, 0x400000, BAD_DUMP CRC(e0ea10e6) SHA1(e81be78fea93e72d4b1f4c0b58560bda46cf7948) ) // not dumped for this set, FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "haunted-h_ext.u12", 0x400000, 0x400000, BAD_DUMP CRC(662eb883) SHA1(831ebe29e1e7a8b2c2fff7fbc608975771c3486c) ) // not dumped for this set, FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x200000, "samples", 0 ) // Oki M6295 samples, missing sample table, bad?
	ROM_LOAD( "haunted-h_sp.u3", 0x00000, 0x200000,  BAD_DUMP CRC(fe3fcddf) SHA1(ac57ab6d4e4883747c093bd19d0025cf6588cb2c) ) // not dumped for this set

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "hu_u38a.u38", 0x000, 0x117, NO_DUMP ) // ATF16V8B, protected
	ROM_LOAD( "hu_u39.u39",  0x200, 0x2dd, CRC(75f58b46) SHA1(7cb136a41899ddd50c95a67ca6353ce5d8d92149) ) // AT22V10
ROM_END

ROM_START( haunthiga ) // IGS PCB-0575-04-HU - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 2x 8-dip banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "h2_igs027a", 0x00000, 0x4000, NO_DUMP ) // sticker marked 'H2'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "hauntedhouse_ver-101us.u34", 0x000000, 0x80000, CRC(4bf045d4) SHA1(78c848fd69961df8d9b75f92ad57c3534fbf08db) )

	ROM_REGION( 0x10000, "xa", 0 )
	ROM_LOAD( "hauntedhouse.u17", 0x000000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) ) // MX10EXAQC (80C51 XA based MCU) marked J9, not read protected?

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "haunted-h_text.u15", 0x000000, 0x80000, CRC(c23f48c8) SHA1(0cb1b6c61611a081ae4a3c0be51812045ff632fe) )

	// are these PGM-like sprites?
	ROM_REGION( 0x800000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "haunted-h_cg.u32",  0x000000, 0x400000, CRC(e0ea10e6) SHA1(e81be78fea93e72d4b1f4c0b58560bda46cf7948) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	ROM_LOAD( "haunted-h_ext.u12", 0x400000, 0x400000, CRC(662eb883) SHA1(831ebe29e1e7a8b2c2fff7fbc608975771c3486c) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x200000, "samples", 0 ) // Oki M6295 samples, missing sample table, bad?
	ROM_LOAD( "haunted-h_sp.u3", 0x00000, 0x200000, BAD_DUMP CRC(fe3fcddf) SHA1(ac57ab6d4e4883747c093bd19d0025cf6588cb2c) )

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "hu_u38a.u38", 0x000, 0x117, NO_DUMP ) // ATF16V8B, protected
	ROM_LOAD( "hu_u39.u39",  0x200, 0x2dd, CRC(75f58b46) SHA1(7cb136a41899ddd50c95a67ca6353ce5d8d92149) ) // AT22V10
ROM_END

ROM_START( crzybugs ) // IGS PCB-0447-05-GM - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 3x 8-DIP banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "m7_igs27a.u37", 0x00000, 0x4000, CRC(1b20532c) SHA1(e08d0110a843915a8ba8627ae6d3947cccc22048) ) // sticker marked 'M7'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "crazy_bugs_v-204us.u23", 0x000000, 0x80000, CRC(d1232462) SHA1(685a292f39bf57a80d6ef31289cf9f673ba06dd4) ) // MX27C4096

	ROM_REGION( 0x10000, "xa", 0 ) // MX10EXAQC (80C51 XA based MCU) marked J9
	ROM_LOAD( "j9.u27", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "crazy_bugs_text_u10.u10", 0x000000, 0x80000, CRC(db0d679a) SHA1(c5d039aa4fa2218b6f574ccb5b6da983b8d4067d) )

	// are these PGM-like sprites?
	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "crazy_bugs_cg.u19",  0x000000, 0x200000, CRC(9d53ad47) SHA1(46690a37acf8bd88c7fbe973db2faf5ef0cff805) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "samples", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "crazybugs_sp.u15", 0x000000, 0x200000, CRC(591b315b) SHA1(fda1816d83e202170dba4afc6e7898b706a76087) ) // M27C160
ROM_END

ROM_START( crzybugsa )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "m7_igs27a.u37", 0x00000, 0x4000, CRC(1b20532c) SHA1(e08d0110a843915a8ba8627ae6d3947cccc22048) ) // sticker marked 'M7' (not verified for this set)

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "crazy_bugs_v-202us.u23", 0x000000, 0x80000, CRC(210da1e6) SHA1(c726497bebd25d6a9053e331b4c26acc7e2db0b2) ) // MX27C4096

	ROM_REGION( 0x10000, "xa", 0 ) // MX10EXAQC (80C51 XA based MCU)
	ROM_LOAD( "j9.u27", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "crazy_bugs_text_u10.u10", 0x000000, 0x80000, CRC(db0d679a) SHA1(c5d039aa4fa2218b6f574ccb5b6da983b8d4067d) ) // M27C4002

	// are these PGM-like sprites?
	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "crazy_bugs_cg.u19",  0x000000, 0x200000, CRC(9d53ad47) SHA1(46690a37acf8bd88c7fbe973db2faf5ef0cff805) ) // M27C160, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "samples", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "crazy_bugs_sp.u15", 0x000000, 0x200000, CRC(591b315b) SHA1(fda1816d83e202170dba4afc6e7898b706a76087) ) // M27C160
ROM_END

ROM_START( crzybugsb )
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "m7_igs27a.u37", 0x00000, 0x4000, CRC(1b20532c) SHA1(e08d0110a843915a8ba8627ae6d3947cccc22048) ) // sticker marked 'M7' (not verified for this set)

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "crazy_bugs_v-202us.u23", 0x000000, 0x80000, CRC(129e36e9) SHA1(53f20bc3792249de8ef276f84283baa9abd30acd) ) // MX27C4096

	ROM_REGION( 0x10000, "xa", 0 ) // MX10EXAQC (80C51 XA based MCU)
	ROM_LOAD( "j9.u27", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "crazy_bugs_text_u10.u10", 0x000000, 0x80000, BAD_DUMP CRC(db0d679a) SHA1(c5d039aa4fa2218b6f574ccb5b6da983b8d4067d) ) // not dumped for this set

	// are these PGM-like sprites?
	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "crazy_bugs_cg.u19",  0x000000, 0x200000, BAD_DUMP CRC(9d53ad47) SHA1(46690a37acf8bd88c7fbe973db2faf5ef0cff805) ) // not dumped for this set, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "samples", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "crazy_bugs_sp.u15", 0x000000, 0x200000, BAD_DUMP CRC(591b315b) SHA1(fda1816d83e202170dba4afc6e7898b706a76087) ) // not dumped for this set
ROM_END

ROM_START( crzybugsj ) // IGS PCB-0575-04-HU - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 2x 8-dip banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "m6.u42", 0x00000, 0x4000, NO_DUMP ) // sticker marked 'M6'

	ROM_REGION32_LE( 0x200000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "crazy_bugs_v-103jp.u34", 0x000000, 0x200000, CRC(1e35ed79) SHA1(0e4f8b706cdfcaf2aacdc40eec422df9d865b311) )

	ROM_REGION( 0x10000, "xa", 0 )
	ROM_LOAD( "e9.u17", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) ) // MX10EXAQC (80C51 XA based MCU) marked E9, same as haunthig

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "crazy_bugs_text_u15.u15", 0x000000, 0x80000, CRC(db0d679a) SHA1(c5d039aa4fa2218b6f574ccb5b6da983b8d4067d) )
	// u14 not populated

	// are these PGM-like sprites?
	ROM_REGION( 0x200000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "crazy_bugs_ani-cg-u32.u32",  0x000000, 0x200000, CRC(9d53ad47) SHA1(46690a37acf8bd88c7fbe973db2faf5ef0cff805) ) // FIXED BITS (xxxxxxx0xxxxxxxx)
	// u12 not populated

	ROM_REGION( 0x200000, "samples", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "crazy_bugs_sp_u3.u3", 0x000000, 0x200000,  CRC(b15974a1) SHA1(82509902bbb33a2120d815e7879b9b8591a29976) )

	ROM_REGION( 0x500, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "hu_u38.u38", 0x000, 0x117, NO_DUMP ) // ATF16V8B, protected
	ROM_LOAD( "hu_u39.u39", 0x200, 0x2dd, CRC(75f58b46) SHA1(7cb136a41899ddd50c95a67ca6353ce5d8d92149) ) // AT22V10
ROM_END

ROM_START( tripfev ) // IGS PCB-0447-05-GM - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 3x 8-DIP banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "m1_igs027a.u37", 0x00000, 0x4000, CRC(a40ec1f8) SHA1(f6f7005d61522934758fd0a98bf383c6076b6afe) ) // sticker marked 'M1'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "triple_fever_u23_v107_us.u23", 0x000000, 0x80000, CRC(aa56d888) SHA1(0b8b2765079259b76ea803289841d867c33c8cb2) ) // 27C4096

	ROM_REGION( 0x10000, "xa", 0 ) // MX10EXAQC (80C51 XA based MCU) marked P7
	ROM_LOAD( "p7.u27", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "triple_fever_u10_text.u10", 0x000000, 0x80000, CRC(522a1030) SHA1(9a7a5ba9b26bceb0d251be6139c10e4655fc19ec) ) // M27C4002

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "triple_fever_u19_cg.u19",  0x000000, 0x400000, CRC(cd45bbf2) SHA1(7f1cf270245bbe4604de2cacade279ab13584dbd) ) // M27C322, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "samples", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "triplef_sp_u15.u15", 0x000000, 0x200000, CRC(98b9cafd) SHA1(3bf3971f0d9520c98fc6b1c2e77ab9c178d21c62) ) // M27C160
ROM_END

ROM_START( wldfruit ) // IGS PCB-0447-05-GM - Has IGS027A, MX10EXAQC, IGS031, Oki M6295, 3x 8-DIP banks
	ROM_REGION( 0x04000, "maincpu", 0 )
	// Internal ROM of IGS027A ARM based MCU
	ROM_LOAD( "w1.u37", 0x00000, 0x4000, NO_DUMP ) // sticker marked 'W1'

	ROM_REGION32_LE( 0x80000, "user1", 0 ) // external ARM data / prg
	ROM_LOAD( "wild_fruit_v-208us.u23", 0x000000, 0x80000, CRC(d43398f1) SHA1(ecc4bd5cb6da16b35c63b843cf7beec1ab84ed9d) ) // M27C4002

	ROM_REGION( 0x10000, "xa", 0 ) // MX10EXAQC (80C51 XA based MCU) marked J9
	ROM_LOAD( "j9.u27", 0x00000, 0x10000, CRC(3c76b157) SHA1(d8d3a434fd649577a30d5855e3fb34998041f4e5) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "wild_fruit_text.u10", 0x000000, 0x80000, CRC(d6f0fd58) SHA1(5ddae5d4df53504dbb2e0fe9f7caea961c961ef8) ) // 27C4096

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "wild_fruit_cg.u19",  0x000000, 0x400000, CRC(119686a8) SHA1(22583c1a1018cfdd20f0ef696d91fa1f6e01ab00) ) // M27C322, FIXED BITS (xxxxxxx0xxxxxxxx)
	// u18 not populated

	ROM_REGION( 0x200000, "samples", 0 ) // plain Oki M6295 samples
	ROM_LOAD( "wild_fruit_sp.u15", 0x000000, 0x200000, CRC(9da3e9dd) SHA1(7e447492713549e6be362d4aca6d223dad20771a) ) // M27C160
ROM_END


void igs_m027xa_state::pgm_create_dummy_internal_arm_region()
{
	u16 *temp16 = (u16 *)memregion("maincpu")->base();

	// fill with RX 14
	for (int i = 0; i < 0x4000 / 2; i += 2)
	{
		temp16[i] = 0xff1e;
		temp16[ i +1] = 0xe12f;

	}

	// jump straight to external area
	temp16[(0x0000) / 2] = 0xd088;
	temp16[(0x0002) / 2] = 0xe59f;
	temp16[(0x0004) / 2] = 0x0680;
	temp16[(0x0006) / 2] = 0xe3a0;
	temp16[(0x0008) / 2] = 0xff10;
	temp16[(0x000a) / 2] = 0xe12f;
	temp16[(0x0090) / 2] = 0x0400;
	temp16[(0x0092) / 2] = 0x1000;
}


void igs_m027xa_state::init_hauntedh()
{
	hauntedh_decrypt(machine());
	//m_igs017_igs031->sdwx_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027xa_state::init_crzybugs()
{
	crzybugs_decrypt(machine());
	m_igs017_igs031->sdwx_gfx_decrypt();
	m_igs017_igs031->tarzan_decrypt_sprites(0, 0);
}

void igs_m027xa_state::init_crzybugsj()
{
	crzybugsj_decrypt(machine());
	//qlgs_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

void igs_m027xa_state::init_tripfev()
{
	tripfev_decrypt(machine());
	m_igs017_igs031->sdwx_gfx_decrypt();
	m_igs017_igs031->tarzan_decrypt_sprites(0, 0);
}

void igs_m027xa_state::init_wldfruit()
{
	wldfruit_decrypt(machine());
	//qlgs_gfx_decrypt(machine());
	pgm_create_dummy_internal_arm_region();
}

} // anonymous namespace

// These use the MX10EXAQC (80c51XA from Philips) and maybe don't belong in here
// the PCBs are closer to igs_fear.cpp in terms of layout
GAME( 2008, haunthig,  0,        igs_mahjong_xa, base,     igs_m027xa_state, init_hauntedh,  ROT0, "IGS", "Haunted House (IGS, V109US)", MACHINE_IS_SKELETON ) // IGS FOR V109US 2008 10 14
GAME( 2006, haunthiga, haunthig, igs_mahjong_xa, base,     igs_m027xa_state, init_hauntedh,  ROT0, "IGS", "Haunted House (IGS, V101US)", MACHINE_IS_SKELETON ) // IGS FOR V101US 2006 08 23
GAME( 2009, crzybugs,  0,        igs_mahjong_xa, base,     igs_m027xa_state, init_crzybugs,  ROT0, "IGS", "Crazy Bugs (V204US)", MACHINE_IS_SKELETON ) // IGS FOR V204US 2009 5 19
GAME( 2006, crzybugsa, crzybugs, igs_mahjong_xa, base,     igs_m027xa_state, init_crzybugs,  ROT0, "IGS", "Crazy Bugs (V202US)", MACHINE_IS_SKELETON ) // IGS FOR V100US 2006 3 29 but also V202US string
GAME( 2005, crzybugsb, crzybugs, igs_mahjong_xa, base,     igs_m027xa_state, init_crzybugs,  ROT0, "IGS", "Crazy Bugs (V200US)", MACHINE_IS_SKELETON ) // FOR V100US 2005 7 20 but also V200US string
GAME( 2007, crzybugsj, crzybugs, igs_mahjong_xa, base,     igs_m027xa_state, init_crzybugsj, ROT0, "IGS", "Crazy Bugs (V103JP)", MACHINE_IS_SKELETON ) // IGS FOR V101JP 2007 06 08
GAME( 2006, tripfev,   0,        igs_mahjong_xa, base,     igs_m027xa_state, init_tripfev,   ROT0, "IGS", "Triple Fever (V107US)", MACHINE_IS_SKELETON ) // IGS FOR V107US 2006 09 07
GAME( 200?, wldfruit,  0,        igs_mahjong_xa, base,     igs_m027xa_state, init_wldfruit,  ROT0, "IGS", "Wild Fruit (V208US)", MACHINE_IS_SKELETON ) // IGS-----97----V208US
