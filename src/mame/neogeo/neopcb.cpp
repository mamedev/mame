// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Ernesto Corvi,Andrew Prime,Zsolt Vasvari
// thanks-to:Fuzz


#include "emu.h"
#include "neogeo.h"

#include "bus/neogeo/prot_pcm2.h"
#include "bus/neogeo/prot_cmc.h"
#include "bus/neogeo/prot_pvc.h"


namespace {

class neopcb_state : public ngarcade_base_state
{
public:
	neopcb_state(const machine_config &mconfig, device_type type, const char *tag)
		: ngarcade_base_state(mconfig, type, tag)
		, m_cmc_prot(*this, "cmc50")
		, m_pcm2_prot(*this, "pcm2")
		, m_pvc_prot(*this, "pvc")
		, m_cpu_bank(*this, "cpu_bank")
		, m_bios_bank(*this, "bios_bank")
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(select_bios);

	void init_ms5pcb();
	void init_svcpcb();
	void init_kf2k3pcb();

	void neopcb(machine_config &config);

protected:
	// device overrides
	virtual void machine_start() override ATTR_COLD;

	virtual void device_post_load() override;

	void write_bankpvc(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void install_common();
	void install_banked_bios();

	// non-carts
	void svcpcb_gfx_decrypt();
	void svcpcb_s1data_decrypt();
	void kf2k3pcb_gfx_decrypt();
	void kf2k3pcb_decrypt_s1data();
	void kf2k3pcb_sp1_decrypt();

private:
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<pvc_prot_device> m_pvc_prot;
	memory_bank_creator m_cpu_bank;
	memory_bank_creator m_bios_bank;
};


void neopcb_state::machine_start()
{
	ngarcade_base_state::machine_start();

	m_sprgen->set_screen(m_screen);
}

void neopcb_state::device_post_load()
{
	ngarcade_base_state::device_post_load();

	m_cpu_bank->set_base(m_region_maincpu->base() + m_bank_base);
}

void neopcb_state::neopcb(machine_config &config)
{
	neogeo_arcade(config);
	neogeo_mono(config);

	NEOGEO_CTRL_EDGE_CONNECTOR(config, m_edge, neogeo_arc_edge, "joy", true);

	NG_CMC_PROT(config, "cmc50", 0);
	NG_PCM2_PROT(config, "pcm2", 0);
	NG_PVC_PROT(config, "pvc", 0);
}


// Game specific input definitions

INPUT_CHANGED_MEMBER(neopcb_state::select_bios)
{
	m_bios_bank->set_entry(newval ? 0 : 1);
}

static INPUT_PORTS_START( dualbios )
	PORT_INCLUDE( neogeo )

	/* the rom banking seems to be tied directly to the dipswitch */
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Region ) ) PORT_DIPLOCATION("SW:3") PORT_CHANGED_MEMBER(DEVICE_SELF, neopcb_state, select_bios, 0)
	PORT_DIPSETTING(    0x00, DEF_STR( Asia ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Japan ) )
INPUT_PORTS_END


/*************************************
 *
 *  Jamma PCB sets of NeoGeo games
 *
 *************************************/

#define ROM_Y_ZOOM \
	ROM_REGION( 0x20000, "spritegen:zoomy", 0 ) \
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )





/****************************************
 ID-2680
 . MV-0 ????
 NEO-MVH MVOBR 2003.8.4
****************************************/

