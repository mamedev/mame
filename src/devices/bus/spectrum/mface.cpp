// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Romantic Robot Multiface One/128/3
    ----------------------------------

    " MULTI-PURPOSE INTERFACE FOR THE ZX SPECTRUM "

    MULTIFACE ONE comprises three interfaces in one box:
    1) Fully  universal  and  100%  automatic  SAVE  facility  for  tape,  microdrive,  wafadrive,  Beta,
       Discovery and indirectly (via tape) for other disc systems
    2) Joystick interface – Kempston compatible (IN 31)
    3) 8K  RAM  extension  –  fully  accessible,  usable  as a  RAM  disk,  buffer  etc.  Also  used  by
       MULTIFACE for MULTI TOOLKIT routines, buffer & other purposes.

    © Romantic Robot UK Ltd 1985


    Compatibility (real hardware)
    -------------
    mface1 models work with 48K, 128K/+2 (only in 48K mode), don't work with +2A/+3
    mface128 models work with 48K/128K/+2, don't work with +2A/+3
    mface3 works only with +2A/+3
    mprint works with 48K/128K/+2, doesn't work with +2A/+3
    incompatibility is due mainly to expansion slot pinout changes on last Amstrad built models.


    Multiface One
    -------------
    Many versions exist, a very good source of info is:  https://x128.speccy.cz/multiface/multiface.htm

    Summary:
    Earliest version has 2KB of RAM, composite video output, and no pokes or toolkit.
    Next version has 8KB of RAM, composite video output, and basic toolkit (just pokes).
    Latest and most common version dropped the composite video output, added an enable/disable switch, and full-featured toolkit.
    A special (and rare) version supports the Kempston Disc interface but drops Beta support. (not sold in stores, available only on request).
    At some point the page out port changed from 0x5f to 0x1f (rev 2.0 pcb?)
    Three pcb revs are known: 1.2, 2.0, 2.1.
    Various clone/hacked rom versions are known to exist as well.

    Roms:
    The MUxx in the rom name is pcb revision (silkscreen marking).
    The two hex digits are the rom checksum.
    With the MF menu on-screen, press Symbol Shift + A (STOP) to see checksum, space to return.

    The "joystick disable" jumper is required for use with Beta disk which would otherwise not work due to port clash.
    Real world mf1/beta combo only works spectrum->mf1->beta, with jumper open, the user manual suggests Beta users may like to fit a switch instead.
    Some confusion exactly how this works, as an original-looking schematic shows the jumper simply setting data bits 6,7 hi-z, with the actual joystick still operational.
    This isn't enough to stop the clash so perhaps schematic is for some non-Beta-supporting version or has an error.
    Assume for now the entire joystick is disabled as that's the only way Beta can work.
    Beta support seems limited to Beta v3/plus (TR-DOS 3/4.xx) models, beta128 (TR-DOS 5.xx) doesn't work.
    BIOS rom "mu21e7" has issues with Betaplus, doesn't work with betav3,beta128,betacbi, perhaps it's for some other interface entirely...?

    The enable/disable switch became necessary on later versions as games had started including checks to detect presence of the interface.
    eg. Renegade ("The Hit Squad" re-release) whilst loading, reads from 0x9f specifically to cause the MF (if present) to page in and crash the machine.

    rom maps to 0x0000
    ram maps to 0x2000

    I/O        R/W   early ver   late ver
    ---------+-----+-----------+-----------
    page in     R      0x9f        0x9f
    page out    R      0x5f        0x1f
    nmi reset   W      0x5f        0x1f
    joystick    R      0x1f        0x1f


    Multiface 128
    -------------
    128K/+2 support (also works with 48K)
    Software enable/disable (in menu, press 'o' to toggle on/off)
    Some new 128k-exclusive menu features when running in 128k mode (mode is auto-detected)
    No joystick port
    No Beta support
    Pass-through connector was optional
    Original hardware version isn't compatible with DISCiPLE/+D (port clash), has "hypertape" high-speed tape recording
    Second hardware version added DISCiPLE/+D compatibility and support as save device (uses different ports) but lost hypertape support

                   supported devices                   rom
    -----+----------------------------------------+-------------
    v1       Tape, Hypertape, Microdrive, Opus      87.1, 87.12
    v2       Tape, Microdrive, Opus, DISCiPLE/+D    87.2

    I/O        R/W      v1          v2
    ---------+-----+-----------+-----------
    page in     R      0x9f        0xbf
    page out    R      0x1f        0x3f
    nmi reset   W      0x9f        0xbf
    hide        W      0x1f        0x3f


    Multiface 3
    -----------
    +2A/+2B/+3/+3B support (doesn't work with 48K/128K/+2)
    +3 disk support
    Pass-through connector was optional
    No support for older external disk interfaces (Beta, Opus, +D etc.)
    Pcb/logic is quite different to other models, has a PAL16L8 and 74LS670 4x4 register file.
    A schematic created from the Hard Mirco clone pcb is available, used for various modern "re-creation" projects,
    official version assumed to be very similar if not identical.
    Hard Mirco clone has double size ROM (16K), but contains two identical 8K halves of data. Pcb does have ability to select rom half via software,
    so perhaps an unused multi-bios feature? (unknown if unique to clone pcb or present on official pcb also).


    Multiprint
    ----------
    Printer interface with some mface features
    No backup/snapshot feature
    Has all other usual mface features: cheat/poke, 8KB RAM, magic button etc.
    Centronics parallel printer port
    Kempston E and ZX Lprint compatible (LPRINT, LLIST, COPY etc.)
    Unique "REM" command set (commands "hidden" in rem comments)
    Pass-through connector was optional

    Todo: find other print command hooks, only magic button triggered menu works for now...


*********************************************************************/

#include "emu.h"
#include "mface.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_MFACE1V1, spectrum_mface1v1_device, "spectrum_mface1v1", "Multiface One v1")
DEFINE_DEVICE_TYPE(SPECTRUM_MFACE1V2, spectrum_mface1v2_device, "spectrum_mface1v2", "Multiface One v2")
DEFINE_DEVICE_TYPE(SPECTRUM_MFACE1V3, spectrum_mface1v3_device, "spectrum_mface1v3", "Multiface One v3")
DEFINE_DEVICE_TYPE(SPECTRUM_MFACE1, spectrum_mface1_device, "spectrum_mface1", "Multiface One")
DEFINE_DEVICE_TYPE(SPECTRUM_MFACE128V1, spectrum_mface128v1_device, "spectrum_mface128v1", "Multiface 128 v1")
DEFINE_DEVICE_TYPE(SPECTRUM_MFACE128, spectrum_mface128_device, "spectrum_mface128", "Multiface 128")
DEFINE_DEVICE_TYPE(SPECTRUM_MFACE3, spectrum_mface3_device, "spectrum_mface3", "Multiface 3")
DEFINE_DEVICE_TYPE(SPECTRUM_MPRINT, spectrum_mprint_device, "spectrum_mprint", "MultiPrint")


//-------------------------------------------------
//  INPUT_PORTS( mface )
//-------------------------------------------------

INPUT_PORTS_START( mface )
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Red Button") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(spectrum_mface_base_device::magic_button), 0)
INPUT_PORTS_END

INPUT_PORTS_START( mface1v2 )
	PORT_INCLUDE( mface )

	PORT_START("CONFIG")
	PORT_CONFNAME(0x02, 0x00, "Joystick Enable Jumper")
	PORT_CONFSETTING(0x00, "Closed")
	PORT_CONFSETTING(0x02, "Open")
	PORT_BIT(0xfd, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
INPUT_PORTS_END

INPUT_PORTS_START( mface1 )
	PORT_INCLUDE( mface1v2 )

	PORT_MODIFY("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "Interface Enable Switch")
	PORT_CONFSETTING(0x00, "Enabled")
	PORT_CONFSETTING(0x01, "Disabled")
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_mface_base_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mface);
}

ioport_constructor spectrum_mface1v2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mface1v2);
}

