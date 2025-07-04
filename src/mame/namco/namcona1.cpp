// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

Namco NA-1 / NA-2 System

NA-1 Games:
-   Bakuretsu Quiz Ma-Q Dai Bouken (C348)
-   F/A / Fighter & Attacker (C349)
-   Super World Court (C357)
-   Exvania (C350)
-   Cosmo Gang the Puzzle (C356)
-   Tinkle Pit (C367)
-   Emeraldia (C358)

NA-2 Games:
-   Knuckle Heads (C360)
-   Numan Athletics (C359)
-   Emeraldia (C358)
-   Nettou! Gekitou! Quiztou!! (C365)
-   X-Day 2 (C394)
-   Zelos (no keycus)


To Do:
- Remove remaining MCU simulation hacks
- View area / screen resolution controlled by registers?
- Canned EEPROM data for [see below*] oughtn't be necessary; when their EEPROM
  area is uninitialized, the game software automatically writes these values there,
  but then hangs.
  *cgangpzl, cgangpzlj, exvania, exvaniaj, knckheadjp, quiztou
- xday2: unemulated printer and RTC devices (check test mode game options)

- X-Day 2:
    Rom board  M112
    Rom board custom Key chip i.d. C394
    Game uses a small cash-register type printer (connects to rom board)
    Game also has a large L.E.D. type score board with several displays for various scores. (connects to rom board)
    Game uses coin-type battery on rom board. (not suicide)
    Game won't startup unless printer is connected and with paper.


The board has a 28c16 EEPROM

No other CPUs on the board, but there are many custom chips.
Mother Board:
210 (28pin SOP)
C69 or C70 (80pin PQFP)
215 (176pin PQFP)
C218 (208pin PQFP)
219 (160pin PQFP)
Plus 1 or 2 custom chips on ROM board.

Notes:

-   NA-2 is backwards compatible with NA-1.

-   Some NA-1 PCBs are known to have the NA-2 (C70) MCU BIOS, so that cannot
    be used to tell them apart.

-   All rom pcb's have PLD's NA1R10 and NA1R2 OR they have a C354 in place of
    the 2 PLD's doing the same job (enable lines on the rom board)
    Most games have been seen in both configurations / board styles.
    C354 is not to be confused with the Keycus, Both devices are 32pin and C3xx.

-   Test mode for NA2 games includes an additional item: UART Test.
    No games are known to actually link up and use the UART feature.
    It's been confirmed that a Numan Athletics fails the UART test,
    behaving as it does in MAME.

-   Quiz games use 1p button 1 to pick test, 2p button 1 to begin test,
    and 2p button 2 to exit. Because quiz games doesn't have joystick.

-   Almost all quiz games using JAMMA edge connector assign
    button1 to up, button 2 to down, button 3 to left, button 4 to right.
    But Taito F2 quiz games assign button 3 to right and button 4 to left.

-   Emeralda: Byte 0x25 of the NVRAM, when zero, enables the FBI logo.


Namco NA2 hardware
------------------

PCB Layout
----------

NA-2 GAME PCB
8627961102
(8627963102)
|---------------------------------------------|
|  LA4705           T062               |----| |
|         JP3   51568  LC7881          | 6  | |
|J        210              LH52250     | 8  | |
|A    VOL1      VOL2       LH52250     | 0  | |
|M                         |--------|  | 0  | |
|M            |------|     |  C218  |  | 0  | |
|A  TC514256  | 215  |     |        |  |----| |
|   TC514256  |      |     |        |         |
|             |------|     |--------|50.113MHz|
|            |---------ROM-BOARD--------------+
|     TC51832|  LH5168       TC514256         |
|   DSW(2)   |               TC514256         |
|            |               TC514256         |
|            |               TC514256         |
|            | |----|       |--------|    J103|
|4   IR2C24  | |C70 |       |  219   |        |
|8           | |----|       |        |        |
|W   PC9D10  |              |        |        |
|A     IR2C24|              |--------|        |
|Y           |       3773           TC51832   |
|------------+--------------------------------|
Notes:
       68000 clock - 12.5282MHz [50.113/4]
       TC51832     - 32k x8 SRAM (SOP28)
       TC514256    - 256k x4 DRAM (ZIP19)
       LH5168      - 8k x8 SRAM (SOP28)
       LC7881      - Sanyo 16bit DAC (SOIC20)
       LH52250     - 32k x8 SRAM (SOP28)
       51568       - ??, possibly audio related functions, manufactured by Mitsubishi (SOIC24)
       LA4705      - 15W 2-Channel Audio Power Amplifier (SIL18)
       T062        - Op AMP? (SOIC8)
       3773        - System reset and watchdog IC (SOIC8)
       J103        - Multi pin connector for ROM Board
       JP3         - Stereo/Mono jumper. For stereo, use pin 1 and 2 of the 48-WAY Namco connector for the 2nd speaker
       VSync       - 60Hz
       HSync       - 15.73kHz
       Namco Custom-
                     C218- (QFP208)
                     C70 - rebadged M37702 (QFP80)
                     219 - (QFP160)
                     215 - (QFP176)
                     210 - Video output related, tied to C215 (SOP28)

ROM Board
---------

NA E4M8 ROM PCB
8626961200
(8626963200)
|--------------------------------+
| PAL1    28C16        KEYCUS    |
| PAL2                           |
|                                |
| ROM.7F   ROM.5F  ROM.3F        |
|   ROM.6F  ROM.4F  ROM.2F   J201|
|                                |
|                                |
|                                |
| ROM.7C   ROM.5C  ROM.3C        |
|   ROM.6C  ROM.4C  ROM.2C       |
+--------------------------------|
Notes:
      J201   - Multi pin connector for main board
      KEYCUS - C365 for 'Nettou! Gekitou! Quiztou!!' (DIP32)
      PAL1   - PAL16L8 labelled 'NR1R2' (DIP20)
      PAL2   - PAL16L8 labelled 'NR1R10' (DIP20)
      ROMs   -
               7F/6F/7C/6C - 27C040 EPROM (DIP32)
               All other ROMs are MB838000 8MBit MaskROM (DIP32)
               ROM Names/locations for 'Nettou! Gekitou! Quiztou!!'....
               QT1EP1L.7C
               QT1EP1U.7F
               QT1EP0L.6C
               QT1EP0U.6F
               QT1MA3L.5C
               QT1MA3U.5F
               QT1MA2L.4C
               QT1MA2U.4F
               QT1MA1L.3C
               QT1MA1U.3F
               QT1MA0L.2C
               QT1MA0U.2F

***************************************************************************/

#include "emu.h"
#include "namcona1.h"
#include "cpu/m68000/m68000.h"
#include "speaker.h"

#define MASTER_CLOCK    XTAL(50'113'000)


/*************************************************************************/

/* FIXME: These two functions shouldn't be necessary? */
void namcona1_state::simulate_mcu()
{
	m_workram[0xf60 / 2] = 0x0000; /* mcu ready */
}


/* NA2 hardware sends a special command to the MCU, then tests to
 * see if the proper BIOS version string appears in shared memory.
 */
void namcona1_state::write_version_info()
{
	static const u16 source[0x8] =
	{
		/* "NSA-BIOS ver"... */
		0x534e,0x2d41,0x4942,0x534f,0x7620,0x7265,0x2e31,0x3133
	};
	for (int i = 0; i < 8; i++)
	{
		m_workram[0x1000 / 2 + i] = source[i];
	}
}

/**
 * "Custom Key" Emulation
 *
 * The primary purpose of the custom key chip is protection.  It prevents
 * ROM swaps from working with games that otherwise run on the same hardware.
 *
 * The secondary purpose of the custom key chip is to act as a random number
 * generator in some games.
 */
u16 namcona1_state::custom_key_r(offs_t offset)
{
	const u16 old_count = m_count;
	if (!machine().side_effects_disabled())
	{
		do
		{
			m_count = machine().rand();
		} while (old_count == m_count);
	}

	switch (m_gametype)
	{
	case NAMCO_BKRTMAQ:
		if (offset == 2) return 0x015c;
		break;

	case NAMCO_FA:
		if (offset == 2) return 0x015d;
		if (offset == 4) return m_count;
		break;

	case NAMCO_EXVANIA:
		if (offset == 2) return 0x015e;
		break;

	case NAMCO_CGANGPZL:
		if (offset == 1) return 0x0164;
		if (offset == 2) return m_count;
		break;

	case NAMCO_SWCOURT:
		if (offset == 1) return 0x0165;
		if (offset == 2) return m_count;
		break;

	case NAMCO_EMERALDA:
		if (offset == 1) return 0x0166;
		if (offset == 2) return m_count;
		break;

	case NAMCO_NUMANATH:
		if (offset == 1) return 0x0167;
		if (offset == 2) return m_count;
		break;

	case NAMCO_KNCKHEAD:
		if (offset == 1) return 0x0168;
		if (offset == 2) return m_count;
		break;

	case NAMCO_QUIZTOU:
		if (offset == 2) return 0x016d;
		break;

	case NAMCO_TINKLPIT:
		if (offset == 7) return 0x016f;
		if (offset == 4) m_keyval = 0;
		if (offset == 3)
		{
			u16 res = bitswap<16>(m_keyval, 22,26,31,23,18,20,16,30,24,21,25,19,17,29,28,27);

			if (!machine().side_effects_disabled())
			{
				m_keyval >>= 1;
//              printf("popcount(%08X) = %d\n", m_keyval & 0x58000c00, population_count_32(m_keyval & 0x58000c00));
				if ((!m_keyval) || (population_count_32(m_keyval & 0x58000c00) & 1))
					m_keyval ^= 0x80000000;
			}
			return res;
		}
		break;

	case NAMCO_XDAY2:
		if (offset == 2) return 0x018a;
		if (offset == 3) return m_count;
		break;

	case NAMCO_SWCOURTB: // TODO: this hasn't got a real keycus, see comments above ROM definitions
		if (offset == 1) return 0x8061;
		if (offset == 2) return m_count;
		break;

	default:
		return 0;
	}
	return machine().rand()&0xffff;
}

void namcona1_state::custom_key_w(u16 data)
{
}

/***************************************************************/

