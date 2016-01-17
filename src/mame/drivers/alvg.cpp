// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic

#include "emu.h"
#include "cpu/m6502/m65c02.h"

class alvg_state : public driver_device
{
public:
	alvg_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
public:
	DECLARE_DRIVER_INIT(alvg);
};


static ADDRESS_MAP_START( alvg_map, AS_PROGRAM, 8, alvg_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( alvg )
INPUT_PORTS_END

void alvg_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(alvg_state,alvg)
{
}

static MACHINE_CONFIG_START( alvg, alvg_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65C02, 2000000)
	MCFG_CPU_PROGRAM_MAP(alvg_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ A.G. Soccer Ball
/-------------------------------------------------------------------*/
ROM_START(agsoccer)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("agscpu1r.18u", 0x0000, 0x10000, CRC(37affcf4) SHA1(017d47f54d5b34a4b71c2f5b84ba9bdb1c924299))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("ags_snd.v21", 0x0000, 0x10000, CRC(aa30bfe4) SHA1(518f7019639a0284461e83ad849bee0be5371580))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("ags_voic.v12", 0x000000, 0x40000, CRC(bac70b18) SHA1(0a699eb95d7d6b071b2cd9d0bf73df355e2ffce8))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ Al's Garage Band Goes On A World Tour
/-------------------------------------------------------------------*/
ROM_START(wrldtour)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu27c.512", 0x0000, 0x10000, CRC(c9572fb5) SHA1(47a3e8943ef4207011a33f4a03a6e722c937cc48))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("soundc.512", 0x0000, 0x10000, CRC(b44bee01) SHA1(795d8500e5bd73ce23756bf1f5c96db1a3621a70))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("samp_0.c21", 0x000000, 0x40000, CRC(37beb831) SHA1(2b90d2be0a1bd7c59469846631d2b44bdf9f5f9d))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_1.c21", 0x100000, 0x40000, CRC(621533c6) SHA1(ca0ed9e89c340cb3b08f9a9002af9997372c1cbf))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_2.c21", 0x200000, 0x40000, CRC(454a5cca) SHA1(66b1a5832134365fd762fcba4cf4d666f60ebd65))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_3.c21", 0x300000, 0x40000, CRC(1f4928f4) SHA1(9949ab96644984fab8037224f52ec28d7d7cc967))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x110000, "gfx3", 0)
	ROM_LOAD("romdef1.c20", 0x00000, 0x40000, CRC(045b21c1) SHA1(134b7eb0f71506d12d9ded24999d530126c558fc))
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_LOAD("romdef2.c20", 0x40000, 0x40000, CRC(23c32ee5) SHA1(429b3b069251bb8b681bbc6382ceb6b85125eb79))
	ROM_RELOAD( 0xc0000, 0x40000)
	ROM_LOAD("dot27c.512", 0x100000, 0x10000, CRC(c8bd48e7) SHA1(e2dc513dd42c05c2018e6d8c0b6f0b2c56e6e059))
	ROM_REGION(0x20000, "cpu3", 0)
	ROM_COPY("gfx3",0x108000,0x0000,0x8000)
ROM_END

ROM_START(wrldtour2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu02b.512", 0x0000, 0x10000, CRC(1658bf40) SHA1(7af9eedab4e7d0cedaf8bfdbc1f27b989a7171cd))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("soundc.512", 0x0000, 0x10000, CRC(b44bee01) SHA1(795d8500e5bd73ce23756bf1f5c96db1a3621a70))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("samp_0.c21", 0x000000, 0x40000, CRC(37beb831) SHA1(2b90d2be0a1bd7c59469846631d2b44bdf9f5f9d))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_1.c21", 0x100000, 0x40000, CRC(621533c6) SHA1(ca0ed9e89c340cb3b08f9a9002af9997372c1cbf))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_2.c21", 0x200000, 0x40000, CRC(454a5cca) SHA1(66b1a5832134365fd762fcba4cf4d666f60ebd65))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_3.c21", 0x300000, 0x40000, CRC(1f4928f4) SHA1(9949ab96644984fab8037224f52ec28d7d7cc967))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x110000, "gfx3", 0)
	ROM_LOAD("romdef1.c20", 0x00000, 0x40000, CRC(045b21c1) SHA1(134b7eb0f71506d12d9ded24999d530126c558fc))
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_LOAD("romdef2.c20", 0x40000, 0x40000, CRC(23c32ee5) SHA1(429b3b069251bb8b681bbc6382ceb6b85125eb79))
	ROM_RELOAD( 0xc0000, 0x40000)
	ROM_LOAD("dot02b.512", 0x100000, 0x10000, CRC(50e3d59d) SHA1(db6df3482fc485af6bde341750bf8072a296b8da))
	ROM_REGION(0x20000, "cpu3", 0)
	ROM_COPY("gfx3",0x108000,0x0000,0x8000)
