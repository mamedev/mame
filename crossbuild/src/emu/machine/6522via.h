/**********************************************************************

    Rockwell 6522 VIA interface and emulation

    This function emulates all the functionality of up to 8 6522
    versatile interface adapters.

    This is based on the M6821 emulation in MAME.

    Written by Mathis Rosenhauer

**********************************************************************/

#ifndef VIA_6522
#define VIA_6522


#define MAX_VIA 8

#define	VIA_PB	    0
#define	VIA_PA	    1
#define	VIA_DDRB    2
#define	VIA_DDRA    3
#define	VIA_T1CL    4
#define	VIA_T1CH    5
#define	VIA_T1LL    6
#define	VIA_T1LH    7
#define	VIA_T2CL    8
#define	VIA_T2CH    9
#define	VIA_SR     10
#define	VIA_ACR    11
#define	VIA_PCR    12
#define	VIA_IFR    13
#define	VIA_IER    14
#define	VIA_PANH   15

struct via6522_interface
{
	read8_handler in_a_func;
	read8_handler in_b_func;
	read8_handler in_ca1_func;
	read8_handler in_cb1_func;
	read8_handler in_ca2_func;
	read8_handler in_cb2_func;
	write8_handler out_a_func;
	write8_handler out_b_func;
	write8_handler out_ca1_func;
	write8_handler out_cb1_func;
	write8_handler out_ca2_func;
	write8_handler out_cb2_func;
	void (*irq_func)(int state);
};

void via_set_clock(int which,int clck);
void via_config(int which, const struct via6522_interface *intf);
void via_reset(void);
int via_read(int which, int offset);
void via_write(int which, int offset, int data);
void via_set_input_a(int which, int data);
void via_set_input_ca1(int which, int data);
void via_set_input_ca2(int which, int data);
void via_set_input_b(int which, int data);
void via_set_input_cb1(int which, int data);
void via_set_input_cb2(int which, int data);

/******************* Standard 8-bit CPU interfaces, D0-D7 *******************/

READ8_HANDLER( via_0_r );
READ8_HANDLER( via_1_r );
READ8_HANDLER( via_2_r );
READ8_HANDLER( via_3_r );
READ8_HANDLER( via_4_r );
READ8_HANDLER( via_5_r );
READ8_HANDLER( via_6_r );
READ8_HANDLER( via_7_r );

WRITE8_HANDLER( via_0_w );
WRITE8_HANDLER( via_1_w );
WRITE8_HANDLER( via_2_w );
WRITE8_HANDLER( via_3_w );
WRITE8_HANDLER( via_4_w );
WRITE8_HANDLER( via_5_w );
WRITE8_HANDLER( via_6_w );
WRITE8_HANDLER( via_7_w );

/******************* 8-bit A/B port interfaces *******************/

WRITE8_HANDLER( via_0_porta_w );
WRITE8_HANDLER( via_1_porta_w );
WRITE8_HANDLER( via_2_porta_w );
WRITE8_HANDLER( via_3_porta_w );
WRITE8_HANDLER( via_4_porta_w );
WRITE8_HANDLER( via_5_porta_w );
WRITE8_HANDLER( via_6_porta_w );
WRITE8_HANDLER( via_7_porta_w );

WRITE8_HANDLER( via_0_portb_w );
WRITE8_HANDLER( via_1_portb_w );
WRITE8_HANDLER( via_2_portb_w );
WRITE8_HANDLER( via_3_portb_w );
WRITE8_HANDLER( via_4_portb_w );
WRITE8_HANDLER( via_5_portb_w );
WRITE8_HANDLER( via_6_portb_w );
WRITE8_HANDLER( via_7_portb_w );

READ8_HANDLER( via_0_porta_r );
READ8_HANDLER( via_1_porta_r );
READ8_HANDLER( via_2_porta_r );
READ8_HANDLER( via_3_porta_r );
READ8_HANDLER( via_4_porta_r );
READ8_HANDLER( via_5_porta_r );
READ8_HANDLER( via_6_porta_r );
READ8_HANDLER( via_7_porta_r );

