// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nathan Woods,Angelo Salese, Robbbert
/***************************************************************************

    Atari Jaguar (Home) & Atari CoJag (Arcade) hardware

    The CoJag arcade system is based on the Jaguar system, but with an upgraded
    main CPU (68020 or MIPS instead of the plain 68000 found in the Jaguar)

    driver by Aaron Giles
    console support originally by Nathan Woods

    CoJag Games supported:
        * Area 51 (4 Sets)
        * Maximum Force (3 Sets)
        * Area 51/Maximum Force Duo (2 Sets)
        * Vicious Circle
        * Fishin' Frenzy
        * Freeze

    To do:
        * (CoJag) map out unused RAM per-game via memory_nop_read/write
        * (Jaguar) support is very poor, most games aren't properly playable
          or have severe performance issues or crashes related to the unsafe
          blitter code. Please refer to jaguar SW list file for more details.
        * The code (GPU/DSP access) should probably be refactored around the
          16-bit interface from the plain 68k, the driver currently uses
          trampoline functions due to the original driver being entirely
          32-bit due to the CPUs on CoJag.

    Note: There is believed to be a 68020 version of Maximum Force
            (not confirmed or dumped)

****************************************************************************

    Area51/Maximum Force (c)1997 Atari Games
    Maximum Force
    A055451

    Components:
    sdt79r3041-20j
    Atari Jaguar CPU V1.0 6sc880hf106
    Atari Jaguar DSP V1.0 sc414201ft (has Motorolla logo)
    Altera epm7128elc84-15 marked A-21652
    VIA vt83c461 IDE controller
    Actel a1010b marked A-22096 near IDE and gun inputs
    Dallas ds1232s watchdog
    52MHz osc near Altera PLCC
    40MHz osc near 79R3041
    14.318180MHz osc near Jag DSP
    12x hm514260cj7 RAM (near Jaguar CPU/DSP)
    4x  sdt71256 RAM (near Boot ROMs's)
    Atmel atf16v8b marked a-21647 (near Jag CPU)
    Altera ep22lc-10 marked A-21648 (near Jag DSP)
    ICT 22cv10aj marked A-21649 (near Jag CPU)
    ICT 22cv10aj marked A-21650 (near Jag CPU)
    ICT 22cv10aj marked A-21651 (near Jag CPU)
    tea6320t
    AKM ak4310vm
    tda1554q amplifier
    Microchip 28c16a-15 BRAM

    ROM's:
    27c4001
    R3K MAX/A51 KIT
    LL
    (c)1997 Atari
    V 1.0

    27c4001
    R3K MAX/A51 KIT
    LH
    (c)1997 Atari
    V 1.0

    27c4001
    R3K MAX/A51 KIT
    HL
    (c)1997 Atari
    V 1.0

    27c4001
    R3K MAX/A51 KIT
    HH
    (c)1997 Atari
    V 1.0

    Jumpers:
    jsp1 (1/2 connected= w/sub 2/3 connected= normal speaker)
    jsp2 (1/2 connected= w/sub 2/3 connected= normal speaker)
    jamaud (1/2 connected=stereo 2/3 connected=mono)
    jimpr (1/2 connected=hi video R impedance 2/3 connected=lo)
    jimpg (1/2 connected=hi video G impedance 2/3 connected=lo)
    jimpb (1/2 connected=hi video B impedance 2/3 connected=lo)

    Connectors:
    idea  standard IDE connector
    ideb laptop size IDE connector
    jgun1 8 pin gun input
    jgun2 8 pin gun input
    xtracoin 1 6 pin (coin3/4 bills?)
    jvupdn 3 pin (?)
    jsync 3 pin (?)
    JAMMA
    jspkr left/right/subwoofer output
    hdpower 4 pin PC power connector for HD

****************************************************************************

Area 51 (Time Warner Interactive License)
Atari/Mesa Logic, 1995

This game runs on Atari Cojag 68k hardware.

PCB Layouts
-----------

Main Board:

COJAG A053538
ATARI GAMES (C) 1994
(Atari comment near AMP - 'MMM, DONUTS.')
|--------------------------------------------------|
|TDA1554Q  TEA6320T   AK4310            JXCLKTRM   |
|JSPKR     ST91D314                        XC7336  |
|         JSP1 JWELLSR  PAL4  PAL5                 |
|   JSP4  JSP2 PAL1            45160 45160  *   *  |
| HDPOWER JSP3 JWELLSB                        JXBUS|
| SELFTEST     PAL2     PAL6   45160 45160  *   *  |
|     LED      JWELLSG                             |
|     LED              |------|45160 45160  *   *  |
|J    LED              |JAGUAR|                    |
|A                     | CPU  |45160 45160  *   *  |
|M                     |      |                    |
|M                     |------|                    |
|A                                                 |
|   14.31818MHz        |------|                    |
|   LM613              |JAGUAR|                    |
|   52MHz  PAL3        | DSP  |                    |
|                      |      |                    |
|  JSYNC               |------|                    |
|  JVUPDN           DIP8                           |
|              A1010B                              |
|JPLY3   JVCR            VT83C461                  |
|                                                  |
|                               LED                |
|  JPLY4    JGUN2  JGUN1  JIDEB                    |
|--------------------------------------------------|
Notes:
      JAGUAR CPU - Atari Jaguar CPU (QFP208)
      JAGUAR DSP - Atari Jaguar DSP (QFP160)
      45160      - TMS45160DZ-70 512K x16 DRAM (SOJ40)
                   Note: This RAM is in banks. There are 4 banks, each bank is 2MBytes x 64bit
                   Only banks 0 and 1 are populated.
      *          - Unpopulated DRAM positions (banks 2 and 3)
      TDA1554Q   - Philips TDA1554Q power AMP
      TEA6320T   - Philips TEA6320T op AMP (SOP32)
      AK4310     - AKM AK4310VM (SOIC24)
      ST91D314   - STS Microelectronics ST91D314 3403 op AMP (SOIC14)
      PAL1       - ATMEL ATF16V8B PAL (labelled '136105-0006', PLCC20)
      PAL2       - ATMEL ATF16V8B PAL (labelled '136105-0006', PLCC20)
      PAL3       - ATMEL ATF16V8B PAL (labelled '136105-0007', PLCC20)
      PAL4, PAL5 - Philips PL22V10-10A PAL (labelled 'MYF 136105-0008', PLCC28)
      PAL6       - ATMEL ATF16V8B (labelled '136105-0005', PLCC20)
      A1010B     - ACTEL A1010B Complex Programmable Logic Device (labeled 'MYF 136105-0010', PLCC44)
      XC7336     - Xilinx XC7336 Complex PRogrammable Logic Device (labelled 'MYF 136105-0009', PLCC44)
      VT83C461   - VIA VT83C461 IDE Hard Drive Controller (QFP100)
      LM613      - (DIP16)
      SELFTEST   - Test Switch
      HDPOWER    - Standard (PC-type) 4 pin hard drive power connector
      JSP1       - 3 pin jumper block, set to 2-3
      JSP2       - 3 pin jumper block, set to 2-3
      JSP3       - 3 pin jumper block, jumper not installed
      JSP4       - 3 pin jumper block, set to 2-3
      JWELLSR    - 3 pin jumper block, set to 2-3
      JWELLSB    - 3 pin jumper block, set to 2-3
      JWELLSG    - 3 pin jumper block, set to 2-3
      JSYNC      - 3 pin jumper block, jumper not installed
      JVUPDN     - 3 pin jumper block, jumper not installed
      JVCR       - 3 pin jumper block, jumper not installed
      JXCLKTRM   - 2 pin header, not shorted
      JPLY3      - Connector for player 3 controls
      JPLY4      - Connector for player 4 controls
      JGUN1      - Connector for player 1 gun
      JGUN2      - Connector for player 2 gun
      JIDEB      - Connector for 40 pin IDE cable connected to Quantum Fireball 1080AT IDE hard drive (C/H/S = 2112/16/63)
      JSPKR      - Connector for left/right speaker for stereo sound output
      DIP8       - Unpopulated DIP8 socket
      JXBUS      - 96 pin connector to top board


Top Board:

EC20X32
A053448
ATARI GAMES (C) 1994
|------------------------------|
|               136105-0002C.3P|
| 71256                        |
|               136105-0001C.3M|
| 71256                        |
|               136105-0000C.3K|
| 71256                        |
|               136105-0003C.3H|
| 71256                        |
|              LED             |
|                              |
| AT28C16  DS1232   XC7354     |
|                              |
|                              |
|      MC68EC020               |
|                              |
| 50MHz                        |
|------------------------------|
Notes:
      MC68EC020       - Motorola 68EC020FG25 CPU clocked at 25MHz (QFP100)
      AT28C16         - ATMEL 2k x8 EEPROM (DIP24)
      XC7354          - Xilinx XC7354 Complex Programmable Logic Device (labelled 'MYF 136105-0004', PLCC68, socketed)
      DS1232          - Dallas DS1232 System Reset IC (DIP8)
      71256           - 32K x8 SRAM (SOJ28)
      136105-0002C.3P - 27C040 EPROM (labelled 'AREA 51 136105-0002C (C)1995 ATARI GMS CS 55FE', DIP32)
      136105-0001C.3M - 27C040 EPROM (labelled 'AREA 51 136105-0001C (C)1995 ATARI GMS CS 3DFD', DIP32)
      136105-0000C.3K - 27C040 EPROM (labelled 'AREA 51 136105-0000C (C)1995 ATARI GMS CS 63FC', DIP32)
      136105-0003C.3H - 27C040 EPROM (labelled 'AREA 51 136105-0003C (C)1995 ATARI GMS CS 45FF', DIP32)

****************************************************************************

Maximum Force
Atari, 1997

PCB Layout
----------

MAXIMUM FORCE A055451 ATARI GAMES (C) 1996
|-----------------------------------------------------------------------------------|
|HM514260CJ7 HM514260CJ7 HM514260CJ7 HM514260CJ7            AT28C16                 |
|                                                                                   |
|HM514260CJ7 HM514260CJ7 HM514260CJ7 HM514260CJ7            PROGLL.17Y   PROGLH.21Y |
|                                                                                   |
|HM514260CJ7 HM514260CJ7 HM514260CJ7 HM514260CJ7            PROGHL.21V   PROGHH.21V |
|                                                                                   |
|                                                                                   |
|             |------|      |------|                                                |
|             |JAGUAR|      |JAGUAR|                                                |
|GAL     GAL  |GPU   |      |DSP   |                     |----------|               |
|             |------|      |------|                     |          |               |
|                                                        |IDT       |               |
|GAL     GAL                                     40MHz   |79R3041-20|               |
|                                          GAL           |          |               |
|                                    14.31818MHz         |----------|           IDE |
|AK4310VM                                                                      CONN |
|                                                       |-----------|               |
|LM78L05                                     52MHz      |ALTERA MAX |               |
|                                                       |EPM7128ELC84               |
|        TEA6320                                        |           |               |
|78L09                                    LM613         |           |      VIA      |
|            MONO/STEREO                                |-----------|      VT83C461 |
|TDA1554           SELFTEST                                                         |
|                                                  DS1232      ACTEL_A1010B         |
|           HDD_PWR                                                                 |
| JSPKR |--|           JAMMA            |--| JSYNC  JVUPDN   XTRACOIN1  JGUN1  JGUN2|
|-------|  |----------------------------|  |----------------------------------------|
Notes:
      JGUN1 / JGUN2 - Connector for Guns
      VIA VT83C461 - IDE Controller
      JSPKR - Speaker Output Connector (for use with Stereo jumper)

****************************************************************************

    Memory map (TBA)

    ========================================================================
    MAIN CPU
    ========================================================================

    ------------------------------------------------------------
    000000-3FFFFF   R/W   xxxxxxxx xxxxxxxx   DRAM 0
    400000-7FFFFF   R/W   xxxxxxxx xxxxxxxx   DRAM 1
    800000-BFFFFF   R     xxxxxxxx xxxxxxxx   Graphic ROM bank
    C00000-DFFFFF   R     xxxxxxxx xxxxxxxx   Sound ROM bank
    F00000-F000FF   R/W   xxxxxxxx xxxxxxxx   Tom Internal Registers
    F00400-F005FF   R/W   xxxxxxxx xxxxxxxx   CLUT - color lookup table A
    F00600-F007FF   R/W   xxxxxxxx xxxxxxxx   CLUT - color lookup table B
    F00800-F00D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer A
    F01000-F0159F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer B
    F01800-F01D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer currently selected
    F02000-F021FF   R/W   xxxxxxxx xxxxxxxx   GPU control registers
    F02200-F022FF   R/W   xxxxxxxx xxxxxxxx   Blitter registers
    F03000-F03FFF   R/W   xxxxxxxx xxxxxxxx   Local GPU RAM
    F08800-F08D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - 32-bit access to line buffer A
    F09000-F0959F   R/W   xxxxxxxx xxxxxxxx   LBUF - 32-bit access to line buffer B
    F09800-F09D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - 32-bit access to line buffer currently selected
    F0B000-F0BFFF   R/W   xxxxxxxx xxxxxxxx   32-bit access to local GPU RAM
    F10000-F13FFF   R/W   xxxxxxxx xxxxxxxx   Jerry
    F14000-F17FFF   R/W   xxxxxxxx xxxxxxxx   Joysticks and GPIO0-5
    F18000-F1AFFF   R/W   xxxxxxxx xxxxxxxx   Jerry DSP
    F1B000-F1CFFF   R/W   xxxxxxxx xxxxxxxx   Local DSP RAM
    F1D000-F1DFFF   R     xxxxxxxx xxxxxxxx   Wavetable ROM
    ------------------------------------------------------------

  Jaguar console schematics include a ADC0844 at U16, selected by GPIOL5. This IC
  may or may not be populated.

  Jaguar System Notes:

    Protection Check

    At power on, a checksum is performed on the cart to ensure it has been
    certified by Atari. The actual checksum calculation is performed by the GPU,
    the result being left in GPU RAM at address f03000. The GPU is instructed to
    do the calculation when the bios sends a 1 to f02114 while it is in the
    initialisation stage. The bios then loops, waiting for the GPU to finish the
    calculation. When it does, it sets bit 15 of f02114 high. The bios then does
    the compare of the checksum. The checksum algorithm is unknown, but the
    final result must be 03d0dead. The bios checks for this particular result,
    and if found, the cart is allowed to start. Otherwise, the background turns
    red, and the console freezes.


    Jaguar Logo

    A real Jaguar will show the red Jaguar logo, the falling white Atari letters,
    and the turning jaguar's head, accompanied by the sound of a flushing toilet.
    The cart will then start. All Jaguar emulators (including this one) skip the
    logo with the appropriate memory hack. The cart can also instruct the logo
    be skipped by placing non-zero at location 800408. We do the same thing when
    the cart is loaded (see the DEVICE_IMAGE_LOAD section below).


    Start Address

    The start address of a cart may be found at 800404. It is normally 802000.

***************************************************************************/


#include "emu.h"
#include "jaguar.h"

#include "bus/ata/hdd.h"
#include "bus/generic/carts.h"
#include "cpu/jaguar/jaguar.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "cpu/mips/mips1.h"
#include "imagedev/cdromimg.h"
#include "imagedev/snapquik.h"
#include "machine/eepromser.h"
#include "machine/vt83c461.h"
#include "machine/watchdog.h"
#include "sound/cdda.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "cdrom.h"

#define COJAG_CLOCK         XTAL(52'000'000)
#define R3000_CLOCK         XTAL(40'000'000)
#define M68K_CLOCK          XTAL(50'000'000)


/*************************************
 *
 *  Local variables
 *
 *************************************/

void cojag_devices(device_slot_interface &device)
{
	device.option_add("hdd", IDE_HARDDISK);
}


/*************************************
 *
 *  Machine init
 *
 *************************************/

void jaguar_state::machine_start()
{
	/* configure banks for gfx/sound ROMs */
	if (m_romboard_region != nullptr)
	{
		uint8_t *romboard = m_romboard_region->base();

		/* graphics banks */
		if (m_maingfxbank.found())
		{
			m_maingfxbank->configure_entries(0, 2, romboard + 0x800000, 0x400000);
		}
		m_gpugfxbank->configure_entries(0, 2, romboard + 0x800000, 0x400000);

		/* sound banks */
		m_mainsndbank->configure_entries(0, 8, romboard + 0x000000, 0x200000);
		m_dspsndbank->configure_entries(0, 8, romboard + 0x000000, 0x200000);
	}
}

