// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force SYS68K CPU-30 VME SBC drivers
 *
 *  21/05/2016
 *
 * Thanks to Al Kossow his site http://www.bitsavers.org/ I got the information
 * required to start the work with this driver.
 *
 * The driver is currently starting up and the Boot ROM asks for input do start FGA-002
 * diagnostics and do SRAM setup, which it does. After that it jumps to zeroed memory
 * and crashes so needs some more work to be useful
 *
 *
 *       ||
 * ||    ||  CPU-30
 * ||||--||_____________________________________________________________
 * ||||--||                                                             |
 * ||    ||                                                           _ |__
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |VME|
 *       ||                                                          | |   |
 *       ||                                                          | |P1 |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          |_|   |
 *       ||                                                            |___|
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |___
 *       ||                                                           _|   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |VME|
 *       ||                                                          | |   |
 *       ||                                                          | |P2 |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          |_|   |
 *       ||                                                            |___|
 * ||    ||                                                              +
 * ||||--||                                                              |
 * ||||--||--------------------------------------------------------------+
 * ||
 *
 * History of Force Computers
 *---------------------------
 *
 * Misc links about Force Computes and this board:
 *------------------------------------------------
 * http://bitsavers.trailing-edge.com/pdf/forceComputers/CPU30/204030_CPU-30_R4_Technical_Reference_Oct96.pdf
 * http://www.artisantg.com/info/P_wUovN.pdf
 *
 * Description(s)
 * -------------
 * CPU-30 has the following feature set
 *      -  16.7 or 25 MHz MC68030 enhanced 32-bit microprocessor
 *      -  16.7 or 25 MHz MC68882 floating-point coprocessor
 *      -  32-512 Kb of SRAM with battery backup
 *      -  4, 8, 16, or 32MB of shared DRAM, with byte parity
 *      -  Up to 8Mb Flash memory
 *      -  128,256 or 512 Mb boot flash or upto 1Mb of boot OTP PROM
 *      -  Double High (6U) VMEmodule
 *      -  A32/D32 VMEbus master/slave interface with system controller function (VMEchip ASIC)
 *      -  Ethernet transceiver interface (AM79C90)
 *      -  SCSI bus interface with independent data bus on P2 connector (MB87033/34)
 *      -  Flopyy disk interface on P2 connector (FCD37C65C)
 *      -  Four serial ports (DUSCC SCN68562 x 2)
 *      -  20 bit digital i/o for user applications( 2 x 68230 PI/T )
 *      -  Real-Time Clock with interrupt (72423)
 *      -  4-level requester, 7-level interrupter, and 7-level interrupt handler for VMEbus (VMEchip ASIC)
 *
 * NOTE: This driver currently mimics the CPU-30xyz configuration: 16MHz, 4Mb RAM, no parity, no ethernet (See TODO)
 *
 * Address Map
 * --------------------------------------------------------------------------
 *  Range                   Decscription
 * --------------------------------------------------------------------------
 * 00000000-0xxFFFFF        Shared DRAM D8-D32 xx=0x1F-0x03 for 32Mb-4Mb
 * 0yy00000-FAFFFFFF        VME A32 D8-D32     yy=xx+1
 * FB000000-FBFEFFFF        VME A24 D8-D32
 * FBFF0000-FBFFFFFF        VME A16 D8-D32
 * FC000000-FCFEFFFF        VME A24 D8-D16
 * FCFF0000-FCFFFFFF        VME A16 D8-D16
 * FD000000-FEEFFFFF        Reserved
 * FEF00000-FEF7FFFF        LAN RAM D8-D32
 * FEF80000-FEFFFFFF        LAN Controller D16 (AM79C90)
 * FF000000-FF7FFFFF        System PROM D8-D32 (read) D32 (flash write)
 * FF800000-FF800BFF        Reserved
 * FF800C00-FF800DFF        PIT1 D8 (68230)
 * FF800E00-FF800FFF        PIT2 D8 (68230)
 * FF801000-FF801FFF        Reserved
 * FF802000-FF8021FF        DUSCC1 D8 (SCN68562)
 * FF802200-FF8023FF        DUSCC2 D8 (SCN68562)
 * FF802400-FF802FFF        Reserved
 * FF803000-FF8031FF        RTC (72423) D8
 * FF803200-FF8033FF        Reserved
 * FF803400-FF8035FF        SCSI controller (MB87033/34) D8
 * FF803600-FF8037FF        Reserved
 * FF803800-FF80397F        Floppy controller (FDC37C65C) D8
 * FF803980-FF8039FF        Slot 1 status register (read) D8
 * FFC00000-FFCFFFFF        Local SRAM D8-D32
 * FFD00000-FFDFFFFF        FGA-002 Gate Array D8-D32
 * FFE00000-FFEFFFFF        Boot PROM D8-D32
 * FFF00000-FFFFFFFF        Reserved
 * --------------------------------------------------------------------------
 *
 * Interrupt sources MVME
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources
 *
 * Off board Sources (other VME boards)
 *
 * ----------------------------------------------------------
 *
 * DMAC Channel Assignments
 * ----------------------------------------------------------
 * Channel         MVME147
 * ----------------------------------------------------------
 *
 *
 *  TODO:
 *  - Investigate and fix crash
 *  - Add VxWorks proms
 *  - Add more devices
 *  - Write VME device
 *  - Add variants of boards
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/scnxx562.h"
#include "machine/68230pit.h"
#include "machine/nvram.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
//#include "machine/timekpr.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#if VERBOSE >= 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define DUSCC_CLOCK XTAL_14_7456MHz /* Verified */

class fccpu30_state : public driver_device
{
public:
fccpu30_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device (mconfig, type, tag),
		m_maincpu (*this, "maincpu")
		,m_dusccterm(*this, "duscc")
		,m_pit1 (*this, "pit1")
		,m_pit2 (*this, "pit2")
	{
	}

	DECLARE_READ32_MEMBER (bootvect_r);
	DECLARE_WRITE32_MEMBER (bootvect_w);
	/* FGA-002 - Force Gate Array  */
	DECLARE_READ8_MEMBER (fga8_r);
	DECLARE_WRITE8_MEMBER (fga8_w);

	/* Rotary switch PIT input */
	DECLARE_READ8_MEMBER (rotary_rd);
	DECLARE_READ8_MEMBER (board_mem_id_rd);

	/* VME bus accesses */
	//DECLARE_READ16_MEMBER (vme_a24_r);
	//DECLARE_WRITE16_MEMBER (vme_a24_w);
	//DECLARE_READ16_MEMBER (vme_a16_r);
	//DECLARE_WRITE16_MEMBER (vme_a16_w);
	virtual void machine_start () override;
	virtual void machine_reset () override;
protected:

private:
	required_device<cpu_device> m_maincpu;
	required_device<duscc68562_device> m_dusccterm;

	required_device<pit68230_device> m_pit1;
	required_device<pit68230_device> m_pit2;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	UINT32  *m_sysrom;
	UINT32  m_sysram[2];

