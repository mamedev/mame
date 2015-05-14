// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  macadb.c - handles various aspects of ADB on the Mac.

***************************************************************************/

#include "emu.h"
#include "includes/mac.h"

#define LOG_ADB             0
#define LOG_ADB_MCU_CMD     0
#define LOG_ADB_TALK_LISTEN 0

// ADB states
#define ADB_STATE_NEW_COMMAND   (0)
#define ADB_STATE_XFER_EVEN (1)
#define ADB_STATE_XFER_ODD  (2)
#define ADB_STATE_IDLE      (3)
#define ADB_STATE_NOTINIT   (4)

// ADB commands
#define ADB_CMD_RESET       (0)
#define ADB_CMD_FLUSH       (1)

// ADB line states
enum
{
	// receive states
	LST_IDLE = 0,
	LST_ATTENTION,
	LST_BIT0,
	LST_BIT1,
	LST_BIT2,
	LST_BIT3,
	LST_BIT4,
	LST_BIT5,
	LST_BIT6,
	LST_BIT7,
	LST_TSTOP,
	LST_WAITT1T,
	LST_RCVSTARTBIT,
	LST_SRQNODATA,

	// send states
	LST_TSTOPSTART,
	LST_TSTOPSTARTa,
	LST_STARTBIT,
	LST_SENDBIT0,
	LST_SENDBIT0a,
	LST_SENDBIT1,
	LST_SENDBIT1a,
	LST_SENDBIT2,
	LST_SENDBIT2a,
	LST_SENDBIT3,
	LST_SENDBIT3a,
	LST_SENDBIT4,
	LST_SENDBIT4a,
	LST_SENDBIT5,
	LST_SENDBIT5a,
	LST_SENDBIT6,
	LST_SENDBIT6a,
	LST_SENDBIT7,
	LST_SENDBIT7a,
	LST_SENDSTOP,
	LST_SENDSTOPa
};

/* *************************************************************************
 * High-level ADB primitives used by all lower-level implementations
 * *************************************************************************/

#if LOG_ADB
static const char *const adb_statenames[4] = { "NEW", "EVEN", "ODD", "IDLE" };
#endif

int mac_state::adb_pollkbd(int update)
{
	int i, j, keybuf, report, codes[2], result;
	ioport_port *ports[6] = { m_key0, m_key1,   m_key2, m_key3, m_key4, m_key5 };

	codes[0] = codes[1] = 0xff; // key up
	report = result = 0;

	for (i = 0; i < 6; i++)
	{
		keybuf = ports[i]->read();

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
			if(m_adb_currentkeys[0] != codes[0]) {
				m_adb_keybuf[m_adb_keybuf_end] = codes[0];
				m_adb_keybuf_end = (m_adb_keybuf_end+1) % kADBKeyBufSize;
			}
			if(m_adb_currentkeys[1] != codes[1]) {
				m_adb_keybuf[m_adb_keybuf_end] = codes[1];
				m_adb_keybuf_end = (m_adb_keybuf_end+1) % kADBKeyBufSize;
			}
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

	NewButton = m_mouse0->read() & 0x01;
	NewX = m_mouse2->read();
	NewY = m_mouse1->read();

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

	m_adb_lastbutton = m_mouse0->read() & 0x01;

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

					default:    // reserved/unused
						break;
				}
				break;

			case 2: // listen
				m_adb_datasize = 0;
				if ((addr == m_adb_keybaddr) || (addr == m_adb_mouseaddr))
				{
					#if LOG_ADB || LOG_ADB_TALK_LISTEN
					printf("ADB LISTEN: reg %x address %x\n", reg, addr);
					#endif
					m_adb_direction = 1;    // input from Mac
					m_adb_command = 0;
					m_adb_listenreg = reg;
					m_adb_listenaddr = addr;
					if ((ADB_IS_EGRET) || (ADB_IS_CUDA))
					{
						m_adb_stream_ptr = 0;
						memset(m_adb_buffer, 0, sizeof(m_adb_buffer));
					}
				}
				else
				{
					#if LOG_ADB || LOG_ADB_TALK_LISTEN
					printf("ADB LISTEN to unknown device, timing out\n");
					#endif
					m_adb_direction = 0;
				}
				break;

			case 3: // talk
				#if LOG_ADB || LOG_ADB_TALK_LISTEN
				printf("ADB TALK: reg %x address %x (K %x M %x)\n", reg, addr, m_adb_keybaddr, m_adb_mouseaddr);
				#endif

				// keep track of what device the Mac last TALKed to
				m_adb_last_talk = addr;

				m_adb_direction = 0;    // output to Mac
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
							m_adb_buffer[0] = 0x60 | (m_adb_mouseaddr&0xf); // SRQ enable, no exceptional event
							m_adb_buffer[1] = 0x01; // handler 1
							m_adb_datasize = 2;

							m_adb_mouse_initialized = 1;
							break;

						default:
							break;
					}

					if (adb_pollkbd(0))
					{
						m_adb_srqflag = true;
					}
				}
				else if (addr == m_adb_keybaddr)
				{
					int kbd_has_data = 1;
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
								kbd_has_data = this->adb_pollkbd(1);
							}

