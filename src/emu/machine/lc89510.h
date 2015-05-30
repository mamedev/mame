// license:BSD-3-Clause
// copyright-holders:David Haywood



class lc89510_device : public device_t
{
public:
	lc89510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);



protected:
	virtual void device_start();
	virtual void device_reset();

private:


};


extern const device_type LC89510;
