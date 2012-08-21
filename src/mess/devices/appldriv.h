/*********************************************************************

    appldriv.h

    Apple 5.25" floppy drive emulation (to be interfaced with applefdc.c)

*********************************************************************/

#ifndef APPLDRIV_H
#define APPLDRIV_H

#include "emu.h"
#include "imagedev/flopdrv.h"

void apple525_set_lines(device_t *device,UINT8 lines);
void apple525_set_enable_lines(device_t *device,int enable_mask);

UINT8 apple525_read_data(device_t *device);
void apple525_write_data(device_t *device,UINT8 data);
int apple525_read_status(device_t *device);
int apple525_get_count(running_machine &machine);

class apple525_floppy_image_device :	public legacy_floppy_image_device
{
public:
	// construction/destruction
	apple525_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual bool call_load();
	virtual void call_unload();
	void set_params(int dividend, int divisor) { m_dividend = dividend; m_divisor = divisor;}

	int get_dividend() { return m_dividend; }
	int get_divisor() { return m_divisor; }
protected:
	virtual void device_start();
private:
	int	m_dividend;
	int	m_divisor;
};

// device type definition
extern const device_type FLOPPY_APPLE;

#define MCFG_LEGACY_FLOPPY_APPLE_PARAMS(_dividend,_divisor)	\
	downcast<apple525_floppy_image_device *>(device)->set_params(_dividend,_divisor);

#define MCFG_LEGACY_FLOPPY_APPLE_2_DRIVES_ADD(_config,_dividend,_divisor)	\
	MCFG_DEVICE_ADD(FLOPPY_0, FLOPPY_APPLE, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_LEGACY_FLOPPY_APPLE_PARAMS(_dividend,_divisor) \
	MCFG_DEVICE_ADD(FLOPPY_1, FLOPPY_APPLE, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_LEGACY_FLOPPY_APPLE_PARAMS(_dividend,_divisor)

#define MCFG_LEGACY_FLOPPY_APPLE_4_DRIVES_ADD(_config,_dividend,_divisor)	\
	MCFG_DEVICE_ADD(FLOPPY_0, FLOPPY_APPLE, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_LEGACY_FLOPPY_APPLE_PARAMS(_dividend,_divisor) \
	MCFG_DEVICE_ADD(FLOPPY_1, FLOPPY_APPLE, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_LEGACY_FLOPPY_APPLE_PARAMS(_dividend,_divisor) \
	MCFG_DEVICE_ADD(FLOPPY_2, FLOPPY_APPLE, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_LEGACY_FLOPPY_APPLE_PARAMS(_dividend,_divisor) \
	MCFG_DEVICE_ADD(FLOPPY_3, FLOPPY_APPLE, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_LEGACY_FLOPPY_APPLE_PARAMS(_dividend,_divisor)

#define MCFG_LEGACY_FLOPPY_APPLE_2_DRIVES_REMOVE()	\
	MCFG_DEVICE_REMOVE(FLOPPY_0)		\
	MCFG_DEVICE_REMOVE(FLOPPY_1)

#endif /* APPLDRIV_H */
