// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*
Namco System NB-1

Notes:
- tilemap system is identical to Namco System2

ToDo:
- improve interrupts
- gunbulet force feedback
- music tempo is too fast in Nebulas Ray, JLS V-Shoot and the Great Sluggers games

Main CPU : Motorola 68020 32-bit processor @ 25MHz
Secondary CPUs : C329 + 137 (both custom)
Custom Graphics Chips : GFX:123,145,156,C116 - Motion Objects:C355,187,C347
Sound CPU : C75 (Mitsu M37702 with internal ROM)
PCM Sound chip : C352 (custom)
I/O Chip : 160 (custom)
Board composition : Single board

Known games using this hardware:
- Great Sluggers '93
- Great Sluggers '94
- Gun Bullet / Point Blank
- Nebulas Ray
- Super World Stadium '95
- Super World Stadium '96
- Super World Stadium '97
- The Outfoxies
- Mach Breakers

*****************************************************

Gun Bullet (JPN Ver.)
(c)1994 Namco
KeyCus.:C386

Note: CPU - Main PCB  (NB-1)
      MEM - MEMEXT OBJ8 PCB at J103 on the main PCB
      SPR - MEMEXT SPR PCB at location 5B on the main PCB

          - Namco main PCB:  NB-1  8634961101, (8634963101)
          - MEMEXT OBJ8 PCB:       8635902201, (8635902301)

        * - Surface mounted ROMs
        # - 32 pin DIP Custom IC (16bytes x 16-bit)

Brief hardware overview
-----------------------

Main processor    - MC68EC020FG25 25MHz   (100 pin PQFP)
                  - C329 custom           (100 pin PQFP)
                  - 137  custom PLD       (28 pin NDIP)
                  - C366 Key Custom

Sound processor   - C75  custom
 (PCM)            - C352 custom           (100 pin PQFP)
 (control inputs) - 160  custom           (80 pin PQFP)

GFX               - 123  custom           (80 pin PQFP)
                  - 145  custom           (80 pin PQFP)
                  - 156  custom           (64 pin PQFP)
                  - C116 custom           (80 pin PQFP)

Motion Objects    - C355 custom           (160 pin PQFP)
                  - 187  custom           (160 pin PQFP)
                  - C347 custom           (80 pin PQFP)

PCB Jumper Settings
-------------------

Location      Setting       Alt. Setting
----------------------------------------
  JP1           1M              4M
  JP2           4M              1M
  JP5          <1M              1M
  JP6           8M             >8M
  JP7           4M              1M
  JP8           4M              1M
  JP9           cON             cOFF
  JP10          24M (MHz)       12M (MHz)
  JP11          24M (MHz)       28M (MHz)
  JP12          355             F32

*****************************************************

Super World Stadium '95 (JPN Ver.)
(c)1986-1993 1995 Namco

Namco NB-1 system

KeyCus.:C393

*****************************************************

Super World Stadium '96 (JPN Ver.)
(c)1986-1993 1995 1996 Namco

Namco NB-1 system

KeyCus.:C426

*****************************************************

Super World Stadium '97 (JPN Ver.)
(c)1986-1993 1995 1996 1997 Namco

Namco NB-1 system

KeyCus.:C434

*****************************************************

Great Sluggers Featuring 1994 Team Rosters
(c)1993 1994 Namco / 1994 MLBPA

Namco NB-1 system

KeyCus.:C359

*****************************************************

--------------------------
Nebulasray by NAMCO (1994)
--------------------------
Location      Device        File ID      Checksum
-------------------------------------------------
CPU 13B         27C402      NR1-MPRU       B0ED      [  MAIN PROG  ]
CPU 15B         27C240      NR1-MPRL       90C4      [  MAIN PROG  ]
SPR            27C1024      NR1-SPR0       99A6      [  SOUND PRG  ]
CPU 5M       MB834000B      NR1-SHA0       DD59      [    SHAPE    ]
CPU 8J       MB838000B      NR1-CHR0       22A4      [  CHARACTER  ]
CPU 9J       MB838000B      NR1-CHR1       19D0      [  CHARACTER  ]
CPU 10J      MB838000B      NR1-CHR2       B524      [  CHARACTER  ]
CPU 11J      MB838000B      NR1-CHR3       0AF4      [  CHARACTER  ]
CPU 5J       KM2316000      NR1-VOI0       8C41      [    VOICE    ]
MEM IC1     MT26C1600 *     NR1-OBJ0L      FD7C      [ MOTION OBJL ]
MEM IC3     MT26C1600 *     NR1-OBJ1L      7069      [ MOTION OBJL ]
MEM IC5     MT26C1600 *     NR1-OBJ2L      07DC      [ MOTION OBJL ]
MEM IC7     MT26C1600 *     NR1-OBJ3L      A61E      [ MOTION OBJL ]
MEM IC2     MT26C1600 *     NR1-OBJ0U      44D3      [ MOTION OBJU ]
MEM IC4     MT26C1600 *     NR1-OBJ1U      F822      [ MOTION OBJU ]
MEM IC6     MT26C1600 *     NR1-OBJ2U      DD24      [ MOTION OBJU ]
MEM IC8     MT26C1600 *     NR1-OBJ3U      F750      [ MOTION OBJU ]
CPU 11D        Custom #      C366.BIN      1C93      [  KEYCUSTUM  ]

Note: CPU - Main PCB  (NB-1)
      MEM - MEMEXT OBJ8 PCB at J103 on the main PCB
      SPR - MEMEXT SPR PCB at location 5B on the main PCB

          - Namco main PCB:  NB-1  8634961101, (8634963101)
          - MEMEXT OBJ8 PCB:       8635902201, (8635902301)

        * - Surface mounted ROMs
        # - 32 pin DIP Custom IC (16bytes x 16-bit)

Brief hardware overview
-----------------------

Main processor    - MC68EC020FG25 25MHz   (100 pin PQFP)
                  - C329 custom           (100 pin PQFP)
                  - 137  custom PLD       (28 pin NDIP)
                  - C366 Key Custom

Sound processor   - C75  custom
 (PCM)            - C352 custom           (100 pin PQFP)
 (control inputs) - 160  custom           (80 pin PQFP)

GFX               - 123  custom           (80 pin PQFP)
                  - 145  custom           (80 pin PQFP)
                  - 156  custom           (64 pin PQFP)
                  - C116 custom           (80 pin PQFP)

Motion Objects    - C355 custom           (160 pin PQFP)
                  - 187  custom           (160 pin PQFP)
                  - C347 custom           (80 pin PQFP)

PCB Jumper Settings
-------------------

Location      Setting       Alt. Setting
----------------------------------------
  JP1           1M              4M
  JP2           4M              1M
  JP5          <1M              1M
  JP6           8M             >8M
  JP7           4M              1M
  JP8           4M              1M
  JP9           cON             cOFF
  JP10          24M (MHz)       12M (MHz)
  JP11          24M (MHz)       28M (MHz)
  JP12          355             F32


Namco System NB2

Games running on this hardware:
- The Outfoxies
- Mach Breakers

Changes from Namcon System NB1 include:
- different memory map
- more complex sprite and tile banking
- 2 additional ROZ layers

-----------------------------
The Outfoxies by NAMCO (1994)
-----------------------------
Location            Device     File ID     Checksum
----------------------------------------------------
CPU 11C PRGL       27C4002     OU2-MPRL      166F
CPU 11D PRGU       27C4002     OU2-MPRU      F4C1
CPU 5B  SPR0        27C240     OU1-SPR0      7361
CPU 20A DAT0       27C4002     OU1-DAT0      FCD1
CPU 20B DAT1       27C4002     OU1-DAT1      0973
CPU 18S SHAPE-R    MB83800     OU1-SHAR      C922
CPU 12S SHAPE-S    MB83400     OU1-SHAS      2820
CPU 6N  VOICE0    MB831600     OU1-VOI0      4132
ROM 4C  OBJ0L    16Meg SMD     OU1-OBJ0L     171B
ROM 8C  OBJ0U    16Meg SMD     OU1-OBJ0U     F961
ROM 4B  OBJ1L    16Meg SMD     OU1-OBJ1L     1579
ROM 8B  OBJ1U    16Meg SMD     OU1-OBJ1U     E8DF
ROM 4A  OBJ2L    16Meg SMD     OU1-OBJ2L     AE7B
ROM 8A  OBJ2U    16Meg SMD     OU1-OBJ2U     6588
ROM 6C  OBJ3L    16Meg SMD     OU1-OBJ3L     9ED3
ROM 9C  OBJ3U    16Meg SMD     OU1-OBJ3U     ED3B
ROM 6B  OBJ4L    16Meg SMD     OU1-OBJ4L     59D4
ROM 9B  OBJ4U    16Meg SMD     OU1-OBJ4U     56CA
ROM 3D  ROT0     16Meg SMD     OU1-ROT0      A615
ROM 3C  ROT1     16Meg SMD     OU1-ROT1      6C0A
ROM 3B  ROT2     16Meg SMD     OU1-ROT2      313E
ROM 1D  SCR0     16Meg SMD     OU1-SCR0      751A

CPU 8B  DEC75     PAL16L8A        NB1-2
CPU 16N MIXER     PAL16V8H        NB2-1
CPU 11E SIZE      PAL16L8A        NB2-2
CPU 22C KEYCUS   KeyCustom         C390

CPU  -  Namco NB-2 Main PCB        8639960102 (8639970102)
ROM  -  Namco NB-2 Mask ROM PCB    8639969800 (8639979800)

     -  Audio out is Stereo

Jumper Settings:

     Setting     Alternate
JP1    4M           1M
JP2    GND          A20
JP3    GND          A20
JP6    4M           1M
JP8    GND          A20
JP9    CON          COFF
JP10   GND          A20

Hardware info:

Main CPU:           MC68EC020FG25
                    Custom C383    (100 pin PQFP)
                    Custom C385    (144 pin PQFP)

Slave CPU:         ?Custom C382    (160 pin PQFP)
                    Custom 160     ( 80 pin PQFP)
                    Custom C352    (100 pin PQFP)

GFX:                Custom 145     ( 80 pin PQFP)
                    Custom 156     ( 64 pin PQFP)
                    Custom 123     ( 64 pin PQFP)
                 3x Custom 384     ( 48 pin PQFP)
                    Custom C355    (160 pin PQFP)
                    Custom 187     (120 pin PQFP)
                    Custom 169     (120 pin PQFP)
*/
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/namconb1.h"
#include "includes/namcoic.h"
#include "machine/namcomcu.h"
#include "sound/c352.h"

#define MASTER_CLOCK    XTAL_48_384MHz


/****************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(namconb1_state::namconb_scantimer)
{
	int scanline = param;

	// Handle VBLANK
	if (scanline == NAMCONB1_VBSTART)
	{
		if (m_vbl_irq_level != 0)
			m_maincpu->set_input_line(m_vbl_irq_level, ASSERT_LINE);
	}

	// Handle POSIRQ
	UINT32 posirq_scanline = m_c116->get_reg(5) - 32;

	if (scanline == posirq_scanline)
	{
		m_screen->update_partial(posirq_scanline);

		if (m_pos_irq_level != 0)
			m_maincpu->set_input_line(m_pos_irq_level, ASSERT_LINE);
	}

	// TODO: Real sources of these
	if (scanline == 224)
		m_mcu->set_input_line(M37710_LINE_IRQ0, HOLD_LINE);
	else if (scanline == 0)
		m_mcu->set_input_line(M37710_LINE_IRQ2, HOLD_LINE);
	else if (scanline == 128)
		m_mcu->set_input_line(M37710_LINE_ADC, HOLD_LINE);
}



/****************************************************************************/

