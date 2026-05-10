// license:BSD-3-Clause
// copyright-holders:Alex "trap15" Marshall

/****************************************************************************

    Gunbuster Link "Controller"

    The link system utilizes a 256 word share RAM, logically split into
    two banks of 128. For some reason the memory test clears 8192 words,
    might be a leftover from Double Axle which uses 8192 word share RAM.

    The last two words of each bank are used for control words,
    the former ($7E) being a 'lock' control for the given network node,
    and the latter ($7F) being an 'ID' value for the node.
    It doesn't seem to actually use any hardware-enforced locking,
    and software never seems to read back the lock values,
    so in theory race conditions could be possible. This seems to
    only avoid issues because each node strictly only writes "real"
    data into its own bank. Since locks aren't read back, clobbering
    each other's lock words has no impact.

    At startup, a node places $FF00|selfID into the 'ID' word, checks
    if the write succeeded (check for RAM presence), then checks for
    the presence of the opposite ID value in the opposite bank. If this
    fails, "COMMUNICATION ERROR" occurs.

    When a node wants to read a bank, it will write $0001 to the lock
    control of the _opposite_ bank, then read the data, then clear the lock.
    When a node wants to write a bank, it will write $0002 to the lock
    control of _that_ bank, then write the data, then clear the lock.
    In theory this should mean a node only ever writes to its own lock word,
    but in practice the code reads back both its bank and the other's bank,
    so it has to write both lock words.

    Every frame, the nodes synchronize with each other over their share
    RAM banks, only writing to its own. Every frame should increase the
    longword at $7C~$7D in its bank, and if that longword doesn't increase
    for 16 consecutive frames, "COMMUNICATION CUT" occurs.

 ***************************************************************************/

#include "emu.h"
#include "gunbustr_l.h"
#include "emuopts.h"

// configurable logging
#define LOG_LINKPROC	(1U << 1)
#define LOG_LINKTX		(1U << 2)
#define LOG_LINKRX		(1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_LINKPROC)

#include "logmacro.h"

//**************************************************************************
//  DEVICE SETUP
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(GUNBUSTR_LINK, gunbustr_link_device, "gunbustr_l", "Gunbuster Link Controller")

#define BASEADDR_NODE(x) ((x) * LINKWORD_COUNT*2)

void gunbustr_link_device::map(address_map &map)
{
	map(BASEADDR_NODE(0)+              0,BASEADDR_NODE(0)+LINKWORD_CTRL*2-1)
		.rw(FUNC(gunbustr_link_device::ram0_r), FUNC(gunbustr_link_device::ram0_w));
	map(BASEADDR_NODE(0)+LINKWORD_CTRL*2,BASEADDR_NODE(0)+LINKWORD_COUNT*2-1)
		.rw(FUNC(gunbustr_link_device::ctrl0_r), FUNC(gunbustr_link_device::ctrl0_w));

	map(BASEADDR_NODE(1)+              0,BASEADDR_NODE(1)+LINKWORD_CTRL*2-1)
		.rw(FUNC(gunbustr_link_device::ram1_r), FUNC(gunbustr_link_device::ram1_w));
	map(BASEADDR_NODE(1)+LINKWORD_CTRL*2,BASEADDR_NODE(1)+LINKWORD_COUNT*2-1)
		.rw(FUNC(gunbustr_link_device::ctrl1_r), FUNC(gunbustr_link_device::ctrl1_w));
}

//-------------------------------------------------
//  gunbustr_link_device - constructor
//-------------------------------------------------

gunbustr_link_device::gunbustr_link_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock ) :
	device_t(mconfig, GUNBUSTR_LINK, tag, owner, clock),
	device_execute_interface(mconfig, *this),
	m_icount(0)
{
	snprintf(m_localhost,sizeof(m_localhost), "socket.%s:%s",
			mconfig.options().comm_localhost(), mconfig.options().comm_localport());
	m_localhost[sizeof(m_localhost)-1] = '\0';
	snprintf(m_remotehost,sizeof(m_remotehost), "socket.%s:%s",
			mconfig.options().comm_remotehost(), mconfig.options().comm_remoteport());
	m_remotehost[sizeof(m_remotehost)-1] = '\0';
}


//-------------------------------------------------
//  device_start - device-specific startup
//------------------------------------------------

void gunbustr_link_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	save_item(NAME(m_ram));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gunbustr_link_device::device_reset()
{
}


//-------------------------------------------------
//  execute_run - device update
//-------------------------------------------------

void gunbustr_link_device::execute_run()
{
	while (m_icount > 0)
	{
		phy_rx_check();
		phy_tx_check();
		// no reason to check it multiple times in a row
		m_icount = 0;
	}
}


