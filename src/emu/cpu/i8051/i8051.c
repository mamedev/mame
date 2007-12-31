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
 *   Copyright (c) 2003 Steve Ellenoff, all rights reserved.
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
#include "i8051.h"

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

#if (HAS_I8052 || HAS_I8752)
static READ8_HANDLER(i8052_internal_ram_iread);
static WRITE8_HANDLER(i8052_internal_ram_iwrite);
#endif
//

typedef struct {
	UINT8	timerbaud;		//Flag set if timer overflow is controlling baud
	UINT8	sending;		//Flag set when uart is sending
	UINT8	data_out;		//Data to send out
	UINT8	bits_to_send;	//How many bits left to send when transmitting out the serial port
	UINT8	bitcycles;		//# of bitcycles passed since last bit was sent
} I8051_UART;

typedef struct {

	//Internal stuff
	UINT16	ppc;			//previous pc
	UINT16	pc;				//current pc
	UINT16	subtype;		//specific version of the cpu, ie 8031, or 8051 for example
	UINT8	cur_irq;		//Holds value of any current IRQ being serviced
	UINT8	irq_priority;	//Holds value of the current IRQ Priority Level
	UINT8	rwm;			//Signals that the current instruction is a read/write/modify instruction
	int prev_used_cycles;	//Track previous # of used cycles
	int last_int0;			//Store state of int0
	int last_int1;			//Store state of int1
	UINT8 int_vec;			//Pending Interrupt Vector
	int priority_request;	//Priority level of incoming new irq
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
	//8052 Only registers
	#if (HAS_I8052 || HAS_I8752)
		UINT8	t2con;		//Timer/Counter 2 Control
		UINT8	rcap2l;		//Timer/Counter 2 Capture Register Lo
		UINT8	rcap2h;		//Timer/Counter 2 Capture Register Hi
		UINT8	tl2;		//Timer 2 Lo
		UINT8	th2;		//Timer 2 Hi
	#endif
	UINT8	psw;			//Program Status Word
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

	//External Ram Address Callback for generating the hardware specific access to external ram
	READ32_HANDLER((*eram_iaddr_callback));

}	I8051;

static int i8051_icount;

static I8051 i8051;
static I8051_UART uart;

//Hold callback functions so they can be set by caller (before the cpu reset)
static void (*hold_serial_tx_callback)(int data);
static int (*hold_serial_rx_callback)(void);
static READ32_HANDLER((*hold_eram_iaddr_callback));

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
#define CODEMEM_R(a)	(UINT8)program_read_byte_8(a)
/*****************************************************************************
 * Read/Write a byte from/to External Data Memory (Usually RAM or other I/O)
 *****************************************************************************
 This area is *ALSO* mapped from 0-FFFF internally (64K)
                        ** HOWEVER **
 We *FORCE* the address space into the range 10000-1FFFF to allow both
 Code Memory and Data Memory to be pyshically separate while mapped @ the same
 addresses, w/o any contention.
 As far as the 8051 program code which is executing knows data memory still lives
 in the 0-FFFF range.*/
#define DATAMEM_R(a)	(UINT8)data_read_byte_8(a)
#define DATAMEM_W(a,v)	data_write_byte_8(a,v)

/***************************************************************
 * Read/Write a byte from/to the Internal RAM
 ***************************************************************/
#define IRAM_R(a)		internal_ram_read(a)
#define IRAM_W(a,v)		internal_ram_write(a,v)
/***************************************************************
 * Read/Write a byte from/to the Internal RAM indirectly (called from indirect addressing)
 ***************************************************************/
#define IRAM_IR(a)		internal_ram_iread(a)
#define IRAM_IW(a,v)		internal_ram_iwrite(a,v)
/***************************************************************
 * Form an Address to Read/Write to External RAM indirectly (called from indirect addressing)
 ***************************************************************/
#define ERAM_ADDR(a,m)		external_ram_iaddr(a,m)
/***************************************************************
 * Read/Write a byte from/to the SFR Registers
 ***************************************************************/
#define SFR_R(a)		sfr_read(a)
#define SFR_W(a,v)		sfr_write(a,v)
/***************************************************************
 * Read/Write a bit from Bit Addressable Memory
 ***************************************************************/
#define BIT_R(a)		bit_address_r(a)
#define BIT_W(a,v)		bit_address_w(a,v)
/***************************************************************
 * Input/Output a byte from given I/O port
 ***************************************************************/
#define IN(port)		((UINT8)io_read_byte(port))
#define OUT(port,value) 	io_write_byte(port,value)
/***************************************************************
 * Access the 4 banks of R registers (R0...R7)
 ***************************************************************/
#define R_R(n)			i8051.IntRam[(GET_RS*8)+(n)]
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
#if (HAS_I8052 || HAS_I8752)
   #define SET_ET2(n)		R_IE = (R_IE & 0xdf) | (n<<5);	//Timer 2 Interrupt Enable/Disable
#endif
#define SET_ES(n)		R_IE = (R_IE & 0xef) | (n<<4);		//Serial Interrupt Enable/Disable
#define SET_ET1(n)		R_IE = (R_IE & 0xf7) | (n<<3);		//Timer 1 Interrupt Enable/Disable
#define SET_EX1(n)		R_IE = (R_IE & 0xfb) | (n<<2);		//External Int 1 Interrupt Enable/Disable
#define SET_ET0(n)		R_IE = (R_IE & 0xfd) | (n<<1);		//Timer 0 Interrupt Enable/Disable
#define SET_EX0(n)		R_IE = (R_IE & 0xfe) | (n<<0);		//External Int 0 Interrupt Enable/Disable
/*IP Flags*/
#if (HAS_I8052 || HAS_I8752)
   #define SET_PT2(n)		R_IP = (R_IP & 0xdf) | (n<<5);	//Set Timer 2 Priority Level
