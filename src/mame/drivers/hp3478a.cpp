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
* Current status : runs, AD LINK ERROR on stock ROM due to unimplemented AD link
* - patching the AD comms, we get to a mostly functional state
* - pressing Shift+Sgl to force a 'test/reset' gets the firmware in an endless loop, not sure why.
*
*
* TODO next level
* * annunciators
* * DIP switches
* * do something for analog CPU serial link (not quite uart), or dump+emulate CPU
* * better display render and layout (show button subtext)
*
* TODO level 9000
* * Connect this with the existing i8291.cpp driver
* * dump + add analog CPU (8049)
* * validate one single chipselect active when doing external access (movx)


**** Hardware details (refer to service manual for schematics)
Main CPU : i8039 , no internal ROM
Analog (floating) CPU : i8049, internal ROM (never dumped ?)
ROM : 2764 (64kbit, org 8kB)
RAM : 5101 , 256 * 4bit (!), battery-backed calibration data
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
#include "hp3478a.lh"


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

// IO banking : indexes of m_iobank maps
#define CALRAM_ENTRY 0
#define GPIB_ENTRY 1
#define DIP_ENTRY 2

/**** optional debug outputs, must be before #include logmacro.*/
#define DEBUG_PORTS (LOG_GENERAL << 1)
#define DEBUG_BANKING (LOG_GENERAL << 2)
#define DEBUG_BUS (LOG_GENERAL << 3)	//not used after all
#define DEBUG_KEYPAD (LOG_GENERAL << 4)
#define DEBUG_LCD (LOG_GENERAL << 5)	//low level
#define DEBUG_LCD2 (LOG_GENERAL << 6)
#define DEBUG_CAL (LOG_GENERAL << 7)

#define VERBOSE (DEBUG_BUS)	//can be combined, like (DEBUG_CAL | DEBUG_KEYPAD)

#include "logmacro.h"



/***** callbacks */
/* port1 manages the keypad matrix */

READ8_MEMBER( hp3478a_state::p1read )
{
	unsigned i;
	uint8_t data = m_maincpu->p1_r() | 0x0F; //P10-P13 "pull-up"

	// for each column, set Px=0 for pressed buttons (active low)
	for (i = 0; i < 4; i++) {
		if (!(data & (0x10 << i))) {
			data &= (0xF0 | m_keypad[i]->read());	//not sure if the undefined upper bits will read as 1 ?
		}
	}
	LOGMASKED(DEBUG_KEYPAD, "port1 read: 0x%02X\n", data);
	return data;
}


/** a lot of stuff multiplexed on the P2 pins.
 * parse the chipselect lines, A12 line, and LCD interface.
 */
