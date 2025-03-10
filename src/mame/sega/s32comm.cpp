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
( http://images.arianchen.de/sega-comm/orunners-front.jpg / http://images.arianchen.de/sega-comm/orunners-back.jpg )
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
#include "s32comm.h"
#include "emuopts.h"

#define VERBOSE 0
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_SYSTEM32_COMM, sega_s32comm_device, "s32comm", "Sega System 32 Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sega_s32comm_device::device_add_mconfig(machine_config &config)
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_s32comm_device - constructor
//-------------------------------------------------

sega_s32comm_device::sega_s32comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_SYSTEM32_COMM, tag, owner, clock)
{
	std::fill(std::begin(m_shared), std::end(m_shared), 0);

	// prepare "filenames"
	m_localhost = util::string_format("socket.%s:%s", mconfig.options().comm_localhost(), mconfig.options().comm_localport());
	m_remotehost = util::string_format("socket.%s:%s", mconfig.options().comm_remotehost(), mconfig.options().comm_remoteport());

	m_framesync = mconfig.options().comm_framesync() ? 0x01 : 0x00;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_s32comm_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_s32comm_device::device_reset()
{
	m_zfg = 0;
	m_cn = 0;
	m_fg = 0;
}

uint8_t sega_s32comm_device::zfg_r(offs_t offset)
{
	uint8_t result = m_zfg | 0xFE;
	if (!machine().side_effects_disabled())
		LOG("s32comm-zfg_r: read register %02x for value %02x\n", offset, result);
	return result;
}

void sega_s32comm_device::zfg_w(uint8_t data)
{
	LOG("s32comm-zfg_w: %02x\n", data);
	m_zfg = data & 0x01;
}

uint8_t sega_s32comm_device::share_r(offs_t offset)
{
	uint8_t result = m_shared[offset];
	if (!machine().side_effects_disabled())
		LOG("s32comm-share_r: read shared memory %02x for value %02x\n", offset, result);
	return result;
}

void sega_s32comm_device::share_w(offs_t offset, uint8_t data)
{
	LOG("s32comm-share_w: %02x %02x\n", offset, data);
	m_shared[offset] = data;
}

uint8_t sega_s32comm_device::cn_r()
{
	return m_cn | 0xFE;
}

void sega_s32comm_device::cn_w(uint8_t data)
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

uint8_t sega_s32comm_device::fg_r()
{
	return m_fg | (~m_zfg << 7) | 0x7E;
}

void sega_s32comm_device::fg_w(uint8_t data)
{
	if (!m_cn)
		return;

	m_fg = data & 0x01;
}

void sega_s32comm_device::check_vint_irq()
{
#ifndef S32COMM_SIMULATION
#else
	comm_tick();
#endif
}

#ifdef S32COMM_SIMULATION
void sega_s32comm_device::set_linktype(uint16_t linktype)
{
	m_linktype = linktype;

	switch (m_linktype)
	{
		case 14084:
			// Rad Rally
			LOG("S32COMM: set mode 'EPR-14084 - Rad Rally'\n");
			break;
		case 15033:
			// Stadium Cross / OutRunners
			LOG("S32COMM: set mode 'EPR-15033 - Stadium Cross / OutRunners'\n");
			break;
		case 15612:
			// F1 Super Lap
			LOG("S32COMM: set mode 'EPR-15612 - F1 Super Lap'\n");
			break;
		default:
			logerror("S32COMM-set_linktype: unknown linktype %d\n", linktype);
			break;
	}
}

void sega_s32comm_device::comm_tick()
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
		default:
			// do nothing
			break;
	}
}

