// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 Expansion Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       C1 LOW
                    +5V       3      C       _BRESET
                   _IRQ       4      D       _RAS
                   R/_W       5      E       phi0
                C1 HIGH       6      F       A15
                 C2 LOW       7      H       A14
                C2 HIGH       8      J       A13
                   _CS1       9      K       A12
                   _CS0      10      L       A11
                   _CAS      11      M       A10
                    MUX      12      N       A9
                     BA      13      P       A8
                     D7      14      R       A7
                     D6      15      S       A6
                     D5      16      T       A5
                     D4      17      U       A4
                     D3      18      V       A3
                     D2      19      W       A2
                     D1      20      X       A1
                     D0      21      Y       A0
                    AEC      22      Z       N.C. (RAMEN)
              EXT AUDIO      23      AA      N.C.
                   phi2      24      BB      N.C.
                    GND      25      CC      GND

**********************************************************************/

#ifndef MAME_BUS_PLUS4_EXP_H
#define MAME_BUS_PLUS4_EXP_H

#pragma once

#include "imagedev/cartrom.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> plus4_expansion_window

class plus4_expansion_window
{
public:
	class variant
	{
	public:
		void install_rom(offs_t start, offs_t end, void *baseptr) { install_rom(start, end, 0, baseptr); }
		void install_rom(offs_t start, offs_t end, offs_t mirror, void *baseptr);

		template <typename R> void install_read_handler(offs_t start, offs_t end, R &&rhandler) { install_read_handler(start, end, 0, std::forward<R>(rhandler)); }
		template <typename R> void install_read_handler(offs_t start, offs_t end, offs_t mirror, R &&rhandler)
		{ m_window.m_view[m_slot].install_read_handler(m_window.m_start + start, m_window.m_start + end, 0, mirror, 0, rhandler); }

		template <typename W> void install_write_handler(offs_t start, offs_t end, W &&whandler) { install_write_handler(start, end, 0, std::forward<W>(whandler)); }
		template <typename W> void install_write_handler(offs_t start, offs_t end, offs_t mirror, W &&whandler)
		{ m_window.m_view[m_slot].install_write_handler(m_window.m_start + start, m_window.m_start + end, 0, mirror, 0, whandler); }

		template <typename R, typename W> void install_readwrite_handler(offs_t start, offs_t end, R &&rhandler, W &&whandler) { install_readwrite_handler(start, end, 0, std::forward<R>(rhandler), std::forward<W>(whandler)); }
		template <typename R, typename W> void install_readwrite_handler(offs_t start, offs_t end, offs_t mirror, R &&rhandler, W &&whandler)
		{ m_window.m_view[m_slot].install_readwrite_handler(m_window.m_start + start, m_window.m_start + end, 0, mirror, 0, rhandler, whandler); }

	private:
		friend class plus4_expansion_window;

		variant(plus4_expansion_window &window, int slot) : m_window(window), m_slot(slot) { }

		void install_rom_segment(offs_t start, offs_t end, uint8_t *base);

		plus4_expansion_window &m_window;
		int const m_slot;
	};

	plus4_expansion_window(device_t &device, const char *name, offs_t start, offs_t end);
	plus4_expansion_window(device_t &device, const char *name, offs_t start, offs_t end, offs_t video_start);
	plus4_expansion_window(device_t &device, const char *name, offs_t start, offs_t end, offs_t video_start, offs_t hole_start, offs_t hole_end);

	variant operator[](int slot) { return variant(*this, slot); }

	void select(int slot);
	void unmap();

	template <typename... T> void install_rom(T &&... args) { (*this)[0].install_rom(std::forward<T>(args)...); select(0); }
	template <typename... T> void install_read_handler(T &&... args) { (*this)[0].install_read_handler(std::forward<T>(args)...); select(0); }
	template <typename... T> void install_write_handler(T &&... args) { (*this)[0].install_write_handler(std::forward<T>(args)...); select(0); }
	template <typename... T> void install_readwrite_handler(T &&... args) { (*this)[0].install_readwrite_handler(std::forward<T>(args)...); select(0); }

	void install_views(address_space_installer &program, address_space_installer *video = nullptr);

private:
	memory_view m_view;
	memory_view m_video_view;
	offs_t const m_start;
	offs_t const m_end;
	offs_t const m_video_start;
	offs_t const m_hole_start;
	offs_t const m_hole_end;
	bool const m_has_video;
	bool const m_has_hole;
	bool m_video_installed;
};


// ======================> plus4_expansion_slot_device

class device_plus4_expansion_card_interface;

class plus4_expansion_slot_device : public device_t,
									public device_single_card_slot_interface<device_plus4_expansion_card_interface>,
									public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	plus4_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: plus4_expansion_slot_device(mconfig, tag, owner, clock)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}
	plus4_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void add_passthrough(machine_config &config, const char *tag);

	auto irq_wr_callback() { return m_write_irq.bind(); }
	auto aec_wr_callback() { return m_write_aec.bind(); }

	// cartridge interface
	plus4_expansion_window &c1l() { return m_root->m_c1l; }
	plus4_expansion_window &c1h() { return m_root->m_c1h; }
	plus4_expansion_window &c2l() { return m_root->m_c2l; }
	plus4_expansion_window &c2h() { return m_root->m_c2h; }
	plus4_expansion_window &io() { return m_root->m_io; }

	void irq_w(int state) { m_write_irq(state); }
	void aec_w(int state) { m_write_aec(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "plus4_cart"; }
	virtual const char *file_extensions() const noexcept override { return "rom,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_aec;

	device_plus4_expansion_card_interface *m_card;

private:
	plus4_expansion_slot_device *find_root(device_t *owner);

	plus4_expansion_slot_device *const m_root;

	plus4_expansion_window m_c1l;
	plus4_expansion_window m_c1h;
	plus4_expansion_window m_c2l;
	plus4_expansion_window m_c2h;
	plus4_expansion_window m_io;
};


// ======================> device_plus4_expansion_card_interface

class device_plus4_expansion_card_interface : public device_interface
{
	friend class plus4_expansion_slot_device;

public:
	// construction/destruction
	virtual ~device_plus4_expansion_card_interface();

protected:
	device_plus4_expansion_card_interface(const machine_config &mconfig, device_t &device);

	plus4_expansion_slot_device *m_slot;

private:
	void set_slot(plus4_expansion_slot_device &slot) { m_slot = &slot; }
};


// device type declaration
DECLARE_DEVICE_TYPE(PLUS4_EXPANSION_SLOT, plus4_expansion_slot_device)


void plus4_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_PLUS4_EXP_H