ROM_START( ms5pcb ) /* Encrypted Set, JAMMA PCB */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "268-p1r.p1", 0x000000, 0x400000, CRC(d0466792) SHA1(880819933d997fab398f91061e9dbccb959ae8a1) )
	ROM_LOAD32_WORD_SWAP( "268-p2r.p2", 0x000002, 0x400000, CRC(fbf6b61e) SHA1(9ec743d5988b5e3183f37f8edf45c72a8c0c893e) )

	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0x000000 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	/* this contains both an ASIA and JAPAN bios, HARDDIP3 on the PCB selects which to use */
	ROM_LOAD16_WORD_SWAP( "sp-4x.sp1", 0x00000, 0x80000, CRC(b4590283) SHA1(47047ed5b6062babc0a0bebcc30e4b3f021e115a) )

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "268-m1.m1", 0x00000, 0x80000, CRC(4a5a6e0e) SHA1(df0f660f2465e1db7be5adfcaf5e88ad61a74a42) ) /* mask rom TC534000 */
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	/* Encrypted */

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd:adpcma", 0 )
	/* Encrypted */
	ROM_LOAD( "268-v1.v1", 0x000000, 0x1000000, CRC(8458afe5) SHA1(62b4c6e7db763e9ff2697bbcdb43dc5a56b48c68) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD32_WORD( "268-c1.c1", 0x0000000, 0x1000000, BAD_DUMP CRC(802042e8) SHA1(ff028b65f60f0b51b255a380cc47ec19fdc0c0cf) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "268-c2.c2", 0x0000002, 0x1000000, BAD_DUMP CRC(3b89fb9f) SHA1(cbc0729aae961f683b105ec3e1cda58b3f985abc) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "268-c3.c3", 0x2000000, 0x1000000, BAD_DUMP CRC(0f3f59e3) SHA1(8cc751dc7d4e94864a9ce3346f23b8f011082fcc) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "268-c4.c4", 0x2000002, 0x1000000, BAD_DUMP CRC(3ad8435a) SHA1(b333c8993c9b4c4ea59450ad0a3560e0b28056bc) ) /* Plane 2,3 */
ROM_END

/****************************************
 ID-2690
 . MV-0 ????
 NEO-MVH MVO 2003.6.5
****************************************/

ROM_START( svcpcb ) /* Encrypted Set, JAMMA PCB */
	ROM_REGION( 0x2000000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "269-p1.p1", 0x000000, 0x2000000, CRC(432cfdfc) SHA1(19b40d32188a8bace6d2d570c6cf3d2f1e31e379) )

	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x80000, 0x000000 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	/* this contains both an ASIA and JAPAN bios, HARDDIP3 on the PCB selects which to use */
	ROM_LOAD16_WORD_SWAP( "sp-4x.sp1", 0x00000, 0x80000, CRC(b4590283) SHA1(47047ed5b6062babc0a0bebcc30e4b3f021e115a) )

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "269-m1.m1", 0x00000, 0x80000, CRC(f6819d00) SHA1(d3bbe09df502464f104e53501708ac6e2c1832c6) ) /* mask rom TC534000 */
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd:adpcma", 0 )
	/* Encrypted */
	ROM_LOAD( "269-v1.v1", 0x000000, 0x800000, CRC(c659b34c) SHA1(1931e8111ef43946f68699f8707334c96f753a1e) )
	ROM_LOAD( "269-v2.v2", 0x800000, 0x800000, CRC(dd903835) SHA1(e58d38950a7a8697bb22a1cc7a371ae6664ae8f9) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD( "269-c1.c1", 0x0000000, 0x2000000, CRC(1b608f9c) SHA1(4e70ad182da2ca18815bd3936efb04a06ebce01e) ) /* Plane 0,1 */
	ROM_LOAD( "269-c2.c2", 0x2000000, 0x2000000, CRC(5a95f294) SHA1(6123cc7b20b494076185d27c2ffea910e124b195) ) /* Plane 0,1 */
ROM_END

/****************************************
 ID-2690
 . MV-0 ????
 NEO-MVH MVOB 2003.7.9
****************************************/

ROM_START( svcpcba ) /* Encrypted Set, JAMMA PCB */
	/* alt PCB version, this one has the same program roms as the MVS set, and different GFX / Sound rom arrangements */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "269-p1a.p1", 0x000000, 0x400000, CRC(38e2005e) SHA1(1b902905916a30969282f1399a756e32ff069097)  )
	ROM_LOAD32_WORD_SWAP( "269-p2a.p2", 0x000002, 0x400000, CRC(6d13797c) SHA1(3cb71a95cea6b006b44cac0f547df88aec0007b7)  )

	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x80000, 0x000000 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	/* this contains both an ASIA and JAPAN bios, HARDDIP3 on the PCB selects which to use */
	ROM_LOAD16_WORD_SWAP( "sp-4x.sp1", 0x00000, 0x80000, CRC(b4590283) SHA1(47047ed5b6062babc0a0bebcc30e4b3f021e115a) )

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "269-m1.m1", 0x00000, 0x80000, CRC(f6819d00) SHA1(d3bbe09df502464f104e53501708ac6e2c1832c6) )
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd:adpcma", 0 )
	/* Encrypted */
	ROM_LOAD( "269-v1a.v1", 0x000000, 0x1000000, CRC(a6af4753) SHA1(ec4f61a526b707a7faec4653b773beb3bf3a17ba) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD32_WORD( "269-c1a.c1", 0x0000000, 0x1000000, CRC(e64d2b0c) SHA1(0714198c400e5c273181e4c6f906b49e35fef75d) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "269-c2a.c2", 0x0000002, 0x1000000, CRC(249089c2) SHA1(1c0ca19e330efe1a74b2d35a1a9a8d61481e16a9) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "269-c3a.c3", 0x2000000, 0x1000000, CRC(d32f2fab) SHA1(273d58cb3c9075075b1ca39a3b247a2cd545fbe7) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "269-c4a.c4", 0x2000002, 0x1000000, CRC(bf77e878) SHA1(e6e76f8eed0d04ee9ad39bf38ce305930b10e2c1) ) /* Plane 2,3 */
ROM_END

/****************************************
 ID-2710
 . MV-0 ????
 NEO-MVH MVOC 2003.11.3
****************************************/

ROM_START( kf2k3pcb ) /* Encrypted Set, JAMMA PCB */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "271-p1.p1", 0x000000, 0x400000, CRC(b9da070c) SHA1(1a26325af142a4dd221c336061761468598c4634) )
	ROM_LOAD32_WORD_SWAP( "271-p2.p2", 0x000002, 0x400000, CRC(da3118c4) SHA1(582e4f44f03276adecb7b2848d3b96bf6da57f1e) )
	ROM_LOAD16_WORD_SWAP( "271-p3.p3", 0x800000, 0x100000, CRC(5cefd0d2) SHA1(cddc3164629fed4b6f715e12b109ad35d1009355) )

	ROM_REGION( 0x100000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x100000, 0x000000 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	ROM_LOAD16_WORD_SWAP( "spj.sp1", 0x00000, 0x080000, CRC(148dd727) SHA1(2cf592a16c7157de02a989675d47965f2b3a44dd) ) // encrypted

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "271-m1.m1", 0x00000, 0x80000, CRC(d6bcf2bc) SHA1(df78bc95990eb8e8f3638dde6e1876354df7fe84) )
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd:adpcma", 0 )
	/* Encrypted */
	ROM_LOAD( "271-v1.v1", 0x000000, 0x1000000, CRC(1d96154b) SHA1(1d4e262b0d30cee79a4edc83bb9706023c736668) )

	ROM_REGION( 0x6000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD32_WORD( "271-c1.c1", 0x0000000, 0x1000000, CRC(f5ebb327) SHA1(e4f799a54b09adcca13b1b0cf95971a1f4291b61) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "271-c2.c2", 0x0000002, 0x1000000, CRC(2be21620) SHA1(872c658f53bbc558e90f18d5db9cbaa82e748a6a) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "271-c3.c3", 0x2000000, 0x1000000, CRC(ddded4ff) SHA1(ff7b356125bc9e6637b164f5e81b13eabeb8d804) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "271-c4.c4", 0x2000002, 0x1000000, CRC(d85521e6) SHA1(62278fa8690972ed32aca07a4f7f97e7203d9f3a) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "271-c5.c5", 0x4000000, 0x1000000, CRC(18aa3540) SHA1(15e0a8c4e0927b1f7eb9bee8f532acea6818d5eb) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "271-c6.c6", 0x4000002, 0x1000000, CRC(1c40de87) SHA1(8d6425aed43ff6a96c88194e203df6a783286373) ) /* Plane 2,3 */
ROM_END




/*************************************
 *
 *  Additional encryption
 *
 *************************************/

/* ms5pcb and svcpcb have an additional scramble on top of the standard CMC scrambling */
void neopcb_state::svcpcb_gfx_decrypt()
{
	static const uint8_t xorval[4] = { 0x34, 0x21, 0xc4, 0xe9 };
	int rom_size = memregion("sprites")->bytes();
	uint8_t *rom = memregion("sprites")->base();
	std::vector<uint8_t> buf(rom_size);

	for (int i = 0; i < rom_size; i++)
		rom[i] ^= xorval[(i % 4)];

	for (int i = 0; i < rom_size; i += 4)
	{
		uint32_t rom32 = rom[i] | rom[i+1]<<8 | rom[i+2]<<16 | rom[i+3]<<24;
		rom32 = bitswap<32>(rom32, 0x09, 0x0d, 0x13, 0x00, 0x17, 0x0f, 0x03, 0x05, 0x04, 0x0c, 0x11, 0x1e, 0x12, 0x15, 0x0b, 0x06, 0x1b, 0x0a, 0x1a, 0x1c, 0x14, 0x02, 0x0e, 0x1d, 0x18, 0x08, 0x01, 0x10, 0x19, 0x1f, 0x07, 0x16);
		buf[i]   = rom32       & 0xff;
		buf[i+1] = (rom32>>8)  & 0xff;
		buf[i+2] = (rom32>>16) & 0xff;
		buf[i+3] = (rom32>>24) & 0xff;
	}

	for (int i = 0; i < rom_size / 4; i++)
	{
		int ofst =  bitswap<24>((i & 0x1fffff), 0x17, 0x16, 0x15, 0x04, 0x0b, 0x0e, 0x08, 0x0c, 0x10, 0x00, 0x0a, 0x13, 0x03, 0x06, 0x02, 0x07, 0x0d, 0x01, 0x11, 0x09, 0x14, 0x0f, 0x12, 0x05);
		ofst ^= 0x0c8923;
		ofst += (i & 0xffe00000);
		memcpy(&rom[i * 4], &buf[ofst * 4], 0x04);
	}
}


/* and a further swap on the s1 data */
void neopcb_state::svcpcb_s1data_decrypt()
{
	uint8_t *s1 = memregion("fixed")->base();
	size_t s1_size = memregion("fixed")->bytes();

	for (int i = 0; i < s1_size; i++) // Decrypt S
		s1[i] = bitswap<8>(s1[i] ^ 0xd2, 4, 0, 7, 2, 5, 1, 6, 3);

}


/* kf2k3pcb has an additional scramble on top of the standard CMC scrambling */
/* Thanks to Razoola & Halrin for the info */
void neopcb_state::kf2k3pcb_gfx_decrypt()
{
	static const uint8_t xorval[4] = { 0x34, 0x21, 0xc4, 0xe9 };
	int rom_size = memregion("sprites")->bytes();
	uint8_t *rom = memregion("sprites")->base();
	std::vector<uint8_t> buf(rom_size);

	for (int i = 0; i < rom_size; i++)
		rom[ i ] ^= xorval[ (i % 4) ];

	for (int i = 0; i < rom_size; i +=4)
	{
		uint32_t rom32 = rom[i] | rom[i+1]<<8 | rom[i+2]<<16 | rom[i+3]<<24;
		rom32 = bitswap<32>(rom32, 0x09, 0x0d, 0x13, 0x00, 0x17, 0x0f, 0x03, 0x05, 0x04, 0x0c, 0x11, 0x1e, 0x12, 0x15, 0x0b, 0x06, 0x1b, 0x0a, 0x1a, 0x1c, 0x14, 0x02, 0x0e, 0x1d, 0x18, 0x08, 0x01, 0x10, 0x19, 0x1f, 0x07, 0x16);
		buf[i]   =  rom32      & 0xff;
		buf[i+1] = (rom32>>8)  & 0xff;
		buf[i+2] = (rom32>>16) & 0xff;
		buf[i+3] = (rom32>>24) & 0xff;
	}

	for (int i = 0; i < rom_size; i+=4)
	{
		int ofst = bitswap<24>((i & 0x7fffff), 0x17, 0x15, 0x0a, 0x14, 0x13, 0x16, 0x12, 0x11, 0x10, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00);
		ofst ^= 0x000000;
		ofst += (i & 0xff800000);
		memcpy(&rom[ofst], &buf[i], 0x04);
	}
}


/* and a further swap on the s1 data */
void neopcb_state::kf2k3pcb_decrypt_s1data()
{
	uint8_t *src;
	uint8_t *dst;
	int tx_size = memregion("fixed")->bytes();
	int srom_size = memregion("sprites")->bytes();
	src = memregion("sprites")->base() + srom_size - 0x1000000 - 0x80000; // Decrypt S
	dst = memregion("fixed")->base();

	for (int i = 0; i < tx_size / 2; i++)
		dst[i] = src[(i & ~0x1f) + ((i & 7) << 2) + ((~i & 8) >> 2) + ((i & 0x10) >> 4)];

	src = memregion("sprites")->base() + srom_size - 0x80000;
	dst = memregion("fixed")->base() + 0x80000;

	for (int i = 0; i < tx_size / 2; i++)
		dst[i] = src[(i & ~0x1f) + ((i & 7) << 2) + ((~i & 8) >> 2) + ((i & 0x10) >> 4)];

	dst = memregion("fixed")->base();

	for (int i = 0; i < tx_size; i++)
		dst[i] = bitswap<8>(dst[i] ^ 0xd2, 4, 0, 7, 2, 5, 1, 6, 3);
}


/* only found on kf2k3pcb */
void neopcb_state::kf2k3pcb_sp1_decrypt()
{
	static const uint8_t address[0x40] = {
		0x04,0x0a,0x04,0x0a,0x04,0x0a,0x04,0x0a,
		0x0a,0x04,0x0a,0x04,0x0a,0x04,0x0a,0x04,
		0x09,0x07,0x09,0x07,0x09,0x07,0x09,0x07,
		0x09,0x09,0x04,0x04,0x09,0x09,0x04,0x04,
		0x0b,0x0d,0x0b,0x0d,0x03,0x05,0x03,0x05,
		0x0e,0x0e,0x03,0x03,0x0e,0x0e,0x03,0x03,
		0x03,0x05,0x0b,0x0d,0x03,0x05,0x0b,0x0d,
		0x04,0x00,0x04,0x00,0x0e,0x0a,0x0e,0x0a
	};

	uint16_t *rom = (uint16_t *)memregion("mainbios")->base();
	std::vector<uint16_t> buf(0x80000/2);

	for (int i = 0; i < 0x80000/2; i++)
	{
		// address xor
		int addr = i ^ 0x0020;
		if ( i & 0x00020) addr ^= 0x0010;
		if (~i & 0x00010) addr ^= 0x0040;
		if (~i & 0x00004) addr ^= 0x0080;
		if ( i & 0x00200) addr ^= 0x0100;
		if (~i & 0x02000) addr ^= 0x0400;
		if (~i & 0x10000) addr ^= 0x1000;
		if ( i & 0x02000) addr ^= 0x8000;
		addr ^= address[((i >> 1) & 0x38) | (i & 7)];
		buf[i] = rom[addr];

		// data xor
		if (buf[i] & 0x0004) buf[i] ^= 0x0001;
		if (buf[i] & 0x0010) buf[i] ^= 0x0002;
		if (buf[i] & 0x0020) buf[i] ^= 0x0008;
	}

	memcpy(rom, &buf[0], 0x80000);
}


/*************************************
 *
 *  Game-specific inits
 *
 *************************************/

// macros allow code below to be copy+pasted into slot devices more easily
#define cpuregion memregion("maincpu")->base()
#define cpuregion_size memregion("maincpu")->bytes()
#define spr_region memregion("sprites")->base()
#define spr_region_size memregion("sprites")->bytes()
#define fix_region memregion("fixed")->base()
#define fix_region_size memregion("fixed")->bytes()
#define ym_region memregion("ymsnd:adpcma")->base()
#define ym_region_size memregion("ymsnd:adpcma")->bytes()
#define audiocpu_region memregion("audiocpu")->base()
#define audio_region_size memregion("audiocpu")->bytes()
#define audiocrypt_region memregion("audiocrypt")->base()
#define audiocrypt_region_size memregion("audiocrypt")->bytes()


/*********************************************** non-carts */


void neopcb_state::write_bankpvc(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// write to cart ram
	m_pvc_prot->protection_w(offset, data, mem_mask);

	// actual bankswitch
	if (offset >= 0xff8)
	{
		m_bank_base = m_pvc_prot->get_bank_base();
		m_cpu_bank->set_base(m_region_maincpu->base() + m_bank_base);
	}
}

void neopcb_state::install_common()
{
	// install memory bank
	m_maincpu->space(AS_PROGRAM).install_rom(0x000080, 0x0fffff, (uint16_t *)m_region_maincpu->base() + 0x80/2);
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x200000, 0x2fffff, m_cpu_bank);
	m_cpu_bank->set_base(m_region_maincpu->base() + 0x100000);

	// install protection handlers + bankswitch handler
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2fe000, 0x2fffff, read16sm_delegate(*m_pvc_prot, FUNC(pvc_prot_device::protection_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x2fe000, 0x2fffff, write16s_delegate(*this, FUNC(neopcb_state::write_bankpvc)));

	// perform basic memory initialization that are usually done on-cart
	m_curr_slot = 0;
	m_bank_base = 0;
	init_audio();
	m_audiocpu->reset();
	m_ym->reset();
	init_sprites();
}

void neopcb_state::install_banked_bios()
{
	m_maincpu->space(AS_PROGRAM).install_read_bank(0xc00000, 0xc1ffff, 0x0e0000, m_bios_bank);
	m_bios_bank->configure_entries(0, 2, memregion("mainbios")->base(), 0x20000);
	m_bios_bank->set_entry(1);

}

void neopcb_state::init_ms5pcb()
{
	install_common();
	install_banked_bios();

	m_sprgen->m_fixed_layer_bank_type = 2;

	m_pvc_prot->mslug5_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 2);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);

	svcpcb_gfx_decrypt();
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, MSLUG5_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);

	svcpcb_s1data_decrypt();
}


