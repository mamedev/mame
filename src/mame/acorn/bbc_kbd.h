// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro Keyboard

****************************************************************************/

#ifndef MAME_ACORN_BBC_KBD_H
#define MAME_ACORN_BBC_KBD_H


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_kbd_device

class bbc_kbd_device : public device_t
{
public:
	// construction/destruction
	bbc_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto rst_handler() { return m_rst_handler.bind(); }
	auto ca2_handler() { return m_ca2_handler.bind(); }

	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);

	void write_kb_en(int state);
	uint8_t read(uint8_t data);

protected:
	bbc_kbd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport_array<16> m_kbd_col;

	devcb_write_line m_rst_handler;
	devcb_write_line m_ca2_handler;

	uint8_t m_column;
	bool m_kbd_enable;

	TIMER_CALLBACK_MEMBER(keyscan);

	emu_timer *m_kbd_timer = nullptr;
};


// ======================> bbc_kbd_no_device

class bbc_kbd_no_device : public bbc_kbd_device
{
public:
	bbc_kbd_no_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> bbcm_kbd_device

class bbcm_kbd_device : public bbc_kbd_device
{
public:
	bbcm_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> bbcmc_kbd_device

class bbcmc_kbd_device : public bbc_kbd_device
{
public:
	bbcmc_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> bbcmc_kbd_ar_device

class bbcmc_kbd_ar_device : public bbc_kbd_device
{
public:
	bbcmc_kbd_ar_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> abc_kbd_device

class abc_kbd_device : public bbc_kbd_device
{
public:
	abc_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> autoc15_kbd_device

class autoc15_kbd_device : public bbc_kbd_device
{
public:
	autoc15_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> ht280_kbd_device

class ht280_kbd_device : public bbc_kbd_device
{
public:
	ht280_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::KEYBOARD; }

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> se3010_kbd_device

class se3010_kbd_device : public bbc_kbd_device
{
public:
	se3010_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::KEYBOARD; }

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> torchb_kbd_device

class torchb_kbd_device : public bbc_kbd_device
{
public:
	torchb_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> torchi_kbd_device

class torchi_kbd_device : public bbc_kbd_device
{
public:
	torchi_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::KEYBOARD; }

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_KBD,       bbc_kbd_device);
DECLARE_DEVICE_TYPE(BBC_KBD_NO,    bbc_kbd_no_device);
DECLARE_DEVICE_TYPE(BBCM_KBD,      bbcm_kbd_device);
DECLARE_DEVICE_TYPE(BBCMC_KBD,     bbcmc_kbd_device);
DECLARE_DEVICE_TYPE(BBCMC_KBD_AR,  bbcmc_kbd_ar_device);
DECLARE_DEVICE_TYPE(ABC_KBD,       abc_kbd_device);
DECLARE_DEVICE_TYPE(AUTOC15_KBD,   autoc15_kbd_device);
DECLARE_DEVICE_TYPE(HT280_KBD,     ht280_kbd_device);
DECLARE_DEVICE_TYPE(SE3010_KBD,    se3010_kbd_device);
DECLARE_DEVICE_TYPE(TORCHB_KBD,    torchb_kbd_device);
DECLARE_DEVICE_TYPE(TORCHI_KBD,    torchi_kbd_device);

#endif // MAME_ACORN_BBC_KBD_H
