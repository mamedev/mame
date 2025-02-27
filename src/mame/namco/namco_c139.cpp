// license:BSD-3-Clause
// copyright-holders:Angelo Salese, John Bennett, Ariane Fugmann
/***************************************************************************

    Namco C139 - Serial I/F Controller

    (from assault schematics, page 5-18 and 5-19)
    connected to M5M5179P RAM with a 13-bit address bus, and 9 bit data bus
    connected to host cpu with a 14*-bit address bus, and 13 bit data bus
    2 clock inputs - 16M and 12M
    currently there are 4 known modes of operation:

    mode 0x08:
    - ridgera2
    - raverace

    mode 0x09:
    - fourtrax
    - suzuka8h
    - suzuk8h2
    - winrungp
    - winrun91
    - driveyes (center)
    - cybsled
    - cybrcomm
    - acedrive
    - victlap
    - cybrcycc
    - adillor

    mode 0x0c:
    - ridgeracf

    mode 0x0d:
    - finallap
    - finallap2
    - finallap3
    - driveyes (sides)
    - tokyowar
    - aircomb
    - dirtdash
    - alpiner2b (uses 0xfd)

    mode 0x0f (configuration mode)
    - 0x02 used to setup byte/word adressing

    NOTES:
      apparently mode 0x09 and 0x0d modify the received data.
      mode 0x09 does not update *anything* after data got changed. might be automatic.
      mode 0x0d updates the tx offset pointing to the rx buffer (which is not supported right now)

    TODO:
    - hook a real chip and test in detail
    - mode 0x0d shows 1 machine in service mode and attract mode, however most games seem to work "okay" in multiplayer.

***************************************************************************/

#include "emu.h"
#include "namco_c139.h"
#include "emuopts.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
#define REG_0_STATUS 0
#define REG_1_MODE 1
#define REG_2_CONTROL 2
#define REG_3_START 3
#define REG_4_RXSIZE 4
#define REG_5_TXSIZE 5
#define REG_6_RXOFFSET 6
#define REG_7_TXOFFSET 7

// device type definition
DEFINE_DEVICE_TYPE(NAMCO_C139, namco_c139_device, "namco_c139", "Namco C139 Serial")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

void namco_c139_device::data_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(namco_c139_device::ram_r),FUNC(namco_c139_device::ram_w));
}

void namco_c139_device::regs_map(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(namco_c139_device::reg_r), FUNC(namco_c139_device::reg_w));
}


//-------------------------------------------------
//  namco_c139_device - constructor
//-------------------------------------------------

namco_c139_device::namco_c139_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_C139, tag, owner, clock)
	, m_irq_cb(*this)
{
#ifdef C139_SIMULATION
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

	// come up with some magic number for identification
	m_linkid = 0;
	for (int x = 0; x < sizeof(m_remotehost) && m_remotehost[x] != 0; x++)
	{

		m_linkid ^= m_remotehost[x];
	}

	osd_printf_verbose("C139: ID byte = %02d\n", m_linkid);
#endif
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_c139_device::device_start()
{
	m_tick_timer = timer_alloc(FUNC(namco_c139_device::tick_timer_callback), this);
	m_tick_timer->adjust(attotime::never);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_c139_device::device_reset()
{
	m_linktimer = 0x0000;

	m_tick_timer->adjust(attotime::from_hz(600),0,attotime::from_hz(600));
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint16_t namco_c139_device::ram_r(offs_t offset)
{
	return m_ram[offset];
}

void namco_c139_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ram[offset]);

	m_txsize = offset;
}

uint16_t namco_c139_device::reg_r(offs_t offset)
{
	uint16_t result = m_reg[offset];

	if (!machine().side_effects_disabled())
		osd_printf_verbose("C139: reg_r[%02x] = %04x\n", offset, result);

	return result;
}

void namco_c139_device::reg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled())
		osd_printf_verbose("C139: reg_w[%02x] = %04x\n", offset, data);

	m_reg[offset] = data;

	// status reset / irq ack?
	if (offset == REG_0_STATUS && data == 0)
		m_reg[offset] = 4;

	// hack to get alpinr2b working
	if (offset == REG_1_MODE && data >= 0x000f)
		m_reg[REG_1_MODE] &= 0x000f;

	// possibly setup/config mode?
	if (m_reg[REG_1_MODE] == 0x0f && offset == REG_3_START)
	{
		m_reg_f3 = data;
		osd_printf_warning("C139: m_reg_f3 = %02x\n", m_reg_f3);
	}

	// mode 09 tx trigger
	if (offset == REG_1_MODE && data == 0x09 && m_txsize > 0) 
		m_txblock = 0x00;

	// mode 08 & 0c tx trigger
	if (offset == REG_2_CONTROL && data == 0x03) 
		m_txblock = 0x00;

	// mode 0d tx trigger
	if (offset == REG_5_TXSIZE && data > 0) 
		m_txblock = 0x00;

	// hack to get raverace working
	if (m_reg[REG_1_MODE] == 0x08 && offset == REG_2_CONTROL)
	{
		if (data == 1)
			m_txsize = 0;
		if (data == 3)
		{
			m_reg[REG_5_TXSIZE] = m_txsize + 2;
		}
	}
}