#endif
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

#if (HAS_I8052 || HAS_I8752)
  /*T2CON Flags*/
  #define SET_TF2(n)		R_T2CON = (R_T2CON & 0x7f) | (n<<7);	//Indicated Timer 2 Overflow Int Triggered
  #define SET_EXF2(n)		R_T2CON = (R_T2CON & 0xbf) | (n<<6);	//Indicates Timer 2 External Flag
  #define SET_RCLK(n)		R_T2CON = (R_T2CON & 0xdf) | (n<<5);	//Receive Clock
  #define SET_TCLK(n)		R_T2CON = (R_T2CON & 0xef) | (n<<4);	//Transmit Clock
  #define SET_EXEN2(n)		R_T2CON = (R_T2CON & 0xf7) | (n<<3);	//Timer 2 External Interrupt Enable
  #define SET_TR2(n)		R_T2CON = (R_T2CON & 0xfb) | (n<<2);	//Indicates Timer 2 is running
  #define SET_CT2(n)		R_T2CON = (R_T2CON & 0xfd) | (n<<1);	//Sets Timer 2 Counter/Timer Mode
  #define SET_CP(n)			R_T2CON = (R_T2CON & 0xfe) | (n<<0);	//Sets Timer 2 Capture/Reload Mode
#endif

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
/*IP Flags*/
#if (HAS_I8052 || HAS_I8752)
  #define GET_PT2			((R_IP & 0x20)>>5)
#endif
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

#if (HAS_I8052 || HAS_I8752)
  /*T2CON Flags*/
  #define GET_TF2			((R_T2CON & 0x80)>>7)
  #define GET_EXF2			((R_T2CON & 0x40)>>6)
  #define GET_RCLK			((R_T2CON & 0x20)>>5)
  #define GET_TCLK			((R_T2CON & 0x10)>>4)
  #define GET_EXEN2			((R_T2CON & 0x08)>>3)
  #define GET_TR2			((R_T2CON & 0x04)>>2)
  #define GET_CT2			((R_T2CON & 0x02)>>1)
  #define GET_CP			((R_T2CON & 0x01)>>0)
#endif

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

#if (HAS_I8052 || HAS_I8752)
#define V_TF2	0x02b	/* Timer 2 Overflow */
#endif

/* Any pending IRQ */
#define SERIALPORT_IRQ    ((R_SCON & 0x03) && GET_ES)

#if (HAS_I8052 || HAS_I8752)
#define NO_PENDING_IRQ  !(R_TCON & 0xaa) && !(SERIALPORT_IRQ) && !(GET_ET2 && (GET_TF2 || GET_EXF2))
#else
#define NO_PENDING_IRQ  !(R_TCON & 0xaa) && !(SERIALPORT_IRQ)
#endif

/* Clear Current IRQ  */
#define CLEAR_CURRENT_IRQ i8051.cur_irq = 0xff;\
						  i8051.irq_priority = 0;

/* shorter names for the I8051 structure elements */

//Internal stuff
#define PPC 	i8051.ppc
#define PC		i8051.pc
#define TYPE	i8051.subtype
#define RWM		i8051.rwm

//SFR Registers
#define R_P0	i8051.po
#define R_SP	i8051.sp
#define R_DPL	i8051.dpl
#define R_DPH	i8051.dph
#define R_PCON	i8051.pcon
#define R_TCON	i8051.tcon
#define R_TMOD	i8051.tmod
#define R_TL0	i8051.tl0
#define R_TL1	i8051.tl1
#define R_TH0	i8051.th0
#define R_TH1	i8051.th1
#define R_P1	i8051.p1
#define R_SCON	i8051.scon
#define R_SBUF	i8051.sbuf
#define R_P2	i8051.p2
#define R_IE	i8051.ie
#define R_P3	i8051.p3
#define R_IP	i8051.ip
//8052 Only registers
#if (HAS_I8052 || HAS_I8752)
  #define R_T2CON	i8051.t2con
  #define R_RCAP2L	i8051.rcap2l
  #define R_RCAP2H	i8051.rcap2h
  #define R_TL2		i8051.tl2
  #define R_TH2		i8051.th2
#endif
#define R_PSW	i8051.psw
#define R_ACC	i8051.acc
#define R_B		i8051.b

/* # of oscilations each opcode requires*/
static const UINT8 i8051_cycles[] = {
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

/* Include Opcode functions */
#include "i8051ops.c"

void i8051_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	i8051_set_irq_callback(irqcallback);

	//Internal stuff
	state_save_register_item("i8051", index, i8051.ppc);
	state_save_register_item("i8051", index, i8051.pc);
	state_save_register_item("i8051", index, i8051.subtype);
	state_save_register_item("i8051", index, i8051.rwm );
	state_save_register_item("i8051", index, i8051.cur_irq );
	//SFR Registers
	state_save_register_item("i8051", index, i8051.po);
	state_save_register_item("i8051", index, i8051.sp);
	state_save_register_item("i8051", index, i8051.dpl);
	state_save_register_item("i8051", index, i8051.dph);
	state_save_register_item("i8051", index, i8051.pcon);
	state_save_register_item("i8051", index, i8051.tcon);
	state_save_register_item("i8051", index, i8051.tmod);
	state_save_register_item("i8051", index, i8051.tl0);
	state_save_register_item("i8051", index, i8051.tl1);
	state_save_register_item("i8051", index, i8051.th0);
	state_save_register_item("i8051", index, i8051.th1);
	state_save_register_item("i8051", index, i8051.p1);
	state_save_register_item("i8051", index, i8051.scon);
	state_save_register_item("i8051", index, i8051.sbuf);
	state_save_register_item("i8051", index, i8051.p2);
	state_save_register_item("i8051", index, i8051.ie);
	state_save_register_item("i8051", index, i8051.p3);
	state_save_register_item("i8051", index, i8051.ip);
	//8052 Only registers
	#if (HAS_I8052 || HAS_I8752)
		state_save_register_item("i8051", index, i8051.rcap2l);
		state_save_register_item("i8051", index, i8051.rcap2h);
		state_save_register_item("i8051", index, i8051.tl2);
		state_save_register_item("i8051", index, i8051.th2);
	#endif
	state_save_register_item("i8051", index, i8051.psw);
	state_save_register_item("i8051", index, i8051.acc);
	state_save_register_item("i8051", index, i8051.b);
}

