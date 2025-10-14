// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
#ifndef MAME_ENSONIQ_VFXCART_H
#define MAME_ENSONIQ_VFXCART_H

#pragma once

#include "emu.h"

class ensoniq_vfx_cartridge;

class ensoniq_vfx_cartslot: 
  public device_t,
  public device_single_card_slot_interface<ensoniq_vfx_cartridge>
{
public:
	DISABLE_COPYING(ensoniq_vfx_cartslot);

  ensoniq_vfx_cartslot(
      const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			u32 clock = 0);

  virtual ~ensoniq_vfx_cartslot();

  u8 read(offs_t offset);
  void write(offs_t offset, u8 data);
  auto cartridge_present_cb() { return m_cartridge_present_cb.bind(); }
protected:
	virtual void device_start() override ATTR_COLD {}
private:
  devcb_write_line m_cartridge_present_cb;
};

// device type declaration
DECLARE_DEVICE_TYPE(ENSONIQ_VFX_CARTRIDGE_SLOT, ensoniq_vfx_cartslot)

class ensoniq_vfx_cartridge:
  public device_t,
  public device_image_interface
{
public:
	typedef delegate<void (ensoniq_vfx_cartridge *)> load_cb;
	typedef delegate<void (ensoniq_vfx_cartridge *)> unload_cb;

  ensoniq_vfx_cartridge(
      const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			u32 clock = 0);

  virtual ~ensoniq_vfx_cartridge();

  u8 read(offs_t offset);
  void write(offs_t offset, u8 data);
  virtual const char *filename() const { return device_image_interface::filename(); }

  void setup_load_cb(load_cb cb);
	void setup_unload_cb(unload_cb cb);

	// device_image_interface overrides
  virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

	virtual bool        is_readable()                   const noexcept override { return true; }
	virtual bool        is_writeable()                  const noexcept override { return true; }
	virtual bool        is_creatable()                  const noexcept override { return true; }
	virtual bool        is_reset_on_load()              const noexcept override { return false; }
	virtual char const *file_extensions()               const noexcept override { return "rom,cart,eeprom,sc32"; }
	virtual char const *image_type_name()               const noexcept override { return "VFX-Cartridge"; }
	virtual char const *image_brief_type_name()         const noexcept override { return "cart"; }

protected:
	virtual void device_start() override ATTR_COLD {}

private:

  enum class state : int {
    IDLE,
    CMD1,
    CMD2,
    WR
  };
  state m_state = state::IDLE;
  std::array<uint8_t, 32 * 1024> m_storage;
  bool m_is_writable;

  load_cb m_load_cb;
	unload_cb m_unload_cb;
};


// device type declaration
DECLARE_DEVICE_TYPE(ENSONIQ_VFX_CARTRIDGE, ensoniq_vfx_cartridge)

#endif // MAME_ENSONIQ_VFXCART_H