WRITE8_MEMBER(namconb1_state::namconb1_cpureg_w)
{
	/**
	 * 400000 0x00
	 * 400001 POS IRQ enable/level
	 * 400002 ??? IRQ enable/level
	 * 400003 0x00
	 * 400004 VBL IRQ enable/level
	 * 400005 0x00
	 * 400006 POS IRQ ack
	 * 400007 ??? IRQ ack
	 * 400008 0x00
	 * 400009 VBL IRQ ack
	 * 40000a ??? (0x00)
	 * 40000b ??? (0x03)
	 * 40000c ??? (0x07)
	 * 40000d ??? (0x01)
	 * 40000e ??? (0x10)
	 * 40000f ??? (0x03)
	 * 400010 ??? (0x00)
	 * 400011 ??? (0x07)
	 * 400012 ??? (0x10)
	 * 400013 ??? (0x10)
	 * 400014 ??? (0x00)
	 * 400015 ??? (0x01)
	 * 400016 Watchdog
	 * 400017 ??? (0x00)
	 * 400018 C75 Control
	 * 400019 ??? (0x00)
	 * 40001a ??? (0x00)
	 * 40001b ??? (0x00)
	 * 40001c ??? (0x00)
	 * 40001d ??? (0x00)
	 * 40001e ??? (0x00)
	 * 40001f ??? (0x00)
	 */
	switch (offset)
	{
		case 0x01:
			// Bits 5-4 unknown
			m_maincpu->set_input_line(m_pos_irq_level, CLEAR_LINE);
			m_pos_irq_level = data & 0x0f;
			break;

		case 0x02:
			m_maincpu->set_input_line(m_unk_irq_level, CLEAR_LINE);
			m_unk_irq_level = data & 0x0f;
			break;

		case 0x04:
			m_maincpu->set_input_line(m_vbl_irq_level, CLEAR_LINE);
			m_vbl_irq_level = data & 0x0f;
			break;

		case 0x06:
			m_maincpu->set_input_line(m_pos_irq_level, CLEAR_LINE);
			break;

		case 0x07:
			m_maincpu->set_input_line(m_unk_irq_level, CLEAR_LINE);
			break;

		case 0x09:
			m_maincpu->set_input_line(m_vbl_irq_level, CLEAR_LINE);
			break;

		case 0x16:
			break;

		case 0x18:
			if (data & 1)
			{
				m_mcu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				m_mcu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				m_mcu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			else
				m_mcu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			break;

		default:
			logerror("Unhandled CPU reg write to [0x%.2x] with 0x%.2x (PC=0x%x)\n", offset, data, space.device().safe_pc());
	}
}


WRITE8_MEMBER(namconb1_state::namconb2_cpureg_w)
{
	/**
	 * f00000 VBL IRQ enable/level
	 * f00001 ??? IRQ enable/level
	 * f00002 POS IRQ enable/level
	 * f00003 ??? (0x00)
	 * f00004 VBL IRQ ack
	 * f00005 ??? IRQ ack
	 * f00006 POS IRQ ack
	 * f00007
	 * f00008
	 * f00009 ??? (0x62)
	 * f0000a ??? (0x0f)
	 * f0000b ??? (0x41)
	 * f0000c ??? (0x70)
	 * f0000d ??? (0x70)
	 * f0000e ??? (0x23)
	 * f0000f ??? (0x50)
	 * f00010 ??? (0x00)
	 * f00011 ??? (0x64)
	 * f00012 ??? (0x18)
	 * f00013 ??? (0xe7)
	 * f00014 Watchdog
	 * f00015
	 * f00016 C75 Control
	 * f00017
	 * f00018
	 * f00019
	 * f0001a
	 * f0001b
	 * f0001c
	 * f0001d
	 * f0001e ??? (0x00)
	 * f0001f ??? (0x01)
	 */
	switch (offset)
	{
		case 0x00:
			m_maincpu->set_input_line(m_vbl_irq_level, CLEAR_LINE);
			m_vbl_irq_level = data & 0x0f;
			break;

		case 0x01:
			m_maincpu->set_input_line(m_unk_irq_level, CLEAR_LINE);
			m_unk_irq_level = data & 0x0f;
			break;

		case 0x02:
			m_maincpu->set_input_line(m_pos_irq_level, CLEAR_LINE);
			m_pos_irq_level = data & 0x0f;
			break;

		case 0x04:
			m_maincpu->set_input_line(m_vbl_irq_level, CLEAR_LINE);
			break;

		case 0x05:
			m_maincpu->set_input_line(m_unk_irq_level, CLEAR_LINE);
			break;

		case 0x06:
			m_maincpu->set_input_line(m_pos_irq_level, CLEAR_LINE);
			break;

		case 0x14:
			break;

		case 0x16:
			if (data & 1)
			{
				m_mcu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				m_mcu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				m_mcu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			else
			{
				m_mcu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			}
			break;

		default:
			logerror("Unhandled CPU reg write to [0x%.2x] with 0x%.2x (PC=0x%x)\n", offset, data, space.device().safe_pc());
	}
}


READ8_MEMBER(namconb1_state::namconb1_cpureg_r)
{
	// 16: Watchdog
	if (offset != 0x16)
		logerror("Unhandled CPU reg read from [0x%.2x] (PC=0x%x)\n", offset, space.device().safe_pc());

	return 0xff;
}


READ8_MEMBER(namconb1_state::namconb2_cpureg_r)
{
	// 14: Watchdog
	if (offset != 0x14)
		logerror("Unhandled CPU reg read from [0x%.2x] (PC=0x%x)\n", offset, space.device().safe_pc());

	return 0xff;
}


/****************************************************************************/

READ32_MEMBER(namconb1_state::custom_key_r)
{
	UINT16 old_count = m_count;

	do
	{ /* pick a random number, but don't pick the same twice in a row */
		m_count = machine().rand();
	} while( m_count==old_count );

	switch (m_gametype)
	{
	/*
	    Gunbullet/Point Blank keycus notes (thanks Guru):

	    These games use the keycus in an unconventional way.  Instead of reading it for a PRNG or a
	    magic value, it writes a scratch value to the keycus once per frame.

	    On hardware, if there is no keycus or the wrong keycus is present, this write will stall the
	    68000 (probably nothing completes the bus cycle in that case) and the game will hang instead
	    of booting.

	    Patching these writes out causes the game to run fine with no keycus present.
	*/
	case NAMCONB1_GUNBULET:
		return 0;

	case NAMCONB1_SWS95:
		switch (offset)
		{
			case 0: return 0x0189;
			case 1: return  m_count<<16;
		}
		break;

	case NAMCONB1_SWS96:
		switch (offset)
		{
			case 0: return 0x01aa<<16;
			case 4: return m_count<<16;
		}
		break;

	case NAMCONB1_SWS97:
		switch (offset)
		{
			case 2: return 0x1b2<<16;
			case 5: return m_count<<16;
		}
		break;

	case NAMCONB1_GSLGR94U:
		switch (offset)
		{
			case 0: return 0x0167;
			case 1: return m_count<<16;
		}
		break;

	case NAMCONB1_GSLGR94J:
		switch (offset)
		{
		case 1: return 0;
		case 3: return (0x0171<<16) | m_count;
		}
		break;

	case NAMCONB1_NEBULRAY:
		switch (offset)
		{
			case 1: return 0x016e;
			case 3: return m_count;
		}
		break;

	case NAMCONB1_VSHOOT:
		switch (offset)
		{
			case 2: return m_count<<16;
			case 3: return 0x0170<<16;
		}
		break;

	case NAMCONB2_OUTFOXIES:
		switch (offset)
		{
			case 0: return 0x0186;
			case 1: return m_count<<16;
		}
		break;

	case NAMCONB2_MACH_BREAKERS:
		break; /* no protection? */
	}

	logerror( "custom_key_r(%d); pc=%08x\n", offset, space.device().safe_pc() );
	return 0;
} /* custom_key_r */

/***************************************************************/


READ32_MEMBER(namconb1_state::gunbulet_gun_r)
{
	int result = 0;

	switch (offset)
	{
		case 0: case 1: result = (UINT8)(0x0f + ioport("LIGHT1_Y")->read() * 224/255); break; /* Y (p2) */
		case 2: case 3: result = (UINT8)(0x26 + ioport("LIGHT1_X")->read() * 288/314); break; /* X (p2) */
		case 4: case 5: result = (UINT8)(0x0f + ioport("LIGHT0_Y")->read() * 224/255); break; /* Y (p1) */
		case 6: case 7: result = (UINT8)(0x26 + ioport("LIGHT0_X")->read() * 288/314); break; /* X (p1) */
	}
	return result<<24;
} /* gunbulet_gun_r */

READ32_MEMBER(namconb1_state::randgen_r)
{
	return machine().rand();
} /* randgen_r */

WRITE32_MEMBER(namconb1_state::srand_w)
{
	/**
	 * Used to seed the hardware random number generator.
	 * We don't yet know the algorithm that is used, so for now this is a NOP.
	 */
} /* srand_w */

READ32_MEMBER(namconb1_state::namconb_share_r)
{
	return (m_namconb_shareram[offset*2] << 16) | m_namconb_shareram[offset*2+1];
}

WRITE32_MEMBER(namconb1_state::namconb_share_w)
{
	COMBINE_DATA(m_namconb_shareram+offset*2+1);
	data >>= 16;
	mem_mask >>= 16;
	COMBINE_DATA(m_namconb_shareram+offset*2);
}

static ADDRESS_MAP_START( namconb1_am, AS_PROGRAM, 32, namconb1_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10001f) AM_READ(gunbulet_gun_r)
	AM_RANGE(0x1c0000, 0x1cffff) AM_RAM
	AM_RANGE(0x1e4000, 0x1e4003) AM_READWRITE(randgen_r,srand_w)
	AM_RANGE(0x200000, 0x207fff) AM_READWRITE(namconb_share_r, namconb_share_w)
	AM_RANGE(0x208000, 0x2fffff) AM_RAM
	AM_RANGE(0x400000, 0x40001f) AM_READWRITE8(namconb1_cpureg_r, namconb1_cpureg_w, 0xffffffff)
	AM_RANGE(0x580000, 0x5807ff) AM_DEVREADWRITE8("eeprom", eeprom_parallel_28xx_device, read, write, 0xffffffff)
	AM_RANGE(0x600000, 0x61ffff) AM_READWRITE16(c355_obj_ram_r,c355_obj_ram_w,0xffffffff) AM_SHARE("objram")
	AM_RANGE(0x620000, 0x620007) AM_READWRITE16(c355_obj_position_r,c355_obj_position_w,0xffffffff)
	AM_RANGE(0x640000, 0x64ffff) AM_READWRITE16(c123_tilemap_videoram_r,c123_tilemap_videoram_w,0xffffffff)
	AM_RANGE(0x660000, 0x66003f) AM_READWRITE16(c123_tilemap_control_r,c123_tilemap_control_w,0xffffffff)
	AM_RANGE(0x680000, 0x68000f) AM_RAM AM_SHARE("spritebank32")
	AM_RANGE(0x6e0000, 0x6e001f) AM_READ(custom_key_r) AM_WRITENOP
	AM_RANGE(0x700000, 0x707fff) AM_DEVREADWRITE8("c116", namco_c116_device, read, write, 0xffffffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( namconb2_am, AS_PROGRAM, 32, namconb1_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x1c0000, 0x1cffff) AM_RAM
	AM_RANGE(0x1e4000, 0x1e4003) AM_READWRITE(randgen_r,srand_w)
	AM_RANGE(0x200000, 0x207fff) AM_READWRITE(namconb_share_r, namconb_share_w)
	AM_RANGE(0x208000, 0x2fffff) AM_RAM
	AM_RANGE(0x400000, 0x4fffff) AM_ROM AM_REGION("data", 0)
	AM_RANGE(0x600000, 0x61ffff) AM_READWRITE16(c355_obj_ram_r,c355_obj_ram_w,0xffffffff) AM_SHARE("objram")
	AM_RANGE(0x620000, 0x620007) AM_READWRITE16(c355_obj_position_r,c355_obj_position_w,0xffffffff)
	AM_RANGE(0x640000, 0x64000f) AM_RAM /* unknown xy offset */
	AM_RANGE(0x680000, 0x68ffff) AM_READWRITE16(c123_tilemap_videoram_r,c123_tilemap_videoram_w,0xffffffff)
	AM_RANGE(0x6c0000, 0x6c003f) AM_READWRITE16(c123_tilemap_control_r,c123_tilemap_control_w,0xffffffff)
	AM_RANGE(0x700000, 0x71ffff) AM_READWRITE16(c169_roz_videoram_r,c169_roz_videoram_w,0xffffffff) AM_SHARE("rozvideoram")
	AM_RANGE(0x740000, 0x74001f) AM_READWRITE16(c169_roz_control_r,c169_roz_control_w,0xffffffff)
	AM_RANGE(0x800000, 0x807fff) AM_DEVREADWRITE8("c116", namco_c116_device, read, write, 0xffffffff)
	AM_RANGE(0x900008, 0x90000f) AM_RAM AM_SHARE("spritebank32")
	AM_RANGE(0x940000, 0x94000f) AM_RAM AM_SHARE("tilebank32")
	AM_RANGE(0x980000, 0x98000f) AM_READWRITE16(c169_roz_bank_r,c169_roz_bank_w,0xffffffff)
	AM_RANGE(0xa00000, 0xa007ff) AM_DEVREADWRITE8("eeprom", eeprom_parallel_28xx_device, read, write, 0xffffffff)
	AM_RANGE(0xc00000, 0xc0001f) AM_READ(custom_key_r) AM_WRITENOP
	AM_RANGE(0xf00000, 0xf0001f) AM_READWRITE8(namconb1_cpureg_r, namconb2_cpureg_w, 0xffffffff)
ADDRESS_MAP_END

WRITE16_MEMBER(namconb1_state::nbmcu_shared_w)
{
	// HACK!  Many games data ROM routines redirect the vector from the sound command read to an RTS.
	// This needs more investigation.  nebulray and vshoot do NOT do this.
	// Timers A2 and A3 are set up in "external input counter" mode, this may be related.
#if 0
	if ((offset == 0x647c/2) && (data != 0))
	{
		data = 0xd2f6;
	}
#endif

	COMBINE_DATA(&m_namconb_shareram[offset]);

	// C74 BIOS has a very short window on the CPU sync signal, so immediately let the '020 at it
	if ((offset == 0x6000/2) && (data & 0x80))
	{
		space.device().execute().spin_until_time(downcast<cpu_device *>(&space.device())->cycles_to_attotime(300)); // was 300
	}
}

static ADDRESS_MAP_START( namcoc75_am, AS_PROGRAM, 16, namconb1_state )
	AM_RANGE(0x002000, 0x002fff) AM_DEVREADWRITE("c352", c352_device, read, write)
	AM_RANGE(0x004000, 0x00bfff) AM_RAM_WRITE(nbmcu_shared_w) AM_SHARE("namconb_share")
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("c75data", 0)
ADDRESS_MAP_END


READ8_MEMBER(namconb1_state::port6_r)
{
	return m_nbx_port6;
}

WRITE8_MEMBER(namconb1_state::port6_w)
{
	m_nbx_port6 = data;
}

READ8_MEMBER(namconb1_state::port7_r)
{
	switch (m_nbx_port6 & 0xf0)
	{
		case 0x00:
			return read_safe(ioport("P4"), 0xff);

		case 0x20:
			return ioport("MISC")->read();

		case 0x40:
			return ioport("P1")->read();

		case 0x60:
			return ioport("P2")->read();

		default:
			break;
	}

	return 0xff;
}

// Is this madness?  No, this is Namco.  They didn't have enough digital ports for all 4 players,
// so the 8 bits of player 3 got routed to the 8 analog inputs.  +5V on the analog input will
// register full scale, so it works...
READ8_MEMBER(namconb1_state::dac7_r)// bit 7
{
	return read_safe(ioport("P3"), 0xff)&0x80;
}

READ8_MEMBER(namconb1_state::dac6_r)// bit 3
{
	return (read_safe(ioport("P3"), 0xff)<<1)&0x80;
}

READ8_MEMBER(namconb1_state::dac5_r)// bit 2
{
	return (read_safe(ioport("P3"), 0xff)<<2)&0x80;
}

READ8_MEMBER(namconb1_state::dac4_r)// bit 1
{
	return (read_safe(ioport("P3"), 0xff)<<3)&0x80;
}

READ8_MEMBER(namconb1_state::dac3_r)// bit 0
{
	return (read_safe(ioport("P3"), 0xff)<<4)&0x80;
}

READ8_MEMBER(namconb1_state::dac2_r)// bit 4
{
	return (read_safe(ioport("P3"), 0xff)<<5)&0x80;
}

READ8_MEMBER(namconb1_state::dac1_r)// bit 5
{
	return (read_safe(ioport("P3"), 0xff)<<6)&0x80;
}

READ8_MEMBER(namconb1_state::dac0_r)// bit 6
{
	return (read_safe(ioport("P3"), 0xff)<<7)&0x80;
}

static ADDRESS_MAP_START( namcoc75_io, AS_IO, 8, namconb1_state )
	AM_RANGE(M37710_PORT6, M37710_PORT6) AM_READWRITE(port6_r, port6_w)
	AM_RANGE(M37710_PORT7, M37710_PORT7) AM_READ(port7_r)
	AM_RANGE(M37710_ADC7_L, M37710_ADC7_L) AM_READ(dac7_r)
	AM_RANGE(M37710_ADC6_L, M37710_ADC6_L) AM_READ(dac6_r)
	AM_RANGE(M37710_ADC5_L, M37710_ADC5_L) AM_READ(dac5_r)
	AM_RANGE(M37710_ADC4_L, M37710_ADC4_L) AM_READ(dac4_r)
	AM_RANGE(M37710_ADC3_L, M37710_ADC3_L) AM_READ(dac3_r)
	AM_RANGE(M37710_ADC2_L, M37710_ADC2_L) AM_READ(dac2_r)
	AM_RANGE(M37710_ADC1_L, M37710_ADC1_L) AM_READ(dac1_r)
	AM_RANGE(M37710_ADC0_L, M37710_ADC0_L) AM_READ(dac0_r)
ADDRESS_MAP_END


/****************************************************************************/

static INPUT_PORTS_START( gunbulet )
	PORT_START("MISC")
	PORT_DIPUNUSED_DIPLOC(0x01, 0x01, "SW1: 2")
	PORT_SERVICE_DIPLOC(0x02, 0x02, "SW1: 1")

	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x40, 0x40, "Test switch" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("P1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(4)
	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(4)
	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(4) PORT_PLAYER(2)
	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(4) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( outfxies )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("MISC")
	PORT_DIPNAME( 0x01, 0x01, "Freeze Screen" ) PORT_DIPLOCATION("SW1: 2")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x02, 0x02, "SW1: 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

static INPUT_PORTS_START( nbsports )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("MISC")
	PORT_DIPNAME( 0x01, 0x01, "Freeze Screen" ) PORT_DIPLOCATION("SW1: 2")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x02, 0x02, "SW1: 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

static INPUT_PORTS_START( namconb1 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("MISC")
	PORT_DIPNAME( 0x01, 0x01, "Freeze Screen" ) PORT_DIPLOCATION("SW1: 2")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x02, 0x02, "SW1: 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_DIPNAME( 0x40, 0x40, "Test switch" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END


/****************************************************************************/

DRIVER_INIT_MEMBER(namconb1_state,nebulray)
{
	m_gametype = NAMCONB1_NEBULRAY;
} /* nebulray */

DRIVER_INIT_MEMBER(namconb1_state,gslgr94u)
{
	m_gametype = NAMCONB1_GSLGR94U;
} /* gslgr94u */

DRIVER_INIT_MEMBER(namconb1_state,gslgr94j)
{
	m_gametype = NAMCONB1_GSLGR94J;
} /* gslgr94j */

DRIVER_INIT_MEMBER(namconb1_state,sws95)
{
	m_gametype = NAMCONB1_SWS95;
} /* sws95 */

DRIVER_INIT_MEMBER(namconb1_state,sws96)
{
	m_gametype = NAMCONB1_SWS96;
} /* sws96 */

DRIVER_INIT_MEMBER(namconb1_state,sws97)
{
	m_gametype = NAMCONB1_SWS97;
} /* sws97 */

DRIVER_INIT_MEMBER(namconb1_state,gunbulet)
{
	m_gametype = NAMCONB1_GUNBULET;
} /* gunbulet */

DRIVER_INIT_MEMBER(namconb1_state,vshoot)
{
	m_gametype = NAMCONB1_VSHOOT;
} /* vshoot */

DRIVER_INIT_MEMBER(namconb1_state,machbrkr)
{
	m_gametype = NAMCONB2_MACH_BREAKERS;
}

DRIVER_INIT_MEMBER(namconb1_state,outfxies)
{
	m_gametype = NAMCONB2_OUTFOXIES;
}

/***************************************************************/

static const gfx_layout obj_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8, /* bits per pixel */
	{
		/* plane offsets */
		0,1,2,3,4,5,6,7,
	},
	{
		0*16+8,1*16+8,0*16,1*16,
		2*16+8,3*16+8,2*16,3*16,
		4*16+8,5*16+8,4*16,5*16,
		6*16+8,7*16+8,6*16,7*16
	},
	{
		0x0*128,0x1*128,0x2*128,0x3*128,0x4*128,0x5*128,0x6*128,0x7*128,
		0x8*128,0x9*128,0xa*128,0xb*128,0xc*128,0xd*128,0xe*128,0xf*128
	},
	16*128
}; /* obj_layout */

static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	{ 0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64 },
	8*64
}; /* tile_layout */

static const gfx_layout roz_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	{
		0*128,1*128,2*128,3*128,4*128,5*128,6*128,7*128,8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128
	},
	16*128
}; /* roz_layout */

static GFXDECODE_START( namconb1 )
	GFXDECODE_ENTRY( NAMCONB1_TILEGFXREGION,    0, tile_layout, 0x1000, 0x10 )
	GFXDECODE_ENTRY( NAMCONB1_SPRITEGFXREGION,  0, obj_layout,  0x0000, 0x10 )
GFXDECODE_END /* gfxdecodeinfo */

static GFXDECODE_START( 2 )
	GFXDECODE_ENTRY( NAMCONB1_TILEGFXREGION,    0, tile_layout, 0x1000, 0x08 )
	GFXDECODE_ENTRY( NAMCONB1_SPRITEGFXREGION,  0, obj_layout,  0x0000, 0x10 )
	GFXDECODE_ENTRY( NAMCONB1_ROTGFXREGION, 0, roz_layout,      0x1800, 0x08 )
GFXDECODE_END /* gfxdecodeinfo2 */


/***************************************************************/

MACHINE_RESET_MEMBER(namconb1_state, namconb)
{
	m_pos_irq_level = 0;
	m_unk_irq_level = 0;
	m_vbl_irq_level = 0;
}


/***************************************************************/

static MACHINE_CONFIG_START( namconb1, namconb1_state )
	MCFG_CPU_ADD("maincpu", M68EC020, MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(namconb1_am)

	MCFG_CPU_ADD("mcu", NAMCO_C75, MASTER_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(namcoc75_am)
	MCFG_CPU_IO_MAP(namcoc75_io)

	MCFG_EEPROM_2816_ADD("eeprom")
	MCFG_MACHINE_RESET_OVERRIDE(namconb1_state, namconb)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", namconb1_state, namconb_scantimer, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.7)
	MCFG_SCREEN_SIZE(NAMCONB1_HTOTAL, NAMCONB1_VTOTAL)
	MCFG_SCREEN_VISIBLE_AREA(0, NAMCONB1_HBSTART-1, 0, NAMCONB1_VBSTART-1)
	MCFG_SCREEN_UPDATE_DRIVER(namconb1_state, screen_update_namconb1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", namconb1)
	MCFG_PALETTE_ADD("palette", 0x2000)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_DEVICE_ADD("c116", NAMCO_C116, 0)
	MCFG_GFX_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(namconb1_state,namconb1)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_C352_ADD("c352", MASTER_CLOCK/2, 288)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( namconb2, namconb1_state )
	MCFG_CPU_ADD("maincpu", M68EC020, MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(namconb2_am)

	MCFG_CPU_ADD("mcu", NAMCO_C75, MASTER_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(namcoc75_am)
	MCFG_CPU_IO_MAP(namcoc75_io)

	MCFG_EEPROM_2816_ADD("eeprom")
	MCFG_MACHINE_RESET_OVERRIDE(namconb1_state, namconb)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", namconb1_state, namconb_scantimer, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.7)
	MCFG_SCREEN_SIZE(NAMCONB1_HTOTAL, NAMCONB1_VTOTAL)
	MCFG_SCREEN_VISIBLE_AREA(0, NAMCONB1_HBSTART-1, 0, NAMCONB1_VBSTART-1)
	MCFG_SCREEN_UPDATE_DRIVER(namconb1_state, screen_update_namconb2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 2)
	MCFG_PALETTE_ADD("palette", 0x2000)
	MCFG_PALETTE_ENABLE_SHADOWS()

	MCFG_DEVICE_ADD("c116", NAMCO_C116, 0)
	MCFG_GFX_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(namconb1_state,namconb2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_C352_ADD("c352", MASTER_CLOCK/2, 288)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_CONFIG_END


/***************************************************************/

ROM_START( ptblank ) /* World set using 4Mb sound data rom (verified) */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "gn2_mprlb.15b", 0x00002, 0x80000, CRC(fe2d9425) SHA1(51b166a629cbb522720d63720558816b496b6b76) )
	ROM_LOAD32_WORD( "gn2_mprub.13b", 0x00000, 0x80000, CRC(3bf4985a) SHA1(f559e0d5f55d23d886fe61bd7d5ca556acc7f87c) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data - JP1 jumper selectable between 1Mb (27C1024) or 4Mb (27C4096) either rom is correct */
//  ROM_LOAD( "gn1_spr0.5b", 0, 0x20000, CRC(6836ba38) SHA1(6ea17ea4bbb59be108e8887acd7871409580732f) ) /* 1Megabit, same data as the 4Mb rom at 0x00000-0x1ffff */
	ROM_LOAD( "gn1-spr0.5b", 0, 0x80000, CRC(71773811) SHA1(e482784d9b9ebf8c2e4a2a3f6f6c4dc8304d2251) ) /* 4Megabit, same data at 0x00000-0x1ffff, 0x20000-0x7ffff is 0xff filled */

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "gn1-voi0.5j", 0, 0x200000, CRC(05477eb7) SHA1(f2eaacb5dbac06c37c56b9b131230c9cf6602221) )

	ROM_REGION( 0x800000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "gn1obj0l.ic1", 0x000001, 0x200000, CRC(06722dc8) SHA1(56fee4e17ed707fa6dbc6bad0d0281fc8cdf72d1) ) /* These four located on MEMEXT OBJ8 PCB daughter-card */
	ROM_LOAD16_BYTE( "gn1obj0u.ic2", 0x000000, 0x200000, CRC(fcefc909) SHA1(48c19b6032096dd80777aa6d5eb5f90463095cbe) )
	ROM_LOAD16_BYTE( "gn1obj1l.ic3", 0x400001, 0x200000, CRC(48468df7) SHA1(c5fb9082c84ac2ffceb6f5f4cbc1d40047c55e3d) )
	ROM_LOAD16_BYTE( "gn1obj1u.ic4", 0x400000, 0x200000, CRC(3109a071) SHA1(4bb16df5a3aecdf37baf843edfc82952d46f5227) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "gn1-chr0.8j",  0x000000, 0x100000, CRC(a5c61246) SHA1(d1d9f286b93b5b9880160029c53384d13c08dd8a) )
	ROM_LOAD( "gn1-chr1.9j",  0x100000, 0x100000, CRC(c8c59772) SHA1(91de633a300e3b25a919579eaada5549640ab6f0) )
	ROM_LOAD( "gn1-chr2.10j", 0x200000, 0x100000, CRC(dc96d999) SHA1(d006a401762b57fef6716f56eb3a7edcb3d3c00e) )
	ROM_LOAD( "gn1-chr3.11j", 0x300000, 0x100000, CRC(4352c308) SHA1(785c13df219dceac2f940519141665b630a29f86) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "gn1-sha0.5m", 0, 0x80000, CRC(86d4ff85) SHA1(a71056b2bcbba50c834fe28269ebda9719df354a) )

	ROM_REGION( 0x0800, "eeprom", 0 ) // default gun calibration and settings
	ROM_LOAD( "eeprom", 0x0000, 0x0800, CRC(95760d0f) SHA1(94ac5a261d9afc77c2a163a50950b0e86b1f8041) )
ROM_END

ROM_START( ptblanka ) /* World set using non standard ROM labels (NR is Namco's prefix for Nebulas Ray, but NOT here!!), verified on 2 seperate PCBs */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "nr3_spr0.15b", 0x00002, 0x80000, CRC(fe2d9425) SHA1(51b166a629cbb522720d63720558816b496b6b76) ) // == gn2_mprlb.15b
	ROM_LOAD32_WORD( "nr2_spr0.13b", 0x00000, 0x80000, CRC(3bf4985a) SHA1(f559e0d5f55d23d886fe61bd7d5ca556acc7f87c) ) // == gn2_mprub.12b

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data - JP1 jumper selectable between 1Mb (27C1024) or 4Mb (27C4096) either rom is correct */
//  ROM_LOAD( "nr1_spr0.5b", 0, 0x20000, CRC(6836ba38) SHA1(6ea17ea4bbb59be108e8887acd7871409580732f) ) /* 1Megabit, same data as the 4Mb rom at 0x00000-0x1ffff */
	ROM_LOAD( "nr1_spr0.5b", 0, 0x80000, CRC(a0bde3fb) SHA1(b5fac1d0339b1df6b8880fcd7aa2725a774765a4) ) /* 4Megabit, same data at 0x00000-0x1ffff repeated 4 time */

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "nr4_spr0.5j", 0, 0x200000, CRC(05477eb7) SHA1(f2eaacb5dbac06c37c56b9b131230c9cf6602221) ) // == gn1-voi0.5j

	ROM_REGION( 0x800000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "gn1obj0l.ic1", 0x000001, 0x200000, CRC(06722dc8) SHA1(56fee4e17ed707fa6dbc6bad0d0281fc8cdf72d1) ) /* These four located on MEMEXT OBJ8 PCB daughter-card */
	ROM_LOAD16_BYTE( "gn1obj0u.ic2", 0x000000, 0x200000, CRC(fcefc909) SHA1(48c19b6032096dd80777aa6d5eb5f90463095cbe) )
	ROM_LOAD16_BYTE( "gn1obj1l.ic3", 0x400001, 0x200000, CRC(48468df7) SHA1(c5fb9082c84ac2ffceb6f5f4cbc1d40047c55e3d) )
	ROM_LOAD16_BYTE( "gn1obj1u.ic4", 0x400000, 0x200000, CRC(3109a071) SHA1(4bb16df5a3aecdf37baf843edfc82952d46f5227) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "nr5_spr0.8j",  0x000000, 0x100000, CRC(a5c61246) SHA1(d1d9f286b93b5b9880160029c53384d13c08dd8a) ) // == gn1_chr0.8j
	ROM_LOAD( "nr6_spr0.9j",  0x100000, 0x100000, CRC(c8c59772) SHA1(91de633a300e3b25a919579eaada5549640ab6f0) ) // == gn1_chr1.8j
	ROM_LOAD( "nr7_spr0.10j", 0x200000, 0x100000, CRC(dc96d999) SHA1(d006a401762b57fef6716f56eb3a7edcb3d3c00e) ) // == gn1_chr2.10j
	ROM_LOAD( "nr8_spr0.11j", 0x300000, 0x100000, CRC(4352c308) SHA1(785c13df219dceac2f940519141665b630a29f86) ) // == gn1_chr3.11j

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "nr9_spr0.5m", 0, 0x80000, CRC(86d4ff85) SHA1(a71056b2bcbba50c834fe28269ebda9719df354a) ) // == gn1-sha0.5m

	ROM_REGION( 0x0800, "eeprom", 0 ) // default gun calibration and settings
	ROM_LOAD( "eeprom", 0x0000, 0x0800, CRC(95760d0f) SHA1(94ac5a261d9afc77c2a163a50950b0e86b1f8041) )
ROM_END

ROM_START( gunbuletw ) /* World set using 4Mb sound data rom (verified) */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "gn3_mprlb.15b", 0x00002, 0x80000, CRC(9260fce5) SHA1(064579be1ac90e04082a8b403c6adf35dbb46a7e) )
	ROM_LOAD32_WORD( "gn3_mprub.13b", 0x00000, 0x80000, CRC(6c1ac697) SHA1(7b52b5ef8154a5d741ac24673f3e6bbfa246a494) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data - JP1 jumper selectable between 1Mb (27C1024) or 4Mb (27C4096) either rom is correct */
//  ROM_LOAD( "gn1_spr0.5b", 0, 0x20000, CRC(6836ba38) SHA1(6ea17ea4bbb59be108e8887acd7871409580732f) ) /* 1Megabit, same data as the 4Mb rom at 0x00000-0x1ffff */
	ROM_LOAD( "gn1-spr0.5b", 0, 0x80000, CRC(71773811) SHA1(e482784d9b9ebf8c2e4a2a3f6f6c4dc8304d2251) ) /* 4Megabit, same data at 0x00000-0x1ffff, 0x20000-0x7ffff is 0xff filled */

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "gn1-voi0.5j", 0, 0x200000, CRC(05477eb7) SHA1(f2eaacb5dbac06c37c56b9b131230c9cf6602221) )

	ROM_REGION( 0x800000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "gn1obj0l.ic1", 0x000001, 0x200000, CRC(06722dc8) SHA1(56fee4e17ed707fa6dbc6bad0d0281fc8cdf72d1) ) /* These four located on MEMEXT OBJ8 PCB daughter-card */
	ROM_LOAD16_BYTE( "gn1obj0u.ic2", 0x000000, 0x200000, CRC(fcefc909) SHA1(48c19b6032096dd80777aa6d5eb5f90463095cbe) )
	ROM_LOAD16_BYTE( "gn1obj1l.ic3", 0x400001, 0x200000, CRC(48468df7) SHA1(c5fb9082c84ac2ffceb6f5f4cbc1d40047c55e3d) )
	ROM_LOAD16_BYTE( "gn1obj1u.ic4", 0x400000, 0x200000, CRC(3109a071) SHA1(4bb16df5a3aecdf37baf843edfc82952d46f5227) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "gn1-chr0.8j",  0x000000, 0x100000, CRC(a5c61246) SHA1(d1d9f286b93b5b9880160029c53384d13c08dd8a) )
	ROM_LOAD( "gn1-chr1.9j",  0x100000, 0x100000, CRC(c8c59772) SHA1(91de633a300e3b25a919579eaada5549640ab6f0) )
	ROM_LOAD( "gn1-chr2.10j", 0x200000, 0x100000, CRC(dc96d999) SHA1(d006a401762b57fef6716f56eb3a7edcb3d3c00e) )
	ROM_LOAD( "gn1-chr3.11j", 0x300000, 0x100000, CRC(4352c308) SHA1(785c13df219dceac2f940519141665b630a29f86) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "gn1-sha0.5m", 0, 0x80000, CRC(86d4ff85) SHA1(a71056b2bcbba50c834fe28269ebda9719df354a) )

	ROM_REGION( 0x0800, "eeprom", 0 ) // default gun calibration and settings
	ROM_LOAD( "eeprom", 0x0000, 0x0800, CRC(95760d0f) SHA1(94ac5a261d9afc77c2a163a50950b0e86b1f8041) )
ROM_END

ROM_START( gunbuletj ) /* Japanese set using 1Mb sound data rom (verified) */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "gn1_mprl.15b", 0x00002, 0x80000, CRC(f99e309e) SHA1(3fe0ddf756e6849f8effc7672456cbe32f65c98a) )
	ROM_LOAD32_WORD( "gn1_mpru.13b", 0x00000, 0x80000, CRC(72a4db07) SHA1(8c5e1e51cd961b311d03f7b21f36a5bd5e8e9104) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data - JP1 jumper selectable between 1Mb (27C1024) or 4Mb (27C4096) either rom is correct */
	ROM_LOAD( "gn1_spr0.5b", 0, 0x20000, CRC(6836ba38) SHA1(6ea17ea4bbb59be108e8887acd7871409580732f) ) /* 1Megabit, same data as the 4Mb rom at 0x00000-0x1ffff */
//  ROM_LOAD( "gn1-spr0.5b", 0, 0x80000, CRC(71773811) SHA1(e482784d9b9ebf8c2e4a2a3f6f6c4dc8304d2251) ) /* 4Megabit, same data at 0x00000-0x1ffff, 0x20000-0x7ffff is 0xff filled */

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "gn1-voi0.5j", 0, 0x200000, CRC(05477eb7) SHA1(f2eaacb5dbac06c37c56b9b131230c9cf6602221) )

	ROM_REGION( 0x800000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "gn1obj0l.ic1", 0x000001, 0x200000, CRC(06722dc8) SHA1(56fee4e17ed707fa6dbc6bad0d0281fc8cdf72d1) ) /* These four located on MEMEXT OBJ8 PCB daughter-card */
	ROM_LOAD16_BYTE( "gn1obj0u.ic2", 0x000000, 0x200000, CRC(fcefc909) SHA1(48c19b6032096dd80777aa6d5eb5f90463095cbe) )
	ROM_LOAD16_BYTE( "gn1obj1l.ic3", 0x400001, 0x200000, CRC(48468df7) SHA1(c5fb9082c84ac2ffceb6f5f4cbc1d40047c55e3d) )
	ROM_LOAD16_BYTE( "gn1obj1u.ic4", 0x400000, 0x200000, CRC(3109a071) SHA1(4bb16df5a3aecdf37baf843edfc82952d46f5227) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "gn1-chr0.8j",  0x000000, 0x100000, CRC(a5c61246) SHA1(d1d9f286b93b5b9880160029c53384d13c08dd8a) )
	ROM_LOAD( "gn1-chr1.9j",  0x100000, 0x100000, CRC(c8c59772) SHA1(91de633a300e3b25a919579eaada5549640ab6f0) )
	ROM_LOAD( "gn1-chr2.10j", 0x200000, 0x100000, CRC(dc96d999) SHA1(d006a401762b57fef6716f56eb3a7edcb3d3c00e) )
	ROM_LOAD( "gn1-chr3.11j", 0x300000, 0x100000, CRC(4352c308) SHA1(785c13df219dceac2f940519141665b630a29f86) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "gn1-sha0.5m", 0, 0x80000, CRC(86d4ff85) SHA1(a71056b2bcbba50c834fe28269ebda9719df354a) )

	ROM_REGION( 0x0800, "eeprom", 0 ) // default gun calibration and settings
	ROM_LOAD( "eeprom", 0x0000, 0x0800, CRC(95760d0f) SHA1(94ac5a261d9afc77c2a163a50950b0e86b1f8041) )
ROM_END

ROM_START( nebulray )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "nr2_mprl.15b", 0x00002, 0x80000, CRC(0431b6d4) SHA1(54c96e8ac9e753956c31bdef79d390f1c20e10ff) )
	ROM_LOAD32_WORD( "nr2_mpru.13b", 0x00000, 0x80000, CRC(049b97cb) SHA1(0e344b29a4d4bdc854fa9849589772df2eeb0a05) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "nr1-spr0", 0, 0x20000, CRC(1cc2b44b) SHA1(161f4ed39fabe89d7ee1d539f8b9f08cd0ff3111) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "nr1-voi0", 0, 0x200000, CRC(332d5e26) SHA1(9daddac3fbe0709e25ed8e0b456bac15bfae20d7) )

	ROM_REGION( 0x1000000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "nr1obj0u", 0x000000, 0x200000, CRC(fb82a881) SHA1(c9fa0728a37376a5c85bff1f6e8400c13ce15769) )
	ROM_LOAD16_BYTE( "nr1obj0l", 0x000001, 0x200000, CRC(0e99ef46) SHA1(450fe61e448270b633f312361bd5ca89bb9684dd) )
	ROM_LOAD16_BYTE( "nr1obj1u", 0x400000, 0x200000, CRC(49d9dbd7) SHA1(2dbd842c192d65888f931cdb5c9387127b1ab632) )
	ROM_LOAD16_BYTE( "nr1obj1l", 0x400001, 0x200000, CRC(f7a898f0) SHA1(a25a134a42adeb9088019bde42a96d120f20407e) )
	ROM_LOAD16_BYTE( "nr1obj2u", 0x800000, 0x200000, CRC(8c8205b1) SHA1(2c5fb9392d8cd5f8d1f9aba6ddbbafd061271cd4) )
	ROM_LOAD16_BYTE( "nr1obj2l", 0x800001, 0x200000, CRC(b39871d1) SHA1(a8f910702bb88a001f2bfd1b33ad355aa3b0f429) )
	ROM_LOAD16_BYTE( "nr1obj3u", 0xc00000, 0x200000, CRC(d5918c9e) SHA1(530781fb44d7bbf01669bb265b658cb60e27bcd7) )
	ROM_LOAD16_BYTE( "nr1obj3l", 0xc00001, 0x200000, CRC(c90d13ae) SHA1(675f7b8b3325aac91b2bae1cbebe274a65aedc43) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "nr1-chr0", 0x000000, 0x100000,CRC(8d5b54ea) SHA1(616d5729f474da91da19a8246066280652da998c) )
	ROM_LOAD( "nr1-chr1", 0x100000, 0x100000,CRC(cd21630c) SHA1(9974c0eb1051ca52f001e6631264a1936bb50620) )
	ROM_LOAD( "nr1-chr2", 0x200000, 0x100000,CRC(70a11023) SHA1(bead486a86bd96c6fdfd2ea4d4d37c38bbe9bfbb) )
	ROM_LOAD( "nr1-chr3", 0x300000, 0x100000,CRC(8f4b1d51) SHA1(b48fb2c8ccd9105a5b48be44dd3fe4309769efa4) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "nr1-sha0", 0, 0x80000,CRC(ca667e13) SHA1(685032603224cb81bcb85361921477caec570d5e) )

	ROM_REGION( 0x20, "proms", 0 ) /* custom key data? */
	ROM_LOAD( "c366.bin", 0, 0x20, CRC(8c96f31d) SHA1(d186859cfc19a63266084372080d0a5bee687ae2) )
ROM_END

ROM_START( nebulrayj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "nr1_mprl.15b", 0x00002, 0x80000, CRC(fae5f62c) SHA1(143d716abbc834aac6270db3bbb89ec71ea3804d) )
	ROM_LOAD32_WORD( "nr1_mpru.13b", 0x00000, 0x80000, CRC(42ef71f9) SHA1(20e3cb63e1fde293c60c404b378d901d635c4b79) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "nr1-spr0", 0, 0x20000, CRC(1cc2b44b) SHA1(161f4ed39fabe89d7ee1d539f8b9f08cd0ff3111) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "nr1-voi0", 0, 0x200000, CRC(332d5e26) SHA1(9daddac3fbe0709e25ed8e0b456bac15bfae20d7) )

	ROM_REGION( 0x1000000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "nr1obj0u", 0x000000, 0x200000, CRC(fb82a881) SHA1(c9fa0728a37376a5c85bff1f6e8400c13ce15769) )
	ROM_LOAD16_BYTE( "nr1obj0l", 0x000001, 0x200000, CRC(0e99ef46) SHA1(450fe61e448270b633f312361bd5ca89bb9684dd) )
	ROM_LOAD16_BYTE( "nr1obj1u", 0x400000, 0x200000, CRC(49d9dbd7) SHA1(2dbd842c192d65888f931cdb5c9387127b1ab632) )
	ROM_LOAD16_BYTE( "nr1obj1l", 0x400001, 0x200000, CRC(f7a898f0) SHA1(a25a134a42adeb9088019bde42a96d120f20407e) )
	ROM_LOAD16_BYTE( "nr1obj2u", 0x800000, 0x200000, CRC(8c8205b1) SHA1(2c5fb9392d8cd5f8d1f9aba6ddbbafd061271cd4) )
	ROM_LOAD16_BYTE( "nr1obj2l", 0x800001, 0x200000, CRC(b39871d1) SHA1(a8f910702bb88a001f2bfd1b33ad355aa3b0f429) )
	ROM_LOAD16_BYTE( "nr1obj3u", 0xc00000, 0x200000, CRC(d5918c9e) SHA1(530781fb44d7bbf01669bb265b658cb60e27bcd7) )
	ROM_LOAD16_BYTE( "nr1obj3l", 0xc00001, 0x200000, CRC(c90d13ae) SHA1(675f7b8b3325aac91b2bae1cbebe274a65aedc43) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "nr1-chr0", 0x000000, 0x100000,CRC(8d5b54ea) SHA1(616d5729f474da91da19a8246066280652da998c) )
	ROM_LOAD( "nr1-chr1", 0x100000, 0x100000,CRC(cd21630c) SHA1(9974c0eb1051ca52f001e6631264a1936bb50620) )
	ROM_LOAD( "nr1-chr2", 0x200000, 0x100000,CRC(70a11023) SHA1(bead486a86bd96c6fdfd2ea4d4d37c38bbe9bfbb) )
	ROM_LOAD( "nr1-chr3", 0x300000, 0x100000,CRC(8f4b1d51) SHA1(b48fb2c8ccd9105a5b48be44dd3fe4309769efa4) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "nr1-sha0", 0, 0x80000,CRC(ca667e13) SHA1(685032603224cb81bcb85361921477caec570d5e) )

	ROM_REGION( 0x20, "proms", 0 ) /* custom key data? */
	ROM_LOAD( "c366.bin", 0, 0x20, CRC(8c96f31d) SHA1(d186859cfc19a63266084372080d0a5bee687ae2) )
ROM_END

ROM_START( gslgr94u )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "gse2mprl.15b", 0x00002, 0x80000, CRC(a514349c) SHA1(1f7ec81cd6193410d2f01e6f0f84878561fc8035) )
	ROM_LOAD32_WORD( "gse2mpru.13b", 0x00000, 0x80000, CRC(b6afd238) SHA1(438a3411ac8ce3d22d5da8c0800738cb8d2994a9) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "gse2spr0.bin", 0, 0x20000, CRC(17e87cfc) SHA1(9cbeadb6dfcb736e8c80eab344f70fc2f58469d6) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "gse-voi0.bin", 0, 0x200000, CRC(d3480574) SHA1(0c468ed060769b36b7e41cf4919cb6d8691d64f6) )

	ROM_REGION( 0x400000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "gseobj0l.bin", 0x000001, 0x200000, CRC(531520ca) SHA1(2a1a5282549c6f7a37d5fb8c0b342edb9dc45315) )
	ROM_LOAD16_BYTE( "gseobj0u.bin", 0x000000, 0x200000, CRC(fcc1283c) SHA1(fb44ed742f362e6737412cabf3f67d9506456a9e) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "gse-chr0.bin", 0x000000, 0x100000, CRC(9314085d) SHA1(150e8ea908861337f9be2749aa7f9e1d52570586) )
	ROM_LOAD( "gse-chr1.bin", 0x100000, 0x100000, CRC(c128a887) SHA1(4faf78064dd48ec50684a7dc8d120f8c5985bf2a) )
	ROM_LOAD( "gse-chr2.bin", 0x200000, 0x100000, CRC(48f0a311) SHA1(e39adcce835542e64ca87f6019d4a85fcbe388c2) )
	ROM_LOAD( "gse-chr3.bin", 0x300000, 0x100000, CRC(adbd1f88) SHA1(3c7bb1a9a398412bd3c98cadf8ce63a16e2bfed5) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "gse-sha0.bin", 0, 0x80000, CRC(6b2beabb) SHA1(815f7aef44735584edd4a9ca7e672471d07f225e) )