/* Reset registers to the initial values */
void i8051_reset(void)
{
	int (*save_irqcallback)(int);

	save_irqcallback = i8051.irq_callback;
	memset(&i8051, 0, sizeof(I8051));
	i8051.irq_callback = save_irqcallback;

	memset(&uart, 0, sizeof(I8051_UART));
	i8051.subtype = 8051;

	//Set up 8051 specific internal read/write (indirect) handlers..
	i8051.iram_iread = internal_ram_read;		//Indirect ram read/write handled the same as direct for 8051!
	i8051.iram_iwrite = internal_ram_write;		//Indirect ram read/write handled the same as direct for 8051!

	//Set up serial call back handlers
	i8051.serial_tx_callback = hold_serial_tx_callback;
	hold_serial_tx_callback = NULL;
	i8051.serial_rx_callback = hold_serial_rx_callback;
	hold_serial_rx_callback = NULL;

	//Setup External ram callback handlers
	i8051.eram_iaddr_callback = hold_eram_iaddr_callback;
	hold_eram_iaddr_callback = NULL;

	//Clear Ram (w/0xff)
	memset(&i8051.IntRam,0xff,sizeof(i8051.IntRam));

	/* these are all defined reset states */
	PC = 0;
	SFR_W(SP, 0x7);
	SFR_W(PSW, 0);
	SFR_W(DPH, 0);
	SFR_W(DPL, 0);
	SFR_W(ACC, 0);
	SFR_W(B, 0);
	SFR_W(IP, 0);
	SFR_W(IE, 0);
	SFR_W(SCON, 0);
	SFR_W(TCON, 0);
	SFR_W(TMOD, 0);
	SFR_W(TH1, 0);
	SFR_W(TH0, 0);
	SFR_W(TL1, 0);
	SFR_W(TL0, 0);
	/* set the port configurations to all 1's */
	SFR_W(P3, 0xff);
	SFR_W(P2, 0xff);
	SFR_W(P1, 0xff);
	SFR_W(P0, 0xff);

	/* Flag as NO IRQ in Progress */
	CLEAR_CURRENT_IRQ
}

/* Shut down CPU core */
void i8051_exit(void)
{
	/* nothing to do */
}

/* Execute cycles - returns number of cycles actually run */
int i8051_execute(int cycles)
{
	i8051_icount = cycles;

	do
	{
		//Read next opcode
		UINT8 op = cpu_readop(PC);

		//Store previous PC
		PPC = PC;

		//Call Debugger
		CALL_MAME_DEBUG;

		//remove after testing
		if(PC != PPC)	op = cpu_readop(PC);

		//Update Timer (if any timers are running)
		if(R_TCON & 0x50)
			update_timer(i8051.prev_used_cycles);

		//Update Serial (if serial port sending data)
		if(uart.sending)
			update_serial(i8051.prev_used_cycles);

		//Update PC
		PC += 1;
		//Decrement total count by # of cycles used for this opcode
		i8051_icount -= (i8051_cycles[op]);

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

		//Store # of used cycles for this opcode (for timer & serial check at top of code)
		i8051.prev_used_cycles = i8051_cycles[op];

		//Check for pending interrupts & handle - remove cycles used
		i8051_icount-=check_interrupts();

	} while( i8051_icount > 0 );

	return cycles - i8051_icount;
}

/* Get registers, return context size */
void i8051_get_context(void *dst)
{
	if( dst )
		memcpy(dst, &i8051, sizeof(I8051));
}

/* Set registers */
void i8051_set_context(void *src)
{
	if( src )
		memcpy(&i8051, src, sizeof(I8051));
}

/* Get Internal RAM value */
unsigned i8051_get_intram(int offset)
{
    UINT8 ram_ret = 0;
    RWM = 1;
    ram_ret = IRAM_R(offset);
    RWM = 0;
    return ram_ret;
}

/*NOTE: These are not used by the opcode functions, they are here only for MAME requirements*/
unsigned i8051_get_reg(int regnum)
{
	switch( regnum )
	{
	case REG_PREVIOUSPC: return PPC;
	case REG_PC:
	case I8051_PC:	return PC;
	case REG_SP:
	case I8051_SP:	return R_SP;
	case I8051_ACC:	return R_ACC;
	case I8051_PSW: return R_PSW;
	case I8051_B:	return R_B;
	case I8051_DPH:	return R_DPH;
	case I8051_DPL:	return R_DPL;
	case I8051_IE:	return R_IE;
	//Not real registers
	case I8051_R0:	return R_R(0);
	case I8051_R1:	return R_R(1);
	case I8051_R2:	return R_R(2);
	case I8051_R3:	return R_R(3);
	case I8051_R4:	return R_R(4);
	case I8051_R5:	return R_R(5);
	case I8051_R6:	return R_R(6);
	case I8051_R7:	return R_R(7);
	case I8051_RB:	return GET_RS;
	default:
		return 0;
	}
	return 0;
}

