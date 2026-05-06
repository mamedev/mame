// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_SHARED_MAHJONG_H
#define MAME_SHARED_MAHJONG_H

#pragma once


class device_mahjong_panel_interface : public device_interface
{
public:
	virtual ~device_mahjong_panel_interface();

	virtual u8 read(u8 select) = 0;

protected:
	device_mahjong_panel_interface(machine_config const &mconfig, device_t &device);
};


class mahjong_panel_connector_device : public device_t, public device_single_card_slot_interface<device_mahjong_panel_interface>
{
public:
	template <typename T>
	mahjong_panel_connector_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			T &&opts,
			char const *dflt,
			bool const fixed)
		: mahjong_panel_connector_device(mconfig, tag, owner, u32(0))
	{
		set_options(std::forward<T>(opts), dflt, fixed);
	}
	mahjong_panel_connector_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual ~mahjong_panel_connector_device();

	u8 read(u8 select) { return m_panel ? m_panel->read(select) : 0x3f; }

	static void standard_panels(device_slot_interface &device) ATTR_COLD;
	static void mahjong_panels(device_slot_interface &device) ATTR_COLD;
	static void hanafuda_panels(device_slot_interface &device) ATTR_COLD;
	static void medal_panels(device_slot_interface &device) ATTR_COLD;
	static void amusement_panels(device_slot_interface &device) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;

private:
	device_mahjong_panel_interface *m_panel;
};


class mahjong_panel_device_base : public device_t, public device_mahjong_panel_interface
{
public:
	virtual u8 read(u8 select) override;

protected:
	mahjong_panel_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

private:
	required_ioport_array<6> m_keys;
};


DECLARE_DEVICE_TYPE(MAHJONG_PANEL_CONNECTOR, mahjong_panel_connector_device)

DECLARE_DEVICE_TYPE(MAHJONG_PANEL,        device_mahjong_panel_interface)
DECLARE_DEVICE_TYPE(MAHJONG_MEDAL_PANEL,  device_mahjong_panel_interface)
DECLARE_DEVICE_TYPE(HANAFUDA_PANEL,       device_mahjong_panel_interface)
DECLARE_DEVICE_TYPE(HANAFUDA_MEDAL_PANEL, device_mahjong_panel_interface)
DECLARE_DEVICE_TYPE(HANAROKU_PANEL,       device_mahjong_panel_interface)


INPUT_PORTS_EXTERN(mahjong_matrix_1p);          // letters, start, kan/pon/chi/reach/ron
INPUT_PORTS_EXTERN(mahjong_matrix_1p_ff);       // adds flip flop
INPUT_PORTS_EXTERN(mahjong_matrix_1p_bet);      // adds bet/last chance
INPUT_PORTS_EXTERN(mahjong_matrix_1p_bet_wup);  // adds take score/double up/big/small

INPUT_PORTS_EXTERN(mahjong_matrix_2p);          // letters, start, kan/pon/chi/reach/ron
INPUT_PORTS_EXTERN(mahjong_matrix_2p_ff);       // adds flip flop
INPUT_PORTS_EXTERN(mahjong_matrix_2p_bet);      // adds bet/last chance
INPUT_PORTS_EXTERN(mahjong_matrix_2p_bet_wup);  // adds take score/double up/big/small

#endif // MAME_SHARED_MAHJONG_H
