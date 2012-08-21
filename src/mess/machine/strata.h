/*
    strata.h: header file for strata.c
*/

DECLARE_LEGACY_DEVICE(STRATAFLASH, strataflash);

#define MCFG_STRATAFLASH_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, STRATAFLASH, 0)

int strataflash_load(device_t *device, emu_file *file);
int strataflash_save(device_t *device, emu_file *file);
UINT8 strataflash_8_r(device_t *device, UINT32 address);
void strataflash_8_w(device_t *device, UINT32 address, UINT8 data);
UINT16 strataflash_16_r(device_t *device, offs_t offset);
void strataflash_16_w(device_t *device, offs_t offset, UINT16 data);