ROM_END

/*-------------------------------------------------------------------
/ Dinosaur Eggs
/-------------------------------------------------------------------*/
ROM_START(dinoeggs)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dinoeggs.512", 0x0000, 0x10000, CRC(4712f97f) SHA1(593351dcfd475e685c1e5eb2c1006769d3325c8b))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("eps071.r02", 0x0000, 0x10000, CRC(288f116c) SHA1(5d03ce66bffe39ec02173525078ff07c5005ef18))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("eps072.r02", 0x000000, 0x40000, CRC(780a4364) SHA1(d8a972debee669f0fe66c7407fbed5ef9cd2ce01))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ Mystery Castle
/-------------------------------------------------------------------*/
ROM_START(mystcast)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mcastle.cpu", 0x0000, 0x10000, CRC(936e6799) SHA1(aa29fb5f12f34c695d1556232744f65cd576a2b1))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("mcastle.102", 0x0000, 0x10000, CRC(752822d0) SHA1(36461ef03cac5aefa0c03dfdc63c3d294a3b9c09))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("mcastle.sr0", 0x000000, 0x40000, CRC(0855cc73) SHA1(c46e08432bcff24594c33171f20669ba63828931))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr1", 0x100000, 0x40000, CRC(3b5d76e0) SHA1(b2e1bca3c596eba89feda868fa56c71a6b22414c))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr2", 0x200000, 0x40000, CRC(c3ffd277) SHA1(d16d1b22089b89bbf0db7d2b66c9745a56034322))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr3", 0x300000, 0x40000, CRC(740858bb) SHA1(d2e9a0a178977dcc873368b042cea7052578df66))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x20000, "cpu3", 0)
	ROM_LOAD("mcastle.du4", 0x00000, 0x10000, CRC(686e253a) SHA1(28aff34c120c61e231e2111dc396df515bcbbb89))
	ROM_REGION(0x100000, "gfx3", 0)
	ROM_LOAD("mcastle.du5", 0x00000, 0x40000, CRC(9095c367) SHA1(9d3e9416f662ee2aad891eef059278c530448fcc))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_RELOAD( 0xc0000, 0x40000)
ROM_END