void jaguar_state::machine_reset()
{
	m_protection_check = 0;

	/* 68020 only: copy the interrupt vectors into RAM */
	if (!m_is_r3000)
	{
		if (m_rom_base.found())
		{
			for (offs_t addr = 0; addr < 0x400; addr += 4)    // do not increase, or Doom breaks
				m_shared_ram[addr/4] = m_rom_base[addr/2] << 16 | m_rom_base[addr/2+1];
		}
		else
			std::copy_n(reinterpret_cast<uint32_t *>(memregion("maincpu")->base()), 0x100, &m_shared_ram[0]);
	}

	/* reset banks for gfx/sound ROMs */
	if (m_romboard_region != nullptr)
	{
		/* graphics banks */
		if (m_maingfxbank.found())
		{
			m_maingfxbank->set_entry(0);
		}
		m_gpugfxbank->set_entry(0);

		/* sound banks */
		m_mainsndbank->set_entry(0);
		m_dspsndbank->set_entry(0);
	}

	/* clear any spinuntil stuff */
	gpu_resume();
	dsp_resume();

	/* halt the CPUs */
	m_gpu->go_w(false);
	m_dsp->go_w(false);

	/* set blitter idle flag */
	m_blitter_status = 1;
	m_joystick_data = 0xffffffff;
	m_eeprom_bit_count = 0;

	if ((m_using_cart) && (m_config_io->read() & 2))
	{
		m_cart_base[0x102] = 1;
		m_using_cart = false;
	}
}

void jaguarcd_state::machine_reset()
{
	jaguar_state::machine_reset();

	m_shared_ram[0x4/4] = 0x00802000; /* hack until I understand */

	m_butch_cmd_index = 0;
	m_butch_cmd_size = 1;
}


/********************************************************************
*
*  EEPROM
*  ======
*
*   The EEPROM is accessed by a serial protocol using the registers
*   0xF14000 (read data), F14800 (increment clock, write data), F15000 (reset for next word)
*
********************************************************************/
/*
emu_file jaguar_state::*jaguar_nvram_fopen( uint32_t openflags)
{
    device_image_interface *image = dynamic_cast<device_image_interface *>(machine().device("cart"));
    osd_file::error filerr;
    emu_file *file;
    if (image->exists())
    {
        std::string fname(machine().system().name, PATH_SEPARATOR, image->basename_noext(), ".nv");
        filerr = mame_fopen( SEARCHPATH_NVRAM, fname, openflags, &file);
        return (filerr == osd_file::error::NONE) ? file : nullptr;
    }
    else
        return nullptr;
}

void jaguar_state::jaguar_nvram_load()
{
    emu_file *nvram_file = nullptr;
    device_t *device;

    for (device = machine().m_devicelist.first(); device != nullptr; device = device->next())
    {
        device_nvram_func nvram = (device_nvram_func)device->get_config_fct(DEVINFO_FCT_NVRAM);
        if (nvram != nullptr)
        {
            if (nvram_file == nullptr)
                nvram_file = jaguar_nvram_fopen(machine, OPEN_FLAG_READ);
            (*nvram)(device, nvram_file, 0);
        }
    }
    if (nvram_file != nullptr)
        mame_fclose(nvram_file);
}


void jaguar_state::jaguar_nvram_save()
{
    emu_file *nvram_file = nullptr;
    device_t *device;

    for (device = machine().m_devicelist.first(); device != nullptr; device = device->next())
    {
        device_nvram_func nvram = (device_nvram_func)device->get_config_fct(DEVINFO_FCT_NVRAM);
        if (nvram != nullptr)
        {
            if (nvram_file == nullptr)
                nvram_file = jaguar_nvram_fopen(machine, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
            // check nvram_file to avoid crash when no image is mounted or cannot be created
            if (nvram_file)
                (*nvram)(device, nvram_file, 1);
        }
    }

    if (nvram_file != nullptr)
        mame_fclose(nvram_file);
}

static NVRAM_HANDLER( jaguar )
{
    if (read_or_write)  {
        jaguar_nvram_save(machine);
    }
    else
    {
        if (file)
            jaguar_nvram_load(machine);
    }
}
*/
void jaguar_state::eeprom_w(uint32_t data)
{
	m_eeprom_bit_count++;
	if (m_eeprom_bit_count != 9)        /* kill extra bit at end of address */
	{
		m_eeprom->di_write(data >> 31);
		m_eeprom->clk_write(0);
		m_eeprom->clk_write(1);
	}
}

uint32_t jaguar_state::eeprom_clk()
{
	if (!machine().side_effects_disabled())
	{
		m_eeprom->clk_write(0);
		m_eeprom->clk_write(1); /* get next bit when reading */
	}
	return 0;
}

uint32_t jaguar_state::eeprom_cs()
{
	if (!machine().side_effects_disabled())
	{
		m_eeprom->cs_write(CLEAR_LINE);   /* must do at end of an operation */
		m_eeprom->cs_write(ASSERT_LINE);        /* enable chip for next operation */
		m_eeprom->di_write(1);           /* write a start bit */
		m_eeprom->clk_write(0);
		m_eeprom->clk_write(1);
		m_eeprom_bit_count = 0;
	}
	return 0;
}



/*************************************
 *
 *  Misc. control bits
 *
 *************************************/

uint32_t jaguar_state::misc_control_r()
{
	/*  D7    = board reset (low)
	    D6    = audio must & reset (high)
	    D5    = volume control data (invert on write)
	    D4    = volume control clock
	    D3-D1 = audio bank 2-0
	    D0    = shared memory select (0=XBUS) */

	return m_misc_control_data ^ 0x20;
}


void jaguar_state::misc_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("%s:misc_control_w(%02X)\n", machine().describe_context(), data);

	/*  D7    = board reset (low)
	    D6    = audio must & reset (high)
	    D5    = volume control data (invert on write)
	    D4    = volume control clock
	    D3-D1 = audio bank 2-0
	    D0    = shared memory select (0=XBUS) */

	/* handle resetting the DSPs */
	if (!(data & 0x80))
	{
		/* clear any spinuntil stuff */
		gpu_resume();
		dsp_resume();

		/* halt the CPUs */
		m_gpu->go_w(false);
		m_dsp->go_w(false);
	}

	/* adjust banking */
	if (m_romboard_region != nullptr)
	{
		m_mainsndbank->set_entry((data >> 1) & 7);
		m_dspsndbank->set_entry((data >> 1) & 7);
	}

	COMBINE_DATA(&m_misc_control_data);
}


/*************************************
 *
 *  32-bit access to the GPU
 *
 *************************************/

uint32_t jaguar_state::gpuctrl_r(offs_t offset, uint32_t mem_mask)
{
	return m_gpu->iobus_r(offset, mem_mask);
}

void jaguar_state::gpuctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_gpu->iobus_w(offset, data, mem_mask);
}


/*************************************
 *
 *  32-bit access to the DSP
 *
 *************************************/

uint32_t jaguar_state::dspctrl_r(offs_t offset, uint32_t mem_mask)
{
	return m_dsp->iobus_r(offset, mem_mask);
}

void jaguar_state::dspctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_dsp->iobus_w(offset, data, mem_mask);
}


/*************************************
 *
 *  Input ports
 *
 *  Information from "The Jaguar Underground Documentation"
 *  by Klaus and Nat!
 *
 *************************************/

uint32_t jaguar_state::joystick_r()
{
	uint16_t joystick_result = 0xfffe;
	uint16_t joybuts_result = 0xffef;

	/*
	 *   16        12        8         4         0
	 *   +---------+---------+---------^---------+
	 *   |  pad 1  |  pad 0  |      unused       |
	 *   +---------+---------+-------------------+
	 *     15...12   11...8          7...0
	 *
	 *   Reading this register gives you the output of the selected columns
	 *   of the pads.
	 *   The buttons pressed will appear as cleared bits.
	 *   See the description of the column addressing to map the bits
	 *   to the buttons.
	 */

	for (int i = 0; i < 8; i++)
	{
		if ((m_joystick_data & (0x10000 << i)) == 0)
		{
			joystick_result &= m_joy[i]->read();
			joybuts_result &= m_buttons[i]->read();
		}
	}

	joystick_result |= m_eeprom->do_read();
	joybuts_result |= (m_config_io->read() & 0x10);

	return (joystick_result << 16) | joybuts_result;
}

void jaguar_state::joystick_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/*
	 *   16        12         8         4         0
	 *   +-+-------^------+--+---------+---------+
	 *   |r|    unused    |mu|  col 1  |  col 0  |
	 *   +-+--------------+--+---------+---------+
	 *    15                8   7...4     3...0
	 *
	 *   col 0:   column control of joypad 0
	 *
	 *      Here you select which column of the joypad to poll.
	 *      The columns are:
	 *
	 *                Joystick       Joybut
	 *      col_bit|11 10  9  8     1    0
	 *      -------+--+--+--+--    ---+------
	 *         0   | R  L  D  U     A  PAUSE       (RLDU = Joypad directions)
	 *         1   | 1  4  7  *     B
	 *         2   | 2  5  8  0     C
	 *         3   | 3  6  9  #   OPTION
	 *
	 *      You select a column my clearing the appropriate bit and setting
	 *      all the other "column" bits.
	 *
	 *
	 *   col1:    column control of joypad 1
	 *
	 *      This is pretty much the same as for joypad EXCEPT that the
	 *      column addressing is reversed (strange!!)
	 *
	 *                Joystick      Joybut
	 *      col_bit|15 14 13 12     3    2
	 *      -------+--+--+--+--    ---+------
	 *         4   | 3  6  9  #   OPTION
	 *         5   | 2  5  8  0     C
	 *         6   | 1  4  7  *     B
	 *         7   | R  L  D  U     A  PAUSE     (RLDU = Joypad directions)
	 *
	 *   mute (mu):   sound control
	 *
	 *      You can turn off the sound by clearing this bit.
	 *
	 *   read enable (r):
	 *
	 *      Set this bit to read from the joysticks, clear it to write
	 *      to them.
	 */
	COMBINE_DATA(&m_joystick_data);
}


/*************************************
 *
 *  Output ports
 *
 *************************************/

void jaguar_state::latch_w(uint32_t data)
{
	logerror("%08X:latch_w(%X)\n", m_maincpu->pcbase(), data);

	/* adjust banking */
	if (m_romboard_region != nullptr)
	{
		if (m_maingfxbank.found())
		{
			m_maingfxbank->set_entry(data & 1);
		}
		m_gpugfxbank->set_entry(data & 1);
	}
}



/*************************************
 *
 *  EEPROM access
 *
 *************************************/

uint32_t jaguar_state::eeprom_data_r(offs_t offset)
{
	if (m_is_r3000)
		return m_nvram[offset] | 0xffffff00;
	else
		return m_nvram[offset] | 0x00ffffff;
}


void jaguar_state::eeprom_enable_w(uint32_t data)
{
	m_eeprom_enable = true;
}


void jaguar_state::eeprom_data_w(offs_t offset, uint32_t data)
{
//  if (m_eeprom_enable)
	{
		if (m_is_r3000)
			m_nvram[offset] = data & 0x000000ff;
		else
			m_nvram[offset] = data & 0xff000000;
	}
//  else
//      logerror("%s:error writing to disabled EEPROM\n", machine().describe_context());
	m_eeprom_enable = false;
}



/*************************************
 *
 *  GPU synchronization & speedup
 *
 *************************************/

/*
    Explanation:

    The GPU generally sits in a tight loop waiting for the main CPU to store
    a jump address into a specific memory location. This speedup is designed
    to catch that loop, which looks like this:

        load    (r28),r21
        jump    (r21)
        nop

    When nothing is pending, the GPU keeps the address of the load instruction
    at (r28) so that it loops back on itself. When the main CPU wants to execute
    a command, it stores an alternate address to (r28).

    Even if we don't optimize this case, we do need to detect when a command
    is written to the GPU in order to improve synchronization until the GPU
    has finished. To do this, we start a temporary high frequency timer and
    run it until we get back to the spin loop.
*/

void jaguar_state::gpu_jump_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/* update the data in memory */
	COMBINE_DATA(m_gpu_jump_address);
	logerror("%08X:GPU jump address = %08X\n", m_gpu->pcbase(), *m_gpu_jump_address);

	/* if the GPU is suspended, release it now */
	gpu_resume();

	/* start the sync timer going, and note that there is a command pending */
	m_gpu_sync_timer->adjust(attotime::zero);
	m_gpu_command_pending = true;
}


uint32_t jaguar_state::gpu_jump_r()
{
	/* if the current GPU command is just pointing back to the spin loop, and */
	/* we're reading it from the spin loop, we can optimize */
	if (*m_gpu_jump_address == m_gpu_spin_pc && m_gpu->pcbase() == m_gpu_spin_pc)
	{
#if ENABLE_SPEEDUP_HACKS
		/* spin if we're allowed */
		if (m_hacks_enabled) gpu_suspend();
#endif

		/* no command is pending */
		m_gpu_command_pending = false;
	}

	/* return the current value */
	return *m_gpu_jump_address;
}



/*************************************
 *
 *  Main CPU speedup (R3000 games)
 *
 *************************************/

/*
    Explanation:

    Instead of sitting in a tight loop, the CPU will run the random number
    generator over and over while waiting for an interrupt. In order to catch
    that, we snoop the memory location it is polling, and see if it is read
    at least 5 times in a row, each time less than 200 cycles apart. If so,
    we assume it is spinning. Also, by waiting for 5 iterations, we let it
    crank through some random numbers, just not several thousand every frame.
*/

uint32_t jaguar_state::cojagr3k_main_speedup_r()
{
	uint64_t curcycles = m_maincpu->total_cycles();

	/* if it's been less than main_speedup_max_cycles cycles since the last time */
	if (curcycles - m_main_speedup_last_cycles < m_main_speedup_max_cycles)
	{
		/* increment the count; if we hit 5, we can spin until an interrupt comes */
		if (m_main_speedup_hits++ > 5)
		{
			m_maincpu->spin_until_interrupt();
			m_main_speedup_hits = 0;
		}
	}

	/* if it's been more than main_speedup_max_cycles cycles, reset our count */
	else
		m_main_speedup_hits = 0;

	/* remember the last cycle count */
	m_main_speedup_last_cycles = curcycles;

	/* return the real value */
	return *m_main_speedup;
}



/*************************************
 *
 *  Additional main CPU speedup
 *  (Freeze only)
 *
 *************************************/

/*
    Explanation:

    The main CPU hands data off to the GPU to process. But rather than running
    in parallel, the main CPU just sits and waits for the result. This speedup
    makes sure we don't waste time emulating that spin loop.
*/

uint32_t jaguar_state::main_gpu_wait_r()
{
	if (m_gpu_command_pending)
		m_maincpu->spin_until_interrupt();
	return *m_main_gpu_wait;
}



/*************************************
 *
 *  Main CPU speedup (Area 51)
 *
 *************************************/

/*
    Explanation:

    Very similar to the R3000 code, except we need to verify that the value in
    *main_speedup is actually 0.
*/

void jaguar_state::area51_main_speedup_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint64_t curcycles = m_maincpu->total_cycles();

	/* store the data */
	COMBINE_DATA(m_main_speedup);

	/* if it's been less than 400 cycles since the last time */
	if (*m_main_speedup == 0 && curcycles - m_main_speedup_last_cycles < 400)
	{
		/* increment the count; if we hit 5, we can spin until an interrupt comes */
		if (m_main_speedup_hits++ > 5)
		{
			m_maincpu->spin_until_interrupt();
			m_main_speedup_hits = 0;
		}
	}

	/* if it's been more than 400 cycles, reset our count */
	else
		m_main_speedup_hits = 0;

	/* remember the last cycle count */
	m_main_speedup_last_cycles = curcycles;
}


/*
    Explanation:

    The Area 51/Maximum Force duo writes to a non-aligned address, so our check
    against 0 must handle that explicitly.
*/

