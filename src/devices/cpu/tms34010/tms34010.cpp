// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari
/***************************************************************************

    TMS34010: Portable Texas Instruments TMS34010 emulator

    Copyright Alex Pasadyn/Zsolt Vasvari
    Parts based on code by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tms34010.h"


/***************************************************************************
    DEBUG STATE & STRUCTURES
***************************************************************************/

#define VERBOSE             0
#define LOG_CONTROL_REGS    0
#define LOG_GRAPHICS_OPS    0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


const device_type TMS34010 = &device_creator<tms34010_device>;
const device_type TMS34020 = &device_creator<tms34020_device>;


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

tms340x0_device::tms340x0_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, __FILE__)
	, device_video_interface(mconfig, *this)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 32, 3)
	, m_halt_on_reset(FALSE)
	, m_pixclock(0)
	, m_pixperclock(0)
	, m_output_int_cb(*this)
{
}


tms34010_device::tms34010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms340x0_device(mconfig, TMS34010, "TMS34010", tag, owner, clock, "tms34010")
{
	m_is_34020 = 0;
}


tms34020_device::tms34020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms340x0_device(mconfig, TMS34020, "TMS34020", tag, owner, clock, "tms34020")
{
	m_is_34020 = 1;
}


#include "34010ops.h"


/***************************************************************************
    MACROS
***************************************************************************/

/* status register definitions */
#define STBIT_N         (1 << 31)
#define STBIT_C         (1 << 30)
#define STBIT_Z         (1 << 29)
#define STBIT_V         (1 << 28)
#define STBIT_P         (1 << 25)
#define STBIT_IE        (1 << 21)
#define STBIT_FE1       (1 << 11)
#define STBITS_F1       (0x1f << 6)
#define STBIT_FE0       (1 << 5)
#define STBITS_F0       (0x1f << 0)

/* register definitions and shortcuts */
#define N_FLAG()       (m_st & STBIT_N)
#define Z_FLAG()       (m_st & STBIT_Z)
#define C_FLAG()       (m_st & STBIT_C)
#define V_FLAG()       (m_st & STBIT_V)
#define P_FLAG()       (m_st & STBIT_P)
#define IE_FLAG()      (m_st & STBIT_IE)
#define FE0_FLAG()     (m_st & STBIT_FE0)
#define FE1_FLAG()     (m_st & STBIT_FE1)

/* register file access */
#define AREG(i)       (m_regs[i].reg)
#define AREG_XY(i)    (m_regs[i].xy)
#define AREG_X(i)     (m_regs[i].xy.x)
#define AREG_Y(i)     (m_regs[i].xy.y)
#define BREG(i)       (m_regs[30 - (i)].reg)
#define BREG_XY(i)    (m_regs[30 - (i)].xy)
#define BREG_X(i)     (m_regs[30 - (i)].xy.x)
#define BREG_Y(i)     (m_regs[30 - (i)].xy.y)
#define SP()          AREG(15)
#define FW(i)         ((m_st >> (i ? 6 : 0)) & 0x1f)
#define FWEX(i)       ((m_st >> (i ? 6 : 0)) & 0x3f)

/* opcode decode helpers */
#define SRCREG(O)       (((O) >> 5) & 0x0f)
#define DSTREG(O)       ((O) & 0x0f)
#define SKIP_WORD()     (m_pc += (2 << 3))
#define SKIP_LONG()     (m_pc += (4 << 3))
#define PARAM_K(O)      (((O) >> 5) & 0x1f)
#define PARAM_N(O)      ((O) & 0x1f)
#define PARAM_REL8(O)   ((INT8)(O))

/* memory I/O */
#define WFIELD0(a,b)  (this->*s_wfield_functions[FW(0)])(a,b)
#define WFIELD1(a,b)  (this->*s_wfield_functions[FW(1)])(a,b)
#define RFIELD0(a)    (this->*s_rfield_functions[FWEX(0)])(a)
#define RFIELD1(a)    (this->*s_rfield_functions[FWEX(1)])(a)
#define WPIXEL(a,b)   (this->*m_pixel_write)(a,b)
#define RPIXEL(a)     (this->*m_pixel_read)(a)

/* Implied Operands */
#define SADDR()        BREG(0)
#define SADDR_X()      BREG_X(0)
#define SADDR_Y()      BREG_Y(0)
#define SADDR_XY()     BREG_XY(0)
#define SPTCH()        BREG(1)
#define DADDR()        BREG(2)
#define DADDR_X()      BREG_X(2)
#define DADDR_Y()      BREG_Y(2)
#define DADDR_XY()     BREG_XY(2)
#define DPTCH()        BREG(3)
#define OFFSET()       BREG(4)
#define WSTART_X()     BREG_X(5)
#define WSTART_Y()     BREG_Y(5)
#define WEND_X()       BREG_X(6)
#define WEND_Y()       BREG_Y(6)
#define DYDX_X()       BREG_X(7)
#define DYDX_Y()       BREG_Y(7)
#define COLOR0()       BREG(8)
#define COLOR1()       BREG(9)
#define COUNT()        BREG(10)
#define INC1_X()       BREG_X(11)
#define INC1_Y()       BREG_Y(11)
#define INC2_X()       BREG_X(12)
#define INC2_Y()       BREG_Y(12)
#define PATTRN()       BREG(13)
#define TEMP()         BREG(14)

/* I/O registers */
#define WINDOW_CHECKING()  ((IOREG(REG_CONTROL) >> 6) & 0x03)



/***************************************************************************
    INLINE SHORTCUTS
***************************************************************************/

/* Break up Status Register into indiviual flags */
inline void tms340x0_device::SET_ST(UINT32 st)
{
	m_st = st;
	/* interrupts might have been enabled, check it */
	check_interrupt();
}

/* Intialize Status to 0x0010 */
inline void tms340x0_device::RESET_ST()
{
	SET_ST(0x00000010);
}

/* shortcuts for reading opcodes */
inline UINT32 tms340x0_device::ROPCODE()
{
	UINT32 pc = TOBYTE(m_pc);
	m_pc += 2 << 3;
	return m_direct->read_word(pc);
}

inline INT16 tms340x0_device::PARAM_WORD()
{
	UINT32 pc = TOBYTE(m_pc);
	m_pc += 2 << 3;
	return m_direct->read_word(pc);
}

inline INT32 tms340x0_device::PARAM_LONG()
{
	UINT32 pc = TOBYTE(m_pc);
	m_pc += 4 << 3;
	return (UINT16)m_direct->read_word(pc) | (m_direct->read_word(pc + 2) << 16);
}

inline INT16 tms340x0_device::PARAM_WORD_NO_INC()
{
	return m_direct->read_word(TOBYTE(m_pc));
}

inline INT32 tms340x0_device::PARAM_LONG_NO_INC()
{
	UINT32 pc = TOBYTE(m_pc);
	return (UINT16)m_direct->read_word(pc) | (m_direct->read_word(pc + 2) << 16);
}

/* read memory byte */
inline UINT32 tms340x0_device::RBYTE(offs_t offset)
{
	UINT32 ret;
	RFIELDMAC_8();
	return ret;
}

/* write memory byte */
inline void tms340x0_device::WBYTE(offs_t offset, UINT32 data)
{
	WFIELDMAC_8();
}

/* read memory long */
inline UINT32 tms340x0_device::RLONG(offs_t offset)
{
	RFIELDMAC_32();
}

/* write memory long */
inline void tms340x0_device::WLONG(offs_t offset, UINT32 data)
{
	WFIELDMAC_32();
}

