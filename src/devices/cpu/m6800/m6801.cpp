// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "cpu/m6800/m6801.h"

#define LOG_GENERAL (1U << 0)
#define LOG_TX      (1U << 1)
#define LOG_TXTICK  (1U << 2)
#define LOG_RX      (1U << 3)
#define LOG_RXTICK  (1U << 4)
#define LOG_PORT    (1U << 5)

//#define VERBOSE (LOG_GENERAL | LOG_TX | LOG_RX | LOG_PORT)
//#define LOG_OUTPUT_STREAM std::cerr
#include "logmacro.h"

#define LOGTX(...)      LOGMASKED(LOG_TX, __VA_ARGS__)
#define LOGTXTICK(...)  LOGMASKED(LOG_TXTICK, __VA_ARGS__)
#define LOGRX(...)      LOGMASKED(LOG_RX, __VA_ARGS__)
#define LOGRXTICK(...)  LOGMASKED(LOG_RXTICK, __VA_ARGS__)
#define LOGPORT(...)    LOGMASKED(LOG_PORT, __VA_ARGS__)


#define CT      m_counter.w.l
#define CTH     m_counter.w.h
#define CTD     m_counter.d
#define OC      m_output_compare.w.l
#define OCH     m_output_compare.w.h
#define OCD     m_output_compare.d
#define TOH     m_timer_over.w.l
#define TOD     m_timer_over.d

#define MODIFIED_tcsr { \
	m_irq2 = (m_tcsr&(m_tcsr<<3))&(TCSR_ICF|TCSR_OCF|TCSR_TOF); \
}

#define SET_TIMER_EVENT {                   \
	m_timer_next = (OCD - CTD < TOD - CTD) ? OCD : TOD;   \
}

/* when change freerunningcounter or outputcapture */
#define MODIFIED_counters {                     \
	OCH = (OC >= CT) ? CTH : CTH+1;             \
	SET_TIMER_EVENT;                            \
}

// I/O registers

enum
{
	IO_P1DDR = 0,
	IO_P2DDR,
	IO_P1DATA,
	IO_P2DATA,
	IO_P3DDR,
	IO_P4DDR,
	IO_P3DATA,
	IO_P4DATA,
	IO_TCSR,
	IO_CH,
	IO_CL,
	IO_OCRH,
	IO_OCRL,
	IO_ICRH,
	IO_ICRL,
	IO_P3CSR,
	IO_RMCR,
	IO_TRCSR,
	IO_RDR,
	IO_TDR,
	IO_RCR,
	IO_CAAH,
	IO_CAAL,
	IO_TCR1,
	IO_TCR2,
	IO_TSR,
	IO_OCR2H,
	IO_OCR2L,
	IO_OCR3H,
	IO_OCR3L,
	IO_ICR2H,
	IO_ICR2L
};

// serial I/O

#define M6801_RMCR_SS_MASK      0x03 // Speed Select
#define M6801_RMCR_SS_4096      0x03 // E / 4096
#define M6801_RMCR_SS_1024      0x02 // E / 1024
#define M6801_RMCR_SS_128       0x01 // E / 128
#define M6801_RMCR_SS_16        0x00 // E / 16
#define M6801_RMCR_CC_MASK      0x0c // Clock Control/Format Select

#define M6801_TRCSR_RDRF        0x80 // Receive Data Register Full
#define M6801_TRCSR_ORFE        0x40 // Over Run Framing Error
#define M6801_TRCSR_TDRE        0x20 // Transmit Data Register Empty
#define M6801_TRCSR_RIE         0x10 // Receive Interrupt Enable
#define M6801_TRCSR_RE          0x08 // Receive Enable
#define M6801_TRCSR_TIE         0x04 // Transmit Interrupt Enable
#define M6801_TRCSR_TE          0x02 // Transmit Enable
#define M6801_TRCSR_WU          0x01 // Wake Up

#define M6801_PORT2_IO4         0x10
#define M6801_PORT2_IO3         0x08

#define M6801_P3CSR_LE          0x08
#define M6801_P3CSR_OSS         0x10
#define M6801_P3CSR_IS3_ENABLE  0x40
#define M6801_P3CSR_IS3_FLAG    0x80

static const int M6801_RMCR_SS[] = { 16, 128, 1024, 4096 };

#define M6801_SERIAL_START      0
#define M6801_SERIAL_STOP       9

enum
{
	M6801_TX_STATE_INIT = 0,
	M6801_TX_STATE_READY
};

/* take interrupt */
#define TAKE_ICI enter_interrupt("take ICI\n",0xfff6)
#define TAKE_OCI enter_interrupt("take OCI\n",0xfff4)
#define TAKE_TOI enter_interrupt("take TOI\n",0xfff2)
#define TAKE_SCI enter_interrupt("take SCI\n",0xfff0)

/* mnemonicos for the Timer Control and Status Register bits */
#define TCSR_OLVL 0x01
#define TCSR_IEDG 0x02
#define TCSR_ETOI 0x04
#define TCSR_EOCI 0x08
#define TCSR_EICI 0x10
#define TCSR_TOF  0x20
#define TCSR_OCF  0x40
#define TCSR_ICF  0x80

/* Note: don't use 0 cycles here for invalid opcodes so that we don't */
/* hang in an infinite loop if we hit one */
#define XX 5 // invalid opcode unknown cc
const uint8_t m6801_cpu_device::cycles_6803[256] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ XX, 2,XX,XX, 3, 3, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2,
	/*1*/  2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
	/*2*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/*3*/  3, 3, 4, 4, 3, 3, 3, 3, 5, 5, 3,10, 4,10, 9,12,
	/*4*/  2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	/*5*/  2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	/*6*/  6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
	/*7*/  6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
	/*8*/  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 4, 6, 3, 3,
	/*9*/  3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4,
	/*A*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
	/*B*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
	/*C*/  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 3,XX, 3, 3,
	/*D*/  3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
	/*E*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*F*/  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
};

const uint8_t m6801_cpu_device::cycles_63701[256] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ XX, 1,XX,XX, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/*1*/  1, 1,XX,XX,XX,XX, 1, 1, 2, 2, 4, 1,XX,XX,XX,XX,
	/*2*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/*3*/  1, 1, 3, 3, 1, 1, 4, 4, 4, 5, 1,10, 5, 7, 9,12,
	/*4*/  1,XX,XX, 1, 1,XX, 1, 1, 1, 1, 1,XX, 1, 1,XX, 1,
	/*5*/  1,XX,XX, 1, 1,XX, 1, 1, 1, 1, 1,XX, 1, 1,XX, 1,
	/*6*/  6, 7, 7, 6, 6, 7, 6, 6, 6, 6, 6, 5, 6, 4, 3, 5,
	/*7*/  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 4, 3, 5,
	/*8*/  2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 5, 3, 3,
	/*9*/  3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 4, 4,
	/*A*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*B*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5,
	/*C*/  2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3,XX, 3, 3,
	/*D*/  3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
	/*E*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*F*/  4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
};
#undef XX // /invalid opcode unknown cc


