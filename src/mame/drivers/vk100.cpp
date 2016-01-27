// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/***************************************************************************

        DEC VK100 'GIGI'

        12/05/2009 Skeleton driver.
        28/07/2009 added Guru-readme(TM)
        08/01/2012 Fleshed out driver.

        Todo:
              * fix vector generator hardware enough to pass the startup self test
                the tests are described on page 6-5 thru 6-8 of the tech reference
                * hook up the bresenham DU/DVM/ERR stuff, currently only simple directional vectors work
                * hook up the vector and sync proms to the sync counter
                * figure out how the erase prom actually works at a hardware level
                * redump the vector prom, the first two bytes look bad
              * figure out the correct meaning of systat b register - needed for communications selftest
              * hook up smc com5016t baud generator to i8251 rx and tx clocks - begun

        Notes:
              The directions for the DIR value are arranged, starting from the *
              as the vector origin:
                 3  2  1
                  \ | /
                   \|/
                 4--*--0
                   /|\
                  / | \
                 5  6  7

               The X and Y counters are techincally 12 bits long each, though
               only the low 9 and 10 bits respectively are used for ram addressing.
               The MSB bit of each counter does have a special purpose with
               regards to the RAS/ERASE prom though, perhaps to detect an
               underflow 000->FFF

 Tony DiCenzo, now the director of standards and architecture at Oracle, was on the team that developed the VK100
 see http://startup.nmnaturalhistory.org/visitorstories/view.php?ii=79
 Robert "Bob" C. Quinn was definitely lead engineer on the VT125
 Robert "Bob" T. Collins was the lead engineer on the VK100
 Pedro Ortiz (https://www.linkedin.com/pub/pedro-ortiz/16/68b/196) did the drafting for the enclosure and case

 The prototype name for the VK100 was 'SMAKY' (Smart Keyboard)

****************************************************************************/
/*
DEC VK100
DEC, 1982

This is a VK100 terminal, otherwise known as a DEC Gigi graphics terminal.
There's a technical manual dated 1982 here:
http://web.archive.org/web/20091015205827/http://www.computer.museum.uq.edu.au/pdf/EK-VK100-TM-001%20VK100%20Technical%20Manual.pdf
Installation and owner's manual is at:
http://www.bitsavers.org/pdf/dec/terminal/gigi/EK-VK100-IN-002_GIGI_Terminal_Installation_and_Owners_Manual_Apr81.pdf
An enormous amount of useful info can be derived from the VT125 technical manual:
http://www.bitsavers.org/pdf/dec/terminal/vt100/EK-VT100-TM-003_VT100_Technical_Manual_Jul82.pdf starting on page 6-70, pdf page 316
And its schematics:
http://bitsavers.org/pdf/dec/terminal/vt125/MP01053_VT125_Mar82.pdf

PCB Layout
----------

VK100 LOGICBOARD
    |-------|    |---------|  |---------|    |-| |-| |-|  |-|
|---|-20 mA-|----|---EIA---|--|HARD-COPY|----|B|-|G|-|R|--|-|--DSW(8)--|
|                                                         BW           |
|                                                             POWER    |
|                 PR2                                                  |
|                           HD46505SP              4116 4116 4116 4116 |
|                                                                      |
|                                                  4116 4116 4116 4116 |
|      PR5        INTEL           ROM1                                 |
|         PR1 PR6 P8251A                           4116 4116 4116 4116 |
|                     45.6192MHz  ROM2  PR3                            |
|                                                  4116 4116 4116 4116 |
| 4116 4116 4116  INTEL           ROM3                                 |
|                 D8202A                                               |
| 4116 4116 4116       5.0688MHz  ROM4                       PR4       |
|                                                                      |
| 4116 4116       INTEL    SMC_5016T                            PIEZO  |
|                 D8085A                        IDC40   LM556   75452  |
|----------------------------------------------------------------------|
Notes:
      ROM1 - TP-01 (C) DEC 23-031E4-00 (M) SCM91276L 8114
      ROM2 - TP-01 (C) DEC 1980 23-017E4-00 MOSTEK MK36444N 8116
      ROM3 - TP-01 (C) MICROSOFT 1979 23-018E4-00 MOSTEK MK36445N 8113
      ROM4 - TP-01 (C) DEC 1980 23-190E2-00 P8316E AMD 35517 8117DPP

    LED meanings:
    The LEDS on the vk100 (there are 7) are set up above the keyboard as:
    Label: ON LINE   LOCAL     NO SCROLL BASIC     HARD-COPY L1        L2
    Bit:   !d5       d5        !d4       !d3       !d2       !d1       !d0 (of port 0x68)
according to manual from http://www.bitsavers.org/pdf/dec/terminal/gigi/EK-VK100-IN-002_GIGI_Terminal_Installation_and_Owners_Manual_Apr81.pdf
where X = on, 0 = off, ? = variable (- = off)
    - X 0 0 0 0 0 (0x1F) = Microprocessor error
    X - 0 X X X X (0x30) "

    - X 0 0 0 0 X (0x1E) = ROM error
    X - 0 0 ? ? ? (0x3x) "
1E 3F = rom error, rom 1 (0000-0fff)
1E 3E = rom error, rom 1 (1000-1fff)
1E 3D = rom error, rom 2 (2000-2fff)
1E 3C = rom error, rom 2 (3000-3fff)
1E 3B = rom error, rom 3 (4000-4fff)
1E 3A = rom error, rom 3 (5000-5fff)
1E 39 = rom error, rom 4 (6000-6fff)

    - X 0 0 0 X 0 (0x1D) = RAM error
    X - 0 ? ? ? ? (0x3x) "

    - X 0 0 0 X X (0x1C) = CRT Controller error
    X - 0 X X X X (0x30) "
This test writes 0xF to port 00 (crtc address reg) and writes a pattern to it
via port 01 (crtc data reg) then reads it back and checks to be sure the data
matches.

    - X 0 0 X 0 0 (0x1B) = CRT Controller time-out
    X - 0 X X X X (0x30) "
This test writes 00 to all the crtc registers and checks to be sure an rst7.5
(vblank) interrupt fires on the 8085 within a certain time period.

    - X 0 0 X 0 X (0x1A) = Vector time-out error
    X - 0 X X X X (0x30) "
Not sure exactly what this tests, likely tries firing the vector generator
state machine and sees if the GO bit ever finishes and goes back to 0
*/

// named timer IDs
#define TID_I8251_RX 1
#define TID_I8251_TX 2
#define TID_SYNC 3

// show messages related to writes to the 0x4x VG registers
#undef VG40_VERBOSE
// show messages related to writes to the 0x6x VG registers
#undef VG60_VERBOSE
// show messages related to LED/beeper writes
#undef LED_VERBOSE
// show messages related to KYBD writes
#undef KBD_VERBOSE
// debug the pattern reg
#undef PAT_DEBUG
// show reads from the two systat registers
#undef SYSTAT_A_VERBOSE
#undef SYSTAT_B_VERBOSE

// debug state dump for the vector generator
#undef DEBUG_VG_STATE

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "vk100.lh"

#define RS232_TAG       "rs232"
#define COM5016T_TAG    "com5016t"

class vk100_state : public driver_device
{
public:
	enum
	{
		TIMER_EXECUTE_VG
	};

	vk100_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_speaker(*this, "beeper"),
		m_uart(*this, "i8251"),
		m_dbrg(*this, COM5016T_TAG),
		//m_i8251_rx_timer(NULL),
		//m_i8251_tx_timer(NULL),
		//m_sync_timer(NULL),

		m_capsshift(*this, "CAPSSHIFT"),
		m_dipsw(*this, "SWITCHES")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<beep_device> m_speaker;
	required_device<i8251_device> m_uart;
	required_device<com8116_device> m_dbrg;
	//required_device<> m_i8251_rx_timer;
	//required_device<> m_i8251_tx_timer;
	//required_device<> m_sync_timer;

	required_ioport m_capsshift;
	required_ioport m_dipsw;

	UINT8* m_vram;
	UINT8* m_trans;
	UINT8* m_pattern;
	UINT8* m_dir;
	UINT8* m_sync;
	UINT8* m_vector;
	UINT8* m_ras_erase;
	UINT8 m_dir_a6; // latched a6 of dir rom
	UINT8 m_cout; // carry out from vgERR adder
	UINT8 m_vsync; // vsync pin of crtc
	UINT16 m_vgX; // 12 bit X value for vector draw position
	UINT16 m_vgY; // 12 bit Y value for vector draw position
	UINT16 m_vgERR; // error register can cause carries which need to be caught
	UINT8 m_vgSOPS;
	UINT8 m_vgPAT;
	UINT16 m_vgPAT_Mask; // current mask for PAT
	UINT8 m_vgPMUL; // reload value for PMUL_Count
	UINT8 m_vgPMUL_Count;
	UINT8 m_vgDownCount; // down counter = number of pixels, loaded from vgDU on execute
#define VG_DU m_vgRegFile[0]
#define VG_DVM m_vgRegFile[1]
#define VG_DIR m_vgRegFile[2]
#define VG_WOPS m_vgRegFile[3]
	UINT8 m_vgRegFile[4];
	UINT8 m_VG_MODE; // 2 bits, latched on EXEC
	UINT8 m_vgGO; // activated on next SYNC pulse after EXEC
	UINT8 m_ACTS;
	UINT8 m_ADSR;
	ioport_port* m_col_array[16];

	DECLARE_WRITE8_MEMBER(vgLD_X);
	DECLARE_WRITE8_MEMBER(vgLD_Y);
	DECLARE_WRITE8_MEMBER(vgERR);
	DECLARE_WRITE8_MEMBER(vgSOPS);
	DECLARE_WRITE8_MEMBER(vgPAT);
	DECLARE_WRITE8_MEMBER(vgPMUL);
	DECLARE_WRITE8_MEMBER(vgREG);
	DECLARE_WRITE8_MEMBER(vgEX);
	DECLARE_WRITE8_MEMBER(KBDW);
	DECLARE_WRITE8_MEMBER(BAUD);
	DECLARE_READ8_MEMBER(vk100_keyboard_column_r);
	DECLARE_READ8_MEMBER(SYSTAT_A);
	DECLARE_READ8_MEMBER(SYSTAT_B);
	DECLARE_DRIVER_INIT(vk100);
	virtual void machine_start() override;
	virtual void video_start() override;
	TIMER_CALLBACK_MEMBER(execute_vg);
	DECLARE_WRITE_LINE_MEMBER(crtc_vsync);
	DECLARE_WRITE_LINE_MEMBER(i8251_rxrdy_int);
	DECLARE_WRITE_LINE_MEMBER(i8251_txrdy_int);
	DECLARE_WRITE_LINE_MEMBER(i8251_rts);
	UINT8 vram_read();
	UINT8 vram_attr_read();
	MC6845_UPDATE_ROW(crtc_update_row);
	void vram_write(UINT8 data);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

// vram access functions:
	/* figure out vram address based on tech manual page 5-24:
	 * real address to 16-bit chunk a13 a12 | a11 a10 a9  a8 | a7  a6  a5  a4 | a3  a2  a1  a0
	 * X+Y input                    Y8  Y7  | Y6  Y5  Y4  Y3 | Y2  Y1  X9' X8'| X7' X6' X5' X4'
	 *          X3' and X2' choose a 4-bit block, X1 and X0 choose a bit within that.
	 * Figure out the ram address in address space "vram" based on this:
	 * vram is 0x8000 long (0x4000 16-bit blocks in sequence)
	 * so:
	 * vram: a14 a13 a12 | a11 a10 a9  a8 | a7  a6  a5  a4 | a3  a2  a1  a0
	 * vg:   Y8  Y7  Y6  | Y5  Y4  Y3  Y2 | Y1  X9' X8' X7'| X6' X5' X4'(x3') x2' x1 x0
	 *     X3' is handled by the nybbleNum statement, as is X2'
	 *     X1 and X0 are handled by the pattern rom directly:
	 *     x0 -> a0, x1 -> a1
	 *     this handles bits like:
	 *     x1 x0
	 *      0  0 -> bit 0
	 *      0  1 -> bit 1
	 *      1  0 -> bit 2
	 *      1  1 -> bit 3
	 */

// returns one nybble from vram array based on X and Y regs
UINT8 vk100_state::vram_read()
{
	// XFinal is (X'&0x3FC)|(X&0x3)
	UINT16 XFinal = m_trans[(m_vgX&0x3FC)>>2]<<2|(m_vgX&0x3); // appears correct
	// EA is the effective ram address for a 16-bit block
	UINT16 EA = ((m_vgY&0x1FE)<<5)|(XFinal>>4); // appears correct
	// block is the 16 bit block directly (note EA has to be <<1 to correctly index a byte)
	UINT16 block = m_vram[(EA<<1)+1] | (m_vram[(EA<<1)]<<8);
	// nybbleNum is which of the four nybbles within the block to address. should NEVER be 3!
	UINT8 nybbleNum = (XFinal&0xC)>>2;
	return (block>>(4*nybbleNum))&0xF;
}

// returns the attribute nybble for the current pixel based on X and Y regs
UINT8 vk100_state::vram_attr_read()
{
	// XFinal is (X'&0x3FC)|(X&0x3)
	UINT16 XFinal = m_trans[(m_vgX&0x3FC)>>2]<<2|(m_vgX&0x3); // appears correct
	// EA is the effective ram address for a 16-bit block
	UINT16 EA = ((m_vgY&0x1FE)<<5)|(XFinal>>4); // appears correct
	// block is the 16 bit block directly (note EA has to be <<1 to correctly index a byte)
	UINT16 block = m_vram[(EA<<1)+1] | (m_vram[(EA<<1)]<<8);
	// nybbleNum is the attribute nybble, which in this case is always 3
	UINT8 nybbleNum = 3;
	return (block>>(4*nybbleNum))&0xF;
}

// writes one nybble to vram array based on X and Y regs, and updates the attrib ram if needed
void vk100_state::vram_write(UINT8 data)
{
	// XFinal is (X'&0x3FC)|(X&0x3)
	UINT16 XFinal = m_trans[(m_vgX&0x3FC)>>2]<<2|(m_vgX&0x3); // appears correct
	// EA is the effective ram address for a 16-bit block
	UINT16 EA = ((m_vgY&0x1FE)<<5)|(XFinal>>4); // appears correct
	// block is the 16 bit block directly (note EA has to be <<1 to correctly index a byte)
	UINT16 block = m_vram[(EA<<1)+1] | (m_vram[(EA<<1)]<<8);
	// nybbleNum is which of the four nybbles within the block to address. should NEVER be 3!
	UINT8 nybbleNum = (XFinal&0xC)>>2;
	block &= ~((UINT16)0xF<<(nybbleNum*4)); // mask out the part we want to replace
	block |= data<<(nybbleNum*4); // write the new part
	// NOTE: this next part may have to be made conditional on VG_MODE
	// check if the attribute nybble is supposed to be modified, and if so do so
	if (VG_WOPS&0x08) block = (block&0x0FFF)|(((UINT16)VG_WOPS&0xF0)<<8);
	m_vram[(EA<<1)+1] = block&0xFF; // write block back to vram
	m_vram[(EA<<1)] = (block&0xFF00)>>8; // ''
}

void vk100_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_EXECUTE_VG:
		execute_vg(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in vk100_state::device_timer");
	}
}

/* this is the "DIRECTION ROM"  == mb6309 (256x8, 82s135)
 * see figure 5-24 on page 5-39
 * It tells the direction and enable for counting on the X and Y counters
 * and also handles the non-math related parts of the bresenham line algorithm
 * control bits:
 *            /CE1 ----- DCOUNT 0 H [verified via tracing]
 *            /CE2 ----- ENA ERROR L [verified via tracing]
 * addr bits: 76543210
 *            ||||\\\\-- DIR (vgDIR register low 4 bits)
 *            |||\------ C OUT aka ERROR CARRY (strobed in by STROBE L from the error counter's adder) [verified via tracing]
 *            ||\------- Y0 (the otherwise unused lsb of the Y register, used for bresenham) [verified via tracing]
 *            |\-------- feedback bit from d5 strobed by V CLK [verified via tracing]
 *            \--------- GND; the second half of the prom is blank (0x00)
 * data bits: 76543210
 *            |||||||\-- ENA Y (enables change on Y counter)
 *            ||||||\--- ENA X (enables change on X counter)
 *            |||||\---- Y DIRECTION (high is count down, low is count up)
 *            ||||\----- X DIRECTION (high is count down, low is count up)
 *            |||\------ PIXEL WRT
 *            ||\------- feedback bit to a6, this bit is held in PRESET/1 condition by GO being inactive, and if the vector prom is disabled it is pulled to 1 [verified via tracing and schematics]
 *            |\-------- UNUSED, always 0
 *            \--------- UNUSED, always 0
 * The VT125 prom @ E41 is literally identical to this, the same exact part: 23-059B1
 */
TIMER_CALLBACK_MEMBER(vk100_state::execute_vg)
{
	m_cout = 1; // hack for now
	UINT8 dirbyte = m_dir[(m_dir_a6<<6)|((m_vgY&1)<<5)|(m_cout<<4)|VG_DIR];
#ifdef DEBUG_VG_STATE
	static const char *const vg_functions[] = { "Move", "Dot", "Vector", "Erase" };
	fprintf(stderr, "VGMODE: %s; DIR: A:%02x; D:%02x; X: %03X; Y: %03X; DownCount: %02X\n", vg_functions[m_VG_MODE], ((m_dir_a6<<6)|((m_vgY&1)<<5)|(m_cout<<4)|VG_DIR), dirbyte, m_vgX, m_vgY, m_vgDownCount);
#endif
	m_dir_a6 = m_vgGO?((dirbyte&0x20)>>5):1;
	if (dirbyte&2) // ena_x is active
	{
		if (dirbyte&0x80) m_vgX--;
		else m_vgX++;
	}
	if (dirbyte&1) // ena_y is active
	{
		if (dirbyte&0x40) m_vgY--;
		else m_vgY++;
	}
	if (dirbyte&0x10) m_vgDownCount--; // decrement the down counter
	UINT8 thisNyb = vram_read(); // read in the nybble
	// pattern rom addressing is a complex mess. see the pattern rom def later in this file.
	UINT8 newNyb = m_pattern[((m_vgPAT&m_vgPAT_Mask)?0x200:0)|((VG_WOPS&7)<<6)|((m_vgX&3)<<4)|thisNyb]; // calculate new nybble based on pattern rom
	// finally write the block back to ram depending on the VG_MODE (sort of a hack until we get the vector and sync and dir roms all hooked up)
	// but only do it if the direction rom said so!
	switch (m_VG_MODE)
	{
		case 0: // move; adjusts the x and y but doesn't write anything. do nothing
			break;
		case 1: // dot: only write the LAST pixel in the chain? TODO: some fallthrough magic here?
			if ((m_vgDownCount) == 0x00)
			{
				if (dirbyte&0x10) vram_write(newNyb); // write out the modified nybble
			}
			break;
		case 2: // vec: draw the vector
				if (dirbyte&0x10) vram_write(newNyb); // write out the modified nybble
			break;
		case 3: // er: erase: special case here: wipe the entire screen (except for color/attrib?) and then set done.
			for (int i = 0; i < 0x8000; i++)
			{
				if (!(i&1)) // avoid stomping attribute
					m_vram[i] = m_vram[i]&0xF0;
				else // (i&1)
					m_vram[i] = 0;
			}
			m_vgGO = 0; // done
			break;
	}
	if ((m_vgDownCount) == 0x00) m_vgGO = 0; // check if the down counter hit terminal count (0), if so we're done.
	if (((++m_vgPMUL_Count)&0xF)==0) // if pattern multiplier counter overflowed
	{
		m_vgPMUL_Count = m_vgPMUL; // reload counter
		m_vgPAT_Mask >>= 1; // shift the mask
		if (m_vgPAT_Mask == 0) m_vgPAT_Mask = 0x80; // reset mask if it hits 0
	}
	if (m_vgGO) timer_set(attotime::from_hz(XTAL_45_6192Mhz/3/12/2), TIMER_EXECUTE_VG); // /3/12/2 is correct. the sync counter is clocked by the dot clock, despite the error on figure 5-21
}

/* ports 0x40 and 0x41: load low and high bytes of vector gen X register */
WRITE8_MEMBER(vk100_state::vgLD_X)
{
	m_vgX &= 0xFF << ((1-offset)*8);
	m_vgX |= ((UINT16)data) << (offset*8);
#ifdef VG40_VERBOSE
	logerror("VG: 0x%02X: X Reg loaded with %04X, new X value is %04X\n", 0x40+offset, ((UINT16)data) << (offset*8), m_vgX);
#endif
}

/* ports 0x42 and 0x43: load low and high bytes of vector gen Y register */
WRITE8_MEMBER(vk100_state::vgLD_Y)
{
	m_vgY &= 0xFF << ((1-offset)*8);
	m_vgY |= ((UINT16)data) << (offset*8);
#ifdef VG40_VERBOSE
	logerror("VG: 0x%02X: Y Reg loaded with %04X, new Y value is %04X\n", 0x42+offset, ((UINT16)data) << (offset*8), m_vgY);
#endif
}

/* port 0x44: "ERR" load bresenham line algorithm 'error' count */
WRITE8_MEMBER(vk100_state::vgERR)
{
	m_vgERR = data;
#ifdef VG40_VERBOSE
	logerror("VG: 0x44: ERR Reg loaded with %02X\n", m_vgERR);
#endif
}

/* port 0x45: "SOPS" screen options
 * (handled by 74LS273 @ E55, schematic sheet 10, all signals called 'VVG1 BDx' where x is 7 to 0)
 * Blink   --Background color--    Blink   Serial  Serial  Reverse
 * Enable  Green   Red     Blue    Control SL1     SL0     BG/FG
 * d7      d6      d5      d4      d3      d2      d1      d0
 * apparently, SLx: 00 = rs232/eia(J6), 01 = 20ma(J1), 10 = hardcopy(J7), 11 = test/loopback
 * Serial Select (SLx) routing controls are rather complex, shown on schematic
 * page 9:
 * VDC2    |  I8251 pins                                                      |  SYSTAT_B bits
 * SL1 SL0 |  8251RXD   8251RTS    8251TXD    8251/DTR    8251/DSR    8251CTS |  SYSTATB_ACTS   SYSTATB_ADSR
 * 0   0      J6 /RXD   J6 /RTS    J6 TXD     J6 /DTR     J7 URTS     GND        J6 /CTS        J6 /DSR
 * 0   1      J1 +-R    ACTS(loop) J1 +-T     J6 /DTR     J7 URTS     GND        8251RTS(loop)  J6 /DSR
 * 1   0      J7 DRXD   J7 DRTS*   J7 DTXD    J6 /DTR     J7 URTS     GND        J7 /DCTS       J6 /DSR
 * 1   1      8251TXD   ACTS(loop) 8251RXD    J6 /DTR     J7 URTS     GND        8251RTS(loop)  J6 /DSR
 *                      * and UCTS, the pin drives both pins on J7
 */
WRITE8_MEMBER(vk100_state::vgSOPS)
{
	m_vgSOPS = data;
#ifdef VG40_VERBOSE
	static const char *const serialDest[4] = { "EIA232", "20ma", "Hardcopy", "Loopback/test" };
	logerror("VG: 0x45: SOPS Reg loaded with %02X: Background KGRB: %d%d%d%d, Blink: %d, Serial select: %s, Reverse BG/FG: %d\n", m_vgSOPS, (m_vgSOPS>>7)&1, (m_vgSOPS>>6)&1, (m_vgSOPS>>5)&1, (m_vgSOPS>>4)&1, (m_vgSOPS>>3)&1, serialDest[(m_vgSOPS>>1)&3], m_vgSOPS&1);
#endif
}

/* port 0x46: "PAT" load vg Pattern register */
WRITE8_MEMBER(vk100_state::vgPAT)
{
	m_vgPAT = data;
#ifdef PAT_DEBUG
	for (int i = 7; i >= 0; i--)
	{
		printf("%s", (data&(1<<i))?"X":".");
	}
	printf("\n");
#endif
#ifdef VG40_VERBOSE
	logerror("VG: 0x46: PAT Reg loaded with %02X\n", m_vgPAT);
#endif
}

/* port 0x47: "PMUL" load vg Pattern Multiply Register
   The pattern multiply register loads a counter which counts reads from
   the pattern register and increments on each one. if it overflows from
   1111 to 0000 the pattern register is shifted right one bit and the
   counter is reloaded from PMUL */
WRITE8_MEMBER(vk100_state::vgPMUL)
{
	m_vgPMUL = data;
#ifdef VG40_VERBOSE
	logerror("VG: 0x47: PMUL Reg loaded with %02X\n", m_vgPMUL);
#endif
}

/* port 0x60: "DU" load vg vector major register */
/* port 0x61: "DVM" load vg vector minor register */
/* port 0x62: "DIR" load vg Direction register */
/* port 0x63: "WOPS" vector 'pixel' write options
 * --Attributes to change --   Enable --  Functions --
 * Blink  Green  Red    Blue   Attrib F1     F0     FN
 *                             Change
 * d7     d6     d5     d4     d3     d2     d1     d0
 */
WRITE8_MEMBER(vk100_state::vgREG)
{
	m_vgRegFile[offset] = data;
#ifdef VG60_VERBOSE
	static const char *const regDest[4] = { "DU", "DVM", "DIR", "WOPS" };
	static const char *const wopsFunctions[] = { "Overlay", "Replace", "Complement", "Erase" };
	if (offset < 3) logerror("VG: 0x%02x: %s Reg loaded with %02X\n", (0x60+offset), regDest[offset], m_vgRegFile[offset]);
	else logerror("VG: 0x63: WOPS Reg loaded with %02X: KGRB %d%d%d%d, AttrChange %d, Function %s, Negate %d\n", data, (VG_WOPS>>7)&1, (VG_WOPS>>6)&1, (VG_WOPS>>5)&1, (VG_WOPS>>4)&1, (VG_WOPS>>3)&1, wopsFunctions[(VG_WOPS>>1)&3], VG_WOPS&1);
#endif
}


/* port 0x64: "EX MOV" execute a move (relative move of x and y using du/dvm/dir/err, no writing) */
/* port 0x65: "EX DOT" execute a dot (draw a dot at x,y?) */
/* port 0x66: "EX VEC" execute a vector (draw a vector from x,y to a destination using du/dvm/dir/err ) */
/* port 0x67: "EX ER" execute an erase (clear the screen to the bg color, i.e. fill vram with zeroes) */
WRITE8_MEMBER(vk100_state::vgEX)
{
#ifdef VG60_VERBOSE
	static const char *const ex_functions[] = { "Move", "Dot", "Vector", "Erase" };
	logerror("VG Execute %s 0x%02X written with %d\n", ex_functions[offset&3], 0x67+offset, data);
	//fprintf(stderr, "VG Execute %s 0x%02X written with %d\n", ex_functions[offset&3], 0x67+offset, data);
#endif
	m_vgPMUL_Count = m_vgPMUL; // load PMUL_Count
	m_vgPAT_Mask = 0x80;
	m_vgDownCount = VG_DU; // set down counter to length of major vector
	m_VG_MODE = offset&3;
	m_vgGO = 1;
	timer_set(attotime::zero, TIMER_EXECUTE_VG);
}


/* port 0x68: "KBDW" d7 is beeper, d6 is keyclick, d5-d0 are keyboard LEDS */
WRITE8_MEMBER(vk100_state::KBDW)
{
	output().set_value("online_led",BIT(data, 5) ? 1 : 0);
	output().set_value("local_led", BIT(data, 5) ? 0 : 1);
	output().set_value("noscroll_led",BIT(data, 4) ? 1 : 0);
	output().set_value("basic_led", BIT(data, 3) ? 1 : 0);
	output().set_value("hardcopy_led", BIT(data, 2) ? 1 : 0);
	output().set_value("l1_led", BIT(data, 1) ? 1 : 0);
	output().set_value("l2_led", BIT(data, 0) ? 1 : 0);
#ifdef LED_VERBOSE
	if (BIT(data, 6)) logerror("kb keyclick bit 6 set: not emulated yet (multivibrator)!\n");
#endif
	m_speaker->set_state(BIT(data, 7));
#ifdef LED_VERBOSE
	logerror("LED state: %02X: %s %s %s %s %s %s\n", data&0xFF, (data&0x20)?"------- LOCAL ":"ON LINE ----- ", (data&0x10)?"--------- ":"NO SCROLL ", (data&0x8)?"----- ":"BASIC ", (data&0x4)?"--------- ":"HARD-COPY ", (data&0x2)?"-- ":"L1 ", (data&0x1)?"-- ":"L2 ");
#endif
}

/* port 0x6C: "BAUD" controls the smc com5016t dual baud generator which
 * controls the divisors for the rx and tx clocks on the 8251 from the
    5.0688Mhz cpu xtal.
   It has 5v,12v on pins 2 and 9, pin 10 is NC.
 * A later part that replaced this on the market is SMC COM8116(T)/8136(T), which
    was a 5v-only part (pin 9 and 10 are NC, 10 is a clock out on the 8136.
 *  Note that even on the SMC COM5016T version, SMC would allow the user
     to mask their own dividers on custom ordered chips if desired.
 *  The COM8116(T)/8136(T) came it at least 4 mask rom types meant for different
     input clocks:
     -000 or no mark for 5.0688Mhz (which exactly matches the table below) (synertek sy2661-3 also matches this exactly)
     -003 is for 6.01835MHz
     -005 is for 4.915200Mhz
     -006 is for 5.0688Mhz but omits the 2000 baud entry, instead has 200,
      and output frequencies are 2x as fast (meant for a 32X clock uart)
     -013 is for 2.76480MHz
     -013A is for 5.52960MHz
     (several other unknown refclock masks appear on partscalper sites)
    GI also made a clone of the 8116 5v chip called the AY-5-8116(T)/8136(T)
     which had at least two masks: -000/no mark and -005, matching speeds above
    WD made the WD1943 which is similarly 5v compatible, with -00, -05, -06 masks
    The COM8046(T) has 5 bits for selection instead of 4, but still expects
     a 5.0688MHz reference clock, and the second half of the table matches the
     values below; the first half of the table is the values below /2, rounded
     down (for uarts which need a clock rate of 32x baud instead of 16x).
    WD's BR1941 is also functionally compatible but uses 5v,12v,-5v on pins 2,9,10
 * The baud divisor lookup table has 16 entries, but only entries 2,5,6,7,A,C,E,F are documented/used in the vk100 tech manual
 * The others are based on page 13 of http://www.hartetechnologies.com/manuals/Tarbell/Tarbell%20Z80%20CPU%20Board%20Model%203033.pdf
 * D C B A   Divisor                                Expected Baud
 * 0 0 0 0 - 6336 (5068800 / 6336 = 16*50         = 50 baud
 * 0 0 0 1 - 4224 (5068800 / 4224 = 16*75)        = 75 baud
 * 0 0 1 0 - 2880 (5068800 / 2880 = 16*110)       = 110 baud
 * 0 0 1 1 - 2355 (5068800 / 2355 = 16*134.5223) ~= 134.5 baud
 * 0 1 0 0 - 2112 (5068800 / 2112 = 16*150)       = 150 baud
 * 0 1 0 1 - 1056 (5068800 / 1056 = 16*300)       = 300 baud
 * 0 1 1 0 - 528  (5068800 / 528 = 16*600)        = 600 baud
 * 0 1 1 1 - 264  (5068800 / 264 = 16*1200)       = 1200 baud
 * 1 0 0 0 - 176  (5068800 / 176 = 16*1800)       = 1800 baud
 * 1 0 0 1 - 158  (5068800 / 158 = 16*2005.0633) ~= 2000 baud
 * 1 0 1 0 - 132  (5068800 / 132 = 16*2400)       = 2400 baud
 * 1 0 1 1 - 88   (5068800 / 88 = 16*3600)        = 3600 baud
 * 1 1 0 0 - 66   (5068800 / 66 = 16*4800)        = 4800 baud
 * 1 1 0 1 - 44   (5068800 / 44 = 16*7200)        = 7200 baud
 * 1 1 1 0 - 33   (5068800 / 33 = 16*9600)        = 9600 baud
 * 1 1 1 1 - 16   (5068800 / 16 = 16*19800)      ~= 19200 baud
 */
WRITE8_MEMBER(vk100_state::BAUD)
{
	m_dbrg->str_w(data & 0x0f);
	m_dbrg->stt_w(data >> 4);
}

/* port 0x40-0x47: "SYSTAT A"; various status bits, poorly documented in the tech manual
 * /GO    VDM1   VDM1   VDM1   VDM1   Dip     RST7.5 GND***
 *        BIT3   BIT2   BIT1   BIT0   Switch  VSYNC
 * d7     d6     d5     d4     d3     d2      d1     d0
  bit3, 2, 1, 0 are the 4 bits output from the VRAM 12->4 multiplexer
   which are also inputs to the pattern rom; they are constantly updated
   by the sync rom and related circuitry.
  This is the only way the vram can be read by the cpu.
  d7 is from the /Q output of the GO latch
  d6,5,4,3 are from the 74ls298 at ic4 (right edge of pcb)
  d2 is where the dipswitch values are read from, based on the offset
  d1 is connected to 8085 rst7.5 (pin 7) and crtc pin 40 (VSYNC) [verified via tracing]
  d0 is tied to GND [verified via tracing] but the schematics both tie it to GND
     and call it LP FLAG, may be a leftover from development.

 31D reads and checks d7 in a loop
 205 reads, xors with 0x55 (from reg D), ANDS result with 0x78 and branches if it is not zero (checking for bit pattern 1010?)
 299 reads, rotates result right 3 times and ANDs the result with 0x0F
 2A4 reads, rotates result left 1 time and ANDS the result with 0xF0
*/
READ8_MEMBER(vk100_state::SYSTAT_A)
{
	UINT8 dipswitchLUT[8] = { 1,3,5,7,6,4,2,0 }; // the dipswitches map in a weird order to offsets
#ifdef SYSTAT_A_VERBOSE
	if (m_maincpu->pc() != 0x31D) logerror("0x%04X: SYSTAT_A Read!\n", m_maincpu->pc());
#endif
	return ((m_vgGO?0:1)<<7)|(vram_read()<<3)|(((m_dipsw->read()>>dipswitchLUT[offset])&1)?0x4:0)|(m_vsync?0x2:0);
}

/* port 0x48: "SYSTAT B"; NOT documented in the tech manual at all.
 * when in loopback/test mode, SYSTAT_B is read and expected the following, around 0x606:
 * reset 8751, modewrite of 0x5E
 * write command -> 0x20 (normal, normal, /RTS is 0, normal, normal, receive off, /DTR is 1, transmit off)
 * read SYSTAT B (and xor with 0xe), expect d7 to be CLEAR or jump to error
 * write command -> 0x05 (normal, normal, /RTS is 1, normal, normal, receive ON, /DTR is 0, transmit off)
 * read SYSTAT B (and xor with 0xe), expect d7 to be SET or jump to error
 * after this it does something and waits for an rxrdy interrupt

 shows the results of:
 * ACTS (/CTS)  ADSR (/DSR)  GND    GND    ATTR3  ATTR2  ATTR1  ATTR0
 * d7           d6           d5     d4     d3     d2     d1     d0
 * the ACTS (inverse of DCTS) signal lives in one of these bits (see 5-62)
 * it XORs the read of systat_b with the E register (which holds 0x6)
 * and checks the result
 * The 4 attribute ram bits for the cell being pointed at by the X and Y regs are readable as the low nybble.
 * The DSR pin is readable as bit 6.
 */
READ8_MEMBER(vk100_state::SYSTAT_B)
{
#ifdef SYSTAT_B_VERBOSE
	logerror("0x%04X: SYSTAT_B Read!\n", m_maincpu->pc());
#endif
	return (m_ACTS<<7)|(m_ADSR<<6)|vram_attr_read();
}

READ8_MEMBER(vk100_state::vk100_keyboard_column_r)
{
	UINT8 code = m_col_array[offset&0xF]->read() | m_capsshift->read();
#ifdef KBD_VERBOSE
	logerror("Keyboard column %X read, returning %02X\n", offset&0xF, code);
#endif
	return code;
}

static ADDRESS_MAP_START(vk100_mem, AS_PROGRAM, 8, vk100_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM
	AM_RANGE( 0x7000, 0x700f ) AM_MIRROR(0x0ff0) AM_READ(vk100_keyboard_column_r)
	AM_RANGE( 0x8000, 0xbfff ) AM_RAM
ADDRESS_MAP_END

/*
 * 8085 IO address map (x = ignored; * = selects address within this range; ? = not sure; ** = subparts check this bit)
 * (a15 to a8 are latched the same value as a7-a0 on the 8080 and 8085)
 * [this map is derived from extensive tracing as well as some guesswork, noted for the crtc, systat_b and the uart]
   a7  a6  a5  a4  a3  a2  a1  a0
   x   0   x   x   x   x   x   0     W     CRTC address
   x   0   x   x   x   x   x   1     RW    CRTC register r/w
   x   1   *   *   *   **  **  **        read area (rightmost 74ls138):
   x   1   0   0   0   *   *   *     R     SYSTAT_A (a0-a3 chooses the bit of the dipswitches read via d3)
   x   1   0   0   1   x   x   x    R     SYSTAT_B
   x   1   0   1   0   x   x   0     R     i8251 UART data
   x   1   0   1   0   x   x   1     R     i8251 UART status
   x   1   0   1   1   x   x   x     R     unused
   x   1   1   0   0   x   x   x     R     unused
   x   1   1   0   1   x   x   x     R     unused
   x   1   1   1   0   x   x   x     R     unused
   x   1   1   1   1   x   x   x     R     unused
   x   1   0   x   x   *   *   *         write area (right 74ls138):
   x   1   0   x   x   0   0   0     W     X (low 8 bits)
   x   1   0   x   x   0   0   1     W     X (high 4 bits)
   x   1   0   x   x   0   1   0     W     Y (low 8 bits)
   x   1   0   x   x   0   1   1     W     Y (high 4 bits)
   x   1   0   x   x   1   0   0     W     ERR
   x   1   0   x   x   1   0   1     W     SOPS
   x   1   0   x   x   1   1   0     W     PAT
   x   1   0   x   x   1   1   1     W     PMUL
   x   1   1   *   *   *   **  **        write area (middle 74ls138):
   x   1   1   0   0   0   **  **          write to register file 2x 74ls670:
   x   1   1   0   0   0   0   0     W       DU
   x   1   1   0   0   0   0   1     W       DVM
   x   1   1   0   0   0   1   0     W       DIR
   x   1   1   0   0   0   1   1     W       WOPS
   x   1   1   0   0   1   *   *     W     set VG_MODE to * * XOR 3 and Execute (if GO is not active)
   x   1   1   0   1   0   x   x     W     KYBDW
   x   1   1   0   1   1   x   x     W     BAUD
   x   1   1   1   0   0   x   0     W     i8251 UART data
   x   1   1   1   0   0   x   1     W     i8251 UART control
   x   1   1   1   0   1   x   x     W     unused
   x   1   1   1   1   0   x   x     W     unused
   x   1   1   1   1   1   x   x     W     unused
*/
static ADDRESS_MAP_START(vk100_io, AS_IO, 8, vk100_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x7f) // guess, probably correct
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xBE) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0xBE) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	// Comments are from page 118 (5-14) of http://web.archive.org/web/20091015205827/http://www.computer.museum.uq.edu.au/pdf/EK-VK100-TM-001%20VK100%20Technical%20Manual.pdf
	AM_RANGE (0x40, 0x41) AM_MIRROR(0x98) AM_WRITE(vgLD_X)  //LD X LO + HI 12 bits
	AM_RANGE (0x42, 0x43) AM_MIRROR(0x98) AM_WRITE(vgLD_Y)  //LD Y LO + HI 12 bits
	AM_RANGE (0x44, 0x44) AM_MIRROR(0x98) AM_WRITE(vgERR)    //LD ERR ('error' in bresenham algorithm)
	AM_RANGE (0x45, 0x45) AM_MIRROR(0x98) AM_WRITE(vgSOPS)   //LD SOPS (screen options (plus uart dest))
	AM_RANGE (0x46, 0x46) AM_MIRROR(0x98) AM_WRITE(vgPAT)    //LD PAT (pattern register)
	AM_RANGE (0x47, 0x47) AM_MIRROR(0x98) AM_WRITE(vgPMUL)   //LD PMUL (pattern multiplier)
	AM_RANGE (0x60, 0x63) AM_MIRROR(0x80) AM_WRITE(vgREG)     //LD DU, DVM, DIR, WOPS (register file)
	AM_RANGE (0x64, 0x67) AM_MIRROR(0x80) AM_WRITE(vgEX)    //EX MOV, DOT, VEC, ER
	AM_RANGE (0x68, 0x68) AM_MIRROR(0x83) AM_WRITE(KBDW)   //KBDW (probably AM_MIRROR(0x03))
	AM_RANGE (0x6C, 0x6C) AM_MIRROR(0x83) AM_WRITE(BAUD)   //LD BAUD (baud rate clock divider setting for i8251 tx and rx clocks) (probably AM_MIRROR(0x03))
	AM_RANGE (0x70, 0x70) AM_MIRROR(0x82) AM_DEVWRITE("i8251", i8251_device, data_w) //LD COMD (i8251 data reg)
	AM_RANGE (0x71, 0x71) AM_MIRROR(0x82) AM_DEVWRITE("i8251", i8251_device, control_w) //LD COM (i8251 control reg)
	//AM_RANGE (0x74, 0x74) AM_MIRROR(0x83) AM_WRITE(unknown_74)
	//AM_RANGE (0x78, 0x78) AM_MIRROR(0x83) AM_WRITE(kbdw)   //KBDW ?(mirror?)
	//AM_RANGE (0x7C, 0x7C) AM_MIRROR(0x83) AM_WRITE(unknown_7C)
	AM_RANGE (0x40, 0x47) AM_MIRROR(0x80) AM_READ(SYSTAT_A) // SYSTAT A (state machine done and last 4 bits of vram, as well as dipswitches)
	AM_RANGE (0x48, 0x48) AM_MIRROR(0x87/*0x80*/) AM_READ(SYSTAT_B) // SYSTAT B (uart stuff)
	AM_RANGE (0x50, 0x50) AM_MIRROR(0x86) AM_DEVREAD("i8251", i8251_device, data_r) // UART O
	AM_RANGE (0x51, 0x51) AM_MIRROR(0x86) AM_DEVREAD("i8251", i8251_device, status_r) // UAR
	//AM_RANGE (0x58, 0x58) AM_MIRROR(0x87) AM_READ(unknown_58)
	//AM_RANGE (0x60, 0x60) AM_MIRROR(0x87) AM_READ(unknown_60)
	//AM_RANGE (0x68, 0x68) AM_MIRROR(0x87) AM_READ(unknown_68) // NOT USED
	//AM_RANGE (0x70, 0x70) AM_MIRROR(0x87) AM_READ(unknown_70)
	//AM_RANGE (0x78, 0x7f) AM_MIRROR(0x87) AM_READ(unknown_78)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vk100 )
	// the dipswitches are common ground: when open (upward) the lines are pulled to 5v, otherwise they read as 0
	PORT_START("SWITCHES")
		PORT_DIPNAME( 0x01, 0x00, "Power Frequency" )           PORT_DIPLOCATION("SW:!1")
		PORT_DIPSETTING( 0x00, "60Hz" )
		PORT_DIPSETTING( 0x01, "50Hz" )
		PORT_DIPNAME( 0x02, 0x00, "Default Serial Port" )           PORT_DIPLOCATION("SW:!2")
		PORT_DIPSETTING( 0x00, "EIA port" )
		PORT_DIPSETTING( 0x02, "20ma port" )
		PORT_DIPNAME( 0x04, 0x00, "Default US/UK" )         PORT_DIPLOCATION("SW:!3")
		PORT_DIPSETTING( 0x00, "US" )
		PORT_DIPSETTING( 0x04, "UK" )
		PORT_DIPNAME( 0x18, 0x00, "Default Parity" )            PORT_DIPLOCATION("SW:!4,!5")
		PORT_DIPSETTING( 0x00, "Off" )
		PORT_DIPSETTING( 0x10, "Even" )
		PORT_DIPSETTING( 0x08, "Odd" )
		PORT_DIPSETTING( 0x18, "Do Not Use This Setting" )
		PORT_DIPNAME( 0xe0, 0x00, "Default Baud Rate" )         PORT_DIPLOCATION("SW:!6,!7,!8")
		PORT_DIPSETTING( 0x00, "110" )
		PORT_DIPSETTING( 0x80, "300" )
		PORT_DIPSETTING( 0x40, "600" )
		PORT_DIPSETTING( 0xc0, "1200" )
		PORT_DIPSETTING( 0x20, "2400" )
		PORT_DIPSETTING( 0xa0, "4800" )
		PORT_DIPSETTING( 0x60, "9600" )
		PORT_DIPSETTING( 0xe0, "19200" )

	PORT_START("CAPSSHIFT") // CAPS LOCK and SHIFT appear as the high 2 bits on all rows
		PORT_BIT(0x3f, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_TOGGLE PORT_NAME("Caps lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_START("COL0")
		PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED) // row 0 bit 6 is always low, checked by keyboard test
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED) // all rows have these bits left low to save a mask op later
	PORT_START("COL1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Set Up") PORT_CODE(KEYCODE_F5)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num Enter") PORT_CODE(KEYCODE_ENTER_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF1/Hardcopy") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COL2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("No scroll") PORT_CODE(KEYCODE_LALT)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 1") PORT_CODE(KEYCODE_1_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF2/Locator") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COL3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 2") PORT_CODE(KEYCODE_2_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF3/Text") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COL4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 3") PORT_CODE(KEYCODE_3_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF4/Reset") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COL5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 4") PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COL6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 5") PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COL7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 6") PORT_CODE(KEYCODE_6_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COL8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 7") PORT_CODE(KEYCODE_7_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COL9")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 8") PORT_CODE(KEYCODE_8_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLA")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 9") PORT_CODE(KEYCODE_9_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLB")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 0") PORT_CODE(KEYCODE_0_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLC")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num -") PORT_CODE(KEYCODE_MINUS_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLD")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num ,") PORT_CODE(KEYCODE_PLUS_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLE")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("~") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line feed") PORT_CODE(KEYCODE_RALT)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num .") PORT_CODE(KEYCODE_DEL_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLF")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x18, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


