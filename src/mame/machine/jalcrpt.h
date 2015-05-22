// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
void astyanax_rom_decode(running_machine &machine, const char *region);
void phantasm_rom_decode(running_machine &machine, const char *region);
void rodland_rom_decode (running_machine &machine, const char *region);

void ms32_rearrange_sprites(running_machine &machine, const char *region);
void decrypt_ms32_tx(running_machine &machine, int addr_xor,int data_xor, const char *region);
void decrypt_ms32_bg(running_machine &machine, int addr_xor,int data_xor, const char *region);
