// license:BSD-3-Clause
// copyright-holders:fenugrec
/******************************************************************************
* HP 3478A Digital Multimeter
*
* Emulating test equipment is not very meaningful except for developping ROM patches.
* This aims to be the minimal emulation sufficient to run the UI (keypad and display).
* Ideally, faking ADC readings could be useful too.
*
* Some of this will probably be applicable to HP 3468A units too.
*
* Current status : runs (with banking !), fails CAL checksum due to unimplemented
*
*
* TODO kindof important
* * keypad on port1
* * something something display
*
* TODO next level
* * DIP switches
* * proper CAL RAM (see NVRAM() macro ?)
* * ability to preload CAL RAM on startup ("media" or "software" ?)
*
* TODO level 9000
* * Connect this with the existing i8291.cpp driver
* * dump + add analog CPU (8049)


**** Hardware details (refer to service manual for schematics)
Main CPU : i8039 , no internal ROM
Analog (floating) CPU : i8049, internal ROM (never dumped ?)
ROM : 2764 (64kbit, org 8kB) . Stores calibration data
RAM : 5101 , 256 * 4bit
GPIB:  i8291
Display : unknown; similar protocol for HP 3457A documented on
	http://www.eevblog.com/forum/projects/led-display-for-hp-3457a-multimeter-i-did-it-)/25/



Main cpu I/O ports:
Port1
P14-P17 : keypad out (cols)
P10-P13 : keypad in (rows)

P20 : disp.clk1
P21 : !CS for GPIB, and disp.IWA
P22 : !CS for DIPswitch; disp.ISA (for instructions)
P23 = !OE for RAM ; disp.sync (enable instruction)
P24 = disp.PWO	(enable)
P25 = disp.clk2
P26 : address bit12 ! (0x1000) => hardware banking
P27 : data out thru isol, to analog CPU

T1 : data in thru isol, from analog CPU (opcodes jt1 / jnt1)
*/

#include "emu.h"
#include "includes/hp3478a.h"

#include "cpu/mcs48/mcs48.h"



#define CPU_CLOCK       XTAL(5'856'000)

#define A12_PIN	P26
#define CALRAM_CS P23
#define DIPSWITCH_CS P22
#define GPIB_CS P21

#define DISP_PWO P24
#define DISP_SYNC P23
#define DISP_ISA P22
#define DISP_IWA P21
#define DISP_CK1 P20
	//don't care about CK2 since it's supposed to be a delayed copy of CK1
#define DISP_MASK (DISP_PWO | DISP_SYNC | DISP_ISA | DISP_IWA | DISP_CK1)	//used for edge detection


/**** optional debug outputs, must be before #include */
#define DEBUG_PORTS (LOG_GENERAL << 1)
#define DEBUG_BANKING (LOG_GENERAL << 2)
#define DEBUG_BUS (LOG_GENERAL << 3)
#define DEBUG_KEYPAD (LOG_GENERAL << 4)
#define DEBUG_LCD (LOG_GENERAL << 5)	//low level
#define DEBUG_LCD2 (LOG_GENERAL << 6)

#define VERBOSE (DEBUG_LCD2)

#include "logmacro.h"



/***** callbacks */

WRITE8_MEMBER( hp3478a_state::p1write )
{
	LOGMASKED(DEBUG_PORTS, "port1 write: %02X\n", data);
}

READ8_MEMBER( hp3478a_state::p1read )
{
	uint8_t data = m_maincpu->p1_r();
	LOGMASKED(DEBUG_PORTS, "port1 read: 0x%02X\n", data);
	return data;
}

WRITE8_MEMBER( hp3478a_state::p2write )
{
	LOGMASKED(DEBUG_PORTS, "port2 write: %02X\n", data);

	if ((p2_oldstate ^ data) & A12_PIN) {
		/* A12 pin state changed */
		if (data & A12_PIN) {
			m_bank0->set_entry(1);
			LOGMASKED(DEBUG_BANKING, "changed to bank1\n");
		} else {
			m_bank0->set_entry(0);
			LOGMASKED(DEBUG_BANKING, "changed to bank0\n");
		}
	}

	if ((p2_oldstate ^ data) & DISP_MASK) {
		/* display signals changed */
		lcd_interface(data);
	}

	p2_oldstate = data;
}

