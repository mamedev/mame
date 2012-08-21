/***************************************************************************

  macadb.c - handles various aspects of ADB on the Mac.

***************************************************************************/

#include "emu.h"
#include "includes/mac.h"

#define LOG_ADB				0
#define LOG_ADB_MCU_CMD		0
#define LOG_ADB_TALK_LISTEN 0

// ADB states
#define ADB_STATE_NEW_COMMAND	(0)
#define ADB_STATE_XFER_EVEN	(1)
#define ADB_STATE_XFER_ODD	(2)
#define ADB_STATE_IDLE		(3)
#define ADB_STATE_NOTINIT	(4)

// ADB commands
#define ADB_CMD_RESET		(0)
#define ADB_CMD_FLUSH		(1)

/* *************************************************************************
 * ADB (Mac II-style)
 * *************************************************************************/

#if LOG_ADB
static const char *const adb_statenames[4] = { "NEW", "EVEN", "ODD", "IDLE" };
#endif

int mac_state::adb_pollkbd(int update)
{
	int i, j, keybuf, report, codes[2], result;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };

	codes[0] = codes[1] = 0xff;	// key up
	report = result = 0;

	for (i = 0; i < 6; i++)
	{
		keybuf = ioport(keynames[i])->read();

		// any changes in this row?
		if ((keybuf != m_key_matrix[i]) && (report < 2))
		{
			// check each column bit
			for (j=0; j<16; j++)
			{
				if (((keybuf ^ m_key_matrix[i]) >> j) & 1)
				{
					// update m_key_matrix
					if (update)
					{
						m_key_matrix[i] = (m_key_matrix[i] & ~ (1 << j)) | (keybuf & (1 << j));
					}

					codes[report] = (i<<4)|j;

					// key up?
					if (!(keybuf & (1 << j)))
					{
						codes[report] |= 0x80;
					}

					// update modifier state
					if (update)
					{
						if (((i<<4)|j) == 0x39)
						{
							if (codes[report] & 0x80)
							{
								m_adb_modifiers &= ~0x20;
							}
							else
							{
								m_adb_modifiers |= 0x20;
							}
						}
						if (((i<<4)|j) == 0x36)
						{
							if (codes[report] & 0x80)
							{
								m_adb_modifiers &= ~0x8;
							}
							else
							{
								m_adb_modifiers |= 0x08;
							}
						}
						if (((i<<4)|j) == 0x38)
						{
							if (codes[report] & 0x80)
							{
								m_adb_modifiers &= ~0x4;
							}
							else
							{
								m_adb_modifiers |= 0x04;
							}
						}
						if (((i<<4)|j) == 0x3a)
						{
							if (codes[report] & 0x80)
							{
								m_adb_modifiers &= ~0x2;
							}
							else
							{
								m_adb_modifiers |= 0x02;
							}
						}
						if (((i<<4)|j) == 0x37)
						{
							if (codes[report] & 0x80)
							{
								m_adb_modifiers &= ~0x1;
							}
							else
							{
								m_adb_modifiers |= 0x01;
							}
						}
					}

					// we run out of keys we can track?
					report++;
					if (report == 2)
					{
						break;
					}
				}
			}

			// we run out of keys we can track?
			if (report == 2)
			{
				break;
			}
		}
	}

//  printf("ADB keyboard: update %d keys %02x %02x\n", update, codes[0], codes[1]);

	// figure out if there was a change
	if ((m_adb_currentkeys[0] != codes[0]) || (m_adb_currentkeys[1] != codes[1]))
	{
		result = 1;

		// if we want to update the current read, do so
		if (update)
		{
			m_adb_currentkeys[0] = codes[0];
			m_adb_currentkeys[1] = codes[1];
		}
	}

	return result;
}

int mac_state::adb_pollmouse()
{
	int NewX, NewY, NewButton;

	if (!m_adb_mouse_initialized)
	{
		return 0;
	}

	NewButton = ioport("MOUSE0")->read() & 0x01;
	NewX = ioport("MOUSE2")->read();
	NewY = ioport("MOUSE1")->read();

	if ((NewX != m_adb_lastmousex) || (NewY != m_adb_lastmousey) || (NewButton != m_adb_lastbutton))
	{
		return 1;
	}

	return 0;
}

