// license:BSD-3-Clause
// copyright-holders: Angelo Salese

/****************************************************************************

    招级疯斗 - "Chaoji Fengdou" - "Crazy Class" HW

    driver by Angelo Salese, based off original ddz.cpp by ElSemi

    TODO:
    - Decryption;
    - Once decrypted, verify if BattleFront needs to be moved to own driver

=============================================================================

    AI notes on the TJF sets (crzclass, baibians, cjmx, btlfront)

    Scrambling
    ----------
    The program area is stored with the order of the 16-bit words reversed,
    bytes within each word untouched:

        plain_word[k] = enc_word[N - 1 - k]

    It is a single region, not per-block. Length is 0x80000 for the three card
    games and 0x20000 (head of rom5) for btlfront. Confirmed by the libc
    strings coming out contiguous and in order ("(null)", "0123456789ABCDEF",
    "bug in vfprintf: bad base", "Inf", "NaN") and by the short conditional
    branches that straddle 64K boundaries: with a single region they all
    resolve onto valid instructions, with 64K blocks one of them lands on data.

    ddz does not belong to this family: the transform does not apply to it and
    its scrambling is still unknown.

    In crzclass "(null)" decodes as ".null)" and "0123456789abcdef" is
    truncated after "cd". This is not a descrambling error - the same strings
    are clean in btlfront. That sector of the crzclass flash is damaged or
    rewritten at runtime.

    ISP stub
    --------
    The three card games each contain a 1 KB blob stored differently from the
    rest: its four 0x100-byte pieces are in reverse order but internally
    forward. It is byte-identical in all three, at different offsets:

        crzclass  0x00C400
        baibians  0x004800
        cjmx      0x00CC00
        SHA1      07d586fe24fbbd498e0318a700824ea71d16dce2

    Reassembled it has an 80-entry vector table at the start, reset = 0x140,
    and at 0x140 the stack setup followed by the usual VRender0 init (PLL at
    0x01804000/4, SDRAM at 0x01800400/8, video at 0x03000100). It then reads a
    16-bit load address from a device at 0x0154xxxx, downloads 0x200 bytes to
    it and jumps there. It is a factory bootstrap, not the game boot path.
    On the btlfront PCB the DB25 connector is the likely counterpart.

    Load bases
    ----------
    Derived from absolute stores (LERI + ST with index register 0, which can
    only target data) and from LDI constants:

        crzclass  0x02100000      absolute stores cluster at 0x02100000,
        baibians  0x02200000      0x02120000 and 0x02140000, all of which fall
        cjmx      0x02200000      inside the image only with this base
        btlfront  0x02720000      four independent constants agree exactly:
                                  0272E1EC-0xE1EC, 0272EEB0-0xEEB0,
                                  0272F208-0xF208, 0272F272-0xF272

    ROM layout
    ----------
    crzclass / baibians / cjmx (3 x 4 MB, loaded as one 12 MB region):

        0x000000-0x070000   SE3208 code (scrambled)
        0x070000-0x080000   rodata and padding, ~82% blank (scrambled)
        0x080000-0xC00000   graphics and audio, NOT scrambled

    The asset area is stored plain: crzclass has 261 "GoldWave" strings in
    clear but no RIFF/WAVE/fmt/data headers, so the audio is raw PCM blocks
    with metadata rather than WAV files. No SE3208 code exists anywhere
    outside 0x000000-0x070000.

    btlfront:

        rom5  MX 29L1611GPC-10, 2 MB
              0x000000-0x020000   SE3208 code (scrambled)
              rest                not code in any orientation
        rom1  mxs0331   graphics, 0x000000-0x3C0000 + 0x400000-0x630000
        rom2  mxs0304   8-bit unsigned PCM, 0x000000-0x440000 + 0x460000-0x4C0000
                        (sample mean 126.5-126.9, i.e. centred on 128)
        rom3  mxs0239   graphics, sparse, seven regions, 0x520000 used
        rom4  mxs0524   graphics, full 8 MB

    rom1-rom4 are dumped as 0x800400 bytes but the trailing 0x400 is all FF in
    every one of them: real content is 8 MB, the extra 1 KB is dump padding.

    Graphics and samples are addressed in the vr0soc audiovideo space: rom5
    code references 0x03E00000, 0x03E10000, 0x03E50000, 0x03E80000, 0x03ED0000
    and 0x03F00000; crzclass code references 0x03ADxxxx.

    Why none of them boots
    ----------------------
    No set has a valid reset vector at 0, so nothing runs as mapped. Forcing
    the descrambled image into work RAM at the base above and setting the PC
    by hand runs real code for a few dozen instructions and then derails:
    offset 0 of the image is a function epilogue, not an entry point.

    Beyond that, the descrambled images contain absolute references to code
    addresses that do not correspond to code anywhere in the dumps under any
    base tried - 0x020E8xxx in crzclass (the vfprintf switch table),
    0x020EAxxx in cjmx. In btlfront the equivalent table does resolve inside
    rom5, so at least that module is self-contained there.

    A statistical argument was also attempted: a linear sweep of long
    (LERI-prefixed) CALL/JMP shows ~25% of targets falling outside the image,
    clustered in narrow bands. That measurement should NOT be trusted: on the
    same data, in-range targets land on a plausible function entry only 15% of
    the time against a 12% random baseline, so the sweep is dominated by false
    decodes in data regions. A proper recursive-descent pass from known-good
    seeds is needed before claiming how much code is actually missing.

    What is certain is only that something outside the dumped data is needed.
    A separate BIOS ROM was suspected by analogy with crystal.cpp, where the
    boot ROM sits at 0x00000000 and the game media is mapped elsewhere. Photos
    of the btlfront PCB show no EPROM, however: besides the VRenderZERO+, the
    three SDRAMs, the five mask/flash ROMs and the buffers, the only remaining
    candidates are two unmarked QFPs of roughly 144 and 44 pins. The same
    unmarked parts are present on the crzclass board.

    The current suspicion is that the BIOS lives inside the larger of the two
    QFPs, as an ASIC or microcontroller with embedded ROM, and would therefore
    have to be extracted rather than simply dumped. That would fit what the
    code does: the missing region is only ever read and called, never written,
    and it sits at fixed absolute addresses that do not move with the game's
    load base, which is what a resident firmware looks like. The SoC itself is
    ruled out - the VRenderZERO+ is documented and has no on-die ROM - so the
    only remaining candidates are the two unmarked parts, with the larger one
    by far the more likely.

trivia: TJF stands for Tai Ji Feng
****************************************************************************/