/* pushes/pops a value from the stack */
inline void tms340x0_device::PUSH(UINT32 data)
{
	SP() -= 0x20;
	WLONG(SP(), data);
}

inline INT32 tms340x0_device::POP()
{
	INT32 ret = RLONG(SP());
	SP() += 0x20;
	return ret;
}



/***************************************************************************
    PIXEL READS
***************************************************************************/

#define RP(m1,m2)                                           \
	/* TODO: Plane masking */                               \
	return (TMS34010_RDMEM_WORD(TOBYTE(offset & 0xfffffff0)) >> (offset & m1)) & m2;

UINT32 tms340x0_device::read_pixel_1(offs_t offset) { RP(0x0f,0x01) }
UINT32 tms340x0_device::read_pixel_2(offs_t offset) { RP(0x0e,0x03) }
UINT32 tms340x0_device::read_pixel_4(offs_t offset) { RP(0x0c,0x0f) }
UINT32 tms340x0_device::read_pixel_8(offs_t offset) { RP(0x08,0xff) }
UINT32 tms340x0_device::read_pixel_16(offs_t offset)
{
	/* TODO: Plane masking */
	return TMS34010_RDMEM_WORD(TOBYTE(offset & 0xfffffff0));
}
UINT32 tms340x0_device::read_pixel_32(offs_t offset)
{
	/* TODO: Plane masking */
	return TMS34010_RDMEM_DWORD(TOBYTE(offset & 0xffffffe0));
}

/* Shift register read */
UINT32 tms340x0_device::read_pixel_shiftreg(offs_t offset)
{
	if (!m_to_shiftreg_cb.isnull())
		m_to_shiftreg_cb(*m_program, offset, &m_shiftreg[0]);
	else
		fatalerror("To ShiftReg function not set. PC = %08X\n", m_pc);
	return m_shiftreg[0];
}



/***************************************************************************
    PIXEL WRITES
***************************************************************************/

/* No Raster Op + No Transparency */
#define WP(m1,m2)                                                                           \
	UINT32 a = TOBYTE(offset & 0xfffffff0);                                                 \
	UINT32 pix = TMS34010_RDMEM_WORD(a);                                                    \
	UINT32 shiftcount = offset & m1;                                                        \
																							\
	/* TODO: plane masking */                                                               \
	data &= m2;                                                                             \
	pix = (pix & ~(m2 << shiftcount)) | (data << shiftcount);                               \
	TMS34010_WRMEM_WORD(a, pix);

/* No Raster Op + Transparency */
#define WP_T(m1,m2)                                                                         \
	/* TODO: plane masking */                                                               \
	data &= m2;                                                                             \
	if (data)                                                                               \
	{                                                                                       \
		UINT32 a = TOBYTE(offset & 0xfffffff0);                                             \
		UINT32 pix = TMS34010_RDMEM_WORD(a);                                                \
		UINT32 shiftcount = offset & m1;                                                    \
																							\
		/* TODO: plane masking */                                                           \
		pix = (pix & ~(m2 << shiftcount)) | (data << shiftcount);                           \
		TMS34010_WRMEM_WORD(a, pix);                                                        \
	}
/* Raster Op + No Transparency */
#define WP_R(m1,m2)                                                                         \
	UINT32 a = TOBYTE(offset & 0xfffffff0);                                                 \
	UINT32 pix = TMS34010_RDMEM_WORD(a);                                                    \
	UINT32 shiftcount = offset & m1;                                                        \
																							\
	/* TODO: plane masking */                                                               \
	data = (this->*m_raster_op)(data & m2, (pix >> shiftcount) & m2) & m2;                  \
	pix = (pix & ~(m2 << shiftcount)) | (data << shiftcount);                               \
	TMS34010_WRMEM_WORD(a, pix);

/* Raster Op + Transparency */
#define WP_R_T(m1,m2)                                                                       \
	UINT32 a = TOBYTE(offset & 0xfffffff0);                                                 \
	UINT32 pix = TMS34010_RDMEM_WORD(a);                                                    \
	UINT32 shiftcount = offset & m1;                                                        \
																							\
	/* TODO: plane masking */                                                               \
	data = (this->*m_raster_op)(data & m2, (pix >> shiftcount) & m2) & m2;                  \
	if (data)                                                                               \
	{                                                                                       \
		pix = (pix & ~(m2 << shiftcount)) | (data << shiftcount);                           \
		TMS34010_WRMEM_WORD(a, pix);                                                        \
	}

/* No Raster Op + No Transparency */
void tms340x0_device::write_pixel_1(offs_t offset, UINT32 data) { WP(0x0f, 0x01); }
void tms340x0_device::write_pixel_2(offs_t offset, UINT32 data) { WP(0x0e, 0x03); }
void tms340x0_device::write_pixel_4(offs_t offset, UINT32 data) { WP(0x0c, 0x0f); }
void tms340x0_device::write_pixel_8(offs_t offset, UINT32 data) { WP(0x08, 0xff); }
void tms340x0_device::write_pixel_16(offs_t offset, UINT32 data)
{
	/* TODO: plane masking */
	TMS34010_WRMEM_WORD(TOBYTE(offset & 0xfffffff0), data);
}
void tms340x0_device::write_pixel_32(offs_t offset, UINT32 data)
{
	/* TODO: plane masking */
	TMS34010_WRMEM_WORD(TOBYTE(offset & 0xffffffe0), data);
}

/* No Raster Op + Transparency */
void tms340x0_device::write_pixel_t_1(offs_t offset, UINT32 data) { WP_T(0x0f, 0x01); }
void tms340x0_device::write_pixel_t_2(offs_t offset, UINT32 data) { WP_T(0x0e, 0x03); }
void tms340x0_device::write_pixel_t_4(offs_t offset, UINT32 data) { WP_T(0x0c, 0x0f); }
void tms340x0_device::write_pixel_t_8(offs_t offset, UINT32 data) { WP_T(0x08, 0xff); }
void tms340x0_device::write_pixel_t_16(offs_t offset, UINT32 data)
{
	/* TODO: plane masking */
	if (data)
		TMS34010_WRMEM_WORD(TOBYTE(offset & 0xfffffff0), data);
}
void tms340x0_device::write_pixel_t_32(offs_t offset, UINT32 data)
{
	/* TODO: plane masking */
	if (data)
		TMS34010_WRMEM_DWORD(TOBYTE(offset & 0xffffffe0), data);
}

/* Raster Op + No Transparency */
void tms340x0_device::write_pixel_r_1(offs_t offset, UINT32 data) { WP_R(0x0f, 0x01); }
void tms340x0_device::write_pixel_r_2(offs_t offset, UINT32 data) { WP_R(0x0e, 0x03); }
void tms340x0_device::write_pixel_r_4(offs_t offset, UINT32 data) { WP_R(0x0c, 0x0f); }
void tms340x0_device::write_pixel_r_8(offs_t offset, UINT32 data) { WP_R(0x08, 0xff); }
void tms340x0_device::write_pixel_r_16(offs_t offset, UINT32 data)
{
	/* TODO: plane masking */
	UINT32 a = TOBYTE(offset & 0xfffffff0);
	TMS34010_WRMEM_WORD(a, (this->*m_raster_op)(data, TMS34010_RDMEM_WORD(a)));
}
void tms340x0_device::write_pixel_r_32(offs_t offset, UINT32 data)
{
	/* TODO: plane masking */
	UINT32 a = TOBYTE(offset & 0xffffffe0);
	TMS34010_WRMEM_DWORD(a, (this->*m_raster_op)(data, TMS34010_RDMEM_DWORD(a)));
}

