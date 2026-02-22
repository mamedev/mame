// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Nokia MikroMikko 2 expansion bus

**********************************************************************

                              A     C
                +5 V    ---   *  1  *   --- +5 V
                +5 V    ---   *  2  *   --- +BATT
                +12 V   ---   *  3  *   --- -12 V
                GND     ---   *  4  *   --- GND
                C8M     ---   *  5  *   --- C4M
                RESET   ---   *  6  *   --- NMI
                IR2     ---   *  7  *   --- IR3
                IR4     ---   *  8  *   --- IR5
                IR6     ---   *  9  *   ---
                INTA    ---   * 10  *   --- BCAS0
                BCAS1   ---   * 11  *   --- BCAS2
                BHLDA   ---   * 12  *   --- HOLD1
                HOLD2   ---   * 13  *   --- HOLD3
                HOLD4   ---   * 14  *   --- HOLD5
                BCS3    ---   * 15  *   --- BCS4
                BCS5    ---   * 16  *   --- BCS6
                AMEMR   ---   * 17  *   ---
                MEMR    ---   * 18  *   --- IOR
                MEMW    ---   * 19  *   --- IOW
                AEN     ---   * 20  *   --- READY
                AST     ---   * 21  *   --- BBHE
                BAD19   ---   * 22  *   --- BAD18
                BAD17   ---   * 23  *   --- BAD16
                BAD15   ---   * 24  *   --- BAD7
                BAD14   ---   * 25  *   --- BAD6
                BAD13   ---   * 26  *   --- BAD5
                BAD12   ---   * 27  *   --- BAD4
                BAD11   ---   * 28  *   --- BAD3
                BAD10   ---   * 29  *   --- BAD2
                BAD9    ---   * 30  *   --- BAD1
                BAD8    ---   * 31  *   --- BAD0
                GND     ---   * 32  *   --- GND

**********************************************************************/

#ifndef MAME_BUS_MM2_EXP_H
#define MAME_BUS_MM2_EXP_H

#pragma once

class mikromikko2_expansion_bus_device;

class mikromikko2_expansion_bus_slot_device : public device_t, public device_slot_interface
{
public:
	template <typename T, typename U>
	mikromikko2_expansion_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&bus_tag, U &&opts, const char *dflt, bool fixed)
		: mikromikko2_expansion_bus_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
		m_bus.set_tag(std::forward<T>(bus_tag));
	}
	mikromikko2_expansion_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	mikromikko2_expansion_bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	required_device<mikromikko2_expansion_bus_device> m_bus;
};

DECLARE_DEVICE_TYPE(MIKROMIKKO2_EXPANSION_BUS_SLOT, mikromikko2_expansion_bus_slot_device)

class device_mikromikko2_expansion_bus_card_interface;

class mikromikko2_expansion_bus_device : public device_t
{
public:
	mikromikko2_expansion_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_memspace(T &&tag, int spacenum) { m_memspace.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }
	auto nmi_callback() { return m_out_nmi_cb.bind(); }
	auto ir2_callback() { return m_out_ir2_cb.bind(); }
	auto ir3_callback() { return m_out_ir3_cb.bind(); }
	auto ir4_callback() { return m_out_ir4_cb.bind(); }
	auto ir5_callback() { return m_out_ir5_cb.bind(); }
	auto ir6_callback() { return m_out_ir6_cb.bind(); }
	auto hold1_callback() { return m_out_hold1_cb.bind(); }
	auto hold2_callback() { return m_out_hold2_cb.bind(); }
	auto hold3_callback() { return m_out_hold3_cb.bind(); }
	auto hold4_callback() { return m_out_hold4_cb.bind(); }
	auto hold5_callback() { return m_out_hold5_cb.bind(); }

	void nmi_w(int state) { m_out_nmi_cb(state); };

	void ir2_w(int state) { m_out_ir2_cb(state); };
	void ir3_w(int state) { m_out_ir3_cb(state); };
	void ir4_w(int state) { m_out_ir4_cb(state); };
	void ir5_w(int state) { m_out_ir5_cb(state); };
	void ir6_w(int state) { m_out_ir6_cb(state); };

	void hold1_w(int state) { m_out_hold1_cb(state); }
	void hold2_w(int state) { m_out_hold2_cb(state); }
	void hold3_w(int state) { m_out_hold3_cb(state); }
	void hold4_w(int state) { m_out_hold4_cb(state); }
	void hold5_w(int state) { m_out_hold5_cb(state); }

	address_space &memspace() const { return *m_memspace; }
	address_space &iospace() const { return *m_iospace; }

protected:
	mikromikko2_expansion_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	required_address_space m_memspace;
	required_address_space m_iospace;

	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_ir2_cb;
	devcb_write_line    m_out_ir3_cb;
	devcb_write_line    m_out_ir4_cb;
	devcb_write_line    m_out_ir5_cb;
	devcb_write_line    m_out_ir6_cb;
	devcb_write_line    m_out_hold1_cb;
	devcb_write_line    m_out_hold2_cb;
	devcb_write_line    m_out_hold3_cb;
	devcb_write_line    m_out_hold4_cb;
	devcb_write_line    m_out_hold5_cb;
};

DECLARE_DEVICE_TYPE(MIKROMIKKO2_EXPANSION_BUS, mikromikko2_expansion_bus_device)

class device_mikromikko2_expansion_bus_card_interface : public device_interface
{
	friend class mikromikko2_expansion_bus_device;

public:
	device_mikromikko2_expansion_bus_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_mikromikko2_expansion_bus_card_interface() {}

	void set_bus(mikromikko2_expansion_bus_device *bus) { m_bus = bus; }

	mikromikko2_expansion_bus_device *m_bus;
	device_t *m_card;
};

void mikromikko2_expansion_bus_cards(device_slot_interface &device);

#endif // MAME_BUS_MM2_EXP_H