#include "emu.h"

#include "cpu/se3208/se3208.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/vrender0.h"
#include "sound/vrender0.h"
#include "video/vrender0.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "endianness.h"

#include <algorithm>


namespace {

class ddz_state : public driver_device
{
public:
	ddz_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_encdata(*this, "enc_data"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc")
	{ }


	void init_ddz() ATTR_COLD;
	void ddz(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_shared_ptr<u32> m_workram;
	required_region_ptr<u8> m_encdata;

	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;

	void main_map(address_map &map) ATTR_COLD;
};


void ddz_state::main_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).rom().nopw().region("ipl", 0);

//  map(0x01500000, 0x01500003).portr("IN0");
//  map(0x01500004, 0x01500007).portr("IN1");
//  map(0x01500008, 0x0150000b).portr("IN2");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));

	map(0x02000000, 0x027fffff).ram().share(m_workram);

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));
}

static INPUT_PORTS_START( ddz )
	PORT_START("IN0")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void ddz_state::machine_start()
{
	// ...
}

void ddz_state::machine_reset()
{
	// ...
}

void ddz_state::ddz(machine_config &config)
{
	SE3208(config, m_maincpu, 14'318'180 * 3); // TODO : dynamic via PLL
	m_maincpu->set_addrmap(AS_PROGRAM, &ddz_state::main_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	VRENDER0_SOC(config, m_vr0soc, 14'318'180 * 6); // TODO : dynamic via PLL
	m_vr0soc->set_host_space_tag(m_maincpu, AS_PROGRAM);
	m_vr0soc->int_callback().set_inputline(m_maincpu, se3208_device::SE3208_INT);

	SPEAKER(config, "speaker", 2).front();
	m_vr0soc->add_route(0, "speaker", 1.0, 0);
	m_vr0soc->add_route(1, "speaker", 1.0, 1);
}

ROM_START( ddz )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD("ddz.001.rom",  0x000000, 0x400000, CRC(b379f823) SHA1(531885b35d668d22c75a9759994f4aca6eacb046) )
	ROM_LOAD("ddz.002.rom",  0x400000, 0x400000, CRC(285c744d) SHA1(2f8bc70825e55e3114015cb263e786df35cde275) )
	ROM_LOAD("ddz.003.rom",  0x800000, 0x400000, CRC(61c9b5c9) SHA1(0438417398403456a1c49408881797a94aa86f49) )
ROM_END

/* "Chaoji Fengdou" PCB
     ______________________________________________________________
    |                             _________  ____________________  |
  __|                            HD74HC245P |ROM1 H28F320BDJ-TTL80 |
 |__           RAM3 ________      _________ |____________________| |
 |__ M         K4S641632K-UC60   HD74HC245P   ____________________ |
 |__ A             |________|     _________  |ROM2 H28F320BDJ-TTL80|
 |__ H                           HD74HC245P  |____________________||
 |__ -                    XTAL    _________   ____________________ |
 |__ J       ___________  14.    HD74HC245P  |ROM3 H28F320BDJ-TTL80|
 |__ O      |VRenderZero| 318     ________   |____________________||
 |__ N      |MagicEyes  | MHz     HD74HC00P   ____________________ |
 |__ G      |EISC       |         ________   |ROM4 - EMPTY        ||
 |__        |___________|        |Unknown|   |____________________||
    |                                                              |
    |  RAM1 ________  RAM2 ________    _________   _____           |
    | K4S641632K-UC60 K4S641632K-UC60 | Unknown|  |Unkn|           |
    |      |________|     |________|  |        |  |____|      LED  |
    | ________                        |________|               o   |
  __| ULN2003AN                                         __         |
 |                                                      | |        |
 |__                                        ULN2003AN-> | |  ____  |
 |                                                      |_|  |K1|  |
 |__                                                         |__|  |
    |       _____       ___     JAMMA               _____   Switch |
    |______|     |_|_| |   |_|_|_|_|_|_|_|_|_|_|_|_|     |_________|
*/
ROM_START( crzclass ) // 超级疯斗
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD("tjf-mahjong-rom1.bin",  0x000000, 0x400000, CRC(0a8af816) SHA1(9f292e847873078ed2b7584f463633cf9086c7e8) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD("tjf-mahjong-rom2.bin",  0x400000, 0x400000, CRC(2a04e84a) SHA1(189b16fd4314fd2a5f8a1214618b5db83f8ac59a) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD("tjf-mahjong-rom3.bin",  0x800000, 0x400000, CRC(1cacf3f9) SHA1(e6c88c98aeb7df4098f8e20f412018617005724d) ) // SHARP LH28F320BJD-TTL80
	// rom4 not populated
ROM_END

ROM_START( baibians ) // 百变双扣, same PCB as crzclass
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD("rom1", 0x000000, 0x400000, CRC(b9a61efe) SHA1(e0114c231bf301779033b357bc65b133f2ab00be) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD("rom2", 0x400000, 0x400000, CRC(8b17c3a8) SHA1(b7c65fe9c262ec819ecb0d92e885c6b67d1ee938) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD("rom3", 0x800000, 0x400000, CRC(41b9b943) SHA1(3379ddf851818209cfede641e7bcc4be8b982034) ) // SHARP LH28F320BJD-TTL80
	// rom4 not populated
ROM_END

ROM_START( cjmx ) // 超级明星, same PCB as crzclass, but this one has an Amazon LF instead of the VRenderZero
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD("rom1", 0x000000, 0x400000, CRC(518ff760) SHA1(68e3fc3269060e0abd8473abfc141f038d63e4e4) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD("rom2", 0x400000, 0x400000, CRC(9caa6b70) SHA1(9914793ac772b9ae113f820c181a653ab8a9634d) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD("rom3", 0x800000, 0x400000, CRC(738db095) SHA1(073c92d8d22cbec5e6f5f5e4c25e532d2ab914d5) ) // SHARP LH28F320BJD-TTL80
	// rom4 not populated
ROM_END

ROM_START( btlfront )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x2201000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD("mxg023338.rom5", 0x0000000, 0x200000, CRC(9d27c8dd) SHA1(9ff30bde6cc8711167876e78319e6ba6a2c12390) ) // 29L1611GPC-10, there are remains of a sticker
	ROM_LOAD("mxs0331.rom1",   0x0200000, 0x800400, CRC(f3319e25) SHA1(87a743b425693434b380adb5bd73ea02a4c63796) ) // 26L6420MC-10, no sticker
	ROM_LOAD("mxs0304.rom2",   0x0a00400, 0x800400, CRC(ebc4f521) SHA1(193c13688899183230e8fb3c82355cd3662d4294) ) // 26L6420MC-90, "
	ROM_LOAD("mxs0239.rom3",   0x1200800, 0x800400, CRC(b5314232) SHA1(0e9b878ccf0c59c99a338ef824597d0a5856893a) ) // 26L6420MC-90, "
	ROM_LOAD("mxs0524.rom4",   0x1a00c00, 0x800400, CRC(fe1f0e9d) SHA1(3705d9c5318fa011de3b7a2520298524f71c606d) ) // 26L6420MC-90, "
ROM_END

void ddz_state::init_ddz()
{
	u8 *ipl = reinterpret_cast<u8 *>(memregion("ipl")->base());
	for (u32 x = 0; x < m_encdata.bytes(); x += 16)
	{
		// TODO
		for (int y = 0; y < 16; y++)
			ipl[BYTE4_XOR_LE(x + y)] = m_encdata[x + y];
//          ipl[BYTE4_XOR_LE(x + 15 - y)] = m_encdata[x + y];
	}
}

} // anonymous namespace


GAME( 200?, ddz,      0, ddz, ddz, ddz_state, init_ddz,   ROT0, "IGS?", "Dou Dizhu (IGS?)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, crzclass, 0, ddz, ddz, ddz_state, init_ddz,   ROT0, "TJF",  "Chaoji Fengdou",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 'Crazy Class'
GAME( 200?, baibians, 0, ddz, ddz, ddz_state, init_ddz,   ROT0, "TJF",  "Baibian Shuangkou", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, cjmx,     0, ddz, ddz, ddz_state, init_ddz,   ROT0, "TJF",  "Chaoji Mingxing",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2005, btlfront, 0, ddz, ddz, ddz_state, empty_init, ROT0, "TJF",  "BattleFront",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // encrypted, V133F?