/* Raster Op + Transparency */
void tms340x0_device::write_pixel_r_t_1(offs_t offset, UINT32 data) { WP_R_T(0x0f,0x01); }
void tms340x0_device::write_pixel_r_t_2(offs_t offset, UINT32 data) { WP_R_T(0x0e,0x03); }
void tms340x0_device::write_pixel_r_t_4(offs_t offset, UINT32 data) { WP_R_T(0x0c,0x0f); }
void tms340x0_device::write_pixel_r_t_8(offs_t offset, UINT32 data) { WP_R_T(0x08,0xff); }
void tms340x0_device::write_pixel_r_t_16(offs_t offset, UINT32 data)
{
	/* TODO: plane masking */
	UINT32 a = TOBYTE(offset & 0xfffffff0);
	data = (this->*m_raster_op)(data, TMS34010_RDMEM_WORD(a));

	if (data)
		TMS34010_WRMEM_WORD(a, data);
}
void tms340x0_device::write_pixel_r_t_32(offs_t offset, UINT32 data)
{
	/* TODO: plane masking */
	UINT32 a = TOBYTE(offset & 0xffffffe0);
	data = (this->*m_raster_op)(data, TMS34010_RDMEM_DWORD(a));

	if (data)
		TMS34010_WRMEM_DWORD(a, data);
}

/* Shift register write */
void tms340x0_device::write_pixel_shiftreg(offs_t offset, UINT32 data)
{
	if (!m_from_shiftreg_cb.isnull())
		m_from_shiftreg_cb(*m_program, offset, &m_shiftreg[0]);
	else
		fatalerror("From ShiftReg function not set. PC = %08X\n", m_pc);
}



/***************************************************************************
    RASTER OPS
***************************************************************************/

/* Raster operations */
UINT32 tms340x0_device::raster_op_1(UINT32 newpix, UINT32 oldpix)  { return newpix & oldpix; }
UINT32 tms340x0_device::raster_op_2(UINT32 newpix, UINT32 oldpix)  { return newpix & ~oldpix; }
UINT32 tms340x0_device::raster_op_3(UINT32 newpix, UINT32 oldpix)  { return 0; }
UINT32 tms340x0_device::raster_op_4(UINT32 newpix, UINT32 oldpix)  { return newpix | ~oldpix; }
UINT32 tms340x0_device::raster_op_5(UINT32 newpix, UINT32 oldpix)  { return ~(newpix ^ oldpix); }
UINT32 tms340x0_device::raster_op_6(UINT32 newpix, UINT32 oldpix)  { return ~oldpix; }
UINT32 tms340x0_device::raster_op_7(UINT32 newpix, UINT32 oldpix)  { return ~(newpix | oldpix); }
UINT32 tms340x0_device::raster_op_8(UINT32 newpix, UINT32 oldpix)  { return newpix | oldpix; }
UINT32 tms340x0_device::raster_op_9(UINT32 newpix, UINT32 oldpix)  { return oldpix; }
UINT32 tms340x0_device::raster_op_10(UINT32 newpix, UINT32 oldpix) { return newpix ^ oldpix; }
UINT32 tms340x0_device::raster_op_11(UINT32 newpix, UINT32 oldpix) { return ~newpix & oldpix; }
UINT32 tms340x0_device::raster_op_12(UINT32 newpix, UINT32 oldpix) { return 0xffff; }
UINT32 tms340x0_device::raster_op_13(UINT32 newpix, UINT32 oldpix) { return ~newpix | oldpix; }
UINT32 tms340x0_device::raster_op_14(UINT32 newpix, UINT32 oldpix) { return ~(newpix & oldpix); }
UINT32 tms340x0_device::raster_op_15(UINT32 newpix, UINT32 oldpix) { return ~newpix; }
UINT32 tms340x0_device::raster_op_16(UINT32 newpix, UINT32 oldpix) { return newpix + oldpix; }
UINT32 tms340x0_device::raster_op_17(UINT32 newpix, UINT32 oldpix)
{
	UINT32 max = (UINT32)0xffffffff >> (32 - IOREG(REG_PSIZE));
	UINT32 res = newpix + oldpix;
	return (res > max) ? max : res;
}
UINT32 tms340x0_device::raster_op_18(UINT32 newpix, UINT32 oldpix) { return oldpix - newpix; }
UINT32 tms340x0_device::raster_op_19(UINT32 newpix, UINT32 oldpix) { return (oldpix > newpix) ? oldpix - newpix : 0; }
UINT32 tms340x0_device::raster_op_20(UINT32 newpix, UINT32 oldpix) { return (oldpix > newpix) ? oldpix : newpix; }
UINT32 tms340x0_device::raster_op_21(UINT32 newpix, UINT32 oldpix) { return (oldpix > newpix) ? newpix : oldpix; }



/***************************************************************************
    OPCODE TABLE & IMPLEMENTATIONS
***************************************************************************/

#include "34010fld.cpp"

/* includes the static function prototypes and the master opcode table */
#include "34010tbl.cpp"

/* includes the actual opcode implementations */
#include "34010ops.cpp"
#include "34010gfx.cpp"



/***************************************************************************
    Internal interrupt check
****************************************************************************/

/* Generate pending interrupts. */
void tms340x0_device::check_interrupt()
{
	int vector = 0;
	int irqline = -1;
	int irq;

	/* if we're not actively executing, skip it */
	if (!m_executing)
		return;

	/* check for NMI first */
	if (IOREG(REG_HSTCTLH) & 0x0100)
	{
		LOG(("TMS34010 '%s' takes NMI\n", tag()));

		/* ack the NMI */
		IOREG(REG_HSTCTLH) &= ~0x0100;

		/* handle NMI mode bit */
		if (!(IOREG(REG_HSTCTLH) & 0x0200))
		{
			PUSH(m_pc);
			PUSH(m_st);
		}

		/* leap to the vector */
		RESET_ST();
		m_pc = RLONG(0xfffffee0);
		COUNT_CYCLES(16);
		return;
	}

	/* early out if everything else is disabled */
	irq = IOREG(REG_INTPEND) & IOREG(REG_INTENB);
	if (!IE_FLAG() || !irq)
		return;

	/* host interrupt */
	if (irq & TMS34010_HI)
	{
		LOG(("TMS34010 '%s' takes HI\n", tag()));
		vector = 0xfffffec0;
	}

	/* display interrupt */
	else if (irq & TMS34010_DI)
	{
		LOG(("TMS34010 '%s' takes DI\n", tag()));
		vector = 0xfffffea0;
	}

	/* window violation interrupt */
	else if (irq & TMS34010_WV)
	{
		LOG(("TMS34010 '%s' takes WV\n", tag()));
		vector = 0xfffffe80;
	}

	/* external 1 interrupt */
	else if (irq & TMS34010_INT1)
	{
		LOG(("TMS34010 '%s' takes INT1\n", tag()));
		vector = 0xffffffc0;
		irqline = 0;
	}

	/* external 2 interrupt */
	else if (irq & TMS34010_INT2)
	{
		LOG(("TMS34010 '%s' takes INT2\n", tag()));
		vector = 0xffffffa0;
		irqline = 1;
	}

	/* if we took something, generate it */
	if (vector)
	{
		PUSH(m_pc);
		PUSH(m_st);
		RESET_ST();
		m_pc = RLONG(vector);
		COUNT_CYCLES(16);

		/* call the callback for externals */
		if (irqline >= 0)
			standard_irq_callback(irqline);
	}
}