void vk100_state::machine_start()
{
	output().set_value("online_led",1);
	output().set_value("local_led", 0);
	output().set_value("noscroll_led",1);
	output().set_value("basic_led", 1);
	output().set_value("hardcopy_led", 1);
	output().set_value("l1_led", 1);
	output().set_value("l2_led", 1);
	m_vsync = 0;
	m_dir_a6 = 1;
	m_cout = 0;
	m_vgX = 0;
	m_vgY = 0;
	m_vgERR = 0;
	m_vgSOPS = 0;
	m_vgPAT = 0;
	m_vgPAT_Mask = 0x80;
	m_vgPMUL = 0;
	m_vgPMUL_Count = 0;
	m_vgDownCount = 0;
	VG_DU = 0;
	VG_DVM = 0;
	VG_DIR = 0;
	VG_WOPS = 0;
	m_VG_MODE = 0;
	m_vgGO = 0;
	m_ACTS = 1;
	m_ADSR = 1;
	char kbdcol[8];
	// look up all 16 tags 'the slow way' but only once on reset
	for (int i = 0; i < 16; i++)
	{
		sprintf(kbdcol,"COL%X", i);
		m_col_array[i] = ioport(kbdcol);
	}
}

WRITE_LINE_MEMBER(vk100_state::crtc_vsync)
{
	m_maincpu->set_input_line(I8085_RST75_LINE, state? ASSERT_LINE : CLEAR_LINE);
	m_vsync = state;
}

