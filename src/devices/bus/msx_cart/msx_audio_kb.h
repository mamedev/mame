// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_AUDIO_KB_H__
#define __MSX_AUDIO_KB_H__


extern const device_type MSX_AUDIO_KBDC_PORT;


#define MCFG_MSX_AUDIO_KBDC_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, MSX_AUDIO_KBDC_PORT, 0 ) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


class msx_audio_kb_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	msx_audio_kb_port_interface(const machine_config &mconfig, device_t &device)
		: device_slot_card_interface(mconfig, device)
	{ };

	virtual uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return 0xff; };
	virtual void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { };
};


class msx_audio_kbdc_port_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	msx_audio_kbdc_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// Physical connection simply consists of 8 input and 8 output lines split across 2 connectors
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	msx_audio_kb_port_interface *m_keyboard;
};


SLOT_INTERFACE_EXTERN( msx_audio_keyboards );


#endif
