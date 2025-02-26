// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
834-6740
( http://images.arianchen.de/sega-comm/pdriftl_link_big.jpg )
|-------------| |-| |--------|---|-|
| _ 315-5336  RX  TX          CNC  |
||C|                               |
||N| MB8421 EPR-12028  MB89372P-SH |
||A|          5563                 |
||_|                      315-5337 |
|    SW1     LED   16MHz     Z80E  |
|----------------------------------|
Notes:
     PALs - 315-5337, 315-5336, both PAL16L8
      Z80 - Clock 8.000MHz [16/2]
     5563 - Toshiba TC5563APL-12L 8kBx8-bit SRAM
   MB8421 - Fujitsu 2kBx8-bit Dual-Port SRAM (SDIP52)
  MB89372 - Fujitsu Multi-Protocol Controller (SDIP64)
            Uses 3 serial data transfer protocols: ASYNC, COP & BOP. Has a built
            in DMA controller and Interrupt controller to handle interrupts
            from the serial interface unit (SIU) & DMA controller (DMAC)
EPR-12028 - 27C256 EPROM
      CNA - 50 Pin connector joining to main board
      CNC - 10 pin connector
      SW1 - 8-position DIP switch
*/

#include "emu.h"
#include "emuopts.h"
#include "ybdcomm.h"

#define Z80_TAG     "commcpu"

#define VERBOSE 0
#include "logmacro.h"

/*************************************
 *  YBDCOMM Memory Map
 *************************************/
void ybdcomm_device::ybdcomm_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).ram();
	map(0x4000, 0x47ff).rw(m_dpram, FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
}

/*************************************
 *  YBDCOMM I/O Map
 *************************************/
void ybdcomm_device::ybdcomm_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).rw(m_mpc, FUNC(mb89372_device::read), FUNC(mb89372_device::write));
	map(0x40, 0x40).r(FUNC(ybdcomm_device::z80_stat_r));
	map(0x80, 0x80).w(FUNC(ybdcomm_device::z80_stat_w));
	map(0xc0, 0xc0).portr("Link_SW1");
}

static INPUT_PORTS_START( ybdcomm )
	PORT_START("Link_SW1")
	PORT_DIPNAME( 0x0f, 0x01, "Cabinet ID" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	// enabled for debugging
	PORT_DIPSETTING(    0x09, "9 (invalid)" )
	PORT_DIPSETTING(    0x0a, "10 (invalid)" )
	PORT_DIPSETTING(    0x0b, "11 (invalid)" )
	PORT_DIPSETTING(    0x0c, "12 (invalid)" )
	PORT_DIPSETTING(    0x0d, "13 (invalid)" )
	PORT_DIPSETTING(    0x0e, "14 (invalid)" )
	PORT_DIPSETTING(    0x0f, "15 (invalid)" )

	PORT_DIPNAME( 0xf0, 0x10, "Cabinet Count" )
	PORT_DIPSETTING(    0x00, "0 (invalid)" )
	PORT_DIPSETTING(    0x10, "1 (invalid)" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x50, "5" )
	PORT_DIPSETTING(    0x60, "6" )
	PORT_DIPSETTING(    0x70, "7" )
	PORT_DIPSETTING(    0x80, "8" )
	// enabled for debugging
	PORT_DIPSETTING(    0x90, "9 (invalid)" )
	PORT_DIPSETTING(    0xa0, "10 (invalid)" )
	PORT_DIPSETTING(    0xb0, "11 (invalid)" )
	PORT_DIPSETTING(    0xc0, "12 (invalid)" )
	PORT_DIPSETTING(    0xd0, "13 (invalid)" )
	PORT_DIPSETTING(    0xe0, "14 (invalid)" )
	PORT_DIPSETTING(    0xf0, "15 (invalid)" )
INPUT_PORTS_END

ROM_START( ybdcomm )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("epr12028")

	// found on Power Drift Link
	ROM_SYSTEM_BIOS( 0, "epr12028", "EPR-12028" )
	ROMX_LOAD( "epr-12028", 0x000000, 0x008000, CRC(bb682a92) SHA1(0445bdbca0db9edecd826da37cd2d3afc57c5cf6), ROM_BIOS(0) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(YBDCOMM, ybdcomm_device, "ybdcomm", "Sega Y-Board Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ybdcomm_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, 8000000); // 16 MHz / 2
	m_cpu->set_memory_map(&ybdcomm_device::ybdcomm_mem);
	m_cpu->set_io_map(&ybdcomm_device::ybdcomm_io);

	MB8421(config, m_dpram).intl_callback().set(FUNC(ybdcomm_device::dpram_int5_w));

	MB89372(config, m_mpc, 8000000); // 16 MHz / 2
	m_mpc->out_hreq_callback().set(FUNC(ybdcomm_device::mpc_hreq_w));
	m_mpc->out_irq_callback().set(FUNC(ybdcomm_device::mpc_int7_w));
	m_mpc->in_memr_callback().set(FUNC(ybdcomm_device::mpc_mem_r));
	m_mpc->out_memw_callback().set(FUNC(ybdcomm_device::mpc_mem_w));
}

ioport_constructor ybdcomm_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ybdcomm );
}