void namco_c139_device::sci_de_hack(uint8_t data)
{
	// prepare "filename"
	m_localhost[0] = 0;
	m_remotehost[0] = 0;
	strcat(m_localhost, "socket.127.0.0.1:");
	strcat(m_remotehost, "socket.127.0.0.1:");

	switch (data)
	{
		case 0:
			strcat(m_localhost, "15112");
			strcat(m_remotehost, "15113");
			break;
		case 1:
			strcat(m_localhost, "15113");
			strcat(m_remotehost, "15114");
			break;
		case 2:
			strcat(m_localhost, "15114");
			strcat(m_remotehost, "15112");
			break;
	}

	// come up with some magic number for identification
	m_linkid = 0;
	for (int x = 0; x < sizeof(m_remotehost) && m_remotehost[x] != 0; x++)
	{

		m_linkid ^= m_remotehost[x];
	}

	osd_printf_verbose("C139: ID byte = %02d\n", m_linkid);
}

#ifdef C139_SIMULATION
TIMER_CALLBACK_MEMBER(namco_c139_device::tick_timer_callback)
{
	comm_tick();
}

void namco_c139_device::comm_tick()
{
	if (m_linktimer > 0x0000)
		m_linktimer--;

	if (m_linktimer == 0x0000)
	{
		std::error_condition filerr;

		// check rx socket
		if (!m_line_rx)
		{
			osd_printf_verbose("C139: listen on %d\n", m_localhost);
			uint64_t filesize; // unused
			filerr = osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
			if (filerr.value() != 0)
			{
				osd_printf_verbose("C139: rx connection failed\n");
				m_line_tx.reset();
				m_linktimer = 0x0100;
			}
		}

		// check tx socket
		if (!m_line_tx)
		{
			osd_printf_verbose("C139: connect to %s\n", m_remotehost);
			uint64_t filesize; // unused
			filerr = osd_file::open(m_remotehost, 0, m_line_tx, filesize);
			if (filerr.value() != 0)
			{
				osd_printf_verbose("C139: tx connection failed\n");
				m_line_tx.reset();
				m_linktimer = 0x0100;
			}
		}

		// if both sockets are connected
		if (m_line_rx && m_line_tx)
		{
			// link established
			int dataSize = 0x200;
			switch (m_reg[REG_1_MODE])
			{
				case 0x08:
					// ridgera2, raverace
					// 0b1000
					// reg2 - 1 > write mem > 3 > 1 > write mem > 3 etc.
					// txcount NOT cleared on send
					read_data(dataSize);
					if (m_reg[REG_2_CONTROL] == 0x03 && m_reg[REG_5_TXSIZE] > 0x00)
						send_data(dataSize);
					break;

				case 0x09:
					// suzuka8h, acedrive, winrungp, cybrcycc, driveyes (center)
					// 0b1001 - auto-send via sync bit (and auto offset)
					read_data(dataSize);
					send_data(dataSize);
					break;

				case 0x0c:
					// ridgeracf
					// 0b1100 - send by register / txwords
					// txcount IS cleared on send
					read_data(dataSize);
					if (m_reg[REG_2_CONTROL] == 0x03 && m_reg[REG_3_START] == 0x00)
						send_data(dataSize);
					break;

				case 0x0d:
					// final lap, driveyes (left & right)
					// 0b1101 - auto-send via register?
					read_data(dataSize);
					if (m_reg[REG_3_START] == 0x00 && m_reg[REG_5_TXSIZE] > 0x00)
						send_data(dataSize);
					break;

				case 0x0f:
					// init / reset
					return;
			}
		}
	}
}

