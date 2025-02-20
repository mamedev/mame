// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "saa7110.h"

#define VERBOSE (1)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SAA7110A, saa7110a_device, "saa7110a", "SAA7110A OCF1")

// TODO: pin address overridable with SA pin = 1 (0x9e >> 1)
saa7110a_device::saa7110a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SAA7110A, tag, owner, clock)
	, i2c_hle_interface(mconfig, *this, 0x9c >> 1)
	//, device_memory_interface(mconfig, *this)
{}

void saa7110a_device::device_start()
{
}

void saa7110a_device::device_reset()
{

}

u8 saa7110a_device::read_data(u16 offset)
{
    //printf("%02x\n", offset);
	return 0xff;
}

void saa7110a_device::write_data(u16 offset, u8 data)
{
    //printf("%02x %02x\n", offset, data);
}

