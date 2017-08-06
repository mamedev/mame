// license:BSD-3-Clause
// copyright-holders:Ville Linde, Peter Ferrie

extern int i386_dasm_one(std::ostream &stream, uint32_t pc, const uint8_t *oprom, int mode);
extern int i386_dasm_one_ex(std::ostream &stream, uint64_t eip, const uint8_t *oprom, int mode);
