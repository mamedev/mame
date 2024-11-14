// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "cpu/i8085/i8085.h"
#include "cpu/i86/i186.h"


namespace {

class bingo_state : public driver_device
{
public:
	bingo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void bingo(machine_config &config);
	void bingo_map(address_map &map) ATTR_COLD;
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override ATTR_COLD;
public:
	void init_bingo();
};


void bingo_state::bingo_map(address_map &map)
{
	map(0x0000, 0x7fff).noprw();
	map(0x0000, 0x1eff).rom();
	map(0x1f00, 0x1fff).ram();
}

static INPUT_PORTS_START( bingo )
INPUT_PORTS_END

void bingo_state::machine_reset()
{
}

void bingo_state::init_bingo()
{
}

void bingo_state::bingo(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, 1000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &bingo_state::bingo_map);
}

class seeben_state : public driver_device
{
public:
	seeben_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void seeben(machine_config &config);
	void seeben_map(address_map &map) ATTR_COLD;
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override ATTR_COLD;
public:
	void init_seeben();
};


void seeben_state::seeben_map(address_map &map)
{
}

static INPUT_PORTS_START( seeben )
INPUT_PORTS_END

void seeben_state::machine_reset()
{
}

void seeben_state::init_seeben()
{
}

void seeben_state::seeben(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 1000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &seeben_state::seeben_map);
}

class splin_state : public driver_device
{
public:
	splin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void splin(machine_config &config);
	void splin_map(address_map &map) ATTR_COLD;
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override ATTR_COLD;
public:
	void init_splin();
};

void splin_state::splin_map(address_map &map)
{
	map(0x00000, 0x0bfff).ram();
	map(0x0d900, 0x0d9ff).ram();
	map(0xe0000, 0xfffff).rom();
}

static INPUT_PORTS_START( splin )
INPUT_PORTS_END

void splin_state::machine_reset()
{
}

void splin_state::init_splin()
{
}

void splin_state::splin(machine_config &config)
{
	/* basic machine hardware */
	I80186(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &splin_state::splin_map);
}

ROM_START(cntinntl)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("bingo.u37", 0x1800, 0x0800, CRC(3b21b22c) SHA1(21b002dd0dd11ee55674955c67c627470f427591))
	ROM_LOAD("bingo.u40", 0x1000, 0x0800, CRC(67160fc8) SHA1(6b93c1a7edcd7079a1e7d8a926e72febe2b39e9e))
	ROM_LOAD("bingo.u44", 0x0800, 0x0800, CRC(068acc49) SHA1(34fa2977513276bd5adc0b06cf258bb5a3702ed2))
	ROM_LOAD("bingo.u48", 0x0000, 0x0800, CRC(81bbcb19) SHA1(17c3d900d1cbe3cb5332d830288ef2c578afe8f8))
ROM_END

ROM_START(cntinntl2)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("u36.bin", 0x1800, 0x0800, CRC(205cca08) SHA1(ae21794a63f1c50e3c7239275f7a58caf701a7bc))
	ROM_LOAD("bingo.u40", 0x1000, 0x0800, CRC(67160fc8) SHA1(6b93c1a7edcd7079a1e7d8a926e72febe2b39e9e))
	ROM_LOAD("bingo.u44", 0x0800, 0x0800, CRC(068acc49) SHA1(34fa2977513276bd5adc0b06cf258bb5a3702ed2))
	ROM_LOAD("u48.bin", 0x0000, 0x0800, CRC(8fda0bf9) SHA1(ea2926bb2c1cc394a060d88cc6ef53b7cf39790b))
ROM_END

