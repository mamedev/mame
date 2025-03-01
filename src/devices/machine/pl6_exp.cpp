// license:BSD-3-Clause
// copyright-holders:NaokiS
/*
 * Heber Pluto 6 Expansion bus
 *
 */

#include "emu.h"
#include "pl6_exp.h"

#include "screen.h"

pluto6_expansion_card_interface::pluto6_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "pl6_exp")
{
	m_slot = dynamic_cast<pluto6_expansion_slot_device *>(device.owner());
}


DEFINE_DEVICE_TYPE(HEBER_CALYPSO_GPU, pluto6_calypso32_device, "pl6gpu", "Heber Calypso32 GPU")

pluto6_calypso32_device::pluto6_calypso32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pluto6_calypso32_device(mconfig, HEBER_CALYPSO_GPU, tag, owner, clock){
}

pluto6_calypso32_device::pluto6_calypso32_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	pluto6_expansion_card_interface(mconfig, *this),
	m_gpu(*this, "cremson"),
	m_vram(*this, "vram")
{
}

void pluto6_calypso32_device::device_add_mconfig(machine_config &config) {
	screen_device &m_screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	m_screen.set_raw(XTAL(14'318'181), 608, 0, 480, 262, 0, 234);
	m_screen.set_screen_update("cremson", FUNC(mb86292_device::screen_update));

	// Configured as FCRAM 16MBit with 8MB / 64-bit data bus thru MMR register
	RAM(config, m_vram);
	m_vram->set_default_size("8M");
	m_vram->set_default_value(0);
    
    MB86292(config, m_gpu, XTAL(14'318'181));
	m_gpu->set_vram("vram");
	m_gpu->set_screen(m_screen);
}

DEFINE_DEVICE_TYPE(PLUTO6_EXPANSION_SLOT, pluto6_expansion_slot_device, "pl6expslot", "Heber Pluto 6 Expansion Slot")

pluto6_expansion_slot_device::pluto6_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PLUTO6_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<pluto6_expansion_card_interface>(mconfig, *this)
{
}

void pluto6_expansion_slot_device::device_start()
{
	m_dev = get_card_device();
}