/** external bus read callback
 * runs when main cpu accesses an external IC, e.g. GPIB or CAL RAM.
 */
READ8_MEMBER( hp3478a_state::busread )
{
	uint8_t p2_state;
	uint8_t data = 0;
	unsigned found = 0;

	p2_state = m_maincpu->p2_r();
	// check which CS line is active.

	if (!(p2_state & CALRAM_CS)) {
		//XXX read from calram
		found += 1;
		LOGMASKED(DEBUG_BUS, "read 0x%02X from CAL RAM[0x%02X]\n", data, offset);
	}
	if (!(p2_state & DIPSWITCH_CS)) {
		//XXX parse inputs
		found += 1;
		LOGMASKED(DEBUG_BUS, "read DIP state : 0x%02X\n", data);
	}
	if (!(p2_state & GPIB_CS)) {
		found += 1 ;
		LOGMASKED(DEBUG_BUS, "read GPIB register %X\n", offset & 0x07);
	}

	if (!found) {
		logerror("Bus read with no CS active !\n");
		return 0xFF;	//pulled up
	}

	if (found > 1) {
		logerror("Bus read with more than one CS active !\n");
	}
	return data;
}



WRITE8_MEMBER( hp3478a_state::buswrite )
{
	uint8_t p2_state;
	unsigned found = 0;

	p2_state = m_maincpu->p2_r();
	// check which CS line is active.

	if (!(p2_state & CALRAM_CS)) {
		//XXX write from calram
		found += 1;
		LOGMASKED(DEBUG_BUS, "write 0x%02X to CAL RAM[0x%02X]\n", data, offset);
	}
	if (!(p2_state & DIPSWITCH_CS)) {
		logerror("Illegal write to DIP switch !\n");
		found += 1;
	}
	if (!(p2_state & GPIB_CS)) {
		found += 1 ;
		LOGMASKED(DEBUG_BUS, "GPIB register %X write 0x%02X\n", offset & 0x07, data);
	}


	if (!found) {
		logerror("Bus write with no CS active !\n");
	}

	if (found > 1) {
		logerror("Bus write with more than one CS active !\n");
	}
}


/* Yuck. Emulate serial LCD module interface. don't really want to make a separate driver for this...
 * The protocol is common to many HP products of the era. Some sources have the instruction words written as 10-bit
 * words, but it would appear more consistent (and matches the intent guessed from the disassembled functions)
 * that they are actually 8-bit bytes. The 2-bit difference is a "bogus" 2 clock cycles for when SYNC or PWO changes ?
 *
*/

/** copy data in shiftreg to the high nibble of each digit in chrbuf */
static void lcd_update_hinib(uint8_t *chrbuf, uint64_t shiftreg)
{
	int i;
	for (i=11; i >= 0; i--) {
		chrbuf[i] &= 0x0F;
		chrbuf[i] |= (shiftreg & 0x0F) << 4;
		shiftreg >>= 4;
	}
}

/** copy data in shiftreg to the low nibble of each digit in chrbuf */
static void lcd_update_lonib(uint8_t *chrbuf, uint64_t shiftreg)
{
	int i;
	for (i=11; i >= 0; i--) {
		chrbuf[i] &= 0xF0;
		chrbuf[i] |= (shiftreg & 0x0F);
		shiftreg >>= 4;
	}
}

/** map LCD char buffer to ASCII
 *
 * discards extra bits
 */
static void lcd_map_ascii(uint8_t *chrbuf, uint8_t *sbuf)
{
	int i;
	for (i=0; i < 12; i++) {
		sbuf[i] = (chrbuf[i] & 0x3F)+ 0x40;
	}
	sbuf[12] = 0;
}

// ISA command bytes
#define DISP_ISA_WANNUN 0xBC	//annunciators
#define DISP_ISA_WA 0x0A	//low nibbles
#define DISP_ISA_WB 0x1A	//hi nib
#define DISP_ISA_WC 0x2A	// "extended bit" ?