ROM_END

ROM_START( gslgr94j )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "gs41mprl.15b", 0x00002, 0x80000, CRC(5759bdb5) SHA1(a0fb332c484e168369a69cd9dd8ea72e5f4565df) )
	ROM_LOAD32_WORD( "gs41mpru.13b", 0x00000, 0x80000, CRC(78bde1e7) SHA1(911d33897f03c59c6505f5f755d80471ff019812) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "gs41spr0.5b", 0, 0x80000, CRC(3e2b6d55) SHA1(f6a1ecaee3a9a7a535850084e469aa7f873f301e) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "gs4voi0.5j", 0, 0x200000, CRC(c3053a90) SHA1(e76799b33b2457421255b03786bc24266d59c7dd) )

	ROM_REGION( 0x800000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "gs4obj0l.bin", 0x000001, 0x200000, CRC(3b499da0) SHA1(91ad5f68dbda64dd07e1133eb09ee69da3da3103) )
	ROM_LOAD16_BYTE( "gs4obj0u.bin", 0x000000, 0x200000, CRC(80016b50) SHA1(9f7604c196835d31894ba4db1de43d7d2614da84) )
	ROM_LOAD16_BYTE( "gs4obj1l.bin", 0x400001, 0x200000, CRC(1f4847a7) SHA1(908e419e42fa8bd786cc3bc96d5ccb3a47c8e2dc) )
	ROM_LOAD16_BYTE( "gs4obj1u.bin", 0x400000, 0x200000, CRC(49bc48cd) SHA1(6bcc41546f3bd609e3aa962e5ce3bf5bc6b9229a) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "gs4chr0.8j",  0x000000, 0x100000, CRC(8c6c682e) SHA1(ecf21035d5af28299c9cdb98d5d811b4d52857b8) )
	ROM_LOAD( "gs4chr1.9j",  0x100000, 0x100000, CRC(523989f7) SHA1(fae0e2f58e9a8d0ddc7297b567579849e24e0a40) )
	ROM_LOAD( "gs4chr2.10j", 0x200000, 0x100000, CRC(37569559) SHA1(ce31673f51c6302f4fb4e4c377e6693a40874f81) )
	ROM_LOAD( "gs4chr3.11j", 0x300000, 0x100000, CRC(73ca58f6) SHA1(44bdc943fb10dc53279662cd528169a27d57e478) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "gs4sha0.5m", 0, 0x80000, CRC(40e7e6a5) SHA1(70af76b6034e0d6e1b96bf54c973ab411e5907ab) )
