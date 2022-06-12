// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "m6801.h"
#include "6800dasm.h"

#define LOG_GENERAL (1U << 0)
#define LOG_TX      (1U << 1)
#define LOG_TXTICK  (1U << 2)
#define LOG_RX      (1U << 3)
#define LOG_RXTICK  (1U << 4)
#define LOG_PORT    (1U << 5)
#define LOG_SER     (1U << 6)
#define LOG_TIMER   (1U << 7)

//#define VERBOSE (LOG_SER)
//#define LOG_OUTPUT_STREAM std::cout
//#define LOG_OUTPUT_STREAM std::cerr
#include "logmacro.h"

#define LOGTX(...)      LOGMASKED(LOG_TX, __VA_ARGS__)
#define LOGTXTICK(...)  LOGMASKED(LOG_TXTICK, __VA_ARGS__)
#define LOGRX(...)      LOGMASKED(LOG_RX, __VA_ARGS__)
#define LOGRXTICK(...)  LOGMASKED(LOG_RXTICK, __VA_ARGS__)
#define LOGPORT(...)    LOGMASKED(LOG_PORT, __VA_ARGS__)
#define LOGSER(...)     LOGMASKED(LOG_SER, __VA_ARGS__)
#define LOGTIMER(...)   LOGMASKED(LOG_TIMER, __VA_ARGS__)


#define CT      m_counter.w.l
#define CTH     m_counter.w.h
#define CTD     m_counter.d
#define OC      m_output_compare.w.l
#define OCH     m_output_compare.w.h
#define OCD     m_output_compare.d
#define OC2     m_output_compare2.w.l
#define OC2H    m_output_compare2.w.h
#define OC2D    m_output_compare2.d
#define TOH     m_timer_over.w.h
#define TOD     m_timer_over.d

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
#define TAKE_ISI enter_interrupt("take ISI\n",0xfff8)
#define TAKE_ICI enter_interrupt("take ICI\n",0xfff6)
#define TAKE_OCI enter_interrupt("take OCI\n",0xfff4)
#define TAKE_TOI enter_interrupt("take TOI\n",0xfff2)
#define TAKE_SCI enter_interrupt("take SCI\n",0xfff0)
#define TAKE_CMI enter_interrupt("take CMI\n",0xffec)

/* mnemonics for the Timer Control and Status Register bits */
#define TCSR_OLVL 0x01
#define TCSR_IEDG 0x02
#define TCSR_ETOI 0x04
#define TCSR_EOCI 0x08
#define TCSR_EICI 0x10
#define TCSR_TOF  0x20
#define TCSR_OCF  0x40
#define TCSR_ICF  0x80

#define TCSR2_OE1   0x01
#define TCSR2_OE2   0x02
#define TCSR2_OLVL2 0x04
#define TCSR2_EOCI2 0x08
#define TCSR2_OCF2  0x20

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


void m6801_cpu_device::m6801_io(address_map &map)
{
	map(0x0000, 0x0000).rw(FUNC(m6801_cpu_device::ff_r), FUNC(m6801_cpu_device::p1_ddr_w));
	map(0x0001, 0x0001).rw(FUNC(m6801_cpu_device::ff_r), FUNC(m6801_cpu_device::p2_ddr_w));
	map(0x0002, 0x0002).rw(FUNC(m6801_cpu_device::p1_data_r), FUNC(m6801_cpu_device::p1_data_w));
	map(0x0003, 0x0003).rw(FUNC(m6801_cpu_device::p2_data_r), FUNC(m6801_cpu_device::p2_data_w));
	map(0x0004, 0x0004).rw(FUNC(m6801_cpu_device::ff_r), FUNC(m6801_cpu_device::p3_ddr_w)); // TODO: external in 6801 modes 0–3 & 6
	map(0x0005, 0x0005).rw(FUNC(m6801_cpu_device::ff_r), FUNC(m6801_cpu_device::p4_ddr_w)); // TODO: external in 6801 modes 0–3
	map(0x0006, 0x0006).rw(FUNC(m6801_cpu_device::p3_data_r), FUNC(m6801_cpu_device::p3_data_w)); // TODO: external in 6801 modes 0–3 & 6
	map(0x0007, 0x0007).rw(FUNC(m6801_cpu_device::p4_data_r), FUNC(m6801_cpu_device::p4_data_w)); // TODO: external in 6801 modes 0–3
	map(0x0008, 0x0008).rw(FUNC(m6801_cpu_device::tcsr_r), FUNC(m6801_cpu_device::tcsr_w));
	map(0x0009, 0x0009).rw(FUNC(m6801_cpu_device::ch_r), FUNC(m6801_cpu_device::ch_w));
	map(0x000a, 0x000a).rw(FUNC(m6801_cpu_device::cl_r), FUNC(m6801_cpu_device::cl_w));
	map(0x000b, 0x000b).rw(FUNC(m6801_cpu_device::ocrh_r), FUNC(m6801_cpu_device::ocrh_w));
	map(0x000c, 0x000c).rw(FUNC(m6801_cpu_device::ocrl_r), FUNC(m6801_cpu_device::ocrl_w));
	map(0x000d, 0x000d).r(FUNC(m6801_cpu_device::icrh_r));
	map(0x000e, 0x000e).r(FUNC(m6801_cpu_device::icrl_r));
	map(0x000f, 0x000f).rw(FUNC(m6801_cpu_device::p3_csr_r), FUNC(m6801_cpu_device::p3_csr_w)); // TODO: external in 6801 modes 0–3, 5 & 6
	map(0x0010, 0x0010).rw(FUNC(m6801_cpu_device::sci_rmcr_r), FUNC(m6801_cpu_device::sci_rmcr_w));
	map(0x0011, 0x0011).rw(FUNC(m6801_cpu_device::sci_trcsr_r), FUNC(m6801_cpu_device::sci_trcsr_w));
	map(0x0012, 0x0012).r(FUNC(m6801_cpu_device::sci_rdr_r));
	map(0x0013, 0x0013).w(FUNC(m6801_cpu_device::sci_tdr_w));
	map(0x0014, 0x0014).rw(FUNC(m6801_cpu_device::rcr_r), FUNC(m6801_cpu_device::rcr_w));
}

void m6801_cpu_device::m6803_mem(address_map &map)
{
	m6801_io(map);
	map(0x0080, 0x00ff).ram();        /* 6803 internal RAM */
}