/***************************************************************************
    Reset the CPU emulation
***************************************************************************/

void tms340x0_device::device_start()
{
	m_scanline_ind16_cb.bind_relative_to(*owner());
	m_scanline_rgb32_cb.bind_relative_to(*owner());
	m_output_int_cb.resolve();
	m_to_shiftreg_cb.bind_relative_to(*owner());
	m_from_shiftreg_cb.bind_relative_to(*owner());

	m_external_host_access = FALSE;

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	/* set up the state table */
	{
		state_add(TMS34010_PC,     "PC",        m_pc);
		state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
		state_add(STATE_GENPCBASE, "GENPCBASE", m_ppc).noshow();
		state_add(TMS34010_SP,     "SP",        m_regs[15].reg);
		state_add(STATE_GENSP,     "GENSP",     m_regs[15].reg).noshow();
		state_add(TMS34010_ST,     "ST",        m_st);
		state_add(STATE_GENFLAGS,  "GENFLAGS",  m_st).noshow().formatstr("%18s");

		std::string tempstr;
		for (int regnum = 0; regnum < 15; regnum++)
		{
			state_add(TMS34010_A0 + regnum, strformat(tempstr, "A%d", regnum).c_str(), m_regs[regnum].reg);
		}
		for (int regnum = 0; regnum < 15; regnum++)
		{
			state_add(TMS34010_B0 + regnum, strformat(tempstr, "B%d", regnum).c_str(), m_regs[30 - regnum].reg);
		}
	}

	/* allocate a scanline timer and set it to go off at the start */
	m_scantimer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(tms340x0_device::scanline_callback), this));
	m_scantimer->adjust(attotime::zero);

	save_item(NAME(m_pc));
	save_item(NAME(m_st));
	save_item(NAME(m_reset_deferred));
	save_item(NAME(m_shiftreg));
	save_item(NAME(m_IOregs));
	save_item(NAME(m_convsp));
	save_item(NAME(m_convdp));
	save_item(NAME(m_convmp));
	save_item(NAME(m_pixelshift));
	save_item(NAME(m_gfxcycles));
	save_pointer(NAME(&m_regs[0].reg), ARRAY_LENGTH(m_regs));
	machine().save().register_postload(save_prepost_delegate(FUNC(tms340x0_device::tms34010_state_postload), this));

	m_icountptr = &m_icount;
}

void tms340x0_device::device_reset()
{
	m_ppc = 0;
	m_st = 0;
	m_pixel_write = NULL;
	m_pixel_read = NULL;
	m_raster_op = NULL;
	m_pixel_op = NULL;
	m_pixel_op_timing = 0;
	m_convsp = 0;
	m_convdp = 0;
	m_convmp = 0;
	m_gfxcycles = 0;
	m_pixelshift = 0;
	m_hblank_stable = 0;
	m_external_host_access = 0;
	m_executing = 0;
	memset(m_regs, 0, sizeof(m_regs));
	memset(m_IOregs, 0, sizeof(m_IOregs));
	memset(m_shiftreg, 0, sizeof(m_shiftreg));

	/* fetch the initial PC and reset the state */
	m_pc = RLONG(0xffffffe0) & 0xfffffff0;
	RESET_ST();

	/* HALT the CPU if requested, and remember to re-read the starting PC */
	/* the first time we are run */
	m_reset_deferred = m_halt_on_reset;

	if (m_reset_deferred)
	{
		io_register_w(*m_program, REG_HSTCTLH, 0x8000, 0xffff);
	}
}


/***************************************************************************
    Set IRQ line state
***************************************************************************/

void tms340x0_device::execute_set_input(int inputnum, int state)
{
	LOG(("TMS34010 '%s' set irq line %d state %d\n", tag(), inputnum, state));

	/* set the pending interrupt */
	switch (inputnum)
	{
		case 0:
			if (state != CLEAR_LINE)
				IOREG(REG_INTPEND) |= TMS34010_INT1;
			else
				IOREG(REG_INTPEND) &= ~TMS34010_INT1;
			break;

		case 1:
			if (state != CLEAR_LINE)
				IOREG(REG_INTPEND) |= TMS34010_INT2;
			else
				IOREG(REG_INTPEND) &= ~TMS34010_INT2;
			break;
	}
}



/***************************************************************************
    Generate internal interrupt
***************************************************************************/

TIMER_CALLBACK_MEMBER( tms340x0_device::internal_interrupt_callback )
{
	int type = param;

	/* call through to the CPU to generate the int */
	IOREG(REG_INTPEND) |= type;
	LOG(("TMS34010 '%s' set internal interrupt $%04x\n", tag(), type));

	/* generate triggers so that spin loops can key off them */
	signal_interrupt_trigger();
}



/***************************************************************************
    Execute
***************************************************************************/

void tms340x0_device::execute_run()
{
	/* Get out if CPU is halted. Absolutely no interrupts must be taken!!! */
	if (IOREG(REG_HSTCTLH) & 0x8000)
	{
		m_icount = 0;
		return;
	}
	/* if the CPU's reset was deferred, do it now */
	if (m_reset_deferred)
	{
		m_reset_deferred = FALSE;
		m_pc = RLONG(0xffffffe0);
	}

	/* check interrupts first */
	m_executing = TRUE;
	check_interrupt();
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		do
		{
			UINT16 op;
			m_ppc = m_pc;
			op = ROPCODE();
			(this->*s_opcode_table[op >> 4])(op);
		} while (m_icount > 0);
	}
	else
	{
		do
		{
			UINT16 op;
			debugger_instruction_hook(this, m_pc);
			m_ppc = m_pc;
			op = ROPCODE();
			(this->*s_opcode_table[op >> 4])(op);
		} while (m_icount > 0);
	}
	m_executing = FALSE;
}



/***************************************************************************
    PIXEL OPS
***************************************************************************/

const tms340x0_device::pixel_write_func tms340x0_device::s_pixel_write_ops[4][6] =
{
	{ &tms340x0_device::write_pixel_1,     &tms340x0_device::write_pixel_2,     &tms340x0_device::write_pixel_4,     &tms340x0_device::write_pixel_8,     &tms340x0_device::write_pixel_16,     &tms340x0_device::write_pixel_32     },
	{ &tms340x0_device::write_pixel_r_1,   &tms340x0_device::write_pixel_r_2,   &tms340x0_device::write_pixel_r_4,   &tms340x0_device::write_pixel_r_8,   &tms340x0_device::write_pixel_r_16,   &tms340x0_device::write_pixel_r_32   },
	{ &tms340x0_device::write_pixel_t_1,   &tms340x0_device::write_pixel_t_2,   &tms340x0_device::write_pixel_t_4,   &tms340x0_device::write_pixel_t_8,   &tms340x0_device::write_pixel_t_16,   &tms340x0_device::write_pixel_t_32   },
	{ &tms340x0_device::write_pixel_r_t_1, &tms340x0_device::write_pixel_r_t_2, &tms340x0_device::write_pixel_r_t_4, &tms340x0_device::write_pixel_r_t_8, &tms340x0_device::write_pixel_r_t_16, &tms340x0_device::write_pixel_r_t_32 }
};

const tms340x0_device::pixel_read_func tms340x0_device::s_pixel_read_ops[6] =
{
	&tms340x0_device::read_pixel_1,        &tms340x0_device::read_pixel_2,      &tms340x0_device::read_pixel_4,      &tms340x0_device::read_pixel_8,      &tms340x0_device::read_pixel_16,      &tms340x0_device::read_pixel_32
};


