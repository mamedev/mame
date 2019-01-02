// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    devcb.cpp

    Device callback interface helpers.

***************************************************************************/

#include "emu.h"
#include "devcb.h"


template class devcb_read<u8>;
template class devcb_read<u16>;
template class devcb_read<u32>;
template class devcb_read<u64>;
template class devcb_read<int, 1U>;

template class devcb_read8::delegate_builder<read8_delegate>;
template class devcb_read8::delegate_builder<read16_delegate>;
template class devcb_read8::delegate_builder<read32_delegate>;
template class devcb_read8::delegate_builder<read64_delegate>;
template class devcb_read8::delegate_builder<read8s_delegate>;
template class devcb_read8::delegate_builder<read16s_delegate>;
template class devcb_read8::delegate_builder<read32s_delegate>;
template class devcb_read8::delegate_builder<read64s_delegate>;
template class devcb_read8::delegate_builder<read8sm_delegate>;
template class devcb_read8::delegate_builder<read16sm_delegate>;
template class devcb_read8::delegate_builder<read32sm_delegate>;
template class devcb_read8::delegate_builder<read64sm_delegate>;
template class devcb_read8::delegate_builder<read8smo_delegate>;
template class devcb_read8::delegate_builder<read16smo_delegate>;
template class devcb_read8::delegate_builder<read32smo_delegate>;
template class devcb_read8::delegate_builder<read64smo_delegate>;
template class devcb_read8::delegate_builder<read_line_delegate>;

template class devcb_read16::delegate_builder<read8_delegate>;
template class devcb_read16::delegate_builder<read16_delegate>;
template class devcb_read16::delegate_builder<read32_delegate>;
template class devcb_read16::delegate_builder<read64_delegate>;
template class devcb_read16::delegate_builder<read8s_delegate>;
template class devcb_read16::delegate_builder<read16s_delegate>;
template class devcb_read16::delegate_builder<read32s_delegate>;
template class devcb_read16::delegate_builder<read64s_delegate>;
template class devcb_read16::delegate_builder<read8sm_delegate>;
template class devcb_read16::delegate_builder<read16sm_delegate>;
template class devcb_read16::delegate_builder<read32sm_delegate>;
template class devcb_read16::delegate_builder<read64sm_delegate>;
template class devcb_read16::delegate_builder<read8smo_delegate>;
template class devcb_read16::delegate_builder<read16smo_delegate>;
template class devcb_read16::delegate_builder<read32smo_delegate>;
template class devcb_read16::delegate_builder<read64smo_delegate>;
template class devcb_read16::delegate_builder<read_line_delegate>;

template class devcb_read32::delegate_builder<read8_delegate>;
template class devcb_read32::delegate_builder<read16_delegate>;
template class devcb_read32::delegate_builder<read32_delegate>;
template class devcb_read32::delegate_builder<read64_delegate>;
template class devcb_read32::delegate_builder<read8s_delegate>;
template class devcb_read32::delegate_builder<read16s_delegate>;
template class devcb_read32::delegate_builder<read32s_delegate>;
template class devcb_read32::delegate_builder<read64s_delegate>;
template class devcb_read32::delegate_builder<read8sm_delegate>;
template class devcb_read32::delegate_builder<read16sm_delegate>;
template class devcb_read32::delegate_builder<read32sm_delegate>;
template class devcb_read32::delegate_builder<read64sm_delegate>;
template class devcb_read32::delegate_builder<read8smo_delegate>;
template class devcb_read32::delegate_builder<read16smo_delegate>;
template class devcb_read32::delegate_builder<read32smo_delegate>;
template class devcb_read32::delegate_builder<read64smo_delegate>;
template class devcb_read32::delegate_builder<read_line_delegate>;

template class devcb_read64::delegate_builder<read8_delegate>;
template class devcb_read64::delegate_builder<read16_delegate>;
template class devcb_read64::delegate_builder<read32_delegate>;
template class devcb_read64::delegate_builder<read64_delegate>;
template class devcb_read64::delegate_builder<read8s_delegate>;
template class devcb_read64::delegate_builder<read16s_delegate>;
template class devcb_read64::delegate_builder<read32s_delegate>;
template class devcb_read64::delegate_builder<read64s_delegate>;
template class devcb_read64::delegate_builder<read8sm_delegate>;
template class devcb_read64::delegate_builder<read16sm_delegate>;
template class devcb_read64::delegate_builder<read32sm_delegate>;
template class devcb_read64::delegate_builder<read64sm_delegate>;
template class devcb_read64::delegate_builder<read8smo_delegate>;
template class devcb_read64::delegate_builder<read16smo_delegate>;
template class devcb_read64::delegate_builder<read32smo_delegate>;
template class devcb_read64::delegate_builder<read64smo_delegate>;
template class devcb_read64::delegate_builder<read_line_delegate>;

