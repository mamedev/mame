// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANV02 Music 500

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANV02_Music500.html

    Hybrid Music 5000 Synthesiser

    https://www.retro-kit.co.uk/page.cfm/content/Hybrid-Music-5000-Synthesiser/
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Hybrid_Music5000.html

    Hybrid Music 3000 Expander

    https://www.retro-kit.co.uk/page.cfm/content/Hybrid-Music-3000-Expander/

    Peartree Music 87 Synthesiser

    http://www.computinghistory.org.uk/det/4535/Peartree-Computers-Music-87-Synthesizer-(M500)/

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_M5000_H
#define MAME_BUS_BBC_1MHZBUS_M5000_H

#include "1mhzbus.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> htmusic_device

class htmusic_device : public device_t, public device_sound_interface
{
public:
	htmusic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ram_w(offs_t offset, uint8_t data) { m_wave_ram[offset & 0x7ff] = data; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	TIMER_CALLBACK_MEMBER(dsp_tick);

private:
	uint16_t m_antilog[128];
	uint8_t m_wave_ram[0x800];
	uint32_t m_phase_ram[0x200];

	uint8_t m_counter;
	uint8_t m_c4d;
	uint8_t m_sign;
	int16_t m_sam;
	bool m_disable;
	bool m_modulate;

	int32_t m_sam_l[16];
	int32_t m_sam_r[16];

	emu_timer *m_dsp_timer;

	sound_stream *m_stream;
};


// ======================> bbc_m500_device

class bbc_m500_device : public device_t, public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_m500_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_m500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void add_common_devices(machine_config &config);

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;
	required_device<htmusic_device> m_hybrid;

	uint8_t m_page;
};


// ======================> bbc_m5000_device

class bbc_m5000_device : public bbc_m500_device
{
public:
	// construction/destruction
	bbc_m5000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	//virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> bbc_m3000_device

class bbc_m3000_device : public bbc_m500_device
{
public:
	// construction/destruction
	bbc_m3000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void jim_w(offs_t offset, uint8_t data) override;
};


// ======================> bbc_m87_device

class bbc_m87_device : public bbc_m500_device
{
public:
	// construction/destruction
	bbc_m87_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_M500,  bbc_m500_device);
DECLARE_DEVICE_TYPE(BBC_M5000, bbc_m5000_device);
DECLARE_DEVICE_TYPE(BBC_M3000, bbc_m3000_device);
DECLARE_DEVICE_TYPE(BBC_M87,   bbc_m87_device);


#endif /* MAME_BUS_BBC_1MHZBUS_M5000_H */