void jaguar_state::area51mx_main_speedup_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint64_t curcycles = m_maincpu->total_cycles();

	/* store the data */
	COMBINE_DATA(&m_main_speedup[offset]);

	/* if it's been less than 450 cycles since the last time */
	if (((m_main_speedup[0] << 16) | (m_main_speedup[1] >> 16)) == 0 && curcycles - m_main_speedup_last_cycles < 450)
	{
		/* increment the count; if we hit 5, we can spin until an interrupt comes */
		if (m_main_speedup_hits++ > 10)
		{
			m_maincpu->spin_until_interrupt();
			m_main_speedup_hits = 0;
		}
	}

	/* if it's been more than 450 cycles, reset our count */
	else
		m_main_speedup_hits = 0;

	/* remember the last cycle count */
	m_main_speedup_last_cycles = curcycles;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

// surely these should be 16-bit natively if the standard Jaguar is driven by a plain 68k?
// all these trampolines are not good for performance ;-)

uint16_t jaguar_state::gpuctrl_r16(offs_t offset, uint16_t mem_mask){ if (!(offset&1)) { return gpuctrl_r(offset>>1, mem_mask<<16) >> 16;  } else { return gpuctrl_r(offset>>1, mem_mask); } }
void jaguar_state::gpuctrl_w16(offs_t offset, uint16_t data, uint16_t mem_mask){ if (!(offset&1)) { gpuctrl_w(offset>>1, data << 16, mem_mask << 16); } else { gpuctrl_w(offset>>1, data, mem_mask); } }
uint16_t jaguar_state::blitter_r16(offs_t offset, uint16_t mem_mask){ if (!(offset&1)) { return blitter_r(offset>>1, mem_mask<<16) >> 16;  } else { return blitter_r(offset>>1, mem_mask); } }
void jaguar_state::blitter_w16(offs_t offset, uint16_t data, uint16_t mem_mask){ if (!(offset&1)) { blitter_w(offset>>1, data << 16, mem_mask << 16); } else { blitter_w(offset>>1, data, mem_mask); } }
uint16_t jaguar_state::serial_r16(offs_t offset){ if (!(offset&1)) { return serial_r(offset>>1) >> 16;  } else { return serial_r(offset>>1); } }
void jaguar_state::serial_w16(offs_t offset, uint16_t data){ if (!(offset&1)) { serial_w(offset>>1, data << 16); } else { serial_w(offset>>1, data); } }
uint16_t jaguar_state::dspctrl_r16(offs_t offset, uint16_t mem_mask){ if (!(offset&1)) { return dspctrl_r(offset>>1, mem_mask<<16) >> 16;  } else { return dspctrl_r(offset>>1, mem_mask); } }
void jaguar_state::dspctrl_w16(offs_t offset, uint16_t data, uint16_t mem_mask){ if (!(offset&1)) { dspctrl_w(offset>>1, data << 16, mem_mask << 16); } else { dspctrl_w(offset>>1, data, mem_mask); } }
uint16_t jaguar_state::eeprom_cs16(offs_t offset){ if (!(offset&1)) { return eeprom_cs() >> 16;  } else { return eeprom_cs(); } }
uint16_t jaguar_state::eeprom_clk16(offs_t offset){ if (!(offset&1)) { return eeprom_clk() >> 16;  } else { return eeprom_clk(); } }
void jaguar_state::eeprom_w16(offs_t offset, uint16_t data){ if (!(offset&1)) { eeprom_w(data << 16); } else { eeprom_w(data); } }
uint16_t jaguar_state::joystick_r16(offs_t offset){ if (!(offset&1)) { return joystick_r() >> 16;  } else { return joystick_r(); } }
void jaguar_state::joystick_w16(offs_t offset, uint16_t data, uint16_t mem_mask){ if (!(offset&1)) { joystick_w(offset>>1, data << 16, mem_mask << 16); } else { joystick_w(offset>>1, data, mem_mask); } }

uint32_t jaguar_state::shared_ram_r(offs_t offset){ return m_shared_ram[offset]; }
void jaguar_state::shared_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask){ COMBINE_DATA(&m_shared_ram[offset]); }
uint32_t jaguar_state::rom_base_r(offs_t offset){ return m_rom_base[offset*2+1] << 16 | m_rom_base[offset*2]; }
uint32_t jaguar_state::wave_rom_r(offs_t offset){ return m_wave_rom[offset*2+1] << 16 | m_wave_rom[offset*2]; }
uint32_t jaguarcd_state::cd_bios_r(offs_t offset){ return m_cd_bios[offset*2+1] << 16 | m_cd_bios[offset*2]; }
uint32_t jaguar_state::dsp_ram_r(offs_t offset){ return m_dsp_ram[offset]; }
void jaguar_state::dsp_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask){ COMBINE_DATA(&m_dsp_ram[offset]); }
uint32_t jaguar_state::gpu_clut_r(offs_t offset){ return m_gpu_clut[offset]; }
void jaguar_state::gpu_clut_w(offs_t offset, uint32_t data, uint32_t mem_mask){ COMBINE_DATA(&m_gpu_clut[offset]); }
uint32_t jaguar_state::gpu_ram_r(offs_t offset){ return m_gpu_ram[offset]; }
void jaguar_state::gpu_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask){ COMBINE_DATA(&m_gpu_ram[offset]); }

uint16_t jaguar_state::shared_ram_r16(offs_t offset){ if (!(offset&1)) { return shared_ram_r(offset>>1) >> 16;  } else { return shared_ram_r(offset>>1); } }
void jaguar_state::shared_ram_w16(offs_t offset, uint16_t data, uint16_t mem_mask){ if (!(offset&1)) { shared_ram_w(offset>>1, data << 16, mem_mask << 16); } else { shared_ram_w(offset>>1, data, mem_mask); } }
uint16_t jaguar_state::cart_base_r16(offs_t offset){ if (!(offset&1)) { return m_cart_base[offset>>1] >> 16;  } else { return m_cart_base[offset>>1] & 0xffff; } }
uint16_t jaguar_state::dsp_ram_r16(offs_t offset){ if (!(offset&1)) { return dsp_ram_r(offset>>1) >> 16;  } else { return dsp_ram_r(offset>>1); } }
void jaguar_state::dsp_ram_w16(offs_t offset, uint16_t data, uint16_t mem_mask){ if (!(offset&1)) { dsp_ram_w(offset>>1, data << 16, mem_mask << 16); } else { dsp_ram_w(offset>>1, data, mem_mask); } }
uint16_t jaguar_state::gpu_clut_r16(offs_t offset){ if (!(offset&1)) { return gpu_clut_r(offset>>1) >> 16;  } else { return gpu_clut_r(offset>>1); } }
void jaguar_state::gpu_clut_w16(offs_t offset, uint16_t data, uint16_t mem_mask){ if (!(offset&1)) { gpu_clut_w(offset>>1, data << 16, mem_mask << 16); } else { gpu_clut_w(offset>>1, data, mem_mask); } }
uint16_t jaguar_state::gpu_ram_r16(offs_t offset){ if (!(offset&1)) { return gpu_ram_r(offset>>1) >> 16;  } else { return gpu_ram_r(offset>>1); } }
void jaguar_state::gpu_ram_w16(offs_t offset, uint16_t data, uint16_t mem_mask){ if (!(offset&1)) { gpu_ram_w(offset>>1, data << 16, mem_mask << 16); } else { gpu_ram_w(offset>>1, data, mem_mask); } }

void jaguar_state::console_base_map(address_map &map)
{
	map(0x000000, 0x1fffff).mirror(0x200000).rw(FUNC(jaguar_state::shared_ram_r16), FUNC(jaguar_state::shared_ram_w16));
	map(0xe00000, 0xe1ffff).rom().region("mainrom", 0);
	map(0xf00000, 0xf003ff).rw(FUNC(jaguar_state::tom_regs_r), FUNC(jaguar_state::tom_regs_w)); // might be reversed endian of the others..
	map(0xf00400, 0xf005ff).mirror(0x000200).rw(FUNC(jaguar_state::gpu_clut_r16), FUNC(jaguar_state::gpu_clut_w16));
	map(0xf02100, 0xf021ff).rw(FUNC(jaguar_state::gpuctrl_r16), FUNC(jaguar_state::gpuctrl_w16));
	map(0xf02200, 0xf022ff).mirror(0x008000).rw(FUNC(jaguar_state::blitter_r16), FUNC(jaguar_state::blitter_w16));
	map(0xf03000, 0xf03fff).mirror(0x008000).rw(FUNC(jaguar_state::gpu_ram_r16), FUNC(jaguar_state::gpu_ram_w16));
	map(0xf10000, 0xf103ff).rw(FUNC(jaguar_state::jerry_regs_r), FUNC(jaguar_state::jerry_regs_w)); // might be reversed endian of the others..
	map(0xf14000, 0xf14003).rw(FUNC(jaguar_state::joystick_r16), FUNC(jaguar_state::joystick_w16));
	map(0xf14800, 0xf14803).rw(FUNC(jaguar_state::eeprom_clk16), FUNC(jaguar_state::eeprom_w16));  // GPIO0
	map(0xf15000, 0xf15003).r(FUNC(jaguar_state::eeprom_cs16));               // GPIO1
	map(0xf1a100, 0xf1a13f).rw(FUNC(jaguar_state::dspctrl_r16), FUNC(jaguar_state::dspctrl_w16));
	map(0xf1a140, 0xf1a17f).rw(FUNC(jaguar_state::serial_r16), FUNC(jaguar_state::serial_w16));
	map(0xf1b000, 0xf1cfff).rw(FUNC(jaguar_state::dsp_ram_r16), FUNC(jaguar_state::dsp_ram_w16));
	map(0xf1d000, 0xf1dfff).rom().region("waverom", 0);
}

void jaguar_state::jaguar_map(address_map &map)
{
	console_base_map(map);
	map(0x800000, 0xdfffff).r(FUNC(jaguar_state::cart_base_r16));
}

void jaguar_state::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff5, 0xfffff5).lr8([] () -> u8 { return 0x40; }, "level2");
}

/*
CD-Rom emulation, chip codename Butch (the HW engineer was definitely obsessed with T&J somehow ...)
TODO: this needs to be device-ized, of course ...

[0x00]: irq register
(R)
-x-- ---- ---- ---- CD uncorrectable data error pending
--x- ---- ---- ---- Response from CD drive pending
---x ---- ---- ---- Command to CD drive pending
---- x--- ---- ---- Subcode data pending
---- -x-- ---- ---- Frame pending
---- --x- ---- ---- CD data FIFO half-full flag pending
(W)
---- ---- -x-- ---- CIRC failure irq
---- ---- --x- ---- CD module command RX buffer full irq
---- ---- ---x ---- CD module command TX buffer empty irq
---- ---- ---- x--- Enable pre-set subcode time-match found irq
---- ---- ---- -x-- Enable CD subcode frame-time irq
---- ---- ---- --x- Enable CD data FIFO half full irq
---- ---- ---- ---x set to enable irq
[0x04]: DSA control register
[0x0a]: DSA TX/RX data (sends commands with this)
    0x01 Play Title (?)
    0x02 Stop
    0x03 Read TOC
    0x04 Pause
    0x05 Unpause
    0x09 Get Title Len
    0x0a Open Tray
    0x0b Close Tray
    0x0d Get Comp Time
    0x10 Goto ABS Min
    0x11 Goto ABS Sec
    0x12 Goto ABS Frame
    0x14 Read Long TOC
    0x15 Set Mode
    0x16 Get Error
    0x17 Clear Error
    0x18 Spin Up
    0x20 Play AB Min
    0x21 Play AB Sec
    0x22 Play AB Frame
    0x23 Stop AB Min
    0x24 Stop AB Sec
    0x25 Stop AB Frame
    0x26 AB Release
    0x50 Get Disc Status
    0x51 Set Volume
    0x54 Get Maxsession
    0x70 Set DAC mode (?)
    0xa0-0xaf User Define (???)
    0xf0 Service
    0xf1 Sledge
    0xf2 Focus
    0xf3 Turntable
    0xf4 Radial

[0x10]: I2S bus control register
[0x14]: CD subcode control register
[0x18]: Subcode data register A
[0x1C]: Subcode data register B
[0x20]: Subcode time and compare enable
[0x24]: I2S FIFO data
[0x28]: I2S FIFO data (old)
[0x2c]: ? (used at start-up)

*/

uint16_t jaguarcd_state::butch_regs_r16(offs_t offset){ if (!(offset&1)) { return butch_regs_r(offset>>1) >> 16;  } else { return butch_regs_r(offset>>1); } }
void jaguarcd_state::butch_regs_w16(offs_t offset, uint16_t data, uint16_t mem_mask){ if (!(offset&1)) { butch_regs_w(offset>>1, data << 16, mem_mask << 16); } else { butch_regs_w(offset>>1, data, mem_mask); } }

uint32_t jaguarcd_state::butch_regs_r(offs_t offset)
{
	switch(offset*4)
	{
		case 8: //DS DATA
			//m_butch_regs[0] &= ~0x2000;
			return m_butch_cmd_response[(m_butch_cmd_index++) % m_butch_cmd_size];
	}

	return m_butch_regs[offset];
}

void jaguarcd_state::butch_regs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_butch_regs[offset]);

	switch(offset*4)
	{
		case 8: //DS DATA
			switch((m_butch_regs[offset] & 0xff00) >> 8)
			{
				case 0x03: // Read TOC
				{
					if(!m_cdrom->exists()) // No disc
					{
						m_butch_cmd_response[0] = 0x400;
						m_butch_regs[0] |= 0x2000;
						m_butch_cmd_index = 0;
						m_butch_cmd_size = 1;
						return;
					}
					if(m_butch_regs[offset] & 0xff) // Multi Session CD, TODO
					{
						m_butch_cmd_response[0] = 0x0029; // illegal value
						m_butch_regs[0] |= 0x2000;
						m_butch_cmd_index = 0;
						m_butch_cmd_size = 1;
						return;
					}

					uint32_t msf = m_cdrom->get_track_start(0) + 150;

					/* first track number */
					m_butch_cmd_response[0] = 0x2000 | 1;
					/* last track number */
					m_butch_cmd_response[1] = 0x2100 | m_cdrom->get_last_track();

					/* start of first track minutes */
					m_butch_cmd_response[2] = 0x2200 | ((msf / 60) / 60);
					/* start of first track seconds */
					m_butch_cmd_response[3] = 0x2300 | (msf / 60) % 60;
					/* start of first track frame */
					m_butch_cmd_response[4] = 0x2400 | (msf % 75);
					m_butch_regs[0] |= 0x2000;
					m_butch_cmd_index = 0;
					m_butch_cmd_size = 5;
					break;
				}

				case 0x14: // Read Long TOC
				{
					if(!m_cdrom->exists()) // No disc
					{
						m_butch_cmd_response[0] = 0x400;
						m_butch_regs[0] |= 0x2000;
						m_butch_cmd_index = 0;
						m_butch_cmd_size = 1;
						return;
					}

					int ntrks = m_cdrom->get_last_track();

					for(int i=0;i<ntrks;i++)
					{
						uint32_t msf = m_cdrom->get_track_start(i) + 150;

						/* track number */
						m_butch_cmd_response[i*5+0] = 0x6000 | (i+1);
						/* attributes (?) */
						m_butch_cmd_response[i*5+1] = 0x6100 | 0x00;

						/* start of track minutes */
						m_butch_cmd_response[i*5+2] = 0x6200 | ((msf / 60) / 60);
						/* start of track seconds */
						m_butch_cmd_response[i*5+3] = 0x6300 | (msf / 60) % 60;
						/* start of track frame */
						m_butch_cmd_response[i*5+4] = 0x6400 | (msf % 75);
					}
					m_butch_regs[0] |= 0x2000;
					m_butch_cmd_index = 0;
					m_butch_cmd_size = 5*ntrks;
					break;
				}

				case 0x15: // Set Mode
					m_butch_regs[0] |= 0x2000;
					m_butch_cmd_response[0] = 0x1700 | (m_butch_regs[offset] & 0xff);
					m_butch_cmd_index = 0;
					m_butch_cmd_size = 1;
					break;

				case 0x70: // Set DAC Mode
					m_butch_regs[0] |= 0x2000;
					m_butch_cmd_response[0] = 0x7000 | (m_butch_regs[offset] & 0xff);
					m_butch_cmd_index = 0;
					m_butch_cmd_size = 1;
					break;

				default:
					logerror("%04x CMD\n", m_butch_regs[offset]);
					break;
			}
			break;
	}
}