void mac_state::adb_accummouse( UINT8 *MouseX, UINT8 *MouseY )
{
	int MouseCountX = 0, MouseCountY = 0;
	int NewX, NewY;

	NewX = ioport("MOUSE2")->read();
	NewY = ioport("MOUSE1")->read();

	/* see if it moved in the x coord */
	if (NewX != m_adb_lastmousex)
	{
		int diff = NewX - m_adb_lastmousex;

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100-diff;
		if  (diff < -0x80)
			diff = -0x100-diff;

		MouseCountX += diff;
		m_adb_lastmousex = NewX;
	}

	/* see if it moved in the y coord */
	if (NewY != m_adb_lastmousey)
	{
		int diff = NewY - m_adb_lastmousey;

		/* check for wrap */
		if (diff > 0x80)
			diff = 0x100-diff;
		if  (diff < -0x80)
			diff = -0x100-diff;

		MouseCountY += diff;
		m_adb_lastmousey = NewY;
	}

	m_adb_lastbutton = ioport("MOUSE0")->read() & 0x01;

	*MouseX = (UINT8)MouseCountX;
	*MouseY = (UINT8)MouseCountY;
}

void mac_state::adb_talk()
{
	int addr, reg;

	addr = (m_adb_command>>4);
	reg = (m_adb_command & 3);

//  printf("Mac sent %x (cmd %d addr %d reg %d mr %d kr %d)\n", mac->m_adb_command, (mac->m_adb_command>>2)&3, addr, reg, m_adb_mouseaddr, m_adb_keybaddr);

	if (m_adb_waiting_cmd)
	{
		switch ((m_adb_command>>2)&3)
		{
			case 0:
			case 1:
				switch (reg)
				{
					case ADB_CMD_RESET:
						#if LOG_ADB || LOG_ADB_TALK_LISTEN
						printf("ADB RESET: reg %x address %x\n", reg, addr);
						#endif
						m_adb_direction = 0;
						m_adb_send = 0;
						break;

					case ADB_CMD_FLUSH:
						#if LOG_ADB || LOG_ADB_TALK_LISTEN
						printf("ADB FLUSH: reg %x address %x\n", reg, addr);
						#endif

						m_adb_direction = 0;
						m_adb_send = 0;
						break;

					default:	// reserved/unused
						break;
				}
				break;

			case 2:	// listen
				#if LOG_ADB || LOG_ADB_TALK_LISTEN
				printf("ADB LISTEN: reg %x address %x\n", reg, addr);
				#endif

				m_adb_direction = 1;	// input from Mac
				m_adb_listenreg = reg;
				m_adb_listenaddr = addr;
				m_adb_command = 0;
				break;

			case 3: // talk
				#if LOG_ADB || LOG_ADB_TALK_LISTEN
				printf("ADB TALK: reg %x address %x\n", reg, addr);
				#endif

				// keep track of what device the Mac last TALKed to
				m_adb_last_talk = addr;

				m_adb_direction = 0;	// output to Mac
				if (addr == m_adb_mouseaddr)
				{
					UINT8 mouseX, mouseY;

					#if LOG_ADB || LOG_ADB_TALK_LISTEN
					printf("Talking to mouse, register %x\n", reg);
					#endif

					switch (reg)
					{
						// read mouse
						case 0:
							if (m_adb_srq_switch)
							{
								m_adb_srq_switch = 0;
								mouseX = mouseY = 0;
							}
							else
							{
								this->adb_accummouse(&mouseX, &mouseY);
							}
							m_adb_buffer[0] = (m_adb_lastbutton & 0x01) ? 0x00 : 0x80;
							m_adb_buffer[0] |= mouseX & 0x7f;
							m_adb_buffer[1] = mouseY & 0x7f;
							m_adb_datasize = 2;
							break;

						// get ID/handler
						case 3:
							m_adb_buffer[0] = 0x60 | ((m_adb_mouseaddr<<8)&0xf);	// SRQ enable, no exceptional event
							m_adb_buffer[1] = 0x01;	// handler 1
							m_adb_datasize = 2;

							m_adb_mouse_initialized = 1;
							break;

						default:
							break;
					}
				}
				else if (addr == m_adb_keybaddr)
				{
					#if LOG_ADB || LOG_ADB_TALK_LISTEN
					printf("Talking to keyboard, register %x\n", reg);
					#endif

					switch (reg)
					{
						// read keyboard
						case 0:
							if (m_adb_srq_switch)
							{
								m_adb_srq_switch = 0;
							}
							else
							{
								this->adb_pollkbd(1);
							}
//                          printf("keyboard = %02x %02x\n", m_adb_currentkeys[0], m_adb_currentkeys[1]);
							m_adb_buffer[0] = m_adb_currentkeys[1];
							m_adb_buffer[1] = m_adb_currentkeys[0];
							m_adb_datasize = 2;
							break;

						// read modifier keys
						case 2:
							this->adb_pollkbd(1);
							m_adb_buffer[0] = m_adb_modifiers;	// nothing pressed
							m_adb_buffer[1] = 0;
							m_adb_datasize = 2;
							break;

						// get ID/handler
						case 3:
							m_adb_buffer[0] = 0x60 | ((m_adb_keybaddr<<8)&0xf);	// SRQ enable, no exceptional event
							m_adb_buffer[1] = 0x01;	// handler 1
							m_adb_datasize = 2;

							m_adb_keybinitialized = 1;
							break;

						default:
							break;
					}
				}
				else
				{
					#if LOG_ADB || LOG_ADB_TALK_LISTEN
					printf("ADB: talking to unconnected device %d (K %d M %d)\n", addr, m_adb_keybaddr, m_adb_mouseaddr);
					#endif
					m_adb_buffer[0] = m_adb_buffer[1] = 0;
					m_adb_datasize = 0;
				}
				break;
		}

		m_adb_waiting_cmd = 0;
	}
	else
	{
		#if LOG_ADB || LOG_ADB_TALK_LISTEN
		printf("Got LISTEN data %x for device %x reg %x\n", m_adb_command, m_adb_listenaddr, m_adb_listenreg);
		#endif

		if (m_adb_listenaddr == m_adb_mouseaddr)
		{
			if ((m_adb_listenreg == 3) && (m_adb_command > 0) && (m_adb_command < 16))
			{
				#if LOG_ADB || LOG_ADB_TALK_LISTEN
				printf("MOUSE: moving to address %x\n", m_adb_command);
				#endif
				m_adb_mouseaddr = m_adb_command&0x0f;
			}
		}
		else if (m_adb_listenaddr == m_adb_keybaddr)
		{
			if ((m_adb_listenreg == 3) && (m_adb_command > 0) && (m_adb_command < 16))
			{
				#if LOG_ADB || LOG_ADB_TALK_LISTEN
				printf("KEYBOARD: moving to address %x\n", m_adb_command);
				#endif
				m_adb_keybaddr = m_adb_command&0x0f;
			}
		}
	}
}

