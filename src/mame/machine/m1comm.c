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
      Z80      - Zilog Z0840004PSC Z80 CPU, running at 4.000MHz (DIP40)
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

#include "machine/m1comm.h"

#define Z80_TAG     "m1commcpu"

//#define __M1COMM_VERBOSE__

/*************************************
 *  M1COMM Memory Map
 *************************************/
static ADDRESS_MAP_START( m1comm_mem, AS_PROGRAM, 8, m1comm_device )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xC000, 0xffff) AM_READWRITE(share_r, share_w)
ADDRESS_MAP_END

/*************************************
 *  M1COMM I/O Map
 *************************************/
static ADDRESS_MAP_START( m1comm_io, AS_IO, 8, m1comm_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x1F) AM_READWRITE(dlc_reg_r, dlc_reg_w)
	AM_RANGE(0x20, 0x2F) AM_READWRITE(dma_reg_r, dma_reg_w)
	AM_RANGE(0x40, 0x40) AM_READWRITE(syn_r, syn_w)
	AM_RANGE(0x60, 0x60) AM_READWRITE(zfg_r, zfg_w)
	AM_RANGE(0xFFFF, 0xFFFF) AM_RAM
ADDRESS_MAP_END

MACHINE_CONFIG_FRAGMENT( m1comm )
	MCFG_CPU_ADD(Z80_TAG, Z80, 4000000) /* 32 MHz / 8 */
	MCFG_CPU_PROGRAM_MAP(m1comm_mem)
	MCFG_CPU_IO_MAP(m1comm_io)
MACHINE_CONFIG_END

