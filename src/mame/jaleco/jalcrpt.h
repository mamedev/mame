// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
void jaleco_gs88000_rom_decode(running_machine &machine, const char *region);
void jaleco_d65006_rom_decode(running_machine &machine, const char *region);
void rodland_rom_decode (running_machine &machine, const char *region);

void decrypt_ms32_tx(running_machine &machine, int addr_xor,int data_xor, const char *region);
void decrypt_ms32_bg(running_machine &machine, int addr_xor,int data_xor, const char *region);
