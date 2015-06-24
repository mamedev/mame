// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*----------- defined in machine/decocrpt.c -----------*/

void deco56_decrypt_gfx(running_machine &machine, const char *tag);
void deco74_decrypt_gfx(running_machine &machine, const char *tag);
void deco56_remap_gfx(running_machine &machine, const char *tag);


/*----------- defined in machine/deco102.c -----------*/

void deco102_decrypt_cpu(UINT16 *rom, UINT16 *opcodes, int size, int address_xor, int data_select_xor, int opcode_select_xor);


/*----------- defined in machine/deco156.c -----------*/

void deco156_decrypt(running_machine &machine);
