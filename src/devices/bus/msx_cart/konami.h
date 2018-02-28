// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_KONAMI_H
#define MAME_BUS_MSX_CART_KONAMI_H

#include "bus/msx_cart/cartridge.h"
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


class msx_cart_konami_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_konami_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void restore_banks();

private:
	uint8_t m_bank_mask;
	uint8_t m_selected_bank[4];
	uint8_t *m_bank_base[8];
};


class msx_cart_konami_scc_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_konami_scc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

	void restore_banks();

private:
	required_device<k051649_device> m_k051649;

	uint8_t m_bank_mask;
	uint8_t m_selected_bank[4];
	uint8_t *m_bank_base[8];
	bool m_scc_active;
};


class msx_cart_gamemaster2_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_gamemaster2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void restore_banks();

private:
	uint8_t m_selected_bank[3];
	uint8_t *m_bank_base[8];

	void setup_bank(uint8_t bank);
};


class msx_cart_synthesizer_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void device_add_mconfig(machine_config &config) override;

private:
	uint8_t *m_bank_base;
	required_device<dac_byte_interface> m_dac;
};


class msx_cart_konami_sound_device : public device_t, public msx_cart_interface
{
public:
	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

protected:
	msx_cart_konami_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

	void restore_banks();

	uint8_t *m_ram_bank[16];

private:
	// This is actually a K052539
	required_device<k051649_device> m_k052539;

	uint8_t m_selected_bank[4];
	uint8_t *m_bank_base[8];
	bool m_scc_active;
	bool m_sccplus_active;
	bool m_ram_enabled[4];
	uint8_t m_scc_mode;

	void setup_bank(uint8_t bank);
};


class msx_cart_konami_sound_snatcher_device : public msx_cart_konami_sound_device
{
public:
	msx_cart_konami_sound_snatcher_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;
};


class msx_cart_konami_sound_sdsnatcher_device : public msx_cart_konami_sound_device
{
public:
	msx_cart_konami_sound_sdsnatcher_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;
};



class msx_cart_keyboard_master_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_keyboard_master_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void device_add_mconfig(machine_config &config) override;

	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;

private:
	required_device<vlm5030_device> m_vlm5030;

	DECLARE_READ8_MEMBER(read_vlm);
	DECLARE_WRITE8_MEMBER(io_20_w);
	DECLARE_READ8_MEMBER(io_00_r);

	void vlm_map(address_map &map);
};



#endif // MAME_BUS_MSX_CART_KONAMI_H