WRITE8_MEMBER( hp3478a_state::p2write )
{
	LOGMASKED(DEBUG_PORTS, "port2 write: %02X\n", data);

	// check which CS line is active. No collision checking is done here
	// because the LCD interface reuses those pins and we'd get spurious errors.
	// So the last evaluated condition will be kept.

	if (!(data & CALRAM_CS)) {
		//will read lower 4 bits from calram
		m_iobank->set_bank(CALRAM_ENTRY);
	}
	if (!(data & DIPSWITCH_CS)) {
		m_iobank->set_bank(DIP_ENTRY);
	}
	if (!(data & GPIB_CS)) {
		m_iobank->set_bank(GPIB_ENTRY);
	}

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


READ8_MEMBER( hp3478a_state::dipread )
{
	logerror("dip read unimpl\n");
	return 0;
}


/* CAL RAM write handler, to implement "CAL enable" front panel switch
*/
WRITE8_MEMBER( hp3478a_state::nvwrite ) {
	if (m_calenable->read()) {
		m_nvram_raw[offset] = data;
		LOGMASKED(DEBUG_CAL, "write %02X to cal[%02X]\n", data, offset);
	} else {
		LOGMASKED(DEBUG_CAL, "write %02X to cal[%02X]:dropped\n", data, offset);
	}
}


/**** LCD emulation
 *
 * Yuck. Emulate serial LCD module interface. don't really want to make a separate driver for this...
 * The protocol is common to many HP products of the era. Some sources have the instruction words written as 10-bit
 * words, but it would appear more consistent (and matches the intent guessed from the disassembled functions)
 * that they are actually 8-bit bytes. The 2-bit difference is a "bogus" 2 clock cycles for when SYNC or PWO changes ?
 *
*/

/** charset copied from roc10937 driver. Some special chars are wrong.
 * Interestingly, the 3478a usually doesn't use "0x30" for the number 0, but instead
 * maps it to the character 'O' ! It does use 0x30 when printing the GPIB address however.
 */
static const uint16_t hpcharset[]=
{           // FEDC BA98 7654 3210
	0x507F, // 0101 0000 0111 1111 @.
	0x44CF, // 0100 0100 1100 1111 A.
	0x153F, // 0001 0101 0011 1111 B.
	0x00F3, // 0000 0000 1111 0011 C.
	0x113F, // 0001 0001 0011 1111 D.
	0x40F3, // 0100 0000 1111 0011 E.
	0x40C3, // 0100 0000 1100 0011 F.
	0x04FB, // 0000 0100 1111 1011 G.
	0x44CC, // 0100 0100 1100 1100 H.
	0x1133, // 0001 0001 0011 0011 I.
	0x007C, // 0000 0000 0111 1100 J.
	0x4AC0, // 0100 1010 1100 0000 K.
	0x00F0, // 0000 0000 1111 0000 L.
	0x82CC, // 1000 0010 1100 1100 M.
	0x88CC, // 1000 1000 1100 1100 N.
	0x00FF, // 0000 0000 1111 1111 O.
	0x44C7, // 0100 0100 1100 0111 P.
	0x08FF, // 0000 1000 1111 1111 Q.
	0x4CC7, // 0100 1100 1100 0111 R.
	0x44BB, // 0100 0100 1011 1011 S.
	0x1103, // 0001 0001 0000 0011 T.
	0x00FC, // 0000 0000 1111 1100 U.
	0x22C0, // 0010 0010 1100 0000 V.
	0x28CC, // 0010 1000 1100 1100 W.
	0xAA00, // 1010 1010 0000 0000 X.
	0x9200, // 1001 0010 0000 0000 Y.
	0x2233, // 0010 0010 0011 0011 Z.
	0x00E1, // 0000 0000 1110 0001 [.
	0x8800, // 1000 1000 0000 0000 \.
	0x001E, // 0000 0000 0001 1110 ].
	0x2800, // 0010 1000 0000 0000 ^.
	0x0030, // 0000 0000 0011 0000 _.
	0x0000, // 0000 0000 0000 0000 [space] , 0x20
	0x8121, // 1000 0001 0010 0001 !.
	0x0180, // 0000 0001 1000 0000 ".
	0x553C, // 0101 0101 0011 1100 #.
	0x55BB, // 0101 0101 1011 1011 $.
	0x7799, // 0111 0111 1001 1001 %.
	0xC979, // 1100 1001 0111 1001 &.
	0x0200, // 0000 0010 0000 0000 '.
	0x0A00, // 0000 1010 0000 0000 (.
	0xA050, // 1010 0000 0000 0000 ).
	0xFF00, // 1111 1111 0000 0000 *.
	0x5500, // 0101 0101 0000 0000 +.
	0x0000, // 0000 0000 0000 0000  //XXX (0x2C)
	0x4400, // 0100 0100 0000 0000 --.
	0x0000, // 0000 0000 0000 0000  //XXX (0x2E)
	0x2200, // 0010 0010 0000 0000 /.
	0x22FF, // 0010 0010 1111 1111 0. (0x30)
	0x1100, // 0001 0001 0000 0000 1.
	0x4477, // 0100 0100 0111 0111 2.
	0x443F, // 0100 0100 0011 1111 3.
	0x448C, // 0100 0100 1000 1100 4.
	0x44BB, // 0100 0100 1011 1011 5.
	0x44FB, // 0100 0100 1111 1011 6.
	0x000F, // 0000 0000 0000 1111 7.
	0x44FF, // 0100 0100 1111 1111 8.
	0x44BF, // 0100 0100 1011 1111 9.
	0xFFFF, // 1111 1111 1111 1111 [all segs] (0x3A)
	0x2001, // 0010 0000 0000 0001  //XXX
	0x2230, // 0010 0010 0011 0000 <.
	0x4430, // 0100 0100 0011 0000 =.
	0x8830, // 1000 1000 0011 0000 >.
	0x1407, // 0001 0100 0000 0111 ?.
};

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

/** map LCD char to ASCII and segment data + update
 *
 * discards extra bits
 */
void hp3478a_state::lcd_map_chars()
{
	int i;
	LOGMASKED(DEBUG_LCD2, "LCD : map ");
	for (i=0; i < 12; i++) {
		bool dp = lcd_chrbuf[i] & 0x40;	//check decimal point. Needs to be mapped to seg_bit16
		bool comma = lcd_chrbuf[i] & 0x80;	//check comma, maps to seg17
		lcd_text[i] = (lcd_chrbuf[i] & 0x3F) + 0x40;
		lcd_segdata[i] = hpcharset[lcd_chrbuf[i] & 0x3F] | (dp << 16) | (comma << 17);
		LOGMASKED(DEBUG_LCD2, "[%02X>%04X] ", lcd_chrbuf[i] & 0x3F, lcd_segdata[i]);
	}
	LOGMASKED(DEBUG_LCD2, "\n");
}

/** ?? from roc10937 */
static uint32_t lcd_set_display(uint32_t segin)
{
	return bitswap<32>(segin, 31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,11,9,15,13,12,8,10,14,7,6,5,4,3,2,1,0);
}

// ISA command bytes
#define DISP_ISA_WANNUN 0xBC	//annunciators
#define DISP_ISA_WA 0x0A	//low nibbles
#define DISP_ISA_WB 0x1A	//hi nib
#define DISP_ISA_WC 0x2A	// "extended bit" ?

/** LCD serial interface state machine. I cheat and don't implement all commands.
 * Also, it's not clear when exactly the display should be updated. After each regA/regB write
 * seems to generate some glitches. After PWO deselect causes some half-written text to appear sometimes.
 */
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
		//not selected, reset everything
		LOGMASKED(DEBUG_LCD, "LCD : state=IDLE, PWO deselected, %d stray bits(0x...%02X)\n",lcd_bitcount, lcd_bitbuf & 0xFF);
		m_lcdstate = lcd_state::IDLE;
		m_lcdiwa = lcd_iwatype::DISCARD;
		std::transform(std::begin(lcd_segdata), std::end(lcd_segdata), std::begin(*m_outputs), lcd_set_display);
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
				//changing to SELECTED_IWA
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
			if (lcd_bitcount != lcd_want) {
				break;
			}
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
			break;
		case lcd_state::SELECTED_IWA:
			if (sync_state) {
				//changing to SELECTED_ISA
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
				lcd_update_lonib(lcd_chrbuf, lcd_bitbuf);
				lcd_map_chars();
				LOGMASKED(DEBUG_LCD2, "LCD : write reg A (lonib) %I64X, text=%s\n", lcd_bitbuf, (char *) lcd_text);
				break;
			case lcd_iwatype::REG_B:
				lcd_update_hinib(lcd_chrbuf, lcd_bitbuf);
				lcd_map_chars();
				LOGMASKED(DEBUG_LCD2, "LCD : write reg B (lonib) %I64X, text=%s\n", lcd_bitbuf, (char *) lcd_text);
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

	m_outputs = std::make_unique<output_finder<16> >(*this, "vfd%u", (unsigned) 0);
	m_outputs->resolve();

}

/******************************************************************************
 Address Maps
******************************************************************************/

void hp3478a_state::i8039_map(address_map &map)
{
	map(0x0000, 0x0fff).bankr("bank0");	// CPU address space (4kB), banked according to P26 pin
}

void hp3478a_state::i8039_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).m(m_iobank, FUNC(address_map_bank_device::amap8));
}