void tms340x0_device::set_pixel_function()
{
	UINT32 i1,i2;

	if (IOREG(REG_DPYCTL) & 0x0800)
	{
		/* Shift Register Transfer */
		m_pixel_write = &tms340x0_device::write_pixel_shiftreg;
		m_pixel_read  = &tms340x0_device::read_pixel_shiftreg;
		return;
	}

	switch (IOREG(REG_PSIZE))
	{
		default:
		case 0x01: i2 = 0; break;
		case 0x02: i2 = 1; break;
		case 0x04: i2 = 2; break;
		case 0x08: i2 = 3; break;
		case 0x10: i2 = 4; break;
		case 0x20: i2 = 5; break;
	}

	if (IOREG(REG_CONTROL) & 0x20)
		i1 = m_raster_op ? 3 : 2;
	else
		i1 = m_raster_op ? 1 : 0;

	m_pixel_write = s_pixel_write_ops[i1][i2];
	m_pixel_read  = s_pixel_read_ops [i2];
}



/***************************************************************************
    RASTER OPS
***************************************************************************/

const tms340x0_device::raster_op_func tms340x0_device::s_raster_ops[32] =
{
				NULL, &tms340x0_device::raster_op_1 , &tms340x0_device::raster_op_2 , &tms340x0_device::raster_op_3,
	&tms340x0_device::raster_op_4 , &tms340x0_device::raster_op_5 , &tms340x0_device::raster_op_6 , &tms340x0_device::raster_op_7,
	&tms340x0_device::raster_op_8 , &tms340x0_device::raster_op_9 , &tms340x0_device::raster_op_10, &tms340x0_device::raster_op_11,
	&tms340x0_device::raster_op_12, &tms340x0_device::raster_op_13, &tms340x0_device::raster_op_14, &tms340x0_device::raster_op_15,
	&tms340x0_device::raster_op_16, &tms340x0_device::raster_op_17, &tms340x0_device::raster_op_18, &tms340x0_device::raster_op_19,
	&tms340x0_device::raster_op_20, &tms340x0_device::raster_op_21,            NULL,            NULL,
				NULL,            NULL,            NULL,            NULL,
				NULL,            NULL,            NULL,            NULL,
};


void tms340x0_device::set_raster_op()
{
	m_raster_op = s_raster_ops[(IOREG(REG_CONTROL) >> 10) & 0x1f];
}



/***************************************************************************
    VIDEO TIMING HELPERS
***************************************************************************/

TIMER_CALLBACK_MEMBER( tms340x0_device::scanline_callback )
{
	int vsblnk, veblnk, vtotal;
	int vcount = param;
	int enabled;
	int master;

	/* fetch the core timing parameters */
	const rectangle &current_visarea = m_screen->visible_area();
	enabled = SMART_IOREG(DPYCTL) & 0x8000;
	master = (m_is_34020 || (SMART_IOREG(DPYCTL) & 0x2000));
	vsblnk = SMART_IOREG(VSBLNK);
	veblnk = SMART_IOREG(VEBLNK);
	vtotal = SMART_IOREG(VTOTAL);
	if (!master)
	{
		vtotal = MIN(m_screen->height() - 1, vtotal);
		vcount = m_screen->vpos();
	}

	/* update the VCOUNT */
	SMART_IOREG(VCOUNT) = vcount;

	/* if we match the display interrupt scanline, signal an interrupt */
	if (enabled && vcount == SMART_IOREG(DPYINT))
	{
		/* generate the display interrupt signal */
		internal_interrupt_callback(NULL, TMS34010_DI);
	}

	/* at the start of VBLANK, load the starting display address */
	if (vcount == vsblnk)
	{
		/* 34010 loads DPYADR with DPYSTRT, and inverts if the origin is 0 */
		if (!m_is_34020)
		{
			IOREG(REG_DPYADR) = IOREG(REG_DPYSTRT);
			LOG(("Start of VBLANK, DPYADR = %04X\n", IOREG(REG_DPYADR)));
		}

		/* 34020 loads DPYNXx with DPYSTx */
		else
		{
			IOREG(REG020_DPYNXL) = IOREG(REG020_DPYSTL) & 0xffe0;
			IOREG(REG020_DPYNXH) = IOREG(REG020_DPYSTH);
		}
	}

	/* at the end of the screen, update the display parameters */
	if (vcount == vtotal)
	{
		/* only do this if we have an incoming pixel clock */
		/* also, only do it if the HEBLNK/HSBLNK values are stable */
		if (master && (!m_scanline_ind16_cb.isnull() || !m_scanline_rgb32_cb.isnull()))
		{
			int htotal = SMART_IOREG(HTOTAL);
			if (htotal > 0 && vtotal > 0)
			{
				attoseconds_t refresh = HZ_TO_ATTOSECONDS(m_pixclock) * (htotal + 1) * (vtotal + 1);
				int width = (htotal + 1) * m_pixperclock;
				int height = vtotal + 1;
				rectangle visarea;

				/* extract the visible area */
				visarea.min_x = SMART_IOREG(HEBLNK) * m_pixperclock;
				visarea.max_x = SMART_IOREG(HSBLNK) * m_pixperclock - 1;
				visarea.min_y = veblnk;
				visarea.max_y = vsblnk - 1;

				/* if everything looks good, set the info */
				if (visarea.min_x < visarea.max_x && visarea.max_x <= width && visarea.min_y < visarea.max_y && visarea.max_y <= height)
				{
					/* because many games play with the HEBLNK/HSBLNK for effects, we don't change
					   if they are the only thing that has changed, unless they are stable for a couple
					   of frames */
					int current_width  = m_screen->width();
					int current_height = m_screen->height();

					if (width != current_width || height != current_height || visarea.min_y != current_visarea.min_y || visarea.max_y != current_visarea.max_y ||
						(m_hblank_stable > 2 && (visarea.min_x != current_visarea.min_x || visarea.max_x != current_visarea.max_x)))
					{
						m_screen->configure(width, height, visarea, refresh);
					}
					m_hblank_stable++;
				}

				LOG(("Configuring screen: HTOTAL=%3d BLANK=%3d-%3d VTOTAL=%3d BLANK=%3d-%3d refresh=%f\n",
						htotal, SMART_IOREG(HEBLNK), SMART_IOREG(HSBLNK), vtotal, veblnk, vsblnk, ATTOSECONDS_TO_HZ(refresh)));

				/* interlaced timing not supported */
				if ((SMART_IOREG(DPYCTL) & 0x4000) == 0)
					fatalerror("Interlaced video configured on the TMS34010 (unsupported)\n");
			}
		}
	}

	/* force a partial update within the visible area */
	if (vcount >= current_visarea.min_y && vcount <= current_visarea.max_y && (!m_scanline_ind16_cb.isnull() || !m_scanline_rgb32_cb.isnull()))
		m_screen->update_partial(vcount);

	/* if we are in the visible area, increment DPYADR by DUDATE */
	if (vcount >= veblnk && vcount < vsblnk)
	{
		/* 34010 increments by the DUDATE field in DPYCTL */
		if (!m_is_34020)
		{
			UINT16 dpyadr = IOREG(REG_DPYADR);
			if ((dpyadr & 3) == 0)
				dpyadr = ((dpyadr & 0xfffc) - (IOREG(REG_DPYCTL) & 0x03fc)) | (IOREG(REG_DPYSTRT) & 0x0003);
			else
				dpyadr = (dpyadr & 0xfffc) | ((dpyadr - 1) & 3);
			IOREG(REG_DPYADR) = dpyadr;
		}

		/* 34020 updates based on the DINC register, including zoom */
		else
		{
			UINT32 dpynx = IOREG(REG020_DPYNXL) | (IOREG(REG020_DPYNXH) << 16);
			UINT32 dinc = IOREG(REG020_DINCL) | (IOREG(REG020_DINCH) << 16);
			dpynx = (dpynx & 0xffffffe0) | ((dpynx + dinc) & 0x1f);
			if ((dpynx & 0x1f) == 0)
				dpynx += dinc & 0xffffffe0;
			IOREG(REG020_DPYNXL) = dpynx;
			IOREG(REG020_DPYNXH) = dpynx >> 16;
		}
	}

	/* adjust for the next callback */
	vcount++;
	if (vcount > vtotal)
		vcount = 0;

	/* note that we add !master (0 or 1) as a attoseconds value; this makes no practical difference */
	/* but helps ensure that masters are updated first before slaves */
	m_scantimer->adjust(m_screen->time_until_pos(vcount) + attotime(0, !master), vcount);
}