	// FGA-002
	UINT8   m_fga002[0x500];
};

static ADDRESS_MAP_START (fccpu30_mem, AS_PROGRAM, 32, fccpu30_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00000000, 0x00000007) AM_ROM AM_READ  (bootvect_r)   /* ROM mirror just during reset */
	AM_RANGE (0x00000000, 0x00000007) AM_RAM AM_WRITE (bootvect_w)   /* After first write we act as RAM */
	AM_RANGE (0x00000008, 0x003fffff) AM_RAM /* 4 Mb RAM */
	AM_RANGE (0xff000000, 0xff7fffff) AM_ROM AM_REGION("maincpu", 0xff000000)
	AM_RANGE (0xff802000, 0xff8021ff) AM_DEVREADWRITE8("duscc", duscc68562_device, read, write, 0xffffffff) /* Port 1&2 - Dual serial port DUSCC   */
	AM_RANGE (0xff800c00, 0xff800dff) AM_DEVREADWRITE8("pit1", pit68230_device, read, write, 0xffffffff)
	AM_RANGE (0xff800e00, 0xff800fff) AM_DEVREADWRITE8("pit2", pit68230_device, read, write, 0xffffffff)
	AM_RANGE (0xffc00000, 0xffcfffff) AM_RAM AM_SHARE ("nvram") /* On-board SRAM with battery backup (nvram) */
	AM_RANGE (0xffd00000, 0xffd004ff) AM_READWRITE8(fga8_r, fga8_w, 0xffffffff)  /* FGA-002 Force Gate Array */
	AM_RANGE (0xffe00000, 0xffefffff) AM_ROM AM_REGION("maincpu", 0xffe00000)

	//AM_RANGE(0x100000, 0xfeffff)  AM_READWRITE(vme_a24_r, vme_a24_w) /* VMEbus Rev B addresses (24 bits) - not verified */
	//AM_RANGE(0xff0000, 0xffffff)  AM_READWRITE(vme_a16_r, vme_a16_w) /* VMEbus Rev B addresses (16 bits) - not verified */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (fccpu30)
INPUT_PORTS_END

#define FGA_ICRMBOX0 0x0000
#define FGA_ICRMBOX1 0x0004
#define FGA_ICRMBOX2 0x0008
#define FGA_ICRMBOX3 0x000c
#define FGA_ICRMBOX4 0x0010
#define FGA_ICRMBOX5 0x0014
#define FGA_ICRMBOX6 0x0018
#define FGA_ICRMBOX7 0x001C
#define FGA_VMEPAGE  0x0200
#define FGA_ICRVME1 0x0204
#define FGA_ICRVME2 0x0208
#define FGA_ICRVME3 0x020c
#define FGA_ICRVME4 0x0210
#define FGA_ICRVME5 0x0214
#define FGA_ICRVME6 0x0218
#define FGA_ICRVME7 0x021c
#define FGA_ICRTIM0 0x0220
#define FGA_ICRDMANORM 0x0230
#define FGA_ICRDMAERR 0x0234
#define FGA_CTL1 0x0238
#define FGA_CTL2 0x023c
#define FGA_ICRFMB0REF 0x0240
#define FGA_ICRFMB1REF 0x0244
#define FGA_ICRFMB0MES 0x0248
#define FGA_ICRFMB1MES 0x024c
#define FGA_CTL3 0x0250
#define FGA_CTL4 0x0254
#define FGA_ICRPARITY 0x0258
#define FGA_AUXPINCTL 0x0260
#define FGA_CTL5 0x0264
#define FGA_AUXFIFWEX 0x0268
#define FGA_AUXFIFREX 0x026c
#define FGA_CTL6 0x0270
#define FGA_CTL7 0x0274
#define FGA_CTL8 0x0278
#define FGA_CTL9 0x027c
#define FGA_ICRABORT 0x0280
#define FGA_ICRACFAIL 0x0284
#define FGA_ICRSYSFAIL 0x0288
#define FGA_ICRLOCAL0 0x028c
#define FGA_ICRLOCAL1 0x0290
#define FGA_ICRLOCAL2 0x0294
#define FGA_ICRLOCAL3 0x0298
#define FGA_ICRLOCAL4 0x029c
#define FGA_ICRLOCAL5 0x02a0
#define FGA_ICRLOCAL6 0x02a4
#define FGA_ICRLOCAL7 0x02a8
#define FGA_ENAMCODE 0x02b4
#define FGA_CTL10 0x02c0
#define FGA_CTL11 0x02c4
#define FGA_MAINUM 0x02c8
#define FGA_MAINUU 0x02cc
#define FGA_BOTTOMPAGEU 0x02d0
#define FGA_BOTTOMPAGEL 0x02d4
#define FGA_TOPPAGEU 0x02d8
#define FGA_TOPPAGEL 0x02dc
#define FGA_MYVMEPAGE 0x02fc
#define FGA_TIM0PRELOAD 0x0300
#define FGA_TIM0CTL 0x0310
#define FGA_DMASRCATT 0x0320
#define FGA_DMADSTATT 0x0324
#define FGA_DMA_GENERAL 0x0328
#define FGA_CTL12 0x032c
#define FGA_LIOTIMING 0x0330
#define FGA_LOCALIACK 0x0334
#define FGA_FMBCTL 0x0338
#define FGA_FMBAREA 0x033c
#define FGA_AUXSRCSTART 0x0340
#define FGA_AUXDSTSTART 0x0344
#define FGA_AUXSRCTERM 0x0348
#define FGA_AUXDSTTERM 0x034c
#define FGA_CTL13 0x0350
#define FGA_CTL14 0x0354
#define FGA_CTL15 0x0358
#define FGA_CTL16 0x035c
#define FGA_SPECIALENA 0x0424
#define FGA_ISTIM0 0x04a0
#define FGA_ISDMANORM 0x04b0
#define FGA_ISDMAERR 0x04b4
#define FGA_ISFMB0REF 0x04b8
#define FGA_ISFMB1REF 0x04bc
#define FGA_ISPARITY 0x04c0
#define FGA_DMARUNCTL 0x04c4
#define FGA_ISABORT 0x04c8
#define FGA_ISACFAIL 0x04cc
#define FGA_ISFMB0MES 0x04e0
#define FGA_ISFMB1MES 0x04e4
#define FGA_ISSYSFAIL 0x04d0
#define FGA_ABORTPIN 0x04d4
#define FGA_RSVMECALL 0x04f0
#define FGA_RSKEYRES 0x04f4
#define FGA_RSCPUCALL 0x04f8
#define FGA_RSLOCSW 0x04fc

/* Start it up */
void fccpu30_state::machine_start ()
{
	LOG(("--->%s\n", FUNCNAME));

	save_pointer (NAME (m_sysrom), sizeof(m_sysrom));
	save_pointer (NAME (m_sysram), sizeof(m_sysram));
	save_pointer (NAME (m_fga002), sizeof(m_fga002));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (UINT32*)(memregion ("maincpu")->base () + 0xffe00000);
}

