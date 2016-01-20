// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
Sega MODEL2 COMMUNICATION BOARD 837-10537
( http://images.arianchen.de/sega-comm/daytona_comm.jpg )
|-----------------------------------------------------------------------------|
| |-------------------|                             |-------------------|     |
| |-------------------|                             |-------------------|     |
|          CN5                                               CN4           -- |
| LED    Z80        LH5268A          16726.7        JP6 JP5 JP4 JP1        || |
|                                                                         C|| |
|        315-5751   315-5752                        uPD72103              N|| |
|                                                                     JP7 8|| |
|        LH5268A    LH5268A                                           JP8  || |
|                                                                     JP9  -- |
|        LH5268A    LH5268A                         315-5753A    315-5547     |
|                                                                             |
|                                                                         CN6 |
| CN9                                                                         |
|                                                                         CN7 |
|          CN3                      CN2                      CN1              |
| |-------------------|    |-------------------|    |-------------------|     |
| |-------------------|    |-------------------|    |-------------------|     |
|-----------------------------------------------------------------------------|

    Setup:
        Z80        Zilog Z0840008PSC Z80 CPU (DIP40)
        LH5268A    SHARP LH5268AD-10LL 8k x8 SRAM (DIP28)
        uPD72103   NEC uPD72103 HDLC/Frame Relay Controller (QFP80)
        16726.7    M27C1001 128k x8 EPROM (DIP32, labelled 'EPR-16726')
        315-5751   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5752   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5753A  AMI 18CV8PC-15 PAL (DIP20)
        315-5547   AMI 18CV8PC-25 PAL (DIP20)

    Board:
        837-10537  Daytona USA

    EEPROM:
        16726.7    Daytona USA


Sega PC BD MODEL2 A-CRX COMMUNICATION 837-11525
( http://images.arianchen.de/sega-comm/srally_comm.jpg )
|-------------------------------------------------------------------------------------------|
| |-------------------|                             |-------------------|    |---------|    |
| |-------------------|                             |-------------------|    |---------|    |
|          CN5                                               CN4                CN10        |
| LED    Z80        LH5268A          16726.7        JP6 JP5 JP4 JP1                         |
|                                                                                           |
|        315-5751   315-5752                        uPD72103                                |
|                                                                                           |
|        LH5268A    LH5268A                                                                 |
|                                                                                        -- |
|        LH5268A    LH5268A                         315-5753A    315-5547                || |
|                                                                                       C|| |
|                                                                                       N|| |
| CN9                                                                                   8|| |
|                                                                                   JP7  || |
|          CN3                      CN2                      CN1                    JP8  -- |
| |-------------------|    |-------------------|    |-------------------|           JP9     |
| |-------------------|    |-------------------|    |-------------------|                   |
|-------------------------------------------------------------------------------------------|

    Setup:
        Z80        Zilog Z0840008PSC Z80 CPU (DIP40)
        LH5268A    SHARP LH5268AD-10LL 8k x8 SRAM (DIP28)
        uPD72103   NEC uPD72103 HDLC/Frame Relay Controller (QFP80)
        16726.7    M27C1001 128k x8 EPROM (DIP32, labelled 'EPR-16726')
        315-5751   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5752   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5753A  AMI 18CV8PC-15 PAL (DIP20)
        315-5547   AMI 18CV8PC-25 PAL (DIP20)

    Board:
        837-11525
        837-11572  Sega Rally Championship

    EEPROM:
        16726.7    Sega Rally Championship
        18643A.7   ManxTT


Sega PC BD MODEL2 B-CRX COMMUNICATION 837-11615
( http://images.arianchen.de/sega-comm/model2b-com_top.jpg )
|-------------------------------------------------------------------------------------------|
|                                                                                           |
|  --                                                                                       |
|  ||          18643A.7    MB84256A        Z80                                              |
| C||                                                      MB84256A                         |
| N||  JP9                       uPD72103A 315-5751                                         |
| 8||  JP8                                                 MB84256A                         |
|  ||  JP7                                 315-5752                                         |
|  --                                                      MB84256A                         |
|                                                                                           |
|                                                          MB84256A                         |
|                                          315-5753A                                        |
| LED                                                                                       |
|                                                    315-5547A                              |
|                                                                                           |
|                                                                              CN1          |
|                                                                    |-------------------|  |
|                                                  CN3               |-------------------|  |
|-------------------------------------------------------------------------------------------|

    Setup:
        Z80          Zilog Z0840008PSC Z80 CPU (DIP40)
        MB84256A     Fujitsu MB84256A-70LL 32k x8 SRAM (SDIP28)
        uPD72103A    NEC uPD72103 HDLC/Frame Relay Controller (QFP80)
        18643A.7     M27C1001 128k x8 EPROM (DIP32, labelled 'EPR-18643A')
        315-5751     Lattice GAL16V8B-25LP PAL (DIP20)
        315-5752     Lattice GAL16V8B-25LP PAL (DIP20)
        315-5753A    ICT PEEL18CV8P-15 PAL (DIP20)
        315-5547A    AMI 18CV8PC-25 PAL (DIP20)

    Board:
        837-11615    Virtua On
        837-11615-02 Virtua On

    EEPROM:
        18643.7      Virtua On
        18643A.7     Virtua On




Sega PC BD MODEL2 C-CRX COMMUNICATION 837-12839
|-------------------------------------------------------------------------------------------|
|                                                                                           |
|  --                                                                                       |
|  ||          18643A.7    MB84256A        Z80                                              |
| C||                                                      MB84256A                         |
| N||  JP9                       uPD72103A 315-5751                                         |
| 8||  JP8                                                 MB84256A                         |
|  ||  JP7                                 315-5752                                         |
|  --                                                      MB84256A                         |
|                                                                                           |
|                                                          MB84256A                         |
|                                          315-5753A                                        |
| LED                                                                                       |
|                                                    315-5547A                              |
|                                                                                           |
|                                                                              CN1          |
|                                                                    |-------------------|  |
|                                                  CN3               |-------------------|  |
|-------------------------------------------------------------------------------------------|

    Setup:
        Z80        Zilog Z0840008PSC Z80 CPU (DIP40)
        MB84256A   Fujitsu MB84256A-70LL 32k x8 SRAM (SDIP28)
        uPD72103A  NEC uPD72103 HDLC/Frame Relay Controller (QFP80)
        18643A.7   M27C1001 128k x8 EPROM (DIP32, labelled 'EPR-18643A')
        315-5751   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5752   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5753A  ICT PEEL18CV8P-15 PAL (DIP20)
        315-5547A  AMI 18CV8PC-25 PAL (DIP20)

    Board:
        837-12839  Sega Touring Car Championship

    EEPROM:
        18643A.7   Sega Touring Car Championship
*/

#include "machine/m2comm.h"

//#define __M2COMM_VERBOSE__

MACHINE_CONFIG_FRAGMENT( m2comm )
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type M2COMM = &device_creator<m2comm_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor m2comm_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( m2comm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m2comm_device - constructor
//-------------------------------------------------

m2comm_device::m2comm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, M2COMM, "MODEL2 COMMUNICATION BD", tag, owner, clock, "m2comm", __FILE__),
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

void m2comm_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m2comm_device::device_reset()
{
	set_linktype(16726);
	m_zfg = 0;
	m_cn = 0;
	m_fg = 0;
}

READ8_MEMBER(m2comm_device::zfg_r)
{
	UINT8 result = m_zfg | 0xFE;
#ifdef __M2COMM_VERBOSE__
	printf("m2comm-zfg_r: read register %02x for value %02x\n", offset, result);
#endif
	return result;
}

WRITE8_MEMBER(m2comm_device::zfg_w)
{
#ifdef __M2COMM_VERBOSE__
	printf("m2comm-zfg_w: %02x\n", data);
#endif
	m_zfg = data & 0x01;
}

READ8_MEMBER(m2comm_device::share_r)
{
	UINT8 result = m_shared[offset];
#ifdef __M2COMM_VERBOSE__
	printf("m2comm-share_r: read shared memory %02x for value %02x\n", offset, result);
#endif
	return result;
}

WRITE8_MEMBER(m2comm_device::share_w)
{
#ifdef __M2COMM_VERBOSE__
	printf("m2comm-share_w: %02x %02x\n", offset, data);
#endif
	m_shared[offset] = data;
}

READ8_MEMBER(m2comm_device::cn_r)
{
	return m_cn;
}

WRITE8_MEMBER(m2comm_device::cn_w)
{
	m_cn = data & 0x01;

#ifndef __M2COMM_SIMULATION__
	if (!m_cn)
		device_reset();
#else
	if (!m_cn)
	{
		// reset command
		printf("M2COMM: board disabled\n");
		m_linkenable = 0x00;
	}
	else
	{
		// init command
		printf("M2COMM: board enabled\n");
		m_linkenable = 0x01;
		m_linkid = 0x00;
		m_linkalive = 0x00;
		m_linkcount = 0x00;
		m_linktimer = 0x04; //0x00E8; // 58 fps * 4s

		comm_tick();
	}
#endif
}

READ8_MEMBER(m2comm_device::fg_r)
{
	return m_fg | (~m_zfg << 7);
}

WRITE8_MEMBER(m2comm_device::fg_w)
{
	m_fg = data & 0x01;
}

void m2comm_device::check_vint_irq()
{
#ifndef __M2COMM_SIMULATION__
#else
	comm_tick();
#endif
}

#ifdef __M2COMM_SIMULATION__
void m2comm_device::set_linktype(UINT16 linktype)
{
	m_linktype = linktype;

	switch (m_linktype)
	{
		case 16726:
			// Daytona USA / Sega Rally Championship
			printf("M2COMM: set mode 'EPR-16726 - Daytona USA'\n");
			break;
	}
}

void m2comm_device::comm_tick()
{
	switch (m_linktype)
	{
		case 16726:
			// Daytona USA / Sega Rally Championship
			comm_tick_16726();
			break;
	}
}

void m2comm_device::comm_tick_16726()
{
	if (m_linkenable == 0x01)
	{
		m_zfg ^= 1;

		int frameStart = 0x2000;
		int frameOffset = 0x0000;
		int frameSize = 0x01c0;
		int dataSize = frameSize + 1;
		int togo = 0;
		int recv = 0;
		int idx = 0;

		bool isMaster = (m_fg == 0x01);
		bool isSlave = (m_fg == 0x00);

		// if link not yet established...
		if (m_linkalive == 0x00)
		{
			// waiting...
			m_shared[0] = 0x00;
			m_shared[2] = 0xff;
			m_shared[3] = 0xff;

			// check rx socket
			if (!m_line_rx.is_open())
			{
				printf("M2COMM: listen on %s\n", m_localhost);
				m_line_rx.open(m_localhost);
			}

			// check tx socket
			if (!m_line_tx.is_open())
			{
				printf("M2COMM: connect to %s\n", m_remotehost);
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
						if (idx == 0xff)
						{
							if (isMaster)
							{
								// master gets first id and starts next state
								m_linkid = 0x01;
								m_linkcount = m_buffer[1];
								m_linktimer = 0x01;
							}
							else if (isSlave)
							{
								m_buffer[1]++;
								m_linkid = m_buffer[1];

								// forward message
								m_line_tx.write(m_buffer, dataSize);
							}
						}

						// 0xFE - link size
						else if (idx == 0xfe)
						{
							if (isSlave)
							{
								m_linkcount = m_buffer[1];

								// forward message
								m_line_tx.write(m_buffer, dataSize);
							}

							// consider it done
							printf("M2COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
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
						printf("M2COMM: droped a message...\n");
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
						m_buffer[0] = 0xff;
						m_buffer[1] = 0x01;
						m_line_tx.write(m_buffer, dataSize);
					}

					// send second packet
					else if (m_linktimer == 0x01)
					{
						m_buffer[0] = 0xfe;
						m_buffer[1] = m_linkcount;
						m_line_tx.write(m_buffer, dataSize);

						// consider it done
						printf("M2COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
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
						int slotFrom = m_linkid - idx;
						int slotDest = slotFrom + m_linkcount;
						while (slotDest < 9) {
							slotDest += m_linkcount;
						}
						while (slotDest - m_linkcount > 0) {
							slotFrom = slotDest - m_linkcount;
							if (slotDest < 9) {
								int frameOffset1 = frameStart + slotFrom * frameSize;
								int frameOffset2 = frameStart + slotDest * frameSize;
								for (int j = 0x00 ; j < frameSize ; j++)
								{
									m_shared[frameOffset2 + j] = m_shared[frameOffset1 + j];
								}
							}
							slotDest -= m_linkcount;
						}
						if (slotDest > 0) {
							// save message to "ring buffer"
							frameOffset = frameStart + (slotDest * frameSize);
							for (int j = 0x00 ; j < frameSize ; j++)
							{
								m_shared[frameOffset + j] = m_buffer[1 + j];
							}
						}
						if (idx != m_linkid)
						{
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
					printf("M2COMM: droped a message...\n");
				}
				recv = m_line_rx.read(m_buffer, dataSize);
			}

			// push message to other nodes
			m_buffer[0] = m_linkid;
			for (int j = 0x00 ; j < frameSize ; j++)
			{
				m_buffer[1 + j] = m_shared[frameStart + j];
			}
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
		//TODO:m_shared[3] = 0x00;
	}
}
#endif