const m6800_cpu_device::op_func m6801_cpu_device::m6803_insn[0x100] = {
&m6801_cpu_device::illegl1,&m6801_cpu_device::nop,    &m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,&m6801_cpu_device::lsrd,   &m6801_cpu_device::asld,   &m6801_cpu_device::tap,    &m6801_cpu_device::tpa,
&m6801_cpu_device::inx,    &m6801_cpu_device::dex,    &m6801_cpu_device::clv,    &m6801_cpu_device::sev,    &m6801_cpu_device::clc,    &m6801_cpu_device::sec,    &m6801_cpu_device::cli,    &m6801_cpu_device::sei,
&m6801_cpu_device::sba,    &m6801_cpu_device::cba,    &m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,&m6801_cpu_device::tab,    &m6801_cpu_device::tba,
&m6801_cpu_device::illegl1,&m6801_cpu_device::daa,    &m6801_cpu_device::illegl1,&m6801_cpu_device::aba,    &m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,
&m6801_cpu_device::bra,    &m6801_cpu_device::brn,    &m6801_cpu_device::bhi,    &m6801_cpu_device::bls,    &m6801_cpu_device::bcc,    &m6801_cpu_device::bcs,    &m6801_cpu_device::bne,    &m6801_cpu_device::beq,
&m6801_cpu_device::bvc,    &m6801_cpu_device::bvs,    &m6801_cpu_device::bpl,    &m6801_cpu_device::bmi,    &m6801_cpu_device::bge,    &m6801_cpu_device::blt,    &m6801_cpu_device::bgt,    &m6801_cpu_device::ble,
&m6801_cpu_device::tsx,    &m6801_cpu_device::ins,    &m6801_cpu_device::pula,   &m6801_cpu_device::pulb,   &m6801_cpu_device::des,    &m6801_cpu_device::txs,    &m6801_cpu_device::psha,   &m6801_cpu_device::pshb,
&m6801_cpu_device::pulx,   &m6801_cpu_device::rts,    &m6801_cpu_device::abx,    &m6801_cpu_device::rti,    &m6801_cpu_device::pshx,   &m6801_cpu_device::mul,    &m6801_cpu_device::wai,    &m6801_cpu_device::swi,
&m6801_cpu_device::nega,   &m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,&m6801_cpu_device::coma,   &m6801_cpu_device::lsra,   &m6801_cpu_device::illegl1,&m6801_cpu_device::rora,   &m6801_cpu_device::asra,
&m6801_cpu_device::asla,   &m6801_cpu_device::rola,   &m6801_cpu_device::deca,   &m6801_cpu_device::illegl1,&m6801_cpu_device::inca,   &m6801_cpu_device::tsta,   &m6801_cpu_device::illegl1,&m6801_cpu_device::clra,
&m6801_cpu_device::negb,   &m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,&m6801_cpu_device::comb,   &m6801_cpu_device::lsrb,   &m6801_cpu_device::illegl1,&m6801_cpu_device::rorb,   &m6801_cpu_device::asrb,
&m6801_cpu_device::aslb,   &m6801_cpu_device::rolb,   &m6801_cpu_device::decb,   &m6801_cpu_device::illegl1,&m6801_cpu_device::incb,   &m6801_cpu_device::tstb,   &m6801_cpu_device::illegl1,&m6801_cpu_device::clrb,
&m6801_cpu_device::neg_ix, &m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,&m6801_cpu_device::com_ix, &m6801_cpu_device::lsr_ix, &m6801_cpu_device::illegl1,&m6801_cpu_device::ror_ix, &m6801_cpu_device::asr_ix,
&m6801_cpu_device::asl_ix, &m6801_cpu_device::rol_ix, &m6801_cpu_device::dec_ix, &m6801_cpu_device::illegl1,&m6801_cpu_device::inc_ix, &m6801_cpu_device::tst_ix, &m6801_cpu_device::jmp_ix, &m6801_cpu_device::clr_ix,
&m6801_cpu_device::neg_ex, &m6801_cpu_device::illegl1,&m6801_cpu_device::illegl1,&m6801_cpu_device::com_ex, &m6801_cpu_device::lsr_ex, &m6801_cpu_device::illegl1,&m6801_cpu_device::ror_ex, &m6801_cpu_device::asr_ex,
&m6801_cpu_device::asl_ex, &m6801_cpu_device::rol_ex, &m6801_cpu_device::dec_ex, &m6801_cpu_device::illegl1,&m6801_cpu_device::inc_ex, &m6801_cpu_device::tst_ex, &m6801_cpu_device::jmp_ex, &m6801_cpu_device::clr_ex,
&m6801_cpu_device::suba_im,&m6801_cpu_device::cmpa_im,&m6801_cpu_device::sbca_im,&m6801_cpu_device::subd_im,&m6801_cpu_device::anda_im,&m6801_cpu_device::bita_im,&m6801_cpu_device::lda_im, &m6801_cpu_device::sta_im,
&m6801_cpu_device::eora_im,&m6801_cpu_device::adca_im,&m6801_cpu_device::ora_im, &m6801_cpu_device::adda_im,&m6801_cpu_device::cpx_im ,&m6801_cpu_device::bsr,    &m6801_cpu_device::lds_im, &m6801_cpu_device::sts_im,
&m6801_cpu_device::suba_di,&m6801_cpu_device::cmpa_di,&m6801_cpu_device::sbca_di,&m6801_cpu_device::subd_di,&m6801_cpu_device::anda_di,&m6801_cpu_device::bita_di,&m6801_cpu_device::lda_di, &m6801_cpu_device::sta_di,
&m6801_cpu_device::eora_di,&m6801_cpu_device::adca_di,&m6801_cpu_device::ora_di, &m6801_cpu_device::adda_di,&m6801_cpu_device::cpx_di ,&m6801_cpu_device::jsr_di, &m6801_cpu_device::lds_di, &m6801_cpu_device::sts_di,
&m6801_cpu_device::suba_ix,&m6801_cpu_device::cmpa_ix,&m6801_cpu_device::sbca_ix,&m6801_cpu_device::subd_ix,&m6801_cpu_device::anda_ix,&m6801_cpu_device::bita_ix,&m6801_cpu_device::lda_ix, &m6801_cpu_device::sta_ix,
&m6801_cpu_device::eora_ix,&m6801_cpu_device::adca_ix,&m6801_cpu_device::ora_ix, &m6801_cpu_device::adda_ix,&m6801_cpu_device::cpx_ix ,&m6801_cpu_device::jsr_ix, &m6801_cpu_device::lds_ix, &m6801_cpu_device::sts_ix,
&m6801_cpu_device::suba_ex,&m6801_cpu_device::cmpa_ex,&m6801_cpu_device::sbca_ex,&m6801_cpu_device::subd_ex,&m6801_cpu_device::anda_ex,&m6801_cpu_device::bita_ex,&m6801_cpu_device::lda_ex, &m6801_cpu_device::sta_ex,
&m6801_cpu_device::eora_ex,&m6801_cpu_device::adca_ex,&m6801_cpu_device::ora_ex, &m6801_cpu_device::adda_ex,&m6801_cpu_device::cpx_ex ,&m6801_cpu_device::jsr_ex, &m6801_cpu_device::lds_ex, &m6801_cpu_device::sts_ex,
&m6801_cpu_device::subb_im,&m6801_cpu_device::cmpb_im,&m6801_cpu_device::sbcb_im,&m6801_cpu_device::addd_im,&m6801_cpu_device::andb_im,&m6801_cpu_device::bitb_im,&m6801_cpu_device::ldb_im, &m6801_cpu_device::stb_im,
&m6801_cpu_device::eorb_im,&m6801_cpu_device::adcb_im,&m6801_cpu_device::orb_im, &m6801_cpu_device::addb_im,&m6801_cpu_device::ldd_im, &m6801_cpu_device::std_im, &m6801_cpu_device::ldx_im, &m6801_cpu_device::stx_im,
&m6801_cpu_device::subb_di,&m6801_cpu_device::cmpb_di,&m6801_cpu_device::sbcb_di,&m6801_cpu_device::addd_di,&m6801_cpu_device::andb_di,&m6801_cpu_device::bitb_di,&m6801_cpu_device::ldb_di, &m6801_cpu_device::stb_di,
&m6801_cpu_device::eorb_di,&m6801_cpu_device::adcb_di,&m6801_cpu_device::orb_di, &m6801_cpu_device::addb_di,&m6801_cpu_device::ldd_di, &m6801_cpu_device::std_di, &m6801_cpu_device::ldx_di, &m6801_cpu_device::stx_di,
&m6801_cpu_device::subb_ix,&m6801_cpu_device::cmpb_ix,&m6801_cpu_device::sbcb_ix,&m6801_cpu_device::addd_ix,&m6801_cpu_device::andb_ix,&m6801_cpu_device::bitb_ix,&m6801_cpu_device::ldb_ix, &m6801_cpu_device::stb_ix,
&m6801_cpu_device::eorb_ix,&m6801_cpu_device::adcb_ix,&m6801_cpu_device::orb_ix, &m6801_cpu_device::addb_ix,&m6801_cpu_device::ldd_ix, &m6801_cpu_device::std_ix, &m6801_cpu_device::ldx_ix, &m6801_cpu_device::stx_ix,
&m6801_cpu_device::subb_ex,&m6801_cpu_device::cmpb_ex,&m6801_cpu_device::sbcb_ex,&m6801_cpu_device::addd_ex,&m6801_cpu_device::andb_ex,&m6801_cpu_device::bitb_ex,&m6801_cpu_device::ldb_ex, &m6801_cpu_device::stb_ex,
&m6801_cpu_device::eorb_ex,&m6801_cpu_device::adcb_ex,&m6801_cpu_device::orb_ex, &m6801_cpu_device::addb_ex,&m6801_cpu_device::ldd_ex, &m6801_cpu_device::std_ex, &m6801_cpu_device::ldx_ex, &m6801_cpu_device::stx_ex
};