ROM_START(mystcasta)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_103.bin", 0x0000, 0x10000, CRC(70ab8ece) SHA1(2bf8cd042450968b7500552419a9af5df2589c13))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("mcastle.103", 0x0000, 0x10000, CRC(bd4849ac) SHA1(f477ea369539a65c0960be1f1c3b4c5503dd6b75))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("mcastle.sr0", 0x000000, 0x40000, CRC(0855cc73) SHA1(c46e08432bcff24594c33171f20669ba63828931))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr1", 0x100000, 0x40000, CRC(3b5d76e0) SHA1(b2e1bca3c596eba89feda868fa56c71a6b22414c))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr2", 0x200000, 0x40000, CRC(c3ffd277) SHA1(d16d1b22089b89bbf0db7d2b66c9745a56034322))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr3", 0x300000, 0x40000, CRC(740858bb) SHA1(d2e9a0a178977dcc873368b042cea7052578df66))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x20000, "cpu3", 0)
	ROM_LOAD("u4.bin", 0x00000, 0x10000, CRC(a6969efc) SHA1(82da976cb3d30d6fb1576e4c67febd7235f73f51))
	ROM_REGION(0x100000, "gfx3", 0)
	ROM_LOAD("u5.bin", 0x00000, 0x40000, CRC(e5126980) SHA1(2c6d412c87bf27098dae4351958d84e8f9348423))
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_LOAD("u6.bin", 0x40000, 0x40000, CRC(eb241633) SHA1(8e5db75b32ed2ea74088615bbe1403d4c8feafbd))
	ROM_RELOAD( 0xc0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ Pistol Poker
/-------------------------------------------------------------------*/
ROM_START(pstlpkr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("p_peteu2.512", 0x0000, 0x10000, CRC(490a1e2d) SHA1(907dd858ed948681e7366a64a0e7537ebe301d6b))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("p_pu102.512", 0x0000, 0x10000, CRC(b8fb806e) SHA1(c2dc19820ea22bbcf5808db2fb4be76a4033d6ea))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("p_parom0.c20", 0x000000, 0x40000, CRC(99986af2) SHA1(52fa7d2979f7f2d6d65ab6d4f7bbfbed16303991))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("p_parom1.c20", 0x100000, 0x40000, CRC(ae2af238) SHA1(221d3a0e3fb1daad261d723e873ef0727b88889e))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("p_parom2.c20", 0x200000, 0x40000, CRC(f39560a4) SHA1(cdfdf7b44ff4c3f9f4d39fbd8ecbf141d8568088))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("p_parom3.c20", 0x300000, 0x40000, CRC(19d5e4de) SHA1(fb59166ebf992e81b92a42898e351d8443adb1c3))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x20000, "cpu3", 0)
	ROM_LOAD("p_peteu4.512", 0x00000, 0x10000, CRC(caa0cabd) SHA1(caff6ca4a9cce4e3d846502696c8838805673261))
	ROM_REGION(0x100000, "gfx3", 0)
	ROM_LOAD("p_peteu5.c20", 0x00000, 0x40000, CRC(1d2cecd8) SHA1(6072a0f744fb9eef728fe7cf5e17d0007edbddd7))
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_LOAD("p_peteu6.c20", 0x40000, 0x40000, CRC(3a56376c) SHA1(69febc17b8416c03a58e651447bbe1e14ff27e50))
	ROM_RELOAD( 0xc0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ Punchy The Clown
/-------------------------------------------------------------------*/
ROM_START(punchy)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("epc061.r02", 0x0000, 0x10000, CRC(732fca88) SHA1(dff0aa4b856bafb95b08dae675dd2ad59e1860e1))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("eps061.r02", 0x0000, 0x10000, CRC(cfde1b9a) SHA1(cbf9e67df6a6762843272493c2caa1413f70fb27))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("eps062.r02", 0x000000, 0x40000, CRC(7462a5cd) SHA1(05141bcc91b1a786444bff7fa8ba2a785dc0d376))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ U.S.A. Football
/-------------------------------------------------------------------*/
ROM_START(usafootb)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("usa_cpu.bin", 0x0000, 0x10000, CRC(53b00873) SHA1(96812c4722026554a830c62eca64f09d25a0de82))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("usa_snd.bin", 0x0000, 0x10000, CRC(9d509cbc) SHA1(0be629945b5102adf75e88661e0f956e32ca77da))
	ROM_REGION(0x400000, "sound1", 0)
	ROM_LOAD("usa_vox.bin", 0x000000, 0x40000, CRC(baae0aa3) SHA1(7933bffcf1509ceeea58a4449268c10c9fac554c))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END


GAME(1991,  agsoccer,   0,          alvg,   alvg, alvg_state,   alvg,   ROT0,   "Alvin G",              "A.G. Soccer Ball",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  wrldtour,   0,          alvg,   alvg, alvg_state,   alvg,   ROT0,   "Alvin G",              "Al's Garage Band Goes On A World Tour",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  wrldtour2,  wrldtour,   alvg,   alvg, alvg_state,   alvg,   ROT0,   "Alvin G",              "Al's Garage Band Goes On A World Tour R02b",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  usafootb,   0,          alvg,   alvg, alvg_state,   alvg,   ROT0,   "Alvin G",              "U.S.A. Football",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  mystcast,   0,          alvg,   alvg, alvg_state,   alvg,   ROT0,   "Alvin G",              "Mystery Castle",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  mystcasta,  mystcast,   alvg,   alvg, alvg_state,   alvg,   ROT0,   "Alvin G",              "Mystery Castle (alternate set)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  pstlpkr,    0,          alvg,   alvg, alvg_state,   alvg,   ROT0,   "Alvin G",              "Pistol Poker",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  punchy,     0,          alvg,   alvg, alvg_state,   alvg,   ROT0,   "Alvin G",              "Punchy The Clown",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993,  dinoeggs,   0,          alvg,   alvg, alvg_state,   alvg,   ROT0,   "Alvin G",              "Dinosaur Eggs",                MACHINE_IS_SKELETON_MECHANICAL)
