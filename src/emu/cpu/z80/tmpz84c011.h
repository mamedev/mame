/***************************************************************************

    Toshiba TMPZ84C011, TLCS-Z80 ASSP Family
    Z80 CPU, CTC, CGC(6/8MHz), I/O8x5

***************************************************************************/

#include "emu.h"
#include "z80.h"
#include "machine/z80ctc.h"

// NOTE: for CTC callbacks, see machine/z80ctc.h
// TMPZ84C011 PIO callbacks
#define MCFG_TMPZ84C011_PORTA_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inportsa_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTB_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inportsb_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTC_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inportsc_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTD_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inportsd_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTE_READ_CB(_devcb) \
	devcb = &tmpz84c011_device::set_inportse_cb(*device, DEVCB_##_devcb);


#define MCFG_TMPZ84C011_PORTA_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outportsa_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTB_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outportsb_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTC_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outportsc_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTD_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outportsd_cb(*device, DEVCB_##_devcb);

#define MCFG_TMPZ84C011_PORTE_WRITE_CB(_devcb) \
	devcb = &tmpz84c011_device::set_outportse_cb(*device, DEVCB_##_devcb);


class tmpz84c011_device : public z80_device
{
public:
	tmpz84c011_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	// static configuration helpers
	template<class _Object> static devcb_base & set_outportsa_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outportsa.set_callback(object); }
	template<class _Object> static devcb_base & set_outportsb_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outportsb.set_callback(object); }
	template<class _Object> static devcb_base & set_outportsc_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outportsc.set_callback(object); }
	template<class _Object> static devcb_base & set_outportsd_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outportsd.set_callback(object); }
	template<class _Object> static devcb_base & set_outportse_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_outportse.set_callback(object); }

	template<class _Object> static devcb_base & set_inportsa_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inportsa.set_callback(object); }
	template<class _Object> static devcb_base & set_inportsb_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inportsb.set_callback(object); }
	template<class _Object> static devcb_base & set_inportsc_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inportsc.set_callback(object); }
	template<class _Object> static devcb_base & set_inportsd_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inportsd.set_callback(object); }
	template<class _Object> static devcb_base & set_inportse_cb(device_t &device, _Object object) { return downcast<tmpz84c011_device &>(device).m_inportse.set_callback(object); }

	// devices/pointers
	required_device<z80ctc_device> m_ctc;

	DECLARE_READ8_MEMBER(tmpz84c011_pa_r);
	DECLARE_READ8_MEMBER(tmpz84c011_pb_r);
	DECLARE_READ8_MEMBER(tmpz84c011_pc_r);
	DECLARE_READ8_MEMBER(tmpz84c011_pd_r);
	DECLARE_READ8_MEMBER(tmpz84c011_pe_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_pa_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_pb_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_pc_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_pd_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_pe_w);
	DECLARE_READ8_MEMBER(tmpz84c011_dir_pa_r);
	DECLARE_READ8_MEMBER(tmpz84c011_dir_pb_r);
	DECLARE_READ8_MEMBER(tmpz84c011_dir_pc_r);
	DECLARE_READ8_MEMBER(tmpz84c011_dir_pd_r);
	DECLARE_READ8_MEMBER(tmpz84c011_dir_pe_r);
	DECLARE_WRITE8_MEMBER(tmpz84c011_dir_pa_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_dir_pb_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_dir_pc_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_dir_pd_w);
	DECLARE_WRITE8_MEMBER(tmpz84c011_dir_pe_w);

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

private:
	// internal state
	UINT8 m_pio_dir[5];
	UINT8 m_pio_latch[5];

	// callbacks
	devcb_write8 m_outportsa;
	devcb_write8 m_outportsb;
	devcb_write8 m_outportsc;
	devcb_write8 m_outportsd;
	devcb_write8 m_outportse;

	devcb_read8 m_inportsa;
	devcb_read8 m_inportsb;
	devcb_read8 m_inportsc;
	devcb_read8 m_inportsd;
	devcb_read8 m_inportse;
};

extern const device_type TMPZ84C011;