const m6800_cpu_device::op_func m6801_cpu_device::hd63701_insn[0x100] = {
&m6801_cpu_device::trap,   &m6801_cpu_device::nop,    &m6801_cpu_device::trap,   &m6801_cpu_device::trap,   &m6801_cpu_device::lsrd,   &m6801_cpu_device::asld,   &m6801_cpu_device::tap,    &m6801_cpu_device::tpa,
&m6801_cpu_device::inx,    &m6801_cpu_device::dex,    &m6801_cpu_device::clv,    &m6801_cpu_device::sev,    &m6801_cpu_device::clc,    &m6801_cpu_device::sec,    &m6801_cpu_device::cli,    &m6801_cpu_device::sei,
&m6801_cpu_device::sba,    &m6801_cpu_device::cba,    &m6801_cpu_device::undoc1, &m6801_cpu_device::undoc2, &m6801_cpu_device::trap,   &m6801_cpu_device::trap,   &m6801_cpu_device::tab,    &m6801_cpu_device::tba,
&m6801_cpu_device::xgdx,   &m6801_cpu_device::daa,    &m6801_cpu_device::slp,    &m6801_cpu_device::aba,    &m6801_cpu_device::trap,   &m6801_cpu_device::trap,   &m6801_cpu_device::trap,   &m6801_cpu_device::trap,
&m6801_cpu_device::bra,    &m6801_cpu_device::brn,    &m6801_cpu_device::bhi,    &m6801_cpu_device::bls,    &m6801_cpu_device::bcc,    &m6801_cpu_device::bcs,    &m6801_cpu_device::bne,    &m6801_cpu_device::beq,
&m6801_cpu_device::bvc,    &m6801_cpu_device::bvs,    &m6801_cpu_device::bpl,    &m6801_cpu_device::bmi,    &m6801_cpu_device::bge,    &m6801_cpu_device::blt,    &m6801_cpu_device::bgt,    &m6801_cpu_device::ble,
&m6801_cpu_device::tsx,    &m6801_cpu_device::ins,    &m6801_cpu_device::pula,   &m6801_cpu_device::pulb,   &m6801_cpu_device::des,    &m6801_cpu_device::txs,    &m6801_cpu_device::psha,   &m6801_cpu_device::pshb,
&m6801_cpu_device::pulx,   &m6801_cpu_device::rts,    &m6801_cpu_device::abx,    &m6801_cpu_device::rti,    &m6801_cpu_device::pshx,   &m6801_cpu_device::mul,    &m6801_cpu_device::wai,    &m6801_cpu_device::swi,
&m6801_cpu_device::nega,   &m6801_cpu_device::trap,   &m6801_cpu_device::trap,   &m6801_cpu_device::coma,   &m6801_cpu_device::lsra,   &m6801_cpu_device::trap,   &m6801_cpu_device::rora,   &m6801_cpu_device::asra,
&m6801_cpu_device::asla,   &m6801_cpu_device::rola,   &m6801_cpu_device::deca,   &m6801_cpu_device::trap,   &m6801_cpu_device::inca,   &m6801_cpu_device::tsta,   &m6801_cpu_device::trap,   &m6801_cpu_device::clra,
&m6801_cpu_device::negb,   &m6801_cpu_device::trap,   &m6801_cpu_device::trap,   &m6801_cpu_device::comb,   &m6801_cpu_device::lsrb,   &m6801_cpu_device::trap,   &m6801_cpu_device::rorb,   &m6801_cpu_device::asrb,
&m6801_cpu_device::aslb,   &m6801_cpu_device::rolb,   &m6801_cpu_device::decb,   &m6801_cpu_device::trap,   &m6801_cpu_device::incb,   &m6801_cpu_device::tstb,   &m6801_cpu_device::trap,   &m6801_cpu_device::clrb,
&m6801_cpu_device::neg_ix, &m6801_cpu_device::aim_ix, &m6801_cpu_device::oim_ix, &m6801_cpu_device::com_ix, &m6801_cpu_device::lsr_ix, &m6801_cpu_device::eim_ix, &m6801_cpu_device::ror_ix, &m6801_cpu_device::asr_ix,
&m6801_cpu_device::asl_ix, &m6801_cpu_device::rol_ix, &m6801_cpu_device::dec_ix, &m6801_cpu_device::tim_ix, &m6801_cpu_device::inc_ix, &m6801_cpu_device::tst_ix, &m6801_cpu_device::jmp_ix, &m6801_cpu_device::clr_ix,
&m6801_cpu_device::neg_ex, &m6801_cpu_device::aim_di, &m6801_cpu_device::oim_di, &m6801_cpu_device::com_ex, &m6801_cpu_device::lsr_ex, &m6801_cpu_device::eim_di, &m6801_cpu_device::ror_ex, &m6801_cpu_device::asr_ex,
&m6801_cpu_device::asl_ex, &m6801_cpu_device::rol_ex, &m6801_cpu_device::dec_ex, &m6801_cpu_device::tim_di, &m6801_cpu_device::inc_ex, &m6801_cpu_device::tst_ex, &m6801_cpu_device::jmp_ex, &m6801_cpu_device::clr_ex,
&m6801_cpu_device::suba_im,&m6801_cpu_device::cmpa_im,&m6801_cpu_device::sbca_im,&m6801_cpu_device::subd_im,&m6801_cpu_device::anda_im,&m6801_cpu_device::bita_im,&m6801_cpu_device::lda_im, &m6801_cpu_device::sta_im,
&m6801_cpu_device::eora_im,&m6801_cpu_device::adca_im,&m6801_cpu_device::ora_im, &m6801_cpu_device::adda_im,&m6801_cpu_device::cpx_im ,&m6801_cpu_device::bsr,    &m6801_cpu_device::lds_im, &m6801_cpu_device::sts_im,
&m6801_cpu_device::suba_di,&m6801_cpu_device::cmpa_di,&m6801_cpu_device::sbca_di,&m6801_cpu_device::subd_di,&m6801_cpu_device::anda_di,&m6801_cpu_device::bita_di,&m6801_cpu_device::lda_di, &m6801_cpu_device::sta_di,
&m6801_cpu_device::eora_di,&m6801_cpu_device::adca_di,&m6801_cpu_device::ora_di, &m6801_cpu_device::adda_di,&m6801_cpu_device::cpx_di ,&m6801_cpu_device::jsr_di, &m6801_cpu_device::lds_di, &m6801_cpu_device::sts_di,
&m6801_cpu_device::suba_ix,&m6801_cpu_device::cmpa_ix,&m6801_cpu_device::sbca_ix,&m6801_cpu_device::subd_ix,&m6801_cpu_device::anda_ix,&m6801_cpu_device::bita_ix,&m6801_cpu_device::lda_ix, &m6801_cpu_device::sta_ix,
&m6801_cpu_device::eora_ix,&m6801_cpu_device::adca_ix,&m6801_cpu_device::ora_ix, &m6801_cpu_device::adda_ix,&m6801_cpu_device::cpx_ix ,&m6801_cpu_device::jsr_ix, &m6801_cpu_device::lds_ix, &m6801_cpu_device::sts_ix,
&m6801_cpu_device::suba_ex,&m6801_cpu_device::cmpa_ex,&m6801_cpu_device::sbca_ex,&m6801_cpu_device::subd_ex,&m6801_cpu_device::anda_ex,&m6801_cpu_device::bita_ex,&m6801_cpu_device::lda_ex, &m6801_cpu_device::sta_ex,
&m6801_cpu_device::eora_ex,&m6801_cpu_device::adca_ex,&m6801_cpu_device::ora_ex, &m6801_cpu_device::adda_ex,&m6801_cpu_device::cpx_ex ,&m6801_cpu_device::jsr_ex, &m6801_cpu_device::lds_ex, &m6801_cpu_device::sts_ex,
&m6801_cpu_device::subb_im,&m6801_cpu_device::cmpb_im,&m6801_cpu_device::sbcb_im,&m6801_cpu_device::addd_im,&m6801_cpu_device::andb_im,&m6801_cpu_device::bitb_im,&m6801_cpu_device::ldb_im, &m6801_cpu_device::stb_im,
&m6801_cpu_device::eorb_im,&m6801_cpu_device::adcb_im,&m6801_cpu_device::orb_im, &m6801_cpu_device::addb_im,&m6801_cpu_device::ldd_im, &m6801_cpu_device::std_im, &m6801_cpu_device::ldx_im, &m6801_cpu_device::stx_im,
&m6801_cpu_device::subb_di,&m6801_cpu_device::cmpb_di,&m6801_cpu_device::sbcb_di,&m6801_cpu_device::addd_di,&m6801_cpu_device::andb_di,&m6801_cpu_device::bitb_di,&m6801_cpu_device::ldb_di, &m6801_cpu_device::stb_di,
&m6801_cpu_device::eorb_di,&m6801_cpu_device::adcb_di,&m6801_cpu_device::orb_di, &m6801_cpu_device::addb_di,&m6801_cpu_device::ldd_di, &m6801_cpu_device::std_di, &m6801_cpu_device::ldx_di, &m6801_cpu_device::stx_di,
&m6801_cpu_device::subb_ix,&m6801_cpu_device::cmpb_ix,&m6801_cpu_device::sbcb_ix,&m6801_cpu_device::addd_ix,&m6801_cpu_device::andb_ix,&m6801_cpu_device::bitb_ix,&m6801_cpu_device::ldb_ix, &m6801_cpu_device::stb_ix,
&m6801_cpu_device::eorb_ix,&m6801_cpu_device::adcb_ix,&m6801_cpu_device::orb_ix, &m6801_cpu_device::addb_ix,&m6801_cpu_device::ldd_ix, &m6801_cpu_device::std_ix, &m6801_cpu_device::ldx_ix, &m6801_cpu_device::stx_ix,
&m6801_cpu_device::subb_ex,&m6801_cpu_device::cmpb_ex,&m6801_cpu_device::sbcb_ex,&m6801_cpu_device::addd_ex,&m6801_cpu_device::andb_ex,&m6801_cpu_device::bitb_ex,&m6801_cpu_device::ldb_ex, &m6801_cpu_device::stb_ex,
&m6801_cpu_device::eorb_ex,&m6801_cpu_device::adcb_ex,&m6801_cpu_device::orb_ex, &m6801_cpu_device::addb_ex,&m6801_cpu_device::ldd_ex, &m6801_cpu_device::std_ex, &m6801_cpu_device::ldx_ex, &m6801_cpu_device::stx_ex
};


