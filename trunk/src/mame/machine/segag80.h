typedef UINT8 (*segag80_decrypt_func)(offs_t, UINT8);

segag80_decrypt_func segag80_security(int chip);
