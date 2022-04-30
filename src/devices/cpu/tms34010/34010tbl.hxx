// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari
/*** TMS34010: Portable TMS34010 emulator ***********************************

    Copyright Alex Pasadyn/Zsolt Vasvari

    Opcode Table

*****************************************************************************/

// 34010 Opcode Table
const tms34010_device::opcode_func tms34010_device::s_opcode_table[65536 >> 4] =
{
	// 0x0000 0x0010 0x0020 0x0030 ... 0x00f0
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::rev_a,      &tms34010_device::rev_b,      &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	// 0x0100
	&tms34010_device::emu,        &tms34010_device::unimpl,     &tms34010_device::exgpc_a,    &tms34010_device::exgpc_b,    &tms34010_device::getpc_a,    &tms34010_device::getpc_b,    &tms34010_device::jump_a,     &tms34010_device::jump_b,
	&tms34010_device::getst_a,    &tms34010_device::getst_b,    &tms34010_device::putst_a,    &tms34010_device::putst_b,    &tms34010_device::popst,      &tms34010_device::unimpl,     &tms34010_device::pushst,     &tms34010_device::unimpl,
	// 0x0200
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x0300
	&tms34010_device::nop,        &tms34010_device::unimpl,     &tms34010_device::clrc,       &tms34010_device::unimpl,     &tms34010_device::movb_aa,    &tms34010_device::unimpl,     &tms34010_device::dint,       &tms34010_device::unimpl,
	&tms34010_device::abs_a,      &tms34010_device::abs_b,      &tms34010_device::neg_a,      &tms34010_device::neg_b,      &tms34010_device::negb_a,     &tms34010_device::negb_b,     &tms34010_device::not_a,      &tms34010_device::not_b,
	// 0x0400
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x0500
	&tms34010_device::sext0_a,    &tms34010_device::sext0_b,    &tms34010_device::zext0_a,    &tms34010_device::zext0_b,    &tms34010_device::setf0,      &tms34010_device::setf0,      &tms34010_device::setf0,      &tms34010_device::setf0,
	&tms34010_device::move0_ra_a, &tms34010_device::move0_ra_b, &tms34010_device::move0_ar_a, &tms34010_device::move0_ar_b, &tms34010_device::move0_aa,   &tms34010_device::unimpl,     &tms34010_device::movb_ra_a,  &tms34010_device::movb_ra_b,
	// 0x0600
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	// 0x0700
	&tms34010_device::sext1_a,    &tms34010_device::sext1_b,    &tms34010_device::zext1_a,    &tms34010_device::zext1_b,    &tms34010_device::setf1,      &tms34010_device::setf1,      &tms34010_device::setf1,      &tms34010_device::setf1,
	&tms34010_device::move1_ra_a, &tms34010_device::move1_ra_b, &tms34010_device::move1_ar_a, &tms34010_device::move1_ar_b, &tms34010_device::move1_aa,   &tms34010_device::unimpl,     &tms34010_device::movb_ar_a,  &tms34010_device::movb_ar_b,
	// 0x0800
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x0900
	&tms34010_device::trap,       &tms34010_device::trap,       &tms34010_device::call_a,     &tms34010_device::call_b,     &tms34010_device::reti,       &tms34010_device::unimpl,     &tms34010_device::rets,       &tms34010_device::rets,
	&tms34010_device::mmtm_a,     &tms34010_device::mmtm_b,     &tms34010_device::mmfm_a,     &tms34010_device::mmfm_b,     &tms34010_device::movi_w_a,   &tms34010_device::movi_w_b,   &tms34010_device::movi_l_a,   &tms34010_device::movi_l_b,
	// 0x0a00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x0b00
	&tms34010_device::addi_w_a,   &tms34010_device::addi_w_b,   &tms34010_device::addi_l_a,   &tms34010_device::addi_l_b,   &tms34010_device::cmpi_w_a,   &tms34010_device::cmpi_w_b,   &tms34010_device::cmpi_l_a,   &tms34010_device::cmpi_l_b,
	&tms34010_device::andi_a,     &tms34010_device::andi_b,     &tms34010_device::ori_a,      &tms34010_device::ori_b,      &tms34010_device::xori_a,     &tms34010_device::xori_b,     &tms34010_device::subi_w_a,   &tms34010_device::subi_w_b,
	// 0x0c00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x0d00
	&tms34010_device::subi_l_a,   &tms34010_device::subi_l_b,   &tms34010_device::unimpl,     &tms34010_device::callr,      &tms34010_device::unimpl,     &tms34010_device::calla,      &tms34010_device::eint,       &tms34010_device::unimpl,
	&tms34010_device::dsj_a,      &tms34010_device::dsj_b,      &tms34010_device::dsjeq_a,    &tms34010_device::dsjeq_b,    &tms34010_device::dsjne_a,    &tms34010_device::dsjne_b,    &tms34010_device::setc,       &tms34010_device::unimpl,
	// 0x0e00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x0f00
	&tms34010_device::pixblt_l_l, &tms34010_device::unimpl,     &tms34010_device::pixblt_l_xy,&tms34010_device::unimpl,     &tms34010_device::pixblt_xy_l,&tms34010_device::unimpl,     &tms34010_device::pixblt_xy_xy,&tms34010_device::unimpl,
	&tms34010_device::pixblt_b_l, &tms34010_device::unimpl,     &tms34010_device::pixblt_b_xy,&tms34010_device::unimpl,     &tms34010_device::fill_l,     &tms34010_device::unimpl,     &tms34010_device::fill_xy,    &tms34010_device::unimpl,
	// 0x1000
	&tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,
	&tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,
	// 0x1100
	&tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,
	&tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,
	// 0x1200
	&tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,
	&tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,
	// 0x1300
	&tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,
	&tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,     &tms34010_device::addk_a,     &tms34010_device::addk_b,
	// 0x1400
	&tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,
	&tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,
	// 0x1500
	&tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,
	&tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,
	// 0x1600
	&tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,
	&tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,
	// 0x1700
	&tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,
	&tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,     &tms34010_device::subk_a,     &tms34010_device::subk_b,
	// 0x1800
	&tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,
	&tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,
	// 0x1900
	&tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,
	&tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,
	// 0x1a00
	&tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,
	&tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,
	// 0x1b00
	&tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,
	&tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,     &tms34010_device::movk_a,     &tms34010_device::movk_b,
	// 0x1c00
	&tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,
	&tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,
	// 0x1d00
	&tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,
	&tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,
	// 0x1e00
	&tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,
	&tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,
	// 0x1f00
	&tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,
	&tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,   &tms34010_device::btst_k_a,   &tms34010_device::btst_k_b,
	// 0x2000
	&tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,
	&tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,
	// 0x2100
	&tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,
	&tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,
	// 0x2200
	&tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,
	&tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,
	// 0x2300
	&tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,
	&tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,    &tms34010_device::sla_k_a,    &tms34010_device::sla_k_b,
	// 0x2400
	&tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,
	&tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,
	// 0x2500
	&tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,
	&tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,
	// 0x2600
	&tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,
	&tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,
	// 0x2700
	&tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,
	&tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,    &tms34010_device::sll_k_a,    &tms34010_device::sll_k_b,
	// 0x2800
	&tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,
	&tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,
	// 0x2900
	&tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,
	&tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,
	// 0x2a00
	&tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,
	&tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,
	// 0x2b00
	&tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,
	&tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,    &tms34010_device::sra_k_a,    &tms34010_device::sra_k_b,
	// 0x2c00
	&tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,
	&tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,
	// 0x2d00
	&tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,
	&tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,
	// 0x2e00
	&tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,
	&tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,
	// 0x2f00
	&tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,
	&tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,    &tms34010_device::srl_k_a,    &tms34010_device::srl_k_b,
	// 0x3000
	&tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,
	&tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,
	// 0x3100
	&tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,
	&tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,
	// 0x3200
	&tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,
	&tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,
	// 0x3300
	&tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,
	&tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,     &tms34010_device::rl_k_a,     &tms34010_device::rl_k_b,
	// 0x3400
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x3500
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x3600
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x3700
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x3800
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	// 0x3900
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	// 0x3a00
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	// 0x3b00
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	// 0x3c00
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	// 0x3d00
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	// 0x3e00
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	// 0x3f00
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	&tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,     &tms34010_device::dsjs_a,     &tms34010_device::dsjs_b,
	// 0x4000
	&tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,
	&tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,
	// 0x4100
	&tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,
	&tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,      &tms34010_device::add_a,      &tms34010_device::add_b,
	// 0x4200
	&tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,
	&tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,
	// 0x4300
	&tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,
	&tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,     &tms34010_device::addc_a,     &tms34010_device::addc_b,
	// 0x4400
	&tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,
	&tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,
	// 0x4500
	&tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,
	&tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,      &tms34010_device::sub_a,      &tms34010_device::sub_b,
	// 0x4600
	&tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,
	&tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,
	// 0x4700
	&tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,
	&tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,     &tms34010_device::subb_a,     &tms34010_device::subb_b,
	// 0x4800
	&tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,
	&tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,
	// 0x4900
	&tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,
	&tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,      &tms34010_device::cmp_a,      &tms34010_device::cmp_b,
	// 0x4a00
	&tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,
	&tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,
	// 0x4b00
	&tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,
	&tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,   &tms34010_device::btst_r_a,   &tms34010_device::btst_r_b,
	// 0x4c00
	&tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,
	&tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,
	// 0x4d00
	&tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,
	&tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,  &tms34010_device::move_rr_a,  &tms34010_device::move_rr_b,
	// 0x4e00
	&tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx,
	&tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx,
	// 0x4f00
	&tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx,
	&tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx, &tms34010_device::move_rr_ax, &tms34010_device::move_rr_bx,
	// 0x5000
	&tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,
	&tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,
	// 0x5100
	&tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,
	&tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,      &tms34010_device::and_a,      &tms34010_device::and_b,
	// 0x5200
	&tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,
	&tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,
	// 0x5300
	&tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,
	&tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,     &tms34010_device::andn_a,     &tms34010_device::andn_b,
	// 0x5400
	&tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,
	&tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,
	// 0x5500
	&tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,
	&tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,       &tms34010_device::or_a,       &tms34010_device::or_b,
	// 0x5600
	&tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,
	&tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,
	// 0x5700
	&tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,
	&tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,      &tms34010_device::xor_a,      &tms34010_device::xor_b,
	// 0x5800
	&tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,
	&tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,
	// 0x5900
	&tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,
	&tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,     &tms34010_device::divs_a,     &tms34010_device::divs_b,
	// 0x5a00
	&tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,
	&tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,
	// 0x5b00
	&tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,
	&tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,     &tms34010_device::divu_a,     &tms34010_device::divu_b,
	// 0x5c00
	&tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,
	&tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,
	// 0x5d00
	&tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,
	&tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,     &tms34010_device::mpys_a,     &tms34010_device::mpys_b,
	// 0x5e00
	&tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,
	&tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,
	// 0x5f00
	&tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,
	&tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,     &tms34010_device::mpyu_a,     &tms34010_device::mpyu_b,
	// 0x6000
	&tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,
	&tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,
	// 0x6100
	&tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,
	&tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,    &tms34010_device::sla_r_a,    &tms34010_device::sla_r_b,
	// 0x6200
	&tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,
	&tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,
	// 0x6300
	&tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,
	&tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,    &tms34010_device::sll_r_a,    &tms34010_device::sll_r_b,
	// 0x6400
	&tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,
	&tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,
	// 0x6500
	&tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,
	&tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,    &tms34010_device::sra_r_a,    &tms34010_device::sra_r_b,
	// 0x6600
	&tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,
	&tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,
	// 0x6700
	&tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,
	&tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,    &tms34010_device::srl_r_a,    &tms34010_device::srl_r_b,
	// 0x6800
	&tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,
	&tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,
	// 0x6900
	&tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,
	&tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,     &tms34010_device::rl_r_a,     &tms34010_device::rl_r_b,
	// 0x6a00
	&tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,
	&tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,
	// 0x6b00
	&tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,
	&tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,      &tms34010_device::lmo_a,      &tms34010_device::lmo_b,
	// 0x6c00
	&tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,
	&tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,
	// 0x6d00
	&tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,
	&tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,     &tms34010_device::mods_a,     &tms34010_device::mods_b,
	// 0x6e00
	&tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,
	&tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,
	// 0x6f00
	&tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,
	&tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,     &tms34010_device::modu_a,     &tms34010_device::modu_b,
	// 0x7000
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7100
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7200
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7300
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7400
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7500
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7600
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7700
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7800
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7900
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7a00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7b00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7c00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7d00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7e00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x7f00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x8000
	&tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b,
	&tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b,
	// 0x8100
	&tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b,
	&tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b, &tms34010_device::move0_rn_a, &tms34010_device::move0_rn_b,
	// 0x8200
	&tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b,
	&tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b,
	// 0x8300
	&tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b,
	&tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b, &tms34010_device::move1_rn_a, &tms34010_device::move1_rn_b,
	// 0x8400
	&tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b,
	&tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b,
	// 0x8500
	&tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b,
	&tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b, &tms34010_device::move0_nr_a, &tms34010_device::move0_nr_b,
	// 0x8600
	&tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b,
	&tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b,
	// 0x8700
	&tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b,
	&tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b, &tms34010_device::move1_nr_a, &tms34010_device::move1_nr_b,
	// 0x8800
	&tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b,
	&tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b,
	// 0x8900
	&tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b,
	&tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b, &tms34010_device::move0_nn_a, &tms34010_device::move0_nn_b,
	// 0x8a00
	&tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b,
	&tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b,
	// 0x8b00
	&tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b,
	&tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b, &tms34010_device::move1_nn_a, &tms34010_device::move1_nn_b,
	// 0x8c00
	&tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,
	&tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,
	// 0x8d00
	&tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,
	&tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,  &tms34010_device::movb_rn_a,  &tms34010_device::movb_rn_b,
	// 0x8e00
	&tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,
	&tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,
	// 0x8f00
	&tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,
	&tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,  &tms34010_device::movb_nr_a,  &tms34010_device::movb_nr_b,
	// 0x9000
	&tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,
	&tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,
	// 0x9100
	&tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,
	&tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,   &tms34010_device::move0_r_ni_a,   &tms34010_device::move0_r_ni_b,
	// 0x9200
	&tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,
	&tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,
	// 0x9300
	&tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,
	&tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,   &tms34010_device::move1_r_ni_a,   &tms34010_device::move1_r_ni_b,
	// 0x9400
	&tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,
	&tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,
	// 0x9500
	&tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,
	&tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,   &tms34010_device::move0_ni_r_a,   &tms34010_device::move0_ni_r_b,
	// 0x9600
	&tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,
	&tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,
	// 0x9700
	&tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,
	&tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,   &tms34010_device::move1_ni_r_a,   &tms34010_device::move1_ni_r_b,
	// 0x9800
	&tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,
	&tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,
	// 0x9900
	&tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,
	&tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,  &tms34010_device::move0_ni_ni_a,  &tms34010_device::move0_ni_ni_b,
	// 0x9a00
	&tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,
	&tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,
	// 0x9b00
	&tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,
	&tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,  &tms34010_device::move1_ni_ni_a,  &tms34010_device::move1_ni_ni_b,
	// 0x9c00
	&tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,
	&tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,
	// 0x9d00
	&tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,
	&tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,  &tms34010_device::movb_nn_a,  &tms34010_device::movb_nn_b,
	// 0x9e00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0x9f00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xa000
	&tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,
	&tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,
	// 0xa100
	&tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,
	&tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,   &tms34010_device::move0_r_dn_a,   &tms34010_device::move0_r_dn_b,
	// 0xa200
	&tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,
	&tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,
	// 0xa300
	&tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,
	&tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,   &tms34010_device::move1_r_dn_a,   &tms34010_device::move1_r_dn_b,
	// 0xa400
	&tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,
	&tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,
	// 0xa500
	&tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,
	&tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,   &tms34010_device::move0_dn_r_a,   &tms34010_device::move0_dn_r_b,
	// 0xa600
	&tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,
	&tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,
	// 0xa700
	&tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,
	&tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,   &tms34010_device::move1_dn_r_a,   &tms34010_device::move1_dn_r_b,
	// 0xa800
	&tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,
	&tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,
	// 0xa900
	&tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,
	&tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,  &tms34010_device::move0_dn_dn_a,  &tms34010_device::move0_dn_dn_b,
	// 0xaa00
	&tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,
	&tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,
	// 0xab00
	&tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,
	&tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,  &tms34010_device::move1_dn_dn_a,  &tms34010_device::move1_dn_dn_b,
	// 0xac00
	&tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,
	&tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,
	// 0xad00
	&tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,
	&tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,    &tms34010_device::movb_r_no_a,    &tms34010_device::movb_r_no_b,
	// 0xae00
	&tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,
	&tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,
	// 0xaf00
	&tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,
	&tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,    &tms34010_device::movb_no_r_a,    &tms34010_device::movb_no_r_b,
	// 0xb000
	&tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,
	&tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,
	// 0xb100
	&tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,
	&tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,   &tms34010_device::move0_r_no_a,   &tms34010_device::move0_r_no_b,
	// 0xb200
	&tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,
	&tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,
	// 0xb300
	&tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,
	&tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,   &tms34010_device::move1_r_no_a,   &tms34010_device::move1_r_no_b,
	// 0xb400
	&tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,
	&tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,
	// 0xb500
	&tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,
	&tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,   &tms34010_device::move0_no_r_a,   &tms34010_device::move0_no_r_b,
	// 0xb600
	&tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,
	&tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,
	// 0xb700
	&tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,
	&tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,   &tms34010_device::move1_no_r_a,   &tms34010_device::move1_no_r_b,
	// 0xb800
	&tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,
	&tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,
	// 0xb900
	&tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,
	&tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,  &tms34010_device::move0_no_no_a,  &tms34010_device::move0_no_no_b,
	// 0xba00
	&tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,
	&tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,
	// 0xbb00
	&tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,
	&tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,  &tms34010_device::move1_no_no_a,  &tms34010_device::move1_no_no_b,
	// 0xbc00
	&tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,
	&tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,
	// 0xbd00
	&tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,
	&tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,   &tms34010_device::movb_no_no_a,   &tms34010_device::movb_no_no_b,
	// 0xbe00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xbf00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xc000
	&tms34010_device::j_UC_0,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,
	&tms34010_device::j_UC_8,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,     &tms34010_device::j_UC_x,
	// 0xc100
	&tms34010_device::j_P_0,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,
	&tms34010_device::j_P_8,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,      &tms34010_device::j_P_x,
	// 0xc200
	&tms34010_device::j_LS_0,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,
	&tms34010_device::j_LS_8,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,     &tms34010_device::j_LS_x,
	// 0xc300
	&tms34010_device::j_HI_0,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,
	&tms34010_device::j_HI_8,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,     &tms34010_device::j_HI_x,
	// 0xc400
	&tms34010_device::j_LT_0,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,
	&tms34010_device::j_LT_8,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,     &tms34010_device::j_LT_x,
	// 0xc500
	&tms34010_device::j_GE_0,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,
	&tms34010_device::j_GE_8,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,     &tms34010_device::j_GE_x,
	// 0xc600
	&tms34010_device::j_LE_0,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,
	&tms34010_device::j_LE_8,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,     &tms34010_device::j_LE_x,
	// 0xc700
	&tms34010_device::j_GT_0,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,
	&tms34010_device::j_GT_8,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,     &tms34010_device::j_GT_x,
	// 0xc800
	&tms34010_device::j_C_0,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,
	&tms34010_device::j_C_8,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,      &tms34010_device::j_C_x,
	// 0xc900
	&tms34010_device::j_NC_0,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,
	&tms34010_device::j_NC_8,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,     &tms34010_device::j_NC_x,
	// 0xca00
	&tms34010_device::j_EQ_0,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,
	&tms34010_device::j_EQ_8,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,     &tms34010_device::j_EQ_x,
	// 0xcb00
	&tms34010_device::j_NE_0,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,
	&tms34010_device::j_NE_8,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,     &tms34010_device::j_NE_x,
	// 0xcc00
	&tms34010_device::j_V_0,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,
	&tms34010_device::j_V_8,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,      &tms34010_device::j_V_x,
	// 0xcd00
	&tms34010_device::j_NV_0,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,
	&tms34010_device::j_NV_8,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,     &tms34010_device::j_NV_x,
	// 0xce00
	&tms34010_device::j_N_0,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,
	&tms34010_device::j_N_8,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,      &tms34010_device::j_N_x,
	// 0xcf00
	&tms34010_device::j_NN_0,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,
	&tms34010_device::j_NN_8,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,     &tms34010_device::j_NN_x,
	// 0xd000
	&tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,
	&tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,
	// 0xd100
	&tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,
	&tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,  &tms34010_device::move0_no_ni_a,  &tms34010_device::move0_no_ni_b,
	// 0xd200
	&tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,
	&tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,
	// 0xd300
	&tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,
	&tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,  &tms34010_device::move1_no_ni_a,  &tms34010_device::move1_no_ni_b,
	// 0xd400
	&tms34010_device::move0_a_ni_a,&tms34010_device::move0_a_ni_b,&tms34010_device::unimpl,   &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	// 0xd500
	&tms34010_device::exgf0_a,    &tms34010_device::exgf0_b,    &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	// 0xd600
	&tms34010_device::move1_a_ni_a,&tms34010_device::move1_a_ni_b,&tms34010_device::unimpl,   &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	// 0xd700
	&tms34010_device::exgf1_a,    &tms34010_device::exgf1_b,    &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	// 0xd800
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xd900
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xda00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xdb00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xdc00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xdd00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xde00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xdf00
	&tms34010_device::unimpl,     &tms34010_device::line,       &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	&tms34010_device::unimpl,     &tms34010_device::line,       &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	// 0xe000
	&tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,
	&tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,
	// 0xe100
	&tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,
	&tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,   &tms34010_device::add_xy_a,   &tms34010_device::add_xy_b,
	// 0xe200
	&tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,
	&tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,
	// 0xe300
	&tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,
	&tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,   &tms34010_device::sub_xy_a,   &tms34010_device::sub_xy_b,
	// 0xe400
	&tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,
	&tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,
	// 0xe500
	&tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,
	&tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,   &tms34010_device::cmp_xy_a,   &tms34010_device::cmp_xy_b,
	// 0xe600
	&tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,
	&tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,
	// 0xe700
	&tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,
	&tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,      &tms34010_device::cpw_a,      &tms34010_device::cpw_b,
	// 0xe800
	&tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,
	&tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,
	// 0xe900
	&tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,
	&tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,    &tms34010_device::cvxyl_a,    &tms34010_device::cvxyl_b,
	// 0xea00
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	// 0xeb00
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	&tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,     &tms34010_device::unimpl,
	// 0xec00
	&tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,
	&tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,
	// 0xed00
	&tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,
	&tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,     &tms34010_device::movx_a,     &tms34010_device::movx_b,
	// 0xee00
	&tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,
	&tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,
	// 0xef00
	&tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,
	&tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,     &tms34010_device::movy_a,     &tms34010_device::movy_b,
	// 0xf000
	&tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,
	&tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,
	// 0xf100
	&tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,
	&tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,    &tms34010_device::pixt_rixy_a,    &tms34010_device::pixt_rixy_b,
	// 0xf200
	&tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,
	&tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,
	// 0xf300
	&tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,
	&tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,    &tms34010_device::pixt_ixyr_a,    &tms34010_device::pixt_ixyr_b,
	// 0xf400
	&tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,
	&tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,
	// 0xf500
	&tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,
	&tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,  &tms34010_device::pixt_ixyixy_a,  &tms34010_device::pixt_ixyixy_b,
	// 0xf600
	&tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,
	&tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,
	// 0xf700
	&tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,
	&tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,     &tms34010_device::drav_a,     &tms34010_device::drav_b,
	// 0xf800
	&tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,
	&tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,
	// 0xf900
	&tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,
	&tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,  &tms34010_device::pixt_ri_a,  &tms34010_device::pixt_ri_b,
	// 0xfa00
	&tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,
	&tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,
	// 0xfb00
	&tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,
	&tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,  &tms34010_device::pixt_ir_a,  &tms34010_device::pixt_ir_b,
	// 0xfc00
	&tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,
	&tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,
	// 0xfd00
	&tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,
	&tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,  &tms34010_device::pixt_ii_a,  &tms34010_device::pixt_ii_b,
	// 0xfe00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	// 0xff00
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop,
	&tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,      &tms34010_device::illop,    &tms34010_device::illop
};