void hd6301x_cpu_device::hd6301x_io(address_map &map)
{
	map(0x0001, 0x0001).rw(FUNC(hd6301x_cpu_device::ff_r), FUNC(hd6301x_cpu_device::p2_ddr_2bit_w));
	map(0x0002, 0x0002).rw(FUNC(hd6301x_cpu_device::p1_data_r), FUNC(hd6301x_cpu_device::p1_data_w)); // TODO: external except in single-chip mode
	map(0x0003, 0x0003).rw(FUNC(hd6301x_cpu_device::p2_data_r), FUNC(hd6301x_cpu_device::p2_data_w));
	map(0x0004, 0x0004).rw(FUNC(hd6301x_cpu_device::ff_r), FUNC(hd6301x_cpu_device::p3_ddr_w)); // TODO: external except in single-chip mode
	map(0x0005, 0x0005).rw(FUNC(hd6301x_cpu_device::ff_r), FUNC(hd6301x_cpu_device::p4_ddr_w)); // TODO: external except in single-chip mode
	map(0x0006, 0x0006).rw(FUNC(hd6301x_cpu_device::p3_data_r), FUNC(hd6301x_cpu_device::p3_data_w));
	map(0x0007, 0x0007).rw(FUNC(hd6301x_cpu_device::p4_data_r), FUNC(hd6301x_cpu_device::p4_data_w));
	map(0x0008, 0x0008).rw(FUNC(hd6301x_cpu_device::tcsr_r), FUNC(hd6301x_cpu_device::tcsr_w));
	map(0x0009, 0x0009).rw(FUNC(hd6301x_cpu_device::ch_r), FUNC(hd6301x_cpu_device::ch_w));
	map(0x000a, 0x000a).rw(FUNC(hd6301x_cpu_device::cl_r), FUNC(hd6301x_cpu_device::cl_w));
	map(0x000b, 0x000b).rw(FUNC(hd6301x_cpu_device::ocrh_r), FUNC(hd6301x_cpu_device::ocrh_w));
	map(0x000c, 0x000c).rw(FUNC(hd6301x_cpu_device::ocrl_r), FUNC(hd6301x_cpu_device::ocrl_w));
	map(0x000d, 0x000d).r(FUNC(hd6301x_cpu_device::icrh_r));
	map(0x000e, 0x000e).r(FUNC(hd6301x_cpu_device::icrl_r));
	map(0x000f, 0x000f).rw(FUNC(hd6301x_cpu_device::tcsr2_r), FUNC(hd6301x_cpu_device::tcsr2_w));
	map(0x0010, 0x0010).rw(FUNC(hd6301x_cpu_device::sci_rmcr_r), FUNC(hd6301x_cpu_device::sci_rmcr_w));
	map(0x0011, 0x0011).rw(FUNC(hd6301x_cpu_device::sci_trcsr_r), FUNC(hd6301x_cpu_device::sci_trcsr_w));
	map(0x0012, 0x0012).r(FUNC(hd6301x_cpu_device::sci_rdr_r));
	map(0x0013, 0x0013).w(FUNC(hd6301x_cpu_device::sci_tdr_w));
	map(0x0014, 0x0014).rw(FUNC(hd6301x_cpu_device::rcr_r), FUNC(hd6301x_cpu_device::rcr_w));
	map(0x0015, 0x0015).r(FUNC(hd6301x_cpu_device::p5_data_r));
	map(0x0016, 0x0016).rw(FUNC(hd6301x_cpu_device::ff_r), FUNC(hd6301x_cpu_device::p6_ddr_w));
	map(0x0017, 0x0017).rw(FUNC(hd6301x_cpu_device::p6_data_r), FUNC(hd6301x_cpu_device::p6_data_w));
	map(0x0018, 0x0018).rw(FUNC(hd6301x_cpu_device::p7_data_r), FUNC(hd6301x_cpu_device::p7_data_w)); // TODO: external except in single-chip mode
	map(0x0019, 0x0019).rw(FUNC(hd6301x_cpu_device::ocr2h_r), FUNC(hd6301x_cpu_device::ocr2h_w));
	map(0x001a, 0x001a).rw(FUNC(hd6301x_cpu_device::ocr2l_r), FUNC(hd6301x_cpu_device::ocr2l_w));
	map(0x001b, 0x001b).rw(FUNC(hd6301x_cpu_device::tcsr3_r), FUNC(hd6301x_cpu_device::tcsr3_w));
	map(0x001c, 0x001c).rw(FUNC(hd6301x_cpu_device::ff_r), FUNC(hd6301x_cpu_device::tconr_w));
	map(0x001d, 0x001d).rw(FUNC(hd6301x_cpu_device::t2cnt_r), FUNC(hd6301x_cpu_device::t2cnt_w));
	//map(0x001f, 0x001f).rw(FUNC(hd6301x_cpu_device::tstreg_r), FUNC(hd6301x_cpu_device::tstreg_w));
}

void hd6301y_cpu_device::hd6301y_io(address_map &map)
{
	hd6301x_io(map);
	map(0x0000, 0x0000).rw(FUNC(hd6301y_cpu_device::ff_r), FUNC(hd6301y_cpu_device::p1_ddr_w)); // TODO: external except in single-chip mode
	map(0x0001, 0x0001).w(FUNC(hd6301y_cpu_device::p2_ddr_w));
	map(0x0015, 0x0015).w(FUNC(hd6301y_cpu_device::p5_data_w));
	//map(0x001e, 0x001e).rw(FUNC(hd6301y_cpu_device::sci_trcsr2_r), FUNC(hd6301y_cpu_device::sci_trcsr2_w));
	map(0x0020, 0x0020).rw(FUNC(hd6301y_cpu_device::ff_r), FUNC(hd6301y_cpu_device::p5_ddr_w));
	map(0x0021, 0x0021).rw(FUNC(hd6301y_cpu_device::p6_csr_r), FUNC(hd6301y_cpu_device::p6_csr_w));
}


DEFINE_DEVICE_TYPE(M6801, m6801_cpu_device, "m6801", "Motorola MC6801")
DEFINE_DEVICE_TYPE(M6803, m6803_cpu_device, "m6803", "Motorola MC6803")
DEFINE_DEVICE_TYPE(M6803E, m6803e_cpu_device, "m6803e", "Motorola MC6803E")
DEFINE_DEVICE_TYPE(HD6301V1, hd6301v1_cpu_device, "hd6301v1", "Hitachi HD6301V1")
DEFINE_DEVICE_TYPE(HD6301X0, hd6301x0_cpu_device, "hd6301x0", "Hitachi HD6301X0")
DEFINE_DEVICE_TYPE(HD6301Y0, hd6301y0_cpu_device, "hd6301y0", "Hitachi HD6301Y0")
DEFINE_DEVICE_TYPE(HD63701V0, hd63701v0_cpu_device, "hd63701v0", "Hitachi HD63701V0")
DEFINE_DEVICE_TYPE(HD63701X0, hd63701x0_cpu_device, "hd63701x0", "Hitachi HD63701X0")
DEFINE_DEVICE_TYPE(HD63701Y0, hd63701y0_cpu_device, "hd63701y0", "Hitachi HD63701Y0")
DEFINE_DEVICE_TYPE(HD6303R, hd6303r_cpu_device, "hd6303r", "Hitachi HD6303R")
DEFINE_DEVICE_TYPE(HD6303X, hd6303x_cpu_device, "hd6303x", "Hitachi HD6303X")
DEFINE_DEVICE_TYPE(HD6303Y, hd6303y_cpu_device, "hd6303y", "Hitachi HD6303Y")

