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

READ8_MEMBER(sns_rom_sgb_device::gb_cart_r)
{
	if (offset < 0x100 && !m_bios_disabled)
	{
		return m_region_bios->base()[offset];
	}
	return m_cartslot->read_rom(space, offset);
}

WRITE8_MEMBER(sns_rom_sgb_device::gb_bank_w)
{
	m_cartslot->write_bank(space, offset, data);
}

READ8_MEMBER(sns_rom_sgb_device::gb_ram_r)
{
	return m_cartslot->read_ram(space, offset);
}

WRITE8_MEMBER(sns_rom_sgb_device::gb_ram_w)
{
	m_cartslot->write_ram(space, offset, data);
}

READ8_MEMBER(sns_rom_sgb_device::gb_echo_r)
{
	return space.read_byte(0xc000 + offset);
}

WRITE8_MEMBER(sns_rom_sgb_device::gb_echo_w)
{
	return space.write_byte(0xc000 + offset, data);
}

READ8_MEMBER(sns_rom_sgb_device::gb_io_r)
{
	return 0;
}

WRITE8_MEMBER(sns_rom_sgb_device::gb_io_w)
{
}

READ8_MEMBER(sns_rom_sgb_device::gb_ie_r)
{
	return m_sgb_cpu->get_ie();
}

WRITE8_MEMBER(sns_rom_sgb_device::gb_ie_w)
{
	m_sgb_cpu->set_ie(data);
}



ADDRESS_MAP_START(sns_rom_sgb_device::supergb_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(gb_cart_r, gb_bank_w)
	AM_RANGE(0x8000, 0x9fff) AM_DEVREADWRITE("sgb_ppu", sgb_ppu_device, vram_r, vram_w)  /* 8k VRAM */
	AM_RANGE(0xa000, 0xbfff) AM_READWRITE(gb_ram_r, gb_ram_w )   /* 8k switched RAM bank (cartridge) */
	AM_RANGE(0xc000, 0xdfff) AM_RAM                              /* 8k low RAM */
	AM_RANGE(0xe000, 0xfdff) AM_READWRITE(gb_echo_r, gb_echo_w)
	AM_RANGE(0xfe00, 0xfeff) AM_DEVREADWRITE("sgb_ppu", sgb_ppu_device, oam_r, oam_w)    /* OAM RAM */
	AM_RANGE(0xff00, 0xff0f) AM_READWRITE(gb_io_r, gb_io_w)      /* I/O */
	AM_RANGE(0xff10, 0xff26) AM_DEVREADWRITE("sgb_apu", gameboy_sound_device, sound_r, sound_w)      /* sound registers */
	AM_RANGE(0xff27, 0xff2f) AM_NOP                     /* unused */
	AM_RANGE(0xff30, 0xff3f) AM_DEVREADWRITE("sgb_apu", gameboy_sound_device, wave_r, wave_w)        /* Wave RAM */
	AM_RANGE(0xff40, 0xff7f) AM_DEVREADWRITE("sgb_ppu", sgb_ppu_device, video_r, video_w) /* also disable bios?? */        /* Video controller & BIOS flip-flop */
	AM_RANGE(0xff80, 0xfffe) AM_RAM                     /* High RAM */
	AM_RANGE(0xffff, 0xffff) AM_READWRITE(gb_ie_r, gb_ie_w)        /* Interrupt enable register */
ADDRESS_MAP_END



WRITE8_MEMBER( sns_rom_sgb_device::gb_timer_callback )
{
}


static SLOT_INTERFACE_START(supergb_cart)
	SLOT_INTERFACE_INTERNAL("rom",  GB_STD_ROM)
	SLOT_INTERFACE_INTERNAL("rom_mbc1",  GB_ROM_MBC1)
SLOT_INTERFACE_END


MACHINE_CONFIG_START(sns_rom_sgb1_device::device_add_mconfig)
	MCFG_CPU_ADD("sgb_cpu", LR35902, 4295454)   /* 4.295454 MHz */
	MCFG_CPU_PROGRAM_MAP(supergb_map)
	MCFG_LR35902_TIMER_CB(WRITE8(sns_rom_sgb_device, gb_timer_callback))
	MCFG_LR35902_HALT_BUG

	MCFG_SGB_PPU_ADD("sgb_ppu", "sgb_cpu")

	MCFG_SOUND_ADD("sgb_apu", DMG_APU, 4295454)

	MCFG_GB_CARTRIDGE_ADD("gb_slot", supergb_cart, nullptr)
MACHINE_CONFIG_END


ROM_START( supergb )
	ROM_REGION(0x100, "sgb_cpu", 0)
	ROM_LOAD("sgb_boot.bin", 0x0000, 0x0100, CRC(ec8a83b9) SHA1(aa2f50a77dfb4823da96ba99309085a3c6278515))
ROM_END


const tiny_rom_entry *sns_rom_sgb1_device::device_rom_region() const
{
	return ROM_NAME( supergb );
}


MACHINE_CONFIG_START(sns_rom_sgb2_device::device_add_mconfig)
	MCFG_CPU_ADD("sgb_cpu", LR35902, XTAL(4'194'304))   /* 4.194MHz derived from clock on sgb2 pcb */
	MCFG_CPU_PROGRAM_MAP(supergb_map)
	MCFG_LR35902_TIMER_CB(WRITE8(sns_rom_sgb_device, gb_timer_callback))
	MCFG_LR35902_HALT_BUG

	MCFG_SGB_PPU_ADD("sgb_ppu", "sgb_cpu")

	MCFG_SOUND_ADD("sgb_apu", DMG_APU, XTAL(4'194'304))

	MCFG_GB_CARTRIDGE_ADD("gb_slot", supergb_cart, nullptr)
MACHINE_CONFIG_END


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


READ8_MEMBER(sns_rom_sgb_device::read_l)
{
	return read_h(space, offset);
}

READ8_MEMBER(sns_rom_sgb_device::read_h)
{
	int bank = offset / 0x10000;
	return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
}

READ8_MEMBER( sns_rom_sgb_device::chip_read )
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

WRITE8_MEMBER( sns_rom_sgb_device::chip_write )
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
