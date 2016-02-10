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

#include "machine/s32comm.h"

//#define __S32COMM_VERBOSE__

MACHINE_CONFIG_FRAGMENT( s32comm )
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type S32COMM = &device_creator<s32comm_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor s32comm_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( s32comm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s32comm_device - constructor
//-------------------------------------------------

s32comm_device::s32comm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, S32COMM, "SYSTEM32 COMMUNICATION BD", tag, owner, clock, "s32comm", __FILE__),
	m_line_rx(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE ),
	m_line_tx(OPEN_FLAG_READ)
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
}

READ8_MEMBER(s32comm_device::zfg_r)
{
	UINT8 result = m_zfg | 0xFE;
#ifdef __S32COMM_VERBOSE__
	printf("s32comm-zfg_r: read register %02x for value %02x\n", offset, result);
#endif
	return result;
}

WRITE8_MEMBER(s32comm_device::zfg_w)
{
#ifdef __S32COMM_VERBOSE__
	printf("s32comm-zfg_w: %02x\n", data);
#endif
	m_zfg = data & 0x01;
}

READ8_MEMBER(s32comm_device::share_r)
{
	UINT8 result = m_shared[offset];
#ifdef __S32COMM_VERBOSE__
	printf("s32comm-share_r: read shared memory %02x for value %02x\n", offset, result);
#endif
	return result;
}

WRITE8_MEMBER(s32comm_device::share_w)
{
#ifdef __S32COMM_VERBOSE__
	printf("s32comm-share_w: %02x %02x\n", offset, data);
#endif
	m_shared[offset] = data;
}

READ8_MEMBER(s32comm_device::cn_r)
{
	return m_cn | 0xFE;
}

WRITE8_MEMBER(s32comm_device::cn_w)
{
	m_cn = data & 0x01;

#ifndef __S32COMM_SIMULATION__
	if (!m_cn)
		device_reset();
#else
	if (!m_cn)
	{
		// reset command
		printf("S32COMM: board disabled\n");
		m_linkenable = 0x00;
	}
	else
	{
		// init command
		printf("S32COMM: board enabled\n");
		m_linkenable = 0x01;
		m_linkid = 0x00;
		m_linkalive = 0x00;
		m_linkcount = 0x00;
		m_linktimer = 0x04; //0x00E8; // 58 fps * 4s

		comm_tick();
	}
#endif
}

READ8_MEMBER(s32comm_device::fg_r)
{
	return m_fg | (~m_zfg << 7) | 0x7E;
}

WRITE8_MEMBER(s32comm_device::fg_w)
{
	if (!m_cn)
		return;

	m_fg = data & 0x01;
}

void s32comm_device::check_vint_irq()
{
#ifndef __S32COMM_SIMULATION__
#else
	comm_tick();
#endif
}