void jaguarcd_state::jaguarcd_map(address_map &map)
{
	console_base_map(map);
	map(0x800000, 0x83ffff).rom().region("cdbios", 0);
	map(0xdfff00, 0xdfff3f).rw(FUNC(jaguarcd_state::butch_regs_r16), FUNC(jaguarcd_state::butch_regs_w16));
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void jaguar_state::r3000_map(address_map &map)
{
	map(0x04000000, 0x047fffff).ram().share("sharedram");
	map(0x04e00030, 0x04e0003f).rw(m_ide, FUNC(vt83c461_device::config_r), FUNC(vt83c461_device::config_w));
	map(0x04e001f0, 0x04e001f7).rw(m_ide, FUNC(vt83c461_device::cs0_r), FUNC(vt83c461_device::cs0_w));
	map(0x04e003f0, 0x04e003f7).rw(m_ide, FUNC(vt83c461_device::cs1_r), FUNC(vt83c461_device::cs1_w));
	map(0x04f00000, 0x04f003ff).rw(FUNC(jaguar_state::tom_regs_r), FUNC(jaguar_state::tom_regs_w));
	map(0x04f00400, 0x04f007ff).ram().share("gpuclut");
	map(0x04f02100, 0x04f021ff).rw(FUNC(jaguar_state::gpuctrl_r), FUNC(jaguar_state::gpuctrl_w));
	map(0x04f02200, 0x04f022ff).rw(FUNC(jaguar_state::blitter_r), FUNC(jaguar_state::blitter_w));
	map(0x04f03000, 0x04f03fff).mirror(0x00008000).ram().share("gpuram");
	map(0x04f10000, 0x04f103ff).rw(FUNC(jaguar_state::jerry_regs_r), FUNC(jaguar_state::jerry_regs_w));
	map(0x04f16000, 0x04f1600b).r(FUNC(jaguar_state::cojag_gun_input_r)); // GPIO2
	map(0x04f17000, 0x04f17003).lr16(NAME([this] () { return uint16_t(m_system->read()); })); // GPIO3
	map(0x04f17800, 0x04f17803).w(FUNC(jaguar_state::latch_w));          // GPIO4
	map(0x04f17c00, 0x04f17c03).portr("P1_P2");      // GPIO5
	map(0x04f1a100, 0x04f1a13f).rw(FUNC(jaguar_state::dspctrl_r), FUNC(jaguar_state::dspctrl_w));
	map(0x04f1a140, 0x04f1a17f).rw(FUNC(jaguar_state::serial_r), FUNC(jaguar_state::serial_w));
	map(0x04f1b000, 0x04f1cfff).ram().share("dspram");

	map(0x06000000, 0x06000003).rw(FUNC(jaguar_state::misc_control_r), FUNC(jaguar_state::misc_control_w));
	map(0x10000000, 0x1007ffff).ram().share("mainram");
	map(0x12000000, 0x120fffff).ram().share("mainram2");    // tested in self-test only?
	map(0x14000004, 0x14000007).w("watchdog", FUNC(watchdog_timer_device::reset32_w));
	map(0x16000000, 0x16000003).w(FUNC(jaguar_state::eeprom_enable_w));
	map(0x18000000, 0x18001fff).rw(FUNC(jaguar_state::eeprom_data_r), FUNC(jaguar_state::eeprom_data_w)).share("nvram");
	map(0x1fc00000, 0x1fdfffff).rom().region("maincpu", 0);
}

void jaguar_state::r3000_rom_map(address_map &map)
{
	r3000_map(map);
	map(0x04800000, 0x04bfffff).bankr("maingfxbank");
	map(0x04c00000, 0x04dfffff).bankr("mainsndbank");
}


void jaguar_state::m68020_map(address_map &map)
{
	map(0x000000, 0x7fffff).ram().share("sharedram");
	map(0x800000, 0x9fffff).rom().region("maincpu", 0);
	map(0xa00000, 0xa1ffff).ram().share("mainram");
	map(0xa20000, 0xa21fff).rw(FUNC(jaguar_state::eeprom_data_r), FUNC(jaguar_state::eeprom_data_w)).share("nvram");
	map(0xa30000, 0xa30003).w("watchdog", FUNC(watchdog_timer_device::reset32_w));
	map(0xa40000, 0xa40003).w(FUNC(jaguar_state::eeprom_enable_w));
	map(0xb70000, 0xb70003).rw(FUNC(jaguar_state::misc_control_r), FUNC(jaguar_state::misc_control_w));
//  map(0xc00000, 0xdfffff).bankr("mainsndbank");
	map(0xe00030, 0xe0003f).rw(m_ide, FUNC(vt83c461_device::config_r), FUNC(vt83c461_device::config_w));
	map(0xe001f0, 0xe001f7).rw(m_ide, FUNC(vt83c461_device::cs0_r), FUNC(vt83c461_device::cs0_w));
	map(0xe003f0, 0xe003f7).rw(m_ide, FUNC(vt83c461_device::cs1_r), FUNC(vt83c461_device::cs1_w));
	map(0xf00000, 0xf003ff).rw(FUNC(jaguar_state::tom_regs_r), FUNC(jaguar_state::tom_regs_w));
	map(0xf00400, 0xf007ff).ram().share("gpuclut");
	map(0xf02100, 0xf021ff).rw(FUNC(jaguar_state::gpuctrl_r), FUNC(jaguar_state::gpuctrl_w));
	map(0xf02200, 0xf022ff).rw(FUNC(jaguar_state::blitter_r), FUNC(jaguar_state::blitter_w));
	map(0xf03000, 0xf03fff).mirror(0x008000).ram().share("gpuram");
	map(0xf10000, 0xf103ff).rw(FUNC(jaguar_state::jerry_regs_r), FUNC(jaguar_state::jerry_regs_w));
	map(0xf16000, 0xf1600b).r(FUNC(jaguar_state::cojag_gun_input_r)); // GPIO2
	map(0xf17000, 0xf17003).lr16(NAME([this] () { return uint16_t(m_system->read()); })); // GPIO3
//  map(0xf17800, 0xf17803).w(FUNC(jaguar_state::(latch_w));          // GPIO4
	map(0xf17c00, 0xf17c03).portr("P1_P2");      // GPIO5
	map(0xf1a100, 0xf1a13f).rw(FUNC(jaguar_state::dspctrl_r), FUNC(jaguar_state::dspctrl_w));
	map(0xf1a140, 0xf1a17f).rw(FUNC(jaguar_state::serial_r), FUNC(jaguar_state::serial_w));
	map(0xf1b000, 0xf1cfff).ram().share("dspram");
}


/*************************************
 *
 *  GPU memory handlers
 *
 *************************************/

void jaguar_state::gpu_map(address_map &map)
{
	map(0x000000, 0x7fffff).ram().share("sharedram");
	map(0xe00030, 0xe0003f).rw(m_ide, FUNC(vt83c461_device::config_r), FUNC(vt83c461_device::config_w));
	map(0xe001f0, 0xe001f7).rw(m_ide, FUNC(vt83c461_device::cs0_r), FUNC(vt83c461_device::cs0_w));
	map(0xe003f0, 0xe003f7).rw(m_ide, FUNC(vt83c461_device::cs1_r), FUNC(vt83c461_device::cs1_w));
	map(0xf00000, 0xf003ff).rw(FUNC(jaguar_state::tom_regs_r), FUNC(jaguar_state::tom_regs_w));
	map(0xf00400, 0xf007ff).ram().share("gpuclut");
	map(0xf02100, 0xf021ff).rw(FUNC(jaguar_state::gpuctrl_r), FUNC(jaguar_state::gpuctrl_w));
	map(0xf02200, 0xf022ff).rw(FUNC(jaguar_state::blitter_r), FUNC(jaguar_state::blitter_w));
	map(0xf03000, 0xf03fff).ram().share("gpuram");
	map(0xf10000, 0xf103ff).rw(FUNC(jaguar_state::jerry_regs_r), FUNC(jaguar_state::jerry_regs_w));
}

void jaguar_state::gpu_rom_map(address_map &map)
{
	gpu_map(map);
	map(0x800000, 0xbfffff).bankr("gpugfxbank");
	map(0xc00000, 0xdfffff).bankr("dspsndbank");
}


/*************************************
 *
 *  DSP memory handlers
 *
 *************************************/

void jaguar_state::dsp_map(address_map &map)
{
	map(0x000000, 0x7fffff).ram().share("sharedram");
	map(0xf10000, 0xf103ff).rw(FUNC(jaguar_state::jerry_regs_r), FUNC(jaguar_state::jerry_regs_w));
	map(0xf1a100, 0xf1a13f).rw(FUNC(jaguar_state::dspctrl_r), FUNC(jaguar_state::dspctrl_w));
	map(0xf1a140, 0xf1a17f).rw(FUNC(jaguar_state::serial_r), FUNC(jaguar_state::serial_w));
	map(0xf1b000, 0xf1cfff).ram().share("dspram");
	map(0xf1d000, 0xf1dfff).r(FUNC(jaguar_state::wave_rom_r));
}

void jaguar_state::dsp_rom_map(address_map &map)
{
	dsp_map(map);
	map(0x800000, 0xbfffff).bankr("gpugfxbank");
	map(0xc00000, 0xdfffff).bankr("dspsndbank");
}

/* ToDo, these maps SHOULD be merged with the ones above */

void jaguar_state::console_base_gpu_map(address_map &map)
{
	map.global_mask(0xffffff);
	map(0x000000, 0x1fffff).ram().mirror(0x200000).share("sharedram");
	map(0xe00000, 0xe1ffff).r(FUNC(jaguar_state::rom_base_r));
	map(0xf00000, 0xf003ff).rw(FUNC(jaguar_state::tom_regs_r), FUNC(jaguar_state::tom_regs_w));
	map(0xf00400, 0xf005ff).mirror(0x000200).ram().share("gpuclut");
	map(0xf02100, 0xf021ff).rw(FUNC(jaguar_state::gpuctrl_r), FUNC(jaguar_state::gpuctrl_w));
	map(0xf02200, 0xf022ff).mirror(0x008000).rw(FUNC(jaguar_state::blitter_r), FUNC(jaguar_state::blitter_w));
	map(0xf03000, 0xf03fff).mirror(0x008000).ram().share("gpuram");
	map(0xf10000, 0xf103ff).rw(FUNC(jaguar_state::jerry_regs_r), FUNC(jaguar_state::jerry_regs_w));
	map(0xf14000, 0xf14003).rw(FUNC(jaguar_state::joystick_r), FUNC(jaguar_state::joystick_w));
	map(0xf1a100, 0xf1a13f).rw(FUNC(jaguar_state::dspctrl_r), FUNC(jaguar_state::dspctrl_w));
	map(0xf1a140, 0xf1a17f).rw(FUNC(jaguar_state::serial_r), FUNC(jaguar_state::serial_w));
	map(0xf1b000, 0xf1cfff).ram().share("dspram");
	map(0xf1d000, 0xf1dfff).r(FUNC(jaguar_state::wave_rom_r));
}

void jaguar_state::jag_gpu_dsp_map(address_map &map)
{
	console_base_gpu_map(map);
	map(0x800000, 0xdfffff).rom().region("cart", 0);
}

void jaguarcd_state::jagcd_gpu_dsp_map(address_map &map)
{
	console_base_gpu_map(map);
	map(0x800000, 0x83ffff).r(FUNC(jaguarcd_state::cd_bios_r));
	map(0xdfff00, 0xdfff3f).rw(FUNC(jaguarcd_state::butch_regs_r), FUNC(jaguarcd_state::butch_regs_w));
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

/* "SYSTEM" is read at 0x04f17000
    D23-20 = /SER-4-1
    D19-16 = COINR4-1
    D7     = /VSYNCNEQ
    D6     = /S-TEST
    D5     = /VOLUMEUP
    D4     = /VOLUMEDOWN
    D3-D0  = ACTC4-1
*/
static INPUT_PORTS_START( area51 )
	PORT_START("P1_P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0000fe00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )           // s-test
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM )  // vsyncneq
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("FAKE1_X")               /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 320.0/(320.0 - 7 -7), 0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("FAKE1_Y")               /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, (240.0 - 1)/240, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("FAKE2_X")               /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 320.0/(320.0 - 7 -7), 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("FAKE2_Y")               /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, (240.0 - 1)/240, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("IN3")           /* gun triggers */
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_CUSTOM )  // gun data valid
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_CUSTOM )  // gun data valid
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xfff00000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( freezeat )
	PORT_START("P1_P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM )  // volume down
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM )  // volume up
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )           // s-test
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM )  // vsyncneq
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x000f0000, IP_ACTIVE_HIGH, IPT_CUSTOM ) // coin returns
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( fishfren )
	PORT_START("P1_P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM )  // volume down
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM )  // volume up
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )           // s-test
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM )  // vsyncneq
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x000f0000, IP_ACTIVE_HIGH, IPT_CUSTOM ) // coin returns
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( vcircle )
	PORT_START("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x000000f8, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x00f80000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )           // s-test
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM )  // vsyncneq
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x000f0000, IP_ACTIVE_HIGH, IPT_CUSTOM ) // coin returns
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( jaguar )
	PORT_START("JOY0")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0xf0ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY1")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad 1") PORT_CODE(KEYCODE_1) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad 4") PORT_CODE(KEYCODE_4) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad 7") PORT_CODE(KEYCODE_7) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad *") PORT_CODE(KEYCODE_K) PORT_PLAYER(1)
	PORT_BIT( 0xf0ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY2")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad 2") PORT_CODE(KEYCODE_2) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad 5") PORT_CODE(KEYCODE_5) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad 8") PORT_CODE(KEYCODE_8) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad 0") PORT_CODE(KEYCODE_0) PORT_PLAYER(1)
	PORT_BIT( 0xf0ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY3")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad 3") PORT_CODE(KEYCODE_3) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad 6") PORT_CODE(KEYCODE_6) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad 9") PORT_CODE(KEYCODE_9) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P1 Keypad #") PORT_CODE(KEYCODE_L) PORT_PLAYER(1)
	PORT_BIT( 0xf0ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY4")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad 3") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad 6") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad 9") PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad #") PORT_PLAYER(2)
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY5")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad 2") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad 5") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad 8") PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad 0") PORT_PLAYER(2)
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY6")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad 1") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad 4") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad 7") PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Keypad *") PORT_PLAYER(2)
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY7")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Pause") PORT_CODE(KEYCODE_I) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 A") PORT_PLAYER(1)
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 B") PORT_PLAYER(1)
	PORT_BIT( 0xfffd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 C") PORT_PLAYER(1)
	PORT_BIT( 0xfffd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS3")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Option") PORT_CODE(KEYCODE_O) PORT_PLAYER(1)
	PORT_BIT( 0xfffd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS4")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Option") PORT_PLAYER(2)
	PORT_BIT( 0xfffd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS5")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 C") PORT_PLAYER(2)
	PORT_BIT( 0xfffd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS6")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 B") PORT_PLAYER(2)
	PORT_BIT( 0xfffd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS7")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Pause") PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 A") PORT_PLAYER(2)
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x02, 0x00, "Show Logo")
	PORT_CONFSETTING(    0x00, "Yes")
	PORT_CONFSETTING(    0x02, "No")
	PORT_CONFNAME( 0x10, 0x10, "TV System")
	PORT_CONFSETTING(    0x00, "PAL")
	PORT_CONFSETTING(    0x10, "NTSC")
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jaguar_state::video_config(machine_config &config, const XTAL clock)
{
	JAGUARGPU(config, m_gpu, clock);
	m_gpu->irq().set(FUNC(jaguar_state::gpu_cpu_int));

	JAGUARDSP(config, m_dsp, clock);
	m_dsp->irq().set(FUNC(jaguar_state::dsp_cpu_int));

	// TODO: Tom
	// TODO: Object Processor

	JAG_BLITTER(config, m_blitter, clock);
}

