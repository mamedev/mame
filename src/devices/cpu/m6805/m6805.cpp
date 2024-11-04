// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/*** m6805: Portable 6805 emulator ******************************************

    m6805.c (Also supports hd68705 and hd63705 variants)

    References:

        6809 Simulator V09, By L.C. Benschop, Eindhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    uint16_t must be 16 bit unsigned int
                            uint8_t must be 8 bit unsigned int
                            uint32_t must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

  Additional Notes:

  K.Wilkins 18/03/99 - Added 63705 functonality and modified all CPU functions
                       necessary to support:
                           Variable width address bus
                           Different stack pointer
                           Alternate boot vectors
                           Alternate interrups vectors


*****************************************************************************/

#include "emu.h"
#include "m6805.h"
#include "m6805defs.h"
#include "6805dasm.h"

#include <algorithm>

#define OP(name)        (&m6805_base_device::name<big>)
#define OPN(name,n)     (&m6805_base_device::name<big, n>)
#define OP_T(name)      (&m6805_base_device::name<big, true>)
#define OP_F(name)      (&m6805_base_device::name<big, false>)
#define OP_IM(name)     (&m6805_base_device::name<big, addr_mode::IM>)
#define OP_DI(name)     (&m6805_base_device::name<big, addr_mode::DI>)
#define OP_EX(name)     (&m6805_base_device::name<big, addr_mode::EX>)
#define OP_IX(name)     (&m6805_base_device::name<big, addr_mode::IX>)
#define OP_IX1(name)    (&m6805_base_device::name<big, addr_mode::IX1>)
#define OP_IX2(name)    (&m6805_base_device::name<big, addr_mode::IX2>)

#define big false

const m6805_base_device::op_handler_table m6805_base_device::s_hmos_s_ops =
{
	/*      0/8          1/9          2/A          3/B          4/C          5/D          6/E          7/F */
	/* 0 */ OPN(brset,0),OPN(brclr,0),OPN(brset,1),OPN(brclr,1),OPN(brset,2),OPN(brclr,2),OPN(brset,3),OPN(brclr,3),
			OPN(brset,4),OPN(brclr,4),OPN(brset,5),OPN(brclr,5),OPN(brset,6),OPN(brclr,6),OPN(brset,7),OPN(brclr,7),
	/* 1 */ OPN(bset,0), OPN(bclr,0), OPN(bset,1), OPN(bclr,1), OPN(bset,2), OPN(bclr,2), OPN(bset,3), OPN(bclr,3),
			OPN(bset,4), OPN(bclr,4), OPN(bset,5), OPN(bclr,5), OPN(bset,6), OPN(bclr,6), OPN(bset,7), OPN(bclr,7),
	/* 2 */ OP_T(bra),   OP_F(bra),   OP_T(bhi),   OP_F(bhi),   OP_T(bcc),   OP_F(bcc),   OP_T(bne),   OP_F(bne),
			OP_T(bhcc),  OP_F(bhcc),  OP_T(bpl),   OP_F(bpl),   OP_T(bmc),   OP_F(bmc),   OP_T(bil),   OP_F(bil),
	/* 3 */ OP_DI(neg),  OP(illegal), OP(illegal), OP_DI(com),  OP_DI(lsr),  OP(illegal), OP_DI(ror),  OP_DI(asr),
			OP_DI(lsl),  OP_DI(rol),  OP_DI(dec),  OP(illegal), OP_DI(inc),  OP_DI(tst),  OP(illegal), OP_DI(clr),
	/* 4 */ OP(nega),    OP(illegal), OP(illegal), OP(coma),    OP(lsra),    OP(illegal), OP(rora),    OP(asra),
			OP(lsla),    OP(rola),    OP(deca),    OP(illegal), OP(inca),    OP(tsta),    OP(illegal), OP(clra),
	/* 5 */ OP(negx),    OP(illegal), OP(illegal), OP(comx),    OP(lsrx),    OP(illegal), OP(rorx),    OP(asrx),
			OP(lslx),    OP(rolx),    OP(decx),    OP(illegal), OP(incx),    OP(tstx),    OP(illegal), OP(clrx),
	/* 6 */ OP_IX1(neg), OP(illegal), OP(illegal), OP_IX1(com), OP_IX1(lsr), OP(illegal), OP_IX1(ror), OP_IX1(asr),
			OP_IX1(lsl), OP_IX1(rol), OP_IX1(dec), OP(illegal), OP_IX1(inc), OP_IX1(tst), OP(illegal), OP_IX1(clr),
	/* 7 */ OP_IX(neg),  OP(illegal), OP(illegal), OP_IX(com),  OP_IX(lsr),  OP(illegal), OP_IX(ror),  OP_IX(asr),
			OP_IX(lsl),  OP_IX(rol),  OP_IX(dec),  OP(illegal), OP_IX(inc),  OP_IX(tst),  OP(illegal), OP_IX(clr),
	/* 8 */ OP(rti),     OP(rts),     OP(illegal), OP(swi),     OP(illegal), OP(illegal), OP(illegal), OP(illegal),
			OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal),
	/* 9 */ OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(tax),
			OP(clc),     OP(sec),     OP(cli),     OP(sei),     OP(rsp),     OP(nop),     OP(illegal), OP(txa),
	/* A */ OP_IM(suba), OP_IM(cmpa), OP_IM(sbca), OP_IM(cpx),  OP_IM(anda), OP_IM(bita), OP_IM(lda),  OP(illegal),
			OP_IM(eora), OP_IM(adca), OP_IM(ora),  OP_IM(adda), OP(illegal), OP(bsr),     OP_IM(ldx),  OP(illegal),
	/* B */ OP_DI(suba), OP_DI(cmpa), OP_DI(sbca), OP_DI(cpx),  OP_DI(anda), OP_DI(bita), OP_DI(lda),  OP_DI(sta),
			OP_DI(eora), OP_DI(adca), OP_DI(ora),  OP_DI(adda), OP_DI(jmp),  OP_DI(jsr),  OP_DI(ldx),  OP_DI(stx),
	/* C */ OP_EX(suba), OP_EX(cmpa), OP_EX(sbca), OP_EX(cpx),  OP_EX(anda), OP_EX(bita), OP_EX(lda),  OP_EX(sta),
			OP_EX(eora), OP_EX(adca), OP_EX(ora),  OP_EX(adda), OP_EX(jmp),  OP_EX(jsr),  OP_EX(ldx),  OP_EX(stx),
	/* D */ OP_IX2(suba),OP_IX2(cmpa),OP_IX2(sbca),OP_IX2(cpx), OP_IX2(anda),OP_IX2(bita),OP_IX2(lda), OP_IX2(sta),
			OP_IX2(eora),OP_IX2(adca),OP_IX2(ora), OP_IX2(adda),OP_IX2(jmp), OP_IX2(jsr), OP_IX2(ldx), OP_IX2(stx),
	/* E */ OP_IX1(suba),OP_IX1(cmpa),OP_IX1(sbca),OP_IX1(cpx), OP_IX1(anda),OP_IX1(bita),OP_IX1(lda), OP_IX1(sta),
			OP_IX1(eora),OP_IX1(adca),OP_IX1(ora), OP_IX1(adda),OP_IX1(jmp), OP_IX1(jsr), OP_IX1(ldx), OP_IX1(stx),
	/* F */ OP_IX(suba), OP_IX(cmpa), OP_IX(sbca), OP_IX(cpx),  OP_IX(anda), OP_IX(bita), OP_IX(lda),  OP_IX(sta),
			OP_IX(eora), OP_IX(adca), OP_IX(ora),  OP_IX(adda), OP_IX(jmp),  OP_IX(jsr),  OP_IX(ldx),  OP_IX(stx)
};