int namcona1_state::transfer_dword(u32 dest, u32 source)
{
	u16 data;

	if (source >= 0x400000 && source < 0xc00000)
	{
		data = m_maskrom[(source - 0x400000) / 2];
	}
	else if (source >= 0xc00000 && source < 0xe00000)
	{
		data = m_prgrom[(source - 0xc00000) / 2];
	}
	else if (source < 0x80000 && source >= 0x1000)
	{
		data = m_workram[source / 2];
	}
	else
	{
		logerror("bad blt src %08x\n", source);
		return -1;
	}
	if (dest >= 0xf00000 && dest < 0xf02000)
	{
		paletteram_w((dest - 0xf00000) / 2, data);
	}
	else if (dest >= 0xf40000 && dest < 0xf80000)
	{
		gfxram_w((dest - 0xf40000) / 2, data);
	}
	else if (dest >= 0xff0000 && dest < 0xffc000)
	{
		videoram_w((dest - 0xff0000) / 2, data);
	}
	else if (dest >= 0xfff000 && dest < 0x1000000)
	{
		m_spriteram[(dest - 0xfff000) / 2] = data;
	}
	// xday2 writes to 0x1e01200 / 0x1e01400, assume it's just a mirror for paletteram transfer
	else if (dest >= 0xf00000 * 2 && dest < 0xf02000 * 2)
	{
		paletteram_w((dest - 0xf00000 * 2) / 2, data);
	}
	else
	{
		logerror("bad blit dest %08x\n", dest);
		return -1;
	}
	return 0;
}

void namcona1_state::blit_setup(int format, int *bytes_per_row, int *pitch, int mode)
{
	if (mode == 3)
	{
		/* TILE DATA */
		switch (format)
		{
		case 0x0001:
			*bytes_per_row = 0x1000;
			*pitch = 0x1000;
			break;

		case 0x0081:
			*bytes_per_row = 4 << 3;
			*pitch = 36 << 3;
			break;

		default:
//      case 0x00f1:
//      case 0x00f9:
//      case 0x00fd:
			*bytes_per_row = (64 - (format >> 2)) << 3;
			*pitch = 0x200;
			break;
		}
	}
	else
	{
		/* SHAPE DATA */
		switch (format)
		{
		case 0x00bd: /* Numan Athletics */
			*bytes_per_row = 4;
			*pitch = 0x120;
			break;
		case 0x008d: /* Numan Athletics */
			*bytes_per_row = 8;
			*pitch = 0x120;
			break;

		case 0x0000: /* Numan (used to clear spriteram) */
//      0000 0000 0000 : src0
//      0000 0001 0000 : dst0
//      003d 75a0      : src (7AEB40)
//      ---- ----      : spriteram
//      0800           : numbytes
//      0000           : blit
			*bytes_per_row = 0x10;
			*pitch = 0;
			break;

		case 0x0001:
			*bytes_per_row = 0x1000;
			*pitch = 0x1000;
			break;

		case 0x0401: /* F/A */
			*bytes_per_row = 4 << 6;
			*pitch = 36 << 6;
			break;

		default:
//      case 0x00f1:
//      case 0x0781:
//      case 0x07c1:
//      case 0x07e1:
			*bytes_per_row = (64 - (format >> 5)) << 6;
			*pitch = 0x1000;
			break;
		}
	}
}

void namcona1_state::blit()
{
	int src0 = m_vreg[0x0];
	int src1 = m_vreg[0x1];
	int src2 = m_vreg[0x2];

	int dst0 = m_vreg[0x3];
	int dst1 = m_vreg[0x4];
	int dst2 = m_vreg[0x5];

	int gfxbank = m_vreg[0x6];

	/* dest and source are provided as dword offsets */
	u32 src_baseaddr = 2 * (0xffffff & ((m_vreg[0x7] << 16) | m_vreg[0x8]));
	u32 dst_baseaddr = 2 * (0xffffff & ((m_vreg[0x9] << 16) | m_vreg[0xa]));

	int num_bytes = m_vreg[0xb];

	int dst_offset, src_offset;
	int dst_bytes_per_row, dst_pitch;
	int src_bytes_per_row, src_pitch;

	(void)dst2;
	(void)dst0;
	(void)src2;
	(void)src0;
/*
    logerror("%s: blt(%08x,%08x,numBytes=%04x);src=%04x %04x %04x; dst=%04x %04x %04x; gfx=%04x\n",
        machine().describe_context(),
        dst_baseaddr,src_baseaddr,num_bytes,
        src0,src1,src2,
        dst0,dst1,dst2,
        gfxbank);
*/
	blit_setup(dst1, &dst_bytes_per_row, &dst_pitch, gfxbank);
	blit_setup(src1, &src_bytes_per_row, &src_pitch, gfxbank);

	if (num_bytes&1)
	{
		num_bytes++;
	}

	if (dst_baseaddr < 0xf00000)
	{
		dst_baseaddr += 0xf40000;
	}

	dst_offset = 0;
	src_offset = 0;

	while (num_bytes > 0)
	{
		if (transfer_dword(
			dst_baseaddr + dst_offset,
			src_baseaddr + src_offset))
		{
			return;
		}

		num_bytes -= 2;

		dst_offset+=2;
		if (dst_offset >= dst_bytes_per_row)
		{
			dst_baseaddr += dst_pitch;
			dst_offset = 0;
		}

		src_offset+=2;
		if (src_offset >= src_bytes_per_row)
		{
			src_baseaddr += src_pitch;
			src_offset = 0;
		}
	}
}

void namcona1_state::vreg_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 old = m_vreg[offset];
	data = COMBINE_DATA(&m_vreg[offset]);

	switch (offset)
	{
	case 0x18 / 2:
		blit();
		/* see also 0x1e */
		break;

	case 0x1a / 2:
		m_enable_interrupts = 1;
		/* interrupt enable mask; 0 enables INT level */
		break;
	case 0xb0 / 2:
	case 0xb2 / 2:
	case 0xb4 / 2:
	case 0xb6 / 2:
		m_bg_tilemap[offset - (0xb0 / 2)]->set_palette_offset((data & 0xf) << 8);
		break;

	case 0xba / 2:
		m_bg_tilemap[4]->set_palette_offset((data & 0xf) << 8);
		break;

	case 0xbc / 2:
		for (int i = 0; i <= 4; i++)
		{
			if (BIT(old ^ data, i))
				m_bg_tilemap[i]->mark_all_dirty();
		}
		break;
	}
}

/***************************************************************/

// MCU "mailslot" handler - has 8 16-bit slots mirrored

u16 namcona1_state::mcu_mailbox_r(offs_t offset)
{
	return m_mcu_mailbox[offset & 7];
}

void namcona1_state::mcu_mailbox_w_68k(offs_t offset, u16 data, u16 mem_mask)
{
//  logerror("mailbox_w_68k: %x @ %x\n", data, offset);

	if (offset == 4)
		m_mcu->set_input_line(M37710_LINE_IRQ0, HOLD_LINE);

	COMBINE_DATA(&m_mcu_mailbox[offset & 7]);

	/* FIXME: This shouldn't be necessary now that the C70 BIOS is implemented,
	but for some reason the MCU never responds to the version string command */
	if ((m_gametype == NAMCO_NUMANATH) || (m_gametype == NAMCO_KNCKHEAD))
	{
		if ((m_workram[0xf72 / 2] >> 8) == 7)
			write_version_info();
	}
}

void namcona1_state::mcu_mailbox_w_mcu(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mcu_mailbox[offset & 7]);
}

void namcona1_state::namcona1_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).ram().share("workram");
	map(0x3f8000, 0x3fffff).rw(FUNC(namcona1_state::mcu_mailbox_r), FUNC(namcona1_state::mcu_mailbox_w_68k));
	map(0x400000, 0xbfffff).rom().region("maskrom", 0);  // data
	map(0xc00000, 0xdfffff).rom().region("maincpu", 0);  // code
	map(0xe00000, 0xe00fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0xe40000, 0xe4000f).rw(FUNC(namcona1_state::custom_key_r), FUNC(namcona1_state::custom_key_w));
	map(0xefff00, 0xefffff).ram().w(FUNC(namcona1_state::vreg_w)).share("vreg");
	map(0xf00000, 0xf01fff).ram().w(FUNC(namcona1_state::paletteram_w)).share("paletteram");
	map(0xf40000, 0xf7ffff).rw(FUNC(namcona1_state::gfxram_r), FUNC(namcona1_state::gfxram_w)).share("cgram");
	map(0xff0000, 0xffbfff).ram().w(FUNC(namcona1_state::videoram_w)).share("videoram");
	map(0xffc000, 0xffdfff).ram();                         /* expects RAM here for some games or it won't boot*/
	map(0xffe000, 0xffefff).ram().share("scroll");      /* scroll registers */
	map(0xfff000, 0xffffff).ram().share("spriteram");   /* spriteram */
}

void namcona1_state::namcona1_c219_map(address_map &map)
{
	map(0x000000, 0x07ffff).ram().share("workram");
}

void namcona2_state::zelos_ctrl_w(u16 data)
{
	// bit 15 to 7 are set during I/O test when switching between the 9 'windows'. Output test? Maybe which 'window' screen to update?
	// at least bit 4 to 1 are used, too

	m_zelos_ctrl = data;

	//if (data & 0x007f)
	//  logerror("zelos_ctrl_w: %04x\n", data);
}

void namcona2_state::zelos_main_map(address_map &map)
{
	namcona1_main_map(map);

	map(0xd00000, 0xd00001).w(FUNC(namcona2_state::zelos_ctrl_w));
	// map(0xd40000, 0xd40001).w(FUNC(namcona2_state::)); // bit 1 alternatively set and cleared in test mode
}

u8 xday2_namcona2_state::printer_r()
{
	// --xx ---- printer status related, if bit 5 held 1 long enough causes printer error
	// ---- --11 battery ok, any other setting causes ng
	return 3;
}

void xday2_namcona2_state::printer_w(u8 data)
{
	// ...
}