template class devcb_read_line::delegate_builder<read8_delegate>;
template class devcb_read_line::delegate_builder<read16_delegate>;
template class devcb_read_line::delegate_builder<read32_delegate>;
template class devcb_read_line::delegate_builder<read64_delegate>;
template class devcb_read_line::delegate_builder<read8s_delegate>;
template class devcb_read_line::delegate_builder<read16s_delegate>;
template class devcb_read_line::delegate_builder<read32s_delegate>;
template class devcb_read_line::delegate_builder<read64s_delegate>;
template class devcb_read_line::delegate_builder<read8sm_delegate>;
template class devcb_read_line::delegate_builder<read16sm_delegate>;
template class devcb_read_line::delegate_builder<read32sm_delegate>;
template class devcb_read_line::delegate_builder<read64sm_delegate>;
template class devcb_read_line::delegate_builder<read8smo_delegate>;
template class devcb_read_line::delegate_builder<read16smo_delegate>;
template class devcb_read_line::delegate_builder<read32smo_delegate>;
template class devcb_read_line::delegate_builder<read64smo_delegate>;
template class devcb_read_line::delegate_builder<read_line_delegate>;

template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read8_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read16_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read32_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read64_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read8s_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read16s_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read32s_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read64s_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read8sm_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read16sm_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read32sm_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read64sm_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read8smo_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read16smo_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read32smo_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read64smo_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read_line_delegate> >;
template class devcb_read8::creator_impl<devcb_read8::ioport_builder>;

template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read8_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read16_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read32_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read64_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read8s_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read16s_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read32s_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read64s_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read8sm_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read16sm_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read32sm_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read64sm_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read8smo_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read16smo_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read32smo_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read64smo_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read_line_delegate> >;
template class devcb_read16::creator_impl<devcb_read16::ioport_builder>;

template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read8_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read16_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read32_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read64_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read8s_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read16s_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read32s_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read64s_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read8sm_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read16sm_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read32sm_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read64sm_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read8smo_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read16smo_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read32smo_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read64smo_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read_line_delegate> >;
template class devcb_read32::creator_impl<devcb_read32::ioport_builder>;

template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read8_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read16_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read32_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read64_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read8s_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read16s_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read32s_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read64s_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read8sm_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read16sm_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read32sm_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read64sm_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read8smo_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read16smo_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read32smo_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read64smo_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read_line_delegate> >;
template class devcb_read64::creator_impl<devcb_read64::ioport_builder>;

template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read8_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read16_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read32_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read64_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read8s_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read16s_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read32s_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read64s_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read8sm_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read16sm_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read32sm_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read64sm_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read8smo_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read16smo_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read32smo_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read64smo_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read_line_delegate> >;
template class devcb_read_line::creator_impl<devcb_read_line::ioport_builder>;

template class devcb_write<u8>;
template class devcb_write<u16>;
template class devcb_write<u32>;
template class devcb_write<u64>;
template class devcb_write<int, 1U>;

template class devcb_write8::delegate_builder<write8_delegate>;
template class devcb_write8::delegate_builder<write16_delegate>;
template class devcb_write8::delegate_builder<write32_delegate>;
template class devcb_write8::delegate_builder<write64_delegate>;
template class devcb_write8::delegate_builder<write8s_delegate>;
template class devcb_write8::delegate_builder<write16s_delegate>;
template class devcb_write8::delegate_builder<write32s_delegate>;
template class devcb_write8::delegate_builder<write64s_delegate>;
template class devcb_write8::delegate_builder<write8sm_delegate>;
template class devcb_write8::delegate_builder<write16sm_delegate>;
template class devcb_write8::delegate_builder<write32sm_delegate>;
template class devcb_write8::delegate_builder<write64sm_delegate>;
template class devcb_write8::delegate_builder<write8smo_delegate>;
template class devcb_write8::delegate_builder<write16smo_delegate>;
template class devcb_write8::delegate_builder<write32smo_delegate>;
template class devcb_write8::delegate_builder<write64smo_delegate>;
template class devcb_write8::delegate_builder<write_line_delegate>;