ROM_START(goldgame)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("h9925_1.e", 0x80000, 0x10000, CRC(c5ec9181) SHA1(fac7fc0fbfddca44c728c78973ee5273a3d0bc43))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE("h9925_1.o", 0x80001, 0x10000, CRC(2a019eea) SHA1(3f013f97b0a92fc9085c7be3903cbf42e67c41e5))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(goldgkitb)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("ah0127.evn", 0x80000, 0x10000, CRC(6456a021) SHA1(98137d3b63aa7453c624f477a0c6ea1e0996d3c2))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE("ah0127.ods", 0x80001, 0x10000, CRC(b538f435) SHA1(4d939554e997d630ffe7337e1f21ee53d6f06130))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(goldgstake)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "h0127.evn", 0x80000, 0x10000, CRC(477ddee2) SHA1(a4d16b44ee43838120fbc1e4642867c3d375fe5f))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE( "h0127.ods", 0x80001, 0x10000, CRC(89aea35a) SHA1(65bb5c5448de05180d0fd5b593f783b860de5b7c))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(goldgnew)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "h018.e", 0x80000, 0x10000, CRC(0b209318) SHA1(56e36d6672820f5610dfdcb6dc93c3aa92286992))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE( "h018.o", 0x80001, 0x10000, CRC(d7ec2522) SHA1(5c09be69db1338483b7b65193f29b5bea4fb6195))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(goldgkit1)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "v87k.e", 0x80000, 0x10000, CRC(57f4f0c4) SHA1(7b8cb888a55a2aa46e7737a8dc44b2f983a189c0))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE( "v87k.o", 0x80001, 0x10000, CRC(6178d4e5) SHA1(6d4c8056324157c448963761d7f49ce80e42d912))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(michkit1)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "ac001.e",   0x80000, 0x10000, CRC(c92af347) SHA1(7d46408b37a7f88232fc87b3289beb244a94390c))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE( "ac001.o",   0x80001, 0x10000, CRC(e321600a) SHA1(3e99a162a34d3ca28f6ca33ee442820bae8a1574))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(michkitb)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "ac0127.evn",0x80000, 0x10000, CRC(cce266d0) SHA1(e45df0bb58758e727767965cc0edcee0f25ce97e))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE( "ac0127.ods",0x80001, 0x10000, CRC(1cde7f17) SHA1(dbfe9b94f768e24e58ec19d0152e98bfa2965b6f))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(michstake)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "c0128.evn", 0x80000, 0x10000, CRC(68af141a) SHA1(0e94f70c2fd74bfc5829f19ba502e9b288432685))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE( "c0128.ods", 0x80001, 0x10000, CRC(ff2752f6) SHA1(bd6c45ac0a533aeb5930b5a1705152eec704b5e9))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(michnew)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "c017.e",    0x80000, 0x10000, CRC(8a1cf5d7) SHA1(aad87d13503753b13dbdae2a2792afa004747e6e))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE( "c017.o",    0x80001, 0x10000, CRC(0bfdff4d) SHA1(5dacaa689ae7eaba5f28d88a8d9f3d6de0421744))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(michigan)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "c9925_1.e", 0x80000, 0x10000, CRC(ab42e3b8) SHA1(917b5a7a005baf6bae676e54a0292e32d11a7df1))
	ROM_RELOAD(0xe0000, 0x10000)
	ROM_LOAD16_BYTE( "c9925_1.o", 0x80001, 0x10000, CRC(7a0d6c70) SHA1(1d410b9f5df69cc9cbf17dbc9c73fee928e167d7))
	ROM_RELOAD(0xe0001, 0x10000)
ROM_END

ROM_START(montana)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "m0128.e", 0x80000, 0x20000, CRC(51a56929) SHA1(4a1d9939ff441f82661e1adcb0d698061f383429))
	ROM_RELOAD(0xc0000, 0x20000)
	ROM_LOAD16_BYTE( "m0128.o", 0x80001, 0x20000, CRC(03431945) SHA1(da441895f3f6db9e573fcb5de8e287e65cc9a00d))
	ROM_RELOAD(0xc0001, 0x20000)
ROM_END

