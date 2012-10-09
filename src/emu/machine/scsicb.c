/*

scsicb.c

Implementation of a raw SCSI/SASI bus for machines that don't use a SCSI
controler chip such as the RM Nimbus, which implements it as a bunch of
74LS series chips.

*/

#include "scsicb.h"
#include "scsibus.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF( 3, 4 ) verboselog( int n_level, running_machine &machine, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine.describe_context( ), buf );
	}
}

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

	linestate = SCSI_MASK_ALL;

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

	trigger_callback( mask, SCSI_MASK_BSY, out_bsy_func );
	trigger_callback( mask, SCSI_MASK_SEL, out_sel_func );
	trigger_callback( mask, SCSI_MASK_CD, out_cd_func );
	trigger_callback( mask, SCSI_MASK_IO, out_io_func );
	trigger_callback( mask, SCSI_MASK_MSG, out_msg_func );
	trigger_callback( mask, SCSI_MASK_REQ, out_req_func );
	trigger_callback( mask, SCSI_MASK_ACK, out_ack_func );
	trigger_callback( mask, SCSI_MASK_ATN, out_atn_func );
	trigger_callback( mask, SCSI_MASK_RST, out_rst_func );
}

UINT8 scsicb_device::scsi_data_r()
{
	UINT8 data = linestate & SCSI_MASK_DATA;
	verboselog( 1, machine(), "%s scsi_data_r() %02x\n", tag(), data );
	return data;
}

void scsicb_device::scsi_data_w( UINT8 data )
{
	verboselog( 1, machine(), "%s scsi_data_w( %02x )\n", tag(), data );
	scsi_out( data, SCSI_MASK_DATA );
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

UINT8 scsicb_device::get_scsi_line( UINT32 mask )
{
	UINT8 state = (int)( ( linestate & mask ) != 0 );

	verboselog( 1, machine(), "%s get_scsi_line %s %d\n", tag(), get_line_name( mask ), state ); 

	return state;
}

void scsicb_device::set_scsi_line( UINT32 mask, UINT8 state )
{
	verboselog( 1, machine(), "%s set_scsi_line %s %d\n", tag(), get_line_name( mask ), state ); 

	scsi_out( state * mask, mask );
}

void scsicb_device::trigger_callback( UINT32 update_mask, UINT32 line_mask, devcb_resolved_write_line &write_line )
{
	if( ( update_mask & line_mask ) != 0 && !write_line.isnull() )
	{
		int state = (int)( ( linestate & line_mask ) != 0 );

		verboselog( 1, machine(), "%s trigger_callback %s %d\n", tag(), get_line_name( line_mask ), state );

		write_line( state );
	}
}

const char *scsicb_device::get_line_name( UINT32 mask )
{
	switch( mask )
	{
	case SCSI_MASK_BSY:
		return "bsy";

	case SCSI_MASK_SEL:
		return "sel";

	case SCSI_MASK_CD:
		return "cd";

	case SCSI_MASK_IO:
		return "io";

	case SCSI_MASK_MSG:
		return "msg";

	case SCSI_MASK_REQ:
		return "req";

	case SCSI_MASK_ACK:
		return "ack";

	case SCSI_MASK_ATN:
		return "atn";

	case SCSI_MASK_RST:
		return "rst";
	}

	return "?";
}

const device_type SCSICB = &device_creator<scsicb_device>;