void m6801_cpu_device::m6803_mem(address_map &map)
{
	map(0x0000, 0x001f).rw(FUNC(m6801_cpu_device::m6801_io_r), FUNC(m6801_cpu_device::m6801_io_w));
	map(0x0020, 0x007f).noprw();        /* unused */
	map(0x0080, 0x00ff).ram();        /* 6803 internal RAM */
}


DEFINE_DEVICE_TYPE(M6801, m6801_cpu_device, "m6801", "Motorola M6801")
DEFINE_DEVICE_TYPE(M6803, m6803_cpu_device, "m6803", "Motorola M6803")
DEFINE_DEVICE_TYPE(HD6301, hd6301_cpu_device, "hd6301", "Hitachi HD6301")
DEFINE_DEVICE_TYPE(HD63701, hd63701_cpu_device, "hd63701", "Hitachi HD63701")
DEFINE_DEVICE_TYPE(HD6303R, hd6303r_cpu_device, "hd6303r", "Hitachi HD6303R")
DEFINE_DEVICE_TYPE(HD6303Y, hd6303y_cpu_device, "hd6303y", "Hitachi HD6303Y")

m6801_cpu_device::m6801_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6801_cpu_device(mconfig, M6801, tag, owner, clock, m6803_insn, cycles_6803, address_map_constructor())
{
}

