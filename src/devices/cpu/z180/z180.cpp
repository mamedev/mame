// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Portable Z180 emulator V0.3
 *
 *****************************************************************************/

/*****************************************************************************

    TODO:
        - HALT processing is not yet perfect. The manual states that
          during HALT, all dma and internal i/o incl. timers continue to
          work. Currently, only timers are implemented. Ideally, the
          (currently unused) burn_cycles routine would go away and halt
          processing be implemented in execute_run.

        - Documentation for RXS/CTS1 pin is contradictory.

          From page 12:
          "During RESET, this pin is initialized as RXS pin. If CTS1E
          bit in ASCI status register ch1 (STAT1) is set to 1, CTS1
          function is selected.  If CTS1E bit is set to 0, RXS function
          is selected."

          However, there is no CTS1E bit in the ASCI status register.

          From pages 43-44:
          "CTS/PS: Clear to Send/Prescale (bit 5)—If bit 5 of the System
          Configuration Register is 0, the CTS0/RxS pin features the
          CTS0 function, and the state of the pin can be read in bit 5
          of CNTLB0 in a real-time, positive-logic fashion (HIGH = 1 ,
          LOW = 0).  If bit 5 in the System Configuration Register is 0
          to auto-enable CTS0, and the pin is negated (High), the TDRE
          bit is inhibited (forced to 0).  Bit 5 of CNTLB1 reads back as
          0."

          This contradicts everything else in the documentation as it
          implies RXS shares a pin with CTS0 (rather than CTS1)

          For now, the input is always sent to both RXS and CTS1.

 *****************************************************************************/

/*****************************************************************************

Z180 Info:

Known clock speeds (from ZiLOG): 6 (actually ~6.15), 8, 10, 20 & 33MHz

Much like the 80186/188, the operating frequency is generated from the
XTAL/EXTAL clock by an internal divide-by-2 circuit. The maximum frequency of
the EXTAL input is therefore twice the speed rating for the specific part.

ZiLOG Z180 codes:

  Speed: 10 = 10MHZ
         20 = 20MHz
         33 = 33MHz
Package: P = 64-Pin Plastic SDIP
         V = 68-Pin PLCC
         F = 80-Pin QFP
   Temp: S = 0C to +70C
         E = -40C to +85C

Environmental Flow: C = Plastic Standard


Example from Ms.Pac-Man/Galaga - 20 year Reunion hardware (see src/mame/drivers/20pacgal.c):

   CPU is Z8S18020VSC = Z180, 20MHz, 68-Pin PLCC, 0C to +70C, Plastic Standard


Other CPUs that use a compatible Z180 core:

Hitachi HD647180 series:
  Available in QFP80, PLCC84 & DIP90 packages (the QFP80 is not pinout compatible)
  The HD647180 also has an internal ROM

 *****************************************************************************/

#include "emu.h"
#include "z180.h"
#include "z180dasm.h"

//#define VERBOSE 1
#include "logmacro.h"

/* interrupt priorities */
#define Z180_INT_TRAP   0           /* Undefined opcode */
#define Z180_INT_NMI    1           /* NMI */
#define Z180_INT_IRQ0   2           /* Execute IRQ1 */
#define Z180_INT_IRQ1   3           /* Execute IRQ1 */
#define Z180_INT_IRQ2   4           /* Execute IRQ2 */
#define Z180_INT_PRT0   5           /* Internal PRT channel 0 */
#define Z180_INT_PRT1   6           /* Internal PRT channel 1 */
#define Z180_INT_DMA0   7           /* Internal DMA channel 0 */
#define Z180_INT_DMA1   8           /* Internal DMA channel 1 */
#define Z180_INT_CSIO   9           /* Internal CSI/O */
#define Z180_INT_ASCI0  10          /* Internal ASCI channel 0 */
#define Z180_INT_ASCI1  11          /* Internal ASCI channel 1 */
#define Z180_INT_MAX    Z180_INT_ASCI1

/****************************************************************************/
/* The Z180 registers. HALT is set to 1 when the CPU is halted, the refresh */
/* register is calculated as follows: refresh=(Regs.R&127)|(Regs.R2&128)    */
/****************************************************************************/

DEFINE_DEVICE_TYPE(Z80180, z80180_device, "z80180", "Zilog Z80180") // equivalent to Hitachi HD64180R or HD64180Z
DEFINE_DEVICE_TYPE(HD64180RP, hd64180rp_device, "hd64180rp", "Hitachi HD64180RP") // DIP64 version, identical to Zilog Z80180xxPSC
DEFINE_DEVICE_TYPE(Z8S180, z8s180_device, "z8s180", "Zilog Z8S180") // enhanced Z80180
DEFINE_DEVICE_TYPE(Z80182, z80182_device, "z80182", "Zilog Z80182") // further enhanced Z8S180


z180_device::z180_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool extended_io, address_map_constructor internal_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, z80_daisy_chain_interface(mconfig, *this)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0, 16, 12, internal_map)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_decrypted_opcodes_config("opcodes", ENDIANNESS_LITTLE, 8, 20, 0, 16, 12, internal_map)
	, m_asci(*this, "asci_%u", 0U)
	, m_csio(*this, "csio")
	, m_extended_io(extended_io)
	, m_tend0_cb(*this)
	, m_tend1_cb(*this)
{
	// some arbitrary initial values
	m_tmdr[0].w = m_tmdr[1].w = 0;
	m_rldr[0].w = m_rldr[1].w = 0xffff;
	m_dma_sar0.d = 0;
	m_dma_dar0.d = 0;
	m_dma_mar1.d = 0;
	m_dma_iar1.d = 0;
	m_dma_bcr[0].w = m_dma_bcr[1].w = 0;
}

std::unique_ptr<util::disasm_interface> z180_device::create_disassembler()
{
	return std::make_unique<z180_disassembler>();
}

z80180_device::z80180_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180_device(mconfig, Z80180, tag, owner, clock, false, address_map_constructor())
{
}

hd64180rp_device::hd64180rp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z180_device(mconfig, HD64180RP, tag, owner, clock, false, address_map_constructor())
{
	// 64-pin DIP versions omit A19
	set_address_width(19);
}

z8s180_device::z8s180_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: z180_device(mconfig, type, tag, owner, clock, false, address_map_constructor())
{
}

z8s180_device::z8s180_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8s180_device(mconfig, Z8S180, tag, owner, clock)
{
}

z80182_device::z80182_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z8s180_device(mconfig, Z80182, tag, owner, clock)
{
}

#define CF  0x01
#define NF  0x02
#define PF  0x04
#define VF  PF
#define XF  0x08
#define HF  0x10
#define YF  0x20
#define ZF  0x40
#define SF  0x80

/* I/O line status flags */
#define Z180_CKA0     0x00000001  /* I/O asynchronous clock 0 (active high) or DREQ0 (mux) */
#define Z180_CKA1     0x00000002  /* I/O asynchronous clock 1 (active high) or TEND1 (mux) */
#define Z180_CKS      0x00000004  /* I/O serial clock (active high) */
#define Z180_CTS0     0x00000100  /* I   clear to send 0 (active low) */
#define Z180_CTS1     0x00000200  /* I   clear to send 1 (active low) or RXS (mux) */
#define Z180_DCD0     0x00000400  /* I   data carrier detect (active low) */
#define Z180_DREQ0    0x00000800  /* I   data request DMA ch 0 (active low) or CKA0 (mux) */
#define Z180_DREQ1    0x00001000  /* I   data request DMA ch 1 (active low) */
#define Z180_RXA0     0x00002000  /* I   asynchronous receive data 0 (active high) */
#define Z180_RXA1     0x00004000  /* I   asynchronous receive data 1 (active high) */
#define Z180_RXS      0x00008000  /* I   clocked serial receive data (active high) or CTS1 (mux) */
#define Z180_RTS0     0x00010000  /*   O request to send (active low) */
#define Z180_TEND0    0x00020000  /*   O transfer end 0 (active low) or CKA1 (mux) */
#define Z180_TEND1    0x00040000  /*   O transfer end 1 (active low) */
#define Z180_A18_TOUT 0x00080000  /*   O transfer out (PRT channel, active low) or A18 (mux) */
#define Z180_TXA0     0x00100000  /*   O asynchronous transmit data 0 (active high) */
#define Z180_TXA1     0x00200000  /*   O asynchronous transmit data 1 (active high) */
#define Z180_TXS      0x00400000  /*   O clocked serial transmit data (active high) */

bool z180_device::get_tend0()
{
	return !!(m_iol & Z180_TEND0);
}

bool z180_device::get_tend1()
{
	return !!(m_iol & Z180_TEND1);
}

/*
 * Prevent warnings on NetBSD.  All identifiers beginning with an underscore
 * followed by an uppercase letter are reserved by the C standard (ISO/IEC
 * 9899:1999, 7.1.3) to be used by the implementation.  It'd be best to rename
 * all such instances, but this is less intrusive and error-prone.
 */
#undef _B
#undef _C
#undef _L

#define _PPC    m_PREPC.d /* previous program counter */

#define _PCD    m_PC.d
#define _PC     m_PC.w.l

#define _SPD    m_SP.d
#define _SP     m_SP.w.l

