// license:BSD-3-Clause
// copyright-holders:Xing Xing

/* PGM 3 hardware.

  Games on this platform

  Knights of Valour 3 HD

  according to Xing Xing
  "The main cpu of PGM3 whiched coded as 'SOC38' is an ARM1176@800M designed by SOCLE(http://www.socle-tech.com/). Not much infomation is available on this asic"

  the card images seem to have encrypted data up to the C2000000 mark, then
  some text string about a non-bootable disk followed by mostly blank data

  Offset(h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

0C2000000  EB 3C 90 6D 6B 64 6F 73 66 73 00 00 02 04 04 00  ë<.mkdosfs......
0C2000010  02 00 02 00 00 F8 00 01 3F 00 FF 00 00 00 00 00  .....ø..?.ÿ.....
0C2000020  00 00 04 00 00 00 29 4C 88 BA 7C 20 20 20 20 20  ......)L.º|
0C2000030  20 20 20 20 20 20 46 41 54 31 36 20 20 20 0E 1F        FAT16   ..
0C2000040  BE 5B 7C AC 22 C0 74 0B 56 B4 0E BB 07 00 CD 10  ¾[|¬"Àt.V´.»..Í.
0C2000050  5E EB F0 32 E4 CD 16 CD 19 EB FE 54 68 69 73 20  ^ëð2äÍ.Í.ëþThis
0C2000060  69 73 20 6E 6F 74 20 61 20 62 6F 6F 74 61 62 6C  is not a bootabl
0C2000070  65 20 64 69 73 6B 2E 20 20 50 6C 65 61 73 65 20  e disk.  Please
0C2000080  69 6E 73 65 72 74 20 61 20 62 6F 6F 74 61 62 6C  insert a bootabl
0C2000090  65 20 66 6C 6F 70 70 79 20 61 6E 64 0D 0A 70 72  e floppy and..pr
0C20000A0  65 73 73 20 61 6E 79 20 6B 65 79 20 74 6F 20 74  ess any key to t
0C20000B0  72 79 20 61 67 61 69 6E 20 2E 2E 2E 20 0D 0A 00  ry again ... ...


  DSW:
    1: OFF = Game mode / ON = Test mode
    2: OFF = JAMMA / ON = JVS
    3: OFF = 16/9 (1280x720) / ON = 4/3 (800x600)
    4: NO USE

  todo: add other hardware details?

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "emupal.h"
#include "screen.h"

#include "util/aes256cbc.h"
#include "util/endianness.h"


namespace {

class pgm3_state : public driver_device
{
public:
	pgm3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram")
	{ }

	void pgm3(machine_config &config) ATTR_COLD;

	void init_kov3hd() ATTR_COLD;

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update_pgm3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_pgm3(int state);
	void decryptaes(const uint8_t *key, const uint8_t *iv, int source, int dest, int length);

	void pgm3_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u32> m_mainram;
};

void pgm3_state::decryptaes(const uint8_t *key, const uint8_t *iv, int source, int dest, int length)
{
	using namespace aes256cbc;

	address_space &mem = m_maincpu->space(AS_PROGRAM);
	AES_CTX ctx;
	uint8_t inbuffer[16];
	uint8_t outbufer[16];

	AES_DecryptInit(&ctx, key, iv);

	for (int i = 0; i < length; i += 16)
	{
		for (int j = 0; j < 16; j++)
		{
			inbuffer[j] = mem.read_byte(source + i + j);
		}

		AES_Decrypt(&ctx, inbuffer, outbufer);

		for (int j = 0; j < 16; j++)
		{
			mem.write_byte(dest + i + j, outbufer[j]);
		}
	}

	AES_CTX_Free(&ctx);
}

void pgm3_state::pgm3_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram().share("mainram");
	//map(0x00000000, 0x00007fff).rom().region("internal_mask", 0); // for full boot process the internal mask area would initially appear at 0

	map(0x10000000, 0x1007ffff).rom().region("internal_flash", 0);
	map(0x28000000, 0x2801ffff).ram();
}

static INPUT_PORTS_START( pgm3 )
INPUT_PORTS_END

uint32_t pgm3_state::screen_update_pgm3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void pgm3_state::screen_vblank_pgm3(int state)
{
}

void pgm3_state::video_start()
{
}

void pgm3_state::machine_start()
{
}

void pgm3_state::machine_reset()
{
	// perform a bootstrap to bypass the level 0 internal_mask as it might not be properly dumped
	auto bootrom = util::little_endian_cast<uint8_t>(&memregion("internal_mask")->as_u32());
	uint8_t rom_aes_key[32];
	uint8_t rom_aes_iv[16];

	for (int i = 0; i < 32; i++)
		rom_aes_key[i] = bootrom[0x42b8 + i];

	for (int i = 0; i < 16; i++)
		rom_aes_iv[i] = bootrom[0x44a8 + i];

	// the first 0x20000 bytes are encrypted with this key, it then uses other keys to decrypt the rest
	// the decryption is done in hardware, not software
	decryptaes(rom_aes_key, rom_aes_iv, 0x10000000, 0x00000000, 0x20000);

	// if we want to boot from somewhere else, change this
	//m_maincpu->set_state_int(arm7_cpu_device::ARM7_R15, 0x04000000);
}

void pgm3_state::pgm3(machine_config &config)
{
	/* basic machine hardware */
	ARM1176JZF_S(config, m_maincpu, 800000000); // SOC38 / IGS038 - ARM1176JZ based SoC
	m_maincpu->set_addrmap(AS_PROGRAM, &pgm3_state::pgm3_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1280, 720);
	screen.set_visarea(0, 1280-1, 0, 720-1);
	screen.set_screen_update(FUNC(pgm3_state::screen_update_pgm3));
	screen.screen_vblank().set(FUNC(pgm3_state::screen_vblank_pgm3));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x1000);
}