int sega_s32comm_device::read_frame(int data_size)
{
	if (!m_line_rx)
		return 0;

	// try to read a message (handling partial reads)
	uint32_t bytes_total = 0;
	uint32_t bytes_read = 0;
	while (bytes_total < data_size)
	{
		std::error_condition filerr = m_line_rx->read(&m_buffer0[bytes_total], 0, data_size - bytes_total, bytes_read);
		if (filerr)
		{
			bytes_read = 0;
			// special case if first read returned "no data"
			if (bytes_total == 0 && std::errc::operation_would_block == filerr)
				return 0;
		}
		if ((!filerr && bytes_read == 0) || (filerr && std::errc::operation_would_block != filerr))
		{
			osd_printf_verbose("S32COMM: rx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
			m_line_rx.reset();
			if (m_linkalive == 0x01)
			{
				osd_printf_verbose("S32COMM: link lost\n");
				m_linkalive = 0x02;
				m_linktimer = 0x00;
			}
			return 0;
		}
		bytes_total += bytes_read;
	}
	return data_size;
}

void sega_s32comm_device::send_data(uint8_t frame_type, int frame_start, int frame_size, int data_size)
{
	m_buffer0[0] = frame_type;
	for (int i = 0x00 ; i < frame_size ; i++)
	{
		m_buffer0[1 + i] = m_shared[frame_start + i];
	}
	// forward message to next node
	send_frame(data_size);
}

void sega_s32comm_device::send_frame(int data_size)
{
	if (!m_line_tx)
		return;

	// try to send a message (handling partial writes)
	uint32_t bytes_total = 0;
	uint32_t bytes_sent = 0;
	while (bytes_total < data_size)
	{
		std::error_condition filerr = m_line_tx->write(&m_buffer0[bytes_total], 0, data_size - bytes_total, bytes_sent);
		if (filerr)
		{
			osd_printf_verbose("S32COMM: tx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
			m_line_tx.reset();
			if (m_linkalive == 0x01)
			{
				osd_printf_verbose("S32COMM: link lost\n");
				m_linkalive = 0x02;
				m_linktimer = 0x00;
			}
			return;
		}
		bytes_total += bytes_sent;
	}
}

void sega_s32comm_device::comm_tick_14084()
{
	// m_shared[0] = node count
	// m_shared[1] = node id
	// m_shared[2] = node mode (0 = slave, 1 = master, ff = relay)
	// m_shared[3] = node ready-to-send
	// m_shared[4] = node link status (0 = offline, 1 = online)
	if (m_linkenable == 0x01)
	{
		int frame_size = 0x0080;
		int data_size = frame_size + 1;

		bool is_master = (m_shared[2] == 0x01);
		bool is_slave = (m_shared[2] == 0x00);
		bool is_relay = (m_shared[2] == 0xFF);

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
				osd_printf_verbose("S32COMM: rx listen on %s\n", m_localhost);
				uint64_t filesize; // unused
				std::error_condition filerr = osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
				if (filerr.value() != 0)
				{
					osd_printf_verbose("S32COMM: rx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
					m_line_rx.reset();
				}
			}

			// check tx socket
			if (!m_line_tx)
			{
				osd_printf_verbose("S32COMM: tx connect to %s\n", m_remotehost);
				uint64_t filesize; // unused
				std::error_condition filerr = osd_file::open(m_remotehost, 0, m_line_tx, filesize);
				if (filerr.value() != 0)
				{
					osd_printf_verbose("S32COMM: tx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
					m_line_tx.reset();
				}
			}

			// if both sockets are there check ring
			if (m_line_rx && m_line_tx)
			{
				// try to read one message
				int recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if message id
					uint8_t idx = m_buffer0[0];

					// 0xff - link id
					if (idx == 0xff)
					{
						if (is_master)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer0[1];
							m_linktimer = 0x00;
						}
						else if (is_slave || is_relay)
						{
							// slave gets own id
							if (is_slave)
							{
								m_buffer0[1]++;
								m_linkid = m_buffer0[1];
							}

							// slave and relay forward message
							send_frame(data_size);
						}
					}

					// 0xfe - link size
					else if (idx == 0xfe)
					{
						if (is_slave || is_relay)
						{
							m_linkcount = m_buffer0[1];

							// slave and relay forward message
							send_frame(data_size);
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
						send_frame(data_size);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer0[0] = 0xfe;
						m_buffer0[1] = m_linkcount;
						send_frame(data_size);

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
				int recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					uint8_t idx = m_buffer0[0];
					if (idx > 0 && idx <= m_linkcount)
					{
						// if not own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							int frame_offset = idx * frame_size;
							for (int j = 0x00 ; j < frame_size ; j++)
							{
								m_shared[frame_offset + j] = m_buffer0[1 + j];
							}

							// forward message to other nodes
							send_frame(data_size);
						}
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xfc - VSYNC
							m_linktimer = 0x00;
							if (!is_master)
								// forward message to other nodes
								send_frame(data_size);
						}
						if (idx == 0xfd)
						{
							// 0xfd - master addional bytes
							if (!is_master)
							{
								// save message to "ring buffer"
								int frame_offset = 0x05;
								for (int j = 0x00 ; j < 0x0b ; j++)
								{
									m_shared[frame_offset + j] = m_buffer0[1 + j];
								}

								// forward message to other nodes
								send_frame(data_size);
							}
						}
					}

					// try to read another message
					recv = read_frame(data_size);
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
					int frame_start = 0x0480;
					send_data(m_linkid, frame_start, frame_size, data_size);

					// save message to "ring buffer"
					int frame_offset = m_linkid * frame_size;
					for (int j = 0x00 ; j < frame_size ; j++)
					{
						m_shared[frame_offset + j] = m_buffer0[1 + j];
					}
				}

				if (is_master)
				{
					// master sends some additional status bytes
					send_data(0xfd, 0x05, 0x0b, data_size);

					// send vsync
					m_buffer0[0] = 0xfc;
					m_buffer0[1] = 0x01;
					send_frame(data_size);
				}
			}

			// clear 03
			m_shared[3] = 0x00;
		}
	}
}