/*NOTE: These are not used by the opcode functions, they are here only for MAME requirements*/
void i8051_set_reg (int regnum, unsigned val)
{
	switch( regnum )
	{
	case REG_PC:
	case I8051_PC:	PC = val & 0xffff; break;
	case REG_SP:
	case I8051_SP:	SFR_W(SP,val);  break;		//Use SFR to handle error checking
	case I8051_ACC:	SFR_W(ACC,val); break;
	case I8051_PSW: SFR_W(PSW,val); break;
	case I8051_B:   SFR_W(B,val);   break;
	case I8051_DPH: SFR_W(DPH,val); break;
	case I8051_DPL: SFR_W(DPL,val); break;
	case I8051_IE:  SFR_W(IE,val);  break;
	//Not real registers
	case I8051_R0:	R_R(0) = val; break;
	case I8051_R1:	R_R(1) = val; break;
	case I8051_R2:	R_R(2) = val; break;
	case I8051_R3:	R_R(3) = val; break;
	case I8051_R4:	R_R(4) = val; break;
	case I8051_R5:	R_R(5) = val; break;
	case I8051_R6:	R_R(6) = val; break;
	case I8051_R7:	R_R(7) = val; break;
	case I8051_RB:  SET_RS( (val&3) ); break;

	default:
		return;
	}
}

