#ifndef __AM8530H_H__
#define __AM8530H_H__

#include "emu.h"

#define MCFG_AM8530H_ADD(_tag, _int_change_cb)	\
	MCFG_DEVICE_ADD(_tag, AM8530H, 0)	\
	downcast<am8530h_device *>(device)->set_int_change_cb(_int_change_cb);

class am8530h_device : public device_t {
public:
	typedef delegate<void ()> int_cb_t;

	am8530h_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void set_int_change_cb(int_cb_t int_change_cb);

	DECLARE_READ8_MEMBER(ca_r);
	DECLARE_READ8_MEMBER(cb_r);
	DECLARE_READ8_MEMBER(da_r);
	DECLARE_READ8_MEMBER(db_r);

	DECLARE_WRITE8_MEMBER(ca_w);
	DECLARE_WRITE8_MEMBER(cb_w);
	DECLARE_WRITE8_MEMBER(da_w);
	DECLARE_WRITE8_MEMBER(db_w);

	void data_a_w(UINT8 val);
	void data_b_w(UINT8 val);

	void int_ack();
	bool int_level_get();

protected:
	virtual void device_start();

private:
	int_cb_t int_change_cb;
};

extern const device_type AM8530H;

#endif