//**************************************************************************
//  "PHYSICAL" NETWORK HANDLING
//**************************************************************************

bool gunbustr_link_device::phy_rx_check(void)
{
	if (m_line_rx) return true;

	LOGMASKED(LOG_LINKPROC, "listen on %s\n", m_localhost);
	uint64_t filesize; // unused
	osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);

	return !!m_line_rx;
}

bool gunbustr_link_device::phy_tx_check(void)
{
	if (m_line_tx) return true;

	// TODO: How to make this non-blocking?
	LOGMASKED(LOG_LINKPROC, "connect to %s\n", m_remotehost);
	uint64_t filesize; // unused
	osd_file::open(m_remotehost, 0, m_line_tx, filesize);

	return !!m_line_tx;
}

bool gunbustr_link_device::phy_recv_raw(void *buf, size_t bytes)
{
	if (!m_line_rx) return false;

	uint32_t amt = 0;
	std::error_condition err = m_line_rx->read(buf, 0, bytes, amt);
	if (err)
	{
		LOGMASKED(LOG_LINKRX, "RX ERROR [%s]\n", err.message().c_str());
		return false;
	}
	if (amt != bytes)
	{
		LOGMASKED(LOG_LINKRX, "RX NOT ENOUGH %d => %d\n", bytes, amt);
		return false;
	}
	return true;
}

bool gunbustr_link_device::phy_send_raw(const void *buf, size_t bytes)
{
	if(!m_line_tx) return false;

	uint32_t amt = 0;
	std::error_condition err = m_line_tx->write(buf, 0, bytes, amt);
	if (err)
	{
		LOGMASKED(LOG_LINKPROC, "TX ERROR [%s]\n", err.message().c_str());
		// TX error almost certainly requires a reconnect, so disconnect now.
		m_line_tx = nullptr;
		return false;
	}
	if (amt != bytes)
	{
		LOGMASKED(LOG_LINKTX, "TX NOT ENOUGH %d => %d\n", bytes, amt);
		return false;
	}
	return true;
}

// Ensure network format is the same for all machines
#ifdef BIGENDIAN
# define BE32(x) (x)
#else
# define BE32(x) (	(((x) >> 24) & 0x000000FF) | \
					(((x) >>  8) & 0x0000FF00) | \
					(((x) <<  8) & 0x00FF0000) | \
					(((x) << 24) & 0xFF000000U))
#endif

bool gunbustr_link_device::phy_recv_cmd(linkcmd *cmd)
{
	uint32_t cmdpkt[3];
	if (!phy_recv_raw(cmdpkt, sizeof(cmdpkt)))
	{
		cmd->type = LINKCMD_NOP;
		return false;
	}
	cmd->type	= BE32(cmdpkt[0]);
	cmd->nodeid = BE32(cmdpkt[1]);
	cmd->data	= BE32(cmdpkt[2]);
	return true;
}

bool gunbustr_link_device::phy_send_cmd(const linkcmd *cmd)
{
	uint32_t cmdpkt[3];
	cmdpkt[0] = BE32(cmd->type);
	cmdpkt[1] = BE32(cmd->nodeid);
	cmdpkt[2] = BE32(cmd->data);
	return phy_send_raw(cmdpkt, sizeof(cmdpkt));
}

bool gunbustr_link_device::phy_recv_ram(uint8_t nodeid)
{
	return phy_recv_raw(m_ram[nodeid], LINKWORD_PAYLOAD_COUNT*2);
}

bool gunbustr_link_device::phy_send_ram(uint8_t nodeid)
{
	return phy_send_raw(m_ram[nodeid], LINKWORD_PAYLOAD_COUNT*2);
}

//**************************************************************************
//  LINK PROTOCOL HANDLING
//**************************************************************************

void gunbustr_link_device::link_update(void)
{
	linkcmd cmd;
	while (phy_recv_cmd(&cmd))
	{
		if(cmd.nodeid > 2) continue;

		switch (cmd.type)
		{
			default:
				break;
			case LINKCMD_RECV_DATA:
				LOGMASKED(LOG_LINKPROC, "[proc] RecvData %d\n", cmd.nodeid);
				phy_send_ram(cmd.nodeid);
				break;
			case LINKCMD_SEND_DATA:
				LOGMASKED(LOG_LINKPROC, "[proc] SendData %d\n", cmd.nodeid);
				phy_recv_ram(cmd.nodeid);
				break;
			case LINKCMD_SEND_CTRL:
				LOGMASKED(LOG_LINKPROC, "[proc] RecvCtrl %d\n", cmd.nodeid);
				m_ram[cmd.nodeid][LINKWORD_LOCK] = (uint16_t)(cmd.data >>16);
				m_ram[cmd.nodeid][LINKWORD_ID] = (uint16_t)(cmd.data >> 0);
				break;
			case LINKCMD_SEND_LOCK:
				LOGMASKED(LOG_LINKPROC, "[proc] SendLock %d 0x%X\n", cmd.nodeid, cmd.data);
				m_ram[cmd.nodeid][LINKWORD_LOCK] = cmd.data;
				break;
			case LINKCMD_SEND_NODE_ID:
				LOGMASKED(LOG_LINKPROC, "[proc] SendNodeId %d 0x%X\n", cmd.nodeid, cmd.data);
				m_ram[cmd.nodeid][LINKWORD_ID] = cmd.data;
				break;
		}
	}
}

