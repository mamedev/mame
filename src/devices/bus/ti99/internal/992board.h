// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/2 main board logic

    This component implements the custom video controller and interface chip
    from the TI-99/2 console.

    See 992board.cpp for documentation

    Michael Zapf

*****************************************************************************/

#ifndef MAME_BUS_TI99_INTERNAL_992BOARD_H
#define MAME_BUS_TI99_INTERNAL_992BOARD_H

#pragma once

#include "screen.h"
#include "bus/hexbus/hexbus.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"

#define TI992_CASSETTE   "cassette"
#define TI992_VDC_TAG    "vdc"
#define TI992_HEXBUS_TAG "hexbus"

namespace bus::ti99::internal {

class video992_device : public device_t,
							public device_video_interface
{
public:
	// Complete line (31.848 us)
	static constexpr unsigned TOTAL_HORZ                 = 342;
	static constexpr unsigned TOTAL_VERT_NTSC            = 262;

	// Delay(2) + ColorBurst(14) + Delay(8) + LeftBorder(13)
	static constexpr unsigned HORZ_DISPLAY_START         = 2 + 14 + 8 + 13;
	// RightBorder(15) + Delay(8) + HorizSync(26)
	static constexpr unsigned VERT_DISPLAY_START_NTSC    = 13 + 27;

	// update the screen
	uint32_t screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );

	// Video enable
	void videna(int state);

	// Callbacks
	auto readmem_cb() { return m_mem_read_cb.bind(); }
	auto hold_cb() { return m_hold_cb.bind(); }
	auto int_cb() { return m_int_cb.bind(); }

protected:
	video992_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	int     m_beol;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(hold_cpu);
	TIMER_CALLBACK_MEMBER(free_cpu);

private:
	devcb_read8   m_mem_read_cb; // Callback to read memory
	devcb_write_line m_hold_cb;
	devcb_write_line m_int_cb;

	bitmap_rgb32 m_tmpbmp;
	emu_timer   *m_free_timer;
	emu_timer   *m_hold_timer;

	int         m_top_border;
	int         m_vertical_size;

	rgb_t       m_border_color;
	rgb_t       m_background_color;
	rgb_t       m_text_color;

	bool        m_videna;

	attotime    m_hold_time;
};

/* Variant for TI-99/2 24K */
class video992_24_device : public video992_device
{
public:
	video992_24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

/* Variant for TI-99/2 32K */
class video992_32_device : public video992_device
{
public:
	video992_32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

/*
    I/O custom chip
*/
class io992_device : public bus::hexbus::hexbus_chained_device
{
public:
	io992_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t cruread(offs_t offset);
	void cruwrite(offs_t offset, uint8_t data);
	auto rombank_cb() { return m_set_rom_bank.bind(); }

protected:
	io992_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	bool m_have_banked_rom;

	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void hexbus_value_changed(uint8_t data) override;

private:
	required_device<hexbus::hexbus_device> m_hexbus;
	required_device<cassette_image_device> m_cassette;
	required_device<video992_device> m_videoctrl;
	required_ioport_array<8> m_keyboard;

	// Set ROM bank
	devcb_write_line m_set_rom_bank;

	// Keyboard row
	int m_key_row;

	// HSK* released flag. This is queried as CRU bit 6 (with the current HSK* level).
	bool m_hsk_released;
};


/* Variant for TI-99/2 24K */
class io992_24_device : public io992_device
{
public:
	io992_24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

/* Variant for TI-99/2 32K */
class io992_32_device : public io992_device
{
public:
	io992_32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

/********************************************************************

    Expansion port
    This is modeled after the ioport connector of the TI-99/4A

    However, since there are basically no expansion cards available,
    and only the memory expansion was described in the specs, we
    only include the necessary connections.

********************************************************************/
#define TI992_EXPPORT_TAG      "expport"

class ti992_expport_attached_device : public device_t
{
public:
	ti992_expport_attached_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock)
	{ }

	// Methods called from the console
	virtual void readz(offs_t offset, uint8_t *value) { }
	virtual void write(offs_t offset, uint8_t data) { }
};

class ti992_expport_device : public device_t, public device_single_card_slot_interface<ti992_expport_attached_device>
{
public:
	template <typename U>
	ti992_expport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, U &&opts, const char *dflt)
		: ti992_expport_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	ti992_expport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Methods called from the console
	// More methods should be added, once we can find further 99/2 cartridges
	void readz(offs_t offset, uint8_t *value);
	void write(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override { }
	virtual void device_config_complete() override;

private:
	ti992_expport_attached_device*    m_connected;
};

// RAM expansion

class ti992_expram_device : public ti992_expport_attached_device
{
public:
	ti992_expram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void readz(offs_t offset, uint8_t *value) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override {}
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ram_device> m_ram;
};


} // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(TI992_EXPPORT, bus::ti99::internal, ti992_expport_device)

void ti992_expport_options(device_slot_interface &device);

DECLARE_DEVICE_TYPE_NS(VIDEO99224, bus::ti99::internal, video992_24_device)
DECLARE_DEVICE_TYPE_NS(VIDEO99232, bus::ti99::internal, video992_32_device)
DECLARE_DEVICE_TYPE_NS(IO99224, bus::ti99::internal, io992_24_device)
DECLARE_DEVICE_TYPE_NS(IO99232, bus::ti99::internal, io992_32_device)

DECLARE_DEVICE_TYPE_NS(TI992_RAM32K, bus::ti99::internal, ti992_expram_device)

#endif // MAME_BUS_TI99_INTERNAL_992BOARD_H
