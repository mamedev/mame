/*****************************************************************************
 *   DS5002FP emulator by Manuel Abadia
 *
 *   The emulator is just a modified version of the MCS-51 Family Emulator by Steve Ellenoff
 *
 *   What has been added?
 *      - Extra SFRs
 *      - Bytewide Bus Support
 *      - Memory Partition and Memory Range
 *      - Bootstrap Configuration
 *      - Power Fail Interrupt
 *      - Timed Access
 *      - Stop Mode
 *      - Idle Mode
 *
 *   What is not implemented?
 *      - Peripherals and Reprogrammable Peripheral Controller
 *      - CRC-16
 *      - Watchdog timer
 *
 *   The main features of the DS5002FP are:
 *      - 100% code-compatible with 8051
 *      - Directly addresses 64kB program/64kB data memory
 *      - Nonvolatile memory control circuitry
 *      - 10-year data retention in the absence of power
 *      - In-system reprogramming via serial port
 *      - Dedicated memory bus, preserving four 8-bit ports for general purpose I/O
 *      - Power-fail reset
 *      - Early warning power-fail interrupt
 *      - Watchdog timer
 *      - Accesses up to 128kB on the bytewide bus
 *      - Decodes memory for 32kB x 8 or 128kB x 8 SRAMs
 *      - Four additional decoded peripheral-chip enables
 *      - CRC hardware for checking memory validity
 *      - Optionally emulates an 8042-style slave interface
 *      - Memory encryption using an 80-bit encryption key
 *      - Automatic random generation of encryption keys
 *      - Self-destruct input for tamper protection
 *      - Optional top-coating prevents microprobe
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *   i8051.c
 *   Portable MCS-51 Family Emulator
 *
 *   Chips in the family:
 *   8051 Product Line (8031,8051,8751)
 *   8052 Product Line (8032,8052,8752)
 *   8054 Product Line (8054)
 *   8058 Product Line (8058)
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     sellenoff@hotmail.com
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on:
 *  #1) 'Intel(tm) MC51 Microcontroller Family Users Manual' and
 *  #2) 8051 simulator by Travis Marlatte
 *  #3) Portable UPI-41/8041/8741/8042/8742 emulator V0.1 by Juergen Buchmueller (MAME CORE)
 *
 *****************************************************************************/

/******************************************************************************
 *  Notes:
 *        *Important*: Internal ROM needs to be treated the same as external rom by the programmer
 *                     creating the driver (ie, use standard cpu rom region)
 *
 *        The term cycles is used here to really refer to clock oscilations, because 1 machine cycle
 *        actually takes 12 oscilations.
 *
 *        Read/Write/Modify Instruction -
 *          Data is read from the Port Latch (not the Port Pin!), possibly modified, and
 *          written back to (the pin? and) the latch!
 *
 *          The following all perform this on a port address..
 *          (anl, orl, xrl, jbc, cpl, inc, dec, djnz, mov px.y,c, clr px.y, setb px.y)
 *
 *        Serial UART emulation is not really accurate, but faked enough to work as far as i can tell
 *
 *        August 27,2003: Currently support for only 8031/8051/8751 chips (ie 128 RAM)
 *        October 14,2003: Added initial support for the 8752 (ie 256 RAM)
 *        October 22,2003: Full support for the 8752 (ie 256 RAM)
 *        July 28,2004: Fixed MOVX command and added External Ram Paging Support
 *        July 31,2004: Added Serial Mode 0 Support & Fixed Interrupt Flags for Serial Port
 *
 *        Todo: Full Timer support (all modes)
 *
 *        NOW Implemented: RAM paging using hardware configured addressing...
 *        (July 28,2004)   the "MOVX a,@R0/R1" and "MOVX @R0/R1,a" commands can use any of the other ports
 *                 to output a page offset into external ram, but it is totally based on the hardware setup.
 *
 *        Timing needs to be implemented via MAME timers perhaps?
 *
 *****************************************************************************/

#include "debugger.h"
#include "deprecat.h"
#include "ds5002fp.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)


//Prototypes
INLINE void push_pc(void);
INLINE void pop_pc(void);
INLINE void set_parity(void);
INLINE void do_add_flags(UINT8 a, UINT8 data, UINT8 c);
INLINE void do_sub_flags(UINT8 a, UINT8 data, UINT8 c);
INLINE UINT8 check_interrupts(void);
INLINE void update_timer(int cyc);
INLINE void	update_serial(int cyc);
INLINE void serial_transmit(UINT8 data);
static READ8_HANDLER(internal_ram_read);
static WRITE8_HANDLER(internal_ram_write);
static READ8_HANDLER(internal_ram_iread);
static READ32_HANDLER(external_ram_iaddr);
static WRITE8_HANDLER(internal_ram_iwrite);
static READ8_HANDLER(sfr_read);
static WRITE8_HANDLER(sfr_write);
static WRITE8_HANDLER( bit_address_w );
static READ8_HANDLER( bit_address_r );

typedef struct {
	UINT8	timerbaud;		//Flag set if timer overflow is controlling baud
	UINT8	sending;		//Flag set when uart is sending
	UINT8	data_out;		//Data to send out
	UINT8	bits_to_send;	//How many bits left to send when transmitting out the serial port
	UINT8	bitcycles;		//# of bitcycles passed since last bit was sent
} DS5002FP_UART;

typedef struct _DS5002FP {

	//Internal stuff
	UINT16	ppc;			//previous pc
	UINT16	pc;				//current pc
	UINT16	subtype;		//specific version of the cpu, ie 8031, or 8051 for example
	UINT8	cur_irq;		//Holds value of any current IRQ being serviced
	UINT8	irq_priority;	//Holds value of the current IRQ Priority Level
	UINT8	rwm;			//Signals that the current instruction is a read/write/modify instruction
	UINT8   prev_used_cycles;	//Track previous # of used cycles
	UINT8   last_int0;			//Store state of int0
	UINT8   last_int1;			//Store state of int1
	UINT8   last_power_int;		//Store state of last power interrupt
	UINT8   int_vec;			//Pending Interrupt Vector
	UINT8   priority_request; 	//Priority level of incoming new irq
	UINT8	previous_ta;		//Previous Timed Access value
	INT8	ta_window;			//Timed Access window
	UINT8	range;				//Memory Range
	const ds5002fp_config *config;	//Bootstrap Configuration
	//SFR Registers         (Note: Appear in order as they do in memory)
	UINT8	po;				//Port 0
	UINT8	sp;				//Stack Pointer
	UINT8	dpl;			//Data Pointer (DPTR) (Hi bit)
	UINT8	dph;			//Data Pointer (DPTR) (Lo bit)
	UINT8	pcon;			//Power Mode Control
	UINT8	tcon;			//Timer/Counter Control
	UINT8	tmod;			//Timer/Counter Mode Control
	UINT8	tl0;			//Timer 0 Lo
	UINT8	tl1;			//Timer 1 Lo
	UINT8	th0;			//Timer 0 Hi
	UINT8	th1;			//Timer 1 Hi
	UINT8	p1;				//Port 1
	UINT8	scon;			//Serial Control
	UINT8	sbuf;			//Serial Data Buffer
	UINT8	p2;				//Port 2
	UINT8	ie;				//Interrupt Enable
	UINT8	p3;				//Port 3
	UINT8	ip;				//Interrupt Priority
	UINT8	crc;			//CRC
	UINT8	crcl;			//CRC Result (low nibble)
	UINT8	crch;			//CRC Result (high nibble)
	UINT8	mcon;			//Memory Control
	UINT8	ta;				//Timed Access
	UINT8	rnr;			//Random Number Generator
	UINT8	psw;			//Program Status Word
	UINT8	rpctl;			//RPC control
	UINT8	rps;			//RPC status
	UINT8	acc;			//Accumulator
	UINT8	b;				//Register B

	//Internal Ram
	UINT8	IntRam[0xff+1];	//Max 256 Bytes of Internal RAM (8031/51 have 128, 8032/52 have 256)

	//Interrupt Callback
	int 	(*irq_callback)(int irqline);

	//Serial Port TX/RX Call backs
	void    (*serial_tx_callback)(int data);	//Call back funciton when sending data out of serial port
	int		(*serial_rx_callback)(void);		//Call back function to retrieve data when receiving serial port data

	//Internal Indirect Read/Write Handlers
	READ8_HANDLER((*iram_iread));
	WRITE8_HANDLER((*iram_iwrite));

	//External/Bytewide Ram Address Callback for generating the hardware specific access to external ram
	READ32_HANDLER((*ebram_iaddr_callback));

} DS5002FP;

static int ds5002fp_icount;

static DS5002FP ds5002fp;
static DS5002FP_UART uart;

//Hold callback functions so they can be set by caller (before the cpu reset)
static void (*hold_serial_tx_callback)(int data);
static int (*hold_serial_rx_callback)(void);
static READ32_HANDLER((*hold_ebram_iaddr_callback));

/*Short cuts*/

/***************************************************************
 * Read Opcode/Opcode Arguments from Program Code
 ***************************************************************/
#define ROP(pc)			cpu_readop(pc)
#define ROP_ARG(pc)		cpu_readop_arg(pc)
/*****************************************************************************
 * Read a byte from External Code Memory (Usually Program Rom(s) Space)
 *****************************************************************************
 This area is mapped from 0-FFFF internally (64K) */
#define CODEMEM_R(a)	(UINT8)program_read_byte_8le(a)
/*****************************************************************************
 * Read/Write a byte from/to Expanded/Byte-wide Data Memory (Usually RAM or other I/O)
 *****************************************************************************
 This area is mapped from 0-FFFF internally (64K) */
#define DATAMEM_R(a)	(UINT8)data_read_byte_8le(a)
#define DATAMEM_W(a,v)	data_write_byte_8le(a,v)

/***************************************************************
 * Read/Write a byte from/to the Internal RAM
 ***************************************************************/
#define IRAM_R(a)		internal_ram_read(Machine,a)
#define IRAM_W(a,v)		internal_ram_write(Machine,a,v)
/***************************************************************
 * Read/Write a byte from/to the Internal RAM indirectly (called from indirect addressing)
 ***************************************************************/
#define IRAM_IR(a)		internal_ram_iread(Machine,a)
#define IRAM_IW(a,v)	internal_ram_iwrite(Machine,a,v)
/***************************************************************
 * Form an Address to Read/Write to External RAM indirectly (called from indirect addressing)
 ***************************************************************/
#define ERAM_ADDR(a,m)	external_ram_iaddr(Machine,a,m)
/***************************************************************
 * Read/Write a byte from/to the SFR Registers
 ***************************************************************/
#define SFR_R(a)		sfr_read(Machine,a)
#define SFR_W(a,v)		sfr_write(Machine,a,v)
/***************************************************************
 * Read/Write a bit from Bit Addressable Memory
 ***************************************************************/
#define BIT_R(a)		bit_address_r(Machine,a)
#define BIT_W(a,v)		bit_address_w(Machine,a,v)
/***************************************************************
 * Input/Output a byte from given I/O port
 ***************************************************************/
#define IN(port)		((UINT8)io_read_byte(port))
#define OUT(port,value) io_write_byte(port,value)
/***************************************************************
 * Access the 4 banks of R registers (R0...R7)
 ***************************************************************/
#define R_R(n)			ds5002fp.IntRam[(GET_RS*8)+(n)]
/***************************************************************
 * Easy macro for working with 16 bit DPTR
 ***************************************************************/
#define R_DPTR			((R_DPH<<8) | R_DPL)
#define DPTR_W(n)		SFR_W(DPH, ((n)>>8)&0xff);\
						SFR_W(DPL, ((n)&0xff));
/***************************************************************
 * Easy macros for Setting Flags
 ***************************************************************/