ioport_constructor spectrum_mface1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mface1);
}

//-------------------------------------------------
//  ROM( mface )
//-------------------------------------------------

ROM_START(mface1v1)  // pcb rev 1.2
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("mu12cb")
	ROM_SYSTEM_BIOS(0, "mu12cb", "MU12 CB")  // 2KB RAM, no pokes or toolkit, page out port 0x5f
	ROMX_LOAD("mf1_12_cb.rom", 0x0000, 0x2000, CRC(c88fbf9f) SHA1(c3018d1b495b8bc0a135038db0987de7091c9d4c), ROM_BIOS(0))
ROM_END

ROM_START(mface1v2)  // unknown pcb rev (probably <2.0)
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("muxx23")
	ROM_SYSTEM_BIOS(0, "muxx23", "MU x.x 23")  // 8KB RAM, pokes only (no toolkit), page out port 0x5f
	ROMX_LOAD("mf1_xx_23.rom", 0x0000, 0x2000, CRC(d4ae8953) SHA1(b442eb634a72fb63f1ccbbd0021a7a581152888d), ROM_BIOS(0))
ROM_END

ROM_START(mface1v3)  // pcb rev 2.0
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("mu20fe")
	ROM_SYSTEM_BIOS(0, "mu20fe", "MU 2.0 FE")  // pokes only (no toolkit)
	ROMX_LOAD("mf1_20_fe.rom", 0x0000, 0x2000, CRC(fa1b8b0d) SHA1(20cd508b0143166558a7238c7a9ccfbe37b90b0d), ROM_BIOS(0))
	// ROM_SYSTEM_BIOS(1, "mu2090", "MU 2.0, 90")  // alt, unknown if earlier or later, no dump
	// ROMX_LOAD("mf1_20_90.rom", 0x0000, 0x2000, CRC(2eaf8e41) SHA1(?), ROM_BIOS(1))