void xday2_namcona2_state::xday2_main_map(address_map &map)
{
	namcona1_main_map(map);
	// these two seems lamps and flash related (mux too?)
	map(0xd00000, 0xd00001).nopw();
	map(0xd40000, 0xd40001).nopw();
	map(0xd80001, 0xd80001).rw(FUNC(xday2_namcona2_state::printer_r), FUNC(xday2_namcona2_state::printer_w)); /* xday: serial out? */
	map(0xdc0000, 0xdc001f).rw(m_rtc, FUNC(msm6242_device::read), FUNC(msm6242_device::write)).umask16(0x00ff); /* RTC device */

	// seems bigger than standard na1 (otherwise you won't get proper ranking defaults)
	map(0xe00000, 0xe03fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
}


/* ----- NA-1 MCU handling ----------------------------------- */

u16 namcona1_state::na1mcu_shared_r(offs_t offset)
{
	u16 data = swapendian_int16(m_workram[offset]);

#if 0
	if (offset >= 0x70000 / 2)
	{
		logerror("MD: %04x @ %x %s\n", data, offset*2, machine().describe_context());
	}
#endif
	return data;
}

void namcona1_state::na1mcu_shared_w(offs_t offset, u16 data, u16 mem_mask)
{
	mem_mask = swapendian_int16(mem_mask);
	data = swapendian_int16(data);

	COMBINE_DATA(&m_workram[offset]);
}

void namcona1_state::namcona1_mcu_map(address_map &map)
{
	map(0x000800, 0x000fff).rw(FUNC(namcona1_state::mcu_mailbox_r), FUNC(namcona1_state::mcu_mailbox_w_mcu)); // "Mailslot" communications ports
	map(0x001000, 0x0011ff).mirror(0x000e00).rw(m_c219, FUNC(c219_device::c219_le_r), FUNC(c219_device::c219_le_w)); // C140-alike sound chip
	map(0x002000, 0x002fff).rw(FUNC(namcona1_state::na1mcu_shared_r), FUNC(namcona1_state::na1mcu_shared_w)); // mirror of first page of shared work RAM
	map(0x003000, 0x00afff).ram();                     // there is a 32k RAM chip according to CGFM
	map(0x200000, 0x27ffff).rw(FUNC(namcona1_state::na1mcu_shared_r), FUNC(namcona1_state::na1mcu_shared_w)); // shared work RAM
}


// port 4: bit 3 (0x08) enables the 68000 (see the 68k launch code at c604 in swcourt's BIOS)
u8 namcona1_state::port4_r()
{
	return m_mcu_port4;
}

void namcona1_state::port4_w(u8 data)
{
	if ((data & 0x08) && !(m_mcu_port4 & 0x08))
	{
		logerror("launching 68k, %s\n", machine().describe_context());

		// reset and launch the 68k
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}

	m_mcu_port4 = data;
}

// port 5: not sure yet, but MCU code requires this interaction at least
u8 namcona1_state::port5_r()
{
	return m_mcu_port5;
}

void namcona1_state::port5_w(u8 data)
{
	m_mcu_port5 = data;

	// bit 0 must mirror bit 1 - this is checked at CCD3 in the C69 BIOS
	m_mcu_port5 &= 0xfe;
	m_mcu_port5 |= ((m_mcu_port5 & 0x2)>>1);
}

u8 namcona1_state::port6_r()
{
	return 0;
}

void namcona1_state::port6_w(u8 data)
{
	m_mcu_port6 = data;
}

u8 namcona1_state::port7_r()
{
	if ((m_mcu_port6 & 0x80) == 0)
		return m_muxed_inputs[m_mcu_port6 >> 5]->read();
	else
		return 0xff;
}

void namcona1_state::port7_w(u8 data)
{
}

// port 8: bit 5 (0x20) toggles, watchdog?
u8 namcona1_state::port8_r()
{
	return m_mcu_port8;
}

void namcona1_state::port8_w(u8 data)
{
	m_mcu_port8 = data;
}


void namcona1_state::machine_start()
{
	m_enable_interrupts = 0;
	std::fill(std::begin(m_mcu_mailbox), std::end(m_mcu_mailbox), 0);
	save_item(NAME(m_enable_interrupts));
	save_item(NAME(m_count));
	save_item(NAME(m_mcu_mailbox));
	save_item(NAME(m_mcu_port4));
	save_item(NAME(m_mcu_port5));
	save_item(NAME(m_mcu_port6));
	save_item(NAME(m_mcu_port8));

	m_scan_timer = timer_alloc(FUNC(namcona1_state::set_scanline_interrupt), this);
}

// the MCU boots the 68000
void namcona1_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_mcu_port4 = 0;
	m_mcu_port5 = 1;

	m_scan_timer->adjust(m_screen->scan_period(), 0, m_screen->scan_period());
}

// each bit of player 3's inputs is split across one of the 8 analog input ports
// bit 6 => port 0
// bit 5 => port 1
// bit 4 => port 2
// bit 0 => port 3
// bit 1 => port 4
// bit 2 => port 5
// bit 3 => port 6
// bit 7 => port 7
template <int Bit>
u16 namcona1_state::portana_r()
{
	return BIT(m_io_p3->read(), Bit) ? 0xffff : 0x0000;
}


/***************************************************************************/

static INPUT_PORTS_START(namcona1_joy)
	PORT_START("P1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START1)

	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START2)

	PORT_START("P3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START3)

	PORT_START("P4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START4)

	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x01, "DIP2 (Freeze)")
	PORT_DIPSETTING(   0x01, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "DIP1 (Test)")
	PORT_DIPSETTING(   0x02, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_SERVICE(0x40, IP_ACTIVE_LOW)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SERVICE1)
INPUT_PORTS_END

static INPUT_PORTS_START(namcona1_quiz)
	PORT_START("P1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START1)

	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START2)

	PORT_START("P3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START3)

	PORT_START("P4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START4)

	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x01, "DIP2 (Freeze)")
	PORT_DIPSETTING(   0x01, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "DIP1 (Test)")
	PORT_DIPSETTING(   0x02, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_SERVICE(0x40, IP_ACTIVE_LOW)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SERVICE1)
INPUT_PORTS_END

static INPUT_PORTS_START(zelost) // TODO: to be adjusted when the game will work, for now using PORT_NAME to name them as the I/O test does
	PORT_START("P1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON8) PORT_PLAYER(3) PORT_NAME("LINE 8")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(3) PORT_NAME("LINE 6")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(3) PORT_NAME("LINE 4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(3) PORT_NAME("LINE 2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("D.GAME") // double up game?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("PAY OUT")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN) // no effect in I/O test
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN) // "

	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("A.L.B.") // ???
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_PLAYER(3) PORT_NAME("LINE 7")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(3) PORT_NAME("LINE 5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(3) PORT_NAME("LINE 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("T.SCORE") // take score?
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN) // no effect in I/O test
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN) // "
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(3) PORT_NAME("LINE 1")

	PORT_START("P3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN) // no effect in I/O test

	PORT_START("P4")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN) // no effect in I/O test

	PORT_START("DSW")
	PORT_DIPNAME(0x01, 0x01, "DIP2 (Freeze)")
	PORT_DIPSETTING(   0x01, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "DIP1 (Test)")
	PORT_DIPSETTING(   0x02, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // no effect in I/O test
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN) // no effect in I/O test
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN) // no effect in I/O test
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN) // no effect in I/O test
	PORT_SERVICE(0x40, IP_ACTIVE_LOW)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN) // no effect in I/O test
INPUT_PORTS_END

/***************************************************************************/

