#include "am8530h.h"

const device_type AM8530H = &device_creator<am8530h_device>;

am8530h_device::am8530h_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : device_t(mconfig, AM8530H, "AM8530H", tag, owner, clock)
{
}

void am8530h_device::set_int_change_cb(int_cb_t _int_change_cb)
{
	int_change_cb = _int_change_cb;
}


void am8530h_device::device_start()
{
}

READ8_MEMBER( am8530h_device::ca_r )
{
	return 0xff;
}

READ8_MEMBER( am8530h_device::cb_r )
{
	return 0xff;
}

READ8_MEMBER( am8530h_device::da_r )
{
	return 0x40;
}

READ8_MEMBER( am8530h_device::db_r )
{
	return 0x40;
}

WRITE8_MEMBER( am8530h_device::ca_w )
{
	fprintf(stderr, "ca_w %x, %02x\n", offset, data);
}

WRITE8_MEMBER( am8530h_device::cb_w )
{
	fprintf(stderr, "cb_w %x, %02x\n", offset, data);
}

WRITE8_MEMBER( am8530h_device::da_w )
{
	fprintf(stderr, "da_w %x, %02x\n", offset, data);
}

WRITE8_MEMBER( am8530h_device::db_w )
{
	fprintf(stderr, "db_w %x, %02x\n", offset, data);
}