WRITE_LINE_MEMBER(vk100_state::i8251_rxrdy_int)
{
	m_maincpu->set_input_line(I8085_RST65_LINE, state?ASSERT_LINE:CLEAR_LINE);
}

WRITE_LINE_MEMBER(vk100_state::i8251_txrdy_int)
{
	m_maincpu->set_input_line(I8085_RST55_LINE, state?ASSERT_LINE:CLEAR_LINE);
}

WRITE_LINE_MEMBER(vk100_state::i8251_rts)
{
	logerror("callback: RTS state changed to %d\n", state);
	// TODO: only change this during loopback mode!
	m_ACTS = state;
}

DRIVER_INIT_MEMBER(vk100_state,vk100)
{
	// figure out how the heck to initialize the timers here
	//m_i8251_rx_timer = timer_alloc(TID_I8251_RX);
	//m_i8251_tx_timer = timer_alloc(TID_I8251_TX);
	//m_i8251_sync_timer = timer_alloc(TID_SYNC);
	//machine().scheduler().timer_set(attotime::from_hz(10000), FUNC(i8251_rx_clk));
}

void vk100_state::video_start()
{
	m_vram = memregion("vram")->base();
	m_trans = memregion("trans")->base();
	m_pattern = memregion("pattern")->base();
	m_dir = memregion("dir")->base();
	m_sync = memregion("sync")->base();
	m_vector = memregion("vector")->base();
	m_ras_erase = memregion("ras_erase")->base();
}