m6801_cpu_device::m6801_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const op_func *insn, const uint8_t *cycles, address_map_constructor internal)
	: m6800_cpu_device(mconfig, type, tag, owner, clock, insn, cycles, internal)
	, m_in_port_func{{*this}, {*this}, {*this}, {*this}}
	, m_out_port_func{{*this}, {*this}, {*this}, {*this}}
	, m_out_sc2_func(*this)
	, m_out_sertx_func(*this)
{
}

m6803_cpu_device::m6803_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6801_cpu_device(mconfig, M6803, tag, owner, clock, m6803_insn, cycles_6803, address_map_constructor(FUNC(m6803_cpu_device::m6803_mem), this))
{
}

hd6301_cpu_device::hd6301_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301_cpu_device(mconfig, HD6301, tag, owner, clock)
{
}

hd6301_cpu_device::hd6301_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: m6801_cpu_device(mconfig, type, tag, owner, clock, hd63701_insn, cycles_63701)
{
}

hd63701_cpu_device::hd63701_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6801_cpu_device(mconfig, HD63701, tag, owner, clock, hd63701_insn, cycles_63701)
{
}

hd6303r_cpu_device::hd6303r_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301_cpu_device(mconfig, HD6303R, tag, owner, clock)
{
}

hd6303y_cpu_device::hd6303y_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301_cpu_device(mconfig, HD6303Y, tag, owner, clock)
{
}

void m6801_cpu_device::m6800_check_irq2()
{
	if ((m_tcsr & (TCSR_EICI|TCSR_ICF)) == (TCSR_EICI|TCSR_ICF))
	{
		TAKE_ICI;
		standard_irq_callback(M6801_TIN_LINE);
	}
	else if ((m_tcsr & (TCSR_EOCI|TCSR_OCF)) == (TCSR_EOCI|TCSR_OCF))
	{
		TAKE_OCI;
	}
	else if ((m_tcsr & (TCSR_ETOI|TCSR_TOF)) == (TCSR_ETOI|TCSR_TOF))
	{
		TAKE_TOI;
	}
	else if (((m_trcsr & (M6801_TRCSR_RIE|M6801_TRCSR_RDRF)) == (M6801_TRCSR_RIE|M6801_TRCSR_RDRF)) ||
				((m_trcsr & (M6801_TRCSR_RIE|M6801_TRCSR_ORFE)) == (M6801_TRCSR_RIE|M6801_TRCSR_ORFE)) ||
				((m_trcsr & (M6801_TRCSR_TIE|M6801_TRCSR_TDRE)) == (M6801_TRCSR_TIE|M6801_TRCSR_TDRE)))
	{
		LOG("SCI interrupt\n");
		TAKE_SCI;
	}
}

/* check OCI or TOI */
void m6801_cpu_device::check_timer_event()
{
	/* OCI */
	if (CTD >= OCD)
	{
		OCH++;  // next IRQ point
		m_tcsr |= TCSR_OCF;
		m_pending_tcsr |= TCSR_OCF;
		MODIFIED_tcsr;
		if((m_tcsr & TCSR_EOCI) && m_wai_state & M6800_SLP)
			m_wai_state &= ~M6800_SLP;
		if ( !(m_cc & 0x10) && (m_tcsr & TCSR_EOCI))
			TAKE_OCI;

		// if output on P21 is enabled, let's do it
		if (m_port_ddr[1] & 2)
		{
			m_port_data[1] &= ~2;
			m_port_data[1] |= (m_tcsr & TCSR_OLVL) << 1;
			m_port2_written = 1;
			write_port2();
		}
	}
	/* TOI */
	if (CTD >= TOD)
	{
		TOH++;  // next IRQ point
#if 0
		CLEANUP_COUNTERS();
#endif
		m_tcsr |= TCSR_TOF;
		m_pending_tcsr |= TCSR_TOF;
		MODIFIED_tcsr;
		if((m_tcsr & TCSR_ETOI) && m_wai_state & M6800_SLP)
			m_wai_state &= ~M6800_SLP;
		if ( !(m_cc & 0x10) && (m_tcsr & TCSR_ETOI))
			TAKE_TOI;
	}
	/* set next event */
	SET_TIMER_EVENT;
}

void m6801_cpu_device::increment_counter(int amount)
{
	m6800_cpu_device::increment_counter(amount);
	CTD += amount;
	if (CTD >= m_timer_next)
		check_timer_event();
}

void m6801_cpu_device::EAT_CYCLES()
{
	int cycles_to_eat = std::min(int(m_timer_next - CTD), m_icount);
	if (cycles_to_eat > 0)
		increment_counter(cycles_to_eat);
}

/* cleanup high-word of counters */
void m6801_cpu_device::CLEANUP_COUNTERS()
{
	OCH -= CTH;
	TOH -= CTH;
	CTH = 0;
	SET_TIMER_EVENT;
}

void m6801_cpu_device::set_rmcr(uint8_t data)
{
	if (m_rmcr == data) return;

	m_rmcr = data;

	switch ((m_rmcr & M6801_RMCR_CC_MASK) >> 2)
	{
	case 0:
		m_sci_timer->enable(false);
		m_use_ext_serclock = false;
		break;

	case 3: // external clock
		m_use_ext_serclock = true;
		m_sci_timer->enable(false);
		break;

	case 1:
	case 2:
		{
			int divisor = M6801_RMCR_SS[m_rmcr & M6801_RMCR_SS_MASK];
			attotime period = cycles_to_attotime(divisor);

			m_sci_timer->adjust(period, 0, period);
			m_use_ext_serclock = false;
		}
		break;
	}
}

int m6801_cpu_device::m6800_rx()
{
	return (m_in_port_func[1]() & M6801_PORT2_IO3) >> 3;
}

void m6801_cpu_device::serial_transmit()
{
	LOGTXTICK("Tx Tick\n");

	if (m_trcsr & M6801_TRCSR_TE)
	{
		// force Port 2 bit 4 as output
		m_port_ddr[1] |= M6801_PORT2_IO4;

		switch (m_txstate)
		{
		case M6801_TX_STATE_INIT:
			m_tx = 1;
			m_txbits++;

			if (m_txbits == 10)
			{
				m_txstate = M6801_TX_STATE_READY;
				m_txbits = M6801_SERIAL_START;
			}
			break;

		case M6801_TX_STATE_READY:
			switch (m_txbits)
			{
			case M6801_SERIAL_START:
				if (m_trcsr & M6801_TRCSR_TDRE)
				{
					// transmit buffer is empty, send nothing
					return;
				}
				else
				{
					// transmit buffer is full, send data

					// load TDR to shift register
					m_tsr = m_tdr;

					// transmit buffer is empty, set TDRE flag
					m_trcsr |= M6801_TRCSR_TDRE;

					// send start bit '0'
					m_tx = 0;

					m_txbits++;

					LOGTX("Transmit START Data %02x\n", m_tsr);
				}
				break;

			case M6801_SERIAL_STOP:
				// send stop bit '1'
				m_tx = 1;

				CHECK_IRQ_LINES();

				m_txbits = M6801_SERIAL_START;

				LOGTX("Transmit STOP\n");
				break;

			default:
				// send data bit '0' or '1'
				m_tx = m_tsr & 0x01;

				// shift transmit register
				m_tsr >>= 1;

				LOGTX("Transmit Bit %u: %u\n", m_txbits, m_tx);

				m_txbits++;
				break;
			}
			break;
		}

		m_out_sertx_func((m_tx == 1) ? ASSERT_LINE : CLEAR_LINE);
		m_port2_written = 1;
		write_port2();
	}
}

