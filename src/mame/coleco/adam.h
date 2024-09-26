// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef MAME_COLECO_ADAM_H
#define MAME_COLECO_ADAM_H

#include "bus/adam/exp.h"
#include "bus/adamnet/adamnet.h"
#include "bus/coleco/controller/ctrl.h"
#include "bus/coleco/cartridge/exp.h"
#include "cpu/z80/z80.h"
#include "cpu/m6800/m6801.h"
#include "coleco_m.h"
#include "machine/ram.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"

#define Z80_TAG         "u1"
#define M6801_TAG       "u6"
#define SN76489A_TAG    "u20"
#define TMS9928A_TAG    "tms9928a"
#define SCREEN_TAG      "screen"
#define CONTROL1_TAG    "joy1"
#define CONTROL2_TAG    "joy2"

class adam_state : public driver_device
{
public:
	adam_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_netcpu(*this, M6801_TAG),
		m_vdc(*this, TMS9928A_TAG),
		m_psg(*this, SN76489A_TAG),
		m_ram(*this, RAM_TAG),
		m_adamnet(*this, "adamnet"),
		m_slot(*this, "slot%u", 1U),
		m_cart(*this, COLECOVISION_CARTRIDGE_SLOT_TAG),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG),
		m_boot_rom(*this, "boot"),
		m_os7_rom(*this, "os7"),
		m_mioc(0),
		m_game(0),
		m_an(0),
		m_dma(1),
		m_bwr(1),
		m_spindis(1)
	{ }

	void adam(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<m6801_cpu_device> m_netcpu;
	required_device<tms9928a_device> m_vdc;
	required_device<sn76489a_device> m_psg;
	required_device<ram_device> m_ram;
	required_device<adamnet_device> m_adamnet;
	required_device_array<adam_expansion_slot_device, 3> m_slot;
	required_device<colecovision_cartridge_slot_device> m_cart;
	required_device<colecovision_control_port_device> m_joy1;
	required_device<colecovision_control_port_device> m_joy2;
	required_memory_region m_boot_rom;
	required_memory_region m_os7_rom;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t mreq_r(offs_t offset);
	void mreq_w(offs_t offset, uint8_t data);
	uint8_t iorq_r(offs_t offset);
	void iorq_w(offs_t offset, uint8_t data);

	uint8_t adamnet_r();
	void adamnet_w(uint8_t data);
	uint8_t mioc_r();
	void mioc_w(uint8_t data);

	void m6801_p1_w(uint8_t data);
	uint8_t m6801_p2_r();
	void m6801_p2_w(uint8_t data);
	uint8_t m6801_p3_r();
	void m6801_p3_w(uint8_t data);
	void m6801_p4_w(uint8_t data);

	void vdc_int_w(int state);

	void os3_w(int state);

	void joy1_irq_w(int state);
	void joy2_irq_w(int state);

	// memory state
	uint8_t m_mioc;
	int m_game;

	// ADAMnet state
	uint8_t m_an;

	// DMA state
	uint16_t m_ba = 0U;
	int m_dma;
	int m_bwr;
	uint8_t m_data_in = 0U;
	uint8_t m_data_out = 0U;

	// paddle state
	int m_spindis;

	// video state
	int m_vdp_nmi = 0;
	void adam_io(address_map &map) ATTR_COLD;
	void adam_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_COLECO_ADAM_H