ROM_END

/*
Great Sluggers (Japan)
Namco, 1993

This game runs on Namco NB-1 hardware.

PCB Layout
----------
NB-1 MAIN PCB                                  MEMEXT OBJ2 PCB
8634961101 (8634963101)                    8635901201 (8635901301)
|------------------------------------------------------|---------|
|                            62256       62256         |         |
|LA4705 VOL M5M5178          62256       62256         |GS1OBJ-0 |
|           M5M5178                              C347  |         |
| 4558      M5M5178          62256       62256         |         |
|                            62256       62256         |         |
| LC78815    C116   156                                |         |
|JP3                62256    62256       62256         |         |
|              145  62256    62256       62256         |GS1OBJ-1 |
|   GS1SHA-0                                           |         |
|J  JP2                      62256       62256     C355|---------|
|A                           62256       62256          62256    |
|M             123                                      62256    |
|M                                                    JP12       |
|A                                   %3  JP11                    |
|              JP5  GS1CHR-0   GS1CHR-2  JP10   137   187   M3771|
|   GS1VOI-0   JP6  GS1CHR-1   GS1CHR-3      48.384MHz           |
|                                                                |
|              C352                                              |
|SW1   75                                                        |
|                                                                |
|   TC551001                   %1                                |
|                      KM28C16         GS1MPRU      JP9          |
|   TC551001   PAL2                %2  GS1MPRL                   |
|   JP1                         JP7    JP8                       |
|   GS1SPR0    PAL1     C329                      68EC020        |
|                                                                |
|----------------------------------------------------------------|
Notes:

CLOCKs
------
MASTER clock : 48.384 MHz
68020 clock  : 24.192MHz (MASTER / 2)
HSYNC        : 15.75kHz
VSYNC        : 59.7Hz

DIPs
----
SW1: 2 position, both are OFF. Position 1 toggles TEST mode, position 2 is freeze.

RAM
---
TC551001AFL x 2  (SOP32, 128k x8 SRAM)
62256       x 20 (SOP28, 32k  x8 SRAM)
M5M5178     x 3  (SOP28, 8k   x8 SRAM)

NAMCO CUSTOM CHIPS
------------------
75       (QFP80, M37702 in disguise; sound CPU with internal BIOS)
123      (QFP80)
137      (NDIP28)
145      (QFP80)
156      (QFP64)
187      (QFP120)
C116     (QFP64)
C329     (QFP100)
C347     (QFP80)
C351     (QFP160)
C352     (QFP100)
C355     (QFP160)

OTHER
-----
KM28C16  2K x8 EEPROM (DIP24)
%1       Unpopulated KEYCUS socket
%2       Unpopulated DATA ROM socket
%3       Unpopulated position for 28MHz OSC

PALs
----
PAL1 PALCE16V8 (NAMCO CODE = NB1-1)
PAL2 PAL16L8   (NAMCO CODE = NB1-2)

JUMPERs
-------
JP1      4M  O-O O  1M    Config jumper for ROM size, 4M = 27C4096, 1M = 27C1024
JP2      4M  O-O O  1M    Config jumper for ROM size, 4M = 27C4096, 1M = 27C1024
JP3          O-O          (2 pins shorted, hardwired on PCB)
JP5     /1M  O-O O  1M    Config jumper for ROM size (hardwired on PCB)
JP6      8M  O-O O  /8M   Config jumper for ROM size (hardwired on PCB)
JP7      4M  O-O O  1M    Config jumper for ROM size (hardwired on PCB), 4M = 27C4096, 1M = 27C1024
JP8      4M  O-O O  1M    Config jumper for ROM size (hardwired on PCB), 4M = 27C4096, 1M = 27C1024
JP9     CON  O-O O  COFF  (hardwired on PCB)
JP10    24M  O-O O  28M   Config jumper for 28MHz OSC (hardwired on PCB)
JP11    24M  O-O O  12M   Config jumper for 28MHz OSC (hardwired on PCB)
JP12    F32  O O-O  355   (hardwired on PCB)

ROMs, MAIN PCB
--------------
Filename /      PCB       ROM
ROM Label       Label     Type
------------------------------------------------------------------------------
GS1MPRU.13B     PRGU      27C240        \ Main program
GS1MPRL.15B     PRGL      27C240        /
GS1SPR0.5B      SPRG      27C240        Sound program, linked to 75, C351 and C352
GS1VOI-0.5J     VOICE     16M MASK      Sound voices
GS1CHR-0.8J     CHR0      8M MASK       Character
GS1CHR-1.9J     CHR1      8M MASK       Character
GS1CHR-2.10J    CHR2      8M MASK       Character
GS1CHR-3.11J    CHR3      8M MASK       Character
GS1SHA-0.5M     SHAPE     4M MASK       Shape

ROMs, MEMEXT OBJ2 PCB  (All ROMs surface mounted)
---------------------
Filename /      PCB       ROM
ROM Label       Label     Type
----------------------------------------
GS1OBJ-0.IC1    OBJL      16M MASK SOP44
GS1OBJ-1.IC2    OBJU      16M MASK SOP44

Note! All ROMs are different to the Great Sluggers '94 set.

*/

