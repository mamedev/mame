// license:BSD-3-Clause
// copyright-holders:James Wallace
///////////////////////////////////////////////////////////////////////////
//                                                                       //
// steppers.c steppermotor emulation                                     //
//                                                                       //
// Emulates : Stepper motors driven with full step or half step          //
//            also emulates the index optic                              //
//                                                                       //
// 26-05-2012: J. Wallace - Implemented proper phase alignment, we no    //
//                          longer need reverse interfaces here, the     //
//                          layout will suffice. Added belt reel handler.//
// 09-04-2012: J. Wallace - Studied some old reel motors and added a     //
//                          number of new stepper types. I am yet to     //
//                          add them to drivers, but barring some init   //
//                          stuff, they should work.                     //
// 15-01-2012: J. Wallace - Total internal rewrite to remove the table   //
//                          hoodoo that stops anyone but me actually     //
//                          updating this. In theory, we should be able  //
//                          to adapt the phase code to any reel type by  //
//                          studying a game's startup                    //
//                          Documentation is much better now.            //
// 04-04-2011: J. Wallace - Added reverse spin (this is necessary for    //
//                          accuracy), and improved wraparound logic     //
//    03-2011:              New 2D array to remove reel bounce and       //
//                          make more realistic                          //
// 26-01-2007: J. Wallace - Rewritten to make it more flexible           //
//                          and to allow indices to be set in drivers    //
// 29-12-2006: J. Wallace - Added state save support                     //
// 05-03-2004: Re-Animator                                               //
//                                                                       //
// TODO:  add further types of stepper motors if needed (Konami/IGT?)    //
//        200 Step reels can alter their relative opto tab position,     //
//        may be worth adding the phase setting to the interface         //
//        There are reports that some games use a pulse that is too short//
//        to give a 'judder' effect for holds, etc. We'll need to time   //
//        the pulses to keep tack of this without going out of sync.     //
//        Check 20RM and Starpoint 200 step                              //
///////////////////////////////////////////////////////////////////////////

#include "emu.h"
#include "steppers.h"

const device_type STEPPER = &device_creator<stepper_device>;

stepper_device::stepper_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, STEPPER, "Stepper Motor", tag, owner, clock, "stepper", __FILE__),
		m_optic_cb(*this)
	{
		m_max_steps=(48*2);
	}
///////////////////////////////////////////////////////////////////////////

void stepper_device::update_optic()
{
	int pos   = m_step_pos,
		start = m_index_start,
		end = m_index_end;

	if (start > end) // cope with index patterns that wrap around
	{
		if ( (( pos > start ) || ( pos < end )) &&
		( ( m_pattern == m_index_patt || m_index_patt==0) ||
		( m_pattern == 0 &&
		(m_old_pattern == m_index_patt || m_index_patt==0)
		) ) )
		{
			m_optic = 1;
		}
		else m_optic = 0;
		}
	else
	{
		if ( (( pos > start ) && ( pos < end )) &&
		( ( m_pattern == m_index_patt || m_index_patt==0) ||
		( m_pattern == 0 &&
		(m_old_pattern == m_index_patt || m_index_patt==0)
		) ) )
		{
		m_optic = 1;
		}
		else m_optic = 0;
	}

	m_optic_cb(m_optic);
}
///////////////////////////////////////////////////////////////////////////

void stepper_device::device_start()
{
	/* resolve callbacks */
	m_optic_cb.resolve_safe();

	/* register for state saving */
	save_item(NAME(m_index_start));
	save_item(NAME(m_index_end));
	save_item(NAME(m_index_patt));
	save_item(NAME(m_initphase));
	save_item(NAME(m_phase));
	save_item(NAME(m_old_phase));
	save_item(NAME(m_pattern));
	save_item(NAME(m_old_pattern));
	save_item(NAME(m_step_pos));
	save_item(NAME(m_abs_step_pos));
	save_item(NAME(m_max_steps));
	save_item(NAME(m_type));
}

///////////////////////////////////////////////////////////////////////////

void stepper_device::device_reset()
{
	m_step_pos     = 0x00;
	m_abs_step_pos = 0x00;
	m_pattern      = 0x00;
	m_old_pattern  = 0x00;
	m_phase        = m_initphase;
	m_old_phase    = m_initphase;
	update_optic();
}

///////////////////////////////////////////////////////////////////////////

