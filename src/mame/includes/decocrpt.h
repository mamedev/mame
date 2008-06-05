/*----------- defined in machine/decocrpt.c -----------*/

void deco56_decrypt(int region);
void deco74_decrypt(int region);
void deco56_remap(int region);


/*----------- defined in machine/deco102.c -----------*/

void deco102_decrypt(int region, int address_xor, int data_select_xor, int opcode_select_xor);


/*----------- defined in machine/deco156.c -----------*/

void deco156_decrypt(void);