void tms340x0_device::get_display_params(tms34010_display_params *params)
{
	params->enabled = ((SMART_IOREG(DPYCTL) & 0x8000) != 0);
	params->vcount = SMART_IOREG(VCOUNT);
	params->veblnk = SMART_IOREG(VEBLNK);
	params->vsblnk = SMART_IOREG(VSBLNK);
	params->heblnk = SMART_IOREG(HEBLNK) * m_pixperclock;
	params->hsblnk = SMART_IOREG(HSBLNK) * m_pixperclock;

	/* 34010 gets its address from DPYADR and DPYTAP */
	if (!m_is_34020)
	{
		UINT16 dpyadr = IOREG(REG_DPYADR);
		if (!(IOREG(REG_DPYCTL) & 0x0400))
			dpyadr ^= 0xfffc;
		params->rowaddr = dpyadr >> 4;
		params->coladdr = ((dpyadr & 0x007c) << 4) | (IOREG(REG_DPYTAP) & 0x3fff);
		params->yoffset = (IOREG(REG_DPYSTRT) - IOREG(REG_DPYADR)) & 3;
	}

	/* 34020 gets its address from DPYNX */
	else
	{
		params->rowaddr = IOREG(REG020_DPYNXH);
		params->coladdr = IOREG(REG020_DPYNXL) & 0xffe0;
		params->yoffset = 0;
		if ((IOREG(REG020_DINCL) & 0x1f) != 0)
			params->yoffset = (IOREG(REG020_DPYNXL) & 0x1f) / (IOREG(REG020_DINCL) & 0x1f);
	}
}

UINT32 tms340x0_device::tms340x0_ind16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	pen_t blackpen = screen.palette()->black_pen();
	tms34010_display_params params;
	int x;

	/* get the display parameters for the screen */
	get_display_params(&params);

	/* if the display is enabled, call the scanline callback */
	if (params.enabled)
	{
		/* call through to the callback */
		LOG(("  Update: scan=%3d ROW=%04X COL=%04X\n", cliprect.min_y, params.rowaddr, params.coladdr));
		m_scanline_ind16_cb(screen, bitmap, cliprect.min_y, &params);
	}

	/* otherwise, just blank the current scanline */
	else
		params.heblnk = params.hsblnk = cliprect.max_x + 1;

	/* blank out the blank regions */
	UINT16 *dest = &bitmap.pix16(cliprect.min_y);
	for (x = cliprect.min_x; x < params.heblnk; x++)
		dest[x] = blackpen;
	for (x = params.hsblnk; x <= cliprect.max_x; x++)
		dest[x] = blackpen;
	return 0;

}

UINT32 tms340x0_device::tms340x0_rgb32(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t blackpen = rgb_t::black;
	tms34010_display_params params;
	int x;

	/* get the display parameters for the screen */
	get_display_params(&params);

	/* if the display is enabled, call the scanline callback */
	if (params.enabled)
	{
		/* call through to the callback */
		LOG(("  Update: scan=%3d ROW=%04X COL=%04X\n", cliprect.min_y, params.rowaddr, params.coladdr));
		m_scanline_rgb32_cb(screen, bitmap, cliprect.min_y, &params);
	}

	/* otherwise, just blank the current scanline */
	else
		params.heblnk = params.hsblnk = cliprect.max_x + 1;

	/* blank out the blank regions */
	UINT32 *dest = &bitmap.pix32(cliprect.min_y);
	for (x = cliprect.min_x; x < params.heblnk; x++)
		dest[x] = blackpen;
	for (x = params.hsblnk; x <= cliprect.max_x; x++)
		dest[x] = blackpen;
	return 0;
}


/***************************************************************************
    I/O REGISTER WRITES
***************************************************************************/

#if 0
static const char *const ioreg_name[] =
{
	"HESYNC", "HEBLNK", "HSBLNK", "HTOTAL",
	"VESYNC", "VEBLNK", "VSBLNK", "VTOTAL",
	"DPYCTL", "DPYSTART", "DPYINT", "CONTROL",
	"HSTDATA", "HSTADRL", "HSTADRH", "HSTCTLL",

	"HSTCTLH", "INTENB", "INTPEND", "CONVSP",
	"CONVDP", "PSIZE", "PMASK", "RESERVED",
	"RESERVED", "RESERVED", "RESERVED", "DPYTAP",
	"HCOUNT", "VCOUNT", "DPYADR", "REFCNT"
};
#endif

WRITE16_MEMBER( tms34010_device::io_register_w )
{
	int oldreg, newreg;

	/* Set register */
	oldreg = IOREG(offset);
	IOREG(offset) = data;

	switch (offset)
	{
		case REG_CONTROL:
			set_raster_op();
			set_pixel_function();
			break;

		case REG_PSIZE:
			set_pixel_function();

			switch (data)
			{
				default:
				case 0x01: m_pixelshift = 0; break;
				case 0x02: m_pixelshift = 1; break;
				case 0x04: m_pixelshift = 2; break;
				case 0x08: m_pixelshift = 3; break;
				case 0x10: m_pixelshift = 4; break;
			}
			break;

		case REG_PMASK:
			if (data) logerror("Plane masking not supported. PC=%08X\n", space.device().safe_pc());
			break;

		case REG_DPYCTL:
			set_pixel_function();
			break;

		case REG_HSTCTLH:
			/* if the CPU is halting itself, stop execution right away */
			if (mem_mask & 0xff00)
			{
				if ((data & 0x8000) && !m_external_host_access)
					m_icount = 0;

				set_input_line(INPUT_LINE_HALT, (data & 0x8000) ? ASSERT_LINE : CLEAR_LINE);

				/* NMI issued? */
				if (data & 0x0100)
					machine().scheduler().synchronize(timer_expired_delegate(FUNC(tms340x0_device::internal_interrupt_callback), this), 0);
			}
			break;

		case REG_HSTCTLL:
			if (mem_mask & 0x00ff)
			{
				/* the TMS34010 can change MSGOUT, can set INTOUT, and can clear INTIN */
				if (!m_external_host_access)
				{
					newreg = (oldreg & 0xff8f) | (data & 0x0070);
					newreg |= data & 0x0080;
					newreg &= data | ~0x0008;
				}

				/* the host can change MSGIN, can set INTIN, and can clear INTOUT */
				else
				{
					newreg = (oldreg & 0xfff8) | (data & 0x0007);
					newreg &= data | ~0x0080;
					newreg |= data & 0x0008;
				}
				IOREG(offset) = newreg;

				/* the TMS34010 can set output interrupt? */
				if (!(oldreg & 0x0080) && (newreg & 0x0080))
				{
					if (!m_output_int_cb.isnull())
						m_output_int_cb(1);
				}
				else if ((oldreg & 0x0080) && !(newreg & 0x0080))
				{
					if (!m_output_int_cb.isnull())
						m_output_int_cb(0);
				}

				/* input interrupt? (should really be state-based, but the functions don't exist!) */
				if (!(oldreg & 0x0008) && (newreg & 0x0008))
					machine().scheduler().synchronize(timer_expired_delegate(FUNC(tms340x0_device::internal_interrupt_callback), this), TMS34010_HI);
				else if ((oldreg & 0x0008) && !(newreg & 0x0008))
					IOREG(REG_INTPEND) &= ~TMS34010_HI;
			}
			break;

		case REG_CONVSP:
			m_convsp = 1 << (~data & 0x1f);
			break;

		case REG_CONVDP:
			m_convdp = 1 << (~data & 0x1f);
			break;

		case REG_INTENB:
			check_interrupt();
			break;

		case REG_INTPEND:
			/* X1P, X2P and HIP are read-only */
			/* WVP and DIP can only have 0's written to them */
			IOREG(REG_INTPEND) = oldreg;
			if (!(data & TMS34010_WV))
				IOREG(REG_INTPEND) &= ~TMS34010_WV;
			if (!(data & TMS34010_DI))
				IOREG(REG_INTPEND) &= ~TMS34010_DI;
			break;

		case REG_HEBLNK:
		case REG_HSBLNK:
			if (oldreg != data)
				m_hblank_stable = 0;
			break;
	}

//  if (LOG_CONTROL_REGS)
//      logerror("%s: %s = %04X (%d)\n", machine().describe_context(), ioreg_name[offset], IOREG(offset), m_screen.vpos());
}


