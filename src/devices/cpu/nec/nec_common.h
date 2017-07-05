#ifndef NEC_COMMON_H
#define NEC_COMMON_H

#include <stdint.h>

extern int necv_dasm_one(std::ostream &stream, uint32_t eip, const device_disasm_interface::data_buffer &opcodes, const device_disasm_interface::data_buffer &params, const uint8_t *decryption_table);

#endif // NEC_COMMON_H
