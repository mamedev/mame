/*
 * Serial eeproms
 *
 */

#if !defined( EEPROMDEV_H )
#define EEPROMDEV_H ( 1 )

typedef struct _eeprom_interface eeprom_interface;
struct _eeprom_interface
{
	int address_bits;		/* EEPROM has 2^address_bits cells */
	int data_bits;			/* every cell has this many bits (8 or 16) */
	const char *cmd_read;	/*   read command string, e.g. "0110" */
	const char *cmd_write;	/*  write command string, e.g. "0111" */
	const char *cmd_erase;	/*  erase command string, or 0 if n/a */
	const char *cmd_lock;	/*   lock command string, or 0 if n/a */
	const char *cmd_unlock;	/* unlock command string, or 0 if n/a */
	int enable_multi_read;	/* set to 1 to enable multiple values to be read from one read command */
	int reset_delay;		/* number of times eeprom_read_bit() should return 0 after a reset, */
							/* before starting to return 1. */
};

typedef struct _eeprom_config eeprom_config;
struct _eeprom_config
{
	eeprom_interface *pinterface;
	UINT8 *default_data;
	int default_data_size;
};

/* 93C46 */
extern const eeprom_interface eepromdev_interface_93C46;

/* 93C66B */
extern const eeprom_interface eepromdev_interface_93C66B;

#define EEPROM DEVICE_GET_INFO_NAME(eeprom)
DEVICE_GET_INFO(eeprom);

#define MDRV_EEPROM_ADD(_tag, _interface, _data_size, _data) \
	MDRV_DEVICE_ADD(_tag, EEPROM, 0) \
	MDRV_DEVICE_CONFIG_DATAPTR(eeprom_config, pinterface, &_interface) \
	MDRV_EEPROM_DATA(_data, _data_size)

#define MDRV_EEPROM_93C46_ADD(_tag, _data_size, _data) \
	MDRV_EEPROM_ADD(_tag, eepromdev_interface_93C46, _data_size, _data)

#define MDRV_EEPROM_93C66B_ADD(_tag, _data_size, _data) \
	MDRV_EEPROM_ADD(_tag, eepromdev_interface_93C66B, _data_size, _data)

#define MDRV_EEPROM_DATA(_data, _size) \
	MDRV_DEVICE_CONFIG_DATAPTR(eeprom_config, default_data, &_data) \
	MDRV_DEVICE_CONFIG_DATA32(eeprom_config, default_data_size, _size)

/* FIXME: many drivers do not need default_data / default_data_size and put them to 0 in the drivers seems a waste of code */
extern UINT8 *eeprom_empty_default_data;

#define MDRV_EEPROM_NODEFAULT_ADD(_tag, _interface) \
	MDRV_EEPROM_ADD(_tag, _interface, 0, eeprom_empty_default_data)

void eepromdev_write_bit( const device_config *device, int bit );
int eepromdev_read_bit( const device_config *device );
CUSTOM_INPUT( eepromdev_bit_r );
void eepromdev_set_cs_line( const device_config *device, int state );
void eepromdev_set_clock_line( const device_config *device, int state );

void eepromdev_load( const device_config *device, mame_file *file );
void eepromdev_save( const device_config *device, mame_file *file );

void eepromdev_set_data( const device_config *device, const UINT8 *data, int length );
void *eepromdev_get_data_pointer( const device_config *device, UINT32 *length, UINT32 *size );

#endif