ROM_START( gslugrsj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "gs1mprl.15b", 0x00002, 0x80000, CRC(1e6c3626) SHA1(56abe21884fd87df10996db19c49ce14214d4b73) )
	ROM_LOAD32_WORD( "gs1mpru.13b", 0x00000, 0x80000, CRC(ef355179) SHA1(0ab0ef4301a318681bb5827d35734a0732b35484) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "gs1spr0.5b", 0, 0x80000, CRC(561ea20f) SHA1(adac6b77effc3a82079a9b228bafca0fcef72ba5) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "gs1voi-0.5j", 0, 0x200000, CRC(6f8262aa) SHA1(beea98d9f8b927a572eb0bfcf678e9d6e40fc68d) )

	ROM_REGION( 0x400000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "gs1obj-0.ic1", 0x000001, 0x200000, CRC(9a55238f) SHA1(fc3fd4b8b6322bbe343edbcad7815b597562266b) )
	ROM_LOAD16_BYTE( "gs1obj-1.ic2", 0x000000, 0x200000, CRC(31c66f76) SHA1(8903e6586dff6f34a6ffca2d7c75343c0a5bff56) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "gs1chr-0.8j",  0x000000, 0x100000, CRC(e7ced86a) SHA1(de90c2e3870b317431d3910f581660681b46ff9d) )
	ROM_LOAD( "gs1chr-1.9j",  0x100000, 0x100000, CRC(1fe46749) SHA1(f4c0ea666d52cb1c8b1da93e7486ade5eae336cc) )
	ROM_LOAD( "gs1chr-2.10j", 0x200000, 0x100000, CRC(f53afa20) SHA1(5c317e276ca2355e9737c1e8114dccbb5e11058a) )
	ROM_LOAD( "gs1chr-3.11j", 0x300000, 0x100000, CRC(b149d7da) SHA1(d50c6258db0ccdd69b563e880d1711aae811fbe3) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "gs1sha-0.5m", 0, 0x80000, CRC(8a2832fe) SHA1(a1f54754fb01bbbc87274b1a0a4127fa9296ad1a) )
