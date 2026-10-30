// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-20 Expansion Port emulation

**********************************************************************

                    GND       1      A       GND
                    CD0       2      B       CA0
                    CD1       3      C       CA1
                    CD2       4      D       CA2
                    CD3       5      E       CA3
                    CD4       6      F       CA4
                    CD5       7      H       CA5
                    CD6       8      J       CA6
                    CD7       9      K       CA7
                  _BLK1      10      L       CA8
                  _BLK2      11      M       CA9
                  _BLK3      12      N       CA10
                  _BLK5      13      P       CA11
                  _RAM1      14      R       CA12
                  _RAM2      15      S       CA13
                  _RAM3      16      T       _I/O2
                  VR/_W      17      U       _I/O3
                  CR/_W      18      V       Sphi2
                   _IRQ      19      W       _NMI
                   N.C.      20      X       _RES
                    +5V      21      Y       N.C.
                    GND      22      Z       GND

**********************************************************************/

#ifndef MAME_BUS_VIC20_EXP_H
#define MAME_BUS_VIC20_EXP_H

#pragma once

#include "imagedev/cartrom.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_expansion_window

class vic20_expansion_window
{
public:
	class variant
	{
	public:
		void install_rom(offs_t start, offs_t end, void *baseptr) { install_rom(start, end, 0, baseptr); }
		void install_rom(offs_t start, offs_t end, offs_t mirror, void *baseptr)
		{ install([&] (address_space_installer &space, offs_t base) { space.install_rom(base + start, base + end, mirror, baseptr); }); }

		void install_writeonly(offs_t start, offs_t end, void *baseptr) { install_writeonly(start, end, 0, baseptr); }
		void install_writeonly(offs_t start, offs_t end, offs_t mirror, void *baseptr)
		{ install([&] (address_space_installer &space, offs_t base) { space.install_writeonly(base + start, base + end, mirror, baseptr); }); }

		void install_ram(offs_t start, offs_t end, void *baseptr) { install_ram(start, end, 0, baseptr); }
		void install_ram(offs_t start, offs_t end, offs_t mirror, void *baseptr)
		{ install([&] (address_space_installer &space, offs_t base) { space.install_ram(base + start, base + end, mirror, baseptr); }); }

		void install_read_bank(offs_t start, offs_t end, memory_bank *bank) { install_read_bank(start, end, 0, bank); }
		void install_read_bank(offs_t start, offs_t end, offs_t mirror, memory_bank *bank)
		{ install([&] (address_space_installer &space, offs_t base) { space.install_read_bank(base + start, base + end, mirror, bank); }); }

		void install_readwrite_bank(offs_t start, offs_t end, memory_bank *bank) { install_readwrite_bank(start, end, 0, bank); }
		void install_readwrite_bank(offs_t start, offs_t end, offs_t mirror, memory_bank *bank)
		{ install([&] (address_space_installer &space, offs_t base) { space.install_readwrite_bank(base + start, base + end, mirror, bank); }); }

		template <typename R> void install_read_handler(offs_t start, offs_t end, R &&rhandler) { install_read_handler(start, end, 0, std::forward<R>(rhandler)); }
		template <typename R> void install_read_handler(offs_t start, offs_t end, offs_t mirror, R &&rhandler)
		{ install([&] (address_space_installer &space, offs_t base) { space.install_read_handler(base + start, base + end, 0, mirror, 0, rhandler); }); }

		template <typename W> void install_write_handler(offs_t start, offs_t end, W &&whandler) { install_write_handler(start, end, 0, std::forward<W>(whandler)); }
		template <typename W> void install_write_handler(offs_t start, offs_t end, offs_t mirror, W &&whandler)
		{ install([&] (address_space_installer &space, offs_t base) { space.install_write_handler(base + start, base + end, 0, mirror, 0, whandler); }); }

		template <typename R, typename W> void install_readwrite_handler(offs_t start, offs_t end, R &&rhandler, W &&whandler) { install_readwrite_handler(start, end, 0, std::forward<R>(rhandler), std::forward<W>(whandler)); }
		template <typename R, typename W> void install_readwrite_handler(offs_t start, offs_t end, offs_t mirror, R &&rhandler, W &&whandler)
		{ install([&] (address_space_installer &space, offs_t base) { space.install_readwrite_handler(base + start, base + end, 0, mirror, 0, rhandler, whandler); }); }

	private:
		friend class vic20_expansion_window;

		variant(vic20_expansion_window &window, int slot) : m_window(window), m_slot(slot) { }

		template <typename F> void install(F &&f)
		{
			f(m_window.m_view[m_slot], m_window.m_start);

			if (m_window.m_video_installed)
				f(m_window.m_video_view[m_slot], m_window.m_video_start);
		}

		vic20_expansion_window &m_window;
		int const m_slot;
	};

