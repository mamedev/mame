#include "scsicb.h"

scsicb_device::scsicb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, SCSICB, "SCSI callback", tag, owner, clock)
{
}

void scsicb_device::device_config_complete()
{
	// inherit a copy of the static data
	const SCSICB_interface *intf = reinterpret_cast<const SCSICB_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<SCSICB_interface *>(this) = *intf;
	}
}

void scsicb_device::device_start()
{
	out_bsy_func.resolve(_out_bsy_func, *this);
	out_sel_func.resolve(_out_sel_func, *this);
	out_cd_func.resolve(_out_cd_func, *this);
	out_io_func.resolve(_out_io_func, *this);
	out_msg_func.resolve(_out_msg_func, *this);
	out_req_func.resolve(_out_req_func, *this);
	out_rst_func.resolve(_out_rst_func, *this);
}

const device_type SCSICB = &device_creator<scsicb_device>;
