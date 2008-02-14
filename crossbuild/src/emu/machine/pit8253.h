/*****************************************************************************
 *
 *  Programmable Interval Timer 8253/8254
 *
 *****************************************************************************/

#ifndef PIT8253_H
#define PIT8253_H

typedef enum { TYPE8253, TYPE8254 } PIT8253_TYPE;

struct pit8253_config
{
    PIT8253_TYPE type;
	struct
	{
		double clockin;
		void (*output_callback)(int state);
		void (*clock_callback)(double clockout);
	} timer[3];
};



int pit8253_init(int count, const struct pit8253_config *config);
void pit8253_reset(int which);

READ8_HANDLER ( pit8253_0_r );
READ8_HANDLER ( pit8253_1_r );
WRITE8_HANDLER ( pit8253_0_w );
WRITE8_HANDLER ( pit8253_1_w );

READ16_HANDLER ( pit8253_0_lsb_r );
READ16_HANDLER ( pit8253_1_lsb_r );
WRITE16_HANDLER ( pit8253_0_lsb_w );
WRITE16_HANDLER ( pit8253_1_lsb_w );

READ16_HANDLER ( pit8253_16le_0_r );
READ16_HANDLER ( pit8253_16le_1_r );
WRITE16_HANDLER ( pit8253_16le_0_w );
WRITE16_HANDLER ( pit8253_16le_1_w );

READ32_HANDLER ( pit8253_32le_0_r );
READ32_HANDLER ( pit8253_32le_1_r );
WRITE32_HANDLER ( pit8253_32le_0_w );
WRITE32_HANDLER ( pit8253_32le_1_w );

READ64_HANDLER ( pit8253_64be_0_r );
READ64_HANDLER ( pit8253_64be_1_r );
WRITE64_HANDLER ( pit8253_64be_0_w );
WRITE64_HANDLER ( pit8253_64be_1_w );

WRITE8_HANDLER ( pit8253_0_gate_w );
WRITE8_HANDLER ( pit8253_1_gate_w );

int pit8253_get_frequency(int which, int timer);
int pit8253_get_output(int which, int timer);
void pit8253_set_clockin(int which, int timer, double new_clockin);


#endif	/* PIT8253_H */