ROM_END

ROM_START(mface1)  // pcb rev 2.1
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("mu21e4")
	ROM_SYSTEM_BIOS(0, "mu21e4", "MU 2.1 E4")  // most common version
	ROMX_LOAD("mf1_21_e4.rom", 0x0000, 0x2000, CRC(4b31a971) SHA1(ba28754a3cc31a4ca579829ed4310c313409cf5d), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mu21e7", "MU 2.1 E7")  // alt, unknown if earlier or later, glitchy with Betaplus
	ROMX_LOAD("mf1_21_e7.rom", 0x0000, 0x2000, CRC(670f0ec2) SHA1(50fba2d628f3a2e9219f72980e4efd62fc9ec1f8), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "mu2167", "MU 2.1 67")  // Kempston Disc support, no Beta support
	ROMX_LOAD("mf1_21_67.rom", 0x0000, 0x2000, CRC(d720ec1b) SHA1(91a40d8f503ef825df3e2ed712897dbf4ca3671d), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "bc96", "Brazilian clone 96")  // Brazilian clone, unknown pcb version
	ROMX_LOAD("mf1_bc_96.rom", 0x0000, 0x2000, CRC(e6fe4507) SHA1(a7e03e7fee3aa05ce1501072aab4534ea7bae257), ROM_BIOS(3))
	// ROM_SYSTEM_BIOS(4, "bc93", "Brazilian clone 93")  // another Brazilian clone, unknown pcb version, no dump
	// ROMX_LOAD("mf1_bc_93.rom", 0x0000, 0x2000, CRC(8c17113b) SHA1(?), ROM_BIOS(4))
ROM_END

ROM_START(mface128v1)
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("v350f")
	ROM_SYSTEM_BIOS(0, "v340d", "87.1 V34 0D")
	ROMX_LOAD("mf128_34_0d.rom", 0x0000, 0x2000, CRC(8d8cfd39) SHA1(2104962bb6097e58fcab63969bbaca424a872bb5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v350f", "87.12 V35 0F")
	ROMX_LOAD("mf128_35_0f.rom", 0x0000, 0x2000, CRC(cfefd560) SHA1(6cd6fd2c0fbb40a989a568db9d08ba8eed49cbbd), ROM_BIOS(1))
ROM_END

ROM_START(mface128)  // DISCiPLE/+D compatible version
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("v363c")
	ROM_SYSTEM_BIOS(0, "vxx1d", "87.2 Vxx 1D") // unknown PCB version
	ROMX_LOAD("mf128_xx_1d.rom", 0x0000, 0x2000, CRC(f473991e) SHA1(f03f4ecbcf4a654f4775d16bda0d4cc47f884379), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v363c", "87.2 V36 3C")
	ROMX_LOAD("mf128_36_3c.rom", 0x0000, 0x2000, CRC(78ec8cfd) SHA1(8df204ab490b87c389971ce0c7fb5f9cbd281f14), ROM_BIOS(1))
ROM_END

ROM_START(mface3)
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("v5013")
	ROM_SYSTEM_BIOS(0, "v50fe", "3.9 V50 FE") // doesn't have "lock 48K" option in save menu
	ROMX_LOAD("mf3_50_fe.rom", 0x0000, 0x2000, CRC(b5c00f28) SHA1(983699a07665186f498f5827f9b35c442c2178ba), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v5013", "3.c V50 13")
	ROMX_LOAD("mf3_50_13.rom", 0x0000, 0x2000, CRC(2d594640) SHA1(5d74d2e2e5a537639da92ff120f8a6d86f474495), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "vxx9a", "Hard Micro Multiface 3 clone Vxx 9A")
	ROMX_LOAD("mf3_hm_9a.rom", 0x0000, 0x2000, CRC(2ce53095) SHA1(5fa286f1552f26575a14ab32125d59c26ce95978), ROM_BIOS(2))
	ROM_IGNORE(0x2000) // ROM is 16K, but contains two identical 8K halves

	ROM_REGION( 0x200, "plds", 0 ) // pal from Hard Micro clone, probably the same for all
	ROM_LOAD( "mf3_pal16l8a.icx", 0x000, 0x104, CRC(710186a1) SHA1(6573c2b8f55f66ef71ce3992f654080141e09739) )
ROM_END

ROM_START(mprint)
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("mpa8")
	ROM_SYSTEM_BIOS(0, "mp5a", "MP 5A")
	ROMX_LOAD("mprint_5a.rom", 0x0000, 0x2000, CRC(3a26e84b) SHA1(4714469bf25f69291f61188f52bfb11fbb8d0b33), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mpa8", "MP A8")
	ROMX_LOAD("mprint_a8.rom", 0x0000, 0x2000, CRC(a5c58022) SHA1(1356bfae3264b952f83a33e25af536c0f13f50e7), ROM_BIOS(1))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_mface_base_device::device_add_mconfig(machine_config &config)
{
	/* passthru */
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	m_exp->fb_r_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::fb_r));
}

void spectrum_mface3_device::device_add_mconfig(machine_config &config)
{
	spectrum_mface_base_device::device_add_mconfig(config);

	/* passthru (+3 compatible devices only) */
	SPECTRUM_EXPANSION_SLOT(config.replace(), m_exp, specpls3_expansion_devices, nullptr);
}

void spectrum_mprint_device::device_add_mconfig(machine_config &config)
{
	spectrum_mface_base_device::device_add_mconfig(config);

	/* printer port */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(spectrum_mprint_device::busy_w));
}

const tiny_rom_entry *spectrum_mface1v1_device::device_rom_region() const
{
	return ROM_NAME(mface1v1);
}

const tiny_rom_entry *spectrum_mface1v2_device::device_rom_region() const
{
	return ROM_NAME(mface1v2);
}

const tiny_rom_entry *spectrum_mface1v3_device::device_rom_region() const
{
	return ROM_NAME(mface1v3);
}

const tiny_rom_entry *spectrum_mface1_device::device_rom_region() const
{
	return ROM_NAME(mface1);
}

const tiny_rom_entry *spectrum_mface128v1_device::device_rom_region() const
{
	return ROM_NAME(mface128v1);
}

const tiny_rom_entry *spectrum_mface128_device::device_rom_region() const
{
	return ROM_NAME(mface128);
}

const tiny_rom_entry *spectrum_mface3_device::device_rom_region() const
{
	return ROM_NAME(mface3);
}

const tiny_rom_entry *spectrum_mprint_device::device_rom_region() const
{
	return ROM_NAME(mprint);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_mface_base_device - constructor
//-------------------------------------------------

spectrum_mface_base_device::spectrum_mface_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_exp(*this, "exp")
{
}

spectrum_mface1v2_device::spectrum_mface1v2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface_base_device(mconfig, type, tag, owner, clock)
	, m_joy(*this, "JOY")
	, m_hwconfig(*this, "CONFIG")
{
}

spectrum_mface1v2_device::spectrum_mface1v2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface1v2_device(mconfig, SPECTRUM_MFACE1V2, tag, owner, clock)
{
}

spectrum_mface1v1_device::spectrum_mface1v1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface1v2_device(mconfig, SPECTRUM_MFACE1V1, tag, owner, clock)
{
}

spectrum_mface1v3_device::spectrum_mface1v3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface1v2_device(mconfig, type, tag, owner, clock)
{
}

spectrum_mface1v3_device::spectrum_mface1v3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface1v3_device(mconfig, SPECTRUM_MFACE1V3, tag, owner, clock)
{
}

spectrum_mface1_device::spectrum_mface1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface1v3_device(mconfig, SPECTRUM_MFACE1, tag, owner, clock)
{
}

spectrum_mface128_base_device::spectrum_mface128_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface_base_device(mconfig, type, tag, owner, clock)
{
}

spectrum_mface128v1_device::spectrum_mface128v1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface128_base_device(mconfig, type, tag, owner, clock)
{
}

spectrum_mface128v1_device::spectrum_mface128v1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface128v1_device(mconfig, SPECTRUM_MFACE128V1, tag, owner, clock)
{
}

spectrum_mface128_device::spectrum_mface128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface128v1_device(mconfig, SPECTRUM_MFACE128, tag, owner, clock)
{
}

spectrum_mface3_device::spectrum_mface3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface128_base_device(mconfig, SPECTRUM_MFACE3, tag, owner, clock)
{
}

spectrum_mprint_device::spectrum_mprint_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_mface128_base_device(mconfig, SPECTRUM_MPRINT, tag, owner, clock)
	, m_centronics(*this, "centronics")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_mface_base_device::device_start()
{
	save_item(NAME(m_romcs));
	save_item(NAME(m_nmi_pending));
}

void spectrum_mface1v2_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(8 * 1024);
	save_pointer(NAME(m_ram), 8 * 1024);
	spectrum_mface_base_device::device_start();
}

void spectrum_mface1v1_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(2 * 1024);
	save_pointer(NAME(m_ram), 2 * 1024);
	spectrum_mface_base_device::device_start();
}

void spectrum_mface128_base_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(8 * 1024);
	save_pointer(NAME(m_ram), 8 * 1024);
	save_item(NAME(m_hidden));
	spectrum_mface_base_device::device_start();
}

void spectrum_mface128v1_device::device_start()
{
	spectrum_mface128_base_device::device_start();
	save_item(NAME(m_d3_ff));
}

void spectrum_mface3_device::device_start()
{
	spectrum_mface128_base_device::device_start();
	save_item(NAME(m_disable));
}

void spectrum_mprint_device::device_start()
{
	spectrum_mface128_base_device::device_start();
	m_busy = 0;
	save_item(NAME(m_busy));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_mface_base_device::device_reset()
{
	m_romcs = 0;
	m_nmi_pending = 0;
}

void spectrum_mface128_base_device::device_reset()
{
	spectrum_mface_base_device::device_reset();
	m_hidden = 1;
}

void spectrum_mface3_device::device_reset()
{
	spectrum_mface128_base_device::device_reset();
	m_disable = 0;
}

void spectrum_mprint_device::device_reset()
{
	spectrum_mface128_base_device::device_reset();
	m_centronics->write_strobe(1);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_mface_base_device::romcs()
{
	return m_romcs || m_exp->romcs();
}

void spectrum_mface_base_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		if (offset == 0x0066 && m_nmi_pending)
			m_romcs = 1;
	}
}

void spectrum_mface128_device::pre_opcode_fetch(offs_t offset)
{
	// pass-thru pin a15 is controlled by some extra logic on v2 pcb
	m_exp->pre_opcode_fetch((m_nmi_pending || m_romcs) ? offset | 0x8000 : offset);

	if (!machine().side_effects_disabled())
	{
		if (offset == 0x0066 && m_nmi_pending)
			m_romcs = 1;
	}
}

void spectrum_mprint_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		if (offset == 0x0066 && m_nmi_pending)
			m_romcs = 1;

		// todo: must be some other hooks here...
		// at least LPRINT, LLIST and COPY should be caught
		// also unique REM xx commands
		// menu should pop up for LPRINT and REM MP
	}
}