/*PSW Flags*/
#define SET_CY(n)		R_PSW = (R_PSW & 0x7f) | (n<<7);	//Carry Flag
#define SET_AC(n)		R_PSW = (R_PSW & 0xbf) | (n<<6);	//Aux.Carry Flag
#define SET_FO(n)		R_PSW = (R_PSW & 0xdf) | (n<<5);	//User Flag
#define SET_RS(n)		R_PSW = (R_PSW & 0xe7) | (n<<3);	//R Bank Select
#define SET_OV(n)		R_PSW = (R_PSW & 0xfb) | (n<<2);	//Overflow Flag
#define SET_P(n)		R_PSW = (R_PSW & 0xfe) | (n<<0);	//Parity Flag
/*IE Flags*/
#define SET_EA(n)		R_IE = (R_IE & 0x7f) | (n<<7);		//Global Interrupt Enable/Disable
#define SET_ES(n)		R_IE = (R_IE & 0xef) | (n<<4);		//Serial Interrupt Enable/Disable
#define SET_ET1(n)		R_IE = (R_IE & 0xf7) | (n<<3);		//Timer 1 Interrupt Enable/Disable
#define SET_EX1(n)		R_IE = (R_IE & 0xfb) | (n<<2);		//External Int 1 Interrupt Enable/Disable
#define SET_ET0(n)		R_IE = (R_IE & 0xfd) | (n<<1);		//Timer 0 Interrupt Enable/Disable
#define SET_EX0(n)		R_IE = (R_IE & 0xfe) | (n<<0);		//External Int 0 Interrupt Enable/Disable
/*IP Flags*/
#define SET_PS0(n)		R_IP = (R_IP & 0xef) | (n<<4);		//Set Serial Priority Level
#define SET_PT1(n)		R_IP = (R_IP & 0xf7) | (n<<3);		//Set Timer 1 Priority Level
#define SET_PX1(n)		R_IP = (R_IP & 0xfb) | (n<<2);		//Set External Int 1 Priority Level
#define SET_PT0(n)		R_IP = (R_IP & 0xfd) | (n<<1);		//Set Timer 0 Priority Level
#define SET_PX0(n)		R_IP = (R_IP & 0xfe) | (n<<0);		//Set External Int 0 Priority Level
/*TCON Flags*/
#define SET_TF1(n)		R_TCON = (R_TCON & 0x7f) | (n<<7);	//Indicated Timer 1 Overflow Int Triggered
#define SET_TR1(n)		R_TCON = (R_TCON & 0xbf) | (n<<6);  //IndicateS Timer 1 is running
#define SET_TF0(n)		R_TCON = (R_TCON & 0xdf) | (n<<5);	//Indicated Timer 0 Overflow Int Triggered
#define SET_TR0(n)		R_TCON = (R_TCON & 0xef) | (n<<4);  //IndicateS Timer 0 is running
#define SET_IE1(n)		R_TCON = (R_TCON & 0xf7) | (n<<3);  //Indicated External Int 1 Triggered
#define SET_IT1(n)		R_TCON = (R_TCON & 0xfb) | (n<<2);  //Indicates how External Int 1 is Triggered
#define SET_IE0(n)		R_TCON = (R_TCON & 0xfd) | (n<<1);  //Indicated External Int 0 Triggered
#define SET_IT0(n)		R_TCON = (R_TCON & 0xfe) | (n<<0);  //Indicates how External Int 0 is Triggered
/*SCON Flags*/
#define SET_SM0(n)		R_SCON = (R_SCON & 0x7f) | (n<<7);	//Sets Serial Port Mode
#define SET_SM1(n)		R_SCON = (R_SCON & 0xbf) | (n<<6);  //Sets Serial Port Mode
#define SET_SM2(n)		R_SCON = (R_SCON & 0xdf) | (n<<5);	//Sets Serial Port Mode (Multiprocesser mode)
#define SET_REN(n)		R_SCON = (R_SCON & 0xef) | (n<<4);  //Sets Serial Port Receive Enable
#define SET_TB8(n)		R_SCON = (R_SCON & 0xf7) | (n<<3);  //Transmit 8th Bit
#define SET_RB8(n)		R_SCON = (R_SCON & 0xfb) | (n<<2);  //Receive 8th Bit
#define SET_TI(n)		R_SCON = (R_SCON & 0xfd) | (n<<1);  //Indicates Transmit Interrupt Occurred
#define SET_RI(n)		R_SCON = (R_SCON & 0xfe) | (n<<0);  //Indicates Receive Interrupt Occurred
/*TMOD Flags*/
#define SET_GATE1(n)	R_TMOD = (R_TMOD & 0x7f) | (n<<7);	//Timer 1 Gate Mode
#define SET_CT1(n)		R_TMOD = (R_TMOD & 0xbf) | (n<<6);  //Timer 1 Counter Mode
#define SET_M1_1(n)		R_TMOD = (R_TMOD & 0xdf) | (n<<5);	//Timer 1 Timer Mode Bit 1
#define SET_M1_0(n)		R_TMOD = (R_TMOD & 0xef) | (n<<4);  //Timer 1 Timer Mode Bit 0
#define SET_GATE0(n)	R_TMOD = (R_TMOD & 0xf7) | (n<<3);  //Timer 0 Gate Mode
#define SET_CT0(n)		R_TMOD = (R_TMOD & 0xfb) | (n<<2);  //Timer 0 Counter Mode
#define SET_M0_1(n)		R_TMOD = (R_TMOD & 0xfd) | (n<<1);  //Timer 0 Timer Mode Bit 1
#define SET_M0_0(n)		R_TMOD = (R_TMOD & 0xfe) | (n<<0);  //Timer 0 Timer Mode Bit 0
/*PCON Flags*/
#define SET_POR(n)		R_PCON = (R_PCON & 0xbf) | (n<<6);  //Power On Reset
#define SET_PFW(n)		R_PCON = (R_PCON & 0xdf) | (n<<5);	//Power Fail Warning
#define SET_WTR(n)		R_PCON = (R_PCON & 0xef) | (n<<4);  //Watchdog Timer Reset
#define SET_EPFW(n)		R_PCON = (R_PCON & 0xf7) | (n<<3);  //Enable Power Fail Interrupt
#define SET_EWT(n)		R_PCON = (R_PCON & 0xfb) | (n<<2);  //Enable Watchdog Timer
#define SET_STOP(n)		R_PCON = (R_PCON & 0xfd) | (n<<1);  //Stop
#define SET_IDLE(n)		R_PCON = (R_PCON & 0xfe) | (n<<0);  //Idle

/***************************************************************
 * Easy macros for Getting Flags
 ***************************************************************/
/*PSW Flags*/
#define GET_CY			((R_PSW & 0x80)>>7)
#define GET_AC			((R_PSW & 0x40)>>6)
#define GET_FO			((R_PSW & 0x20)>>5)
#define GET_RS			((R_PSW & 0x18)>>3)
#define GET_OV			((R_PSW & 0x04)>>2)
#define GET_P			((R_PSW & 0x01)>>0)
/*IE Flags*/
#define GET_EA			((R_IE & 0x80)>>7)
#define GET_ET2			((R_IE & 0x20)>>5)
#define GET_ES			((R_IE & 0x10)>>4)
#define GET_ET1			((R_IE & 0x08)>>3)
#define GET_EX1			((R_IE & 0x04)>>2)
#define GET_ET0			((R_IE & 0x02)>>1)
#define GET_EX0			((R_IE & 0x01)>>0)
#define GET_PS			((R_IP & 0x10)>>4)
#define GET_PT1			((R_IP & 0x08)>>3)
#define GET_PX1			((R_IP & 0x04)>>2)
#define GET_PT0			((R_IP & 0x02)>>1)
#define GET_PX0			((R_IP & 0x01)>>0)
/*TCON Flags*/
#define GET_TF1			((R_TCON & 0x80)>>7)
#define GET_TR1			((R_TCON & 0x40)>>6)
#define GET_TF0			((R_TCON & 0x20)>>5)
#define GET_TR0			((R_TCON & 0x10)>>4)
#define GET_IE1			((R_TCON & 0x08)>>3)
#define GET_IT1			((R_TCON & 0x04)>>2)
#define GET_IE0			((R_TCON & 0x02)>>1)
#define GET_IT0			((R_TCON & 0x01)>>0)
/*SCON Flags*/
#define GET_SM0			((R_SCON & 0x80)>>7)
#define GET_SM1			((R_SCON & 0x40)>>6)
#define GET_SM2			((R_SCON & 0x20)>>5)
#define GET_REN			((R_SCON & 0x10)>>4)
#define GET_TB8			((R_SCON & 0x08)>>3)
#define GET_RB8			((R_SCON & 0x04)>>2)
#define GET_TI			((R_SCON & 0x02)>>1)
#define GET_RI			((R_SCON & 0x01)>>0)
/*TMOD Flags*/
#define GET_GATE1		((R_TMOD & 0x80)>>7)
#define GET_CT1			((R_TMOD & 0x40)>>6)
#define GET_M1_1		((R_TMOD & 0x20)>>5)
#define GET_M1_0		((R_TMOD & 0x10)>>4)
#define GET_GATE0		((R_TMOD & 0x08)>>3)
#define GET_CT0			((R_TMOD & 0x04)>>2)
#define GET_M0_1		((R_TMOD & 0x02)>>1)
#define GET_M0_0		((R_TMOD & 0x01)>>0)
/*PCON Flags*/
#define GET_SMOD		((R_PCON & 0x80)>>7)
#define GET_POR			((R_PCON & 0x40)>>6)
#define GET_PFW			((R_PCON & 0x20)>>5)
#define GET_WTR			((R_PCON & 0x10)>>4)
#define GET_EPFW		((R_PCON & 0x08)>>3)
#define GET_EWT			((R_PCON & 0x04)>>2)
#define GET_STOP		((R_PCON & 0x02)>>1)
#define GET_IDLE		((R_PCON & 0x01)>>0)
/*MCON Flags*/
#define GET_PA			((R_MCON & 0xf0)>>4)
#define GET_RG1			((R_MCON & 0x08)>>3)
#define GET_PES			((R_MCON & 0x04)>>2)
#define GET_PM			((R_MCON & 0x02)>>1)
#define GET_SL			((R_MCON & 0x01)>>0)
/*RPCTL Flags*/
#define GET_RNR			((R_RPCTL & 0x80)>>7)
#define GET_EXBS		((R_RPCTL & 0x20)>>5)
#define GET_AE			((R_RPCTL & 0x10)>>4)
#define GET_IBI			((R_RPCTL & 0x08)>>3)
#define GET_DMA			((R_RPCTL & 0x04)>>2)
#define GET_RPCON		((R_RPCTL & 0x02)>>1)
#define GET_RG0			((R_RPCTL & 0x01)>>0)

/*Add and Subtract Flag settings*/
#define DO_ADD_FLAGS(a,d,c)	do_add_flags(a,d,c);
#define DO_SUB_FLAGS(a,d,c)	do_sub_flags(a,d,c);

#define SET_PARITY	set_parity();
#define PUSH_PC		push_pc();
#define POP_PC		pop_pc();

/* PC vectors */
#define V_RESET 0x000	/* power on address */
#define V_IE0	0x003	/* External Interrupt 0 */
#define V_TF0	0x00b	/* Timer 0 Overflow */
#define V_IE1	0x013	/* External Interrupt 1 */
#define V_TF1	0x01b	/* Timer 1 Overflow */
#define V_RITI	0x023	/* Serial Receive/Transmit */
#define V_PFI	0x02b	/* Power Failure Interrupt */

/* Any pending IRQ */
#define SERIALPORT_IRQ    ((R_SCON & 0x03) && GET_ES)

/* Clear Current IRQ  */
#define CLEAR_CURRENT_IRQ ds5002fp.cur_irq = 0xff;\
						  ds5002fp.irq_priority = 0;

/* shorter names for the DS5002FP structure elements */

//Internal stuff
#define PPC 	ds5002fp.ppc
#define PC		ds5002fp.pc
#define TYPE	ds5002fp.subtype
#define RWM		ds5002fp.rwm

//SFR Registers
#define R_P0	ds5002fp.po
#define R_SP	ds5002fp.sp
#define R_DPL	ds5002fp.dpl
#define R_DPH	ds5002fp.dph
#define R_PCON	ds5002fp.pcon
#define R_TCON	ds5002fp.tcon
#define R_TMOD	ds5002fp.tmod
#define R_TL0	ds5002fp.tl0
#define R_TL1	ds5002fp.tl1
#define R_TH0	ds5002fp.th0
#define R_TH1	ds5002fp.th1
#define R_P1	ds5002fp.p1
#define R_SCON	ds5002fp.scon
#define R_SBUF	ds5002fp.sbuf
#define R_P2	ds5002fp.p2
#define R_IE	ds5002fp.ie
#define R_P3	ds5002fp.p3
#define R_IP	ds5002fp.ip
#define R_CRC	ds5002fp.crc
#define R_CRCL	ds5002fp.crcl
#define R_CRCH	ds5002fp.crch
#define R_MCON	ds5002fp.mcon
#define R_TA	ds5002fp.ta
#define R_RNR	ds5002fp.rnr
#define R_PSW	ds5002fp.psw
#define R_RPCTL	ds5002fp.rpctl
#define R_RPS	ds5002fp.rps
#define R_ACC	ds5002fp.acc
#define R_B		ds5002fp.b