void hp3478a_state::lcd_interface(uint8_t p2new)
{
	bool pwo_state, sync_state, isa_state, iwa_state;

	pwo_state = p2new & DISP_PWO;
	sync_state = p2new	 & DISP_SYNC;
	isa_state = p2new & DISP_ISA;
	iwa_state = p2new & DISP_IWA;

	if (!((p2new ^ p2_oldstate) & DISP_CK1)) {
		// no clock edge : boring.
		//LOGMASKED(DEBUG_LCD, "LCD : pwo(%d), sync(%d), isa(%d), iwa(%d)\n",
		//		pwo_state, sync_state, isa_state, iwa_state);
		return;
	}

	if (!(p2new & DISP_CK1)) {
		//neg edge
		return;
	}

	// CK1 clock positive edge
	if (!pwo_state) {
		//not selected, reset everything.
		LOGMASKED(DEBUG_LCD, "LCD : state=IDLE, PWO deselected, %d stray bits(0x...%02X)\n",lcd_bitcount, lcd_bitbuf & 0xFF);
		m_lcdstate = lcd_state::IDLE;
		lcd_bitcount = 0;
		lcd_bitbuf = 0;
		return;
	}
	switch (m_lcdstate) {
		case lcd_state::IDLE:
			lcd_want = 8;
			m_lcdstate = lcd_state::SYNC_SKIP;
			break;
		case lcd_state::SYNC_SKIP:
			// if SYNC changed, we need to ignore two clock pulses.
			lcd_bitcount++;
			if (lcd_bitcount < 1) {
				break;
			}
			lcd_bitcount = 0;
			lcd_bitbuf = 0;
			if (sync_state) {
				m_lcdstate = lcd_state::SELECTED_ISA;
				lcd_want = 8;
				LOGMASKED(DEBUG_LCD, "LCD : state=SELECTED_ISA\n");
			} else {
				//don't touch lcd_want since it was possibly set in the ISA stage
				m_lcdstate = lcd_state::SELECTED_IWA;
				LOGMASKED(DEBUG_LCD, "LCD : state=SELECTED_IWA, want %d\n", lcd_want);
			}
			break;
		case lcd_state::SELECTED_ISA:
			if (!sync_state) {
				m_lcdstate = lcd_state::SYNC_SKIP;
				if (lcd_bitcount) {
					LOGMASKED(DEBUG_LCD, "LCD : ISA->IWA, %d stray bits (0x%0X)\n", lcd_bitcount, lcd_bitbuf);
				} else {
					LOGMASKED(DEBUG_LCD, "LCD : ISA->IWA\n");
				}
				lcd_bitcount = 0;
				lcd_bitbuf = 0;
				break;
			}
			lcd_bitbuf |= (isa_state << lcd_bitcount);
			lcd_bitcount++;
			if (lcd_bitcount == lcd_want) {
				LOGMASKED(DEBUG_LCD, "LCD : Instruction 0x%02X\n", lcd_bitbuf & 0xFF);
				//shouldn't get extra bits, but we have nothing better to do so just reset the shiftreg.
				lcd_bitcount = 0;
				switch (lcd_bitbuf & 0xFF) {
				case DISP_ISA_WANNUN:
					lcd_want = 44;
					m_lcdiwa = lcd_iwatype::ANNUNS;
					break;
				case DISP_ISA_WA:
					lcd_want = 100;	//no, doesn't fit in a uint64, but only the first 36 bits are significant.
					m_lcdiwa = lcd_iwatype::REG_A;
					break;
				case DISP_ISA_WB:
					lcd_want = 100;
					m_lcdiwa = lcd_iwatype::REG_B;
					break;
				case DISP_ISA_WC:
					lcd_want = 44;
					m_lcdiwa = lcd_iwatype::REG_C;
					break;
				default:
					lcd_want = 44;
					m_lcdiwa = lcd_iwatype::DISCARD;
					break;
				}
				lcd_bitbuf = 0;
			}
			break;
		case lcd_state::SELECTED_IWA:
			if (sync_state) {
				m_lcdstate = lcd_state::SYNC_SKIP;
				if (lcd_bitcount) {
					LOGMASKED(DEBUG_LCD, "LCD : IWA->ISA, %d stray bits (0x%I64X)\n", lcd_bitcount, lcd_bitbuf);
				} else {
					LOGMASKED(DEBUG_LCD, "LCD : IWA->ISA\n");
				}
				lcd_bitcount = 0;
				lcd_bitbuf = 0;
				break;
			}
			if (lcd_bitcount <= 0x3F) {
				//clamp to bit 63;
				lcd_bitbuf |= ((uint64_t) iwa_state << lcd_bitcount);
			}
			lcd_bitcount++;
			if (lcd_bitcount != lcd_want) {
				break;
			}
			LOGMASKED(DEBUG_LCD, "LCD : data 0x%I64X\n", lcd_bitbuf);
			switch (m_lcdiwa) {
			case lcd_iwatype::ANNUNS:
				LOGMASKED(DEBUG_LCD2, "LCD : write annuns 0x%02X\n", lcd_bitbuf & 0xFF);
				break;
			case lcd_iwatype::REG_A:
				LOGMASKED(DEBUG_LCD2, "LCD : write reg A (lonib)\n");
				lcd_update_lonib(lcd_chrbuf, lcd_bitbuf);
				lcd_map_ascii(lcd_chrbuf, lcd_text);
				LOGMASKED(DEBUG_LCD2, "LCD text: %s\n", (char *) lcd_text);
				break;
			case lcd_iwatype::REG_B:
				LOGMASKED(DEBUG_LCD2, "LCD : write reg B (hinib) %I64X\n", lcd_bitbuf);
				lcd_update_hinib(lcd_chrbuf, lcd_bitbuf);
				lcd_map_ascii(lcd_chrbuf, lcd_text);
				LOGMASKED(DEBUG_LCD2, "LCD text: %s\n", (char *) lcd_text);
				break;
			default:
				//discard
				break;
			}
			//shouldn't get extra bits, but we have nothing better to do so just reset the shiftreg.
			lcd_bitcount = 0;
			lcd_bitbuf = 0;
			break;	//case SELECTED_IWA
	}

	return;
}