void m6801_cpu_device::serial_receive()
{
	LOGRXTICK("Rx Tick TRCSR %02x bits %u check %02x\n", m_trcsr, m_rxbits, m_trcsr & M6801_TRCSR_RE);

	if (m_trcsr & M6801_TRCSR_RE)
	{
		if (m_trcsr & M6801_TRCSR_WU)
		{
			// wait for 10 bits of '1'
			if (m6800_rx() == 1)
			{
				m_rxbits++;

				LOGRX("Received WAKE UP bit %u\n", m_rxbits);

				if (m_rxbits == 10)
				{
					LOGRX("Receiver Wake Up\n");

					m_trcsr &= ~M6801_TRCSR_WU;
					m_rxbits = M6801_SERIAL_START;
				}
			}
			else
			{
				LOGRX("Receiver Wake Up interrupted\n");

				m_rxbits = M6801_SERIAL_START;
			}
		}
		else
		{
			// receive data
			switch (m_rxbits)
			{
			case M6801_SERIAL_START:
				if (m6800_rx() == 0)
				{
					// start bit found
					m_rxbits++;

					LOGRX("Received START bit\n");
				}
				break;

			case M6801_SERIAL_STOP:
				if (m6800_rx() == 1)
				{
					LOGRX("Received STOP bit\n");

					if (m_trcsr & M6801_TRCSR_RDRF)
					{
						// overrun error
						m_trcsr |= M6801_TRCSR_ORFE;

						LOGRX("Receive Overrun Error\n");

						CHECK_IRQ_LINES();
					}
					else
					{
						if (!(m_trcsr & M6801_TRCSR_ORFE))
						{
							// transfer data into receive register
							m_rdr = m_rsr;

							LOGRX("Receive Data Register: %02x\n", m_rdr);

							// set RDRF flag
							m_trcsr |= M6801_TRCSR_RDRF;

							CHECK_IRQ_LINES();
						}
					}
				}
				else
				{
					// framing error
					if (!(m_trcsr & M6801_TRCSR_ORFE))
					{
						// transfer unframed data into receive register
						m_rdr = m_rsr;
					}

					m_trcsr |= M6801_TRCSR_ORFE;
					m_trcsr &= ~M6801_TRCSR_RDRF;

					LOGRX("Receive Framing Error\n");

					CHECK_IRQ_LINES();
				}

				m_rxbits = M6801_SERIAL_START;
				break;

			default:
				// shift receive register
				m_rsr >>= 1;

				// receive bit into register
				m_rsr |= (m6800_rx() << 7);

				LOGRX("Received DATA bit %u: %u\n", m_rxbits, BIT(m_rsr, 7));

				m_rxbits++;
				break;
			}
		}
	}
}

TIMER_CALLBACK_MEMBER( m6801_cpu_device::sci_tick )
{
	serial_transmit();
	serial_receive();
}


void m6801_cpu_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case M6801_SC1_LINE:
		if (!m_sc1_state && (CLEAR_LINE != state))
		{
			if (!m_port3_latched && (m_p3csr & M6801_P3CSR_LE))
			{
				// latch input data to port 3
				m_port_data[2] = (m_in_port_func[2]() & (m_port_ddr[2] ^ 0xff)) | (m_port_data[2] & m_port_ddr[2]);
				m_port3_latched = 1;
				LOGPORT("Latched Port 3 Data: %02x\n", m_port_data[2]);

				// set IS3 flag bit
				m_p3csr |= M6801_P3CSR_IS3_FLAG;
			}
			else
			{
				LOGPORT("Not latching Port 3 Data:%s%s", m_port3_latched ? " already latched" : "", (m_p3csr & M6801_P3CSR_LE) ? "" : " LE clear");
			}
		}
		m_sc1_state = ASSERT_LINE == state;
		if (CLEAR_LINE != state)
			standard_irq_callback(M6801_SC1_LINE); // re-entrant - do it after setting m_sc1_state
		break;

	case M6801_TIN_LINE:
		if (state != m_irq_state[M6801_TIN_LINE])
		{
			m_irq_state[M6801_TIN_LINE] = state;
			//eddge = (state == CLEAR_LINE ) ? 2 : 0;
			if( ((m_tcsr&TCSR_IEDG) ^ (state==CLEAR_LINE ? TCSR_IEDG : 0))==0 )
				return;
			/* active edge in */
			m_tcsr |= TCSR_ICF;
			m_pending_tcsr |= TCSR_ICF;
			m_input_capture = CT;
			MODIFIED_tcsr;
		}
		break;

	default:
		m6800_cpu_device::execute_set_input(irqline, state);
		break;
	}
}


void m6801_cpu_device::device_resolve_objects()
{
	for (auto &cb : m_in_port_func)
		cb.resolve_safe(0xff);
	for (auto &cb : m_out_port_func)
		cb.resolve_safe();
	m_out_sc2_func.resolve_safe();
	m_out_sertx_func.resolve_safe();
}


void m6801_cpu_device::device_start()
{
	m6800_cpu_device::device_start();

	m_sci_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m6801_cpu_device::sci_tick),this));

	m_port_ddr[3] = 0;
	m_port_data[3] = 0;
	m_input_capture = 0;
	m_rdr = 0;
	m_tdr = 0;
	m_rmcr = 0;
	m_ram_ctrl = 0;

	save_item(NAME(m_port_ddr));
	save_item(NAME(m_port_data));
	save_item(NAME(m_p3csr));
	save_item(NAME(m_tcsr));
	save_item(NAME(m_pending_tcsr));
	save_item(NAME(m_irq2));
	save_item(NAME(m_ram_ctrl));

	save_item(NAME(m_counter.d));
	save_item(NAME(m_output_compare.d));
	save_item(NAME(m_input_capture));
	save_item(NAME(m_p3csr_is3_flag_read));
	save_item(NAME(m_port3_latched));
	save_item(NAME(m_port2_written));

	save_item(NAME(m_trcsr));
	save_item(NAME(m_rmcr));
	save_item(NAME(m_rdr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_rxbits));
	save_item(NAME(m_txbits));
	save_item(NAME(m_txstate));
	save_item(NAME(m_trcsr_read_tdre));
	save_item(NAME(m_trcsr_read_orfe));
	save_item(NAME(m_trcsr_read_rdrf));
	save_item(NAME(m_tx));
	save_item(NAME(m_ext_serclock));
	save_item(NAME(m_use_ext_serclock));

	save_item(NAME(m_latch09));

	save_item(NAME(m_timer_over.d));

	save_item(NAME(m_timer_next));

	save_item(NAME(m_sc1_state));
}

