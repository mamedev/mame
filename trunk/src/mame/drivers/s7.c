/*
    Williams System 7
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class williams_s7_state : public driver_device
{
public:
	williams_s7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( williams_s7_map, AS_PROGRAM, 8, williams_s7_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( williams_s7 )
INPUT_PORTS_END

void williams_s7_state::machine_reset()
{
}

static DRIVER_INIT( williams_s7 )
{
}

static MACHINE_CONFIG_START( williams_s7, williams_s7_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(williams_s7_map)
MACHINE_CONFIG_END

/*----------------------------
/ Barracora- Sys.7 (Game #510)
/----------------------------*/
ROM_START(barra_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(522e944e) SHA1(0fa17b7912f8129e40de5fed8c3ccccc0a2a9366) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(2a0e0171) SHA1(f1f2d4c1baed698d3b7cf2e88a2c28056e859920) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound4.716", 0x7800, 0x0800, CRC(67ea12e7) SHA1(f81e97183442736d5766a7e5e074bc6539e8ced0))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Black Knight - Sys.7 (Game #500)
/----------------------------*/
ROM_START(bk_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(fcbe3d44) SHA1(92ec4d41beea205ba29530624b68dd1139053535) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(104b78da) SHA1(c3af2563b3b380fe0e154b737799f6beacf8998c) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.716", 0x7800, 0x0800, CRC(6d454c0e) SHA1(21640b9ed3bdbae8bf27629891f355304e467c64))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("speech7.532", 0x3000, 0x1000, CRC(c7e229bf) SHA1(3b2ab41031f507963af828639f1690dc350737af))
	ROM_RELOAD( 0xb000, 0x1000)
	ROM_LOAD("speech5.532", 0x4000, 0x1000, CRC(411bc92f) SHA1(6c8d26fd13ed5eeba5cc40886d39c65a64beb377))
	ROM_RELOAD( 0xc000, 0x1000)
	ROM_LOAD("speech6.532", 0x5000, 0x1000, CRC(fc985005) SHA1(9df4ad12cf98a5a92b8f933e6b6788a292c8776b))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("speech4.532", 0x6000, 0x1000, CRC(f36f12e5) SHA1(24fb192ad029cd35c08f4899b76d527776a4895b))
	ROM_RELOAD( 0xe000, 0x1000)
ROM_END

ROM_START(bk_f4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(fcbe3d44) SHA1(92ec4d41beea205ba29530624b68dd1139053535) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(104b78da) SHA1(c3af2563b3b380fe0e154b737799f6beacf8998c) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.716", 0x7800, 0x0800, CRC(6d454c0e) SHA1(21640b9ed3bdbae8bf27629891f355304e467c64))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("speech7f.532", 0x3000, 0x1000, CRC(01debff6) SHA1(dc02199b63ae3309fdac819985f7a40010831634))
	ROM_RELOAD( 0xb000, 0x1000)
	ROM_LOAD("speech5f.532", 0x4000, 0x1000, CRC(2d310dce) SHA1(ad2ad3844659787ee9be4db50b17b8af6f5d0d42))
	ROM_RELOAD( 0xc000, 0x1000)
	ROM_LOAD("speech6f.532", 0x5000, 0x1000, CRC(96bb719b) SHA1(d602129ce1af1902e46ca26645a9a51324a788d0))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("speech4f.532", 0x6000, 0x1000, CRC(8ee8fc3c) SHA1(ba7c00f16bdbd7413cec025c28f8b7e7bbcb12bb))
	ROM_RELOAD( 0xe000, 0x1000)
ROM_END

ROM_START(bk_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bkl3_14.bin", 0xe000, 0x0800, CRC(74c37e4f) SHA1(8946b110901d0660676fba0c204aa2bc78223508) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("bkl3_26.bin", 0xd800, 0x0800, CRC(6acc34a0) SHA1(3adad61d27e6416630f96554687bb66d3016166a) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.716", 0x7800, 0x0800, CRC(6d454c0e) SHA1(21640b9ed3bdbae8bf27629891f355304e467c64))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("speech7.532", 0x3000, 0x1000, CRC(c7e229bf) SHA1(3b2ab41031f507963af828639f1690dc350737af))
	ROM_RELOAD( 0xb000, 0x1000)
	ROM_LOAD("speech5.532", 0x4000, 0x1000, CRC(411bc92f) SHA1(6c8d26fd13ed5eeba5cc40886d39c65a64beb377))
	ROM_RELOAD( 0xc000, 0x1000)
	ROM_LOAD("speech6.532", 0x5000, 0x1000, CRC(fc985005) SHA1(9df4ad12cf98a5a92b8f933e6b6788a292c8776b))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("speech4.532", 0x6000, 0x1000, CRC(f36f12e5) SHA1(24fb192ad029cd35c08f4899b76d527776a4895b))
	ROM_RELOAD( 0xe000, 0x1000)
ROM_END

/*-----------------------------------
/ Cosmic Gunfight - Sys.7 (Game #502)
/-----------------------------------*/
ROM_START(csmic_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(ac66c0dc) SHA1(9e2ac0e956008c2d56ffd564c983e127bc4af7ae) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(a259eba0) SHA1(0c5acae3beacb8abb0160dd8a580d3514ca557fe) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.716", 0x7800, 0x0800, CRC(af41737b) SHA1(8be4e7cebe5a821e859550c0350f0fc9cc00b2a9))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

 /*----------------------------
/ Defender - Sys.7 (Game #517)
/----------------------------*/
// Multiplex solenoid requires custom solenoid handler.
ROM_START(dfndr_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic20.532", 0xd000, 0x1000, CRC(e99e64a2) SHA1(a6cde9cb771063778cae706c740b73ce9bce9aa5))
	ROM_RELOAD( 0x5000, 0x1000)
	ROM_LOAD("ic14.532", 0xe000, 0x1000, CRC(959ec419) SHA1(f400d3a1feba0e149d24f4e1a8d240fe900b3f0b))
	ROM_RELOAD( 0x6000, 0x1000)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165))
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.716", 0x7800, 0x0800, CRC(cabaec58) SHA1(9605a1c299ed109a4ebcfa7ed6985ecc815c9e0c))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END


/*--------------------------------
/ Firepower II- Sys.7 (Game #521)
/-------------------------------*/
ROM_START(fpwr2_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(a29688dd) SHA1(83815154bbaf51dd789112664d772a876efee3da) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(1068939d) SHA1(f15c3a149bafee6d74e359399de88fd122b93441) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound3.716", 0x7800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------
/ Hyperball - Sys.7 - (Game #509)
/-------------------------------*/
ROM_START(hypbl_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic20.532", 0xd000, 0x1000, CRC(d13962e8) SHA1(e23310be100060c9803682680066b965aa5efb16))
	ROM_RELOAD( 0x5000, 0x1000)
	ROM_LOAD("ic14.532", 0xe000, 0x1000, CRC(8090fe71) SHA1(0f1f40c0ee8da5b2fd51efeb8be7c20d6465239e))
	ROM_RELOAD( 0x6000, 0x1000)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(6f4c0c4c) SHA1(1036067e2c85da867983e6e51ee2a7b5135000df))
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.532", 0x7000, 0x1000, CRC(06051e5e) SHA1(f0ab4be812ceaf771829dd549f2a612156102a93))
	ROM_RELOAD( 0xf000, 0x1000)
ROM_END

/*---------------------------
/ Joust - Sys.7 (Game #519)
/--------------------------*/
ROM_START(jst_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(c4cae4bf) SHA1(ff6e48364561402b16e40a41fa1b89e7723dd38a) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(63eea5d8) SHA1(55c26ee94809f087bd886575a5e47efc93160190) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.532", 0x7000, 0x1000, CRC(3bbc90bf) SHA1(82154e719ceca5c72d1ab034bc4ff5e3ebb36832))
	ROM_RELOAD( 0xf000, 0x1000)
ROM_END


/*--------------------------------
/ Jungle Lord - Sys.7 (Game #503)
/--------------------------------*/
ROM_START(jngld_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(6e5a6374) SHA1(738ecef807de9fee6fd1e832b35511c11173914c) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(4714b1f1) SHA1(01f8593a926df69fb8ae79260f11c5f6b868cd51) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound3.716", 0x7800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("speech7.532", 0x3000, 0x1000, CRC(83ffb695) SHA1(f9151bdfdefd5c178ca7eb5122f62b700d64f41a))
	ROM_RELOAD( 0xb000, 0x1000)
	ROM_LOAD("speech5.532", 0x4000, 0x1000, CRC(754bd474) SHA1(c05f48bb07085683de469603880eafd28dffd9f5))
	ROM_RELOAD( 0xc000, 0x1000)
	ROM_LOAD("speech6.532", 0x5000, 0x1000, CRC(f2ac6a52) SHA1(5b3e743eac382d571fd049f92ea9955342b9ffa0))
	ROM_RELOAD( 0xd000, 0x1000)
ROM_END

/*---------------------------
/ Laser Cue - Sys.7 (Game #520)
/--------------------------*/
ROM_START(lsrcu_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(39fc350d) SHA1(46e95f4016907c21c69472e6ef4a68a9adc3be77) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(db4a09e7) SHA1(5ea454c852303e12cc606c2c1e403b72e0a99f25) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.716", 0x7800, 0x0800, CRC(1888c635) SHA1(5dcdaee437a69c6027c24310f0cd2cae4e89fa05))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*--------------------------------
/ Pharaoh - Sys.7 (Game #504)
/--------------------------------*/
ROM_START(pharo_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(cef00088) SHA1(e0c6b776eddc060c42a483de6cc96a1c9f2afcf7) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(2afbcd1f) SHA1(98bb3a74548b7d9c5d7b8432369658ed32e8be07) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.716", 0x7800, 0x0800, CRC(b0e3a04b) SHA1(eac54376fe77acf46e485ab561a01220910c1fd6))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("speech7.532", 0x3000, 0x1000, CRC(e087f8a1) SHA1(49c2ad60d82d02f0529329f7cb4b57339d6546c6))
	ROM_RELOAD( 0xb000, 0x1000)
	ROM_LOAD("speech5.532", 0x4000, 0x1000, CRC(d72863dc) SHA1(e24ad970ed202165230fab999be42bea0f861fdd))
	ROM_RELOAD( 0xc000, 0x1000)
	ROM_LOAD("speech6.532", 0x5000, 0x1000, CRC(d29830bd) SHA1(88f6c508f2a7000bbf6c9c26e1029cf9a241d5ca))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("speech4.532", 0x6000, 0x1000, CRC(9ecc23fd) SHA1(bf5947d186141504fd182065533d4efbfd27441d))
	ROM_RELOAD( 0xe000, 0x1000)
ROM_END

/*-----------------------------------
/ Solar Fire - Sys.7 (Game #507)
/-----------------------------------*/
ROM_START(solar_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(cec19a55) SHA1(a1c0f7cc36e5fc7be4e8bcc80896f77eb4c23b1a) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(b667ee32) SHA1(bb4b5270d9cd36207b68e8c6883538d08aae1778) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.716", 0x7800, 0x0800, CRC(05a2230c) SHA1(c57cd7628310aa8f68ca24217aad1ead066a1a82))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-----------------------------
/ Star Light - Sys.7 (Game #530)
/-----------------------------*/
ROM_START(strlt_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic20.532", 0xd000, 0x1000, CRC(66876b56) SHA1(6fab43fbb67c7b602ca595c20a41fc1553afdb65))
	ROM_RELOAD( 0x5000, 0x1000)
	ROM_LOAD("ic14.532", 0xe000, 0x1000, CRC(292f1c4a) SHA1(0b5d50331364655672be16236d38d72b28f6dec2))
	ROM_RELOAD( 0x6000, 0x1000)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(a43d8518) SHA1(fb2289bb7380838d0d817e78c39e5bcb2709373f))
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound3.716", 0x7800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-----------------------------
/ Time Fantasy - Sys.7 (Game #515)
/-----------------------------*/
ROM_START(tmfnt_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(56b8e5ad) SHA1(84d6ab59032282cdccb3bdce0365c1fc766d0e5b) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(0f86947c) SHA1(e775f44b4ca5dae5ec2626fa84fae83c4f0c5c33) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound3.716", 0x7800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Varkon- Sys.7 (Game #512)
/----------------------------*/
ROM_START(vrkon_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(3baba324) SHA1(522654e0d81458d8b31150dcb0cb53c29b334358) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(df20330c) SHA1(22157c6480ad38b9c53c390f5e7bfa63a8abd0e8) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.716", 0x7800, 0x0800, CRC(d13db2bb) SHA1(862546bbdd1476906948f7324b7434c29df79baa))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*----------------------------
/ Warlok- Sys.7 (Game #516)
/----------------------------*/
ROM_START(wrlok_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic14.716", 0xe000, 0x0800, CRC(291be241) SHA1(77fffa878f760583ef152a7939867621a61d58dc) )
	ROM_RELOAD( 0x6000, 0x0800)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165) )
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_LOAD("ic20.716", 0xe800, 0x0800, CRC(dfb4b75a) SHA1(bcf017b01236f755cee419e398bbd8955ae3576a) )
	ROM_RELOAD( 0x6800, 0x0800)
	ROM_LOAD("ic26.716", 0xd800, 0x0800, CRC(44f8b507) SHA1(cdd8455c1e34584e8f1b75d430b8b37d4dd7dff0) )
	ROM_RELOAD( 0x5800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.716", 0x7800, 0x0800, CRC(5d8e46d6) SHA1(68f8760ad85b8ada81f6ed00eadb9daf37191c53))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-----------------------------------
/ Thunderball - Sys.7 (Game #508) - Prototype
/-----------------------------------*/
ROM_START(thund_p1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic20.532", 0xd000, 0x1000, CRC(aa3f07dc) SHA1(f31662972046f9a874380a8dcd1bc9259de5f6ba))
	ROM_RELOAD( 0x5000, 0x1000)
	ROM_LOAD("ic14.532", 0xe000, 0x1000, CRC(1cd34f1f) SHA1(3f5b5a319570c26a3d34d640fef2ac6c04b83b70))
	ROM_RELOAD( 0x6000, 0x1000)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(bb571a17) SHA1(fb0b7f247673dae0744d4188e1a03749a2237165))
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound12.532", 0x7000, 0x1000, CRC(cc70af52) SHA1(d9c2840acdcd69aab39fc647dd4819eccc06af33))
	ROM_RELOAD( 0xf000, 0x1000)
	ROM_LOAD("speech7.532", 0x3000, 0x1000, CRC(33e1b041) SHA1(f50c0311bde69fa6e8071e297a81cc3ef3dcf44f))
	ROM_RELOAD( 0xb000, 0x1000)
	ROM_LOAD("speech5.532", 0x4000, 0x1000, CRC(11780c80) SHA1(bcc5efcd69b4f776feef32484a872863847d64cd))
	ROM_RELOAD( 0xc000, 0x1000)
	ROM_LOAD("speech6.532", 0x5000, 0x1000, CRC(ab688698) SHA1(e0cbac44a6fe30a49da478c32500a0b43903cc2b))
	ROM_RELOAD( 0xd000, 0x1000)
	ROM_LOAD("speech4.532", 0x6000, 0x1000, CRC(2a4d6f4b) SHA1(e6f8a1a6e6abc81f980a4938d98abb250e8e1e3b))
	ROM_RELOAD( 0xe000, 0x1000)
ROM_END

/*-----------------------------
/ Rat Race - Sys.7 (Game #527)- Prototype
/-----------------------------*/
ROM_START(ratrc_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ic20.532", 0xd000, 0x1000, CRC(0c5c7c09) SHA1(c93b39ba1460feee5850fcd3ca7cacb72c4c8ff3))
	ROM_RELOAD( 0x5000, 0x1000)
	ROM_LOAD("ic14.532", 0xe000, 0x1000, CRC(c6f4bcf4) SHA1(d71c86299139abe3dd376a324315a039be82875c))
	ROM_RELOAD( 0x6000, 0x1000)
	ROM_LOAD("ic17.532", 0xf000, 0x1000, CRC(0800c214) SHA1(3343c07fd550bb0759032628e01bb750135dab15))
	ROM_RELOAD( 0x7000, 0x1000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("b486.bin", 0xe000, 0x2000, CRC(c54b9402) SHA1(c56fc5f105fc2c1166e3b22bb09b72af79e0aec1))
	ROM_RELOAD(0xc000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
ROM_END


GAME(1982,	vrkon_l1,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Varkon (L-1)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1981,	barra_l1,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Barracora (L-1)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	bk_l4,		0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Black Knight (L-4)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	bk_f4,		bk_l4,		williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Black Knight (L-4, French speech)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	bk_l3,		bk_l4,		williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Black Knight (L-3)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1980,	csmic_l1,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Cosmic Gunfight (L-1)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1982,	dfndr_l4,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Defender (L-4)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1983,	fpwr2_l2,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Firepower II (L-2)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1981,	hypbl_l4,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"HyperBall (L-4)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1983,	jst_l2,		0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Joust (L-2)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1981,	jngld_l2,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Jungle Lord (L-2)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1983,	lsrcu_l2,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Laser Cue (L-2)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1981,	pharo_l2,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Pharaoh (L-2)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1981,	solar_l2,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Solar Fire (L-2)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1984,	strlt_l1,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Star Light (L-1)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1982,	tmfnt_l5,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Time Fantasy (L-5)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1982,	wrlok_l3,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Warlok (L-3)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1982,	thund_p1,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Thunderball (P-1)",				GAME_IS_SKELETON_MECHANICAL)
GAME(1983,	ratrc_l1,	0,			williams_s7,	williams_s7,	williams_s7,	ROT0,	"Williams",				"Rat Race (L-1)",				GAME_IS_SKELETON_MECHANICAL)