#define _AFD    m_AF.d
#define _AF     m_AF.w.l
#define _A      m_AF.b.h
#define _F      m_AF.b.l

#define _BCD    m_BC.d
#define _BC     m_BC.w.l
#define _B      m_BC.b.h
#define _C      m_BC.b.l

#define _DED    m_DE.d
#define _DE     m_DE.w.l
#define _D      m_DE.b.h
#define _E      m_DE.b.l

#define _HLD    m_HL.d
#define _HL     m_HL.w.l
#define _H      m_HL.b.h
#define _L      m_HL.b.l

#define _IXD    m_IX.d
#define _IX     m_IX.w.l
#define _HX     m_IX.b.h
#define _LX     m_IX.b.l

#define _IYD    m_IY.d
#define _IY     m_IY.w.l
#define _HY     m_IY.b.h
#define _LY     m_IY.b.l


/* 10 TIMER control register (TIF1 and TIF0 are read-only) */
#define Z180_TCR_TIF1           0x80
#define Z180_TCR_TIF0           0x40
#define Z180_TCR_TIE1           0x20
#define Z180_TCR_TIE0           0x10
#define Z180_TCR_TOC1           0x08
#define Z180_TCR_TOC0           0x04
#define Z180_TCR_TDE1           0x02
#define Z180_TCR_TDE0           0x01

/* 1e (Z8S180/Z8L180) clock multiplier */
#define Z180_CMR_X2             0x80
#define Z180_CMR_LOW_NOISE      0x40

#define Z180_CMR_MASK           0xc0

/* 1f (Z8S180/Z8L180) chip control register */
#define Z180_CCR_CLOCK_DIVIDE   0x80
#define Z180_CCR_STDBY_IDLE1    0x40
#define Z180_CCR_BREXT          0x20
#define Z180_CCR_LNPHI          0x10
#define Z180_CCR_STDBY_IDLE0    0x08
#define Z180_CCR_LNIO           0x04
#define Z180_CCR_LNCPU_CTL      0x02
#define Z180_CCR_LNAD_DATA      0x01

/* 20-22 DMA source address register ch 0 L, H, B */
#define Z180_SAR0_MASK          0x0fffff

/* 23-25 DMA destination address register ch 0 L, H, B */
#define Z180_DAR0_MASK          0x0fffff

/* 28-2a DMA memory address register ch 1 L, H, B */
#define Z180_MAR1_MASK          0x0fffff

/* 2b-2d DMA I/O address register ch 1 L, H, (Z8S180/Z8L180) B */
#define Z180_IAR1_ATF           0x800000
#define Z180_IAR1_ATC           0x400000
#define Z180_IAR1_TOUT_DREQ     0x080000
#define Z180_IAR1_SS            0x070000

#define Z180_IAR1_MASK          0xcfffff

/* 30 DMA status register (DWE1 and DWE0 are write-only, DME is read-only) */
#define Z180_DSTAT_DE1          0x80    /* DMA enable ch 1 */
#define Z180_DSTAT_DE0          0x40    /* DMA enable ch 0 */
#define Z180_DSTAT_DWE1         0x20    /* DMA write enable ch 0 (active low) */
#define Z180_DSTAT_DWE0         0x10    /* DMA write enable ch 1 (active low) */
#define Z180_DSTAT_DIE1         0x08    /* DMA IRQ enable ch 1 */
#define Z180_DSTAT_DIE0         0x04    /* DMA IRQ enable ch 0 */
#define Z180_DSTAT_DME          0x01    /* DMA enable (read only) */

#define Z180_DSTAT_MASK         0xfd

/* 31 DMA mode register */
#define Z180_DMODE_DM           0x30    /* DMA ch 0 destination addressing mode */
#define Z180_DMODE_SM           0x0c    /* DMA ch 0 source addressing mode */
#define Z180_DMODE_MMOD         0x02    /* DMA cycle steal/burst mode select */

#define Z180_DMODE_MASK         0x3e

/* 32 DMA/WAIT control register */
#define Z180_DCNTL_MWI1         0x80
#define Z180_DCNTL_MWI0         0x40
#define Z180_DCNTL_IWI1         0x20
#define Z180_DCNTL_IWI0         0x10
#define Z180_DCNTL_DMS1         0x08
#define Z180_DCNTL_DMS0         0x04
#define Z180_DCNTL_DIM1         0x02
#define Z180_DCNTL_DIM0         0x01

/* 33 INT vector low register */
#define Z180_IL_IL              0xe0

#define Z180_IL_MASK            0xe0

/* 34 INT/TRAP control register (UFO is read-only) */
#define Z180_ITC_TRAP           0x80
#define Z180_ITC_UFO            0x40
#define Z180_ITC_ITE2           0x04
#define Z180_ITC_ITE1           0x02
#define Z180_ITC_ITE0           0x01

#define Z180_ITC_MASK           0xc7

/* 36 refresh control register */
#define Z180_RCR_REFE           0x80
#define Z180_RCR_REFW           0x40
#define Z180_RCR_CYC            0x03

#define Z180_RCR_MASK           0xc3

/* 3a MMU common/bank area register */
#define Z180_CBAR_CA            0xf0
#define Z180_CBAR_BA            0x0f

/* 3e operation mode control register (M1TE is write-onlu) */
#define Z180_OMCR_M1E           0x80
#define Z180_OMCR_M1TE          0x40
#define Z180_OMCR_IOC           0x20

#define Z180_OMCR_MASK          0xe0

/* 3f I/O control register */
#define Z180_IOCR_IOSTP         0x20

/***************************************************************************
    CPU PREFIXES

    order is important here - see z180tbl.h
***************************************************************************/

#define Z180_PREFIX_op          0
#define Z180_PREFIX_cb          1
#define Z180_PREFIX_dd          2
#define Z180_PREFIX_ed          3
#define Z180_PREFIX_fd          4
#define Z180_PREFIX_xycb        5

#define Z180_PREFIX_COUNT       (Z180_PREFIX_xycb + 1)



static uint8_t SZ[256];       /* zero and sign flags */
static uint8_t SZ_BIT[256];   /* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static uint8_t SZP[256];      /* zero, sign and parity flags */
static uint8_t SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static uint8_t SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

static std::unique_ptr<uint8_t[]> SZHVC_add;
static std::unique_ptr<uint8_t[]> SZHVC_sub;

#include "z180ops.h"
#include "z180tbl.h"

#include "z180cb.hxx"
#include "z180xy.hxx"
#include "z180dd.hxx"
#include "z180fd.hxx"
#include "z180ed.hxx"
#include "z180op.hxx"


void z180_device::set_address_width(int bits)
{
	m_program_config.m_addr_width = bits;
	m_decrypted_opcodes_config.m_addr_width = bits;
}

device_memory_interface::space_config_vector z180_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_OPCODES, &m_decrypted_opcodes_config),
			std::make_pair(AS_IO,      &m_io_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_io_config)
		};
}

uint8_t z180_device::z180_readcontrol(offs_t port)
{
	// normal external readport (ignore the data)
	(void)m_io.read_byte(port);

	// read the internal register
	return z180_internal_port_read(port & (m_extended_io ? 0x7f : 0x3f));
}