#define KOV3HD_SOC_ROMS \
	/* The mask part of the SOC38, this gets hidden by the system after use */ \
	/* Same ROM is likely used by other games as it contains a number of unused keys, with the key to use being selected by a HW register */ \
	/* kov3hd uses the AES keys at 0x42b8 and 0x44a8 in this ROM */ \
	/* the only purpose of this bootloader is to decrypt the flash part of the internal ROM below, copying it into RAM */ \
	/* (ROM was reconstructed from several decap attempts, hence BAD_DUMP as there could be errors, furthermore it was dumped from a chip */ \
	/*  marked 'production sample' so could be different from final) */ \
	ROM_REGION32_LE( 0x8000, "internal_mask", ROMREGION_ERASE00 ) \
	ROM_LOAD( "internal_boot.bin", 0x0000, 0x8000, BAD_DUMP CRC(f6877f92) SHA1(4431d73cc7e5bbb11cd53449284fff1435a6ea32) ) \
	/* internal flash is for KOV3HD, and gets decrypted using hardware AES decryption and a key from internal_mask */ \
	ROM_REGION32_LE( 0x80000, "internal_flash", ROMREGION_ERASE00 ) \
	ROM_LOAD( "internal_flash.bin", 0x0000, 0x80000, CRC(5925187d) SHA1(3acc29891142d47a7bf3c73016a06bc436977b40) )

ROM_START( kov3hd )
	KOV3HD_SOC_ROMS

	DISK_REGION( "card" )
	DISK_IMAGE( "kov3hd_m105", 0, SHA1(81af30aa6e1a34b2a8fab8c5c23a313a7164767c) )
	//DISK_IMAGE( "kov3hd_v105", 0, SHA1(c185888c59880805bb76b5c0a42b05c614dcff37) ) bad dump, doesn't work on real hardware
ROM_END

ROM_START( kov3hd104 )
	KOV3HD_SOC_ROMS

	DISK_REGION( "card" )
	DISK_IMAGE( "kov3hd_m104", 0, SHA1(899b3b81825e6f23ae8f39aa67ad5b019f387cf9) )
ROM_END

ROM_START( kov3hd103 )
	KOV3HD_SOC_ROMS

	DISK_REGION( "card" )
	DISK_IMAGE( "kov3hd_m103", 0, SHA1(0d4fd981f477cd5ed62609b875f4ddec939a2bb0) )
ROM_END

ROM_START( kov3hd102 )
	KOV3HD_SOC_ROMS

	DISK_REGION( "card" )
	DISK_IMAGE( "kov3hd_m102", 0, SHA1(a5a872f9add5527b94019ec77ff1cd0f167f040f) )
ROM_END

ROM_START( kov3hd101 )
	KOV3HD_SOC_ROMS

	DISK_REGION( "card" )
	DISK_IMAGE( "kov3hd_m101", 0, SHA1(086d6f1b8b2c01a8670fd6480da44b9c507f6e08) )
ROM_END


void pgm3_state::init_kov3hd()
{
}

} // anonymous namespace


// all dumped sets might be China region, unless region info comes from elsewhere
GAME( 2011, kov3hd,     0,      pgm3,    pgm3, pgm3_state, init_kov3hd, ROT0, "IGS", "Knights of Valour 3 HD (M-105CN 13-07-04 18:54:01)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2011, kov3hd104,  kov3hd, pgm3,    pgm3, pgm3_state, init_kov3hd, ROT0, "IGS", "Knights of Valour 3 HD (V104)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2011, kov3hd103,  kov3hd, pgm3,    pgm3, pgm3_state, init_kov3hd, ROT0, "IGS", "Knights of Valour 3 HD (V103)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2011, kov3hd102,  kov3hd, pgm3,    pgm3, pgm3_state, init_kov3hd, ROT0, "IGS", "Knights of Valour 3 HD (V102)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2011, kov3hd101,  kov3hd, pgm3,    pgm3, pgm3_state, init_kov3hd, ROT0, "IGS", "Knights of Valour 3 HD (V101)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