MC6845_UPDATE_ROW( vk100_state::crtc_update_row )
{
	static const UINT32 colorTable[16] = {
	0x000000, 0x0000FF, 0xFF0000, 0xFF00FF, 0x00FF00, 0x00FFFF, 0xFFFF00, 0xFFFFFF,
	0x000000, 0x0000FF, 0xFF0000, 0xFF00FF, 0x00FF00, 0x00FFFF, 0xFFFF00, 0xFFFFFF };
	static const UINT32 colorTable2[16] = {
	0x000000, 0x0000FF, 0xFF0000, 0xFF00FF, 0x00FF00, 0x00FFFF, 0xFFFF00, 0xFFFFFF,
	0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000 };
	//printf("y=%d, ma=%04x, ra=%02x, x_count=%02x ", y, ma, ra, x_count);
	/* figure out ram address based on tech manual page 5-23:
	 * real address to 16-bit chunk a13  a12  a11 a10 a9  a8  a7  a6  a5  a4  a3  a2  a1  a0
	 * crtc input                  MA11 MA10 MA9 MA8 MA7 MA6 RA1 RA0 MA5 MA4 MA3 MA2 MA1 MA0
	 */
	UINT16 EA = ((ma&0xfc0)<<2)|((ra&0x3)<<6)|(ma&0x3F);
	// display the 64 different 12-bit-wide chunks
	for (int i = 0; i < 64; i++)
	{
		UINT16 block = m_vram[(EA<<1)+(2*i)+1] | (m_vram[(EA<<1)+(2*i)]<<8);
		UINT32 fgColor = (m_vgSOPS&0x08)?colorTable[(block&0xF000)>>12]:colorTable2[(block&0xF000)>>12];
		UINT32 bgColor = (m_vgSOPS&0x08)?colorTable[(m_vgSOPS&0xF0)>>4]:colorTable2[(m_vgSOPS&0xF0)>>4];
		// display a 12-bit wide chunk
		for (int j = 0; j < 12; j++)
		{
			bitmap.pix32(y, (12*i)+j) = (((block&(0x0001<<j))?1:0)^(m_vgSOPS&1))?fgColor:bgColor;
		}
	}
}


