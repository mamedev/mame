// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "stcart.h"

#include "replay.h"

DEFINE_DEVICE_TYPE(STCART_CONNECTOR, stcart_connector, "stcart_connector", "Atari ST cartridge connector")

stcart_connector::stcart_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    device_t(mconfig, STCART_CONNECTOR, tag, owner, clock),
    device_single_card_slot_interface<device_stcart_interface>(mconfig, *this)
{
}

void stcart_connector::device_start()
{
}

void stcart_connector::map(address_space_installer &space)
{
  auto card = get_card_device();
  if(card)
    card->map(space);
}

void stcart_intf(device_slot_interface &device)
{
  device.option_add("replay", ST_REPLAY);
}

device_stcart_interface::device_stcart_interface(const machine_config &mconfig, device_t &device) :
  device_interface(device, "stcart")
{
}

device_stcart_interface::~device_stcart_interface()
{
}