void fccpu30_state::machine_reset ()
{
	LOG(("--->%s\n", FUNCNAME));

	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (UINT32*)(memregion ("maincpu")->base () + 0xffe00000);

	/* Reset values for the FGA-002 */
	memset(&m_fga002[0], 0, sizeof(m_fga002));
	m_fga002[FGA_RSVMECALL]     = 0x80;
	m_fga002[FGA_RSKEYRES]      = 0x80;
	m_fga002[FGA_RSCPUCALL]     = 0x80;
	m_fga002[FGA_RSLOCSW]       = 0x80;
	m_fga002[FGA_ISTIM0]        = 0x80;
	m_fga002[FGA_ISDMANORM]     = 0x80;
	m_fga002[FGA_ISDMAERR]      = 0x80;
	m_fga002[FGA_ISFMB0REF]     = 0x80;
	m_fga002[FGA_ISFMB1REF]     = 0x80;
	m_fga002[FGA_ISPARITY]      = 0x80;
	m_fga002[FGA_ISABORT]       = 0x80;
	m_fga002[FGA_ISACFAIL]      = 0x80;
	m_fga002[FGA_ISSYSFAIL]     = 0x80;
	m_fga002[FGA_ISFMB0MES]     = 0x80;
	m_fga002[FGA_ISFMB1MES]     = 0x80;
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset*/
READ32_MEMBER (fccpu30_state::bootvect_r){
	return m_sysrom[offset];
}

WRITE32_MEMBER (fccpu30_state::bootvect_w){
	m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
	m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcomming accesses to masking RAM until reset.
}

/*
 * FGA-002 driver, might deserve its own driver but will rest here until another board wants it
 *
The FGA-002 gate array is a high speed CMOS device manufactured in 1.2 micron technology and
containing 24,000 gates in a 281 pin PGA package. It provides interfaces to the 68020/30 microprocessor
as well as a VMEbus compatible interface. The auxilary interface of the gate array is a high speed data
channel used by the internal 32 bit DMA controller. The interface allows data transfer rates of up to
6 MByte/second. The timing of the local I/O interface is programmable and provides easy interfacing of
local I/O devices. All control, address and data lines of the CPU and the VMEbus are either directly
connected or connected via buffers to the gate array allowing easy implementation and usage.
The gate array registers are programmed by the local CPU.

FEATURES:
- Programmable decoding for CPU and VME access to the local main memory
- Interrupt management for internal and external interrupt sources
- 32 bit multi-port DMA Controller
- FORCE Message Broadcast slave interface with 2 message channels
- 8 interrupt capable MAILBOXES
- 8 bit TIMER with 16 selectable internal source clocks
*/
WRITE8_MEMBER (fccpu30_state::fga8_w){
	LOG(("%s[%04x] <- %02x    - ", FUNCNAME, offset, data));
	switch(offset)
	{
	case FGA_SPECIALENA     : LOG(("FGA_SPECIALENA - not implemented\n")); m_fga002[FGA_SPECIALENA] = data; break;
	case FGA_RSVMECALL      : LOG(("FGA_RSVMECALL - not implemented\n")); m_fga002[FGA_RSVMECALL] = data; break;
	case FGA_RSKEYRES       : LOG(("FGA_RSKEYRES - not implemented\n")); m_fga002[FGA_RSKEYRES] = data; break;
	case FGA_RSCPUCALL      : LOG(("FGA_RSCPUCALL - not implemented\n")); m_fga002[FGA_RSCPUCALL] = data; break;
	case FGA_RSLOCSW        : LOG(("FGA_RSLOCSW - not implemented\n")); m_fga002[FGA_RSLOCSW] = data; break;
	case FGA_ICRMBOX0       : LOG(("FGA_ICRMBOX0 - not implemented\n")); m_fga002[FGA_ICRMBOX0] = data; break;
	case FGA_ICRMBOX1       : LOG(("FGA_ICRMBOX1 - not implemented\n")); m_fga002[FGA_ICRMBOX1] = data; break;
	case FGA_ICRMBOX2       : LOG(("FGA_ICRMBOX2 - not implemented\n")); m_fga002[FGA_ICRMBOX2] = data; break;
	case FGA_ICRMBOX3       : LOG(("FGA_ICRMBOX3 - not implemented\n")); m_fga002[FGA_ICRMBOX3] = data; break;
	case FGA_ICRMBOX4       : LOG(("FGA_ICRMBOX4 - not implemented\n")); m_fga002[FGA_ICRMBOX4] = data; break;
	case FGA_ICRMBOX5       : LOG(("FGA_ICRMBOX5 - not implemented\n")); m_fga002[FGA_ICRMBOX5] = data; break;
	case FGA_ICRMBOX6       : LOG(("FGA_ICRMBOX6 - not implemented\n")); m_fga002[FGA_ICRMBOX6] = data; break;
	case FGA_ICRMBOX7       : LOG(("FGA_ICRMBOX7 - not implemented\n")); m_fga002[FGA_ICRMBOX7] = data; break;
	case FGA_VMEPAGE        : LOG(("FGA_VMEPAGE  - not implemented\n")); m_fga002[FGA_VMEPAGE ] = data; break;
	case FGA_ICRVME1        : LOG(("FGA_ICRVME1 - not implemented\n")); m_fga002[FGA_ICRVME1] = data; break;
	case FGA_ICRVME2        : LOG(("FGA_ICRVME2 - not implemented\n")); m_fga002[FGA_ICRVME2] = data; break;
	case FGA_ICRVME3        : LOG(("FGA_ICRVME3 - not implemented\n")); m_fga002[FGA_ICRVME3] = data; break;
	case FGA_ICRVME4        : LOG(("FGA_ICRVME4 - not implemented\n")); m_fga002[FGA_ICRVME4] = data; break;
	case FGA_ICRVME5        : LOG(("FGA_ICRVME5 - not implemented\n")); m_fga002[FGA_ICRVME5] = data; break;
	case FGA_ICRVME6        : LOG(("FGA_ICRVME6 - not implemented\n")); m_fga002[FGA_ICRVME6] = data; break;
	case FGA_ICRVME7        : LOG(("FGA_ICRVME7 - not implemented\n")); m_fga002[FGA_ICRVME7] = data; break;
	case FGA_ICRTIM0        : LOG(("FGA_ICRTIM0 - not implemented\n")); m_fga002[FGA_ICRTIM0] = data; break;
	case FGA_ICRDMANORM     : LOG(("FGA_ICRDMANORM - not implemented\n")); m_fga002[FGA_ICRDMANORM] = data; break;
	case FGA_ICRDMAERR      : LOG(("FGA_ICRDMAERR - not implemented\n")); m_fga002[FGA_ICRDMAERR] = data; break;
	case FGA_CTL1           : LOG(("FGA_CTL1 - not implemented\n")); m_fga002[FGA_CTL1] = data; break;
	case FGA_CTL2           : LOG(("FGA_CTL2 - not implemented\n")); m_fga002[FGA_CTL2] = data; break;
	case FGA_ICRFMB0REF     : LOG(("FGA_ICRFMB0REF - not implemented\n")); m_fga002[FGA_ICRFMB0REF] = data; break;
	case FGA_ICRFMB1REF     : LOG(("FGA_ICRFMB1REF - not implemented\n")); m_fga002[FGA_ICRFMB1REF] = data; break;
	case FGA_ICRFMB0MES     : LOG(("FGA_ICRFMB0MES - not implemented\n")); m_fga002[FGA_ICRFMB0MES] = data; break;
	case FGA_ICRFMB1MES     : LOG(("FGA_ICRFMB1MES - not implemented\n")); m_fga002[FGA_ICRFMB1MES] = data; break;
	case FGA_CTL3           : LOG(("FGA_CTL3 - not implemented\n")); m_fga002[FGA_CTL3] = data; break;
	case FGA_CTL4           : LOG(("FGA_CTL4 - not implemented\n")); m_fga002[FGA_CTL4] = data; break;
	case FGA_ICRPARITY      : LOG(("FGA_ICRPARITY - not implemented\n")); m_fga002[FGA_ICRPARITY] = data; break;
	case FGA_AUXPINCTL      : LOG(("FGA_AUXPINCTL - not implemented\n")); m_fga002[FGA_AUXPINCTL] = data; break;
	case FGA_CTL5           : LOG(("FGA_CTL5 - not implemented\n")); m_fga002[FGA_CTL5] = data; break;
	case FGA_AUXFIFWEX      : LOG(("FGA_AUXFIFWEX - not implemented\n")); m_fga002[FGA_AUXFIFWEX] = data; break;
	case FGA_AUXFIFREX      : LOG(("FGA_AUXFIFREX - not implemented\n")); m_fga002[FGA_AUXFIFREX] = data; break;
	case FGA_CTL6           : LOG(("FGA_CTL6 - not implemented\n")); m_fga002[FGA_CTL6] = data; break;
	case FGA_CTL7           : LOG(("FGA_CTL7 - not implemented\n")); m_fga002[FGA_CTL7] = data; break;
	case FGA_CTL8           : LOG(("FGA_CTL8 - not implemented\n")); m_fga002[FGA_CTL8] = data; break;
	case FGA_CTL9           : LOG(("FGA_CTL9 - not implemented\n")); m_fga002[FGA_CTL9] = data; break;
	case FGA_ICRABORT       : LOG(("FGA_ICRABORT - not implemented\n")); m_fga002[FGA_ICRABORT] = data; break;
	case FGA_ICRACFAIL      : LOG(("FGA_ICRACFAIL - not implemented\n")); m_fga002[FGA_ICRACFAIL] = data; break;
	case FGA_ICRSYSFAIL     : LOG(("FGA_ICRSYSFAIL - not implemented\n")); m_fga002[FGA_ICRSYSFAIL] = data; break;
	case FGA_ICRLOCAL0      : LOG(("FGA_ICRLOCAL0 - not implemented\n")); m_fga002[FGA_ICRLOCAL0] = data; break;
	case FGA_ICRLOCAL1      : LOG(("FGA_ICRLOCAL1 - not implemented\n")); m_fga002[FGA_ICRLOCAL1] = data; break;
	case FGA_ICRLOCAL2      : LOG(("FGA_ICRLOCAL2 - not implemented\n")); m_fga002[FGA_ICRLOCAL2] = data; break;
	case FGA_ICRLOCAL3      : LOG(("FGA_ICRLOCAL3 - not implemented\n")); m_fga002[FGA_ICRLOCAL3] = data; break;
	case FGA_ICRLOCAL4      : LOG(("FGA_ICRLOCAL4 - not implemented\n")); m_fga002[FGA_ICRLOCAL4] = data; break;
	case FGA_ICRLOCAL5      : LOG(("FGA_ICRLOCAL5 - not implemented\n")); m_fga002[FGA_ICRLOCAL5] = data; break;
	case FGA_ICRLOCAL6      : LOG(("FGA_ICRLOCAL6 - not implemented\n")); m_fga002[FGA_ICRLOCAL6] = data; break;
	case FGA_ICRLOCAL7      : LOG(("FGA_ICRLOCAL7 - not implemented\n")); m_fga002[FGA_ICRLOCAL7] = data; break;
	case FGA_ENAMCODE       : LOG(("FGA_ENAMCODE - not implemented\n")); m_fga002[FGA_ENAMCODE] = data; break;
	case FGA_CTL10          : LOG(("FGA_CTL10 - not implemented\n")); m_fga002[FGA_CTL10] = data; break;
	case FGA_CTL11          : LOG(("FGA_CTL11 - not implemented\n")); m_fga002[FGA_CTL11] = data; break;
	case FGA_MAINUM         : LOG(("FGA_MAINUM - not implemented\n")); m_fga002[FGA_MAINUM] = data; break;
	case FGA_MAINUU         : LOG(("FGA_MAINUU - not implemented\n")); m_fga002[FGA_MAINUU] = data; break;
	case FGA_BOTTOMPAGEU    : LOG(("FGA_BOTTOMPAGEU - not implemented\n")); m_fga002[FGA_BOTTOMPAGEU] = data; break;
	case FGA_BOTTOMPAGEL    : LOG(("FGA_BOTTOMPAGEL - not implemented\n")); m_fga002[FGA_BOTTOMPAGEL] = data; break;
	case FGA_TOPPAGEU       : LOG(("FGA_TOPPAGEU - not implemented\n")); m_fga002[FGA_TOPPAGEU] = data; break;
	case FGA_TOPPAGEL       : LOG(("FGA_TOPPAGEL - not implemented\n")); m_fga002[FGA_TOPPAGEL] = data; break;
	case FGA_MYVMEPAGE      : LOG(("FGA_MYVMEPAGE - not implemented\n")); m_fga002[FGA_MYVMEPAGE] = data; break;
	case FGA_TIM0PRELOAD    : LOG(("FGA_TIM0PRELOAD - not implemented\n")); m_fga002[FGA_TIM0PRELOAD] = data; break;
	case FGA_TIM0CTL        : LOG(("FGA_TIM0CTL - not implemented\n")); m_fga002[FGA_TIM0CTL] = data; break;
	case FGA_DMASRCATT      : LOG(("FGA_DMASRCATT - not implemented\n")); m_fga002[FGA_DMASRCATT] = data; break;
	case FGA_DMADSTATT      : LOG(("FGA_DMADSTATT - not implemented\n")); m_fga002[FGA_DMADSTATT] = data; break;
	case FGA_DMA_GENERAL    : LOG(("FGA_DMA_GENERAL - not implemented\n")); m_fga002[FGA_DMA_GENERAL] = data; break;
	case FGA_CTL12          : LOG(("FGA_CTL12 - not implemented\n")); m_fga002[FGA_CTL12] = data; break;
	case FGA_LIOTIMING      : LOG(("FGA_LIOTIMING - not implemented\n")); m_fga002[FGA_LIOTIMING] = data; break;
	case FGA_LOCALIACK      : LOG(("FGA_LOCALIACK - not implemented\n")); m_fga002[FGA_LOCALIACK] = data; break;
	case FGA_FMBCTL         : LOG(("FGA_FMBCTL - not implemented\n")); m_fga002[FGA_FMBCTL] = data; break;
	case FGA_FMBAREA        : LOG(("FGA_FMBAREA - not implemented\n")); m_fga002[FGA_FMBAREA] = data; break;
	case FGA_AUXSRCSTART    : LOG(("FGA_AUXSRCSTART - not implemented\n")); m_fga002[FGA_AUXSRCSTART] = data; break;
	case FGA_AUXDSTSTART    : LOG(("FGA_AUXDSTSTART - not implemented\n")); m_fga002[FGA_AUXDSTSTART] = data; break;
	case FGA_AUXSRCTERM     : LOG(("FGA_AUXSRCTERM - not implemented\n")); m_fga002[FGA_AUXSRCTERM] = data; break;
	case FGA_AUXDSTTERM     : LOG(("FGA_AUXDSTTERM - not implemented\n")); m_fga002[FGA_AUXDSTTERM] = data; break;
	case FGA_CTL13          : LOG(("FGA_CTL13 - not implemented\n")); m_fga002[FGA_CTL13] = data; break;
	case FGA_CTL14          : LOG(("FGA_CTL14 - not implemented\n")); m_fga002[FGA_CTL14] = data; break;
	case FGA_CTL15          : LOG(("FGA_CTL15 - not implemented\n")); m_fga002[FGA_CTL15] = data; break;
	case FGA_CTL16          : LOG(("FGA_CTL16 - not implemented\n")); m_fga002[FGA_CTL16] = data; break;
	case FGA_ISTIM0         : LOG(("FGA_ISTIM0 - not implemented\n")); m_fga002[FGA_ISTIM0] = data; break;
	case FGA_ISDMANORM      : LOG(("FGA_ISDMANORM - not implemented\n")); m_fga002[FGA_ISDMANORM] = data; break;
	case FGA_ISDMAERR       : LOG(("FGA_ISDMAERR - not implemented\n")); m_fga002[FGA_ISDMAERR] = data; break;
	case FGA_ISFMB0REF      : LOG(("FGA_ISFMB0REF - not implemented\n")); m_fga002[FGA_ISFMB0REF] = data; break;
	case FGA_ISFMB1REF      : LOG(("FGA_ISFMB1REF - not implemented\n")); m_fga002[FGA_ISFMB1REF] = data; break;
	case FGA_ISPARITY       : LOG(("FGA_ISPARITY - not implemented\n")); m_fga002[FGA_ISPARITY] = data; break;
	case FGA_DMARUNCTL      : LOG(("FGA_DMARUNCTL - not implemented\n")); m_fga002[FGA_DMARUNCTL] = data; break;
	case FGA_ISABORT        : LOG(("FGA_ISABORT - not implemented\n")); m_fga002[FGA_ISABORT] = data; break;
	case FGA_ISFMB0MES      : LOG(("FGA_ISFMB0MES - not implemented\n")); m_fga002[FGA_ISFMB0MES] = data; break;
	case FGA_ISFMB1MES      : LOG(("FGA_ISFMB1MES - not implemented\n")); m_fga002[FGA_ISFMB1MES] = data; break;
	case FGA_ABORTPIN       : LOG(("FGA_ABORTPIN - not implemented\n")); m_fga002[FGA_ABORTPIN] = data; break;
	default:
		LOG(("Unsupported register %04x\n", offset));
	}
}

READ8_MEMBER (fccpu30_state::fga8_r){
	UINT8 ret = 0;

	LOG(("%s[%04x]      ", FUNCNAME, offset));
	switch(offset)
	{
	case FGA_SPECIALENA     : ret = m_fga002[FGA_SPECIALENA]; LOG(("FGA_SPECIALENA returns %02x - not implemented\n", ret)); break;
	case FGA_RSVMECALL      : ret = m_fga002[FGA_RSVMECALL]; LOG(("FGA_RSVMECALL returns %02x - not implemented\n", ret)); break;
	case FGA_RSKEYRES       : ret = m_fga002[FGA_RSKEYRES]; LOG(("FGA_RSKEYRES returns %02x - not implemented\n", ret)); break;
	case FGA_RSCPUCALL      : ret = m_fga002[FGA_RSCPUCALL]; LOG(("FGA_RSCPUCALL returns %02x - not implemented\n", ret)); break;
	case FGA_RSLOCSW        : ret = m_fga002[FGA_RSLOCSW]; LOG(("FGA_RSLOCSW returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX0       : ret = m_fga002[FGA_ICRMBOX0]; LOG(("FGA_ICRMBOX0 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX1       : ret = m_fga002[FGA_ICRMBOX1]; LOG(("FGA_ICRMBOX1 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX2       : ret = m_fga002[FGA_ICRMBOX2]; LOG(("FGA_ICRMBOX2 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX3       : ret = m_fga002[FGA_ICRMBOX3]; LOG(("FGA_ICRMBOX3 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX4       : ret = m_fga002[FGA_ICRMBOX4]; LOG(("FGA_ICRMBOX4 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX5       : ret = m_fga002[FGA_ICRMBOX5]; LOG(("FGA_ICRMBOX5 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX6       : ret = m_fga002[FGA_ICRMBOX6]; LOG(("FGA_ICRMBOX6 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRMBOX7       : ret = m_fga002[FGA_ICRMBOX7]; LOG(("FGA_ICRMBOX7 returns %02x - not implemented\n", ret)); break;
	case FGA_VMEPAGE        : ret = m_fga002[FGA_VMEPAGE]; LOG(("FGA_VMEPAGE  returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME1        : ret = m_fga002[FGA_ICRVME1]; LOG(("FGA_ICRVME1 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME2        : ret = m_fga002[FGA_ICRVME2]; LOG(("FGA_ICRVME2 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME3        : ret = m_fga002[FGA_ICRVME3]; LOG(("FGA_ICRVME3 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME4        : ret = m_fga002[FGA_ICRVME4]; LOG(("FGA_ICRVME4 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME5        : ret = m_fga002[FGA_ICRVME5]; LOG(("FGA_ICRVME5 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME6        : ret = m_fga002[FGA_ICRVME6]; LOG(("FGA_ICRVME6 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRVME7        : ret = m_fga002[FGA_ICRVME7]; LOG(("FGA_ICRVME7 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRTIM0        : ret = m_fga002[FGA_ICRTIM0]; LOG(("FGA_ICRTIM0 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRDMANORM     : ret = m_fga002[FGA_ICRDMANORM]; LOG(("FGA_ICRDMANORM returns %02x - not implemented\n", ret)); break;
	case FGA_ICRDMAERR      : ret = m_fga002[FGA_ICRDMAERR]; LOG(("FGA_ICRDMAERR returns %02x - not implemented\n", ret)); break;
	case FGA_CTL1           : ret = m_fga002[FGA_CTL1]; LOG(("FGA_CTL1 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL2           : ret = m_fga002[FGA_CTL2]; LOG(("FGA_CTL2 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRFMB0REF     : ret = m_fga002[FGA_ICRFMB0REF]; LOG(("FGA_ICRFMB0REF returns %02x - not implemented\n", ret)); break;
	case FGA_ICRFMB1REF     : ret = m_fga002[FGA_ICRFMB1REF]; LOG(("FGA_ICRFMB1REF returns %02x - not implemented\n", ret)); break;
	case FGA_ICRFMB0MES     : ret = m_fga002[FGA_ICRFMB0MES]; LOG(("FGA_ICRFMB0MES returns %02x - not implemented\n", ret)); break;
	case FGA_ICRFMB1MES     : ret = m_fga002[FGA_ICRFMB1MES]; LOG(("FGA_ICRFMB1MES returns %02x - not implemented\n", ret)); break;
	case FGA_CTL3           : ret = m_fga002[FGA_CTL3]; LOG(("FGA_CTL3 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL4           : ret = m_fga002[FGA_CTL4]; LOG(("FGA_CTL4 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRPARITY      : ret = m_fga002[FGA_ICRPARITY]; LOG(("FGA_ICRPARITY returns %02x - not implemented\n", ret)); break;
	case FGA_AUXPINCTL      : ret = m_fga002[FGA_AUXPINCTL]; LOG(("FGA_AUXPINCTL returns %02x - not implemented\n", ret)); break;
	case FGA_CTL5           : ret = m_fga002[FGA_CTL5]; LOG(("FGA_CTL5 returns %02x - not implemented\n", ret)); break;
	case FGA_AUXFIFWEX      : ret = m_fga002[FGA_AUXFIFWEX]; LOG(("FGA_AUXFIFWEX returns %02x - not implemented\n", ret)); break;
	case FGA_AUXFIFREX      : ret = m_fga002[FGA_AUXFIFREX]; LOG(("FGA_AUXFIFREX returns %02x - not implemented\n", ret)); break;
	case FGA_CTL6           : ret = m_fga002[FGA_CTL6]; LOG(("FGA_CTL6 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL7           : ret = m_fga002[FGA_CTL7]; LOG(("FGA_CTL7 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL8           : ret = m_fga002[FGA_CTL8]; LOG(("FGA_CTL8 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL9           : ret = m_fga002[FGA_CTL9]; LOG(("FGA_CTL9 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRABORT       : ret = m_fga002[FGA_ICRABORT]; LOG(("FGA_ICRABORT returns %02x - not implemented\n", ret)); break;
	case FGA_ICRACFAIL      : ret = m_fga002[FGA_ICRACFAIL]; LOG(("FGA_ICRACFAIL returns %02x - not implemented\n", ret)); break;
	case FGA_ICRSYSFAIL     : ret = m_fga002[FGA_ICRSYSFAIL]; LOG(("FGA_ICRSYSFAIL returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL0      : ret = m_fga002[FGA_ICRLOCAL0]; LOG(("FGA_ICRLOCAL0 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL1      : ret = m_fga002[FGA_ICRLOCAL1]; LOG(("FGA_ICRLOCAL1 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL2      : ret = m_fga002[FGA_ICRLOCAL2]; LOG(("FGA_ICRLOCAL2 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL3      : ret = m_fga002[FGA_ICRLOCAL3]; LOG(("FGA_ICRLOCAL3 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL4      : ret = m_fga002[FGA_ICRLOCAL4]; LOG(("FGA_ICRLOCAL4 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL5      : ret = m_fga002[FGA_ICRLOCAL5]; LOG(("FGA_ICRLOCAL5 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL6      : ret = m_fga002[FGA_ICRLOCAL6]; LOG(("FGA_ICRLOCAL6 returns %02x - not implemented\n", ret)); break;
	case FGA_ICRLOCAL7      : ret = m_fga002[FGA_ICRLOCAL7]; LOG(("FGA_ICRLOCAL7 returns %02x - not implemented\n", ret)); break;
	case FGA_ENAMCODE       : ret = m_fga002[FGA_ENAMCODE]; LOG(("FGA_ENAMCODE returns %02x - not implemented\n", ret)); break;
	case FGA_CTL10          : ret = m_fga002[FGA_CTL10]; LOG(("FGA_CTL10 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL11          : ret = m_fga002[FGA_CTL11]; LOG(("FGA_CTL11 returns %02x - not implemented\n", ret)); break;
	case FGA_MAINUM         : ret = m_fga002[FGA_MAINUM]; LOG(("FGA_MAINUM returns %02x - not implemented\n", ret)); break;
	case FGA_MAINUU         : ret = m_fga002[FGA_MAINUU]; LOG(("FGA_MAINUU returns %02x - not implemented\n", ret)); break;
	case FGA_BOTTOMPAGEU    : ret = m_fga002[FGA_BOTTOMPAGEU]; LOG(("FGA_BOTTOMPAGEU returns %02x - not implemented\n", ret)); break;
	case FGA_BOTTOMPAGEL    : ret = m_fga002[FGA_BOTTOMPAGEL]; LOG(("FGA_BOTTOMPAGEL returns %02x - not implemented\n", ret)); break;
	case FGA_TOPPAGEU       : ret = m_fga002[FGA_TOPPAGEU]; LOG(("FGA_TOPPAGEU returns %02x - not implemented\n", ret)); break;
	case FGA_TOPPAGEL       : ret = m_fga002[FGA_TOPPAGEL]; LOG(("FGA_TOPPAGEL returns %02x - not implemented\n", ret)); break;
	case FGA_MYVMEPAGE      : ret = m_fga002[FGA_MYVMEPAGE]; LOG(("FGA_MYVMEPAGE returns %02x - not implemented\n", ret)); break;
	case FGA_TIM0PRELOAD    : ret = m_fga002[FGA_TIM0PRELOAD]; LOG(("FGA_TIM0PRELOAD returns %02x - not implemented\n", ret)); break;
	case FGA_TIM0CTL        : ret = m_fga002[FGA_TIM0CTL]; LOG(("FGA_TIM0CTL returns %02x - not implemented\n", ret)); break;
	case FGA_DMASRCATT      : ret = m_fga002[FGA_DMASRCATT]; LOG(("FGA_DMASRCATT returns %02x - not implemented\n", ret)); break;
	case FGA_DMADSTATT      : ret = m_fga002[FGA_DMADSTATT]; LOG(("FGA_DMADSTATT returns %02x - not implemented\n", ret)); break;
	case FGA_DMA_GENERAL    : ret = m_fga002[FGA_DMA_GENERAL]; LOG(("FGA_DMA_GENERAL returns %02x - not implemented\n", ret)); break;
	case FGA_CTL12          : ret = m_fga002[FGA_CTL12]; LOG(("FGA_CTL12 returns %02x - not implemented\n", ret)); break;
	case FGA_LIOTIMING      : ret = m_fga002[FGA_LIOTIMING]; LOG(("FGA_LIOTIMING returns %02x - not implemented\n", ret)); break;
	case FGA_LOCALIACK      : ret = m_fga002[FGA_LOCALIACK]; LOG(("FGA_LOCALIACK returns %02x - not implemented\n", ret)); break;
	case FGA_FMBCTL         : ret = m_fga002[FGA_FMBCTL]; LOG(("FGA_FMBCTL returns %02x - not implemented\n", ret)); break;
	case FGA_FMBAREA        : ret = m_fga002[FGA_FMBAREA]; LOG(("FGA_FMBAREA returns %02x - not implemented\n", ret)); break;
	case FGA_AUXSRCSTART    : ret = m_fga002[FGA_AUXSRCSTART]; LOG(("FGA_AUXSRCSTART returns %02x - not implemented\n", ret)); break;
	case FGA_AUXDSTSTART    : ret = m_fga002[FGA_AUXDSTSTART]; LOG(("FGA_AUXDSTSTART returns %02x - not implemented\n", ret)); break;
	case FGA_AUXSRCTERM     : ret = m_fga002[FGA_AUXSRCTERM]; LOG(("FGA_AUXSRCTERM returns %02x - not implemented\n", ret)); break;
	case FGA_AUXDSTTERM     : ret = m_fga002[FGA_AUXDSTTERM]; LOG(("FGA_AUXDSTTERM returns %02x - not implemented\n", ret)); break;
	case FGA_CTL13          : ret = m_fga002[FGA_CTL13]; LOG(("FGA_CTL13 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL14          : ret = m_fga002[FGA_CTL14]; LOG(("FGA_CTL14 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL15          : ret = m_fga002[FGA_CTL15]; LOG(("FGA_CTL15 returns %02x - not implemented\n", ret)); break;
	case FGA_CTL16          : ret = m_fga002[FGA_CTL16]; LOG(("FGA_CTL16 returns %02x - not implemented\n", ret)); break;
	case FGA_ISTIM0         : ret = m_fga002[FGA_ISTIM0]; LOG(("FGA_ISTIM0 returns %02x - not implemented\n", ret)); break;
	case FGA_ISDMANORM      : ret = m_fga002[FGA_ISDMANORM]; LOG(("FGA_ISDMANORM returns %02x - not implemented\n", ret)); break;
	case FGA_ISDMAERR       : ret = m_fga002[FGA_ISDMAERR]; LOG(("FGA_ISDMAERR returns %02x - not implemented\n", ret)); break;
	case FGA_ISFMB0REF      : ret = m_fga002[FGA_ISFMB0REF]; LOG(("FGA_ISFMB0REF returns %02x - not implemented\n", ret)); break;
	case FGA_ISFMB1REF      : ret = m_fga002[FGA_ISFMB1REF]; LOG(("FGA_ISFMB1REF returns %02x - not implemented\n", ret)); break;
	case FGA_ISPARITY       : ret = m_fga002[FGA_ISPARITY]; LOG(("FGA_ISPARITY returns %02x - not implemented\n", ret)); break;
	case FGA_DMARUNCTL      : ret = m_fga002[FGA_DMARUNCTL]; LOG(("FGA_DMARUNCTL returns %02x - not implemented\n", ret)); break;
	case FGA_ISABORT        : ret = m_fga002[FGA_ISABORT]; LOG(("FGA_ISABORT returns %02x - not implemented\n", ret)); break;
	case FGA_ISFMB0MES      : ret = m_fga002[FGA_ISFMB0MES]; LOG(("FGA_ISFMB0MES returns %02x - not implemented\n", ret)); break;
	case FGA_ISFMB1MES      : ret = m_fga002[FGA_ISFMB1MES]; LOG(("FGA_ISFMB1MES returns %02x - not implemented\n", ret)); break;
	case FGA_ABORTPIN       : ret = m_fga002[FGA_ABORTPIN]; LOG(("FGA_ABORTPIN returns %02x - not implemented\n", ret)); break;
	default:
		LOG(("Unsupported register %04x\n", offset));
	}
	return ret;
}

/*
 * Rotary Switches - to configure the board
 *
 * Table 25: PI/T #1 Interface Signals
 * Pin     Function  In/Out
 * PA0-PA3   SW1      In
 * PA4 PA7   SW2      In
 *
 * Table 38: Upper Rotary Switch (SW2)
 * Bit 3: This bit indicates whether the RAM disk should be initialized after reset. If this bit is set to "0" (settings 0-7),
 *  the RAM disk is initialized as defined by bit 0 and 1. When the disk is initialized, all data on the disk is lost.
 * Bit 2: This bit defines the default data size on the VMEbus. If the bit is set to "0", 16 bits are selected, if it is set
 *  to "1", 32 bits are selected.
 * Bit 1 and Bit 0: These two bits define the default RAM disk. See Table 40, "RAM Disk Usage," a detailed description.
 *  If AUTOBOOT is set by bit 2 and 3 of SW1, bit 1 and 0 of SW2 define which operating system will be booted. See Table 42,
 *  "Boot an Operating System (if AUTOBOOT is selected)," on page 129 for detailed description.
 *
 * Table 39: Lower Rotary Switch (SW1)
 * Bit 3 and Bit 2: These two bits define which program is to be invoked after reset. Please refer
 *  to Table 41, "Program After Reset," on page 129 for a detailed description.
 * Bit 1: If this switch is "0" (settings 0,1,4,5,8,9,C,D), VMEPROM tries to execute a start-up file after reset. The default
 *  filename is SY$STRT. If the bit is "1", VMEPROM comes up with the default banner.
 * Bit 0: If this switch is set to "0" (settings 0,2,4,6,8,A,C,E), VMEPROM checks the VMEbus for available hardware after reset.
 *  In addition VMEPROM waits for SYSFAIL to disappear from the VMEbus. The following hardware can be detected:
 *  - Contiguous memory
 *  - ASCU-1/2
 *  - ISIO-1/2
 *  - SIO-1/2
 *  - ISCSI-1
 *  - WFC-1
 *
 * Table 40: RAM Disk Usage
 * Bit 1 Bit 0 Upper Switch (SW 2) selected on
 *  1     1     RAM DISK AT TOP OF MEMORY (32 Kbytes) 3,7,B,F
 *  1     0     RAM DISK AT 0xFC80 0000 (512 Kbytes) 2,6,A,E
 *  0     1     RAM DISK AT 0x4070 0000 (512 Kbytes) 1,5,9,D
 *  0     0     RAM DISK AT 0x4080 0000 (512 Kbytes) 0,4,8,C
 *
 * Table 41: Program After Reset
 * Bit 3 Bit 2 Lower Switch (SW 1)          selected on
 *  1     1     VMEPROM                         C,D,E,F
 *  1     0     USER PROGRAM AT 0x4070 0000     8,9,A,B
 *  0     1     AUTOBOOT SYSTEM                 4,5,6,7
 *  0     0     USER PROGRAM AT 4080.000016     0,1,2,3
 *
 * Table 42: Boot an Operating System (if AUTOBOOT is selected)
 * Bit 1 Bit 0 Upper Switch (SW 2)          selected on
 *  1     1     reserved                        3,7,B,F
 *  1     0     Boot UNIX/PDOS 4.x              2,6,A,E
 *  0     1     Boot another operating system   1,5,9,D
 *  0     0     Setup for UNIX mailbox driver   0,4,8,C
 *
 * "To start VMEPROM, the rotary switches must both be set to 'F':" Hmm...
 */
READ8_MEMBER (fccpu30_state::rotary_rd){
	LOG(("%s\n", FUNCNAME));
	return 0xff; // TODO: make this configurable from commandline or artwork
}

/*
 * PI/T #2 Factory settings
 * B0-B2 Shared Memory Size - From these lines, the on-board Shared RAM capacity can be read in by software.
 * 0 0 0 32 Mb
 * 0 0 1 16 Mb
 * 0 1 0  8 Mb
 * 0 1 1  4 Mb
 * 1 x x  Reserved
 * B3-B7 Board ID(s) -  From these lines, the CPU board identification number can be read in by
 * 0 1 0 1 0 CPU-30 R4  software. Every CPU board has a unique number. Different versions of
 * (fill in more)       one CPU board (i.e. different speeds, capacity of memory, or modules)
 *                      contain the same identification number. In the case of the CPU-30 R4, the
 *                      number is ten ("10" decimal or 0A16 hexadecimal "01010" binary).
 */
READ8_MEMBER (fccpu30_state::board_mem_id_rd){
	LOG(("%s\n", FUNCNAME));
	return 0x6A; // CPU-30 R4 with 4Mb of shared RAM. TODO: make this configurable from commandline or artwork
}

#if 0
/* Dummy VME access methods until the VME bus device is ready for use */
READ16_MEMBER (fccpu30_state::vme_a24_r){
	LOG (logerror ("vme_a24_r\n"));
	return (UINT16) 0;
}

WRITE16_MEMBER (fccpu30_state::vme_a24_w){
	LOG (logerror ("vme_a24_w\n"));
}

READ16_MEMBER (fccpu30_state::vme_a16_r){
	LOG (logerror ("vme_16_r\n"));
	return (UINT16) 0;
}

WRITE16_MEMBER (fccpu30_state::vme_a16_w){
	LOG (logerror ("vme_a16_w\n"));
}
#endif

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (fccpu30, fccpu30_state)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68030, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP (fccpu30_mem)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Terminal Port config */
	MCFG_DUSCC68562_ADD("duscc", DUSCC_CLOCK, 0, 0, 0, 0 )
	MCFG_DUSCC_OUT_TXDA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD ("rs232trm", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, rxa_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, ctsa_w))

//  MCFG_DUSCC68562_ADD("duscc2", DUSCC_CLOCK, 0, 0, 0, 0 )

	/* PIT Parallel Interface and Timer device, assuming strapped for on board clock */
	MCFG_DEVICE_ADD ("pit1", PIT68230, XTAL_16MHz / 2)
	MCFG_PIT68230_PA_INPUT_CB(READ8(fccpu30_state, rotary_rd))
	MCFG_DEVICE_ADD ("pit2", PIT68230, XTAL_16MHz / 2)
	MCFG_PIT68230_PB_INPUT_CB(READ8(fccpu30_state, board_mem_id_rd))
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (fccpu30)
ROM_REGION32_BE(0xfff00000, "maincpu", 0)

ROM_LOAD16_BYTE("CPU30LO.BIN",  0xff000000, 0x20000, CRC (fefa88ed) SHA1 (71a9ad807c0c2da5c6f6a6dc68c73ad8b52f3ea9))
ROM_LOAD16_BYTE("CPU30UP.BIN",  0xff000001, 0x20000, CRC (dfed1f68) SHA1 (71478a77d5ab5da0fabcd78e69537919b560e3b8))
ROM_LOAD("PGA-002.BIN",         0xffe00000, 0x10000, CRC (faa38972) SHA1 (651dfc2f9a865fc6adf49dad90f9e705f2889919))

/*
 * System ROM information
 *
 * FGA-002 Bootprom version 3.1 is released May 28, 1990, coprighted by FORCE Computers Gmbh
 *
 * Bootprom PIT setup sequence
 *  0a 00 <- read port A without side effects
 *  0b 00 <- read port B without side effects
 *  10 00 -> TCR - Timer Control register: Disable timer
 *  13 ff -> CPRH - Counter Preload Regsiter High
 *  14 ff -> CPRM - Counter Preload Regsiter Mid
 *  15 ff -> CPRL - Counter Preload Regsiter Low
 *  10 01 -> TCR - Timer Control register: Enable timer
 * ------ init ends -------- clock: 4217
 *
 * To start VMEPROM, the rotary switches must both be set to 'F' (PI/T #1 port A)
 *
 * ------ next config -------- clock: 1964222
 *  10 00 -> TCR - Timer Control register: Disable timer
 *  17 00 -> CRH - Counter Register High
 *  18 00 -> CRM - Counter Register Medium
 *  19 00 -> CRL - Counter Register Low
 *
 * DUSCC #1 channel A setup sequence
 *  0f 00 -> REG_CCR    - reset Tx Command
 *  0f 40 -> REG_CCR    - reset Rx Command
 *  00 07 -> REG_CMR1   - Async mode
 *  01 38 -> REG_CMR2   - Normal polled or interrupt mode, no DMA
 *  04 7f -> REG_TPR    - Tx 8 bits, CTS and RTS, 1 STOP bit
 *  06 1b -> REG_RPR    - Rx RTS, 8 bits, no DCD, no parity
 *  05 3d -> REG_TTR    - Tx BRG 9600 (assuming a 14.7456 crystal)
 *  07 2d -> REG_RTR    - Rx BRG 9600 (assuming a 14.7456 crystal)
 *  0e 27 -> REG_PCR    - TRxC = RxCLK 1x, RTxC is input, RTS, GPO2, crystal oscillator connected to X2
 *  0b f1 -> REG_OMR    - RTS low, OUT1 = OUT2 = high, RxRdy asserted for each character,
 *                        TxRdy asserted on threshold, Same Tx Residual Character Length as for REG_TPR
 *  0f 00 -> REG_CCR    - reset Tx Command
 *  0f 40 -> REG_CCR    - reset Rx Command
 *  0f 02 -> REG_CCR    - enable Tx Command
 *  0f 42 -> REG_CCR    - enable Rx Command
 *--- end of setup sequence ---
 *  loop:
 *    read  <- REG_GSR
 *  until something needs attention
 */
ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1990, fccpu30,      0,      0,       fccpu30,        fccpu30, driver_device, 0,   "Force Computers Gmbh",   "SYS68K/CPU-30", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
