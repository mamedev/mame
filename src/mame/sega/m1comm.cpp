// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
Comm PCB
--------

MODEL-1 COMMUNICATION BD 837-8842 171-6293B (C) SEGA 1992
|--------------------------------------------------------------------------------|
|                                                                                |
|    MB89237A            MB89374                                                 |
|       JP4                                                                 LED1 |
|    15112.17            Z80                                                     |
|    JP2  JP3                                                       75179        |
|    MB8464              315-5624                                     JP6        |
|                                                       315-5547                 |
|        315-5611                                            SW1    PC910     CN4|
|                                                                                |
|                                                                   PC910     CN5|
|     MB8421             MB8431                                JP7               |
|                                                                   JP5          |
|        JP8                                                                  CN7|
|                CN1                                    CN2                      |
| |---------------------------------|   |---------------------------------|   CN6|
| |---------------------------------|   |---------------------------------|      |
|--------------------------------------------------------------------------------|
Notes:
      15112.17 - AMD AM27C100 128k x8 EPROM (DIP32, labelled 'EPR-15112')
      Z80      - Zilog Z0840008PSC Z80 CPU, running at 8.000MHz (DIP40)
      MB8464   - Fujitsu MB8464 8k x8 SRAM (DIP28)
      MB8421   - Fujitsu MB8421-12LP 2k x8 SRAM (SDIP52)
      MB8431   - Fujitsu MB8431-90LP 2k x8 SRAM (SDIP52)
      MB89237A - Fujitsu MB89237A DMA-Controller (DIP20) [most likely i8237A clone]
      MB89374  - Fujitsu MB89374 Data Link Controller (SDIP42)
      75179    - Texas Instruments SN75179 Differential Driver and Receiver Pair (DIP8)
      315-5547 - AMI 18CV8PC-25 PAL (DIP20)
      315-5624 - MMI PAL16L8BCN PAL (DIP20)
      315-5611 - Lattice GAL16V8A PAL (DIP20)
      PC910    - Sharp PC910 opto-isolator (x2, DIP8)
      SW1      - Push Button Switch (enables board)
      CN1, CN2 - Connectors to join Comm board to Video board
      CN4      - 8 pin connector (DIFFERENTIAL port)
      CN5      - 6 pin connector (SERIAL port)
      CN6, CN7 - TOSLINK-Connectors for network optical cable link
      JP2      - Jumper, set to 2-3 (connected to EPROM A15)
      JP3      - Jumper, set to 1-2 (connected to EPROM A16)
      JP4      - Jumper, set to 1-2
      JP5      - Jumper, shorted (enables TOSLINK RX channel)
      JP6      - Jumper, not shorted (enables DIFFERERENTIAL RX channel)
      JP7      - Jumper, not shorted (enables SERIAL RX channel)
      JP8      - Jumper, set to 1-2 (selects CLOCK SOURCE)
*/

#include "emu.h"
#include "machine/m1comm.h"
#include "emuopts.h"

#define Z80_TAG     "commcpu"

/*************************************
 *  M1COMM Memory Map
 *************************************/
void m1comm_device::m1comm_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xc000, 0xffff).mask(0x0fff).rw(FUNC(m1comm_device::share_r), FUNC(m1comm_device::share_w));
}

/*************************************
 *  M1COMM I/O Map
 *************************************/
void m1comm_device::m1comm_io(address_map &map)
{
	map.global_mask(0x7f);
	map(0x00, 0x1f).rw(m_dlc, FUNC(mb89374_device::read), FUNC(mb89374_device::write));
	map(0x20, 0x2f).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x40, 0x5f).mask(0x01).rw(FUNC(m1comm_device::syn_r), FUNC(m1comm_device::syn_w));
	map(0x60, 0x7f).mask(0x01).rw(FUNC(m1comm_device::zfg_r), FUNC(m1comm_device::zfg_w));
}