int stepper_device::update(UINT8 pattern)
{
	int changed = 0;

	/* This code probably makes more sense if you visualise what is being emulated, namely
	a spinning drum with two electromagnets inside. Essentially, the CPU
	activates a pair of windings on these magnets leads as necessary to attract and repel the drum to pull it round and
	display as appropriate. To attempt to visualise the rotation effect, take a look at the compass rose below, representing a side on view of the reel,
	the numbers indicate the phase information as used

	    7
	    N
	1 W   E 5
	    S
	    3

	For sake of accuracy, we're representing all possible phases of the motor, effectively moving the motor one half step at a time, so a 48 step motor becomes
	96 half steps. This is necessary because of some programs running the wiring in series with a distinct delay between the pair being completed. This causes
	a small movement that may trigger the optic tab.
	*/

	int pos,steps;
	m_pattern = pattern;
	switch ( m_type )
	{
		default:
		logerror("No reel type specified!\n");
		break;
		case NOT_A_REEL :
		case STARPOINT_48STEP_REEL : /* STARPOINT RMxxx */
		case GAMESMAN_200STEP_REEL : /* Gamesman GMxxxx */
		case STARPOINT_144STEP_DICE :/* STARPOINT 1DCU DICE mechanism */
		case STARPOINT_200STEP_REEL :/* STARPOINT 1DCU DICE mechanism */
		//Standard drive table is 2,6,4,5,1,9,8,a
		//NOTE: This runs through the stator patterns in such a way as to drive the reel forward (downwards from the player's view, clockwise on our rose)
		//The Heber 'Pluto' controller runs this in reverse
		switch (pattern)
		{             //Black  Blue  Red  Yellow
			case 0x02://  0     0     1     0
			m_phase = 7;
			break;
			case 0x06://  0     1     1     0
			m_phase = 6;
			break;
			case 0x04://  0     1     0     0
			m_phase = 5;
			break;
			case 0x05://  0     1     0     1
			m_phase = 4;
			break;
			case 0x01://  0     0     0     1
			m_phase = 3;
			break;
			case 0x09://  1     0     0     1
			m_phase = 2;
			break;
			case 0x08://  1     0     0     0
			m_phase = 1;
			break;
			case 0x0A://  1     0     1     0
			m_phase = 0;
			break;
			//          Black  Blue  Red  Yellow
			case 0x03://  0     0     1     1
			{
				if ((m_old_phase ==6)||(m_old_phase == 0)) // if the previous pattern had the drum in the northern quadrant, it will point north now
				{
					m_phase = 7;
				}
				else //otherwise it will line up due south
				{
					m_phase = 3;
				}
			}
			break;
			case 0x0C://  1     1     0     0
			{
				if ((m_old_phase ==6)||(m_old_phase == 4)) // if the previous pattern had the drum in the eastern quadrant, it will point east now
				{
					m_phase = 5;
				}
				else //otherwise it will line up due west
				{
					m_phase = 1;
				}
			}
			break;
		}
		break;

		case BARCREST_48STEP_REEL :
		case GAMESMAN_48STEP_REEL :
		case GAMESMAN_100STEP_REEL :
		//Standard drive table is 1,3,2,6,4,C,8,9
		//Gamesman 48 step uses this pattern shifted one place forward, though this shouldn't matter
		switch (pattern)
		{
			//             Yellow   Brown  Orange Black
			case 0x01://  0        0      0      1
			m_phase = 7;
			break;
			case 0x03://  0        0      1      1
			m_phase = 6;
			break;
			case 0x02://  0        0      1      0
			m_phase = 5;
			break;
			case 0x06://  0        1      1      0
			m_phase = 4;
			break;
			case 0x04://  0        1      0      0
			m_phase = 3;
			break;
			case 0x0C://  1        1      0      0
			m_phase = 2;
			break;
			case 0x08://  1        0      0      0
			m_phase = 1;
			break;//YOLB
			case 0x09://  1        0      0      1
			m_phase = 0;
			break;

			// The below values should not be used by anything sane, as they effectively ignore one stator side entirely
			//          Yellow   Brown  Orange Black
			case 0x05://   0       1       0     1
			{
				if ((m_old_phase ==6)||(m_old_phase == 0)) // if the previous pattern had the drum in the northern quadrant, it will point north now
				{
					m_phase = 7;
				}
				else //otherwise it will line up due south
				{
					m_phase = 3;
				}
			}
			break;

			case 0x0A://   1       0       1     0
			{
				if ((m_old_phase ==6)||(m_old_phase == 4)) // if the previous pattern had the drum in the eastern quadrant, it will point east now
				{
					m_phase = 5;
				}
				else //otherwise it will line up due west
				{
					m_phase = 1;
				}
			}
			break;
		}
		break;

		case MPU3_48STEP_REEL :
		/* The MPU3 interface is actually the same as the MPU4 setup, but with two active lines instead of four
		   Inverters are used so if a pin is low, the higher bit of the pair is activated, and if high the lower bit is activated.
		   TODO:Check this, 2 and 1 could be switched over.
		 */
		switch (pattern)
		{
		//             Yellow(2)   Brown(1)  Orange(!2) Black(!1)
			case 0x00 :// 0          0          1         1
			m_phase = 6;
			break;
			case 0x01 :// 0          1          1         0
			m_phase = 4;
			break;
			case 0x03 :// 1          1          0         0
			m_phase = 2;
			break;
			case 0x02 :// 1          0          0         1
			m_phase = 0;
			break;
		}
		break;

		case ECOIN_200STEP_REEL :
		//While the 48 and 100 step models appear to be reverse driven Starpoint reels, the 200 step model seems bespoke, certainly in terms of wiring.
		//On a Proconn machine this same pattern is seen but running in reverse
		//Standard drive table is 8,c,4,6,2,3,1,9
		switch (pattern)
		{
			case 0x08://  0     0     1     0
			m_phase = 7;
			break;
			case 0x0c://  0     1     1     0
			m_phase = 6;
			break;
			case 0x04://  0     1     0     0
			m_phase = 5;
			break;
			case 0x06://  0     1     0     1
			m_phase = 4;
			break;
			case 0x02://  0     0     0     1
			m_phase = 3;
			break;
			case 0x03://  1     0     0     1
			m_phase = 2;
			break;
			case 0x01://  1     0     0     0
			m_phase = 1;
			break;
			case 0x09://  1     0     1     0
			m_phase = 0;
			break;
			case 0x0a://  0     0     1     1
			{
				if ((m_old_phase ==6)||(m_old_phase == 0)) // if the previous pattern had the drum in the northern quadrant, it will point north now
				{
					m_phase = 7;
				}
				else //otherwise it will line up due south
				{
					m_phase = 3;
				}
			}
			break;
			case 0x07://  1     1     0     0
			{
				if ((m_old_phase ==6)||(m_old_phase == 4)) // if the previous pattern had the drum in the eastern quadrant, it will point east now
				{
					m_phase = 5;
				}
				else //otherwise it will line up due west
				{
					m_phase = 1;
				}
			}
			break;
		}
		break;

		case PROJECT_48STEP_REEL :
		//Standard drive table is 8,c,4,5,1,3,2,a
		//This appears to be basically a rewired Gamesman (the reel PCB looks like it does some shuffling)
		//TODO: Not sure if this should be represented as a type here, or by defining it as a Gamesman in the driver and bitswapping.
		switch (pattern)
		{
			case 0x08://  0     0     1     0
			m_phase = 7;
			break;
			case 0x0c://  0     1     1     0
			m_phase = 6;
			break;
			case 0x04://  0     1     0     0
			m_phase = 5;
			break;
			case 0x05://  0     1     0     1
			m_phase = 4;
			break;
			case 0x01://  0     0     0     1
			m_phase = 3;
			break;
			case 0x03://  1     0     0     1
			m_phase = 2;
			break;
			case 0x02://  1     0     0     0
			m_phase = 1;
			break;
			case 0x0a://  1     0     1     0
			m_phase = 0;
			break;
			case 0x09://  0     0     1     1
			{
				if ((m_old_phase ==6)||(m_old_phase == 0)) // if the previous pattern had the drum in the northern quadrant, it will point north now
				{
					m_phase = 7;
				}
				else //otherwise it will line up due south
				{
					m_phase = 3;
				}
			}
			break;
			case 0x06://  1     1     0     0
			{
				if ((m_old_phase ==6)||(m_old_phase == 4)) // if the previous pattern had the drum in the eastern quadrant, it will point east now
				{
					m_phase = 5;
				}
				else //otherwise it will line up due west
				{
					m_phase = 1;
				}
			}
			break;
		}
		break;



	}

	steps = m_old_phase - m_phase;

	if (steps < -4)
	{
		steps = steps +8;
	}
	if (steps > 4)
	{
		steps = steps -8;
	}

	m_old_phase   = m_phase;
	m_old_pattern = m_pattern;

	int max = m_max_steps;
	pos = 0;

	if (max!=0)
	{
		m_abs_step_pos += steps;
		pos = (m_step_pos + steps + max) % max;
	}

	if (pos != m_step_pos)
	{
		changed++;
	}

	m_step_pos = pos;
	update_optic();

	return changed;
}