uint8_t spectrum_mface1v2_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xff)  // real decodes tbc...
		{
		case 0x1f:
			if (!(m_hwconfig->read() & 0x02))
				data = m_joy->read() & 0x1f;
			break;
		case 0x5f:
			m_romcs = 0;
			break;
		case 0x9f:
			m_romcs = 1;
			break;
		}
	}
	return data;
}

uint8_t spectrum_mface1v3_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xff)  // real decodes tbc...
		{
		case 0x1f:
			m_romcs = 0;
			if (!(m_hwconfig->read() & 0x02))
				data = m_joy->read() & 0x1f;
			break;
		case 0x9f:
			m_romcs = 1;
			break;
		}
	}
	return data;
}

uint8_t spectrum_mface1_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (m_hwconfig->read() & 0x01)  // disable switch
		return data;

	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xf2)  // -001 --1-
		{
		case 0x12:       // 0001 --1-   uses 0x1f
			m_romcs = 0;
			if (!(m_hwconfig->read() & 0x02))
				data = m_joy->read() & 0x1f;
			break;
		case 0x92:       // 1001 --1-   uses 0x9f
			m_romcs = 1;
			break;
		}
	}
	return data;
}

uint8_t spectrum_mface128v1_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xf0)  // -001 ----
		{
		case 0x10:         // 0001 ----   uses 0x1f
			m_romcs = 0;
			break;
		case 0x90:         // 1001 ----   uses 0x9f
			if (!m_hidden)
			{
				m_romcs = 1;
				data = (data & 0x7f) | (m_d3_ff << 7);
			}
			break;
		}
	}
	return data;
}

