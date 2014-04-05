#ifndef __NEXTKBD_H__
#define __NEXTKBD_H__

#include "emu.h"

#define MCFG_NEXTKBD_INT_CHANGE_CALLBACK(_write) \
	devcb = &nextkbd_device::set_int_change_wr_callback(*device, DEVCB2_##_write);

#define MCFG_NEXTKBD_INT_POWER_CALLBACK(_write) \
	devcb = &nextkbd_device::set_int_power_wr_callback(*device, DEVCB2_##_write);

#define MCFG_NEXTKBD_INT_NMI_CALLBACK(_write) \
	devcb = &nextkbd_device::set_int_nmi_wr_callback(*device, DEVCB2_##_write);

class nextkbd_device : public device_t {
public:
	nextkbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb2_base &set_int_change_wr_callback(device_t &device, _Object object) { return downcast<nextkbd_device &>(device).int_change_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_int_power_wr_callback(device_t &device, _Object object) { return downcast<nextkbd_device &>(device).int_power_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_int_nmi_wr_callback(device_t &device, _Object object) { return downcast<nextkbd_device &>(device).int_nmi_cb.set_callback(object); }

	DECLARE_ADDRESS_MAP(amap, 32);

	DECLARE_READ32_MEMBER(ctrl_r);
	DECLARE_READ32_MEMBER(ctrl2_r);
	DECLARE_READ32_MEMBER(data_r);

	DECLARE_WRITE32_MEMBER(ctrl_w);
	DECLARE_WRITE32_MEMBER(ctrl2_w);
	DECLARE_WRITE32_MEMBER(data_w);

	bool int_level_get();

	DECLARE_INPUT_CHANGED_MEMBER(update);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual ioport_constructor device_input_ports() const;

private:
	enum { FLAG_INT = 0x800000, FLAG_DATA = 0x400000, FLAG_RESET = 0x000200 };
	enum { FIFO_SIZE = 32 };
	enum { KEYDOWN = 0x0080, KEYVALID = 0x8000 };

	devcb2_write_line int_change_cb, int_power_cb, int_nmi_cb;
	emu_timer *kbd_timer;
	bool nmi_active;

	UINT32 control, control2, data, fifo_ir, fifo_iw, fifo_size;
	UINT32 fifo[FIFO_SIZE];
	UINT16 modifiers_state;

	void fifo_push(UINT32 val);
	UINT32 fifo_pop();
	bool fifo_empty() const;

	void send();
};

extern const device_type NEXTKBD;

#endif
