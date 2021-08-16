// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
Sega System 32 Comm PCB 837-9409
( http://images.arianchen.de/sega-comm/f1superlap_comm.jpg )
|--------------------------------------------------------------------------------|
|                 |-----------------|   |---------------------------------|      |
|                 |-----------------|   |---------------------------------|      |
|                         CNJ        PAL                CNH                      |
|                                                       MB89374                  |
|                                    Z80                                         |
|                                                       MB89237A                 |
|    PAL                           15612.17                                      |
|                                                                                |
|    MB8464                          MB8464                                      |
|                CN?                                                             |
| |---------------------------------|                                            |
| |---------------------------------|  LED     CN? CN?                 CNK       |
|--------------------------------------------------------------------------------|
    Setup:
        Z80      - Zilog Z0840004PSC Z80 CPU, running at 4.000MHz (DIP40)
        MB89237A - Fujitsu MB89237A DMA-Controller (DIP20) [most likely i8237A clone]
        MB89374  - Fujitsu MB89374 Data Link Controller (SDIP42)
        MB8464   - Fujitsu MB8464 8k x8 SRAM (DIP28)
        MB8421   - Fujitsu MB8421-12LP 2k x8 SRAM (SDIP52)

    Board:
        837-9409    F1 Super Lap

    EEPROM:
        14084.17    Rad Rally
        15612.17    F1 Super Lap

Sega System Multi32 Comm PCB 837-8792-91
( http://images.arianchen.de/sega-comm/orunners-front.jpg )
|--------------------------------------------------------------------------------|
| |---------------------------------|   |---------------------------------|      |
| |---------------------------------|   |---------------------------------|      |
|                CN3                                    CN4                      |
|                                                                                |
|    Z80                   MB89374                MB89237A            MB8421     |
|                                                                                |
|    15033.17             315-5610                                               |
|                                                                                |
|    MB8464A                                                        315-5506     |
|                CN1                                         CN2                 |
| |---------------------------------|        |---------------------------------| |
| |---------------------------------|        |---------------------------------| |
|                                  CN8 CN9                                       |
|--------------------------------------------------------------------------------|
    Setup:
        15033.17 - INTEL D27C100 128k x8 EPROM (DIP32, labelled 'EPR-15033')
        Z80      - Zilog Z0840004PSC Z80 CPU, running at 4.000MHz (DIP40)
        MB89237A - Fujitsu MB89237A DMA-Controller (DIP20) [most likely i8237A clone]
        MB89374  - Fujitsu MB89374 Data Link Controller (SDIP42)
        MB8421   - Fujitsu MB8421-12LP 2k x8 SRAM (SDIP52)
        MB8464A  - Fujitsu MB8464-10LL 8k x8 SRAM (DIP28)
        315-5611 - Lattice GAL16V8A PAL (DIP20)
        315-5506 - Lattice GAL16V8A PAL (DIP20)

    Board:
        837-8792  OutRunners, Stadium Cross

    EEPROM:
        15033.17  OutRunners, Stadium Cross
*/

#include "emu.h"
#include "emuopts.h"
#include "machine/s32comm.h"

#define VERBOSE 0
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(S32COMM, s32comm_device, "s32comm", "System 32 Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void s32comm_device::device_add_mconfig(machine_config &config)
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s32comm_device - constructor
//-------------------------------------------------

s32comm_device::s32comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, S32COMM, tag, owner, clock)
{
	// prepare localhost "filename"
	m_localhost[0] = 0;
	strcat(m_localhost, "socket.");
	strcat(m_localhost, mconfig.options().comm_localhost());
	strcat(m_localhost, ":");
	strcat(m_localhost, mconfig.options().comm_localport());

	// prepare remotehost "filename"
	m_remotehost[0] = 0;
	strcat(m_remotehost, "socket.");
	strcat(m_remotehost, mconfig.options().comm_remotehost());
	strcat(m_remotehost, ":");
	strcat(m_remotehost, mconfig.options().comm_remoteport());

	m_framesync = mconfig.options().comm_framesync() ? 0x01 : 0x00;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s32comm_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s32comm_device::device_reset()
{
	m_zfg = 0;
	m_cn = 0;
	m_fg = 0;

	std::fill(std::begin(m_shared), std::end(m_shared), 0);
}

uint8_t s32comm_device::zfg_r(offs_t offset)
{
	uint8_t result = m_zfg | 0xFE;
	LOG("s32comm-zfg_r: read register %02x for value %02x\n", offset, result);
	return result;
}

void s32comm_device::zfg_w(uint8_t data)
{
	LOG("s32comm-zfg_w: %02x\n", data);
	m_zfg = data & 0x01;
}

uint8_t s32comm_device::share_r(offs_t offset)
{
	uint8_t result = m_shared[offset];
	LOG("s32comm-share_r: read shared memory %02x for value %02x\n", offset, result);
	return result;
}

void s32comm_device::share_w(offs_t offset, uint8_t data)
{
	LOG("s32comm-share_w: %02x %02x\n", offset, data);
	m_shared[offset] = data;
}

uint8_t s32comm_device::cn_r()
{
	return m_cn | 0xFE;
}

void s32comm_device::cn_w(uint8_t data)
{
	m_cn = data & 0x01;

#ifndef S32COMM_SIMULATION
	if (!m_cn)
		device_reset();
#else
	if (!m_cn)
	{
		// reset command
		osd_printf_verbose("S32COMM: board disabled\n");
		m_linkenable = 0x00;
	}
	else
	{
		// init command
		osd_printf_verbose("S32COMM: board enabled\n");
		m_linkenable = 0x01;
		m_linkid = 0x00;
		m_linkalive = 0x00;
		m_linkcount = 0x00;
		m_linktimer = 0x00e8; // 58 fps * 4s

		comm_tick();
	}
#endif
}

uint8_t s32comm_device::fg_r()
{
	return m_fg | (~m_zfg << 7) | 0x7E;
}

void s32comm_device::fg_w(uint8_t data)
{
	if (!m_cn)
		return;

	m_fg = data & 0x01;
}

void s32comm_device::check_vint_irq()
{
#ifndef S32COMM_SIMULATION
#else
	comm_tick();
#endif
}

#ifdef S32COMM_SIMULATION
void s32comm_device::set_linktype(uint16_t linktype)
{
	m_linktype = linktype;

	switch (m_linktype)
	{
		case 14084:
			// Rad Rally
			osd_printf_verbose("S32COMM: set mode 'EPR-14084 - Rad Rally'\n");
			break;
		case 15033:
			// Stadium Cross / OutRunners
			osd_printf_verbose("S32COMM: set mode 'EPR-15033 - Stadium Cross / OutRunners'\n");
			break;
		case 15612:
			// F1 Super Lap
			osd_printf_verbose("S32COMM: set mode 'EPR-15612 - F1 Super Lap'\n");
			break;
	}
}

void s32comm_device::comm_tick()
{
	switch (m_linktype)
	{
		case 14084:
			// Rad Rally
			comm_tick_14084();
			break;
		case 15033:
			// Stadium Cross / OutRunners
			comm_tick_15033();
			break;
		case 15612:
			// F1 Super Lap
			comm_tick_15612();
			break;
	}
}

int s32comm_device::read_frame(int dataSize)
{
	if (!m_line_rx)
		return 0;

	// try to read a message
	std::uint32_t recv = 0;
	std::error_condition filerr = m_line_rx->read(m_buffer0, 0, dataSize, recv);
	if (recv > 0)
	{
		// check if message complete
		if (recv != dataSize)
		{
			// only part of a message - read on
			std::uint32_t togo = dataSize - recv;
			int offset = recv;
			while (togo > 0)
			{
				filerr = m_line_rx->read(m_buffer1, 0, togo, recv);
				if (recv > 0)
				{
					for (int i = 0 ; i < recv ; i++)
					{
						m_buffer0[offset + i] = m_buffer1[i];
					}
					togo -= recv;
					offset += recv;
				}
				else if (!filerr && recv == 0)
				{
					togo = 0;
				}
			}
		}
	}
	else if (!filerr && recv == 0)
	{
		if (m_linkalive == 0x01)
		{
			osd_printf_verbose("S32COMM: rx connection lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
			m_line_rx.reset();
		}
	}
	return recv;
}

void s32comm_device::send_data(uint8_t frameType, int frameStart, int frameSize, int dataSize)
{
	m_buffer0[0] = frameType;
	for (int i = 0x00 ; i < frameSize ; i++)
	{
		m_buffer0[1 + i] = m_shared[frameStart + i];
	}
	// forward message to next node
	send_frame(dataSize);
}

void s32comm_device::send_frame(int dataSize){
	if (!m_line_tx)
		return;

	std::error_condition filerr;
	std::uint32_t written;

	filerr = m_line_tx->write(&m_buffer0, 0, dataSize, written);
	if (filerr)
	{
		if (m_linkalive == 0x01)
		{
			osd_printf_verbose("S32COMM: tx connection lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
			m_line_tx.reset();
		}
	}
}
void s32comm_device::comm_tick_14084()
{
	// m_shared[0] = node count
	// m_shared[1] = node id
	// m_shared[2] = node mode (0 = slave, 1 = master, ff = relay)
	// m_shared[3] = node ready-to-send
	// m_shared[4] = node link status (0 = offline, 1 = online)
	if (m_linkenable == 0x01)
	{
		int frameStart = 0x0480;
		int frameOffset = 0x0000;
		int frameSize = 0x0080;
		int dataSize = frameSize + 1;
		int recv = 0;
		int idx = 0;

		bool isMaster = (m_shared[2] == 0x01);
		bool isSlave = (m_shared[2] == 0x00);
		bool isRelay = (m_shared[2] == 0xFF);

		if (m_linkalive == 0x02)
		{
			// link failed...
			m_shared[4] = 0xff;
			return;
		}
		else if (m_linkalive == 0x00)
		{
			// link not yet established...
			m_shared[4] = 0x00;

			// check rx socket
			if (!m_line_rx)
			{
				osd_printf_verbose("S32COMM: listen on %s\n", m_localhost);
				uint64_t filesize; // unused
				osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
			}

			// check tx socket
			if (!m_line_tx)
			{
				osd_printf_verbose("S32COMM: connect to %s\n", m_remotehost);
				uint64_t filesize; // unused
				osd_file::open(m_remotehost, 0, m_line_tx, filesize);
			}

			// if both sockets are there check ring
			if (m_line_rx && m_line_tx)
			{
				// try to read one message
				recv = read_frame(dataSize);
				while (recv > 0)
				{
					// check if message id
					idx = m_buffer0[0];

					// 0xFF - link id
					if (idx == 0xff)
					{
						if (isMaster)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer0[1];
							m_linktimer = 0x00;
						}
						else if (isSlave || isRelay)
						{
							// slave gets own id
							if (isSlave)
							{
								m_buffer0[1]++;
								m_linkid = m_buffer0[1];
							}

							// slave and relay forward message
							send_frame(dataSize);
						}
					}

					// 0xFE - link size
					else if (idx == 0xfe)
					{
						if (isSlave || isRelay)
						{
							m_linkcount = m_buffer0[1];

							// slave and relay forward message
							send_frame(dataSize);
						}

						// consider it done
						osd_printf_verbose("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[4] = 0x01;
						m_shared[1] = m_linkid;
						m_shared[0] = m_linkcount;
					}

					if (m_linkalive == 0x00)
						recv = read_frame(dataSize);
					else
						recv = 0;
				}

				// if we are master and link is not yet established
				if (isMaster && (m_linkalive == 0x00))
				{
					// send first packet
					if (m_linktimer == 0x01)
					{
						m_buffer0[0] = 0xff;
						m_buffer0[1] = 0x01;
						send_frame(dataSize);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer0[0] = 0xfe;
						m_buffer0[1] = m_linkcount;
						send_frame(dataSize);

						// consider it done
						osd_printf_verbose("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[4] = 0x01;
						m_shared[1] = m_linkid;
						m_shared[0] = m_linkcount;
					}

					else if (m_linktimer > 0x01)
					{
						// decrease delay timer
						m_linktimer--;
					}
				}
			}
		}

		// update "ring buffer" if link established
		if (m_linkalive == 0x01)
		{
			do
			{
				// try to read one message
				recv = read_frame(dataSize);
				while (recv > 0)
				{
					// check if valid id
					idx = m_buffer0[0];
					if (idx > 0 && idx <= m_linkcount)
					{
						// if not own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							frameOffset = idx * frameSize;
							for (int j = 0x00 ; j < frameSize ; j++)
							{
								m_shared[frameOffset + j] = m_buffer0[1 + j];
							}

							// forward message to other nodes
							send_frame(dataSize);
						}
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xFC - VSYNC
							m_linktimer = 0x00;
							if (!isMaster)
								// forward message to other nodes
								send_frame(dataSize);
						}
						if (idx == 0xfd)
						{
							// 0xFD - master addional bytes
							if (!isMaster)
							{
								// save message to "ring buffer"
								frameOffset = 0x05;
								for (int j = 0x00 ; j < 0x0b ; j++)
								{
									m_shared[frameOffset + j] = m_buffer0[1 + j];
								}

								// forward message to other nodes
								send_frame(dataSize);
							}
						}
					}

					// try to read another message
					recv = read_frame(dataSize);
				}
			}
			while (m_linktimer == 0x01);

			// enable wait for vsync
			m_linktimer = m_framesync;

			// update "ring buffer" if link established
			// live relay does not send data
			if (m_linkid != 0x00)
			{
				// check ready-to-send flag
				if (m_shared[3] != 0x00)
				{
					send_data(m_linkid, frameStart, frameSize, dataSize);

					// save message to "ring buffer"
					frameOffset = m_linkid * frameSize;
					for (int j = 0x00 ; j < frameSize ; j++)
					{
						m_shared[frameOffset + j] = m_buffer0[1 + j];
					}
				}

				if (isMaster)
				{
					// master sends some additional status bytes
					send_data(0xfd, 0x05, 0x0b, dataSize);

					// send vsync
					m_buffer0[0] = 0xfc;
					m_buffer0[1] = 0x01;
					send_frame(dataSize);
				}
			}

			// clear 03
			m_shared[3] = 0x00;
		}
	}
}

void s32comm_device::comm_tick_15033()
{
	// m_shared[0] = node count
	// m_shared[1] = node id
	// m_shared[2] = node mode (0 = slave, 1 = master, ff = relay)
	// m_shared[3] = node ready-to-send
	// m_shared[4] = node link status (0 = offline, 1 = online)
	if (m_linkenable == 0x01)
	{
		int frameStartTX = 0x0710;
		int frameStartRX = 0x0010;
		int frameOffset = 0x0000;
		int frameSize = 0x00E0;
		int dataSize = frameSize + 1;
		int recv = 0;
		int idx = 0;

		bool isMaster = (m_shared[2] == 0x01);
		bool isSlave = (m_shared[2] == 0x00);
		bool isRelay = (m_shared[2] == 0x02);

		if (m_linkalive == 0x02)
		{
			// link failed...
			m_shared[0] = 0xff;
			return;
		}
		else if (m_linkalive == 0x00)
		{
			// link not yet established...
			if (m_shared[0] == 0x56 && m_shared[1] == 0x37 && m_shared[2] == 0x30)
			{
				for (int j = 0x003 ; j < 0x0800 ; j++)
				{
					m_shared[j] = 0;
				}
				m_shared[0x08] = 0x5A;
				m_shared[0x09] = 0x38;
				m_shared[0x0A] = 0x30;
			}

			// waiting...
			m_shared[4] = 0x00;

			// check rx socket
			if (!m_line_rx)
			{
				osd_printf_verbose("S32COMM: listen on %s\n", m_localhost);
				uint64_t filesize; // unused
				osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
			}

			// check tx socket
			if (!m_line_tx)
			{
				osd_printf_verbose("S32COMM: connect to %s\n", m_remotehost);
				uint64_t filesize; // unused
				osd_file::open(m_remotehost, 0, m_line_tx, filesize);
			}

			// if both sockets are there check ring
			if (m_line_rx && m_line_tx)
			{
				// try to read one messages
				recv = read_frame(dataSize);
				while (recv > 0)
				{
					// check if message id
					idx = m_buffer0[0];

					// 0xFF - link id
					if (idx == 0xff)
					{
						if (isMaster)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer0[1];
							m_linktimer = 0x00;
						}
						else if (isSlave || isRelay)
						{
							// slave gets own id
							if (isSlave)
							{
								m_buffer0[1]++;
								m_linkid = m_buffer0[1];
							}

							// slave and relay forward message
							send_frame(dataSize);
						}
					}

					// 0xFE - link size
					else if (idx == 0xfe)
					{
						if (isSlave || isRelay)
						{
							m_linkcount = m_buffer0[1];

							// slave and relay forward message
							send_frame(dataSize);
						}

						// consider it done
						osd_printf_verbose("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[4] = 0x01;
						m_shared[1] = m_linkid;
						m_shared[0] = m_linkcount;
					}

					if (m_linkalive == 0x00)
						recv = read_frame(dataSize);
					else
						recv = 0;
				}

				// if we are master and link is not yet established
				if (isMaster && (m_linkalive == 0x00))
				{
					// send first packet
					if (m_linktimer == 0x01)
					{
						m_buffer0[0] = 0xff;
						m_buffer0[1] = 0x01;
						send_frame(dataSize);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer0[0] = 0xfe;
						m_buffer0[1] = m_linkcount;
						send_frame(dataSize);

						// consider it done
						osd_printf_verbose("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[4] = 0x01;
						m_shared[1] = m_linkid;
						m_shared[0] = m_linkcount;
					}

					else if (m_linktimer > 0x01)
					{
						// decrease delay timer
						m_linktimer--;
					}
				}
			}
		}

		// if link established
		if (m_linkalive == 0x01)
		{
			do
			{
				// try to read a message
				recv = read_frame(dataSize);
				while (recv > 0)
				{
					// check if valid id
					idx = m_buffer0[0];
					if (idx > 0 && idx <= m_linkcount)
					{
						// if not own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							frameOffset = frameStartRX + ((idx - 1) * frameSize);
							for (int j = 0x00 ; j < frameSize ; j++)
							{
								m_shared[frameOffset + j] = m_buffer0[1 + j];
							}

							// forward message to other nodes
							send_frame(dataSize);
						}
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xFC - VSYNC
							m_linktimer = 0x00;
							if (!isMaster)
								// forward message to other nodes
								send_frame(dataSize);
						}
						if (idx == 0xfd)
						{
							// 0xFD - master addional bytes
							if (!isMaster)
							{
								// save message to "ring buffer"
								frameOffset = 0x05;
								for (int j = 0x00 ; j < 0x0b ; j++)
								{
									m_shared[frameOffset + j] = m_buffer0[1 + j];
								}

								// forward message to other nodes
								send_frame(dataSize);
							}
						}
					}

					// try to read another message
					recv = read_frame(dataSize);
				}
			}
			while (m_linktimer == 0x01);

			// enable wait for vsync
			m_linktimer = m_framesync;

			// update "ring buffer" if link established
			// live relay does not send data
			if (m_linkid != 0x00)
			{
				// check ready-to-send flag
				if (m_shared[3] != 0x00)
				{
					send_data(m_linkid, frameStartTX, frameSize, dataSize);

					// save message to "ring buffer"
					frameOffset = frameStartRX + ((m_linkid - 1) * frameSize);
					for (int j = 0x00 ; j < frameSize ; j++)
					{
						m_shared[frameOffset + j] = m_buffer0[1 + j];
					}
				}

				if (isMaster){
					// master sends some additional status bytes
					send_data(0xfd, 0x05, 0x0b, dataSize);

					// send vsync
					m_buffer0[0] = 0xfc;
					m_buffer0[1] = 0x01;
					send_frame(dataSize);
				}
			}

			// clear 03
			m_shared[3] = 0x00;
		}
	}
}

void s32comm_device::comm_tick_15612()
{
	// m_shared[0] = node link status (5 = linking, 1 = online)
	// m_shared[1] = node mode (0 = relay, 1 = master, 2 = slave)
	// m_shared[2] = node id
	// m_shared[3] = node count
	// m_shared[4] = ready-to-send
	if (m_linkenable == 0x01)
	{
		int frameStart = 0x0010;
		int frameOffset = 0x0000;
		int frameSize = 0x00E0;
		int dataSize = frameSize + 1;
		int recv = 0;
		int idx = 0;

		bool isMaster = (m_shared[1] == 0x01);
		bool isSlave = (m_shared[1] == 0x02);
		bool isRelay = (m_shared[1] == 0x00);

		if (m_linkalive == 0x02)
		{
			// link failed...
			m_shared[0] = 0xff;
			return;
		}
		else if (m_linkalive == 0x00)
		{
			// link not yet established...
			m_shared[0] = 0x05;

			// check rx socket
			if (!m_line_rx)
			{
				osd_printf_verbose("S32COMM: listen on %s\n", m_localhost);
				uint64_t filesize; // unused
				osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
			}

			// check tx socket
			if (!m_line_tx)
			{
				osd_printf_verbose("S32COMM: connect to %s\n", m_remotehost);
				uint64_t filesize; // unused
				osd_file::open(m_remotehost, 0, m_line_tx, filesize);
			}

			// if both sockets are there check ring
			if (m_line_rx && m_line_tx)
			{
				// try to read one message
				recv = read_frame(dataSize);
				while (recv > 0)
				{
					// check if message id
					idx = m_buffer0[0];

					// 0xFF - link id
					if (idx == 0xff)
					{
						if (isMaster)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer0[1];
							m_linktimer = 0x00;
						}
						else if (isSlave || isRelay)
						{
							// slave gets own id
							if (isSlave)
							{
								m_buffer0[1]++;
								m_linkid = m_buffer0[1];
							}

							// slave and relay forward message
							send_frame(dataSize);
						}
					}

					// 0xFE - link size
					else if (idx == 0xfe)
					{
						if (isSlave || isRelay)
						{
							m_linkcount = m_buffer0[1];

							// slave and relay forward message
							send_frame(dataSize);
						}

						// consider it done
						osd_printf_verbose("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[0] = 0x01;
						m_shared[2] = m_linkid;
						m_shared[3] = m_linkcount;
					}

					if (m_linkalive == 0x00)
						recv = read_frame(dataSize);
					else
						recv = 0;
				}

				// if we are master and link is not yet established
				if (isMaster && (m_linkalive == 0x00))
				{
					// send first packet
					if (m_linktimer == 0x01)
					{
						m_buffer0[0] = 0xFF;
						m_buffer0[1] = 0x01;
						send_frame(dataSize);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer0[0] = 0xFE;
						m_buffer0[1] = m_linkcount;
						send_frame(dataSize);

						// consider it done
						osd_printf_verbose("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[0] = 0x01;
						m_shared[2] = m_linkid;
						m_shared[3] = m_linkcount;
					}

					else if (m_linktimer > 0x01)
					{
						// decrease delay timer
						m_linktimer--;
					}
				}
			}
		}

		// update "ring buffer" if link established
		if (m_linkalive == 0x01)
		{
			do
			{
				// try to read a message
				recv = read_frame(dataSize);
				while (recv > 0)
				{
					// check if valid id
					idx = m_buffer0[0];
					if (idx > 0 && idx <= m_linkcount)
					{
						// if not own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							frameOffset = frameStart + (idx * frameSize);
							for (int j = 0x00 ; j < frameSize ; j++)
							{
								m_shared[frameOffset + j] = m_buffer0[1 + j];
							}

							// forward message to other nodes
							send_frame(dataSize);
						}
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xFC - VSYNC
							m_linktimer = 0x00;
							if (!isMaster)
								// forward message to other nodes
								send_frame(dataSize);
						}
						if (idx == 0xfd)
						{
							// 0xFD - master addional bytes
							if (!isMaster)
							{
								// save message to "ring buffer"
								frameOffset = 0x05;
								for (int j = 0x00 ; j < 0x0b ; j++)
								{
									m_shared[frameOffset + j] = m_buffer0[1 + j];
								}

								// forward message to other nodes
								send_frame(dataSize);
							}
						}
					}

					// try to read another message
					recv = read_frame(dataSize);
				}
			}
			while (m_linktimer == 0x01);

			// enable wait for vsync
			m_linktimer = m_framesync;

			// update "ring buffer" if link established
			// live relay does not send data
			if (m_linkid != 0x00)
			{
				// check ready-to-send flag
				if (m_shared[4] != 0x00)
				{
					send_data(m_linkid, frameStart, frameSize, dataSize);

					// save message to "ring buffer"
					frameOffset = frameStart + (m_linkid * frameSize);
					for (int j = 0x00 ; j < frameSize ; j++)
					{
						m_shared[frameOffset + j] = m_buffer0[1 + j];
					}
				}

				if (isMaster)
				{
					// master sends some additional status bytes
					// master sends additional status bytes
					send_data(0xfd, 0x05, 0x0b, dataSize);

					// send vsync
					m_buffer0[0] = 0xfc;
					m_buffer0[1] = 0x01;
					send_frame(dataSize);
				}
			}

			// clear 04
			m_shared[4] = 0x00;
		}
	}
}
#endif
