// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_KONAMI_H
#define __MSX_CART_KONAMI_H

#include "bus/msx_cart/cartridge.h"
#include "sound/k051649.h"
#include "sound/vlm5030.h"
#include "sound/dac.h"


extern const device_type MSX_CART_KONAMI;
extern const device_type MSX_CART_KONAMI_SCC;
extern const device_type MSX_CART_GAMEMASTER2;
extern const device_type MSX_CART_SYNTHESIZER;
extern const device_type MSX_CART_SOUND_SNATCHER;
extern const device_type MSX_CART_SOUND_SDSNATCHER;
extern const device_type MSX_CART_KEYBOARD_MASTER;


class msx_cart_konami : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_konami(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

	void restore_banks();

private:
	UINT8 m_bank_mask;
	UINT8 m_selected_bank[4];
	UINT8 *m_bank_base[8];
};


class msx_cart_konami_scc : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_konami_scc(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

	void restore_banks();

private:
	required_device<k051649_device> m_k051649;

	UINT8 m_bank_mask;
	UINT8 m_selected_bank[4];
	UINT8 *m_bank_base[8];
	bool m_scc_active;
};


class msx_cart_gamemaster2 : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_gamemaster2(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

	void restore_banks();

private:
	UINT8 m_selected_bank[3];
	UINT8 *m_bank_base[8];

	void setup_bank(UINT8 bank);
};


class msx_cart_synthesizer : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_synthesizer(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

private:
	UINT8 *m_bank_base;
	required_device<dac_device> m_dac;
};


class msx_cart_konami_sound : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_konami_sound(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

	void restore_banks();

protected:
	UINT8 *m_ram_bank[16];

private:
	// This is actually a K052539
	required_device<k051649_device> m_k052539;

	UINT8 m_selected_bank[4];
	UINT8 *m_bank_base[8];
	bool m_scc_active;
	bool m_sccplus_active;
	bool m_ram_enabled[4];
	UINT8 m_scc_mode;

	void setup_bank(UINT8 bank);
};


class msx_cart_konami_sound_snatcher : public msx_cart_konami_sound
{
public:
	msx_cart_konami_sound_snatcher(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void initialize_cartridge();
};


class msx_cart_konami_sound_sdsnatcher : public msx_cart_konami_sound
{
public:
	msx_cart_konami_sound_sdsnatcher(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void initialize_cartridge();
};



class msx_cart_keyboard_master : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_keyboard_master(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);

	DECLARE_WRITE8_MEMBER(io_20_w);
	DECLARE_READ8_MEMBER(io_00_r);

private:
	required_device<vlm5030_device> m_vlm5030;
};



#endif