uint8_t z180_device::z180_internal_port_read(uint8_t port)
{
	uint8_t data = 0xff;

	switch (port)
	{
	case 0x00:
	case 0x01:
		data = m_asci[port & 1]->cntla_r();
		break;

	case 0x02:
	case 0x03:
		data = m_asci[port & 1]->cntlb_r();
		break;

	case 0x04:
	case 0x05:
		data = m_asci[port & 1]->stat_r();
		break;

	case 0x06:
	case 0x07:
		data = m_asci[port & 1]->tdr_r();
		break;

	case 0x08:
	case 0x09:
		data = m_asci[port & 1]->rdr_r();
		break;

	case 0x0a:
		data = m_csio->cntr_r();
		break;

	case 0x0b:
		data = m_csio->trdr_r();
		break;

	case 0x0c:
		data = m_tmdr_value[0] & 0x00ff;
		LOG("Z180 TMDR0L rd $%02x ($%04x)\n", data, m_tmdr[0].w);
		/* if timer is counting, latch the MSB and set the latch flag */
		if ((m_tcr & Z180_TCR_TDE0) == 0)
		{
			m_tmdr_latch |= 1;
			m_tmdrh[0] = (m_tmdr_value[0] & 0xff00) >> 8;
		}

		if(m_read_tcr_tmdr[0])
		{
			m_tcr &= ~Z180_TCR_TIF0; // reset TIF0
			m_read_tcr_tmdr[0] = 0;
		}
		else
		{
			m_read_tcr_tmdr[0] = 1;
		}
		break;

	case 0x0d:
		/* read latched value? */
		if (m_tmdr_latch & 1)
		{
			m_tmdr_latch &= ~1;
			data = m_tmdrh[0];
		}
		else
		{
			data = (m_tmdr_value[0] & 0xff00) >> 8;
		}

		if(m_read_tcr_tmdr[0])
		{
			m_tcr &= ~Z180_TCR_TIF0; // reset TIF0
			m_read_tcr_tmdr[0] = 0;
		}
		else
		{
			m_read_tcr_tmdr[0] = 1;
		}
		LOG("Z180 TMDR0H rd $%02x ($%04x)\n", data, m_tmdr[0].w);
		break;

	case 0x0e:
		data = m_rldr[0].b.l;
		LOG("Z180 RLDR0L rd $%02x ($%04x)\n", data, m_rldr[0].w);
		break;

	case 0x0f:
		data = m_rldr[0].b.h;
		LOG("Z180 RLDR0H rd $%02x ($%04x)\n", data, m_rldr[0].w);
		break;

	case 0x10:
		data = m_tcr;

		if(m_read_tcr_tmdr[0])
		{
			m_tcr &= ~Z180_TCR_TIF0; // reset TIF0
			m_read_tcr_tmdr[0] = 0;
		}
		else
		{
			m_read_tcr_tmdr[0] = 1;
		}

		if(m_read_tcr_tmdr[1])
		{
			m_tcr &= ~Z180_TCR_TIF1; // reset TIF1
			m_read_tcr_tmdr[1] = 0;
		}
		else
		{
			m_read_tcr_tmdr[1] = 1;
		}

		LOG("Z180 TCR    rd $%02x ($%02x)\n", data, m_tcr);
		break;

	case 0x14:
		data = m_tmdr_value[1];
		LOG("Z180 TMDR1L rd $%02x ($%02x)\n", data, m_tmdr[1].w);
		/* if timer is counting, latch the MSB and set the latch flag */
		if ((m_tcr & Z180_TCR_TDE1) == 0)
		{
			m_tmdr_latch |= 2;
			m_tmdrh[1] = (m_tmdr_value[1] & 0xff00) >> 8;
		}

		if(m_read_tcr_tmdr[1])
		{
			m_tcr &= ~Z180_TCR_TIF1; // reset TIF1
			m_read_tcr_tmdr[1] = 0;
		}
		else
		{
			m_read_tcr_tmdr[1] = 1;
		}
		break;

	case 0x15:
		/* read latched value? */
		if (m_tmdr_latch & 2)
		{
			m_tmdr_latch &= ~2;
			data = m_tmdrh[1];
		}
		else
		{
			data = (m_tmdr_value[1] & 0xff00) >> 8;
		}

		if(m_read_tcr_tmdr[1])
		{
			m_tcr &= ~Z180_TCR_TIF1; // reset TIF1
			m_read_tcr_tmdr[1] = 0;
		}
		else
		{
			m_read_tcr_tmdr[1] = 1;
		}
		LOG("Z180 TMDR1H rd $%02x ($%04x)\n", data, m_tmdr[1].w);
		break;

	case 0x16:
		data = m_rldr[1].b.l;
		LOG("Z180 RLDR1L rd $%02x ($%04x)\n", data, m_rldr[1].w);
		break;

	case 0x17:
		data = m_rldr[1].b.h;
		LOG("Z180 RLDR1H rd $%02x ($%04x)\n", data, m_rldr[1].w);
		break;

	case 0x18:
		data = m_frc;
		LOG("Z180 FRC    rd $%02x\n", data);
		break;

	case 0x19:
		data = 0xff;
		LOG("Z180 IO19   rd $%02x\n", data);
		break;

	case 0x20:
		data = m_dma_sar0.b.l;
		LOG("Z180 SAR0L  rd $%02x ($%05x)\n", data, m_dma_sar0.d);
		break;

	case 0x21:
		data = m_dma_sar0.b.h;
		LOG("Z180 SAR0H  rd $%02x ($%05x)\n", data, m_dma_sar0.d);
		break;

	case 0x22:
		data = m_dma_sar0.b.h2 & (Z180_SAR0_MASK >> 16);
		LOG("Z180 SAR0B  rd $%02x ($%05x)\n", data, m_dma_sar0.d);
		break;

	case 0x23:
		data = m_dma_dar0.b.l;
		LOG("Z180 DAR0L  rd $%02x ($%05x)\n", data, m_dma_dar0.d);
		break;

	case 0x24:
		data = m_dma_dar0.b.h;
		LOG("Z180 DAR0H  rd $%02x ($%05x)\n", data, m_dma_dar0.d);
		break;

	case 0x25:
		data = m_dma_dar0.b.h2 & (Z180_DAR0_MASK >> 16);
		LOG("Z180 DAR0B  rd $%02x ($%05x)\n", data, m_dma_dar0.d);
		break;

	case 0x26:
		data = m_dma_bcr[0].b.l;
		LOG("Z180 BCR0L  rd $%02x ($%04x)\n", data, m_dma_bcr[0].w);
		break;

	case 0x27:
		data = m_dma_bcr[0].b.h;
		LOG("Z180 BCR0H  rd $%02x ($%04x)\n", data, m_dma_bcr[1].w);
		break;

	case 0x28:
		data = m_dma_mar1.b.l;
		LOG("Z180 MAR1L  rd $%02x ($%05x)\n", data, m_dma_mar1.d);
		break;

	case 0x29:
		data = m_dma_mar1.b.h;
		LOG("Z180 MAR1H  rd $%02x ($%05x)\n", data, m_dma_mar1.d);
		break;

	case 0x2a:
		data = m_dma_mar1.b.h2 & (Z180_MAR1_MASK >> 16);
		LOG("Z180 MAR1B  rd $%02x ($%05x)\n", data, m_dma_mar1.d);
		break;

	case 0x2b:
		data = m_dma_iar1.b.l;
		LOG("Z180 IAR1L  rd $%02x ($%05x)\n", data, m_dma_iar1.d);
		break;

	case 0x2c:
		data = m_dma_iar1.b.h;
		LOG("Z180 IAR1H  rd $%02x ($%05x)\n", data, m_dma_iar1.d);
		break;

	case 0x2d:
		data = m_dma_iar1.b.h2 & (Z180_IAR1_MASK >> 16);
		LOG("Z180 IAR1B  rd $%02x ($%05x)\n", data, m_dma_iar1.d);
		break;

	case 0x2e:
		data = m_dma_bcr[1].b.l;
		LOG("Z180 BCR1L  rd $%02x ($%04x)\n", data, m_dma_bcr[1].w);
		break;

	case 0x2f:
		data = m_dma_bcr[1].b.h;
		LOG("Z180 BCR1H  rd $%02x ($%04x)\n", data, m_dma_bcr[1].w);
		break;

	case 0x30:
		data = m_dstat | ~Z180_DSTAT_MASK;
		LOG("Z180 DSTAT  rd $%02x ($%02x)\n", data, m_dstat);
		break;

	case 0x31:
		data = m_dmode | ~Z180_DMODE_MASK;
		LOG("Z180 DMODE  rd $%02x ($%02x)\n", data, m_dmode);
		break;

	case 0x32:
		data = m_dcntl;
		LOG("Z180 DCNTL  rd $%02x\n", data);
		break;

	case 0x33:
		data = m_il & Z180_IL_MASK;
		LOG("Z180 IL     rd $%02x ($%02x)\n", data, m_il);
		break;

	case 0x34:
		data = m_itc | ~Z180_ITC_MASK;
		LOG("Z180 ITC    rd $%02x ($%02x)\n", data, m_itc);
		break;

	case 0x36:
		data = m_rcr | ~Z180_RCR_MASK;
		LOG("Z180 RCR    rd $%02x ($%02x)\n", data, m_rcr);
		break;

	case 0x38:
		data = m_mmu_cbr;
		LOG("Z180 CBR    rd $%02x\n", data);
		break;

	case 0x39:
		data = m_mmu_bbr;
		LOG("Z180 BBR    rd $%02x\n", data);
		break;

	case 0x3a:
		data = m_mmu_cbar;
		LOG("Z180 CBAR   rd $%02x\n", data);
		break;

	case 0x3e:
		data = m_omcr | Z180_OMCR_M1TE | ~Z180_OMCR_MASK;
		LOG("Z180 OMCR   rd $%02x ($%02x)\n", data, m_omcr);
		break;

	case 0x3f:
		data = m_iocr | ~(m_extended_io ? 0xa0 : 0xe0);
		LOG("Z180 IOCR   rd $%02x ($%02x)\n", data, m_iocr);
		break;

	default:
		data = 0xff;
		LOG("Z180 IO%02X   rd $%02x\n", port, data);
		break;
	}

	return data;
}

void z180_device::z180_writecontrol(offs_t port, uint8_t data)
{
	// normal external write port
	m_io.write_byte(port, data);

	// store the data in the internal register
	z180_internal_port_write(port & (m_extended_io ? 0x7f : 0x3f), data);
}