/* # of oscilations each opcode requires*/
static const UINT8 ds5002fp_cycles[] = {
	12,24,24,12,12,12,12,12,12,12,12,12,12,12,12,12,
	24,24,24,12,12,12,12,12,12,12,12,12,12,12,12,12,
	24,24,24,12,12,12,12,12,12,12,12,12,12,12,12,12,
	24,24,24,12,12,12,12,12,12,12,12,12,12,12,12,12,
	24,24,12,24,12,12,12,12,12,12,12,12,12,12,12,12,
	24,24,12,24,12,12,12,12,12,12,12,12,12,12,12,12,
	24,24,12,24,12,12,12,12,12,12,12,12,12,12,12,12,
	24,24,24,24,12,24,12,12,12,12,12,12,12,12,12,12,
	24,24,24,24,48,24,24,24,24,24,24,24,24,24,24,24,
	24,24,24,24,12,12,12,12,12,12,12,12,12,12,12,12,
	24,24,12,24,48,12,24,24,24,24,24,24,24,24,24,24,
	24,24,12,12,24,24,24,24,24,24,24,24,24,24,24,24,
	24,24,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	24,24,12,12,12,24,12,12,24,24,24,24,24,24,24,24,
	24,24,24,24,12,12,12,12,12,12,12,12,12,12,12,12,
	24,24,24,24,12,12,12,12,12,12,12,12,12,12,12,12
};

/* default configuration */
static const ds5002fp_config default_config =
{
	0x00,
	0x00,
	0x00
};

/* Memory Range (RG1 and RG0 @ MCON and RPCTL registers) */
static const UINT16 ds5002fp_ranges[4] = {
	0x1fff,
	0x3fff,
	0x7fff,
	0xffff
};

/* Memory Partition Table (RG1 & RG0 @ MCON & RPCTL registers) */
static const UINT32 ds5002fp_partitions[16] = {
	0x0000,
	0x1000,
	0x2000,
	0x3000,
	0x4000,
	0x5000,
	0x6000,
	0x7000,
	0x8000,
	0x9000,
	0xa000,
	0xb000,
	0xc000,
	0xd000,
	0xe000,
	0x10000
};

/* Include Opcode functions */
#include "ds5002fpops.c"

void ds5002fp_init(int index, int clock, const void *_config, int (*irqcallback)(int))
{
	const ds5002fp_config *config = _config ? _config : &default_config;

	ds5002fp.config = config;
	ds5002fp_set_irq_callback(irqcallback);

	//Internal stuff
	state_save_register_item("ds5002fp", index, ds5002fp.ppc);
	state_save_register_item("ds5002fp", index, ds5002fp.pc);
	state_save_register_item("ds5002fp", index, ds5002fp.subtype);
	state_save_register_item("ds5002fp", index, ds5002fp.rwm );
	state_save_register_item("ds5002fp", index, ds5002fp.cur_irq );
	state_save_register_item("ds5002fp", index, ds5002fp.irq_priority );
	state_save_register_item("ds5002fp", index, ds5002fp.prev_used_cycles );
	state_save_register_item("ds5002fp", index, ds5002fp.last_int0 );
	state_save_register_item("ds5002fp", index, ds5002fp.last_int1 );
	state_save_register_item("ds5002fp", index, ds5002fp.last_power_int );
	state_save_register_item("ds5002fp", index, ds5002fp.int_vec );
	state_save_register_item("ds5002fp", index, ds5002fp.priority_request );
	state_save_register_item("ds5002fp", index, ds5002fp.previous_ta );
	state_save_register_item("ds5002fp", index, ds5002fp.ta_window );
	state_save_register_item("ds5002fp", index, ds5002fp.range );
	//SFR Registers
	state_save_register_item("ds5002fp", index, ds5002fp.po);
	state_save_register_item("ds5002fp", index, ds5002fp.sp);
	state_save_register_item("ds5002fp", index, ds5002fp.dpl);
	state_save_register_item("ds5002fp", index, ds5002fp.dph);
	state_save_register_item("ds5002fp", index, ds5002fp.pcon);
	state_save_register_item("ds5002fp", index, ds5002fp.tcon);
	state_save_register_item("ds5002fp", index, ds5002fp.tmod);
	state_save_register_item("ds5002fp", index, ds5002fp.tl0);
	state_save_register_item("ds5002fp", index, ds5002fp.tl1);
	state_save_register_item("ds5002fp", index, ds5002fp.th0);
	state_save_register_item("ds5002fp", index, ds5002fp.th1);
	state_save_register_item("ds5002fp", index, ds5002fp.p1);
	state_save_register_item("ds5002fp", index, ds5002fp.scon);
	state_save_register_item("ds5002fp", index, ds5002fp.sbuf);
	state_save_register_item("ds5002fp", index, ds5002fp.p2);
	state_save_register_item("ds5002fp", index, ds5002fp.ie);
	state_save_register_item("ds5002fp", index, ds5002fp.p3);
	state_save_register_item("ds5002fp", index, ds5002fp.ip);
	state_save_register_item("ds5002fp", index, ds5002fp.crc);
	state_save_register_item("ds5002fp", index, ds5002fp.crcl);
	state_save_register_item("ds5002fp", index, ds5002fp.crch);
	state_save_register_item("ds5002fp", index, ds5002fp.mcon);
	state_save_register_item("ds5002fp", index, ds5002fp.ta);
	state_save_register_item("ds5002fp", index, ds5002fp.rnr);
	state_save_register_item("ds5002fp", index, ds5002fp.psw);
	state_save_register_item("ds5002fp", index, ds5002fp.rpctl);
	state_save_register_item("ds5002fp", index, ds5002fp.rps);
	state_save_register_item("ds5002fp", index, ds5002fp.acc);
	state_save_register_item("ds5002fp", index, ds5002fp.b);
	state_save_register_item_array("ds5002fp", index, ds5002fp.IntRam);
}

/* Reset registers to the initial values */
void ds5002fp_reset(void)
{
	int (*save_irqcallback)(int);
	const ds5002fp_config *save_config;

	save_irqcallback = ds5002fp.irq_callback;
	save_config = ds5002fp.config;
	memset(&ds5002fp, 0, sizeof(DS5002FP));
	ds5002fp.irq_callback = save_irqcallback;
	ds5002fp.config = save_config;

	memset(&uart, 0, sizeof(DS5002FP_UART));
	ds5002fp.subtype = 8051;

	//Set up 8051 specific internal read/write (indirect) handlers..
	ds5002fp.iram_iread = internal_ram_read;		//Indirect ram read/write handled the same as direct for 8051!
	ds5002fp.iram_iwrite = internal_ram_write;		//Indirect ram read/write handled the same as direct for 8051!

	//Set up serial call back handlers
	ds5002fp.serial_tx_callback = hold_serial_tx_callback;
	hold_serial_tx_callback = NULL;
	ds5002fp.serial_rx_callback = hold_serial_rx_callback;
	hold_serial_rx_callback = NULL;

	//Setup Expanded/Byte-wide ram callback handlers
	ds5002fp.ebram_iaddr_callback = hold_ebram_iaddr_callback;
	hold_ebram_iaddr_callback = NULL;

	//Clear Ram (w/0xff)
	memset(&ds5002fp.IntRam, 0xff, sizeof(ds5002fp.IntRam));

	/* these are all defined reset states */
	PC = 0;
	SFR_W(ACC, 0);
	SFR_W(B, 0);
	SFR_W(PSW, 0);
	SFR_W(SP, 0x7);
	SFR_W(DPH, 0);
	SFR_W(DPL, 0);
	SFR_W(P3, 0xff);
	SFR_W(P2, 0xff);
	SFR_W(P1, 0xff);
	SFR_W(P0, 0xff);
	SFR_W(IP, 0);
	SFR_W(IE, 0);
	SFR_W(TMOD, 0);
	SFR_W(TCON, 0);
	SFR_W(TH1, 0);
	SFR_W(TH0, 0);
	SFR_W(TL1, 0);
	SFR_W(TL0, 0);
	SFR_W(SCON, 0);

	// set initial values (some of them are set using the bootstrap loader)
	R_PCON = 0;
	R_MCON = ds5002fp.config->mcon & 0xfb;
	R_RPCTL = ds5002fp.config->rpctl & 0x01;
	R_RPS = 0;
	R_RNR = 0;
	R_CRC = ds5002fp.config->crc & 0xf0;
	R_CRCL = 0;
	R_CRCH = 0;
	R_TA = 0;

	/* Flag as NO IRQ in Progress */
	CLEAR_CURRENT_IRQ

	// set internal CPU state
	ds5002fp.last_power_int = CLEAR_LINE;
	ds5002fp.previous_ta = 0;
	ds5002fp.ta_window = 0;
	ds5002fp.range = (GET_RG1 << 1) | GET_RG0;
}

/* Shut down CPU core */
void ds5002fp_exit(void)
{
	/* nothing to do */
}