void jaguar_state::cojagr3k(machine_config &config)
{
	/* basic machine hardware */
	R3041(config, m_maincpu, R3000_CLOCK).set_endianness(ENDIANNESS_BIG);
	m_maincpu->set_addrmap(AS_PROGRAM, &jaguar_state::r3000_map);

	video_config(config, COJAG_CLOCK/2);
	m_gpu->set_addrmap(AS_PROGRAM, &jaguar_state::gpu_map);
	m_dsp->set_addrmap(AS_PROGRAM, &jaguar_state::dsp_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	WATCHDOG_TIMER(config, "watchdog");

	VT83C461(config, m_ide).options(cojag_devices, "hdd", nullptr, true);
	m_ide->irq_handler().set(FUNC(jaguar_state::external_int));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(COJAG_PIXEL_CLOCK/2, 456, 42, 402, 262, 17, 257);
	m_screen->set_screen_update(FUNC(jaguar_state::screen_update));

	PALETTE(config, m_palette, FUNC(jaguar_state::jagpal_ycc), 65536);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0).add_route(ALL_OUTPUTS, "lspeaker", 1.0); // unknown DAC
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0).add_route(ALL_OUTPUTS, "rspeaker", 1.0); // unknown DAC

	// TODO: subwoofer speaker
}

void jaguar_state::cojagr3k_rom(machine_config &config)
{
	cojagr3k(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jaguar_state::r3000_rom_map);
	m_gpu->set_addrmap(AS_PROGRAM, &jaguar_state::gpu_rom_map);
	m_dsp->set_addrmap(AS_PROGRAM, &jaguar_state::dsp_rom_map);

	m_ide->slot(0).set_default_option(nullptr);
}

void jaguar_state::cojag68k(machine_config &config)
{
	cojagr3k(config);

	/* basic machine hardware */
	M68EC020(config.replace(), m_maincpu, M68K_CLOCK/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &jaguar_state::m68020_map);
}

void jaguar_state::jaguar(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, JAGUAR_CLOCK/2); // MC68000FN12F 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &jaguar_state::jaguar_map);
	m_maincpu->set_addrmap(m68000_device::AS_CPU_SPACE, &jaguar_state::cpu_space_map);

	video_config(config, JAGUAR_CLOCK);
	m_gpu->set_addrmap(AS_PROGRAM, &jaguar_state::jag_gpu_dsp_map);
	m_dsp->set_addrmap(AS_PROGRAM, &jaguar_state::jag_gpu_dsp_map);

//  MCFG_NVRAM_HANDLER(jaguar)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(JAGUAR_CLOCK, 456, 42, 402, 262, 17, 257);
	m_screen->set_screen_update(FUNC(jaguar_state::screen_update));

	PALETTE(config, m_palette, FUNC(jaguar_state::jagpal_ycc), 65536);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0).add_route(ALL_OUTPUTS, "lspeaker", 1.0); // unknown DAC
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0).add_route(ALL_OUTPUTS, "rspeaker", 1.0); // unknown DAC

	/* quickload */
	QUICKLOAD(config, "quickload", "abs,bin,cof,jag,prg,rom", attotime::from_seconds(1)).set_load_callback(FUNC(jaguar_state::quickload_cb));

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "jaguar_cart", "j64"));
	cartslot.set_device_load(FUNC(jaguar_state::cart_load));

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("jaguar");

	EEPROM_93C46_16BIT(config, m_eeprom);
}

void jaguarcd_state::jaguarcd(machine_config &config)
{
	jaguar(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jaguarcd_state::jaguarcd_map);

	m_gpu->set_addrmap(AS_PROGRAM, &jaguarcd_state::jagcd_gpu_dsp_map);

	m_dsp->set_addrmap(AS_PROGRAM, &jaguarcd_state::jagcd_gpu_dsp_map);

	CDROM(config, "cdrom").set_interface("cdrom");

	// TODO: software list, requires multisession support first
}


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void jaguar_state::fix_endian( void *base, uint32_t size )
{
	uint32_t *mem = reinterpret_cast<uint32_t *>(base);

	for (uint32_t i = 0; i < size; i+=4)
		mem[i/4] = big_endianize_int32(mem[i/4]);
}

void jaguar_state::init_jaguar()
{
	m_is_cojag = false;
	m_hacks_enabled = false;
	save_item(NAME(m_joystick_data));

	/* Initialize for no cartridge present */
	m_using_cart = false;
}

void jaguarcd_state::init_jaguarcd()
{
	m_hacks_enabled = false;
	save_item(NAME(m_joystick_data));
}

std::pair<std::error_condition, std::string> jaguar_state::quickload_cb(snapshot_image_device &image)
{
	offs_t quickload_begin = 0x1000, start = 0x4000, skip = 0;

	offs_t quickload_size = std::min(offs_t(image.length()), 0x20000 - start);

	image.fread( &m_shared_ram[quickload_begin], quickload_size);

	fix_endian(&m_shared_ram[quickload_begin], quickload_size);

	/* Deal with some of the numerous homebrew header systems */
		/* COF */
	if ((m_shared_ram[0x1000] & 0xffff0000) == 0x01500000)
	{
		start = m_shared_ram[0x100e];
		skip = m_shared_ram[0x1011];
	}
	else    /* PRG */
	if (((m_shared_ram[0x1000] & 0xffff0000) == 0x601A0000) && (m_shared_ram[0x1007] == 0x4A414752))
	{
		uint32_t type = m_shared_ram[0x1008] >> 16;
		start = ((m_shared_ram[0x1008] & 0xffff) << 16) | (m_shared_ram[0x1009] >> 16);
		skip = 28;
		if (type == 2) skip = 42;
		else if (type == 3) skip = 46;
	}
	else    /* ABS with header */
	if ((m_shared_ram[0x1000] & 0xffff0000) == 0x601B0000)
	{
		start = ((m_shared_ram[0x1005] & 0xffff) << 16) | (m_shared_ram[0x1006] >> 16);
		skip = 36;
	}

	else    /* A header used by Badcoder */
	if ((m_shared_ram[0x1000] & 0xffff0000) == 0x72000000)
		skip = 96;

	else    /* ABS binary */
	if (image.is_filetype("abs"))
		start = 0xc000;

	else    /* JAG binary */
	if (image.is_filetype("jag"))
		start = 0x5000;
	else
	if (image.is_filetype("rom"))
		start = 0x802000;

	quickload_size = image.length();

	/* Now that we have the info, reload the file */
	if ((start + quickload_size) < 0x200000)
	{
		memset(m_shared_ram, 0, 0x200000);
		image.fseek(skip, SEEK_SET);
		image.fread( &m_shared_ram[start/4], quickload_size-skip);
		fix_endian(&m_shared_ram[start/4], quickload_size-skip);
	}
	else
	if (start >= 0x800000)
	{
		image.fseek(skip, SEEK_SET);
		image.fread( &m_cart_base[(start - 0x800000) / 4], quickload_size - skip);
		fix_endian(&m_cart_base[(start - 0x800000) / 4], quickload_size - skip);
	}
	else
		return std::make_pair(image_error::UNSUPPORTED, "Unsupported start address for this quickload.");

	/* Some programs are too lazy to set a stack pointer */
	m_maincpu->set_state_int(M68K_SP, 0x1000);
	m_shared_ram[0]=0x1000;

	/* Transfer control to image */
	m_maincpu->set_pc(quickload_begin);
	m_shared_ram[1]=quickload_begin;
	return std::make_pair(std::error_condition(), std::string());
}

DEVICE_IMAGE_LOAD_MEMBER( jaguar_state::cart_load )
{
	uint32_t size, load_offset = 0;

	if (!image.loaded_through_softlist())
	{
		size = image.length();

		/* Load cart into memory */
		image.fread(&m_cart_base[load_offset/4], size);
		fix_endian(&m_cart_base[load_offset/4], size);
	}
	else
	{
		size = image.get_software_region_length("rom");

		memcpy(&m_cart_base[0], image.get_software_region("rom"), size);
	}

	memset(&m_shared_ram[0], 0, 0x200000);

	/* Skip the logo */
	m_using_cart = true;
	//  m_cart_base[0x102] = 1;

	/* Transfer control to the bios */
	m_maincpu->reset();
	return std::make_pair(std::error_condition(), std::string());
}


/*************************************
 *
 *  ROM definition(s)
 *
 *  Date Information comes from either
 *   ROM labels or from the Self-Test
 *   as "Main"
 *
 *************************************/

/* Home System */

ROM_START( jaguar )
	ROM_REGION16_BE( 0x20000, "mainrom", 0 )
	ROM_LOAD16_WORD( "jagboot.rom", 0x00000, 0x20000, CRC(fb731aaa) SHA1(f8991b0c385f4e5002fa2a7e2f5e61e8c5213356) )

	ROM_REGION32_BE( 0x600000, "cart", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )
ROM_END

ROM_START( jaguarcd )
	ROM_REGION16_BE( 0x20000, "mainrom", 0 )
	ROM_LOAD16_WORD( "jagboot.rom", 0x00000, 0x20000, CRC(fb731aaa) SHA1(f8991b0c385f4e5002fa2a7e2f5e61e8c5213356) )

	ROM_REGION32_BE( 0x600000, "cart", ROMREGION_ERASE00 )
	// TODO: cart needs to be removed (CD BIOS runs in the cart space)

	ROM_REGION16_BE(0x40000, "cdbios", 0 )
	ROM_SYSTEM_BIOS( 0, "default", "Jaguar CD" )
	ROMX_LOAD( "jag_cd.bin", 0x00000, 0x040000, CRC(687068d5) SHA1(73883e7a6e9b132452436f7ab1aeaeb0776428e5), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "dev", "Jaguar Developer CD" )
	ROMX_LOAD( "jagdevcd.bin", 0x00000, 0x040000, CRC(55a0669c) SHA1(d61b7b5912118f114ef00cf44966a5ef62e455a5), ROM_GROUPWORD | ROM_BIOS(1) )

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )
ROM_END


/****************************************

       ROM & Hard Disk based games

****************************************/

ROM_START( area51t ) /* 68020 based, Area51 Time Warner License - MAIN: Oct 17 1996 17:15:41 / OS: 2.03CJ Oct 17 1996 17:15:01 */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for 68020 code */
	ROM_LOAD32_BYTE( "136105-0003-q_h.3h", 0x00000, 0x80000, CRC(0681f398) SHA1(9e96db5a4ff90800685a5b95f8d758d211d3b982) ) /* Also found labeled as AREA51, 68K, D2FF, 3H, 11/20/96 (each item on a separate line) */
	ROM_LOAD32_BYTE( "136105-0002-q_p.3p", 0x00001, 0x80000, CRC(f76cfc68) SHA1(01a781b42b61279e09e0cb1d924e2a3e0df44591) ) /* Also found labeled as AREA51, 68K, 69FE, 3P, 11/20/96 (each item on a separate line) */
	ROM_LOAD32_BYTE( "136105-0001-q_m.3m", 0x00002, 0x80000, CRC(f422b4a8) SHA1(f95ef428be18adafae65e35f412eb03dcdaf7ed4) ) /* Also found labeled as AREA51, 68K, FCFD, 3M, 11/20/96 (each item on a separate line) */
	ROM_LOAD32_BYTE( "136105-0000-q_k.3k", 0x00003, 0x80000, CRC(1fb2f2b5) SHA1(cbed65463dd93eaf945750a9dc3a123d1c6bda42) ) /* Also found labeled as AREA51, 68K, 65FC, 3K, 11/20/96 (each item on a separate line) */

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "area51t", 0, SHA1(d2865cc7b1bb08a4393a72013a90e18d8a8f9860) )
ROM_END

ROM_START( area51ta ) /* 68020 based, Area51 Time Warner License - MAIN: Nov 27 1995 15:51:56 / OS 2.03CJ Nov 15 1995 13:32:32 */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for 68020 code */
	ROM_LOAD32_BYTE( "136105-0003c.3h", 0x00000, 0x80000, CRC(e70a97c4) SHA1(39dabf6bf3dc6f717a587f362d040bfb332be9e1) ) /* Usually found with "orange" labels */
	ROM_LOAD32_BYTE( "136105-0002c.3p", 0x00001, 0x80000, CRC(e9c9f4bd) SHA1(7c6c50372d45dca8929767241b092339f3bab4d2) )
	ROM_LOAD32_BYTE( "136105-0001c.3m", 0x00002, 0x80000, CRC(6f135a81) SHA1(2d9660f240b14481e8c46bc98713e9dc12035063) )
	ROM_LOAD32_BYTE( "136105-0000c.3k", 0x00003, 0x80000, CRC(94f50c14) SHA1(a54552e3ac5c4f481ba4f2fc7d724534576fe76c) )

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "area51t", 0, SHA1(d2865cc7b1bb08a4393a72013a90e18d8a8f9860) )
ROM_END

ROM_START( area51a ) /* 68020 based, Area51 Atari Games License - MAIN: Oct 25 1995 11:08:10 / OS: 2.03CJ Oct 25 1995 10:19:38 */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for 68020 code */
	ROM_LOAD32_BYTE( "136105-0003a.3h", 0x00000, 0x80000, CRC(116d37e6) SHA1(5d36cae792dd349faa77cd2d8018722a28ee55c1) ) /* Usually found with "green" labels */
	ROM_LOAD32_BYTE( "136105-0002a.3p", 0x00001, 0x80000, CRC(eb10f539) SHA1(dadc4be5a442dd4bd17385033056555e528ed994) )
	ROM_LOAD32_BYTE( "136105-0001a.3m", 0x00002, 0x80000, CRC(c6d8322b) SHA1(90cf848a4195c51b505653cc2c74a3b9e3c851b8) )
	ROM_LOAD32_BYTE( "136105-0000a.3k", 0x00003, 0x80000, CRC(729eb1b7) SHA1(21864b4281b1ad17b2903e3aa294e4be74161e80) )

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "area51", 0, SHA1(3b303bc37e206a6d7339352c869f050d04186f11) )
ROM_END

ROM_START( area51 ) /* R3000 based, Area51 Atari Games License - MAIN: Oct 24 1996 12:02:23 / GUTS: 2.06CJ Nov 11 1996 11:46:43 */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "2-c_area_51_hh.hh", 0x00000, 0x80000, CRC(13af6a1e) SHA1(69da54ed6886e825156bbcc256e8d7abd4dc1ff8) ) /* Green labels: 2-C AREA 51 HH  - also found labeled as AREA 51 HH   136105-1004 */
	ROM_LOAD32_BYTE( "2-c_area_51_hl.hl", 0x00001, 0x80000, CRC(8ab6649b) SHA1(9b4945bc04f8a73161638a2c5fa2fd84c6fd31b4) ) /* Green labels: 2-C AREA 51 HL  - also found labeled as AREA 51 HL   136105-1003 */
	ROM_LOAD32_BYTE( "2-c_area_51_lh.lh", 0x00002, 0x80000, CRC(a6524f73) SHA1(ae377a6803a4f7d1bbcc111725af121a3e82317d) ) /* Green labels: 2-C AREA 51 LH  - also found labeled as AREA 51 LH   136105-1002 */
	ROM_LOAD32_BYTE( "2-c_area_51_ll.ll", 0x00003, 0x80000, CRC(471b15d2) SHA1(4b5f45ee140b03a6be61475cae1c2dbef0f07457) ) /* Green labels: 2-C AREA 51 LL  - also found labeled as AREA 51 LL   136105-1001 */

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "area51", 0, SHA1(3b303bc37e206a6d7339352c869f050d04186f11) )
ROM_END


ROM_START( maxforce ) /* R3000 based, labeled as "Maximum Force 5-23-97 v1.05" - Usually found with "light grey" labels */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "1.05_maximum_force_hh_5-23-97.hh", 0x00000, 0x80000, CRC(ec7f8167) SHA1(0cf057bfb1f30c2c9621d3ed25021e7ba7bdd46e) ) /* Also found labeled as "MAXIMUM FORCE EE FIX PROG" */
	ROM_LOAD32_BYTE( "1.05_maximum_force_hl_5-23-97.hl", 0x00001, 0x80000, CRC(3172611c) SHA1(00f14f871b737c66c20f95743740d964d0be3f24) )
	ROM_LOAD32_BYTE( "1.05_maximum_force_lh_5-23-97.lh", 0x00002, 0x80000, CRC(84d49423) SHA1(88d9a6724f1118f2bbef5dfa27accc2b65c5ba1d) )
	ROM_LOAD32_BYTE( "1.05_maximum_force_ll_5-23-97.ll", 0x00003, 0x80000, CRC(16d0768d) SHA1(665a6d7602a7f2f5b1f332b0220b1533143d56b1) )

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "maxforce", 0, SHA1(d54e7a8f3866bb2a1d28ae637e7c92ffa4dbe558) )
ROM_END