uint8_t spectrum_mface128_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xf4)  // -011 -1--
		{
		case 0x34:         // 0011 -1--   uses 0x3f
			m_romcs = 0;
			break;
		case 0xb4:         // 1011 -1--   uses 0xbf
			if (!m_hidden)
			{
				m_romcs = 1;
				data = (data & 0x7f) | (m_d3_ff << 7);
			}
			break;
		}
	}
	return data;
}

uint8_t spectrum_mface3_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xff)  // fully decoded by PAL16L8
		{
		case 0x3f:
			if (!m_hidden)
				m_romcs = 1;
			break;
		case 0xbf:
			m_romcs = 0;
			break;
		}
	}

	// mface3 monitors writes to the +3 memory ctrl registers (ports 1ffd, 7ffd)
	// keeps copies of the low nibbles in a 74LS670 4x4 register file
	if ((offset & 0x7f) == 0x3f && !m_hidden)
	{
		data = 0xf0;   // get stored copy of 1ffd, 7ffd
		data |= m_reg_file[(offset >> 13) & 3] & 0xf;
	}

	return data;
}

uint8_t spectrum_mprint_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset & 0xff)  // real decodes tbc...
		{
		case 0xbf:
			m_romcs = 0;
			break;
		case 0xbb:
			if (!m_hidden)
				m_romcs = 1;
			data &= ~0x40;
			data |= m_busy << 6;
			break;
		}
	}
	return data;
}