void m6801_cpu_device::device_reset()
{
	m6800_cpu_device::device_reset();

	m_irq_state[M6801_TIN_LINE] = 0;
	m_sc1_state = 0;

	m_port_ddr[0] = 0x00;
	m_port_ddr[1] = 0x00;
	m_port_ddr[2] = 0x00;
	m_port_data[0] = 0;
	m_p3csr = 0x00;
	m_p3csr_is3_flag_read = 0;
	m_port2_written = 0;
	m_port3_latched = 0;
	/* TODO: on reset port 2 should be read to determine the operating mode (bits 0-2) */
	m_tcsr = 0x00;
	m_pending_tcsr = 0x00;
	m_irq2 = 0;
	CTD = 0x0000;
	OCD = 0xffff;
	TOD = 0xffff;
	m_timer_next = 0xffff;
	m_ram_ctrl |= 0x40;
	m_latch09 = 0;

	m_trcsr = M6801_TRCSR_TDRE;

	m_txstate = M6801_TX_STATE_INIT;
	m_txbits = m_rxbits = 0;
	m_tx = 1;
	m_trcsr_read_tdre = 0;
	m_trcsr_read_orfe = 0;
	m_trcsr_read_rdrf = 0;
	m_ext_serclock = 0;
	m_use_ext_serclock = false;

	set_rmcr(0);
}


void m6801_cpu_device::write_port2()
{
	if (!m_port2_written) return;

	uint8_t data = m_port_data[1];
	uint8_t ddr = m_port_ddr[1] & 0x1f;

	if ((ddr != 0x1f) && ddr)
	{
		data = (m_port_data[1] & ddr) | (ddr ^ 0xff);
	}

	if (m_trcsr & M6801_TRCSR_TE)
	{
		data = (data & 0xef) | (m_tx << 4);
	}

	data &= 0x1f;

	m_out_port_func[1](data);
}

/*
    if change_pc() direccted these areas ,Call hd63701_trap_pc().
    'mode' is selected by the sense of p2.0,p2.1,and p2.3 at reset timming.
    mode 0,1,2,4,6 : $0000-$001f
    mode 5         : $0000-$001f,$0200-$efff
    mode 7         : $0000-$001f,$0100-$efff
*/

void m6801_cpu_device::set_os3(int state)
{
	LOG("OS3: %u\n", state);

	m_out_sc2_func(state);
}

READ8_MEMBER( m6801_cpu_device::m6801_io_r )
{
	uint8_t data = 0;

	switch (offset)
	{
	case IO_P1DDR:
		data = m_port_ddr[0];
		break;

	case IO_P2DDR:
		data = m_port_ddr[1];
		break;

	case IO_P1DATA:
		if(m_port_ddr[0] == 0xff)
			data = m_port_data[0];
		else
			data = (m_in_port_func[0]() & (m_port_ddr[0] ^ 0xff))
				| (m_port_data[0] & m_port_ddr[0]);
		break;

	case IO_P2DATA:
		if(m_port_ddr[1] == 0xff)
			data = m_port_data[1];
		else
			data = (m_in_port_func[1]() & (m_port_ddr[1] ^ 0xff))
				| (m_port_data[1] & m_port_ddr[1]);
		break;

	case IO_P3DDR:
		data = 0xff;
		break;

	case IO_P4DDR:
		data = m_port_ddr[3];
		break;

	case IO_P3DATA:
		if (!machine().side_effects_disabled())
		{
			if (m_p3csr_is3_flag_read)
			{
				LOGPORT("Cleared IS3\n");
				m_p3csr &= ~M6801_P3CSR_IS3_FLAG;
				m_p3csr_is3_flag_read = 0;
			}

			if (!(m_p3csr & M6801_P3CSR_OSS))
			{
				set_os3(ASSERT_LINE);
			}
		}

		if ((m_p3csr & M6801_P3CSR_LE) || (m_port_ddr[2] == 0xff))
			data = m_port_data[2];
		else
			data = (m_in_port_func[2]() & (m_port_ddr[2] ^ 0xff))
				| (m_port_data[2] & m_port_ddr[2]);

		if (!machine().side_effects_disabled())
		{
			m_port3_latched = 0;

			if (!(m_p3csr & M6801_P3CSR_OSS))
			{
				set_os3(CLEAR_LINE);
			}
		}
		break;

	case IO_P4DATA:
		if(m_port_ddr[3] == 0xff)
			data = m_port_data[3];
		else
			data = (m_in_port_func[3]() & (m_port_ddr[3] ^ 0xff))
				| (m_port_data[3] & m_port_ddr[3]);
		break;

	case IO_TCSR:
		m_pending_tcsr = 0;
		data = m_tcsr;
		break;

	case IO_CH:
		if(!(m_pending_tcsr&TCSR_TOF) && !machine().side_effects_disabled())
		{
			m_tcsr &= ~TCSR_TOF;
			MODIFIED_tcsr;
		}
		data = m_counter.b.h;
		break;

	case IO_CL:
		data = m_counter.b.l;
		// HACK there should be a break here, but Coleco Adam won't boot with it present, proper fix required to the free-running counter

	case IO_OCRH:
		if(!(m_pending_tcsr&TCSR_OCF) && !machine().side_effects_disabled())
		{
			m_tcsr &= ~TCSR_OCF;
			MODIFIED_tcsr;
		}
		data = m_output_compare.b.h;
		break;

	case IO_OCRL:
		if(!(m_pending_tcsr&TCSR_OCF) && !machine().side_effects_disabled())
		{
			m_tcsr &= ~TCSR_OCF;
			MODIFIED_tcsr;
		}
		data = m_output_compare.b.l;
		break;

	case IO_ICRH:
		if(!(m_pending_tcsr&TCSR_ICF) && !machine().side_effects_disabled())
		{
			m_tcsr &= ~TCSR_ICF;
			MODIFIED_tcsr;
		}
		data = (m_input_capture >> 0) & 0xff;
		break;

	case IO_ICRL:
		data = (m_input_capture >> 8) & 0xff;
		break;

	case IO_P3CSR:
		if ((m_p3csr & M6801_P3CSR_IS3_FLAG) && !machine().side_effects_disabled())
		{
			m_p3csr_is3_flag_read = 1;
		}

		data = m_p3csr;
		break;

	case IO_RMCR:
		data = m_rmcr;
		break;

	case IO_TRCSR:
		if (!machine().side_effects_disabled())
		{
			if (m_trcsr & M6801_TRCSR_TDRE)
			{
				m_trcsr_read_tdre = 1;
			}

			if (m_trcsr & M6801_TRCSR_ORFE)
			{
				m_trcsr_read_orfe = 1;
			}

			if (m_trcsr & M6801_TRCSR_RDRF)
			{
				m_trcsr_read_rdrf = 1;
			}
		}

		data = m_trcsr;
		break;

	case IO_RDR:
		if (!machine().side_effects_disabled())
		{
			if (m_trcsr_read_orfe)
			{
				LOG("Cleared ORFE\n");
				m_trcsr_read_orfe = 0;
				m_trcsr &= ~M6801_TRCSR_ORFE;
			}

			if (m_trcsr_read_rdrf)
			{
				LOG("Cleared RDRF\n");
				m_trcsr_read_rdrf = 0;
				m_trcsr &= ~M6801_TRCSR_RDRF;
			}
		}

		data = m_rdr;
		break;

	case IO_TDR:
		data = m_tdr;
		break;

	case IO_RCR:
		data = m_ram_ctrl;
		break;

	case IO_CAAH:
	case IO_CAAL:
	case IO_TCR1:
	case IO_TCR2:
	case IO_TSR:
	case IO_OCR2H:
	case IO_OCR2L:
	case IO_OCR3H:
	case IO_OCR3L:
	case IO_ICR2H:
	case IO_ICR2L:
	default:
		logerror("PC %04x: warning - read from reserved internal register %02x\n", pc(),offset);
	}

	return data;
}

