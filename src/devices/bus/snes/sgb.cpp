// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 Super Game Boy emulation (for SNES/SFC)

 TODO: almost everything, e.g.
 * implement gb_timer_callback
 * gb_io_r/w
 * add hook-up to copy LCD scanline to m_lcd_buffer

 ***********************************************************************************************************/


#include "emu.h"
#include "sgb.h"

//-------------------------------------------------
//  sns_rom_sgb_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(SNS_LOROM_SUPERGB,  sns_rom_sgb1_device, "sns_rom_sgb",  "SNES Super Game Boy Cart")
DEFINE_DEVICE_TYPE(SNS_LOROM_SUPERGB2, sns_rom_sgb2_device, "sns_rom_sgb2", "SNES Super Game Boy 2 Cart")


sns_rom_sgb_device::sns_rom_sgb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_device(mconfig, type, tag, owner, clock),
	m_sgb_cpu(*this, "sgb_cpu"),
	m_sgb_apu(*this, "sgb_apu"),
	m_sgb_ppu(*this, "sgb_ppu"),
	m_cartslot(*this, "gb_slot"),
	m_region_bios(*this, "sgb_cpu"),
	m_sgb_ly(0),
	m_sgb_row(0),
	m_vram(0),
	m_port(0),
	m_joy1(0),
	m_joy2(0),
	m_joy3(0),
	m_joy4(0),
	m_vram_offs(0),
	m_lcd_row(0),
	m_packetsize(0),
	m_bios_disabled(false)
{
}


sns_rom_sgb1_device::sns_rom_sgb1_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	sns_rom_sgb_device(mconfig, SNS_LOROM_SUPERGB, tag, owner, clock)
{
}


sns_rom_sgb2_device::sns_rom_sgb2_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	sns_rom_sgb_device(mconfig, SNS_LOROM_SUPERGB2, tag, owner, clock)
{
}


void sns_rom_sgb_device::device_start()
{
}

void sns_rom_sgb_device::device_reset()
{
}



// SuperGB emulation

//-------------------------------------------------
//  ADDRESS_MAP( supergb_map )
//-------------------------------------------------

uint8_t sns_rom_sgb_device::gb_cart_r(offs_t offset)
{
	if (offset < 0x100 && !m_bios_disabled)
	{
		return m_region_bios->base()[offset];
	}
	return m_cartslot->read_rom(offset);
}

void sns_rom_sgb_device::gb_bank_w(offs_t offset, uint8_t data)
{
	m_cartslot->write_bank(offset, data);
}

uint8_t sns_rom_sgb_device::gb_ram_r(offs_t offset)
{
	return m_cartslot->read_ram(offset);
}

void sns_rom_sgb_device::gb_ram_w(offs_t offset, uint8_t data)
{
	m_cartslot->write_ram(offset, data);
}

uint8_t sns_rom_sgb_device::gb_echo_r(offs_t offset)
{
	return m_sgb_cpu->space(AS_PROGRAM).read_byte(0xc000 + offset);
}

void sns_rom_sgb_device::gb_echo_w(offs_t offset, uint8_t data)
{
	return m_sgb_cpu->space(AS_PROGRAM).write_byte(0xc000 + offset, data);
}

uint8_t sns_rom_sgb_device::gb_io_r(offs_t offset)
{
	return 0;
}

void sns_rom_sgb_device::gb_io_w(offs_t offset, uint8_t data)
{
}

uint8_t sns_rom_sgb_device::gb_ie_r(offs_t offset)
{
	return m_sgb_cpu->get_ie();
}

void sns_rom_sgb_device::gb_ie_w(offs_t offset, uint8_t data)
{
	m_sgb_cpu->set_ie(data);
}