// 34020 Opcode Table
const tms34020_device::opcode_func tms34020_device::s_opcode_table[65536 >> 4] =
{
	// 0x0000 0x0010 0x0020 0x0030 ... 0x00f0
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::rev_a,      &tms34020_device::rev_b,      &tms34020_device::idle,       &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::mwait,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::blmove,
	// 0x0100
	&tms34020_device::emu,        &tms34020_device::illop,      &tms34020_device::exgpc_a,    &tms34020_device::exgpc_b,    &tms34020_device::getpc_a,    &tms34020_device::getpc_b,    &tms34020_device::jump_a,     &tms34020_device::jump_b,
	&tms34020_device::getst_a,    &tms34020_device::getst_b,    &tms34020_device::putst_a,    &tms34020_device::putst_b,    &tms34020_device::popst,      &tms34020_device::illop,      &tms34020_device::pushst,     &tms34020_device::illop,
	// 0x0200
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::setcsp,     &tms34020_device::illop,      &tms34020_device::setcdp,
	&tms34020_device::rpix_a,     &tms34020_device::rpix_b,     &tms34020_device::exgps_a,    &tms34020_device::exgps_b,    &tms34020_device::getps_a,    &tms34020_device::getps_b,    &tms34020_device::illop,      &tms34020_device::setcmp,
	// 0x0300
	&tms34020_device::nop,        &tms34020_device::illop,      &tms34020_device::clrc,       &tms34020_device::illop,      &tms34020_device::movb_aa,    &tms34020_device::illop,      &tms34020_device::dint,       &tms34020_device::illop,
	&tms34020_device::abs_a,      &tms34020_device::abs_b,      &tms34020_device::neg_a,      &tms34020_device::neg_b,      &tms34020_device::negb_a,     &tms34020_device::negb_b,     &tms34020_device::not_a,      &tms34020_device::not_b,
	// 0x0400
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x0500
	&tms34020_device::sext0_a,    &tms34020_device::sext0_b,    &tms34020_device::zext0_a,    &tms34020_device::zext0_b,    &tms34020_device::setf0,      &tms34020_device::setf0,      &tms34020_device::setf0,      &tms34020_device::setf0,
	&tms34020_device::move0_ra_a, &tms34020_device::move0_ra_b, &tms34020_device::move0_ar_a, &tms34020_device::move0_ar_b, &tms34020_device::move0_aa,   &tms34020_device::illop,      &tms34020_device::movb_ra_a,  &tms34020_device::movb_ra_b,
	// 0x0600
	&tms34020_device::cexec_l,    &tms34020_device::illop,      &tms34020_device::cmovgc_a,   &tms34020_device::cmovgc_b,   &tms34020_device::cmovgc_a_s, &tms34020_device::cmovgc_b_s, &tms34020_device::cmovcg_a,   &tms34020_device::cmovcg_b,
	&tms34020_device::cmovmc_f,   &tms34020_device::cmovmc_f,   &tms34020_device::cmovcm_f,   &tms34020_device::cmovcm_f,   &tms34020_device::cmovcm_b,   &tms34020_device::cmovcm_b,   &tms34020_device::cmovmc_f_va,&tms34020_device::cmovmc_f_vb,
	// 0x0700
	&tms34020_device::sext1_a,    &tms34020_device::sext1_b,    &tms34020_device::zext1_a,    &tms34020_device::zext1_b,    &tms34020_device::setf1,      &tms34020_device::setf1,      &tms34020_device::setf1,      &tms34020_device::setf1,
	&tms34020_device::move1_ra_a, &tms34020_device::move1_ra_b, &tms34020_device::move1_ar_a, &tms34020_device::move1_ar_b, &tms34020_device::move1_aa,   &tms34020_device::illop,      &tms34020_device::movb_ar_a,  &tms34020_device::movb_ar_b,
	// 0x0800
	&tms34020_device::trapl,      &tms34020_device::illop,      &tms34020_device::cmovmc_b,   &tms34020_device::cmovmc_b,   &tms34020_device::illop,      &tms34020_device::vblt_b_l,   &tms34020_device::retm,       &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::clip,
	// 0x0900
	&tms34020_device::trap,       &tms34020_device::trap,       &tms34020_device::call_a,     &tms34020_device::call_b,     &tms34020_device::reti,       &tms34020_device::illop,      &tms34020_device::rets,       &tms34020_device::rets,
	&tms34020_device::mmtm_a,     &tms34020_device::mmtm_b,     &tms34020_device::mmfm_a,     &tms34020_device::mmfm_b,     &tms34020_device::movi_w_a,   &tms34020_device::movi_w_b,   &tms34020_device::movi_l_a,   &tms34020_device::movi_l_b,
	// 0x0a00
	&tms34020_device::vlcol,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::pfill_xy,   &tms34020_device::illop,      &tms34020_device::vfill_l,    &tms34020_device::cvmxyl_a,   &tms34020_device::cvmxyl_b,
	&tms34020_device::cvdxyl_a,   &tms34020_device::cvdxyl_b,   &tms34020_device::illop,      &tms34020_device::fpixeq,     &tms34020_device::illop,      &tms34020_device::fpixne,     &tms34020_device::illop,      &tms34020_device::illop,
	// 0x0b00
	&tms34020_device::addi_w_a,   &tms34020_device::addi_w_b,   &tms34020_device::addi_l_a,   &tms34020_device::addi_l_b,   &tms34020_device::cmpi_w_a,   &tms34020_device::cmpi_w_b,   &tms34020_device::cmpi_l_a,   &tms34020_device::cmpi_l_b,
	&tms34020_device::andi_a,     &tms34020_device::andi_b,     &tms34020_device::ori_a,      &tms34020_device::ori_b,      &tms34020_device::xori_a,     &tms34020_device::xori_b,     &tms34020_device::subi_w_a,   &tms34020_device::subi_w_b,
	// 0x0c00
	&tms34020_device::addxyi_a,   &tms34020_device::addxyi_b,   &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x0d00
	&tms34020_device::subi_l_a,   &tms34020_device::subi_l_b,   &tms34020_device::illop,      &tms34020_device::callr,      &tms34020_device::illop,      &tms34020_device::calla,      &tms34020_device::eint,       &tms34020_device::illop,
	&tms34020_device::dsj_a,      &tms34020_device::dsj_b,      &tms34020_device::dsjeq_a,    &tms34020_device::dsjeq_b,    &tms34020_device::dsjne_a,    &tms34020_device::dsjne_b,    &tms34020_device::setc,       &tms34020_device::illop,
	// 0x0e00
	&tms34020_device::illop,      &tms34020_device::pixblt_l_m_l,&tms34020_device::illop,     &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::tfill_xy,
	// 0x0f00
	&tms34020_device::pixblt_l_l, &tms34020_device::illop,      &tms34020_device::pixblt_l_xy,&tms34020_device::illop,      &tms34020_device::pixblt_xy_l,&tms34020_device::illop,      &tms34020_device::pixblt_xy_xy,&tms34020_device::illop,
	&tms34020_device::pixblt_b_l, &tms34020_device::illop,      &tms34020_device::pixblt_b_xy,&tms34020_device::illop,      &tms34020_device::fill_l,     &tms34020_device::illop,      &tms34020_device::fill_xy,    &tms34020_device::illop,
	// 0x1000
	&tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,
	&tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,
	// 0x1100
	&tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,
	&tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,
	// 0x1200
	&tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,
	&tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,
	// 0x1300
	&tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,
	&tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,     &tms34020_device::addk_a,     &tms34020_device::addk_b,
	// 0x1400
	&tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,
	&tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,
	// 0x1500
	&tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,
	&tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,
	// 0x1600
	&tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,
	&tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,
	// 0x1700
	&tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,
	&tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,     &tms34020_device::subk_a,     &tms34020_device::subk_b,
	// 0x1800
	&tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,
	&tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,
	// 0x1900
	&tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,
	&tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,
	// 0x1a00
	&tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,
	&tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,
	// 0x1b00
	&tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,
	&tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,     &tms34020_device::movk_a,     &tms34020_device::movk_b,
	// 0x1c00
	&tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,
	&tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,
	// 0x1d00
	&tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,
	&tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,
	// 0x1e00
	&tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,
	&tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,
	// 0x1f00
	&tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,
	&tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,   &tms34020_device::btst_k_a,   &tms34020_device::btst_k_b,
	// 0x2000
	&tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,
	&tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,
	// 0x2100
	&tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,
	&tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,
	// 0x2200
	&tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,
	&tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,
	// 0x2300
	&tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,
	&tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,    &tms34020_device::sla_k_a,    &tms34020_device::sla_k_b,
	// 0x2400
	&tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,
	&tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,
	// 0x2500
	&tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,
	&tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,
	// 0x2600
	&tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,
	&tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,
	// 0x2700
	&tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,
	&tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,    &tms34020_device::sll_k_a,    &tms34020_device::sll_k_b,
	// 0x2800
	&tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,
	&tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,
	// 0x2900
	&tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,
	&tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,
	// 0x2a00
	&tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,
	&tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,
	// 0x2b00
	&tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,
	&tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,    &tms34020_device::sra_k_a,    &tms34020_device::sra_k_b,
	// 0x2c00
	&tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,
	&tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,
	// 0x2d00
	&tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,
	&tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,
	// 0x2e00
	&tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,
	&tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,
	// 0x2f00
	&tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,
	&tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,    &tms34020_device::srl_k_a,    &tms34020_device::srl_k_b,
	// 0x3000
	&tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,
	&tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,
	// 0x3100
	&tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,
	&tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,
	// 0x3200
	&tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,
	&tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,
	// 0x3300
	&tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,
	&tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,     &tms34020_device::rl_k_a,     &tms34020_device::rl_k_b,
	// 0x3400
	&tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,
	&tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,
	// 0x3500
	&tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,
	&tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,
	// 0x3600
	&tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,
	&tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,
	// 0x3700
	&tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,
	&tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,    &tms34020_device::cmp_k_a,    &tms34020_device::cmp_k_b,
	// 0x3800
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	// 0x3900
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	// 0x3a00
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	// 0x3b00
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	// 0x3c00
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	// 0x3d00
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	// 0x3e00
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	// 0x3f00
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	&tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,     &tms34020_device::dsjs_a,     &tms34020_device::dsjs_b,
	// 0x4000
	&tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,
	&tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,
	// 0x4100
	&tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,
	&tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,      &tms34020_device::add_a,      &tms34020_device::add_b,
	// 0x4200
	&tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,
	&tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,
	// 0x4300
	&tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,
	&tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,     &tms34020_device::addc_a,     &tms34020_device::addc_b,
	// 0x4400
	&tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,
	&tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,
	// 0x4500
	&tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,
	&tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,      &tms34020_device::sub_a,      &tms34020_device::sub_b,
	// 0x4600
	&tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,
	&tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,
	// 0x4700
	&tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,
	&tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,     &tms34020_device::subb_a,     &tms34020_device::subb_b,
	// 0x4800
	&tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,
	&tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,
	// 0x4900
	&tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,
	&tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,      &tms34020_device::cmp_a,      &tms34020_device::cmp_b,
	// 0x4a00
	&tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,
	&tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,
	// 0x4b00
	&tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,
	&tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,   &tms34020_device::btst_r_a,   &tms34020_device::btst_r_b,
	// 0x4c00
	&tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,
	&tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,
	// 0x4d00
	&tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,
	&tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,  &tms34020_device::move_rr_a,  &tms34020_device::move_rr_b,
	// 0x4e00
	&tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx,
	&tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx,
	// 0x4f00
	&tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx,
	&tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx, &tms34020_device::move_rr_ax, &tms34020_device::move_rr_bx,
	// 0x5000
	&tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,
	&tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,
	// 0x5100
	&tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,
	&tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,      &tms34020_device::and_a,      &tms34020_device::and_b,
	// 0x5200
	&tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,
	&tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,
	// 0x5300
	&tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,
	&tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,     &tms34020_device::andn_a,     &tms34020_device::andn_b,
	// 0x5400
	&tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,
	&tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,
	// 0x5500
	&tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,
	&tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,       &tms34020_device::or_a,       &tms34020_device::or_b,
	// 0x5600
	&tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,
	&tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,
	// 0x5700
	&tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,
	&tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,      &tms34020_device::xor_a,      &tms34020_device::xor_b,
	// 0x5800
	&tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,
	&tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,
	// 0x5900
	&tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,
	&tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,     &tms34020_device::divs_a,     &tms34020_device::divs_b,
	// 0x5a00
	&tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,
	&tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,
	// 0x5b00
	&tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,
	&tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,     &tms34020_device::divu_a,     &tms34020_device::divu_b,
	// 0x5c00
	&tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,
	&tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,
	// 0x5d00
	&tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,
	&tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,     &tms34020_device::mpys_a,     &tms34020_device::mpys_b,
	// 0x5e00
	&tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,
	&tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,
	// 0x5f00
	&tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,
	&tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,     &tms34020_device::mpyu_a,     &tms34020_device::mpyu_b,
	// 0x6000
	&tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,
	&tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,
	// 0x6100
	&tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,
	&tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,    &tms34020_device::sla_r_a,    &tms34020_device::sla_r_b,
	// 0x6200
	&tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,
	&tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,
	// 0x6300
	&tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,
	&tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,    &tms34020_device::sll_r_a,    &tms34020_device::sll_r_b,
	// 0x6400
	&tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,
	&tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,
	// 0x6500
	&tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,
	&tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,    &tms34020_device::sra_r_a,    &tms34020_device::sra_r_b,
	// 0x6600
	&tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,
	&tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,
	// 0x6700
	&tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,
	&tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,    &tms34020_device::srl_r_a,    &tms34020_device::srl_r_b,
	// 0x6800
	&tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,
	&tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,
	// 0x6900
	&tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,
	&tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,     &tms34020_device::rl_r_a,     &tms34020_device::rl_r_b,
	// 0x6a00
	&tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,
	&tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,
	// 0x6b00
	&tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,
	&tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,      &tms34020_device::lmo_a,      &tms34020_device::lmo_b,
	// 0x6c00
	&tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,
	&tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,
	// 0x6d00
	&tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,
	&tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,     &tms34020_device::mods_a,     &tms34020_device::mods_b,
	// 0x6e00
	&tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,
	&tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,
	// 0x6f00
	&tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,
	&tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,     &tms34020_device::modu_a,     &tms34020_device::modu_b,
	// 0x7000
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7100
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7200
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7300
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7400
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7500
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7600
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7700
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7800
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7900
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7a00
	&tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,
	&tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,
	// 0x7b00
	&tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,
	&tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,      &tms34020_device::rmo_a,      &tms34020_device::rmo_b,
	// 0x7c00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7d00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x7e00
	&tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,
	&tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,
	// 0x7f00
	&tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,
	&tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,    &tms34020_device::swapf_a,    &tms34020_device::swapf_b,
	// 0x8000
	&tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b,
	&tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b,
	// 0x8100
	&tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b,
	&tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b, &tms34020_device::move0_rn_a, &tms34020_device::move0_rn_b,
	// 0x8200
	&tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b,
	&tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b,
	// 0x8300
	&tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b,
	&tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b, &tms34020_device::move1_rn_a, &tms34020_device::move1_rn_b,
	// 0x8400
	&tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b,
	&tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b,
	// 0x8500
	&tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b,
	&tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b, &tms34020_device::move0_nr_a, &tms34020_device::move0_nr_b,
	// 0x8600
	&tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b,
	&tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b,
	// 0x8700
	&tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b,
	&tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b, &tms34020_device::move1_nr_a, &tms34020_device::move1_nr_b,
	// 0x8800
	&tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b,
	&tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b,
	// 0x8900
	&tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b,
	&tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b, &tms34020_device::move0_nn_a, &tms34020_device::move0_nn_b,
	// 0x8a00
	&tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b,
	&tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b,
	// 0x8b00
	&tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b,
	&tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b, &tms34020_device::move1_nn_a, &tms34020_device::move1_nn_b,
	// 0x8c00
	&tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,
	&tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,
	// 0x8d00
	&tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,
	&tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,  &tms34020_device::movb_rn_a,  &tms34020_device::movb_rn_b,
	// 0x8e00
	&tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,
	&tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,
	// 0x8f00
	&tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,
	&tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,  &tms34020_device::movb_nr_a,  &tms34020_device::movb_nr_b,
	// 0x9000
	&tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,
	&tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,
	// 0x9100
	&tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,
	&tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,   &tms34020_device::move0_r_ni_a,   &tms34020_device::move0_r_ni_b,
	// 0x9200
	&tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,
	&tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,
	// 0x9300
	&tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,
	&tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,   &tms34020_device::move1_r_ni_a,   &tms34020_device::move1_r_ni_b,
	// 0x9400
	&tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,
	&tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,
	// 0x9500
	&tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,
	&tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,   &tms34020_device::move0_ni_r_a,   &tms34020_device::move0_ni_r_b,
	// 0x9600
	&tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,
	&tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,
	// 0x9700
	&tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,
	&tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,   &tms34020_device::move1_ni_r_a,   &tms34020_device::move1_ni_r_b,
	// 0x9800
	&tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,
	&tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,
	// 0x9900
	&tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,
	&tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,  &tms34020_device::move0_ni_ni_a,  &tms34020_device::move0_ni_ni_b,
	// 0x9a00
	&tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,
	&tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,
	// 0x9b00
	&tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,
	&tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,  &tms34020_device::move1_ni_ni_a,  &tms34020_device::move1_ni_ni_b,
	// 0x9c00
	&tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,
	&tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,
	// 0x9d00
	&tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,
	&tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,  &tms34020_device::movb_nn_a,  &tms34020_device::movb_nn_b,
	// 0x9e00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0x9f00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xa000
	&tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,
	&tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,
	// 0xa100
	&tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,
	&tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,   &tms34020_device::move0_r_dn_a,   &tms34020_device::move0_r_dn_b,
	// 0xa200
	&tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,
	&tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,
	// 0xa300
	&tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,
	&tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,   &tms34020_device::move1_r_dn_a,   &tms34020_device::move1_r_dn_b,
	// 0xa400
	&tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,
	&tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,
	// 0xa500
	&tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,
	&tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,   &tms34020_device::move0_dn_r_a,   &tms34020_device::move0_dn_r_b,
	// 0xa600
	&tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,
	&tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,
	// 0xa700
	&tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,
	&tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,   &tms34020_device::move1_dn_r_a,   &tms34020_device::move1_dn_r_b,
	// 0xa800
	&tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,
	&tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,
	// 0xa900
	&tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,
	&tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,  &tms34020_device::move0_dn_dn_a,  &tms34020_device::move0_dn_dn_b,
	// 0xaa00
	&tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,
	&tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,
	// 0xab00
	&tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,
	&tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,  &tms34020_device::move1_dn_dn_a,  &tms34020_device::move1_dn_dn_b,
	// 0xac00
	&tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,
	&tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,
	// 0xad00
	&tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,
	&tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,    &tms34020_device::movb_r_no_a,    &tms34020_device::movb_r_no_b,
	// 0xae00
	&tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,
	&tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,
	// 0xaf00
	&tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,
	&tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,    &tms34020_device::movb_no_r_a,    &tms34020_device::movb_no_r_b,
	// 0xb000
	&tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,
	&tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,
	// 0xb100
	&tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,
	&tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,   &tms34020_device::move0_r_no_a,   &tms34020_device::move0_r_no_b,
	// 0xb200
	&tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,
	&tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,
	// 0xb300
	&tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,
	&tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,   &tms34020_device::move1_r_no_a,   &tms34020_device::move1_r_no_b,
	// 0xb400
	&tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,
	&tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,
	// 0xb500
	&tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,
	&tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,   &tms34020_device::move0_no_r_a,   &tms34020_device::move0_no_r_b,
	// 0xb600
	&tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,
	&tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,
	// 0xb700
	&tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,
	&tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,   &tms34020_device::move1_no_r_a,   &tms34020_device::move1_no_r_b,
	// 0xb800
	&tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,
	&tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,
	// 0xb900
	&tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,
	&tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,  &tms34020_device::move0_no_no_a,  &tms34020_device::move0_no_no_b,
	// 0xba00
	&tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,
	&tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,
	// 0xbb00
	&tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,
	&tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,  &tms34020_device::move1_no_no_a,  &tms34020_device::move1_no_no_b,
	// 0xbc00
	&tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,
	&tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,
	// 0xbd00
	&tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,
	&tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,   &tms34020_device::movb_no_no_a,   &tms34020_device::movb_no_no_b,
	// 0xbe00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xbf00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xc000
	&tms34020_device::j_UC_0,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,
	&tms34020_device::j_UC_8,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,     &tms34020_device::j_UC_x,
	// 0xc100
	&tms34020_device::j_P_0,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,
	&tms34020_device::j_P_8,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,      &tms34020_device::j_P_x,
	// 0xc200
	&tms34020_device::j_LS_0,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,
	&tms34020_device::j_LS_8,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,     &tms34020_device::j_LS_x,
	// 0xc300
	&tms34020_device::j_HI_0,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,
	&tms34020_device::j_HI_8,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,     &tms34020_device::j_HI_x,
	// 0xc400
	&tms34020_device::j_LT_0,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,
	&tms34020_device::j_LT_8,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,     &tms34020_device::j_LT_x,
	// 0xc500
	&tms34020_device::j_GE_0,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,
	&tms34020_device::j_GE_8,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,     &tms34020_device::j_GE_x,
	// 0xc600
	&tms34020_device::j_LE_0,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,
	&tms34020_device::j_LE_8,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,     &tms34020_device::j_LE_x,
	// 0xc700
	&tms34020_device::j_GT_0,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,
	&tms34020_device::j_GT_8,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,     &tms34020_device::j_GT_x,
	// 0xc800
	&tms34020_device::j_C_0,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,
	&tms34020_device::j_C_8,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,      &tms34020_device::j_C_x,
	// 0xc900
	&tms34020_device::j_NC_0,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,
	&tms34020_device::j_NC_8,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,     &tms34020_device::j_NC_x,
	// 0xca00
	&tms34020_device::j_EQ_0,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,
	&tms34020_device::j_EQ_8,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,     &tms34020_device::j_EQ_x,
	// 0xcb00
	&tms34020_device::j_NE_0,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,
	&tms34020_device::j_NE_8,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,     &tms34020_device::j_NE_x,
	// 0xcc00
	&tms34020_device::j_V_0,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,
	&tms34020_device::j_V_8,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,      &tms34020_device::j_V_x,
	// 0xcd00
	&tms34020_device::j_NV_0,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,
	&tms34020_device::j_NV_8,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,     &tms34020_device::j_NV_x,
	// 0xce00
	&tms34020_device::j_N_0,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,
	&tms34020_device::j_N_8,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,      &tms34020_device::j_N_x,
	// 0xcf00
	&tms34020_device::j_NN_0,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,
	&tms34020_device::j_NN_8,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,     &tms34020_device::j_NN_x,
	// 0xd000
	&tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,
	&tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,
	// 0xd100
	&tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,
	&tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,  &tms34020_device::move0_no_ni_a,  &tms34020_device::move0_no_ni_b,
	// 0xd200
	&tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,
	&tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,
	// 0xd300
	&tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,
	&tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,  &tms34020_device::move1_no_ni_a,  &tms34020_device::move1_no_ni_b,
	// 0xd400
	&tms34020_device::move0_a_ni_a,&tms34020_device::move0_a_ni_b,&tms34020_device::illop,    &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xd500
	&tms34020_device::exgf0_a,    &tms34020_device::exgf0_b,    &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xd600
	&tms34020_device::move1_a_ni_a,&tms34020_device::move1_a_ni_b,&tms34020_device::illop,    &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xd700
	&tms34020_device::exgf1_a,    &tms34020_device::exgf1_b,    &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xd800
	&tms34020_device::cexec_s,    &tms34020_device::cexec_s,    &tms34020_device::cexec_s,    &tms34020_device::cexec_s,    &tms34020_device::cexec_s,    &tms34020_device::cexec_s,    &tms34020_device::cexec_s,    &tms34020_device::cexec_s,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xd900
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xda00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xdb00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xdc00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xdd00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xde00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xdf00
	&tms34020_device::illop,      &tms34020_device::line,       &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::line,       &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,     &tms34020_device::illop,
	// 0xe000
	&tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,
	&tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,
	// 0xe100
	&tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,
	&tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,   &tms34020_device::add_xy_a,   &tms34020_device::add_xy_b,
	// 0xe200
	&tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,
	&tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,
	// 0xe300
	&tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,
	&tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,   &tms34020_device::sub_xy_a,   &tms34020_device::sub_xy_b,
	// 0xe400
	&tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,
	&tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,
	// 0xe500
	&tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,
	&tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,   &tms34020_device::cmp_xy_a,   &tms34020_device::cmp_xy_b,
	// 0xe600
	&tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,
	&tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,
	// 0xe700
	&tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,
	&tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,      &tms34020_device::cpw_a,      &tms34020_device::cpw_b,
	// 0xe800
	&tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,
	&tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,
	// 0xe900
	&tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,
	&tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,    &tms34020_device::cvxyl_a,    &tms34020_device::cvxyl_b,
	// 0xea00
	&tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,
	&tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,
	// 0xeb00
	&tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,
	&tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,   &tms34020_device::cvsxyl_a,   &tms34020_device::cvsxyl_b,
	// 0xec00
	&tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,
	&tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,
	// 0xed00
	&tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,
	&tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,     &tms34020_device::movx_a,     &tms34020_device::movx_b,
	// 0xee00
	&tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,
	&tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,
	// 0xef00
	&tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,
	&tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,     &tms34020_device::movy_a,     &tms34020_device::movy_b,
	// 0xf000
	&tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,
	&tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,
	// 0xf100
	&tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,
	&tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,    &tms34020_device::pixt_rixy_a,    &tms34020_device::pixt_rixy_b,
	// 0xf200
	&tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,
	&tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,
	// 0xf300
	&tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,
	&tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,    &tms34020_device::pixt_ixyr_a,    &tms34020_device::pixt_ixyr_b,
	// 0xf400
	&tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,
	&tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,
	// 0xf500
	&tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,
	&tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,  &tms34020_device::pixt_ixyixy_a,  &tms34020_device::pixt_ixyixy_b,
	// 0xf600
	&tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,
	&tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,
	// 0xf700
	&tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,
	&tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,     &tms34020_device::drav_a,     &tms34020_device::drav_b,
	// 0xf800
	&tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,
	&tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,
	// 0xf900
	&tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,
	&tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,  &tms34020_device::pixt_ri_a,  &tms34020_device::pixt_ri_b,
	// 0xfa00
	&tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,
	&tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,
	// 0xfb00
	&tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,
	&tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,  &tms34020_device::pixt_ir_a,  &tms34020_device::pixt_ir_b,
	// 0xfc00
	&tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,
	&tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,
	// 0xfd00
	&tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,
	&tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,  &tms34020_device::pixt_ii_a,  &tms34020_device::pixt_ii_b,
	// 0xfe00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,
	// 0xff00
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,
	&tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop,      &tms34020_device::illop
};
