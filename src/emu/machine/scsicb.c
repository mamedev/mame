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
	: scsidev_device(mconfig, SCSICB, "SCSI callback", tag, owner, clock),
	m_bsy_handler(*this),
	m_sel_handler(*this),
	m_cd_handler(*this),
	m_io_handler(*this),
	m_msg_handler(*this),
	m_req_handler(*this),
	m_ack_handler(*this),
	m_atn_handler(*this),
	m_rst_handler(*this)
{
}

void scsicb_device::device_start()
{
	scsidev_device::device_start();

	linestate = 0;

	m_bsy_handler.resolve_safe();
	m_sel_handler.resolve_safe();
	m_cd_handler.resolve_safe();
	m_io_handler.resolve_safe();
	m_msg_handler.resolve_safe();
	m_req_handler.resolve_safe();
	m_ack_handler.resolve_safe();
	m_atn_handler.resolve_safe();
	m_rst_handler.resolve_safe();
}

void scsicb_device::scsi_in( UINT32 data, UINT32 mask )
{
	linestate = data;

	trigger_callback( mask, SCSI_MASK_BSY, m_bsy_handler );
	trigger_callback( mask, SCSI_MASK_SEL, m_sel_handler );
	trigger_callback( mask, SCSI_MASK_CD, m_cd_handler );
	trigger_callback( mask, SCSI_MASK_IO, m_io_handler );
	trigger_callback( mask, SCSI_MASK_MSG, m_msg_handler );
	trigger_callback( mask, SCSI_MASK_REQ, m_req_handler );
	trigger_callback( mask, SCSI_MASK_ACK, m_ack_handler );
	trigger_callback( mask, SCSI_MASK_ATN, m_atn_handler );
	trigger_callback( mask, SCSI_MASK_RST, m_rst_handler );
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
	UINT8 state;

	if( ( linestate & mask ) != 0 )
	{
		state = 1;
	}
	else
	{
		state = 0;
	}

	verboselog( 1, machine(), "%s get_scsi_line %s %d\n", tag(), get_line_name( mask ), state );

	return state;
}

void scsicb_device::set_scsi_line( UINT32 mask, UINT8 state )
{
	verboselog( 1, machine(), "%s set_scsi_line %s %d\n", tag(), get_line_name( mask ), state );

	if( state )
	{
		scsi_out( mask, mask );
	}
	else
	{
		scsi_out( 0, mask );
	}
}

void scsicb_device::trigger_callback( UINT32 update_mask, UINT32 line_mask, devcb2_write_line &write_line )
{
	if( ( update_mask & line_mask ) != 0 && !write_line.isnull() )
	{
		UINT8 state;

		if( ( linestate & line_mask ) != 0 )
		{
			state = 1;
		}
		else
		{
			state = 0;
		}

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
