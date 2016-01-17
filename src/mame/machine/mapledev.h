// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef _MAPLEDEV_H_
#define _MAPLEDEV_H_

#define MCFG_MAPLE_DEVICE_ADD(_tag, _type, _clock, _host_tag, _host_port) \
	MCFG_DEVICE_ADD(_tag, _type, _clock) \
	maple_device::static_set_host(*device, _host_tag, _host_port);

class maple_device : public device_t
{
public:
	maple_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	static void static_set_host(device_t &device, const char *_host_tag, int _host_port);
	virtual void maple_w(const UINT32 *data, UINT32 in_size) = 0;
	void maple_r(UINT32 *data, UINT32 &out_size, bool &partial);
	virtual void maple_reset();

protected:
	enum { TIMER_ID = 1000 };

	UINT32 reply_size;
	bool reply_partial;
	UINT32 reply_buffer[256];

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void reply_ready();
	void reply_ready_with_delay();

	// Copy a string and complete it with spaces up to size len
	void copy_with_spaces(UINT8 *dest, const char *source, int len);

	// Setup the first UINT32 of reply with the type, source and length.
	// Also setup reply_size and clear reply_partial
	void reply_start(UINT8 code, UINT8 source, UINT8 size);

	// Configuration
	class maple_dc_device *host;
	const char *host_tag;
	int host_port;

private:
	emu_timer *timer;
};

#endif // _MAPLEDEV_H_