#if 0
static const char *const ioreg020_name[] =
{
	"VESYNC", "HESYNC", "VEBLNK", "HEBLNK",
	"VSBLNK", "HSBLNK", "VTOTAL", "HTOTAL",
	"DPYCTL", "DPYSTRT", "DPYINT", "CONTROL",
	"HSTDATA", "HSTADRL", "HSTADRH", "HSTCTLL",

	"HSTCTLH", "INTENB", "INTPEND", "CONVSP",
	"CONVDP", "PSIZE", "PMASKL", "PMASKH",
	"CONVMP", "CONTROL2", "CONFIG", "DPYTAP",
	"VCOUNT", "HCOUNT", "DPYADR", "REFADR",

	"DPYSTL", "DPYSTH", "DPYNXL", "DPYNXH",
	"DINCL", "DINCH", "RES0", "HESERR",
	"RES1", "RES2", "RES3", "RES4",
	"SCOUNT", "BSFLTST", "DPYMSK", "RES5",

	"SETVCNT", "SETHCNT", "BSFLTDL", "BSFLTDH",
	"RES6", "RES7", "RES8", "RES9",
	"IHOST1L", "IHOST1H", "IHOST2L", "IHOST2H",
	"IHOST3L", "IHOST3H", "IHOST4L", "IHOST4H"
};
#endif

WRITE16_MEMBER( tms34020_device::io_register_w )
{
	int oldreg, newreg;

	/* Set register */
	oldreg = IOREG(offset);
	IOREG(offset) = data;

//  if (LOG_CONTROL_REGS)
//      logerror("%s: %s = %04X (%d)\n", machine().describe_context(), ioreg020_name[offset], IOREG(offset), m_screen.vpos());

	switch (offset)
	{
		case REG020_CONTROL:
		case REG020_CONTROL2:
			IOREG(REG020_CONTROL) = data;
			IOREG(REG020_CONTROL2) = data;
			set_raster_op();
			set_pixel_function();
			break;

		case REG020_PSIZE:
			set_pixel_function();

			switch (data)
			{
				default:
				case 0x01: m_pixelshift = 0; break;
				case 0x02: m_pixelshift = 1; break;
				case 0x04: m_pixelshift = 2; break;
				case 0x08: m_pixelshift = 3; break;
				case 0x10: m_pixelshift = 4; break;
				case 0x20: m_pixelshift = 5; break;
			}
			break;

		case REG020_PMASKL:
		case REG020_PMASKH:
			if (data) logerror("Plane masking not supported. PC=%08X\n", space.device().safe_pc());
			break;

		case REG020_DPYCTL:
			set_pixel_function();
			break;

		case REG020_HSTCTLH:
			/* if the CPU is halting itself, stop execution right away */
			if ((data & 0x8000) && !m_external_host_access)
				m_icount = 0;
			set_input_line(INPUT_LINE_HALT, (data & 0x8000) ? ASSERT_LINE : CLEAR_LINE);

			/* NMI issued? */
			if (data & 0x0100)
				machine().scheduler().synchronize(timer_expired_delegate(FUNC(tms340x0_device::internal_interrupt_callback), this), 0);
			break;

		case REG020_HSTCTLL:
			/* the TMS34010 can change MSGOUT, can set INTOUT, and can clear INTIN */
			if (!m_external_host_access)
			{
				newreg = (oldreg & 0xff8f) | (data & 0x0070);
				newreg |= data & 0x0080;
				newreg &= data | ~0x0008;
			}

			/* the host can change MSGIN, can set INTIN, and can clear INTOUT */
			else
			{
				newreg = (oldreg & 0xfff8) | (data & 0x0007);
				newreg &= data | ~0x0080;
				newreg |= data & 0x0008;
			}
			IOREG(offset) = newreg;

			/* the TMS34010 can set output interrupt? */
			if (!(oldreg & 0x0080) && (newreg & 0x0080))
			{
				if (!m_output_int_cb.isnull())
					m_output_int_cb(1);
			}
			else if ((oldreg & 0x0080) && !(newreg & 0x0080))
			{
				if (!m_output_int_cb.isnull())
					m_output_int_cb(0);
			}

			/* input interrupt? (should really be state-based, but the functions don't exist!) */
			if (!(oldreg & 0x0008) && (newreg & 0x0008))
				machine().scheduler().synchronize(timer_expired_delegate(FUNC(tms340x0_device::internal_interrupt_callback), this), TMS34010_HI);
			else if ((oldreg & 0x0008) && !(newreg & 0x0008))
				IOREG(REG020_INTPEND) &= ~TMS34010_HI;
			break;

		case REG020_INTENB:
			check_interrupt();
			break;

		case REG020_INTPEND:
			/* X1P, X2P and HIP are read-only */
			/* WVP and DIP can only have 0's written to them */
			IOREG(REG020_INTPEND) = oldreg;
			if (!(data & TMS34010_WV))
				IOREG(REG020_INTPEND) &= ~TMS34010_WV;
			if (!(data & TMS34010_DI))
				IOREG(REG020_INTPEND) &= ~TMS34010_DI;
			break;

		case REG020_CONVSP:
			if (data & 0x001f)
			{
				if (data & 0x1f00)
					m_convsp = (1 << (~data & 0x1f)) + (1 << (~(data >> 8) & 0x1f));
				else
					m_convsp = 1 << (~data & 0x1f);
			}
			else
				m_convsp = data;
			break;

		case REG020_CONVDP:
			if (data & 0x001f)
			{
				if (data & 0x1f00)
					m_convdp = (1 << (~data & 0x1f)) + (1 << (~(data >> 8) & 0x1f));
				else
					m_convdp = 1 << (~data & 0x1f);
			}
			else
				m_convdp = data;
			break;

		case REG020_CONVMP:
			if (data & 0x001f)
			{
				if (data & 0x1f00)
					m_convmp = (1 << (~data & 0x1f)) + (1 << (~(data >> 8) & 0x1f));
				else
					m_convmp = 1 << (~data & 0x1f);
			}
			else
				m_convmp = data;
			break;

		case REG020_DPYSTRT:
		case REG020_DPYADR:
		case REG020_DPYTAP:
			break;

		case REG020_HEBLNK:
		case REG020_HSBLNK:
			if (oldreg != data)
				m_hblank_stable = 0;
			break;
	}
}