const m6805_base_device::op_handler_table m6805_base_device::s_hc_s_ops =
{
	/*      0/8          1/9          2/A          3/B          4/C          5/D          6/E          7/F */
	/* 0 */ OPN(brset,0),OPN(brclr,0),OPN(brset,1),OPN(brclr,1),OPN(brset,2),OPN(brclr,2),OPN(brset,3),OPN(brclr,3),
			OPN(brset,4),OPN(brclr,4),OPN(brset,5),OPN(brclr,5),OPN(brset,6),OPN(brclr,6),OPN(brset,7),OPN(brclr,7),
	/* 1 */ OPN(bset,0), OPN(bclr,0), OPN(bset,1), OPN(bclr,1), OPN(bset,2), OPN(bclr,2), OPN(bset,3), OPN(bclr,3),
			OPN(bset,4), OPN(bclr,4), OPN(bset,5), OPN(bclr,5), OPN(bset,6), OPN(bclr,6), OPN(bset,7), OPN(bclr,7),
	/* 2 */ OP_T(bra),   OP_F(bra),   OP_T(bhi),   OP_F(bhi),   OP_T(bcc),   OP_F(bcc),   OP_T(bne),   OP_F(bne),
			OP_T(bhcc),  OP_F(bhcc),  OP_T(bpl),   OP_F(bpl),   OP_T(bmc),   OP_F(bmc),   OP_T(bil),   OP_F(bil),
	/* 3 */ OP_DI(neg),  OP(illegal), OP(illegal), OP_DI(com),  OP_DI(lsr),  OP(illegal), OP_DI(ror),  OP_DI(asr),
			OP_DI(lsl),  OP_DI(rol),  OP_DI(dec),  OP(illegal), OP_DI(inc),  OP_DI(tst),  OP(illegal), OP_DI(clr),
	/* 4 */ OP(nega),    OP(illegal), OP(mul),     OP(coma),    OP(lsra),    OP(illegal), OP(rora),    OP(asra),
			OP(lsla),    OP(rola),    OP(deca),    OP(illegal), OP(inca),    OP(tsta),    OP(illegal), OP(clra),
	/* 5 */ OP(negx),    OP(illegal), OP(illegal), OP(comx),    OP(lsrx),    OP(illegal), OP(rorx),    OP(asrx),
			OP(lslx),    OP(rolx),    OP(decx),    OP(illegal), OP(incx),    OP(tstx),    OP(illegal), OP(clrx),
	/* 6 */ OP_IX1(neg), OP(illegal), OP(illegal), OP_IX1(com), OP_IX1(lsr), OP(illegal), OP_IX1(ror), OP_IX1(asr),
			OP_IX1(lsl), OP_IX1(rol), OP_IX1(dec), OP(illegal), OP_IX1(inc), OP_IX1(tst), OP(illegal), OP_IX1(clr),
	/* 7 */ OP_IX(neg),  OP(illegal), OP(illegal), OP_IX(com),  OP_IX(lsr),  OP(illegal), OP_IX(ror),  OP_IX(asr),
			OP_IX(lsl),  OP_IX(rol),  OP_IX(dec),  OP(illegal), OP_IX(inc),  OP_IX(tst),  OP(illegal), OP_IX(clr),
	/* 8 */ OP(rti),     OP(rts),     OP(illegal), OP(swi),     OP(illegal), OP(illegal), OP(illegal), OP(illegal),
			OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(stop),    OP(wait),
	/* 9 */ OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(tax),
			OP(clc),     OP(sec),     OP(cli),     OP(sei),     OP(rsp),     OP(nop),     OP(illegal), OP(txa),
	/* A */ OP_IM(suba), OP_IM(cmpa), OP_IM(sbca), OP_IM(cpx),  OP_IM(anda), OP_IM(bita), OP_IM(lda),  OP(illegal),
			OP_IM(eora), OP_IM(adca), OP_IM(ora),  OP_IM(adda), OP(illegal), OP(bsr),     OP_IM(ldx),  OP(illegal),
	/* B */ OP_DI(suba), OP_DI(cmpa), OP_DI(sbca), OP_DI(cpx),  OP_DI(anda), OP_DI(bita), OP_DI(lda),  OP_DI(sta),
			OP_DI(eora), OP_DI(adca), OP_DI(ora),  OP_DI(adda), OP_DI(jmp),  OP_DI(jsr),  OP_DI(ldx),  OP_DI(stx),
	/* C */ OP_EX(suba), OP_EX(cmpa), OP_EX(sbca), OP_EX(cpx),  OP_EX(anda), OP_EX(bita), OP_EX(lda),  OP_EX(sta),
			OP_EX(eora), OP_EX(adca), OP_EX(ora),  OP_EX(adda), OP_EX(jmp),  OP_EX(jsr),  OP_EX(ldx),  OP_EX(stx),
	/* D */ OP_IX2(suba),OP_IX2(cmpa),OP_IX2(sbca),OP_IX2(cpx), OP_IX2(anda),OP_IX2(bita),OP_IX2(lda), OP_IX2(sta),
			OP_IX2(eora),OP_IX2(adca),OP_IX2(ora), OP_IX2(adda),OP_IX2(jmp), OP_IX2(jsr), OP_IX2(ldx), OP_IX2(stx),
	/* E */ OP_IX1(suba),OP_IX1(cmpa),OP_IX1(sbca),OP_IX1(cpx), OP_IX1(anda),OP_IX1(bita),OP_IX1(lda), OP_IX1(sta),
			OP_IX1(eora),OP_IX1(adca),OP_IX1(ora), OP_IX1(adda),OP_IX1(jmp), OP_IX1(jsr), OP_IX1(ldx), OP_IX1(stx),
	/* F */ OP_IX(suba), OP_IX(cmpa), OP_IX(sbca), OP_IX(cpx),  OP_IX(anda), OP_IX(bita), OP_IX(lda),  OP_IX(sta),
			OP_IX(eora), OP_IX(adca), OP_IX(ora),  OP_IX(adda), OP_IX(jmp),  OP_IX(jsr),  OP_IX(ldx),  OP_IX(stx)
};

