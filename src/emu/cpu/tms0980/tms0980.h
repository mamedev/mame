#ifndef _TMS0980_H_
#define _TMS0980_H_


/* Registers */
enum {
	TMS0980_PC=1, TMS0980_SR, TMS0980_PA, TMS0980_PB,
	TMS0980_A, TMS0980_X, TMS0980_Y, TMS0980_STATUS
};


struct tms0980_config {
	/* O-output PLA configuration 5bit -> 8/11bit translation */
	UINT16			o_pla[0x20];
	devcb_read8		read_k;
	devcb_write16	write_o;		/* tms1270 has 10 O-outputs */
	devcb_write16	write_r;
};


/* 9-bit family */
DECLARE_LEGACY_CPU_DEVICE(TMS0980, tms0980);

/* 8-bit family */
DECLARE_LEGACY_CPU_DEVICE(TMS1000, tms1000);
DECLARE_LEGACY_CPU_DEVICE(TMS1070, tms1070);
DECLARE_LEGACY_CPU_DEVICE(TMS1100, tms1100);
DECLARE_LEGACY_CPU_DEVICE(TMS1200, tms1200);
DECLARE_LEGACY_CPU_DEVICE(TMS1270, tms1270);
DECLARE_LEGACY_CPU_DEVICE(TMS1300, tms1300);

extern CPU_DISASSEMBLE( tms0980 );
extern CPU_DISASSEMBLE( tms1000 );
extern CPU_DISASSEMBLE( tms1100 );

#endif /* _TMS0980_H_ */

