// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_VCS_ROM_H
#define MAME_BUS_VCS_ROM_H

#pragma once

#include "vcs_slot.h"


// ======================> a26_rom_2k_device

class a26_rom_base_device : public device_t,
							public device_vcs_cart_interface
{
protected:
	a26_rom_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_vcs_cart_interface(mconfig, *this)
	{ }

	virtual void device_start() override { }

	bool is_ram_present(size_t minimum_size);
	uint8_t read_ram(offs_t offset);
	void write_ram(offs_t offset, uint8_t data);
};


class a26_rom_2k_4k_device : public a26_rom_base_device
{
public:
	a26_rom_2k_4k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;
};


// ======================> a26_rom_f6_device

class a26_rom_f6_device : public a26_rom_base_device
{
public:
	a26_rom_f6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	a26_rom_f6_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	void install_super_chip_handlers(address_space *space);

	virtual void device_reset() override ATTR_COLD;
	virtual uint8_t get_start_bank() { return 0; }
	void switch_bank(offs_t offset, uint8_t data);

	memory_bank_creator m_bank;
};


// ======================> a26_rom_f4_device

class a26_rom_f4_device : public a26_rom_f6_device
{
public:
	a26_rom_f4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual uint8_t get_start_bank() override { return 7; }
};


// ======================> a26_rom_f8_device

class a26_rom_f8_device : public a26_rom_f6_device
{
public:
	a26_rom_f8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	a26_rom_f8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> a26_rom_f8_sw_device

class a26_rom_f8_sw_device : public a26_rom_f8_device
{
public:
	a26_rom_f8_sw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// Snow White proto starts from bank 1
	virtual uint8_t get_start_bank() override { return 1; }
};


// ======================> a26_rom_fa_device

class a26_rom_fa_device : public a26_rom_f6_device
{
public:
	a26_rom_fa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;
};


// ======================> a26_rom_fe_device

class a26_rom_fe_device : public a26_rom_base_device
{
public:
	a26_rom_fe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	void trigger_bank();
	void switch_bank(uint8_t data);

	memory_bank_creator m_bank;
	bool m_trigger_on_next_access;
	bool m_ignore_first_read;
};


// ======================> a26_rom_3e_device

class a26_rom_3e_device : public a26_rom_base_device
{
public:
	a26_rom_3e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_reset() override ATTR_COLD;
	void select_ram_bank(offs_t offset, uint8_t data);
	void select_rom_bank(offs_t offset, uint8_t data);

	memory_bank_creator m_rom_bank;
	memory_bank_creator m_ram_bank;
	memory_view m_view;
	uint8_t m_rom_bank_mask;
	uint8_t m_ram_bank_mask;
};


// ======================> a26_rom_3f_device

class a26_rom_3f_device : public a26_rom_base_device
{
public:
	a26_rom_3f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_reset() override ATTR_COLD;
	void select_bank(offs_t offset, uint8_t data);

	memory_bank_creator m_bank;
	uint8_t m_bank_mask;
};


// ======================> a26_rom_e0_device

class a26_rom_e0_device : public a26_rom_base_device
{
public:
	a26_rom_e0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_reset() override ATTR_COLD;
	template <uint8_t Bank> void switch_bank(offs_t offset, uint8_t data);

	memory_bank_array_creator<3> m_bank;
};


// ======================> a26_rom_e7_device

class a26_rom_e7_device : public a26_rom_base_device
{
public:
	a26_rom_e7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_reset() override ATTR_COLD;
	void switch_rom_bank(offs_t offset, uint8_t data);
	void switch_ram_bank(offs_t offset, uint8_t data);

	memory_bank_creator m_rom_bank;
	memory_bank_creator m_lo_ram_bank;
	memory_bank_creator m_hi_ram_bank;
	memory_view m_view;
};


// ======================> a26_rom_ua_device

class a26_rom_ua_device : public a26_rom_base_device
{
public:
	a26_rom_ua_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_reset() override ATTR_COLD;
	void change_bank(offs_t offset);

