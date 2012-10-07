/*

scsicb.c

Implementation of a raw SCSI/SASI bus for machines that don't use a SCSI
controler chip such as the RM Nimbus, which implements it as a bunch of
74LS series chips.

*/

#include "scsicb.h"
#include "scsibus.h"

scsicb_device::scsicb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : scsidev_device(mconfig, SCSICB, "SCSI callback", tag, owner, clock)
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
	scsidev_device::device_start();

	out_bsy_func.resolve(_out_bsy_func, *this);
	out_sel_func.resolve(_out_sel_func, *this);
	out_cd_func.resolve(_out_cd_func, *this);
	out_io_func.resolve(_out_io_func, *this);
	out_msg_func.resolve(_out_msg_func, *this);
	out_req_func.resolve(_out_req_func, *this);
	out_ack_func.resolve(_out_ack_func, *this);
	out_atn_func.resolve(_out_atn_func, *this);
	out_rst_func.resolve(_out_rst_func, *this);
}

void scsicb_device::scsi_in( UINT32 data, UINT32 mask )
{
	linestate = data;

	if( ( mask & SCSI_MASK_BSY ) != 0 )
	{
		out_bsy_func( (int) ( data & SCSI_MASK_BSY ) != 0 );
	}

	if( ( mask & SCSI_MASK_SEL ) != 0 )
	{
		out_sel_func( (int) ( data & SCSI_MASK_SEL ) != 0 );
	}

	if( ( mask & SCSI_MASK_CD ) != 0 )
	{
		out_cd_func( (int) ( data & SCSI_MASK_CD ) != 0 );
	}

	if( ( mask & SCSI_MASK_IO ) != 0 )
	{
		out_io_func( (int) ( data & SCSI_MASK_IO ) != 0 );
	}

	if( ( mask & SCSI_MASK_MSG ) != 0 )
	{
		out_msg_func( (int) ( data & SCSI_MASK_MSG ) != 0 );
	}

	if( ( mask & SCSI_MASK_REQ ) != 0 )
	{
		out_req_func( (int) ( data & SCSI_MASK_REQ ) != 0 );
	}

	if( ( mask & SCSI_MASK_ACK ) != 0 )
	{
		out_ack_func( (int) ( data & SCSI_MASK_ACK ) != 0 );
	}

	if( ( mask & SCSI_MASK_ATN ) != 0 )
	{
		out_ack_func( (int) ( data & SCSI_MASK_ATN ) != 0 );
	}

	if( ( mask & SCSI_MASK_RST ) != 0 )
	{
		out_rst_func( (int) ( data & SCSI_MASK_RST ) != 0 );
	}
}

UINT8 scsicb_device::scsi_data_r()
{
	return linestate & SCSI_MASK_DATA;
}

void scsicb_device::scsi_data_w( UINT8 data )
{
	scsi_out( data, SCSI_MASK_DATA );
}

UINT8 scsicb_device::get_scsi_line( UINT32 line )
{
	UINT8 result = (int)( ( linestate & line ) != 0 );

//	LOG(3,"get_scsi_line(%s)=%d\n",linenames[lineno],result);

	return result;
}

void scsicb_device::set_scsi_line( UINT32 mask, UINT8 state )
{
	scsi_out( state * mask, mask );
}

READ8_MEMBER( scsicb_device::scsi_data_r )
{
	return scsi_data_r();
}

WRITE8_MEMBER( scsicb_device::scsi_data_w )
{
	scsi_data_w( data );
}

READ_LINE_MEMBER( scsicb_device::scsi_bsy_r ) { return get_scsi_line(SCSI_MASK_BSY); }
READ_LINE_MEMBER( scsicb_device::scsi_sel_r ) { return get_scsi_line(SCSI_MASK_SEL); }
READ_LINE_MEMBER( scsicb_device::scsi_cd_r ) { return get_scsi_line(SCSI_MASK_CD); }
READ_LINE_MEMBER( scsicb_device::scsi_io_r ) { return get_scsi_line(SCSI_MASK_IO); }
READ_LINE_MEMBER( scsicb_device::scsi_msg_r ) { return get_scsi_line(SCSI_MASK_MSG); }
READ_LINE_MEMBER( scsicb_device::scsi_req_r ) { return get_scsi_line(SCSI_MASK_REQ); }
READ_LINE_MEMBER( scsicb_device::scsi_ack_r ) { return get_scsi_line(SCSI_MASK_ACK); }
READ_LINE_MEMBER( scsicb_device::scsi_atn_r ) { return get_scsi_line(SCSI_MASK_ATN); }
READ_LINE_MEMBER( scsicb_device::scsi_rst_r ) { return get_scsi_line(SCSI_MASK_RST); }

WRITE_LINE_MEMBER( scsicb_device::scsi_bsy_w ) { set_scsi_line(SCSI_MASK_BSY, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_sel_w ) { set_scsi_line(SCSI_MASK_SEL, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_cd_w ) { set_scsi_line(SCSI_MASK_CD, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_io_w ) { set_scsi_line(SCSI_MASK_IO, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_msg_w ) { set_scsi_line(SCSI_MASK_MSG, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_req_w ) { set_scsi_line(SCSI_MASK_REQ, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_ack_w ) { set_scsi_line(SCSI_MASK_ACK, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_atn_w ) { set_scsi_line(SCSI_MASK_ATN, state); }
WRITE_LINE_MEMBER( scsicb_device::scsi_rst_w ) { set_scsi_line(SCSI_MASK_RST, state); }

const device_type SCSICB = &device_creator<scsicb_device>;