static MACHINE_CONFIG_START( vk100, vk100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, XTAL_5_0688MHz)
	MCFG_CPU_PROGRAM_MAP(vk100_mem)
	MCFG_CPU_IO_MAP(vk100_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_45_6192Mhz/3, 882, 0, 720, 370, 0, 350 ) // fake screen timings for startup until 6845 sets real ones
	MCFG_SCREEN_UPDATE_DEVICE( "crtc", mc6845_device, screen_update )

	MCFG_MC6845_ADD( "crtc", H46505, "screen", XTAL_45_6192Mhz/3/12)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(12)
	MCFG_MC6845_UPDATE_ROW_CB(vk100_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(vk100_state, crtc_vsync))

	/* i8251 uart */
	MCFG_DEVICE_ADD("i8251", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(vk100_state, i8251_rxrdy_int))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE(vk100_state, i8251_txrdy_int))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8251", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("i8251", i8251_device, write_dsr))

	MCFG_DEVICE_ADD(COM5016T_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(DEVWRITELINE("i8251", i8251_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE("i8251", i8251_device, write_txc))

	MCFG_DEFAULT_LAYOUT( layout_vk100 )

	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 116 ) // 116 hz (page 172 of TM), but duty cycle is wrong here!
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 0.25 )
MACHINE_CONFIG_END