TIMER_CALLBACK(mac_adb_tick)
{
	mac_state *mac = machine.driver_data<mac_state>();

	// do one clock transition on CB1 to advance the VIA shifter
	mac->m_adb_extclock ^= 1;
	mac->m_via1->write_cb1(mac->m_adb_extclock);
	mac->m_adb_extclock ^= 1;
	mac->m_via1->write_cb1(mac->m_adb_extclock);

	mac->m_adb_timer_ticks--;
	if (!mac->m_adb_timer_ticks)
	{
		mac->m_adb_timer->adjust(attotime::never);

		if ((mac->m_adb_direction) && (ADB_IS_BITBANG))
		{
			mac->adb_talk();
		}
	}
	else
	{
		mac->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
	}
}

void mac_state::mac_adb_newaction(int state)
{
	if (state != m_adb_state)
	{
		#if LOG_ADB
		printf("New ADB state: %s\n", adb_statenames[state]);
		#endif

		m_adb_state = state;
		m_adb_timer_ticks = 8;

		switch (state)
		{
			case ADB_STATE_NEW_COMMAND:
				m_adb_command = m_adb_send = 0;
				m_adb_direction = 1;	// Mac is shifting us a command
				m_adb_waiting_cmd = 1;	// we're going to get a command
				m_adb_irq_pending = 0;
				m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
				break;

			case ADB_STATE_XFER_EVEN:
			case ADB_STATE_XFER_ODD:
				if (m_adb_datasize > 0)
				{
					int i;

					// is something trying to send to the Mac?
					if (m_adb_direction == 0)
					{
						// set up the byte
						m_adb_send = m_adb_buffer[0];
						m_adb_datasize--;

						// move down the rest of the buffer, if any
						for (i = 0; i < m_adb_datasize; i++)
						{
							m_adb_buffer[i] = m_adb_buffer[i+1];
						}
					}

				}
				else
				{
					m_adb_send = 0;
					m_adb_irq_pending = 1;
				}

				m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
				break;

			case ADB_STATE_IDLE:
				m_adb_irq_pending = 0;
				break;
		}
	}
}

