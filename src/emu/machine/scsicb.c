#include "scsicb.h"
#include "scsibus.h"

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

UINT8 scsicb_device::scsi_data_r()
{
	scsibus_device *m_scsibus = downcast<scsibus_device *>( owner() );
	return m_scsibus->scsi_data_r();
}

void scsicb_device::scsi_data_w( UINT8 data )
{
	scsibus_device *m_scsibus = downcast<scsibus_device *>( owner() );
	m_scsibus->scsi_data_w( data );
}

UINT8 scsicb_device::get_scsi_line( UINT8 line )
{
	scsibus_device *m_scsibus = downcast<scsibus_device *>( owner() );
	return m_scsibus->get_scsi_line( line );
}

void scsicb_device::set_scsi_line( UINT8 line, UINT8 state )
{
	scsibus_device *m_scsibus = downcast<scsibus_device *>( owner() );
	m_scsibus->set_scsi_line( line, state );
}

READ8_MEMBER( scsicb_device::scsi_data_r )
{
	return scsi_data_r();
}

WRITE8_MEMBER( scsicb_device::scsi_data_w )
{
	scsi_data_w( data );
}

READ_LINE_MEMBER( scsicb_device::scsi_bsy_r ) { return get_scsi_line(SCSI_LINE_BSY); }
READ_LINE_MEMBER( scsicb_device::scsi_sel_r ) { return get_scsi_line(SCSI_LINE_SEL); }
READ_LINE_MEMBER( scsicb_device::scsi_cd_r ) { return get_scsi_line(SCSI_LINE_CD); }
READ_LINE_MEMBER( scsicb_device::scsi_io_r ) { return get_scsi_line(SCSI_LINE_IO); }
READ_LINE_MEMBER( scsicb_device::scsi_msg_r ) { return get_scsi_line(SCSI_LINE_MSG); }
READ_LINE_MEMBER( scsicb_device::scsi_req_r ) { return get_scsi_line(SCSI_LINE_REQ); }
READ_LINE_MEMBER( scsicb_device::scsi_ack_r ) { return get_scsi_line(SCSI_LINE_ACK); }
READ_LINE_MEMBER( scsicb_device::scsi_rst_r ) { return get_scsi_line(SCSI_LINE_RESET); }

WRITE_LINE_MEMBER( scsicb_device::scsi_bsy_w ) { set_scsi_line(SCSI_LINE_BSY, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_sel_w ) { set_scsi_line(SCSI_LINE_SEL, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_cd_w ) { set_scsi_line(SCSI_LINE_CD, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_io_w ) { set_scsi_line(SCSI_LINE_IO, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_msg_w ) { set_scsi_line(SCSI_LINE_MSG, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_req_w ) { set_scsi_line(SCSI_LINE_REQ, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_ack_w ) { set_scsi_line(SCSI_LINE_ACK, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_rst_w ) { set_scsi_line(SCSI_LINE_RESET, state); }

const device_type SCSICB = &device_creator<scsicb_device>;