void sega_s32comm_device::comm_tick_15033()
{
	// m_shared[0] = node count
	// m_shared[1] = node id
	// m_shared[2] = node mode (0 = slave, 1 = master, ff = relay)
	// m_shared[3] = node ready-to-send
	// m_shared[4] = node link status (0 = offline, 1 = online)
	if (m_linkenable == 0x01)
	{
		int frame_size = 0x00E0;
		int data_size = frame_size + 1;

		bool is_master = (m_shared[2] == 0x01);
		bool is_slave = (m_shared[2] == 0x00);
		bool is_relay = (m_shared[2] == 0x02);

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
				osd_printf_verbose("S32COMM: rx listen on %s\n", m_localhost);
				uint64_t filesize; // unused
				std::error_condition filerr = osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
				if (filerr.value() != 0)
				{
					osd_printf_verbose("S32COMM: rx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
					m_line_rx.reset();
				}
			}

			// check tx socket
			if (!m_line_tx)
			{
				osd_printf_verbose("S32COMM: x connect to %s\n", m_remotehost);
				uint64_t filesize; // unused
				std::error_condition filerr = osd_file::open(m_remotehost, 0, m_line_tx, filesize);
				if (filerr.value() != 0)
				{
					osd_printf_verbose("S32COMM: tx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
					m_line_tx.reset();
				}
			}

			// if both sockets are there check ring
			if (m_line_rx && m_line_tx)
			{
				// try to read one messages
				int recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if message id
					uint8_t idx = m_buffer0[0];

					// 0xff - link id
					if (idx == 0xff)
					{
						if (is_master)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer0[1];
							m_linktimer = 0x00;
						}
						else if (is_slave || is_relay)
						{
							// slave gets own id
							if (is_slave)
							{
								m_buffer0[1]++;
								m_linkid = m_buffer0[1];
							}

							// slave and relay forward message
							send_frame(data_size);
						}
					}

					// 0xfe - link size
					else if (idx == 0xfe)
					{
						if (is_slave || is_relay)
						{
							m_linkcount = m_buffer0[1];

							// slave and relay forward message
							send_frame(data_size);
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
						send_frame(data_size);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer0[0] = 0xfe;
						m_buffer0[1] = m_linkcount;
						send_frame(data_size);

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
			int frame_start_rx = 0x0010;
			int frame_start_tx = 0x0710;

			do
			{
				// try to read a message
				int recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					uint8_t idx = m_buffer0[0];
					if (idx > 0 && idx <= m_linkcount)
					{
						// if not own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							int frame_offset = frame_start_rx + ((idx - 1) * frame_size);
							for (int j = 0x00 ; j < frame_size ; j++)
							{
								m_shared[frame_offset + j] = m_buffer0[1 + j];
							}

							// forward message to other nodes
							send_frame(data_size);
						}
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xfc - VSYNC
							m_linktimer = 0x00;
							if (!is_master)
								// forward message to other nodes
								send_frame(data_size);
						}
						if (idx == 0xfd)
						{
							// 0xfd - master addional bytes
							if (!is_master)
							{
								// save message to "ring buffer"
								int frame_offset = 0x05;
								for (int j = 0x00 ; j < 0x0b ; j++)
								{
									m_shared[frame_offset + j] = m_buffer0[1 + j];
								}

								// forward message to other nodes
								send_frame(data_size);
							}
						}
					}

					// try to read another message
					recv = read_frame(data_size);
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
					send_data(m_linkid, frame_start_tx, frame_size, data_size);

					// save message to "ring buffer"
					int frame_offset = frame_start_rx + ((m_linkid - 1) * frame_size);
					for (int j = 0x00 ; j < frame_size ; j++)
					{
						m_shared[frame_offset + j] = m_buffer0[1 + j];
					}
				}

				if (is_master){
					// master sends some additional status bytes
					send_data(0xfd, 0x05, 0x0b, data_size);

					// send vsync
					m_buffer0[0] = 0xfc;
					m_buffer0[1] = 0x01;
					send_frame(data_size);
				}
			}

			// clear 03
			m_shared[3] = 0x00;
		}
	}
}