TIMER_CALLBACK(mac_pmu_tick)
{
	mac_state *mac = machine.driver_data<mac_state>();

	// state 10 means this is in response to an ADB command
	if (mac->m_pm_state == 10)
	{
		#if LOG_ADB
		printf("PM: was state 10, chunk-chunking CB1\n");
		#endif
		mac->m_pm_state = 0;

		// tick CB1, which should cause a PMU interrupt on PMU machines
		mac->m_adb_extclock ^= 1;
		mac->m_via1->write_cb1(mac->m_adb_extclock);
		mac->m_adb_extclock ^= 1;
		mac->m_via1->write_cb1(mac->m_adb_extclock);
	}
	else
	{
		#if LOG_ADB
		printf("PM: timer tick, lowering ACK\n");
		#endif
		mac->m_pm_ack &= ~2;	// lower ACK to handshake next step
	}
}

void mac_state::pmu_one_byte_reply(UINT8 result)
{
	m_pm_out[0] = m_pm_out[1] = 1;	// length
	m_pm_out[2] = result;
	m_pm_slen = 3;
	m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
}

void mac_state::pmu_three_byte_reply(UINT8 result1, UINT8 result2, UINT8 result3)
{
	m_pm_out[0] = m_pm_out[1] = 3;	// length
	m_pm_out[2] = result1;
	m_pm_out[3] = result2;
	m_pm_out[4] = result3;
	m_pm_slen = 5;
	m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
}

