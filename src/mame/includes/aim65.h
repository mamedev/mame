// license:GPL-2.0+
// copyright-holders:Peter Trauner, Dan Boris, Dirk Best, Robbbert
/*****************************************************************************
 *
 * includes/aim65.h
 *
 * Rockwell AIM-65
 *
 ****************************************************************************/

#ifndef AIM65_H_
#define AIM65_H_

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/dl1416.h"
#include "machine/6522via.h"
#include "machine/mos6530n.h"
#include "machine/6821pia.h"
#include "machine/ram.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


/** R6502 Clock.
 *
 * The R6502 on AIM65 operates at 1 MHz. The frequency reference is a 4 MHz
 * crystal controlled oscillator. Dual D-type flip-flop Z10 divides the 4 MHz
 * signal by four to drive the R6502 phase 0 (O0) input with a 1 MHz clock.
 */
#define AIM65_CLOCK  XTAL_4MHz/4


class aim65_state : public driver_device
{
public:
	aim65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette1(*this, "cassette"),
		m_cassette2(*this, "cassette2"),
		m_z24(*this, "z24"),
		m_z25(*this, "z25"),
		m_z26(*this, "z26"),
		m_ram(*this, RAM_TAG),
		m_ds1(*this, "ds1"),
		m_ds2(*this, "ds2"),
		m_ds3(*this, "ds3"),
		m_ds4(*this, "ds4"),
		m_ds5(*this, "ds5")
	{ }

	void aim65_pia_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aim65_pia_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t aim65_riot_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void aim65_riot_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void aim65_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t aim65_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t m_pia_a;
	uint8_t m_pia_b;
	uint8_t m_riot_port_a;
	uint8_t m_pb_save;

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	required_device<generic_slot_device> m_z24;
	required_device<generic_slot_device> m_z25;
	required_device<generic_slot_device> m_z26;
	required_device<ram_device> m_ram;
	required_device<dl1416_device> m_ds1;
	required_device<dl1416_device> m_ds2;
	required_device<dl1416_device> m_ds3;
	required_device<dl1416_device> m_ds4;
	required_device<dl1416_device> m_ds5;
	virtual void machine_start() override;
	void aim65_pia();

	void aim65_update_ds1(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aim65_update_ds2(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aim65_update_ds3(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aim65_update_ds4(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void aim65_update_ds5(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void dl1416_update(dl1416_device *device, int index);

	image_init_result load_cart(device_image_interface &image, generic_slot_device *slot, const char *slot_tag);
	image_init_result device_image_load_z24_load(device_image_interface &image) { return load_cart(image, m_z24, "z24"); }
	image_init_result device_image_load_z25_load(device_image_interface &image) { return load_cart(image, m_z25, "z25"); }
	image_init_result device_image_load_z26_load(device_image_interface &image) { return load_cart(image, m_z26, "z26"); }
};


#endif /* AIM65_H_ */
