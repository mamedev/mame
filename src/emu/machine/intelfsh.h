/*
    Intel Flash ROM emulation
*/

#ifndef _INTELFLASH_H_
#define _INTELFLASH_H_

#define FLASH_CHIPS_MAX	( 56 )

#define FLASH_INTEL_28F016S5 ( 0 )
#define FLASH_SHARP_LH28F400 ( 1 )
#define FLASH_FUJITSU_29F016A ( 2 )
#define FLASH_INTEL_E28F008SA ( 3 )
#define FLASH_INTEL_TE28F160 ( 4 )
#define FLASH_SHARP_LH28F016S ( 5 )
#define FLASH_INTEL_E28F400 ( 6 )
#define FLASH_SHARP_UNK128MBIT ( 7 )
#define FLASH_MACRONIX_29L001MC ( 8 )
#define FLASH_PANASONIC_MN63F805MNP ( 9 )
#define FLASH_SANYO_LE26FV10N1TS ( 10 )

extern void intelflash_init( running_machine *machine, int chip, int type, void *data );
extern UINT32 intelflash_read( int chip, UINT32 address );
extern void intelflash_write( int chip, UINT32 address, UINT32 value );
extern void nvram_handler_intelflash( running_machine *machine, int chip, mame_file *file, int read_or_write );
extern void* intelflash_getmemptr(int chip);

#endif
