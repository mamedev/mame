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
#include "ldp1000.h"

#define DUMP_BCD 1
#define FIFO_MAX 0x10

ROM_START( ldp1000 )
	ROM_REGION( 0x2000, "ldp1000", 0 )
	ROM_LOAD( "ldp1000_bios.bin", 0x0000, 0x2000, NO_DUMP )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SONY_LDP1000, sony_ldp1000_device, "ldp1000", "Sony LDP-1000")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sony_ldp1000_device - constructor
//-------------------------------------------------

sony_ldp1000_device::sony_ldp1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: laserdisc_device(mconfig, SONY_LDP1000, tag, owner, clock)
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

	for(int i=0;i<0x10;i++)
		m_internal_bcd[i] = 0;

}

//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const tiny_rom_entry *sony_ldp1000_device::device_rom_region() const
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

int32_t sony_ldp1000_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	//printf("%d update\n",fieldnum);

	return fieldnum;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint8_t sony_ldp1000_device::status_r()
{
	uint8_t res = m_status;
	m_status = stat_undef;
	return res;
}

void sony_ldp1000_device::set_new_player_state(ldp1000_player_state which)
{
	m_player_state = which;
	m_index_state = 0;
	logerror("set new player state\n");
}

// TODO: probably don't even need a size ...
void sony_ldp1000_device::set_new_player_bcd(uint8_t data)
{
	m_internal_bcd[m_index_state] = data;
	m_index_state ++;
	if(m_index_state >= FIFO_MAX)
		throw emu_fatalerror("FIFO MAX reached");

	m_status = stat_ack;
}

uint32_t sony_ldp1000_device::bcd_to_raw()
{
	uint32_t res = 0;
	for(int i=0;i<6;i++)
		res |= (m_internal_bcd[i] & 0xf) << i*4;
	return res;
}

void sony_ldp1000_device::exec_enter_cmd()
{
	//const uint32_t saved_frame = bcd_to_raw();

	switch(m_player_state)
	{
		case player_standby:
			throw emu_fatalerror("Unimplemented standby state detected");

		case player_search:
			// TODO: move to timer
			//advance_slider(1);
			//set_slider_speed(saved_frame);
			break;
	}
	m_player_state = player_standby;
}


// TODO: de-instantize this
void sony_ldp1000_device::command_w(uint8_t data)
{
	logerror("CMD %02x\n",data);
	// 0x30 to 0x69 range causes an ACK, anything else is invalid
	m_command = data;

	if((m_command & 0xf0) == 0x30 && (m_command & 0xf) < 0x0a)
	{
		set_new_player_bcd(data);
		return;
	}

	switch(m_command)
	{
		case 0x40: // enter, process BCD command
			exec_enter_cmd();
			m_status = stat_ack;
			break;

		case 0x43: // search
			set_new_player_state(player_search);
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
