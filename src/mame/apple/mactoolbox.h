// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont

#ifndef MAME_APPLE_MACTOOLBOX_H
#define MAME_APPLE_MACTOOLBOX_H

#pragma once

extern offs_t mac68k_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

#endif // MAME_APPLE_MACTOOLBOX_H