void sns_rom_sgb_device::supergb_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(FUNC(sns_rom_sgb_device::gb_cart_r), FUNC(sns_rom_sgb_device::gb_bank_w));
	map(0x8000, 0x9fff).rw("sgb_ppu", FUNC(sgb_ppu_device::vram_r), FUNC(sgb_ppu_device::vram_w));  /* 8k VRAM */
	map(0xa000, 0xbfff).rw(FUNC(sns_rom_sgb_device::gb_ram_r), FUNC(sns_rom_sgb_device::gb_ram_w));   /* 8k switched RAM bank (cartridge) */
	map(0xc000, 0xdfff).ram();                              /* 8k low RAM */
	map(0xe000, 0xfdff).rw(FUNC(sns_rom_sgb_device::gb_echo_r), FUNC(sns_rom_sgb_device::gb_echo_w));
	map(0xfe00, 0xfeff).rw("sgb_ppu", FUNC(sgb_ppu_device::oam_r), FUNC(sgb_ppu_device::oam_w));    /* OAM RAM */
	map(0xff00, 0xff0f).rw(FUNC(sns_rom_sgb_device::gb_io_r), FUNC(sns_rom_sgb_device::gb_io_w));      /* I/O */
	map(0xff10, 0xff26).rw("sgb_apu", FUNC(gameboy_sound_device::sound_r), FUNC(gameboy_sound_device::sound_w));      /* sound registers */
	map(0xff27, 0xff2f).noprw();                     /* unused */
	map(0xff30, 0xff3f).rw("sgb_apu", FUNC(gameboy_sound_device::wave_r), FUNC(gameboy_sound_device::wave_w));        /* Wave RAM */
	map(0xff40, 0xff7f).rw("sgb_ppu", FUNC(sgb_ppu_device::video_r), FUNC(sgb_ppu_device::video_w)); /* also disable bios?? */        /* Video controller & BIOS flip-flop */
	map(0xff80, 0xfffe).ram();                     /* High RAM */
	map(0xffff, 0xffff).rw(FUNC(sns_rom_sgb_device::gb_ie_r), FUNC(sns_rom_sgb_device::gb_ie_w));        /* Interrupt enable register */
}



void sns_rom_sgb_device::gb_timer_callback(uint8_t data)
{
}


static void supergb_cart(device_slot_interface &device)
{
	device.option_add_internal("rom",  GB_STD_ROM);
	device.option_add_internal("rom_mbc1",  GB_ROM_MBC1);
}


void sns_rom_sgb1_device::device_add_mconfig(machine_config &config)
{
	LR35902(config, m_sgb_cpu, 4295454);   /* 4.295454 MHz */
	m_sgb_cpu->set_addrmap(AS_PROGRAM, &sns_rom_sgb1_device::supergb_map);
	m_sgb_cpu->timer_cb().set(FUNC(sns_rom_sgb_device::gb_timer_callback));
	m_sgb_cpu->set_halt_bug(true);

	SGB_PPU(config, m_sgb_ppu, m_sgb_cpu);

	DMG_APU(config, m_sgb_apu, 4295454);

	GB_CART_SLOT(config, m_cartslot, supergb_cart, nullptr);
}


ROM_START( supergb )
	ROM_REGION(0x100, "sgb_cpu", 0)
	ROM_LOAD("sgb_boot.bin", 0x0000, 0x0100, CRC(ec8a83b9) SHA1(aa2f50a77dfb4823da96ba99309085a3c6278515))
ROM_END


const tiny_rom_entry *sns_rom_sgb1_device::device_rom_region() const
{
	return ROM_NAME( supergb );
}


void sns_rom_sgb2_device::device_add_mconfig(machine_config &config)
{
	LR35902(config, m_sgb_cpu, XTAL(4'194'304)); /* 4.194MHz derived from clock on sgb2 pcb */
	m_sgb_cpu->set_addrmap(AS_PROGRAM, &sns_rom_sgb2_device::supergb_map);
	m_sgb_cpu->timer_cb().set(FUNC(sns_rom_sgb_device::gb_timer_callback));
	m_sgb_cpu->set_halt_bug(true);

	SGB_PPU(config, m_sgb_ppu, m_sgb_cpu);

	DMG_APU(config, m_sgb_apu, XTAL(4'194'304));

	GB_CART_SLOT(config, m_cartslot, supergb_cart, nullptr);
}


