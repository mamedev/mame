/***************************************************************************

        Sun-3 Models
        ------------

    3/160
        Processor(s):   68020 @ 16.67MHz, 68881, Sun-3 MMU, 8 hardware
                        contexts, 2 MIPS
        CPU:            501-1074/1096/1163/1164/1208
        Chassis type:   deskside
        Bus:            VME, 12 slots
        Memory:         16M physical (documented), 256M virtual, 270ns cycle
        Notes:          First 68020-based Sun machine. Uses the 3004
                        "Carrera" CPU, which is used in most other Sun
                        3/1xx models and the 3/75. Sun supplied 4M
                        memory expansion boards; third parties had up to
                        32M on one card. SCSI optional. One variant of
                        the memory card holds a 6U VME SCSI board; there
                        is also a SCSI board which sits in slot 7 of the
                        backplane and runs the SCSI bus out the back of
                        the backplane to the internal disk/tape (slot 6
                        in very early backplanes). CPU has two serial
                        ports, Ethernet, keyboard. Type 3 keyboard plugs
                        into the CPU; Sun-3 mouse plugs into the
                        keyboard. Upgradeable to a 3/260 by replacing
                        CPU and memory boards.

    3/75
        Processor(s):   68020 @ 16.67MHz, 68881, Sun-3 MMU, 8 hardware
                        contexts, 2 MIPS
        CPU:            501-1074/1094/1163/1164
        Chassis type:   wide pizza box
        Bus:            VME, 2 slot
        Memory:         16M physical (documented), 256M virtual, 270ns cycle
        Notes:          Optional SCSI sits on memory expansion board in
                        second slot.

    3/140
        Processor(s):   68020 @ 16.67MHz, 68881, Sun-3 MMU, 8 hardware
                        contexts, 2 MIPS
        CPU:            501-1074/1094/1163/1164/1208
        Chassis type:   deskside
        Bus:            VME, 3 slots
        Memory:         16M physical (documented), 256M virtual, 270ns cycle

    3/150
        Processor(s):   68020 @ 16.67MHz, 68881, Sun-3 MMU, 8 hardware
                        contexts, 2 MIPS
        CPU:            501-1074/1094/1163/1164/1208
        Chassis type:   deskside
        Bus:            VME, 6 slots
        Memory:         16M physical (documented), 256M virtual, 270ns cycle

    3/180
        Processor(s):   68020 @ 16.67MHz, 68881, Sun-3 MMU, 8 hardware
                        contexts, 2 MIPS
        CPU:            501-1074/1094/1163/1164/1208
        Chassis type:   rackmount
        Bus:            VME, 12 slots
        Memory:         16M physical (documented), 256M virtual, 270ns cycle
        Notes:          Rackmount version of 3/160. Upgradeable to a
                        3/280 by replacing the CPU and memory boards.
                        Very early backplanes have the special SCSI
                        hookup on slot 6 rather than 7.

    3/110
        Processor(s):   68020
        CPU:            501-1134/1209
        Chassis type:   deskside
        Bus:            VME, 3 slots
        Notes:          Similar to the "Carerra" CPU, but has 8-bit
                        color framebuffer (cgfour) on board and uses 1M
                        RAM chips for 4M on-CPU memory. Code-named
                        "Prism".

    3/50
        Processor(s):   68020 @ 15.7MHz, 68881 (socket for
                        501-1075/1133/1162, installed for 501-1207),
                        Sun-3 MMU, 8 hardware contexts, 1.5 MIPS
        CPU:            501-1075/1133/1162/1207
        Chassis type:   wide pizza box
        Bus:            none
        Memory:         4M physical (documented), 256M virtual, 270ns cycle
        Notes:          Cycle-stealing monochrome frame buffer. 4M
                        memory maximum stock, but third-party memory
                        expansion boards were sold, allowing up to at
                        least 12M. No bus or P4 connector. Onboard SCSI.
                        Thin coax or AUI Ethernet. Code-named "Model
                        25".

    3/60
        Processor(s):   68020 @ 20MHz, 68881 (stock), Sun-3 MMU,
                        8 hardware contexts, 3 MIPS
        CPU:            501-1205/1322/1334/1345
        Chassis type:   wide pizza box
        Bus:            P4 connector (not same as P4 on 3/80)
        Memory:         24M physical, 256M virtual, 200ns cycle
        Notes:          VRAM monochome frame buffer for 501-1205/1334.
                        Optional color frame buffer (can run mono and
                        color simultaneously) on P4 connector. Onboard
                        SCSI. SIMM memory (100ns 1M x 9 SIMMs). High
                        (1600 * 1100) or low (1152 * 900) resolution
                        mono selectable by jumper. Thin coax or AUI
                        Ethernet. Code-named "Ferrari". 4M stock on
                        501-1205/1322, 0M stock on 501-1322/1345.

    3/60LE
        Processor(s):   68020 @ 20MHz, 68881 (stock), Sun-3 MMU,
                        8 hardware contexts, 3 MIPS
        CPU:            501-1378
        Bus:            P4 connector (not same as P4 on 3/80)
        Memory:         12M physical, 256M virtual, 200ns cycle
        Notes:          A version of the 3/60 with no onboard
                        framebuffer and limited to 12M of RAM (4M of
                        256K SIMMs and 8M of 1M SIMMs).

    3/260
        Processor(s):   68020 @ 25MHz, 68881 @ 20MHz (stock), Sun-3 MMU,
                        8 hardware contexts, 4 MIPS
        CPU:            501-1100/1206
        Chassis type:   deskside
        Bus:            VME, 12 slot
        Memory:         64M (documented) physical with ECC, 256M virtual;
                        64K write-back cache, direct-mapped,
                        virtually-indexed and virtually-tagged, with
                        16-byte lines; 80ns cycle
        Notes:          Two serial ports, AUI Ethernet, keyboard, and
                        video on CPU. Video is mono, high-resolution
                        only. Sun supplied 8M memory boards. Sun 4/2xx
                        32M boards work up to 128M. First Sun with an
                        off-chip cache. Upgradeable to a 4/260 by
                        replacing the CPU board. Code-named "Sirius".

    3/280
        Processor(s):   68020 @ 25MHz, 68881 @ 20MHz (stock), Sun-3 MMU,
                        8 hardware contexts, 4 MIPS
        CPU:            501-1100/1206
        Chassis type:   rackmount
        Bus:            VME, 12 slot
        Memory:         64M (documented) physical with ECC, 256M virtual;
                        64K write-back cache, direct-mapped,
                        virtually-indexed and virtually-tagged, with
                        16-byte lines; 80ns cycle
        Notes:          Rackmount version of the 3/260. Upgradeable to a
                        4/280 by replacing the CPU board. Code-named
                        "Sirius".

    3/80
        Processor(s):   68030 @ 20MHz, 68882 @ 20MHz, 68030 on-chip
                        MMU, 3 MIPS, 0.16 MFLOPS
        CPU:            501-1401/1650
        Chassis type:   square pizza box
        Bus:            P4 connector (not same as P4 on 3/60)
        Memory:         16M or 40M physical, 4G virtual, 100ns cycle
        Notes:          Similar packaging to SparcStation 1. Parallel
                        port, SCSI port, AUI Ethernet, 1.44M 3.5" floppy
                        (720K on early units?). No onboard framebuffer.
                        Code-named "Hydra". Type-4 keyboard and Sun-4
                        mouse, plugged together and into the machine
                        with a small DIN plug. 1M x 9 30-pin 100ns
                        SIMMs. Boot ROM versions 3.0.2 and later allow
                        using 4M SIMMs in some slots for up to 40M (see
                        Misc Q&A #15).

    3/460
        Processor(s):   68030 @ 33 MHz, 68882, 68030 on-chip MMU,
                        7 MIPS, 0.6 MFLOPS
        CPU:            501-1299/1550
        Bus:            VME
        Memory:         128M physical with ECC, 4G/process virtual,
                        64K cache, 80ns cycle
        Notes:          A 3/260 upgraded with a 3/4xx CPU board. Uses
                        original 3/2xx memory boards.

    3/470
        Processor(s):   68030 @ 33 MHz, 68882, 68030 on-chip MMU,
                        7 MIPS, 0.6 MFLOPS
        CPU:            501-1299/1550
        Chassis type:   deskside
        Bus:            VME
        Memory:         128M physical with ECC, 4G/process virtual,
                        64K cache, 80ns cycle
        Notes:          Rare. Code-named "Pegasus". 8M standard, uses
                        same memory boards as 3/2xx.

    3/480
        Processor(s):   68030 @ 33 MHz, 68882, 68030 on-chip MMU,
                        7 MIPS, 0.6 MFLOPS
        CPU:            501-1299/1550
        Chassis type:   rackmount
        Bus:            VME
        Memory:         128M physical with ECC, 4G/process virtual,
                        64K cache, 80ns cycle
        Notes:          Rare. Code-named "Pegasus". 8M standard, uses
                        same memory boards as 3/2xx.

    3/E
        Processor(s):   68020
        CPU:            501-8028
        Bus:            VME
        Notes:          Single-board VME Sun-3, presumably for use as a
                        controller, not as a workstation. 6U form
                        factor. Serial and keyboard ports. External RAM,
                        framebuffer, and SCSI/ethernet boards
                        available.


        25/08/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"


class sun3_state : public driver_device
{
public:
	sun3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	,
		m_p_ram(*this, "p_ram"){ }

	required_device<cpu_device> m_maincpu;
	virtual void machine_reset();

	required_shared_ptr<UINT32> m_p_ram;
};

static ADDRESS_MAP_START(sun3_mem, AS_PROGRAM, 32, sun3_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x00ffffff) AM_RAM AM_SHARE("p_ram") // 16MB
	AM_RANGE(0x0fef0000, 0x0fefffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(sun3x_mem, AS_PROGRAM, 32, sun3_state)
	AM_RANGE(0x00000000, 0x00ffffff) AM_RAM AM_SHARE("p_ram") // 16MB
	AM_RANGE(0xfefe0000, 0xfefeffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sun3 )
INPUT_PORTS_END


void sun3_state::machine_reset()
{
	UINT8* user1 = memregion("user1")->base();

	memcpy((UINT8*)m_p_ram.target(),user1,0x10000);

	machine().device("maincpu")->reset();
}


static MACHINE_CONFIG_START( sun3, sun3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68020, 16670000)
	MCFG_CPU_PROGRAM_MAP(sun3_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sun3x, sun3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68030, 16670000)
	MCFG_CPU_PROGRAM_MAP(sun3x_mem)
MACHINE_CONFIG_END

/* ROM definition */

