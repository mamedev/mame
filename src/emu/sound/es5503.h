#pragma once

#ifndef __ES5503_H__
#define __ES5503_H__

typedef struct _es5503_interface es5503_interface;
struct _es5503_interface
{
	void (*irq_callback)(running_machine *machine, int state);
	read8_machine_func adc_read;
	UINT8 *wave_memory;
};

READ8_HANDLER(es5503_reg_0_r);
WRITE8_HANDLER(es5503_reg_0_w);
void es5503_set_base_0(UINT8 *wavemem);

#endif /* __ES5503_H__ */