/* Execute cycles - returns number of cycles actually run */
int ds5002fp_execute(int cycles)
{
	//In stop mode, no op takes place
	if (R_PCON & 0x02) {
		return cycles;
	}

	ds5002fp_icount = cycles;

	//In idle mode, update timers and serial port (not sure if we need to do more things here)
	if (R_PCON & 0x01) {
		do {
			//Update Timer (if any timers are running)
			if((GET_TR0 && GET_ET0) || (GET_TR1 && GET_ET1))
				update_timer(12);

			//Update Serial (if serial port sending data)
			if(uart.sending)
				update_serial(12);

			ds5002fp_icount -= 12;
		} while( ds5002fp_icount > 0 );

		return cycles - ds5002fp_icount;
	}

	do {
		//Read next opcode
		UINT8 op = cpu_readop(PC);

		//Store previous PC
		PPC = PC;

		//Call Debugger
		CALL_DEBUGGER(PC);

		//remove after testing
		if(PC != PPC)	op = cpu_readop(PC);

		//Update Timer (if any timers are running)
		if((GET_TR0 && GET_ET0) || (GET_TR1 && GET_ET1))
			update_timer(ds5002fp.prev_used_cycles);

		//Update Serial (if serial port sending data)
		if(uart.sending)
			update_serial(ds5002fp.prev_used_cycles);

		//Update PC
		PC += 1;
		//Decrement total count by # of cycles used for this opcode
		ds5002fp_icount -= (ds5002fp_cycles[op]);

		switch( op )
		{
			//NOP
			case 0x00:						/* 1: 0000 0000 */
				nop();
				break;
			//AJMP code addr                /* 1: aaa0 0001 */
			case 0x01:
				ajmp();
				break;
			//LJMP code addr
			case 0x02:						/* 1: 0000 0010 */
				ljmp();
				break;
			//RR A
			case 0x03:						/* 1: 0000 0011 */
				rr_a();
				break;
			//INC A
			case 0x04:						/* 1: 0000 0100 */
				inc_a();
				break;
			//INC data addr
			case 0x05:						/* 1: 0000 0101 */
				RWM=1;
				inc_mem();
				RWM=0;
			break;
			//INC @R0/@R1                   /* 1: 0000 011i */
			case 0x06:
			case 0x07:
				inc_ir(op&1);
				break;
			//INC R0 to R7                  /* 1: 0000 1rrr */
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
			case 0x0c:
			case 0x0d:
			case 0x0e:
			case 0x0f:
				inc_r(op&7);
				break;
			//JBC bit addr, code addr
			case 0x10:						/* 1: 0001 0000 */
				RWM=1;
				jbc();
				RWM=0;
				break;
			//ACALL code addr               /* 1: aaa1 0001 */
			case 0x11:
				acall();
				break;
			//LCALL code addr
			case 0x12:						/* 1: 0001 0010 */
				lcall();
				break;
			//RRC A
			case 0x13:						/* 1: 0001 0011 */
				rrc_a();
				break;
			//DEC A
			case 0x14:						/* 1: 0001 0100 */
				dec_a();
				break;
			//DEC data addr
			case 0x15:						/* 1: 0001 0101 */
				RWM=1;
				dec_mem();
				RWM=0;
				break;
			//DEC @R0/@R1                   /* 1: 0001 011i */
			case 0x16:
			case 0x17:
				dec_ir(op&1);
				break;
			//DEC R0 to R7                  /* 1: 0001 1rrr */
			case 0x18:
			case 0x19:
			case 0x1a:
			case 0x1b:
			case 0x1c:
			case 0x1d:
			case 0x1e:
			case 0x1f:
				dec_r(op&7);
				break;
			//JB  bit addr, code addr
			case 0x20:						/* 1: 0010 0000 */
				jb();
				break;
			//AJMP code addr                /* 1: aaa0 0001 */
			case 0x21:
				ajmp();
				break;
			//RET
			case 0x22:						/* 1: 0010 0010 */
				ret();
				break;
			//RL A
			case 0x23:						/* 1: 0010 0011 */
				rl_a();
				break;
			//ADD A, #data
			case 0x24:						/* 1: 0010 0100 */
				add_a_byte();
				break;
			//ADD A, data addr
			case 0x25:						/* 1: 0010 0101 */
				add_a_mem();
				break;
			//ADD A, @R0/@R1                /* 1: 0010 011i */
			case 0x26:
			case 0x27:
				add_a_ir(op&1);
				break;
			//ADD A, R0 to R7               /* 1: 0010 1rrr */
			case 0x28:
			case 0x29:
			case 0x2a:
			case 0x2b:
			case 0x2c:
			case 0x2d:
			case 0x2e:
			case 0x2f:
				add_a_r(op&7);
				break;
			//JNB bit addr, code addr
			case 0x30:						/* 1: 0011 0000 */
				jnb();
				break;
			//ACALL code addr               /* 1: aaa1 0001 */
			case 0x31:
				acall();
				break;
			//RETI
			case 0x32:						/* 1: 0011 0010 */
				reti();
				break;
			//RLC A
			case 0x33:						/* 1: 0011 0011 */
				rlc_a();
				break;
			//ADDC A, #data
			case 0x34:						/* 1: 0011 0100 */
				addc_a_byte();
				break;
			//ADDC A, data addr
			case 0x35:						/* 1: 0011 0101 */
				addc_a_mem();
				break;
			//ADDC A, @R0/@R1               /* 1: 0011 011i */
			case 0x36:
			case 0x37:
				addc_a_ir(op&1);
				break;
			//ADDC A, R0 to R7              /* 1: 0011 1rrr */
			case 0x38:
			case 0x39:
			case 0x3a:
			case 0x3b:
			case 0x3c:
			case 0x3d:
			case 0x3e:
			case 0x3f:
				addc_a_r(op&7);
				break;
			//JC code addr
			case 0x40:						/* 1: 0100 0000 */
				jc();
				break;
			//AJMP code addr                /* 1: aaa0 0001 */
			case 0x41:
				ajmp();
				break;
			//ORL data addr, A
			case 0x42:						/* 1: 0100 0010 */
				RWM=1;
				orl_mem_a();
				RWM=0;
				break;
			//ORL data addr, #data
			case 0x43:						/* 1: 0100 0011 */
				RWM=1;
				orl_mem_byte();
				RWM=0;
				break;
			//ORL A, #data
			case 0x44:						/* 1: 0100 0100 */
				orl_a_byte();
				break;
			//ORL A, data addr
			case 0x45:						/* 1: 0100 0101 */
				orl_a_mem();
				break;
			//ORL A, @RO/@R1                /* 1: 0100 011i */
			case 0x46:
			case 0x47:
				orl_a_ir(op&1);
				break;
			//ORL A, RO to R7               /* 1: 0100 1rrr */
			case 0x48:
			case 0x49:
			case 0x4a:
			case 0x4b:
			case 0x4c:
			case 0x4d:
			case 0x4e:
			case 0x4f:
				orl_a_r(op&7);
				break;
			//JNC code addr
			case 0x50:						/* 1: 0101 0000 */
				jnc();
				break;
			//ACALL code addr               /* 1: aaa1 0001 */
			case 0x51:
				acall();
				break;
			//ANL data addr, A
			case 0x52:						/* 1: 0101 0010 */
				RWM=1;
				anl_mem_a();
				RWM=0;
				break;
			//ANL data addr, #data
			case 0x53:						/* 1: 0101 0011 */
				RWM=1;
				anl_mem_byte();
				RWM=0;
				break;
			//ANL A, #data
			case 0x54:						/* 1: 0101 0100 */
				anl_a_byte();
				break;
			//ANL A, data addr
			case 0x55:						/* 1: 0101 0101 */
				anl_a_mem();
				break;
			//ANL A, @RO/@R1                /* 1: 0101 011i */
			case 0x56:
			case 0x57:
				anl_a_ir(op&1);
				break;
			//ANL A, RO to R7               /* 1: 0101 1rrr */
			case 0x58:
			case 0x59:
			case 0x5a:
			case 0x5b:
			case 0x5c:
			case 0x5d:
			case 0x5e:
			case 0x5f:
				anl_a_r(op&7);
				break;
			//JZ code addr
			case 0x60:						/* 1: 0110 0000 */
				jz();
				break;
			//AJMP code addr                /* 1: aaa0 0001 */
			case 0x61:
				ajmp();
				break;
			//XRL data addr, A
			case 0x62:						/* 1: 0110 0010 */
				RWM=1;
				xrl_mem_a();
				RWM=0;
				break;
			//XRL data addr, #data
			case 0x63:						/* 1: 0110 0011 */
				RWM=1;
				xrl_mem_byte();
				RWM=0;
				break;
			//XRL A, #data
			case 0x64:						/* 1: 0110 0100 */
				xrl_a_byte();
				break;
			//XRL A, data addr
			case 0x65:						/* 1: 0110 0101 */
				xrl_a_mem();
				break;
			//XRL A, @R0/@R1                /* 1: 0110 011i */
			case 0x66:
			case 0x67:
				xrl_a_ir(op&1);
				break;
			//XRL A, R0 to R7               /* 1: 0110 1rrr */
			case 0x68:
			case 0x69:
			case 0x6a:
			case 0x6b:
			case 0x6c:
			case 0x6d:
			case 0x6e:
			case 0x6f:
				xrl_a_r(op&7);
				break;
			//JNZ code addr
			case 0x70:						/* 1: 0111 0000 */
				jnz();
				break;
			//ACALL code addr               /* 1: aaa1 0001 */
			case 0x71:
				acall();
				break;
			//ORL C, bit addr
			case 0x72:						/* 1: 0111 0010 */
				orl_c_bitaddr();
				break;
			//JMP @A+DPTR
			case 0x73:						/* 1: 0111 0011 */
				jmp_iadptr();
				break;
			//MOV A, #data
			case 0x74:						/* 1: 0111 0100 */
				mov_a_byte();
				break;
			//MOV data addr, #data
			case 0x75:						/* 1: 0111 0101 */
				mov_mem_byte();
				break;
			//MOV @R0/@R1, #data            /* 1: 0111 011i */
			case 0x76:
			case 0x77:
				mov_ir_byte(op&1);
				break;
			//MOV R0 to R7, #data           /* 1: 0111 1rrr */
			case 0x78:
			case 0x79:
			case 0x7a:
			case 0x7b:
			case 0x7c:
			case 0x7d:
			case 0x7e:
			case 0x7f:
				mov_r_byte(op&7);
				break;
			//SJMP code addr
			case 0x80:						/* 1: 1000 0000 */
				sjmp();
				break;
			//AJMP code addr                /* 1: aaa0 0001 */
			case 0x81:
				ajmp();
				break;
			//ANL C, bit addr
			case 0x82:						/* 1: 1000 0010 */
				anl_c_bitaddr();
				break;
			//MOVC A, @A + PC
			case 0x83:						/* 1: 1000 0011 */
				movc_a_iapc();
				break;
			//DIV AB
			case 0x84:						/* 1: 1000 0100 */
				div_ab();
				break;
			//MOV data addr, data addr
			case 0x85:						/* 1: 1000 0101 */
				mov_mem_mem();
				break;
			//MOV data addr, @R0/@R1        /* 1: 1000 011i */
			case 0x86:
			case 0x87:
				mov_mem_ir(op&1);
				break;
			//MOV data addr,R0 to R7        /* 1: 1000 1rrr */
			case 0x88:
			case 0x89:
			case 0x8a:
			case 0x8b:
			case 0x8c:
			case 0x8d:
			case 0x8e:
			case 0x8f:
				mov_mem_r(op&7);
				break;
			//MOV DPTR, #data
			case 0x90:						/* 1: 1001 0000 */
				mov_dptr_byte();
				break;
			//ACALL code addr               /* 1: aaa1 0001 */
			case 0x91:
				acall();
				break;
			//MOV bit addr, C
			case 0x92:						/* 1: 1001 0010 */
				RWM = 1;
				mov_bitaddr_c();
				RWM = 0;
				break;
			//MOVC A, @A + DPTR
			case 0x93:						/* 1: 1001 0011 */
				movc_a_iadptr();
				break;
			//SUBB A, #data
			case 0x94:						/* 1: 1001 0100 */
				subb_a_byte();
				break;
			//SUBB A, data addr
			case 0x95:						/* 1: 1001 0101 */
				subb_a_mem();
				break;
			//SUBB A, @R0/@R1               /* 1: 1001 011i */
			case 0x96:
			case 0x97:
				subb_a_ir(op&1);
				break;
			//SUBB A, R0 to R7              /* 1: 1001 1rrr */
			case 0x98:
			case 0x99:
			case 0x9a:
			case 0x9b:
			case 0x9c:
			case 0x9d:
			case 0x9e:
			case 0x9f:
				subb_a_r(op&7);
				break;
			//ORL C, /bit addr
			case 0xa0:						/* 1: 1010 0000 */
				orl_c_nbitaddr();
				break;
			//AJMP code addr                /* 1: aaa0 0001 */
			case 0xa1:
				ajmp();
				break;
			//MOV C, bit addr
			case 0xa2:						/* 1: 1010 0010 */
				mov_c_bitaddr();
				break;
			//INC DPTR
			case 0xa3:						/* 1: 1010 0011 */
				inc_dptr();
				break;
			//MUL AB
			case 0xa4:						/* 1: 1010 0100 */
				mul_ab();
				break;
			//reserved
			case 0xa5:						/* 1: 1010 0101 */
				illegal();
				break;
			//MOV @R0/@R1, data addr        /* 1: 1010 011i */
			case 0xa6:
			case 0xa7:
				mov_ir_mem(op&1);
				break;
			//MOV R0 to R7, data addr       /* 1: 1010 1rrr */
			case 0xa8:
			case 0xa9:
			case 0xaa:
			case 0xab:
			case 0xac:
			case 0xad:
			case 0xae:
			case 0xaf:
				mov_r_mem(op&7);
				break;
			//ANL C,/bit addr
			case 0xb0:						/* 1: 1011 0000 */
				anl_c_nbitaddr();
				break;
			//ACALL code addr               /* 1: aaa1 0001 */
			case 0xb1:
				acall();
				break;
			//CPL bit addr
			case 0xb2:						/* 1: 1011 0010 */
				RWM=1;
				cpl_bitaddr();
				RWM=0;
				break;
			//CPL C
			case 0xb3:						/* 1: 1011 0011 */
				cpl_c();
				break;
			//CJNE A, #data, code addr
			case 0xb4:						/* 1: 1011 0100 */
				cjne_a_byte();
				break;
			//CJNE A, data addr, code addr
			case 0xb5:						/* 1: 1011 0101 */
				cjne_a_mem();
				break;
			//CJNE @R0/@R1, #data, code addr /* 1: 1011 011i */
			case 0xb6:
			case 0xb7:
				cjne_ir_byte(op&1);
				break;
			//CJNE R0 to R7, #data, code addr/* 1: 1011 1rrr */
			case 0xb8:
			case 0xb9:
			case 0xba:
			case 0xbb:
			case 0xbc:
			case 0xbd:
			case 0xbe:
			case 0xbf:
				cjne_r_byte(op&7);
				break;
			//PUSH data addr
			case 0xc0:						/* 1: 1100 0000 */
				push();
				break;
			//AJMP code addr                /* 1: aaa0 0001 */
			case 0xc1:
				ajmp();
				break;
			//CLR bit addr
			case 0xc2:						/* 1: 1100 0010 */
				RWM=1;
				clr_bitaddr();
				RWM=0;
				break;
			//CLR C
			case 0xc3:						/* 1: 1100 0011 */
				clr_c();
				break;
			//SWAP A
			case 0xc4:						/* 1: 1100 0100 */
				swap_a();
				break;
			//XCH A, data addr
			case 0xc5:						/* 1: 1100 0101 */
				xch_a_mem();
				break;
			//XCH A, @RO/@R1                /* 1: 1100 011i */
			case 0xc6:
			case 0xc7:
				xch_a_ir(op&1);
				break;
			//XCH A, RO to R7               /* 1: 1100 1rrr */
			case 0xc8:
			case 0xc9:
			case 0xca:
			case 0xcb:
			case 0xcc:
			case 0xcd:
			case 0xce:
			case 0xcf:
				xch_a_r(op&7);
				break;
			//POP data addr
			case 0xd0:						/* 1: 1101 0000 */
				pop();
				break;
			//ACALL code addr               /* 1: aaa1 0001 */
			case 0xd1:
				acall();
				break;
			//SETB bit addr
			case 0xd2:						/* 1: 1101 0010 */
				RWM=1;
				setb_bitaddr();
				RWM=0;
				break;
			//SETB C
			case 0xd3:						/* 1: 1101 0011 */
				setb_c();
				break;
			//DA A
			case 0xd4:						/* 1: 1101 0100 */
				da_a();
				break;
			//DJNZ data addr, code addr
			case 0xd5:						/* 1: 1101 0101 */
				RWM=1;
				djnz_mem();
				RWM=0;
				break;
			//XCHD A, @R0/@R1               /* 1: 1101 011i */
			case 0xd6:
			case 0xd7:
				xchd_a_ir(op&1);
				break;
			//DJNZ R0 to R7,code addr       /* 1: 1101 1rrr */
			case 0xd8:
			case 0xd9:
			case 0xda:
			case 0xdb:
			case 0xdc:
			case 0xdd:
			case 0xde:
			case 0xdf:
				djnz_r(op&7);
				break;
			//MOVX A,@DPTR
			case 0xe0:						/* 1: 1110 0000 */
				movx_a_idptr();
				break;
			//AJMP code addr                /* 1: aaa0 0001 */
			case 0xe1:
				ajmp();
				break;
			//MOVX A, @R0/@R1               /* 1: 1110 001i */
			case 0xe2:
			case 0xe3:
				movx_a_ir(op&1);
				break;
			//CLR A
			case 0xe4:						/* 1: 1110 0100 */
				clr_a();
				break;
			//MOV A, data addr
			case 0xe5:						/* 1: 1110 0101 */
				mov_a_mem();
				break;
			//MOV A,@RO/@R1                 /* 1: 1110 011i */
			case 0xe6:
			case 0xe7:
				mov_a_ir(op&1);
				break;
			//MOV A,R0 to R7                /* 1: 1110 1rrr */
			case 0xe8:
			case 0xe9:
			case 0xea:
			case 0xeb:
			case 0xec:
			case 0xed:
			case 0xee:
			case 0xef:
				mov_a_r(op&7);
				break;
			//MOVX @DPTR,A
			case 0xf0:						/* 1: 1111 0000 */
				movx_idptr_a();
				break;
			//ACALL code addr               /* 1: aaa1 0001 */
			case 0xf1:
				acall();
				break;
			//MOVX @R0/@R1,A                /* 1: 1111 001i */
			case 0xf2:
			case 0xf3:
				movx_ir_a(op&1);
				break;
			//CPL A
			case 0xf4:						/* 1: 1111 0100 */
				cpl_a();
				break;
			//MOV data addr, A
			case 0xf5:						/* 1: 1111 0101 */
				mov_mem_a();
				break;
			//MOV @R0/@R1, A                /* 1: 1111 011i */
			case 0xf6:
			case 0xf7:
				mov_ir_a(op&1);
				break;
			//MOV R0 to R7, A               /* 1: 1111 1rrr */
			case 0xf8:
			case 0xf9:
			case 0xfa:
			case 0xfb:
			case 0xfc:
			case 0xfd:
			case 0xfe:
			case 0xff:
				mov_r_a(op&7);
				break;
			default:
				illegal();
		}

		//If the chip entered in stop mode, end execution
		if (R_PCON & 0x02) {
			return cycles;
		}

		//Store # of used cycles for this opcode (for timer & serial check at top of code)
		ds5002fp.prev_used_cycles = ds5002fp_cycles[op];

		// decrement the timed access window
		if (ds5002fp.ta_window > 0) {
			ds5002fp.ta_window -= ds5002fp_cycles[op];
			if (ds5002fp.ta_window < 0) {
				ds5002fp.ta_window = 0;
			}
		}

		//Check for pending interrupts & handle - remove cycles used
		ds5002fp_icount-=check_interrupts();

		//If the chip entered in idle mode, end the loop
		if (R_PCON & 0x01) {
			return cycles - ds5002fp_icount;
		}
	} while( ds5002fp_icount > 0 );

	return cycles - ds5002fp_icount;
}

