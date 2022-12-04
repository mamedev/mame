// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_KONAMI_H
#define MAME_BUS_MSX_CART_KONAMI_H

#pragma once

#include "bus/msx/slot/cartridge.h"
#include "sound/k051649.h"
#include "sound/vlm5030.h"
#include "sound/dac.h"


DECLARE_DEVICE_TYPE(MSX_CART_KONAMI,           msx_cart_konami_device)
DECLARE_DEVICE_TYPE(MSX_CART_KONAMI_SCC,       msx_cart_konami_scc_device)
DECLARE_DEVICE_TYPE(MSX_CART_GAMEMASTER2,      msx_cart_gamemaster2_device)
DECLARE_DEVICE_TYPE(MSX_CART_SYNTHESIZER,      msx_cart_synthesizer_device)
DECLARE_DEVICE_TYPE(MSX_CART_SOUND_SNATCHER,   msx_cart_konami_sound_snatcher_device)
DECLARE_DEVICE_TYPE(MSX_CART_SOUND_SDSNATCHER, msx_cart_konami_sound_sdsnatcher_device)
DECLARE_DEVICE_TYPE(MSX_CART_KEYBOARD_MASTER,  msx_cart_keyboard_master_device)
DECLARE_DEVICE_TYPE(MSX_CART_EC701,            msx_cart_ec701_device)


class msx_cart_konami_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_konami_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<4> m_rombank;
	u8 m_bank_mask;
};


class msx_cart_konami_scc_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_konami_scc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

private:
	template <int Bank> void bank_w(u8 data);

	required_device<k051649_device> m_k051649;
	memory_bank_array_creator<4> m_rombank;
	memory_view m_scc_view;

	u8 m_bank_mask;
};


class msx_cart_gamemaster2_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_gamemaster2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<3> m_rombank;
	memory_bank_array_creator<3> m_rambank;
	memory_view m_view0;
	memory_view m_view1;
	memory_view m_view2;
};


class msx_cart_synthesizer_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }

	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<dac_byte_interface> m_dac;
};


class msx_cart_konami_sound_device : public device_t, public msx_cart_interface
{
public:
	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	msx_cart_konami_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 min_rambank, u8 max_rambank);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

private:
	static constexpr u8 VIEW_READ = 0;
	static constexpr u8 VIEW_RAM = 1;
	static constexpr u8 VIEW_INVALID = 2;
	static constexpr u8 VIEW_SCC = 4;

	template <int Bank> void bank_w(u8 data);
	template <int Bank> void switch_bank();
	void control_w(u8 data);

	// This is actually a K052539
	required_device<k051649_device> m_k052539;
	memory_bank_array_creator<4> m_rambank;
	memory_view m_view0;
	memory_view m_view1;
	memory_view m_view2;
	memory_view m_view3;

	u8 m_min_rambank;
	u8 m_max_rambank;
	u8 m_selected_bank[4];
	u8 m_control;
};


class msx_cart_konami_sound_snatcher_device : public msx_cart_konami_sound_device
{
public:
	msx_cart_konami_sound_snatcher_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;
};


class msx_cart_konami_sound_sdsnatcher_device : public msx_cart_konami_sound_device
{
public:
	msx_cart_konami_sound_sdsnatcher_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;
};



class msx_cart_keyboard_master_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_keyboard_master_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void device_add_mconfig(machine_config &config) override;

	virtual image_init_result initialize_cartridge(std::string &message) override;

private:
	required_device<vlm5030_device> m_vlm5030;

	uint8_t read_vlm(offs_t offset);
	void io_20_w(uint8_t data);
	uint8_t io_00_r();

	void vlm_map(address_map &map);
};



class msx_cart_ec701_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_ec701_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

	virtual image_init_result initialize_cartridge(std::string &message) override;

private:
	void bank_w(u8 data);

	memory_bank_creator m_rombank;
	memory_view m_view;
};



#endif // MAME_BUS_MSX_CART_KONAMI_H