void i8051_set_irq_line(int irqline, int state)
{
	switch( irqline )
	{
		//External Interrupt 0
		case I8051_INT0_LINE:
			//Line Asserted?
			if (state != CLEAR_LINE) {
				//Is the enable flag for this interrupt set?
				if(GET_EX0) {
					//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT0 active lo!
					if(GET_IT0){
						if(i8051.last_int0 == CLEAR_LINE)
							SET_IE0(1);
					}
					else
						SET_IE0(1);		//Nope, just set it..
				}
			}
			else
				SET_IE0(0);		//Clear Int occurred flag
			i8051.last_int0 = state;

			//Do the interrupt & handle - remove machine cycles used
			if(GET_IE0)
				i8051_icount-=check_interrupts();
			break;

		//External Interrupt 1
		case I8051_INT1_LINE:

			//Line Asserted?
			if (state != CLEAR_LINE) {
				if(GET_EX1) {
					//Need cleared->active line transition? (Logical 1-0 Pulse on the line) - CLEAR->ASSERT Transition since INT1 active lo!
					if(GET_IT1){
						if(i8051.last_int1 == CLEAR_LINE)
							SET_IE1(1);
					}
				else
					SET_IE1(1);		//Nope, just set it..
				}
			}
			else
				SET_IE1(0);		//Clear Int occurred flag
			i8051.last_int1 = state;

			//Do the interrupt & handle - remove machine cycles used
			if(GET_IE1)
				i8051_icount-=check_interrupts();
			break;

		//Serial Port Receive
		case I8051_RX_LINE:
			//Is the enable flags for this interrupt set?
			if(GET_ES && GET_REN) {
				int data = 0;
				//Call our callball function to retrieve the data
				if(i8051.serial_rx_callback)
					data = i8051.serial_rx_callback();
				//Update the register directly, since SFR_W() will trigger a serial transmit instead!
				R_SBUF=data;
				//Flag the IRQ
				SET_RI(1);
			}
			break;
			//Note: we won't call check interrupts, we'll let the main loop catch it
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
	//If All Inerrupts Disabled or no pending abort..
	if(!GET_EA)	return 0;

	//Any Interrupts Pending?
	if(NO_PENDING_IRQ) return 0;

	//Skip if current irq in progress is high priority!
	if(i8051.irq_priority)	{ /* LOG(("high priority irq in progress, skipping irq request\n")); */ return 0; }

	//Check which interrupt(s) requests have occurred..
	//NOTE: The order of checking is based on the internal/default priority levels when levels are the same

	//External Int 0
	if(GET_IE0) {
		//Set vector & priority level request
		i8051.int_vec = V_IE0;
		i8051.priority_request = GET_PX0;
	}
	//Timer 0 overflow
	if(!i8051.priority_request && GET_TF0 && (!i8051.int_vec || (i8051.int_vec && GET_PT0))) {
		//Set vector & priority level request
		i8051.int_vec = V_TF0;
		i8051.priority_request = GET_PT0;
	}
	//External Int 1
	if(!i8051.priority_request && GET_IE1 && (!i8051.int_vec || (i8051.int_vec && GET_PX1))) {
		//Set vector & priority level request
		i8051.int_vec = V_IE1;
		i8051.priority_request = GET_PX1;
	}
	//Timer 1 overflow
	if(!i8051.priority_request && GET_TF1 && (!i8051.int_vec || (i8051.int_vec && GET_PT1))) {
		//Set vector & priority level request
		i8051.int_vec = V_TF1;
		i8051.priority_request = GET_PT1;
	}
	//Serial Interrupt Transmit/Receive Interrupts (Note: ES Bit - Serial Interrupts must be enabled)
	if(!i8051.priority_request && GET_ES && (GET_TI || GET_RI) && (!i8051.int_vec || (i8051.int_vec && GET_PS))) {
		//Set vector & priority level request
		i8051.int_vec = V_RITI;
		i8051.priority_request = GET_PS;
	}
#if (HAS_I8052 || HAS_I8752)
	//Timer 2 overflow (Either Timer Overflow OR External Interrupt)
	if(!i8051.priority_request && GET_ET2 && (GET_TF2 || GET_EXF2) && (!i8051.int_vec || (i8051.int_vec && GET_PT2))) {
		//Set vector & priority level request
		i8051.int_vec = V_TF2;
		i8051.priority_request = GET_PT2;
	}
#endif

	//Skip the interrupt request if currently processing is lo priority, and the new request IS NOT HI PRIORITY!
	if(i8051.cur_irq < 0xff && !i8051.priority_request)
		{ LOG(("low priority irq in progress already, skipping low irq request\n")); return 0; }

    /*** --- Perform the interrupt --- ***/

	//Save current pc to stack, set pc to new interrupt vector
	push_pc();
	PC = i8051.int_vec;

	//Set current Irq & Priority being serviced
	i8051.cur_irq = i8051.int_vec;
	i8051.irq_priority = i8051.priority_request;

	//Clear any interrupt flags that should be cleared since we're servicing the irq!
	switch(i8051.cur_irq) {
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
#if (HAS_I8052 || HAS_I8752)
		case V_TF2:
			// no flags are cleared according to manual
			break;
#endif
	}

	//Clear vars.. (these are part of the 8051 structure for speed, so we don't have to dynamically allocate space each time)
	i8051.int_vec = 0;
	i8051.priority_request = 0;

	//All interrupts use 2 machine cycles
	return 24;
}


void i8051_set_irq_callback(int (*callback)(int irqline))
{
	i8051.irq_callback = callback;
}

void i8051_set_serial_tx_callback(void (*callback)(int data))
{
	//Hold in static variable since this function can get called before reset has run, which wipes i8051 memory clean
	hold_serial_tx_callback = callback;
}
void i8051_set_serial_rx_callback(int (*callback)(void))
{
	//Hold in static variable since this function can get called before reset has run, which wipes i8051 memory clean
	hold_serial_rx_callback = callback;
}

void i8051_set_eram_iaddr_callback(READ32_HANDLER((*callback)))
{
	//Hold in static variable since this function can get called before reset has run, which wipes i8051 memory clean
	hold_eram_iaddr_callback = callback;
}


void i8051_state_save(void *file)
{
}

void i8051_state_load(void *file)
{
}

/* HELPER FUNCTIONS */

/*All writes to SFR are handled here*/
static WRITE8_HANDLER(sfr_write)
{
	data &= 0xff;		//Ensure only 8 bits
	switch (offset)
	{
		case P0:
			R_P0 = data;
			OUT(0,data);
			break;

		case SP:
			if(offset > 0xff)
				LOG(("i8051 #%d: attemping to write value to SP past 256 bytes at 0x%04x\n", cpu_getactivecpu(), PC));
			R_SP = data&0xff; //keep sp w/in 256 bytes
			break;

		case DPL:		R_DPL = data; break;
		case DPH:		R_DPH = data; break;
		case PCON:		R_PCON= data; break;
		case TCON:		R_TCON= data; break;
		case TMOD:		R_TMOD= data; break;
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

		case IP:		R_IP  = data; break;

	//8052 Only registers
	#if (HAS_I8052 || HAS_I8752)
		case T2CON:		R_T2CON = data; break;
		case RCAP2L:	R_RCAP2L = data; break;
		case RCAP2H:	R_RCAP2H = data; break;
		case TL2:		R_TL2 = data; break;
		case TH2:		R_TH2 = data; break;
	#endif

		case PSW:
			R_PSW = data;
			SET_PARITY;
			break;

		case ACC:
			R_ACC = data;
			SET_PARITY;
			break;

		case B:			R_B   = data; break;

		/* Illegal or non-implemented sfr */
		default:
			LOG(("i8051 #%d: attemping to write to an invalid/non-implemented SFR address: %x at 0x%04x, data=%x\n", cpu_getactivecpu(), offset,PC,data));
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
	//8052 Only registers
	#if (HAS_I8052 || HAS_I8752)
		case T2CON:		return R_T2CON;
		case RCAP2L:	return R_RCAP2L;
		case RCAP2H:	return R_RCAP2H;
		case TL2:		return R_TL2;
		case TH2:		return R_TH2;
	#endif
		case PSW:		return R_PSW;
		case ACC:		return R_ACC;
		case B:			return R_B;

		/* Illegal or non-implemented sfr */
		default:
			LOG(("i8051 #%d: attemping to read an invalid/non-implemented SFR address: %x at 0x%04x\n", cpu_getactivecpu(), offset,PC));
	}
	return 0xff;
}

/* Reads the contents of the Internal RAM memory   */
/* Anything above 0x7f is a sfr/register */
static READ8_HANDLER(internal_ram_read)
{
	if (offset < 0x80)
		return i8051.IntRam[offset];
	else {
		if (offset < 0x100)
			return SFR_R(offset);
		else
			LOG(("i8051 #%d: attemping to read from an invalid Internal Ram address: %x at 0x%04x\n", cpu_getactivecpu(), offset,PC));
	}
	return 0xff;
}

/* Writes the contents of the Internal RAM memory   */
/* Anything above 0x7f is a sfr/register */
static WRITE8_HANDLER(internal_ram_write)
{
	data &= 0xff;				//Ensure it's only 8 bits
	if (offset < 0x80)
		i8051.IntRam[offset] = data;
	else {
		if (offset < 0x100)
			SFR_W(offset,data);
		else
			LOG(("i8051 #%d: attemping to write to invalid Internal Ram address: %x at 0x%04x\n", cpu_getactivecpu(), offset,PC));
	}
}

/* Reads the contents of the Internal RAM memory (BUT CALLED FROM AN INDIRECT ADDRESSING MODE)   */
/* Different chip types handle differently, for speed, simply call the chip's handler */
static READ8_HANDLER(internal_ram_iread)
{
	return i8051.iram_iread(offset);
}

/* Writes the contents of the Internal RAM memory (BUT CALLED FROM AN INDIRECT ADDRESSING MODE)   */
/* Different chip types handle differently, for speed, simply call the chip's handler */
static WRITE8_HANDLER(internal_ram_iwrite)
{
	i8051.iram_iwrite(offset,data);
}

/*Generate an external ram address for read/writing using indirect addressing mode */
/*The lowest 8 bits of the address are passed in (from the R0/R1 register), however
  the hardware can be configured to set the rest of the address lines to any available output port pins, which
  means the only way we can implement this is to allow the driver to setup a callback to generate the
  address as defined by the specific hardware setup. We'll assume the address won't be bigger than 32 bits
*/
static READ32_HANDLER(external_ram_iaddr)
{
    if(i8051.eram_iaddr_callback)
        return i8051.eram_iaddr_callback(offset,mem_mask);
    else
        LOG(("i8051 #%d: external ram address requested (8 bit offset=%02x), but no callback available! at PC:%04x\n", cpu_getactivecpu(), offset, PC));

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

READ8_HANDLER(i8051_internal_r)
{
	//Restrict internal ram to 256 Bytes max
	if(offset < 0x100)
		return IRAM_R(offset);
	else
		return 0;
}
WRITE8_HANDLER(i8051_internal_w)
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

#ifdef MAME_DEBUG
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

#ifdef MAME_DEBUG
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

#if (HAS_I8052 || HAS_I8752)
	//Update Timer 2
	if(GET_TR2) {
		int timerinc, overflow, isoverflow;
		UINT16 count = ((R_TH2<<8) | R_TL2);
		timerinc = overflow = isoverflow = 0;

		//Are we in counter mode?
		if(GET_CT2)		{
			//Not supported
		}
		//Are we in timer mode?
		else {
			//16 Bit Timer Mode
			overflow = 0xffff;
			//Timer 2 Used as Baud Generator? (For now, only *same* send/receive rates supported)
			if(GET_TCLK || GET_RCLK)
				timerinc = cyc/2;						//Timer increments ever 1/2 cycle in baud mode
			else
			//REGULAR TIMER -
				timerinc = cyc/12;						//Timer increments ever 1/12 cycles in normal mode

			//Check for overflow
			if((UINT32)(count+timerinc)>overflow) {
				//Set Interrupt flag *unless* used as baud generator
				if(!GET_TCLK && !GET_RCLK) {
					SET_TF2(1);
				}
				else {
				//Update bits sent if sending & bits left to send!
					if(uart.sending && uart.bits_to_send && uart.timerbaud)
						uart.bits_to_send-=1;
				}
				//Auto reload?
				if(!GET_CP)
					count = ((R_RCAP2H<<8) | R_RCAP2L); //+(overflow-count);
				else
					count = overflow-count;
			}
			else {
			//No overflow, just increment timer
				count+=timerinc;
			}
			//Update flags
			R_TH2 = (count>>8) & 0xff;
			R_TL2 = count & 0xff;
		}
	}
#endif
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
			LOG(("Serial mode 2 & 3 not supported in i8051!\n"));
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
		if(i8051.serial_tx_callback)
			i8051.serial_tx_callback(uart.data_out);
		//Set Interrupt Flag
		SET_TI(1);
		//Note: we'll let the main loop catch the interrupt
	}
}