void z180_device::z180_internal_port_write(uint8_t port, uint8_t data)
{
	switch (port)
	{
	case 0x00:
	case 0x01:
		m_asci[port & 1]->cntla_w(data);
		break;

	case 0x02:
	case 0x03:
		m_asci[port & 1]->cntlb_w(data);
		break;

	case 0x04:
	case 0x05:
		m_asci[port & 1]->stat_w(data);
		break;

	case 0x06:
	case 0x07:
		m_asci[port & 1]->tdr_w(data);
		break;

	case 0x08:
	case 0x09:
		m_asci[port & 1]->rdr_w(data);
		break;

	case 0x0a:
		m_csio->cntr_w(data);
		break;

	case 0x0b:
		m_csio->trdr_w(data);
		break;

	case 0x0c:
		LOG("Z180 TMDR0L wr $%02x\n", data);
		m_tmdr[0].b.l = data;
		m_tmdr_value[0] = (m_tmdr_value[0] & 0xff00) | m_tmdr[0].b.l;
		break;

	case 0x0d:
		LOG("Z180 TMDR0H wr $%02x\n", data);
		m_tmdr[0].b.h = data;
		m_tmdr_value[0] = (m_tmdr_value[0] & 0x00ff) | (m_tmdr[0].b.h << 8);
		break;

	case 0x0e:
		LOG("Z180 RLDR0L wr $%02x\n", data);
		m_rldr[0].b.l = data;
		break;

	case 0x0f:
		LOG("Z180 RLDR0H wr $%02x\n", data);
		m_rldr[0].b.h = data;
		break;

	case 0x10:
		LOG("Z180 TCR    wr $%02x ($%02x)\n", data,  data & ~(Z180_TCR_TIF1 | Z180_TCR_TIF0));
		{
			uint16_t old = m_tcr;
			/* Force reload on state change */
			m_tcr = (m_tcr & (Z180_TCR_TIF1 | Z180_TCR_TIF0)) | (data & ~(Z180_TCR_TIF1 | Z180_TCR_TIF0));
			if (!(old & Z180_TCR_TDE0) && (m_tcr & Z180_TCR_TDE0))
				m_tmdr_value[0] = 0; //m_rldr[0].w;
			if (!(old & Z180_TCR_TDE1) && (m_tcr & Z180_TCR_TDE1))
				m_tmdr_value[1] = 0; //m_rldr[1].w;
		}

		break;

	case 0x14:
		LOG("Z180 TMDR1L wr $%02x\n", data);
		m_tmdr[1].b.l = data;
		m_tmdr_value[1] = (m_tmdr_value[1] & 0xff00) | m_tmdr[1].b.l;
		break;

	case 0x15:
		LOG("Z180 TMDR1H wr $%02x\n", data);
		m_tmdr[1].b.h = data;
		m_tmdr_value[1] = (m_tmdr_value[1] & 0x00ff) | m_tmdr[1].b.h;
		break;

	case 0x16:
		LOG("Z180 RLDR1L wr $%02x\n", data);
		m_rldr[1].b.l = data;
		break;

	case 0x17:
		LOG("Z180 RLDR1H wr $%02x\n", data);
		m_rldr[1].b.h = data;
		break;

	case 0x18:
		LOG("Z180 FRC    wr $%02x\n", data);
		// FRC is read-only
		break;

	case 0x20:
		LOG("Z180 SAR0L  wr $%02x\n", data);
		m_dma_sar0.b.l = data;
		break;

	case 0x21:
		LOG("Z180 SAR0H  wr $%02x\n", data);
		m_dma_sar0.b.h = data;
		break;

	case 0x22:
		LOG("Z180 SAR0B  wr $%02x ($%02x)\n", data,  data & (Z180_SAR0_MASK >> 16));
		m_dma_sar0.b.h2 = data & (Z180_SAR0_MASK >> 16);
		break;

	case 0x23:
		LOG("Z180 DAR0L  wr $%02x\n", data);
		m_dma_dar0.b.l = data;
		break;

	case 0x24:
		LOG("Z180 DAR0H  wr $%02x\n", data);
		m_dma_dar0.b.h = data;
		break;

	case 0x25:
		LOG("Z180 DAR0B  wr $%02x ($%02x)\n", data,  data & (Z180_DAR0_MASK >> 16));
		m_dma_dar0.b.h2 = data & (Z180_DAR0_MASK >> 16);
		break;

	case 0x26:
		LOG("Z180 BCR0L  wr $%02x\n", data);
		m_dma_bcr[0].b.l = data;
		break;

	case 0x27:
		LOG("Z180 BCR0H  wr $%02x\n", data);
		m_dma_bcr[0].b.h = data;
		break;

	case 0x28:
		LOG("Z180 MAR1L  wr $%02x\n", data);
		m_dma_mar1.b.l = data;
		break;

	case 0x29:
		LOG("Z180 MAR1H  wr $%02x\n", data);
		m_dma_mar1.b.h = data;
		break;

	case 0x2a:
		LOG("Z180 MAR1B  wr $%02x ($%02x)\n", data,  data & (Z180_MAR1_MASK >> 16));
		m_dma_mar1.b.h2 = data & (Z180_MAR1_MASK >> 16);
		break;

	case 0x2b:
		LOG("Z180 IAR1L  wr $%02x\n", data);
		m_dma_iar1.b.l = data;
		break;

	case 0x2c:
		LOG("Z180 IAR1H  wr $%02x\n", data);
		m_dma_iar1.b.h = data;
		break;

	case 0x2d:
		LOG("Z180 IAR1B  wr $%02x ($%02x)\n", data,  data & (Z180_IAR1_MASK >> 16));
		m_dma_iar1.b.h2 = data & (Z180_IAR1_MASK >> 16);
		break;

	case 0x2e:
		LOG("Z180 BCR1L  wr $%02x\n", data);
		m_dma_bcr[1].b.l = data;
		break;

	case 0x2f:
		LOG("Z180 BCR1H  wr $%02x\n", data);
		m_dma_bcr[1].b.h = data;
		break;

	case 0x30:
		LOG("Z180 DSTAT  wr $%02x ($%02x)\n", data,  data & Z180_DSTAT_MASK & ~Z180_DSTAT_DME);
		m_dstat = (m_dstat & Z180_DSTAT_DME) | (data & Z180_DSTAT_MASK & ~Z180_DSTAT_DME);
		if ((data & (Z180_DSTAT_DE1 | Z180_DSTAT_DWE1)) == Z180_DSTAT_DE1)
		{
			m_dstat |= Z180_DSTAT_DME;  /* DMA enable */
		}
		if ((data & (Z180_DSTAT_DE0 | Z180_DSTAT_DWE0)) == Z180_DSTAT_DE0)
		{
			m_dstat |= Z180_DSTAT_DME;  /* DMA enable */
		}
		break;

	case 0x31:
		LOG("Z180 DMODE  wr $%02x ($%02x)\n", data,  data & Z180_DMODE_MASK);
		m_dmode = data & Z180_DMODE_MASK;
		break;

	case 0x32:
		LOG("Z180 DCNTL  wr $%02x\n", data);
		m_dcntl = data;
		break;

	case 0x33:
		LOG("Z180 IL     wr $%02x ($%02x)\n", data,  data & Z180_IL_MASK);
		m_il = data & Z180_IL_MASK;
		break;

	case 0x34:
		LOG("Z180 ITC    wr $%02x ($%02x)\n", data,  data & Z180_ITC_MASK & ~Z180_ITC_UFO);
		m_itc = (m_itc & Z180_ITC_UFO) | (data & Z180_ITC_MASK & ~Z180_ITC_UFO);
		break;

	case 0x36:
		LOG("Z180 RCR    wr $%02x ($%02x)\n", data,  data & Z180_RCR_MASK);
		m_rcr = data & Z180_RCR_MASK;
		break;

	case 0x38:
		LOG("Z180 CBR    wr $%02x\n", data);
		m_mmu_cbr = data;
		z180_mmu();
		break;

	case 0x39:
		LOG("Z180 BBR    wr $%02x\n", data);
		m_mmu_bbr = data;
		z180_mmu();
		break;

	case 0x3a:
		LOG("Z180 CBAR   wr $%02x\n", data);
		m_mmu_cbar = data;
		z180_mmu();
		break;

	case 0x3e:
		LOG("Z180 OMCR   wr $%02x ($%02x)\n", data,  data & Z180_OMCR_MASK);
		m_omcr = data & Z180_OMCR_MASK;
		break;

	case 0x3f:
		LOG("Z180 IOCR   wr $%02x ($%02x)\n", data,  data & (m_extended_io ? 0xa0 : 0xe0));
		m_iocr = data & (m_extended_io ? 0xa0 : 0xe0);
		break;

	default:
		LOG("Z180 IO%02X   wr $%02x\n", port, data);
		break;

	}
}

uint8_t z8s180_device::z180_internal_port_read(uint8_t port)
{
	uint8_t data = 0xff;

	switch (port)
	{
	case 0x12:
	case 0x13:
		data = m_asci[port & 1]->asext_r();
		break;

	case 0x1a:
	case 0x1b:
		data = m_asci[port & 1]->astcl_r();
		break;

	case 0x1c:
	case 0x1d:
		data = m_asci[port & 1]->astcl_r();
		break;

	case 0x1e:
		data = m_cmr | ~Z180_CMR_MASK;
		LOG("Z180 CMR    rd $%02x ($%02x)\n", data, m_cmr);
		break;

	case 0x1f:
		data = m_ccr;
		LOG("Z180 CCR    rd $%02x\n", data);
		break;

	default:
		data = z180_device::z180_internal_port_read(port);
		break;
	}

	return data;
}

void z8s180_device::z180_internal_port_write(uint8_t port, uint8_t data)
{
	switch (port)
	{
	case 0x12:
	case 0x13:
		m_asci[port & 1]->asext_w(data);
		break;

	case 0x1a:
	case 0x1b:
		m_asci[port & 1]->astcl_w(data);
		break;

	case 0x1c:
	case 0x1d:
		m_asci[port & 1]->astch_w(data);
		break;

	case 0x1e:
		LOG("Z180 CMR    wr $%02x ($%02x)\n", data,  data & Z180_CMR_MASK);
		m_cmr = data & Z180_CMR_MASK;
		notify_clock_changed();
		break;

	case 0x1f:
		LOG("Z180 CCR    wr $%02x\n", data);
		m_ccr = data;
		notify_clock_changed();
		break;

	default:
		z180_device::z180_internal_port_write(port, data);
		break;
	}
}

