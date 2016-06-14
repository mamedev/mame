// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

	Sony LDP-1000 laserdisc emulation.

	TODO:
	- Dump BIOSes (seven of them according to docs);
	- Serial interface, needs BIOS dump;
	- Hookup with Sony SMC-70 / SMC-777;

***************************************************************************/

#include "emu.h"
#include "machine/ldp1000.h"

#define DUMP_BCD 1

ROM_START( ldp1000 )
	ROM_REGION( 0x2000, "ldp1000", 0 )
	ROM_LOAD( "ldp1000_bios.bin", 0x0000, 0x2000, NO_DUMP )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SONY_LDP1000 = &device_creator<sony_ldp1000_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ldp1000_device - constructor
//-------------------------------------------------

sony_ldp1000_device::sony_ldp1000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: laserdisc_device(mconfig, SONY_LDP1000, "Sony LDP-1000", tag, owner, clock, "ldp1000", __FILE__)
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void sony_ldp1000_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sony_ldp1000_device::device_start()
{
	laserdisc_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sony_ldp1000_device::device_reset()
{
	laserdisc_device::device_reset();

}

//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const rom_entry *sony_ldp1000_device::device_rom_region() const
{
	return ROM_NAME(ldp1000);
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void sony_ldp1000_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	//printf("%d vsync\n",fieldnum);
}


//-------------------------------------------------
//  player_update - update callback, called on
//  the first visible line of the frame
//-------------------------------------------------

INT32 sony_ldp1000_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	//printf("%d update\n",fieldnum);

	return fieldnum;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( sony_ldp1000_device::status_r )
{
	UINT8 res = m_status;
	m_status = stat_undef;
	return res;
}

void sony_ldp1000_device::set_new_player_state(ldp1000_player_state which, UINT8 fifo_size)
{
	m_player_state = which;
	m_index_state = 0;
	m_index_size = fifo_size;
	printf("set new player state\n");
}

// TODO: probably don't even need a size ...
void sony_ldp1000_device::set_new_player_bcd(UINT8 data)
{
	m_internal_bcd[m_index_state] = data;
	m_index_state ++;
	if(m_index_state >= m_index_size)
	{
		#if DUMP_BCD
			for(int i=0;i<m_index_size;i++)
				printf("%02x ",m_internal_bcd[i]);
			
			printf("[size = %02x]\n",m_index_size);
		#endif
		m_status = stat_ack;
	}
	else
		m_status = stat_ack;
}


// TODO: de-instantize this
WRITE8_MEMBER( sony_ldp1000_device::command_w )
{
	printf("CMD %02x\n",data);
	// 0x30 to 0x69 range causes an ACK, anything else is invalid
	m_command = data;

	if((m_command & 0xf0) == 0x30)
	{
		set_new_player_bcd(data);
		return;
	}

	switch(m_command)
	{
		case 0x40: // enter bcd command, todo
			break;
		
		case 0x43: // search
			set_new_player_state(player_search,5);
			m_status = stat_ack;
			break;
		
		/*
			audio channels absolute enable / disable
			---- --x- select channel
			---- ---x enable channel (active low)
		*/
		case 0x46: 
		case 0x47:
		case 0x48:
		case 0x49:
			m_audio_enable[(m_command & 2) >> 1] = (m_command & 1) == 0;
			m_status = stat_ack;
			break;

		case 0x56: // Clear All
			m_status = stat_ack;
			// reset any pending operation here
			break;

		default:
			m_status = stat_undef;
			break;
	}
}