template class devcb_write16::delegate_builder<write8_delegate>;
template class devcb_write16::delegate_builder<write16_delegate>;
template class devcb_write16::delegate_builder<write32_delegate>;
template class devcb_write16::delegate_builder<write64_delegate>;
template class devcb_write16::delegate_builder<write8s_delegate>;
template class devcb_write16::delegate_builder<write16s_delegate>;
template class devcb_write16::delegate_builder<write32s_delegate>;
template class devcb_write16::delegate_builder<write64s_delegate>;
template class devcb_write16::delegate_builder<write8sm_delegate>;
template class devcb_write16::delegate_builder<write16sm_delegate>;
template class devcb_write16::delegate_builder<write32sm_delegate>;
template class devcb_write16::delegate_builder<write64sm_delegate>;
template class devcb_write16::delegate_builder<write8smo_delegate>;
template class devcb_write16::delegate_builder<write16smo_delegate>;
template class devcb_write16::delegate_builder<write32smo_delegate>;
template class devcb_write16::delegate_builder<write64smo_delegate>;
template class devcb_write16::delegate_builder<write_line_delegate>;

template class devcb_write32::delegate_builder<write8_delegate>;
template class devcb_write32::delegate_builder<write16_delegate>;
template class devcb_write32::delegate_builder<write32_delegate>;
template class devcb_write32::delegate_builder<write64_delegate>;
template class devcb_write32::delegate_builder<write8s_delegate>;
template class devcb_write32::delegate_builder<write16s_delegate>;
template class devcb_write32::delegate_builder<write32s_delegate>;
template class devcb_write32::delegate_builder<write64s_delegate>;
template class devcb_write32::delegate_builder<write8sm_delegate>;
template class devcb_write32::delegate_builder<write16sm_delegate>;
template class devcb_write32::delegate_builder<write32sm_delegate>;
template class devcb_write32::delegate_builder<write64sm_delegate>;
template class devcb_write32::delegate_builder<write8smo_delegate>;
template class devcb_write32::delegate_builder<write16smo_delegate>;
template class devcb_write32::delegate_builder<write32smo_delegate>;
template class devcb_write32::delegate_builder<write64smo_delegate>;
template class devcb_write32::delegate_builder<write_line_delegate>;

template class devcb_write64::delegate_builder<write8_delegate>;
template class devcb_write64::delegate_builder<write16_delegate>;
template class devcb_write64::delegate_builder<write32_delegate>;
template class devcb_write64::delegate_builder<write64_delegate>;
template class devcb_write64::delegate_builder<write8s_delegate>;
template class devcb_write64::delegate_builder<write16s_delegate>;
template class devcb_write64::delegate_builder<write32s_delegate>;
template class devcb_write64::delegate_builder<write64s_delegate>;
template class devcb_write64::delegate_builder<write8sm_delegate>;
template class devcb_write64::delegate_builder<write16sm_delegate>;
template class devcb_write64::delegate_builder<write32sm_delegate>;
template class devcb_write64::delegate_builder<write64sm_delegate>;
template class devcb_write64::delegate_builder<write8smo_delegate>;
template class devcb_write64::delegate_builder<write16smo_delegate>;
template class devcb_write64::delegate_builder<write32smo_delegate>;
template class devcb_write64::delegate_builder<write64smo_delegate>;
template class devcb_write64::delegate_builder<write_line_delegate>;

template class devcb_write_line::delegate_builder<write8_delegate>;
template class devcb_write_line::delegate_builder<write16_delegate>;
template class devcb_write_line::delegate_builder<write32_delegate>;
template class devcb_write_line::delegate_builder<write64_delegate>;
template class devcb_write_line::delegate_builder<write8s_delegate>;
template class devcb_write_line::delegate_builder<write16s_delegate>;
template class devcb_write_line::delegate_builder<write32s_delegate>;
template class devcb_write_line::delegate_builder<write64s_delegate>;
template class devcb_write_line::delegate_builder<write8sm_delegate>;
template class devcb_write_line::delegate_builder<write16sm_delegate>;
template class devcb_write_line::delegate_builder<write32sm_delegate>;
template class devcb_write_line::delegate_builder<write64sm_delegate>;
template class devcb_write_line::delegate_builder<write8smo_delegate>;
template class devcb_write_line::delegate_builder<write16smo_delegate>;
template class devcb_write_line::delegate_builder<write32smo_delegate>;
template class devcb_write_line::delegate_builder<write64smo_delegate>;
template class devcb_write_line::delegate_builder<write_line_delegate>;

template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write8_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write16_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write32_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write64_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write8s_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write16s_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write32s_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write64s_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write8sm_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write16sm_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write32sm_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write64sm_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write8smo_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write16smo_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write32smo_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write64smo_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write_line_delegate> >;
template class devcb_write8::creator_impl<devcb_write8::inputline_builder>;
template class devcb_write8::creator_impl<devcb_write8::latched_inputline_builder>;
template class devcb_write8::creator_impl<devcb_write8::ioport_builder>;
template class devcb_write8::creator_impl<devcb_write8::membank_builder>;
template class devcb_write8::creator_impl<devcb_write8::output_builder>;
template class devcb_write8::creator_impl<devcb_write8::log_builder>;