ROM_END

ROM_START( sws95 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "ss51mprl.bin", 0x00002, 0x80000, CRC(c9e0107d) SHA1(0f10582416023a86ea1ef2679f3f06016c086e08) )
	ROM_LOAD32_WORD( "ss51mpru.bin", 0x00000, 0x80000, CRC(0d93d261) SHA1(5edef26e2c86dbc09727d910af92747d022e4fed) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "ss51spr0.bin", 0, 0x80000, CRC(71cb12f5) SHA1(6e13bd16a5ba14d6e47a21875db3663ada3c06a5) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "ss51voi0.bin", 0, 0x200000, CRC(2740ec72) SHA1(9694a7378ea72771d2b1d43db6d74ed347ba27d3) )


	ROM_REGION( 0x400000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "ss51ob0l.bin", 0x000001, 0x200000, CRC(e0395694) SHA1(e52045a7af4c4b0f9935695cfc5ff729bf9bd7c1) )
	ROM_LOAD16_BYTE( "ss51ob0u.bin", 0x000000, 0x200000, CRC(b0745ca0) SHA1(579ea7fd7b9a181fd9d08c50c6c5941264aa0b6d) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "ss51chr0.bin", 0x000000, 0x100000, CRC(86dd3280) SHA1(07ba6d3edc5c38bf82ddaf8f6de7ef0f5d0788b2) )
	ROM_LOAD( "ss51chr1.bin", 0x100000, 0x100000, CRC(2ba0fb9e) SHA1(39ceddad7bc0073b361eb776762002a9fc61b337) )
	ROM_LOAD( "ss51chr2.bin", 0x200000, 0x100000, CRC(ca0e6c1a) SHA1(1221cd30894e97e2f7d456509c7b6732ec3d06a5) )
	ROM_LOAD( "ss51chr3.bin", 0x300000, 0x100000, CRC(73ca58f6) SHA1(44bdc943fb10dc53279662cd528169a27d57e478) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "ss51sha0.bin", 0, 0x80000, CRC(3bf4d081) SHA1(7b07b86f753ea6bcd90eb7d152c12884a6fe785a) )
ROM_END

ROM_START( sws96 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "ss61mprl.bin", 0x00002, 0x80000, CRC(06f55e73) SHA1(6be26f8a2ef600bf07c580f210d7b265ac464002) )
	ROM_LOAD32_WORD( "ss61mpru.bin", 0x00000, 0x80000, CRC(0abdbb83) SHA1(67e8b712291f9bcf2c3a52fbc451fad54679cab8) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "ss61spr0.bin", 0, 0x80000, CRC(71cb12f5) SHA1(6e13bd16a5ba14d6e47a21875db3663ada3c06a5) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "ss61voi0.bin", 0, 0x200000, CRC(2740ec72) SHA1(9694a7378ea72771d2b1d43db6d74ed347ba27d3) )

	ROM_REGION( 0x400000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "ss61ob0l.bin", 0x000001, 0x200000, CRC(579b19d4) SHA1(7f18097c683d2b1c532f54ee514dd499f5965165) )
	ROM_LOAD16_BYTE( "ss61ob0u.bin", 0x000000, 0x200000, CRC(a69bbd9e) SHA1(8f4c44e2caa31d25433a04c19c51904ec9461e2f) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "ss61chr0.bin", 0x000000, 0x100000, CRC(9d2ae07b) SHA1(7d268f6c7d8145c913f80049369ae3106d69e939) )
	ROM_LOAD( "ss61chr1.bin", 0x100000, 0x100000, CRC(4dc75da6) SHA1(a29932b4fb39648e2c02df668f46cafb80c53619) )
	ROM_LOAD( "ss61chr2.bin", 0x200000, 0x100000, CRC(1240704b) SHA1(a24281681053cc6649f00ec5a31c7249101eaee1) )
	ROM_LOAD( "ss61chr3.bin", 0x300000, 0x100000, CRC(066581d4) SHA1(999cd478d9da452bb57793cd276c6c0d87e2825e) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "ss61sha0.bin", 0, 0x80000, CRC(fceaa19c) SHA1(c9303a755ac7af19c4804a264d1a09d987f39e74) )
ROM_END

ROM_START( sws97 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "ss71mprl.bin", 0x00002, 0x80000, CRC(bd60b50e) SHA1(9e00bacd506182ab2af2c0efdd5cc401b3e46485) )
	ROM_LOAD32_WORD( "ss71mpru.bin", 0x00000, 0x80000, CRC(3444f5a8) SHA1(8d0f35b3ba8f65dbc67c3b2d273833227a8b8b2a) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "ss71spr0.bin", 0, 0x80000, CRC(71cb12f5) SHA1(6e13bd16a5ba14d6e47a21875db3663ada3c06a5) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "ss71voi0.bin", 0, 0x200000, CRC(2740ec72) SHA1(9694a7378ea72771d2b1d43db6d74ed347ba27d3) )

	ROM_REGION( 0x400000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "ss71ob0l.bin", 0x000001, 0x200000, CRC(9559ad44) SHA1(fd56a8620f6958cc090f783d74cb38bba46d2423) )
	ROM_LOAD16_BYTE( "ss71ob0u.bin", 0x000000, 0x200000, CRC(4df4a722) SHA1(07eb94628ceeb7cbce2d39d479f33c37583a346a) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "ss71chr0.bin", 0x000000, 0x100000, CRC(bd606356) SHA1(a62c55600e46f8821db0b84d79fc2588742ad7ad) )
	ROM_LOAD( "ss71chr1.bin", 0x100000, 0x100000, CRC(4dc75da6) SHA1(a29932b4fb39648e2c02df668f46cafb80c53619) )
	ROM_LOAD( "ss71chr2.bin", 0x200000, 0x100000, CRC(1240704b) SHA1(a24281681053cc6649f00ec5a31c7249101eaee1) )
	ROM_LOAD( "ss71chr3.bin", 0x300000, 0x100000, CRC(066581d4) SHA1(999cd478d9da452bb57793cd276c6c0d87e2825e) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "ss71sha0.bin", 0, 0x80000, CRC(be8c2758) SHA1(0a1b6c03cdaec6103ae8483b67faf3840234f825) )
ROM_END

ROM_START( vshoot )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "vsj1mprl.15b", 0x00002, 0x80000, CRC(83a60d92) SHA1(c3db0c79f772a79418914353a3d6ecc4883ea54e) )
	ROM_LOAD32_WORD( "vsj1mpru.13b", 0x00000, 0x80000, CRC(c63eb92d) SHA1(f93bd4b91daee645677955020dc8df14dc9bfd27) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "vsj1spr0.5b", 0, 0x80000, CRC(b0c71aa6) SHA1(a94fae02b46a645ff728d2f98827c85ff155892b) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "vsjvoi-0.5j", 0, 0x200000, CRC(0528c9ed) SHA1(52b67978fdeb97b77065575774a7ddeb49fe1d81) )

	ROM_REGION( 0x800000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "vsjobj0l.ic1", 0x000001, 0x200000, CRC(e134faa7) SHA1(a844c8a5bd6d8907f9e5c7ba9e2ee8e9a886cd1e) ) /* These four located on MEMEXT OBJ8 PCB daughter-card */
	ROM_LOAD16_BYTE( "vsjobj0u.ic2", 0x000000, 0x200000, CRC(974d0714) SHA1(976050eaf82d4b66e13c1c579e5521eb867527fb) )
	ROM_LOAD16_BYTE( "vsjobj1l.ic3", 0x400001, 0x200000, CRC(ba46f967) SHA1(ddfb0ac7fba7369869e4df9a66d465a662eba2e6) )
	ROM_LOAD16_BYTE( "vsjobj1u.ic4", 0x400000, 0x200000, CRC(09da7e9c) SHA1(e98e07a886a4fe369748fc97f3cee6a4bb668385) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "vsjchr-0.8j",  0x000000, 0x100000, CRC(2af8ba7c) SHA1(74f5a382425974a9b2167bb01672dd13dea882f5) )
	ROM_LOAD( "vsjchr-1.9j",  0x100000, 0x100000, CRC(b789d53e) SHA1(48b4cf956f9025e3c2b6f59b317596dfe0b6b142) )
	ROM_LOAD( "vsjchr-2.10j", 0x200000, 0x100000, CRC(7ef80758) SHA1(c7e6d14f0823607dfd8a13ea6f164ffa85b5563e) )
	ROM_LOAD( "vsjchr-3.11j", 0x300000, 0x100000, CRC(73ca58f6) SHA1(44bdc943fb10dc53279662cd528169a27d57e478) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "vsjsha-0.5m", 0, 0x80000, CRC(78335ea4) SHA1(d4b9f179b1b456a866354ea308664c036de6414d) )
