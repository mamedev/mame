#ifndef tms5220_h
#define tms5220_h

void *tms5220_create(int index);
void tms5220_destroy(void *chip);

void tms5220_reset_chip(void *chip);
void tms5220_set_irq(void *chip, void (*func)(int));

void tms5220_data_write(void *chip, int data);
int tms5220_status_read(void *chip);
int tms5220_ready_read(void *chip);
int tms5220_cycles_to_ready(void *chip);
int tms5220_int_read(void *chip);

void tms5220_process(void *chip, INT16 *buffer, unsigned int size);

/* three variables added by R Nabet */
void tms5220_set_read(void *chip, int (*func)(int));
void tms5220_set_load_address(void *chip, void (*func)(int));
void tms5220_set_read_and_branch(void *chip, void (*func)(void));

typedef enum
{
	variant_tms5220,
	variant_tms0285
} tms5220_variant;

void tms5220_set_variant(void *chip, tms5220_variant new_variant);

#endif