m6801_cpu_device::m6801_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6801_cpu_device(mconfig, M6801, tag, owner, clock, m6803_insn, cycles_6803, address_map_constructor())
{
}

m6801_cpu_device::m6801_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const op_func *insn, const uint8_t *cycles, address_map_constructor internal)
	: m6800_cpu_device(mconfig, type, tag, owner, clock, insn, cycles, internal)
	, m_in_port_func(*this)
	, m_out_port_func(*this)
	, m_out_sc2_func(*this)
	, m_out_sertx_func(*this)
	, m_sclk_divider(8)
{
}

m6803_cpu_device::m6803_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6801_cpu_device(mconfig, M6803, tag, owner, clock, m6803_insn, cycles_6803, address_map_constructor(FUNC(m6803_cpu_device::m6803_mem), this))
{
}

m6803e_cpu_device::m6803e_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6801_cpu_device(mconfig, M6803E, tag, owner, clock, m6803_insn, cycles_6803, address_map_constructor(FUNC(m6803e_cpu_device::m6803_mem), this))
{
}

hd6301_cpu_device::hd6301_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: m6801_cpu_device(mconfig, type, tag, owner, clock, hd63701_insn, cycles_63701)
{
}

hd6301v1_cpu_device::hd6301v1_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301_cpu_device(mconfig, HD6301V1, tag, owner, clock)
{
}

hd63701v0_cpu_device::hd63701v0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301_cpu_device(mconfig, HD63701V0, tag, owner, clock)
{
}

hd6303r_cpu_device::hd6303r_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301_cpu_device(mconfig, HD6303R, tag, owner, clock)
{
}

hd6301x_cpu_device::hd6301x_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: hd6301_cpu_device(mconfig, type, tag, owner, clock)
	, m_in_portx_func(*this)
	, m_out_portx_func(*this)
{
	m_sclk_divider = 16;
}

hd6301x0_cpu_device::hd6301x0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301x_cpu_device(mconfig, HD6301X0, tag, owner, clock)
{
}

hd63701x0_cpu_device::hd63701x0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301x_cpu_device(mconfig, HD63701X0, tag, owner, clock)
{
}

hd6303x_cpu_device::hd6303x_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301x_cpu_device(mconfig, HD6303X, tag, owner, clock)
{
}

hd6301y_cpu_device::hd6301y_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: hd6301x_cpu_device(mconfig, type, tag, owner, clock)
{
}

hd6301y0_cpu_device::hd6301y0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301y_cpu_device(mconfig, HD6301Y0, tag, owner, clock)
{
}

hd63701y0_cpu_device::hd63701y0_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301y_cpu_device(mconfig, HD63701Y0, tag, owner, clock)
{
}

hd6303y_cpu_device::hd6303y_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6301y_cpu_device(mconfig, HD6303Y, tag, owner, clock)
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
		TAKE_SCI;
	}
}

void hd6301x_cpu_device::m6800_check_irq2()
{
	if ((m_tcsr & (TCSR_EICI|TCSR_ICF)) == (TCSR_EICI|TCSR_ICF))
	{
		TAKE_ICI;
		standard_irq_callback(M6801_TIN_LINE);
	}
	else if ((m_tcsr & (TCSR_EOCI|TCSR_OCF)) == (TCSR_EOCI|TCSR_OCF) ||
				(m_tcsr2 & (TCSR2_EOCI2|TCSR2_OCF2)) == (TCSR2_EOCI2|TCSR2_OCF2))
	{
		TAKE_OCI;
	}
	else if ((m_tcsr & (TCSR_ETOI|TCSR_TOF)) == (TCSR_ETOI|TCSR_TOF))
	{
		TAKE_TOI;
	}
	else if ((m_tcsr3 & 0xc0) == 0xc0)
	{
		TAKE_CMI;
	}
	else if (((m_trcsr & (M6801_TRCSR_RIE|M6801_TRCSR_RDRF)) == (M6801_TRCSR_RIE|M6801_TRCSR_RDRF)) ||
				((m_trcsr & (M6801_TRCSR_RIE|M6801_TRCSR_ORFE)) == (M6801_TRCSR_RIE|M6801_TRCSR_ORFE)) ||
				((m_trcsr & (M6801_TRCSR_TIE|M6801_TRCSR_TDRE)) == (M6801_TRCSR_TIE|M6801_TRCSR_TDRE)))
	{
		TAKE_SCI;
	}
}

void hd6301y_cpu_device::m6800_check_irq2()
{
	if ((m_p6csr & 0xc0) == 0xc0)
	{
		TAKE_ISI;
		standard_irq_callback(M6801_IS_LINE);
	}
	else
		hd6301x_cpu_device::m6800_check_irq2();
}

void m6801_cpu_device::modified_tcsr()
{
	m_irq2 = (m_tcsr&(m_tcsr<<3))&(TCSR_ICF|TCSR_OCF|TCSR_TOF);
}

void hd6301x_cpu_device::modified_tcsr()
{
	m6801_cpu_device::modified_tcsr();
	if ((m_tcsr2 & TCSR2_EOCI2) && (m_tcsr2 & TCSR2_OCF2))
		m_irq2 |= TCSR_OCF;
}

void m6801_cpu_device::set_timer_event()
{
	m_timer_next = (OCD - CTD < TOD - CTD) ? OCD : TOD;
}

void hd6301x_cpu_device::set_timer_event()
{
	m6801_cpu_device::set_timer_event();
	if (OC2D - CTD < OCD - CTD && OC2D - CTD < TOD - CTD)
		m_timer_next = OC2D;
}

/* when change freerunningcounter or outputcapture */
void m6801_cpu_device::modified_counters()
{
	OCH = (OC >= CT) ? CTH : CTH+1;
	set_timer_event();
}

void hd6301x_cpu_device::modified_counters()
{
	OCH = (OC >= CT) ? CTH : CTH+1;
	OC2H = (OC2 >= CT) ? CTH : CTH+1;
	set_timer_event();
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
		modified_tcsr();

		// if output on P21 is enabled, let's do it
		if (m_port_ddr[1] & 2)
		{
			m_port_data[1] &= ~2;
			m_port_data[1] |= (m_tcsr & TCSR_OLVL) << 1;
			m_port2_written = true;
			write_port2();
		}
	}
	/* TOI */
	if (CTD >= TOD)
	{
		TOH++;  // next IRQ point
#if 0
		cleanup_counters();
#endif
		m_tcsr |= TCSR_TOF;
		m_pending_tcsr |= TCSR_TOF;
		modified_tcsr();
	}

	if (m_irq2 & (TCSR_OCF | TCSR_TOF))
	{
		if (m_wai_state & M6800_SLP)
			m_wai_state &= ~M6800_SLP;
		if (!(m_cc & 0x10))
			m6800_check_irq2();
	}
	/* set next event */
	set_timer_event();
}