/* Get registers, return context size */
void ds5002fp_get_context(void *dst)
{
	if( dst )
		memcpy(dst, &ds5002fp, sizeof(DS5002FP));
}

/* Set registers */
void ds5002fp_set_context(void *src)
{
	if( src )
		memcpy(&ds5002fp, src, sizeof(DS5002FP));
}

/* Get Internal RAM value */
unsigned ds5002fp_get_intram(int offset)
{
    UINT8 ram_ret = 0;
    RWM = 1;
    ram_ret = IRAM_R(offset);
    RWM = 0;
    return ram_ret;
}

/*NOTE: These are not used by the opcode functions, they are here only for MAME requirements*/
unsigned ds5002fp_get_reg(int regnum)
{
	switch( regnum )
	{
	case REG_PREVIOUSPC: return PPC;
	case REG_PC:
	case DS5002FP_PC:	return PC;
	case REG_SP:
	case DS5002FP_SP:	return R_SP;
	case DS5002FP_ACC:	return R_ACC;
	case DS5002FP_PSW: return R_PSW;
	case DS5002FP_B:	return R_B;
	case DS5002FP_DPH:	return R_DPH;
	case DS5002FP_DPL:	return R_DPL;
	case DS5002FP_IE:	return R_IE;

	case DS5002FP_PCON:	return R_PCON;
	case DS5002FP_MCON:	return R_MCON;
	case DS5002FP_RPCTL:return R_RPCTL;
	case DS5002FP_CRC:	return R_CRC;
	//Not real registers
	case DS5002FP_R0:	return R_R(0);
	case DS5002FP_R1:	return R_R(1);
	case DS5002FP_R2:	return R_R(2);
	case DS5002FP_R3:	return R_R(3);
	case DS5002FP_R4:	return R_R(4);
	case DS5002FP_R5:	return R_R(5);
	case DS5002FP_R6:	return R_R(6);
	case DS5002FP_R7:	return R_R(7);
	case DS5002FP_RB:	return GET_RS;
	default:
		return 0;
	}
}

/*NOTE: These are not used by the opcode functions, they are here only for MAME requirements*/
void ds5002fp_set_reg (int regnum, unsigned val)
{
	switch( regnum )
	{
	case REG_PC:
	case DS5002FP_PC:	PC = val & 0xffff; break;
	case REG_SP:
	case DS5002FP_SP:	SFR_W(SP,val);  break;		//Use SFR to handle error checking
	case DS5002FP_ACC:	SFR_W(ACC,val); break;
	case DS5002FP_PSW: SFR_W(PSW,val); break;
	case DS5002FP_B:   SFR_W(B,val);   break;
	case DS5002FP_DPH: SFR_W(DPH,val); break;
	case DS5002FP_DPL: SFR_W(DPL,val); break;
	case DS5002FP_IE:  SFR_W(IE,val);  break;
	case DS5002FP_PCON:	SFR_W(PCON,val); break;
	case DS5002FP_MCON:	SFR_W(MCON,val); break;
	case DS5002FP_RPCTL: SFR_W(RPCTL,val); break;
	case DS5002FP_CRC:	SFR_W(CRCR,val); break;
	//Not real registers
	case DS5002FP_R0:	R_R(0) = val; break;
	case DS5002FP_R1:	R_R(1) = val; break;
	case DS5002FP_R2:	R_R(2) = val; break;
	case DS5002FP_R3:	R_R(3) = val; break;
	case DS5002FP_R4:	R_R(4) = val; break;
	case DS5002FP_R5:	R_R(5) = val; break;
	case DS5002FP_R6:	R_R(6) = val; break;
	case DS5002FP_R7:	R_R(7) = val; break;
	case DS5002FP_RB:  SET_RS( (val&3) ); break;

	default:
		return;
	}
}

void ds5002fp_set_irq_line(int irqline, int state)
{
	switch( irqline )
	{
		//External Interrupt 0
		case DS5002FP_INT0_LINE:
			//Line Asserted?
			if (state != CLEAR_LINE) {
				//Is the enable flag for this interrupt set?
				if(GET_EX0) {
					//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT0 active lo!
					if(GET_IT0){
						if(ds5002fp.last_int0 == CLEAR_LINE)
							SET_IE0(1);
					}
					else
						SET_IE0(1);		//Nope, just set it..
				}
			}
			else
				SET_IE0(0);		//Clear Int occurred flag
			ds5002fp.last_int0 = state;

			//Do the interrupt & handle - remove machine cycles used
			if(GET_IE0)
				ds5002fp_icount-=check_interrupts();
			break;

		//External Interrupt 1
		case DS5002FP_INT1_LINE:

			//Line Asserted?
			if (state != CLEAR_LINE) {
				if(GET_EX1) {
					//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT1 active lo!
					if(GET_IT1){
						if(ds5002fp.last_int1 == CLEAR_LINE)
							SET_IE1(1);
					}
				else
					SET_IE1(1);		//Nope, just set it..
				}
			}
			else
				SET_IE1(0);		//Clear Int occurred flag
			ds5002fp.last_int1 = state;

			//Do the interrupt & handle - remove machine cycles used
			if(GET_IE1)
				ds5002fp_icount-=check_interrupts();
			break;

		//Serial Port Receive
		case DS5002FP_RX_LINE:
			//Is the enable flags for this interrupt set?
			if(GET_ES && GET_REN) {
				int data = 0;
				//Call our callball function to retrieve the data
				if(ds5002fp.serial_rx_callback)
					data = ds5002fp.serial_rx_callback();
				//Update the register directly, since SFR_W() will trigger a serial transmit instead!
				R_SBUF=data;
				//Flag the IRQ
				SET_RI(1);
			}
			break;
			//Note: we won't call check interrupts, we'll let the main loop catch it

		//Power Fail Interrupt
		case DS5002FP_PFI_LINE:
			//Line Asserted?
			if (state != CLEAR_LINE) {
				if (GET_EPFW) {
					SET_PFW(1);
					//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT1 active lo!
					if (ds5002fp.last_power_int == CLEAR_LINE) {
						ds5002fp_icount-=check_interrupts();
					}
				}
			}
			ds5002fp.last_power_int = state;
			break;
	}
}

/***********************************************************************************
 Check for pending Interrupts and process - returns # of cycles used for the int

 Note about priority & interrupting interrupts..
 1) A high priority interrupt cannot be interrupted by anything!
 2) A low priority interrupt can ONLY be interrupted by a high priority interrupt
 3) If more than 1 Interrupt Flag is set (ie, 2 simultaneous requests occur),
    the following logic works as follows:
    1) If two requests come in of different priority levels, the higher one is selected..
    2) If the requests are of the same level, an internal order is used:
        a) IEO
        b) TFO
        c) IE1
        d) TF1
        e) RI+TI
        f) TF2+EXF2
 **********************************************************************************/