/*                            if (m_adb_currentkeys[0] != 0xff)
                            {
                                printf("Keys[0] = %02x\n", m_adb_currentkeys[0]);
                            }
                            if (m_adb_currentkeys[1] != 0xff)
                            {
                                printf("Keys[1] = %02x\n", m_adb_currentkeys[1]);
                            }*/

							if(kbd_has_data)
							{
								if(m_adb_keybuf_start == m_adb_keybuf_end)
								{
	//                              printf("%s: buffer empty\n", __func__);
									m_adb_buffer[0] = 0xff;
									m_adb_buffer[1] = 0xff;
								}
								else
								{
									m_adb_buffer[1] = m_adb_keybuf[m_adb_keybuf_start];
									m_adb_keybuf_start = (m_adb_keybuf_start+1) % kADBKeyBufSize;
									if(m_adb_keybuf_start != m_adb_keybuf_end)
									{
										m_adb_buffer[0] = m_adb_keybuf[m_adb_keybuf_start];
										m_adb_keybuf_start = (m_adb_keybuf_start+1) % kADBKeyBufSize;
									}
									else
									{
										m_adb_buffer[0] = 0xff;
									}
								}
								m_adb_datasize = 2;
							}
							break;

						// read modifier keys
						case 2:
							{
								this->adb_pollkbd(1);
								m_adb_buffer[0] = m_adb_modifiers;
								m_adb_buffer[1] = 0xff;
								m_adb_datasize = 2;
							}
							break;

						// get ID/handler
						case 3:
							m_adb_buffer[0] = 0x60 | (m_adb_keybaddr&0xf);  // SRQ enable, no exceptional event
							m_adb_buffer[1] = 0x01; // handler 1
							m_adb_datasize = 2;

							m_adb_keybinitialized = 1;
							break;

						default:
							break;
					}

					if (adb_pollmouse())
					{
						m_adb_srqflag = true;
					}
				}
				else
				{
					#if LOG_ADB || LOG_ADB_TALK_LISTEN
					printf("ADB: talking to unconnected device %d (K %d M %d)\n", addr, m_adb_keybaddr, m_adb_mouseaddr);
					#endif
					m_adb_buffer[0] = m_adb_buffer[1] = 0;
					m_adb_datasize = 0;

					if ((adb_pollkbd(0)) || (adb_pollmouse()))
					{
						m_adb_srqflag = true;
					}
				}
				break;
		}

		m_adb_waiting_cmd = 0;
	}
	else
	{
		#if LOG_ADB || LOG_ADB_TALK_LISTEN
		printf("Got LISTEN data %02x %02x for device %x reg %x\n", m_adb_command, m_adb_buffer[1], m_adb_listenaddr, m_adb_listenreg);
		#endif

		m_adb_direction = 0;

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

	if ((ADB_IS_EGRET_NONCLASS) || (ADB_IS_CUDA_NONCLASS))
	{
		switch (mac->m_adb_linestate)
		{
			case LST_SRQNODATA:
				mac->set_adb_line(ASSERT_LINE);
				mac->m_adb_linestate = LST_IDLE;
				break;

			case LST_TSTOPSTART:
				mac->set_adb_line(ASSERT_LINE);
				mac->m_adb_timer->adjust(attotime::from_ticks(57, 1000000));
				mac->m_adb_linestate++;
				break;

			case LST_TSTOPSTARTa:
				mac->set_adb_line(CLEAR_LINE);
				mac->m_adb_timer->adjust(attotime::from_ticks(57, 1000000));
				mac->m_adb_linestate++;
				break;

			case LST_STARTBIT:
				mac->set_adb_line(ASSERT_LINE);
				mac->m_adb_timer->adjust(attotime::from_ticks(105, 1000000));
				mac->m_adb_linestate++;
				break;

			case LST_SENDBIT0:
			case LST_SENDBIT1:
			case LST_SENDBIT2:
			case LST_SENDBIT3:
			case LST_SENDBIT4:
			case LST_SENDBIT5:
			case LST_SENDBIT6:
			case LST_SENDBIT7:
				mac->set_adb_line(CLEAR_LINE);
				if (mac->m_adb_buffer[mac->m_adb_stream_ptr] & 0x80)
				{
//                    printf("1 ");
					mac->m_adb_timer->adjust(attotime::from_ticks(57, 1000000));
				}
				else
				{
//                    printf("0 ");
					mac->m_adb_timer->adjust(attotime::from_ticks(105, 1000000));
				}
				mac->m_adb_linestate++;
				break;

			case LST_SENDBIT0a:
			case LST_SENDBIT1a:
			case LST_SENDBIT2a:
			case LST_SENDBIT3a:
			case LST_SENDBIT4a:
			case LST_SENDBIT5a:
			case LST_SENDBIT6a:
				mac->set_adb_line(ASSERT_LINE);
				if (mac->m_adb_buffer[mac->m_adb_stream_ptr] & 0x80)
				{
					mac->m_adb_timer->adjust(attotime::from_ticks(105, 1000000));
				}
				else
				{
					mac->m_adb_timer->adjust(attotime::from_ticks(57, 1000000));
				}
				mac->m_adb_buffer[mac->m_adb_stream_ptr] <<= 1;
				mac->m_adb_linestate++;
				break;

			case LST_SENDBIT7a:
				mac->set_adb_line(ASSERT_LINE);
				if (mac->m_adb_buffer[mac->m_adb_stream_ptr] & 0x80)
				{
//                    printf("  ");
					mac->m_adb_timer->adjust(attotime::from_ticks(105, 1000000));
				}
				else
				{
//                    printf("  ");
					mac->m_adb_timer->adjust(attotime::from_ticks(57, 1000000));
				}

				mac->m_adb_stream_ptr++;
				if (mac->m_adb_stream_ptr == mac->m_adb_datasize)
				{
					mac->m_adb_linestate++;
				}
				else
				{
					mac->m_adb_linestate = LST_SENDBIT0;
				}
				break;

			case LST_SENDSTOP:
				mac->set_adb_line(CLEAR_LINE);
				mac->m_adb_timer->adjust(attotime::from_ticks((57*2), 1000000));
				mac->m_adb_linestate++;
				break;

			case LST_SENDSTOPa:
				mac->set_adb_line(ASSERT_LINE);
				mac->m_adb_timer->adjust(attotime::never);
				mac->m_adb_linestate = LST_IDLE;
				break;
		}
	}
	else
	{
		// do one clock transition on CB1 to advance the VIA shifter
		mac->m_adb_extclock ^= 1;
		mac->m_via1->write_cb1(mac->m_adb_extclock);

		if (mac->m_adb_direction)
		{
			mac->m_adb_command <<= 1;
		}
		else
		{
			mac->m_via1->write_cb2((mac->m_adb_send & 0x80)>>7);
			mac->m_adb_send <<= 1;
		}

		mac->m_adb_extclock ^= 1;
		mac->m_via1->write_cb1(mac->m_adb_extclock);

		mac->m_adb_timer_ticks--;
		if (!mac->m_adb_timer_ticks)
		{
			mac->m_adb_timer->adjust(attotime::never);

			if ((mac->m_adb_direction) && (ADB_IS_BITBANG))
			{
				mac->adb_talk();
				if((mac->m_adb_last_talk == 2) && mac->m_adb_datasize) {
					mac->m_adb_timer_ticks = 8;
					mac->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(100)));
				}
			}
		}
		else
		{
			mac->m_adb_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
		}
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
				m_adb_direction = 1;    // Mac is shifting us a command
				m_adb_waiting_cmd = 1;  // we're going to get a command
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
		mac->m_pm_ack &= ~2;    // lower ACK to handshake next step
	}
}