void hd6301x_cpu_device::check_timer_event()
{
	/* OCI */
	if (CTD >= OCD)
	{
		OCH++;  // next IRQ point
		m_tcsr |= TCSR_OCF;
		m_pending_tcsr |= TCSR_OCF;
		modified_tcsr();

		// if output on P21 is enabled, let's do it
		if (m_tcsr2 & TCSR2_OE1)
		{
			m_port_data[1] &= ~2;
			m_port_data[1] |= (m_tcsr & TCSR_OLVL) << 1;
			m_port2_written = true;
			write_port2();
		}
	}
	if (CTD >= OC2D)
	{
		OC2H++;  // next IRQ point
		m_tcsr2 |= TCSR2_OCF2;
		m_pending_tcsr2 |= TCSR2_OCF2;
		modified_tcsr();

		// if output on P25 is enabled, let's do it
		if (m_tcsr2 & TCSR2_OE2)
		{
			if (m_tcsr2 & TCSR2_OLVL2)
				m_port_data[1] |= 0x20;
			else
				m_port_data[1] &= ~0x20;
			m_port2_written = true;
			write_port2();
		}
	}
	/* TOI */
	if (CTD >= TOD)
	{
		TOH++;  // next IRQ point
#if 0
		cleanup_counters();
#endif
		m_tcsr |= TCSR_TOF;
		m_pending_tcsr |= TCSR_TOF;
		modified_tcsr();
	}

	if ((m_irq2 & (TCSR_OCF | TCSR_TOF)) || (m_tcsr3 & 0xc0) == 0xc0)
	{
		if (m_wai_state & M6800_SLP)
			m_wai_state &= ~M6800_SLP;
		if (!(m_cc & 0x10))
			m6800_check_irq2();
	}
	/* set next event */
	set_timer_event();
}

void m6801_cpu_device::increment_counter(int amount)
{
	m6800_cpu_device::increment_counter(amount);
	CTD += amount;
	if (CTD >= m_timer_next)
		check_timer_event();
}

void hd6301x_cpu_device::increment_counter(int amount)
{
	m6800_cpu_device::increment_counter(amount);
	if (m_t2cnt_written)
		m_t2cnt_written = false;
	else if (BIT(m_tcsr3, 4))
	{
		switch (m_tcsr3 & 0x03)
		{
		case 0x00:
			// Timer 2 input = E clock
			increment_t2cnt(amount);
			break;

		case 0x01:
			// Timer 2 input = E clock/8
			increment_t2cnt((amount + (CTD & 0x0007)) >> 3);
			break;

		case 0x02:
			// Timer 2 input = E clock/128
			increment_t2cnt((amount + (CTD & 0x007f)) >> 7);
			break;

		case 0x03:
			// Timer 2 input = external Tclk
			break;
		}
	}
	CTD += amount;
	if (CTD >= m_timer_next || (m_tcsr3 & 0xc0) == 0xc0)
		check_timer_event();
}

void m6801_cpu_device::eat_cycles()
{
	int cycles_to_eat = std::min(int(m_timer_next - CTD), m_icount);
	if (cycles_to_eat > 0)
		increment_counter(cycles_to_eat);
}

/* cleanup high-word of counters */
void m6801_cpu_device::cleanup_counters()
{
	OCH -= CTH;
	TOH -= CTH;
	CTH = 0;
	set_timer_event();
	if (CTD >= m_timer_next)
		check_timer_event();
}

void hd6301x_cpu_device::cleanup_counters()
{
	OC2H -= CTH;
	m6801_cpu_device::cleanup_counters();
}

void m6801_cpu_device::set_rmcr(uint8_t data)
{
	if (m_rmcr == data) return;

	m_rmcr = data;

	switch ((m_rmcr & M6801_RMCR_CC_MASK) >> 2)
	{
	case 0:
		LOGSER("SCI: Using external serial clock: false\n");
		m_sci_timer->enable(false);
		m_use_ext_serclock = false;
		break;

	case 3: // external clock
		LOGSER("SCI: Using external serial clock: true\n");
		m_use_ext_serclock = true;
		m_sci_timer->enable(false);
		break;

	case 1:
	case 2:
		{
			int divisor = M6801_RMCR_SS[m_rmcr & M6801_RMCR_SS_MASK];
			attotime period = cycles_to_attotime(divisor);
			LOGSER("SCI: Setting serial rate, Divisor: %d Hz: %d\n", divisor, period.as_hz());
			m_sci_timer->adjust(period, 0, period);
			m_use_ext_serclock = false;
		}
		break;
	}
}

void hd6301x_cpu_device::set_rmcr(uint8_t data)
{
	if (m_rmcr == data) return;

	m_rmcr = data;

	switch ((m_rmcr & 0x1c) >> 2)
	{
	case 0: // TODO: clock sync
	case 3:
	case 7: // external clock
		LOGSER("SCI: Using external serial clock: true\n");
		m_use_ext_serclock = true;
		m_sci_timer->enable(false);
		break;

	case 1:
	case 2:
	case 4: // TODO: clock sync
	case 5:
	case 6:
		if (BIT(m_rmcr, 5))
		{
			LOGSER("SCI: Using Timer 2 clock\n");
			m_sci_timer->enable(false);
		}
		else
		{
			int divisor = M6801_RMCR_SS[m_rmcr & M6801_RMCR_SS_MASK];
			attotime period = cycles_to_attotime(divisor);
			LOGSER("SCI: Setting serial rate, Divisor: %d Hz: %d\n", divisor, period.as_hz());
			m_sci_timer->adjust(period, 0, period);
		}
		m_use_ext_serclock = false;
		break;
	}
}

int m6801_cpu_device::m6800_rx()
{
	return (m_in_port_func[1]() & M6801_PORT2_IO3) >> 3;
}

void m6801_cpu_device::serial_transmit()
{
	LOGTXTICK("SCI Tx Tick presenting: %d\n", m_tx);

	if (m_trcsr & M6801_TRCSR_TE)
	{
		int old_m_tx = m_tx; // Detect line change

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

					LOGTX("SCI Transmit START Data %02x\n", m_tsr);
				}
				break;

			case M6801_SERIAL_STOP:
				// send stop bit '1'
				m_tx = 1;

				check_irq_lines();

				m_txbits = M6801_SERIAL_START;

				LOGTX("SCI Transmit STOP\n");
				break;

			default:
				// send data bit '0' or '1'
				m_tx = m_tsr & 0x01;

				// shift transmit register
				m_tsr >>= 1;

				LOGTX("SCI Tx Present Bit %u: %u\n", m_txbits, m_tx);

				m_txbits++;
				break;
			}
			break;
		}

		if (old_m_tx != m_tx) // call callback only if line has changed
		{
			m_out_sertx_func((m_tx == 1) ? ASSERT_LINE : CLEAR_LINE);
		}
		m_port2_written = true;
		write_port2();
	}
}