#undef big
#define big true

const m6805_base_device::op_handler_table m6805_base_device::s_hmos_b_ops =
{
	/*      0/8          1/9          2/A          3/B          4/C          5/D          6/E          7/F */
	/* 0 */ OPN(brset,0),OPN(brclr,0),OPN(brset,1),OPN(brclr,1),OPN(brset,2),OPN(brclr,2),OPN(brset,3),OPN(brclr,3),
			OPN(brset,4),OPN(brclr,4),OPN(brset,5),OPN(brclr,5),OPN(brset,6),OPN(brclr,6),OPN(brset,7),OPN(brclr,7),
	/* 1 */ OPN(bset,0), OPN(bclr,0), OPN(bset,1), OPN(bclr,1), OPN(bset,2), OPN(bclr,2), OPN(bset,3), OPN(bclr,3),
			OPN(bset,4), OPN(bclr,4), OPN(bset,5), OPN(bclr,5), OPN(bset,6), OPN(bclr,6), OPN(bset,7), OPN(bclr,7),
	/* 2 */ OP_T(bra),   OP_F(bra),   OP_T(bhi),   OP_F(bhi),   OP_T(bcc),   OP_F(bcc),   OP_T(bne),   OP_F(bne),
			OP_T(bhcc),  OP_F(bhcc),  OP_T(bpl),   OP_F(bpl),   OP_T(bmc),   OP_F(bmc),   OP_T(bil),   OP_F(bil),
	/* 3 */ OP_DI(neg),  OP(illegal), OP(illegal), OP_DI(com),  OP_DI(lsr),  OP(illegal), OP_DI(ror),  OP_DI(asr),
			OP_DI(lsl),  OP_DI(rol),  OP_DI(dec),  OP(illegal), OP_DI(inc),  OP_DI(tst),  OP(illegal), OP_DI(clr),
	/* 4 */ OP(nega),    OP(illegal), OP(illegal), OP(coma),    OP(lsra),    OP(illegal), OP(rora),    OP(asra),
			OP(lsla),    OP(rola),    OP(deca),    OP(illegal), OP(inca),    OP(tsta),    OP(illegal), OP(clra),
	/* 5 */ OP(negx),    OP(illegal), OP(illegal), OP(comx),    OP(lsrx),    OP(illegal), OP(rorx),    OP(asrx),
			OP(lslx),    OP(rolx),    OP(decx),    OP(illegal), OP(incx),    OP(tstx),    OP(illegal), OP(clrx),
	/* 6 */ OP_IX1(neg), OP(illegal), OP(illegal), OP_IX1(com), OP_IX1(lsr), OP(illegal), OP_IX1(ror), OP_IX1(asr),
			OP_IX1(lsl), OP_IX1(rol), OP_IX1(dec), OP(illegal), OP_IX1(inc), OP_IX1(tst), OP(illegal), OP_IX1(clr),
	/* 7 */ OP_IX(neg),  OP(illegal), OP(illegal), OP_IX(com),  OP_IX(lsr),  OP(illegal), OP_IX(ror),  OP_IX(asr),
			OP_IX(lsl),  OP_IX(rol),  OP_IX(dec),  OP(illegal), OP_IX(inc),  OP_IX(tst),  OP(illegal), OP_IX(clr),
	/* 8 */ OP(rti),     OP(rts),     OP(illegal), OP(swi),     OP(illegal), OP(illegal), OP(illegal), OP(illegal),
			OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal),
	/* 9 */ OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(tax),
			OP(clc),     OP(sec),     OP(cli),     OP(sei),     OP(rsp),     OP(nop),     OP(illegal), OP(txa),
	/* A */ OP_IM(suba), OP_IM(cmpa), OP_IM(sbca), OP_IM(cpx),  OP_IM(anda), OP_IM(bita), OP_IM(lda),  OP(illegal),
			OP_IM(eora), OP_IM(adca), OP_IM(ora),  OP_IM(adda), OP(illegal), OP(bsr),     OP_IM(ldx),  OP(illegal),
	/* B */ OP_DI(suba), OP_DI(cmpa), OP_DI(sbca), OP_DI(cpx),  OP_DI(anda), OP_DI(bita), OP_DI(lda),  OP_DI(sta),
			OP_DI(eora), OP_DI(adca), OP_DI(ora),  OP_DI(adda), OP_DI(jmp),  OP_DI(jsr),  OP_DI(ldx),  OP_DI(stx),
	/* C */ OP_EX(suba), OP_EX(cmpa), OP_EX(sbca), OP_EX(cpx),  OP_EX(anda), OP_EX(bita), OP_EX(lda),  OP_EX(sta),
			OP_EX(eora), OP_EX(adca), OP_EX(ora),  OP_EX(adda), OP_EX(jmp),  OP_EX(jsr),  OP_EX(ldx),  OP_EX(stx),
	/* D */ OP_IX2(suba),OP_IX2(cmpa),OP_IX2(sbca),OP_IX2(cpx), OP_IX2(anda),OP_IX2(bita),OP_IX2(lda), OP_IX2(sta),
			OP_IX2(eora),OP_IX2(adca),OP_IX2(ora), OP_IX2(adda),OP_IX2(jmp), OP_IX2(jsr), OP_IX2(ldx), OP_IX2(stx),
	/* E */ OP_IX1(suba),OP_IX1(cmpa),OP_IX1(sbca),OP_IX1(cpx), OP_IX1(anda),OP_IX1(bita),OP_IX1(lda), OP_IX1(sta),
			OP_IX1(eora),OP_IX1(adca),OP_IX1(ora), OP_IX1(adda),OP_IX1(jmp), OP_IX1(jsr), OP_IX1(ldx), OP_IX1(stx),
	/* F */ OP_IX(suba), OP_IX(cmpa), OP_IX(sbca), OP_IX(cpx),  OP_IX(anda), OP_IX(bita), OP_IX(lda),  OP_IX(sta),
			OP_IX(eora), OP_IX(adca), OP_IX(ora),  OP_IX(adda), OP_IX(jmp),  OP_IX(jsr),  OP_IX(ldx),  OP_IX(stx)
};

