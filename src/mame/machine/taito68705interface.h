// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria, David Haywood

#include "cpu/m6805/m6805.h"

class taito68705_mcu_device : public device_t
{
public:
	taito68705_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	taito68705_mcu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, u32 clock, const char *shortname, const char *source);

	~taito68705_mcu_device() {}

	virtual DECLARE_WRITE8_MEMBER( mcu_w );
	DECLARE_READ8_MEMBER( mcu_r );
	DECLARE_READ8_MEMBER( mcu_porta_r );
	virtual DECLARE_READ8_MEMBER( mcu_portc_r );
	DECLARE_WRITE8_MEMBER( mcu_porta_w );
	virtual DECLARE_WRITE8_MEMBER( mcu_portb_w );
	
	DECLARE_READ8_MEMBER( mcu_status_r );
	DECLARE_CUSTOM_INPUT_MEMBER( mcu_sent_r );
	DECLARE_CUSTOM_INPUT_MEMBER( main_sent_r );

	bool get_main_sent() { return m_main_sent; };
	bool get_mcu_sent() { return m_mcu_sent; };


protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	bool m_mcu_sent;
	bool m_main_sent;
	uint8_t m_from_main;
	uint8_t m_from_mcu;
	uint8_t m_from_mcu_latch;
	uint8_t m_to_mcu_latch;
	uint8_t m_old_portB;

	required_device<cpu_device> m_mcu;
};

extern const device_type TAITO68705_MCU;

#define MCFG_TAITO_M68705_EXTENSION_CB(_devcb) \
	taito68705_mcu_slap_device::set_extension_cb(*device, DEVCB_##_devcb);

class taito68705_mcu_slap_device : public taito68705_mcu_device
{
public:
	taito68705_mcu_slap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual DECLARE_WRITE8_MEMBER( mcu_portb_w ) override;
	
	template<class _Object> static devcb_base &set_extension_cb(device_t &device, _Object object) { return downcast<taito68705_mcu_slap_device &>(device).m_extension_cb_w.set_callback(object); }

	devcb_write8 m_extension_cb_w;

protected:
	virtual void device_start() override;
};

extern const device_type TAITO68705_MCU_SLAP;

class taito68705_mcu_tiger_device : public taito68705_mcu_device
{
public:
	taito68705_mcu_tiger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual DECLARE_READ8_MEMBER( mcu_portc_r ) override;
};

extern const device_type TAITO68705_MCU_TIGER;

class taito68705_mcu_beg_device : public taito68705_mcu_device
{
public:
	taito68705_mcu_beg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual DECLARE_WRITE8_MEMBER(mcu_w) override;
	virtual DECLARE_WRITE8_MEMBER(mcu_portb_w) override;
};

extern const device_type TAITO68705_MCU_BEG;