/****************************************************************************
 * 8752 Section
 ****************************************************************************/
#if (HAS_I8052 || HAS_I8752)
void i8752_init (int index, int clock, const void *config, int (*irqcallback)(int))	{ i8051_init(index, clock, config, irqcallback); }
void i8752_reset (void)
{
	memset(&i8051, 0, sizeof(I8051));
	memset(&uart, 0, sizeof(I8051_UART));
	i8051.subtype = 8752;

	//Set up 8052 specific internal read/write (indirect) handlers..
	i8051.iram_iread  = i8052_internal_ram_iread;
	i8051.iram_iwrite = i8052_internal_ram_iwrite;

	//Set up serial call back handlers
	i8051.serial_tx_callback = hold_serial_tx_callback;
	hold_serial_tx_callback = NULL;
	i8051.serial_rx_callback = hold_serial_rx_callback;
	hold_serial_rx_callback = NULL;

	//Setup External ram callback handlers
	i8051.eram_iaddr_callback = hold_eram_iaddr_callback;
	hold_eram_iaddr_callback = NULL;

	//Clear Ram (w/0xff)
	memset(&i8051.IntRam,0xff,sizeof(i8051.IntRam));

	/* these are all defined reset states */
	PC = 0;
	SFR_W(SP, 0x7);
	SFR_W(PSW, 0);
	SFR_W(DPH, 0);
	SFR_W(DPL, 0);
	SFR_W(ACC, 0);
	SFR_W(B, 0);
	SFR_W(IP, 0);
	SFR_W(IE, 0);
	SFR_W(SCON, 0);
	SFR_W(TCON, 0);
	SFR_W(TMOD, 0);
	SFR_W(TH1, 0);
	SFR_W(TH0, 0);
	SFR_W(TL1, 0);
	SFR_W(TL0, 0);
	//8052 Only registers
	SFR_W(T2CON, 0);
	SFR_W(RCAP2L, 0);
	SFR_W(RCAP2H, 0);
	SFR_W(TL2, 0);
	SFR_W(TH2, 0);

	/* set the port configurations to all 1's */
	SFR_W(P3, 0xff);
	SFR_W(P2, 0xff);
	SFR_W(P1, 0xff);
	SFR_W(P0, 0xff);

	/* Flag as NO IRQ in Progress */
	CLEAR_CURRENT_IRQ
}

