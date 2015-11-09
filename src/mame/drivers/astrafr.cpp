// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Astra Fruit Machines
 -- Unknown 68340 based HW platform (seems related to pluto5.c?)
 -- dumps are of an unknown quality
 -- These might all be the same system with different rom configurations, or different systems (unknown)
 -- Some sets contain what look to be additional sound roms, others have them in the main program roms?

Platform also used by Lowen? (at least some of their sets use the same address line scheme)

 some Astra games require linked machines with a 'master' and shared percentage system.

*/

#include "emu.h"
#include "machine/68340.h"



class astrafr_state : public driver_device
{
public:
	astrafr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_slavecpu(*this, "slavecpu")

	{
		fgpa_first_read_addr = 0xffff;
		fgpa_rom_write_addr = 0xffff;
		fgpa_after_rom_write_addr = 0xffff;
	}

	UINT32* m_cpuregion;
	int  m_cpuregion_size;
	UINT32* m_mainram;

	UINT32* m_slavecpuregion;
	int  m_slavecpuregion_size;
	UINT32* m_slaveram;



	DECLARE_READ32_MEMBER(astrafr_mem_r);
	DECLARE_WRITE32_MEMBER(astrafr_mem_w);

	DECLARE_READ32_MEMBER(astrafr_slave_mem_r);
	DECLARE_WRITE32_MEMBER(astrafr_slave_mem_w);


	// ports move above from game to game..
	UINT16 fgpa_rom_write_addr;
	UINT16 fgpa_first_read_addr;
	UINT16 fgpa_after_rom_write_addr;

	DECLARE_READ8_MEMBER( astra_fgpa_r )
	{
		int pc = space.device().safe_pc();

		if (offset==fgpa_first_read_addr)
		{
			return 0xff;
		}
		else
		{
			logerror("%08x astra_fgpa_r offset %02x\n", pc, offset);
			return 0xff;
		}
	}

	DECLARE_WRITE8_MEMBER( astra_fgpa_w )
	{
		int pc = space.device().safe_pc();

		if (offset==fgpa_rom_write_addr)
		{
			// the games write most of the ROM data to a port
			// I think it kicks the FPGA into life?
		}
		else
		{
			logerror("%08x astra_fgpa_w offset %02x %02x\n", pc, offset, data);
		}
	}

	/* 2nd copy for the 2nd board (assume same addresses for now */
	DECLARE_READ8_MEMBER( astra_fgpa_slave_r )
	{
		int pc = space.device().safe_pc();

		if (offset==fgpa_first_read_addr)
		{
			return 0xff;
		}
		else
		{
			logerror("%08x astra_fgpa_slave_r offset %02x\n", pc, offset);
			return 0xff;
		}
	}

	DECLARE_WRITE8_MEMBER( astra_fgpa_slave_w )
	{
		int pc = space.device().safe_pc();

		if (offset==fgpa_rom_write_addr)
		{
			// the games write most of the ROM data to a port
			// I think it kicks the FPGA into life?
		}
		else
		{
			logerror("%08x astra_fgpa_slave_w offset %02x %02x\n", pc, offset, data);
		}
	}

	// devices
	required_device<m68340cpu_device> m_maincpu;
	optional_device<m68340cpu_device> m_slavecpu;

	DECLARE_DRIVER_INIT(astradec_sml);
	DECLARE_DRIVER_INIT(astradec);
	DECLARE_DRIVER_INIT(astradec_dual);
	DECLARE_DRIVER_INIT(astradec_sml_dual);
	DECLARE_MACHINE_START(astra_common);
	DECLARE_MACHINE_START(astra_2e);
	DECLARE_MACHINE_START(astra_37);
	DECLARE_MACHINE_START(astra_57);
};



READ32_MEMBER(astrafr_state::astrafr_mem_r)
{
	int pc = space.device().safe_pc();
	int cs = m68340_get_cs(m_maincpu, offset * 4);

	switch ( cs )
	{
		case 1:
			if (offset<m_cpuregion_size)
				return m_cpuregion[offset];
			else
				return 0x0000;

		case 2:
			offset &= 0x3fff;
			return m_mainram[offset];

		default:
			logerror("%08x maincpu read access offset %08x mem_mask %08x cs %d\n", pc, offset*4, mem_mask, cs);

	}

	return 0x0000;
}



WRITE32_MEMBER(astrafr_state::astrafr_mem_w)
{
	int pc = space.device().safe_pc();
	int address = offset * 4;
	int cs = m68340_get_cs(m_maincpu, address);


	switch ( cs )
	{
		case 0: // some sets end up writng the FGPA data with CS0, I guess the CS logic is wrong??
		case 3:
			address &= 0xfffff;

			if (mem_mask&0xff000000) astra_fgpa_w(space, address+0, data >> 24);
			if (mem_mask&0x00ff0000) astra_fgpa_w(space, address+1, data >> 16);
			if (mem_mask&0x0000ff00) astra_fgpa_w(space, address+2, data >> 8);
			if (mem_mask&0x000000ff) astra_fgpa_w(space, address+3, data >> 0);
			break;

		case 2:
			offset &= 0x3fff;
			COMBINE_DATA(&m_mainram[offset]);
			break;

		default:
			logerror("%08x maincpu write access offset %08x data %08x mem_mask %08x cs %d\n", pc, address, data, mem_mask, cs);

	}
}

READ32_MEMBER(astrafr_state::astrafr_slave_mem_r)
{
	int pc = space.device().safe_pc();
	int cs = m68340_get_cs(m_slavecpu, offset * 4);

	switch ( cs )
	{
		case 1:
			if (offset<m_slavecpuregion_size)
				return m_slavecpuregion[offset];
			else
				return 0x0000;

		case 2:
			offset &= 0x3fff;
			return m_slaveram[offset];

		default:
			logerror("%08x slavecpu read access offset %08x mem_mask %08x cs %d\n", pc, offset*4, mem_mask, cs);

	}

	return 0x0000;
}

WRITE32_MEMBER(astrafr_state::astrafr_slave_mem_w)
{
	int pc = space.device().safe_pc();
	int address = offset * 4;
	int cs = m68340_get_cs(m_slavecpu, address);


	switch ( cs )
	{
		case 0: // some sets end up writng the FGPA data with CS0, I guess the CS logic is wrong??
		case 3:
			address &= 0xfffff;

			if (mem_mask&0xff000000) astra_fgpa_slave_w(space, address+0, data >> 24);
			if (mem_mask&0x00ff0000) astra_fgpa_slave_w(space, address+1, data >> 16);
			if (mem_mask&0x0000ff00) astra_fgpa_slave_w(space, address+2, data >> 8);
			if (mem_mask&0x000000ff) astra_fgpa_slave_w(space, address+3, data >> 0);
			break;

		case 2:
			offset &= 0x3fff;
			COMBINE_DATA(&m_slaveram[offset]);
			break;

		default:
			logerror("%08x slavecpu write access offset %08x data %08x mem_mask %08x cs %d\n", pc, address, data, mem_mask, cs);

	}
}



