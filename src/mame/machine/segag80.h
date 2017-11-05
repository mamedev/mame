// license:BSD-3-Clause
// copyright-holders:Aaron Giles
typedef uint8_t (*segag80_decrypt_func)(offs_t, uint8_t);

segag80_decrypt_func segag80_security(int chip);