INLINE UINT8 check_interrupts(void)
{
	//If All Inerrupts Disabled or no pending abort if no Power Fail Interrupt is pending
	if(!GET_EA && !GET_PFW) {
		return 0;
	}

	//Skip if current irq in progress is high priority and no Power Fail Interrupt is pending
	if(!GET_PFW && ds5002fp.irq_priority)	{ /* LOG(("high priority irq in progress, skipping irq request\n")); */ return 0; }

	//Check which interrupt(s) requests have occurred..
	//NOTE: The order of checking is based on the internal/default priority levels when levels are the same

	// Power Fail Interrupt
	if(GET_PFW) {
		if (ds5002fp.last_power_int == CLEAR_LINE) {
			//Set vector & priority level request
			ds5002fp.int_vec = V_PFI;
			ds5002fp.priority_request = GET_PFW;
		} else if (!GET_EA) {
			return 0;
		}
	}
	//External Int 0
	if(!ds5002fp.priority_request && GET_IE0) {
		//Set vector & priority level request
		ds5002fp.int_vec = V_IE0;
		ds5002fp.priority_request = GET_PX0;
	}
	//Timer 0 overflow
	if(!ds5002fp.priority_request && GET_TF0 && (!ds5002fp.int_vec || (ds5002fp.int_vec && GET_PT0))) {
		//Set vector & priority level request
		ds5002fp.int_vec = V_TF0;
		ds5002fp.priority_request = GET_PT0;
	}
	//External Int 1
	if(!ds5002fp.priority_request && GET_IE1 && (!ds5002fp.int_vec || (ds5002fp.int_vec && GET_PX1))) {
		//Set vector & priority level request
		ds5002fp.int_vec = V_IE1;
		ds5002fp.priority_request = GET_PX1;
	}
	//Timer 1 overflow
	if(!ds5002fp.priority_request && GET_TF1 && (!ds5002fp.int_vec || (ds5002fp.int_vec && GET_PT1))) {
		//Set vector & priority level request
		ds5002fp.int_vec = V_TF1;
		ds5002fp.priority_request = GET_PT1;
	}
	//Serial Interrupt Transmit/Receive Interrupts (Note: ES Bit - Serial Interrupts must be enabled)
	if(!ds5002fp.priority_request && GET_ES && (GET_TI || GET_RI) && (!ds5002fp.int_vec || (ds5002fp.int_vec && GET_PS))) {
		//Set vector & priority level request
		ds5002fp.int_vec = V_RITI;
		ds5002fp.priority_request = GET_PS;
	}

	//Skip the interrupt request if currently processing is lo priority, and the new request IS NOT HI PRIORITY!
	if(ds5002fp.cur_irq < 0xff && !ds5002fp.priority_request)
		{ LOG(("low priority irq in progress already, skipping low irq request\n")); return 0; }

    /*** --- Perform the interrupt --- ***/

	//Save current pc to stack, set pc to new interrupt vector
	push_pc();
	PC = ds5002fp.int_vec;

	//Set current Irq & Priority being serviced
	ds5002fp.cur_irq = ds5002fp.int_vec;
	ds5002fp.irq_priority = ds5002fp.priority_request;

	//Clear any interrupt flags that should be cleared since we're servicing the irq!
	switch(ds5002fp.cur_irq) {
		case V_IE0:
			//External Int Flag only cleared when configured as Edge Triggered..
			//if(GET_IT0)   - for some reason having this, breaks alving dmd games
				SET_IE0(0);
			break;
		case V_TF0:
			//Timer 0 - Always clear Flag
			SET_TF0(0);
			break;
		case V_IE1:
			//External Int Flag only cleared when configured as Edge Triggered..
			//if(GET_IT1)   - for some reason having this, breaks alving dmd games
				SET_IE1(0);
			break;
		case V_TF1:
			//Timer 0 - Always clear Flag
			SET_TF1(0);
			break;
		case V_RITI:
			// no flags are cleared, TI and RI remain set until reset by software
			break;
		case V_PFI:
			// no flags are cleared, PFW is reset by software
			break;
	}

	//Clear vars.. (these are part of the 8051 structure for speed, so we don't have to dynamically allocate space each time)
	ds5002fp.int_vec = 0;
	ds5002fp.priority_request = 0;

	//All interrupts use 2 machine cycles
	return 24;
}


void ds5002fp_set_irq_callback(int (*callback)(int irqline))
{
	ds5002fp.irq_callback = callback;
}

void ds5002fp_set_serial_tx_callback(void (*callback)(int data))
{
	//Hold in static variable since this function can get called before reset has run, which wipes ds5002fp memory clean
	hold_serial_tx_callback = callback;
}
void ds5002fp_set_serial_rx_callback(int (*callback)(void))
{
	//Hold in static variable since this function can get called before reset has run, which wipes ds5002fp memory clean
	hold_serial_rx_callback = callback;
}

void ds5002fp_set_ebram_iaddr_callback(READ32_HANDLER((*callback)))
{
	//Hold in static variable since this function can get called before reset has run, which wipes ds5002fp memory clean
	hold_ebram_iaddr_callback = callback;
}


void ds5002fp_state_save(void *file)
{
}

void ds5002fp_state_load(void *file)
{
}

/* HELPER FUNCTIONS */

/*All writes to SFR are handled here*/
static WRITE8_HANDLER(sfr_write)
{
	UINT8 is_timed_access;
	UINT8 reg_mask1;
	UINT8 reg_mask2;

	data &= 0xff;		//Ensure only 8 bits
	switch (offset)
	{
		case P0:
			R_P0 = data;
			OUT(0,data);
			break;

		case SP:
			if(offset > 0xff)
				LOG(("ds5002fp #%d: attemping to write value to SP past 256 bytes at 0x%04x\n", cpu_getactivecpu(), PC));
			R_SP = data&0xff; //keep sp w/in 256 bytes
			break;

		case DPL:		R_DPL = data; break;
		case DPH:		R_DPH = data; break;
		case PCON:
			reg_mask1 = 0xb9;

			is_timed_access = (ds5002fp.ta_window > 0) && (R_TA == 0x55);
			if (is_timed_access) {
				reg_mask1 = 0xff;
			}
			R_PCON = (R_PCON & (~reg_mask1)) | (data & reg_mask1);
			break;
		case TCON:		R_TCON = data; break;
		case TMOD:		R_TMOD = data; break;
		case TL0:		R_TL0 = data; break;
		case TL1:		R_TL1 = data; break;
		case TH0:		R_TH0 = data; break;
		case TH1:		R_TH1 = data; break;

		case P1:
			R_P1 = data;
			OUT(1,data);
			break;

		case SCON: {
			//Update register
			R_SCON = data;
			break;
		}

		case SBUF:
			//R_SBUF = data;        //This register is used only for "Receiving data coming in!"
			serial_transmit(data);	//Set up to transmit the data
			break;

		case P2:
			R_P2 = data;
			OUT(2,data);
			break;

		case IE:		R_IE  = data; break;

		case P3:
			R_P3 = data;
			OUT(3,data);
			break;

		case IP:
			reg_mask1 = 0x7f;

			is_timed_access = (ds5002fp.ta_window > 0) && (R_TA == 0x55);
			if (is_timed_access) {
				reg_mask1 = 0xff;
			}
			R_IP = (R_IP & (~reg_mask1)) | (data & reg_mask1);
			break;

		case CRCR:		reg_mask2 = 0x0f;
						R_CRC = (R_CRC & (~reg_mask2)) | (data & reg_mask2);
						LOG(("ds5002fp #%d: write to CRC register at 0x%04x, data=%x\n", cpu_getactivecpu(), PC, data));
						break;
		case CRCL:		R_CRCL = data;
						LOG(("ds5002fp #%d: write to CRCL register at 0x%04x, data=%x\n", cpu_getactivecpu(), PC, data));
						break;
		case CRCH:		R_CRCH = data;
						LOG(("ds5002fp #%d: write to CRCH register at 0x%04x, data=%x\n", cpu_getactivecpu(), PC, data));
						break;
		case MCON:		reg_mask1 = 0x0f;
						reg_mask2 = 0xf7;

						is_timed_access = (ds5002fp.ta_window > 0) && (R_TA == 0x55);
						if (is_timed_access) {
							reg_mask1 = 0xff;
						}
						data = (R_MCON & (~reg_mask1)) | (data & reg_mask1);
						R_MCON = (R_MCON & (~reg_mask2)) | (data & reg_mask2);
						LOG(("ds5002fp #%d: write to MCON register at 0x%04x, data=%x\n", cpu_getactivecpu(), PC, data));
						break;
		case TA:		ds5002fp.previous_ta = R_TA;
						R_TA = data;

						// init the time window after having wrote 0xaa
						if ((data == 0xaa) && (ds5002fp.ta_window == 0)) {
							ds5002fp.ta_window = 4*12 + 2*12;
							LOG(("ds5002fp #%d: TA window initiated at 0x%04x\n", cpu_getactivecpu(), PC));
						}
						//LOG(("ds5002fp #%d: write to TA register at 0x%04x, data=%x\n", cpu_getactivecpu(), PC, data));
						break;
		case RNR:		R_RNR = data;
						LOG(("ds5002fp #%d: write to RNR register at 0x%04x, data=%x\n", cpu_getactivecpu(), PC, data));
						break;

		case PSW:
			R_PSW = data;
			SET_PARITY;
			break;

		case RPCTL:		reg_mask1 = 0xef;
						reg_mask2 = 0xfe;

						is_timed_access = (ds5002fp.ta_window > 0) && (R_TA == 0x55);
						if (is_timed_access) {
							reg_mask1 = 0xff;
						}
						data = (R_RPCTL & (~reg_mask1)) | (data & reg_mask1);
						R_RPCTL = (R_RPCTL & (~reg_mask2)) | (data & reg_mask2);
						LOG(("ds5002fp #%d: write to RPCTL register at 0x%04x, data=%x\n", cpu_getactivecpu(), PC, data));
						break;
		case RPS:		R_RPS = data;
						LOG(("ds5002fp #%d: write to RPS register at 0x%04x, data=%x\n", cpu_getactivecpu(), PC, data));
						break;

		case ACC:
			R_ACC = data;
			SET_PARITY;
			break;

		case B:			R_B   = data; break;

		/* Illegal or non-implemented sfr */
		default:
			LOG(("ds5002fp #%d: attemping to write to an invalid/non-implemented SFR address: %x at 0x%04x, data=%x\n", cpu_getactivecpu(), offset, PC, data));
	}
}

/*All reads to SFR are handled here*/
static READ8_HANDLER(sfr_read)
{
	switch (offset)
	{
		case P0:
			if(RWM)
				return R_P0;					//Read directly from port latch
			else
				return IN(0);					//Read from actual port
		case SP:		return R_SP;
		case DPL:		return R_DPL;
		case DPH:		return R_DPH;
		case PCON:		return R_PCON;
		case TCON:		return R_TCON;
		case TMOD:		return R_TMOD;
		case TL0:		return R_TL0;
		case TL1:		return R_TL1;
		case TH0:		return R_TH0;
		case TH1:		return R_TH1;
		case P1:
			if(RWM)
				return R_P1;					//Read directly from port latch
			else
				return IN(1);					//Read from actual port
		case SCON:		return R_SCON;
		case SBUF:		return R_SBUF;
		case P2:
			if(RWM)
				return R_P2;					//Read directly from port latch
			else
				return IN(2);					//Read from actual port
		case IE:		return R_IE;
		case P3:
			if(RWM)
				return R_P3;					//Read directly from port latch
			else
				return IN(3);					//Read from actual port
		case IP:		return R_IP;

		case CRCR:		LOG(("ds5002fp #%d: read CRC register at 0x%04x\n", cpu_getactivecpu(), PC));
						return R_CRC;
		case CRCL:		LOG(("ds5002fp #%d: read CRCL register at 0x%04x\n", cpu_getactivecpu(), PC));
						return R_CRCL;
		case CRCH:		LOG(("ds5002fp #%d: read CRCH register at 0x%04x\n", cpu_getactivecpu(), PC));
						return R_CRCH;
		case MCON:		LOG(("ds5002fp #%d: read MCON register at 0x%04x\n", cpu_getactivecpu(), PC));
						return R_MCON;
		case TA:		LOG(("ds5002fp #%d: read TA register at 0x%04x\n", cpu_getactivecpu(), PC));
						return R_TA;
		case RNR:		LOG(("ds5002fp #%d: read RNR register at 0x%04x\n", cpu_getactivecpu(), PC));
						return R_RNR;

		case PSW:		return R_PSW;

		case RPCTL:		LOG(("ds5002fp #%d: read RPCTL register at 0x%04x\n", cpu_getactivecpu(), PC));
						return R_RPCTL;
		case RPS:		LOG(("ds5002fp #%d: read RPS register at 0x%04x\n", cpu_getactivecpu(), PC));
						return R_RPS;

		case ACC:		return R_ACC;
		case B:			return R_B;

		/* Illegal or non-implemented sfr */
		default:
			LOG(("ds5002fp #%d: attemping to read an invalid/non-implemented SFR address: %x at 0x%04x\n", cpu_getactivecpu(), offset,PC));
	}
	return 0xff;
}

/* Reads the contents of the Internal RAM memory   */
/* Anything above 0x7f is a sfr/register */
static READ8_HANDLER(internal_ram_read)
{
	if (offset < 0x80)
		return ds5002fp.IntRam[offset];
	else {
		if (offset < 0x100)
			return SFR_R(offset);
		else
			LOG(("ds5002fp #%d: attemping to read from an invalid Internal Ram address: %x at 0x%04x\n", cpu_getactivecpu(), offset,PC));
	}
	return 0xff;
}