int z180_device::z180_dma0(int max_cycles)
{
	offs_t sar0 = m_dma_sar0.d;
	offs_t dar0 = m_dma_dar0.d;
	int bcr0 = m_dma_bcr[0].w;

	if (bcr0 == 0)
	{
		bcr0 = 0x10000;
	}

	int count = (m_dmode & Z180_DMODE_MMOD) ? bcr0 : 1;
	int cycles = 0;

	if (!(m_dstat & Z180_DSTAT_DE0))
	{
		return 0;
	}

	while (count > 0)
	{
		m_extra_cycles = 0;
		/* last transfer happening now? */
		if (bcr0 == 1)
		{
			m_iol |= Z180_TEND0;
			m_tend0_cb(ASSERT_LINE);
		}
		switch( m_dmode & (Z180_DMODE_SM | Z180_DMODE_DM) )
		{
		case 0x00:  /* memory SAR0+1 to memory DAR0+1 */
			z180_write_memory(dar0++, z180_read_memory(sar0++));
			cycles += memory_wait_states() * 2;
			bcr0--;
			break;
		case 0x04:  /* memory SAR0-1 to memory DAR0+1 */
			z180_write_memory(dar0++, z180_read_memory(sar0--));
			cycles += memory_wait_states() * 2;
			bcr0--;
			break;
		case 0x08:  /* memory SAR0 fixed to memory DAR0+1 */
			z180_write_memory(dar0++, z180_read_memory(sar0));
			cycles += memory_wait_states() * 2;
			bcr0--;
			break;
		case 0x0c:  /* I/O SAR0 fixed to memory DAR0+1 */
			if (m_iol & Z180_DREQ0)
			{
				z180_write_memory(dar0++, IN(sar0));
				cycles += memory_wait_states();
				bcr0--;
				/* edge sensitive DREQ0 ? */
				if (m_dcntl & Z180_DCNTL_DMS0)
				{
					m_iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x10:  /* memory SAR0+1 to memory DAR0-1 */
			z180_write_memory(dar0--, z180_read_memory(sar0++));
			cycles += memory_wait_states() * 2;
			bcr0--;
			break;
		case 0x14:  /* memory SAR0-1 to memory DAR0-1 */
			z180_write_memory(dar0--, z180_read_memory(sar0--));
			cycles += memory_wait_states() * 2;
			bcr0--;
			break;
		case 0x18:  /* memory SAR0 fixed to memory DAR0-1 */
			z180_write_memory(dar0--, z180_read_memory(sar0));
			cycles += memory_wait_states() * 2;
			bcr0--;
			break;
		case 0x1c:  /* I/O SAR0 fixed to memory DAR0-1 */
			if (m_iol & Z180_DREQ0)
			{
				z180_write_memory(dar0--, IN(sar0));
				cycles += memory_wait_states();
				bcr0--;
				/* edge sensitive DREQ0 ? */
				if (m_dcntl & Z180_DCNTL_DMS0)
				{
					m_iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x20:  /* memory SAR0+1 to memory DAR0 fixed */
			z180_write_memory(dar0, z180_read_memory(sar0++));
			cycles += memory_wait_states() * 2;
			bcr0--;
			break;
		case 0x24:  /* memory SAR0-1 to memory DAR0 fixed */
			z180_write_memory(dar0, z180_read_memory(sar0--));
			cycles += memory_wait_states() * 2;
			bcr0--;
			break;
		case 0x28:  /* reserved */
			break;
		case 0x2c:  /* reserved */
			break;
		case 0x30:  /* memory SAR0+1 to I/O DAR0 fixed */
			if (m_iol & Z180_DREQ0)
			{
				OUT(dar0, z180_read_memory(sar0++));
				cycles += memory_wait_states();
				bcr0--;
				/* edge sensitive DREQ0 ? */
				if (m_dcntl & Z180_DCNTL_DMS0)
				{
					m_iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x34:  /* memory SAR0-1 to I/O DAR0 fixed */
			if (m_iol & Z180_DREQ0)
			{
				OUT(dar0, z180_read_memory(sar0--));
				cycles += memory_wait_states();
				bcr0--;
				/* edge sensitive DREQ0 ? */
				if (m_dcntl & Z180_DCNTL_DMS0)
				{
					m_iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x38:  /* reserved */
			break;
		case 0x3c:  /* reserved */
			break;
		}
		count--;
		cycles += 6 + m_extra_cycles; // use extra_cycles for I/O wait states
		if (cycles > max_cycles)
			break;
	}

	m_dma_sar0.d = sar0;
	m_dma_dar0.d = dar0;
	m_dma_bcr[0].w = bcr0;

	/* DMA terminal count? */
	if (bcr0 == 0)
	{
		m_iol &= ~Z180_TEND0;
		m_tend0_cb(CLEAR_LINE);
		m_dstat &= ~Z180_DSTAT_DE0;
		/* terminal count interrupt enabled? */
		if (m_dstat & Z180_DSTAT_DIE0 && m_IFF1)
			m_int_pending[Z180_INT_DMA0] = 1;
	}
	return cycles;
}

int z180_device::z180_dma1()
{
	offs_t mar1 = m_dma_mar1.d;
	offs_t iar1 = m_dma_iar1.w.l;
	int bcr1 = m_dma_bcr[1].w;

	if (bcr1 == 0)
	{
		bcr1 = 0x10000;
	}

	int cycles = 0;

	if ((m_iol & Z180_DREQ1) == 0)
		return 0;

	if (!(m_dstat & Z180_DSTAT_DE1))
	{
		return 0;
	}

	/* last transfer happening now? */
	if (bcr1 == 1)
	{
		m_iol |= Z180_TEND1;
		m_tend1_cb(ASSERT_LINE);
	}

	m_extra_cycles = 0;

	switch (m_dcntl & (Z180_DCNTL_DIM1 | Z180_DCNTL_DIM0))
	{
	case 0x00:  /* memory MAR1+1 to I/O IAR1 fixed */
		m_io.write_byte(iar1, z180_read_memory(mar1++));
		bcr1--;
		break;
	case 0x01:  /* memory MAR1-1 to I/O IAR1 fixed */
		m_io.write_byte(iar1, z180_read_memory(mar1--));
		bcr1--;
		break;
	case 0x02:  /* I/O IAR1 fixed to memory MAR1+1 */
		z180_write_memory(mar1++, m_io.read_byte(iar1));
		bcr1--;
		break;
	case 0x03:  /* I/O IAR1 fixed to memory MAR1-1 */
		z180_write_memory(mar1--, m_io.read_byte(iar1));
		bcr1--;
		break;
	}

	cycles += memory_wait_states();
	cycles += m_extra_cycles; // use extra_cycles for I/O wait states

	/* edge sensitive DREQ1 ? */
	if (m_dcntl & Z180_DCNTL_DIM1)
		m_iol &= ~Z180_DREQ1;

	m_dma_mar1.d = mar1;
	m_dma_bcr[1].w = bcr1;

	/* DMA terminal count? */
	if (bcr1 == 0)
	{
		m_iol &= ~Z180_TEND1;
		m_tend1_cb(CLEAR_LINE);
		m_dstat &= ~Z180_DSTAT_DE1;
		if (m_dstat & Z180_DSTAT_DIE1 && m_IFF1)
			m_int_pending[Z180_INT_DMA1] = 1;
	}

	/* six cycles per transfer (minimum) */
	return 6 + cycles;
}

void z180_device::z180_write_iolines(uint32_t data)
{
	uint32_t changes = m_iol ^ data;

	/* I/O asynchronous clock 0 (active high) or DREQ0 (mux) */
	if (changes & Z180_CKA0)
	{
		LOG("Z180 CKA0   %d\n", data & Z180_CKA0 ? 1 : 0);
		m_iol = (m_iol & ~Z180_CKA0) | (data & Z180_CKA0);
	}

	/* I/O asynchronous clock 1 (active high) or TEND1 (mux) */
	if (changes & Z180_CKA1)
	{
		LOG("Z180 CKA1   %d\n", data & Z180_CKA1 ? 1 : 0);
		m_iol = (m_iol & ~Z180_CKA1) | (data & Z180_CKA1);
	}

	/* I/O serial clock (active high) */
	if (changes & Z180_CKS)
	{
		LOG("Z180 CKS    %d\n", data & Z180_CKS ? 1 : 0);
		m_iol = (m_iol & ~Z180_CKS) | (data & Z180_CKS);
	}

	/* I   clear to send 0 (active low) */
	if (changes & Z180_CTS0)
	{
		LOG("Z180 CTS0   %d\n", data & Z180_CTS0 ? 1 : 0);
		m_iol = (m_iol & ~Z180_CTS0) | (data & Z180_CTS0);
	}

	/* I   clear to send 1 (active low) or RXS (mux) */
	if (changes & Z180_CTS1)
	{
		LOG("Z180 CTS1   %d\n", data & Z180_CTS1 ? 1 : 0);
		m_iol = (m_iol & ~Z180_CTS1) | (data & Z180_CTS1);
	}

	/* I   data carrier detect (active low) */
	if (changes & Z180_DCD0)
	{
		LOG("Z180 DCD0   %d\n", data & Z180_DCD0 ? 1 : 0);
		m_iol = (m_iol & ~Z180_DCD0) | (data & Z180_DCD0);
	}

	/* I   data request DMA ch 0 (active low) or CKA0 (mux) */
	if (changes & Z180_DREQ0)
	{
		LOG("Z180 DREQ0  %d\n", data & Z180_DREQ0 ? 1 : 0);
		m_iol = (m_iol & ~Z180_DREQ0) | (data & Z180_DREQ0);
	}

	/* I   data request DMA ch 1 (active low) */
	if (changes & Z180_DREQ1)
	{
		LOG("Z180 DREQ1  %d\n", data & Z180_DREQ1 ? 1 : 0);
		m_iol = (m_iol & ~Z180_DREQ1) | (data & Z180_DREQ1);
	}

	/* I   asynchronous receive data 0 (active high) */
	if (changes & Z180_RXA0)
	{
		LOG("Z180 RXA0   %d\n", data & Z180_RXA0 ? 1 : 0);
		m_iol = (m_iol & ~Z180_RXA0) | (data & Z180_RXA0);
	}

	/* I   asynchronous receive data 1 (active high) */
	if (changes & Z180_RXA1)
	{
		LOG("Z180 RXA1   %d\n", data & Z180_RXA1 ? 1 : 0);
		m_iol = (m_iol & ~Z180_RXA1) | (data & Z180_RXA1);
	}

	/* I   clocked serial receive data (active high) or CTS1 (mux) */
	if (changes & Z180_RXS)
	{
		LOG("Z180 RXS    %d\n", data & Z180_RXS ? 1 : 0);
		m_iol = (m_iol & ~Z180_RXS) | (data & Z180_RXS);
	}

	/*   O request to send (active low) */
	if (changes & Z180_RTS0)
	{
		LOG("Z180 RTS0   won't change output\n");
	}

	/*   O transfer end 0 (active low) or CKA1 (mux) */
	if (changes & Z180_TEND0)
	{
		LOG("Z180 TEND0  won't change output\n");
	}

	/*   O transfer end 1 (active low) */
	if (changes & Z180_TEND1)
	{
		LOG("Z180 TEND1  won't change output\n");
	}

	/*   O transfer out (PRT channel, active low) or A18 (mux) */
	if (changes & Z180_A18_TOUT)
	{
		LOG("Z180 TOUT   won't change output\n");
	}

	/*   O asynchronous transmit data 0 (active high) */
	if (changes & Z180_TXA0)
	{
		LOG("Z180 TXA0   won't change output\n");
	}

	/*   O asynchronous transmit data 1 (active high) */
	if (changes & Z180_TXA1)
	{
		LOG("Z180 TXA1   won't change output\n");
	}

	/*   O clocked serial transmit data (active high) */
	if (changes & Z180_TXS)
	{
		LOG("Z180 TXS    won't change output\n");
	}
}

void z180_device::device_start()
{
	int i, p;
	int oldval, newval, val;
	uint8_t *padd, *padc, *psub, *psbc;

	/* allocate big flag arrays once */
	SZHVC_add = std::make_unique<uint8_t[]>(2*256*256);
	SZHVC_sub = std::make_unique<uint8_t[]>(2*256*256);

	padd = &SZHVC_add[  0*256];
	padc = &SZHVC_add[256*256];
	psub = &SZHVC_sub[  0*256];
	psbc = &SZHVC_sub[256*256];
	for (oldval = 0; oldval < 256; oldval++)
	{
		for (newval = 0; newval < 256; newval++)
		{
			/* add or adc w/o carry set */
			val = newval - oldval;
			*padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
			*padd |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */

			if( (newval & 0x0f) < (oldval & 0x0f) ) *padd |= HF;
			if( newval < oldval ) *padd |= CF;
			if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padd |= VF;
			padd++;

			/* adc with carry set */
			val = newval - oldval - 1;
			*padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
			*padc |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
			if( (newval & 0x0f) <= (oldval & 0x0f) ) *padc |= HF;
			if( newval <= oldval ) *padc |= CF;
			if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padc |= VF;
			padc++;

			/* cp, sub or sbc w/o carry set */
			val = oldval - newval;
			*psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
			*psub |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
			if( (newval & 0x0f) > (oldval & 0x0f) ) *psub |= HF;
			if( newval > oldval ) *psub |= CF;
			if( (val^oldval) & (oldval^newval) & 0x80 ) *psub |= VF;
			psub++;

			/* sbc with carry set */
			val = oldval - newval - 1;
			*psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
			*psbc |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
			if( (newval & 0x0f) >= (oldval & 0x0f) ) *psbc |= HF;
			if( newval >= oldval ) *psbc |= CF;
			if( (val^oldval) & (oldval^newval) & 0x80 ) *psbc |= VF;
			psbc++;
		}
	}
	for (i = 0; i < 256; i++)
	{
		p = 0;
		if( i&0x01 ) ++p;
		if( i&0x02 ) ++p;
		if( i&0x04 ) ++p;
		if( i&0x08 ) ++p;
		if( i&0x10 ) ++p;
		if( i&0x20 ) ++p;
		if( i&0x40 ) ++p;
		if( i&0x80 ) ++p;
		SZ[i] = i ? i & SF : ZF;
		SZ[i] |= (i & (YF | XF));       /* undocumented flag bits 5+3 */
		SZ_BIT[i] = i ? i & SF : ZF | PF;
		SZ_BIT[i] |= (i & (YF | XF));   /* undocumented flag bits 5+3 */
		SZP[i] = SZ[i] | ((p & 1) ? 0 : PF);
		SZHV_inc[i] = SZ[i];
		if( i == 0x80 ) SZHV_inc[i] |= VF;
		if( (i & 0x0f) == 0x00 ) SZHV_inc[i] |= HF;
		SZHV_dec[i] = SZ[i] | NF;
		if( i == 0x7f ) SZHV_dec[i] |= VF;
		if( (i & 0x0f) == 0x0f ) SZHV_dec[i] |= HF;
	}

	space(AS_PROGRAM).specific(m_program);
	space(AS_PROGRAM).cache(m_cprogram);
	space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).cache(m_copcodes);
	space(AS_IO).specific(m_io);

	/* set up the state table */
	{
		state_add(Z180_PC,         "PC",        m_PC.w.l);
		state_add(STATE_GENPC,     "GENPC",     _PCD).noshow();
		state_add(STATE_GENPCBASE, "CURPC",     m_PREPC.w.l).noshow();
		state_add(Z180_SP,         "SP",        m_SP.w.l);
		state_add(STATE_GENFLAGS,  "GENFLAGS",  m_AF.b.l).noshow().formatstr("%8s");
		state_add(Z180_A,          "A",         _A).noshow();
		state_add(Z180_B,          "B",         _B).noshow();
		state_add(Z180_C,          "C",         _C).noshow();
		state_add(Z180_D,          "D",         _D).noshow();
		state_add(Z180_E,          "E",         _E).noshow();
		state_add(Z180_H,          "H",         _H).noshow();
		state_add(Z180_L,          "L",         _L).noshow();
		state_add(Z180_AF,         "AF",        m_AF.w.l);
		state_add(Z180_BC,         "BC",        m_BC.w.l);
		state_add(Z180_DE,         "DE",        m_DE.w.l);
		state_add(Z180_HL,         "HL",        m_HL.w.l);
		state_add(Z180_IX,         "IX",        m_IX.w.l);
		state_add(Z180_IY,         "IY",        m_IY.w.l);
		state_add(Z180_AF2,        "AF2",       m_AF2.w.l);
		state_add(Z180_BC2,        "BC2",       m_BC2.w.l);
		state_add(Z180_DE2,        "DE2",       m_DE2.w.l);
		state_add(Z180_HL2,        "HL2",       m_HL2.w.l);
		state_add(Z180_R,          "R",         m_rtemp).callimport().callexport();
		state_add(Z180_I,          "I",         m_I);
		state_add(Z180_IM,         "IM",        m_IM).mask(0x3);
		state_add(Z180_IFF1,       "IFF1",      m_IFF1).mask(0x1);
		state_add(Z180_IFF2,       "IFF2",      m_IFF2).mask(0x1);
		state_add(Z180_HALT,       "HALT",      m_HALT).mask(0x1);

		state_add(Z180_IOLINES,    "IOLINES",   m_ioltemp).mask(0xffffff).callimport();

		m_asci[0]->state_add(*this);
		m_asci[1]->state_add(*this);
		m_csio->state_add(*this);

		state_add(Z180_TMDR0,      "TMDR0",     m_tmdr_value[0]);
		state_add(Z180_RLDR0,      "RLDR0",     m_rldr[0].w);
		state_add(Z180_TMDR1,      "TMDR1",     m_tmdr_value[1]);
		state_add(Z180_RLDR1,      "RLDR1",     m_rldr[1].w);
		state_add(Z180_TCR,        "TCR",       m_tcr);

		state_add(Z180_FRC,        "FRC",       m_frc);

		state_add(Z180_SAR0,       "SAR0",      m_dma_sar0.d).mask(Z180_SAR0_MASK);
		state_add(Z180_DAR0,       "DAR0",      m_dma_dar0.d).mask(Z180_DAR0_MASK);
		state_add(Z180_BCR0,       "BCR0",      m_dma_bcr[0].w);
		state_add(Z180_MAR1,       "MAR1",      m_dma_mar1.d).mask(Z180_MAR1_MASK);
		state_add(Z180_IAR1,       "IAR1",      m_dma_iar1.d).mask(Z180_IAR1_MASK);
		state_add(Z180_BCR1,       "BCR1",      m_dma_bcr[1].w);
		state_add(Z180_DSTAT,      "DSTAT",     m_dstat).mask(Z180_DSTAT_MASK);
		state_add(Z180_DMODE,      "DMODE",     m_dmode).mask(Z180_DMODE_MASK);
		state_add(Z180_DCNTL,      "DCNTL",     m_dcntl);
		state_add(Z180_IL,         "IL",        m_il).mask(Z180_IL_MASK);
		state_add(Z180_ITC,        "ITC",       m_itc).mask(Z180_ITC_MASK);
		state_add(Z180_RCR,        "RCR",       m_rcr).mask(Z180_RCR_MASK);
		state_add(Z180_CBR,        "CBR",       m_mmu_cbr).callimport();
		state_add(Z180_BBR,        "BBR",       m_mmu_bbr).callimport();
		state_add(Z180_CBAR,       "CBAR",      m_mmu_cbar).callimport();
		state_add(Z180_OMCR,       "OMCR",      m_omcr).mask(Z180_OMCR_MASK);
		state_add(Z180_IOCR,       "IOCR",      m_iocr).mask(m_extended_io ? 0xa0 : 0xe0);
	}

	save_item(NAME(m_AF.w.l));
	save_item(NAME(m_BC.w.l));
	save_item(NAME(m_DE.w.l));
	save_item(NAME(m_HL.w.l));
	save_item(NAME(m_IX.w.l));
	save_item(NAME(m_IY.w.l));
	save_item(NAME(m_PC.w.l));
	save_item(NAME(m_SP.w.l));
	save_item(NAME(m_AF2.w.l));
	save_item(NAME(m_BC2.w.l));
	save_item(NAME(m_DE2.w.l));
	save_item(NAME(m_HL2.w.l));
	save_item(NAME(m_R));
	save_item(NAME(m_R2));
	save_item(NAME(m_IFF1));
	save_item(NAME(m_IFF2));
	save_item(NAME(m_HALT));
	save_item(NAME(m_IM));
	save_item(NAME(m_I));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_int_pending));
	save_item(NAME(m_frc_prescale));
	save_item(NAME(m_after_EI));

	save_item(NAME(m_read_tcr_tmdr));
	save_item(NAME(m_tmdr_value));
	save_item(NAME(m_tmdrh));
	save_item(NAME(m_tmdr_latch));

	save_item(NAME(m_tmdr[0].w));
	save_item(NAME(m_tmdr[1].w));
	save_item(NAME(m_rldr[0].w));
	save_item(NAME(m_rldr[1].w));
	save_item(NAME(m_tcr));
	save_item(NAME(m_frc));
	save_item(NAME(m_dma_sar0.d));
	save_item(NAME(m_dma_dar0.d));
	save_item(NAME(m_dma_bcr[0].w));
	save_item(NAME(m_dma_bcr[1].w));
	save_item(NAME(m_dma_mar1.d));
	save_item(NAME(m_dma_iar1.d));
	save_item(NAME(m_dstat));
	save_item(NAME(m_dmode));
	save_item(NAME(m_dcntl));
	save_item(NAME(m_il));
	save_item(NAME(m_itc));
	save_item(NAME(m_rcr));
	save_item(NAME(m_mmu_cbr));
	save_item(NAME(m_mmu_bbr));
	save_item(NAME(m_mmu_cbar));
	save_item(NAME(m_omcr));
	save_item(NAME(m_iocr));

	save_item(NAME(m_iol));
	save_item(NAME(m_ioltemp));

	save_item(NAME(m_mmu));

	set_icountptr(m_icount);
}

void z8s180_device::device_start()
{
	z180_device::device_start();

	state_add(Z180_CMR,        "CMR",       m_cmr).mask(Z180_CMR_MASK);
	state_add(Z180_CCR,        "CCR",       m_ccr);

	save_item(NAME(m_cmr));
	save_item(NAME(m_ccr));
}

/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
void z180_device::device_reset()
{
	_PPC = 0;
	_PCD = 0;
	_SPD = 0;
	_AFD = 0;
	_BCD = 0;
	_DED = 0;
	_HLD = 0;
	_IXD = 0;
	_IYD = 0;
	m_AF2.d = 0;
	m_BC2.d = 0;
	m_DE2.d = 0;
	m_HL2.d = 0;
	m_R = 0;
	m_R2 = 0;
	m_IFF1 = 0;
	m_IFF2 = 0;
	m_HALT = 0;
	m_IM = 0;
	m_I = 0;
	m_tmdr_latch = 0;
	m_read_tcr_tmdr[0] = 0;
	m_read_tcr_tmdr[1] = 0;
	m_iol = 0;
	memset(m_mmu, 0, sizeof(m_mmu));
	m_tmdrh[0] = 0;
	m_tmdrh[1] = 0;
	m_tmdr_value[0] = 0xffff;
	m_tmdr_value[1] = 0xffff;
	m_nmi_state = CLEAR_LINE;
	m_nmi_pending = 0;
	m_irq_state[0] = CLEAR_LINE;
	m_irq_state[1] = CLEAR_LINE;
	m_irq_state[2] = CLEAR_LINE;
	m_after_EI = 0;
	m_ea = 0;

	memcpy(m_cc, (uint8_t *)cc_default, sizeof(m_cc));
	_IX = _IY = 0xffff; /* IX and IY are FFFF after a reset! */
	_F = ZF;          /* Zero flag is set */

	for (int i=0; i <= Z180_INT_MAX; i++)
	{
		m_int_pending[i] = 0;
	}

	m_frc = 0xff;
	m_frc_prescale = 0;

	/* reset io registers */
	m_tcr = 0x00;
	m_dma_iar1.b.h2 = 0x00;
	m_dstat = Z180_DSTAT_DWE1 | Z180_DSTAT_DWE0;
	m_dmode = 0x00;
	m_dcntl = 0xf0; // maximum number of memory and I/O wait states
	m_il = 0x00;
	m_itc = Z180_ITC_ITE0;
	m_rcr = Z180_RCR_REFE | Z180_RCR_REFW;
	m_mmu_cbr = 0x00;
	m_mmu_bbr = 0x00;
	m_mmu_cbar = 0xf0;
	m_omcr = Z180_OMCR_M1E | Z180_OMCR_M1TE | Z180_OMCR_IOC;
	m_iocr = 0x00;

	z180_mmu();
}

void z180_device::device_add_mconfig(machine_config &config)
{
	Z180ASCI_CHANNEL_0(config, m_asci[0], DERIVED_CLOCK(1,2));

	Z180ASCI_CHANNEL_1(config, m_asci[1], DERIVED_CLOCK(1,2));

	Z180CSIO(config, m_csio, DERIVED_CLOCK(1,2));
}

void z8s180_device::device_add_mconfig(machine_config &config)
{
	Z180ASCI_EXT_CHANNEL_0(config, m_asci[0], DERIVED_CLOCK(1,2));

	Z180ASCI_EXT_CHANNEL_1(config, m_asci[1], DERIVED_CLOCK(1,2));

	Z180CSIO(config, m_csio, DERIVED_CLOCK(1,2));
}

void z8s180_device::device_reset()
{
	z180_device::device_reset();

	m_cmr = 0x00;
	m_ccr = 0x00;
	notify_clock_changed();
}

void z8s180_device::device_clock_changed()
{
	auto const rate = (m_cmr & 0x80) ? DERIVED_CLOCK(2,1) : (m_ccr & 0x80) ? DERIVED_CLOCK(1,1) : DERIVED_CLOCK(1,2);
	m_asci[0]->set_clock(rate);
	m_asci[1]->set_clock(rate);
	m_csio->set_clock(rate);
}

/* Handle PRT timers, decreasing them after 20 clocks and returning the new icount base that needs to be used for the next check */
void z180_device::clock_timers()
{
	/* Programmable Reload Timer 0 */
	if(m_tcr & Z180_TCR_TDE0)
	{
		if(m_tmdr_value[0] == 0)
		{
			m_tmdr_value[0] = m_rldr[0].w;
			m_tcr |= Z180_TCR_TIF0;
		}
		else
			m_tmdr_value[0]--;
	}

	/* Programmable Reload Timer 1 */
	if(m_tcr & Z180_TCR_TDE1)
	{
		if(m_tmdr_value[1] == 0)
		{
			m_tmdr_value[1] = m_rldr[1].w;
			m_tcr |= Z180_TCR_TIF1;
		}
		else
			m_tmdr_value[1]--;
	}

	if((m_tcr & Z180_TCR_TIE0) && (m_tcr & Z180_TCR_TIF0))
	{
		// check if we can take the interrupt
		if(m_IFF1 && !m_after_EI)
			m_int_pending[Z180_INT_PRT0] = 1;
	}

	if((m_tcr & Z180_TCR_TIE1) && (m_tcr & Z180_TCR_TIF1))
	{
		// check if we can take the interrupt
		if(m_IFF1 && !m_after_EI)
			m_int_pending[Z180_INT_PRT1] = 1;
	}
}

int z180_device::check_interrupts()
{
	/* check for IRQs before each instruction */
	if (m_IFF1 && !m_after_EI)
	{
		if (m_irq_state[0] != CLEAR_LINE && (m_itc & Z180_ITC_ITE0) == Z180_ITC_ITE0)
			m_int_pending[Z180_INT_IRQ0] = 1;

		if (m_irq_state[1] != CLEAR_LINE && (m_itc & Z180_ITC_ITE1) == Z180_ITC_ITE1)
			m_int_pending[Z180_INT_IRQ1] = 1;

		if (m_irq_state[2] != CLEAR_LINE && (m_itc & Z180_ITC_ITE2) == Z180_ITC_ITE2)
			m_int_pending[Z180_INT_IRQ2] = 1;

		m_int_pending[Z180_INT_CSIO] = m_csio->check_interrupt();
		m_int_pending[Z180_INT_ASCI0] = m_asci[0]->check_interrupt();
		m_int_pending[Z180_INT_ASCI1] = m_asci[1]->check_interrupt();
	}

	int cycles = 0;
	for (int i = 0; i <= Z180_INT_MAX; i++)
	{
		if (m_int_pending[i])
		{
			cycles += take_interrupt(i);
			m_int_pending[i] = 0;
			switch (i)
			{
			case Z180_INT_ASCI0: m_asci[0]->clear_interrupt(); break;
			case Z180_INT_ASCI1: m_asci[1]->clear_interrupt(); break;
			}
			break;
		}
	}

	return cycles;
}

/****************************************************************************
 * Handle I/O and timers
 ****************************************************************************/

void z180_device::handle_io_timers(int cycles)
{
	while (cycles-- > 0)
	{
		// FRC counts down by 1 every 10 cycles
		m_frc_prescale++;
		if (m_frc_prescale >= 10)
		{
			m_frc_prescale = 0;
			m_frc--;

			// Programmable reload timers are clocked once every 20 cycles
			if ((m_frc & 1) == 0)
				clock_timers();
		}
	}
}

/****************************************************************************
 * Execute 'cycles' T-states. Return number of T-states really executed
 ****************************************************************************/
void z180_device::execute_run()
{
	int curcycles;

	/* check for NMIs on the way in; they can only be set externally */
	/* via timers, and can't be dynamically enabled, so it is safe */
	/* to just check here */
	if (m_nmi_pending)
	{
		LOG("Z180 take NMI\n");
		LEAVE_HALT();       /* Check if processor was halted */

		/* disable DMA transfers!! */
		m_dstat &= ~Z180_DSTAT_DME;

		m_IFF2 = m_IFF1;
		m_IFF1 = 0;
		PUSH( PC );
		_PCD = 0x0066;
		m_icount -= 11;
		m_nmi_pending = 0;
		handle_io_timers(11);
	}

again:
	/* check if any DMA transfer is running */
	if ((m_dstat & Z180_DSTAT_DME) == Z180_DSTAT_DME)
	{
		/* check if DMA channel 0 is running and also is in burst mode */
		if ((m_dstat & Z180_DSTAT_DE0) == Z180_DSTAT_DE0 &&
			(m_dmode & Z180_DMODE_MMOD) == Z180_DMODE_MMOD)
		{
			debugger_instruction_hook(_PCD);

			/* FIXME z180_dma0 should be handled in handle_io_timers */
			curcycles = z180_dma0(m_icount);
			m_icount -= curcycles;
			handle_io_timers(curcycles);
		}
		else
		{
			do
			{
				curcycles = check_interrupts();
				m_icount -= curcycles;
				handle_io_timers(curcycles);
				m_after_EI = 0;

				_PPC = _PCD;
				debugger_instruction_hook(_PCD);

				if (!m_HALT)
				{
					m_R++;
					m_extra_cycles = 0;
					curcycles = exec_op(ROP());
					curcycles += m_extra_cycles;
				}
				else
					curcycles = 3;

				m_icount -= curcycles;

				handle_io_timers(curcycles);

				/* if channel 0 was started in burst mode, go recheck the mode */
				if ((m_dstat & Z180_DSTAT_DE0) == Z180_DSTAT_DE0 &&
					(m_dmode & Z180_DMODE_MMOD) == Z180_DMODE_MMOD)
					goto again;

				/* FIXME:
				 * For simultaneous DREQ0 and DREQ1 requests, channel 0 has priority
				 * over channel 1. When channel 0 is performing a memory to/from memory
				 * transfer, channel 1 cannot operate until the channel 0 operation has
				 * terminated. If channel 1 is operating, channel 0 cannot operate until
				 * channel 1 releases control of the bus.
				 *
				 */
				curcycles = z180_dma0(6);
				m_icount -= curcycles;
				handle_io_timers(curcycles);

				curcycles = z180_dma1();
				m_icount -= curcycles;
				handle_io_timers(curcycles);

				/* If DMA is done break out to the faster loop */
				if ((m_dstat & Z180_DSTAT_DME) != Z180_DSTAT_DME)
					break;
			} while( m_icount > 0 );
		}
	}

	if (m_icount > 0)
	{
		do
		{
			/* If DMA is started go to check the mode */
			if ((m_dstat & Z180_DSTAT_DME) == Z180_DSTAT_DME)
				goto again;

			curcycles = check_interrupts();
			m_icount -= curcycles;
			handle_io_timers(curcycles);
			m_after_EI = 0;

			_PPC = _PCD;
			debugger_instruction_hook(_PCD);

			if (!m_HALT)
			{
				m_R++;
				m_extra_cycles = 0;
				curcycles = exec_op(ROP());
				curcycles += m_extra_cycles;
			}
			else
				curcycles = 3;

			m_icount -= curcycles;
			handle_io_timers(curcycles);
		} while( m_icount > 0 );
	}
}

/****************************************************************************
 * Burn 'cycles' T-states. Adjust R register for the lost time
 ****************************************************************************/
void z180_device::burn_cycles(int32_t cycles)
{
	int extra_cycles = memory_wait_states();

	/* FIXME: This is not appropriate for dma */
	while ( (cycles > 0) )
	{
		handle_io_timers(3 + extra_cycles);
		/* NOP takes 3 cycles per instruction */
		m_R += 1;
		m_icount -= 3 + extra_cycles;
		cycles -= 3 + extra_cycles;
	}
}

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
void z180_device::execute_set_input(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		/* mark an NMI pending on the rising edge */
		if (m_nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			m_nmi_pending = 1;
		m_nmi_state = state;
	}
	else
	{
		LOG("Z180 set_irq_line %d = %d\n", irqline,state);

		if(irqline == Z180_INPUT_LINE_IRQ0 || irqline == Z180_INPUT_LINE_IRQ1 || irqline == Z180_INPUT_LINE_IRQ2) {
			/* update the IRQ state */
			m_irq_state[irqline] = state;
			if(daisy_chain_present())
				m_irq_state[0] = daisy_update_irq_state();

			/* the main execute loop will take the interrupt */
		} else if(irqline == Z180_INPUT_LINE_DREQ0) {
			uint32_t iol = m_iol & ~Z180_DREQ0;
			if(state == ASSERT_LINE)
				iol |= Z180_DREQ0;
			z180_write_iolines(iol);
		} else if(irqline == Z180_INPUT_LINE_DREQ1) {
			uint32_t iol = m_iol & ~Z180_DREQ1;
			if(state == ASSERT_LINE)
				iol |= Z180_DREQ1;
			z180_write_iolines(iol);
		}
	}
}

/* logical to physical address translation */
bool z180_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	if (spacenum == AS_PROGRAM)
	{
		address = MMU_REMAP_ADDR(address);
	}
	target_space = &space(spacenum);
	return true;
}


/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

void z180_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case Z180_R:
			m_R = m_rtemp & 0x7f;
			m_R2 = m_rtemp & 0x80;
			break;

		case Z180_CBR:
		case Z180_BBR:
		case Z180_CBAR:
			z180_mmu();
			break;

		case Z180_IOLINES:
			z180_write_iolines(m_ioltemp);
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(z80) called for unexpected value\n");
	}
}


void z180_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case Z180_R:
			m_rtemp = (m_R & 0x7f) | (m_R2 & 0x80);
			break;

		case Z180_IOLINES:
			m_ioltemp = m_iol;
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(z80) called for unexpected value\n");
	}
}

void z180_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_AF.b.l & 0x80 ? 'S':'.',
				m_AF.b.l & 0x40 ? 'Z':'.',
				m_AF.b.l & 0x20 ? '5':'.',
				m_AF.b.l & 0x10 ? 'H':'.',
				m_AF.b.l & 0x08 ? '3':'.',
				m_AF.b.l & 0x04 ? 'P':'.',
				m_AF.b.l & 0x02 ? 'N':'.',
				m_AF.b.l & 0x01 ? 'C':'.');
			break;
	}
}