const m6805_base_device::op_handler_table m6805_base_device::s_cmos_b_ops =
{
	/*      0/8          1/9          2/A          3/B          4/C          5/D          6/E          7/F */
	/* 0 */ OPN(brset,0),OPN(brclr,0),OPN(brset,1),OPN(brclr,1),OPN(brset,2),OPN(brclr,2),OPN(brset,3),OPN(brclr,3),
			OPN(brset,4),OPN(brclr,4),OPN(brset,5),OPN(brclr,5),OPN(brset,6),OPN(brclr,6),OPN(brset,7),OPN(brclr,7),
	/* 1 */ OPN(bset,0), OPN(bclr,0), OPN(bset,1), OPN(bclr,1), OPN(bset,2), OPN(bclr,2), OPN(bset,3), OPN(bclr,3),
			OPN(bset,4), OPN(bclr,4), OPN(bset,5), OPN(bclr,5), OPN(bset,6), OPN(bclr,6), OPN(bset,7), OPN(bclr,7),
	/* 2 */ OP_T(bra),   OP_F(bra),   OP_T(bhi),   OP_F(bhi),   OP_T(bcc),   OP_F(bcc),   OP_T(bne),   OP_F(bne),
			OP_T(bhcc),  OP_F(bhcc),  OP_T(bpl),   OP_F(bpl),   OP_T(bmc),   OP_F(bmc),   OP_T(bil),   OP_F(bil),
	/* 3 */ OP_DI(neg),  OP(illegal), OP(illegal), OP_DI(com),  OP_DI(lsr),  OP(illegal), OP_DI(ror),  OP_DI(asr),
			OP_DI(lsl),  OP_DI(rol),  OP_DI(dec),  OP(illegal), OP_DI(inc),  OP_DI(tst),  OP(illegal), OP_DI(clr),
	/* 4 */ OP(nega),    OP(illegal), OP(illegal), OP(coma),    OP(lsra),    OP(illegal), OP(rora),    OP(asra),
			OP(lsla),    OP(rola),    OP(deca),    OP(illegal), OP(inca),    OP(tsta),    OP(illegal), OP(clra),
	/* 5 */ OP(negx),    OP(illegal), OP(illegal), OP(comx),    OP(lsrx),    OP(illegal), OP(rorx),    OP(asrx),
			OP(lslx),    OP(rolx),    OP(decx),    OP(illegal), OP(incx),    OP(tstx),    OP(illegal), OP(clrx),
	/* 6 */ OP_IX1(neg), OP(illegal), OP(illegal), OP_IX1(com), OP_IX1(lsr), OP(illegal), OP_IX1(ror), OP_IX1(asr),
			OP_IX1(lsl), OP_IX1(rol), OP_IX1(dec), OP(illegal), OP_IX1(inc), OP_IX1(tst), OP(illegal), OP_IX1(clr),
	/* 7 */ OP_IX(neg),  OP(illegal), OP(illegal), OP_IX(com),  OP_IX(lsr),  OP(illegal), OP_IX(ror),  OP_IX(asr),
			OP_IX(lsl),  OP_IX(rol),  OP_IX(dec),  OP(illegal), OP_IX(inc),  OP_IX(tst),  OP(illegal), OP_IX(clr),
	/* 8 */ OP(rti),     OP(rts),     OP(illegal), OP(swi),     OP(illegal), OP(illegal), OP(illegal), OP(illegal),
			OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(stop),    OP(wait),
	/* 9 */ OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(tax),
			OP(clc),     OP(sec),     OP(cli),     OP(sei),     OP(rsp),     OP(nop),     OP(illegal), OP(txa),
	/* A */ OP_IM(suba), OP_IM(cmpa), OP_IM(sbca), OP_IM(cpx),  OP_IM(anda), OP_IM(bita), OP_IM(lda),  OP(illegal),
			OP_IM(eora), OP_IM(adca), OP_IM(ora),  OP_IM(adda), OP(illegal), OP(bsr),     OP_IM(ldx),  OP(illegal),
	/* B */ OP_DI(suba), OP_DI(cmpa), OP_DI(sbca), OP_DI(cpx),  OP_DI(anda), OP_DI(bita), OP_DI(lda),  OP_DI(sta),
			OP_DI(eora), OP_DI(adca), OP_DI(ora),  OP_DI(adda), OP_DI(jmp),  OP_DI(jsr),  OP_DI(ldx),  OP_DI(stx),
	/* C */ OP_EX(suba), OP_EX(cmpa), OP_EX(sbca), OP_EX(cpx),  OP_EX(anda), OP_EX(bita), OP_EX(lda),  OP_EX(sta),
			OP_EX(eora), OP_EX(adca), OP_EX(ora),  OP_EX(adda), OP_EX(jmp),  OP_EX(jsr),  OP_EX(ldx),  OP_EX(stx),
	/* D */ OP_IX2(suba),OP_IX2(cmpa),OP_IX2(sbca),OP_IX2(cpx), OP_IX2(anda),OP_IX2(bita),OP_IX2(lda), OP_IX2(sta),
			OP_IX2(eora),OP_IX2(adca),OP_IX2(ora), OP_IX2(adda),OP_IX2(jmp), OP_IX2(jsr), OP_IX2(ldx), OP_IX2(stx),
	/* E */ OP_IX1(suba),OP_IX1(cmpa),OP_IX1(sbca),OP_IX1(cpx), OP_IX1(anda),OP_IX1(bita),OP_IX1(lda), OP_IX1(sta),
			OP_IX1(eora),OP_IX1(adca),OP_IX1(ora), OP_IX1(adda),OP_IX1(jmp), OP_IX1(jsr), OP_IX1(ldx), OP_IX1(stx),
	/* F */ OP_IX(suba), OP_IX(cmpa), OP_IX(sbca), OP_IX(cpx),  OP_IX(anda), OP_IX(bita), OP_IX(lda),  OP_IX(sta),
			OP_IX(eora), OP_IX(adca), OP_IX(ora),  OP_IX(adda), OP_IX(jmp),  OP_IX(jsr),  OP_IX(ldx),  OP_IX(stx)
};