template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write8_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write16_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write32_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write64_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write8s_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write16s_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write32s_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write64s_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write8sm_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write16sm_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write32sm_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write64sm_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write8smo_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write16smo_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write32smo_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write64smo_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write_line_delegate> >;
template class devcb_write16::creator_impl<devcb_write16::inputline_builder>;
template class devcb_write16::creator_impl<devcb_write16::latched_inputline_builder>;
template class devcb_write16::creator_impl<devcb_write16::ioport_builder>;
template class devcb_write16::creator_impl<devcb_write16::membank_builder>;
template class devcb_write16::creator_impl<devcb_write16::output_builder>;
template class devcb_write16::creator_impl<devcb_write16::log_builder>;

template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write8_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write16_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write32_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write64_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write8s_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write16s_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write32s_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write64s_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write8sm_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write16sm_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write32sm_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write64sm_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write8smo_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write16smo_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write32smo_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write64smo_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write_line_delegate> >;
template class devcb_write32::creator_impl<devcb_write32::inputline_builder>;
template class devcb_write32::creator_impl<devcb_write32::latched_inputline_builder>;
template class devcb_write32::creator_impl<devcb_write32::ioport_builder>;
template class devcb_write32::creator_impl<devcb_write32::membank_builder>;
template class devcb_write32::creator_impl<devcb_write32::output_builder>;
template class devcb_write32::creator_impl<devcb_write32::log_builder>;

template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write8_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write16_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write32_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write64_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write8s_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write16s_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write32s_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write64s_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write8sm_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write16sm_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write32sm_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write64sm_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write8smo_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write16smo_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write32smo_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write64smo_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write_line_delegate> >;
template class devcb_write64::creator_impl<devcb_write64::inputline_builder>;
template class devcb_write64::creator_impl<devcb_write64::latched_inputline_builder>;
template class devcb_write64::creator_impl<devcb_write64::ioport_builder>;
template class devcb_write64::creator_impl<devcb_write64::membank_builder>;
template class devcb_write64::creator_impl<devcb_write64::output_builder>;
template class devcb_write64::creator_impl<devcb_write64::log_builder>;

template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write8_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write16_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write32_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write64_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write8s_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write16s_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write32s_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write64s_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write8sm_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write16sm_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write32sm_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write64sm_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write8smo_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write16smo_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write32smo_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write64smo_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write_line_delegate> >;
template class devcb_write_line::creator_impl<devcb_write_line::inputline_builder>;
template class devcb_write_line::creator_impl<devcb_write_line::latched_inputline_builder>;
template class devcb_write_line::creator_impl<devcb_write_line::ioport_builder>;
template class devcb_write_line::creator_impl<devcb_write_line::membank_builder>;
template class devcb_write_line::creator_impl<devcb_write_line::output_builder>;
template class devcb_write_line::creator_impl<devcb_write_line::log_builder>;


devcb_base::devcb_base(device_t &owner)
	: m_owner(owner)
	, m_dflt_space_finder(owner, finder_base::DUMMY_TAG)
	, m_dflt_space_index(-1)
	, m_dflt_space(nullptr)
{
	owner.register_callback(*this);
}


devcb_base::~devcb_base()
{
}


void devcb_base::validity_check(validity_checker &valid) const
{
	// if a default address space is configured, ensure it exists
	if (m_dflt_space_finder)
	{
		if (!m_dflt_space_finder->has_space(m_dflt_space_index))
			osd_printf_error("Callback references non-existent memory space %d of device %s (%s)\n", m_dflt_space_index, m_dflt_space_finder->device().tag(), m_dflt_space_finder->device().name());
	}
	else if (m_dflt_space_finder.finder_tag() != finder_base::DUMMY_TAG)
	{
		osd_printf_error("Callback references non-existent memory space device tag %s\n", m_dflt_space_finder.finder_tag());
	}
}


void devcb_base::resolve()
{
	if (m_dflt_space_finder && m_dflt_space_finder->has_space(m_dflt_space_index))
		m_dflt_space = &m_dflt_space_finder->space(m_dflt_space_index);
	else
		m_dflt_space = &m_owner.machine().dummy_space();
}


devcb_read_base::~devcb_read_base()
{
}


devcb_write_base::~devcb_write_base()
{
}
