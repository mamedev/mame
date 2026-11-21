// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
#ifndef MAME_NOKIA_NGAGE_ADSP_H
#define MAME_NOKIA_NGAGE_ADSP_H

#pragma once

#include <span>

class ngage_adsp_device : public device_t
{
public:
	ngage_adsp_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	void power_w(bool enabled);
	void tx_w(u8 data);
	bool rx_pending() const { return m_output_count != 0; }
	u8 rx_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum : u8 { WAIT_LINK, READY, WAIT_ACK };

	void reset_link();
	void handle_frame();
	void send_frame(u8 resource, std::span<u8 const> payload);
	void send_startup(bool reset_only);

	static constexpr unsigned MAX_PAYLOAD = 247;
	static constexpr unsigned MAX_FRAME = 10 + MAX_PAYLOAD + 1;

	bool m_enabled = false;
	u8 m_state = WAIT_LINK;
	u8 m_frame[MAX_FRAME]{};
	u16 m_frame_size = 0;
	u8 m_output[64]{};
	u8 m_output_head = 0;
	u8 m_output_count = 0;
	u8 m_device_sequence = 0;
	u8 m_request_sequence = 0;
	u8 m_request[MAX_PAYLOAD]{};
	u8 m_request_size = 0;
	u8 m_reply[12]{};
	u8 m_reply_size = 0;
};

DECLARE_DEVICE_TYPE(NGAGE_ADSP, ngage_adsp_device)

#endif // MAME_NOKIA_NGAGE_ADSP_H