/* depending on the P2 port state, different chipselect lines are activated, which
 * affect the subsequent external accesses (movx)
 * The addresses in here have nothing to do with the mcs48 address space.
 */
void hp3478a_state::io_bank(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x0ff).ram().region("nvram", 0).share("nvram").w(FUNC(hp3478a_state::nvwrite));
	map(0x100, 0x107).ram().share("gpibregs");	//XXX TODO : connect to i8291.cpp
	map(0x200, 0x2ff).r(FUNC(hp3478a_state::dipread));
}


/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( hp3478a )
/* keypad bit matrix:
			0x08|0x04|0x02|0x01
	col.0 : (nc)|shift|ACA|DCA
	col.1 : 4W|2W|ACV|DCV
	col.2 : int|dn|up|auto
	col.3 : (nc)|loc|srq|sgl
*/
	PORT_START("COL.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("DCA")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("ACA")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("SHIFT")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )	//nothing on 0x08
	PORT_START("COL.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("DCV")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("ACV")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("2W")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("4W")
	PORT_START("COL.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("AUTO")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("UP")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("DN")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("INT")
	PORT_START("COL.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("SGL")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("SRQ")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("LOC")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )	//nothing on 0x08

	PORT_START("CAL_EN")
	PORT_CONFNAME(1, 0, "CAL")
	PORT_CONFSETTING(0x00, "disabled")
	PORT_CONFSETTING(0x01, "enabled")

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
	mcu.p2_out_cb().set(FUNC(hp3478a_state::p2write));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ADDRESS_MAP_BANK(config, m_iobank, 0);
	m_iobank->set_map(&hp3478a_state::io_bank);
	m_iobank->set_data_width(8);
	m_iobank->set_addr_width(18);
	m_iobank->set_stride(0x100);

	// video
	config.set_default_layout(layout_hp3478a);
}

/******************************************************************************
 ROM Definitions
******************************************************************************/
ROM_START( hp3478a )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("rom_dc118.bin", 0, 0x2000, CRC(10097ced) SHA1(bd665cf7e07e63f825b2353c8322ed8a4376b3bd))	//main CPU ROM, can match other datecodes too

	ROM_REGION( 0x100, "nvram", 0 )	/* Calibration RAM, battery-backed */
	ROM_LOAD_OPTIONAL( "calram.bin", 0, 0x100, CRC(0))
ROM_END

/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                        FULLNAME             FLAGS
COMP( 1983, hp3478a,  0,      0,  hp3478a, hp3478a,hp3478a_state, empty_init, "HP", "HP 3478A Multimeter", MACHINE_IS_INCOMPLETE | MACHINE_NO_SOUND_HW | MACHINE_TYPE_OTHER)
