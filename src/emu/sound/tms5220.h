#pragma once

#ifndef __TMS5220_H__
#define __TMS5220_H__

void *tms5220_create(const device_config *device);
void tms5220_destroy(void *chip);

void tms5220_reset_chip(void *chip);
void tms5220_set_irq(void *chip, void (*func)(const device_config *, int));

void tms5220_data_write(void *chip, int data);
int tms5220_status_read(void *chip);
int tms5220_ready_read(void *chip);
int tms5220_cycles_to_ready(void *chip);
int tms5220_int_read(void *chip);

void tms5220_process(void *chip, INT16 *buffer, unsigned int size);

/* three variables added by R Nabet */
void tms5220_set_read(void *chip, int (*func)(const device_config *, int));
void tms5220_set_load_address(void *chip, void (*func)(const device_config *, int));
void tms5220_set_read_and_branch(void *chip, void (*func)(const device_config *));


enum _tms5220_variant
{
	variant_tms5220,	/* TMS5220_IS_TMS5220, TMS5220_IS_TMS5220C,  TMS5220_IS_TSP5220C */
	variant_tmc0285		/* TMS5220_IS_TMS5200, TMS5220_IS_CD2501 */
};
typedef enum _tms5220_variant tms5220_variant;

void tms5220_set_variant(void *chip, tms5220_variant new_variant);

#endif /* __TMS5220_H__ */