void spectrum_mface1v2_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xff) == 0x5f)  // real decodes tbc...
		nmi(CLEAR_LINE);

	m_exp->iorq_w(offset, data);
}

void spectrum_mface1v3_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xff) == 0x1f)  // real decodes tbc...
		nmi(CLEAR_LINE);

	m_exp->iorq_w(offset, data);
}

void spectrum_mface1_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xf2) == 0x12)  // 0001 --1-   uses 0x1f
		nmi(CLEAR_LINE);

	m_exp->iorq_w(offset, data);
}

void spectrum_mface128v1_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xf0)  // -001 ----
	{
	case 0x10:           // 0001 ----   uses 0x1f
		m_hidden = 1;
		[[fallthrough]];
	case 0x90:           // 1001 ----   uses 0x9f
		nmi(CLEAR_LINE);
		break;
	}

	// mface128 monitors writes to the 128K memory ctrl register (port 7ffd)
	// keeps a copy of D3 (screen select bit) in a D flip-flop
	if ((offset & 0x8002) == 0)    // 0--- ---- ---- --0-   uses 7ffd
		m_d3_ff = (data >> 3) & 1;

	m_exp->iorq_w(offset, data);
}

void spectrum_mface128_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xf4)  // -011 -1--
	{
	case 0x34:           // 0011 -1--   uses 0x3f
		m_hidden = 1;
		[[fallthrough]];
	case 0xb4:           // 1011 -1--   uses 0xbf
		nmi(CLEAR_LINE);
		break;
	}

	// mface128 monitors writes to the 128K memory ctrl register (port 7ffd)
	// keeps a copy of D3 (screen select bit) in a D flip-flop
	if ((offset & 0x8002) == 0)    // 0--- ---- ---- --0-   uses 7ffd
		m_d3_ff = (data >> 3) & 1;

	m_exp->iorq_w(offset, data);
}

