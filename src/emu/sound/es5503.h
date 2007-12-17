#ifndef _ES5503_H_
#define _ES5503_H_

struct ES5503interface
{
	void (*irq_callback)(int state);
	read8_handler adc_read;
	UINT8 *wave_memory;
};

READ8_HANDLER(ES5503_reg_0_r);
WRITE8_HANDLER(ES5503_reg_0_w);
void ES5503_set_base_0(UINT8 *wavemem);

#endif