READ8_HANDLER( via_0_portb_r );
READ8_HANDLER( via_1_portb_r );
READ8_HANDLER( via_2_portb_r );
READ8_HANDLER( via_3_portb_r );
READ8_HANDLER( via_4_portb_r );
READ8_HANDLER( via_5_portb_r );
READ8_HANDLER( via_6_portb_r );
READ8_HANDLER( via_7_portb_r );

/******************* 1-bit CA1/CA2/CB1/CB2 port interfaces *******************/

WRITE8_HANDLER( via_0_ca1_w );
WRITE8_HANDLER( via_1_ca1_w );
WRITE8_HANDLER( via_2_ca1_w );
WRITE8_HANDLER( via_3_ca1_w );
WRITE8_HANDLER( via_4_ca1_w );
WRITE8_HANDLER( via_5_ca1_w );
WRITE8_HANDLER( via_6_ca1_w );
WRITE8_HANDLER( via_7_ca1_w );
WRITE8_HANDLER( via_0_ca2_w );
WRITE8_HANDLER( via_1_ca2_w );
WRITE8_HANDLER( via_2_ca2_w );
WRITE8_HANDLER( via_3_ca2_w );
WRITE8_HANDLER( via_4_ca2_w );
WRITE8_HANDLER( via_5_ca2_w );
WRITE8_HANDLER( via_6_ca2_w );
WRITE8_HANDLER( via_7_ca2_w );

WRITE8_HANDLER( via_0_cb1_w );
WRITE8_HANDLER( via_1_cb1_w );
WRITE8_HANDLER( via_2_cb1_w );
WRITE8_HANDLER( via_3_cb1_w );
WRITE8_HANDLER( via_4_cb1_w );
WRITE8_HANDLER( via_5_cb1_w );
WRITE8_HANDLER( via_6_cb1_w );
WRITE8_HANDLER( via_7_cb1_w );
WRITE8_HANDLER( via_0_cb2_w );
WRITE8_HANDLER( via_1_cb2_w );
WRITE8_HANDLER( via_2_cb2_w );
WRITE8_HANDLER( via_3_cb2_w );
WRITE8_HANDLER( via_4_cb2_w );
WRITE8_HANDLER( via_5_cb2_w );
WRITE8_HANDLER( via_6_cb2_w );
WRITE8_HANDLER( via_7_cb2_w );

READ8_HANDLER( via_0_ca1_r );
READ8_HANDLER( via_1_ca1_r );
READ8_HANDLER( via_2_ca1_r );
READ8_HANDLER( via_3_ca1_r );
READ8_HANDLER( via_4_ca1_r );
READ8_HANDLER( via_5_ca1_r );
READ8_HANDLER( via_6_ca1_r );
READ8_HANDLER( via_7_ca1_r );
READ8_HANDLER( via_0_ca2_r );
READ8_HANDLER( via_1_ca2_r );
READ8_HANDLER( via_2_ca2_r );
READ8_HANDLER( via_3_ca2_r );
READ8_HANDLER( via_4_ca2_r );
READ8_HANDLER( via_5_ca2_r );
READ8_HANDLER( via_6_ca2_r );
READ8_HANDLER( via_7_ca2_r );

READ8_HANDLER( via_0_cb1_r );
READ8_HANDLER( via_1_cb1_r );
READ8_HANDLER( via_2_cb1_r );
READ8_HANDLER( via_3_cb1_r );
READ8_HANDLER( via_4_cb1_r );
READ8_HANDLER( via_5_cb1_r );
READ8_HANDLER( via_6_cb1_r );
READ8_HANDLER( via_7_cb1_r );
READ8_HANDLER( via_0_cb2_r );
READ8_HANDLER( via_1_cb2_r );
READ8_HANDLER( via_2_cb2_r );
READ8_HANDLER( via_3_cb2_r );
READ8_HANDLER( via_4_cb2_r );
READ8_HANDLER( via_5_cb2_r );
READ8_HANDLER( via_6_cb2_r );
READ8_HANDLER( via_7_cb2_r );

#endif

