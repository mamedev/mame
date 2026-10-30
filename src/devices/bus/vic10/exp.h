// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-10 Expansion Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       _UPROM
                    +5V       3      C       _RESET
                   _IRQ       4      D       _NMI
                  _CR/W       5      E       Sphi2
                     SP       6      F       CA15
                 _EXRAM       7      H       CA14
                    CNT       8      J       CA13
                   _CIA       9      K       CA12
               _CIA PLA      10      L       CA11
                 _LOROM      11      M       CA10
                     BA      12      N       CA9
               R/_W PLA      13      P       CA8
                    CD7      14      R       CA7
                    CD6      15      S       CA6
                    CD5      16      T       CA5
                    CD4      17      U       CA4
                    CD3      18      V       CA3
                    CD2      19      W       CA2
                    CD1      20      X       CA1
                    CD0      21      Y       CA0
                     P2      22      Z       GND

**********************************************************************/

#ifndef MAME_BUS_VIC10_EXP_H
#define MAME_BUS_VIC10_EXP_H

#pragma once

#include "imagedev/cartrom.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic10_expansion_window

class vic10_expansion_window
{
public:
	class variant
	{
	public:
		void install_rom(offs_t start, offs_t end, void *baseptr) { install(start, end, baseptr, false); }
		void install_ram(offs_t start, offs_t end, void *baseptr) { install(start, end, baseptr, true); }

		template <typename W> void install_write_handler(offs_t start, offs_t end, W &&whandler)
		{ m_window.m_view[m_slot].install_write_handler(m_window.m_start + start, m_window.m_start + end, std::forward<W>(whandler)); }

	private:
		friend class vic10_expansion_window;

		variant(vic10_expansion_window &window, int slot) : m_window(window), m_slot(slot) { }

		void install(offs_t start, offs_t end, void *baseptr, bool writable);

		vic10_expansion_window &m_window;
		int const m_slot;
	};

	vic10_expansion_window(device_t &device, const char *name, offs_t start, offs_t end);
	vic10_expansion_window(device_t &device, const char *name, offs_t start, offs_t end, offs_t video_start, offs_t video_end);

	variant operator[](int slot) { return variant(*this, slot); }

	void select(int slot);
	void unmap();

	void install_rom(offs_t start, offs_t end, void *baseptr) { (*this)[0].install_rom(start, end, baseptr); select(0); }
	void install_ram(offs_t start, offs_t end, void *baseptr) { (*this)[0].install_ram(start, end, baseptr); select(0); }
	template <typename W> void install_write_handler(offs_t start, offs_t end, W &&whandler) { (*this)[0].install_write_handler(start, end, std::forward<W>(whandler)); select(0); }

private:
	friend class vic10_expansion_slot_device;

	void install_views(address_space &program, address_space *video);

	memory_view m_view;
	memory_view m_video_view;
	offs_t const m_start;
	offs_t const m_end;
	offs_t const m_video_start;
	offs_t const m_video_end;
	bool const m_has_video;
	bool m_video_installed;
};


// ======================> vic10_expansion_slot_device

class device_vic10_expansion_card_interface;

class vic10_expansion_slot_device : public device_t,
									public device_single_card_slot_interface<device_vic10_expansion_card_interface>,
									public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	vic10_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: vic10_expansion_slot_device(mconfig, tag, owner, clock)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}
	vic10_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_video_space(T &&tag, int spacenum) { m_video.set_tag(std::forward<T>(tag), spacenum); }

	auto irq_callback() { return m_write_irq.bind(); }
	auto res_callback() { return m_write_res.bind(); }
	auto cnt_callback() { return m_write_cnt.bind(); }
	auto sp_callback() { return m_write_sp.bind(); }

	// computer interface
	int p0_r();
	void p0_w(int state);

	// cartridge interface
	vic10_expansion_window &exram() { return m_exram; }
	vic10_expansion_window &lorom() { return m_lorom; }
	vic10_expansion_window &uprom() { return m_uprom; }

	void irq_w(int state) { m_write_irq(state); }
	void res_w(int state) { m_write_res(state); }
	void cnt_w(int state) { m_write_cnt(state); }
	void sp_w(int state) { m_write_sp(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "vic10_cart"; }
	virtual const char *file_extensions() const noexcept override { return "80,e0"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	uint8_t *alloc_region(const char *tag);
	std::error_condition load_region(util::random_read &file, const char *tag, offs_t offset, size_t length);

	optional_address_space m_program;
	optional_address_space m_video;

	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_res;
	devcb_write_line   m_write_cnt;
	devcb_write_line   m_write_sp;

	device_vic10_expansion_card_interface *m_card;

private:
	vic10_expansion_window m_exram;
	vic10_expansion_window m_lorom;
	vic10_expansion_window m_uprom;
};


// ======================> device_vic10_expansion_card_interface

// class representing interface-specific live vic10_expansion card
class device_vic10_expansion_card_interface : public device_interface
{
	friend class vic10_expansion_slot_device;

public:
	// construction/destruction
	virtual ~device_vic10_expansion_card_interface();

	virtual int vic10_p0_r() { return 0; }
	virtual void vic10_p0_w(int state) { }
	virtual void vic10_sp_w(int state) { }
	virtual void vic10_cnt_w(int state) { }

protected:
	device_vic10_expansion_card_interface(const machine_config &mconfig, device_t &device);

	vic10_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC10_EXPANSION_SLOT, vic10_expansion_slot_device)


void vic10_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_VIC10_EXP_H
