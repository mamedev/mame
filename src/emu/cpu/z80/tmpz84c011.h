
#include "emu.h"
#include "z80.h"

#define MCFG_TMPZ84C011_PORTA_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inports0_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTB_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inports1_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTC_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inports2_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTD_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inports3_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTE_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inports4_cb(*device, DEVCB_##_devcb);


#define MCFG_TMPZ84C011_PORTA_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outports0_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTB_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outports1_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTC_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outports2_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTD_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outports3_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTE_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outports4_cb(*device, DEVCB_##_devcb);


#define MCFG_TMPZ84C011_Z80CTC_INTR_CB(_devcb) \
	devcb = &tmpz84c011_device::set_intr_callback(*device, DEVCB_##_devcb);


#define MCFG_TMPZ84C011_Z80CTC_ZC0_CB(_devcb) \
	devcb = &tmpz84c011_device::set_zc0_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_Z80CTC_ZC1_CB(_devcb) \
	devcb = &tmpz84c011_device::set_zc1_callback(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_Z80CTC_ZC2_CB(_devcb) \
	devcb = &tmpz84c011_device::set_zc2_callback(*device, DEVCB_##_devcb);

class tmpz84c011_device : public z80_device
{
public:
	tmpz84c011_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	template<class _Object> static devcb_base & set_outports0_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outports0.set_callback(object); }
	template<class _Object> static devcb_base & set_outports1_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outports1.set_callback(object); }
	template<class _Object> static devcb_base & set_outports2_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outports2.set_callback(object); }
	template<class _Object> static devcb_base & set_outports3_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outports3.set_callback(object); }
	template<class _Object> static devcb_base & set_outports4_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outports4.set_callback(object); }

	template<class _Object> static devcb_base & set_inports0_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inports0.set_callback(object); }
	template<class _Object> static devcb_base & set_inports1_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inports1.set_callback(object); }
	template<class _Object> static devcb_base & set_inports2_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inports2.set_callback(object); }
	template<class _Object> static devcb_base & set_inports3_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inports3.set_callback(object); }
	template<class _Object> static devcb_base & set_inports4_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inports4.set_callback(object); }

	template<class _Object> static devcb_base &set_intr_callback(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_intr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_zc0_callback(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_zc0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_zc1_callback(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_zc1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_zc2_callback(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_zc2_cb.set_callback(object); }


	DECLARE_READ8_MEMBER(tmpz84c011_pio_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_pio_w);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pa_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pb_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pc_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pd_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_pe_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pa_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pb_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pc_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pd_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_pe_w);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pa_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pb_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pc_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pd_r);
	DECLARE_READ8_MEMBER(tmpz84c011_0_dir_pe_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pa_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pb_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pc_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pd_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_0_dir_pe_w);

	DECLARE_READ8_MEMBER(porta_default_r);
	DECLARE_READ8_MEMBER(portb_default_r);
	DECLARE_READ8_MEMBER(portc_default_r);
	DECLARE_READ8_MEMBER(portd_default_r);
	DECLARE_READ8_MEMBER(porte_default_r);

	DECLARE_WRITE8_MEMBER(porta_default_w);
	DECLARE_WRITE8_MEMBER(portb_default_w);
	DECLARE_WRITE8_MEMBER(portc_default_w);
	DECLARE_WRITE8_MEMBER(portd_default_w);
	DECLARE_WRITE8_MEMBER(porte_default_w);

	DECLARE_WRITE_LINE_MEMBER(intr_cb_trampoline_w);
	DECLARE_WRITE_LINE_MEMBER(zc0_cb_trampoline_w);
	DECLARE_WRITE_LINE_MEMBER(zc1_cb_trampoline_w);
	DECLARE_WRITE_LINE_MEMBER(zc2_cb_trampoline_w);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();

	const address_space_config m_io_space_config;

	const address_space_config *memory_space_config(address_spacenum spacenum) const
	{
		switch (spacenum)
		{
			case AS_IO: return &m_io_space_config;
			default: return z80_device::memory_space_config(spacenum);
		}
	}


	UINT8 m_pio_dir[5];
	UINT8 m_pio_latch[5];

private:
	devcb_write8      m_outports0;
	devcb_write8      m_outports1;
	devcb_write8      m_outports2;
	devcb_write8      m_outports3;
	devcb_write8      m_outports4;

	devcb_read8       m_inports0;
	devcb_read8       m_inports1;
	devcb_read8       m_inports2;
	devcb_read8       m_inports3;
	devcb_read8       m_inports4;

	devcb_write_line   m_intr_cb;              // interrupt callback
	devcb_write_line   m_zc0_cb;               // channel 0 zero crossing callbacks
	devcb_write_line   m_zc1_cb;               // channel 1 zero crossing callbacks
	devcb_write_line   m_zc2_cb;               // channel 2 zero crossing callbacks

};

extern const device_type TMPZ84C011;
