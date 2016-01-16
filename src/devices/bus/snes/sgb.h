// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SNS_SGB_H
#define __SNS_SGB_H

#include "snes_slot.h"
#include "rom.h"

#include "cpu/lr35902/lr35902.h"
#include "bus/gameboy/gb_slot.h"
#include "bus/gameboy/rom.h"
#include "bus/gameboy/mbc.h"
#include "video/gb_lcd.h"
#include "audio/gb.h"


// ======================> sns_rom_sgb_device

class sns_rom_sgb_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_sgb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_READ8_MEMBER(chip_read) override;
	virtual DECLARE_WRITE8_MEMBER(chip_write) override;

	virtual DECLARE_READ8_MEMBER(gb_cart_r);
	virtual DECLARE_WRITE8_MEMBER(gb_bank_w);
	virtual DECLARE_READ8_MEMBER(gb_ram_r);
	virtual DECLARE_WRITE8_MEMBER(gb_ram_w);
	virtual DECLARE_READ8_MEMBER(gb_echo_r);
	virtual DECLARE_WRITE8_MEMBER(gb_echo_w);
	virtual DECLARE_READ8_MEMBER(gb_io_r);
	virtual DECLARE_WRITE8_MEMBER(gb_io_w);
	virtual DECLARE_READ8_MEMBER(gb_ie_r);
	virtual DECLARE_WRITE8_MEMBER(gb_ie_w);
	virtual DECLARE_WRITE8_MEMBER(gb_timer_callback);

	required_device<lr35902_cpu_device> m_gb_cpu;
	required_device<gameboy_sound_device> m_gb_snd;
	required_device<sgb_lcd_device> m_gb_lcd;
	required_device<gb_cart_slot_device> m_cartslot;

	void lcd_render(UINT32 *source);

	// ICD2 regs
	UINT8 m_sgb_ly;
	UINT8 m_sgb_row;
	UINT8 m_vram;
	UINT8 m_port;
	UINT8 m_joy1, m_joy2, m_joy3, m_joy4;
	UINT8 m_joy_pckt[16];
	UINT16 m_vram_offs;
	UINT8 m_mlt_req;

	UINT32 m_lcd_buffer[4 * 160 * 8];
	UINT16 m_lcd_output[320];
	UINT16 m_lcd_row;

	// input bits
	int m_packetsize;
	UINT8 m_packet_data[64][16];
};


// device type definition
extern const device_type SNS_LOROM_SUPERGB;

#endif
