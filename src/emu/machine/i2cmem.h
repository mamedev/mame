/*

I2C Memory

*/

#if !defined( I2CMEM_H )
#define I2CMEM_H ( 1 )

#define I2CMEM_E0 ( 1 )
#define I2CMEM_E1 ( 2 )
#define I2CMEM_E2 ( 3 )
#define I2CMEM_SDA ( 5 )
#define I2CMEM_SCL ( 6 )
#define I2CMEM_WC ( 7 )

#define I2CMEM_MAXCHIP ( 1 )

#define I2CMEM_SLAVE_ADDRESS ( 0xa0 )
#define I2CMEM_SLAVE_ADDRESS_ALT ( 0xb0 )

extern void i2cmem_init( running_machine *machine, int chip, int slave_address, int page_size, int data_size, unsigned char *data );
extern void i2cmem_write( int chip, int line, int data );
extern int i2cmem_read( int chip, int line );
extern NVRAM_HANDLER( i2cmem_0 );

#endif