WRITE8_MEMBER( m6801_cpu_device::m6801_io_w )
{
	switch (offset)
	{
	case IO_P1DDR:
		LOGPORT("Port 1 Data Direction Register: %02x\n", data);

		if (m_port_ddr[0] != data)
		{
			m_port_ddr[0] = data;
			m_out_port_func[0]((m_port_data[0] & m_port_ddr[0]) | (m_port_ddr[0] ^ 0xff));
		}
		break;

	case IO_P2DDR:
		LOGPORT("Port 2 Data Direction Register: %02x\n", data);

		if (m_port_ddr[1] != data)
		{
			m_port_ddr[1] = data;
			write_port2();
		}
		break;

	case IO_P1DATA:
		LOGPORT("Port 1 Data Register: %02x\n", data);

		m_port_data[0] = data;
		m_out_port_func[0]((m_port_data[0] & m_port_ddr[0]) | (m_port_ddr[0] ^ 0xff));
		break;

	case IO_P2DATA:
		LOGPORT("Port 2 Data Register: %02x\n", data);

		m_port_data[1] = data;
		m_port2_written = 1;
		write_port2();
		break;

	case IO_P3DDR:
		LOGPORT("Port 3 Data Direction Register: %02x\n", data);

		if (m_port_ddr[2] != data)
		{
			m_port_ddr[2] = data;
			m_out_port_func[2]((m_port_data[2] & m_port_ddr[2]) | (m_port_ddr[2] ^ 0xff));
		}
		break;

	case IO_P4DDR:
		LOGPORT("Port 4 Data Direction Register: %02x\n", data);

		if (m_port_ddr[3] != data)
		{
			m_port_ddr[3] = data;
			m_out_port_func[3]((m_port_data[3] & m_port_ddr[3]) | (m_port_ddr[3] ^ 0xff));
		}
		break;

	case IO_P3DATA:
		LOGPORT("Port 3 Data Register: %02x\n", data);

		if (m_p3csr_is3_flag_read)
		{
			LOGPORT("Cleared IS3\n");
			m_p3csr &= ~M6801_P3CSR_IS3_FLAG;
			m_p3csr_is3_flag_read = 0;
		}

		if (m_p3csr & M6801_P3CSR_OSS)
		{
			set_os3(ASSERT_LINE);
		}

		m_port_data[2] = data;
		m_out_port_func[2]((m_port_data[2] & m_port_ddr[2]) | (m_port_ddr[2] ^ 0xff));

		if (m_p3csr & M6801_P3CSR_OSS)
		{
			set_os3(CLEAR_LINE);
		}
		break;

	case IO_P4DATA:
		LOGPORT("Port 4 Data Register: %02x\n", data);

		m_port_data[3] = data;
		m_out_port_func[3]((m_port_data[3] & m_port_ddr[3]) | (m_port_ddr[3] ^ 0xff));
		break;

	case IO_TCSR:
		LOG("Timer Control and Status Register: %02x\n", data);

		m_tcsr = data;
		m_pending_tcsr &= m_tcsr;
		MODIFIED_tcsr;
		if( !(m_cc & 0x10) )
			m6800_check_irq2();
		break;

	case IO_CH:
		LOG("Counter High Register: %02x\n", data);

		m_latch09 = data & 0xff;    /* 6301 only */
		CT  = 0xfff8;
		TOH = CTH;
		MODIFIED_counters;
		break;

	case IO_CL: /* 6301 only */
		LOG("Counter Low Register: %02x\n", data);

		CT = (m_latch09 << 8) | (data & 0xff);
		TOH = CTH;
		MODIFIED_counters;
		break;

	case IO_OCRH:
		LOG("Output Compare High Register: %02x\n", data);

		if( m_output_compare.b.h != data)
		{
			m_output_compare.b.h = data;
			MODIFIED_counters;
		}
		break;

	case IO_OCRL:
		LOG("Output Compare Low Register: %02x\n", data);

		if( m_output_compare.b.l != data)
		{
			m_output_compare.b.l = data;
			MODIFIED_counters;
		}
		break;

	case IO_ICRH:
	case IO_ICRL:
	case IO_RDR:
		LOG("PC %04x: warning - write %02x to read only internal register %02x\n",pc(),data,offset);
		break;

	case IO_P3CSR:
		LOGPORT("Port 3 Control and Status Register: %02x\n", data);

		m_p3csr = data;
		break;

	case IO_RMCR:
		LOG("Rate and Mode Control Register: %02x\n", data);

		set_rmcr(data);
		break;

	case IO_TRCSR:
		LOG("Transmit/Receive Control and Status Register: %02x\n", data);

		if ((data & M6801_TRCSR_TE) && !(m_trcsr & M6801_TRCSR_TE))
		{
			m_txstate = M6801_TX_STATE_INIT;
			m_txbits = 0;
			m_tx = 1;
		}

		if ((data & M6801_TRCSR_RE) && !(m_trcsr & M6801_TRCSR_RE))
		{
			m_rxbits = 0;
		}

		m_trcsr = (m_trcsr & 0xe0) | (data & 0x1f);
		break;

	case IO_TDR:
		LOGTX("Transmit Data Register: %02x\n", data);

		if (m_trcsr_read_tdre)
		{
			m_trcsr_read_tdre = 0;
			m_trcsr &= ~M6801_TRCSR_TDRE;
		}
		m_tdr = data;
		break;

	case IO_RCR:
		LOG("RAM Control Register: %02x\n", data);

		m_ram_ctrl = data;
		break;

	case IO_CAAH:
	case IO_CAAL:
	case IO_TCR1:
	case IO_TCR2:
	case IO_TSR:
	case IO_OCR2H:
	case IO_OCR2L:
	case IO_OCR3H:
	case IO_OCR3L:
	case IO_ICR2H:
	case IO_ICR2L:
	default:
		logerror("PC %04x: warning - write %02x to reserved internal register %02x\n", pc(),data,offset);
		break;
	}
}


void m6801_cpu_device::m6801_clock_serial()
{
	if (m_use_ext_serclock)
	{
		m_ext_serclock++;

		if (m_ext_serclock >= 8)
		{
			m_ext_serclock = 0;
			serial_transmit();
			serial_receive();
		}
	}
}

std::unique_ptr<util::disasm_interface> m6801_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6801);
}

std::unique_ptr<util::disasm_interface> m6803_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6803);
}

std::unique_ptr<util::disasm_interface> hd6301_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6301);
}

std::unique_ptr<util::disasm_interface> hd63701_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(63701);
}

void hd63701_cpu_device::TAKE_TRAP()
{
	enter_interrupt("take TRAP\n",0xffee);
}
