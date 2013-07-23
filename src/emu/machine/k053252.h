/**  Konami 053252  **/
/* CRT and interrupt control unit */
#pragma once

#ifndef __K053252_H__
#define __K053252_H__


struct k053252_interface
{
	const char         *m_screen_tag;
	devcb_write_line   m_int1_en;
	devcb_write_line   m_int2_en;
	devcb_write_line   m_int1_ack;
	devcb_write_line   m_int2_ack;
//  devcb_write8       m_int_time;
	int                m_offsx;
	int                m_offsy;
};

class k053252_device : public device_t,
						public k053252_interface
{
public:
	k053252_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053252_device() {}

	DECLARE_READ8_MEMBER( read );  // CCU registers
	DECLARE_WRITE8_MEMBER( write );

	void res_change();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	private:
	// internal state
	UINT8   m_regs[16];
	UINT16  m_hc,m_hfp,m_hbp;
	UINT16  m_vc,m_vfp,m_vbp;
	UINT8   m_vsw,m_hsw;

	screen_device *m_screen;
	devcb_resolved_write_line m_int1_en_func;
	devcb_resolved_write_line m_int2_en_func;
	devcb_resolved_write_line m_int1_ack_func;
	devcb_resolved_write_line m_int2_ack_func;
	//devcb_resolved_write8     m_int_time_func;
};

extern const device_type K053252;

#define MCFG_K053252_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, K053252, _clock) \
	MCFG_DEVICE_CONFIG(_interface)


#endif  /* __K033906_H__ */
