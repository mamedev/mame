// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_RS232_XVD701_H
#define MAME_BUS_RS232_XVD701_H

#include "rs232.h"
#include "diserial.h"

class jvc_xvd701_device : public device_t,
		public device_serial_interface,
		public device_rs232_port_interface
{
public:
	jvc_xvd701_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_txd(int state) override { device_serial_interface::rx_w(state); }
protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	enum jvc_xvd701_media_type : uint32_t
	{
		JVC_MEDIA_VCD = 0,
		JVC_MEDIA_DVD = 1,
	};

	enum jvc_xvd701_playback_status : uint32_t
	{
		STATUS_STOP = 0,
		STATUS_PLAYING = 1,
		STATUS_PAUSE = 2,
	};

	TIMER_CALLBACK_MEMBER(send_response);
	unsigned char sum(unsigned char *buffer, int length);
	void create_packet(unsigned char status, const unsigned char response[6]);

	bool seek_chapter(int chapter);

	jvc_xvd701_media_type m_media_type;

	unsigned char m_command[11];
	unsigned char m_response[11];
	int m_response_index;
	emu_timer *m_timer_response;

	jvc_xvd701_playback_status m_playback_status;

	unsigned char m_jlip_id;
	bool m_is_powered;

	int m_chapter;

	enum : unsigned char {
		STATUS_UNKNOWN_COMMAND = 1,
		STATUS_OK = 3,
		STATUS_ERROR = 5,
	};
	const unsigned char NO_RESPONSE[6] = { 0 };
};

DECLARE_DEVICE_TYPE(JVC_XVD701, jvc_xvd701_device)

#endif // MAME_BUS_RS232_XVD701_H
