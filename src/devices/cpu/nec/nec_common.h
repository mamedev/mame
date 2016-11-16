#ifndef NEC_COMMON_H
#define NEC_COMMON_H

#include <stdint.h>

extern int necv_dasm_one(std::ostream &stream, uint32_t eip, const uint8_t *oprom, const uint8_t *decryption_table);
extern int necv_dasm_one(char *buffer, uint32_t eip, const uint8_t *oprom, const uint8_t *decryption_table);

#endif // NEC_COMMON_H