void namco_c139_device::read_data(int dataSize)
{
	int rxSize = 0;
	int rxOffset = 0;
	int bufOffset = 0;
	int recv = 0;

	if (m_reg[REG_0_STATUS] != 0x06)
	{
		// try to read a message
		recv = read_frame(dataSize);
		if (recv > 0)
		{
			// (hack) update linkcount for mode 09
			if (m_buffer0[0] == m_linkid && m_reg[REG_1_MODE] == 0x09 && (m_reg_f3 & 0x2) == 2)
				m_buffer0[0x07] = m_buffer0[0x1ff];

			// save message to "rx buffer"
			rxSize = m_buffer0[2] << 8 | m_buffer0[1];
			rxOffset = m_reg[REG_6_RXOFFSET]; // rx offset in words
			osd_printf_verbose("C139: rxOffset = %04x, rxSize == %02x\n", rxOffset, rxSize);
			bufOffset = 3;
			for (int j = 0x00 ; j < rxSize ; j++)
			{
				m_ram[0x1000 + (rxOffset & 0x0fff)] = m_buffer0[bufOffset + 1] << 8 | m_buffer0[bufOffset];
				rxOffset++;
				bufOffset += 2;
			}

			// relay messages
			if (m_buffer0[0] != m_linkid)
			{
				if (m_reg[REG_1_MODE] == 0x09 && (m_reg_f3 & 0x2) == 2)
					m_buffer0[0x1ff]++;

				send_frame(dataSize);
			}
			else
			{
				if (m_reg[REG_1_MODE] == 0x09)
					m_reg[REG_5_TXSIZE] = 0x00; //m_txblock = 0x00;
			}

			// update regs
			m_reg[REG_0_STATUS] = 0x06;
			if (m_reg[REG_1_MODE] != 0x0d)
				m_reg[REG_4_RXSIZE] += rxSize;
			else
				m_reg[REG_4_RXSIZE] -= rxSize;
			m_reg[REG_6_RXOFFSET] += rxSize;

			// prevent overflow
			m_reg[REG_4_RXSIZE] &= 0x0fff;
			m_reg[REG_6_RXOFFSET] &= 0x0fff;

			// fire interrupt
			m_irq_cb(ASSERT_LINE);
		}
	}
	else
	{
		// fire interrupt (again)
		m_irq_cb(ASSERT_LINE);
	}
}

int namco_c139_device::read_frame(int dataSize)
{
	if (!m_line_rx)
		return 0;

	if (m_reg[REG_0_STATUS] == 0x06)
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
		osd_printf_verbose("C139: rx connection error\n");
		m_line_rx.reset();
		m_linktimer = 0x0200;
		m_txblock = 0x00;
	}
	return recv;
}

void namco_c139_device::send_data(int dataSize)
{
	int txSize = m_reg[REG_5_TXSIZE];
	int txOffset = m_reg[REG_7_TXOFFSET]; // tx offset in words
	int bufOffset = 3;
	int txMask = 0x0fff;

	if (m_txblock == 0x01)
		return;

	if (m_reg[REG_0_STATUS] == 0x06)
		return;

	if ((m_reg_f3 & 0x02) == 0x02)
		txOffset >>= 1; // tx offset in bytes

	if (m_reg[REG_1_MODE] == 0x0d)
		txMask = 0x1fff; // allow txPointer to rxBuffer

	if (m_reg[REG_1_MODE] == 0x09)
	{
		txSize = find_sync_bit(txOffset, txMask);
		if (txSize == 0x01) {
			m_reg[REG_7_TXOFFSET] += 0x100;
			txSize = 0;
		}
	}

	osd_printf_verbose("C139: txOffset = %04x, txSize == %02x\n", txOffset, txSize);
	if (txSize == 0)
		return;

	m_buffer0[0] = m_linkid;
	m_buffer0[1] = txSize & 0xff;
	m_buffer0[2] = (txSize & 0xff00) >> 8;
	m_buffer0[0x1ff] = 1;

	for (int j = 0x00 ; j < txSize ; j++)
	{
		m_buffer0[bufOffset] = m_ram[txOffset & txMask] & 0xff;
		m_buffer0[bufOffset + 1] = 0;

		txOffset++;
		bufOffset += 2;
	}

	// set bit-8 on last byte
	m_buffer0[bufOffset -1] |= 0x01;

	// based on mode, reset tx counter
	switch (m_reg[REG_1_MODE])
	{
		case 0x08:
		case 0x09:
			// do nothing
			m_txblock = 0x01;
			break;
		case 0x0c:
		case 0x0d:
			m_reg[REG_5_TXSIZE] = 0;
			m_txblock = 0x01;
			break;
	}

	m_txsize = 0;
	send_frame(dataSize);
}

void namco_c139_device::send_frame(int dataSize)
{
	if (!m_line_tx)
		return;

	std::uint32_t written;
	std::error_condition filerr = m_line_tx->write(&m_buffer0, 0, dataSize, written);
	if (filerr)
	{
		osd_printf_verbose("C139: tx connection error\n");
		m_line_tx.reset();
		m_linktimer = 0x0200;
		m_txblock = 0x00;
	}
}

int namco_c139_device::find_sync_bit(int txOffset, int txMask)
{
	// cybrcycc
	if ((m_ram[(txOffset) & txMask] & 0x01ff) == 0x1ff)
		return 0;

	// hack to find sync bit in data area
	for (int i = 0; i < 0x08; i++)
	{
		int subOffset = i * 0x80;
		for (int j = 0; j < 0x100; j++)
		{
			if (m_ram[(txOffset + subOffset + j) & txMask] & 0x0100)
			{
				if (i > 0)
					m_reg[REG_7_TXOFFSET] += subOffset * 2;
				return j + 1;
			}
		}
	}
	return 0;
}
#endif