/***************************************************************************
    I/O REGISTER READS
***************************************************************************/

READ16_MEMBER( tms34010_device::io_register_r )
{
	int result, total;

//  if (LOG_CONTROL_REGS)
//      logerror("%s: read %s\n", machine().describe_context(), ioreg_name[offset]);

	switch (offset)
	{
		case REG_HCOUNT:
			/* scale the horizontal position from screen width to HTOTAL */
			result = m_screen->hpos();
			total = IOREG(REG_HTOTAL) + 1;
			result = result * total / m_screen->width();

			/* offset by the HBLANK end */
			result += IOREG(REG_HEBLNK);

			/* wrap around */
			if (result > total)
				result -= total;
			return result;

		case REG_REFCNT:
			return (total_cycles() / 16) & 0xfffc;

		case REG_INTPEND:
			result = IOREG(offset);

			/* Cool Pool loops in mainline code on the appearance of the DI, even though they */
			/* have an IRQ handler. For this reason, we return it signalled a bit early in order */
			/* to make it past these loops. */
			if (SMART_IOREG(VCOUNT) + 1 == SMART_IOREG(DPYINT) &&
				m_scantimer->remaining() < attotime::from_hz(40000000/8/3))
				result |= TMS34010_DI;
			return result;
	}

	return IOREG(offset);
}


READ16_MEMBER( tms34020_device::io_register_r )
{
	int result, total;

//  if (LOG_CONTROL_REGS)
//      logerror("%s: read %s\n", machine().describe_context(), ioreg_name[offset]);

	switch (offset)
	{
		case REG020_HCOUNT:
			/* scale the horizontal position from screen width to HTOTAL */
			result = m_screen->hpos();
			total = IOREG(REG020_HTOTAL) + 1;
			result = result * total / m_screen->width();

			/* offset by the HBLANK end */
			result += IOREG(REG020_HEBLNK);

			/* wrap around */
			if (result > total)
				result -= total;
			return result;

		case REG020_REFADR:
		{
			int refreshrate = (IOREG(REG020_CONFIG) >> 8) & 7;
			if (refreshrate < 6)
				return (total_cycles() / refreshrate) & 0xffff;
			break;
		}
	}

	return IOREG(offset);
}



/***************************************************************************
    SAVE STATE
***************************************************************************/

void tms340x0_device::tms34010_state_postload()
{
	set_raster_op();
	set_pixel_function();
}


/***************************************************************************
    HOST INTERFACE WRITES
***************************************************************************/

WRITE16_MEMBER( tms340x0_device::host_w )
{
	int reg = offset;
	unsigned int addr;

	switch (reg)
	{
		/* upper 16 bits of the address */
		case TMS34010_HOST_ADDRESS_H:
			IOREG(REG_HSTADRH) = data;
			break;

		/* lower 16 bits of the address */
		case TMS34010_HOST_ADDRESS_L:
			IOREG(REG_HSTADRL) = data;
			break;

		/* actual data */
		case TMS34010_HOST_DATA:

			/* write to the address */
			addr = (IOREG(REG_HSTADRH) << 16) | IOREG(REG_HSTADRL);
			TMS34010_WRMEM_WORD(TOBYTE(addr & 0xfffffff0), data);

			/* optional postincrement */
			if (IOREG(REG_HSTCTLH) & 0x0800)
			{
				addr += 0x10;
				IOREG(REG_HSTADRH) = addr >> 16;
				IOREG(REG_HSTADRL) = (UINT16)addr;
			}
			break;

		/* control register */
		case TMS34010_HOST_CONTROL:
		{
			m_external_host_access = TRUE;
			if (mem_mask&0xff00) io_register_w(*m_program, REG_HSTCTLH, data & 0xff00, 0xff00);
			if (mem_mask&0x00ff) io_register_w(*m_program, REG_HSTCTLL, data & 0x00ff, 0x00ff);
			m_external_host_access = FALSE;
			break;
		}

		/* error case */
		default:
			logerror("tms34010_host_control_w called on invalid register %d\n", reg);
			break;
	}
}



/***************************************************************************
    HOST INTERFACE READS
***************************************************************************/

READ16_MEMBER( tms340x0_device::host_r )
{
	int reg = offset;
	unsigned int addr;
	int result = 0;

	/* swap to the target cpu */

	switch (reg)
	{
		/* upper 16 bits of the address */
		case TMS34010_HOST_ADDRESS_H:
			result = IOREG(REG_HSTADRH);
			break;

		/* lower 16 bits of the address */
		case TMS34010_HOST_ADDRESS_L:
			result = IOREG(REG_HSTADRL);
			break;

		/* actual data */
		case TMS34010_HOST_DATA:

			/* read from the address */
			addr = (IOREG(REG_HSTADRH) << 16) | IOREG(REG_HSTADRL);
			result = TMS34010_RDMEM_WORD(TOBYTE(addr & 0xfffffff0));

			/* optional postincrement (it says preincrement, but data is preloaded, so it
			   is effectively a postincrement */
			if (IOREG(REG_HSTCTLH) & 0x1000)
			{
				addr += 0x10;
				IOREG(REG_HSTADRH) = addr >> 16;
				IOREG(REG_HSTADRL) = (UINT16)addr;
			}
			break;

		/* control register */
		case TMS34010_HOST_CONTROL:
			result = (IOREG(REG_HSTCTLH) & 0xff00) | (IOREG(REG_HSTCTLL) & 0x00ff);
			break;

		/* error case */
		default:
			logerror("tms34010_host_control_r called on invalid register %d\n", reg);
			break;
	}

	return result;
}


void tms340x0_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				m_st & 0x80000000 ? 'N':'.',
				m_st & 0x40000000 ? 'C':'.',
				m_st & 0x20000000 ? 'Z':'.',
				m_st & 0x10000000 ? 'V':'.',
				m_st & 0x02000000 ? 'P':'.',
				m_st & 0x00200000 ? 'I':'.',
				m_st & 0x00000800 ? 'E':'.',
				m_st & 0x00000400 ? 'F':'.',
				m_st & 0x00000200 ? 'F':'.',
				m_st & 0x00000100 ? 'F':'.',
				m_st & 0x00000080 ? 'F':'.',
				m_st & 0x00000040 ? 'F':'.',
				m_st & 0x00000020 ? 'E':'.',
				m_st & 0x00000010 ? 'F':'.',
				m_st & 0x00000008 ? 'F':'.',
				m_st & 0x00000004 ? 'F':'.',
				m_st & 0x00000002 ? 'F':'.',
				m_st & 0x00000001 ? 'F':'.');
			break;
	}
}


offs_t tms34010_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms34010 );

	return CPU_DISASSEMBLE_NAME(tms34010)(this, buffer, pc, oprom, opram, options);
}


offs_t tms34020_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms34020 );

	return CPU_DISASSEMBLE_NAME(tms34020)(this, buffer, pc, oprom, opram, options);
}
