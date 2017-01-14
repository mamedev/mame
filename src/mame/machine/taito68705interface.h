// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria, David Haywood, Vas Crabb

#include "cpu/m6805/m68705.h"


extern const device_type TAITO68705_MCU;
extern const device_type TAITO68705_MCU_SLAP;
extern const device_type TAITO68705_MCU_TIGER;
extern const device_type TAITO68705_MCU_BEG;
extern const device_type ARKANOID_68705P3;
extern const device_type ARKANOID_68705P5;


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


class taito68705_mcu_tiger_device : public taito68705_mcu_device
{
public:
	taito68705_mcu_tiger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual DECLARE_READ8_MEMBER( mcu_portc_r ) override;
};


class taito68705_mcu_beg_device : public taito68705_mcu_device
{
public:
	taito68705_mcu_beg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual DECLARE_WRITE8_MEMBER(mcu_w) override;
	virtual DECLARE_WRITE8_MEMBER(mcu_portb_w) override;
};


#define MCFG_ARKANOID_MCU_SEMAPHORE_CB(cb) \
	arkanoid_mcu_device_base::set_semaphore_cb(*device, DEVCB_##cb);

#define MCFG_ARKANOID_MCU_PORTB_R_CB(cb) \
	arkanoid_mcu_device_base::set_portb_r_cb(*device, DEVCB_##cb);

class arkanoid_mcu_device_base : public device_t
{
public:
	template <typename Obj> static devcb_base &set_semaphore_cb(device_t &device, Obj &&object)
	{ return downcast<arkanoid_mcu_device_base &>(device).m_semaphore_cb.set_callback(std::forward<Obj>(object)); }
	template <typename Obj> static devcb_base &set_portb_r_cb(device_t &device, Obj &&object)
	{ return downcast<arkanoid_mcu_device_base &>(device).m_portb_r_cb.set_callback(std::forward<Obj>(object)); }

	// host interface
	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_CUSTOM_INPUT_MEMBER(semaphore_r);
	DECLARE_WRITE_LINE_MEMBER(reset_w);

	// MCU callbacks
	DECLARE_READ8_MEMBER(mcu_pb_r);
	DECLARE_READ8_MEMBER(mcu_pc_r);
	DECLARE_WRITE8_MEMBER(mcu_pa_w);
	DECLARE_WRITE8_MEMBER(mcu_pc_w);

protected:
	arkanoid_mcu_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *name,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			char const *shortname,
			char const *source);

	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<m68705p_device> m_mcu;
	devcb_write_line                m_semaphore_cb;
	devcb_read8                     m_portb_r_cb;

	bool    m_reset_input;
	bool    m_host_flag;
	bool    m_mcu_flag;
	u8      m_host_latch;
	u8      m_mcu_latch;
	u8      m_pa_output;
	u8      m_pc_output;
};


class arkanoid_68705p3_device : public arkanoid_mcu_device_base
{
public:
	arkanoid_68705p3_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};


class arkanoid_68705p5_device : public arkanoid_mcu_device_base
{
public:
	arkanoid_68705p5_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};