void mac_state::pmu_one_byte_reply(UINT8 result)
{
	m_pm_out[0] = m_pm_out[1] = 1;  // length
	m_pm_out[2] = result;
	m_pm_slen = 3;
	m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
}

void mac_state::pmu_three_byte_reply(UINT8 result1, UINT8 result2, UINT8 result3)
{
	m_pm_out[0] = m_pm_out[1] = 3;  // length
	m_pm_out[2] = result1;
	m_pm_out[3] = result2;
	m_pm_out[4] = result3;
	m_pm_slen = 5;
	m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
}

void mac_state::pmu_exec()
{
	m_pm_sptr = 0;  // clear send pointer
	m_pm_slen = 0;  // and send length
	m_pm_dptr = 0;  // and receive pointer

//  printf("PMU: Command %02x\n", mac->m_pm_cmd[0]);
	switch (m_pm_cmd[0])
	{
		case 0x10:  // subsystem power and clock ctrl
			break;

		case 0x20:  // send ADB command (PMU must issue an IRQ on completion)
			#if 0
			printf("PMU: Send ADB %02x %02x cmd %02x flag %02x data %02x %02x\n",
					m_pm_cmd[0],    // 0x20
					m_pm_cmd[1],    // ???
					m_pm_cmd[2],    // adb flags (2 for autopoll active, 3 to reset bus?)
					m_pm_cmd[3],    // length of ADB data
					m_pm_cmd[4],    // adb data
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

			if ((m_pm_cmd[2] & 0xf) == 0xb) // LISTEN register 3 (remap)
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

		case 0x21:  // turn ADB auto-poll off (does this need a reply?)
			break;

		case 0x28:  // read ADB
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
				m_pm_out[4] = 1;    // length of following data
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
					m_adb_pram[i] = m_pm_cmd[1+i];
				}
			}
			break;

		case 0x32:  // write extended PRAM byte(s).  cmd[2] = address, cmd[3] = length, cmd[4...] = data
			if ((m_pm_cmd[2] + m_pm_cmd[3]) < 0x100)
			{
				int i;

				for (i = 0; i < m_pm_cmd[3]; i++)
				{
					m_adb_pram[m_pm_cmd[2] + i] = m_pm_cmd[4+i];
				}
			}
			break;

		case 0x38:  // read time
			{
				m_pm_out[0] = m_pm_out[1] = 4;
				m_pm_out[2] = 0x63; // famous Mac RTC value of 8/27/56 8:35:00 PM
				m_pm_out[3] = 0x0b;
				m_pm_out[4] = 0xd1;
				m_pm_out[5] = 0x78;
				m_pm_slen = 6;
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
			}
			break;

		case 0x39:  // read first 20 bytes of PRAM
			{
				int i;

				m_pm_out[0] = m_pm_out[1] = 20;
				for (i = 0; i < 20; i++)
				{
					m_pm_out[2 + i] = m_adb_pram[i];
				}
				m_pm_slen = 22;
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
			}
			break;

		case 0x3a:  // read extended PRAM byte(s).  cmd[2] = address, cmd[3] = length
			if ((m_pm_cmd[2] + m_pm_cmd[3]) < 0x100)
			{
				int i;

				m_pm_out[0] = m_pm_out[1] = m_pm_cmd[3];
				for (i = 0; i < m_pm_cmd[3]; i++)
				{
					m_pm_out[2 + i] = m_adb_pram[m_pm_cmd[2] + i];
				}
				m_pm_slen = m_pm_out[0] + 2;
				m_pmu_send_timer->adjust(attotime(0, ATTOSECONDS_IN_USEC(200)));
			}
			break;

		case 0x40:  // set screen contrast
			break;

		case 0x41:
			break;

		case 0x58:  // read internal modem status
			pmu_one_byte_reply(0);
			break;

		case 0x60:  // set low power warning and cutoff battery levels
			break;

		case 0x68:  // read battery/charger level
			pmu_three_byte_reply(255, 255, 255);
			break;

		case 0x69:  // read battery/charger instantaneous level and status
			pmu_three_byte_reply(255, 255, 255);
			break;

		case 0x6b:  // read extended battery/charger level and status (wants an 8 byte reply)
			m_pm_out[0] = m_pm_out[1] = 8;  // length
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

		case 0x6c:  // read battery ID
			pmu_one_byte_reply(1);
			break;

		case 0x78:  // read interrupt flag
			if (ADB_IS_PM_VIA2_CLASS)   // PB 140/170 use a "leaner" PMU protocol where you get the data for a PMU interrupt here
			{
				#if 0
				if ((m_pmu_int_status&0xf0) == 0x10)
				{
					if (m_adb_datasize > 0)
					{
						m_adb_datasize = 1; // hack
						m_pm_out[0] = m_pm_out[1] = 2 + m_adb_datasize;
						m_pm_out[2] = m_pmu_int_status; // ADB status in low nibble
						m_pm_out[3] = m_pmu_last_adb_command;         // ADB command that was sent
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
						m_pm_out[3] = m_pmu_last_adb_command;         // ADB command that was sent OR 0x80 for extra error-ness
						m_pm_out[4] = 0;                              // return data
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
					m_pm_out[3] = m_pmu_last_adb_command;         // ADB command that was sent OR 0x80 for extra error-ness
					m_pm_out[4] = 0;                              // return data
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

		case 0x98:  // read sound power state
			pmu_one_byte_reply(1);
			break;

		case 0xd8:  // read A/D converter (not sure what this does)
			pmu_one_byte_reply(0);
			break;

		case 0xe0:  // write PMU internal RAM
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

		case 0xec:  // PMU self-test (send 1 count byte + reply)
			pmu_one_byte_reply(0);
			break;

		default:
			fatalerror("PMU: Unhandled command %02x\n", m_pm_cmd[0]);
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
				m_adb_timer_ticks = 1;  // one tick should be sufficient to make it see  the IRQ
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
				m_adb_timer_ticks = 1;  // one tick should be sufficient to make it see  the IRQ
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
	m_adb_irq_pending = 0;      // no interrupt
	m_adb_timer_ticks = 0;
	m_adb_command = 0;
	m_adb_extclock = 0;
	m_adb_send = 0;
	m_adb_waiting_cmd = 0;
	m_adb_streaming = MCU_STREAMING_NONE;
	m_adb_state = 0;
	m_adb_srqflag = false;
	m_pmu_poll = 0;
	if (ADB_IS_BITBANG_CLASS)
	{
		m_adb_state = ADB_STATE_NOTINIT;
	}
	m_adb_direction = 0;
	m_adb_datasize = 0;
	m_adb_last_talk = -1;

	m_adb_linestate = 0;

	// mouse
	m_adb_mouseaddr = 3;
	m_adb_lastmousex = m_adb_lastmousey = m_adb_lastbutton = 0;
	m_adb_mouse_initialized = 0;

	// keyboard
	m_adb_keybaddr = 2;
	m_adb_keybinitialized = 0;
	m_adb_currentkeys[0] = m_adb_currentkeys[1] = 0xff;
	m_adb_modifiers = 0xff;
	for (i=0; i<7; i++)
	{
		m_key_matrix[i] = 0;
	}
	m_adb_keybuf_start = 0;
	m_adb_keybuf_end = 0;
}

WRITE_LINE_MEMBER(mac_state::adb_linechange_w)
{
	int dtime = 0;
/*    static const char *states[] =
    {
        "idle",
        "attention",
        "bit0",
        "bit1",
        "bit2",
        "bit3",
        "bit4",
        "bit5",
        "bit6",
        "bit7",
        "tstop",
        "waitt1t",
        "rcvstartbit",
        "srqnodata"
    };*/

	if (ADB_IS_EGRET)
	{
		dtime = m_egret->get_adb_dtime();
	}
	else if (ADB_IS_CUDA)
	{
		dtime = m_cuda->get_adb_dtime();
	}

/*    if (m_adb_linestate <= 12)
    {
        printf("linechange: %d -> %d, time %d (state %d = %s)\n", state^1, state, dtime, m_adb_linestate, states[m_adb_linestate]);
    }
    else
    {
        printf("linechange: %d -> %d, time %d (state %d)\n", state^1, state, dtime, m_adb_linestate);
    }*/

	if ((m_adb_direction) && (m_adb_linestate == LST_TSTOP))
	{
		if (m_adb_stream_ptr & 1)   // odd byte, can't end here
		{
//            printf("critical linechange: odd, cont\n");
			m_adb_linestate = LST_BIT0;
		}
		else
		{
			if (dtime < 90)
			{
//                printf("critical linechange: even, and it's another bit\n");
				m_adb_linestate = LST_BIT0;
			}
		}
	}

	switch (m_adb_linestate)
	{
		case LST_IDLE:
			if ((state) && (dtime >= 4500))     // reset
			{
//                printf("ADB RESET\n");
			}
			else if ((state) && (dtime >= 1200))    // attention
			{
//                printf("ADB ATTENTION\n");
				m_adb_waiting_cmd = 1;
				m_adb_direction = 0;
				m_adb_linestate++;
			}
			break;

		case LST_ATTENTION:
			if ((!state) && (dtime >= 90))     // Tsync
			{
//                printf("ADB Tsync\n");
				m_adb_command = 0;
				m_adb_linestate++;
			}
			break;

		case LST_BIT0:
		case LST_BIT1:
		case LST_BIT2:
		case LST_BIT3:
		case LST_BIT4:
		case LST_BIT5:
		case LST_BIT6:
		case LST_BIT7:
			if (!state)
			{
				if (dtime >= 90)    // "1" bit
				{
					m_adb_command |= 1;
				}
//                printf("ADB bit %d\n", m_adb_command & 1);

				if (m_adb_linestate != LST_BIT7)
				{
					m_adb_command <<= 1;
				}
				else
				{
					if (m_adb_direction)
					{
//                        printf("listen byte[%d] = %02x\n", m_adb_stream_ptr, m_adb_command);
						m_adb_buffer[m_adb_stream_ptr++] = m_adb_command;
						m_adb_command = 0;
					}
				}

				m_adb_linestate++;
			}
			break;

		case LST_TSTOP:
			if (state)
			{
//                printf("ADB TSTOP, command byte %02x\n", m_adb_command);

				if (m_adb_direction)
				{
					m_adb_command = m_adb_buffer[0];
				}

				m_adb_srqflag = false;
				adb_talk();

				if (!m_adb_srqflag)
				{
					set_adb_line(ASSERT_LINE);
				}
				else
				{
					set_adb_line(CLEAR_LINE);
				}

				if (m_adb_datasize > 0)
				{
/*                    printf("Device has %d bytes of data: ", m_adb_datasize);
                    for (int i = 0; i < m_adb_datasize; i++)
                    {
                        printf("%02x ", m_adb_buffer[i]);
                    }*/
					m_adb_linestate = LST_TSTOPSTART;   // T1t
					m_adb_timer->adjust(attotime::from_ticks(324/4, 1000000));
					m_adb_stream_ptr = 0;
				}
				else if (m_adb_direction)   // if direction is set, we LISTENed to a valid device
				{
					m_adb_linestate = LST_WAITT1T;
				}
				else    // no valid device targetted, time out
				{
					if (m_adb_srqflag)
					{
						m_adb_linestate = LST_SRQNODATA;
						m_adb_timer->adjust(attotime::from_ticks(486, 1000000));   // SRQ time
					}
					else
					{
						m_adb_linestate = LST_IDLE;
					}
				}
			}
			break;

		case LST_WAITT1T:
			if ((!state) && (dtime >= 300))     // T1t
			{
//                printf("ADB T1t\n");
				m_adb_linestate++;
			}
			break;

		case LST_RCVSTARTBIT:
			if ((!state) && (dtime >= 90))       // start
			{
//                printf("ADB start\n");
				m_adb_linestate = LST_BIT0;
				m_adb_command = 0;
			}
			break;
	}
}

void mac_state::set_adb_line(int linestate)
{
	if (ADB_IS_EGRET)
	{
		m_egret->set_adb_line(linestate);
	}
	else if (ADB_IS_CUDA)
	{
		m_cuda->set_adb_line(linestate);
	}
}
