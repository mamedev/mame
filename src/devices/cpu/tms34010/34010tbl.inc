// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari
/*** TMS34010: Portable TMS34010 emulator ***********************************

    Copyright Alex Pasadyn/Zsolt Vasvari

    Opcode Table

*****************************************************************************/

/* Opcode Table */
const tms340x0_device::opcode_func tms340x0_device::s_opcode_table[65536 >> 4] =
{
	/* 0x0000 0x0010 0x0020 0x0030 ... 0x00f0 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::rev_a,      &tms340x0_device::rev_b,      &tms340x0_device::idle,       &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::mwait,      &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::blmove,
	/* 0x0100 */
	&tms340x0_device::emu,        &tms340x0_device::unimpl,     &tms340x0_device::exgpc_a,    &tms340x0_device::exgpc_b,    &tms340x0_device::getpc_a,    &tms340x0_device::getpc_b,    &tms340x0_device::jump_a,     &tms340x0_device::jump_b,
	&tms340x0_device::getst_a,    &tms340x0_device::getst_b,    &tms340x0_device::putst_a,    &tms340x0_device::putst_b,    &tms340x0_device::popst,      &tms340x0_device::unimpl,     &tms340x0_device::pushst,     &tms340x0_device::unimpl,
	/* 0x0200 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::setcsp,     &tms340x0_device::unimpl,     &tms340x0_device::setcdp,
	&tms340x0_device::rpix_a,     &tms340x0_device::rpix_b,     &tms340x0_device::exgps_a,    &tms340x0_device::exgps_b,    &tms340x0_device::getps_a,    &tms340x0_device::getps_b,    &tms340x0_device::unimpl,     &tms340x0_device::setcmp,
	/* 0x0300 */
	&tms340x0_device::nop,        &tms340x0_device::unimpl,     &tms340x0_device::clrc,       &tms340x0_device::unimpl,     &tms340x0_device::movb_aa,    &tms340x0_device::unimpl,     &tms340x0_device::dint,       &tms340x0_device::unimpl,
	&tms340x0_device::abs_a,      &tms340x0_device::abs_b,      &tms340x0_device::neg_a,      &tms340x0_device::neg_b,      &tms340x0_device::negb_a,     &tms340x0_device::negb_b,     &tms340x0_device::not_a,      &tms340x0_device::not_b,
	/* 0x0400 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x0500 */
	&tms340x0_device::sext0_a,    &tms340x0_device::sext0_b,    &tms340x0_device::zext0_a,    &tms340x0_device::zext0_b,    &tms340x0_device::setf0,      &tms340x0_device::setf0,      &tms340x0_device::setf0,      &tms340x0_device::setf0,
	&tms340x0_device::move0_ra_a, &tms340x0_device::move0_ra_b, &tms340x0_device::move0_ar_a, &tms340x0_device::move0_ar_b, &tms340x0_device::move0_aa,   &tms340x0_device::unimpl,     &tms340x0_device::movb_ra_a,  &tms340x0_device::movb_ra_b,
	/* 0x0600 */
	&tms340x0_device::cexec_l,    &tms340x0_device::unimpl,     &tms340x0_device::cmovgc_a,   &tms340x0_device::cmovgc_b,   &tms340x0_device::cmovgc_a_s, &tms340x0_device::cmovgc_b_s, &tms340x0_device::cmovcg_a,   &tms340x0_device::cmovcg_b,
	&tms340x0_device::cmovmc_f,   &tms340x0_device::cmovmc_f,   &tms340x0_device::cmovcm_f,   &tms340x0_device::cmovcm_f,   &tms340x0_device::cmovcm_b,   &tms340x0_device::cmovcm_b,   &tms340x0_device::cmovmc_f_va,&tms340x0_device::cmovmc_f_vb,
	/* 0x0700 */
	&tms340x0_device::sext1_a,    &tms340x0_device::sext1_b,    &tms340x0_device::zext1_a,    &tms340x0_device::zext1_b,    &tms340x0_device::setf1,      &tms340x0_device::setf1,      &tms340x0_device::setf1,      &tms340x0_device::setf1,
	&tms340x0_device::move1_ra_a, &tms340x0_device::move1_ra_b, &tms340x0_device::move1_ar_a, &tms340x0_device::move1_ar_b, &tms340x0_device::move1_aa,   &tms340x0_device::unimpl,     &tms340x0_device::movb_ar_a,  &tms340x0_device::movb_ar_b,
	/* 0x0800 */
	&tms340x0_device::trapl,      &tms340x0_device::unimpl,     &tms340x0_device::cmovmc_b,   &tms340x0_device::cmovmc_b,   &tms340x0_device::unimpl,     &tms340x0_device::vblt_b_l,   &tms340x0_device::retm,       &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::clip,
	/* 0x0900 */
	&tms340x0_device::trap,       &tms340x0_device::trap,       &tms340x0_device::call_a,     &tms340x0_device::call_b,     &tms340x0_device::reti,       &tms340x0_device::unimpl,     &tms340x0_device::rets,       &tms340x0_device::rets,
	&tms340x0_device::mmtm_a,     &tms340x0_device::mmtm_b,     &tms340x0_device::mmfm_a,     &tms340x0_device::mmfm_b,     &tms340x0_device::movi_w_a,   &tms340x0_device::movi_w_b,   &tms340x0_device::movi_l_a,   &tms340x0_device::movi_l_b,
	/* 0x0a00 */
	&tms340x0_device::vlcol,      &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::pfill_xy,   &tms340x0_device::unimpl,     &tms340x0_device::vfill_l,    &tms340x0_device::cvmxyl_a,   &tms340x0_device::cvmxyl_b,
	&tms340x0_device::cvdxyl_a,   &tms340x0_device::cvdxyl_b,   &tms340x0_device::unimpl,     &tms340x0_device::fpixeq,     &tms340x0_device::unimpl,     &tms340x0_device::fpixne,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x0b00 */
	&tms340x0_device::addi_w_a,   &tms340x0_device::addi_w_b,   &tms340x0_device::addi_l_a,   &tms340x0_device::addi_l_b,   &tms340x0_device::cmpi_w_a,   &tms340x0_device::cmpi_w_b,   &tms340x0_device::cmpi_l_a,   &tms340x0_device::cmpi_l_b,
	&tms340x0_device::andi_a,     &tms340x0_device::andi_b,     &tms340x0_device::ori_a,      &tms340x0_device::ori_b,      &tms340x0_device::xori_a,     &tms340x0_device::xori_b,     &tms340x0_device::subi_w_a,   &tms340x0_device::subi_w_b,
	/* 0x0c00 */
	&tms340x0_device::addxyi_a,   &tms340x0_device::addxyi_b,   &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::linit,      &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x0d00 */
	&tms340x0_device::subi_l_a,   &tms340x0_device::subi_l_b,   &tms340x0_device::unimpl,     &tms340x0_device::callr,      &tms340x0_device::unimpl,     &tms340x0_device::calla,      &tms340x0_device::eint,       &tms340x0_device::unimpl,
	&tms340x0_device::dsj_a,      &tms340x0_device::dsj_b,      &tms340x0_device::dsjeq_a,    &tms340x0_device::dsjeq_b,    &tms340x0_device::dsjne_a,    &tms340x0_device::dsjne_b,    &tms340x0_device::setc,       &tms340x0_device::unimpl,
	/* 0x0e00 */
	&tms340x0_device::unimpl,     &tms340x0_device::pixblt_l_m_l,&tms340x0_device::unimpl,    &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::tfill_xy,
	/* 0x0f00 */
	&tms340x0_device::pixblt_l_l, &tms340x0_device::unimpl,     &tms340x0_device::pixblt_l_xy,&tms340x0_device::unimpl,     &tms340x0_device::pixblt_xy_l,&tms340x0_device::unimpl,     &tms340x0_device::pixblt_xy_xy,&tms340x0_device::unimpl,
	&tms340x0_device::pixblt_b_l, &tms340x0_device::unimpl,     &tms340x0_device::pixblt_b_xy,&tms340x0_device::unimpl,     &tms340x0_device::fill_l,     &tms340x0_device::unimpl,     &tms340x0_device::fill_xy,    &tms340x0_device::unimpl,
	/* 0x1000 */
	&tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,
	&tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,
	/* 0x1100 */
	&tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,
	&tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,
	/* 0x1200 */
	&tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,
	&tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,
	/* 0x1300 */
	&tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,
	&tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,     &tms340x0_device::addk_a,     &tms340x0_device::addk_b,
	/* 0x1400 */
	&tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,
	&tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,
	/* 0x1500 */
	&tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,
	&tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,
	/* 0x1600 */
	&tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,
	&tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,
	/* 0x1700 */
	&tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,
	&tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,     &tms340x0_device::subk_a,     &tms340x0_device::subk_b,
	/* 0x1800 */
	&tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,
	&tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,
	/* 0x1900 */
	&tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,
	&tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,
	/* 0x1a00 */
	&tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,
	&tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,
	/* 0x1b00 */
	&tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,
	&tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,     &tms340x0_device::movk_a,     &tms340x0_device::movk_b,
	/* 0x1c00 */
	&tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,
	&tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,
	/* 0x1d00 */
	&tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,
	&tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,
	/* 0x1e00 */
	&tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,
	&tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,
	/* 0x1f00 */
	&tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,
	&tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,   &tms340x0_device::btst_k_a,   &tms340x0_device::btst_k_b,
	/* 0x2000 */
	&tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,
	&tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,
	/* 0x2100 */
	&tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,
	&tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,
	/* 0x2200 */
	&tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,
	&tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,
	/* 0x2300 */
	&tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,
	&tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,    &tms340x0_device::sla_k_a,    &tms340x0_device::sla_k_b,
	/* 0x2400 */
	&tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,
	&tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,
	/* 0x2500 */
	&tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,
	&tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,
	/* 0x2600 */
	&tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,
	&tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,
	/* 0x2700 */
	&tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,
	&tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,    &tms340x0_device::sll_k_a,    &tms340x0_device::sll_k_b,
	/* 0x2800 */
	&tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,
	&tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,
	/* 0x2900 */
	&tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,
	&tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,
	/* 0x2a00 */
	&tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,
	&tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,
	/* 0x2b00 */
	&tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,
	&tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,    &tms340x0_device::sra_k_a,    &tms340x0_device::sra_k_b,
	/* 0x2c00 */
	&tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,
	&tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,
	/* 0x2d00 */
	&tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,
	&tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,
	/* 0x2e00 */
	&tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,
	&tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,
	/* 0x2f00 */
	&tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,
	&tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,    &tms340x0_device::srl_k_a,    &tms340x0_device::srl_k_b,
	/* 0x3000 */
	&tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,
	&tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,
	/* 0x3100 */
	&tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,
	&tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,
	/* 0x3200 */
	&tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,
	&tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,
	/* 0x3300 */
	&tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,
	&tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,     &tms340x0_device::rl_k_a,     &tms340x0_device::rl_k_b,
	/* 0x3400 */
	&tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,
	&tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,
	/* 0x3500 */
	&tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,
	&tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,
	/* 0x3600 */
	&tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,
	&tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,
	/* 0x3700 */
	&tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,
	&tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,    &tms340x0_device::cmp_k_a,    &tms340x0_device::cmp_k_b,
	/* 0x3800 */
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	/* 0x3900 */
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	/* 0x3a00 */
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	/* 0x3b00 */
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	/* 0x3c00 */
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	/* 0x3d00 */
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	/* 0x3e00 */
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	/* 0x3f00 */
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	&tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,     &tms340x0_device::dsjs_a,     &tms340x0_device::dsjs_b,
	/* 0x4000 */
	&tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,
	&tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,
	/* 0x4100 */
	&tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,
	&tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,      &tms340x0_device::add_a,      &tms340x0_device::add_b,
	/* 0x4200 */
	&tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,
	&tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,
	/* 0x4300 */
	&tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,
	&tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,     &tms340x0_device::addc_a,     &tms340x0_device::addc_b,
	/* 0x4400 */
	&tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,
	&tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,
	/* 0x4500 */
	&tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,
	&tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,      &tms340x0_device::sub_a,      &tms340x0_device::sub_b,
	/* 0x4600 */
	&tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,
	&tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,
	/* 0x4700 */
	&tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,
	&tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,     &tms340x0_device::subb_a,     &tms340x0_device::subb_b,
	/* 0x4800 */
	&tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,
	&tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,
	/* 0x4900 */
	&tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,
	&tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,      &tms340x0_device::cmp_a,      &tms340x0_device::cmp_b,
	/* 0x4a00 */
	&tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,
	&tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,
	/* 0x4b00 */
	&tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,
	&tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,   &tms340x0_device::btst_r_a,   &tms340x0_device::btst_r_b,
	/* 0x4c00 */
	&tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,
	&tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,
	/* 0x4d00 */
	&tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,
	&tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,  &tms340x0_device::move_rr_a,  &tms340x0_device::move_rr_b,
	/* 0x4e00 */
	&tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx,
	&tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx,
	/* 0x4f00 */
	&tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx,
	&tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx, &tms340x0_device::move_rr_ax, &tms340x0_device::move_rr_bx,
	/* 0x5000 */
	&tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,
	&tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,
	/* 0x5100 */
	&tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,
	&tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,      &tms340x0_device::and_a,      &tms340x0_device::and_b,
	/* 0x5200 */
	&tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,
	&tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,
	/* 0x5300 */
	&tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,
	&tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,     &tms340x0_device::andn_a,     &tms340x0_device::andn_b,
	/* 0x5400 */
	&tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,
	&tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,
	/* 0x5500 */
	&tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,
	&tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,       &tms340x0_device::or_a,       &tms340x0_device::or_b,
	/* 0x5600 */
	&tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,
	&tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,
	/* 0x5700 */
	&tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,
	&tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,      &tms340x0_device::xor_a,      &tms340x0_device::xor_b,
	/* 0x5800 */
	&tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,
	&tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,
	/* 0x5900 */
	&tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,
	&tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,     &tms340x0_device::divs_a,     &tms340x0_device::divs_b,
	/* 0x5a00 */
	&tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,
	&tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,
	/* 0x5b00 */
	&tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,
	&tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,     &tms340x0_device::divu_a,     &tms340x0_device::divu_b,
	/* 0x5c00 */
	&tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,
	&tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,
	/* 0x5d00 */
	&tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,
	&tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,     &tms340x0_device::mpys_a,     &tms340x0_device::mpys_b,
	/* 0x5e00 */
	&tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,
	&tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,
	/* 0x5f00 */
	&tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,
	&tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,     &tms340x0_device::mpyu_a,     &tms340x0_device::mpyu_b,
	/* 0x6000 */
	&tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,
	&tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,
	/* 0x6100 */
	&tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,
	&tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,    &tms340x0_device::sla_r_a,    &tms340x0_device::sla_r_b,
	/* 0x6200 */
	&tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,
	&tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,
	/* 0x6300 */
	&tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,
	&tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,    &tms340x0_device::sll_r_a,    &tms340x0_device::sll_r_b,
	/* 0x6400 */
	&tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,
	&tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,
	/* 0x6500 */
	&tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,
	&tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,    &tms340x0_device::sra_r_a,    &tms340x0_device::sra_r_b,
	/* 0x6600 */
	&tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,
	&tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,
	/* 0x6700 */
	&tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,
	&tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,    &tms340x0_device::srl_r_a,    &tms340x0_device::srl_r_b,
	/* 0x6800 */
	&tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,
	&tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,
	/* 0x6900 */
	&tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,
	&tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,     &tms340x0_device::rl_r_a,     &tms340x0_device::rl_r_b,
	/* 0x6a00 */
	&tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,
	&tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,
	/* 0x6b00 */
	&tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,
	&tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,      &tms340x0_device::lmo_a,      &tms340x0_device::lmo_b,
	/* 0x6c00 */
	&tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,
	&tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,
	/* 0x6d00 */
	&tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,
	&tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,     &tms340x0_device::mods_a,     &tms340x0_device::mods_b,
	/* 0x6e00 */
	&tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,
	&tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,
	/* 0x6f00 */
	&tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,
	&tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,     &tms340x0_device::modu_a,     &tms340x0_device::modu_b,
	/* 0x7000 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7100 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7200 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7300 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7400 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7500 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7600 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7700 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7800 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7900 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7a00 */
	&tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,
	&tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,
	/* 0x7b00 */
	&tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,
	&tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,      &tms340x0_device::rmo_a,      &tms340x0_device::rmo_b,
	/* 0x7c00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7d00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x7e00 */
	&tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,
	&tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,
	/* 0x7f00 */
	&tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,
	&tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,    &tms340x0_device::swapf_a,    &tms340x0_device::swapf_b,
	/* 0x8000 */
	&tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b,
	&tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b,
	/* 0x8100 */
	&tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b,
	&tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b, &tms340x0_device::move0_rn_a, &tms340x0_device::move0_rn_b,
	/* 0x8200 */
	&tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b,
	&tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b,
	/* 0x8300 */
	&tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b,
	&tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b, &tms340x0_device::move1_rn_a, &tms340x0_device::move1_rn_b,
	/* 0x8400 */
	&tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b,
	&tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b,
	/* 0x8500 */
	&tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b,
	&tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b, &tms340x0_device::move0_nr_a, &tms340x0_device::move0_nr_b,
	/* 0x8600 */
	&tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b,
	&tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b,
	/* 0x8700 */
	&tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b,
	&tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b, &tms340x0_device::move1_nr_a, &tms340x0_device::move1_nr_b,
	/* 0x8800 */
	&tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b,
	&tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b,
	/* 0x8900 */
	&tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b,
	&tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b, &tms340x0_device::move0_nn_a, &tms340x0_device::move0_nn_b,
	/* 0x8a00 */
	&tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b,
	&tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b,
	/* 0x8b00 */
	&tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b,
	&tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b, &tms340x0_device::move1_nn_a, &tms340x0_device::move1_nn_b,
	/* 0x8c00 */
	&tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,
	&tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,
	/* 0x8d00 */
	&tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,
	&tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,  &tms340x0_device::movb_rn_a,  &tms340x0_device::movb_rn_b,
	/* 0x8e00 */
	&tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,
	&tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,
	/* 0x8f00 */
	&tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,
	&tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,  &tms340x0_device::movb_nr_a,  &tms340x0_device::movb_nr_b,
	/* 0x9000 */
	&tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,
	&tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,
	/* 0x9100 */
	&tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,
	&tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,   &tms340x0_device::move0_r_ni_a,   &tms340x0_device::move0_r_ni_b,
	/* 0x9200 */
	&tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,
	&tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,
	/* 0x9300 */
	&tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,
	&tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,   &tms340x0_device::move1_r_ni_a,   &tms340x0_device::move1_r_ni_b,
	/* 0x9400 */
	&tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,
	&tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,
	/* 0x9500 */
	&tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,
	&tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,   &tms340x0_device::move0_ni_r_a,   &tms340x0_device::move0_ni_r_b,
	/* 0x9600 */
	&tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,
	&tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,
	/* 0x9700 */
	&tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,
	&tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,   &tms340x0_device::move1_ni_r_a,   &tms340x0_device::move1_ni_r_b,
	/* 0x9800 */
	&tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,
	&tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,
	/* 0x9900 */
	&tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,
	&tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,  &tms340x0_device::move0_ni_ni_a,  &tms340x0_device::move0_ni_ni_b,
	/* 0x9a00 */
	&tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,
	&tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,
	/* 0x9b00 */
	&tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,
	&tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,  &tms340x0_device::move1_ni_ni_a,  &tms340x0_device::move1_ni_ni_b,
	/* 0x9c00 */
	&tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,
	&tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,
	/* 0x9d00 */
	&tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,
	&tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,  &tms340x0_device::movb_nn_a,  &tms340x0_device::movb_nn_b,
	/* 0x9e00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0x9f00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xa000 */
	&tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,
	&tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,
	/* 0xa100 */
	&tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,
	&tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,   &tms340x0_device::move0_r_dn_a,   &tms340x0_device::move0_r_dn_b,
	/* 0xa200 */
	&tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,
	&tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,
	/* 0xa300 */
	&tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,
	&tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,   &tms340x0_device::move1_r_dn_a,   &tms340x0_device::move1_r_dn_b,
	/* 0xa400 */
	&tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,
	&tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,
	/* 0xa500 */
	&tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,
	&tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,   &tms340x0_device::move0_dn_r_a,   &tms340x0_device::move0_dn_r_b,
	/* 0xa600 */
	&tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,
	&tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,
	/* 0xa700 */
	&tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,
	&tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,   &tms340x0_device::move1_dn_r_a,   &tms340x0_device::move1_dn_r_b,
	/* 0xa800 */
	&tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,
	&tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,
	/* 0xa900 */
	&tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,
	&tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,  &tms340x0_device::move0_dn_dn_a,  &tms340x0_device::move0_dn_dn_b,
	/* 0xaa00 */
	&tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,
	&tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,
	/* 0xab00 */
	&tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,
	&tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,  &tms340x0_device::move1_dn_dn_a,  &tms340x0_device::move1_dn_dn_b,
	/* 0xac00 */
	&tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,
	&tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,
	/* 0xad00 */
	&tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,
	&tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,    &tms340x0_device::movb_r_no_a,    &tms340x0_device::movb_r_no_b,
	/* 0xae00 */
	&tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,
	&tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,
	/* 0xaf00 */
	&tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,
	&tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,    &tms340x0_device::movb_no_r_a,    &tms340x0_device::movb_no_r_b,
	/* 0xb000 */
	&tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,
	&tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,
	/* 0xb100 */
	&tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,
	&tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,   &tms340x0_device::move0_r_no_a,   &tms340x0_device::move0_r_no_b,
	/* 0xb200 */
	&tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,
	&tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,
	/* 0xb300 */
	&tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,
	&tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,   &tms340x0_device::move1_r_no_a,   &tms340x0_device::move1_r_no_b,
	/* 0xb400 */
	&tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,
	&tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,
	/* 0xb500 */
	&tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,
	&tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,   &tms340x0_device::move0_no_r_a,   &tms340x0_device::move0_no_r_b,
	/* 0xb600 */
	&tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,
	&tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,
	/* 0xb700 */
	&tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,
	&tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,   &tms340x0_device::move1_no_r_a,   &tms340x0_device::move1_no_r_b,
	/* 0xb800 */
	&tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,
	&tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,
	/* 0xb900 */
	&tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,
	&tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,  &tms340x0_device::move0_no_no_a,  &tms340x0_device::move0_no_no_b,
	/* 0xba00 */
	&tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,
	&tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,
	/* 0xbb00 */
	&tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,
	&tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,  &tms340x0_device::move1_no_no_a,  &tms340x0_device::move1_no_no_b,
	/* 0xbc00 */
	&tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,
	&tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,
	/* 0xbd00 */
	&tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,
	&tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,   &tms340x0_device::movb_no_no_a,   &tms340x0_device::movb_no_no_b,
	/* 0xbe00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xbf00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xc000 */
	&tms340x0_device::j_UC_0,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,
	&tms340x0_device::j_UC_8,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,     &tms340x0_device::j_UC_x,
	/* 0xc100 */
	&tms340x0_device::j_P_0,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,
	&tms340x0_device::j_P_8,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,      &tms340x0_device::j_P_x,
	/* 0xc200 */
	&tms340x0_device::j_LS_0,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,
	&tms340x0_device::j_LS_8,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,     &tms340x0_device::j_LS_x,
	/* 0xc300 */
	&tms340x0_device::j_HI_0,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,
	&tms340x0_device::j_HI_8,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,     &tms340x0_device::j_HI_x,
	/* 0xc400 */
	&tms340x0_device::j_LT_0,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,
	&tms340x0_device::j_LT_8,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,     &tms340x0_device::j_LT_x,
	/* 0xc500 */
	&tms340x0_device::j_GE_0,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,
	&tms340x0_device::j_GE_8,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,     &tms340x0_device::j_GE_x,
	/* 0xc600 */
	&tms340x0_device::j_LE_0,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,
	&tms340x0_device::j_LE_8,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,     &tms340x0_device::j_LE_x,
	/* 0xc700 */
	&tms340x0_device::j_GT_0,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,
	&tms340x0_device::j_GT_8,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,     &tms340x0_device::j_GT_x,
	/* 0xc800 */
	&tms340x0_device::j_C_0,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,
	&tms340x0_device::j_C_8,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,      &tms340x0_device::j_C_x,
	/* 0xc900 */
	&tms340x0_device::j_NC_0,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,
	&tms340x0_device::j_NC_8,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,     &tms340x0_device::j_NC_x,
	/* 0xca00 */
	&tms340x0_device::j_EQ_0,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,
	&tms340x0_device::j_EQ_8,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,     &tms340x0_device::j_EQ_x,
	/* 0xcb00 */
	&tms340x0_device::j_NE_0,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,
	&tms340x0_device::j_NE_8,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,     &tms340x0_device::j_NE_x,
	/* 0xcc00 */
	&tms340x0_device::j_V_0,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,
	&tms340x0_device::j_V_8,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,      &tms340x0_device::j_V_x,
	/* 0xcd00 */
	&tms340x0_device::j_NV_0,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,
	&tms340x0_device::j_NV_8,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,     &tms340x0_device::j_NV_x,
	/* 0xce00 */
	&tms340x0_device::j_N_0,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,
	&tms340x0_device::j_N_8,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,      &tms340x0_device::j_N_x,
	/* 0xcf00 */
	&tms340x0_device::j_NN_0,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,
	&tms340x0_device::j_NN_8,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,     &tms340x0_device::j_NN_x,
	/* 0xd000 */
	&tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,
	&tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,
	/* 0xd100 */
	&tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,
	&tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,  &tms340x0_device::move0_no_ni_a,  &tms340x0_device::move0_no_ni_b,
	/* 0xd200 */
	&tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,
	&tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,
	/* 0xd300 */
	&tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,
	&tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,  &tms340x0_device::move1_no_ni_a,  &tms340x0_device::move1_no_ni_b,
	/* 0xd400 */
	&tms340x0_device::move0_a_ni_a,&tms340x0_device::move0_a_ni_b,&tms340x0_device::unimpl,   &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xd500 */
	&tms340x0_device::exgf0_a,    &tms340x0_device::exgf0_b,    &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xd600 */
	&tms340x0_device::move1_a_ni_a,&tms340x0_device::move1_a_ni_b,&tms340x0_device::unimpl,   &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xd700 */
	&tms340x0_device::exgf1_a,    &tms340x0_device::exgf1_b,    &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xd800 */
	&tms340x0_device::cexec_s,    &tms340x0_device::cexec_s,    &tms340x0_device::cexec_s,    &tms340x0_device::cexec_s,    &tms340x0_device::cexec_s,    &tms340x0_device::cexec_s,    &tms340x0_device::cexec_s,    &tms340x0_device::cexec_s,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xd900 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xda00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xdb00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xdc00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xdd00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xde00 */
	&tms340x0_device::unimpl,     &tms340x0_device::fline,      &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::fline,      &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xdf00 */
	&tms340x0_device::unimpl,     &tms340x0_device::line,       &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::line,       &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xe000 */
	&tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,
	&tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,
	/* 0xe100 */
	&tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,
	&tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,   &tms340x0_device::add_xy_a,   &tms340x0_device::add_xy_b,
	/* 0xe200 */
	&tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,
	&tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,
	/* 0xe300 */
	&tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,
	&tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,   &tms340x0_device::sub_xy_a,   &tms340x0_device::sub_xy_b,
	/* 0xe400 */
	&tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,
	&tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,
	/* 0xe500 */
	&tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,
	&tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,   &tms340x0_device::cmp_xy_a,   &tms340x0_device::cmp_xy_b,
	/* 0xe600 */
	&tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,
	&tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,
	/* 0xe700 */
	&tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,
	&tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,      &tms340x0_device::cpw_a,      &tms340x0_device::cpw_b,
	/* 0xe800 */
	&tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,
	&tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,
	/* 0xe900 */
	&tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,
	&tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,    &tms340x0_device::cvxyl_a,    &tms340x0_device::cvxyl_b,
	/* 0xea00 */
	&tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,
	&tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,
	/* 0xeb00 */
	&tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,
	&tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,   &tms340x0_device::cvsxyl_a,   &tms340x0_device::cvsxyl_b,
	/* 0xec00 */
	&tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,
	&tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,
	/* 0xed00 */
	&tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,
	&tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,     &tms340x0_device::movx_a,     &tms340x0_device::movx_b,
	/* 0xee00 */
	&tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,
	&tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,
	/* 0xef00 */
	&tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,
	&tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,     &tms340x0_device::movy_a,     &tms340x0_device::movy_b,
	/* 0xf000 */
	&tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,
	&tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,
	/* 0xf100 */
	&tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,
	&tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,    &tms340x0_device::pixt_rixy_a,    &tms340x0_device::pixt_rixy_b,
	/* 0xf200 */
	&tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,
	&tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,
	/* 0xf300 */
	&tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,
	&tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,    &tms340x0_device::pixt_ixyr_a,    &tms340x0_device::pixt_ixyr_b,
	/* 0xf400 */
	&tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,
	&tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,
	/* 0xf500 */
	&tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,
	&tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,  &tms340x0_device::pixt_ixyixy_a,  &tms340x0_device::pixt_ixyixy_b,
	/* 0xf600 */
	&tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,
	&tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,
	/* 0xf700 */
	&tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,
	&tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,     &tms340x0_device::drav_a,     &tms340x0_device::drav_b,
	/* 0xf800 */
	&tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,
	&tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,
	/* 0xf900 */
	&tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,
	&tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,  &tms340x0_device::pixt_ri_a,  &tms340x0_device::pixt_ri_b,
	/* 0xfa00 */
	&tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,
	&tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,
	/* 0xfb00 */
	&tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,
	&tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,  &tms340x0_device::pixt_ir_a,  &tms340x0_device::pixt_ir_b,
	/* 0xfc00 */
	&tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,
	&tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,
	/* 0xfd00 */
	&tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,
	&tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,  &tms340x0_device::pixt_ii_a,  &tms340x0_device::pixt_ii_b,
	/* 0xfe00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	/* 0xff00 */
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,
	&tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl,     &tms340x0_device::unimpl
};