ROM_START( maxf_102 ) /* R3000 based, labeled as "Maximum Force 2-27-97 v1.02" - Usually found with "yellow" labels */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "1.02_maximum_force_hh_2-27-97.hh", 0x00000, 0x80000, CRC(8ff7009d) SHA1(da22eae298a6e0e36f503fa091ac3913423dcd0f) ) /* Also found labeled as MAX, FORCE, V. 1.02, PROG, HH, 46FF, 2/27/97 (each item on a separate line) */
	ROM_LOAD32_BYTE( "1.02_maximum_force_hl_2-27-97.hl", 0x00001, 0x80000, CRC(96c2cc1d) SHA1(b332b8c042b92c736131c478cefac1c3c2d2673b) ) /* Also found labeled as MAX, FORCE, V. 1.02, PROG, HL, 14FE, 2/27/97 (each item on a separate line) */
	ROM_LOAD32_BYTE( "1.02_maximum_force_lh_2-27-97.lh", 0x00002, 0x80000, CRC(459ffba5) SHA1(adb40db6904e84c17f32ac6518fd2e994da7883f) ) /* Also found labeled as MAX, FORCE, V. 1.02, PROG, LH, 15FD, 2/27/97 (each item on a separate line) */
	ROM_LOAD32_BYTE( "1.02_maximum_force_ll_2-27-97.ll", 0x00003, 0x80000, CRC(e491be7f) SHA1(cbe281c099a4aa87067752d68cf2bb0ab3900531) ) /* Also found labeled as MAX, FORCE, V. 1.02, PROG, LL, 15FC, 2/27/97 (each item on a separate line) */

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "maxforce", 0, SHA1(d54e7a8f3866bb2a1d28ae637e7c92ffa4dbe558) )
ROM_END

ROM_START( maxf_ng ) /* R3000 based - MAIN: Apr 18 1997 11:08:45 */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "maximum_force_no_gore_hh.hh", 0x00000, 0x80000, CRC(08791c02) SHA1(9befbff3201c7d345109b26c296fd8548dbfc95b) )
	ROM_LOAD32_BYTE( "maximum_force_no_gore_hl.hl", 0x00001, 0x80000, CRC(52cf482c) SHA1(ff98b3f04987acef82a97a2ad35a9085fa84e6d5) )
	ROM_LOAD32_BYTE( "maximum_force_no_gore_lh.lh", 0x00002, 0x80000, CRC(ab4ee992) SHA1(69f0fe111d3f5f31151d2922579e5073e484b1e1) )
	ROM_LOAD32_BYTE( "maximum_force_no_gore_ll.ll", 0x00003, 0x80000, CRC(674aab43) SHA1(f79d790538756d1100b7e4ffed192a62a031a2cb) )

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	ROM_REGION( 0x800, "user2", 0 ) /* 28C16 style eeprom, currently loaded but not used */
	ROM_LOAD( "28c16.17z", 0x000, 0x800, CRC(1cdd9088) SHA1(4f01f02ff95f31ced87a3cdd7f171afd92551266) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "maxforce", 0, SHA1(d54e7a8f3866bb2a1d28ae637e7c92ffa4dbe558) )
ROM_END


ROM_START( area51mx )   /* 68020 based - MAIN: Apr 22 1998 17:53:57 / GUTS: 2.04CJ Apr 22 1998 17:45:35 */
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 2MB for 68020 code */
	ROM_LOAD32_BYTE( "2.0_68020_max-a51_kit_3h.3h", 0x00000, 0x80000, CRC(47cbf30b) SHA1(23377bcc65c0fc330d5bc7e76e233bae043ac364) ) /* Labeled as 2.0 68020 MAX/A51 KIT 3H */
	ROM_LOAD32_BYTE( "2.0_68020_max-a51_kit_3p.3p", 0x00001, 0x80000, CRC(a3c93684) SHA1(f6b3357bb69900a176fd6bc6b819b2f57b7d0f59) ) /* Labeled as 2.0 68020 MAX/A51 KIT 3P */
	ROM_LOAD32_BYTE( "2.0_68020_max-a51_kit_3m.3m", 0x00002, 0x80000, CRC(d800ac17) SHA1(3d515c8608d8101ee9227116175b3c3f1fe22e0c) ) /* Labeled as 2.0 68020 MAX/A51 KIT 3M */
	ROM_LOAD32_BYTE( "2.0_68020_max-a51_kit_3k.3k", 0x00003, 0x80000, CRC(0e78f308) SHA1(adc4c8e441eb8fe525d0a6220eb3a2a8791a7289) ) /* Labeled as 2.0 68020 MAX/A51 KIT 3K */

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "area51mx", 0, SHA1(5ff10f4e87094d4449eabf3de7549564ca568c7e) )
ROM_END

ROM_START( a51mxr3k ) /* R3000 based - MAIN: Feb 10 1998 11:52:51 / GUTS: 2.07CJ Feb  5 1998 18:52:26 */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "1.0_r3k_max-a51_kit_hh.hh", 0x00000, 0x80000, CRC(a984dab2) SHA1(debb3bc11ff49e87a52e89a69533a1bab7db700e) ) /* Labeled as 1.0 R3K MAX/A51 KIT HH */
	ROM_LOAD32_BYTE( "1.0_r3k_max-a51_kit_hl.hl", 0x00001, 0x80000, CRC(0af49d74) SHA1(c19f26056a823fd32293e9a7b3ea868640eabf49) ) /* Labeled as 1.0 R3K MAX/A51 KIT HL */
	ROM_LOAD32_BYTE( "1.0_r3k_max-a51_kit_lh.lh", 0x00002, 0x80000, CRC(d7d94dac) SHA1(2060a74715f36a0d7f5dd0855eda48ad1f20f095) ) /* Labeled as 1.0 R3K MAX/A51 KIT LH */
	ROM_LOAD32_BYTE( "1.0_r3k_max-a51_kit_ll.ll", 0x00003, 0x80000, CRC(ece9e5ae) SHA1(7e44402726f5afa6d1670b27aa43ad13d21c4ad9) ) /* Labeled as 1.0 R3K MAX/A51 KIT LL */

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "area51mx", 0, SHA1(5ff10f4e87094d4449eabf3de7549564ca568c7e) )
ROM_END

ROM_START( a51mxr3ka ) /* R3000 based - MAIN: Feb  2 1998 14:10:29 / GUTS: 2.07CJ Jan  9 1998 21:11:55 */
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "maxa51_combo_r3k_hh_prog_2-02-98_67ff.hh", 0x00000, 0x80000, CRC(6af8950a) SHA1(33ae123065b14ed8d83635f3351ac5b5c136d206) ) /* Labeled as MAXA51  COMBO  R3K  HH  PROG  2/02/98  67FF (each item on a separate line) */
	ROM_LOAD32_BYTE( "maxa51_combo_r3k_hl_prog_2-02-98_72fe.hl", 0x00001, 0x80000, CRC(30dc3eea) SHA1(2b4e8d43ee28b2d1446c84ff79553a7ce1909f60) ) /* Labeled as MAXA51  COMBO  R3K  HL  PROG  2/02/98  72FE (each item on a separate line) */
	ROM_LOAD32_BYTE( "maxa51_combo_r3k_lh_prog_2-02-98_7ffd.lh", 0x00002, 0x80000, CRC(2c2124af) SHA1(6158644ef126f842a1a4f145141ce847302bbd62) ) /* Labeled as MAXA51  COMBO  R3K  LH  PROG  2/02/98  7FFD (each item on a separate line) */
	ROM_LOAD32_BYTE( "maxa51_combo_r3k_ll_prog_2-02-98_b3fc.ll", 0x00003, 0x80000, CRC(083f4429) SHA1(2be8db7c756a095c87f056da49b8e8832f18bca9) ) /* Labeled as MAXA51  COMBO  R3K  LL  PROG  2/02/98  B3FC (each item on a separate line) */

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "area51mx", 0, SHA1(5ff10f4e87094d4449eabf3de7549564ca568c7e) )
ROM_END


ROM_START( vcircle )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "hh", 0x00000, 0x80000, CRC(7276f5f5) SHA1(716287e370a4f300b1743103f8031afc82de38ca) )
	ROM_LOAD32_BYTE( "hl", 0x00001, 0x80000, CRC(146060a1) SHA1(f291989f1f0ef228757f1990fb14da5ff8f3cf8d) )
	ROM_LOAD32_BYTE( "lh", 0x00002, 0x80000, CRC(be4b2ef6) SHA1(4332b3036e9cb12685e914d085d9a63aa856f0be) )
	ROM_LOAD32_BYTE( "ll", 0x00003, 0x80000, CRC(ba8753eb) SHA1(0322e0e37d814a38d08ba191b1a97fb1a55fe461) )

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "vcircle", 0, SHA1(bfa79c4cacdc9c2cd6362f62a23056b3e35a2034) )
ROM_END



/****************************************

       ROM based games

****************************************/

ROM_START( fishfren )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "hh", 0x00000, 0x80000, CRC(2ef79767) SHA1(abcea584f2cbd71b05f9d7e61f40ca9da6799215) )
	ROM_LOAD32_BYTE( "hl", 0x00001, 0x80000, CRC(7eefd4a2) SHA1(181be04836704098082fd78cacc68ffa70e77892) )
	ROM_LOAD32_BYTE( "lh", 0x00002, 0x80000, CRC(bbe9ed15) SHA1(889af29afe6d984b39105aa238400392a5dfb2c5) )
	ROM_LOAD32_BYTE( "ll", 0x00003, 0x80000, CRC(d70d0f2c) SHA1(2689cbe56ae3d491348b241528b0fe345fa8484c) )

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	ROM_REGION32_BE( 0x1000000, "romboard", 0 ) /* 16MB for 64-bit ROM data */
	ROM_LOAD64_BYTE( "l63-56", 0x000000, 0x100000, CRC(42764ea5) SHA1(805245f01006bd974fbac56f688cfcf137ddc914) )
	ROM_LOAD64_BYTE( "l55-48", 0x000001, 0x100000, CRC(0c7592bb) SHA1(d5bd6b872abad58947842205f9eac46fd065e88f) )
	ROM_LOAD64_BYTE( "l47-40", 0x000002, 0x100000, CRC(6d7dcdb1) SHA1(914dae3b9df5c861f794b683571c5fb0c2c3c3fd) )
	ROM_LOAD64_BYTE( "l39-32", 0x000003, 0x100000, CRC(ef3b8d98) SHA1(858c3342e9693bfe887b91dde1116a1656a1a105) )
	ROM_LOAD64_BYTE( "l31-24", 0x000004, 0x100000, CRC(132d628e) SHA1(3ff9fa86092eb01f21ca3ccf1ee1e3a583cbdecb) )
	ROM_LOAD64_BYTE( "l23-16", 0x000005, 0x100000, CRC(b841f039) SHA1(79f661aee009aef2f5ad4122ae3e0ac94097a427) )
	ROM_LOAD64_BYTE( "l15-08", 0x000006, 0x100000, CRC(0800214e) SHA1(5372f2c3470619a4967958c76055486f76b5f150) )
	ROM_LOAD64_BYTE( "l07-00", 0x000007, 0x100000, CRC(f83b2e78) SHA1(83ee9d2bfba83e04fb794270926bd3e558c9aaa4) )
	ROM_LOAD64_BYTE( "h63-56", 0x800000, 0x080000, CRC(67740765) SHA1(8b22413d25e0dbfe2227d1a8a023961a4c13cb76) )
	ROM_LOAD64_BYTE( "h55-48", 0x800001, 0x080000, CRC(ffed0091) SHA1(6c8104acd7e6d95a111f9c7a4d3b6984293d72c4) )
	ROM_LOAD64_BYTE( "h47-40", 0x800002, 0x080000, CRC(6f448f72) SHA1(3a298b9851e4ba7aa611aa6c2b0dcf06f4301463) )
	ROM_LOAD64_BYTE( "h39-32", 0x800003, 0x080000, CRC(25a5bd67) SHA1(79f29bd36afb4574b9c923eee293964284713540) )
	ROM_LOAD64_BYTE( "h31-24", 0x800004, 0x080000, CRC(e7088cc0) SHA1(4cb184de748c5633e669a4675e6db9920d34811e) )
	ROM_LOAD64_BYTE( "h23-16", 0x800005, 0x080000, CRC(ab477a76) SHA1(ae9aa97dbc758cd741710fe08c6ea94a0a318451) )
	ROM_LOAD64_BYTE( "h15-08", 0x800006, 0x080000, CRC(25a423f1) SHA1(7530cf2e28e0755bfcbd70789ef5cbbfb3d94f9f) )
	ROM_LOAD64_BYTE( "h07-00", 0x800007, 0x080000, CRC(0f5f4cc6) SHA1(caa2b514fb1f2a815e63f7b8c6b79ce2dfa308c4) )
	ROM_COPY( "romboard", 0x800000, 0xc00000, 0x400000 )
ROM_END

ROM_START( freezeat )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog_eng.hh",          0x000000, 0x040000, CRC(f7cffafd) SHA1(62369de4cf0a5abab86f6bcf9621028b9e171ec3) )
	ROM_LOAD32_BYTE( "prog_eng.hl",          0x000001, 0x040000, CRC(17150705) SHA1(c5a32d334bffb58a816920cc1251a21acc5a6f92) )
	ROM_LOAD32_BYTE( "prog_eng.lh",          0x000002, 0x040000, CRC(12a903bf) SHA1(41f5949d7ed2081917af8411f92666b754564b37) )
	ROM_LOAD32_BYTE( "prog_eng.ll",          0x000003, 0x040000, CRC(cf69f971) SHA1(132b06f5fb49801fff7e5deb7aa71b44d5b1c6ca) )

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	ROM_REGION32_BE( 0x1000000, "romboard", 0 ) /* 16MB for 64-bit ROM data */
	ROM_LOAD64_BYTE( "fish_gr0.63-56", 0x000000, 0x100000, CRC(b61061c5) SHA1(aeb409aa5073232d80ed81b27946e753290234f4) )
	ROM_LOAD64_BYTE( "fish_gr0.55-48", 0x000001, 0x100000, CRC(c85acf42) SHA1(c3365caeb126a83a7e7afcda25f05849ceb5c98b) )
	ROM_LOAD64_BYTE( "fish_gr0.47-40", 0x000002, 0x100000, CRC(67f78f59) SHA1(40b256a8939fad365c7e896cff4a959fcc70a477) )
	ROM_LOAD64_BYTE( "fish_gr0.39-32", 0x000003, 0x100000, CRC(6be0508a) SHA1(20f617278ce1666348822d80686cecd8d9b1bc78) )
	ROM_LOAD64_BYTE( "fish_gr0.31-24", 0x000004, 0x100000, CRC(905606e0) SHA1(866cd98ea2399fed96f76b16dce751e2c7cfdc98) )
	ROM_LOAD64_BYTE( "fish_gr0.23-16", 0x000005, 0x100000, CRC(cdeef6fa) SHA1(1b4d58951b662040540e7d51f88c1b6f282562ee) )
	ROM_LOAD64_BYTE( "fish_gr0.15-08", 0x000006, 0x100000, CRC(ad81f204) SHA1(58584a6c8c6cfb6366eaa10aba8a226e419f5ce9) )
	ROM_LOAD64_BYTE( "fish_gr0.07-00", 0x000007, 0x100000, CRC(10ce7254) SHA1(2a88d45dbe78ea8358ecd8522b38d775a2fdb34a) )
	ROM_LOAD64_BYTE( "fish_eng.63-56", 0x800000, 0x100000, CRC(4a03f971) SHA1(1ae5ad9a6cd2d612c6519193134dcd5a3f6a5049) )
	ROM_LOAD64_BYTE( "fish_eng.55-48", 0x800001, 0x100000, CRC(6bc00de0) SHA1(b1b180c33906826703452875ce250b28352e2797) )
	ROM_LOAD64_BYTE( "fish_eng.47-40", 0x800002, 0x100000, CRC(41ccc677) SHA1(76ee042632cfdcc99a9bfb75f2a4ef04e08f101b) )
	ROM_LOAD64_BYTE( "fish_eng.39-32", 0x800003, 0x100000, CRC(59a8fa03) SHA1(19e91a4791e0d2dbd8578cee0fa07c491204b0dc) )
	ROM_LOAD64_BYTE( "fish_eng.31-24", 0x800004, 0x100000, CRC(c3bb50a1) SHA1(b868ac0812d1c13feae82d293bb323a93a72e1d3) )
	ROM_LOAD64_BYTE( "fish_eng.23-16", 0x800005, 0x100000, CRC(237cfc93) SHA1(15f61dc621c5328cc7752c76b2b1dae265a5e886) )
	ROM_LOAD64_BYTE( "fish_eng.15-08", 0x800006, 0x100000, CRC(65bec279) SHA1(5e99972279ee9ad32e67866fc63799579a10f2dd) )
	ROM_LOAD64_BYTE( "fish_eng.07-00", 0x800007, 0x100000, CRC(13fa20ad) SHA1(0a04fdea025109c0e604ef2a6d58cfb3adce9bd1) )