const m6805_base_device::op_handler_table m6805_base_device::s_hc_b_ops =
{
	/*      0/8          1/9          2/A          3/B          4/C          5/D          6/E          7/F */
	/* 0 */ OPN(brset,0),OPN(brclr,0),OPN(brset,1),OPN(brclr,1),OPN(brset,2),OPN(brclr,2),OPN(brset,3),OPN(brclr,3),
			OPN(brset,4),OPN(brclr,4),OPN(brset,5),OPN(brclr,5),OPN(brset,6),OPN(brclr,6),OPN(brset,7),OPN(brclr,7),
	/* 1 */ OPN(bset,0), OPN(bclr,0), OPN(bset,1), OPN(bclr,1), OPN(bset,2), OPN(bclr,2), OPN(bset,3), OPN(bclr,3),
			OPN(bset,4), OPN(bclr,4), OPN(bset,5), OPN(bclr,5), OPN(bset,6), OPN(bclr,6), OPN(bset,7), OPN(bclr,7),
	/* 2 */ OP_T(bra),   OP_F(bra),   OP_T(bhi),   OP_F(bhi),   OP_T(bcc),   OP_F(bcc),   OP_T(bne),   OP_F(bne),
			OP_T(bhcc),  OP_F(bhcc),  OP_T(bpl),   OP_F(bpl),   OP_T(bmc),   OP_F(bmc),   OP_T(bil),   OP_F(bil),
	/* 3 */ OP_DI(neg),  OP(illegal), OP(illegal), OP_DI(com),  OP_DI(lsr),  OP(illegal), OP_DI(ror),  OP_DI(asr),
			OP_DI(lsl),  OP_DI(rol),  OP_DI(dec),  OP(illegal), OP_DI(inc),  OP_DI(tst),  OP(illegal), OP_DI(clr),
	/* 4 */ OP(nega),    OP(illegal), OP(mul),     OP(coma),    OP(lsra),    OP(illegal), OP(rora),    OP(asra),
			OP(lsla),    OP(rola),    OP(deca),    OP(illegal), OP(inca),    OP(tsta),    OP(illegal), OP(clra),
	/* 5 */ OP(negx),    OP(illegal), OP(illegal), OP(comx),    OP(lsrx),    OP(illegal), OP(rorx),    OP(asrx),
			OP(lslx),    OP(rolx),    OP(decx),    OP(illegal), OP(incx),    OP(tstx),    OP(illegal), OP(clrx),
	/* 6 */ OP_IX1(neg), OP(illegal), OP(illegal), OP_IX1(com), OP_IX1(lsr), OP(illegal), OP_IX1(ror), OP_IX1(asr),
			OP_IX1(lsl), OP_IX1(rol), OP_IX1(dec), OP(illegal), OP_IX1(inc), OP_IX1(tst), OP(illegal), OP_IX1(clr),
	/* 7 */ OP_IX(neg),  OP(illegal), OP(illegal), OP_IX(com),  OP_IX(lsr),  OP(illegal), OP_IX(ror),  OP_IX(asr),
			OP_IX(lsl),  OP_IX(rol),  OP_IX(dec),  OP(illegal), OP_IX(inc),  OP_IX(tst),  OP(illegal), OP_IX(clr),
	/* 8 */ OP(rti),     OP(rts),     OP(illegal), OP(swi),     OP(illegal), OP(illegal), OP(illegal), OP(illegal),
			OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(stop),    OP(wait),
	/* 9 */ OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(illegal), OP(tax),
			OP(clc),     OP(sec),     OP(cli),     OP(sei),     OP(rsp),     OP(nop),     OP(illegal), OP(txa),
	/* A */ OP_IM(suba), OP_IM(cmpa), OP_IM(sbca), OP_IM(cpx),  OP_IM(anda), OP_IM(bita), OP_IM(lda),  OP(illegal),
			OP_IM(eora), OP_IM(adca), OP_IM(ora),  OP_IM(adda), OP(illegal), OP(bsr),     OP_IM(ldx),  OP(illegal),
	/* B */ OP_DI(suba), OP_DI(cmpa), OP_DI(sbca), OP_DI(cpx),  OP_DI(anda), OP_DI(bita), OP_DI(lda),  OP_DI(sta),
			OP_DI(eora), OP_DI(adca), OP_DI(ora),  OP_DI(adda), OP_DI(jmp),  OP_DI(jsr),  OP_DI(ldx),  OP_DI(stx),
	/* C */ OP_EX(suba), OP_EX(cmpa), OP_EX(sbca), OP_EX(cpx),  OP_EX(anda), OP_EX(bita), OP_EX(lda),  OP_EX(sta),
			OP_EX(eora), OP_EX(adca), OP_EX(ora),  OP_EX(adda), OP_EX(jmp),  OP_EX(jsr),  OP_EX(ldx),  OP_EX(stx),
	/* D */ OP_IX2(suba),OP_IX2(cmpa),OP_IX2(sbca),OP_IX2(cpx), OP_IX2(anda),OP_IX2(bita),OP_IX2(lda), OP_IX2(sta),
			OP_IX2(eora),OP_IX2(adca),OP_IX2(ora), OP_IX2(adda),OP_IX2(jmp), OP_IX2(jsr), OP_IX2(ldx), OP_IX2(stx),
	/* E */ OP_IX1(suba),OP_IX1(cmpa),OP_IX1(sbca),OP_IX1(cpx), OP_IX1(anda),OP_IX1(bita),OP_IX1(lda), OP_IX1(sta),
			OP_IX1(eora),OP_IX1(adca),OP_IX1(ora), OP_IX1(adda),OP_IX1(jmp), OP_IX1(jsr), OP_IX1(ldx), OP_IX1(stx),
	/* F */ OP_IX(suba), OP_IX(cmpa), OP_IX(sbca), OP_IX(cpx),  OP_IX(anda), OP_IX(bita), OP_IX(lda),  OP_IX(sta),
			OP_IX(eora), OP_IX(adca), OP_IX(ora),  OP_IX(adda), OP_IX(jmp),  OP_IX(jsr),  OP_IX(ldx),  OP_IX(stx)
};