ROM_START(topgame)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "v252l10.p0", 0x0000, 0x8000, CRC(d3f71f05) SHA1(2bbafeee2e5eda6ff4ed8c7d52f2bb33c50f398c))
	ROM_LOAD( "v252l10.p1", 0x8000, 0x8000, CRC(f98531d1) SHA1(63ae30821788dccbaa0749db55cd85f5a7a609bf))
ROM_END

ROM_START(topgamet)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "v252tr.p0", 0x0000, 0x8000, CRC(c947cccc) SHA1(37837ec030b2e86109d40fc19c24fc6aa73a272c))
	ROM_LOAD( "v252tr.p1", 0x8000, 0x8000, CRC(00a3ee14) SHA1(5ebf2d0ea891e365f5bd1cc03f0bd913a638b49b))
ROM_END

ROM_START(penalty)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("13006-1.epr", 0x8000, 0x8000, CRC(93cfbec9) SHA1(c245604ac42c88c647950db4497a6f9dd3504955))
	ROM_LOAD("13006-2.epr", 0x0000, 0x4000, CRC(41470cc1) SHA1(7050df563fddbe8216317d96664d12567b618645))
ROM_END

ROM_START(brooklyn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("n10207-1.epr", 0x8000, 0x8000, CRC(7851f870) SHA1(8da400108a352954ced8fc942663c0635bec4d1c))
	ROM_LOAD("n10207-2.epr", 0x0000, 0x4000, CRC(861dae09) SHA1(d808fbbf6b50e1482a512b9bd1b18a023694adb2))
ROM_END

ROM_START(brooklyna)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("o10307-1.epr", 0x8000, 0x8000, CRC(628ac640) SHA1(67edb424f15880e874b03028066e6c0039db21fa))
	ROM_LOAD("o10307-2.epr", 0x0000, 0x4000, CRC(c35d83ff) SHA1(e37c03e6960138cb6b628dfc6b12e484bbae96e8))
ROM_END

ROM_START(newdixie)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("10307-1.epr", 0x8000, 0x8000, CRC(7b6b2e9c) SHA1(149c9e1d2a3e7db735835c6fa795e41b2eb45175))
	ROM_LOAD("10307-2.epr", 0x0000, 0x4000, CRC(d99a7866) SHA1(659a0107bc970d2578dcfd7cdd43661da778fd5c))
ROM_END

ROM_START(superdix)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "12906-1.epr", 0x8000, 0x8000, CRC(e90a8aa5) SHA1(88dac74fb020535b535f7c4c245bbece398164ee))
	ROM_LOAD( "12906-2.epr", 0x0000, 0x4000, CRC(4875dfb4) SHA1(722bfa89f69d14e24555eea9cc975012098db25b))
ROM_END

ROM_START(cntine31)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("10107-1.epr", 0x8000, 0x8000, CRC(3b67cce3) SHA1(95f71526c236262ff985148ba7ea057f07fadbe8))
	ROM_LOAD("10107-2.epr", 0x0000, 0x4000, CRC(89d08795) SHA1(dc75502580d681d9b4dc878b0d80346bcef407ae))
ROM_END

ROM_START(domino2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("13006.epr", 0x8000, 0x8000, CRC(8ed9b2a5) SHA1(8f3e730cef3e74cb043691a111e1bf6660642a73))
ROM_END

ROM_START(ggate)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "13006-1.epr", 0x8000, 0x8000, CRC(6a451fc6) SHA1(93287937c8a679dfca1a162977a62357134673b6))
	ROM_LOAD( "13006-2.epr", 0x0000, 0x4000, CRC(217299b0) SHA1(ef3ee8811183dca43699a7c2d75fb99bc3668ae2))
ROM_END

ROM_START(ggatea)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "12906-1.epr", 0x8000, 0x8000, CRC(3792fc4c) SHA1(4ab88b6c73ce1b49e1a4d12cc9fa61c7d74ed780))
	ROM_LOAD( "12906-2.epr", 0x0000, 0x4000, CRC(a1115196) SHA1(dfa549a547b5cd7a9369d30fa1e868e6725cb3f1))