ROM_END

/*

The Outfoxies
Namco, 1994

This game runs on Namco NB-2 hardware.


Main Board
----------

NB-2 MAIN PCB       8639960102  (8639970102)
|------------------------------------------------------------------------|
||----------------------------------------------------------------------||
||  J103                     J104                            J105       ||
||VOL CY7C185                                                           ||
||458 CY7C185     156      123      C384  C384  C384    LH52250    C355 ||
||JP5 CY7C185                                      JP11 LH52250         ||
||LA4705                                                                ||
||LC78815  C116  LH52250    OU1SHAS.12S            OU1SHAR.18S          ||
||               LH52250        JP8                    JP10             ||
|| JP4                                                                  ||
||                                                                      ||
||--------------------NB-2-MASK-ROM-PCB-(ON-TOP)------------------------||
|J                                                                       |
|    JP3                                                                 |
|A   JP2            145                            PAL1      187         |
|      OU1VOI0.6N   VSYNC                  LH52250                       |
|M                  HSYNC                  LH52250                       |
|      C352                       169      LH52250    TC511632 (x4)      |
|M            137  48.384MHz               LH52250                       |
|                                                                        |
|A SW1  75                                                               |
|                         JP7                         TC511632 (x4)      |
|                       PAL3      C383                                   |
|          C382                                                          |
| M5M1008            OU2MPRU.11D        JP9               BR28C16  C390  |
| M5M1008  PAL2      OU2MPRL.11C   68EC020     C385                      |
| OU1SPR0.5B                 JP6                            OU1DAT1.20B  |
|        JP1                                                             |
|                                                           OU1DAT0.20A  |
|------------------------------------------------------------------------|

ROM Board
---------

NB-2 MASK ROM PCB   8639969800  (8639979800)
-------------------------------------------------------------------------|
|   J103                     J104                            J105        |
|OU1SCR0.1D  OU1ROT0.3D                                                  |
|                                                                        |
|            OU1ROT1.3C  OU1OBJ0L.4C OU1OBJ3L.6C OU1OBJ0U.8C OU1OBJ3U.9C |
|                                                                        |
|            OU1ROT2.3B  OU1OBJ1L.4B OU1OBJ4L.6B OU1OBJ1U.8B OU1OBJ4U.9B |
|                                                                        |
|                        OU1OBJ2L.4A             OU1OBJ2U.8A             |
|------------------------------------------------------------------------|

Notes:

CLOCKs
------
MASTER clock 48.384 MHz
68020 clock: 24.192MHz (MASTER / 2)
HSYNC: 15.75kHz
VSYNC: 59.7Hz

DIPs
----
SW1: 2 position, both are OFF. Position 1 toggles TEST mode, position 2 is freeze.

RAM
---
TC511632FL  x 8 (SOP40, 32k x16)
M5M51008AFP x 2 (SOP32, 128k x8)
LH52250AN   x 8 (SOP28, 32k x8)
CY7C185     x 3 (SOP28, 8k x8)

NAMCO CUSTOM CHIPS
------------------
75       (QFP80)
123      (QFP80)
137      (NDIP28)
145      (QFP80)
156      (QFP64)
169      (QFP120)
187      (QFP120)
C116     (QFP64)
C352     (QFP100)
C355     (QFP160)
C382     (QFP120)
C383     (QFP100)
C384 x 3 (QFP48)
C385     (QFP144)
C390     (DIP32, KEYCUS)

OTHER
-----
BR28C16 (DIP24, EEPROM)
2 gold pins labelled HSYNC & VSYNC, connected to Namco custom chip 145
3 connectors for ROM PCB, labelled J103 (SCROLL), J104 (ROTATE), J105 (OBJECT)

PALs
----
PAL1 PALCE16V8 (NAMCO CODE = NB2-1, PCB says "MIXER")
PAL2 PAL16L8   (NAMCO CODE = NB1-2, PCB says "DEC75")  (note! PAL is NB1-2)
PAL3 PAL16L8   (NAMCO CODE = NB2-2, PCB says "SIZE")

JUMPERs
-------
JP1     4M   O-O O   1M    Config jumper for ROM size, 4M = 27C4002, 1M = 27C1024
JP2     A20  O O-O   GND   Config jumper for ROM size, GND = 16M, A20 = 32M
JP3     A20  O O-O   GND   Config jumper for ROM size, GND = 16M, A20 = 32M
JP4          O-O           (2 pins shorted, hardwired on PCB)
JP5     1    O O O   L     (hardwired on PCB, not shorted)
JP6     1M   O O-O   4M    Config jumper for ROM size, 1M = 27C1024, 4M = 27C240
JP7          O O-O   /WDR  (hardwired on PCB)
JP8     GND  O-O O   A20   Config jumper for ROM size, GND = 16M, A20 = 32M
JP9     CON  O-O O   COFF  (hardwired on PCB)
JP10    GND  O-O O   A20   Config jumper for ROM size, GND = 16M, A20 = 32M
JP11    355  O O-O   F32   (hardwired on PCB)

ROMs, Main PCB
--------------
Filename /      PCB       ROM
ROM Label       Label     Type
------------------------------------------------------------------------------
ou1dat0.20a     DATA0     27C4002       Shared Data
ou1dat1.20b     DATA1     27C4002       Shared Data
ou2mprl.11c     PRGL      27C4002       \ Main program
ou2mpru.11d     PRGU      27C4002       /
ou1spr0.5b      SPRG      27C240        Sound program, linked to C352 and C382
ou1voi0.6n      VOICE0    MB8316200B    Sound voices
ou1shas.12s     SHAPE-S   16M MASK      Shape
ou1shar.18s     SHAPE-R   16M MASK      Shape

ROMs, MASK ROM PCB (All ROMs surface mounted)
------------------
Filename /      PCB       ROM
ROM Label       Label     Type
------------------------------------------------
ou1scr0.1d      SCR0      MB8316200B (16M SOP44)
ou1rot0.3d      ROT0      MB8316200B (16M SOP44)
ou1rot1.3c      ROT1      MB8316200B (16M SOP44)
ou1rot2.3b      ROT2      MB8316200B (16M SOP44)
ou1obj0l.4c     OBJ0L     MB8316200B (16M SOP44)
ou1obj1l.4b     OBJ1L     MB8316200B (16M SOP44)
ou1obj2l.4a     OBJ2L     MB8316200B (16M SOP44)
ou1obj3l.6c     OBJ3L     MB8316200B (16M SOP44)
ou1obj4l.6b     OBJ4L     MB8316200B (16M SOP44)
ou1obj0u.8c     OBJ0U     MB8316200B (16M SOP44)
ou1obj1u.8b     OBJ1U     MB8316200B (16M SOP44)
ou1obj2u.8a     OBJ2U     MB8316200B (16M SOP44)
ou1obj3u.9c     OBJ3U     MB8316200B (16M SOP44)
ou1obj4u.9b     OBJ4U     MB8316200B (16M SOP44)

*/

ROM_START( outfxies )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "ou2_mprl.11c", 0x00002, 0x80000, CRC(f414a32e) SHA1(9733ab087cfde1b8fb5b676d8a2eb5325ebdbb56) )
	ROM_LOAD32_WORD( "ou2_mpru.11d", 0x00000, 0x80000, CRC(ab5083fb) SHA1(cb2e7a4838c2b80057edb83ea63116bccb1394d3) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "ou1spr0.5b", 0, 0x80000, CRC(60cee566) SHA1(2f3b96793816d90011586e0f9f71c58b636b6d4c) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "ou1voi0.6n", 0, 0x200000, CRC(2d8fb271) SHA1(bde9d45979728f5a2cd8ec89f5f81bf16b694cc2) )

	ROM_REGION( 0x200000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "ou1shas.12s", 0, 0x200000,CRC(9bcb0397) SHA1(54a32b6394d0e6f51bfd281f8a4bafce6ddf6246) )

	ROM_REGION( 0x200000, NAMCONB1_ROTMASKREGION, 0 )
	ROM_LOAD( "ou1shar.18s", 0, 0x200000,   CRC(fbb48194) SHA1(2d3ec5bc519fad2b755018f83fadfe0cba13c292) )

	ROM_REGION( 0x2000000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "ou1obj0l.4c", 0x0000001, 0x200000, CRC(1b4f7184) SHA1(a05d67842fce92f321d1fdd3bd30aa3427775a0c) )
	ROM_LOAD16_BYTE( "ou1obj0u.8c", 0x0000000, 0x200000, CRC(d0a69794) SHA1(07d449e54e9971abeb9cd5bb7b372270fafa8bac) )
	ROM_LOAD16_BYTE( "ou1obj1l.4b", 0x0400001, 0x200000, CRC(48a93e84) SHA1(6935ec161a12237d4cec732d42070f381c23b47c) )
	ROM_LOAD16_BYTE( "ou1obj1u.8b", 0x0400000, 0x200000, CRC(999de386) SHA1(d4780ab1929a3e2c2df464363d6451a2bcecb2a2) )
	ROM_LOAD16_BYTE( "ou1obj2l.4a", 0x0800001, 0x200000, CRC(30386cd0) SHA1(3563c5378288da58136f102381373bd6fcaeec21) )
	ROM_LOAD16_BYTE( "ou1obj2u.8a", 0x0800000, 0x200000, CRC(ccada5f8) SHA1(75ed95bb295780126879d67bba4d0ae1da63c928) )
	ROM_LOAD16_BYTE( "ou1obj3l.6c", 0x0c00001, 0x200000, CRC(5f41b44e) SHA1(3f5376fcd3e15af772df65b8eda4d5ee07ee5664) )
	ROM_LOAD16_BYTE( "ou1obj3u.9c", 0x0c00000, 0x200000, CRC(bc852c8e) SHA1(4863302c45ee16aaf2c36dac07aceaf287959c53) )
	ROM_LOAD16_BYTE( "ou1obj4l.6b", 0x1000001, 0x200000, CRC(99a5f9d7) SHA1(b0f46f4ac357918137031a19c36a56a47b7aefd6) )
	ROM_LOAD16_BYTE( "ou1obj4u.9b", 0x1000000, 0x200000, CRC(70ecaabb) SHA1(521c6849526fb271e6447f6c4f5bfa081f96b91e) )

	ROM_REGION( 0x600000, NAMCONB1_ROTGFXREGION, 0 )
	ROM_LOAD( "ou1-rot0.3d", 0x000000, 0x200000, CRC(a50c67c8) SHA1(432b8451eb9eaa3078134fce1e5e2d58a8b64be3) )
	ROM_LOAD( "ou1-rot1.3c", 0x200000, 0x200000, CRC(14866780) SHA1(4a54151fada4dfba7232e53e40623e5697eeb7db) )
	ROM_LOAD( "ou1-rot2.3b", 0x400000, 0x200000, CRC(55ccf3af) SHA1(d98489aaa840cbffb21c47609961c1163b0336f3) )

	ROM_REGION( 0x200000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "ou1-scr0.1d", 0x000000, 0x200000, CRC(b3b3f2e9) SHA1(541bd7e9ba12aff4ec4033bd9c6bb19476acb3c4) )

	ROM_REGION32_BE( 0x100000, "data", 0 )
	ROM_LOAD16_WORD_SWAP( "ou1dat0.20a", 0x00000, 0x80000, CRC(1a49aead) SHA1(df243aff1a6fb5bcf4d5d883c5af2374a4aff477) )
	ROM_LOAD16_WORD_SWAP( "ou1dat1.20b", 0x80000, 0x80000, CRC(63bb119d) SHA1(d4c2820243b84c3f5cdf7f9e66bb50f53d0efed2) )
ROM_END