#undef big

const m6805_base_device::cycle_count_table m6805_base_device::s_hmos_cycles =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	/*1*/  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	/*2*/  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	/*3*/  6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 0, 6, 6, 0, 6,
	/*4*/  4, 0, 0, 4, 4, 0, 4, 4, 4, 4, 4, 0, 4, 4, 0, 4,
	/*5*/  4, 0, 0, 4, 4, 0, 4, 4, 4, 4, 4, 0, 4, 4, 0, 4,
	/*6*/  7, 0, 0, 7, 7, 0, 7, 7, 7, 7, 7, 0, 7, 7, 0, 7,
	/*7*/  6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 0, 6, 6, 0, 6,
	/*8*/  9, 6, 0,11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*9*/  0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 2,
	/*A*/  2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 0, 8, 2, 0,
	/*B*/  4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 7, 4, 5,
	/*C*/  5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 4, 8, 5, 6,
	/*D*/  6, 6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 5, 9, 6, 7,
	/*E*/  5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 4, 8, 5, 6,
	/*F*/  4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 7, 4, 5
};

const m6805_base_device::cycle_count_table m6805_base_device::s_cmos_cycles =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	/*1*/  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	/*2*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/*3*/  5, 0, 0, 5, 5, 0, 5, 5, 5, 5, 5, 0, 5, 4, 0, 5,
	/*4*/  3, 0, 0, 3, 3, 0, 3, 3, 3, 3, 3, 0, 3, 3, 0, 3,
	/*5*/  3, 0, 0, 3, 3, 0, 3, 3, 3, 3, 3, 0, 3, 3, 0, 3,
	/*6*/  6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 0, 6, 5, 0, 6,
	/*7*/  5, 0, 0, 5, 5, 0, 5, 5, 5, 5, 5, 0, 5, 4, 0, 5,
	/*8*/  9, 6, 0,10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
	/*9*/  0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 2,
	/*A*/  2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 0, 6, 2, 0,
	/*B*/  3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 2, 5, 3, 4,
	/*C*/  4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 6, 4, 5,
	/*D*/  5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 4, 7, 5, 6,
	/*E*/  4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 6, 4, 5,
	/*F*/  3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 2, 5, 3, 4
};

const m6805_base_device::cycle_count_table m6805_base_device::s_hc_cycles =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	/*1*/  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	/*2*/  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/*3*/  5, 0, 0, 5, 5, 0, 5, 5, 5, 5, 5, 0, 5, 4, 0, 5,
	/*4*/  3, 0,11, 3, 3, 0, 3, 3, 3, 3, 3, 0, 3, 3, 0, 3,
	/*5*/  3, 0, 0, 3, 3, 0, 3, 3, 3, 3, 3, 0, 3, 3, 0, 3,
	/*6*/  6, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 0, 6, 5, 0, 6,
	/*7*/  5, 0, 0, 5, 5, 0, 5, 5, 5, 5, 5, 0, 5, 4, 0, 5,
	/*8*/  9, 6, 0,10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
	/*9*/  0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 2,
	/*A*/  2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 0, 6, 2, 0,
	/*B*/  3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 2, 5, 3, 4,
	/*C*/  4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 6, 4, 5,
	/*D*/  5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 4, 7, 5, 6,
	/*E*/  4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 3, 6, 4, 5,
	/*F*/  3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 2, 5, 3, 4
};


//-------------------------------------------------
//  m6809_base_device - constructor
//-------------------------------------------------

m6805_base_device::m6805_base_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock,
		device_type const type,
		configuration_params const &params)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_params(params)
	, m_program_config("program", ENDIANNESS_BIG, 8, params.m_addr_width)
{
}

m6805_base_device::m6805_base_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock,
		device_type const type,
		configuration_params const &params,
		address_map_constructor internal_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_params(params)
	, m_program_config("program", ENDIANNESS_BIG, 8, params.m_addr_width, 0, internal_map)
{
}



void m6805_base_device::device_start()
{
	if (m_params.m_addr_width > 14) {
		space(AS_PROGRAM).cache(m_cprogram16);
		space(AS_PROGRAM).specific(m_program16);
	} else {
		space(AS_PROGRAM).cache(m_cprogram13);
		space(AS_PROGRAM).specific(m_program13);
	}

	// get the minimum not including the zero placeholders for illegal instructions
	m_min_cycles = *std::min_element(
			std::begin(m_params.m_cycles),
			std::end(m_params.m_cycles),
			[] (u8 x, u8 y) { return u8(x - 1) < u8(y - 1); });
	m_max_cycles = *std::max_element(std::begin(m_params.m_cycles), std::end(m_params.m_cycles));

	// set our instruction counter
	set_icountptr(m_icount);

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_pc.w.l).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc.w.l).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_cc).callimport().callexport().formatstr("%8s").noshow();
	state_add(M6805_A,         "A",         m_a).mask(0xff);
	state_add(M6805_PC,        "PC",        m_pc.w.l).mask(0xffff);
	state_add(M6805_S,         "S",         m_s.w.l).mask(0xff);
	state_add(M6805_X,         "X",         m_x).mask(0xff);
	state_add(M6805_CC,        "CC",        m_cc).mask(0xff);

	// register for savestates
	save_item(NAME(EA));
	save_item(NAME(A));
	save_item(NAME(PC));
	save_item(NAME(S));
	save_item(NAME(X));
	save_item(NAME(CC));
	save_item(NAME(m_pending_interrupts));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_nmi_state));

	std::fill(std::begin(m_irq_state), std::end(m_irq_state), CLEAR_LINE);
}