ROM_START( m1comm )
	ROM_REGION( 0x20000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("epr15112")

	// found on Virtua Racing and WingWar
	ROM_SYSTEM_BIOS( 0, "epr15112", "EPR-15112" )
	ROMX_LOAD( "epr-15112.17", 0x0000, 0x20000, CRC(4950e771) SHA1(99014124e0324dd114cb22f55159d18b597a155a), ROM_BIOS(0) )

	// found on Virtua Formula
	ROM_SYSTEM_BIOS( 1, "epr15624", "EPR-15624" )
	ROMX_LOAD( "epr-15624.17", 0x0000, 0x20000, CRC(9b3ba315) SHA1(0cd0983cc8b2f2d6b41617d0d0a24cc6c188e62a), ROM_BIOS(1) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(M1COMM, m1comm_device, "m1comm", "Model-1 Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void m1comm_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, 8000000); // 32 MHz / 4
	m_cpu->set_memory_map(&m1comm_device::m1comm_mem);
	m_cpu->set_io_map(&m1comm_device::m1comm_io);

	AM9517A(config, m_dma, 8000000); // 32 MHz / 4
	m_dma->out_hreq_callback().set(FUNC(m1comm_device::dma_hreq_w));
	m_dma->in_memr_callback().set(FUNC(m1comm_device::dma_mem_r));
	m_dma->out_memw_callback().set(FUNC(m1comm_device::dma_mem_w));
	m_dma->out_dack_callback<2>().set(m_dlc, FUNC(mb89374_device::pi3_w));
	m_dma->out_dack_callback<3>().set(m_dlc, FUNC(mb89374_device::pi2_w));
	m_dma->out_eop_callback().set(m_dlc, FUNC(mb89374_device::ci_w));
	m_dma->in_ior_callback<2>().set(m_dlc, FUNC(mb89374_device::dma_r));
	m_dma->out_iow_callback<3>().set(m_dlc, FUNC(mb89374_device::dma_w));

	MB89374(config, m_dlc, 8000000); // 32 MHz / 4
	m_dlc->out_po_callback<2>().set(m_dma, FUNC(am9517a_device::dreq3_w));
	m_dlc->out_po_callback<3>().set(m_dma, FUNC(am9517a_device::dreq2_w));
	m_dlc->out_irq_callback().set(FUNC(m1comm_device::dlc_int7_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------
const tiny_rom_entry *m1comm_device::device_rom_region() const
{
	return ROM_NAME( m1comm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m1comm_device - constructor
//-------------------------------------------------

m1comm_device::m1comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M1COMM, tag, owner, clock),
	m_cpu(*this, Z80_TAG),
	m_dma(*this, "commdma"),
	m_dlc(*this, "commdlc")
{
#ifdef M1COMM_SIMULATION
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

void m1comm_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m1comm_device::device_reset()
{
	m_syn = 0;
	m_zfg = 0;
	m_cn = 0;
	m_fg = 0;
}

void m1comm_device::device_reset_after_children()
{
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dma->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dlc->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

WRITE_LINE_MEMBER(m1comm_device::dma_hreq_w)
{
	m_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_dma->hack_w(state);
}

uint8_t m1comm_device::dma_mem_r(offs_t offset)
{
	return m_cpu->space(AS_PROGRAM).read_byte(offset);
}

void m1comm_device::dma_mem_w(offs_t offset, uint8_t data)
{
	m_cpu->space(AS_PROGRAM).write_byte(offset, data);
}

WRITE_LINE_MEMBER(m1comm_device::dlc_int7_w)
{
	m_cpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xff); // Z80
}

uint8_t m1comm_device::syn_r()
{
	return m_syn | 0xfc;
}

void m1comm_device::syn_w(uint8_t data)
{
	m_syn = data & 0x03;
}

uint8_t m1comm_device::zfg_r()
{
	return m_zfg | (~m_fg << 7) | 0x7e;
}

void m1comm_device::zfg_w(uint8_t data)
{
	m_zfg = data & 0x01;
}

uint8_t m1comm_device::share_r(offs_t offset)
{
	return m_shared[offset];
}

void m1comm_device::share_w(offs_t offset, uint8_t data)
{
	m_shared[offset] = data;
}

uint8_t m1comm_device::cn_r()
{
	return m_cn | 0xfe;
}

void m1comm_device::cn_w(uint8_t data)
{
	m_cn = data & 0x01;

#ifndef M1COMM_SIMULATION
	if (!m_cn)
	{
		device_reset();
		device_reset_after_children();
	}
	else
	{
		m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_dma->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_dlc->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
#else
	if (!m_cn)
	{
		// reset command
		osd_printf_verbose("M1COMM: board disabled\n");
		m_linkenable = 0x00;
		m_zfg = 0x00;
	}
	else
	{
		// init command
		osd_printf_verbose("M1COMM: board enabled\n");
		m_linkenable = 0x01;
		m_linkid = 0x00;
		m_linkalive = 0x00;
		m_linkcount = 0x00;
		m_linktimer = 0x00e8; // 58 fps * 4s
	}
#endif
}

uint8_t m1comm_device::fg_r()
{
	return m_fg | (~m_zfg << 7) | 0x7e;
}

void m1comm_device::fg_w(uint8_t data)
{
	if (!m_cn)
		return;

	m_fg = data & 0x01;
}

void m1comm_device::check_vint_irq()
{
#ifndef M1COMM_SIMULATION
	if (m_syn & 0x02)
	{
		m_cpu->set_input_line_and_vector(0, HOLD_LINE, 0xef); // Z80
	}
#else
	comm_tick();
#endif
}

#ifdef M1COMM_SIMULATION
void m1comm_device::comm_tick()
{
	if (m_linkenable == 0x01)
	{
		int frameStart = 0x0010;
		int frameOffset = 0x0000;
		int frameSize = 0x01c4;
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
				osd_printf_verbose("M1COMM: listen on %s\n", m_localhost);
				uint64_t filesize; // unused
				osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
			}

			// check tx socket
			if (!m_line_tx)
			{
				osd_printf_verbose("M1COMM: connect to %s\n", m_remotehost);
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
						osd_printf_verbose("M1COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;
						m_zfg = 0x01;

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
						osd_printf_verbose("M1COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;
						m_zfg = 0x01;

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
								frameOffset = 0x06;
								for (int j = 0x00 ; j < 0x0a ; j++)
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
					// master sends additional status bytes
					send_data(0xfd, 0x06, 0x0a, dataSize);

					// send vsync
					m_buffer0[0] = 0xfc;
					m_buffer0[1] = 0x01;
					send_frame(dataSize);
				}
			}

			// clear 05
			m_shared[5] = 0x00;
		}
	}
}

int m1comm_device::read_frame(int dataSize)
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
			osd_printf_verbose("M1COMM: rx connection lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;

			m_shared[0] = 0xff;

			m_line_rx.reset();
			m_line_tx.reset();
		}
	}
	return recv;
}

void m1comm_device::send_data(uint8_t frameType, int frameStart, int frameSize, int dataSize)
{
	m_buffer0[0] = frameType;
	for (int i = 0x00 ; i < frameSize ; i++)
	{
		m_buffer0[1 + i] = m_shared[frameStart + i];
	}
	send_frame(dataSize);
}

void m1comm_device::send_frame(int dataSize){
	if (!m_line_tx)
		return;

	std::error_condition filerr;
	std::uint32_t written;

	filerr = m_line_tx->write(&m_buffer0, 0, dataSize, written);
	if (filerr)
	{
		if (m_linkalive == 0x01)
		{
			osd_printf_verbose("M1COMM: tx connection lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;

			m_shared[0] = 0xff;

			m_line_rx.reset();
			m_line_tx.reset();
		}
	}
}
#endif