/* ROM definition */
/* according to http://www.computer.museum.uq.edu.au/pdf/EK-VK100-TM-001%20VK100%20Technical%20Manual.pdf
page 5-10 (pdf pg 114), The 4 firmware roms should go from 0x0000-0x1fff,
0x2000-0x3fff, 0x4000-0x5fff and 0x6000-0x63ff; The last rom is actually a
little bit longer and goes to 67ff.
*/
ROM_START( vk100 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "23-031e4-00.rom1.e53", 0x0000, 0x2000, CRC(c8596398) SHA1(a8dc833dcdfb7550c030ac3d4143e266b1eab03a))
	ROM_LOAD( "23-017e4-00.rom2.e52", 0x2000, 0x2000, CRC(e857a01e) SHA1(914b2c51c43d0d181ffb74e3ea59d74e70ab0813))
	ROM_LOAD( "23-018e4-00.rom3.e51", 0x4000, 0x2000, CRC(b3e7903b) SHA1(8ad6ed25cd9b04a9968aa09ab69ba526d35ca550))
	ROM_LOAD( "23-190e2-00.rom4.e50", 0x6000, 0x1000, CRC(ad596fa5) SHA1(b30a24155640d32c1b47a3a16ea33cd8df2624f6)) // probably an overdump, e2 implies the size is 0x800 not 0x1000, and the end is all blank

	ROM_REGION( 0x8000, "vram", ROMREGION_ERASE00 ) // 32k of vram

	ROM_REGION( 0x400, "pattern", ROMREGION_ERASEFF )
	/* This is the "PATTERN ROM", (1k*4, 82s137)
	 * it contains a table of 4 output bits based on input of: (from figure 5-18):
	 * 4 input bits bank-selected from RAM (a0, a1, a2, a3)
	 * 2 select bits to choose one of the 4 bits, sourced from the X reg LSBs (a4, a5)
	 * 3 pattern function bits from WOPS (F0 "N" is a6, F1 and F2 are a7 and a8
	 * and one bit from the lsb of the pattern register shifter (a9)
	 * control bits:
	 *            /CE1 ----- GND [verified via tracing]
	 *            /CE2 ----- GND [verified via tracing]
	 * addr bits 9876543210
	 *           ||||||\\\\- input from ram (A)
	 *           ||||\\----- bit select (from x reg lsb)
	 *           |||\------- negate (N) \___ low 3 bits of WOPS
	 *           |\\-------- function   /
	 *           \---------- pattern bit (P)
	 * data bits: 3210
	 *            \\\\-- output to ram (M), but gated by an io line from vector rom
	 *    functions are:
	 *    Overlay: M=A|(P^N)
	 *    Replace: M=P^N
	 *    Complement: M=A^(P^N)
	 *    Erase: M=N
	 */
	ROM_LOAD( "wb8201_656f1.m1-7643-5.pr4.ic14", 0x0000, 0x0400, CRC(e8ecf59f) SHA1(49e9d109dad3d203d45471a3f4ca4985d556161f)) // label verified from nigwil's board

	ROM_REGION(0x100, "trans", ROMREGION_ERASEFF )
	/* this is the "TRANSLATOR ROM" described in figure 5-17 on page 5-27 (256*8, 82s135)
	 * it contains a table of 256 values which skips every fourth value so 00 01 02 04 05 06 08.. etc, wraps at the end
	 * control bits:
	 *            /CE1 ----- GND [verified via tracing]
	 *            /CE2 ----- GND [verified via tracing]
	 * addr bits: 76543210
	 *            \\\\\\\\- X9 thru X2
	 * data bits: 76543210
	 *            \\\\\\\\- X'9 thru X'2
	 * The VT125 prom @ E60 is literally identical to this, the same exact part: 23-060B1
	 */
	ROM_LOAD( "wb---0_060b1.mmi6309.pr2.ic82", 0x0000, 0x0100, CRC(198317fc) SHA1(00e97104952b3fbe03a4f18d800d608b837d10ae)) // label verified from nigwil's board

	ROM_REGION(0x100, "dir", ROMREGION_ERASEFF )
		/* this is the "DIRECTION ROM"  == mb6309 (256x8, 82s135)
		* see figure 5-24 on page 5-39
		* It tells the direction and enable for counting on the X and Y counters
		* and also handles the non-math related parts of the bresenham line algorithm
		* control bits:
		*            /CE1 ----- DCOUNT 0 H [verified via tracing]
		*            /CE2 ----- ENA ERROR L [verified via tracing]
		* addr bits: 76543210
		*            ||||\\\\-- DIR (vgDIR register low 4 bits)
		*            |||\------ C OUT aka ERROR CARRY (strobed in by STROBE L from the error counter's adder) [verified via tracing]
		*            ||\------- Y0 (the otherwise unused lsb of the Y register, used for bresenham) [verified via tracing]
		*            |\-------- feedback bit from d5 strobed by V CLK [verified via tracing]
		*            \--------- GND; the second half of the prom is blank (0x00)
		* data bits: 76543210
		*            |||||||\-- ENA Y (enables change on X counter) [works with code]
		*            ||||||\--- ENA X (enables change on Y counter) [works with code]
		*            |||||\---- Y DIRECTION (high is count down, low is count up)
		*            ||||\----- X DIRECTION (high is count down, low is count up)
		*            |||\------ PIXEL WRT
		*            ||\------- feedback bit to a6, this bit is held in PRESET/1 condition by GO being inactive, and if the vector prom is disabled it is pulled to 1 [verified via tracing and schematics]
		*            |\-------- UNUSED, always 0
		*            \--------- UNUSED, always 0
		* The VT125 prom @ E41 is literally identical to this, the same exact part: 23-059B1
		*/
	ROM_LOAD( "wb8141_059b1.tbp18s22.pr5.ic111", 0x0000, 0x0100, CRC(4b63857a) SHA1(3217247d983521f0b0499b5c4ef6b5de9844c465))  // label verified from andy's board

	ROM_REGION( 0x100, "ras_erase", ROMREGION_ERASEFF )
	/* this is the "RAS/ERASE ROM" involved with driving the RAS lines and erasing VRAM dram (256*4, 82s129)
	 * control bits:
	 *            /CE1 ----- /WRITE aka WRITE L (pin 6 of vector rom after being latched by its ls273) [verified via tracing and vt125 schematic]
	 *            /CE2 ----- /ENA WRITE aka ENA WRITE L [verified via tracing and vt125 schematic]
	 *                       (INHIBIT WRITE L (pin 5 of ls74 to extreme left edge of translate prom) XOR STROBE D COUNT (pin 5 of ls74 near the 20ma port) (pin 3 of the ls86 above the crtc)
	 * addr bits: 76543210
	 *            |||||||\-- X'2 [verified via tracing]
	 *            ||||||\--- X'3 [verified via tracing]
	 *            |||||\---- register file bit 3/upper file MSB (DIR prom pin 4, ls191 2nd from left edge left of the hd46505 pin 9, upper ls670n pin 6, ls283 at the left edge left of the hd46505 pin 15) [verified via tracing]
	 *            ||||\----- (Y8 NOR !(X10 NAND X9)) (pins 4,5,6 of ls32 left of 8085, and pins 1,2 of the ls04 in the lower left corner, pins 10,9,8 of the ls00 at the left edge drawn from the ls74 between the 8251 and 8202) [verified via tracing]
	 *            |||\------ (X10 NOR Y10) (pins 10,9,8 of ls32 left of 8085) [verified via tracing]
	 *            ||\------- X11 (D out of ls191 left of ls191 left of hd46505) [verified via tracing]
	 *            |\-------- Y11 (D out of ls191 left of hd46505) [verified via tracing]
	 *            \--------- ERASE L/d5 on the vector rom [verified via tracing]
	 * data bits: 3210
	 *            |||\-- /WE for VRAM Attribute bits
	 *            ||\--- /WE for VRAM bits 0-3 (leftmost bits, first to be shifted out)
	 *            |\---- /WE for VRAM bits 4-7
	 *            \----- /WE for VRAM bits 8-11 (rightmost bits, last to be shifted out)
	 * The VT125 prom E93 is mostly equivalent to the ras/erase prom; On the vt125 version, the inputs are:
	 *  (X'10 NOR X'11)
	 *  (Y9 NOR Y10)
	 *  Y11
	 *  X8 (aka PX8)
	 *  X9 (aka PX9)
	 *  ERASE L
	 *  X'3
	 *  X'2
	 * and the outputs are:
	 *  MWR 2
	 *  MWR 1
	 *  MWR 0
	 * since the vt125 has ram laid out differently than the vk100 and
	   only has 3 banks (but 2 planes of 3 banks), I (LN) assume there
	   are four wr banks on the v100, one per nybble, and the X'3 and
	   X'2 inputs lend credence to this.
	 *
	 */
	ROM_LOAD( "wb8151_573a2.mmi6301.pr3.ic41", 0x0000, 0x0100, CRC(75885a9f) SHA1(c721dad6a69c291dd86dad102ed3a8ddd620ecc4)) // Stamp/silkscreen: "WB8151 // 573A2" (23-573A2), 82S129 equivalent @ E41
	// label verified from nigwil's and andy's board

	ROM_REGION( 0x100, "vector", ROMREGION_ERASEFF )
	// WARNING: it is possible that the first two bytes of this prom are bad!
	// The PROM on andy's board appears to be damaged, this will need to be redumped from another board.
	/* this is the "VECTOR ROM" (256*8, 82s135) which runs the vector generator state machine
	 * the vector rom bits are complex and are unfortunately poorly documented
	 * in the tech manual. see figure 5-23.
	 * control bits:
	 *            /CE1 ----- /GO aka GO L [verified via tracing]
	 *            /CE2 ----- GND [verified via tracing]
	 * addr bits: 76543210
	 *            ||||\\\\-- To sync counter, which counts 0xC 0x3 0x2 0x1 0x0 0x5 0x4 0xB 0xA 0x9 0x8 0xD in that order
	 *            |||\------ (MODE 0 NAND GO) \_This effectively means when GO is low, these bits are both 1, when GO is high, these bits are the inverse of the MODE bits; this is DIFFERENT from the vt125, and was probably removed as unnecessary given that /GO is an enable for the vector rom anyway [verified via tracing]
	 *            ||\------- (MODE 1 NAND GO) /
	 *            |\-------- ? C OUT (when set, only one /LD ERROR pulse occurs instead of two)
	 *            \--------- ? FINISH (/LD ERROR only goes active (low) when this is unset)
	 *
	 * data bits: 76543210
	 *            |||||||\-- /WRITE aka WRITE L (fig 5-20, page 5-32, writes the post-pattern-converted value back to vram at X,Y)
	 *            ||||||\--- DONE L [verified via tracing]
	 *            |||||\---- VECTOR CLK aka V CLK [verified via tracing]
	 *            ||||\----- /LD ERROR aka STROBE ERROR L (strobes the adder result value into the vgERR register)
	 *            |||\------ D LOAD [by process of elimination and limited tracing]
	 *            ||\------- ERASE L (latched, forces a4 on the sync rom low and also forces a7 on the ras/erase rom; the counter rom may be involved in blanking all of vram) [verified via tracing]
	 *            |\-------- C0 aka C IN (high during DVM read, low otherwise, a carry in to the adder so DVM is converted from 1s to 2s complement)
	 *            \--------- SHIFT ENA [verified via tracing]
	 * The data bits all have pull-ups to +5v if the /CE1 pin is not low
	 * According to the VT125 tech manual (vt100 tech manual rev 3, page 6-85) the 8 signals here are:
	 * ERASE L - d5
	 * SHIFT ENA - d7
	 * C IN - d6
	 * D LOAD - d4
	 * WRITE L - d0
	 * DONE L - d1
	 * VECTOR CLK - d2
	 * STROBE ERROR L - d3
	 *
	 * The VT125 prom E71 and its latch E70 is mostly equivalent to the vector prom, but the address order is different
	 */
	ROM_LOAD( "wb8146_058b1.mmi6309.pr1.ic99", 0x0000, 0x0100, BAD_DUMP CRC(71b01864) SHA1(e552f5b0bc3f443299282b1da7e9dbfec60e12bf))  // label verified from nigwil's and andy's board

	ROM_REGION( 0x20, "sync", ROMREGION_ERASEFF )
	/* this is the "SYNC ROM" == mb6331 (32x8, 82s123)
	 * It generates the ram RAS/CAS and a few other signals, see figure 5-20 on page 5-32
	 * The exact pins for each signal are not documented.
	 * control bits:
	 *            /CE1 -- GND(Unused)
	 * addr bits: 43210
	 *            |\\\\-- To sync counter, which counts 0xC 0x3 0x2 0x1 0x0 0x5 0x4 0xB 0xA 0x9 0x8 0xD in that order
	 *            \------ comes from the gated ERASE L/d5 from the vector rom, only low during VG_MODE == ER (ERase Screen) [verified via tracing]
	 *                      when high: the sync rom matches figure 5-20 (page 5-32) and 5-23 (page 5-38)
	 *                      when low: RA/RB is fixed on WOPS in the register file
	 *                                LD SHFR does NOT output pulses (effectively blanking the screen)
	 *                                WRT/RD is held at /RD so vg to vram do nothing,
	 *                                however while the crtc is refreshing the screen it instead is writing zeroes to every location
	 * data bits: 76543210
	 *            |||||||\-- WRT/RD (write high when x,y (vg) drives vram, read low when ma (crtc) drives vram)
	 *            ||||||\--- /RAS (for vram)
	 *            |||||\---- STROBE aka STROBE L aka VG STROBE(latches the carry bit from the adder to the direction rom AND causes the data at the X,Y address of vram to be read into the holding register before pattern is applied)
	 *            ||||\----- LD SHFR (loads the 12-bit shift register of data to be shifted to screen)
	 *            |||\------ /CAS (for vram)
	 *            ||\------- RA\__selects which slot of the 8x4 register file (du, dvm, dir, or wops) is selected
	 *            |\-------- RB/
	 *            \--------- SYNC (latches the EXECUTE signal from an EXEC * write to activate the GO signal and enable the Vector rom) [verified via tracing]
	 * The VT125 proms E64/E66 and their respective latches E65 and E83 are mostly equivalent to the sync rom
	 */
	ROM_LOAD( "wb8014_297a1.74s288.pr6.ic89", 0x0000, 0x0020, CRC(e2f7c566) SHA1(a4c3dc5d07667141ad799168a862cb3c489b4934)) // label verified from nigwil's and andy's board
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                       FULLNAME       FLAGS */
COMP( 1980, vk100,  0,      0,       vk100,     vk100, vk100_state,   vk100,  "Digital Equipment Corporation", "VK100 'GIGI'", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