/* Writes the contents of the Internal RAM memory   */
/* Anything above 0x7f is a sfr/register */
static WRITE8_HANDLER(internal_ram_write)
{
	data &= 0xff;				//Ensure it's only 8 bits
	if (offset < 0x80)
		ds5002fp.IntRam[offset] = data;
	else {
		if (offset < 0x100)
			SFR_W(offset,data);
		else
			LOG(("ds5002fp #%d: attemping to write to invalid Internal Ram address: %x at 0x%04x\n", cpu_getactivecpu(), offset,PC));
	}
}

/* Reads the contents of the Internal RAM memory (BUT CALLED FROM AN INDIRECT ADDRESSING MODE)   */
/* Different chip types handle differently, for speed, simply call the chip's handler */
static READ8_HANDLER(internal_ram_iread)
{
	return ds5002fp.iram_iread(machine,offset);
}

/* Writes the contents of the Internal RAM memory (BUT CALLED FROM AN INDIRECT ADDRESSING MODE)   */
/* Different chip types handle differently, for speed, simply call the chip's handler */
static WRITE8_HANDLER(internal_ram_iwrite)
{
	ds5002fp.iram_iwrite(machine,offset,data);
}

/* Generate an expanded bus/bytewide bus ram address for read/writing */
/*
    The DS5002FP has 2 16 bits data address buses (the byte-wide bus and the expanded bus). The exact memory position accessed depends on the
    partition mode, the memory range and the expanded bus select. The partition mode and the expanded bus select can be changed at any time.

    In order to simplify memory mapping to the data address bus, the following address map is assumed for partitioned mode:

    0x00000-0x0ffff -> data memory on the expanded bus
    0x10000-0x1ffff -> data memory on the byte-wide bus

    For non-partitioned mode the following memory map is assumed:

    0x0000-0xffff -> data memory (the bus used to access it does not matter)
*/
static READ32_HANDLER(external_ram_iaddr)
{
	// if partition mode is set, adjust offset based on the bus
	if (!GET_PM) {
		if (!GET_EXBS) {
			if ((offset >= ds5002fp_partitions[GET_PA]) && (offset <= ds5002fp_ranges[ds5002fp.range])) {
				offset += 0x10000;
			}
		}
	}
    if(ds5002fp.ebram_iaddr_callback)
        return ds5002fp.ebram_iaddr_callback(machine,offset,mem_mask);
    else {
    	//LOG(("ds5002fp #%d: external ram address requested (8 bit offset=%02x), but no callback available! at PC:%04x\n", cpu_getactivecpu(), offset, PC));
	}

	return offset;
}

/*Push the current PC to the stack*/
INLINE void push_pc()
{
	UINT8 tmpSP = R_SP;							//Grab and Increment Stack Pointer
	tmpSP++;									// ""
	SFR_W(SP,tmpSP);							// ""
    if (tmpSP == R_SP)							//Ensure it was able to write to new stack location
		IRAM_IW(tmpSP, (PC & 0xff));			//Store low byte of PC to Internal Ram (Use IRAM_IW to store stack above 128 bytes)
	tmpSP = R_SP;								//Increment Stack Pointer
	tmpSP++;									// ""
	SFR_W(SP,tmpSP);							// ""
	if (tmpSP == R_SP)							//Ensure it was able to write to new stack location
		IRAM_IW(tmpSP, ( (PC & 0xff00) >> 8));	//Store hi byte of PC to next address in Internal Ram (Use IRAM_IW to store stack above 128 bytes)
}

/*Pop the current PC off the stack and into the pc*/
INLINE void pop_pc()
{
	UINT8 tmpSP = R_SP;							//Grab Stack Pointer
	PC = (IRAM_IR(tmpSP) & 0xff) << 8;			//Store hi byte to PC (must use IRAM_IR to access stack pointing above 128 bytes)
	tmpSP = R_SP-1;								//Decrement Stack Pointer
	SFR_W(SP,tmpSP);							// ""
	if (tmpSP == R_SP)							//Ensure it was able to write to new stack location
		PC = PC | IRAM_IR(tmpSP);				//Store lo byte to PC (must use IRAM_IR to access stack pointing above 128 bytes)
	SFR_W(SP,tmpSP-1);							//Decrement Stack Pointer
}

//Set the PSW Parity Flag
INLINE void set_parity()
{
	//This flag will be set when the accumulator contains an odd # of bits set..
	int i,
	p = 0;
	for (i=1; i<=128; i=i*2) {		//Test for each of the 8 bits in the ACC!
		if ((R_ACC & i) != 0)
			p++;					//Keep track of how many bits are set
	}

	//Update the PSW Pairty bit
	SET_P(p & 1);
}

static READ8_HANDLER(bit_address_r)
{
	int	word;
	int	mask;
	int	bit_pos;
	int	base;		/* base of bit space or sfr */
	int	distance;	/* distance between bit addressable words */
					/* 1 for normal bits, 8 for sfr bit addresses */

	offset &= 0xff;

	//User defined bit addresses 0x20-0x2f (values are 0x0-0x7f)
	if (offset < 0x80) {
		base = 0x20;
		distance = 1;
	}
	//SFR bit addressable registers
	else {
		base = 0x80;
		distance = 8;
	}
	word = ( (offset & 0x78) >> 3) * distance + base;
	bit_pos = offset & 0x7;
	mask = 0x1 << bit_pos;
	return((IRAM_R(word) & mask) >> bit_pos);			//Do not use IRAM_IR
}


static WRITE8_HANDLER(bit_address_w)
{
	int	word;
	int	mask;
	int	bit_pos;
	int	result;
	int	base;
	int	distance;

	offset &= 0xff;

	//User defined bit addresses 0x20-0x2f (values are 0x0-0x7f)
	if (offset < 0x80) {
		base = 0x20;
		distance = 1;
	}
	//SFR bit addressable registers
	else {
		base = 0x80;
		distance = 8;
	}
	word = ((offset & 0x78) >> 3) * distance + base;
	bit_pos = offset & 0x7;
	data = (data & 0x1) << bit_pos;
	mask = ~(1 << bit_pos) & 0xff;
	result = IRAM_R(word) & mask;	//Do not use IRAM_IR
	result = result | data;
	IRAM_W(word, result);			//Do not use IRAM_IW
}

/* The following two handlers are used by the MAME Debugger Memory Window...
   By keeping these functions separate from the internally used IRAM_W/IRAM_R functions,
   we can manipulate and display internal memory in the debugger memory window in a layout
   that is not necessarily how the real memory is.. this will be especially useful for
   the 8052 chip where both the SFR and the upper 128 bytes of ram are mapped to the same
   address, so we can handle that here by mapping the sfr to a different address */

READ8_HANDLER(ds5002fp_internal_r)
{
	//Restrict internal ram to 256 Bytes max
	if(offset < 0x100)
		return IRAM_R(offset);
	else
		return 0;
}
WRITE8_HANDLER(ds5002fp_internal_w)
{
	//Restrict internal ram to 256 Bytes max
	if(offset < 0x100)
		IRAM_W(offset,data);
}

INLINE void do_add_flags(UINT8 a, UINT8 data, UINT8 c)
{
	UINT16 result = a+data+c;
	INT16 result1 = (INT8)a+(INT8)data+c;
	int cy, ac, ov;

	cy = (result & 0x100) >> 8;
	result = (a&0x0f)+(data&0x0f)+c;
	ac = (result & 0x10) >> 4;
	ov = (result1 < -128 || result1 > 127);

	SET_CY(cy);
	SET_AC(ac);
	SET_OV(ov);

#ifdef ENABLE_DEBUGGER
//  mame_printf_debug("add: result=%x, c=%x, ac=%x, ov=%x\n",a+data+c,cy,ac,ov);
#endif
}

INLINE void do_sub_flags(UINT8 a, UINT8 data, UINT8 c)
{
	UINT16 result = a-(data+c);
	INT16 result1 = (INT8)a-(INT8)(data+c);
	int cy, ac, ov;
	cy = (result & 0x100) >> 8;
	result = (a&0x0f)-((data&0x0f)+c);
	ac = (result & 0x10) >> 4;
	ov = (result1 < -128 || result1 > 127);
	SET_CY(cy);
	SET_AC(ac);
	SET_OV(ov);

#ifdef ENABLE_DEBUGGER
//  mame_printf_debug("sub: a=%x, d=%x, c=%x, result=%x, cy=%x, ac=%x, ov=%x\n",a,data,c,a-data-c,cy,ac,ov);
#endif
}

INLINE void update_timer(int cyc)
{
	//This code sucks, needs to be rewritten SJE

	//Todo: Probably better to store the current mode of the timer on a write, so we don't waste time reading it.

	//Note: Counting modes increment on 1 machine cycle (12 oscilator periods) - except Timer 2 in certain modes

	//Update Timer 0
	if(GET_TR0) {
		//Determine Mode
		int mode = (GET_M0_1<<1) | GET_M0_0;
		int overflow;
		UINT16 count = 0;
		switch(mode) {
			case 0:			//13 Bit Timer Mode
				count = ((R_TH0<<8) | R_TL0);
				overflow = 0x3fff;
				//Todo - really, we update HI counter when LO counter hits 0x20
			case 1:			//16 Bit Timer Mode
				count = ((R_TH0<<8) | R_TL0);
				overflow = 0xffff;
				//Check for overflow
				if((UINT32)(count+(cyc/12))>overflow) {
					//Any overflow from cycles?
					cyc-= (overflow-count)*12;
					count = 0;
                    SET_TF0(1);
				}
				//Update the timer
				if(cyc) {
					int inctimer = 0;
					//Gate Bit Set? Timer only incremented if Int0 is set!
					if(GET_GATE0 && GET_IE0)
						inctimer = (cyc/12);
					//Counter Mode? Only increment on 1-0 transition of the Port 3's T0 Line
					if(GET_CT0) {
						//Not supported
					}
					//Neither, regular timer mode
					if(!GET_GATE0 && !GET_CT0)
						inctimer = (cyc/12);

					count+=inctimer;		//Increment counter
				}
				//Update new values of the counter
				R_TH0 = (count>>8) & 0xff;
				R_TL0 = count & 0xff;
				break;
			case 2:			//8 Bit Autoreload
				overflow = 0xff;
				count = R_TL0;
				//Check for overflow
				if(count+(cyc/12)>overflow) {
                    SET_TF0(1);
					//Reload
					count = R_TH0+(overflow-count);
				}
				else
					count+=(cyc/12);
				//Update new values of the counter
				R_TL0 = count & 0xff;
				break;
			case 3:			//Split Timer
                //Split Timer 1
				overflow = 0xff;
				count = R_TL0;
				//Check for overflow
                if(count+(cyc/12)>overflow) {
					count = overflow-count;
                    SET_TF0(1);
                }
				else
					count+=(cyc/12);
				//Update new values of the counter
				R_TL0 = count & 0xff;

                //Split Timer 2
                if(GET_TR1) {
				    overflow = 0xff;
				    count = R_TH0;
				    //Check for overflow
                    if(count+(cyc/12)>overflow) {
					    count = overflow-count;
                        SET_TF1(1);
                    }
				    else
                        count+=(cyc/12);
				    //Update new values of the counter
				    R_TH0 = count & 0xff;
                }
				break;
		}
	}

	//Update Timer 1
	if(GET_TR1) {
		//Determine Mode
		int mode = (GET_M1_1<<1) | GET_M1_0;
		int overflow;
		UINT16 count = 0;
		switch(mode) {
			case 0:			//13 Bit Timer Mode
				count = ((R_TH1<<8) | R_TL1);
				overflow = 0x3fff;
				//Todo - really, we update HI counter when LO counter hits 0x20
			case 1:			//16 Bit Timer Mode
				count = ((R_TH1<<8) | R_TL1);
				overflow = 0xffff;
				//Check for overflow
				if((UINT32)(count+(cyc/12))>overflow) {

					//TODO: Timer 1 can be set as Serial Baud Rate in the 8051 only... process bits here..

					//Any overflow from cycles?
					cyc-= (overflow-count)*12;
					count = 0;
                    SET_TF1(1);
				}
				//Update the timer
				if(cyc) {
					int inctimer = 0;
					//Gate Bit Set? Timer only incremented if Int0 is set!
					if(GET_GATE1 && GET_IE1)
						inctimer = (cyc/12);
					//Counter Mode? Only increment on 1-0 transition of the Port 3's T0 Line
					if(GET_CT1) {
						//Not supported
					}
					//Neither, regular timer mode
					if(!GET_GATE1 && !GET_CT1)
						inctimer = (cyc/12);

					count+=inctimer;		//Increment counter
				}
				//Update new values of the counter
				R_TH1 = (count>>8) & 0xff;
				R_TL1 = count & 0xff;
				break;
			case 2:			//8 Bit Autoreload
				overflow = 0xff;
				count = R_TL1;
				//Check for overflow
				if(count+(cyc/12)>overflow) {
                    SET_TF1(1);
					//Reload
					count = R_TH1+(overflow-count);
				}
				else
					count+=(cyc/12);
				//Update new values of the counter
				R_TL1 = count & 0xff;
				break;
			case 3:			//Split Timer
				break;
		}
	}
}

