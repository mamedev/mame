/**********************************************************************************************
 *
 *   Ensoniq ES5505/6 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __ES5506_H__
#define __ES5506_H__

typedef struct _es5505_interface es5505_interface;
struct _es5505_interface
{
	const char * region0;						/* memory region where the sample ROM lives */
	const char * region1;						/* memory region where the sample ROM lives */
	void (*irq_callback)(running_machine *machine, int state);	/* irq callback */
	UINT16 (*read_port)(void);			/* input port read */
};

READ16_HANDLER( es5505_data_0_r );
READ16_HANDLER( es5505_data_1_r );
WRITE16_HANDLER( es5505_data_0_w );
WRITE16_HANDLER( es5505_data_1_w );

void es5505_voice_bank_0_w(int voice, int bank);
void es5505_voice_bank_1_w(int voice, int bank);

SND_GET_INFO( es5505 );


typedef struct _es5506_interface es5506_interface;
struct _es5506_interface
{
	const char * region0;						/* memory region where the sample ROM lives */
	const char * region1;						/* memory region where the sample ROM lives */
	const char * region2;						/* memory region where the sample ROM lives */
	const char * region3;						/* memory region where the sample ROM lives */
	void (*irq_callback)(running_machine *machine, int state);	/* irq callback */
	UINT16 (*read_port)(void);			/* input port read */
};

READ8_HANDLER( es5506_data_0_r );
READ8_HANDLER( es5506_data_1_r );
WRITE8_HANDLER( es5506_data_0_w );
WRITE8_HANDLER( es5506_data_1_w );

READ16_HANDLER( es5506_data_0_word_r );
READ16_HANDLER( es5506_data_1_word_r );
WRITE16_HANDLER( es5506_data_0_word_w );
WRITE16_HANDLER( es5506_data_1_word_w );

void es5506_voice_bank_0_w(int voice, int bank);
void es5506_voice_bank_1_w(int voice, int bank);

SND_GET_INFO( es5506 );

#endif /* __ES5506_H__ */