ROM_END

ROM_START(tripjok)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "13006-1", 0x8000, 0x8000, CRC(5682ac90) SHA1(c9fa13c56e9178eb861991fcad6b09fd27cca3cb))
	ROM_LOAD( "13006-2", 0x0000, 0x4000, CRC(c7104e8f) SHA1(a3737f70cb9c97df24b5da915ef53b6d30f2470d))
ROM_END

} // anonymous namespace


GAME(1980,  cntinntl,   0,        bingo,  bingo,  bingo_state,  init_bingo,  ROT0, "Bally",            "Continental (Bingo)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  cntinntl2,  cntinntl, bingo,  bingo,  bingo_state,  init_bingo,  ROT0, "Bally",            "Continental (Bingo, alternate version)",   MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  goldgame,   0,        splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Golden Game (Bingo)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  goldgkitb,  goldgame, splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Golden Game Kit Bingo Stake 6/10 (Bingo)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  goldgstake, goldgame, splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Golden Game Bingo Stake 6/10 (Bingo)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  goldgnew,   goldgame, splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Golden Game Bingo New (Bingo)",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  goldgkit1,  goldgame, splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Golden Game Kit 1 Generation (Bingo)",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  michigan,   0,        splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Michigan (Bingo)",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  michkit1,   michigan, splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Michigan Bingo Kit 1 Generation (Bingo)",  MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  michkitb,   michigan, splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Michigan Kit Bingo Stake 6/10 (Bingo)",    MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  michstake,  michigan, splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Michigan Bingo Stake 6/10 (Bingo)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  michnew,    michigan, splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Michigan Bingo New (Bingo)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  montana,    0,        splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Montana Bingo Stake 6/10 (Bingo)",         MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  topgame,    0,        splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Top Game Laser L10 (Bingo)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  topgamet,   topgame,  splin,  splin,  splin_state,  init_splin,  ROT0, "Splin",            "Top Game Turbo (Bingo)",                   MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  penalty,    0,        seeben, seeben, seeben_state, init_seeben, ROT0, "Seeben (Belgium)", "Penalty (Bingo)",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  brooklyn,   0,        seeben, seeben, seeben_state, init_seeben, ROT0, "Seeben (Belgium)", "Brooklyn (set 1) (Bingo)",                 MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  brooklyna,  brooklyn, seeben, seeben, seeben_state, init_seeben, ROT0, "Seeben (Belgium)", "Brooklyn (set 2) (Bingo)",                 MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  newdixie,   0,        seeben, seeben, seeben_state, init_seeben, ROT0, "Seeben (Belgium)", "New Dixieland (Bingo)",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  superdix,   0,        seeben, seeben, seeben_state, init_seeben, ROT0, "Seeben (Belgium)", "Super Dixieland (Bingo)",                  MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  cntine31,   0,        seeben, seeben, seeben_state, init_seeben, ROT0, "Seeben (Belgium)", "Continental 3 in 1 (Bingo)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  domino2,    0,        seeben, seeben, seeben_state, init_seeben, ROT0, "Seeben (Belgium)", "Domino II (Bingo)",                        MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  tripjok,    0,        seeben, seeben, seeben_state, init_seeben, ROT0, "Seeben (Belgium)", "Triple Joker (Bingo)",                     MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  ggate,      0,        seeben, seeben, seeben_state, init_seeben, ROT0, "Seeben (Belgium)", "Golden Gate (set 1) (Bingo)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??,  ggatea,     ggate,    seeben, seeben, seeben_state, init_seeben, ROT0, "Seeben (Belgium)", "Golden Gate (set 2) (Bingo)",              MACHINE_IS_SKELETON_MECHANICAL)
