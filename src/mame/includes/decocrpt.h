/*----------- defined in machine/decocrpt.c -----------*/

void deco56_decrypt(running_machine *machine, int region);
void deco74_decrypt(running_machine *machine, int region);
void deco56_remap(running_machine *machine, int region);


/*----------- defined in machine/deco102.c -----------*/

void deco102_decrypt(running_machine *machine, int region, int address_xor, int data_select_xor, int opcode_select_xor);


/*----------- defined in machine/deco156.c -----------*/

void deco156_decrypt(running_machine *machine);