void neopcb_state::init_svcpcb()
{
	install_common();
	install_banked_bios();

	m_sprgen->m_fixed_layer_bank_type = 2;

	m_pvc_prot->svc_px_decrypt(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 3);
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);

	svcpcb_gfx_decrypt();
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, SVC_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);

	svcpcb_s1data_decrypt();
}


void neopcb_state::init_kf2k3pcb()
{
	install_common();

	m_sprgen->m_fixed_layer_bank_type = 2;

	m_pvc_prot->kf2k3pcb_decrypt_68k(cpuregion, cpuregion_size);
	m_pcm2_prot->swap(ym_region, ym_region_size, 5);
	kf2k3pcb_sp1_decrypt();
	m_cmc_prot->cmc50_m1_decrypt(audiocrypt_region, audiocrypt_region_size, audiocpu_region, audio_region_size);
	// extra little swap on the m1 - this must be performed AFTER the m1 decrypt
	// or the m1 checksum (used to generate the key) for decrypting the m1 is
	// incorrect
	uint8_t* rom = memregion("audiocpu")->base();
	for (int i = 0; i < 0x90000; i++)
		rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 0, 7, 2);

	kf2k3pcb_gfx_decrypt();
	m_cmc_prot->cmc50_gfx_decrypt(spr_region, spr_region_size, KOF2003_GFX_KEY);
	m_cmc_prot->sfix_decrypt(spr_region, spr_region_size, fix_region, fix_region_size);
	kf2k3pcb_decrypt_s1data();

	m_maincpu->space(AS_PROGRAM).install_rom(0xc00000, 0xc7ffff, 0x080000, memregion("mainbios")->base());  // 512k bios
}

} // anonymous namespace


GAME( 2003, ms5pcb,     0,        neopcb,   dualbios, neopcb_state, init_ms5pcb,   ROT0, "SNK Playmore", "Metal Slug 5 (JAMMA PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 2003, svcpcb,     0,        neopcb,   dualbios, neopcb_state, init_svcpcb,   ROT0, "Playmore / Capcom", "SNK vs. Capcom - SVC Chaos (JAMMA PCB, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 2003, svcpcba,    svcpcb,   neopcb,   dualbios, neopcb_state, init_svcpcb,   ROT0, "Playmore / Capcom", "SNK vs. Capcom - SVC Chaos (JAMMA PCB, set 2)", MACHINE_SUPPORTS_SAVE ) /* Encrypted Code */
GAME( 2003, kf2k3pcb,   0,        neopcb,   neogeo,   neopcb_state, init_kf2k3pcb, ROT0, "SNK Playmore", "The King of Fighters 2003 (Japan, JAMMA PCB)", MACHINE_SUPPORTS_SAVE )