//Set up to transmit data out of serial port
//NOTE: Enable Serial Port Interrupt bit is NOT required to send/receive data!
INLINE void serial_transmit(UINT8 data)
{
	int mode = (GET_SM0<<1) | GET_SM1;

	//Flag that we're sending data
	uart.sending = 1;
	uart.data_out = data;
	switch(mode) {
		//8 bit shifter ( + start,stop bit ) - baud set by clock freq / 12
		case 0:
			uart.timerbaud = 0;
			uart.bitcycles = 0;
			uart.bits_to_send = 8+2;
			break;
		//8 bit uart ( + start,stop bit ) - baud set by timer1 or timer2
		case 1:
			uart.timerbaud = 1;
			uart.bits_to_send = 8+2;
			break;
		//9 bit uart
		case 2:
		case 3:
			LOG(("Serial mode 2 & 3 not supported in ds5002fp!\n"));
			break;
	}
}

//Check and update status of serial port
INLINE void	update_serial(int cyc)
{
	//Any bits left to send?
	if(uart.bits_to_send) {
		//Timer Generated baud?
		if(uart.timerbaud) {
			//Let Timer overflow handle removing bits
		}
		else {
			//Oscillator Based baud rate = Osc/12 baud rate, however it also means 1 bit = 12 cycles.
			uart.bitcycles+=cyc;
			if(uart.bitcycles > 11)	{
				int bits_sent = uart.bitcycles / 12;
				int diff = uart.bitcycles % 12;
				//don't allow more bits sent than ready to send
				if(bits_sent > uart.bits_to_send) {
					bits_sent = uart.bits_to_send;
					diff = 0;
				}
				uart.bits_to_send-=bits_sent;
				uart.bitcycles = diff;
			}
		}
	}
	//If no bits left to send - flag the interrupt & call the callback
	if(!uart.bits_to_send) {
		//Clear sending flag
		uart.sending = 0;
		uart.bitcycles = 0;
		//Call the callback function
		if(ds5002fp.serial_tx_callback)
			ds5002fp.serial_tx_callback(uart.data_out);
		//Set Interrupt Flag
		SET_TI(1);
		//Note: we'll let the main loop catch the interrupt
	}
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void ds5002fp_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:							PC = info->i;							break;
		case CPUINFO_INT_SP:							ds5002fp.sp = info->i;	     				break;

		case CPUINFO_INT_INPUT_STATE + DS5002FP_INT0_LINE:	ds5002fp_set_irq_line(DS5002FP_INT0_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + DS5002FP_INT1_LINE:	ds5002fp_set_irq_line(DS5002FP_INT1_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + DS5002FP_T0_LINE:	ds5002fp_set_irq_line(DS5002FP_T0_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + DS5002FP_T1_LINE:	ds5002fp_set_irq_line(DS5002FP_T1_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + DS5002FP_RX_LINE:	ds5002fp_set_irq_line(DS5002FP_RX_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + DS5002FP_PFI_LINE:	ds5002fp_set_irq_line(DS5002FP_PFI_LINE, info->i); break;

		case CPUINFO_INT_REGISTER + DS5002FP_PC: 			PC = info->i;							break;
		case CPUINFO_INT_REGISTER + DS5002FP_SP: 			R_SP = info->i;							break;
		case CPUINFO_INT_REGISTER + DS5002FP_PSW:			ds5002fp.psw = info->i;					break;
		case CPUINFO_INT_REGISTER + DS5002FP_ACC:			ds5002fp.acc = info->i;					break;
		case CPUINFO_INT_REGISTER + DS5002FP_B:  			ds5002fp.b = info->i;						break;
		case CPUINFO_INT_REGISTER + DS5002FP_DPH:			ds5002fp.dph = info->i;					break;
		case CPUINFO_INT_REGISTER + DS5002FP_DPL:			ds5002fp.dpl = info->i;					break;
		case CPUINFO_INT_REGISTER + DS5002FP_IE: 			ds5002fp.ie = info->i;						break;
		case CPUINFO_INT_REGISTER + DS5002FP_PCON:			ds5002fp.pcon = info->i; 				break;
		case CPUINFO_INT_REGISTER + DS5002FP_MCON:			ds5002fp.mcon = info->i; 				break;
		case CPUINFO_INT_REGISTER + DS5002FP_RPCTL: 		ds5002fp.rpctl = info->i; 				break;
		case CPUINFO_INT_REGISTER + DS5002FP_CRC:			ds5002fp.crc = info->i; 				break;
		case CPUINFO_INT_REGISTER + DS5002FP_R0: 			ds5002fp.IntRam[0+(8*((ds5002fp.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R1: 			ds5002fp.IntRam[1+(8*((ds5002fp.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R2: 			ds5002fp.IntRam[2+(8*((ds5002fp.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R3: 			ds5002fp.IntRam[3+(8*((ds5002fp.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R4: 			ds5002fp.IntRam[4+(8*((ds5002fp.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R5: 			ds5002fp.IntRam[5+(8*((ds5002fp.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R6: 			ds5002fp.IntRam[6+(8*((ds5002fp.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R7: 			ds5002fp.IntRam[7+(8*((ds5002fp.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + DS5002FP_RB: 			ds5002fp.IntRam[8+(8*((ds5002fp.psw & 0x18)>>3))] = info->i; break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

void ds5002fp_get_info(UINT32 state, cpuinfo *info)
{
	DS5002FP *r = &ds5002fp;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(ds5002fp);				break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 20; /* rough guess */			break;
		case CPUINFO_INT_INPUT_LINES:        			info->i = 6;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 17; /* really 16 bits, but it has 2 address bus */ break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 8;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 	info->i = 16;						break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 	info->i = 0;						break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;
		case CPUINFO_INT_PC:	 						info->i = PC;							break;
		case CPUINFO_INT_SP:							info->i = R_SP;							break;

		case CPUINFO_INT_REGISTER + DS5002FP_PC: 			info->i = PC;							break;
		case CPUINFO_INT_REGISTER + DS5002FP_SP: 			info->i = R_SP;							break;
		case CPUINFO_INT_REGISTER + DS5002FP_PSW:			info->i = ds5002fp.psw;					break;
		case CPUINFO_INT_REGISTER + DS5002FP_ACC:			info->i = ds5002fp.acc;					break;
		case CPUINFO_INT_REGISTER + DS5002FP_B:  			info->i = ds5002fp.b;						break;
		case CPUINFO_INT_REGISTER + DS5002FP_DPH:			info->i = ds5002fp.dph;					break;
		case CPUINFO_INT_REGISTER + DS5002FP_DPL:			info->i = ds5002fp.dpl;					break;
		case CPUINFO_INT_REGISTER + DS5002FP_IE: 			info->i = ds5002fp.ie;						break;
		case CPUINFO_INT_REGISTER + DS5002FP_PCON:			info->i = ds5002fp.pcon; 				break;
		case CPUINFO_INT_REGISTER + DS5002FP_MCON:			info->i = ds5002fp.mcon; 				break;
		case CPUINFO_INT_REGISTER + DS5002FP_RPCTL: 		info->i = ds5002fp.rpctl; 				break;
		case CPUINFO_INT_REGISTER + DS5002FP_CRC:			info->i = ds5002fp.crc; 				break;
		case CPUINFO_INT_REGISTER + DS5002FP_R0: 			info->i = ds5002fp.IntRam[0+(8*((ds5002fp.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R1: 			info->i = ds5002fp.IntRam[1+(8*((ds5002fp.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R2: 			info->i = ds5002fp.IntRam[2+(8*((ds5002fp.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R3: 			info->i = ds5002fp.IntRam[3+(8*((ds5002fp.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R4: 			info->i = ds5002fp.IntRam[4+(8*((ds5002fp.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R5: 			info->i = ds5002fp.IntRam[5+(8*((ds5002fp.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R6: 			info->i = ds5002fp.IntRam[6+(8*((ds5002fp.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + DS5002FP_R7: 			info->i = ds5002fp.IntRam[7+(8*((ds5002fp.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + DS5002FP_RB: 			info->i = ds5002fp.IntRam[8+(8*((ds5002fp.psw & 0x18)>>3))]; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = ds5002fp_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = ds5002fp_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = ds5002fp_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = ds5002fp_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = ds5002fp_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = ds5002fp_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = ds5002fp_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef ENABLE_DEBUGGER
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = ds5002fp_dasm;			break;
#endif /* ENABLE_DEBUGGER */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &ds5002fp_icount;			break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map8 = NULL;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map8 = NULL;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_IO:      info->internal_map8 = NULL;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "DS5002FP");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Dallas");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Manuel Abadia"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				r->psw & 0x80 ? 'C':'.',
				r->psw & 0x40 ? 'A':'.',
				r->psw & 0x20 ? 'F':'.',
				r->psw & 0x10 ? '0':'.',
				r->psw & 0x08 ? '1':'.',
				r->psw & 0x04 ? 'V':'.',
				r->psw & 0x02 ? '?':'.',
				r->psw & 0x01 ? 'P':'.');
			break;

		case CPUINFO_STR_REGISTER + DS5002FP_PC:        	sprintf(info->s, "PC:%04X", r->pc);		break;
		case CPUINFO_STR_REGISTER + DS5002FP_SP:        	sprintf(info->s, "SP:%02X", r->sp);		break;
		case CPUINFO_STR_REGISTER + DS5002FP_PSW:       	sprintf(info->s, "PSW:%02X", r->psw);	break;
		case CPUINFO_STR_REGISTER + DS5002FP_ACC:       	sprintf(info->s, "A:%02X", r->acc);		break;
		case CPUINFO_STR_REGISTER + DS5002FP_B:         	sprintf(info->s, "B:%02X", r->b);		break;
		case CPUINFO_STR_REGISTER + DS5002FP_DPH:       	sprintf(info->s, "DPH:%02X", r->dph);	break;
		case CPUINFO_STR_REGISTER + DS5002FP_DPL:       	sprintf(info->s, "DPL:%02X", r->dpl);	break;
		case CPUINFO_STR_REGISTER + DS5002FP_IE:        	sprintf(info->s, "IE:%02X", r->ie);		break;
		case CPUINFO_STR_REGISTER + DS5002FP_PCON:        	sprintf(info->s, "PCON:%02X", r->pcon);	break;
		case CPUINFO_STR_REGISTER + DS5002FP_MCON:        	sprintf(info->s, "MCON:%02X", r->mcon);	break;
		case CPUINFO_STR_REGISTER + DS5002FP_RPCTL:        	sprintf(info->s, "RPCTL:%02X", r->rpctl);break;
		case CPUINFO_STR_REGISTER + DS5002FP_CRC:        	sprintf(info->s, "CRC:%02X", r->crc);	break;
		case CPUINFO_STR_REGISTER + DS5002FP_R0:        	sprintf(info->s, "R0:%02X", r->IntRam[0+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + DS5002FP_R1:        	sprintf(info->s, "R1:%02X", r->IntRam[1+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + DS5002FP_R2:        	sprintf(info->s, "R2:%02X", r->IntRam[2+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + DS5002FP_R3:			sprintf(info->s, "R3:%02X", r->IntRam[3+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + DS5002FP_R4: 			sprintf(info->s, "R4:%02X", r->IntRam[4+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + DS5002FP_R5: 			sprintf(info->s, "R5:%02X", r->IntRam[5+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + DS5002FP_R6: 			sprintf(info->s, "R6:%02X", r->IntRam[6+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + DS5002FP_R7: 			sprintf(info->s, "R7:%02X", r->IntRam[7+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + DS5002FP_RB: 			sprintf(info->s, "RB:%02X", ((r->psw & 0x18)>>3)); break;
	}
}