void m6805_base_device::device_reset()
{
	m_ea.w.l = 0;
	m_pc.w.l = 0;
	m_s.w.l = SP_MASK;
	m_a = 0;
	m_x = 0;
	m_cc = 0;
	m_pending_interrupts = 0;

	m_nmi_state = 0;

	/* IRQ disabled */
	SEI;

	if (m_params.m_addr_width > 14)
		rm16<true>(0xfffe & m_params.m_vector_mask, m_pc);
	else
		rm16<false>(0xfffe & m_params.m_vector_mask, m_pc);
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector m6805_base_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void m6805_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c",
			(m_cc & 0x80) ? '?' : '.',
			(m_cc & 0x40) ? '?' : '.',
			(m_cc & 0x20) ? '?' : '.',
			(m_cc & 0x10) ? 'H' : '.',
			(m_cc & 0x08) ? 'I' : '.',
			(m_cc & 0x04) ? 'N' : '.',
			(m_cc & 0x02) ? 'Z' : '.',
			(m_cc & 0x01) ? 'C' : '.');
		break;
	}
}


bool m6805_base_device::test_il()
{
	return CLEAR_LINE != m_irq_state[M6805_IRQ_LINE];
}

void m6805_base_device::interrupt_vector()
{
	if (m_params.m_addr_width > 14)
		rm16<true>(0xfffa & m_params.m_vector_mask, m_pc);
	else
		rm16<false>(0xfffa & m_params.m_vector_mask, m_pc);
}

