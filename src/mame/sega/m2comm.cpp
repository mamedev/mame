// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
Sega MODEL2 COMMUNICATION BOARD 837-10537
( http://images.arianchen.de/sega-comm/model2-front.jpg / http://images.arianchen.de/sega-comm/model2-back.jpg )
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
( http://images.arianchen.de/sega-comm/model2a-front.jpg / http://images.arianchen.de/sega-comm/model2a-back.jpg )
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
        18643.7    ManxTT
        18643A.7   ManxTT


Sega PC BD MODEL2 B-CRX COMMUNICATION 837-11615
( http://images.arianchen.de/sega-comm/model2b-com_top.jpg / http://images.arianchen.de/sega-comm/model2b-com_bot.jpg )
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
( http://images.arianchen.de/sega-comm/model2c-com_top.jpg )
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

#include "emu.h"
#include "m2comm.h"
#include "emuopts.h"

#define VERBOSE 0
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(M2COMM, m2comm_device, "m2comm", "Model 2 Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void m2comm_device::device_add_mconfig(machine_config &config)
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m2comm_device - constructor
//-------------------------------------------------

m2comm_device::m2comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M2COMM, tag, owner, clock)
{
	// prepare "filenames"
	m_localhost = util::string_format("socket.%s:%s", mconfig.options().comm_localhost(), mconfig.options().comm_localport());
	m_remotehost = util::string_format("socket.%s:%s", mconfig.options().comm_remotehost(), mconfig.options().comm_remoteport());

	m_framesync = mconfig.options().comm_framesync() ? 0x01 : 0x00;

	m_frameoffset = 0x1c0; // default
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
	m_zfg = 0;
	m_cn = 0;
	m_fg = 0;
}

uint8_t m2comm_device::zfg_r(offs_t offset)
{
	uint8_t result = m_zfg | (~m_fg << 7) | 0x7e;
	LOG("m2comm-zfg_r: read register %02x for value %02x\n", offset, result);
	return result;
}

void m2comm_device::zfg_w(uint8_t data)
{
	LOG("m2comm-zfg_w: %02x\n", data);
	m_zfg = data & 0x01;
}

uint8_t m2comm_device::share_r(offs_t offset)
{
	uint8_t result = m_shared[offset];
	LOG("m2comm-share_r: read shared memory %02x for value %02x\n", offset, result);
	return result;
}

void m2comm_device::share_w(offs_t offset, uint8_t data)
{
	LOG("m2comm-share_w: %02x %02x\n", offset, data);
	m_shared[offset] = data;
}

uint8_t m2comm_device::cn_r()
{
	return m_cn | 0xfe;
}

void m2comm_device::cn_w(uint8_t data)
{
	m_cn = data & 0x01;

#ifndef M2COMM_SIMULATION
	if (!m_cn)
		device_reset();
#else
	if (!m_cn)
	{
		// reset command
		osd_printf_verbose("M2COMM: board disabled\n");
		m_linkenable = 0x00;
		m_zfg = 0;
		m_cn = 0;
		m_fg = 0;
	}
	else
	{
		// init command
		osd_printf_verbose("M2COMM: board enabled\n");
		m_linkenable = 0x01;
		m_linkid = 0x00;
		m_linkalive = 0x00;
		m_linkcount = 0x00;
		m_linktimer = 0x00e8; // 58 fps * 4s

		// zero memory
		for (int i = 0; i < 0x4000; i++)
		{
			m_shared[i] = 0x00;
		}

		m_shared[0x01] = 0x02;
		// TODO - check EPR-16726 on Daytona USA and Sega Rally Championship
		// EPR-18643(A) - these are accessed by VirtuaON and Sega Touring Car Championship

		// frame_size - 0x0e00. is it = frameoffset*8 ? if true - it should be 0xc00 for Power Sled
		m_shared[0x12] = 0x00;
		m_shared[0x13] = 0x0e;

		// frame_offset - 0x01c0 in most games or 0x180 in Power Sled
		m_shared[0x14] = m_frameoffset & 0xff;
		m_shared[0x15] = m_frameoffset >> 8;

		comm_tick();
	}
#endif
}

uint8_t m2comm_device::fg_r()
{
#ifdef M2COMM_SIMULATION
	read_fg();
#endif
	return m_fg | (~m_zfg << 7) | 0x7e;
}

void m2comm_device::fg_w(uint8_t data)
{
	m_fg = data & 0x01;
}

void m2comm_device::check_vint_irq()
{
#ifndef M2COMM_SIMULATION
#else
	comm_tick();
#endif
}

#ifdef M2COMM_SIMULATION
void m2comm_device::comm_tick()
{
	if (m_linkenable == 0x01)
	{
		int frame_size = m_shared[0x13] << 8 | m_shared[0x12];
		int data_size = frame_size + 1;

		// EPR-16726 uses m_fg for Master/Slave
		// EPR-18643(A) seems to check m_shared[1], with a fallback to m_fg
		bool is_master = (m_fg == 0x01 || m_shared[1] == 0x01);
		bool is_slave = (m_fg == 0x00 && m_shared[1] == 0x02);
		bool is_relay = (m_fg == 0x00 && m_shared[1] == 0x00);

		if (m_linkalive == 0x02)
		{
			// link failed...
			m_shared[0] = 0xff;
			return;
		}
		else if (m_linkalive == 0x00)
		{
			// link not yet established...
			m_shared[0] = 0x00;
			m_shared[2] = 0xff;
			m_shared[3] = 0xff;

			// check rx socket
			if (!m_line_rx)
			{
				osd_printf_verbose("M2COMM: rx listen on %s\n", m_localhost);
				uint64_t filesize; // unused
				std::error_condition filerr = osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
				if (filerr.value() != 0)
				{
					osd_printf_verbose("M2COMM: rx connection failed - %02x, %s\n", filerr.value(), filerr.message());
					m_line_rx.reset();
				}
			}

			// check tx socket
			if (!m_line_tx)
			{
				osd_printf_verbose("M2COMM: tx connect to %s\n", m_remotehost);
				uint64_t filesize; // unused
				std::error_condition filerr = osd_file::open(m_remotehost, 0, m_line_tx, filesize);
				if (filerr.value() != 0)
				{
					osd_printf_verbose("M2COMM: tx connection failed - %02x, %s\n", filerr.value(), filerr.message());
					m_line_tx.reset();
				}
			}

			// if both sockets are there check ring
			if (m_line_rx && m_line_tx)
			{
				m_zfg ^= 0x01;

				// try to read one message
				int recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if message id
					uint8_t idx = m_buffer0[0];

					// 0xFF - link id
					if (idx == 0xff)
					{
						if (is_master)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer0[1];
							m_linktimer = 0x00;
						}
						else
						{
							// slave get own id, relay does nothing
							if (is_slave)
							{
								m_buffer0[1]++;
							}

							// forward message to other nodes
							send_frame(data_size);
						}
					}

					// 0xFE - link size
					else if (idx == 0xfe)
					{
						if (is_slave)
						{
							// fetch linkid and linkcount, then decrease linkid
							m_linkid = m_buffer0[1];
							m_linkcount = m_buffer0[2];
							m_buffer0[1]--;

							// forward message to other nodes
							send_frame(data_size);
						}
						else if (is_relay)
						{
							// fetch linkid and linkcount, then decrease linkid
							m_linkid = 0x00;
							m_linkcount = m_buffer0[2];

							// forward message to other nodes
							send_frame(data_size);
						}

						// consider it done
						osd_printf_verbose("M2COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[0] = 0x01;
						m_shared[2] = m_linkid;
						m_shared[3] = m_linkcount;
					}

					if (m_linkalive == 0x00)
						recv = read_frame(data_size);
					else
						recv = 0;
				}

				// if we are master and link is not yet established
				if (is_master && (m_linkalive == 0x00))
				{
					// send first packet
					if (m_linktimer == 0x01)
					{
						m_buffer0[0] = 0xff;
						m_buffer0[1] = 0x01;
						m_buffer0[2] = 0x00;
						send_frame(data_size);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer0[0] = 0xfe;
						m_buffer0[1] = m_linkcount;
						m_buffer0[2] = m_linkcount;
						send_frame(data_size);

						// consider it done
						osd_printf_verbose("M2COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
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

		// if link established
		if (m_linkalive == 0x01)
		{
			int frame_start = 0x2000;
			int frame_offset = frame_start | m_shared[0x15] << 8 | m_shared[0x14];

			do
			{
				// try to read a message
				int recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					uint8_t idx = m_buffer0[0];
					if (idx >= 0 && idx <= m_linkcount)
					{
						// save message to "ring buffer"
						for (int j = 0x00 ; j < frame_size ; j++)
						{
							m_shared[frame_offset + j] = m_buffer0[1 + j];
						}
						m_zfg ^= 0x01;
						if (is_slave)
							send_data(m_linkid, frame_start, frame_size, data_size);
						else if (is_relay)
							send_frame(data_size);
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xFC - VSYNC
							m_linktimer = 0x00;

							if (!is_master)
								// forward message to other nodes
								send_frame(data_size);
						}
					}

					// try to read another message
					recv = read_frame(data_size);
				}
			}
			while (m_linktimer == 0x01);

			// enable wait for vsync
			m_linktimer = m_framesync;

			if (is_master)
			{
				// update "ring buffer" if link established
				send_data(m_linkid, frame_start, frame_size, data_size);

				// send vsync
				m_buffer0[0] = 0xfc;
				m_buffer0[1] = 0x01;
				send_frame(data_size);
			}

			m_zfg_delay = 0x02;
		}
	}
}

void m2comm_device::read_fg()
{
	if (m_zfg_delay > 0x00)
	{
		m_zfg_delay--;
		return;
	}
	if (m_linkalive == 0x01)
	{
		int frame_start = 0x2000;
		int frame_offset = frame_start | m_shared[0x15] << 8 | m_shared[0x14];

		int frame_size = m_shared[0x13] << 8 | m_shared[0x12];
		int data_size = frame_size + 1;

		// EPR-16726 uses m_fg for Master/Slave
		// EPR-18643(A) seems to check m_shared[1], with a fallback to m_fg
		bool is_master = (m_fg == 0x01 || m_shared[1] == 0x01);
		bool is_slave = (m_fg == 0x00 && m_shared[1] == 0x02);
		bool is_relay = (m_fg == 0x00 && m_shared[1] == 0x00);

		do
		{
			// try to read a message
			int recv = read_frame(data_size);
			while (recv > 0)
			{
				// check if valid id
				uint8_t idx = m_buffer0[0];
				if (idx >= 0 && idx <= m_linkcount)
				{
					// save message to "ring buffer"
					for (int j = 0x00 ; j < frame_size ; j++)
					{
						m_shared[frame_offset + j] = m_buffer0[1 + j];
					}
					m_zfg ^= 0x01;
					if (is_slave)
						send_data(m_linkid, frame_start, frame_size, data_size);
					else if (is_relay)
						send_frame(data_size);
				}
				else
				{
					if (idx == 0xfc)
					{
						// 0xFC - VSYNC
						m_linktimer = 0x00;

						if (!is_master)
							// forward message to other nodes
							send_frame(data_size);
					}
				}

				// try to read another message
				recv = read_frame(data_size);
			}
		}
		while (m_linktimer == 0x01);
	}
}

int m2comm_device::read_frame(int data_size)
{
	if (!m_line_rx)
		return 0;

	// try to read a message
	uint32_t recv = 0;
	std::error_condition filerr = m_line_rx->read(m_buffer0, 0, data_size, recv);
	if (recv > 0)
	{
		// check if message complete
		if (recv != data_size)
		{
			// only part of a message - read on
			uint32_t togo = data_size - recv;
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
				if ((!filerr && recv == 0) || (filerr && std::errc::operation_would_block != filerr))
				{
					togo = 0;
				}
			}
		}
	}
	if ((!filerr && recv == 0) || (filerr && std::errc::operation_would_block != filerr))
	{
		osd_printf_verbose("M2COMM: rx connection failed - %02x, %s\n", filerr.value(), filerr.message());
		m_line_rx.reset();
		if (m_linkalive == 0x01)
		{
			osd_printf_verbose("M2COMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
		}
	}
	return recv;
}

void m2comm_device::send_data(uint8_t frame_type, int frame_start, int frame_size, int data_size)
{
	m_buffer0[0] = frame_type;
	for (int i = 0x0000 ; i < frame_size ; i++)
	{
		m_buffer0[1 + i] = m_shared[frame_start + i];
	}
	send_frame(data_size);
}

void m2comm_device::send_frame(int data_size){
	if (!m_line_tx)
		return;

	uint32_t written = 0;
	std::error_condition filerr = m_line_tx->write(&m_buffer0, 0, data_size, written);
	if (filerr)
	{
		osd_printf_verbose("M2COMM: tx connection failed - %02x, %s\n", filerr.value(), filerr.message());
		m_line_tx.reset();
		if (m_linkalive == 0x01)
		{
			osd_printf_verbose("M2COMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
		}
	}
}
#endif