static ADDRESS_MAP_START( astrafr_master_map, AS_PROGRAM, 32, astrafr_state )
	AM_RANGE(0x000000, 0xffffffff) AM_READWRITE(astrafr_mem_r, astrafr_mem_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( astrafr_master_alt_map, AS_PROGRAM, 32, astrafr_state )
	AM_RANGE(0x000000, 0xffffffff) AM_READWRITE(astrafr_mem_r, astrafr_mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( astra_map, AS_PROGRAM, 32, astrafr_state )
	AM_RANGE(0x000000, 0xffffffff) AM_READWRITE(astrafr_mem_r, astrafr_mem_w)
ADDRESS_MAP_END

// probably identical, afaik they're linked units..
static ADDRESS_MAP_START( astrafr_slave_map, AS_PROGRAM, 32, astrafr_state )
	AM_RANGE(0x000000, 0xffffffff) AM_READWRITE(astrafr_slave_mem_r, astrafr_slave_mem_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( astrafr )
INPUT_PORTS_END

MACHINE_START_MEMBER(astrafr_state,astra_common)
{
	m_cpuregion = (UINT32*)memregion( "maincpu" )->base();
	m_cpuregion_size = memregion( "maincpu" )->bytes()/4;
	m_mainram = (UINT32*)auto_alloc_array_clear(machine(), UINT32, 0x10000);

	m_slavecpuregion = (UINT32*)memregion( "slavecpu" )->base();
	m_slavecpuregion_size = memregion( "slavecpu" )->bytes()/4;
	m_slaveram = (UINT32*)auto_alloc_array_clear(machine(), UINT32, 0x10000);


}

/* the FPGA area read/write addresses move around ... */
MACHINE_START_MEMBER(astrafr_state,astra_37)
{
	fgpa_after_rom_write_addr = 0x30;
	fgpa_first_read_addr = 0x33;
	fgpa_rom_write_addr = 0x37;
	MACHINE_START_CALL_MEMBER(astra_common);
}

MACHINE_START_MEMBER(astrafr_state,astra_2e)
{
	fgpa_after_rom_write_addr = 0x20;
	fgpa_first_read_addr = 0x23;
	fgpa_rom_write_addr = 0x2e;
	MACHINE_START_CALL_MEMBER(astra_common);
}


static MACHINE_CONFIG_START( astrafr_dual, astrafr_state )
	MCFG_CPU_ADD("maincpu", M68340, 16000000)
	MCFG_CPU_PROGRAM_MAP(astrafr_master_map)

	MCFG_CPU_ADD("slavecpu", M68340, 16000000)
	MCFG_CPU_PROGRAM_MAP(astrafr_slave_map)

	MCFG_MACHINE_START_OVERRIDE(astrafr_state, astra_common )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( astrafr_dual_2e, astrafr_dual )
	MCFG_MACHINE_START_OVERRIDE(astrafr_state, astra_2e )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( astrafr_dual_37, astrafr_dual )
	MCFG_MACHINE_START_OVERRIDE(astrafr_state, astra_37 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( astrafr_dual_alt, astrafr_state )
	MCFG_CPU_ADD("maincpu", M68340, 16000000)
	MCFG_CPU_PROGRAM_MAP(astrafr_master_alt_map)

	MCFG_CPU_ADD("slavecpu", M68340, 16000000)
	MCFG_CPU_PROGRAM_MAP(astrafr_slave_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( astrafr_dual_alt_37, astrafr_dual_alt )
	MCFG_MACHINE_START_OVERRIDE(astrafr_state, astra_37 )
MACHINE_CONFIG_END



static MACHINE_CONFIG_START( astra_single, astrafr_state )
	MCFG_CPU_ADD("maincpu", M68340, 16000000)
	MCFG_CPU_PROGRAM_MAP(astra_map)
	MCFG_MACHINE_START_OVERRIDE(astrafr_state, astra_common )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( astra_single_37, astra_single )
	MCFG_MACHINE_START_OVERRIDE(astrafr_state, astra_37 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( astra_single_2e, astra_single )
	MCFG_MACHINE_START_OVERRIDE(astrafr_state, astra_2e )
MACHINE_CONFIG_END

MACHINE_START_MEMBER(astrafr_state,astra_57)
{
//  fgpa_after_rom_write_addr = 0x20;
//  fgpa_first_read_addr = 0x23;
	fgpa_rom_write_addr = 0x57;
	MACHINE_START_CALL_MEMBER(astra_common);
}


static MACHINE_CONFIG_START( astra_single_alt, astrafr_state )
	MCFG_CPU_ADD("maincpu", M68340, 16000000)
	MCFG_CPU_PROGRAM_MAP(astra_map)
	MCFG_MACHINE_START_OVERRIDE(astrafr_state, astra_common )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( astra_single_alt_57, astra_single_alt )
	MCFG_MACHINE_START_OVERRIDE(astrafr_state, astra_57 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( astra_single_alt_37, astra_single_alt )
	MCFG_MACHINE_START_OVERRIDE(astrafr_state, astra_37 )
MACHINE_CONFIG_END

/* are the ptM roms Master and ptS roms Slave?
  or is as_partyd set actually the master, with the larger roms?
*/

ROM_START( as_party )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ptm105.u1", 0x0000, 0x080000, CRC(383a0132) SHA1(7a8f06afa0b747c328b4137ae6dcbebac2165bb7) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "pts105.u1", 0x0000, 0x080000, CRC(0699143f) SHA1(486dff92c27ede8cd0d9395f6e4418ba0d056e90) )

	ROM_REGION( 0x200000, "altrevs", 0 ) /* these were in 'party time' sets, but I don't think they're all party time! */
	ROM_LOAD( "pts105d.u1", 0x0000, 0x080000, CRC(0f628b8c) SHA1(c970db9274df6b9c0383ae6e0bcf9b24288a40cd) )
	ROM_LOAD( "ptmv0-02.bin", 0x0000, 0x080000, CRC(e9dd8674) SHA1(ace543bc7fea8d09661e76c1ade4e1f27db5a116) )
	ROM_LOAD( "ptmv1-04.bin", 0x0000, 0x080000, CRC(32af5358) SHA1(bd61c396824bb2b6845126162c4ff797564ebdf2) )
	ROM_LOAD( "ptmv1-13.bin", 0x0000, 0x080000, CRC(4f321735) SHA1(3f9b0b64c42011d948291cd774a922393793a4b1) )
	ROM_LOAD( "joker03.u1", 0x0000, 0x080000, CRC(a3dd15e8) SHA1(95d04de6c991b3c1ff43b0f477f03213e83f63f2) )
	ROM_LOAD( "joker03.u2", 0x0000, 0x080000, CRC(57b3305a) SHA1(c38bc7db6c9a3f8f5371bb4c555333404fed6a9f) )
	ROM_LOAD( "jokr03d.u1", 0x0000, 0x080000, CRC(aa268a5b) SHA1(cce89dbaaaf2d44daf127de1ad6d621c46d892fc) )
	ROM_LOAD( "jokr03g.u1", 0x0000, 0x080000, CRC(cb12f8fa) SHA1(7a9c9fe72b70fe1b4a3be2edfde9d7a54d7a8219) )
	ROM_LOAD( "jst0-05.u1", 0x0000, 0x080000, CRC(38f2d7b0) SHA1(11ccb5b5a35e43f505a7d3ebc36a0694111fed11) )
	ROM_LOAD( "jst0-05.u2", 0x0000, 0x080000, CRC(97c14933) SHA1(8515601fbacf76a78a95e4a46a47809fcec021bc) )
	ROM_LOAD( "jst0-05d.u1", 0x0000, 0x080000, CRC(31094803) SHA1(b7b5fd97681c38de5e877ca7b09909c82316d4d8) )
	ROM_LOAD( "jst0-05g.u1", 0x0000, 0x080000, CRC(503d3aa2) SHA1(818e79126ca8ae6bbd1eaac2ae13977b3402f497) )
	ROM_LOAD( "jst0-06.u1", 0x0000, 0x080000, CRC(b6106ca4) SHA1(dedeeff6e8c13da1f62e882cbeb36567a631e563) )
	ROM_LOAD( "jst0-06.u2", 0x0000, 0x080000, CRC(49a78695) SHA1(af99df8c97a8c3bcb5a81a38b985e1fc92176927) )
	ROM_LOAD( "jst0-06d.u1", 0x0000, 0x080000, CRC(bfebf317) SHA1(7f1a2a8e3f3de6a03452f535d2c314810bc652da) )
	ROM_LOAD( "jst0-06g.u1", 0x0000, 0x080000, CRC(dedf81b6) SHA1(0579501c939eacb93d1455eaf74b629c2170b05b) )
	ROM_LOAD( "l7v1-03.bin", 0x0000, 0x080000, CRC(878d1a3d) SHA1(fd7cb08f698bb6bbfed1c57486e53dce062d22e4) )
	ROM_LOAD( "l7v1-03d.bin", 0x0000, 0x080000, CRC(91a81e50) SHA1(6086861bd5a53fa17df8b155acd47e9aa45a032d) )
	ROM_LOAD( "pbmv0-06.u1", 0x0000, 0x080000, CRC(d0283320) SHA1(472f0e0dd45da61081ca12e466ac02dc82eb4431) )
	ROM_LOAD( "pbmv0-06.u2", 0x0000, 0x080000, CRC(38fb2ff6) SHA1(628dcdcbf4767db62b4bdee7b7feff32715e6a2d) )
	ROM_LOAD( "pbs0-06d.u1", 0x0000, 0x080000, CRC(f24e84d2) SHA1(d54e787c91c79a26383971249f935529e2a492f4) )
	ROM_LOAD( "pbs0-06g.u1", 0x0000, 0x080000, CRC(937af673) SHA1(88f33fd3921440a99b662fec7291c8b9845210a5) )
	ROM_LOAD( "pbsv0-06.u1", 0x0000, 0x080000, CRC(fbb51b61) SHA1(c1459f8f2d9f182e5be55bbcaf143315f7efc3b0) )
	ROM_LOAD( "pbsv0-06.u2", 0x0000, 0x080000, CRC(2f0934de) SHA1(915a16898087f396457d712e3847f4f7c0bd5c06) )
	ROM_LOAD( "pgm0-04.u1", 0x0000, 0x040000, CRC(2885367c) SHA1(c33cb554889b1f7390baa416a77953f45a80044f) )
	ROM_LOAD( "pgm0-04.u2", 0x0000, 0x040000, CRC(c5eed515) SHA1(9b832a6ef301a25bccf2d97cca0c9a012ca0090a) )
	ROM_LOAD( "pgm1-00.u1", 0x0000, 0x040000, CRC(1fa4e10f) SHA1(787ec967a9aba5934db79fe67efb32370d2c0258) )
	ROM_LOAD( "pgm1-00.u2", 0x0000, 0x040000, CRC(8b5a6178) SHA1(e1f9898ef37877ce50630a468b3c187a4fe253fa) )
	ROM_LOAD( "pgs0-04d.u1", 0x0000, 0x040000, CRC(1f5ede2c) SHA1(ac67536a021b531efe18027806f1f86504d72493) )
	ROM_LOAD( "pgsv0-04.u1", 0x0000, 0x040000, CRC(cb3387be) SHA1(2add224a8839e83cc04901274acc7ca4a781b7d9) )
	ROM_LOAD( "pgsv0-04.u2", 0x0000, 0x040000, CRC(63054bd6) SHA1(59cf8dd7efdaf2491a2aca8fbcda2d3b8b70fbf7) )
	ROM_LOAD( "pgsv1-00.u1", 0x0000, 0x040000, CRC(725dd2af) SHA1(f8ecd1282809c0906497c62a68429152c10e2da0) )
	ROM_LOAD( "pgsv1-00.u2", 0x0000, 0x040000, CRC(067dd0c2) SHA1(ac36aeb63b33969dc0a49150e41bfdd8624072de) )
	ROM_LOAD( "pgsv100d.u1", 0x0000, 0x040000, CRC(a6308b3d) SHA1(125ed244bcb7a515dfc9c34c12bc74f8cd50e8dd) )
	ROM_LOAD( "ptp003d.u1", 0x0000, 0x100000, CRC(2b2c05b6) SHA1(541a53c84c07bd7e1f09d4d033cf652ab838b4ef) )
	ROM_LOAD( "ptp003g.u1", 0x0000, 0x100000, CRC(d9d0e151) SHA1(35fb4412602b9fd3b66e7170cc1984693b9c9ebd) )
	ROM_LOAD( "ptpv003.u1", 0x0000, 0x100000, CRC(07c189da) SHA1(c4574cdedba87058312db84c6ee7f4a7142eea65) )
	ROM_LOAD( "ptpv003.u2", 0x0000, 0x100000, CRC(e1f78cf4) SHA1(5f72b2604fd7ee300f6bd5b5a12d98c77b03b9ba) )
	ROM_LOAD( "pts1-04d.bin", 0x0000, 0x080000, CRC(95564b9f) SHA1(98091d4badd346578882db75a8d72ddaa810b3f5) )
	ROM_LOAD( "pts1-13d.bin", 0x0000, 0x080000, CRC(876232cf) SHA1(5d2355f85bde636dcb6f3dbd87874294db8e1ded) )
	ROM_LOAD( "ptsv0-02.bin", 0x0000, 0x080000, CRC(3cc9c022) SHA1(1eb3c237971cf407057d077fd08e4436c765ae43) )
	ROM_LOAD( "ptsv1-04.bin", 0x0000, 0x080000, CRC(9cadd42c) SHA1(e6ead7112195a17797672112c7bbd4910ae6eb50) )
	ROM_LOAD( "ptsv1-13.bin", 0x0000, 0x080000, CRC(8e99ad7c) SHA1(d4cfce825b4a718a12d80e79ed943797c6510ad6) )
	ROM_LOAD( "sov0-03.u1", 0x0000, 0x080000, CRC(4e2e7a79) SHA1(77f32b43d2e01cb0223feccb9e29c3fde0a6e9b7) )
	ROM_LOAD( "sov0-03.u2", 0x0000, 0x080000, CRC(1ef8712a) SHA1(1a2ef378679384e720e5b20f1420454102f18258) )
	ROM_LOAD( "sov0-03d.u1", 0x0000, 0x080000, CRC(47d5e5ca) SHA1(d3624a8b8545f67d14bee90bc3967c13b8497e5b) )
	ROM_LOAD( "hog0-03.u1", 0x0000, 0x080000, CRC(eaa26ab0) SHA1(ee7d76b92c3274ba8c5ba59184bb3334fbbc64c4) )
	ROM_LOAD( "jjrv1-02.u1", 0x0000, 0x080000, CRC(0f05e392) SHA1(64c885c92fb26c0ed64b8283793cdf86d2bc0e35) )
	ROM_LOAD( "jjrv1-02.u2", 0x0000, 0x080000, CRC(1d86f26c) SHA1(e7db51b217e9fb8a0440b7c7591c5ea4142540bc) )
ROM_END

ROM_START( as_partya )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ptm110.u1", 0x0000, 0x080000, CRC(dd649eaf) SHA1(7449818635b6542148335e4ede72c6682d07834b) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "pts110.u1", 0x0000, 0x080000, CRC(b2771618) SHA1(40b0b52969a2ee53d38907c210305ae6f9ed7436) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "pts110d.u1", 0x0000, 0x080000, CRC(bb8c89ab) SHA1(2702574177d56768a4cc03f0b2bb95b3510dfe48) )
	ROM_LOAD( "pts110g.u1", 0x0000, 0x080000, CRC(c9cc1d09) SHA1(5ae08d5413d8b8f6ba06c412163168b3e86a67e6) )
ROM_END

ROM_START( as_partyb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ptm112.u1", 0x0000, 0x080000, CRC(ab4a2808) SHA1(4b7c4986d4d3cf65be925085d41b698bbf0b4374) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "pts112.u1", 0x0000, 0x080000, CRC(b89bca1d) SHA1(899bafefa02321f2225276de2eca2a303b64a2b2) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "pts112d.u1", 0x0000, 0x080000, CRC(b16055ae) SHA1(beb411fb06f72557fcdc8c8d42dae0751448855b) )
ROM_END

ROM_START( as_partyc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ptnm206.u1", 0x0000, 0x080000, CRC(46d841b9) SHA1(6c4ffffd685bfa3474f697a1d9c96a36882ce104) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "ptns206.u1", 0x0000, 0x080000, CRC(933ce1a6) SHA1(c75a43e9bcb95f6d1579f7f5d5ed67b80499b5e7) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ptns206d.u1", 0x0000, 0x080000, CRC(9ac77e15) SHA1(a0ee7db100a0303a15f5fde2b44a5982f8252e85) )
ROM_END

ROM_START( as_partyd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ptmv4-01.bin", 0x0000, 0x100000, CRC(3fde519a) SHA1(4d6f19e95e488e05174b2b025310852780e8d916) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "ptsv4-01.bin", 0x0000, 0x100000, CRC(da51bdca) SHA1(675fd673085155ffd4211f2fb4c8e8252fe76072) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "pts4-01d.bin", 0x0000, 0x100000, CRC(f6bc31a6) SHA1(c5c05e40da7c097c3b56a2b48cbf1992128cab6b) )
	ROM_LOAD( "pts4-01g.bin", 0x0000, 0x100000, CRC(0440d541) SHA1(afcef2d6671d23d6635a54faa2f72217f03ab2de) )
ROM_END

ROM_START( as_partye )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ptime9.07m", 0x0000, 0x080000, CRC(0af16f11) SHA1(37e8abea7386fbeda634e546e1a560d7c1eaf913) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "ptime9.07s", 0x0000, 0x080000, CRC(4ff4fb24) SHA1(eb98026518e86a4959fa6039eace55a473ef58c3) )
ROM_END

ROM_START( as_partyf )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "p_time9.06", 0x0000, 0x080000, CRC(5bdf1b05) SHA1(267f66250451cee2afc43d9275a39f23eccd357d) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "p_time9.06s", 0x0000, 0x080000, CRC(4d33ca2c) SHA1(da6623bdf46160bbb178ea7df1f28dfc687e9dcb) )
ROM_END



ROM_START( as_letsp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "lpmv9-04.bin", 0x0000, 0x080000, CRC(c93e4e63) SHA1(3a1d4f589d8cdbcb25e35769104f21ba0883498d) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "lpsv9-04.bin", 0x0000, 0x080000, CRC(e2c5558b) SHA1(a49f6a23523aac3c5c8198fffc2d319e4bc7cfdd) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "lpsv904d.bin", 0x0000, 0x080000, CRC(eb3eca38) SHA1(9655fe22112110f015a6bbe5c43dbda96d800912) )
ROM_END







ROM_START( as_topsl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tsm103.u1", 0x0000, 0x080000, CRC(da37998f) SHA1(202dad521dc15187c40e807b117aa70737e126f4) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "tss103.u1", 0x0000, 0x080000, CRC(f4be316a) SHA1(f96e76b81db17ce1b4b42a45b5b209d140048657) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tss103d.u1", 0x0000, 0x080000, CRC(fd45aed9) SHA1(029b66154ef171449b8712a1ec6a367052ea8aa8) )
ROM_END

ROM_START( as_topsla )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tsm104.u1", 0x0000, 0x080000, CRC(f5af7008) SHA1(73d436c146b46886004b0cf3b1e64a5a58e0e769) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "tss104.u1", 0x0000, 0x080000, CRC(a3ab1bce) SHA1(9c9b9ca6e62219b7e8e2631e56800a212613df0e) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tss104d.u1", 0x0000, 0x080000, CRC(aa50847d) SHA1(78375a73c1f318aaab8c3ea9024d9fdaeb42c2fc) )
ROM_END

ROM_START( as_topslb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tsm201.u1", 0x0000, 0x080000, CRC(acd57fcf) SHA1(69819d6002a2ce0481977369d469fe597fd6618b) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "tss201.u1", 0x0000, 0x080000, CRC(3ead0a9e) SHA1(cf2630c0d555ee7aa6dc9a33265feb26208321b8) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tss201d.u1", 0x0000, 0x080000, CRC(3756952d) SHA1(23a31b95e96df9b1090bc49b13df49cd4e425025) )
	ROM_LOAD( "tss201g.u1", 0x0000, 0x080000, CRC(4516018f) SHA1(008be270e2a9276989e069b757705035668dc8c6) )
	ROM_LOAD( "tss201gt.u1", 0x0000, 0x080000, CRC(5662e78c) SHA1(af9aede67706b3c596a46e5947b768c719883ab2) )
	ROM_LOAD( "tss201st.u1", 0x0000, 0x080000, CRC(2dd9ec9d) SHA1(1ccdee0316f1d73b8cb5c94396c537023b77524e) )
	ROM_LOAD( "tss201t.u1", 0x0000, 0x080000, CRC(2422732e) SHA1(c81a8558a986bfaec3277dcdab64a0b0a9318fb2) )
ROM_END

ROM_START( as_topslc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tsm203.u1", 0x0000, 0x080000, CRC(78e61d11) SHA1(888c76cd44fe1f556eddf0227df03bd0d6210bbf) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "tss203.u1", 0x0000, 0x080000, CRC(5c55f57a) SHA1(ca6172788deb06d3faeb7186f7922bd4f4094430) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tss203d.u1", 0x0000, 0x080000, CRC(55ae6ac9) SHA1(8d297fcadb63e1e9b0ac1e91ff10321b043f1280) )
ROM_END


ROM_START( as_topsld )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tsmv2-05.bin", 0x0000, 0x080000, CRC(d525422d) SHA1(45b39fb0f4c8c8e7159b8832eca6e80b2cb79ced) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD( "tssv2-05.bin", 0x0000, 0x080000, CRC(d67ffde8) SHA1(f4e3b84c0a6a23f2fe1482b97dc48fb44315101f) )
ROM_END


ROM_START( as_hc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hcm107.u1", 0x00000, 0x080000, CRC(6481cf82) SHA1(975ac90615aee6fbda56707e0cc0776d103e6681) )
	ROM_LOAD16_BYTE( "hcm107.u2", 0x00001, 0x080000, CRC(e5dbba3e) SHA1(545828ad4649665bd8b1125a0943108fc41ef44b) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "hcs107.u1", 0x00000, 0x080000, CRC(07c68bad) SHA1(32987bac887a02e9b073d16fb51251f7d9196e19) )
	ROM_LOAD16_BYTE( "hcs107.u2", 0x00001, 0x080000, CRC(856c7bbf) SHA1(cf20481f93ebd10c8907f3f909fb0f6f9504b046) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hcs107d.u1", 0x0000, 0x080000, CRC(0e3d141e) SHA1(66640deb844d3b23d408dcb15c9fcd8076d4ab15) )
ROM_END

ROM_START( as_hca )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hcm109.u1", 0x00000, 0x080000, CRC(25496265) SHA1(72cb61732949cf81db3fa45fc97087ae6f2e0fe2) )
	ROM_LOAD16_BYTE( "hcm109.u2", 0x00001, 0x080000, CRC(85959686) SHA1(866dc6567639a10ddc8628ab7651140c558ac2df) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "hcs109.u1", 0x00000, 0x080000, CRC(f28ecdfb) SHA1(afc7fc048a8a5950bd0209c3dbe71a5a81b3313a) )
	ROM_LOAD16_BYTE( "hcs109.u2", 0x00001, 0x080000, CRC(61d426f8) SHA1(818ae31de701dca318eeb75a98ba387ee619fc02) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hcs109d.u1", 0x00000, 0x080000, CRC(fb755248) SHA1(5432691c46e35592f2975ae2d5c71d69043f43a5) )
ROM_END

ROM_START( as_hcb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hcmv1-10.u1", 0x00000, 0x080000, CRC(25dc38e9) SHA1(727d6de290b9078f1c8552b04b1d848863874107) )
	ROM_LOAD16_BYTE( "hcmv1-10.u2", 0x00001, 0x080000, CRC(7f80cb29) SHA1(fdad25bfab6827486b114896fa5dba3d4a0b6ebe) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "hcsv1-10.u1", 0x00000, 0x080000, CRC(64e90540) SHA1(d0e31d2c9aa5399c07ade7e85fc79f584f3d83ba) )
	ROM_LOAD16_BYTE( "hcsv1-10.u2", 0x00001, 0x080000, CRC(18c61602) SHA1(d4cbd894b65e46914ea858a221e01e222310c7ca) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hcsv110d.u1", 0x00000, 0x080000, CRC(6d129af3) SHA1(49397a77d2c7cab507772793138acb543d16cd53) )
ROM_END


ROM_START( as_hcc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hcmv1-11.u1", 0x00000, 0x080000, CRC(7c29dffc) SHA1(185233d02165066e5b845b2821f40f07850c242e) )
	ROM_LOAD16_BYTE( "hcmv1-11.u2", 0x00001, 0x080000, CRC(96c39b83) SHA1(c7bfcd8a33338579837e008cb5cd19b64d291133) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "hcsv1-11.u1", 0x00000, 0x080000, CRC(cdce1540) SHA1(bf27fbf7670154f6c2048a25754dd79627335314) )
	ROM_LOAD16_BYTE( "hcsv1-11.u2", 0x00001, 0x080000, CRC(4f2b0bfe) SHA1(928ecc985e7d1d23d4db564f6499dd7fea47407a) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hcs1-11d.u1", 0x00000, 0x080000, CRC(c4358af3) SHA1(9c08eb2a5daf1d77a4c81cf339a8a0d10f3c30da) )
ROM_END

ROM_START( as_hcd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hcm909.u1", 0x00000, 0x080000, CRC(c3d10181) SHA1(532e71733578fec11f452968fd82f2e656f27fdc) )
	ROM_LOAD16_BYTE( "hcm909.u2", 0x00001, 0x080000, CRC(0a899237) SHA1(8be4937429732f1cddeaa6565a80a0b5430adaee) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "hcs909.u1", 0x00000, 0x080000, CRC(cc2553ac) SHA1(2f6f45727b5a160ddf2afeb3c481a92046f80a8f) )
	ROM_LOAD16_BYTE( "hcs909.u2", 0x00001, 0x080000, CRC(97f4a5ea) SHA1(f4e625aabfaaaea71f1feddb3f681cbb7ce1de7e) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hcs909d.u1", 0x00000, 0x080000, CRC(c5decc1f) SHA1(2824377a479901262614068406c6e7898bc2d9f3) )
ROM_END




ROM_START( as_bigtm )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bti003.u1", 0x00000, 0x080000, CRC(6751c225) SHA1(c4b69293c40927e81bd473f4b6487f0a05472503) )
	ROM_LOAD16_BYTE( "bti003.u2", 0x00001, 0x080000, CRC(88369abd) SHA1(2d4d06a6598f7b4a2ae1aef0a4c6f571c9de9a0b) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "bti003d.u1", 0x0000, 0x080000, CRC(6eaa5d96) SHA1(6e34c4a20ab140059d80f5f3d2e8a5a5091efd62) )
ROM_END


ROM_START( as_acp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "acpv403.u1", 0x00000, 0x100000, CRC(b1af78be) SHA1(6cacd0e815cea236130d35ac742076d24d825d66) )
	ROM_LOAD16_BYTE( "acpv403.u2", 0x00001, 0x100000, CRC(ba6d3127) SHA1(f2511455d387ee588b106b9f2823e44b6e9328c9) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "403compak.u1", 0x0000, 0x100000, CRC(d3a37514) SHA1(fe58c3172272a119431cdebcb27b50b8c1d36877) )
	ROM_LOAD16_BYTE( "403conx.u1", 0x0000, 0x100000, CRC(6fbe1035) SHA1(7b03a6b1cc4c89e7bdf583befe18d98e5f4a159b) )
ROM_END

ROM_START( as_celeb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cel100.u1", 0x00000, 0x080000, CRC(7eb2de99) SHA1(8d24ac812da727b60a043b875f409223c7bd589f) )
	ROM_LOAD16_BYTE( "cel100.u2", 0x00001, 0x080000, CRC(9f5512a2) SHA1(86ec5df6905fe07bff2176c134a48a196f78a772) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cel100d.u1", 0x0000, 0x080000, CRC(7749412a) SHA1(3014cc7c6da6aff95aff6bb7113928a9223c17c8) )
ROM_END


ROM_START( as_celeba )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cel101.u1", 0x00000, 0x080000, CRC(884187de) SHA1(5a96e47e09607ec91af575c6c29972f6ddcea103) )
	ROM_LOAD16_BYTE( "cel101.u2", 0x00001, 0x080000, CRC(b50b7dfb) SHA1(63b243464eae47a0676b40f2fe65e43e8cea62d7) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cel101d.u1", 0x0000, 0x080000, CRC(81ba186d) SHA1(91c149be902cc028e3e5f8cd4e6c26201366f2a7) )
ROM_END

ROM_START( as_celebb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cel201.u1", 0x00000, 0x080000, CRC(4b5d11ca) SHA1(b4cd631524a67a18515230ef07bb52dca115472c) )
	ROM_LOAD16_BYTE( "cel201.u2", 0x00001, 0x080000, CRC(42f81484) SHA1(305b04c8300c868551c852c6305588b2331155cc) )
//  ROM_LOAD16_BYTE( "cel201u2.bin", 0x00001, 0x080000, CRC(42f81484) SHA1(305b04c8300c868551c852c6305588b2331155cc) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cel2pnd.u1", 0x0000, 0x080000, CRC(58aa2eac) SHA1(bc38fef5187f5e487c6fdeea404f1eef78b82cb2) )
ROM_END


ROM_START( as_cshah )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cash1-050lwn-e.u1", 0x00000, 0x100000, CRC(7297ff77) SHA1(419ca98b29e2ae0b8c056201af631320fffb768a) )
	ROM_LOAD16_BYTE( "cash1-050lwn-e.u2", 0x00001, 0x100000, CRC(4d39a9b0) SHA1(44a78c5068791703547cb1618f1b2ccdd5ddb015) )
ROM_END


ROM_START( as_srb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "srb004.u1", 0x0000, 0x080000, CRC(a0ce3300) SHA1(8db1959ad5a51bd395a9105fe74351d043760cbd) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb004d.u1", 0x0000, 0x080000, CRC(a935acb3) SHA1(3bf3ce56410ff9ef6b1b3fcd069c1eb7a1d76e7a) )
	ROM_LOAD( "srb004g.u1", 0x0000, 0x080000, CRC(3f77c830) SHA1(fb64d0046f0b319c6d8d0968d2233e2ab4523ce0) )
ROM_END

ROM_START( as_srba )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "srb100.u1", 0x0000, 0x080000, CRC(9b12d52e) SHA1(bc65b1c6ba8eeaa07ce8320bba655e4726b61b6c) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb100d.u1", 0x0000, 0x080000, CRC(92e94a9d) SHA1(974b3a8c5db445cc7f8a16d90874fd6c765f5145) )
ROM_END

ROM_START( as_srbb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "srb101.u1", 0x0000, 0x080000, CRC(fa4d87af) SHA1(e553c7f70ad5b5cc1b7ed999103b947df1a18203) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb101d.u1", 0x0000, 0x080000, CRC(f3b6181c) SHA1(a62cb5ed72e956d4d6eb983d3f57f7ab3ae2a169) )
ROM_END

ROM_START( as_srbc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "srb201.u1", 0x0000, 0x080000, CRC(d74fcc7c) SHA1(636b2b847baa25cb9472cc0240852a9995b15e54) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb201d.u1", 0x0000, 0x080000, CRC(deb453cf) SHA1(bda66c1aa0cb39588b2960a3e0065d0a986d0ee0) )
	ROM_LOAD( "srb201g.u1", 0x0000, 0x080000, CRC(48f6374c) SHA1(9f92e1a2a76b6f27cd440decd7e392ee39992936) )
ROM_END

ROM_START( as_srbd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "srb202.u1", 0x0000, 0x080000, CRC(5035b095) SHA1(4113ca5d9ff1d80dbc2593c21d2eeba12352d9fb) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb202d.u1", 0x0000, 0x080000, CRC(59ce2f26) SHA1(69eed1344ae3f8db0d7edcbf4987b71e5a1ff5ee) )
ROM_END

ROM_START( as_srbe )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "srb203.bin", 0x0000, 0x080000, CRC(cf72c049) SHA1(7d29412a6787312a336ca007a9c9490c8a955f1f) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "srb203d.bin", 0x0000, 0x080000, CRC(c6895ffa) SHA1(457d5aa6c746e0d9a7b9529686f469354a0f94d1) )
ROM_END

ROM_START( as_big10 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "astra big10.bin", 0x0000, 0x080000, CRC(9562f4b4) SHA1(867a411e62f089b85a7223c940b401ac136fd127) )
ROM_END


ROM_START( as_big10a )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "btc5_03.bin", 0x0000, 0x080000, CRC(e07008c6) SHA1(5f9950fe6404da8865d4d47fd29a8d6e0a4e878d) )
ROM_END


ROM_START( as_big10b )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "btc507.u1", 0x0000, 0x080000, CRC(311f7c95) SHA1(02e18a6148aaead138e4e36c42d78a6096c8c1dd) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "btc507d.u1", 0x0000, 0x080000, CRC(38e4e326) SHA1(332236b310d746848757fef0931b9cd289d01a59) )

	ROM_LOAD( "btu507.u1", 0x0000, 0x080000, CRC(17f6b093) SHA1(836e9e7c702c2882d03b6dd9a288ec4ee7088f10) )
	ROM_LOAD( "btu507d.u1", 0x0000, 0x080000, CRC(1e0d2f20) SHA1(4e70656f4329e643e41dc2440e2391bbc132e45f) )
ROM_END

ROM_START( as_big10c )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "btc601.u1", 0x0000, 0x080000, CRC(0b57da84) SHA1(0cd2e2cab81a6e807a01a79c9f351bab4b3d5c12) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "btc601d.u1", 0x0000, 0x080000, CRC(02ac4537) SHA1(8645021162f44b873e2e148298fef880ef140857) )
	ROM_LOAD( "btc601g.u1", 0x0000, 0x080000, CRC(94ee21b4) SHA1(577ac20094e8f1faec8289500d390c74511c5c19) )
ROM_END


ROM_START( as_big10d )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bt605.bin", 0x0000, 0x080000, CRC(17b28a5a) SHA1(f4f252bced0f6a8614a0cda1613841935fb07016) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "bt605d.bin", 0x0000, 0x080000, CRC(1e4915e9) SHA1(9579b7d41d7b21de6ed5d2c73fd75593ac860c46) )
ROM_END

ROM_START( as_big15 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "b15101g.u1", 0x0000, 0x080000, CRC(15fd1ec0) SHA1(0eb50529b178cea0b70523c144392d143ad79dfa) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "b15101d.u1", 0x0000, 0x080000, CRC(67bd8a62) SHA1(08abba7c0b8e170b50dfc27b4e5b8818bf824862) )
ROM_END

ROM_START( as_bigcs )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bc1_01g.bin", 0x0000, 0x080000, CRC(b5eeeee5) SHA1(e159ba719606b7667ddfa9fd9abf0d7d8db77c2e) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "bc1_01p.bin", 0x0000, 0x080000, CRC(23ac8a66) SHA1(4c6074da6471103336c77e029a17fdddbcc1cdc5) )
ROM_END

ROM_START( as_bigcsa )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bc103.u1", 0x0000, 0x080000, CRC(ef96e656) SHA1(49bd0ffd4380d7a045bfb1d5a3fccb3d2ad90b05) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "bc103d.u1", 0x0000, 0x080000, CRC(e66d79e5) SHA1(a2ff936782c79846672ffc6cc6444fb2e8846470) )
ROM_END


ROM_START( as_bbr )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bbr101.u1", 0x0000, 0x080000, CRC(6854afcd) SHA1(f8ce1cc7e5835160e01eedf0abf7cd38eb4a1e08) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "bbr101d.u1", 0x0000, 0x080000, CRC(61af307e) SHA1(9b8ee2ed6f46076269f92961e5282e4bb23daf74) )
ROM_END

ROM_START( as_bbra )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bbv1-02.bin", 0x0000, 0x080000, CRC(e9f55c67) SHA1(8c052cd35e39839eafde5dca1daec58a84c04f84) )
ROM_END

ROM_START( as_bbrb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bbv2-01.bin", 0x0000, 0x080000, CRC(20864069) SHA1(0ac0b55bf866cf2dcbe1b042abca58d1740f0c9a) )
ROM_END

ROM_START( as_bbrc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bbv0-03.bin", 0x0000, 0x080000, CRC(9c8f092b) SHA1(96a6a10eff5e847c20dea82dda9d59d87cf0224b) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "bbv0-03d.bin", 0x0000, 0x080000, CRC(95749698) SHA1(c7b4967c329cff96e2a7f29fec73c660c5efa06a) )
	ROM_LOAD( "bbv0-03g.bin", 0x0000, 0x080000, CRC(0336f21b) SHA1(7db0ca8cca6ac376a3e6f25809b92cf3f1ccd5b6) )
ROM_END


ROM_START( as_bbrd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bb102.u1", 0x0000, 0x080000, CRC(6cec69f8) SHA1(bbc289b2b404898283eb10e6142dec43ebb93428) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "bb102d.u1", 0x0000, 0x080000, CRC(6517f64b) SHA1(a31ac28cb29d9606bc3140ceb9868608637d5c03) )
ROM_END



ROM_START( as_bbre )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bbs105.u1", 0x0000, 0x080000, CRC(a481131f) SHA1(2bc94a992dd7871d64c354a4dfab7fcb8e6bb807) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "bbs105d.u1", 0x0000, 0x080000, CRC(ad7a8cac) SHA1(e47f170fe3fef32b0e75c920c4e807e95f654a6e) )
ROM_END

ROM_START( as_bbrf )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bbsb004.u1", 0x0000, 0x080000, CRC(cb582b13) SHA1(996fc410e9d4980d95e92a0dfdbceeaa81bfbc43) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "bbsb004d.u1", 0x0000, 0x080000, CRC(c2a3b4a0) SHA1(ff520ac1661caef9a4273206df32768e1cae4c34) )
ROM_END



ROM_START( as_dblcs )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dc110.u1", 0x0000, 0x080000, CRC(b930219c) SHA1(6c17b4d98b9c51a5e8bac17af1a198c961983e7e) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "dc110d.u1", 0x0000, 0x080000, CRC(b0cbbe2f) SHA1(29d279fa800e5299f592e66117831d00a6020bfe) )
	ROM_LOAD( "dc110g.u1", 0x0000, 0x080000, CRC(2689daac) SHA1(c46c713f8b49fac24e84f739f5aa7cbdf39664dd) )
ROM_END

ROM_START( as_dblcsa )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dcc112.u1", 0x0000, 0x080000, CRC(75c600dd) SHA1(78c0193623c5499b46ec0e316d0c0f8b96cb84cc) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "dcc112d.u1", 0x0000, 0x080000, CRC(7c3d9f6e) SHA1(1feae8b45ad850cd25254c2b77ed796b3862e251) )
ROM_END

ROM_START( as_dblcsb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dcc1_08.bin", 0x0000, 0x080000, CRC(43bfb902) SHA1(23d31dec9125ff5ffa04ca87fe7831bc141092ed) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "dcc1_08g.bin", 0x0000, 0x080000, CRC(dc064232) SHA1(3c17a929c98f7bbc4cbbfb4162698abd2bef4be3) )
	ROM_LOAD( "dcc1_08p.bin", 0x0000, 0x080000, CRC(4a4426b1) SHA1(e31b94597fd51872d639cc1510232259ec862de0) )
ROM_END

ROM_START( as_dblcsc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dcu109.u1", 0x0000, 0x080000, CRC(ad660c95) SHA1(c5cb67148a67c95f79339e088fbffc730bb07049) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "dcu109d.u1", 0x0000, 0x080000, CRC(a49d9326) SHA1(4e11644c13c46a1655d7741eacba0f4efc2c1b74) )
ROM_END

ROM_START( as_dblcsd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dcl1_06.bin", 0x0000, 0x080000, CRC(652b07db) SHA1(c2125abb58f66461f1d9bfdef0677bf3baab3c69) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "dcl1_06g.bin", 0x0000, 0x080000, CRC(fa92fceb) SHA1(651e908c3b7bfaccfe95e8a6b535bb7f2bd53652) )
	ROM_LOAD( "dcl1_06p.bin", 0x0000, 0x080000, CRC(6cd09868) SHA1(788f7c06b950884b40c28c5fcb0c728c858295c9) )
ROM_END

ROM_START( as_dblcse )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dcv1-02.bin", 0x0000, 0x080000, CRC(e6b745bc) SHA1(0c7b71cbbe6c0c5a494ff7891c02addc51ba11ce) )
ROM_END

ROM_START( as_dblcsf )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dcr100.u1", 0x0000, 0x080000, CRC(e56cfd6a) SHA1(bcd2d67daa33d0d65373394f5948384795d7c08a) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "dcr100d.u1", 0x0000, 0x080000, CRC(ec9762d9) SHA1(316cca9af1d949b223f3d79bc780ff89e354ab71) )
ROM_END

ROM_START( as_dblcsg )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dcd100.bin", 0x0000, 0x080000, CRC(83bfdde9) SHA1(75c6f6d3f8fd4d28d46731d70cc3082bb3f18f91) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "dcd100d.bin", 0x0000, 0x080000, CRC(8a44425a) SHA1(559aeb9375dc0dd2d618894ec2a6c61d6e3bc06f) )
ROM_END

ROM_START( as_dblcsh )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "dcb004.u1", 0x0000, 0x080000, CRC(b734d6f1) SHA1(0536fa661ebe9504acd411516f61482e0bade77e) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "dcb004d.u1", 0x0000, 0x080000, CRC(becf4942) SHA1(a1ef2647c7416ba798c003e45baeb57648278bcc) )
ROM_END

ROM_START( as_fortn )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ft009.u1", 0x00000, 0x080000, CRC(fc4413ec) SHA1(b6874584fa328d6029069e11a5b5f0495f6d319e) )
	ROM_LOAD16_BYTE( "ft009.u2", 0x00001, 0x080000, CRC(940eda83) SHA1(06582b7b45a0aeeb59cbf583ebedffc8a45c4cf6) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ft009d.u1", 0x00000, 0x080000, CRC(f5bf8c5f) SHA1(20de9612f22d5459cad6b3938ad5a1eb069a00c3) )
ROM_END

ROM_START( as_gof )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gofv2-08.u1", 0x00000, 0x100000, CRC(8cce26aa) SHA1(4deb704ff102b85b5e6711df2a8644d6c2aea72c) )
	ROM_LOAD16_BYTE( "gofv2-08.u2", 0x00001, 0x100000, CRC(5bd78874) SHA1(41741d0e3ba1ab2371de4eb4783715c8d1c8b697) )
ROM_END

ROM_START( as_hog )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hog002.u1", 0x00000, 0x080000, CRC(e2975c31) SHA1(b77c07548eee991fb55c60906c9d52cf7abcc927) )
	ROM_LOAD16_BYTE( "hog002.u2", 0x00001, 0x080000, CRC(757bc739) SHA1(f406152050f9ccb234c5fddf7356333ae02b36ef) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hog002d.u1", 0x0000, 0x080000, CRC(eb6cc382) SHA1(54bda36d9633dfbecf165890b006cb6fe371c38e) )
ROM_END

ROM_START( as_hxr )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1hxr1_08.bin", 0x00000, 0x040000, CRC(eb1671c3) SHA1(554eb3bd215f1d6ca88585bb4a67dfefd2de3c9a) )
	ROM_LOAD16_BYTE( "2hxr1_08.bin", 0x00001, 0x040000, CRC(4ec8651d) SHA1(5dd1068f1b1523023e0adf5a00752adf0d02ad78) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "1hxr1_08g.bin", 0x0000, 0x040000, CRC(c0a9d669) SHA1(5cb4f6fd5fe19d44935c895a211cb439f5b3134a) )
	ROM_LOAD16_BYTE( "1hxr1_08p.bin", 0x0000, 0x040000, CRC(3f7b2851) SHA1(361b8d680c684ed1d25690f89a917a33a894035f) )
ROM_END


ROM_START( as_hr )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hru004.u1", 0x00000, 0x080000, CRC(0af214ad) SHA1(b58a6cca24c5c144e88ffdb1fcd592801b4e21d3) )
	ROM_LOAD16_BYTE( "hru004.u2", 0x00001, 0x080000, CRC(d7b60ede) SHA1(04069bcea8cb3c25bf7539a6adb05951fe43849e) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hru004d.u1", 0x0000, 0x080000, CRC(03098b1e) SHA1(e68076aa6c16c9486544d30011db39a3d8090a68) )
ROM_END




ROM_START( as_djp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dj107.u1", 0x00000, 0x080000, CRC(13998f29) SHA1(f7c517a6c34a827c9a20e6ace102e3879f839cac) )
	ROM_LOAD16_BYTE( "dj107.u2", 0x00001, 0x080000, CRC(e78cbce6) SHA1(100a13a118ccf5ca34f3a1a4bf7dec1d02b3d511) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "dj107d.u1", 0x0000, 0x080000, CRC(1a62109a) SHA1(180a351bf79f85746531d3f8dc7b0884456028d3) )
ROM_END

ROM_START( as_djpa )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "djb004.u1", 0x00000, 0x040000, CRC(579d1b7a) SHA1(91fef91cbeaaba92bc1d4908ae67ba5f22cced77) )
	ROM_LOAD16_BYTE( "djb004.u2", 0x00001, 0x040000, CRC(b0854c9b) SHA1(ef1cb1da7a0172d3e0ad18fd2d2a8d6ad840be23) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "djb004d.u1", 0x0000, 0x040000, CRC(83f042e8) SHA1(70c1718a24ad3f3b10d5cc15eca1623053a3d148) )
ROM_END

ROM_START( as_djpb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "djv1-09.u1", 0x00000, 0x080000, CRC(1165ab7b) SHA1(6eb8d6ee7cede06175e0d265ad26fc843ddae43a) )
	ROM_LOAD16_BYTE( "djv1-09.u2", 0x00001, 0x080000, CRC(5690e543) SHA1(328ee733dab1346fa1db8effb48fd0ba07d3d16c) )
ROM_END




ROM_START( as_cshcs )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ccv0-06.bin", 0x0000, 0x100000, CRC(c211f807) SHA1(71fc611872f81ad66a476efaa22d3c7862619331) )
ROM_END



ROM_START( as_colmn )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1com2_00.bin", 0x00000, 0x040000, CRC(ae1fcf19) SHA1(8f5ec95940e2e7e1ac6d58be69937e460bde9a21) )
	ROM_LOAD16_BYTE( "2com2_00.bin", 0x00001, 0x040000, CRC(19183568) SHA1(67ccb77077566a14118b2330e55dec09d7382038) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "1com2_00g.bin", 0x0000, 0x040000, CRC(85a068b3) SHA1(c56ff3b2ade04036b991432d5a8f3cd13aa4b729) )
	ROM_LOAD16_BYTE( "1com2_00p.bin", 0x0000, 0x040000, CRC(7a72968b) SHA1(cb60a4532516651d649d2dfee262d339a22eaeb1) )
ROM_END

ROM_START( as_colmna )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ccm1_07.bin", 0x0000, 0x080000, CRC(aa1695fe) SHA1(46d0a45036da3afdfd347eeff7a48a417fb02f9d) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ccm1_07g.bin", 0x0000, 0x080000, CRC(35af6ece) SHA1(c5240c858da3a04b8970b2ff6d54333c25d140a0) )
	ROM_LOAD( "ccm1_07p.bin", 0x0000, 0x080000, CRC(a3ed0a4d) SHA1(0f8f75cd7ff508a723647fde1f6db0388f3ab44f) )
ROM_END

ROM_START( as_colmnb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ccm108.u1", 0x0000, 0x080000, CRC(d252ac41) SHA1(ba663d67e80677184f54d5c059cc671b489f0904) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ccm108d.u1", 0x0000, 0x080000, CRC(dba933f2) SHA1(cd0aae061040e88cbe754b24122096b2e2d776c2) )
ROM_END

ROM_START( as_colmnc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ccm109.u1", 0x0000, 0x080000, CRC(2a8c2794) SHA1(1ef9aacd3eb2776fd8097653fab69a78da4ee8cc) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ccm109d.u1", 0x0000, 0x080000, CRC(2377b827) SHA1(58d3ea4cb2780b45ec696df5d301ad0e9cf6e0b6) )
ROM_END

ROM_START( as_colmnd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "com908.u1", 0x0000, 0x080000, CRC(0938b4f3) SHA1(1e9ca5ba379472fbd982a764757218b2bd79af5b) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "com908d.u1", 0x0000, 0x080000, CRC(00c32b40) SHA1(5757a9203d0e07e6e599e3de4c5757532bc2ae2f) )
ROM_END





ROM_START( as_jackb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "jb104.u1", 0x0000, 0x080000, CRC(a13596c7) SHA1(93a14cce239d0d9e4acb1ab87d0e077526576bf6) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "jb104d.u1", 0x0000, 0x080000, CRC(a8ce0974) SHA1(98ae1d0ac0b690430d4d07bfc5011d7150231ff3) )
ROM_END



ROM_START( as_jpx )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jpx51-00.u1", 0x00000, 0x080000, CRC(87a1984d) SHA1(e82ebbef81e43c465974eaec81a74723b41f9039) )
	ROM_LOAD16_BYTE( "jpx51-00.u2", 0x00001, 0x080000, CRC(4cb35408) SHA1(e134edfc841f6ce4562807eb59282d0bce30d5c4) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "jpx5100d.u1", 0x0000, 0x080000, CRC(8e5a07fe) SHA1(153cca448edd11c6a0300892695cf3a0e8b381f7) )
	ROM_LOAD16_BYTE( "jpx5100g.u1", 0x0000, 0x080000, CRC(ef6e755f) SHA1(a2bfbd410c8d51288c6da1e765cf0759326e406a) )
ROM_END


ROM_START( as_jpxa )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jpx101-2.u1", 0x00000, 0x080000, CRC(e7596323) SHA1(97f75ba13f560d8c41cc3f6d4c3c6071eaa0cfa6) )
	ROM_LOAD16_BYTE( "jpx1-01.u2",  0x00001, 0x080000, CRC(988f7d4f) SHA1(8124e11126510a0a7f12d19e2ae70149618c8a83) )
ROM_END

ROM_START( as_jpxb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jx002d.u1", 0x00000, 0x080000, CRC(7a62d914) SHA1(66fc9ac8951f3ca3c37202e7c56e1fcdbfa2c368) )
	ROM_LOAD16_BYTE( "jx002.u2",  0x00001, 0x080000, CRC(18afeeff) SHA1(1f0616138f165481c55810b1160b593e24fc8570) )
ROM_END



ROM_START( as_jolly )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "jr1_03.bin", 0x0000, 0x080000, CRC(8b69d270) SHA1(c9daa9d2b50d49a771424309aeafc5dc033916a8) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "jr1_03g.bin", 0x0000, 0x080000, CRC(14d02940) SHA1(0c949489da595603b69e400bcf0da1e850760c48) )
	ROM_LOAD( "jr1_03p.bin", 0x0000, 0x080000, CRC(82924dc3) SHA1(138123ced84d1e2ad4df33142837a54afdfaad54) )
	ROM_LOAD( "jrv1-03.bin", 0x0000, 0x080000, CRC(8b69d270) SHA1(c9daa9d2b50d49a771424309aeafc5dc033916a8) )
ROM_END



ROM_START( as_jmpj )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "jj1-00p.bin", 0x0000, 0x080000, CRC(36ff9a47) SHA1(46416d0256c905dbbd7b1b40531180cc9b6f59eb) )
ROM_END

ROM_START( as_jmpja )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "jj102.u1", 0x0000, 0x080000, CRC(0242e991) SHA1(e606c14c64a574a54dfe5e26fab29d5ef15aefd0) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "jj102d.u1", 0x0000, 0x080000, CRC(0bb97622) SHA1(d39878237a824e4450d9bc2f9681c12551d7cb9d) )
ROM_END



ROM_START( as_jjive )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jpush107.u1", 0x00000, 0x100000, CRC(25342d3f) SHA1(fe72985bf80f4221609caa49cf0cfed718a81a25) )
	ROM_LOAD16_BYTE( "jpush107.u2", 0x00001, 0x100000, CRC(b86ff64b) SHA1(f246c891919300e3eff6bcf6117e60652e11f3ff) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "jpus107d.u1", 0x0000, 0x100000, CRC(09d9a153) SHA1(6e07c77a561e62a59c1db353fef463df9bfc5525) )
ROM_END

ROM_START( as_jjivea )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jpush106.u1", 0x00000, 0x100000, CRC(cca6805e) SHA1(a3dd55edb2b82deb5a24216b66a7834962dff653) )
	ROM_LOAD16_BYTE( "jpush106.u2", 0x00001, 0x100000, CRC(e2de49eb) SHA1(be99f68124f6315a54250003fcc185cb8dc77c4f) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "jpus106d.u1", 0x0000, 0x100000, CRC(e04b0c32) SHA1(9173ac327eb7a62a81194e81eb23045fbd4caf35) )
ROM_END

ROM_START( as_jjiveb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jpush104.u1", 0x00000, 0x100000, CRC(3ef7c678) SHA1(05a39a20e14f1b94a849cec587de1bf7fd436ddb) )
	ROM_LOAD16_BYTE( "jpush104.u2", 0x00001, 0x100000, CRC(b4b52aa8) SHA1(638b72f92df6e42dad1e43e1f531a8bd239a6bbe) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "jpsh104d.u1", 0x0000, 0x100000, CRC(121a4a14) SHA1(98dca858b6e38aba7456065462b2865c0bfdc566) )
ROM_END

ROM_START( as_jjivec )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jpush102.u1", 0x00000, 0x100000, CRC(3d005f61) SHA1(f75aff32c61136316c9eae9d81d0d2a1dd8c008e) )
	ROM_LOAD16_BYTE( "jpush102.u2", 0x00001, 0x100000, CRC(e8b4557e) SHA1(b980368bd32e66bfd1b30de730ffc4ea841815eb) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "jpus102d.u1", 0x0000, 0x100000, CRC(11edd30d) SHA1(1e30aad77725fa52434339c73e43b0960c366a6d) )
ROM_END

ROM_START( as_jjived )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jpush101.u1", 0x00000, 0x100000, CRC(6baab9c4) SHA1(884379acd0fa168cf71319ae023aa7bead7653a1) )
	ROM_LOAD16_BYTE( "jpush101.u2", 0x00001, 0x100000, CRC(0882cd5e) SHA1(f728678ec24febb05d2e05c70ef94e7722d7fa09) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "jpsh101d.u1", 0x0000, 0x100000, CRC(474735a8) SHA1(9ff6f17a052c26a837a09c81d0c8123040cf4c9b) )
ROM_END

ROM_START( as_jjivee )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jj101.u1",  0x00000, 0x080000, CRC(108524ba) SHA1(8cd3d33a0f3237b183e397ad61555fdd2d501fbe) )
	ROM_LOAD16_BYTE( "jjr101.u2", 0x00001, 0x080000, CRC(535b367a) SHA1(61d11f9b0f4536b8dd7625f96c24c2f5589031e4) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "jj101d.u1", 0x0000, 0x080000, CRC(197ebb09) SHA1(018489eac9a3ecc8fbdf2a4d7273fc561cb40f10) )
ROM_END


ROM_START( as_jjivef )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jjb004.u1", 0x00000, 0x080000, CRC(af253f87) SHA1(0d39d442102f27556da70011c1085fbbb88ce015) )
	ROM_LOAD16_BYTE( "jjb004.u2", 0x00001, 0x080000, CRC(e35fc34d) SHA1(bfa13bcc0e4d14d0320282f4624f10f47e2a717d) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "jjb004d.u1", 0x0000, 0x080000, CRC(a6dea034) SHA1(9e13576edb4e81d7f44a25d6ffdb5deb7ed440cb) )
ROM_END

ROM_START( as_jjiveg )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jp005.u1", 0x00000, 0x100000, CRC(2528325a) SHA1(653dc439fd4a73559772cbc7154380c0e3e36200) )
	ROM_LOAD16_BYTE( "jp005.u2", 0x00001, 0x100000, CRC(888b53ea) SHA1(166a073c8f54cfd41e75a2eed5d0d41d749192a9) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "jp005d.u1", 0x0000, 0x100000, CRC(09c5be36) SHA1(173e86d803c00a191fdf7361ce2d56776e80b5c0) )
ROM_END




ROM_START( as_kingc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "kc106.u1", 0x0000, 0x080000, CRC(097b8a9c) SHA1(fa2f546f5fb2bf11ebca314fc86d4ce663fe4058) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "kc106d.u1", 0x0000, 0x080000, CRC(0080152f) SHA1(89f2a973e8b1aa0f1e135b756d2fcee816c86370) )
ROM_END

ROM_START( as_kingca )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "kc1_03.bin", 0x0000, 0x080000, CRC(787011fe) SHA1(d1078c4671c98640b6a215b5a7b16a0041946b21) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "kc1_03d.bin", 0x0000, 0x080000, CRC(718b8e4d) SHA1(217285a715fa09120635f2fa6f399d8f361ebd32) )
ROM_END


ROM_START( as_koc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "koc2_00.bin", 0x0000, 0x080000, CRC(8512166a) SHA1(b9b081419d58a6b788b35d4535d34efbadf62e2f) )
ROM_END

ROM_START( as_koca )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "koc101.bin", 0x0000, 0x080000, CRC(8e16f681) SHA1(e8448e7a3b96199f04458c48554a6b2cb27541f2) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "koc101d.bin", 0x0000, 0x080000, CRC(87ed6932) SHA1(03a6639fcf46f699a29b37fad6735343a6fd5cd4) )
ROM_END



ROM_START( as_lbt )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "lbt103.u1", 0x0000, 0x080000, CRC(0b73f9a1) SHA1(f88c475fb9e020962068ab6c0a1533d6ddbda877) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "lbt103d.u1", 0x0000, 0x080000, CRC(02886612) SHA1(77c491233128117928a962a7027e5024778268cb) )
ROM_END

ROM_START( as_lbta )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "lbt1_02.bin", 0x0000, 0x080000, CRC(aa6c8b06) SHA1(47b5ce80fb92563c07362c3e8b40741bbd426b50) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "lbt1_02g.bin", 0x0000, 0x080000, CRC(a39714b5) SHA1(59db3ca9f7fbd30fcc905cbfe0caafa80f0d8527) )
	ROM_LOAD( "lbt1_02p.bin", 0x0000, 0x080000, CRC(35d57036) SHA1(b88412730a34322fd7501da4d3c32272d493891f) )
ROM_END



ROM_START( as_ldl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ld700.u1", 0x00000, 0x040000, CRC(62594e4d) SHA1(486655920dad4cc90c45fcbb13328246876273fd) )
	ROM_LOAD16_BYTE( "ld700.u2", 0x00001, 0x040000, CRC(03939717) SHA1(d2446ad9bf436ca382c839ff946878fef9d50c6f) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ld700d.u1", 0x0000, 0x040000, CRC(b63417df) SHA1(6a872f8bd29939b39b62e1e11692e38e313b4954) )
ROM_END

ROM_START( as_ldla )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ld600.u1", 0x00000, 0x040000, CRC(1def9345) SHA1(d06dbb7858cef18fd01f579cfe59e01958c7a275) )
	ROM_LOAD16_BYTE( "ld600.u2", 0x00001, 0x040000, CRC(7e93bf7f) SHA1(e44b28d564d279db765181810b39ad3d600f6d45) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ld600d.u1", 0x0000, 0x040000, CRC(c982cad7) SHA1(3a472a0e047489d12684fdff024bf763cdaaf0a4) )
ROM_END

ROM_START( as_ldlb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1ld3_12.bin", 0x00000, 0x040000, CRC(f5cd8c64) SHA1(423767374d54823d0b2fe3628580914aa01433af) )
	ROM_LOAD16_BYTE( "2ld3_12.bin", 0x00001, 0x040000, CRC(a1999a2d) SHA1(efeab7695c721826b7df56d334df2136a361995f) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "1ld3_12g.bin", 0x0000, 0x040000, CRC(de722bce) SHA1(00ffd2f0f6729d34428dc4f26716425c1097339f) )
ROM_END

ROM_START( as_ldlc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ldb003.u1", 0x00000, 0x040000, CRC(e9df9b88) SHA1(0febc7c374b2d3136ce86a4957a4174233d12eab) )
	ROM_LOAD16_BYTE( "ldb003.u2", 0x00001, 0x040000, CRC(278223e7) SHA1(3e96d7abf0876acc10922c4585143745f1740c00) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ldb003d.u1", 0x0000, 0x040000, CRC(3db2c21a) SHA1(086e42660eaa477617efd98ceb0e1f09557594be) )
	ROM_LOAD16_BYTE( "ldb003g.u1", 0x0000, 0x040000, CRC(c2603c22) SHA1(2a0fac19c6de1906b28590c7c8462b86963deb1f) )
ROM_END

ROM_START( as_ldld )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ld102.u1", 0x0000, 0x080000, CRC(59e12987) SHA1(cde1c23637a57001ac2eae4c842be3b18fab68b2) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ld102d.u1", 0x0000, 0x080000, CRC(501ab634) SHA1(cead0d33bdaa94638196273cd7a681478c713f27) )
ROM_END

ROM_START( as_ldle )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ld1-03.bin", 0x0000, 0x080000, CRC(6edb3d2d) SHA1(caebdba44ced2a45e48f28b2fe1660d747cc34c4) )
ROM_END




ROM_START( as_mp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mpv118.u1", 0x00000, 0x080000, CRC(4efef116) SHA1(44da5e9ebfdcb5e2d0a5c5312512a2dc08088c37) )
	ROM_LOAD16_BYTE( "mpv118.u2", 0x00001, 0x080000, CRC(b5b54977) SHA1(ab67195d6fc3414e3d9ba86837d352a90340544e) )
ROM_END



ROM_START( as_mp2 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "mp2v1-14.bin", 0x0000, 0x080000, CRC(131fff7a) SHA1(69bb7fa011f6e7687e280126275651093d7f33f9) )
ROM_END



ROM_START( as_otr )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "otr1_041.bin", 0x00000, 0x040000, CRC(2ce53cb2) SHA1(fae967243789d42a410b93fd2b7bc006f1a4a83e) )
	ROM_LOAD16_BYTE( "otr1_042.bin", 0x00001, 0x040000, CRC(81ea6a43) SHA1(4cbbc892c720c333e9f62b6f6fdd0a959c67af7e) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "otr1_04g.bin", 0x0000, 0x040000, CRC(075a9b18) SHA1(d9ded006677be6b2e9a4eec82488147b45152c52) )
	ROM_LOAD16_BYTE( "otr1_04p.bin", 0x0000, 0x040000, CRC(f8886520) SHA1(04c2d5c4954b12480311cac0973a655464e53d7e) )
ROM_END

ROM_START( as_otra )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "otr102.u1", 0x00000, 0x040000, NO_DUMP )
	ROM_LOAD16_BYTE( "otr102.u2", 0x00001, 0x040000, CRC(57310d5e) SHA1(585ff22a52e8c2b0be43051cdd1c534a894a4d6f) )
ROM_END



ROM_START( as_ptf )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pfm_s102.u1", 0x00000, 0x040000, CRC(5cf6ce44) SHA1(762b3bb4abc0e47c699b4e16aeedcf8603af5189) )
	ROM_LOAD16_BYTE( "pfm_s102.u2", 0x00001, 0x040000, CRC(ccd3c63c) SHA1(cdc9edf00703cf31bd9d145aeb631d4791d6f308) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "pfs_s102.u1", 0x00000, 0x040000, CRC(9d8b0b44) SHA1(b9e17eb77b7ba5e247be159aeeec1d7b13eab7ba) )
	ROM_LOAD16_BYTE( "pfs_s102.u2", 0x00001, 0x040000, CRC(f76a921a) SHA1(95817ab675b8f001a75ca2ea44fa529c639f5f24) )

	ROM_REGION( 0x200000, "altrevs", 0 )

	// these seem to be sound roms?!
	ROM_LOAD( "pfm_s100.u1", 0x0000, 0x100000, CRC(933da54e) SHA1(6f2769d21f934387fe2984b98f3b473938213041) )
	ROM_LOAD( "pfs_s100.u1", 0x0000, 0x100000, CRC(09358dbb) SHA1(232004a1b1ec4dc8ec5f50715677656d075cee2c) )
	ROM_LOAD( "pfs_s100.u2", 0x0000, 0x100000, CRC(0b5bee55) SHA1(57b16ee4fbf5d9478d1199b2471404bb8ee3c634) )
	ROM_LOAD( "pfm_s904.u1", 0x0000, 0x100000, CRC(933da54e) SHA1(6f2769d21f934387fe2984b98f3b473938213041) )
	ROM_LOAD( "pfs_s904.u1", 0x0000, 0x100000, CRC(881063cd) SHA1(215905468fc8b67b4e19f10047cb182a7c5851cf) )
	ROM_LOAD( "pfs_s904.u2", 0x0000, 0x100000, CRC(66c0e65b) SHA1(0b845613b6b9aed508e104fa364575e19d275ace) )

	// unmatched rom?
	ROM_LOAD( "pfs_008d.u1", 0x0000, 0x040000, CRC(5abe3aad) SHA1(5a3cfd8772901a896e3ed1545abd52e3a2bc4eac) )
ROM_END

ROM_START( as_ptfa )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pfm803.u1", 0x00000, 0x040000, CRC(f637643c) SHA1(97d2e189000c003fa68372bbd5597159fd507722) )
	ROM_LOAD16_BYTE( "pfm803.u2", 0x00001, 0x040000, CRC(e0b4d85b) SHA1(eee4c36eea128e59ca64153be4bb608df95289e3) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "pfs803.u1", 0x00000, 0x040000, CRC(d56339bd) SHA1(583ce44f746bacea4ce511ee0598e1a8034c1b28) )
	ROM_LOAD16_BYTE( "pfs803.u2", 0x00001, 0x040000, CRC(abe0576f) SHA1(1173f9df6d3fa71df018a4d4ab7d437517093e7d) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "pfs803d.u1", 0x0000, 0x040000, CRC(010e602f) SHA1(3d280bc9050d76f946604fb51ba3db67fd7216ac) )
ROM_END

ROM_START( as_ptfb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pfm905.u1", 0x00000, 0x040000, CRC(b8082360) SHA1(81496a4b81debb838da03b6d7ad6df3a64d70f6c) )
	ROM_LOAD16_BYTE( "pfm905.u2", 0x00001, 0x040000, CRC(594a7b02) SHA1(1af2e410ad4803393b6e90118c470f3a0c90902f) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "pfs905.u1", 0x00000, 0x040000, CRC(1fea9cc5) SHA1(84c1a9dc8df711e4e0c9f3a2f2a95f45525f98ed) )
	ROM_LOAD16_BYTE( "pfs905.u2", 0x00001, 0x040000, CRC(00aa7ff9) SHA1(9a064f0f9b319a4aebf95e9809dc76828c78fd75) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "pfs905d.u1", 0x0000, 0x040000, CRC(cb87c557) SHA1(672c1770a85c4e42f7099bfdaf5588eaf559190b) )
	ROM_LOAD16_BYTE( "pfs905g.u1", 0x0000, 0x040000, CRC(632a743b) SHA1(20ca639c417473c7398964d2c3665a2e67e3d5c9) )
ROM_END

ROM_START( as_ptfc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p_fruits.u1m", 0x00000, 0x040000, CRC(fcb6c345) SHA1(9ebd3dd816e711a88f337fc7d321be53a6ee90e9) )
	ROM_LOAD16_BYTE( "p_fruits.u2m", 0x00001, 0x040000, CRC(f10daa29) SHA1(5bdefb710e6003e72c6dd786ca7e5fb15e88c492) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "p_fruits.u1", 0x00000, 0x040000, CRC(8ed3633f) SHA1(3e7e139838e6eb4f6ca777033807f073a8887993) )
	ROM_LOAD16_BYTE( "p_fruits.u2", 0x00001, 0x040000, CRC(298a75f6) SHA1(8f61c83269e1b600e108d2bd3206f9cd1ade50c1) )
ROM_END






ROM_START( as_pia )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "pia202.u1", 0x0000, 0x080000, CRC(ffb5eb17) SHA1(a08fd091026d397ce0aed48bd801872b6c7939e5) )
ROM_END

ROM_START( as_piaa )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "pia206.u1", 0x0000, 0x080000, CRC(536b24d4) SHA1(38f4eb2fee6fd35d9a603fccd16f4bad46f2f1f2) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "pia206d.u1", 0x0000, 0x080000, CRC(5a90bb67) SHA1(715ae82004e5140a0a208306e837f90ec58475b9) )
ROM_END

ROM_START( as_piab )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "pia904d.u1", 0x0000, 0x080000, CRC(6fa4a228) SHA1(02ae4ae32b8cfd044344c3b194be8f76ef471af4) )
ROM_END




ROM_START( as_pharg )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pp005.u1", 0x00000, 0x100000, CRC(4cf8cc5f) SHA1(0c5ccb00013fe992e6ab63d210eda6f2ea5f39b2) )
	ROM_LOAD16_BYTE( "pp005.u2", 0x00001, 0x100000, CRC(852592d6) SHA1(9b3dc99065b720c3607b4fe454f5470cca4a18b5) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "pp005d.u1", 0x0000, 0x100000, CRC(60154033) SHA1(92a80be25ddd56865a3e253390554a334763a696) )
ROM_END


ROM_START( as_pharga )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ppush101.u1", 0x00000, 0x100000, CRC(c3f297f1) SHA1(a22144e3e7b3a42fd5574978e1e83f27a7a51842) )
	ROM_LOAD16_BYTE( "ppush101.u2", 0x00001, 0x100000, CRC(764a4e5b) SHA1(199191502d8e868075da970e28b3f9219670afc0) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ppsh101d.u1", 0x0000, 0x100000, CRC(ef1f1b9d) SHA1(7197f6cf5620ff5f945af17566ee7f08b4fd7eae) )
ROM_END

ROM_START( as_phargb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ppush102.u1", 0x00000, 0x100000, CRC(38ddfc65) SHA1(b765c882424cef30a9be03d72122df4b0a81993d) )
	ROM_LOAD16_BYTE( "ppush102.u2", 0x00001, 0x100000, CRC(761466dd) SHA1(c85b2f253755c1f8200ead16fd5bc46ce2c00477) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ppus102d.u1", 0x00000, 0x100000, CRC(14307009) SHA1(d008fdbbdc2d379609a68da30d4d54f334e9818b) )
ROM_END

ROM_START( as_phargc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ppush104.u1", 0x00000, 0x100000, CRC(4afdd7f0) SHA1(c07e413f0308f19be5af836763214fb6f36f8bc2) )
	ROM_LOAD16_BYTE( "ppush104.u2", 0x00001, 0x100000, CRC(1aece0dd) SHA1(518916a0f5affa830613f4078b3768e1ca69f6fd) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ppsh104d.u1", 0x0000, 0x100000, CRC(66105b9c) SHA1(8266dd4f7cd2ee2f124e681d0390740485ee8e87) )
ROM_END

ROM_START( as_phargd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ppush106.u1", 0x00000, 0x100000, CRC(64ed1407) SHA1(a646c5a5873d028221f6a1b5f4cee41f8ac4a752) )
	ROM_LOAD16_BYTE( "ppush106.u2", 0x00001, 0x100000, CRC(9af70eec) SHA1(23e1761bbc989f659f1b3bfc4d9fe0369571fb21) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ppus106d.u1", 0x0000, 0x100000, CRC(4800986b) SHA1(758c98ce42a50fd693f059b61dc99918541e3053) )
ROM_END

ROM_START( as_pharge )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ppush107.u1", 0x00000, 0x100000, CRC(5044fd1e) SHA1(efe43cc996b18e89da7a4d8c789e1988aff205c4) )
	ROM_LOAD16_BYTE( "ppush107.u2", 0x00001, 0x100000, CRC(488e57e6) SHA1(1281d47bad90562d478696292a92e57c678a6d7d) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ppus107d.u1", 0x0000, 0x100000, CRC(7ca97172) SHA1(0d6e67489cfdafa4202cccd6addacd9446c7d155) )
ROM_END




ROM_START( as_pb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "pb105.u1", 0x0000, 0x080000, CRC(31b8ebe9) SHA1(510c6f54f411d10ea378834c074650ed05363b28) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "pb105d.u1", 0x0000, 0x080000, CRC(3843745a) SHA1(7432eef2f49f7463ca9e7d78a380738d639a3fe4) )
	ROM_LOAD( "pb105g.u1", 0x0000, 0x080000, CRC(ae0110d9) SHA1(8e3c0a27c57768ef0f3f9b821c2c839abf6d9712) )
ROM_END




ROM_START( as_rtr )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rr1h101.u1", 0x00000, 0x080000, CRC(5563f2cd) SHA1(da8515e17b3d0807ac79896f3eca29b886ce5e08) )
	ROM_LOAD16_BYTE( "rr1h101.u2", 0x00001, 0x080000, CRC(12b0c111) SHA1(319d75f3f04570bc1012facbe0238484cb93e437) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rr1h101d.u1", 0x0000, 0x080000, CRC(5c986d7e) SHA1(9b82d9c2fae0a196e3dc5ae04a78f6c75066bd49) )
ROM_END

ROM_START( as_rtra )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rr1l101.u1", 0x00000, 0x080000, CRC(162b21fd) SHA1(29072fdaeeeba7dfcf446b77f5f8af0fb08c5f19) )
	ROM_LOAD16_BYTE( "rr1l101.u2", 0x00001, 0x080000, CRC(f3158104) SHA1(d5d19af94926a71efda732df2a4c53bcbf512ba9) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rr1l101d.u1", 0x0000, 0x080000, CRC(1fd0be4e) SHA1(b07ad428eec8e58b7ab407ba7b4b02070a8c95f6) )
ROM_END

ROM_START( as_rtrb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rtr101.u1", 0x00000, 0x080000, CRC(8d89cf0b) SHA1(42d7594ac55dc4bde2830e4d19af01fd3ccd6b2b) )
	ROM_LOAD16_BYTE( "rtr101.u2", 0x00001, 0x080000, CRC(8b1ddddf) SHA1(c394f836f1cf07ecf82a00de360bcce93b25e38f) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rtr101d.u1", 0x0000, 0x080000, CRC(847250b8) SHA1(a1ec0168043118c70494825de8b1e3ebaf0bddeb) )
ROM_END

ROM_START( as_rtrc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rr2h101.u1", 0x00000, 0x080000, CRC(0188165b) SHA1(13d88e6c832bd5d299dd6126659bdf541035fcdd) )
	ROM_LOAD16_BYTE( "rr2h101.u2", 0x00001, 0x080000, CRC(810c1075) SHA1(61bc3bf2715a72fb41f334b9f93e30a48d73eda6) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rr2h101d.u1", 0x0000, 0x080000, CRC(2ddd972d) SHA1(fbdf60677526d45bc11184b10aea4bc2c64d1096) )
ROM_END

ROM_START( as_rtrd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rrdh100.u1", 0x00000, 0x080000, CRC(5c06124e) SHA1(a3d7a3248c97db37a6b140a88cd7468bfd9ea1b6) )
	ROM_LOAD16_BYTE( "rrdh100.u2", 0x00001, 0x080000, CRC(39a0aa8d) SHA1(02c89be8f3c0f68207776955e3f5e1b5542f9c47) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rrdh100d.u1", 0x0000, 0x080000, CRC(55fd8dfd) SHA1(ab5dea2db12f848783d36960c235735473540df2) )
ROM_END

ROM_START( as_rtre )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rrdl100.u1", 0x00000, 0x080000, CRC(54e4ee29) SHA1(3bdf32515bbaaca44e18f947a00ef5aced5dd5e3) )
	ROM_LOAD16_BYTE( "rrdl100.u2", 0x00001, 0x080000, CRC(04ce1f27) SHA1(efec934f6ccc23545ffbc89454cb6ce8de47a018) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rrdl100d.u1", 0x0000, 0x080000, CRC(5d1f719a) SHA1(1a1f45e008c85f9b41a73d373779bc531af68792) )
ROM_END

ROM_START( as_rtrf )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rrlo200.u1", 0x00000, 0x080000, CRC(206772f9) SHA1(9b577dc568a84943bd02e18611a0d9451d74a123) )
	ROM_LOAD16_BYTE( "rrlo200.u2", 0x00001, 0x080000, CRC(314778a6) SHA1(85dbc2b68cdc9bdcce28941dec56a5539fcc5f3a) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rrlo200d.u1", 0x0000, 0x080000, CRC(299ced4a) SHA1(94038d38c9d845ae72b110631bc867ed4bb0db77) )
ROM_END


ROM_START( as_rtrg )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rrhi200.u1", 0x00000, 0x080000, CRC(c65a4844) SHA1(a6513c5fcf45b8a2294f996813b83771f87efdb9) )
	ROM_LOAD16_BYTE( "rrhi200.u2", 0x00001, 0x080000, NO_DUMP ) // ??

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rrhi200d.u1", 0x0000, 0x080000, CRC(cfa1d7f7) SHA1(af54edc00b845a987eb1b6b5a2185b21b11fca72) )
ROM_END

ROM_START( as_rtrh )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rrhi202.u1", 0x00000, 0x080000, CRC(9e4751d7) SHA1(209c8d025cbdf1f411cddc6724af22b285220e75) )
	ROM_LOAD16_BYTE( "rrhi202.u2", 0x00001, 0x080000, CRC(703c2a60) SHA1(0cdb1643c2775556c46db7955b110e36d80de61e) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rrhi202d.u1", 0x0000, 0x080000, CRC(97bcce64) SHA1(d7c51bd821cc6a4c436f349828717ebd2ae3a41b) )
ROM_END








ROM_START( as_rab )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rab105.u1", 0x0000, 0x080000, CRC(08185159) SHA1(3185009055fd8c08a58d50635641187a47044c9a) )
	//ROM_LOAD( "rab1_05.bin", 0x0000, 0x080000, CRC(08185159) SHA1(3185009055fd8c08a58d50635641187a47044c9a) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rab105d.u1", 0x0000, 0x080000, CRC(01e3ceea) SHA1(6fdf49702b7df122fc681147122322952ce4f037) )
	ROM_LOAD( "rab1_05g.bin", 0x0000, 0x080000, CRC(97a1aa69) SHA1(17bf853eefd38f4dca5e4df20328101466b2564f) )
ROM_END

ROM_START( as_raba )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rab106.u1", 0x0000, 0x080000, CRC(2df108ad) SHA1(a5221e9fa6966fec1506002a8c4646013d184887) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rab106d.u1", 0x0000, 0x080000, CRC(240a971e) SHA1(e48811389b9781a9e4da57b69b77bd74924f60fc) )
ROM_END

ROM_START( as_rabb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rab107.u1", 0x0000, 0x080000, CRC(4bc3af26) SHA1(68b8999301c275168c9199d64f0a6a624bf068db) )
ROM_END

ROM_START( as_rabc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rab1_04.bin", 0x0000, 0x080000, CRC(df2b4bb7) SHA1(1a664f9a7085606b6f3c2e26d37cd853538a2d0e) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rab1_04g.bin", 0x0000, 0x080000, CRC(4092b087) SHA1(291fd9c8a8d30e28bb764eee9aa778f93b42dafb) )
	ROM_LOAD( "rab1_04p.bin", 0x0000, 0x080000, CRC(d6d0d404) SHA1(90c503bb9cdd9eef01384499828c0421e657d35f) )
ROM_END


ROM_START( as_rbg )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rbg304.u1", 0x0000, 0x080000, CRC(360932ed) SHA1(8ccb3d1aa99f4d23f0b20a42d32895e77f810fd1) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rbg304d.u1", 0x0000, 0x080000, CRC(3ff2ad5e) SHA1(32015e5d6734a4abf3f0e7107e7be3132c26986b) )
ROM_END

ROM_START( as_rbga )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rbg303.u1", 0x0000, 0x080000, CRC(1aa5b429) SHA1(c32ff4492db206dffb7725b78d247333d5095c3a) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rbg303d.u1", 0x0000, 0x080000, CRC(135e2b9a) SHA1(cb7dcfd6b5ce1e14fb4115f5bfd923162be53a28) )
ROM_END

ROM_START( as_rbgb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rbg104.u1", 0x0000, 0x080000, CRC(d1bae82d) SHA1(7334b73e1cab339a09e526de7737511810a37d97) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rbg104d.u1", 0x0000, 0x080000, CRC(d841779e) SHA1(1550ccd14c38c618ccba45d6e49613e2eb6b5e31) )
ROM_END

ROM_START( as_rbgc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rbg1_02.bin", 0x0000, 0x080000, CRC(ff70e9f9) SHA1(dced84105db4d81192383d7dfa31b57a88e440db) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rbg1_02g.bin", 0x0000, 0x080000, CRC(54e16865) SHA1(e3196899c5dc5418ec9f355907dc94d7d96a6313) )
	ROM_LOAD( "rbg1_02p.bin", 0x0000, 0x080000, CRC(f68b764a) SHA1(c40c8c7139fb8ab7c78dc0c969dd59a2bf9a69bf) )
ROM_END

ROM_START( as_rbgd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rbg1_01.bin", 0x0000, 0x080000, CRC(2cc1f8f0) SHA1(7710a72fb9abbcc666a09eb1e4a9bb24cc7cfc2f) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rbg1_01g.bin", 0x0000, 0x080000, CRC(b37803c0) SHA1(29343f7d35efb87a7ea17f83234fd445ae5eafd5) )
	ROM_LOAD( "rbg1_01p.bin", 0x0000, 0x080000, CRC(253a6743) SHA1(7f50ddd1fe1c57bbe8b591fe204761fe1737dad3) )
ROM_END

ROM_START( as_rbge )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rbg0_08.bin", 0x0000, 0x080000, CRC(0906c9d7) SHA1(3a6a5c644398803857672470495a76ef2ff916f0) )
ROM_END

ROM_START( as_rbglo )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rgmv1-06.u1", 0x00000, 0x040000, CRC(3a935eed) SHA1(236acbb4defb007c8dcd63937cd1b3dd50b9dd49) )
	ROM_LOAD16_BYTE( "rgmv1-06.u2", 0x00001, 0x040000, CRC(ab89ff35) SHA1(88a45dfeb98beac47b578cb5679507b4a6dc0fff) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "rgsv1-06.u1", 0x00000, 0x040000, CRC(13f46cb6) SHA1(10a46bfb0c4efb768882c36dbc5c0c88f45c0d13) )
	ROM_LOAD16_BYTE( "rgsv1-06.u2", 0x00001, 0x040000, CRC(408d6007) SHA1(875f812a1d6a6177e45199fba0e5ca04a2fff02c) )
ROM_END


ROM_START( as_rox )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rox006.u1", 0x0000, 0x080000, CRC(8e63812c) SHA1(240810641896a2af22abe362752206061a9d3866) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "rox006d.u1", 0x0000, 0x080000, CRC(87981e9f) SHA1(e18897e2106c8454d9e7adf8fb6217b0739facc6) )
	ROM_LOAD( "rolx101.bin", 0x000000, 0x080000, CRC(60723c2b) SHA1(3764c989cd835e66ebde70b45ae005346b0a278c) )
	ROM_LOAD( "rolx101d.bin", 0x000000, 0x080000, CRC(6989a398) SHA1(c853b57c93b93325913027733b0da67091e480e2) )
	ROM_LOAD( "rolx101g.bin", 0x000000, 0x080000, CRC(08bdd139) SHA1(38606f53179536886a2ad47c248c3a46503311e9) )
ROM_END


ROM_START( as_csv ) // csv code is also used in as_stcl, I suspect this is either not what it's meant to be, or some of those roms are this
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "csv2-02g.bin", 0x0000, 0x080000, CRC(8c903573) SHA1(ba9fe333ca3f4f21114fb052c85efa245059fd05) )
ROM_END


ROM_START( as_sltcl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "csbv9-03.bin", 0x0000, 0x080000, CRC(5c47f413) SHA1(e376cd98d8881e8dfdf91ec2fb2611719059bef4) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "csb9-03d.bin", 0x0000, 0x080000, CRC(55bc6ba0) SHA1(48567733c05235cf642b97f94665f4e510f1e338) )
	ROM_LOAD( "csb9-03g.bin", 0x0000, 0x080000, CRC(27fcff02) SHA1(ae75ba887877f63ae5168c7b12eff8964ec17d99) )


	ROM_LOAD( "csbv9-02.bin", 0x0000, 0x080000, CRC(6034c393) SHA1(6f4eb3c881abe0605d1af39ad1d84d47a9169712) )

	ROM_LOAD( "csv0-05.bin", 0x0000, 0x080000, CRC(4b508216) SHA1(ff656cf89c27ffe3d492a97d1677120677971b28) )
	ROM_LOAD( "csv0-05d.bin", 0x0000, 0x080000, CRC(42ab1da5) SHA1(e1c87605213bb92fc49ff88c50ae51e9cf088320) )
	ROM_LOAD( "csv0-05g.bin", 0x0000, 0x080000, CRC(239f6f04) SHA1(663e0ad8f1df0ecd609fd7aae6050eca625e60bc) )


	ROM_LOAD( "csv0-06.bin", 0x0000, 0x080000, CRC(20f2d45e) SHA1(95b2fc6e3846a6d541bc2c3bd3201d8d3e0cf614) )
	ROM_LOAD( "csv0-06d.bin", 0x0000, 0x080000, CRC(29094bed) SHA1(d53bc58eb9effae054a937c2090d73e31467cca3) )

	ROM_LOAD( "csv0-07.bin", 0x0000, 0x080000, CRC(691b54e3) SHA1(4095be3699b5e50330aaa82c9187b20bf65cac02) )
	ROM_LOAD( "csv0-07d.bin", 0x0000, 0x080000, CRC(60e0cb50) SHA1(be5c3ffee58629cbae36498bf5aa02b8657dd97a) )
	ROM_LOAD( "csv0-07g.bin", 0x0000, 0x080000, CRC(12a05ff2) SHA1(7aad34aa9308114e02670827193ee8c03bf2f35a) )

	ROM_LOAD( "csbv0-03.bin", 0x0000, 0x080000, CRC(5ea64c87) SHA1(8f4afb72505b61299b2292d4fe6c2cea61b3db24) )

	ROM_LOAD( "csb0-03d.bin", 0x0000, 0x080000, CRC(575dd334) SHA1(ba6e687de19f9b8879dacd018556e773357391d9) )

	ROM_LOAD( "csv1-02.bin", 0x0000, 0x080000, CRC(d4ec739e) SHA1(3b8adc3fd663f1de0152bdaaa7d311f9c95fd6a4) )
	ROM_LOAD( "csv1-02d.bin", 0x0000, 0x080000, CRC(dd17ec2d) SHA1(b63514d0f134aef4f361701c8e92a7dcbdff24c1) )

	ROM_LOAD( "cs101.u1", 0x0000, 0x080000, CRC(4d3d2244) SHA1(ee40e5d2bb780606602e72d443f3274dfc49c8d9) )
	ROM_LOAD( "cs101d.u1", 0x0000, 0x080000, CRC(44c6bdf7) SHA1(a576978360d1b202d9aca465c1a016dbeba229f1) )

	ROM_LOAD( "csbv0-02.bin", 0x0000, 0x080000, CRC(aa4e4f8b) SHA1(cacf939e4f983f0917569dd813458859af5783ee) )

	ROM_LOAD( "csv2-01.bin", 0x0000, 0x080000, CRC(a033718e) SHA1(9bea77a5ced59b70082be6aeb317c14f9bb4368f) )
	ROM_LOAD( "csv2-01d.bin", 0x0000, 0x080000, CRC(a9c8ee3d) SHA1(daf2ee5932135f4988ee8b85f5963f980e44e54d) )
	ROM_LOAD( "csv2-01g.bin", 0x0000, 0x080000, CRC(c8fc9c9c) SHA1(9dd47de85bb91f71590f1b8efa1ffacdda6004c3) )

	ROM_LOAD( "csv2-02.bin", 0x0000, 0x080000, CRC(e45fd861) SHA1(bf948df554c9c7d8eea860c43d26ef990858bd38) )
	ROM_LOAD( "csv2-02d.bin", 0x0000, 0x080000, CRC(eda447d2) SHA1(3f8774bd9d19979fe30f10b8c2b1600bac7a38b1) )
ROM_END

ROM_START( as_stp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1st1_03.bin", 0x00000, 0x080000, CRC(6f34a144) SHA1(69f7e58bf7d868f75ef3f4242a0b6624452a3558) )
	ROM_LOAD16_BYTE( "2st1_03.bin", 0x00001, 0x080000, CRC(cc04e763) SHA1(532c914d4925d19b2c416d29711e7eb506eb75e5) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "1st1_03g.bin", 0x0000, 0x080000, CRC(f08d5a74) SHA1(ee2c0fb15354178880f9ecf82f6d5f3ca3f0879d) )
	ROM_LOAD16_BYTE( "1st1_03p.bin", 0x0000, 0x080000, CRC(66cf3ef7) SHA1(c70bc4a19b8843644c2ef30fb0510cc7cd432de3) )
ROM_END


ROM_START( as_stpa )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "stp102.u1", 0x00000, 0x080000, CRC(8df15c17) SHA1(45f6bdd3e213cea5de5feca8e2c4658b17507014) )
	ROM_LOAD16_BYTE( "stp102.u2", 0x00001, 0x080000, CRC(a06d6e4a) SHA1(d0ca62e98863ae9d07a8f81f8e9c0d655c0eab2b) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "stp102d.u1", 0x0000, 0x080000, CRC(840ac3a4) SHA1(d843c0884bfe470ea3bcf778e1786869fb032619) )
ROM_END

ROM_START( as_stpb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "stp105.u1", 0x00000, 0x080000, CRC(c3c600b4) SHA1(e570ed887e8aad6b816872c8468e15ab5b904ad5) )
	ROM_LOAD16_BYTE( "stp105.u2", 0x00001, 0x080000, CRC(7f23fe83) SHA1(e8b0a141ca3c578a1c0d49b237b309326a01300d) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "stp105d.u1", 0x0000, 0x080000, CRC(ca3d9f07) SHA1(2f20259aa1adf5a7144bd8e35746b10a2f9e5f54) )
ROM_END


ROM_START( as_siu )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "siu202.u1", 0x00000, 0x040000, CRC(35889e31) SHA1(66072aa0f2499479e41120d642a2d4fcba8d5e9e) )
	ROM_LOAD16_BYTE( "siu202.u2", 0x00001, 0x040000, CRC(0c68566c) SHA1(97fd26c5e514df137c29521ead71386fa6c1cc8e) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "siu202d.u1", 0x0000, 0x040000, CRC(e1e5c7a3) SHA1(645ec433fcfe476996cd972ae9465c27a0957be3) )
	ROM_LOAD16_BYTE( "siu202g.u1", 0x0000, 0x040000, CRC(1e37399b) SHA1(76262cbaf16a79eb93dbd78b0e340156fbb39d44) )

	// sound roms?
	ROM_LOAD( "siu_s101.u1", 0x0000, 0x100000, CRC(b4f4d737) SHA1(51f1c81a10d65325edd86bc7cbde19b9a53455fe) )
	ROM_LOAD( "siu_s101.u2", 0x0000, 0x100000, CRC(c91e30e6) SHA1(08b517abf960d66aed6700c29f37bc7bcfacb4d9) )
ROM_END

ROM_START( as_siua )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "siu203.u1", 0x00000, 0x040000, CRC(63d8fb2f) SHA1(5d9cb707169281700e76a180a362430c563515be) )
	ROM_LOAD16_BYTE( "siu203.u2", 0x00001, 0x040000, CRC(de111417) SHA1(d1535ed408105797fa7efbbbad209027456ee1a5) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "siu203d.u1", 0x0000, 0x040000, CRC(b7b5a2bd) SHA1(d41d39f29173160ca952b714f9d5726efc831694) )
	ROM_LOAD16_BYTE( "siv2-03g.u1", 0x0000, 0x040000, CRC(b1e68d79) SHA1(6dc55773fc39de1026deddc553a7474403572de4) )
ROM_END


ROM_START( as_sld )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "sld700.u1", 0x0000, 0x080000, CRC(6cd630e1) SHA1(edbfb016ddc68a600b4e72c32e5f60c747227e02) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "sld700d.u1", 0x0000, 0x080000, CRC(652daf52) SHA1(98702e3a3cd6760dbd53b191e36137cf4999a896) )
ROM_END


ROM_START( as_slda )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "sld600.u1", 0x0000, 0x080000, CRC(563c3acf) SHA1(317523362206e367fc968562191d3394c026b012) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "sld600d.u1", 0x0000, 0x080000, CRC(5fc7a57c) SHA1(15428e56833f94f0c1f7d1160b1fbe92a74f440c) )
ROM_END

ROM_START( as_sldb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "sld500.u1", 0x0000, 0x080000, CRC(d0039416) SHA1(bd2bb31a6ad885ccbe8a66f90dfdd977229b1aa4) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "sld500d.u1", 0x0000, 0x080000, CRC(d9f80ba5) SHA1(0633555ae7f1641fd949e9e887f383b7ad6cb815) )
ROM_END

ROM_START( as_sldc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "sld400.u1", 0x0000, 0x080000, CRC(25b4ccfd) SHA1(c590df0f07b3e79a9409f57f8b0dc8f00b1f5119) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "sld400d.u1", 0x0000, 0x080000, CRC(2c4f534e) SHA1(694ed66bc8cb17fef994b4dbc7b1629b91023faf) )
ROM_END

ROM_START( as_sldd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "sld200.u1", 0x0000, 0x080000, CRC(d7cd2b78) SHA1(2de766995bcf25e82511c8fe9ccf45575bee7490) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "sld200d.u1", 0x0000, 0x080000, CRC(de36b4cb) SHA1(204e774161c66d387a18532a97015c389cfbcf8c) )
ROM_END

ROM_START( as_slde )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "sld101.bin", 0x0000, 0x080000, CRC(3350d4a6) SHA1(22f0bb3bbc00c198af4f1546d2adfa8906de9cc1) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "sld101d.bin", 0x0000, 0x080000, CRC(3aab4b15) SHA1(93d25d02821477bbbda9c8381002b47747aa8884) )
ROM_END

ROM_START( as_tem )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "temv1-01.u1", 0x00000, 0x080000, CRC(78cfa14d) SHA1(d234329ba9557c8fdcc61f05af96cc0b3d89c049) )
	ROM_LOAD16_BYTE( "temv1-01.u2", 0x00001, 0x080000, CRC(c432b954) SHA1(1e0a9dc1c9cfcddece5f3efd8e4c00e2b55881fc) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "tem1-01d.u1", 0x00000, 0x080000, CRC(71343efe) SHA1(383c91cbe45ca5173552ead63b86767d85760c0e) )
ROM_END

ROM_START( as_tema )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "temv0-06.u1", 0x00000, 0x080000, CRC(173def07) SHA1(46e3337bfb3a159d8806611f16d023893795e122) )
	ROM_LOAD16_BYTE( "temv0-06.u2", 0x00001, 0x080000, CRC(9c28438b) SHA1(a098f036cc1c5261e720dbe5ae110e6c7a073fae) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "tem0-06d.u1", 0x00000, 0x080000, CRC(1ec670b4) SHA1(1ab0fba00d0612f36e726ff7d856c0a96b610489) )
ROM_END



ROM_START( as_tbl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tb1-04.bin", 0x0000, 0x080000, CRC(726f2643) SHA1(d938b3b7d342bbb04b6400886d93bdae2ed87873) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tb1-04g.bin", 0x0000, 0x080000, CRC(edd6dd73) SHA1(0841b66bcf0d6301719f622d52152028c4e7dc90) )
	ROM_LOAD( "tb1-04p.bin", 0x0000, 0x080000, CRC(7b94b9f0) SHA1(d6b9604564630c40dfe036bdf81aa24ff15bacd0) )
ROM_END

ROM_START( as_tbla )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tb105.u1", 0x0000, 0x080000, CRC(b8479f27) SHA1(1b9f048fbaf0e3fcd4ebff140f14f3e55ea9a127) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tb105d.u1", 0x0000, 0x080000, CRC(b1bc0094) SHA1(4ba18dd5f840524e4fbf8219bbe2d0f93ca89b52) )
ROM_END

ROM_START( as_tblb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tb106.u1", 0x0000, 0x080000, CRC(c286fd39) SHA1(e13c67872b976cb026c77ad245867a3f12906fe6) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tb106d.u1", 0x0000, 0x080000, CRC(cb7d628a) SHA1(4d8afb5b23c96ccc867054aa508384ef7842a931) )
ROM_END

ROM_START( as_tblc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tb1_03.bin", 0x0000, 0x080000, CRC(95cf57c1) SHA1(1292c330d030fa2eb41aba57ec16092a296e7df2) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tb1_03g.bin", 0x0000, 0x080000, CRC(0a76acf1) SHA1(8317a8a45f3ea67edf05c3cd2442bb2e59dbc7a1) )
	ROM_LOAD( "tb1_03p.bin", 0x0000, 0x080000, CRC(9c34c872) SHA1(cb45592bba9cdcaddf0bf9cb5fe5db7bd8ded2eb) )
ROM_END

ROM_START( as_tbld )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tb304.bin", 0x0000, 0x080000, CRC(f10796ae) SHA1(e6698931b28a813e466d64364e652e99706d2090) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tb304d.bin", 0x0000, 0x080000, CRC(f8fc091d) SHA1(0b9530dad46fa04da808c599013e62a8199a7945) )
ROM_END

ROM_START( as_tble )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tb303.u1", 0x0000, 0x080000, CRC(381c1817) SHA1(c88a10a4b68caf55edae7bc8b743e368d1c9bdeb) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tb303d.u1", 0x0000, 0x080000, CRC(31e787a4) SHA1(3c0bf46dd796a4d044d65e9b9a3a5dc7abaeb7d0) )
ROM_END


ROM_START( as_tblf )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "tb3_01.bin", 0x0000, 0x080000, CRC(bd3d6ad3) SHA1(1625bc608f64fcc0f8767375ccbbfd6a31e5e55c) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "tb3_01g.bin", 0x0000, 0x080000, CRC(228491e3) SHA1(f840e460883f3242601e4b73699f62e957f8a429) )
	ROM_LOAD( "tb3_01p.bin", 0x0000, 0x080000, CRC(b4c6f560) SHA1(5b6f45c614198a73aeab20630cb211bd587f1c66) )
ROM_END



ROM_START( as_td )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1td1_03.bin", 0x00000, 0x040000, CRC(c5d7e5eb) SHA1(1c68e05bd20d703d34f3c7d9dfca3e7c7eac08e5) )
	ROM_LOAD16_BYTE( "2td1_03.bin", 0x00001, 0x040000, CRC(49eb2350) SHA1(3c867c2e25eb1c4860f07cbe7c0b10176d75dcc9) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "1td1_03g.bin", 0x0000, 0x040000, CRC(ee684241) SHA1(722ee2d4972cf677d2897ddf73460b25edd91e3c) )
	ROM_LOAD16_BYTE( "1td1_03p.bin", 0x0000, 0x040000, CRC(11babc79) SHA1(646fa69840585e4ab06e4fe62fa8407f5c537c8f) )
ROM_END




ROM_START( as_twp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "twp1_06.bin", 0x0000, 0x080000, CRC(c90090d1) SHA1(1abd0ed0182171b05d3d679e788d7849075783ee) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "twp1_06g.bin", 0x0000, 0x080000, CRC(56b96be1) SHA1(f4640c2f2d2335c07eda546b03c7f5ed5ecf3143) )
	ROM_LOAD( "twp1_06p.bin", 0x0000, 0x080000, CRC(c0fb0f62) SHA1(2e0ef0e5df3e261c3c42609fa5272e8257904ca8) )
ROM_END

ROM_START( as_twpa )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "twp104.u1", 0x0000, 0x080000, CRC(c56576c1) SHA1(f72758a1a16fdc2083df3477cec93dacf3b24ba3) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "twp104d.u1", 0x0000, 0x080000, CRC(cc9ee972) SHA1(1ca54e5528f882d089f9267f2a199c28915f5e73) )
ROM_END



ROM_START( as_vn )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vnm205.u1", 0x00000, 0x080000, CRC(e083b08d) SHA1(3107619046a6dd593d84d4fa16f3fe573fcc6ff5) )
	ROM_LOAD16_BYTE( "vnm205.u2", 0x00001, 0x080000, CRC(531d4eda) SHA1(1293076cef8837577f07566af1d96516c7391b46) )

	ROM_REGION( 0x200000, "slavecpu", 0 )
	ROM_LOAD16_BYTE( "vns205.u1", 0x00000, 0x080000, CRC(3d26aaa0) SHA1(7e2138d53d6c40b0713fa84a93f6a2e5aa647f59) )
	ROM_LOAD16_BYTE( "vns205.u2", 0x00001, 0x080000, CRC(66a7e22b) SHA1(8fc6d88d5e44357c880525b47f3537ed1a5a45af) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "vns205d.u1", 0x00000, 0x080000, CRC(34dd3513) SHA1(b77d24442feeaafe7e1cef84654fd1f05c7c6a3c) )
ROM_END


ROM_START( as_vcv )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vp005.u1", 0x0000, 0x100000, CRC(0777ad2d) SHA1(25eb83ebe54fe68a21f8e78435d171ff0141c718) )
	ROM_LOAD16_BYTE( "vp005.u2", 0x0000, 0x100000, CRC(2d030eff) SHA1(501fea814b38701b4465fa9cfe2d6d1ba4f0cfde) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "vp005d.u1", 0x0000, 0x100000, CRC(2b9a2141) SHA1(c36069305880d5fff8223c2c8b61aaf3bff88eee) )
ROM_END

ROM_START( as_vcva )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vpush107.u1", 0x00000, 0x100000, CRC(79f5268c) SHA1(fd52074e4f050d9ca93f21007a8a18d56dc43d90) )
	ROM_LOAD16_BYTE( "vpush107.u2", 0x00001, 0x100000, CRC(490442bf) SHA1(e01cdd81688c8ec0d9cadd53869e7c53a1c20f61) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "vpus107d.u1", 0x0000, 0x100000, CRC(5518aae0) SHA1(c78b0007d2fb674daa15c625dfe2493769bad885) )
ROM_END

ROM_START( as_vcvb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vpush106.u1", 0x00000, 0x100000, CRC(efa20dd9) SHA1(1fcac063e132ff7287d835577a5614dd9dc9448d) )
	ROM_LOAD16_BYTE( "vpush106.u2", 0x00001, 0x100000, CRC(dfe7c909) SHA1(3e84c24b32ed4f40046ce5334d8cf67f65da8beb) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "vpus106d.u1", 0x0000, 0x100000, CRC(c34f81b5) SHA1(067c513270f14a7eb2ceac3517512fb205b40f17) )
ROM_END

ROM_START( as_vcvc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vpush104.u1", 0x00000, 0x100000, CRC(218e3749) SHA1(524c8e9eeb80a7d07afb89f6b88fdc90388b2444) )
	ROM_LOAD16_BYTE( "vpush104.u2", 0x00001, 0x100000, CRC(684cd8ae) SHA1(c80e29de19a010158d141549d38268adda4f3eb8) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "vpsh104d.u1", 0x0000, 0x100000, CRC(0d63bb25) SHA1(7f8c04303d320c82817b0557cda2f4d04f791ae6) )
ROM_END

ROM_START( as_vcvd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vpush102.u1", 0x00000, 0x100000, CRC(8965317d) SHA1(cf97c3fa5c5b7c93dab781cac9945340e20e8a6a) )
	ROM_LOAD16_BYTE( "vpush102.u2", 0x00001, 0x100000, CRC(f4f724eb) SHA1(cdebb1f889c16890a3f143a361d623fd4faa409f) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "vpus102d.u1", 0x0000, 0x100000, CRC(a588bd11) SHA1(12f15c845c80653ac35148f96af805c66d5323e3) )
ROM_END

ROM_START( as_vcve )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vpush101.u1", 0x00000, 0x100000, CRC(13b2da9e) SHA1(f0e1997b6c684cdf071ebd3c37924f726e4fa514) )
	ROM_LOAD16_BYTE( "vpush101.u2", 0x00001, 0x100000, CRC(047c45d9) SHA1(6a7924b4eba4a3adf90ab5495400aa25ebc03380) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "vpsh101d.u1", 0x0000, 0x100000, CRC(3f5f56f2) SHA1(4a3aaedea3b4b7f24e530ffe028020ca0e0a41b5) )
ROM_END





ROM_START( as_ww )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ww2_03.bin", 0x0000, 0x080000, CRC(8bb26a49) SHA1(9de143cde5578cb4fe0c9295e279d6f155b1337b) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ww2_03g.bin", 0x0000, 0x080000, CRC(140b9179) SHA1(6c01a42a76ab9733fd6775ae3b6fd795f408e2d0) )
	ROM_LOAD( "wickedwilly.bin", 0x0000, 0x080000, CRC(8249f5fa) SHA1(d7739f7aeb529a82390c3104d032024fbe917213) )
ROM_END

ROM_START( as_wwa )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ww204.u1", 0x0000, 0x080000, CRC(0d161888) SHA1(55f1666b8c34da237280cbd266dd3839b428a5e9) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ww204d.u1", 0x0000, 0x080000, CRC(04ed873b) SHA1(fa00273952727cb78bf868a44f02888053c88515) )
ROM_END

ROM_START( as_wwb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ww205.u1", 0x0000, 0x080000, CRC(b0b34fe9) SHA1(f1fa8885768145191c72351b49b5da0d2091d502) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ww205d.u1", 0x0000, 0x080000, CRC(b948d05a) SHA1(a4d969683d77b04a762206c11dcebad74d402604) )
ROM_END

ROM_START( as_wwc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ww104.bin", 0x0000, 0x080000, CRC(791d09b4) SHA1(6852a6da56ed877add503762fba38af7fbe0dc06) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ww104d.bin", 0x0000, 0x080000, CRC(70e69607) SHA1(b3f223f82db44c50325a330f7654d3b621c470ec) )
ROM_END

ROM_START( as_wwd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ww103.bin", 0x0000, 0x080000, CRC(bc23b12e) SHA1(2ff1348d6b92c58d2c5475c16f52d41d8b14e62a) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ww103d.bin", 0x0000, 0x080000, CRC(b5d82e9d) SHA1(4b6a90480f9429f04cd349c1ea83da9f8cfa3f68) )
ROM_END

ROM_START( as_wwe )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "ww1_02.bin", 0x0000, 0x080000, CRC(ff3a167c) SHA1(fce5bd1cb593e71776d2724eda00f6f31843f495) )

	ROM_REGION( 0x200000, "altrevs", 0 )
	ROM_LOAD( "ww1_02g.bin", 0x0000, 0x080000, CRC(6083ed4c) SHA1(33b7b8752c10ebac239e153b317f8cae72a0b8ea) )
	ROM_LOAD( "ww1_02p.bin", 0x0000, 0x080000, CRC(f6c189cf) SHA1(59c61a635b5021fad6afca67be1db0275f49f290) )
ROM_END




ROM_START( as_ws )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ws100.u1", 0x00000, 0x040000, CRC(a8e5a89f) SHA1(3a3182aee75d92934da04cff523cb91dd0ed3d09) )
	ROM_LOAD16_BYTE( "ws100.u2", 0x00001, 0x100000, NO_DUMP )
ROM_END




void astra_addresslines( UINT16* src, size_t srcsize, int small )
{
	std::vector<UINT16> dst(srcsize/2);

	int blocksize;

	if (small) blocksize= 0x100000/2;
	else blocksize= 0x100000;

	for (int block = 0; block < srcsize; block += blocksize)
	{
		for (int x = 0; x<blocksize/2;x+=2)
		{
			dst[((block/2)+(x/2))^1] = src[(block/2)+x+1];
			dst[((block/2)+(x/2+blocksize/4))^1] = src[(block/2)+x];
		}
	}

	memcpy(src,&dst[0], srcsize);
}


DRIVER_INIT_MEMBER(astrafr_state,astradec)
{
	astra_addresslines( (UINT16*)memregion( "maincpu" )->base(), memregion( "maincpu" )->bytes(), 0 );
}



DRIVER_INIT_MEMBER(astrafr_state,astradec_dual)
{
	astra_addresslines( (UINT16*)memregion( "maincpu" )->base(), memregion( "maincpu" )->bytes(), 0 );
	astra_addresslines( (UINT16*)memregion( "slavecpu" )->base(), memregion( "slavecpu" )->bytes(), 0 );
}

DRIVER_INIT_MEMBER(astrafr_state,astradec_sml)
{
	astra_addresslines( (UINT16*)memregion( "maincpu" )->base(), memregion( "maincpu" )->bytes(), 1 );
}

DRIVER_INIT_MEMBER(astrafr_state,astradec_sml_dual)
{
	astra_addresslines( (UINT16*)memregion( "maincpu" )->base(), memregion( "maincpu" )->bytes(), 1 );
	astra_addresslines( (UINT16*)memregion( "slavecpu" )->base(), memregion( "slavecpu" )->bytes(), 1 );
}

// Single games?
GAME( 200?, as_srb,    0            , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V004)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_srba,   as_srb       , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V100)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_srbb,   as_srb       , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V101)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_srbc,   as_srb       , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V201)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_srbd,   as_srb       , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V202)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_srbe,   as_srb       , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Super Ring a Bell (Astra, V203)", MACHINE_IS_SKELETON_MECHANICAL)


// Linked games (single rom per CPU with master/slave?)
GAME( 200?, as_party,    0          , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Party Time (Astra, V105)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_partya,   as_party   , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Party Time (Astra, V110)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_partyb,   as_party   , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Party Time (Astra, V112)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_partyc,   as_party   , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Party Time (Astra, V206)" ,MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_partyd,   as_party   , astrafr_dual_37,    astrafr, driver_device,    0, ROT0,  "Astra", "Party Time (Astra, V401)", MACHINE_IS_SKELETON_MECHANICAL) // significantly different set
GAME( 200?, as_partye,   as_party   , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Party Time (Astra, V907)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_partyf,   as_party   , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Party Time (Astra, V906)", MACHINE_IS_SKELETON_MECHANICAL)

GAME( 200?, as_letsp,    0          , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Let's Party (Astra, V904)", MACHINE_IS_SKELETON_MECHANICAL)

GAME( 200?, as_topsl,   0           , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Top Slot (Astra, V103)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_topsla,  as_topsl    , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Top Slot (Astra, V104)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_topslb,  as_topsl    , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Top Slot (Astra, V201)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_topslc,  as_topsl    , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Top Slot (Astra, V203)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_topsld,  as_topsl    , astrafr_dual_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Top Slot (Astra, V205)", MACHINE_IS_SKELETON_MECHANICAL)


// Other HW? (has u1/u2 pairing)
GAME( 200?, as_bigtm,   0           , astra_single ,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Big Time (Astra, V003)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_acp,     0           , astra_single ,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "unknown Astra 'ACP' (Astra, V403)", MACHINE_IS_SKELETON_MECHANICAL) // no sound data in here?
GAME( 200?, as_celeb,   0           , astra_single ,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Celebration (Astra, V100)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_celeba,  as_celeb    , astra_single ,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Celebration (Astra, V101)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_celebb,  as_celeb    , astra_single ,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Celebration (Astra, V201)", MACHINE_IS_SKELETON_MECHANICAL)


// u1/u2 pairing and Linked?

GAME( 200?, as_hc,     0            , astrafr_dual_37 ,    astrafr, astrafr_state,    astradec_dual, ROT0,  "Astra", "Hokey Cokey (Astra, V107)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_hca,    as_hc        , astrafr_dual_37 ,    astrafr, astrafr_state,    astradec_dual, ROT0,  "Astra", "Hokey Cokey (Astra, V109)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_hcb,    as_hc        , astrafr_dual_37 ,    astrafr, astrafr_state,    astradec_dual, ROT0,  "Astra", "Hokey Cokey (Astra, V110)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_hcc,    as_hc        , astrafr_dual_37 ,    astrafr, astrafr_state,    astradec_dual, ROT0,  "Astra", "Hokey Cokey (Astra, V111)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_hcd,    as_hc        , astrafr_dual_37 ,    astrafr, astrafr_state,    astradec_dual, ROT0,  "Astra", "Hokey Cokey (Astra, V909)", MACHINE_IS_SKELETON_MECHANICAL)

// Non-Astra, same hw?

GAME( 200?, as_cshah,  0            , astra_single_alt_57 ,    astrafr, astrafr_state,    astradec, ROT0,  "Lowen", "Cash Ahoi (Lowen, V105)", MACHINE_IS_SKELETON_MECHANICAL)


GAME( 200?, as_big10,    0          , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Big 10 (Astra, V500)", MACHINE_IS_SKELETON_MECHANICAL) // BB96
GAME( 200?, as_big10a,   as_big10   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Big 10 (Astra, V503)", MACHINE_IS_SKELETON_MECHANICAL) // BB96
GAME( 200?, as_big10b,   as_big10   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Big 10 (Astra, V507)", MACHINE_IS_SKELETON_MECHANICAL) // BB96
GAME( 200?, as_big10c,   as_big10   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Big 10 (Astra, V601)", MACHINE_IS_SKELETON_MECHANICAL) // BB96
GAME( 200?, as_big10d,   as_big10   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Big 10 (Astra, V605)", MACHINE_IS_SKELETON_MECHANICAL) // BB96
GAME( 200?, as_big15,    0          , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Big 15 (Astra, V101)", MACHINE_IS_SKELETON_MECHANICAL) // DL98
GAME( 200?, as_bigcs,    0          , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Big Cash (Astra, V101)", MACHINE_IS_SKELETON_MECHANICAL) // CF97/CF98
GAME( 200?, as_bigcsa,   as_bigcs   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Big Cash (Astra, V103)", MACHINE_IS_SKELETON_MECHANICAL) // CF97/CF98

GAME( 200?, as_bbr,      0          , astra_single_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Bullion Bars (Astra, V101)"        , MACHINE_IS_SKELETON_MECHANICAL) // FA00/CU98
GAME( 200?, as_bbra,     as_bbr     , astra_single_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Bullion Bars (Astra, V102,alt)"    , MACHINE_IS_SKELETON_MECHANICAL) // FA00/CU98
GAME( 200?, as_bbrb,     as_bbr     , astra_single_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Bullion Bars (Astra, V201)"        , MACHINE_IS_SKELETON_MECHANICAL) // FA00/CU98
GAME( 200?, as_bbrc,     as_bbr     , astra_single_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Bullion Bars (Astra, V003)"        , MACHINE_IS_SKELETON_MECHANICAL) // CU98
GAME( 200?, as_bbrd,     as_bbr     , astra_single_2e,    astrafr, driver_device,    0, ROT0,  "Astra", "Bullion Bars (Astra, V102)"        , MACHINE_IS_SKELETON_MECHANICAL) // CU98
GAME( 200?, as_bbre,     as_bbr     , astra_single_37,    astrafr, driver_device,    0, ROT0,  "Astra", "Bullion Bars (Astra, V105)"        , MACHINE_IS_SKELETON_MECHANICAL) // FG01
GAME( 200?, as_bbrf,     as_bbr     , astra_single_37,    astrafr, driver_device,    0, ROT0,  "Astra", "Bullion Bars (Astra, V004)"        , MACHINE_IS_SKELETON_MECHANICAL) // FN01

GAME( 200?, as_dblcs,    0          , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Double Cash (Astra, V110)"        , MACHINE_IS_SKELETON_MECHANICAL) // BR97
GAME( 200?, as_dblcsa,   as_dblcs   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Double Cash (Astra, V112)"        , MACHINE_IS_SKELETON_MECHANICAL) // BR97
GAME( 200?, as_dblcsb,   as_dblcs   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Double Cash (Astra, V108)"        , MACHINE_IS_SKELETON_MECHANICAL) // BR97
GAME( 200?, as_dblcsc,   as_dblcs   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Double Cash (Astra, V109)"        , MACHINE_IS_SKELETON_MECHANICAL) // CN97
GAME( 200?, as_dblcsd,   as_dblcs   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Double Cash (Astra, V106)"        , MACHINE_IS_SKELETON_MECHANICAL) // CN97
GAME( 200?, as_dblcse,   as_dblcs   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Double Cash (Astra, V102)"        , MACHINE_IS_SKELETON_MECHANICAL) // FB00
GAME( 200?, as_dblcsf,   as_dblcs   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Double Cash (Astra, V100)"        , MACHINE_IS_SKELETON_MECHANICAL) // FB00
GAME( 200?, as_dblcsg,   as_dblcs   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Double Cash (Astra, V100, alt)"   , MACHINE_IS_SKELETON_MECHANICAL) // DN98
GAME( 200?, as_dblcsh,   as_dblcs   , astra_single,    astrafr, driver_device,    0, ROT0,  "Astra", "Double Cash (Astra, V004)"        , MACHINE_IS_SKELETON_MECHANICAL) // FO01

GAME( 200?, as_fortn,    0          , astra_single,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Fortune Teller (Astra, V009)"      , MACHINE_IS_SKELETON_MECHANICAL)

GAME( 200?, as_gof,      0          , astra_single,    astrafr, astrafr_state,    astradec, ROT0,  "Lowen", "Game Of Fortune (Lowen, V208)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_hog,      0          , astra_single,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Hearts Of Gold (Astra, V002)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_hxr,      0          , astra_single,    astrafr, astrafr_state,    astradec_sml, ROT0,  "Astra", "Hot Cross Run (Astra, V108)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_hr,       0          , astra_single,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Hot Reel (Astra, V004)"        , MACHINE_IS_SKELETON_MECHANICAL)

GAME( 200?, as_djp,       0         , astra_single,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Double Jackpot (Astra, V107)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_djpa,      0         , astra_single,    astrafr, astrafr_state,    astradec_sml, ROT0,  "Astra", "Double Jackpot (Astra, V004)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_djpb,      0         , astra_single,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Double Jackpot (Astra, V109)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_cshcs,     0         , astra_single,    astrafr, driver_device,    0, ROT0,  "Lowen", "Cash Castle (Lowen, V006)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_colmn,     0         , astra_single,    astrafr, astrafr_state,    astradec_sml, ROT0,  "Astra", "Colour Of Money (Astra, V200)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_colmna,    as_colmn  , astra_single,    astrafr, driver_device,    0,                ROT0,  "Astra", "Colour Of Money (Astra, V107)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_colmnb,    as_colmn  , astra_single,    astrafr, driver_device,    0,                ROT0,  "Astra", "Colour Of Money (Astra, V108)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_colmnc,    as_colmn  , astra_single,    astrafr, driver_device,    0,                ROT0,  "Astra", "Colour Of Money (Astra, V109)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_colmnd,    as_colmn  , astra_single,    astrafr, driver_device,    0,                ROT0,  "Astra", "Colour Of Money (Astra, V908)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jackb,     0         , astra_single,    astrafr, driver_device,    0,    ROT0,  "Astra", "Jackpot Bell (Astra, V104)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jpx,     0           , astra_single,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Jackpot X (Astra, V100)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jpxa,    as_jpx      , astra_single,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Jackpot X (Astra, V101)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jpxb,    as_jpx      , astra_single,    astrafr, astrafr_state,    astradec, ROT0,  "Astra", "Jackpot X (Astra, V002)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jolly,    0      , astra_single,    astrafr, driver_device,    0,    ROT0,  "Astra", "Jolly Roger (Astra, V103)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jmpj,    0       , astra_single,    astrafr, driver_device,    0,    ROT0,  "Astra", "Jumping Jackpots (Astra, V100)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jmpja,   as_jmpj , astra_single,    astrafr, driver_device,    0,    ROT0,  "Astra", "Jumping Jackpots (Astra, V102)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jjive,    0        , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Jungle Jive (Astra, V107)"         , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jjivea,   as_jjive , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Jungle Jive (Astra, V106)"         , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jjiveb,   as_jjive , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Jungle Jive (Astra, V104)"         , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jjivec,   as_jjive , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Jungle Jive (Astra, V102)"         , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jjived,   as_jjive , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Jungle Jive (Astra, V101)"         , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jjivee,   as_jjive , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Jungle Jive (Astra, V101, alt)"    , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jjivef,   as_jjive , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Jungle Jive (Astra, V004)"         , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_jjiveg,   as_jjive , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Jungle Jive (Astra, V005)"         , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_kingc,   0        , astra_single,    astrafr, driver_device,    0,   ROT0,  "Astra", "King Cash (Astra, V106)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_kingca,  as_kingc , astra_single,    astrafr, driver_device,    0,   ROT0,  "Astra", "King Cash (Astra, V103)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_koc,   0      , astra_single,    astrafr, driver_device,    0,   ROT0,  "Astra", "King Of Clubs (Astra, V200)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_koca,  as_koc , astra_single,    astrafr, driver_device,    0,   ROT0,  "Astra", "King Of Clubs (Astra, V101)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_lbt,   0      , astra_single,    astrafr, driver_device,    0,   ROT0,  "Astra", "Little Big 10 (Astra, V103)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_lbta,  as_lbt , astra_single,    astrafr, driver_device,    0,   ROT0,  "Astra", "Little Big 10 (Astra, V102)"       , MACHINE_IS_SKELETON_MECHANICAL)

GAME( 200?, as_ldl,     0           , astra_single_2e,    astrafr, astrafr_state,    astradec_sml,  ROT0,  "Astra", "Little Devils (Astra, V700)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_ldla,    as_ldl      , astra_single,    astrafr, astrafr_state,    astradec_sml, ROT0,  "Astra", "Little Devils (Astra, V600)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_ldlb,    as_ldl      , astra_single,    astrafr, astrafr_state,    astradec_sml, ROT0,  "Astra", "Little Devils (Astra, V312)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_ldlc,    as_ldl      , astra_single,    astrafr, astrafr_state,    astradec_sml, ROT0,  "Astra", "Little Devils (Astra, V003)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_ldld,    as_ldl      , astra_single,    astrafr, driver_device,    0,                ROT0,  "Astra", "Little Devils (Astra, V102)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_ldle,    as_ldl      , astra_single,    astrafr, driver_device,    0,                ROT0,  "Astra", "Little Devils (Astra, V103)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_mp,     0            , astra_single_2e,    astrafr, astrafr_state,    astradec,  ROT0,  "Lowen", "Mission Possible (Lowen, V118)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_mp2,     0           , astra_single,    astrafr, driver_device,    0,    ROT0,  "Lowen", "Mission Possible 2 (Lowen, V114)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_otr,     0           , astra_single_2e,    astrafr, astrafr_state,    astradec_sml,  ROT0,  "Astra", "Over The Rainbow (Astra, V104)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_otra,    as_otr      , astra_single,    astrafr, astrafr_state,    astradec_sml, ROT0,  "Astra", "Over The Rainbow (Astra, V102)"        , MACHINE_IS_SKELETON_MECHANICAL) // incomplete set
GAME( 200?, as_ptf,    0        , astrafr_dual_37,    astrafr, astrafr_state,    astradec_sml_dual, ROT0,  "Astra", "Party Fruits (Astra, V102)"        , MACHINE_IS_SKELETON_MECHANICAL) // strange extra sound(?) roms in here
GAME( 200?, as_ptfa,   as_ptf   , astrafr_dual_37,    astrafr, astrafr_state,    astradec_sml_dual, ROT0,  "Astra", "Party Fruits (Astra, V803)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_ptfb,   as_ptf   , astrafr_dual_37,    astrafr, astrafr_state,    astradec_sml_dual, ROT0,  "Astra", "Party Fruits (Astra, V905)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_ptfc,   as_ptf   , astrafr_dual_37,    astrafr, astrafr_state,    astradec_sml_dual, ROT0,  "Astra", "Party Fruits (Astra)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_pia,     0           , astra_single,    astrafr, driver_device,    0,    ROT0,  "Astra", "Pay It Again (Astra, V202)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_piaa,    as_pia      , astra_single,    astrafr, driver_device,    0,    ROT0,  "Astra", "Pay It Again (Astra, V206)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_piab,    as_pia      , astra_single,    astrafr, driver_device,    0,    ROT0,  "Astra", "Pay It Again (Astra, V904)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_pharg,    0        , astra_single_37,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Pharaoh's Gold (Astra, V005)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_pharga,   as_pharg , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Pharaoh's Gold (Astra, V101)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_phargb,   as_pharg , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Pharaoh's Gold (Astra, V102)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_phargc,   as_pharg , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Pharaoh's Gold (Astra, V104)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_phargd,   as_pharg , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Pharaoh's Gold (Astra, V106)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_pharge,   as_pharg , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Pharaoh's Gold (Astra, V107)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_pb,   0 , astra_single_2e,    astrafr, driver_device,    0,  ROT0,  "Astra", "Piggy Banking (Astra, V105)"       , MACHINE_IS_SKELETON_MECHANICAL)
// this might be another mix of master / slave sets, and multiple games....
GAME( 200?, as_rtr,    0          , astra_single_2e,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Ready To Roll (Astra, V101)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rtra,   as_rtr     , astra_single_2e,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Ready To Roll (Astra, V101, alt 1)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rtrb,   as_rtr     , astra_single_2e,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Ready To Roll (Astra, V101, alt 2)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rtrc,   as_rtr     , astra_single_2e,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Ready To Roll (Astra, V101, alt 3)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rtrd,   as_rtr     , astra_single_2e,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Ready To Roll (Astra, V100, )"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rtre,   as_rtr     , astra_single_2e,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Ready To Roll (Astra, V100, alt)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rtrf,   as_rtr     , astra_single_2e,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Ready To Roll (Astra, V200)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rtrg,   as_rtr     , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Ready To Roll (Astra, V200, alt)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rtrh,   as_rtr     , astra_single_2e,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Ready To Roll (Astra, V202)"       , MACHINE_IS_SKELETON_MECHANICAL)

GAME( 200?, as_rab,    0          , astra_single_2e,    astrafr, driver_device,    0,   ROT0,  "Astra", "Ring A Bell (Astra, V105)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_raba,   as_rab     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Ring A Bell (Astra, V106)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rabb,   as_rab     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Ring A Bell (Astra, V107)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rabc,   as_rab     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Ring A Bell (Astra, V104)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rbg,    0          , astra_single_2e,    astrafr, driver_device,    0,   ROT0,  "Astra", "River Boat Gambler (Astra, V304)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rbga,   as_rab     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "River Boat Gambler (Astra, V303)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rbgb,   as_rab     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "River Boat Gambler (Astra, V104)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rbgc,   as_rab     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "River Boat Gambler (Astra, V102)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rbgd,   as_rab     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "River Boat Gambler (Astra, V101)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rbge,   as_rab     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "River Boat Gambler (Astra, V008)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rbglo,    0        , astrafr_dual_37,    astrafr, astrafr_state,    astradec_sml_dual,   ROT0,  "Lowen", "River Boat Gambler (Lowen, V106)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_rox,    0          , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Roll X (Astra, V006)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_csv,    0          , astra_single_37,    astrafr, driver_device,    0,   ROT0,  "Astra", "Slot Slant (?) (Astra, V202)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_sltcl,    0        , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Slots Classic (?) (Astra)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_stp,    0          , astra_single_2e,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Stampede (Astra, V103)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_stpa,   as_stp     , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Stampede (Astra, V102)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_stpb,   as_stp     , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Stampede (Astra, V105)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_siu,    0          , astra_single_37,    astrafr, astrafr_state,    astradec_sml,    ROT0,  "Astra", "Step It Up (Astra, V202)"      , MACHINE_IS_SKELETON_MECHANICAL) // extra sound roms(?) in this set
GAME( 200?, as_siua,   as_siu     , astra_single,    astrafr, astrafr_state,    astradec_sml,   ROT0,  "Astra", "Step It Up (Astra, V203)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_sld,    0          , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Super Little Devil (Astra, V700)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_slda,   as_sld     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Super Little Devil (Astra, V600)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_sldb,   as_sld     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Super Little Devil (Astra, V500)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_sldc,   as_sld     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Super Little Devil (Astra, V400)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_sldd,   as_sld     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Super Little Devil (Astra, V200)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_slde,   as_sld     , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Super Little Devil (Astra, V101)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_tem,   0           , astra_single_alt_37,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Temptation (Astra, V101)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_tema,  as_tem      , astra_single_alt_37,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Temptation (Astra, V006)"      , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_tbl,   0               , astra_single_2e,    astrafr, driver_device,    0,   ROT0,  "Astra", "Triple Bells (Astra, V104)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_tbla,  as_tbl          , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Triple Bells (Astra, V105)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_tblb,  as_tbl          , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Triple Bells (Astra, V106)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_tblc,  as_tbl          , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Triple Bells (Astra, V103)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_tbld,  as_tbl          , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Triple Bells (Astra, V304)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_tble,  as_tbl          , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Triple Bells (Astra, V303)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_tblf,  as_tbl          , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Triple Bells (Astra, V301)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_td,   0        , astra_single_2e,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Twin Dragons (Astra, V103)"        , MACHINE_IS_SKELETON_MECHANICAL)

GAME( 200?, as_twp,   0           , astra_single_2e,    astrafr, driver_device,    0,   ROT0,  "Astra", "Twin Pots (Astra, V106)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_twpa,  as_twp      , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Twin Pots (Astra, V104)"       , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_vn,    0       , astrafr_dual_alt_37,    astrafr, astrafr_state,    astradec_dual,   ROT0,  "Astra", "Vegas Nights (Astra, V205)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_vcv,    0          , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Viva Cash Vegas (Astra, V005)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_vcva,   as_vcv     , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Viva Cash Vegas (Astra, V107)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_vcvb,   as_vcv     , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Viva Cash Vegas (Astra, V106)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_vcvc,   as_vcv     , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Viva Cash Vegas (Astra, V104)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_vcvd,   as_vcv     , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Viva Cash Vegas (Astra, V102)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_vcve,   as_vcv     , astra_single,    astrafr, astrafr_state,    astradec,   ROT0,  "Astra", "Viva Cash Vegas (Astra, V101)"     , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_ww,   0            , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Wicked Willy (Astra, V203)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_wwa,  as_ww        , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Wicked Willy (Astra, V204)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_wwb,  as_ww        , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Wicked Willy (Astra, V205)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_wwc,  as_ww        , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Wicked Willy (Astra, V104)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_wwd,  as_ww        , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Wicked Willy (Astra, V103)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_wwe,  as_ww        , astra_single,    astrafr, driver_device,    0,  ROT0,  "Astra", "Wicked Willy (Astra, V102)"        , MACHINE_IS_SKELETON_MECHANICAL)
GAME( 200?, as_ws,   0       , astra_single,    astrafr, astrafr_state,    astradec,    ROT0,  "Astra", "Win Streak (Astra, V100)"      , MACHINE_IS_SKELETON_MECHANICAL) // incomplete dump