ROM_START( outfxiesj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "ou1_mprl.11c", 0x00002, 0x80000, CRC(d3b9e530) SHA1(3f5fe5eea817a23dfe42e76f32912ce94d4c49c9) )
	ROM_LOAD32_WORD( "ou1_mpru.11d", 0x00000, 0x80000, CRC(d98308fb) SHA1(fdefeebf56464a20e3aaefd88df4eee9f7b5c4f3) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "ou1spr0.5b", 0, 0x80000, CRC(60cee566) SHA1(2f3b96793816d90011586e0f9f71c58b636b6d4c) )

	ROM_REGION( 0x1000000, "c352", 0 ) // Samples
	ROM_LOAD( "ou1voi0.6n", 0, 0x200000, CRC(2d8fb271) SHA1(bde9d45979728f5a2cd8ec89f5f81bf16b694cc2) )

	ROM_REGION( 0x200000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "ou1shas.12s", 0, 0x200000,CRC(9bcb0397) SHA1(54a32b6394d0e6f51bfd281f8a4bafce6ddf6246) )

	ROM_REGION( 0x200000, NAMCONB1_ROTMASKREGION, 0 )
	ROM_LOAD( "ou1shar.18s", 0, 0x200000,   CRC(fbb48194) SHA1(2d3ec5bc519fad2b755018f83fadfe0cba13c292) )

	ROM_REGION( 0x2000000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "ou1obj0l.4c", 0x0000001, 0x200000, CRC(1b4f7184) SHA1(a05d67842fce92f321d1fdd3bd30aa3427775a0c) )
	ROM_LOAD16_BYTE( "ou1obj0u.8c", 0x0000000, 0x200000, CRC(d0a69794) SHA1(07d449e54e9971abeb9cd5bb7b372270fafa8bac) )
	ROM_LOAD16_BYTE( "ou1obj1l.4b", 0x0400001, 0x200000, CRC(48a93e84) SHA1(6935ec161a12237d4cec732d42070f381c23b47c) )
	ROM_LOAD16_BYTE( "ou1obj1u.8b", 0x0400000, 0x200000, CRC(999de386) SHA1(d4780ab1929a3e2c2df464363d6451a2bcecb2a2) )
	ROM_LOAD16_BYTE( "ou1obj2l.4a", 0x0800001, 0x200000, CRC(30386cd0) SHA1(3563c5378288da58136f102381373bd6fcaeec21) )
	ROM_LOAD16_BYTE( "ou1obj2u.8a", 0x0800000, 0x200000, CRC(ccada5f8) SHA1(75ed95bb295780126879d67bba4d0ae1da63c928) )
	ROM_LOAD16_BYTE( "ou1obj3l.6c", 0x0c00001, 0x200000, CRC(5f41b44e) SHA1(3f5376fcd3e15af772df65b8eda4d5ee07ee5664) )
	ROM_LOAD16_BYTE( "ou1obj3u.9c", 0x0c00000, 0x200000, CRC(bc852c8e) SHA1(4863302c45ee16aaf2c36dac07aceaf287959c53) )
	ROM_LOAD16_BYTE( "ou1obj4l.6b", 0x1000001, 0x200000, CRC(99a5f9d7) SHA1(b0f46f4ac357918137031a19c36a56a47b7aefd6) )
	ROM_LOAD16_BYTE( "ou1obj4u.9b", 0x1000000, 0x200000, CRC(70ecaabb) SHA1(521c6849526fb271e6447f6c4f5bfa081f96b91e) )

	ROM_REGION( 0x600000, NAMCONB1_ROTGFXREGION, 0 )
	ROM_LOAD( "ou1-rot0.3d", 0x000000, 0x200000, CRC(a50c67c8) SHA1(432b8451eb9eaa3078134fce1e5e2d58a8b64be3) )
	ROM_LOAD( "ou1-rot1.3c", 0x200000, 0x200000, CRC(14866780) SHA1(4a54151fada4dfba7232e53e40623e5697eeb7db) )
	ROM_LOAD( "ou1-rot2.3b", 0x400000, 0x200000, CRC(55ccf3af) SHA1(d98489aaa840cbffb21c47609961c1163b0336f3) )

	ROM_REGION( 0x200000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "ou1-scr0.1d", 0x000000, 0x200000, CRC(b3b3f2e9) SHA1(541bd7e9ba12aff4ec4033bd9c6bb19476acb3c4) )

	ROM_REGION32_BE( 0x100000, "data", 0 )
	ROM_LOAD16_WORD_SWAP( "ou1dat0.20a", 0x00000, 0x80000, CRC(1a49aead) SHA1(df243aff1a6fb5bcf4d5d883c5af2374a4aff477) )
	ROM_LOAD16_WORD_SWAP( "ou1dat1.20b", 0x80000, 0x80000, CRC(63bb119d) SHA1(d4c2820243b84c3f5cdf7f9e66bb50f53d0efed2) )
ROM_END


ROM_START( machbrkr )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD( "mb1_mprl.11c", 0x00002, 0x80000, CRC(86cf0644) SHA1(07eeadda1d94c9be2f882edb6f2eb0b98292e500) )
	ROM_LOAD32_WORD( "mb1_mpru.11d", 0x00000, 0x80000, CRC(fb1ff916) SHA1(e0ba96c1f26a60f87d8050e582e164d91e132183) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) /* sound data */
	ROM_LOAD( "mb1_spr0.5b", 0, 0x80000, CRC(d10f6272) SHA1(cb99e06e050dbf86998ea51ef2ca130b2acfb2f6) )

	ROM_REGION( 0x1000000, "c352", 0 )
	ROM_LOAD( "mb1_voi0.6n", 0x000000, 0x200000, CRC(d363ca3b) SHA1(71650b66ca3eb00f6ad7d3f1df0f37210b77b942) )
	ROM_RELOAD( 0x400000, 0x200000)
	ROM_LOAD( "mb1_voi1.6p", 0x800000, 0x200000, CRC(7e1c2603) SHA1(533098a54fb897931f1d75be9e69a5c047e4c446) )
	ROM_RELOAD( 0xc00000, 0x200000)

	ROM_REGION( 0x200000, NAMCONB1_TILEMASKREGION, 0 )
	ROM_LOAD( "mb1_shas.12s", 0, 0x100000, CRC(c51c614b) SHA1(519ecad2e4543c05ec35a727f4c875ab006291af) )

	ROM_REGION( 0x200000, NAMCONB1_ROTMASKREGION, 0 )
	ROM_LOAD( "mb1_shar.18s", 0, 0x080000, CRC(d9329b10) SHA1(149c8804c07350f47af36bc7902371f1dfbed272) )

	ROM_REGION( 0x2000000, NAMCONB1_SPRITEGFXREGION, 0 )
	ROM_LOAD16_BYTE( "mb1obj0l.4c", 0x0000001, 0x200000, CRC(056e6b1c) SHA1(44e49de80c925c8fbe04bf9328a77a50a305a5a7) )
	ROM_LOAD16_BYTE( "mb1obj0u.8c", 0x0000000, 0x200000, CRC(e19b1714) SHA1(ff43bf3c8e8698934c4057c7b4c72db73929e2af) )
	ROM_LOAD16_BYTE( "mb1obj1l.4b", 0x0400001, 0x200000, CRC(af69f7f1) SHA1(414544ec1a9aaffb751beaf63d937ce78d0cf9c6) )
	ROM_LOAD16_BYTE( "mb1obj1u.8b", 0x0400000, 0x200000, CRC(e8ff9082) SHA1(a8c7feb33f6243f1f3bda00deffa695ac2b19171) )
	ROM_LOAD16_BYTE( "mb1obj2l.4a", 0x0800001, 0x200000, CRC(3a5c7379) SHA1(ffe9a229eb04a894e5f3bb8ac2fc4617b5413ac3) )
	ROM_LOAD16_BYTE( "mb1obj2u.8b", 0x0800000, 0x200000, CRC(b59cf5e0) SHA1(eee7511f117a4c1a24e4187e3f30e4d66f914a81) )
	ROM_LOAD16_BYTE( "mb1obj3l.6c", 0x0c00001, 0x200000, CRC(9a765d58) SHA1(2e9ea0f76f80383fcf093e947e1fe161743e33fb) )
	ROM_LOAD16_BYTE( "mb1obj3u.9c", 0x0c00000, 0x200000, CRC(5329c693) SHA1(955b3b8b9813826347a1211f71fa0a294b759ccd) )
	ROM_LOAD16_BYTE( "mb1obj4l.6b", 0x1000001, 0x200000, CRC(a650b05e) SHA1(b247699433c7bf4b6ae990fc06255cfd48a248dd) )
	ROM_LOAD16_BYTE( "mb1obj4u.9b", 0x1000000, 0x200000, CRC(6d0c37e9) SHA1(3a3feb74b890e0a933dcc791e5eee1fb4bdcbb69) )

	ROM_REGION( 0x400000, NAMCONB1_ROTGFXREGION, 0 )
	ROM_LOAD( "mb1_rot0.3d", 0x000000, 0x200000, CRC(bc353630) SHA1(2bbddda632298899716394ddcfe51412576ca74a) )
	ROM_LOAD( "mb1_rot1.3c", 0x200000, 0x200000, CRC(cf7688cb) SHA1(29a040ce2c4e3bf671cff1a7a1ade06103db236a) )

	ROM_REGION( 0x600000, NAMCONB1_TILEGFXREGION, 0 )
	ROM_LOAD( "mb1_scr0.1d", 0x000000, 0x200000, CRC(c678d5f3) SHA1(98d1523bef50d444be9485c4e7f6932cccbea191) )
	ROM_LOAD( "mb1_scr1.1c", 0x200000, 0x200000, CRC(fb2b1939) SHA1(bf9d7b93205e7012aa86693f3d2ba8f4d729bc97) )
	ROM_LOAD( "mb1_scr2.1b", 0x400000, 0x200000, CRC(0e6097a5) SHA1(b6c64b3e34ba913138b6b7c3d99d2be4f3ceda08) )

	ROM_REGION32_BE( 0x100000, "data", 0 )
	ROM_LOAD16_WORD_SWAP( "mb1_dat0.20a", 0x00000, 0x80000, CRC(fb2e3cd1) SHA1(019b1d645a07619036522f42e0b9a537f39b6b93) )
ROM_END


/***************************************************************/

GAME( 1994, nebulray, 0,        namconb1, namconb1, namconb1_state, nebulray, ROT90, "Namco", "Nebulas Ray (World, NR2)", MACHINE_IMPERFECT_SOUND )
GAME( 1994, nebulrayj,nebulray, namconb1, namconb1, namconb1_state, nebulray, ROT90, "Namco", "Nebulas Ray (Japan, NR1)", MACHINE_IMPERFECT_SOUND )
GAME( 1994, ptblank,  0,        namconb1, gunbulet, namconb1_state, gunbulet, ROT0,  "Namco", "Point Blank (World, GN2 Rev B, set 1)", MACHINE_IMPERFECT_SOUND )
GAME( 1994, ptblanka, ptblank,  namconb1, gunbulet, namconb1_state, gunbulet, ROT0,  "Namco", "Point Blank (World, GN2 Rev B, set 2)", MACHINE_IMPERFECT_SOUND )
GAME( 1994, gunbuletj,ptblank,  namconb1, gunbulet, namconb1_state, gunbulet, ROT0,  "Namco", "Gun Bullet (Japan, GN1)", MACHINE_IMPERFECT_SOUND )
GAME( 1994, gunbuletw,ptblank,  namconb1, gunbulet, namconb1_state, gunbulet, ROT0,  "Namco", "Gun Bullet (World, GN3 Rev B)", MACHINE_IMPERFECT_SOUND )
GAME( 1993, gslugrsj, 0,        namconb1, nbsports, namconb1_state, gslgr94u, ROT0,  "Namco", "Great Sluggers (Japan)", MACHINE_IMPERFECT_SOUND )
GAME( 1994, gslgr94u, 0,        namconb1, nbsports, namconb1_state, gslgr94u, ROT0,  "Namco", "Great Sluggers '94", MACHINE_IMPERFECT_SOUND )
GAME( 1994, gslgr94j, gslgr94u, namconb1, nbsports, namconb1_state, gslgr94j, ROT0,  "Namco", "Great Sluggers '94 (Japan)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, sws95,    0,        namconb1, nbsports, namconb1_state, sws95,    ROT0,  "Namco", "Super World Stadium '95 (Japan)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, sws96,    0,        namconb1, nbsports, namconb1_state, sws96,    ROT0,  "Namco", "Super World Stadium '96 (Japan)", MACHINE_IMPERFECT_SOUND )
GAME( 1997, sws97,    0,        namconb1, nbsports, namconb1_state, sws97,    ROT0,  "Namco", "Super World Stadium '97 (Japan)", MACHINE_IMPERFECT_SOUND )
GAME( 1994, vshoot,   0,        namconb1, namconb1, namconb1_state, vshoot,   ROT0,  "Namco", "J-League Soccer V-Shoot (Japan)", MACHINE_IMPERFECT_SOUND )

/*     YEAR, NAME,     PARENT,   MACHINE,  INPUT,    INIT,     MNTR,  COMPANY, FULLNAME,   FLAGS */
GAME( 1994, outfxies, 0,        namconb2, outfxies, namconb1_state, outfxies, ROT0, "Namco", "The Outfoxies (World, OU2)", MACHINE_IMPERFECT_SOUND )
GAME( 1994, outfxiesj,outfxies, namconb2, outfxies, namconb1_state, outfxies, ROT0, "Namco", "The Outfoxies (Japan, OU1)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, machbrkr, 0,        namconb2, namconb1, namconb1_state, machbrkr, ROT0, "Namco", "Mach Breakers - Numan Athletics 2 (Japan)", MACHINE_IMPERFECT_SOUND )