void gunbustr_link_device::link_recv_data(uint8_t nodeid)
{
	linkcmd cmd =
	{
		.type = LINKCMD_RECV_DATA,
		.nodeid = nodeid,
	};
	phy_send_cmd(&cmd);
	phy_recv_ram(nodeid);
}

void gunbustr_link_device::link_send_data(uint8_t nodeid)
{
	linkcmd cmd =
	{
		.type = LINKCMD_SEND_DATA,
		.nodeid = nodeid,
	};
	phy_send_cmd(&cmd);
	phy_send_ram(nodeid);
}

void gunbustr_link_device::link_send_lock(uint8_t nodeid)
{
	linkcmd cmd =
	{
		.type = LINKCMD_SEND_LOCK,
		.nodeid = nodeid,
		.data = m_ram[nodeid][LINKWORD_LOCK],
	};
	phy_send_cmd(&cmd);
}

void gunbustr_link_device::link_send_id(uint8_t nodeid)
{
	linkcmd cmd =
	{
		.type = LINKCMD_SEND_NODE_ID,
		.nodeid = nodeid,
		.data = m_ram[nodeid][LINKWORD_ID],
	};
	phy_send_cmd(&cmd);
}

void gunbustr_link_device::link_send_ctrl(uint8_t nodeid)
{
	linkcmd cmd =
	{
		.type = LINKCMD_SEND_CTRL,
		.nodeid = nodeid,
		.data = ((uint32_t)m_ram[nodeid][LINKWORD_LOCK] << 16) |
				m_ram[nodeid][LINKWORD_ID],
	};
	phy_send_cmd(&cmd);
}

//**************************************************************************
//  MEMORY INTERFACE
//**************************************************************************

uint16_t gunbustr_link_device::ram_r(uint8_t nodeid, offs_t offset)
{
	link_update();
	return m_ram[nodeid][offset];
}

void gunbustr_link_device::ram_w(uint8_t nodeid, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	link_update();
	COMBINE_DATA(&m_ram[nodeid][offset]);
}

uint16_t gunbustr_link_device::ctrl_r(uint8_t nodeid, offs_t offset)
{
	link_update();
	link_send_ctrl(nodeid); // why is this correct?...
	return m_ram[nodeid][LINKWORD_CTRL + offset];
}

void gunbustr_link_device::ctrl_w(uint8_t nodeid, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	link_update();
	uint16_t prev = m_ram[nodeid][LINKWORD_CTRL + offset];
	uint16_t now = COMBINE_DATA(&m_ram[nodeid][LINKWORD_CTRL + offset]);

	switch (LINKWORD_CTRL + offset)
	{
		case LINKWORD_LOCK:
			link_send_lock(nodeid);

			if ((prev & ~now) & 2) // Write release
			{
				link_send_data(nodeid);
			}

			if ((now & ~prev) & 1) // Read assert
			{
				// sync opposite node
				link_recv_data(nodeid ^ 1);
			}
			break;
		case LINKWORD_ID:
			link_send_id(nodeid);
			break;
	}

	link_update();
}

uint16_t gunbustr_link_device::ctrl0_r(offs_t offset)
{
	return ctrl_r(0, offset);
}

void gunbustr_link_device::ctrl0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	ctrl_w(0, offset, data, mem_mask);
}

uint16_t gunbustr_link_device::ctrl1_r(offs_t offset)
{
	return ctrl_r(1, offset);
}

void gunbustr_link_device::ctrl1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	ctrl_w(1, offset, data, mem_mask);
}

uint16_t gunbustr_link_device::ram0_r(offs_t offset)
{
	return ram_r(0, offset);
}

void gunbustr_link_device::ram0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	ram_w(0, offset, data, mem_mask);
}

uint16_t gunbustr_link_device::ram1_r(offs_t offset)
{
	return ram_r(1, offset);
}

void gunbustr_link_device::ram1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	ram_w(1, offset, data, mem_mask);
}
