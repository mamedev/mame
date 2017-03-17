// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef __IE15_KEYBOARD_H__
#define __IE15_KEYBOARD_H__


#define IE_KB_ACK   1

#define IE_KB_RED   0x01
#define IE_KB_SDV   0x02
#define IE_KB_DUP   0x08
#define IE_KB_LIN   0x10
#define IE_KB_DK    0x20
#define IE_KB_PCH   0x40
#define IE_KB_NR    0x80

#define IE_KB_RED_BIT   0
#define IE_KB_SDV_BIT   1
#define IE_KB_DUP_BIT   3
#define IE_KB_LIN_BIT   4
#define IE_KB_DK_BIT    5
#define IE_KB_PCH_BIT   6
#define IE_KB_NR_BIT    7

#define IE_KB_SI    0x0f
#define IE_KB_SO    0x0e


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_IE15_KEYBOARD_CB(_devcb) \
	devcb = &ie15_keyboard_device::set_keyboard_callback(*device, DEVCB_##_devcb);

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class ie15_keyboard_device :
	public device_t
{
public:
	ie15_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	ie15_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_keyboard_callback(device_t &device, _Object object) { return downcast<ie15_keyboard_device &>(device).m_keyboard_cb.set_callback(object); }

	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	required_ioport_array<4> m_io_kbd;
	required_ioport m_io_kbdc;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void send_key(uint16_t code) { m_keyboard_cb((offs_t)0, code); }
	emu_timer *m_timer;

private:
	virtual uint16_t keyboard_handler(uint16_t last_code, uint8_t *scan_line);
	uint8_t row_number(uint32_t code);
	uint16_t m_last_code;
	uint8_t m_scan_line;
	uint8_t m_ruslat;
	uint8_t *m_rom;

	devcb_write16 m_keyboard_cb;
};

extern const device_type IE15_KEYBOARD;

#endif /* __IE15_KEYBOARD_H__ */
