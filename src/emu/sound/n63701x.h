#ifndef namco_63701x_h
#define namco_63701x_h

struct namco_63701x_interface
{
	int region;			/* memory region; region MUST be 0x80000 bytes long */
};

void namco_63701x_write(int offset,int data);

#endif

