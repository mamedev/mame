#ifndef eeprom_H
#define eeprom_H

typedef struct _eeprom_interface eeprom_interface;
struct _eeprom_interface
{
	int address_bits;	/* EEPROM has 2^address_bits cells */
	int data_bits;		/* every cell has this many bits (8 or 16) */
	const char *cmd_read;	/*   read command string, e.g. "0110" */
	const char *cmd_write;	/*  write command string, e.g. "0111" */
	const char *cmd_erase;	/*  erase command string, or 0 if n/a */
	const char *cmd_lock;	/*   lock command string, or 0 if n/a */
	const char *cmd_unlock;	/* unlock command string, or 0 if n/a */
	int enable_multi_read;/* set to 1 to enable multiple values to be read from one read command */
	int reset_delay;	/* number of times eeprom_read_bit() should return 0 after a reset, */
						/* before starting to return 1. */
};


void eeprom_init(const eeprom_interface *interface);

void eeprom_write_bit(int bit);
int eeprom_read_bit(void);
CUSTOM_INPUT( eeprom_bit_r );
void eeprom_set_cs_line(int state);
void eeprom_set_clock_line(int state);

void eeprom_load(mame_file *file);
void eeprom_save(mame_file *file);

void eeprom_set_data(const UINT8 *data, int length);
UINT8 * eeprom_get_data_pointer(int * length);

/* 93C46 */
extern const eeprom_interface eeprom_interface_93C46;
NVRAM_HANDLER( 93C46 );

/* 93C66B */
extern const eeprom_interface eeprom_interface_93C66B;
NVRAM_HANDLER( 93C66B );

#endif