void m6801_cpu_device::serial_receive()
{
	LOGRXTICK("SCI Rx Tick TRCSR %02x bits %u check %02x\n", m_trcsr, m_rxbits, m_trcsr & M6801_TRCSR_RE);

	if (m_trcsr & M6801_TRCSR_RE)
	{
		if (m_trcsr & M6801_TRCSR_WU)
		{
			// wait for 10 bits of '1'
			if (m6800_rx() == 1)
			{
				m_rxbits++;

				LOGRX("SCI Received WAKE UP bit %u\n", m_rxbits);

				if (m_rxbits == 10)
				{
					LOGRX("SCI Receiver Wake Up\n");

					m_trcsr &= ~M6801_TRCSR_WU;
					m_rxbits = M6801_SERIAL_START;
				}
			}
			else
			{
				LOGRX("SCI Receiver Wake Up interrupted\n");

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

					LOGRX("SCI Received START bit\n");
				}
				break;

			case M6801_SERIAL_STOP:
				if (m6800_rx() == 1)
				{
					LOGRX("SCI Received STOP bit\n");

					if (m_trcsr & M6801_TRCSR_RDRF)
					{
						// overrun error
						m_trcsr |= M6801_TRCSR_ORFE;

						LOGRX("SCI Receive Overrun Error\n");

						check_irq_lines();
					}
					else
					{
						if (!(m_trcsr & M6801_TRCSR_ORFE))
						{
							// transfer data into receive register
							m_rdr = m_rsr;

							LOGRX("SCI Receive Data Register: %02x\n", m_rdr);

							// set RDRF flag
							m_trcsr |= M6801_TRCSR_RDRF;

							check_irq_lines();
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

					LOGRX("SCI Receive Framing Error\n");

					check_irq_lines();
				}

				m_rxbits = M6801_SERIAL_START;
				break;

			default:
				// shift receive register
				m_rsr >>= 1;

				// receive bit into register
				m_rsr |= (m6800_rx() << 7);

				LOGRX("SCI RX sampled DATA bit %u: %u\n", m_rxbits, BIT(m_rsr, 7));

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
			//edge = (state == CLEAR_LINE ) ? 2 : 0;
			if (((m_tcsr&TCSR_IEDG) ^ (state==CLEAR_LINE ? TCSR_IEDG : 0)) == 0)
				return;
			/* active edge in */
			m_tcsr |= TCSR_ICF;
			m_pending_tcsr |= TCSR_ICF;
			m_input_capture = CT;
			modified_tcsr();
		}
		break;

	default:
		m6800_cpu_device::execute_set_input(irqline, state);
		break;
	}
}

void hd6301y_cpu_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case M6801_IS_LINE:
		// interrupt at falling edge
		if (!state && m_irq_state[M6801_IS_LINE])
		{
			m_p6csr |= 0x80; // IS flag
			m_pending_isf_clear = false;
		}

		m_irq_state[M6801_IS_LINE] = state;
		break;

	default:
		m6801_cpu_device::execute_set_input(irqline, state);
		break;
	}
}



void m6801_cpu_device::device_resolve_objects()
{
	m6800_cpu_device::device_resolve_objects();

	m_in_port_func.resolve_all_safe(0xff);
	m_out_port_func.resolve_all_safe();
	m_out_sc2_func.resolve_safe();
	m_out_sertx_func.resolve_safe();
}

void hd6301x_cpu_device::device_resolve_objects()
{
	m6801_cpu_device::device_resolve_objects();

	m_in_portx_func.resolve_all_safe(0xff);
	m_out_portx_func.resolve_all_safe();
}