void sega_s32comm_device::comm_tick_15612()
{
	// m_shared[0] = node link status (5 = linking, 1 = online)
	// m_shared[1] = node mode (0 = relay, 1 = master, 2 = slave)
	// m_shared[2] = node id
	// m_shared[3] = node count
	// m_shared[4] = ready-to-send
	if (m_linkenable == 0x01)
	{
		int frame_size = 0x00E0;
		int data_size = frame_size + 1;

		bool is_master = (m_shared[1] == 0x01);
		bool is_slave = (m_shared[1] == 0x02);
		bool is_relay = (m_shared[1] == 0x00);

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
				osd_printf_verbose("S32COMM: rx listen on %s\n", m_localhost);
				uint64_t filesize; // unused
				std::error_condition filerr = osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
				if (filerr.value() != 0)
				{
					osd_printf_verbose("S32COMM: rx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
					m_line_rx.reset();
				}
			}

			// check tx socket
			if (!m_line_tx)
			{
				osd_printf_verbose("S32COMM: tx connect to %s\n", m_remotehost);
				uint64_t filesize; // unused
				std::error_condition filerr = osd_file::open(m_remotehost, 0, m_line_tx, filesize);
				if (filerr.value() != 0)
				{
					osd_printf_verbose("S32COMM: tx connection failed - %s:%d %s\n", filerr.category().name(), filerr.value(), filerr.message());
					m_line_tx.reset();
				}
			}

			// if both sockets are there check ring
			if (m_line_rx && m_line_tx)
			{
				// try to read one message
				int recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if message id
					uint8_t idx = m_buffer0[0];

					// 0xff - link id
					if (idx == 0xff)
					{
						if (is_master)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer0[1];
							m_linktimer = 0x00;
						}
						else if (is_slave || is_relay)
						{
							// slave gets own id
							if (is_slave)
							{
								m_buffer0[1]++;
								m_linkid = m_buffer0[1];
							}

							// slave and relay forward message
							send_frame(data_size);
						}
					}

					// 0xfe - link size
					else if (idx == 0xfe)
					{
						if (is_slave || is_relay)
						{
							m_linkcount = m_buffer0[1];

							// slave and relay forward message
							send_frame(data_size);
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
						m_buffer0[0] = 0xFF;
						m_buffer0[1] = 0x01;
						send_frame(data_size);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer0[0] = 0xFE;
						m_buffer0[1] = m_linkcount;
						send_frame(data_size);

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
			int frame_start = 0x0010;

			do
			{
				// try to read a message
				int recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					uint8_t idx = m_buffer0[0];
					if (idx > 0 && idx <= m_linkcount)
					{
						// if not own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							int frame_offset = frame_start + (idx * frame_size);
							for (int j = 0x00 ; j < frame_size ; j++)
							{
								m_shared[frame_offset + j] = m_buffer0[1 + j];
							}

							// forward message to other nodes
							send_frame(data_size);
						}
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xfc - VSYNC
							m_linktimer = 0x00;
							if (!is_master)
								// forward message to other nodes
								send_frame(data_size);
						}
						if (idx == 0xfd)
						{
							// 0xfd - master addional bytes
							if (!is_master)
							{
								// save message to "ring buffer"
								int frame_offset = 0x05;
								for (int j = 0x00 ; j < 0x0b ; j++)
								{
									m_shared[frame_offset + j] = m_buffer0[1 + j];
								}

								// forward message to other nodes
								send_frame(data_size);
							}
						}
					}

					// try to read another message
					recv = read_frame(data_size);
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
					send_data(m_linkid, frame_start, frame_size, data_size);

					// save message to "ring buffer"
					int frame_offset = frame_start + (m_linkid * frame_size);
					for (int j = 0x00 ; j < frame_size ; j++)
					{
						m_shared[frame_offset + j] = m_buffer0[1 + j];
					}
				}

				if (is_master)
				{
					// master sends some additional status bytes
					// master sends additional status bytes
					send_data(0xfd, 0x05, 0x0b, data_size);

					// send vsync
					m_buffer0[0] = 0xfc;
					m_buffer0[1] = 0x01;
					send_frame(data_size);
				}
			}

			// clear 04
			m_shared[4] = 0x00;
		}
	}
}
#endif
