// this function assumes a fixed portion of ROM at 0000-7FFF, and
// an arbitrary amount of banks at 8000-BFFF.
// numbanks may be 0, meaning there is no ROM to decrypt at 8000-BFFF,
// or 1, meaning 0000-BFFF will be decrypted as a single unit.
void mc8123_decrypt_rom(int cpunum, const UINT8* key, int banknum, int numbanks);
