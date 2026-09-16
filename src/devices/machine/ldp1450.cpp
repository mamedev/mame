// license:BSD-3-Clause
// copyright-holders:James Wallace
/***************************************************************************

    Sony LDP-1450 laserdisc emulation.

    TODO:
    - Dump MCU BIOS(more than one?)
    - Many players support this command set, split out other device stubs (such as LDP-1550P, PAL)
    - Text overlay (needed for practically everything)
***************************************************************************/

#include "emu.h"
#include "ldp1450.h"

#define DUMP_BCD 1
#define FIFO_MAX 0x10

#define LDP_STAT_UNDEF      0x00
#define LDP_STAT_COMPLETION 0x01
#define LDP_STAT_ERROR      0x02
#define LDP_STAT_PGM_END    0x04
#define LDP_STAT_NOT_TARGET 0x05
#define LDP_STAT_NO_FRAME   0x06
#define LDP_STAT_ACK        0x0a
#define LDP_STAT_NAK        0x0b

ROM_START( ldp1450 )
	ROM_REGION( 0x2000, "ldp1450", 0 )
	ROM_LOAD( "ldp1450_bios.bin", 0x0000, 0x2000, NO_DUMP )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SONY_LDP1450, sony_ldp1450_device, "ldp1450", "Sony LDP-1450")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sony_ldp1450_device - constructor
//-------------------------------------------------