void mac_state::pmu_exec()
{
	m_pm_sptr = 0;	// clear send pointer
	m_pm_slen = 0;	// and send length
	m_pm_dptr = 0;	// and receive pointer

//  printf("PMU: Command %02x\n", mac->m_pm_cmd[0]);
	switch (m_pm_cmd[0])
	{
		case 0x10:	// subsystem power and clock ctrl
			break;

		case 0x20:	// send ADB command (PMU must issue an IRQ on completion)
			#if 0
			printf("PMU: Send ADB %02x %02x cmd %02x flag %02x data %02x %02x\n",
				   m_pm_cmd[0],	// 0x20
				   m_pm_cmd[1],	// ???
				   m_pm_cmd[2],	// adb flags (2 for autopoll active, 3 to reset bus?)
				   m_pm_cmd[3],	// length of ADB data
				   m_pm_cmd[4],	// adb data
				   m_pm_cmd[5]);
			#endif

			#if 0
			m_pm_state = 10;
			m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
			if (ADB_IS_PM_VIA1_CLASS)
			{
				m_pmu_int_status = 0x1;
			}
			else if (ADB_IS_PM_VIA2_CLASS)
			{
				m_pmu_int_status = 0x10;
			}
			else
			{
				fatalerror("mac: unknown ADB PMU type\n");
			}
			m_pmu_last_adb_command = m_pm_cmd[2];
			m_adb_command = m_pm_cmd[2];
			m_adb_waiting_cmd = 1;
			adb_talk();

			if ((m_pm_cmd[2] & 0xf) == 0xb)	// LISTEN register 3 (remap)
			{
				m_adb_waiting_cmd = 0;
				m_adb_command = mac->m_pm_cmd[5];
				adb_talk();
			}
			#else
			if (((m_pm_cmd[2] == 0xfc) || (m_pm_cmd[2] == 0x2c)) && (m_pm_cmd[3] == 4))
			{
//              printf("PMU: request to poll ADB, returning nothing\n");
				m_pm_slen = 0;
				m_pmu_int_status = 0;
			}
			else
			{
				m_pm_state = 10;
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
				if (ADB_IS_PM_VIA1_CLASS)
				{
					m_pmu_int_status = 0x1;
				}
				else if (ADB_IS_PM_VIA2_CLASS)
				{
					m_pmu_int_status = 0x10;
				}
				else
				{
					fatalerror("mac: unknown ADB PMU type\n");
				}
				m_pmu_last_adb_command = m_pm_cmd[2];
			}

			m_adb_command = m_pm_cmd[2];
			m_adb_waiting_cmd = 1;
			adb_talk();
			#endif
			break;

		case 0x21:	// turn ADB auto-poll off (does this need a reply?)
			break;

		case 0x28:	// read ADB
			if (m_adb_datasize > 0)
			{
				m_adb_datasize = 1; // hack

				m_pm_out[0] = m_pm_out[1] = 3 + m_adb_datasize;
				m_pm_out[2] = 0;
//              m_pm_out[3] = m_pmu_last_adb_command;
				m_pm_out[3] = 0;
				m_pm_out[4] = m_adb_datasize;
				for (int i = 0; i < m_adb_datasize; i++)
				{
					m_pm_out[5+i] = 0; //mac->m_adb_buffer[i];
				}
				m_pm_slen = 5 + m_adb_datasize;
			}
			else
			{
				m_pm_out[0] = m_pm_out[1] = 4;
				m_pm_out[2] = 0;
				m_pm_out[3] = 0;
//              m_pm_out[3] = m_pmu_last_adb_command;
				m_pm_out[4] = 1;	// length of following data
				m_pm_out[5] = 0;
				m_pm_slen = 6;
			}
/*          printf("ADB packet: ");
            for (int i = 0; i < m_pm_slen; i++)
            {
                printf("%02x ", m_pm_out[i]);
            }
            printf("\n");*/
			m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(1000)));
			break;

		case 0x31:  // write first 20 bytes of PRAM
			{
				for (int i = 0; i < 20; i++)
				{
					m_rtc_ram[i] = m_pm_cmd[1+i];
				}
			}
			break;

		case 0x32:	// write extended PRAM byte(s).  cmd[2] = address, cmd[3] = length, cmd[4...] = data
			if ((m_pm_cmd[2] + m_pm_cmd[3]) < 0x100)
			{
				int i;

				for (i = 0; i < m_pm_cmd[3]; i++)
				{
					m_rtc_ram[m_pm_cmd[2] + i] = m_pm_cmd[4+i];
				}
			}
			break;

		case 0x38:  // read time
			{
				m_pm_out[0] = m_pm_out[1] = 4;
				m_pm_out[2] = m_rtc_seconds[0];
				m_pm_out[3] = m_rtc_seconds[1];
				m_pm_out[4] = m_rtc_seconds[2];
				m_pm_out[5] = m_rtc_seconds[3];
				m_pm_slen = 6;
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
			}
			break;

		case 0x39:	// read first 20 bytes of PRAM
			{
				int i;

				m_pm_out[0] = m_pm_out[1] = 20;
				for (i = 0; i < 20; i++)
				{
					m_pm_out[2 + i] = m_rtc_ram[i];
				}
				m_pm_slen = 22;
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
			}
			break;

		case 0x3a:	// read extended PRAM byte(s).  cmd[2] = address, cmd[3] = length
			if ((m_pm_cmd[2] + m_pm_cmd[3]) < 0x100)
			{
				int i;

				m_pm_out[0] = m_pm_out[1] = m_pm_cmd[3];
				for (i = 0; i < m_pm_cmd[3]; i++)
				{
					m_pm_out[2 + i] = m_rtc_ram[m_pm_cmd[2] + i];
				}
				m_pm_slen = m_pm_out[0] + 2;
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
			}
			break;

		case 0x40:  // set screen contrast
			break;

		case 0x41:
			break;

		case 0x58:	// read internal modem status
			pmu_one_byte_reply(0);
			break;

		case 0x60:	// set low power warning and cutoff battery levels
			break;

		case 0x68:	// read battery/charger level
			pmu_three_byte_reply(255, 255, 255);
			break;

		case 0x69:  // read battery/charger instantaneous level and status
			pmu_three_byte_reply(255, 255, 255);
			break;

		case 0x6b:	// read extended battery/charger level and status (wants an 8 byte reply)
			m_pm_out[0] = m_pm_out[1] = 8;	// length
			m_pm_out[2] = 255;
			m_pm_out[3] = 255;
			m_pm_out[4] = 255;
			m_pm_out[5] = 255;
			m_pm_out[6] = 255;
			m_pm_out[7] = 255;
			m_pm_out[8] = 255;
			m_pm_out[9] = 255;
			m_pm_slen = 10;
			m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
			break;

		case 0x6c:	// read battery ID
			pmu_one_byte_reply(1);
			break;

		case 0x78:	// read interrupt flag
			if (ADB_IS_PM_VIA2_CLASS)	// PB 140/170 use a "leaner" PMU protocol where you get the data for a PMU interrupt here
			{
				#if 0
				if ((m_pmu_int_status&0xf0) == 0x10)
				{
					if (m_adb_datasize > 0)
					{
						m_adb_datasize = 1; // hack
						m_pm_out[0] = m_pm_out[1] = 2 + m_adb_datasize;
						m_pm_out[2] = m_pmu_int_status; // ADB status in low nibble
						m_pm_out[3] = m_pmu_last_adb_command;		  // ADB command that was sent
						for (int i = 0; i < m_adb_datasize; i++)
						{
							m_pm_out[4+i] = 0; //m_adb_buffer[i];
						}
						m_pm_slen = 4 + m_adb_datasize;
						m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(1500)));

/*                      printf("ADB packet: ");
                        for (int i = 0; i < m_pm_slen; i++)
                        {
                            printf("%02x ", m_pm_out[i]);
                        }
                        printf("\n");*/
					}
					else
					{
						m_pm_out[0] = m_pm_out[1] = 2;
						m_pm_out[2] = m_pmu_int_status; // ADB status in low nibble
						m_pm_out[3] = m_pmu_last_adb_command;		  // ADB command that was sent OR 0x80 for extra error-ness
						m_pm_out[4] = 0;							  // return data
						m_pm_slen = 4;
						m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(1500)));
					}
				}
				else
				{
					pmu_one_byte_reply(mac, m_pmu_int_status);
				}
				#else
				if ((m_pmu_int_status&0xf0) == 0x10)
				{
					m_pm_out[0] = m_pm_out[1] = 2;
					m_pm_out[2] = m_pmu_int_status; // ADB status in low nibble
					m_pm_out[3] = m_pmu_last_adb_command;		  // ADB command that was sent OR 0x80 for extra error-ness
					m_pm_out[4] = 0;							  // return data
					m_pm_slen = 4;
					m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(1500)));
				}
				else
				{
					pmu_one_byte_reply(m_pmu_int_status);
				}
				#endif
			}
			else
			{
				pmu_one_byte_reply(m_pmu_int_status);
			}

			m_pmu_int_status = 0;
			break;

		case 0x90: // sound power control
			break;

		case 0x98:	// read sound power state
			pmu_one_byte_reply(1);
			break;

		case 0xd8:	// read A/D converter (not sure what this does)
			pmu_one_byte_reply(0);
			break;

		case 0xe0:	// write PMU internal RAM
			break;

		case 0xe8:  // read PMU internal RAM (just return zeroes)
			{
				int i;

				m_pm_out[0] = m_pm_out[1] = m_pm_cmd[4];
//              printf("PMU read at %x\n", m_pm_cmd[2] | (m_pm_cmd[3]<<8));

				// note: read at 0xEE00 0 = target disk mode, 0xff = normal bootup
				// (actually 0x00EE, the 50753 port 6)

				for (i = 0; i < m_pm_cmd[4]; i++)
				{
					m_pm_out[2 + i] = 0xff;
				}
				m_pm_slen = m_pm_out[0] + 2;
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
			}
			break;

		case 0xec:	// PMU self-test (send 1 count byte + reply)
			pmu_one_byte_reply(0);
			break;

		default:
			fatalerror("PMU: Unhandled command %02x\n", m_pm_cmd[0]);
			break;
	}

	if (m_pm_slen > 0)
	{
		m_pm_state = 1;
	}
}