void i8752_exit	(void)						{ i8051_exit(); }
int	i8752_execute(int cycles)				{ return i8051_execute(cycles); }
void i8752_get_context (void *dst)				{ i8051_get_context(dst); }
void i8752_set_context (void *src)				{ i8051_set_context(src); }
unsigned i8752_get_reg (int regnum)				{ return i8051_get_reg(regnum); }
void i8752_set_reg (int regnum, unsigned val)			{ i8051_set_reg(regnum,val); }
void i8752_set_irq_line(int irqline, int state)			{ i8051_set_irq_line(irqline,state); }
void i8752_set_irq_callback(int (*callback)(int irqline))	{ i8051_set_irq_callback(callback); }
void i8752_set_serial_tx_callback(void (*callback)(int data))	{ i8051_set_serial_tx_callback(callback); }
void i8752_set_serial_rx_callback(int (*callback)(void))	{ i8051_set_serial_rx_callback(callback); }
void i8752_state_save(void *file)				{ i8051_state_save(file); }
void i8752_state_load(void *file)				{ i8051_state_load(file); }

/* The following two handlers are used by the MAME Debugger Memory Window...
   By keeping these functions separate from the internally used IRAM_W/IRAM_R functions,
   we can manipulate and display internal memory in the debugger memory window in a layout
   that is not necessarily how the real memory is.. this will be especially useful for
   the 8052 chip where both the SFR and the upper 128 bytes of ram are mapped to the same
   address, so we can handle that here by mapping the sfr to a different address */

READ8_HANDLER(i8752_internal_r)
{
	//USE INDIRECT READ TO ALLOW FULL 256 Bytes of RAM to show in the debugger
	if(offset < 0x100)
		return IRAM_IR(offset);
	else
	//MAP SFR registers starting at 256 (they are only 128 bytes in size)
	if(offset < 0x100+0x80)
		return SFR_R(offset-0x80);
	else
	//Everything else is 0 (and invalid)
		return 0;
}
WRITE8_HANDLER(i8752_internal_w)
{
	//USE INDIRECT WRITE TO ALLOW FULL 256 Bytes of RAM to show in the debugger
	if(offset < 0x100)
		IRAM_IW(offset,data);
	else
		if(offset < 0x100+0x80)
			SFR_W(offset-0x80,data);
}

/* Reads the contents of the Internal RAM memory INDIRECTLY  */
/* Anything above 0x7f is NOT sfr/register, but rather UPPER 128K OF INTERNAL RAM */
static READ8_HANDLER(i8052_internal_ram_iread)
{
	if (offset < 0x100)
		return i8051.IntRam[offset];
	else
		LOG(("i8051 #%d: attemping to read from an invalid Internal Ram address: %x at 0x%04x\n", cpu_getactivecpu(), offset,PC));
	return 0xff;
}

/* Writes the contents of the Internal RAM memory INDIRECTLY */
/* Anything above 0x7f is NOT sfr/register, but rather UPPER 128K OF INTERNAL RAM */
static WRITE8_HANDLER(i8052_internal_ram_iwrite)
{
	data &= 0xff;				//Ensure it's only 8 bits
	if (offset < 0x100)
		i8051.IntRam[offset] = data;
	else
		LOG(("i8051 #%d: attemping to write to an invalid Internal Ram address: %x at 0x%04x\n", cpu_getactivecpu(), offset,PC));
}

