// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/**  Konami 053252  **/
/* CRT and interrupt control unit */
#pragma once

#ifndef __K053252_H__
#define __K053252_H__


#define MCFG_K053252_INT1_EN_CB(_devcb) \
	devcb = &k053252_device::set_int1_en_callback(*device, DEVCB_##_devcb);

#define MCFG_K053252_INT2_EN_CB(_devcb) \
	devcb = &k053252_device::set_int2_en_callback(*device, DEVCB_##_devcb);

#define MCFG_K053252_INT1_ACK_CB(_devcb) \
	devcb = &k053252_device::set_int1_ack_callback(*device, DEVCB_##_devcb);

#define MCFG_K053252_INT2_ACK_CB(_devcb) \
	devcb = &k053252_device::set_int2_ack_callback(*device, DEVCB_##_devcb);

/*#define MCFG_K053252_INT_TIME_CB(_devcb) \
    devcb = &k053252_device::set_int_time_callback(*device, DEVCB_##_devcb); */

#define MCFG_K053252_OFFSETS(_offsx, _offsy) \
	k053252_device::set_offsets(*device, _offsx, _offsy);

#define MCFG_K053252_SET_SLAVE_SCREEN(_tag) \
	k053252_device::static_set_slave_screen(*device, _tag);


class k053252_device : public device_t,
						public device_video_interface
{
public:
	k053252_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053252_device() {}

	template<class _Object> static devcb_base &set_int1_en_callback(device_t &device, _Object object) { return downcast<k053252_device &>(device).m_int1_en_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_int2_en_callback(device_t &device, _Object object) { return downcast<k053252_device &>(device).m_int2_en_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_int1_ack_callback(device_t &device, _Object object) { return downcast<k053252_device &>(device).m_int1_ack_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_int2_ack_callback(device_t &device, _Object object) { return downcast<k053252_device &>(device).m_int2_ack_cb.set_callback(object); }
	//template<class _Object> static devcb_base &set_int_time_callback(device_t &device, _Object object) { return downcast<k053252_device &>(device).m_int_time_cb.set_callback(object); }
	static void set_offsets(device_t &device, int offsx, int offsy) { downcast<k053252_device &>(device).m_offsx = offsx; downcast<k053252_device &>(device).m_offsy = offsy;}

	DECLARE_READ8_MEMBER( read );  // CCU registers
	DECLARE_WRITE8_MEMBER( write );

	void res_change();

	static void static_set_slave_screen(device_t &device, const char *tag);


protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	private:
	// internal state
	UINT8   m_regs[16];
	UINT16  m_hc,m_hfp,m_hbp;
	UINT16  m_vc,m_vfp,m_vbp;
	UINT8   m_vsw,m_hsw;

	devcb_write_line   m_int1_en_cb;
	devcb_write_line   m_int2_en_cb;
	devcb_write_line   m_int1_ack_cb;
	devcb_write_line   m_int2_ack_cb;
//  devcb_write8       m_int_time_cb;
	int                m_offsx;
	int                m_offsy;

	const char *    m_slave_screen_tag; 
	screen_device * m_slave_screen;

};

extern const device_type K053252;


#endif  /* __K033906_H__ */
