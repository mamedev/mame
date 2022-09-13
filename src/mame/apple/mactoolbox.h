// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont

#ifndef MAME_APPLE_MACTOOLBOX_H
#define MAME_APPLE_MACTOOLBOX_H

#pragma once


// older versions of libc++ are missing deduction guides that the things using this require
// FIXME: find a better place to put this
#if defined(_LIBCPP_VERSION) && (_LIBCPP_VERSION < 10000)
namespace std { inline namespace __1 {
template<class R, class... ArgTypes > function( R(*)(ArgTypes...) ) -> function<R(ArgTypes...)>;
} }
#endif


extern offs_t mac68k_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

#endif // MAME_APPLE_MACTOOLBOX_H