ROM_START( supergb2 )
	ROM_REGION(0x100, "sgb_cpu", 0)
	ROM_LOAD("sgb2_boot.bin", 0x0000, 0x0100, CRC(53d0dd63) SHA1(93407ea10d2f30ab96a314d8eca44fe160aea734))
ROM_END


const tiny_rom_entry *sns_rom_sgb2_device::device_rom_region() const
{
	return ROM_NAME( supergb2 );
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/


uint8_t sns_rom_sgb_device::read_l(offs_t offset)
{
	return read_h(offset);
}

uint8_t sns_rom_sgb_device::read_h(offs_t offset)
{
	int bank = offset / 0x10000;
	return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
}

uint8_t sns_rom_sgb_device::chip_read(offs_t offset)
{
	uint16_t address = offset & 0xffff;

	//LY counter
	if (address == 0x6000)
	{
		m_sgb_ly = 0;// GameBoy PPU LY here
		m_sgb_row = m_lcd_row;
		return m_sgb_ly;
	}

	//command ready port
	if (address == 0x6002)
	{
		bool data = (m_packetsize > 0);
		if (data)
		{
			for (int i = 0; i < 16; i++)
				m_joy_pckt[i] = m_packet_data[0][i];
			m_packetsize--;

			//hack because we still don't emulate input packets!
			if (!m_packetsize) m_packetsize = 64;

			// shift packet
			for (int i = 0; i < m_packetsize; i++)
				for (int j = 0; j < 16; j++)
					m_packet_data[i][j] = m_packet_data[i + 1][j];
		}
		return data;
	}

	//ICD2 revision
	if (address == 0x600f)
		return 0x21;

	//command port
	if ((address & 0xfff0) == 0x7000)
		return m_joy_pckt[address & 0x0f];

	//VRAM port
	if (address == 0x7800)
	{
		uint8_t data = m_lcd_output[m_vram_offs];
		m_vram_offs = (m_vram_offs + 1) % 320;
		return data;
	}

	return 0x00;    // this should never happen?
}

void sns_rom_sgb_device::lcd_render(uint32_t *source)
{
	memset(m_lcd_output, 0x00, 320 * sizeof(uint16_t));

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 160; x++)
		{
			uint32_t pixel = *source++;
			uint16_t addr = y * 2 + (x / 8 * 16);
			m_lcd_output[addr + 0] |= ((pixel & 1) >> 0) << (7 - (x & 7));
			m_lcd_output[addr + 1] |= ((pixel & 2) >> 1) << (7 - (x & 7));
		}
	}
}

void sns_rom_sgb_device::chip_write(offs_t offset, uint8_t data)
{
	uint16_t address = offset & 0xffff;

	//VRAM port
	if (address == 0x6001)
	{
		m_vram = data;
		m_vram_offs = 0;

		uint8_t offset = (m_sgb_row - (4 - (m_vram - (m_sgb_ly & 3)))) & 3;
		lcd_render(m_lcd_buffer + offset * 160 * 8);

		return;
	}

	//control port
	if (address == 0x6003)
	{
		if ((m_port & 0x80) == 0x00 && (data & 0x80) == 0x80)
		{
			//reset
		}

		switch (data & 3)
		{
			//change CPU frequency
		}
		m_port = data;
		return;
	}

	if (address == 0x6004)
	{
		//joypad 1
		m_joy1 = data;
		return;
	}
	if (address == 0x6005)
	{
		//joypad 2
		m_joy2 = data;
		return;
	}
	if (address == 0x6006)
	{
		//joypad 3
		m_joy3 = data;
		return;
	}
	if (address == 0x6007)
	{
		//joypad 4
		m_joy4 = data;
		return;
	}

}