#ifdef __S32COMM_SIMULATION__
void s32comm_device::set_linktype(UINT16 linktype)
{
	m_linktype = linktype;

	switch (m_linktype)
	{
		case 14084:
			// Rad Rally
			printf("S32COMM: set mode 'EPR-14084 - Rad Rally'\n");
			break;
		case 15033:
			// Stadium Cross / OutRunners
			printf("S32COMM: set mode 'EPR-15033 - Stadium Cross / OutRunners'\n");
			break;
		case 15612:
			// F1 Super Lap
			printf("S32COMM: set mode 'EPR-15612 - F1 Super Lap'\n");
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

void s32comm_device::comm_tick_14084()
{
	if (m_linkenable == 0x01)
	{
		int frameStart = 0x0480;
		int frameOffset = 0x0000;
		int frameSize = 0x0080;
		int dataSize = frameSize + 1;
		int togo = 0;
		int recv = 0;
		int idx = 0;

		bool isMaster = (m_shared[2] == 0x01);
		bool isSlave = (m_shared[2] == 0x00);
		bool isRelay = (m_shared[2] == 0xFF);

		// if link not yet established...
		if (m_linkalive == 0x00)
		{
			// waiting...
			m_shared[4] = 0x00;

			// check rx socket
			if (!m_line_rx.is_open())
			{
				printf("S32COMM: listen on %s\n", m_localhost);
				m_line_rx.open(m_localhost);
			}

			// check tx socket
			if (!m_line_tx.is_open())
			{
				printf("S32COMM: connect to %s\n", m_remotehost);
				m_line_tx.open(m_remotehost);
			}

			// if both sockets are there check ring
			if ((m_line_rx.is_open()) && (m_line_tx.is_open()))
			{
				// try to read one messages
				recv = m_line_rx.read(m_buffer, dataSize);
				while (recv != 0)
				{
					// check if complete message
					if (recv == dataSize)
					{
						// check if message id
						idx = m_buffer[0];

						// 0xFF - link id
						if (idx == 0xFF)
						{
							if (isMaster)
							{
								// master gets first id and starts next state
								m_linkid = 0x01;
								m_linkcount = m_buffer[1];
								m_linktimer = 0x01;
							}
							else if (isSlave || isRelay)
							{
								// slave gets own id
								if (isSlave)
								{
									m_buffer[1]++;
									m_linkid = m_buffer[1];
								}

								// slave and relay forward message
								m_line_tx.write(m_buffer, dataSize);
							}
						}

						// 0xFE - link size
						else if (idx == 0xFE)
						{
							if (isSlave || isRelay)
							{
								m_linkcount = m_buffer[1];

								// slave and relay forward message
								m_line_tx.write(m_buffer, dataSize);
							}

							// consider it done
							printf("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
							m_linkalive = 0x01;

							// write to shared mem
							m_shared[4] = 0x01;
							m_shared[1] = m_linkid;
							m_shared[0] = m_linkcount;
						}
					}
					else
					{
						// got only part of a message - read the rest (and drop it)
						// TODO: combine parts and push to "ring buffer"
						togo = dataSize - recv;
						while (togo > 0){
							recv = m_line_rx.read(m_buffer, togo);
							togo -= recv;
						}
						printf("S32COMM: droped a message...\n");
					}

					if (m_linkalive == 0x00)
						recv = m_line_rx.read(m_buffer, dataSize);
					else
						recv = 0;
				}

				// if we are master and link is not yet established
				if (isMaster && (m_linkalive == 0x00))
				{
					// send first packet
					if (m_linktimer == 0x00)
					{
						m_buffer[0] = 0xFF;
						m_buffer[1] = 0x01;
						m_line_tx.write(m_buffer, dataSize);
					}

					// send second packet
					else if (m_linktimer == 0x01)
					{
						m_buffer[0] = 0xFE;
						m_buffer[1] = m_linkcount;
						m_line_tx.write(m_buffer, dataSize);

						// consider it done
						printf("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[4] = 0x01;
						m_shared[1] = m_linkid;
						m_shared[0] = m_linkcount;
					}

					else if (m_linktimer > 0x02)
					{
						// decrease delay timer
						m_linktimer--;
						if (m_linktimer == 0x02)
							m_linktimer = 0x00;
					}
				}
			}
		}

		// update "ring buffer" if link established
		if (m_linkalive == 0x01)
		{
			int togo = 0;
			// try to read one messages
			int recv = m_line_rx.read(m_buffer, dataSize);
			while (recv != 0)
			{
				// check if complete message
				if (recv == dataSize)
				{
					// check if valid id
					int idx = m_buffer[0];
					if (idx > 0 && idx <= m_linkcount) {
						// if not our own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							frameOffset = idx * frameSize;
							for (int j = 0x00 ; j < frameSize ; j++)
							{
								m_shared[frameOffset + j] = m_buffer[1 + j];
							}

							// forward message to other nodes
							m_line_tx.write(m_buffer, dataSize);
						}
					} else {
						if (!isMaster && idx == 0xF0){
							// 0xF0 - master addional bytes
							for (int j = 0x05 ; j < 0x10 ; j++)
							{
								m_shared[j] = m_buffer[1 + j];
							}

							// forward message to other nodes
							m_line_tx.write(m_buffer, dataSize);
						}
					}
				}
				else
				{
					// got only part of a message - read the rest (and drop it)
					// TODO: combine parts and push to "ring buffer"
					togo = dataSize - recv;
					while (togo > 0){
						recv = m_line_rx.read(m_buffer, togo);
						togo -= recv;
					}
					printf("S32COMM: droped a message...\n");
				}
				recv = m_line_rx.read(m_buffer, dataSize);
			}

			// update "ring buffer" if link established
			// live relay does not send data
			if (m_linkid != 0x00 && m_shared[3] != 0x00)
			{
				m_buffer[0] = m_linkid;
				frameOffset = m_linkid * frameSize;
				for (int j = 0x00 ; j < frameSize ; j++)
				{
					// push message to "ring buffer"
					m_shared[frameOffset + j] = m_shared[frameStart + j];
					m_buffer[1 + j] = m_shared[frameStart + j];
				}
				// push message to other nodes
				m_line_tx.write(m_buffer, dataSize);

				// master sends some additional status bytes
				if (isMaster){
					m_buffer[0] = 0xF0;
					for (int j = 0x00 ; j < frameSize ; j++)
					{
						m_buffer[1 + j] = 0x00;
					}
					for (int j = 0x05 ; j < 0x10 ; j++)
					{
						m_buffer[1 + j] = m_shared[j];
					}
					// push message to other nodes
					m_line_tx.write(m_buffer, dataSize);
				}
			}

			// clear 03
			m_shared[3] = 0x00;
		}
	}
}

void s32comm_device::comm_tick_15033()
{
	if (m_linkenable == 0x01)
	{
		int frameStartTX = 0x0710;
		int frameStartRX = 0x0010;
		int frameOffset = 0x0000;
		int frameSize = 0x00E0;
		int dataSize = frameSize + 1;
		int togo = 0;
		int recv = 0;
		int idx = 0;

		bool isMaster = (m_shared[2] == 0x01);
		bool isSlave = (m_shared[2] == 0x00);
		bool isRelay = (m_shared[2] == 0x02);

		// if link not yet established - Z80 reply check?
		if (m_linkalive == 0x00 && m_shared[0] == 0x56 && m_shared[1] == 0x37 && m_shared[2] == 0x30)
		{
			for (int j = 0x003 ; j < 0x0800 ; j++)
			{
				m_shared[j] = 0;
			}
			m_shared[0x08] = 0x5A;
			m_shared[0x09] = 0x38;
			m_shared[0x0A] = 0x30;
		}
		else if (m_linkalive == 0x00)
		{
			// waiting...
			m_shared[4] = 0x00;

			// check rx socket
			if (!m_line_rx.is_open())
			{
				printf("S32COMM: listen on %s\n", m_localhost);
				m_line_rx.open(m_localhost);
			}

			// check tx socket
			if (!m_line_tx.is_open())
			{
				printf("S32COMM: connect to %s\n", m_remotehost);
				m_line_tx.open(m_remotehost);
			}

			// if both sockets are there check ring
			if ((m_line_rx.is_open()) && (m_line_tx.is_open()))
			{
				// try to read one messages
				recv = m_line_rx.read(m_buffer, dataSize);
				while (recv != 0)
				{
					// check if complete message
					if (recv == dataSize)
					{
						// check if message id
						idx = m_buffer[0];

						// 0xFF - link id
						if (idx == 0xFF)
						{
							if (isMaster)
							{
								// master gets first id and starts next state
								m_linkid = 0x01;
								m_linkcount = m_buffer[1];
								m_linktimer = 0x01;
							}
							else if (isSlave || isRelay)
							{
								// slave gets own id
								if (isSlave)
								{
									m_buffer[1]++;
									m_linkid = m_buffer[1];
								}

								// slave and relay forward message
								m_line_tx.write(m_buffer, dataSize);
							}
						}

						// 0xFE - link size
						else if (idx == 0xFE)
						{
							if (isSlave || isRelay)
							{
								m_linkcount = m_buffer[1];

								// slave and relay forward message
								m_line_tx.write(m_buffer, dataSize);
							}

							// consider it done
							printf("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
							m_linkalive = 0x01;

							// write to shared mem
							m_shared[4] = 0x01;
							m_shared[1] = m_linkid;
							m_shared[0] = m_linkcount;
						}
					}
					else
					{
						// got only part of a message - read the rest (and drop it)
						// TODO: combine parts and push to "ring buffer"
						togo = dataSize - recv;
						while (togo > 0){
							recv = m_line_rx.read(m_buffer, togo);
							togo -= recv;
						}
						printf("S32COMM: droped a message...\n");
					}

					if (m_linkalive == 0x00)
						recv = m_line_rx.read(m_buffer, dataSize);
					else
						recv = 0;
				}

				// if we are master and link is not yet established
				if (isMaster && (m_linkalive == 0x00))
				{
					// send first packet
					if (m_linktimer == 0x00)
					{
						m_buffer[0] = 0xFF;
						m_buffer[1] = 0x01;
						m_line_tx.write(m_buffer, dataSize);
					}

					// send second packet
					else if (m_linktimer == 0x01)
					{
						m_buffer[0] = 0xFE;
						m_buffer[1] = m_linkcount;
						m_line_tx.write(m_buffer, dataSize);

						// consider it done
						printf("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[4] = 0x01;
						m_shared[1] = m_linkid;
						m_shared[0] = m_linkcount;
					}

					else if (m_linktimer > 0x02)
					{
						// decrease delay timer
						m_linktimer--;
						if (m_linktimer == 0x02)
							m_linktimer = 0x00;
					}
				}
			}
		}

		// update "ring buffer" if link established
		if (m_linkalive == 0x01)
		{
			int togo = 0;
			// try to read one messages
			int recv = m_line_rx.read(m_buffer, dataSize);
			while (recv != 0)
			{
				// check if complete message
				if (recv == dataSize)
				{
					// check if valid id
					int idx = m_buffer[0];
					if (idx > 0 && idx <= m_linkcount) {
						// if not our own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							frameOffset = frameStartRX + ((idx - 1) * frameSize);
							for (int j = 0x00 ; j < frameSize ; j++)
							{
								m_shared[frameOffset + j] = m_buffer[1 + j];
							}

							// forward message to other nodes
							m_line_tx.write(m_buffer, dataSize);
						}
					} else {
						if (!isMaster && idx == 0xF0){
							// 0xF0 - master addional bytes
							for (int j = 0x05 ; j < 0x10 ; j++)
							{
								m_shared[j] = m_buffer[1 + j];
							}

							// forward message to other nodes
							m_line_tx.write(m_buffer, dataSize);
						}
					}
				}
				else
				{
					// got only part of a message - read the rest (and drop it)
					// TODO: combine parts and push to "ring buffer"
					togo = dataSize - recv;
					while (togo > 0){
						recv = m_line_rx.read(m_buffer, togo);
						togo -= recv;
					}
					printf("S32COMM: droped a message...\n");
				}
				recv = m_line_rx.read(m_buffer, dataSize);
			}

			// update "ring buffer" if link established
			// live relay does not send data
			if (m_linkid != 0x00 && m_shared[3] != 0x00)
			{
				m_buffer[0] = m_linkid;
				frameOffset = frameStartRX + ((m_linkid - 1) * frameSize);
				for (int j = 0x00 ; j < frameSize ; j++)
				{
					// push message to "ring buffer"
					m_shared[frameOffset + j] = m_shared[frameStartTX + j];
					m_buffer[1 + j] = m_shared[frameStartTX + j];
				}
				// push message to other nodes
				m_line_tx.write(m_buffer, dataSize);

				// master sends some additional status bytes
				if (isMaster){
					m_buffer[0] = 0xF0;
					for (int j = 0x00 ; j < frameSize ; j++)
					{
						m_buffer[1 + j] = 0x00;
					}
					for (int j = 0x05 ; j < 0x10 ; j++)
					{
						m_buffer[1 + j] = m_shared[j];
					}
					// push message to other nodes
					m_line_tx.write(m_buffer, dataSize);
				}
			}
			// clear 03
			m_shared[3] = 0x00;
		}
	}
}

void s32comm_device::comm_tick_15612()
{
	if (m_linkenable == 0x01)
	{
		int frameStart = 0x0010;
		int frameOffset = 0x0000;
		int frameSize = 0x00E0;
		int dataSize = frameSize + 1;
		int togo = 0;
		int recv = 0;
		int idx = 0;

		bool isMaster = (m_shared[1] == 0x01);
		bool isSlave = (m_shared[1] == 0x02);
		bool isRelay = (m_shared[1] == 0x00);

		// if link not yet established...
		if (m_linkalive == 0x00)
		{
			// waiting...
			m_shared[0] = 0x05;

			// check rx socket
			if (!m_line_rx.is_open())
			{
				printf("S32COMM: listen on %s\n", m_localhost);
				m_line_rx.open(m_localhost);
			}

			// check tx socket
			if (!m_line_tx.is_open())
			{
				printf("S32COMM: connect to %s\n", m_remotehost);
				m_line_tx.open(m_remotehost);
			}

			// if both sockets are there check ring
			if ((m_line_rx.is_open()) && (m_line_tx.is_open()))
			{
				// try to read one messages
				recv = m_line_rx.read(m_buffer, dataSize);
				while (recv != 0)
				{
					// check if complete message
					if (recv == dataSize)
					{
						// check if message id
						idx = m_buffer[0];

						// 0xFF - link id
						if (idx == 0xFF)
						{
							if (isMaster)
							{
								// master gets first id and starts next state
								m_linkid = 0x01;
								m_linkcount = m_buffer[1];
								m_linktimer = 0x01;
							}
							else if (isSlave || isRelay)
							{
								// slave gets own id
								if (isSlave)
								{
									m_buffer[1]++;
									m_linkid = m_buffer[1];
								}

								// slave and relay forward message
								m_line_tx.write(m_buffer, dataSize);
							}
						}

						// 0xFE - link size
						else if (idx == 0xFE)
						{
							if (isSlave || isRelay)
							{
								m_linkcount = m_buffer[1];

								// slave and relay forward message
								m_line_tx.write(m_buffer, dataSize);
							}

							// consider it done
							printf("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
							m_linkalive = 0x01;

							// write to shared mem
							m_shared[0] = 0x01;
							m_shared[2] = m_linkid;
							m_shared[3] = m_linkcount;
						}
					}
					else
					{
						// got only part of a message - read the rest (and drop it)
						// TODO: combine parts and push to "ring buffer"
						togo = dataSize - recv;
						while (togo > 0){
							recv = m_line_rx.read(m_buffer, togo);
							togo -= recv;
						}
						printf("S32COMM: droped a message...\n");
					}

					if (m_linkalive == 0x00)
						recv = m_line_rx.read(m_buffer, dataSize);
					else
						recv = 0;
				}

				// if we are master and link is not yet established
				if (isMaster && (m_linkalive == 0x00))
				{
					// send first packet
					if (m_linktimer == 0x00)
					{
						m_buffer[0] = 0xFF;
						m_buffer[1] = 0x01;
						m_line_tx.write(m_buffer, dataSize);
					}

					// send second packet
					else if (m_linktimer == 0x01)
					{
						m_buffer[0] = 0xFE;
						m_buffer[1] = m_linkcount;
						m_line_tx.write(m_buffer, dataSize);

						// consider it done
						printf("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[0] = 0x01;
						m_shared[2] = m_linkid;
						m_shared[3] = m_linkcount;
					}

					else if (m_linktimer > 0x02)
					{
						// decrease delay timer
						m_linktimer--;
						if (m_linktimer == 0x02)
							m_linktimer = 0x00;
					}
				}
			}
		}

		// update "ring buffer" if link established
		if (m_linkalive == 0x01)
		{
			int togo = 0;
			// try to read one messages
			int recv = m_line_rx.read(m_buffer, dataSize);
			while (recv != 0)
			{
				// check if complete message
				if (recv == dataSize)
				{
					// check if valid id
					int idx = m_buffer[0];
					if (idx > 0 && idx <= m_linkcount) {
						// if not our own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							frameOffset = frameStart + (idx * frameSize);
							for (int j = 0x00 ; j < frameSize ; j++)
							{
								m_shared[frameOffset + j] = m_buffer[1 + j];
							}

							// forward message to other nodes
							m_line_tx.write(m_buffer, dataSize);
						}
					} else {
						if (!isMaster && idx == 0xF0){
							// 0xF0 - master addional bytes
							for (int j = 0x05 ; j < 0x10 ; j++)
							{
								m_shared[j] = m_buffer[1 + j];
							}

							// forward message to other nodes
							m_line_tx.write(m_buffer, dataSize);
						}
					}
				}
				else
				{
					// got only part of a message - read the rest (and drop it)
					// TODO: combine parts and push to "ring buffer"
					togo = dataSize - recv;
					while (togo > 0){
						recv = m_line_rx.read(m_buffer, togo);
						togo -= recv;
					}
					printf("S32COMM: droped a message...\n");
				}
				recv = m_line_rx.read(m_buffer, dataSize);
			}

			// update "ring buffer" if link established
			// live relay does not send data
			if (m_linkid != 0x00 && m_shared[4] != 0x00)
			{
				m_buffer[0] = m_linkid;
				frameOffset = frameStart + (m_linkid * frameSize);
				for (int j = 0x00 ; j < frameSize ; j++)
				{
					// push message to "ring buffer"
					m_shared[frameOffset + j] = m_shared[frameStart + j];
					m_buffer[1 + j] = m_shared[frameStart + j];
				}
				// push message to other nodes
				m_line_tx.write(m_buffer, dataSize);

				// master sends some additional status bytes
				if (isMaster){
					m_buffer[0] = 0xF0;
					for (int j = 0x00 ; j < frameSize ; j++)
					{
						m_buffer[1 + j] = 0x00;
					}
					for (int j = 0x05 ; j < 0x10 ; j++)
					{
						m_buffer[1 + j] = m_shared[j];
					}
					// push message to other nodes
					m_line_tx.write(m_buffer, dataSize);
				}
			}
			// clear 04
			m_shared[4] = 0x00;
		}
	}
}
#endif
