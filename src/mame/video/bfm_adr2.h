#ifndef INC_BFMADDER2
#define INC_BFMADDER2

int adder2_receive(void);
void adder2_send(int data);
int adder2_status(void);

void adder2_decode_char_roms(running_machine &machine);
MACHINE_CONFIG_EXTERN(adder2);
#endif