	memory_bank_creator m_bank;
};


// ======================> a26_rom_cv_device

class a26_rom_cv_device : public a26_rom_base_device
{
public:
	a26_rom_cv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;
};


// ======================> a26_rom_dc_device

class a26_rom_dc_device : public a26_rom_base_device
{
public:
	a26_rom_dc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_reset() override ATTR_COLD;
	uint8_t read_current_bank(offs_t offset);
	void switch_bank(offs_t offset, uint8_t data);

	memory_bank_creator m_bank;
};


// ======================> a26_rom_fv_device

class a26_rom_fv_device : public a26_rom_base_device
{
public:
	a26_rom_fv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_reset() override ATTR_COLD;
	void switch_bank(offs_t offset, uint8_t data);

	memory_bank_creator m_bank;
};


// ======================> a26_rom_jvp_device

class a26_rom_jvp_device : public a26_rom_base_device
{
public:
	a26_rom_jvp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_reset() override ATTR_COLD;
	void change_bank();

	memory_bank_creator m_bank;
};


// ======================> a26_rom_4in1_device

class a26_rom_4in1_device : public a26_rom_base_device
{
public:
	a26_rom_4in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_bank_creator m_bank;
	uint8_t m_current_game;
};


// ======================> a26_rom_8in1_device

class a26_rom_8in1_device : public a26_rom_base_device
{
public:
	a26_rom_8in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	void switch_bank(offs_t offset, uint8_t data);

	memory_bank_creator m_bank;
	uint8_t m_game_bank;
};


// ======================> a26_rom_32in1_device

class a26_rom_32in1_device : public a26_rom_base_device
{
public:
	a26_rom_32in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_bank_creator m_bank;
	uint8_t m_current_game;
};


// ======================> a26_rom_x07_device

class a26_rom_x07_device : public a26_rom_base_device
{
public:
	a26_rom_x07_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

private:
	void change_bank1(offs_t offset);
	void change_bank2(offs_t offset);

	memory_bank_creator m_bank;
};



// device type definition
DECLARE_DEVICE_TYPE(A26_ROM_2K_4K, a26_rom_2k_4k_device)
DECLARE_DEVICE_TYPE(A26_ROM_F4,    a26_rom_f4_device)
DECLARE_DEVICE_TYPE(A26_ROM_F6,    a26_rom_f6_device)
DECLARE_DEVICE_TYPE(A26_ROM_F8,    a26_rom_f8_device)
DECLARE_DEVICE_TYPE(A26_ROM_F8_SW, a26_rom_f8_sw_device)
DECLARE_DEVICE_TYPE(A26_ROM_FA,    a26_rom_fa_device)
DECLARE_DEVICE_TYPE(A26_ROM_FE,    a26_rom_fe_device)
DECLARE_DEVICE_TYPE(A26_ROM_3E,    a26_rom_3e_device)
DECLARE_DEVICE_TYPE(A26_ROM_3F,    a26_rom_3f_device)
DECLARE_DEVICE_TYPE(A26_ROM_E0,    a26_rom_e0_device)
DECLARE_DEVICE_TYPE(A26_ROM_E7,    a26_rom_e7_device)
DECLARE_DEVICE_TYPE(A26_ROM_UA,    a26_rom_ua_device)
DECLARE_DEVICE_TYPE(A26_ROM_CV,    a26_rom_cv_device)
DECLARE_DEVICE_TYPE(A26_ROM_DC,    a26_rom_dc_device)
DECLARE_DEVICE_TYPE(A26_ROM_FV,    a26_rom_fv_device)
DECLARE_DEVICE_TYPE(A26_ROM_JVP,   a26_rom_jvp_device)
DECLARE_DEVICE_TYPE(A26_ROM_4IN1,  a26_rom_4in1_device)
DECLARE_DEVICE_TYPE(A26_ROM_8IN1,  a26_rom_8in1_device)
DECLARE_DEVICE_TYPE(A26_ROM_32IN1, a26_rom_32in1_device)
DECLARE_DEVICE_TYPE(A26_ROM_X07,   a26_rom_x07_device)


#endif // MAME_BUS_VCS_ROM_H
