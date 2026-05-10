// license:BSD-3-Clause
// copyright-holders:Alex "trap15" Marshall

/****************************************************************************

    Gunbuster Link "Controller"

 ***************************************************************************/

#ifndef MAME_TAITO_GUNBUSTR_L_H
#define MAME_TAITO_GUNBUSTR_L_H

#pragma once

class gunbustr_link_device : public device_t,
					   public device_execute_interface
{
public:
	// construction/destruction
	gunbustr_link_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	int m_icount;

private:
	enum linkcmd_type
	{
		LINKCMD_NOP = 0,
		LINKCMD_SEND_NODE_ID,
		LINKCMD_SEND_LOCK,
		LINKCMD_SEND_CTRL,
		LINKCMD_RECV_DATA,
		LINKCMD_SEND_DATA,
	};

	enum linkram_ofs
	{
		LINKWORD_CTRL = 0x7E,
		LINKWORD_LOCK = 0x7E,
		LINKWORD_ID = 0x7F,
		LINKWORD_COUNT = 0x80,
		LINKWORD_PAYLOAD_COUNT = 0x7E,
	};

	struct linkcmd
	{
		uint32_t type;
		uint32_t nodeid;
		uint32_t data;
	};

	// internal state
	uint16_t m_ram[2][LINKWORD_COUNT];

	osd_file::ptr m_line_rx;
	osd_file::ptr m_line_tx;
	char m_localhost[256];
	char m_remotehost[256];

	// 'Physical' network handling
	bool phy_rx_check(void);
	bool phy_tx_check(void);
	bool phy_recv_raw(void *buf, size_t bytes);
	bool phy_send_raw(const void *buf, size_t bytes);
	bool phy_recv_cmd(linkcmd *cmd);
	bool phy_send_cmd(const linkcmd *cmd);
	bool phy_recv_ram(uint8_t nodeid);
	bool phy_send_ram(uint8_t nodeid);

	// Link 'protocol' handling
	void link_update(void);
	void link_recv_data(uint8_t nodeid);
	void link_send_data(uint8_t nodeid);
	void link_send_lock(uint8_t nodeid);
	void link_send_id(uint8_t nodeid);
	void link_send_ctrl(uint8_t nodeid);

	// Memory handlers
	uint16_t ctrl_r(uint8_t nodeid, offs_t offset);
	void ctrl_w(uint8_t nodeid, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t ctrl0_r(offs_t offset);
	void ctrl0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~uint16_t(0));
	uint16_t ctrl1_r(offs_t offset);
	void ctrl1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~uint16_t(0));

	uint16_t ram_r(uint8_t nodeid, offs_t offset);
	void ram_w(uint8_t nodeid, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t ram0_r(offs_t offset);
	void ram0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~uint16_t(0));
	uint16_t ram1_r(offs_t offset);
	void ram1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~uint16_t(0));
};

// device type definition
DECLARE_DEVICE_TYPE(GUNBUSTR_LINK, gunbustr_link_device)

#endif // MAME_TAITO_GUNBUSTR_L_H