void m6801_cpu_device::device_start()
{
	m6800_cpu_device::device_start();

	m_sci_timer = timer_alloc(FUNC(m6801_cpu_device::sci_tick), this);

	m_irq_state[M6801_IS_LINE] = 0;
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
	save_item(NAME(m_pending_isf_clear));
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

void hd6301x_cpu_device::device_start()
{
	m6801_cpu_device::device_start();

	std::fill(std::begin(m_portx_data), std::end(m_portx_data), 0);

	save_item(NAME(m_portx_ddr));
	save_item(NAME(m_portx_data));
	save_item(NAME(m_tcsr2));
	save_item(NAME(m_pending_tcsr2));
	save_item(NAME(m_output_compare2.d));

	save_item(NAME(m_t2cnt));
	save_item(NAME(m_tconr));
	save_item(NAME(m_tcsr3));
	save_item(NAME(m_tout3));
	save_item(NAME(m_t2cnt_written));
}

void hd6301y_cpu_device::device_start()
{
	hd6301x_cpu_device::device_start();

	save_item(NAME(m_p6csr));
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
	m_pending_isf_clear = false;
	m_port2_written = false;
	m_port3_latched = 0;
	/* TODO: on reset port 2 should be read to determine the operating mode (bits 0-2) */
	m_tcsr = 0x00;
	m_pending_tcsr = 0x00;
	m_irq2 = 0;
	CTD = 0x0000;
	OCD = 0xffff;
	TOD = 0xffff;
	m_timer_next = 0xffff;
	m_ram_ctrl |= 0x78;
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

void hd6301x_cpu_device::device_reset()
{
	m6801_cpu_device::device_reset();

	m_portx_ddr[0] = 0x00;
	m_portx_ddr[1] = 0x00;

	m_tcsr2 = 0x00;
	m_pending_tcsr2 = 0x00;
	OC2D = 0xffff;

	m_t2cnt = 0x00;
	m_tconr = 0xff;
	m_tcsr3 = 0x00;
	m_tout3 = false;
	m_t2cnt_written = false;

	m_p3csr = 0; // does not have this reg
}

void hd6301y_cpu_device::device_reset()
{
	hd6301x_cpu_device::device_reset();

	m_p6csr = 7;
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
		ddr |= 0x10;
	}

	data &= 0x1f;

	m_out_port_func[1](0, data, ddr);
}

void hd6301x_cpu_device::write_port2()
{
	if (!m_port2_written) return;

	uint8_t ddr = m_port_ddr[1];
	if (m_tcsr2 & TCSR2_OE1)
		ddr |= 0x02;
	if (m_tcsr2 & TCSR2_OE2)
		ddr |= 0x20;

	uint8_t data = (m_port_data[1] & ddr) | (ddr ^ 0xff);

	if (m_trcsr & M6801_TRCSR_TE)
	{
		data = (data & 0xef) | (m_tx << 4);
		ddr |= 0x10;
	}
	if ((m_tcsr3 & 0x0c) != 0)
	{
		data = (data & 0xbf) | (m_tout3 << 6);
		ddr |= 0x40;
	}

	m_out_port_func[1](0, data, ddr);
}

/*
    if change_pc() directed these areas, call hd63701_trap_pc().
    'mode' is selected by the sense of p2.0,p2.1,and p2.3 at reset timing.
    mode 0,1,2,4,6 : $0000-$001f
    mode 5         : $0000-$001f,$0200-$efff
    mode 7         : $0000-$001f,$0100-$efff
*/

void m6801_cpu_device::set_os3(int state)
{
	LOG("OS3: %u\n", state);

	m_out_sc2_func(state);
}


void m6801_cpu_device::p1_ddr_w(uint8_t data)
{
	LOGPORT("Port 1 Data Direction Register: %02x\n", data);

	if (m_port_ddr[0] != data)
	{
		m_port_ddr[0] = data;
		m_out_port_func[0](0, (m_port_data[0] & m_port_ddr[0]) | (m_port_ddr[0] ^ 0xff), m_port_ddr[0]);
	}
}

uint8_t m6801_cpu_device::p1_data_r()
{
	if (m_port_ddr[0] == 0xff)
		return m_port_data[0];
	else
		return (m_in_port_func[0]() & (m_port_ddr[0] ^ 0xff)) | (m_port_data[0] & m_port_ddr[0]);
}

void m6801_cpu_device::p1_data_w(uint8_t data)
{
	LOGPORT("Port 1 Data Register: %02x\n", data);

	m_port_data[0] = data;
	m_out_port_func[0](0, (m_port_data[0] & m_port_ddr[0]) | (m_port_ddr[0] ^ 0xff), m_port_ddr[0]);
}

void m6801_cpu_device::p2_ddr_w(uint8_t data)
{
	LOGPORT("Port 2 Data Direction Register: %02x\n", data);

	if (m_port_ddr[1] != data)
	{
		m_port_ddr[1] = data;
		write_port2();
	}
}

// HD6301X0/HD63701X0/HD6303X only
void hd6301x_cpu_device::p2_ddr_2bit_w(uint8_t data)
{
	LOGPORT("Port 2 Data Direction Register: %02x\n", data);

	data = (BIT(data, 1) ? 0xfe : 0x00) | (data & 0x01);
	if (m_port_ddr[1] != data)
	{
		m_port_ddr[1] = data;
		write_port2();
	}
}

uint8_t m6801_cpu_device::p2_data_r()
{
	if (m_port_ddr[1] == 0xff)
		return m_port_data[1];
	else
		return (m_in_port_func[1]() & (m_port_ddr[1] ^ 0xff)) | (m_port_data[1] & m_port_ddr[1]);
}

void m6801_cpu_device::p2_data_w(uint8_t data)
{
	LOGPORT("Port 2 Data Register: %02x\n", data);

	m_port_data[1] = data;
	m_port2_written = true;
	write_port2();
}

void m6801_cpu_device::p3_ddr_w(uint8_t data)
{
	LOGPORT("Port 3 Data Direction Register: %02x\n", data);

	if (m_port_ddr[2] != data)
	{
		m_port_ddr[2] = data;
		m_out_port_func[2](0, (m_port_data[2] & m_port_ddr[2]) | (m_port_ddr[2] ^ 0xff), m_port_ddr[2]);
	}
}

uint8_t m6801_cpu_device::p3_data_r()
{
	uint8_t data;

	if (!machine().side_effects_disabled())
	{
		if (m_pending_isf_clear)
		{
			LOGPORT("Cleared IS3\n");
			m_p3csr &= ~M6801_P3CSR_IS3_FLAG;
			m_pending_isf_clear = false;
		}

		if (!(m_p3csr & M6801_P3CSR_OSS))
		{
			set_os3(ASSERT_LINE);
		}
	}

	if ((m_p3csr & M6801_P3CSR_LE) || (m_port_ddr[2] == 0xff))
		data = m_port_data[2];
	else
		data = (m_in_port_func[2]() & (m_port_ddr[2] ^ 0xff)) | (m_port_data[2] & m_port_ddr[2]);

	if (!machine().side_effects_disabled())
	{
		m_port3_latched = 0;

		if (!(m_p3csr & M6801_P3CSR_OSS))
		{
			set_os3(CLEAR_LINE);
		}
	}
	return data;
}

void m6801_cpu_device::p3_data_w(uint8_t data)
{
	LOGPORT("Port 3 Data Register: %02x\n", data);

	if (m_pending_isf_clear)
	{
		LOGPORT("Cleared IS3\n");
		m_p3csr &= ~M6801_P3CSR_IS3_FLAG;
		m_pending_isf_clear = false;
	}

	if (m_p3csr & M6801_P3CSR_OSS)
	{
		set_os3(ASSERT_LINE);
	}

	m_port_data[2] = data;
	m_out_port_func[2](0, (m_port_data[2] & m_port_ddr[2]) | (m_port_ddr[2] ^ 0xff), m_port_ddr[2]);

	if (m_p3csr & M6801_P3CSR_OSS)
	{
		set_os3(CLEAR_LINE);
	}
}

uint8_t hd6301x_cpu_device::p3_data_r()
{
	// no handshaking protocol
	if (m_port_ddr[2] == 0xff)
		return m_port_data[2];
	else
		return (m_in_port_func[2]() & (m_port_ddr[2] ^ 0xff)) | (m_port_data[2] & m_port_ddr[2]);
}

void hd6301x_cpu_device::p3_data_w(uint8_t data)
{
	// no handshaking protocol
	LOGPORT("Port 3 Data Register: %02x\n", data);

	m_port_data[2] = data;
	m_out_port_func[2](0, (m_port_data[2] & m_port_ddr[2]) | (m_port_ddr[2] ^ 0xff), m_port_ddr[2]);
}

uint8_t m6801_cpu_device::p3_csr_r()
{
	if ((m_p3csr & M6801_P3CSR_IS3_FLAG) && !machine().side_effects_disabled())
	{
		m_pending_isf_clear = true;
	}

	return m_p3csr;
}

void m6801_cpu_device::p3_csr_w(uint8_t data)
{
	LOGPORT("Port 3 Control and Status Register: %02x\n", data);

	m_p3csr = data;
}

void m6801_cpu_device::p4_ddr_w(uint8_t data)
{
	LOGPORT("Port 4 Data Direction Register: %02x\n", data);

	if (m_port_ddr[3] != data)
	{
		m_port_ddr[3] = data;
		m_out_port_func[3](0, (m_port_data[3] & m_port_ddr[3]) | (m_port_ddr[3] ^ 0xff), m_port_ddr[3]);
	}
}

uint8_t m6801_cpu_device::p4_data_r()
{
	if (m_port_ddr[3] == 0xff)
		return m_port_data[3];
	else
		return (m_in_port_func[3]() & (m_port_ddr[3] ^ 0xff)) | (m_port_data[3] & m_port_ddr[3]);
}

void m6801_cpu_device::p4_data_w(uint8_t data)
{
	LOGPORT("Port 4 Data Register: %02x\n", data);

	m_port_data[3] = data;
	m_out_port_func[3](0, (m_port_data[3] & m_port_ddr[3]) | (m_port_ddr[3] ^ 0xff), m_port_ddr[3]);
}

void hd6301y_cpu_device::p5_ddr_w(uint8_t data)
{
	LOGPORT("Port 5 Data Direction Register: %02x\n", data);

	if (m_portx_ddr[0] != data)
	{
		m_portx_ddr[0] = data;
		m_out_portx_func[0](0, (m_portx_data[0] & m_portx_ddr[0]) | (m_portx_ddr[0] ^ 0xff), m_portx_ddr[0]);
	}
}

uint8_t hd6301x_cpu_device::p5_data_r()
{
	// read-only
	return m_in_portx_func[0]();
}

uint8_t hd6301y_cpu_device::p5_data_r()
{
	if (m_portx_ddr[0] == 0xff)
		return m_portx_data[0];
	else
		return ((m_in_portx_func[0]() | ((m_irq_state[M6801_IS_LINE]) ? 0x10 : 0)) & (m_portx_ddr[0] ^ 0xff)) | (m_portx_data[0] & m_portx_ddr[0]);
}

void hd6301y_cpu_device::p5_data_w(uint8_t data)
{
	LOGPORT("Port 5 Data Register: %02x\n", data);

	m_portx_data[0] = data;
	m_out_portx_func[0](0, (m_portx_data[0] & m_portx_ddr[0]) | (m_portx_ddr[0] ^ 0xff), m_portx_ddr[0]);
}

void hd6301x_cpu_device::p6_ddr_w(uint8_t data)
{
	LOGPORT("Port 6 Data Direction Register: %02x\n", data);

	if (m_portx_ddr[1] != data)
	{
		m_portx_ddr[1] = data;
		m_out_portx_func[1](0, (m_portx_data[1] & m_portx_ddr[1]) | (m_portx_ddr[1] ^ 0xff), m_portx_ddr[1]);
	}
}

uint8_t hd6301x_cpu_device::p6_data_r()
{
	if (m_portx_ddr[1] == 0xff)
		return m_portx_data[1];
	else
		return (m_in_portx_func[1]() & (m_portx_ddr[1] ^ 0xff)) | (m_portx_data[1] & m_portx_ddr[1]);
}

void hd6301x_cpu_device::p6_data_w(uint8_t data)
{
	LOGPORT("Port 6 Data Register: %02x\n", data);

	m_portx_data[1] = data;
	m_out_portx_func[1](0, (m_portx_data[1] & m_portx_ddr[1]) | (m_portx_ddr[1] ^ 0xff), m_portx_ddr[1]);
}

uint8_t hd6301y_cpu_device::p6_data_r()
{
	if (!machine().side_effects_disabled())
		clear_pending_isf();

	return hd6301x_cpu_device::p6_data_r();
}

void hd6301y_cpu_device::p6_data_w(uint8_t data)
{
	clear_pending_isf();

	hd6301x_cpu_device::p6_data_w(data);
}

void hd6301y_cpu_device::clear_pending_isf()
{
	// IS flag is cleared when reading/writing P6 after reading P6 CSR
	if (m_pending_isf_clear)
	{
		m_p6csr &= 0x7f;
		m_pending_isf_clear = false;
	}
}

uint8_t hd6301y_cpu_device::p6_csr_r()
{
	if (!machine().side_effects_disabled())
		m_pending_isf_clear = true;

	return m_p6csr | 7;
}

void hd6301y_cpu_device::p6_csr_w(uint8_t data)
{
	LOGPORT("Port 6 Control/Status Register: %02x\n", data);

	m_p6csr = (m_p6csr & 0x80) | (data & 0x7f);
	if (!(m_cc & 0x10) && data & 0x40)
		m6800_check_irq2();
}

uint8_t hd6301x_cpu_device::p7_data_r()
{
	return 0xe0 | m_portx_data[2];
}

void hd6301x_cpu_device::p7_data_w(uint8_t data)
{
	data &= 0x1f;

	LOGPORT("Port 7 Data Register: %02x\n", data);

	m_portx_data[2] = data;
	m_out_portx_func[2](0, m_portx_data[2], 0x1f);
}

uint8_t m6801_cpu_device::tcsr_r()
{
	if (!machine().side_effects_disabled())
		m_pending_tcsr = 0;
	return m_tcsr;
}

void m6801_cpu_device::tcsr_w(uint8_t data)
{
	data &= 0x1f;

	LOGTIMER("Timer Control and Status Register: %02x\n", data);

	m_tcsr = data | (m_tcsr & 0xe0);
	m_pending_tcsr &= m_tcsr;
	modified_tcsr();
	if (!(m_cc & 0x10))
		m6800_check_irq2();
}

uint8_t m6801_cpu_device::ch_r()
{
	if (!(m_pending_tcsr & TCSR_TOF) && !machine().side_effects_disabled())
	{
		m_tcsr &= ~TCSR_TOF;
		modified_tcsr();
	}
	return m_counter.b.h;
}

uint8_t m6801_cpu_device::cl_r()
{
	return m_counter.b.l;
}

void m6801_cpu_device::ch_w(uint8_t data)
{
	LOGTIMER("Counter High Register: %02x\n", data);

	m_latch09 = data & 0xff;    /* 6301 only */
	CT  = 0xfff8;
	TOH = CTH;
	modified_counters();
}

void m6801_cpu_device::cl_w(uint8_t data)
{
	LOGTIMER("Counter Low Register: %02x\n", data);

	CT = (m_latch09 << 8) | (data & 0xff);
	TOH = CTH;
	modified_counters();
}

uint8_t m6801_cpu_device::ocrh_r()
{
	return m_output_compare.b.h;
}

uint8_t m6801_cpu_device::ocrl_r()
{
	return m_output_compare.b.l;
}

void m6801_cpu_device::ocrh_w(uint8_t data)
{
	LOGTIMER("Output Compare High Register: %02x\n", data);

	if (!(m_pending_tcsr & TCSR_OCF))
	{
		m_tcsr &= ~TCSR_OCF;
		modified_tcsr();
	}

	if (m_output_compare.b.h != data)
	{
		m_output_compare.b.h = data;
		modified_counters();
	}
}

void m6801_cpu_device::ocrl_w(uint8_t data)
{
	LOGTIMER("Output Compare Low Register: %02x\n", data);

	if (!(m_pending_tcsr & TCSR_OCF))
	{
		m_tcsr &= ~TCSR_OCF;
		modified_tcsr();
	}

	if (m_output_compare.b.l != data)
	{
		m_output_compare.b.l = data;
		modified_counters();
	}
}

uint8_t m6801_cpu_device::icrh_r()
{
	if (!(m_pending_tcsr & TCSR_ICF) && !machine().side_effects_disabled())
	{
		m_tcsr &= ~TCSR_ICF;
		modified_tcsr();
	}
	return (m_input_capture >> 0) & 0xff;
}

uint8_t m6801_cpu_device::icrl_r()
{
	return (m_input_capture >> 8) & 0xff;
}

uint8_t hd6301x_cpu_device::tcsr2_r()
{
	if (!machine().side_effects_disabled())
	{
		m_pending_tcsr &= ~(TCSR_ICF | TCSR_OCF);
		m_pending_tcsr2 = 0;
	}
	return m_tcsr2 | (m_tcsr & (TCSR_ICF | TCSR_OCF)) | 0x10;
}

void hd6301x_cpu_device::tcsr2_w(uint8_t data)
{
	LOGTIMER("Timer Control and Status Register 2: %02x\n", data);

	data &= TCSR2_OE1 | TCSR2_OE2 | TCSR2_OLVL2 | TCSR2_EOCI2;
	m_tcsr2 = data | (m_tcsr2 & TCSR2_OCF2);
	m_pending_tcsr2 &= m_tcsr2;
	modified_tcsr();
	if (!(m_cc & 0x10))
		m6800_check_irq2();
}

uint8_t hd6301x_cpu_device::ocr2h_r()
{
	return m_output_compare2.b.h;
}

uint8_t hd6301x_cpu_device::ocr2l_r()
{
	return m_output_compare2.b.l;
}

void hd6301x_cpu_device::ocr2h_w(uint8_t data)
{
	LOGTIMER("Output Compare High Register 2: %02x\n", data);

	if (!(m_pending_tcsr2 & TCSR2_OCF2))
	{
		m_tcsr2 &= ~TCSR2_OCF2;
		modified_tcsr();
	}

	if (m_output_compare2.b.h != data)
	{
		m_output_compare2.b.h = data;
		modified_counters();
	}
}

void hd6301x_cpu_device::ocr2l_w(uint8_t data)
{
	LOGTIMER("Output Compare Low Register 2: %02x\n", data);

	if (!(m_pending_tcsr2 & TCSR2_OCF2))
	{
		m_tcsr2 &= ~TCSR2_OCF2;
		modified_tcsr();
	}

	if (m_output_compare2.b.l != data)
	{
		m_output_compare2.b.l = data;
		modified_counters();
	}
}


void hd6301x_cpu_device::increment_t2cnt(int amount)
{
	if (amount > uint8_t(m_tconr - m_t2cnt))
	{
		if (m_t2cnt > m_tconr)
		{
			amount -= 256 - m_t2cnt;
			m_t2cnt = 0;
		}
		m_t2cnt = (m_t2cnt + amount) % (m_tconr + 1);

		if (BIT(m_tcsr3, 3))
		{
			if (m_tout3 != BIT(m_tcsr3, 2))
			{
				m_tout3 = BIT(m_tcsr3, 2);
				m_port2_written = true;
				write_port2();
			}
		}
		else if (BIT(m_tcsr3, 2))
		{
			m_tout3 = !m_tout3;
			m_port2_written = true;
			write_port2();
		}

		if (BIT(m_rmcr, 5) && !m_use_ext_serclock)
		{
			if (m_ext_serclock + amount >= 32)
			{
				m_ext_serclock = (m_ext_serclock + amount) % 32;
				serial_transmit();
				serial_receive();
			}
			else
				m_ext_serclock += amount;
		}

		m_tcsr3 |= 0x80;
		m_timer_next = 0; // HACK
	}
	else
		m_t2cnt += amount;
}

uint8_t hd6301x_cpu_device::t2cnt_r()
{
	return m_t2cnt;
}

void hd6301x_cpu_device::t2cnt_w(uint8_t data)
{
	m_t2cnt = data;
	m_t2cnt_written = true;
}

void hd6301x_cpu_device::tconr_w(uint8_t data)
{
	m_tconr = data;
}

uint8_t hd6301x_cpu_device::tcsr3_r()
{
	return m_tcsr3;
}

void hd6301x_cpu_device::tcsr3_w(uint8_t data)
{
	uint8_t tout3_last_enable = (m_tcsr3 & 0x0c) != 0;

	// Bit 5 does not exist and Bit 7 can only be written with 0
	m_tcsr3 = data & (0x5f | (m_tcsr3 & 0x80));

	if (m_tout3 && !BIT(data, 4))
	{
		m_tout3 = false;
		write_port2();
	}
	else if (tout3_last_enable ? (data & 0x0c) == 0 : (data & 0x0c) != 0)
	{
		m_port2_written = true;
		write_port2();
	}
}


uint8_t m6801_cpu_device::sci_rmcr_r()
{
	return m_rmcr;
}

void m6801_cpu_device::sci_rmcr_w(uint8_t data)
{
	LOGSER("SCI Rate and Mode Control Register: %02x\n", data);

	set_rmcr(data);
}

uint8_t m6801_cpu_device::sci_trcsr_r()
{
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

	return m_trcsr;
}

void m6801_cpu_device::sci_trcsr_w(uint8_t data)
{
	LOGSER("SCI Transmit/Receive Control and Status Register: %02x\n", data);

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
}

uint8_t m6801_cpu_device::sci_rdr_r()
{
	if (!machine().side_effects_disabled())
	{
		if (m_trcsr_read_orfe)
		{
			LOGSER("Cleared ORFE\n");
			m_trcsr_read_orfe = 0;
			m_trcsr &= ~M6801_TRCSR_ORFE;
		}

		if (m_trcsr_read_rdrf)
		{
			LOGSER("Cleared RDRF\n");
			m_trcsr_read_rdrf = 0;
			m_trcsr &= ~M6801_TRCSR_RDRF;
		}
	}

	return m_rdr;
}

void m6801_cpu_device::sci_tdr_w(uint8_t data)
{
	LOGSER("SCI Transmit Data Register: $%02x/%d\n", data, data);

	if (m_trcsr_read_tdre)
	{
		m_trcsr_read_tdre = 0;
		m_trcsr &= ~M6801_TRCSR_TDRE;
	}
	m_tdr = data;
}


uint8_t m6801_cpu_device::rcr_r()
{
	return m_ram_ctrl;
}

void m6801_cpu_device::rcr_w(uint8_t data)
{
	LOG("RAM Control Register: %02x\n", data);

	m_ram_ctrl = data;
}

uint8_t m6801_cpu_device::ff_r()
{
	return 0xff;
}


void m6801_cpu_device::m6801_clock_serial()
{
	if (m_use_ext_serclock)
	{
		m_ext_serclock++;

		if (m_ext_serclock >= m_sclk_divider)
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

std::unique_ptr<util::disasm_interface> m6803e_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6803);
}

std::unique_ptr<util::disasm_interface> hd6301_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6301);
}

void hd6301_cpu_device::take_trap()
{
	enter_interrupt("take TRAP\n",0xffee);
}