ROM_START( sun3_50 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/50 V1.2 Bootprom
Sun 3/50 V1.4 Bootprom
Sun 3/50 V1.6 Bootprom
Sun 3/50 V1.8 Bootprom (Req. to load SunOS QIC-24 1/4" tapes)
Sun 3/50 V2.0 Bootprom
Sun 3/50 V2.1 Bootprom
Sun 3/50 V2.3 Bootprom
Sun 3/50 V2.5 Bootprom (Req. to load SunOS QIC-24 1/4" tapes from a Sun-2 Shoebox)
Sun 3/50 V2.6 Bootprom
Sun 3/50 V2.7 Bootprom
Sun 3/50 V2.8 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev28", "Rev 2.8")
	ROMX_LOAD( "sun3_50_v2.8", 0x0000, 0x10000, CRC(1ca6b0e8) SHA1(5773ac1c46399501d29d1758aa342862b03ec472), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "rev27", "Rev 2.7")
	ROMX_LOAD( "sun3_50_v2.7", 0x0000, 0x10000, CRC(7c4a9e20) SHA1(6dcd4883a170538050fd0e1f151fae413ec9ea52), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "rev26", "Rev 2.6")
	ROMX_LOAD( "sun3_50_v2.6", 0x0000, 0x10000, CRC(08abbb3b) SHA1(6bfb8d5c97d801cd7bb7d564de0e68a48fb807c4), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "rev23", "Rev 2.3")
	ROMX_LOAD( "sun3_50_v2.3", 0x0000, 0x10000, CRC(163500b3) SHA1(437c8d539e12d442ca6877566dbbe165d577fcab), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "rev16", "Rev 1.6")
	ROMX_LOAD( "sun3_50_v1.6", 0x0000, 0x10000, CRC(8be20826) SHA1(2a4d73fcb7fe0f0c83eb0f4c91d957b7bf88b7ed), ROM_BIOS(5))
ROM_END

ROM_START( sun3_60 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/60 V1.0 Bootprom
Sun 3/60 V1.3 Bootprom
Sun 3/60 V1.5 Bootprom
Sun 3/60 V1.6 Bootprom (Req. to load SunOS QIC-24 1/4" tapes
Sun 3/60 V1.9 Bootprom
Sun 3/60 V2.8.3 Bootprom
Sun 3/60 V3.0.1 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev301", "Rev 3.0.1")
	ROMX_LOAD( "sun_3.60v3.0.1", 0x0000, 0x10000, CRC(e55dc1d8) SHA1(6e48414ce2139282e69f57612b20f7d5c475e74c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "rev283", "Rev 2.8.3")
	ROMX_LOAD( "sun_3.60v2.8.3", 0x0000, 0x10000, CRC(de4ec54d) SHA1(e621a9c1a2a7df4975b12fa3a0d7f106383736ef), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "rev19", "Rev 1.9")
	ROMX_LOAD( "sun_3.60v1.9",   0x0000, 0x10000, CRC(32b6d3a9) SHA1(307756ba5698611d51059881057f8086956ce895), ROM_BIOS(3))
ROM_END

ROM_START( sun3_110 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/110 V1.8 Bootprom
Sun 3/110 V2.1 Bootprom
Sun 3/110 V2.3 Bootprom
Sun 3/110 V2.6 Bootprom
Sun 3/110 V2.7 Bootprom
Sun 3/110 V2.8 Bootprom
Sun 3/110 V3.0 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev30", "Rev 3.0")
	ROMX_LOAD( "sun3_110_v3.0", 0x0000, 0x10000, CRC(a193b26b) SHA1(0f54212ee3a5709f70e921069cca1ddb8c143b1b), ROM_BIOS(1))
ROM_END

ROM_START( sun3_150 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/1[4,5,6,8]0 V1.3 Bootprom
Sun 3/1[4,5,6,8]0 V1.4 Bootprom
Sun 3/1[4,5,6,8]0 V1.5 Bootprom
Sun 3/1[4,5,6,8]0 V1.8 Bootprom (Req. to load SunOS QIC-24 1/4" tapes)
Sun 3/1[4,5,6,8]0 V2.1 Bootprom
Sun 3/1[4,5,6,8]0 V2.1 Bootprom with Capricot Rimfire 3200/3400 support (b rf(0,0,0) works)
Sun 3/1[4,5,6,8]0 V2.3 Bootprom
Sun 3/1[4,5,6,8]0 V2.6 Bootprom (Req. to load SunOS QIC-24 1/4" tapes from a Sun-2 Shoebox and for Xylogics 7053)
Sun 3/1[4,5,6,8]0 V2.7 Bootprom
Sun 3/1[4,5,6,8]0 V2.8 Bootprom
Sun 3/1[4,5,6,8]0 V2.8.4 Bootprom
Sun 3/1[4,5,6,8]0 V3.0 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev30", "Rev 3.0")
	ROMX_LOAD( "sun3_160_v3.0",   0x0000, 0x10000, CRC(fee6e4d6) SHA1(440d532e1848298dba0f043de710bb0b001fb675), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "rev284", "Rev 2.8.4")
	ROMX_LOAD( "sun3_160_v2.8.4", 0x0000, 0x10000, CRC(3befd013) SHA1(f642bb42200b794e6e32e2fe6c87d5c269c8656d), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "rev23", "Rev 2.3")
	ROMX_LOAD( "sun3_160_v2.3",   0x0000, 0x10000, CRC(09585745) SHA1(1de1725dd9e27f5a910989bbb5b51acfbdc1d70b), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "rev21rf", "Rev 2.1 RF")
	ROMX_LOAD( "sun3_160_v2.1_rf",   0x0000, 0x10000, CRC(5c7e9271) SHA1(5e4dbb50859a21f9e1d3e4a06c42494d13a9a8eb), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "rev15", "Rev 1.5")
	ROMX_LOAD( "sun3_160_v1.5",   0x0000, 0x10000, CRC(06daee37) SHA1(b9873cd48d78ad8e0c85d69966fc20c21cfc99aa), ROM_BIOS(5))
ROM_END

ROM_START( sun3_260 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/260/280 V1.8 Bootprom
Sun 3/260/280 V2.1 Bootprom ( 2x^G cause system to beep 'till reset)
Sun 3/260/280 V2.3 Bootprom
Sun 3/260/280 V2.6 Bootprom (Req. for Xylogics 7053)
Sun 3/260/280 V2.7 Bootprom
Sun 3/260/280 V2.8 Bootprom
Sun 3/260/280 V2.8.4 Bootprom
Sun 3/260/280 V3.0 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev30", "Rev 3.0")
	ROMX_LOAD( "sun3_260_v3.0", 0x0000, 0x10000, CRC(f43ed1d3) SHA1(204880436bd087ede136f853610403d75e60bd75), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "rev27", "Rev 2.7")
	ROMX_LOAD( "sun3_260_v2.7", 0x0000, 0x10000, CRC(099fcaab) SHA1(4a5233c778676f48103bdd8bab03b4264686b4aa), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "rev26", "Rev 2.6")
	ROMX_LOAD( "sun3_260_v2.6", 0x0000, 0x10000, CRC(e8b17951) SHA1(e1fdef42670a349d99b0eca9c50c8566b8bb7c56), ROM_BIOS(3))
ROM_END

ROM_START( sun3_e )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "rev28", "Rev 3.2")
	ROMX_LOAD( "sun3_e.32", 0x0000, 0x10000, CRC(acedde7e) SHA1(1ab6ec28f4365a613a5e326c34cb37585c3f0ecc), ROM_BIOS(1))
ROM_END

ROM_START( sun3_80 )
	ROM_REGION32_BE( 0x20000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/80 V1.0 Bootprom
Sun 3/80 V2.2 Bootprom
Sun 3/80 V2.3 Bootprom
Sun 3/80 V2.9.2 Bootprom
Sun 3/80 V3.0 Bootprom
Sun 3/80 V3.0.2 Bootprom
Sun 3/80 V3.0.3 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev303", "Rev 3.0.3")
	ROMX_LOAD( "sun3_80_v3.0.3", 0x0000, 0x20000, CRC(8f983115) SHA1(e4be2dcbb29fc5c60ed9d838ab241c634fdd24e5), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "rev302", "Rev 3.0.2")
	ROMX_LOAD( "sun3_80_v3.0.2", 0x0000, 0x20000, CRC(c09a3592) SHA1(830187dfe58e65289533717a797d2c42da86ac4e), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "rev30", "Rev 3.0")
	ROMX_LOAD( "sun3_80_v3.0",   0x0000, 0x20000, CRC(47e3b012) SHA1(1e045b6f542aaf7808d6567c28a9e734a8c5d815), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "rev292", "Rev 2.9.2")
	ROMX_LOAD( "sun3_80_v2.9.2", 0x0000, 0x20000, CRC(32bcf711) SHA1(7ecd4a0d0988c1d1d53fd79ac16c8456ed73ace1), ROM_BIOS(4))
ROM_END

ROM_START( sun3_460 )
	ROM_REGION32_BE( 0x20000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/460/480 V1.2.3 Bootprom
Sun 3/460/480 V2.9.1 Bootprom (2 Files, one for odd and one for even addresses)
Sun 3/460/480 V2.9.2 Bootprom
Sun 3/460/480 V2.9.3 Bootprom
Sun 3/460/480 V3.0 Bootprom (2 Files, one for odd and one for even addresses)
*/
	ROM_SYSTEM_BIOS(0, "rev291", "Rev 2.9.1")
	ROMX_LOAD( "sun3_460_v2.9.1_0", 0x00000, 0x10000, CRC(d62dbf09) SHA1(4a6b5fd7840b44fe93c9058a8973d8dd3c9f7d24), ROM_BIOS(1))
	ROMX_LOAD( "sun3_460_v2.9.1_1", 0x10000, 0x10000, CRC(3b5a5942) SHA1(ed6250e3c07d7cb62d4dd517a8637c8d37e16dc5), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "rev30", "Rev 3.0")
	ROMX_LOAD( "3_400_l.300", 0x00000, 0x10000, CRC(1312a04b) SHA1(6c3b67ba3567991897a48fe20f589ebbfcf0a35d), ROM_BIOS(2))
	ROMX_LOAD( "3_400_h.300", 0x10000, 0x10000, CRC(8d688672) SHA1(a5593844ce6af6c4f7f39bb653dc8f964b73b095), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY         FULLNAME       FLAGS */
COMP( 198?, sun3_50,   0,       0,       sun3,      sun3, driver_device,     0,  "Sun Microsystems", "Sun 3/50", GAME_NOT_WORKING | GAME_NO_SOUND) // Model 25
COMP( 198?, sun3_60,   0,       0,       sun3,      sun3, driver_device,     0,  "Sun Microsystems", "Sun 3/60", GAME_NOT_WORKING | GAME_NO_SOUND) // Ferrari
COMP( 198?, sun3_110,  0,       0,       sun3,      sun3, driver_device,     0,  "Sun Microsystems", "Sun 3/110", GAME_NOT_WORKING | GAME_NO_SOUND) // Prism
COMP( 198?, sun3_150,  0,       0,       sun3,      sun3, driver_device,     0,  "Sun Microsystems", "Sun 3/75/140/150/160/180", GAME_NOT_WORKING | GAME_NO_SOUND) // AKA Carrera
COMP( 198?, sun3_260,  0,       0,       sun3,      sun3, driver_device,     0,  "Sun Microsystems", "Sun 3/260/280", GAME_NOT_WORKING | GAME_NO_SOUND) // Prism
COMP( 198?, sun3_e,    0,       0,       sun3,      sun3, driver_device,     0,  "Sun Microsystems", "Sun 3/E", GAME_NOT_WORKING | GAME_NO_SOUND) // Polaris

COMP( 198?, sun3_80,   0,       0,       sun3x,     sun3, driver_device,     0,  "Sun Microsystems", "Sun 3x/80", GAME_NOT_WORKING | GAME_NO_SOUND) // Hydra
COMP( 198?, sun3_460,  0,       0,       sun3x,     sun3, driver_device,     0,  "Sun Microsystems", "Sun 3x/460/470/480", GAME_NOT_WORKING | GAME_NO_SOUND) // Pegasus