const tiny_rom_entry *ybdcomm_device::device_rom_region() const
{
	return ROM_NAME( ybdcomm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ybdcomm_device - constructor
//-------------------------------------------------

ybdcomm_device::ybdcomm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, YBDCOMM, tag, owner, clock),
	m_cpu(*this, Z80_TAG),
	m_dpram(*this, "dpram"),
	m_mpc(*this, "commmpc")
{
#ifdef YBDCOMM_SIMULATION
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
#endif
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ybdcomm_device::device_start()
{
	m_ybd_stat = 0;
	m_z80_stat = 0;

#ifdef YBDCOMM_SIMULATION
	m_tick_timer = timer_alloc(FUNC(ybdcomm_device::tick_timer), this);
	m_tick_timer->adjust(attotime::from_hz(600), 0, attotime::from_hz(600));
#endif
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ybdcomm_device::device_reset()
{
}

void ybdcomm_device::device_reset_after_children()
{
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_mpc->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

uint8_t ybdcomm_device::ex_r(offs_t offset)
{
	int bank = offset >> 11;

	switch (bank)
	{
		case 0:
			return m_dpram->right_r(offset);

		case 1:
			// z80 status
			LOG("ybdcomm-ex_r: %02x %02x\n", bank, m_z80_stat);
			return m_z80_stat;

		case 2:
			// status register?
			LOG("ybdcomm-ex_r: %02x %02x\n", bank, m_ybd_stat);
			return m_ybd_stat;

		default:
			logerror("ybdcomm-ex_r: %02x\n", offset);
			return 0xff;
}	}


void ybdcomm_device::ex_w(offs_t offset, uint8_t data)
{
	int bank = offset >> 11;
	logerror("ybdcomm-ex_w: %04x %02x %02x\n", offset, bank, data);

	switch (bank)
	{
		case 0:
			m_dpram->right_w(offset, data);
			break;

		case 1:
			// z80 status
			logerror("ybdcomm-ex_w: %02x %02x\n", offset, data);
			break;

		case 2:
			// status register?
			// bit 7 = on/off toggle
			// bit 1 = test flag?
			// bit 0 = ready to send?
			m_ybd_stat = data;
			LOG("ybdcomm-ex_w: %02x %02x\n", offset, data);
#ifndef YBDCOMM_SIMULATION
			if (m_ybd_stat & 0x80)
			{
				m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				m_mpc->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			else
			{
				device_reset();
				device_reset_after_children();
			}
#else
			if (m_ybd_stat & 0x80)
			{
				// link active
				if (!m_linkenable)
				{
					// init command
					osd_printf_verbose("YBDCOMM: board enabled\n");
					m_linkenable = 0x01;
					m_linkid = 0x00;
					m_linkalive = 0x00;
					m_linkcount = 0x00;
					m_linktimer = 0x003a;

					// read dips and write to shared memory
					uint8_t sw1 = ioport("Link_SW1")->read();
					mpc_mem_w(0x4000, sw1 & 0x0f);
					mpc_mem_w(0x4001, (sw1 >> 4) & 0x0f);
				}
			}
			else
			{
				if (m_linkenable)
				{
					// reset command
					osd_printf_verbose("YBDCOMM: board disabled\n");
					m_linkenable = 0x00;
				}
			}
#endif
			break;
		default:
			logerror("ybdcomm-ex_w: %02x %02x\n", offset, data);
	}
}

void ybdcomm_device::mpc_hreq_w(int state)
{
	m_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_mpc->hack_w(state);
}

void ybdcomm_device::dpram_int5_w(int state)
{
	m_cpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xef); // Z80 INT5
}

void ybdcomm_device::mpc_int7_w(int state)
{
	logerror("mpc_int7_w: %02x\n", state);
	m_cpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xff); // Z80 INT7
}

uint8_t ybdcomm_device::mpc_mem_r(offs_t offset)
{
	return m_cpu->space(AS_PROGRAM).read_byte(offset);
}

void ybdcomm_device::mpc_mem_w(offs_t offset, uint8_t data)
{
	m_cpu->space(AS_PROGRAM).write_byte(offset, data);
}

uint8_t ybdcomm_device::z80_stat_r()
{
	return m_ybd_stat;
}

void ybdcomm_device::z80_stat_w(uint8_t data)
{
	m_z80_stat = data;
}


#ifdef YBDCOMM_SIMULATION
TIMER_CALLBACK_MEMBER(ybdcomm_device::tick_timer)
{
	comm_tick();
}

int ybdcomm_device::comm_frameOffset(uint8_t cabIdx)
{
	switch (cabIdx)
	{
		case 1:
			return 0x4010;

		case 2:
			return 0x4190;

		case 3:
			return 0x41c0;
		case 4:
			return 0x41f0;
		case 5:
			return 0x4220;
		case 6:
			return 0x4250;
		case 7:
			return 0x4280;
		case 8:
			return 0x42b0;

		default:
			return 0x4400;
	}
}

int ybdcomm_device::comm_frameSize(uint8_t cabIdx)
{
	switch (cabIdx)
	{
		case 1:
			return 0x0180;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			return 0x0030;

		default:
			return 0x0000;
	}
}

void ybdcomm_device::comm_tick()
{
	if (m_linkenable == 0x01)
	{
		std::error_condition filerr;
		uint8_t cabIdx = mpc_mem_r(0x4000);

		int frameSize;
		int frameOffset;

		int dataSize = 0x180 + 1;
		int recv = 0;
		int idx = 0;

		bool isMaster = (cabIdx == 0x01);
		bool isSlave = (cabIdx > 0x01);
		bool isRelay = (cabIdx == 0x00);

		if (m_linkalive == 0x02)
		{
			// link failed... (guesswork)
			m_z80_stat = 0xff;
			return;
		}
		else if (m_linkalive == 0x00)
		{
			// link not yet established... (guesswork)
			m_z80_stat = 0x00;

			// check rx socket
			if (!m_line_rx)
			{
				osd_printf_verbose("YBDCOMM: listen on %s\n", m_localhost);
				uint64_t filesize; // unused
				filerr = osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
				if (filerr.value() != 0)
				{
					osd_printf_verbose("YBDCOMM: rx connection failed\n");
					m_line_rx.reset();
				}
			}

			// check tx socket
			if (!m_line_tx)
			{
				osd_printf_verbose("YBDCOMM: connect to %s\n", m_remotehost);
				uint64_t filesize; // unused
				filerr = osd_file::open(m_remotehost, 0, m_line_tx, filesize);
				if (filerr.value() != 0)
				{
					osd_printf_verbose("YBDCOMM: tx connection failed\n");
					m_line_tx.reset();
				}
			}

			// if both sockets are there check ring
			if (m_line_rx && m_line_tx)
			{
				// try to read one message
				recv = read_frame(dataSize);
				while (recv > 0)
				{
					// check message id
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
						else
						{
							// slave get own id, relay does nothing
							if (isSlave)
							{
								m_buffer0[1]++;
								m_linkid = m_buffer0[1];
							}

							// forward message to other nodes
							send_frame(dataSize);
						}
					}

					// 0xFE - link size
					else if (idx == 0xfe)
					{
						if (isSlave || isRelay)
						{
							m_linkcount = m_buffer0[1];

							// forward message to other nodes
							send_frame(dataSize);
						}

						// consider it done
						osd_printf_verbose("YBDCOMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_z80_stat = 0x01;
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
						osd_printf_verbose("YBDCOMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_z80_stat = 0x01;
					}

					else if (m_linktimer > 0x01)
					{
						// decrease delay timer
						m_linktimer--;
					}
				}
			}
		}

		if (m_linkalive == 0x01)
		{
			// link established
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
						// save message to "ring buffer"
						frameOffset = comm_frameOffset(idx);
						frameSize = comm_frameSize(idx);
						for (int j = 0x00 ; j < frameSize ; j++)
						{
							mpc_mem_w(frameOffset + j, m_buffer0[1 + j]);
						}

						// if not own message
						if (idx != cabIdx)
						{
							// forward message to other nodes
							send_frame(dataSize);
						}
						else
						{
							m_z80_stat &= 0x7F;
							m_linktimer = 0x00;
						}
					}

					// try to read another message
					recv = read_frame(dataSize);
				}
			}
			while (m_linktimer == 0x01);

			m_z80_stat ^= 0x01;

			// update "ring buffer" if link established
			// live relay does not send data
			if (cabIdx != 0x00)
			{
				// check ready-to-send flag
				if (m_ybd_stat & 0x01)
				{
					frameOffset = comm_frameOffset(cabIdx);
					frameSize = comm_frameSize(cabIdx);
					send_data(cabIdx, frameOffset, frameSize, dataSize);
					m_z80_stat |= 0x80;

					// enable wait for sync
					m_linktimer = m_framesync;
				}
			}

			// clear ready-to-send flag
			m_ybd_stat &= 0xFE;
		}
	}
}

int ybdcomm_device::read_frame(int dataSize)
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
				if ((!filerr && recv == 0) || (filerr && std::errc::operation_would_block != filerr))
					togo = 0;
			}
		}
	}
	if ((!filerr && recv == 0) || (filerr && std::errc::operation_would_block != filerr))
	{
		osd_printf_verbose("YBDCOMM: rx connection error\n");
		m_line_rx.reset();
		if (m_linkalive == 0x01)
		{
			m_linkalive = 0x02;
			m_linktimer = 0x00;
			m_z80_stat = 0xff;
		}
	}
	return recv;
}

void ybdcomm_device::send_data(uint8_t frameType, int frameOffset, int frameSize, int dataSize)
{
	m_buffer0[0] = frameType;
	for (int i = 0x00 ; i < frameSize ; i++)
	{
		m_buffer0[1 + i] = mpc_mem_r(frameOffset + i);
	}
	send_frame(dataSize);
}

void ybdcomm_device::send_frame(int dataSize)
{
	if (!m_line_tx)
		return;

	std::uint32_t written;
	std::error_condition filerr = m_line_tx->write(&m_buffer0, 0, dataSize, written);
	if (filerr)
	{
		osd_printf_verbose("YBDCOMM: tx connection error\n");
		m_line_tx.reset();
		if (m_linkalive == 0x01)
		{
			m_linkalive = 0x02;
			m_linktimer = 0x00;
			m_z80_stat = 0xff;
		}
	}
}
#endif