	vic20_expansion_window(device_t &device, const char *name, offs_t start, offs_t end);
	vic20_expansion_window(device_t &device, const char *name, offs_t start, offs_t end, offs_t video_start);

	variant operator[](int slot) { return variant(*this, slot); }

	void select(int slot);
	void unmap();

	template <typename... T> void install_rom(T &&... args) { (*this)[0].install_rom(std::forward<T>(args)...); select(0); }
	template <typename... T> void install_writeonly(T &&... args) { (*this)[0].install_writeonly(std::forward<T>(args)...); select(0); }
	template <typename... T> void install_ram(T &&... args) { (*this)[0].install_ram(std::forward<T>(args)...); select(0); }
	template <typename... T> void install_read_bank(T &&... args) { (*this)[0].install_read_bank(std::forward<T>(args)...); select(0); }
	template <typename... T> void install_readwrite_bank(T &&... args) { (*this)[0].install_readwrite_bank(std::forward<T>(args)...); select(0); }
	template <typename... T> void install_read_handler(T &&... args) { (*this)[0].install_read_handler(std::forward<T>(args)...); select(0); }
	template <typename... T> void install_write_handler(T &&... args) { (*this)[0].install_write_handler(std::forward<T>(args)...); select(0); }
	template <typename... T> void install_readwrite_handler(T &&... args) { (*this)[0].install_readwrite_handler(std::forward<T>(args)...); select(0); }

private:
	friend class vic20_expansion_slot_device;

	void install_views(address_space &program, address_space *video);

	memory_view m_view;
	memory_view m_video_view;
	offs_t const m_start;
	offs_t const m_end;
	offs_t const m_video_start;
	bool const m_has_video;
	bool m_video_installed;
};


// ======================> vic20_expansion_slot_device

class device_vic20_expansion_card_interface;

class vic20_expansion_slot_device : public device_t,
									public device_single_card_slot_interface<device_vic20_expansion_card_interface>,
									public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	vic20_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: vic20_expansion_slot_device(mconfig, tag, owner, clock)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}
	vic20_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void add_passthrough(machine_config &config, const char *tag);

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_video_space(T &&tag, int spacenum) { m_video.set_tag(std::forward<T>(tag), spacenum); }

	auto irq_wr_callback() { return m_write_irq.bind(); }
	auto nmi_wr_callback() { return m_write_nmi.bind(); }
	auto res_wr_callback() { return m_write_res.bind(); }

	// cartridge interface
	vic20_expansion_window &ram1() { return m_root->m_ram1; }
	vic20_expansion_window &ram2() { return m_root->m_ram2; }
	vic20_expansion_window &ram3() { return m_root->m_ram3; }
	vic20_expansion_window &blk1() { return m_root->m_blk1; }
	vic20_expansion_window &blk2() { return m_root->m_blk2; }
	vic20_expansion_window &blk3() { return m_root->m_blk3; }
	vic20_expansion_window &blk5() { return m_root->m_blk5; }
	vic20_expansion_window &io2() { return m_root->m_io2; }
	vic20_expansion_window &io3() { return m_root->m_io3; }

	void irq_w(int state) { m_write_irq(state); }
	void nmi_w(int state) { m_write_nmi(state); }
	void res_w(int state) { m_write_res(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "vic1001_cart"; }
	virtual const char *file_extensions() const noexcept override { return "20,40,60,70,a0,b0,crt"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	std::error_condition load_region(util::random_read &file, const char *tag, offs_t offset, size_t length);

	optional_address_space m_program;
	optional_address_space m_video;

	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_nmi;
	devcb_write_line   m_write_res;

	device_vic20_expansion_card_interface *m_card;

private:
	vic20_expansion_slot_device *find_root(device_t *owner);

	vic20_expansion_slot_device *const m_root;

	vic20_expansion_window m_ram1;
	vic20_expansion_window m_ram2;
	vic20_expansion_window m_ram3;
	vic20_expansion_window m_blk1;
	vic20_expansion_window m_blk2;
	vic20_expansion_window m_blk3;
	vic20_expansion_window m_blk5;
	vic20_expansion_window m_io2;
	vic20_expansion_window m_io3;
};


// ======================> device_vic20_expansion_card_interface

// class representing interface-specific live vic20_expansion card
class device_vic20_expansion_card_interface : public device_interface
{
	friend class vic20_expansion_slot_device;

public:
	// construction/destruction
	virtual ~device_vic20_expansion_card_interface();

protected:
	device_vic20_expansion_card_interface(const machine_config &mconfig, device_t &device);

	vic20_expansion_slot_device *m_slot;

private:
	void set_slot(vic20_expansion_slot_device &slot) { m_slot = &slot; }
};


// device type definition
DECLARE_DEVICE_TYPE(VIC20_EXPANSION_SLOT, vic20_expansion_slot_device)


void vic20_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_VIC20_EXP_H
