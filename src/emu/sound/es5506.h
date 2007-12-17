/**********************************************************************************************
 *
 *   Ensoniq ES5505/6 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#ifndef ES5506_H
#define ES5506_H

struct ES5505interface
{
	int region0;						/* memory region where the sample ROM lives */
	int region1;						/* memory region where the sample ROM lives */
	void (*irq_callback)(int state);	/* irq callback */
	UINT16 (*read_port)(void);			/* input port read */
};

READ16_HANDLER( ES5505_data_0_r );
READ16_HANDLER( ES5505_data_1_r );
WRITE16_HANDLER( ES5505_data_0_w );
WRITE16_HANDLER( ES5505_data_1_w );

void ES5505_voice_bank_0_w(int voice, int bank);
void ES5505_voice_bank_1_w(int voice, int bank);




struct ES5506interface
{
	int region0;						/* memory region where the sample ROM lives */
	int region1;						/* memory region where the sample ROM lives */
	int region2;						/* memory region where the sample ROM lives */
	int region3;						/* memory region where the sample ROM lives */
	void (*irq_callback)(int state);	/* irq callback */
	UINT16 (*read_port)(void);			/* input port read */
};

READ8_HANDLER( ES5506_data_0_r );
READ8_HANDLER( ES5506_data_1_r );
WRITE8_HANDLER( ES5506_data_0_w );
WRITE8_HANDLER( ES5506_data_1_w );

READ16_HANDLER( ES5506_data_0_word_r );
READ16_HANDLER( ES5506_data_1_word_r );
WRITE16_HANDLER( ES5506_data_0_word_w );
WRITE16_HANDLER( ES5506_data_1_word_w );

void ES5506_voice_bank_0_w(int voice, int bank);
void ES5506_voice_bank_1_w(int voice, int bank);

#endif
