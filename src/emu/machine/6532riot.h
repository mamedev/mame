/***************************************************************************

  RIOT 6532 emulation

***************************************************************************/

#ifndef RIOT_6532
#define RIOT_6532

#define	MAX_R6532	8

struct riot6532_interface
{
	read8_machine_func in_a_func;
	read8_machine_func in_b_func;
	write8_machine_func out_a_func;
	write8_machine_func out_b_func;
	void (*irq_func)(running_machine *machine, int state);
};

void r6532_set_clock(int which, int clock);
void r6532_reset(int which);
void r6532_config(int which, const struct riot6532_interface* intf);
UINT8 r6532_read(int which, offs_t offset);
void r6532_write(int which, offs_t offset, UINT8 data);
void r6532_set_input_a(int which, UINT8 data);
void r6532_set_input_b(int which, UINT8 data);

/******************* Standard 8-bit CPU interfaces, D0-D7 *******************/

READ8_HANDLER( r6532_0_r );
READ8_HANDLER( r6532_1_r );
READ8_HANDLER( r6532_2_r );
READ8_HANDLER( r6532_3_r );
READ8_HANDLER( r6532_4_r );
READ8_HANDLER( r6532_5_r );
READ8_HANDLER( r6532_6_r );
READ8_HANDLER( r6532_7_r );

WRITE8_HANDLER( r6532_0_w );
WRITE8_HANDLER( r6532_1_w );
WRITE8_HANDLER( r6532_2_w );
WRITE8_HANDLER( r6532_3_w );
WRITE8_HANDLER( r6532_4_w );
WRITE8_HANDLER( r6532_5_w );
WRITE8_HANDLER( r6532_6_w );
WRITE8_HANDLER( r6532_7_w );

/******************* 8-bit A/B port interfaces *******************/

WRITE8_HANDLER( r6532_0_porta_w );
WRITE8_HANDLER( r6532_1_porta_w );
WRITE8_HANDLER( r6532_2_porta_w );
WRITE8_HANDLER( r6532_3_porta_w );
WRITE8_HANDLER( r6532_4_porta_w );
WRITE8_HANDLER( r6532_5_porta_w );
WRITE8_HANDLER( r6532_6_porta_w );
WRITE8_HANDLER( r6532_7_porta_w );

WRITE8_HANDLER( r6532_0_portb_w );
WRITE8_HANDLER( r6532_1_portb_w );
WRITE8_HANDLER( r6532_2_portb_w );
WRITE8_HANDLER( r6532_3_portb_w );
WRITE8_HANDLER( r6532_4_portb_w );
WRITE8_HANDLER( r6532_5_portb_w );
WRITE8_HANDLER( r6532_6_portb_w );
WRITE8_HANDLER( r6532_7_portb_w );

READ8_HANDLER( r6532_0_porta_r );
READ8_HANDLER( r6532_1_porta_r );
READ8_HANDLER( r6532_2_porta_r );
READ8_HANDLER( r6532_3_porta_r );
READ8_HANDLER( r6532_4_porta_r );
READ8_HANDLER( r6532_5_porta_r );
READ8_HANDLER( r6532_6_porta_r );
READ8_HANDLER( r6532_7_porta_r );

READ8_HANDLER( r6532_0_portb_r );
READ8_HANDLER( r6532_1_portb_r );
READ8_HANDLER( r6532_2_portb_r );
READ8_HANDLER( r6532_3_portb_r );
READ8_HANDLER( r6532_4_portb_r );
READ8_HANDLER( r6532_5_portb_r );
READ8_HANDLER( r6532_6_portb_r );
READ8_HANDLER( r6532_7_portb_r );

#endif