ROM_START( m1comm )
	ROM_REGION( 0x20000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "epr-15112.17", 0x0000, 0x20000, CRC(4950E771) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type M1COMM = &device_creator<m1comm_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor m1comm_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( m1comm );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *m1comm_device::device_rom_region() const
{
	return ROM_NAME( m1comm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m1comm_device - constructor
//-------------------------------------------------

m1comm_device::m1comm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, M1COMM, "MODEL-1 COMMUNICATION BD", tag, owner, clock, "m1comm", __FILE__),
	m_commcpu(*this, Z80_TAG),
	m_line_rx(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE ),
	m_line_tx(OPEN_FLAG_READ)
{
	// prepare localhost "filename"
	strcat(m_localhost, "socket.");
	strcat(m_localhost, mconfig.options().comm_localhost());
	strcat(m_localhost, ":");
	strcat(m_localhost, mconfig.options().comm_localport());

	// prepare remotehost "filename"
	strcat(m_remotehost, "socket.");
	strcat(m_remotehost, mconfig.options().comm_remotehost());
	strcat(m_remotehost, ":");
	strcat(m_remotehost, mconfig.options().comm_remoteport());
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

	m_commcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

READ8_MEMBER(m1comm_device::dlc_reg_r)
{
	// dirty hack to keep Z80 in RESET state
	if (!m_cn)
	{
		device_reset();
		return 0xFF;
	}
	// dirty hack to keep Z80 in RESET state

	UINT8 result = m_dlc_reg[offset];
#ifdef __M1COMM_VERBOSE__
	printf("m1comm-dlc_reg_r: read register %02x for value %02x\n", offset, result);
#endif
	return result;
}

WRITE8_MEMBER(m1comm_device::dlc_reg_w)
{
	m_dlc_reg[offset] = data;
#ifdef __M1COMM_VERBOSE__
	printf("m1comm-dlc_reg_w: write register %02x for value %02x\n", offset, data);
#endif
}

READ8_MEMBER(m1comm_device::dma_reg_r)
{
	UINT8 result = m_dma_reg[offset];
#ifdef __M1COMM_VERBOSE__
	printf("m1comm-dma_reg_r: read register %02x for value %02x\n", offset, result);
#endif
	return result;
}

WRITE8_MEMBER(m1comm_device::dma_reg_w)
{
#ifdef __M1COMM_VERBOSE__
	printf("m1comm-dma_reg_w: %02x %02x\n", offset, data);
#endif
	m_dma_reg[offset] = data;
}

READ8_MEMBER(m1comm_device::syn_r)
{
	UINT8 result = m_syn | 0xFC;
#ifdef __M1COMM_VERBOSE__
	printf("m1comm-syn_r: read register %02x for value %02x\n", offset, result);
#endif
	return result;
}

WRITE8_MEMBER(m1comm_device::syn_w)
{
	m_syn = data & 0x03;

#ifdef __M1COMM_VERBOSE__
	switch (data & 0x02)
	{
		case 0x00:
			printf("m1comm-syn_w: VINT disabled\n");
			break;

		case 0x02:
			printf("m1comm-syn_w: VINT enabled\n");
			break;

		default:
			printf("m1comm-syn_w: %02x\n", data);
			break;
	}
#endif
}

READ8_MEMBER(m1comm_device::zfg_r)
{
	UINT8 result = m_zfg | 0xFE;
#ifdef __M1COMM_VERBOSE__
	printf("m1comm-zfg_r: read register %02x for value %02x\n", offset, result);
#endif
	return result;
}

WRITE8_MEMBER(m1comm_device::zfg_w)
{
#ifdef __M1COMM_VERBOSE__
	printf("m1comm-zfg_w: %02x\n", data);
#endif
	m_zfg = data & 0x01;
}

READ8_MEMBER(m1comm_device::share_r)
{
	return m_shared[offset];
}

WRITE8_MEMBER(m1comm_device::share_w)
{
	m_shared[offset] = data;
}

READ8_MEMBER(m1comm_device::cn_r)
{
	return m_cn | 0xFE;
}

WRITE8_MEMBER(m1comm_device::cn_w)
{
	m_cn = data & 0x01;

#ifndef __M1COMM_SIMULATION__
	if (!m_cn)
		device_reset();
	else
		m_commcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
#else
	if (!m_cn)
	{
		// reset command
		printf("M1COMM: board disabled\n");
		m_linkenable = 0x00;
	}
	else
	{
		// init command
		printf("M1COMM: board enabled\n");
		m_linkenable = 0x01;
		m_linkid = 0x00;
		m_linkalive = 0x00;
		m_linkcount = 0x00;
		m_linktimer = 0x00E8; // 58 fps * 4s
	}
#endif
}

READ8_MEMBER(m1comm_device::fg_r)
{
	return m_fg | (~m_zfg << 7) | 0x7E;
}

WRITE8_MEMBER(m1comm_device::fg_w)
{
	if (!m_cn)
		return;

	m_fg = data & 0x01;
}

void m1comm_device::check_vint_irq()
{
#ifndef __M1COMM_SIMULATION__
	if (m_syn & 0x02)
	{
		m_commcpu->set_input_line_and_vector(0, HOLD_LINE, 0xEF);
#ifdef __M1COMM_VERBOSE__
		printf("m1comm-INT5\n");
#endif
	}
#else
	comm_tick();
#endif
}

#ifdef __M1COMM_SIMULATION__
void m1comm_device::comm_tick(){
	if (m_linkenable == 0x01)
	{
		int frameStart = 0x0010;
		int frameOffset = 0x0000;
		int frameSize = 0x01C4;
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
			// check rx socket
			if (!m_line_rx.is_open())
			{
				printf("M1COMM: listen on %s\n", m_localhost);
				m_line_rx.open(m_localhost);
			}

			// check tx socket
			if (!m_line_tx.is_open())
			{
				printf("M1COMM: connect to %s\n", m_remotehost);
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
							printf("M1COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
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
						printf("M1COMM: droped a message...\n");
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
						printf("M1COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
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
							for (int j = 0x06 ; j < 0x10 ; j++)
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
					printf("M1COMM: droped a message...\n");
				}
				recv = m_line_rx.read(m_buffer, dataSize);
			}

			// update "ring buffer" if link established
			// live relay does not send data
			if (m_linkid != 0x00 && m_shared[5] == 0x01)
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
					for (int j = 0x06 ; j < 0x10 ; j++)
					{
						m_buffer[1 + j] = m_shared[frameStart + j];
					}
					// push message to other nodes
					m_line_tx.write(m_buffer, dataSize);
				}
			}
			// clear 05
			m_shared[5] = 0x00;
		}
	}
#endif
}