void spectrum_mface3_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xff)  // fully decoded by PAL16L8
	{
	case 0x3f:
		m_hidden = 1;      // also Hard Micro clone: D0 is 16KB rom msb (unused)
		[[fallthrough]];
	case 0xbf:
		nmi(CLEAR_LINE);
		break;
	}

	// mface3 monitors writes to the +3 memory ctrl registers (ports 1ffd, 7ffd)
	// keeps copies of the low nibbles in a 74LS670 4x4 register file
	if ((offset & 0x90fd) == 0x10fd)
		m_reg_file[(offset >> 13) & 3] = data & 0x0f;

	// also disables itself if special paging mode bit is set (CP/M)
	if ((offset & 0xf0fd) == 0x10fd)
		m_disable = data & 1;

	m_exp->iorq_w(offset, data);
}

void spectrum_mprint_device::iorq_w(offs_t offset, uint8_t data)
{
	// write_strobe is fired by wr seq: 0xbf, 0xbb, 0xbf
	switch (offset & 0xff)  // real decodes tbc...
	{
	case 0xbf:
		m_centronics->write_data0(BIT(data, 0));
		m_centronics->write_data1(BIT(data, 1));
		m_centronics->write_data2(BIT(data, 2));
		m_centronics->write_data3(BIT(data, 3));
		m_centronics->write_data4(BIT(data, 4));
		m_centronics->write_data5(BIT(data, 5));
		m_centronics->write_data6(BIT(data, 6));
		m_centronics->write_data7(BIT(data, 7));
		m_centronics->write_strobe(1);
		break;
	case 0xbb:
		m_centronics->write_strobe(0);
		break;
	}

	switch (offset & 0x5ff)
	{
	case 0x4bf:
		nmi(CLEAR_LINE);
		break;
	case 0x4fb:  // not essential yet, for other hooks?
		m_hidden = 0;
		break;
	case 0x5fb:
		m_hidden = 1;
		break;
	}

	m_exp->iorq_w(offset, data);
}

uint8_t spectrum_mface_base_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		switch (offset & 0xe000)
		{
		case 0x0000:
			data = m_rom->base()[offset & 0x1fff];
			break;
		case 0x2000:
			data = m_ram[offset & 0x1fff];
			break;
		}
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

uint8_t spectrum_mface1v1_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		switch (offset & 0xe000)
		{
		case 0x0000:
			data = m_rom->base()[offset & 0x1fff];
			break;
		case 0x2000:
			data = m_ram[offset & 0x7ff];
			break;
		}
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_mface_base_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xe000)
		{
		case 0x2000:
			m_ram[offset & 0x1fff] = data;
			break;
		}
	}

	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

void spectrum_mface1v1_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xe000)
		{
		case 0x2000:
			m_ram[offset & 0x7ff] = data;
			break;
		}
	}

	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

INPUT_CHANGED_MEMBER(spectrum_mface1v2_device::magic_button)
{
	if (!newval)
	{
		nmi(ASSERT_LINE);
	}
}

INPUT_CHANGED_MEMBER(spectrum_mface1_device::magic_button)
{
	if (!newval)
	{
		if (!(m_hwconfig->read() & 0x01))
		{
			nmi(ASSERT_LINE);
		}
	}
}

INPUT_CHANGED_MEMBER(spectrum_mface128_base_device::magic_button)
{
	if (!newval)
	{
		m_hidden = 0;
		nmi(ASSERT_LINE);
	}
}

INPUT_CHANGED_MEMBER(spectrum_mface3_device::magic_button)
{
	if (!newval)
	{
		if (!m_disable)
		{
			m_hidden = 0;
			nmi(ASSERT_LINE);
		}
	}
}