ROM_END

ROM_START( freezeatjp )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog_jpn.hh",          0x000000, 0x040000, CRC(989302bf) SHA1(232927ec0a52b8bb587a3c206af8e1c6cde67860) )
	ROM_LOAD32_BYTE( "prog_jpn.hl",          0x000001, 0x040000, CRC(6262b760) SHA1(12ca749f5cdc6db7d19f88a21f5f955b80206784) )
	ROM_LOAD32_BYTE( "prog_jpn.lh",          0x000002, 0x040000, CRC(c6a12b0c) SHA1(971242b5b09e15164e7c335e684b5043510c6462) )
	ROM_LOAD32_BYTE( "prog_jpn.ll",          0x000003, 0x040000, CRC(241ea755) SHA1(0db3cfbe577fc78387528390ebb14dbb7a09c97d) )

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	ROM_REGION32_BE( 0x1000000, "romboard", 0 ) /* 16MB for 64-bit ROM data */
	ROM_LOAD64_BYTE( "fish_gr0.63-56", 0x000000, 0x100000, CRC(b61061c5) SHA1(aeb409aa5073232d80ed81b27946e753290234f4) )
	ROM_LOAD64_BYTE( "fish_gr0.55-48", 0x000001, 0x100000, CRC(c85acf42) SHA1(c3365caeb126a83a7e7afcda25f05849ceb5c98b) )
	ROM_LOAD64_BYTE( "fish_gr0.47-40", 0x000002, 0x100000, CRC(67f78f59) SHA1(40b256a8939fad365c7e896cff4a959fcc70a477) )
	ROM_LOAD64_BYTE( "fish_gr0.39-32", 0x000003, 0x100000, CRC(6be0508a) SHA1(20f617278ce1666348822d80686cecd8d9b1bc78) )
	ROM_LOAD64_BYTE( "fish_gr0.31-24", 0x000004, 0x100000, CRC(905606e0) SHA1(866cd98ea2399fed96f76b16dce751e2c7cfdc98) )
	ROM_LOAD64_BYTE( "fish_gr0.23-16", 0x000005, 0x100000, CRC(cdeef6fa) SHA1(1b4d58951b662040540e7d51f88c1b6f282562ee) )
	ROM_LOAD64_BYTE( "fish_gr0.15-08", 0x000006, 0x100000, CRC(ad81f204) SHA1(58584a6c8c6cfb6366eaa10aba8a226e419f5ce9) )
	ROM_LOAD64_BYTE( "fish_gr0.07-00", 0x000007, 0x100000, CRC(10ce7254) SHA1(2a88d45dbe78ea8358ecd8522b38d775a2fdb34a) )
	ROM_LOAD64_BYTE( "fish_jpn.63-56", 0x800000, 0x100000, CRC(78c65a1f) SHA1(1a97737c222809930bde9df7a55e1ff1581a3202) )
	ROM_LOAD64_BYTE( "fish_jpn.55-48", 0x800001, 0x100000, CRC(9ffac0f1) SHA1(3ba5f8de32a5febb5d3d22f59ccb477834d33934) )
	ROM_LOAD64_BYTE( "fish_jpn.47-40", 0x800002, 0x100000, CRC(18543fb7) SHA1(4ab9969de9a66d6b7b70cfa5290c1cf7bce54838) )
	ROM_LOAD64_BYTE( "fish_jpn.39-32", 0x800003, 0x100000, CRC(22578f15) SHA1(4e314b22456ed4e4282406e990207e3b9bdf6203) )
	ROM_LOAD64_BYTE( "fish_jpn.31-24", 0x800004, 0x100000, CRC(5c41b91b) SHA1(ce354c8a4a3872b009e8af9f75e8f4f0892c7a7e) )
	ROM_LOAD64_BYTE( "fish_jpn.23-16", 0x800005, 0x100000, CRC(c2462646) SHA1(207d51a2aae076bc78548cf96325e670ea41609c) )
	ROM_LOAD64_BYTE( "fish_jpn.15-08", 0x800006, 0x100000, CRC(f8d998ec) SHA1(ffce4a16fbb2fff3dc0a29c0cede4dfe6316e97b) )
	ROM_LOAD64_BYTE( "fish_jpn.07-00", 0x800007, 0x100000, CRC(e7e0daa5) SHA1(108da84cc6b4df7ae88cfdacd27c1728e59cdb81) )
ROM_END