/* Generate interrupts */
void m6805_base_device::interrupt()
{
	/* the 6805 latches interrupt requests internally, so we don't clear */
	/* pending_interrupts until the interrupt is taken, no matter what the */
	/* external IRQ pin does. */

	if (BIT(m_pending_interrupts, HD63705_INT_NMI))
	{
		if (m_params.m_addr_width > 14) {
			pushword<true>(m_pc);
			pushbyte<true>(m_x);
			pushbyte<true>(m_a);
			pushbyte<true>(m_cc);
		}
		else
		{
			pushword<false>(m_pc);
			pushbyte<false>(m_x);
			pushbyte<false>(m_a);
			pushbyte<false>(m_cc);
		}
		SEI;
		/* no vectors supported, just do the callback to clear irq_state if needed */
		standard_irq_callback(0, m_pc.w.l);

		if (m_params.m_addr_width > 14)
			rm16<true>(0x1ffc, m_pc);
		else
			rm16<false>(0x1ffc, m_pc);
		m_pending_interrupts &= ~(1 << HD63705_INT_NMI);

		m_icount -= 11;
		burn_cycles(11);
	}
	else if ((m_pending_interrupts & ((1 << M6805_IRQ_LINE) | HD63705_INT_MASK)) != 0)
	{
		if ((CC & IFLAG) == 0)
		{
			/* standard IRQ */
			if (m_params.m_addr_width > 14) {
				pushword<true>(m_pc);
				pushbyte<true>(m_x);
				pushbyte<true>(m_a);
				pushbyte<true>(m_cc);
			}
			else
			{
				pushword<false>(m_pc);
				pushbyte<false>(m_x);
				pushbyte<false>(m_a);
				pushbyte<false>(m_cc);
			}
			SEI;
			/* no vectors supported, just do the callback to clear irq_state if needed */
			standard_irq_callback(0, m_pc.w.l);

			interrupt_vector();

			m_pending_interrupts &= ~(1 << M6805_IRQ_LINE);

			m_icount -= 11;
			burn_cycles(11);
		}
	}
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> m6805_base_device::create_disassembler()
{
	return std::make_unique<m6805_disassembler>();
}


#include "6805ops.hxx"

//-------------------------------------------------
//  execute_clocks_to_cycles - convert the raw
//  clock into cycles per second
//-------------------------------------------------

uint64_t m6805_base_device::execute_clocks_to_cycles(uint64_t clocks) const noexcept
{
	return (clocks + 3) / 4;
}


//-------------------------------------------------
//  execute_cycles_to_clocks - convert a cycle
//  count back to raw clocks
//-------------------------------------------------

uint64_t m6805_base_device::execute_cycles_to_clocks(uint64_t cycles) const noexcept
{
	return cycles * 4;
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t m6805_base_device::execute_min_cycles() const noexcept
{
	return m_min_cycles;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t m6805_base_device::execute_max_cycles() const noexcept
{
	return m_max_cycles;
}


/* execute instructions on this CPU until icount expires */
void m6805_base_device::execute_run()
{
	S = SP_ADJUST( S );     /* Taken from CPU_SET_CONTEXT when pointer'afying */

	do
	{
		if (m_pending_interrupts != 0)
		{
			interrupt();
		}

		debugger_instruction_hook(PC);

		u8 const ireg = m_params.m_addr_width > 14 ? rdop<true>(PC++) : rdop<false>(PC++);

		(this->*m_params.m_ops[ireg])();
		m_icount -= m_params.m_cycles[ireg];
		burn_cycles(m_params.m_cycles[ireg]);
	}
	while (m_icount > 0);
}

void m6805_base_device::execute_set_input(int inputnum, int state)
{
	// Basic 6805 only has one IRQ line
	// See HD63705 specific version
	if (m_irq_state[inputnum] != state)
	{
		m_irq_state[inputnum] = (ASSERT_LINE == state) ? ASSERT_LINE : CLEAR_LINE;

		if (CLEAR_LINE != state)
			m_pending_interrupts |= (1 << inputnum);
	}
}


/****************************************************************************
 * M68HC05EG section
 ****************************************************************************/
m68hc05eg_device::m68hc05eg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6805_base_device(
			mconfig,
			tag,
			owner,
			clock,
			M68HC05EG,
			{ s_hc_s_ops, s_hc_cycles, 13, 0x00ff, 0x00c0, 0xfffc })
{
}

void m68hc05eg_device::device_reset()
{
	m6805_base_device::device_reset();

	rm16<false>(0x1ffe, m_pc);
}

void m68hc05eg_device::interrupt_vector()
{
	if (BIT(m_pending_interrupts, M68HC05EG_INT_IRQ))
	{
		m_pending_interrupts &= ~(1 << M68HC05EG_INT_IRQ);
		rm16<false>(0x1ffa, m_pc);
	}
	else if (BIT(m_pending_interrupts, M68HC05EG_INT_TIMER))
	{
		m_pending_interrupts &= ~(1 << M68HC05EG_INT_TIMER);
		rm16<false>(0x1ff8, m_pc);
	}
	else if (BIT(m_pending_interrupts, M68HC05EG_INT_CPI))
	{
		m_pending_interrupts &= ~(1 << M68HC05EG_INT_CPI);
		rm16<false>(0x1ff6, m_pc);
	}
}

u64 m68hc05eg_device::execute_clocks_to_cycles(u64 clocks) const noexcept
{
	return (clocks + 1) / 2;
}

u64 m68hc05eg_device::execute_cycles_to_clocks(u64 cycles) const noexcept
{
	return cycles * 2;
}

std::unique_ptr<util::disasm_interface> m68hc05eg_device::create_disassembler()
{
	return std::make_unique<m68hc05_disassembler>();
}

/****************************************************************************
 * HD63705 section
 ****************************************************************************/
hd6305_device::hd6305_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock,
		device_type const type,
		configuration_params const &params,
		address_map_constructor internal_map)
	: m6805_base_device(mconfig, tag, owner, clock, type, params, internal_map)
{
}

hd6305v0_device::hd6305v0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6305_device(
			mconfig,
			tag,
			owner,
			clock,
			HD6305V0,
			{ s_hmos_s_ops, s_hmos_cycles, 14, 0x00ff, 0x00c0, 0x1fff, 0x1ffc },
			address_map_constructor(FUNC(hd6305v0_device::internal_map), this))
{
}

void hd6305v0_device::internal_map(address_map &map)
{
	// TODO: ports, timer, SCI
	map(0x0040, 0x00ff).ram();
	map(0x1000, 0x1fff).rom().region(DEVICE_SELF, 0);
}

hd6305y2_device::hd6305y2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6305_device(
			mconfig,
			tag,
			owner,
			clock,
			HD6305Y2,
			{ s_hmos_s_ops, s_hmos_cycles, 14, 0x00ff, 0x00c0, 0x1fff, 0x1ffc },
			address_map_constructor(FUNC(hd6305y2_device::internal_map), this))
{
}

void hd6305y2_device::internal_map(address_map &map)
{
	// TODO: ports, timer, SCI
	map(0x0040, 0x013f).ram();
}

hd63705z0_device::hd63705z0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6305_device(
			mconfig,
			tag,
			owner,
			clock,
			HD63705Z0,
			{ s_hmos_b_ops, s_hmos_cycles, 16, 0x017f, 0x0100, 0x1fff, 0x1ffa },
			address_map_constructor(FUNC(hd63705z0_device::internal_map), this))
{
}

void hd63705z0_device::internal_map(address_map &map)
{
	// TODO: ports, timer, SCI
	map(0x0040, 0x01bf).ram();
	map(0x01c0, 0x1fff).rom().region(DEVICE_SELF, 0x01c0);
}

void hd6305_device::device_reset()
{
	m6805_base_device::device_reset();
}

void hd6305_device::execute_set_input(int inputnum, int state)
{
	if (inputnum == INPUT_LINE_NMI)
	{
		if (m_nmi_state != state)
		{
			m_nmi_state = state;

			if (state != CLEAR_LINE)
			{
				m_pending_interrupts |= 1 << HD63705_INT_NMI;
			}
		}
	}
	else if (inputnum <= HD63705_INT_ADCONV)
	{
		if (m_irq_state[inputnum] != state)
		{
			m_irq_state[inputnum] = state;

			if (state != CLEAR_LINE)
			{
				m_pending_interrupts |= 1 << inputnum;
			}
		}
	}
}

void hd6305_device::interrupt_vector()
{
	/* Need to add emulation of other interrupt sources here KW-2/4/99 */
	/* This is just a quick patch for Namco System 2 operation         */

	if ((m_pending_interrupts & (1 << HD63705_INT_IRQ1)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_IRQ1);
		if (m_params.m_addr_width > 14)
			rm16<true>(0x1ff8, m_pc);
		else
			rm16<false>(0x1ff8, m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_IRQ2)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_IRQ2);
		if (m_params.m_addr_width > 14)
			rm16<true>(0x1fec, m_pc);
		else
			rm16<false>(0x1fec, m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_ADCONV)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_ADCONV);
		if (m_params.m_addr_width > 14)
			rm16<true>(0x1fea, m_pc);
		else
			rm16<false>(0x1fea, m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_TIMER1)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_TIMER1);
		if (m_params.m_addr_width > 14)
			rm16<true>(0x1ff6, m_pc);
		else
			rm16<false>(0x1ff6, m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_TIMER2)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_TIMER2);
		if (m_params.m_addr_width > 14)
			rm16<true>(0x1ff4, m_pc);
		else
			rm16<false>(0x1ff4, m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_TIMER3)) != 0)
	{
		m_pending_interrupts &= ~(1<<HD63705_INT_TIMER3);
		if (m_params.m_addr_width > 14)
			rm16<true>(0x1ff2, m_pc);
		else
			rm16<false>(0x1ff2, m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_PCI)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_PCI);
		if (m_params.m_addr_width > 14)
			rm16<true>(0x1ff0, m_pc);
		else
			rm16<false>(0x1ff0, m_pc);
	}
	else if ((m_pending_interrupts & (1 << HD63705_INT_SCI)) != 0)
	{
		m_pending_interrupts &= ~(1 << HD63705_INT_SCI);
		if (m_params.m_addr_width > 14)
			rm16<true>(0x1fee, m_pc);
		else
			rm16<false>(0x1fee, m_pc);
	}
}


DEFINE_DEVICE_TYPE(M68HC05EG, m68hc05eg_device, "m68hc05eg", "Motorola MC68HC05EG")
DEFINE_DEVICE_TYPE(HD6305V0,  hd6305v0_device,  "hd6305v0",  "Hitachi HD6305V0")
DEFINE_DEVICE_TYPE(HD6305Y2,  hd6305y2_device,  "hd6305y2",  "Hitachi HD6305Y2")
DEFINE_DEVICE_TYPE(HD63705Z0, hd63705z0_device, "hd63705z0", "Hitachi HD63705Z0")