#endif	//(HAS_8752)

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void i8051_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:							PC = info->i;							break;
		case CPUINFO_INT_SP:							i8051.sp = info->i;	     				break;

		case CPUINFO_INT_INPUT_STATE + I8051_INT0_LINE:	i8051_set_irq_line(I8051_INT0_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + I8051_INT1_LINE:	i8051_set_irq_line(I8051_INT1_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + I8051_T0_LINE:	i8051_set_irq_line(I8051_T0_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + I8051_T1_LINE:	i8051_set_irq_line(I8051_T1_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + I8051_RX_LINE:	i8051_set_irq_line(I8051_RX_LINE, info->i); break;

		case CPUINFO_INT_REGISTER + I8051_PC: 			PC = info->i;							break;
		case CPUINFO_INT_REGISTER + I8051_SP: 			R_SP = info->i;							break;
		case CPUINFO_INT_REGISTER + I8051_PSW:			i8051.psw = info->i;					break;
		case CPUINFO_INT_REGISTER + I8051_ACC:			i8051.acc = info->i;					break;
		case CPUINFO_INT_REGISTER + I8051_B:  			i8051.b = info->i;						break;
		case CPUINFO_INT_REGISTER + I8051_DPH:			i8051.dph = info->i;					break;
		case CPUINFO_INT_REGISTER + I8051_DPL:			i8051.dpl = info->i;					break;
		case CPUINFO_INT_REGISTER + I8051_IE: 			i8051.ie = info->i;						break;
		case CPUINFO_INT_REGISTER + I8051_R0: 			i8051.IntRam[0+(8*((i8051.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + I8051_R1: 			i8051.IntRam[1+(8*((i8051.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + I8051_R2: 			i8051.IntRam[2+(8*((i8051.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + I8051_R3: 			i8051.IntRam[3+(8*((i8051.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + I8051_R4: 			i8051.IntRam[4+(8*((i8051.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + I8051_R5: 			i8051.IntRam[5+(8*((i8051.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + I8051_R6: 			i8051.IntRam[6+(8*((i8051.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + I8051_R7: 			i8051.IntRam[7+(8*((i8051.psw & 0x18)>>3))] = info->i; break;
		case CPUINFO_INT_REGISTER + I8051_RB: 			i8051.IntRam[8+(8*((i8051.psw & 0x18)>>3))] = info->i; break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

void i8051_get_info(UINT32 state, cpuinfo *info)
{
	I8051 *r = &i8051;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i8051);				break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 20; /* rough guess */			break;
		case CPUINFO_INT_INPUT_LINES:        			info->i = 3;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 8;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 	info->i = 16;						break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 	info->i = 0;						break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;
		case CPUINFO_INT_PC:	 						info->i = PC;							break;
		case CPUINFO_INT_SP:							info->i = R_SP;							break;

		case CPUINFO_INT_REGISTER + I8051_PC: 			info->i = PC;							break;
		case CPUINFO_INT_REGISTER + I8051_SP: 			info->i = R_SP;							break;
		case CPUINFO_INT_REGISTER + I8051_PSW:			info->i = i8051.psw;					break;
		case CPUINFO_INT_REGISTER + I8051_ACC:			info->i = i8051.acc;					break;
		case CPUINFO_INT_REGISTER + I8051_B:  			info->i = i8051.b;						break;
		case CPUINFO_INT_REGISTER + I8051_DPH:			info->i = i8051.dph;					break;
		case CPUINFO_INT_REGISTER + I8051_DPL:			info->i = i8051.dpl;					break;
		case CPUINFO_INT_REGISTER + I8051_IE: 			info->i = i8051.ie;						break;
		case CPUINFO_INT_REGISTER + I8051_R0: 			info->i = i8051.IntRam[0+(8*((i8051.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + I8051_R1: 			info->i = i8051.IntRam[1+(8*((i8051.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + I8051_R2: 			info->i = i8051.IntRam[2+(8*((i8051.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + I8051_R3: 			info->i = i8051.IntRam[3+(8*((i8051.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + I8051_R4: 			info->i = i8051.IntRam[4+(8*((i8051.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + I8051_R5: 			info->i = i8051.IntRam[5+(8*((i8051.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + I8051_R6: 			info->i = i8051.IntRam[6+(8*((i8051.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + I8051_R7: 			info->i = i8051.IntRam[7+(8*((i8051.psw & 0x18)>>3))]; break;
		case CPUINFO_INT_REGISTER + I8051_RB: 			info->i = i8051.IntRam[8+(8*((i8051.psw & 0x18)>>3))]; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = i8051_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = i8051_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = i8051_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = i8051_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = i8051_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = i8051_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = i8051_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = i8051_dasm;			break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &i8051_icount;			break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map = 0;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:    info->internal_map = 0;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_IO:      info->internal_map = 0;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "I8051");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "MCS-51");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 2003-2004 Steve Ellenoff"); break;

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

		case CPUINFO_STR_REGISTER + I8051_PC:        	sprintf(info->s, "PC:%04X", r->pc);		break;
		case CPUINFO_STR_REGISTER + I8051_SP:        	sprintf(info->s, "SP:%02X", r->sp);		break;
		case CPUINFO_STR_REGISTER + I8051_PSW:       	sprintf(info->s, "PSW:%02X", r->psw);	break;
		case CPUINFO_STR_REGISTER + I8051_ACC:       	sprintf(info->s, "A:%02X", r->acc);		break;
		case CPUINFO_STR_REGISTER + I8051_B:         	sprintf(info->s, "B:%02X", r->b);		break;
		case CPUINFO_STR_REGISTER + I8051_DPH:       	sprintf(info->s, "DPH:%02X", r->dph);	break;
		case CPUINFO_STR_REGISTER + I8051_DPL:       	sprintf(info->s, "DPL:%02X", r->dpl);	break;
		case CPUINFO_STR_REGISTER + I8051_IE:        	sprintf(info->s, "IE:%02X", r->ie);		break;
		case CPUINFO_STR_REGISTER + I8051_R0:        	sprintf(info->s, "R0:%02X", r->IntRam[0+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + I8051_R1:        	sprintf(info->s, "R1:%02X", r->IntRam[1+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + I8051_R2:        	sprintf(info->s, "R2:%02X", r->IntRam[2+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + I8051_R3:			sprintf(info->s, "R3:%02X", r->IntRam[3+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + I8051_R4: 			sprintf(info->s, "R4:%02X", r->IntRam[4+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + I8051_R5: 			sprintf(info->s, "R5:%02X", r->IntRam[5+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + I8051_R6: 			sprintf(info->s, "R6:%02X", r->IntRam[6+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + I8051_R7: 			sprintf(info->s, "R7:%02X", r->IntRam[7+(8*((r->psw & 0x18)>>3))]); break;
		case CPUINFO_STR_REGISTER + I8051_RB: 			sprintf(info->s, "RB:%02X", ((r->psw & 0x18)>>3)); break;
	}
}

#if (HAS_I8052)
void i8052_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = i8752_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = i8752_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = i8752_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = i8752_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = i8752_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = i8752_execute;			break;

		case CPUINFO_STR_NAME:							strcpy(info->s, "I8052");				break;

		default:										i8051_get_info(state, info);			break;
	}
}
#endif

#if (HAS_I8751)
void i8751_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_STR_NAME:							strcpy(info->s, "I8751");				break;

		default:										i8051_get_info(state, info);			break;
	}
}
#endif

#if (HAS_I8752)
void i8752_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = i8752_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = i8752_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = i8752_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = i8752_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = i8752_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = i8752_execute;			break;

		case CPUINFO_STR_NAME:							strcpy(info->s, "I8752");				break;

		default:										i8051_get_info(state, info);			break;
	}
}
#endif