void mac_state::adb_vblank()
{
	if ((m_adb_state == ADB_STATE_IDLE) || ((ADB_IS_PM_CLASS) && (m_pmu_poll)))
	{
		if (this->adb_pollmouse())
		{
			// if the mouse was the last TALK, we can just send the new data
			// otherwise we need to pull SRQ
			if ((m_adb_last_talk == m_adb_mouseaddr) && !(ADB_IS_PM_CLASS))
			{
				// repeat last TALK to get updated data
				m_adb_waiting_cmd = 1;
				this->adb_talk();

				m_adb_timer_ticks = 8;
				this->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
			}
			#if 0
			else if (ADB_IS_PM_CLASS)
			{
				m_adb_waiting_cmd = 1;
				this->adb_talk();
				m_pm_state = 10;
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
				if (ADB_IS_PM_VIA1_CLASS)
				{
					m_pmu_int_status = 0x1;
				}
				else if (ADB_IS_PM_VIA2_CLASS)
				{
					m_pmu_int_status = 0x10;
				}
			}
			#endif
			else
			{
				m_adb_irq_pending = 1;
				m_adb_command = m_adb_send = 0;
				m_adb_timer_ticks = 1;	// one tick should be sufficient to make it see  the IRQ
				this->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
				m_adb_srq_switch = 1;
			}
		}
		else if (this->adb_pollkbd(0))
		{
			if ((m_adb_last_talk == m_adb_keybaddr) && !(ADB_IS_PM_CLASS))
			{
				// repeat last TALK to get updated data
				m_adb_waiting_cmd = 1;
				this->adb_talk();

				m_adb_timer_ticks = 8;
				this->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
			}
			#if 0
			else if (ADB_IS_PM_CLASS)
			{
				m_adb_waiting_cmd = 1;
				this->adb_talk();
				m_pm_state = 10;
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
				m_pmu_int_status = 0x1;
			}
			#endif
			else
			{
				m_adb_irq_pending = 1;
				m_adb_command = m_adb_send = 0;
				m_adb_timer_ticks = 1;	// one tick should be sufficient to make it see  the IRQ
				this->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
				m_adb_srq_switch = 1;
			}
		}
	}
}

void mac_state::adb_reset()
{
	int i;

	m_adb_srq_switch = 0;
	m_adb_irq_pending = 0;		// no interrupt
	m_adb_timer_ticks = 0;
	m_adb_command = 0;
	m_adb_extclock = 0;
	m_adb_send = 0;
	m_adb_waiting_cmd = 0;
	m_adb_streaming = MCU_STREAMING_NONE;
	m_adb_state = 0;
	m_pmu_poll = 0;
	if (ADB_IS_BITBANG_CLASS)
	{
		m_adb_state = ADB_STATE_NOTINIT;
	}
	m_adb_direction = 0;
	m_adb_datasize = 0;
	m_adb_last_talk = -1;

	// mouse
	m_adb_mouseaddr = 3;
	m_adb_lastmousex = m_adb_lastmousey = m_adb_lastbutton = 0;
	m_adb_mouse_initialized = 0;

	// keyboard
	m_adb_keybaddr = 2;
	m_adb_keybinitialized = 0;
	m_adb_currentkeys[0] = m_adb_currentkeys[1] = 0xff;
	m_adb_modifiers = 0;
	for (i=0; i<7; i++)
	{
		m_key_matrix[i] = 0;
	}
}

