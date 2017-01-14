// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria, David Haywood, Vas Crabb

#include "cpu/m6805/m68705.h"


extern const device_type TAITO68705_MCU;
extern const device_type TAITO68705_MCU_SLAP;
extern const device_type TAITO68705_MCU_TIGER;
extern const device_type ARKANOID_68705P3;
extern const device_type ARKANOID_68705P5;


class taito68705_mcu_device_base : public device_t
{
public:
	template <typename Obj> static devcb_base &set_semaphore_cb(device_t &device, Obj &&object)
	{ return downcast<taito68705_mcu_device_base &>(device).m_semaphore_cb.set_callback(std::forward<Obj>(object)); }

	// host interface
	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_WRITE_LINE_MEMBER(reset_w);
	DECLARE_CUSTOM_INPUT_MEMBER(semaphore_r);
	DECLARE_CUSTOM_INPUT_MEMBER(host_flag_r);
	DECLARE_CUSTOM_INPUT_MEMBER(mcu_flag_r);

	// MCU callbacks
	DECLARE_WRITE8_MEMBER(mcu_pa_w);

protected:
	taito68705_mcu_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *name,
			char const *tag,
			device_t *owner,
			u32 clock,
			char const *shortname,
			char const *source);

	virtual void device_start() override;
	virtual void device_reset() override;

	u8 pa_value() const;
	void latch_control(u8 data, u8 &value, unsigned host_bit, unsigned mcu_bit);

	required_device<m68705p_device> m_mcu;

	bool    m_host_flag;
	bool    m_mcu_flag;

private:
	devcb_write_line    m_semaphore_cb;

	bool    m_latch_driven;
	bool    m_reset_input;
	u8      m_host_latch;
	u8      m_mcu_latch;
	u8      m_pa_output;
};


class taito68705_mcu_device : public taito68705_mcu_device_base
{
public:
	taito68705_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(mcu_portc_r);
	virtual DECLARE_WRITE8_MEMBER(mcu_portb_w);

	DECLARE_READ8_MEMBER(mcu_status_r);

	bool get_main_sent() { return m_host_flag; };
	bool get_mcu_sent() { return m_mcu_flag; };

protected:
	taito68705_mcu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, u32 clock, const char *shortname, const char *source);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

	u8  m_pb_output;
};


#define MCFG_TAITO_M68705_EXTENSION_CB(_devcb) \
	taito68705_mcu_slap_device::set_extension_cb(*device, DEVCB_##_devcb);

class taito68705_mcu_slap_device : public taito68705_mcu_device
{
public:
	taito68705_mcu_slap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual DECLARE_WRITE8_MEMBER( mcu_portb_w ) override;

	template<class _Object> static devcb_base &set_extension_cb(device_t &device, _Object object) { return downcast<taito68705_mcu_slap_device &>(device).m_extension_cb_w.set_callback(object); }

	devcb_write8 m_extension_cb_w;

protected:
	virtual void device_start() override;
};


class taito68705_mcu_tiger_device : public taito68705_mcu_device
{
public:
	taito68705_mcu_tiger_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual DECLARE_READ8_MEMBER( mcu_portc_r ) override;
};


#define MCFG_ARKANOID_MCU_SEMAPHORE_CB(cb) \
	arkanoid_mcu_device_base::set_semaphore_cb(*device, DEVCB_##cb);

#define MCFG_ARKANOID_MCU_PORTB_R_CB(cb) \
	arkanoid_mcu_device_base::set_portb_r_cb(*device, DEVCB_##cb);

class arkanoid_mcu_device_base : public taito68705_mcu_device_base
{
public:
	template <typename Obj> static devcb_base &set_portb_r_cb(device_t &device, Obj &&object)
	{ return downcast<arkanoid_mcu_device_base &>(device).m_portb_r_cb.set_callback(std::forward<Obj>(object)); }

	// MCU callbacks
	DECLARE_READ8_MEMBER(mcu_pb_r);
	DECLARE_READ8_MEMBER(mcu_pc_r);
	DECLARE_WRITE8_MEMBER(mcu_pc_w);

protected:
	arkanoid_mcu_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *name,
			char const *tag,
			device_t *owner,
			u32 clock,
			char const *shortname,
			char const *source);

	virtual void device_start() override;

	devcb_read8 m_portb_r_cb;

	u8  m_pc_output;
};


class arkanoid_68705p3_device : public arkanoid_mcu_device_base
{
public:
	arkanoid_68705p3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};


class arkanoid_68705p5_device : public arkanoid_mcu_device_base
{
public:
	arkanoid_68705p5_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};