ROM_START( freezeat2 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog.hh",  0x000000, 0x040000, CRC(a8aefa52) SHA1(ba95da93035520de4b15245f68217c59dfb69dbd) ) // sldh
	ROM_LOAD32_BYTE( "prog.hl",  0x000001, 0x040000, CRC(152dd641) SHA1(52fa260baf1979ed8f15f8abcbbeebd8e595d0e4) ) // sldh
	ROM_LOAD32_BYTE( "prog.lh",  0x000002, 0x040000, CRC(416d26ed) SHA1(11cf3b88415a8a5d0bb8e1df08603a85202186ef) ) // sldh
	ROM_LOAD32_BYTE( "prog.ll",  0x000003, 0x040000, CRC(d6a5dbc8) SHA1(0e2176c35cbc59b2a5283366210409d0e930bac7) ) // sldh

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	ROM_REGION32_BE( 0x1000000, "romboard", 0 ) /* 16MB for 64-bit ROM data */
	ROM_LOAD64_BYTE( "fish_gr0.63-56", 0x000000, 0x100000, CRC(99d0dc75) SHA1(b32126eea70c7584d1c34a6ca33282fbaf4b03aa) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.55-48", 0x000001, 0x100000, CRC(2dfdfe62) SHA1(e0554d36ef5cf4b6ce171857ea4f2737f11286a5) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.47-40", 0x000002, 0x100000, CRC(722aee2a) SHA1(bc79433131bed5b08453d1b80324a28a552783de) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.39-32", 0x000003, 0x100000, CRC(919e31b4) SHA1(3807d4629d8277c780dba888c23d17ba47803f27) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.31-24", 0x000004, 0x100000, CRC(a957ac95) SHA1(ddfaca994c06976bee8b123857904e64f40b7f31) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.23-16", 0x000005, 0x100000, CRC(a147ec66) SHA1(6291008158d581b81e025ed34ff0950983c12c67) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.15-08", 0x000006, 0x100000, CRC(206d2f38) SHA1(6aca89df26d3602ff1da3c23f19e0782439623ff) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.07-00", 0x000007, 0x100000, CRC(06559831) SHA1(b2c022457425d7900337cfa2fd1622336c0c0bc5) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.63-56", 0x800000, 0x100000, CRC(30c624d2) SHA1(4ced77d1663169d0cb37d6728ec52e67f05064c5) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.55-48", 0x800001, 0x100000, CRC(049cd60f) SHA1(8a7615a76b57a4e6ef5d95a5ee6c56086671dbb6) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.47-40", 0x800002, 0x100000, CRC(d6aaf3bf) SHA1(1c597bdc0e61fd0941cff5a8a93f24f108bd0daa) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.39-32", 0x800003, 0x100000, CRC(7d6ebc69) SHA1(668769297f75f9c367bc5cde26419ed092fc9dd8) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.31-24", 0x800004, 0x100000, CRC(6e5fee1f) SHA1(1eca79c8d395f881d0a05f10073998fcae70c3b1) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.23-16", 0x800005, 0x100000, CRC(a8b1e9b4) SHA1(066285928e574e656510b90bc212a8d86660bd07) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.15-08", 0x800006, 0x100000, CRC(c90080e6) SHA1(a764bdd6b4e9e727f7468a53424a9211ec5fd5a8) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.07-00", 0x800007, 0x100000, CRC(1f20c020) SHA1(71b32386dc0444264f2f1e2a81899e0e9260994c) ) // sldh
ROM_END

ROM_START( freezeat3 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog.hh",  0x000000, 0x040000, CRC(863942e6) SHA1(c7429c8a5c86ff93c64950e201cffca83dd7b7b0) ) // sldh
	ROM_LOAD32_BYTE( "prog.hl",  0x000001, 0x040000, CRC(2acc18ef) SHA1(ead02566f7641b1d1066bd2e257b695e5c7e8437) ) // sldh
	ROM_LOAD32_BYTE( "prog.lh",  0x000002, 0x040000, CRC(948cf20c) SHA1(86c757aa3c849ef5ba94ed4d5dbf10e833dab6bd) ) // sldh
	ROM_LOAD32_BYTE( "prog.ll",  0x000003, 0x040000, CRC(5f44969e) SHA1(32345d7c56a3a890e71f8c71f25414d442b60af8) ) // sldh

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	ROM_REGION32_BE( 0x1000000, "romboard", 0 ) /* 16MB for 64-bit ROM data */
	ROM_LOAD64_BYTE( "fish_gr0.63-56", 0x000000, 0x100000, CRC(36799449) SHA1(bb706fe7fdc68f840702a127eed7d4519dd45869) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.55-48", 0x000001, 0x100000, CRC(23959947) SHA1(a35a6e62c7b2be57d41b1b64be93713cbf897f0a) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.47-40", 0x000002, 0x100000, CRC(4657e4e0) SHA1(b6c07182babcb0a106bf4a8f2e3f524371dd882d) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.39-32", 0x000003, 0x100000, CRC(b6ea4b64) SHA1(176f94f14307c40b9c611d6f6bc9118e498cdfad) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.31-24", 0x000004, 0x100000, CRC(7d4ce71f) SHA1(a1cf5aa9df8dd29c777c10cfdce0925981584261) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.23-16", 0x000005, 0x100000, CRC(02db4fd1) SHA1(fce6f31802bf36d6b006f0b212f553bdf21f9374) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.15-08", 0x000006, 0x100000, CRC(7d496d6c) SHA1(f82db0621729a00acf4077482e9dfab040ac829b) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.07-00", 0x000007, 0x100000, CRC(3aa389d8) SHA1(52502f2f3c91d7c29261f60fe8f489a352399c96) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.63-56", 0x800000, 0x100000, CRC(ead678c9) SHA1(f83d467f6685965b6176b10adbd4e35ef808baf3) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.55-48", 0x800001, 0x100000, CRC(3591e752) SHA1(df242d2f724edfd78f7191f0ba7a8cde2c09b25f) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.47-40", 0x800002, 0x100000, CRC(e29a7a6c) SHA1(0bfb26076b390492eed81d4c4f0852c64fdccfce) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.39-32", 0x800003, 0x100000, CRC(e980f957) SHA1(78e8ef07f443ce7991a46005627d5802d36d731c) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.31-24", 0x800004, 0x100000, CRC(d90c5221) SHA1(7a330f39f3751d58157f872d92c3c2b91fe60d14) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.23-16", 0x800005, 0x100000, CRC(9be0d4de) SHA1(9bb67a1f1db77483e896fed7096c1e23c153ede4) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.15-08", 0x800006, 0x100000, CRC(122248af) SHA1(80dd5486106d475bd9f6d78919ebeb176e7becff) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.07-00", 0x800007, 0x100000, CRC(5ae08327) SHA1(822d8292793509ebfbfce27e92a74c78c4328bda) ) // sldh
ROM_END

ROM_START( freezeat4 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog.hh",  0x000000, 0x040000, CRC(80336f5e) SHA1(9946e8eebec2cd68db059f40f535ea212f41913d) ) // sldh
	ROM_LOAD32_BYTE( "prog.hl",  0x000001, 0x040000, CRC(55125520) SHA1(13be4fbf32bcd94a2ea97fd690bd1dfdff146d33) ) // sldh
	ROM_LOAD32_BYTE( "prog.lh",  0x000002, 0x040000, CRC(9d99c794) SHA1(f443f05a5979db66d61ef4174f0369a1cf4b7793) ) // sldh
	ROM_LOAD32_BYTE( "prog.ll",  0x000003, 0x040000, CRC(e03700e0) SHA1(24d41750f02ee7e8fb379e517751b661400aa521) ) // sldh

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	ROM_REGION32_BE( 0x1000000, "romboard", 0 ) /* 16MB for 64-bit ROM data */
	ROM_LOAD64_BYTE( "fish_gr0.63-56", 0x000000, 0x100000, CRC(36799449) SHA1(bb706fe7fdc68f840702a127eed7d4519dd45869) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.55-48", 0x000001, 0x100000, CRC(23959947) SHA1(a35a6e62c7b2be57d41b1b64be93713cbf897f0a) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.47-40", 0x000002, 0x100000, CRC(4657e4e0) SHA1(b6c07182babcb0a106bf4a8f2e3f524371dd882d) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.39-32", 0x000003, 0x100000, CRC(b6ea4b64) SHA1(176f94f14307c40b9c611d6f6bc9118e498cdfad) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.31-24", 0x000004, 0x100000, CRC(7d4ce71f) SHA1(a1cf5aa9df8dd29c777c10cfdce0925981584261) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.23-16", 0x000005, 0x100000, CRC(02db4fd1) SHA1(fce6f31802bf36d6b006f0b212f553bdf21f9374) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.15-08", 0x000006, 0x100000, CRC(7d496d6c) SHA1(f82db0621729a00acf4077482e9dfab040ac829b) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.07-00", 0x000007, 0x100000, CRC(3aa389d8) SHA1(52502f2f3c91d7c29261f60fe8f489a352399c96) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.63-56", 0x800000, 0x100000, CRC(c91b6ee4) SHA1(58d2d6b1b9847150b8b3e358842c4a097ef91475) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.55-48", 0x800001, 0x100000, CRC(65528e55) SHA1(18020cababed379f77149b7e89e80b294766df31) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.47-40", 0x800002, 0x100000, CRC(8fe4187f) SHA1(c9ceec40688617e1251142465d0e608f80a83e40) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.39-32", 0x800003, 0x100000, CRC(fdf05a42) SHA1(849e224b68be2fb396ee4cb4729517470af7c282) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.31-24", 0x800004, 0x100000, CRC(bb2cd741) SHA1(ac55a54c702d222cb1b9bb480b0f7a71bc315878) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.23-16", 0x800005, 0x100000, CRC(ea8c5984) SHA1(eca1619c17dfac154a2024ec49b4b4f9f06a50c9) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.15-08", 0x800006, 0x100000, CRC(0b00c816) SHA1(879b0e9d92fe737d740c348dc1cc376c8abfbdb8) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.07-00", 0x800007, 0x100000, CRC(a84335c3) SHA1(340f5ddb9bff1ecd469eab8be36cc0ede84f1f5e) ) // sldh
ROM_END

ROM_START( freezeat5 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog.hh",  0x000000, 0x040000, CRC(95c4fc64) SHA1(cd00efe7f760ef1e4cdc4bc8a3b368427cb15d8a) ) // sldh
	ROM_LOAD32_BYTE( "prog.hl",  0x000001, 0x040000, CRC(ffb9cb71) SHA1(35d6a5440d63bc5b94c4447645365039169da368) ) // sldh
	ROM_LOAD32_BYTE( "prog.lh",  0x000002, 0x040000, CRC(3ddacd80) SHA1(79f9650531847eefd83908b6ea1e8362688b377c) ) // sldh
	ROM_LOAD32_BYTE( "prog.ll",  0x000003, 0x040000, CRC(95ebefb0) SHA1(b88b12adabd7b0902c3a78919bcec8d9a2b04168) ) // sldh

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	ROM_REGION32_BE( 0x1000000, "romboard", 0 ) /* 16MB for 64-bit ROM data */
	ROM_LOAD64_BYTE( "fish_gr0.63-56", 0x000000, 0x100000, CRC(404a10c3) SHA1(8e353ac7608bd54f0fea610c85166ad14f2faadb) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.55-48", 0x000001, 0x100000, CRC(0b262f2f) SHA1(2a963cb5c3344091406d090edfdda498709c6aa6) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.47-40", 0x000002, 0x100000, CRC(43f86d26) SHA1(b31d36b11052514b5bcd5bf8e400457ca572c306) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.39-32", 0x000003, 0x100000, CRC(5cf0228f) SHA1(7a8c59cf9a7744e9f332db5f661f507323375968) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.31-24", 0x000004, 0x100000, CRC(7a24ff98) SHA1(db9e0e8bb417f187267a6e4fc1e66ff060ee4096) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.23-16", 0x000005, 0x100000, CRC(ea163c93) SHA1(d07ed26191d36497c56b15774625a49ecb958386) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.15-08", 0x000006, 0x100000, CRC(d364534f) SHA1(153908bb8929a898945f768f8bc3d853c6aeaceb) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.07-00", 0x000007, 0x100000, CRC(7ba4cb0d) SHA1(16bd487123f499b7080596dc76253081179a0f66) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.63-56", 0x800000, 0x100000, CRC(0e1fc4a9) SHA1(a200bb0af5f1e2c3f8d221ae4e9ba55b9dfb8550) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.55-48", 0x800001, 0x100000, CRC(b696b875) SHA1(16dc4d5cee3f08360cf19926584419c21d781f45) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.47-40", 0x800002, 0x100000, CRC(e78d9302) SHA1(f8b5ed992c433d63677edbeafd3e465b1d42b455) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.39-32", 0x800003, 0x100000, CRC(9b50374c) SHA1(d8af3c9d8e0459e24b974cdf2e75c7c39582912f) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.31-24", 0x800004, 0x100000, CRC(b6a19b7e) SHA1(5668b27db4dade8efb1524b8ecd1fe78498e8460) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.23-16", 0x800005, 0x100000, CRC(ff835b67) SHA1(19da2de1d067069871c33c8b25fd2eac2d03f627) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.15-08", 0x800006, 0x100000, CRC(8daf6995) SHA1(2f44031378b5fb1ba1f80a966dbe902316dc6fe8) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.07-00", 0x800007, 0x100000, CRC(3676ac70) SHA1(640c4d4f53ca2bcae2009e402fd6ad70e40defa4) ) // sldh
ROM_END

ROM_START( freezeat6 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog.hh",  0x000000, 0x040000, CRC(120711fe) SHA1(387e3cc8a1a9ea7d65c528387891d09ed9889fe3) ) // sldh
	ROM_LOAD32_BYTE( "prog.hl",  0x000001, 0x040000, CRC(18dd292a) SHA1(00e79851140716985f43594142c97e510a06b24a) ) // sldh
	ROM_LOAD32_BYTE( "prog.lh",  0x000002, 0x040000, CRC(ce387e72) SHA1(021a274da0b828550a47c3778e1059d4e759693a) ) // sldh
	ROM_LOAD32_BYTE( "prog.ll",  0x000003, 0x040000, CRC(9b307b7c) SHA1(71b696802fe7c867525d2626351dcfacedabd696) ) // sldh

	ROM_REGION16_BE( 0x1000, "waverom", 0 )
	ROM_LOAD16_WORD("jagwave.rom", 0x0000, 0x1000, CRC(7a25ee5b) SHA1(58117e11fd6478c521fbd3fdbe157f39567552f0) )

	ROM_REGION32_BE( 0x1000000, "romboard", 0 ) /* 16MB for 64-bit ROM data */
	ROM_LOAD64_BYTE( "fish_gr0.63-56", 0x000000, 0x100000, CRC(293a3308) SHA1(e4c88759c3b8f8a359db83817dbd0428350b4f7e) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.55-48", 0x000001, 0x100000, CRC(18bb4bdf) SHA1(1f6c49b3b5946390fa7582b531f8d9af3baa2567) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.47-40", 0x000002, 0x100000, CRC(1faedcc6) SHA1(1e4ecbe4553fb3ebfbd03bd7e16066ccb531d00b) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.39-32", 0x000003, 0x100000, CRC(536bc349) SHA1(06d7ac38b2c8cdc85e2cb531bba9c836e50c8247) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.31-24", 0x000004, 0x100000, CRC(813d4a31) SHA1(e024f9da2f15a482d8142870baf487297b995ed9) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.23-16", 0x000005, 0x100000, CRC(f881514b) SHA1(a694f90621e2c1569a6a5ed8920838ba5506f72e) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.15-08", 0x000006, 0x100000, CRC(d7634655) SHA1(d7ac83c0fa5d0ec57d096d4d704fe99ee8160e09) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr0.07-00", 0x000007, 0x100000, CRC(3fca32a3) SHA1(22753a9678e04d9355238e013e58d9f45315579d) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.63-56", 0x800000, 0x100000, CRC(a2b89d3a) SHA1(9cfcd0b88dea192ba39efcdccc78d1a0fd8f3388) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.55-48", 0x800001, 0x100000, CRC(766822a8) SHA1(2c9b14542a5467c1a3451559ea296da09c2cfdb9) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.47-40", 0x800002, 0x100000, CRC(112b519c) SHA1(f0e1ed1b8ad271fa9708f513b11d5cca6e550668) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.39-32", 0x800003, 0x100000, CRC(435b5d37) SHA1(ecb6e7271d993f8e315b85e69166838e66dd41a8) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.31-24", 0x800004, 0x100000, CRC(2637ae7f) SHA1(5e0bd0e08d8c1eaae725b4d55030c2698abd46e7) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.23-16", 0x800005, 0x100000, CRC(e732f1bf) SHA1(a228aee0cc36a0089716f20bfa75d87750692adb) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.15-08", 0x800006, 0x100000, CRC(7d4e2d9e) SHA1(4cb9b754b7585df4cae6bdd7085a57729d53e643) ) // sldh
	ROM_LOAD64_BYTE( "fish_gr1.07-00", 0x800007, 0x100000, CRC(8ea036af) SHA1(1f9baec6712e0ba0e8a744529e41799217760194) ) // sldh
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void jaguar_state::cojag_common_init(uint16_t gpu_jump_offs, uint16_t spin_pc)
{
	m_is_cojag = true;

	/* copy over the ROM */
	m_is_r3000 = (m_maincpu->type() == R3041);

	/* install synchronization hooks for GPU */
	if (m_is_r3000)
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x04f0b000 + gpu_jump_offs, 0x04f0b003 + gpu_jump_offs, write32s_delegate(*this, FUNC(jaguar_state::gpu_jump_w)));
	else
		m_maincpu->space(AS_PROGRAM).install_write_handler(0xf0b000 + gpu_jump_offs, 0xf0b003 + gpu_jump_offs, write32s_delegate(*this, FUNC(jaguar_state::gpu_jump_w)));
	m_gpu->space(AS_PROGRAM).install_read_handler(0xf03000 + gpu_jump_offs, 0xf03003 + gpu_jump_offs, read32smo_delegate(*this, FUNC(jaguar_state::gpu_jump_r)));
	m_gpu_jump_address = &m_gpu_ram[gpu_jump_offs/4];
	m_gpu_spin_pc = 0xf03000 + spin_pc;
}


void jaguar_state::init_area51a()
{
	m_hacks_enabled = true;
	cojag_common_init(0x5c4, 0x5a0);

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xa02030, 0xa02033, write32s_delegate(*this, FUNC(jaguar_state::area51_main_speedup_w)));
	m_main_speedup = &m_mainram[0x2030/4];
#endif
}


void jaguar_state::init_area51()
{
	m_hacks_enabled = true;
	cojag_common_init(0x0c0, 0x09e);

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	m_main_speedup_max_cycles = 120;
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x100062e8, 0x100062eb, read32smo_delegate(*this, FUNC(jaguar_state::cojagr3k_main_speedup_r)));
	m_main_speedup = &m_mainram[0x62e8/4];
#endif
}

void jaguar_state::init_maxforce()
{
	m_hacks_enabled = true;
	cojag_common_init(0x0c0, 0x09e);

	/* patch the protection */
	memregion("maincpu")->as_u32(0x220/4) = 0x03e00008;

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	m_main_speedup_max_cycles = 120;
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000865c, 0x1000865f, read32smo_delegate(*this, FUNC(jaguar_state::cojagr3k_main_speedup_r)));
	m_main_speedup = &m_mainram[0x865c/4];
#endif
}


void jaguar_state::init_area51mx()
{
	m_hacks_enabled = true;
	cojag_common_init(0x0c0, 0x09e);

	/* patch the protection */
	memregion("maincpu")->as_u32(0x418/4) = 0x4e754e75;

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xa19550, 0xa19557, write32s_delegate(*this, FUNC(jaguar_state::area51mx_main_speedup_w)));
	m_main_speedup = &m_mainram[0x19550/4];
#endif
}


void jaguar_state::init_a51mxr3k()
{
	m_hacks_enabled = true;
	cojag_common_init(0x0c0, 0x09e);

	/* patch the protection */
	memregion("maincpu")->as_u32(0x220/4) = 0x03e00008;

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	m_main_speedup_max_cycles = 120;
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x10006f0c, 0x10006f0f, read32smo_delegate(*this, FUNC(jaguar_state::cojagr3k_main_speedup_r)));
	m_main_speedup = &m_mainram[0x6f0c/4];
#endif
}


void jaguar_state::init_fishfren()
{
	m_hacks_enabled = true;
	cojag_common_init(0x578, 0x554);

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	m_main_speedup_max_cycles = 200;
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x10021b60, 0x10021b63, read32smo_delegate(*this, FUNC(jaguar_state::cojagr3k_main_speedup_r)));
	m_main_speedup = &m_mainram[0x21b60/4];
#endif
}


void jaguar_state::init_freeze_common(offs_t main_speedup_addr)
{
	cojag_common_init(0x0bc, 0x09c);

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	m_main_speedup_max_cycles = 200;
	if (main_speedup_addr != 0) {
		m_maincpu->space(AS_PROGRAM).install_read_handler(main_speedup_addr, main_speedup_addr + 3, read32smo_delegate(*this, FUNC(jaguar_state::cojagr3k_main_speedup_r)));
		m_main_speedup = &m_mainram[(main_speedup_addr - 0x10000000)/4];
	}
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0400d900, 0x0400d900 + 3, read32smo_delegate(*this, FUNC(jaguar_state::main_gpu_wait_r)));
	m_main_gpu_wait = &m_shared_ram[0xd900/4];
#endif
}

void jaguar_state::init_freezeat()  { m_hacks_enabled = true; init_freeze_common(0x1001a9f4); }
void jaguar_state::init_freezeat2() { m_hacks_enabled = true; init_freeze_common(0x1001a8c4); }
void jaguar_state::init_freezeat3() { m_hacks_enabled = true; init_freeze_common(0x1001a134); }
void jaguar_state::init_freezeat4() { m_hacks_enabled = true; init_freeze_common(0x1001a134); }
void jaguar_state::init_freezeat5() { m_hacks_enabled = true; init_freeze_common(0x10019b34); }
void jaguar_state::init_freezeat6() { m_hacks_enabled = true; init_freeze_common(0x10019684); }

void jaguar_state::init_vcircle()
{
	m_hacks_enabled = true;
	cojag_common_init(0x5c0, 0x5a0);

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	m_main_speedup_max_cycles = 50;
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x12005b34, 0x12005b37, read32smo_delegate(*this, FUNC(jaguar_state::cojagr3k_main_speedup_r)));
	m_main_speedup = &m_mainram[0x5b34/4];
	m_main_speedup = &m_mainram2[0x5b34/4]; // FIXME: overwriting immediately after assigning?
#endif
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

/*    YEAR   NAME       PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY    FULLNAME */
CONS( 1993,  jaguar,    0,        0,      jaguar,   jaguar,   jaguar_state,   init_jaguar,   "Atari",   "Jaguar (NTSC)",    MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
CONS( 1995,  jaguarcd,  jaguar,   0,      jaguarcd, jaguar,   jaguarcd_state, init_jaguarcd, "Atari",   "Jaguar CD (NTSC)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

/*    YEAR   NAME       PARENT    MACHINE       INPUT     CLASS         INIT            ROT   COMPANY        FULLNAME */
GAME( 1996, area51,     0,        cojagr3k,     area51,   jaguar_state, init_area51,    ROT0, "Atari Games", "Area 51 (R3000)", 0 )
GAME( 1995, area51t,    area51,   cojag68k,     area51,   jaguar_state, init_area51a,   ROT0, "Atari Games (Time Warner license)", "Area 51 (Time Warner license, Oct 17, 1996)", 0 )
GAME( 1995, area51ta,   area51,   cojag68k,     area51,   jaguar_state, init_area51a,   ROT0, "Atari Games (Time Warner license)", "Area 51 (Time Warner license, Nov 27, 1995)", 0 )
GAME( 1995, area51a,    area51,   cojag68k,     area51,   jaguar_state, init_area51a,   ROT0, "Atari Games", "Area 51 (Atari Games license, Oct 25, 1995)", 0 )
GAME( 1995, fishfren,   0,        cojagr3k_rom, fishfren, jaguar_state, init_fishfren,  ROT0, "Time Warner Interactive", "Fishin' Frenzy (prototype)", 0 )
GAME( 1996, freezeat,   0,        cojagr3k_rom, freezeat, jaguar_state, init_freezeat,  ROT0, "Atari Games", "Freeze (Atari) (prototype, English voice, 96/10/25)", 0 )
GAME( 1996, freezeatjp, freezeat, cojagr3k_rom, freezeat, jaguar_state, init_freezeat,  ROT0, "Atari Games", "Freeze (Atari) (prototype, Japanese voice, 96/10/25)", 0 )
GAME( 1996, freezeat2,  freezeat, cojagr3k_rom, freezeat, jaguar_state, init_freezeat2, ROT0, "Atari Games", "Freeze (Atari) (prototype, 96/10/18)", 0 )
GAME( 1996, freezeat3,  freezeat, cojagr3k_rom, freezeat, jaguar_state, init_freezeat3, ROT0, "Atari Games", "Freeze (Atari) (prototype, 96/10/07)", 0 )
GAME( 1996, freezeat4,  freezeat, cojagr3k_rom, freezeat, jaguar_state, init_freezeat4, ROT0, "Atari Games", "Freeze (Atari) (prototype, 96/10/03)", 0 )
GAME( 1996, freezeat5,  freezeat, cojagr3k_rom, freezeat, jaguar_state, init_freezeat5, ROT0, "Atari Games", "Freeze (Atari) (prototype, 96/09/20, AMOA-96)", 0 )
GAME( 1996, freezeat6,  freezeat, cojagr3k_rom, freezeat, jaguar_state, init_freezeat6, ROT0, "Atari Games", "Freeze (Atari) (prototype, 96/09/07, Jamma-96)", 0 )
GAME( 1996, maxforce,   0,        cojagr3k,     area51,   jaguar_state, init_maxforce,  ROT0, "Atari Games", "Maximum Force v1.05", 0 )
GAME( 1996, maxf_102,   maxforce, cojagr3k,     area51,   jaguar_state, init_maxforce,  ROT0, "Atari Games", "Maximum Force v1.02", 0 )
GAME( 1996, maxf_ng,    maxforce, cojagr3k,     area51,   jaguar_state, init_maxforce,  ROT0, "Atari Games", "Maximum Force (No Gore version)", 0 )
GAME( 1998, area51mx,   0,        cojag68k,     area51,   jaguar_state, init_area51mx,  ROT0, "Atari Games", "Area 51 / Maximum Force Duo v2.0", 0 )
GAME( 1998, a51mxr3k,   area51mx, cojagr3k,     area51,   jaguar_state, init_a51mxr3k,  ROT0, "Atari Games", "Area 51 / Maximum Force Duo (R3000, 2/10/98)", 0 )
GAME( 1998, a51mxr3ka,  area51mx, cojagr3k,     area51,   jaguar_state, init_a51mxr3k,  ROT0, "Atari Games", "Area 51 / Maximum Force Duo (R3000, 2/02/98)", 0 )
GAME( 1996, vcircle,    0,        cojagr3k,     vcircle,  jaguar_state, init_vcircle,   ROT0, "Atari Games", "Vicious Circle (prototype)", 0 )