sony_ldp1450_device::sony_ldp1450_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: laserdisc_device(mconfig, SONY_LDP1450, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void sony_ldp1450_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sony_ldp1450_device::device_start()
{
	laserdisc_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sony_ldp1450_device::device_reset()
{
	laserdisc_device::device_reset();

	for(int i=0;i<0x10;i++)
		m_internal_bcd[i] = 0;

	m_ld_input_state = LD_INPUT_GET_COMMAND;
	m_ld_command_current_byte = m_ld_command_total_bytes = 0;
	m_ld_frame_index = 0;

}

//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region definitions
//-------------------------------------------------

const tiny_rom_entry *sony_ldp1450_device::device_rom_region() const
{
	return ROM_NAME(ldp1450);
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void sony_ldp1450_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	//printf("%d vsync\n",fieldnum);
}


//-------------------------------------------------
//  player_update - update callback, called on
//  the first visible line of the frame
//-------------------------------------------------

int32_t sony_ldp1450_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	//printf("%d update\n",fieldnum);

	return fieldnum;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void sony_ldp1450_device::set_new_player_state(ldp1450_player_state which)
{
	m_player_state = which;
	m_index_state = 0;
}

void sony_ldp1450_device::set_new_player_bcd(uint8_t data)
{
	printf("Frame data BCD %02x\n",data);

	m_internal_bcd[m_index_state] = data;
	m_index_state ++;
	if(m_index_state >= FIFO_MAX)
		throw emu_fatalerror("FIFO MAX reached");

	m_status = LDP_STAT_ACK;
}

uint32_t sony_ldp1450_device::bcd_to_raw()
{
	uint32_t res = 0;
	for(int i=0;i<6;i++)
		res |= (m_internal_bcd[i] & 0xf) << i*4;
	return res;
}

void sony_ldp1450_device::exec_enter_cmd()
{
	const uint32_t saved_frame = bcd_to_raw();

	switch(m_player_state)
	{
		case player_standby:
			throw emu_fatalerror("Unimplemented standby state detected");

		case player_search:
			// TODO: move to timer
			advance_slider(1);
			set_slider_speed(saved_frame);
			break;

		default:
		//not handling all states yet
		break;
	}
	m_player_state = player_standby;
}

void sony_ldp1450_device::command_w(uint8_t data)
{
	printf("CMD %02x\n",data);
	m_command = data;

	if((m_command & 0xf0) == 0x30 && (m_command & 0xf) < 0x0a)
	{
		set_new_player_bcd(data);
		return;
	}

	switch(m_command)
	{

		case 0x00: /* text handling (start gotoxy) */
			if ( m_ld_input_state == LD_INPUT_TEXT_COMMAND )
			{
				m_ld_input_state = LD_INPUT_TEXT_GET_X;
			}
		break;
		case 0x01: /* text handling (end of text)*/
			if ( m_ld_input_state == LD_INPUT_TEXT_COMMAND )
			{
				m_ld_input_state = LD_INPUT_TEXT_GET_STRING;
			}
		break;
		case 0x02: /* text 'set window' command */
			if ( m_ld_input_state == LD_INPUT_TEXT_COMMAND )
			{
				m_ld_input_state = LD_INPUT_TEXT_GET_SET_WINDOW;
			}
		break;
		case 0x1a: /* text sent */
		break;
		case 0x24: /* Audio On */
			m_status = LDP_STAT_ACK;
		break;
		case 0x25: /* Audio Off */
			m_status = LDP_STAT_ACK;
		break;
		case 0x26: /* Video off */
			printf("Video OFF \n");
			m_status = LDP_STAT_ACK;
		break;
		case 0x27: /* Video on */
			printf("Video ON \n");
			m_status = LDP_STAT_ACK;
		break;
		case 0x28: /* Stop Codes Enable */
			printf("Stop Code ON \n");
		break;
		case 0x29: /* Stop Codes Disable */
			printf("Stop Code OFF \n");
		break;
		case 0x2a: /* Eject */
		break;
		case 0x2b: /* Step forward */
		break;
		case 0x2c: /* Step reverse */
		break;
		//30 to 39 handled separately, as they are the frame commands
		case 0x3a: /* Play (answer should have delay) */
			printf("play\n");
			set_new_player_state(player_play);
			m_status = LDP_STAT_ACK;
		break;
		case 0x3b: /* Play fast */
		break;
		case 0x3c: /* Play slow */
		break;
		case 0x3d: /* Play step */
		break;
		case 0x3e: /* Play scan */
		break;
		case 0x3f: /* Stop */
			printf("stop\n");
			set_new_player_state(player_stop);
			m_status = LDP_STAT_ACK;
		break;

		case 0x40: // enter, process BCD command
			printf("CMD Enter\n");
			exec_enter_cmd();
			m_status = LDP_STAT_ACK;
			break;
		case 0x41: /* CE */
		break;

		case 0x43: // search
			printf("Search \n");
			set_new_player_state(player_search);
			m_status = LDP_STAT_ACK;
			break;

		case 0x44: // repeat play
			printf("CMD Repeat\n");
			set_new_player_state(player_repeat);
			m_status = LDP_STAT_ACK;
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
			printf("Audio channel %x\n",(m_command & 2) >> 1);
			printf("Audio status %x\n",(m_command & 1) == 0);
			m_audio_enable[(m_command & 2) >> 1] = (m_command & 1) == 0;
			m_status = LDP_STAT_ACK;
			break;
		case 0x4a: /* Play reverse(answer should have delay) */
			printf("play reverse\n");
		break;
		case 0x4b: /* Play rev fast */
		break;
		case 0x4c: /* Play rev slow */
		break;
		case 0x4d: /* Play rev step */
		break;
		case 0x4e: /* Play rev scan */
		break;

		case 0x4f: /* Still (not implemented)*/
			if (m_player_state == player_stop)
			{
				m_status = LDP_STAT_NAK;
			}
			else
			{
				m_status = LDP_STAT_ACK;
			}
		break;
		case 0x55: /* 'frame mode' (unknown function) */
			break;

		case 0x56: // Clear All
			if (m_player_state == player_search)
			{
				printf("clear all\n");

				set_new_player_state(player_search_clr);
			}

			m_status = LDP_STAT_ACK;
			// reset any pending operation here
			break;

		case 0x60: /* Addr Inq (get current frame number) */
			for (uint8_t & elem : m_internal_bcd)
			{
				printf("Return frame %02x\n",elem);
				m_status = elem;
			}
			break;
		case 0x62: /* Motor on */
			break;
		case 0x6e:  // CX enable - anything use it?
		break;

		case 0x80: /* text start */
			printf("CMD Start text\n");
			m_ld_input_state = LD_INPUT_TEXT_COMMAND;
			break;
		case 0x81: /* Turn on text */
			break;
		case 0x82: /* Turn off text */
			break;

		default:
			m_status = LDP_STAT_UNDEF;
			break;
	}
		return;
}