static const gfx_layout cg_layout_8bpp =
{
	8,8,
	RGN_FRAC(1,1),
	8, /* 8BPP */
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout cg_layout_4bpp =
{
	8,8,
	RGN_FRAC(1,1),
	4, /* 4BPP */
	{ STEP4(4, 1) },
	{ STEP8(0, 8) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout shape_layout =
{
	8,8,
	0x1000,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	8*8
};

static GFXDECODE_START(gfx_namcona1)
	GFXDECODE_RAM("cgram",  0, cg_layout_8bpp, 0, 0x2000 / 256)
	GFXDECODE_RAM("cgram",  0, cg_layout_4bpp, 0, 0x2000 / 16 )
	GFXDECODE_RAM( nullptr, 0, shape_layout,   0, 0x2000 / 2  )
GFXDECODE_END

/***************************************************************************/

// MCU interrupts: IRQ 0 => process mail slot (probably set on mail slot write from 68k)
//                 IRQ 1 =>
//                 IRQ 2 =>

void namcona1_state::scanline_interrupt(int scanline)
{
	const u16 enabled = m_enable_interrupts ? ~m_vreg[0x1a / 2] : 0;

	// vblank
	if (scanline == 224)
	{
		m_mcu->set_input_line(M37710_LINE_IRQ1, HOLD_LINE);
		simulate_mcu();
		if (enabled & 8)
			m_maincpu->set_input_line(4, HOLD_LINE);
	}

	// posirq, used with dolphin in Emeraldia's "how to play" attract mode
	const u16 posirq_scanline = m_vreg[0x8a / 2] & 0xff;
	if (scanline == posirq_scanline && enabled & 4)
	{
		if (posirq_scanline)
			m_screen->update_partial(posirq_scanline);

		m_maincpu->set_input_line(3, HOLD_LINE);
	}
}

TIMER_CALLBACK_MEMBER(namcona1_state::set_scanline_interrupt)
{
	scanline_interrupt(m_screen->vpos());
}

void namcona1_state::c69(machine_config &config)
{
	NAMCO_C69(config, m_mcu, MASTER_CLOCK/4);
	m_mcu->set_addrmap(AS_PROGRAM, &namcona1_state::namcona1_mcu_map);
	m_mcu->p4_in_cb().set(FUNC(namcona1_state::port4_r));
	m_mcu->p4_out_cb().set(FUNC(namcona1_state::port4_w));
	m_mcu->p5_in_cb().set(FUNC(namcona1_state::port5_r));
	m_mcu->p5_out_cb().set(FUNC(namcona1_state::port5_w));
	m_mcu->p6_in_cb().set(FUNC(namcona1_state::port6_r));
	m_mcu->p6_out_cb().set(FUNC(namcona1_state::port6_w));
	m_mcu->p7_in_cb().set(FUNC(namcona1_state::port7_r));
	m_mcu->p7_out_cb().set(FUNC(namcona1_state::port7_w));
	m_mcu->p8_in_cb().set(FUNC(namcona1_state::port8_r));
	m_mcu->p8_out_cb().set(FUNC(namcona1_state::port8_w));
	m_mcu->an0_cb().set(FUNC(namcona1_state::portana_r<6>));
	m_mcu->an1_cb().set(FUNC(namcona1_state::portana_r<5>));
	m_mcu->an2_cb().set(FUNC(namcona1_state::portana_r<4>));
	m_mcu->an3_cb().set(FUNC(namcona1_state::portana_r<0>));
	m_mcu->an4_cb().set(FUNC(namcona1_state::portana_r<1>));
	m_mcu->an5_cb().set(FUNC(namcona1_state::portana_r<2>));
	m_mcu->an6_cb().set(FUNC(namcona1_state::portana_r<3>));
	m_mcu->an7_cb().set(FUNC(namcona1_state::portana_r<7>));
}

/* cropped at sides */
void namcona1_state::namcona_base(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, MASTER_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcona1_state::namcona1_main_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(40*8, 32*8);
//  m_screen->set_visarea(8, 38*8-1-8, 4*8, 32*8-1);
	m_screen->set_visarea(0, 38*8-1, 4*8, 32*8-1);
	m_screen->set_screen_update(FUNC(namcona1_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, m_palette).set_entries(0x2000).enable_shadows();

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_namcona1);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	C219(config, m_c219, 44100);
	m_c219->set_addrmap(0, &namcona1_state::namcona1_c219_map);
	m_c219->add_route(0, "speaker", 1.00, 1);
	m_c219->add_route(1, "speaker", 1.00, 0);
}

void namcona1_state::namcona1(machine_config &config)
{
	namcona_base(config);
	c69(config);
	EEPROM_2816(config, "eeprom");
}

void namcona2_state::c70(machine_config &config)
{
	NAMCO_C70(config, m_mcu, MASTER_CLOCK/4);
	m_mcu->set_addrmap(AS_PROGRAM, &namcona2_state::namcona1_mcu_map);
	m_mcu->p4_in_cb().set(FUNC(namcona2_state::port4_r));
	m_mcu->p4_out_cb().set(FUNC(namcona2_state::port4_w));
	m_mcu->p5_in_cb().set(FUNC(namcona2_state::port5_r));
	m_mcu->p5_out_cb().set(FUNC(namcona2_state::port5_w));
	m_mcu->p6_in_cb().set(FUNC(namcona2_state::port6_r));
	m_mcu->p6_out_cb().set(FUNC(namcona2_state::port6_w));
	m_mcu->p7_in_cb().set(FUNC(namcona2_state::port7_r));
	m_mcu->p7_out_cb().set(FUNC(namcona2_state::port7_w));
	m_mcu->p8_in_cb().set(FUNC(namcona2_state::port8_r));
	m_mcu->p8_out_cb().set(FUNC(namcona2_state::port8_w));
	m_mcu->an0_cb().set(FUNC(namcona2_state::portana_r<6>));
	m_mcu->an1_cb().set(FUNC(namcona2_state::portana_r<5>));
	m_mcu->an2_cb().set(FUNC(namcona2_state::portana_r<4>));
	m_mcu->an3_cb().set(FUNC(namcona2_state::portana_r<0>));
	m_mcu->an4_cb().set(FUNC(namcona2_state::portana_r<1>));
	m_mcu->an5_cb().set(FUNC(namcona2_state::portana_r<2>));
	m_mcu->an6_cb().set(FUNC(namcona2_state::portana_r<3>));
	m_mcu->an7_cb().set(FUNC(namcona2_state::portana_r<7>));
}

void namcona2_state::namcona2(machine_config &config)
{
	namcona_base(config);
	c70(config);
	EEPROM_2816(config, "eeprom");

//  m_maincpu->set_addrmap(AS_PROGRAM, &namcona2_state::namcona2_main_map);
}

void namcona2_state::zelos(machine_config &config)
{
	namcona2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &namcona2_state::zelos_main_map);
}

void xday2_namcona2_state::xday2(machine_config &config)
{
	namcona_base(config);
	c70(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &xday2_namcona2_state::xday2_main_map);

	EEPROM_2864(config, "eeprom");

	// TODO: unknown sub type
	MSM6242(config, "rtc", XTAL(32'768));
}

 /* NA-1 Hardware */
void namcona1_state::init_bkrtmaq()         { m_gametype = NAMCO_BKRTMAQ; }
void namcona1_state::init_cgangpzl()        { m_gametype = NAMCO_CGANGPZL; }
void namcona1_state::init_emeraldj()        { m_gametype = NAMCO_EMERALDA; }
void namcona1_state::init_exvania()         { m_gametype = NAMCO_EXVANIA; }
void namcona1_state::init_fa()              { m_gametype = NAMCO_FA; }
void namcona1_state::init_swcourt()         { m_gametype = NAMCO_SWCOURT; }
void namcona1_state::init_swcourtb()        { m_gametype = NAMCO_SWCOURTB; }
void namcona1_state::init_tinklpit()        { m_gametype = NAMCO_TINKLPIT; save_item(NAME(m_keyval)); }

 /* NA-2 Hardware */
void namcona2_state::init_emeralda()        { m_gametype = NAMCO_EMERALDA; }
void namcona2_state::init_knckhead()        { m_gametype = NAMCO_KNCKHEAD; }
void namcona2_state::init_numanath()        { m_gametype = NAMCO_NUMANATH; }
void namcona2_state::init_quiztou()         { m_gametype = NAMCO_QUIZTOU; }
void xday2_namcona2_state::init_xday2()     { m_gametype = NAMCO_XDAY2; }
void namcona2_state::init_zelos()           { m_gametype = -1; save_item(NAME(m_zelos_ctrl)); }

ROM_START(bkrtmaq)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("mq1-ep0l.bin", 0x000001, 0x080000, CRC(f029bc57) SHA1(fdbf8b8b9f69d5755ca5197dda4f887b12dd66f4)) /* 0xc00000 */
	ROM_LOAD16_BYTE("mq1-ep0u.bin", 0x000000, 0x080000, CRC(4cff62b8) SHA1(5cac170dcfbeb3dcfa0840bdbe7541a9d2f44a14))
	ROM_LOAD16_BYTE("mq1-ep1l.bin", 0x100001, 0x080000, CRC(e3be6f4b) SHA1(75d9a4cff25e63a9d6c092aa6e241eccd1c61f91))
	ROM_LOAD16_BYTE("mq1-ep1u.bin", 0x100000, 0x080000, CRC(b44e31b2) SHA1(3d8c63789b98ada3663ba9e28c370815a9a9c3ed))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("mq1-ma0l.bin", 0x000001, 0x100000, CRC(11fed35f) SHA1(511d98b6b42b330238a1874bca031b1892654a48)) /* 0x400000 */
	ROM_LOAD16_BYTE("mq1-ma0u.bin", 0x000000, 0x100000, CRC(23442ac0) SHA1(fac706f24045d51a2712f51530967140ea8e875f))
	ROM_LOAD16_BYTE("mq1-ma1l.bin", 0x200001, 0x100000, CRC(fe82205f) SHA1(860cc7a96ae3f848ce594077c1362e4e22a36908))
	ROM_LOAD16_BYTE("mq1-ma1u.bin", 0x200000, 0x100000, CRC(0cdb6bd0) SHA1(b8b398477c9654e96921110fb30c754240183897))
ROM_END

ROM_START(cgangpzl)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("cp2-ep0l.4c", 0x000001, 0x080000, CRC(8f5cdcc5) SHA1(925db3f3f16224bc28f97a57aba0ab2b51c5067c)) /* 0xc00000 */
	ROM_LOAD16_BYTE("cp2-ep0u.4f", 0x000000, 0x080000, CRC(3a816140) SHA1(613c367e08a0a20ec62e1938faab0128743b26f8))

	ROM_REGION16_BE(0x800000, "maskrom", ROMREGION_ERASE00)
	/* no mask roms */

	ROM_REGION(0x0800, "eeprom", 0) // default eeprom, otherwise game would lock up on 1st boot
	ROM_LOAD("eeprom_cgangpzl", 0x0000, 0x0800, CRC(5f8dfe9e) SHA1(81cc9cdbd8b5d6092a292309f78e3037233078f9))
ROM_END

ROM_START(cgangpzlj)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("cp1-ep0l.4c", 0x000001, 0x080000, CRC(2825f7ba) SHA1(5f6f8df6bdf0f45656904411cdbb31fdcf8f3be0)) /* 0xc00000 */
	ROM_LOAD16_BYTE("cp1-ep0u.4f", 0x000000, 0x080000, CRC(94d7d6fc) SHA1(2460741e0dbb2ccff28f4fbc419a7507382467d2))

	ROM_REGION16_BE(0x800000, "maskrom", ROMREGION_ERASE00)
	/* no mask roms */

	ROM_REGION(0x0800, "eeprom", 0) // default eeprom, otherwise game would lock up on 1st boot
	ROM_LOAD("eeprom_cgangpzlj", 0x0000, 0x0800, CRC(ef5ddff2) SHA1(ea3f8e4da119e27c27f66f169bbf19bc37499048))
ROM_END

ROM_START(emeraldaj) /* NA-1 Game PCB, parent is NA-2 version listed below */
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("em1-ep0lb.6c", 0x000001, 0x080000, CRC(fcd55293) SHA1(fdabf9d5f528c37196ac1e031b097618b4c887b5)) /* 0xc00000 */
	ROM_LOAD16_BYTE("em1-ep0ub.6f", 0x000000, 0x080000, CRC(a52f00d5) SHA1(85f95d2a69a2df2e9195f55583645c064b0b6fe6))
	ROM_LOAD16_BYTE("em1-ep1l.7c",  0x100001, 0x080000, CRC(373c1c59) SHA1(385cb3bc056b798878de890dbff97a8bdd48fe4e))
	ROM_LOAD16_BYTE("em1-ep1u.7f",  0x100000, 0x080000, CRC(4e969152) SHA1(2c89ae5d43585f479f16cf8278f8fc001e077e45))

	ROM_REGION16_BE(0x800000, "maskrom", ROMREGION_ERASE00)
	/* no mask roms */
ROM_END

ROM_START(emeraldaja) /* NA-1 Game PCB, parent is NA-2 version listed below */
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("em1-ep0l.6c", 0x000001, 0x080000, CRC(443f3fce) SHA1(35b6c834e5716c1e9b55f1e39f4e7336dbbe2d9b)) /* 0xc00000 */
	ROM_LOAD16_BYTE("em1-ep0u.6f", 0x000000, 0x080000, CRC(484a2a81) SHA1(1b60c18dfb2aebfd4aa8b2a85a1e90883a1f8e61))
	ROM_LOAD16_BYTE("em1-ep1l.7c", 0x100001, 0x080000, CRC(373c1c59) SHA1(385cb3bc056b798878de890dbff97a8bdd48fe4e))
	ROM_LOAD16_BYTE("em1-ep1u.7c", 0x100000, 0x080000, CRC(4e969152) SHA1(2c89ae5d43585f479f16cf8278f8fc001e077e45))

	ROM_REGION16_BE(0x800000, "maskrom", ROMREGION_ERASE00)
	/* no mask roms */
ROM_END

ROM_START(exvania)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("ex2-ep0l.4c", 0x000001, 0x080000, CRC(ccf46677) SHA1(f9d057a7b1c388323d49ef692f41242769f1a08c)) /* 0xc00000 */
	ROM_LOAD16_BYTE("ex2-ep0u.4f", 0x000000, 0x080000, CRC(37b8d890) SHA1(a0417a1b51d1206bd3eb5e7b58303a9a5691fa43))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("ex1-ma0l.2c", 0x000001, 0x100000, CRC(17922cd4) SHA1(af92c2335e7110c0c5e712f3148c1534d22d3814)) /* 0x400000 */
	ROM_LOAD16_BYTE("ex1-ma0u.2f", 0x000000, 0x100000, CRC(93d66106) SHA1(c5d665db04ae0e8992ef46544e2cb7b0e27c8bfe))
	ROM_LOAD16_BYTE("ex1-ma1l.3c", 0x200001, 0x100000, CRC(e4bba6ed) SHA1(6483ef91e5a5b8ddd13a3d889936c39829fa50d6))
	ROM_LOAD16_BYTE("ex1-ma1u.3f", 0x200000, 0x100000, CRC(04e7c4b0) SHA1(78180d96cd1fae583617d4d227ed4ee24f2f9e29))

	ROM_REGION(0x0800, "eeprom", 0) // default eeprom, otherwise game would lock up on 1st boot
	ROM_LOAD("eeprom", 0x0000, 0x0800, CRC(0f46389d) SHA1(5706a46b02a667f5bddaa3842bb009ea07d23603))
ROM_END

ROM_START(exvaniaj)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("ex1-ep0l.4c", 0x000001, 0x080000, CRC(18c12015) SHA1(e4f3524e798545c434549719b377c8b5863f580f)) /* 0xc00000 */
	ROM_LOAD16_BYTE("ex1-ep0u.4f", 0x000000, 0x080000, CRC(07d054d1) SHA1(e2d2cb81acd309c519686572804648bef4cbd191))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("ex1-ma0l.2c", 0x000001, 0x100000, CRC(17922cd4) SHA1(af92c2335e7110c0c5e712f3148c1534d22d3814)) /* 0x400000 */
	ROM_LOAD16_BYTE("ex1-ma0u.2f", 0x000000, 0x100000, CRC(93d66106) SHA1(c5d665db04ae0e8992ef46544e2cb7b0e27c8bfe))
	ROM_LOAD16_BYTE("ex1-ma1l.3c", 0x200001, 0x100000, CRC(e4bba6ed) SHA1(6483ef91e5a5b8ddd13a3d889936c39829fa50d6))
	ROM_LOAD16_BYTE("ex1-ma1u.3f", 0x200000, 0x100000, CRC(04e7c4b0) SHA1(78180d96cd1fae583617d4d227ed4ee24f2f9e29))

	ROM_REGION(0x0800, "eeprom", 0) // default eeprom, otherwise game would lock up on 1st boot
	ROM_LOAD("eeprom", 0x0000, 0x0800, CRC(0f46389d) SHA1(5706a46b02a667f5bddaa3842bb009ea07d23603))
ROM_END

ROM_START(fghtatck)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("fa2-ep0l.4c", 0x000001, 0x080000, CRC(8996db9c) SHA1(ebbe7d4cb2960a346cfbdf38c77638d71b6ba20e)) /* 0xc00000 */
	ROM_LOAD16_BYTE("fa2-ep0u.4f", 0x000000, 0x080000, CRC(58d5e090) SHA1(950219d4e9bf440f92e3c8765f47e23a9019d2d1))
	ROM_LOAD16_BYTE("fa1-ep1l.5c", 0x100001, 0x080000, CRC(b23a5b01) SHA1(4ba9bc2102fffc93a5ff73a107d557fc0f3beefd))
	ROM_LOAD16_BYTE("fa1-ep1u.5f", 0x100000, 0x080000, CRC(de2eb129) SHA1(912993cab1c2edcaf986478f2ae22a2f10edf807))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("fa1-ma0l.2c", 0x000001, 0x100000, CRC(a0a95e54) SHA1(da35f8a6a5bc9e2b5b6cacf8eb0d900ef1073a67)) /* 0x400000 */
	ROM_LOAD16_BYTE("fa1-ma0u.2f", 0x000000, 0x100000, CRC(1d0135bd) SHA1(2a7f8d09c213629a68376ce0379be61b37711d0a))
	ROM_LOAD16_BYTE("fa1-ma1l.3c", 0x200001, 0x100000, CRC(c4adf0a2) SHA1(4cc7adc68b1db7e725a973b31d52720bd7dc1140))
	ROM_LOAD16_BYTE("fa1-ma1u.3f", 0x200000, 0x100000, CRC(900297be) SHA1(57bb2078ff104c6f631c67219f80f8ede5ddbd09))
ROM_END

ROM_START(fa)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("fa1-ep0l.4c", 0x000001, 0x080000, CRC(182eee5c) SHA1(49769e3b72b59fc3e7b73364fe97168977dbe66b)) /* 0xc00000 */
	ROM_LOAD16_BYTE("fa1-ep0u.4f", 0x000000, 0x080000, CRC(7ea7830e) SHA1(79390943eea0b8029b2b8869233caf27228e776a))
	ROM_LOAD16_BYTE("fa1-ep1l.5c", 0x100001, 0x080000, CRC(b23a5b01) SHA1(4ba9bc2102fffc93a5ff73a107d557fc0f3beefd))
	ROM_LOAD16_BYTE("fa1-ep1u.5f", 0x100000, 0x080000, CRC(de2eb129) SHA1(912993cab1c2edcaf986478f2ae22a2f10edf807))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("fa1-ma0l.2c", 0x000001, 0x100000, CRC(a0a95e54) SHA1(da35f8a6a5bc9e2b5b6cacf8eb0d900ef1073a67)) /* 0x400000 */
	ROM_LOAD16_BYTE("fa1-ma0u.2f", 0x000000, 0x100000, CRC(1d0135bd) SHA1(2a7f8d09c213629a68376ce0379be61b37711d0a))
	ROM_LOAD16_BYTE("fa1-ma1l.3c", 0x200001, 0x100000, CRC(c4adf0a2) SHA1(4cc7adc68b1db7e725a973b31d52720bd7dc1140))
	ROM_LOAD16_BYTE("fa1-ma1u.3f", 0x200000, 0x100000, CRC(900297be) SHA1(57bb2078ff104c6f631c67219f80f8ede5ddbd09))
ROM_END

ROM_START(swcourt)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("sc2-ep0l.4c",  0x000001, 0x080000, CRC(5053a02e) SHA1(8ab5a085969cef5e01be01d8f531233002ea5bff)) /* 0xc00000 */
	ROM_LOAD16_BYTE("sc2-ep0u.4f",  0x000000, 0x080000, CRC(7b3fc7fa) SHA1(f96c03a03339b7677b8dc8689d907f2c8895886c))
	ROM_LOAD16_BYTE("sc1-ep1l.5c", 0x100001, 0x080000, CRC(fb45cf5f) SHA1(6ded351daa9b39d0b8149100caefc4fa0c598e79))
	ROM_LOAD16_BYTE("sc1-ep1u.5f", 0x100000, 0x080000, CRC(1ce07b15) SHA1(b1b28cc480301c9ad642597c7cdd8e9cdec996a6))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("sc1-ma0l.2c", 0x000001, 0x100000, CRC(3e531f5e) SHA1(6da56630bdfbb19f1639c539779c180d106f6ee2)) /* 0x400000 */
	ROM_LOAD16_BYTE("sc1-ma0u.2f", 0x000000, 0x100000, CRC(31e76a45) SHA1(5c278c167c1025c648ce2da2c3764645e96dcd55))
	ROM_LOAD16_BYTE("sc1-ma1l.3c", 0x200001, 0x100000, CRC(8ba3a4ec) SHA1(f881e7b4728f388d18450ba85e13e233071fbc88))
	ROM_LOAD16_BYTE("sc1-ma1u.3f", 0x200000, 0x100000, CRC(252dc4b7) SHA1(f1be6bd045495c7a0ecd97f01d1dc8ad341fecfd))
ROM_END

ROM_START(swcourtj)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("sc1-ep0l.4c",  0x000001, 0x080000, CRC(145111dd) SHA1(f8f74f77fb80af2ea37ea8ddbf02c1f3fcaf3fdb)) /* 0xc00000 */
	ROM_LOAD16_BYTE("sc1-ep0u.4f",  0x000000, 0x080000, CRC(c721c138) SHA1(5d30d66629d982b54c3bb62118be940dc7b69a6b))
	ROM_LOAD16_BYTE("sc1-ep1l.5c", 0x100001, 0x080000, CRC(fb45cf5f) SHA1(6ded351daa9b39d0b8149100caefc4fa0c598e79))
	ROM_LOAD16_BYTE("sc1-ep1u.5f", 0x100000, 0x080000, CRC(1ce07b15) SHA1(b1b28cc480301c9ad642597c7cdd8e9cdec996a6))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("sc1-ma0l.2c", 0x000001, 0x100000, CRC(3e531f5e) SHA1(6da56630bdfbb19f1639c539779c180d106f6ee2)) /* 0x400000 */
	ROM_LOAD16_BYTE("sc1-ma0u.2f", 0x000000, 0x100000, CRC(31e76a45) SHA1(5c278c167c1025c648ce2da2c3764645e96dcd55))
	ROM_LOAD16_BYTE("sc1-ma1l.3c", 0x200001, 0x100000, CRC(8ba3a4ec) SHA1(f881e7b4728f388d18450ba85e13e233071fbc88))
	ROM_LOAD16_BYTE("sc1-ma1u.3f", 0x200000, 0x100000, CRC(252dc4b7) SHA1(f1be6bd045495c7a0ecd97f01d1dc8ad341fecfd))
ROM_END

/*
This bootleg is running on the older type rom board (Cosmo Gang etc). Super World Court normally runs on the newer type 'B' board with extra chip at 6J.
It has a small pcb replacement keycus with a 74hc4060 , LS04 and 2 chips with the ID scratched (possibly PAL chips).
Program ROMs are almost identical. They hacked the keycus routine and the copyright year (from 1992 to 1994):
sc2-ep0l.4c  [2/2]      0l.0l        [2/2]      IDENTICAL
sc2-ep0u.4f  [2/2]      0u.0u        [2/2]      IDENTICAL
sc2-ep0u.4f  [1/2]      0u.0u        [1/2]      99.997711%
sc2-ep0l.4c  [1/2]      0l.0l        [1/2]      99.997330%

GFX ROMs are 27c040's double stacked with flying wires to the PAL board. They are the same as the 801 dumps, chopped in half. Pin 22 of OLH and OUH go to C pad on custom board.
Pin 22 of 1LH and 1UH go to B pad on custom board. All Lower pin '22's are tied high.

Believed to be a Playmark bootleg because the PCB has the typical slightly blue tinge to the solder mask and the same font.
*/

ROM_START(swcourtb)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("0l0l.4c", 0x000001, 0x080000, CRC(669c9b10) SHA1(8c40f5331f899c458699ab856c5900c540e8471e)) /* 0xc00000 */
	ROM_LOAD16_BYTE("0u0u.4f", 0x000000, 0x080000, CRC(742f3da1) SHA1(b3df6afd9849af8dd1643991ac70c93bf9f8fcb2))
	ROM_LOAD16_BYTE("1l1l.5c", 0x100001, 0x080000, CRC(fb45cf5f) SHA1(6ded351daa9b39d0b8149100caefc4fa0c598e79))
	ROM_LOAD16_BYTE("1u.1u.5f", 0x100000, 0x080000, CRC(1ce07b15) SHA1(b1b28cc480301c9ad642597c7cdd8e9cdec996a6))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("oll.ol.2c", 0x000001, 0x80000, CRC(df0920ef) SHA1(c8d583d8967b3eb86ecfbabb906cc82d2a05d139)) /* 0x400000 */
	ROM_LOAD16_BYTE("oul.ou.2f", 0x000000, 0x80000, CRC(844c6a1c) SHA1(ad186c8209688e1bc567fb5015d9d970099139bb))
	ROM_LOAD16_BYTE("olh.ol.2c", 0x100001, 0x80000, CRC(0a21abea) SHA1(cf8f8ff37abdc398cbabf0f0a77aa15ccbc37257))
	ROM_LOAD16_BYTE("ouh.ou.2f", 0x100000, 0x80000, CRC(6b7c93f9) SHA1(2f823a2a2d8ca5cdb679dd1c1ca66803d47c6b40))
	ROM_LOAD16_BYTE("1ll.1l.3c", 0x200001, 0x80000, CRC(f7e30277) SHA1(65db7e07919c36011fa930976d43dd2d4fb7b8e5))
	ROM_LOAD16_BYTE("1ul.1u.3f", 0x200000, 0x80000, CRC(5f03c51a) SHA1(75cd042db716b6cb56f812af9ba6dca0efae8cac))
	ROM_LOAD16_BYTE("1lh.1l.3c", 0x300001, 0x80000, CRC(6531236e) SHA1(9270a2235b6a713d8c4e791da789d8428b461a1a))
	ROM_LOAD16_BYTE("1uh.1u.3f", 0x300000, 0x80000, CRC(acae6746) SHA1(fb06b544e187c71b27318c249f1e52ff86aab00c))

	//PALs? See comments above
ROM_END

ROM_START(tinklpit)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("tk1-ep0l.6c", 0x000001, 0x080000, CRC(fdccae42) SHA1(398384482ccb3eb08bfb9db495513272a5188d92)) /* 0xc00000 */
	ROM_LOAD16_BYTE("tk1-ep0u.6f", 0x000000, 0x080000, CRC(62cdb48c) SHA1(73c7b99b117b8dc567bc254b0ffcc117c9d42fb5))
	ROM_LOAD16_BYTE("tk1-ep1l.7c", 0x100001, 0x080000, CRC(7e90f104) SHA1(79e371426b2e32dc8f687e4d124d23c251198937))
	ROM_LOAD16_BYTE("tk1-ep1u.7f", 0x100000, 0x080000, CRC(9c0b70d6) SHA1(eac44d3470f4c2ddd9c41f82e6398bca0cc8a4fd))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("tk1-ma0l.2c", 0x000001, 0x100000, CRC(c6b4e15d) SHA1(55252ba4d904b14940436f1b4dc5e2a6bd163bdf)) /* 0x400000 */
	ROM_LOAD16_BYTE("tk1-ma0u.2f", 0x000000, 0x100000, CRC(a3ad6f67) SHA1(54289eed5347defb5464ec5a610a6748909159f6))
	ROM_LOAD16_BYTE("tk1-ma1l.3c", 0x200001, 0x100000, CRC(61cfb92a) SHA1(eacf0e7557f33d552045f43a116ff08c533a2771))
	ROM_LOAD16_BYTE("tk1-ma1u.3f", 0x200000, 0x100000, CRC(54b77816) SHA1(9341d07858623e1920eaae7b2b90126c7057297e))
	ROM_LOAD16_BYTE("tk1-ma2l.4c", 0x400001, 0x100000, CRC(087311d2) SHA1(6fe50f9e08551e57d15a15b01e3822a6cb7c8352))
	ROM_LOAD16_BYTE("tk1-ma2u.4f", 0x400000, 0x100000, CRC(5ce20c2c) SHA1(7eaff21714bae44f8b21b6db98f055e04bfbae18))
ROM_END


/**************************************
     NA-2 Based games
**************************************/


// runs on several Namco NA-2 PCBs. Cabinet pic shows 9 screens for main display + 5 screens (one per terminal)
ROM_START(bbbingo) // uses NA E4M8 ROM PCB with unpopulated keycus
	ROM_REGION(0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("bi1-slp1.6c", 0x000001, 0x080000, CRC(07a9a78d) SHA1(73f8933a26adc7725f046ac99c536e151f5c3fa2))
	ROM_LOAD16_BYTE("bi1-slp0.6f", 0x000000, 0x080000, CRC(9642b4fa) SHA1(900687e1596e4f18a09b80b3a38a4597c4da16da))

	ROM_REGION16_BE(0x800000, "maskrom", ROMREGION_ERASE00)
	ROM_LOAD16_BYTE("bi1-slp5.2c", 0x000001, 0x080000, CRC(fb3dbcf2) SHA1(f94d339491215cf05dfe616f963560dc47aed472))
	ROM_LOAD16_BYTE("bi1-slp4.2f", 0x000000, 0x080000, CRC(82d34c59) SHA1(638af6cfec3b09a4b35139bba51bcd21711efa2b))
	// 0x100000 - 0x1fffff empty
	ROM_LOAD16_BYTE("bi1-slp3.3c", 0x200001, 0x080000, CRC(4c1f16c4) SHA1(360eb5aaee83b7e6b6d0902aaab75b045cf72d3f))
	ROM_LOAD16_BYTE("bi1-slp2.3f", 0x200000, 0x080000, CRC(a09cef53) SHA1(96c666a3a4f1747afe32bf791537b211bc8ac6d8))
ROM_END

ROM_START(bbbingot) // this uses a different ROM board: Namco MDROM PCB - 8625961102 with MB8464A-15LL battery-backed RAM
	ROM_REGION(0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("bi1 msp1.3e", 0x000001, 0x080000, CRC(ea5bf674) SHA1(079a74bbc70386a1dc7df6a146d23bf64a941b7e))
	ROM_LOAD16_BYTE("bi1 msp0.6e", 0x000000, 0x080000, CRC(3e5c275d) SHA1(2c20a07b5fc36de6a17e171c7db9e12348eaf1d1))

	ROM_REGION16_BE(0x800000, "maskrom", ROMREGION_ERASE00) // these are identical to the main unit ones
	ROM_LOAD16_BYTE("bi1 msp5.3b", 0x000001, 0x080000, CRC(fb3dbcf2) SHA1(f94d339491215cf05dfe616f963560dc47aed472))
	ROM_LOAD16_BYTE("bi1 msp4.6b", 0x000000, 0x080000, CRC(82d34c59) SHA1(638af6cfec3b09a4b35139bba51bcd21711efa2b))
	// 0x100000 - 0x1fffff empty
	ROM_LOAD16_BYTE("bi1 msp3.3c", 0x200001, 0x080000, CRC(4c1f16c4) SHA1(360eb5aaee83b7e6b6d0902aaab75b045cf72d3f))
	ROM_LOAD16_BYTE("bi1 msp2.6c", 0x200000, 0x080000, CRC(a09cef53) SHA1(96c666a3a4f1747afe32bf791537b211bc8ac6d8))

	ROM_REGION(0x500, "plds", 0)
	ROM_LOAD("na1r1m.10a", 0x000, 0x2dd, CRC(585f0f83) SHA1(1042ea156a039a5d20ef6f349e3dbd68043f04cb)) // PALCE22V10H, bruteforced
	ROM_LOAD("na1r2m.8a",  0x300, 0x117, CRC(273bccd1) SHA1(63bacb7b8792a72fa9b912a2e2207ae74d38ecd3)) // PAL16L8BCN, bruteforced
ROM_END

ROM_START(emeralda) /* NA-2 Game PCB, clones are NA-1 based; see games listed above */
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("em2-ep0l.6c", 0x000001, 0x080000, CRC(ff1479dc) SHA1(ea945d97ed909be13fb6e062742c7142c0d96c31)) /* 0xc00000 */
	ROM_LOAD16_BYTE("em2-ep0u.6f", 0x000000, 0x080000, CRC(ffe750a2) SHA1(d10d31489ae364572d7517dd515a6af2182ac764))
	ROM_LOAD16_BYTE("em2-ep1l.7c", 0x100001, 0x080000, CRC(6c3e5b53) SHA1(72b941e28c7fda8cb81240a8226386fe55c14e2d))
	ROM_LOAD16_BYTE("em2-ep1u.7f", 0x100000, 0x080000, CRC(dee15a81) SHA1(474a264029bd77e4205773a7461dea695e65933f))

	ROM_REGION16_BE(0x800000, "maskrom", ROMREGION_ERASE00)
	/* no mask roms */
ROM_END

ROM_START(knckhead)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("kh2-ep0l.6c", 0x000001, 0x080000, CRC(b4b88077) SHA1(9af03d1832ad6c77222e18427f4afca330a41ce6)) /* 0xc00000 */
	ROM_LOAD16_BYTE("kh2-ep0u.6f", 0x000000, 0x080000, CRC(a578d97e) SHA1(9a5bb6649cca7b98daf538a66c813f61cca2e2ec))
	ROM_LOAD16_BYTE("kh1-ep1l.7c", 0x100001, 0x080000, CRC(27e6ab4e) SHA1(66f397cc2117c1e73652c4800c0937e6d8116380))
	ROM_LOAD16_BYTE("kh1-ep1u.7f", 0x100000, 0x080000, CRC(487b2434) SHA1(2d62db85ceac1fca61c39e4db92c96ae80ba3323))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("kh1-ma0l.2c", 0x000001, 0x100000, CRC(7b2db5df) SHA1(ecc392c4683cf0718d986e73336b69952d324548)) /* 0x400000 */
	ROM_LOAD16_BYTE("kh1-ma0u.2f", 0x000000, 0x100000, CRC(6983228b) SHA1(5f3eeb780e9d91445b4c11da63d4ca580e654f34))
	ROM_LOAD16_BYTE("kh1-ma1l.3c", 0x200001, 0x100000, CRC(b24f93e6) SHA1(3d8951485dc8a2810da9ddf2f4896efa31779bf4))
	ROM_LOAD16_BYTE("kh1-ma1u.3f", 0x200000, 0x100000, CRC(18a60348) SHA1(298e0e0e7649e872791c3c99c81a19f273e9eb8a))
	ROM_LOAD16_BYTE("kh1-ma2l.4c", 0x400001, 0x100000, CRC(82064ee9) SHA1(0b984565d17e580f49fff982a1621ef90e14c064))
	ROM_LOAD16_BYTE("kh1-ma2u.4f", 0x400000, 0x100000, CRC(17fe8c3d) SHA1(88c45076477725faa5f8a23512e65a40385bb27d))
	ROM_LOAD16_BYTE("kh1-ma3l.5c", 0x600001, 0x100000, CRC(ad9a7807) SHA1(c40f18a68306e76acd89ccb3fc82b8106556912e))
	ROM_LOAD16_BYTE("kh1-ma3u.5f", 0x600000, 0x100000, CRC(efeb768d) SHA1(15d016244549f3ea0d19f5cfb04bcebd65ac6134))
ROM_END

ROM_START(knckheadj)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("kh1-ep0l.6c", 0x000001, 0x080000, CRC(94660bec) SHA1(42fa23f759cf66b05f30c2fc03a12fd14ae1f796)) /* 0xc00000 */
	ROM_LOAD16_BYTE("kh1-ep0u.6f", 0x000000, 0x080000, CRC(ad640d69) SHA1(62595a9d1d5952cbe3dd7266cfda9292be51d269))
	ROM_LOAD16_BYTE("kh1-ep1l.7c", 0x100001, 0x080000, CRC(27e6ab4e) SHA1(66f397cc2117c1e73652c4800c0937e6d8116380))
	ROM_LOAD16_BYTE("kh1-ep1u.7f", 0x100000, 0x080000, CRC(487b2434) SHA1(2d62db85ceac1fca61c39e4db92c96ae80ba3323))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("kh1-ma0l.2c", 0x000001, 0x100000, CRC(7b2db5df) SHA1(ecc392c4683cf0718d986e73336b69952d324548)) /* 0x400000 */
	ROM_LOAD16_BYTE("kh1-ma0u.2f", 0x000000, 0x100000, CRC(6983228b) SHA1(5f3eeb780e9d91445b4c11da63d4ca580e654f34))
	ROM_LOAD16_BYTE("kh1-ma1l.3c", 0x200001, 0x100000, CRC(b24f93e6) SHA1(3d8951485dc8a2810da9ddf2f4896efa31779bf4))
	ROM_LOAD16_BYTE("kh1-ma1u.3f", 0x200000, 0x100000, CRC(18a60348) SHA1(298e0e0e7649e872791c3c99c81a19f273e9eb8a))
	ROM_LOAD16_BYTE("kh1-ma2l.4c", 0x400001, 0x100000, CRC(82064ee9) SHA1(0b984565d17e580f49fff982a1621ef90e14c064))
	ROM_LOAD16_BYTE("kh1-ma2u.4f", 0x400000, 0x100000, CRC(17fe8c3d) SHA1(88c45076477725faa5f8a23512e65a40385bb27d))
	ROM_LOAD16_BYTE("kh1-ma3l.5c", 0x600001, 0x100000, CRC(ad9a7807) SHA1(c40f18a68306e76acd89ccb3fc82b8106556912e))
	ROM_LOAD16_BYTE("kh1-ma3u.5f", 0x600000, 0x100000, CRC(efeb768d) SHA1(15d016244549f3ea0d19f5cfb04bcebd65ac6134))
ROM_END

ROM_START(knckheadjp) /* Older or possible prototype. Doesn't show rom test at boot up */
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("2-10_9o.6c", 0x000001, 0x080000, CRC(600faf17) SHA1(21197ad1d54a68c1510d9ae6999ca41efaaed05d)) /* handwritten label 2/10 9O */ /* 0xc00000 */
	ROM_LOAD16_BYTE("2-10_9e.6f", 0x000000, 0x080000, CRC(16ccc0b0) SHA1(e9b98eae7ee47c7cce2cc3de9dc39428e0648a40)) /* handwritten label 2/10 9E */
	ROM_LOAD16_BYTE("2-10_8o.7c", 0x100001, 0x080000, CRC(27e6ab4e) SHA1(66f397cc2117c1e73652c4800c0937e6d8116380)) /* handwritten label 2/10 8O */
	ROM_LOAD16_BYTE("2-10_8e.7f", 0x100000, 0x080000, CRC(487b2434) SHA1(2d62db85ceac1fca61c39e4db92c96ae80ba3323)) /* handwritten label 2/10 8E */

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("kh1-ma0l.2c", 0x000001, 0x100000, CRC(7b2db5df) SHA1(ecc392c4683cf0718d986e73336b69952d324548)) /* 0x400000 */
	ROM_LOAD16_BYTE("kh1-ma0u.2f", 0x000000, 0x100000, CRC(6983228b) SHA1(5f3eeb780e9d91445b4c11da63d4ca580e654f34))
	ROM_LOAD16_BYTE("kh1-ma1l.3c", 0x200001, 0x100000, CRC(b24f93e6) SHA1(3d8951485dc8a2810da9ddf2f4896efa31779bf4))
	ROM_LOAD16_BYTE("kh1-ma1u.3f", 0x200000, 0x100000, CRC(18a60348) SHA1(298e0e0e7649e872791c3c99c81a19f273e9eb8a))
	ROM_LOAD16_BYTE("kh1-ma2l.4c", 0x400001, 0x100000, CRC(82064ee9) SHA1(0b984565d17e580f49fff982a1621ef90e14c064))
	ROM_LOAD16_BYTE("kh1-ma2u.4f", 0x400000, 0x100000, CRC(17fe8c3d) SHA1(88c45076477725faa5f8a23512e65a40385bb27d))
	ROM_LOAD16_BYTE("kh1-ma3l.5c", 0x600001, 0x100000, CRC(ad9a7807) SHA1(c40f18a68306e76acd89ccb3fc82b8106556912e))
	ROM_LOAD16_BYTE("kh1-ma3u.5f", 0x600000, 0x100000, CRC(efeb768d) SHA1(15d016244549f3ea0d19f5cfb04bcebd65ac6134))

	ROM_REGION(0x0800, "eeprom", 0) // default eeprom, otherwise game would lock up on 1st boot
	ROM_LOAD("eeprom", 0x0000, 0x0800, CRC(98875a23) SHA1(2256cd231587351a0768faaedafbd1f80e3fd7c4))
ROM_END

ROM_START(numanath)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("nm2-ep0l.6c", 0x000001, 0x080000, CRC(f24414bb) SHA1(68b13dfdc2292afd5279edb891fe63972f991e7b)) /* 0xc00000 */
	ROM_LOAD16_BYTE("nm2-ep0u.6f", 0x000000, 0x080000, CRC(25c41616) SHA1(68ba67d3dd45f3bdddfa2fd21b574535306c1214))
	ROM_LOAD16_BYTE("nm1-ep1l.7c", 0x100001, 0x080000, CRC(4581dcb4) SHA1(1f46f98e63a7c9cdfde9e8ee2696a13c3f9bcc8e))
	ROM_LOAD16_BYTE("nm1-ep1u.7f", 0x100000, 0x080000, CRC(30cd589a) SHA1(74a14ec41fe4fc9f73e5357b0903f1199ed96337))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("nm1-ma0l.2c", 0x000001, 0x100000, CRC(20faaa57) SHA1(9dbfc0dd48eec37b2c0715a5691c6e6f923fc7f7)) /* 0x400000 */
	ROM_LOAD16_BYTE("nm1-ma0u.2f", 0x000000, 0x100000, CRC(ed7c37f2) SHA1(829751af33754ade941f76982e196b494d56ab0a))
	ROM_LOAD16_BYTE("nm1-ma1l.3c", 0x200001, 0x100000, CRC(2232e3b4) SHA1(e9da3dc34eb2576c8a88e23cb9007129e885496d))
	ROM_LOAD16_BYTE("nm1-ma1u.3f", 0x200000, 0x100000, CRC(6cc9675c) SHA1(fec74da4479f2a088760efc6908e6acfaea3989f))
	ROM_LOAD16_BYTE("nm1-ma2l.4c", 0x400001, 0x100000, CRC(208abb39) SHA1(52d7247a71c6a14467f12f5270921bba1824cc3f))
	ROM_LOAD16_BYTE("nm1-ma2u.4f", 0x400000, 0x100000, CRC(03a3f204) SHA1(9cb0422c8ecc819d0cc8a65c29a228369d78d986))
	ROM_LOAD16_BYTE("nm1-ma3l.5c", 0x600001, 0x100000, CRC(42a539e9) SHA1(1c53a5a031648891ab7a37cf026c979404ce9589))
	ROM_LOAD16_BYTE("nm1-ma3u.5f", 0x600000, 0x100000, CRC(f79e2112) SHA1(8bb8639a9d3a5d3ac5c9bb78e72b3d76582a9c25))
ROM_END

ROM_START(numanathj)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("nm1-ep0l.6c", 0x000001, 0x080000, CRC(4398b898) SHA1(0d1517409ba181f796f7f413cac704c60085b505)) /* 0xc00000 */
	ROM_LOAD16_BYTE("nm1-ep0u.6f", 0x000000, 0x080000, CRC(be90aa79) SHA1(6884a8d72dd34c889527e8e653f5e5b4cf3fb5d6))
	ROM_LOAD16_BYTE("nm1-ep1l.7c", 0x100001, 0x080000, CRC(4581dcb4) SHA1(1f46f98e63a7c9cdfde9e8ee2696a13c3f9bcc8e))
	ROM_LOAD16_BYTE("nm1-ep1u.7f", 0x100000, 0x080000, CRC(30cd589a) SHA1(74a14ec41fe4fc9f73e5357b0903f1199ed96337))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("nm1-ma0l.2c", 0x000001, 0x100000, CRC(20faaa57) SHA1(9dbfc0dd48eec37b2c0715a5691c6e6f923fc7f7)) /* 0x400000 */
	ROM_LOAD16_BYTE("nm1-ma0u.2f", 0x000000, 0x100000, CRC(ed7c37f2) SHA1(829751af33754ade941f76982e196b494d56ab0a))
	ROM_LOAD16_BYTE("nm1-ma1l.3c", 0x200001, 0x100000, CRC(2232e3b4) SHA1(e9da3dc34eb2576c8a88e23cb9007129e885496d))
	ROM_LOAD16_BYTE("nm1-ma1u.3f", 0x200000, 0x100000, CRC(6cc9675c) SHA1(fec74da4479f2a088760efc6908e6acfaea3989f))
	ROM_LOAD16_BYTE("nm1-ma2l.4c", 0x400001, 0x100000, CRC(208abb39) SHA1(52d7247a71c6a14467f12f5270921bba1824cc3f))
	ROM_LOAD16_BYTE("nm1-ma2u.4f", 0x400000, 0x100000, CRC(03a3f204) SHA1(9cb0422c8ecc819d0cc8a65c29a228369d78d986))
	ROM_LOAD16_BYTE("nm1-ma3l.5c", 0x600001, 0x100000, CRC(42a539e9) SHA1(1c53a5a031648891ab7a37cf026c979404ce9589))
	ROM_LOAD16_BYTE("nm1-ma3u.5f", 0x600000, 0x100000, CRC(f79e2112) SHA1(8bb8639a9d3a5d3ac5c9bb78e72b3d76582a9c25))
ROM_END

ROM_START(quiztou)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("qt1ep0l.6c", 0x000001, 0x080000, CRC(b680e543) SHA1(f10f38113a46c821d8e9d66f52d7311d9d52e595)) /* 0xc00000 */
	ROM_LOAD16_BYTE("qt1ep0u.6f", 0x000000, 0x080000, CRC(143c5e4d) SHA1(24c584986c97a5e6fe7e73f0e9af4af28ed20c4a))
	ROM_LOAD16_BYTE("qt1ep1l.7c", 0x100001, 0x080000, CRC(33a72242) SHA1(5d17f033878d28dbebba50931a549ccf84802c05))
	ROM_LOAD16_BYTE("qt1ep1u.7f", 0x100000, 0x080000, CRC(69f876cb) SHA1(d0c7e972a04c45d3ab34ef5be88614d6389189c6))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("qt1ma0l.2c", 0x000001, 0x100000, CRC(5597f2b9) SHA1(747c4be867d4eb37ffab8303740729686a00b825)) /* 0x400000 */
	ROM_LOAD16_BYTE("qt1ma0u.2f", 0x000000, 0x100000, CRC(f0a4cb7d) SHA1(364e85af956e7cfc29c957da11574a4b389f7797))
	ROM_LOAD16_BYTE("qt1ma1l.3c", 0x200001, 0x100000, CRC(1b9ce7a6) SHA1(dac1da9dd8076f238211fed5c780b4b8bededf22))
	ROM_LOAD16_BYTE("qt1ma1u.3f", 0x200000, 0x100000, CRC(58910872) SHA1(c0acbd64e90672564c3839fd21870672aa32e439))
	ROM_LOAD16_BYTE("qt1ma2l.4c", 0x400001, 0x100000, CRC(94739917) SHA1(b5be5c9fd7223d3fb601f769cb80f56a5a586de0))
	ROM_LOAD16_BYTE("qt1ma2u.4f", 0x400000, 0x100000, CRC(6ba5b893) SHA1(071caed9cf261f1f8af7079875bd206177baef1a))
	ROM_LOAD16_BYTE("qt1ma3l.5c", 0x600001, 0x100000, CRC(aa9dc6ff) SHA1(c738f8c59bb5245874576c5bcf88c7138fa9a147))
	ROM_LOAD16_BYTE("qt1ma3u.5f", 0x600000, 0x100000, CRC(14a5a163) SHA1(1107f50e491bedeb4ab7ac3f32cfe47727274ba9))

	ROM_REGION(0x0800, "eeprom", 0) // default eeprom, otherwise game would lock up on 1st boot
	ROM_LOAD("eeprom", 0x0000, 0x0800, CRC(57a478a6) SHA1(b6d66610690f2fdf6643b2de91e2345d15d839b1))
ROM_END

ROM_START(xday2)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_BYTE("xds1-mpr0.4b", 0x000001, 0x080000, CRC(83539aaa) SHA1(42d97bb2daaf5ff48efac70f0ff37869c5ba177d)) /* 0xc00000 */
	ROM_LOAD16_BYTE("xds1-mpr1.8b", 0x000000, 0x080000, CRC(468b36de) SHA1(52817be9913a6938ce6add2834ba1a727b1d677e))

	ROM_REGION16_BE(0x800000, "maskrom", 0)
	ROM_LOAD16_BYTE("xds1-dat0.4b", 0x000001, 0x200000, CRC(42cecc8b) SHA1(7510f16b908dd0f7828887dcfa26c5e4643df66c)) /* 0x400000 */
	ROM_LOAD16_BYTE("xds1-dat1.8b", 0x000000, 0x200000, CRC(d250b7e8) SHA1(b99251ae8e25fae062d33e74ff800ab43fb308a2))
	ROM_LOAD16_BYTE("xds1-dat2.4c", 0x400001, 0x200000, CRC(99d72a08) SHA1(4615b43b9a81240ffee8b0f021037f554f4f1f24))
	ROM_LOAD16_BYTE("xds1-dat3.8c", 0x400000, 0x200000, CRC(8980acc4) SHA1(ecd94a3d3a38923e8e322cd8863671af26e30812))
ROM_END

// an 8-liner game that runs on several Namco NA-2 PCBs. Cabinet pic shows 9 screens (one per reel, called windows in IO test) + 5 screens (one per terminal)
ROM_START(zelos)
	ROM_REGION(0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("zs1 slp 1b.6c", 0x000001, 0x080000, CRC(d71df137) SHA1(457d3e7cb352b44706567e8346dcde82393d13c1))
	ROM_LOAD16_BYTE("zs1 slp 0b.6f", 0x000000, 0x080000, CRC(5807ef9e) SHA1(5dde8d71637de480d1d679d88b25509c229c6056))

	ROM_REGION16_BE(0x800000, "maskrom", ROMREGION_ERASE00)
	// not populated
ROM_END

ROM_START(zelost) // this uses a different ROM board: Namco MDROM PCB - 8625961102 with MB8464A-15LL battery-backed RAM
	ROM_REGION(0x200000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("zs1 stp 1e.3e", 0x000001, 0x080000, CRC(3a593d12) SHA1(1941c5e88e425f83fc8e22e48e7bf18f231efb78))
	ROM_LOAD16_BYTE("zs1 stp 0e.6e", 0x000000, 0x080000, CRC(e331f84c) SHA1(77812e50c49093883a9ec71290f45d398abac5fd))

	ROM_REGION16_BE(0x800000, "maskrom", ROMREGION_ERASE00)
	// not populated

	ROM_REGION(0x500, "plds", 0)
	ROM_LOAD("na1r1m.10a", 0x000, 0x2dd, CRC(585f0f83) SHA1(1042ea156a039a5d20ef6f349e3dbd68043f04cb)) // PALCE22V10H, bruteforced
	ROM_LOAD("na1r2m.8a",  0x300, 0x117, CRC(273bccd1) SHA1(63bacb7b8792a72fa9b912a2e2207ae74d38ecd3)) // PAL16L8BCN, bruteforced

	ROM_REGION(0x0800, "eeprom", 0) // default EEPROM, to avoid error on start-up
	ROM_LOAD("eeprom", 0x0000, 0x0800, CRC(ac117acc) SHA1(fa7d8d1f47cc0cbcc37d1fa4d41d76a109320b0b))
ROM_END

// NA-1 (C69 MCU)
GAME(1992, bkrtmaq,    0,        namcona1, namcona1_quiz,  namcona1_state, init_bkrtmaq,  ROT0, "Namco", "Bakuretsu Quiz Ma-Q Dai Bouken (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1992, cgangpzl,   0,        namcona1,  namcona1_joy,  namcona1_state, init_cgangpzl, ROT0, "Namco", "Cosmo Gang the Puzzle (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1992, cgangpzlj,  cgangpzl, namcona1,  namcona1_joy,  namcona1_state, init_cgangpzl, ROT0, "Namco", "Cosmo Gang the Puzzle (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1992, exvania,    0,        namcona1,  namcona1_joy,  namcona1_state, init_exvania,  ROT0, "Namco", "Exvania (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1992, exvaniaj,   exvania,  namcona1,  namcona1_joy,  namcona1_state, init_exvania,  ROT0, "Namco", "Exvania (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1992, fghtatck,   0,        namcona1,  namcona1_joy,  namcona1_state, init_fa,       ROT90,"Namco", "Fighter & Attacker (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1992, fa,         fghtatck, namcona1,  namcona1_joy,  namcona1_state, init_fa,       ROT90,"Namco", "F/A (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1992, swcourt,    0,        namcona1,  namcona1_joy,  namcona1_state, init_swcourt,  ROT0, "Namco", "Super World Court (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1992, swcourtj,   swcourt,  namcona1,  namcona1_joy,  namcona1_state, init_swcourt,  ROT0, "Namco", "Super World Court (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1994, swcourtb,   swcourt,  namcona1,  namcona1_joy,  namcona1_state, init_swcourtb, ROT0, "bootleg (Playmark?)", "Super World Court (World, bootleg)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1993, emeraldaj,  emeralda, namcona1,  namcona1_joy,  namcona1_state, init_emeraldj, ROT0, "Namco", "Emeraldia (Japan Version B)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL) /* Parent is below on NA-2 Hardware */
GAME(1993, emeraldaja, emeralda, namcona1,  namcona1_joy,  namcona1_state, init_emeraldj, ROT0, "Namco", "Emeraldia (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL) /* Parent is below on NA-2 Hardware */
GAME(1993, tinklpit,   0,        namcona1,  namcona1_joy,  namcona1_state, init_tinklpit, ROT0, "Namco", "Tinkle Pit (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)

// NA-2 (C70 MCU)
GAME(1992, knckhead,   0,        namcona2,  namcona1_joy,  namcona2_state, init_knckhead, ROT0, "Namco", "Knuckle Heads (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1992, knckheadj,  knckhead, namcona2,  namcona1_joy,  namcona2_state, init_knckhead, ROT0, "Namco", "Knuckle Heads (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1992, knckheadjp, knckhead, namcona2,  namcona1_joy,  namcona2_state, init_knckhead, ROT0, "Namco", "Knuckle Heads (Japan, Prototype?)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1993, emeralda,   0,        namcona2,  namcona1_joy,  namcona2_state, init_emeralda, ROT0, "Namco", "Emeraldia (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1993, numanath,   0,        namcona2,  namcona1_joy,  namcona2_state, init_numanath, ROT0, "Namco", "Numan Athletics (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1993, numanathj,  numanath, namcona2,  namcona1_joy,  namcona2_state, init_numanath, ROT0, "Namco", "Numan Athletics (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1993, quiztou,    0,        namcona2,  namcona1_quiz, namcona2_state, init_quiztou,  ROT0, "Namco", "Nettou! Gekitou! Quiztou!! (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1995, xday2,      0,        xday2,     namcona1_joy,  xday2_namcona2_state, init_xday2, ROT0, "Namco", "X-Day 2 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME(1994, zelos,      0,        zelos,     namcona1_joy,  namcona2_state, init_zelos,    ROT0, "Namco", "Zelos (Japan, main unit)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL) // waits for communication with the terminals
GAME(1994, zelost,     0,        zelos,     zelost,        namcona2_state, init_zelos,    ROT0, "Namco", "Zelos (Japan, terminal)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL) // no way to insert medal. Maybe needs communication with main unit?
GAME(1996, bbbingo,    0,        zelos,     namcona1_joy,  namcona2_state, init_zelos,    ROT0, "Namco", "Bin Bin Bingo (Japan, main unit)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL) // waits for communication with the terminals
GAME(1996, bbbingot,   0,        zelos,     namcona1_joy,  namcona2_state, init_zelos,    ROT0, "Namco", "Bin Bin Bingo (Japan, terminal)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL) // waits for communication with the main unit