void hp3478a_state::machine_start()
{
	m_bank0->configure_entries(0, 2, memregion("maincpu")->base(), 0x1000);
}

/******************************************************************************
 Address Maps
******************************************************************************/

void hp3478a_state::i8039_map(address_map &map)
{
	//map(0x0000, 0x0fff).rom(); /* CPU address space : 4kB */
	map(0x0000, 0x0fff).bankr("bank0");	// CPU address space (4kB), banked according to P26 pin
}

void hp3478a_state::i8039_io(address_map &map)
{
	map.global_mask(0xff);
	map(0,0xff).rw(FUNC(hp3478a_state::busread), FUNC(hp3478a_state::buswrite));	//"external" access callbacks
}

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( hp3478a )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

void hp3478a_state::hp3478a(machine_config &config)
{
	auto &mcu(I8039(config, "maincpu", CPU_CLOCK));
	mcu.set_addrmap(AS_PROGRAM, &hp3478a_state::i8039_map);
	mcu.set_addrmap(AS_IO, &hp3478a_state::i8039_io);
	mcu.p1_in_cb().set(FUNC(hp3478a_state::p1read));
	mcu.p1_out_cb().set(FUNC(hp3478a_state::p1write));
	mcu.p2_out_cb().set(FUNC(hp3478a_state::p2write));
}

/******************************************************************************
 ROM Definitions
******************************************************************************/
ROM_START( hp3478a )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD_OPTIONAL("rom_dc118.bin", 0, 0x2000, CRC(10097ced) SHA1(bd665cf7e07e63f825b2353c8322ed8a4376b3bd))	//main CPU ROM, can match other datecodes too
ROM_END

/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                        FULLNAME             FLAGS
COMP( 1983, hp3478a,  0,      0,  hp3478a, hp3478a,hp3478a_state, empty_init, "HP", "HP 3478A Multimeter", MACHINE_IS_INCOMPLETE | MACHINE_NO_SOUND_HW | MACHINE_TYPE_OTHER)
