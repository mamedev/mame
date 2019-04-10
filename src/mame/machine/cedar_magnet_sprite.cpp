// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

 todo: sometimes sprites get left onscreen (xain)

*/
#include "emu.h"
#include "cedar_magnet_sprite.h"


DEFINE_DEVICE_TYPE(CEDAR_MAGNET_SPRITE, cedar_magnet_sprite_device, "cedmag_sprite", "Cedar Sprite")


cedar_magnet_sprite_device::cedar_magnet_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CEDAR_MAGNET_SPRITE, tag, owner, clock),
	cedar_magnet_board_interface(mconfig, *this, "spritecpu", "ram"),
	m_sprite_ram_bankdev(*this, "sp_sub_ram"),
	m_pio0(*this, "z80pio0"),
	m_pio1(*this, "z80pio1"),
	m_pio2(*this, "z80pio2")
{
}

void cedar_magnet_sprite_device::cedar_magnet_sprite_sub_ram_map(address_map &map)
{
// these are 8x SIEMENS HYB 41256-15 AA - 262,144 bit DRAM (32kbytes)
// these are on the sprite board memory sub-board
	map(0x00000, 0x3ffff).ram().share("ram");
}

READ8_MEMBER(cedar_magnet_sprite_device::exzisus_hack_r)
{
	//printf("exzisus_hack_r\n");
	int pc = m_cpu->pc();

	// exzisus has startup code outside of the first 0x400 bytes
	// but the main cpu only transfers 0x400 bytes of the code to the other banks?!
	if ((pc >= 0x3e0) && (pc <= 0x800))
	{
		return m_ram[0x400 + offset];
	}
	else
	{
		return m_ram[0x400 + offset + (pio2_pb_data & 0x3)*0x10000];
	}

}


void cedar_magnet_sprite_device::cedar_magnet_sprite_map(address_map &map)
{
	map(0x00000, 0x0ffff).m("sp_sub_ram", FUNC(address_map_bank_device::amap8));

	map(0x00400, 0x007ff).r(FUNC(cedar_magnet_sprite_device::exzisus_hack_r));
}

void cedar_magnet_sprite_device::cedar_magnet_sprite_io(address_map &map)
{
	map.global_mask(0xff);

	map(0xc0, 0xc3).rw("z80pio0", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0xc4, 0xc7).rw("z80pio1", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0xc8, 0xcb).rw("z80pio2", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));

	map(0x80, 0x80).w(FUNC(cedar_magnet_sprite_device::sprite_port80_w));
	map(0x84, 0x84).w(FUNC(cedar_magnet_sprite_device::sprite_port84_w));

	map(0x88, 0x88).w(FUNC(cedar_magnet_sprite_device::sprite_port88_w)); // increasing values // upper address?

	map(0x8c, 0x8c).w(FUNC(cedar_magnet_sprite_device::sprite_port8c_w)); // written after 88 (possible data upload?)

	map(0x9c, 0x9c).w(FUNC(cedar_magnet_sprite_device::sprite_port9c_w)); // ?

}

void cedar_magnet_sprite_device::do_blit()
{
//  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
//  printf("~~~~~~~~~~~~~~~~~ drawing sprite with x:%02x y:%02x code:%04x size:%02x unk:%02x\n", m_loweraddr, m_upperaddr, (m_spritecodehigh << 8) | m_spritecodelow, m_spritesize, pio0_pb_data);
//  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	int ysize = 0;
	int xsize = 0;
	int erase = 0;

	// bit 0x80 is always set
	if ((m_spritesize & 0x7f) == 0x00)
		ysize = xsize = 8;

	if ((m_spritesize & 0x7f) == 0x01)
		ysize = xsize = 16;

	if ((m_spritesize & 0x7f) == 0x02)
		ysize = xsize = 32;

	if ((m_spritesize & 0x7f) == 0x03)
		ysize = xsize = 64;

	// m_spritesize
	// pio0_pb_data


	int source = (m_spritecodehigh << 8) | m_spritecodelow;

	if (source == 0)
		erase = 1;

	source &= ~0x3f;

	for (int y = 0;y < ysize;y++)
	{
		for (int x = 0;x < xsize;x++)
		{
			int xpos = (m_loweraddr + x);
			int ypos = (m_upperaddr + y);

			uint8_t data = m_ram[source + ((m_uppersprite & 0x3) * 0x10000)];

			if (!(pio0_pb_data & 0x02))
				data = machine().rand();

			source++;

			xpos &= 0xff;

			// without this some sprites incorrectly wraparound on the volcano table.
			if (!erase)
			{
				if (!(pio0_pb_data & 0x40))
				{
					if (xpos >= 0xff-64)
						continue;
				}
				else
				{
					if (xpos < 64)
						continue;
				}
			}

			//if ((ypos >= 0) && (ypos < 0x100))
			ypos &= 0xff;

			{
				int offset = (ypos * 256) + xpos;

				if (erase == 1)
				{
					m_framebuffer[offset] = 0;
				}
				else
				{
					if (data) m_framebuffer[offset] = data;
				}
			}
		}
	}
}

WRITE8_MEMBER(cedar_magnet_sprite_device::sprite_port80_w)
{
	m_spritecodelow = data;
//  printf("%s:sprite numlow / trigger %02x\n", machine().describe_context().c_str(), data);

	do_blit();
}

WRITE8_MEMBER(cedar_magnet_sprite_device::sprite_port84_w)
{
	m_spritecodehigh = data;
	m_high_write = 1;
//  printf("%s:sprite numhigh %02x\n", machine().describe_context().c_str(), data);
}

WRITE8_MEMBER(cedar_magnet_sprite_device::sprite_port88_w)
{
// frequent
//  printf("%s:sprite_y_coordinate %02x\n", machine().describe_context().c_str(), data);
	m_upperaddr = data;
}

WRITE8_MEMBER(cedar_magnet_sprite_device::pio2_pa_w)
{
// frequent
//  printf("%s:sprite_x_coordinate %02x\n", machine().describe_context().c_str(), data);
	m_loweraddr = data;
}

WRITE8_MEMBER(cedar_magnet_sprite_device::sprite_port8c_w)
{
	int address = (m_upperaddr << 8) | m_loweraddr;
	m_framebuffer[address] = data;
	if (data!=0x00) printf("sprite_port8c_w write %04x %02x\n", address, data);
}

// possible watchdog?
WRITE8_MEMBER(cedar_magnet_sprite_device::sprite_port9c_w)
{
//  printf("%s:sprite_port9c_w %02x\n", machine().describe_context().c_str(), data);
}

void cedar_magnet_sprite_device::device_add_mconfig(machine_config &config)
{
	z80_device &spritecpu(Z80(config, "spritecpu", 4000000));
	spritecpu.set_addrmap(AS_PROGRAM, &cedar_magnet_sprite_device::cedar_magnet_sprite_map);
	spritecpu.set_addrmap(AS_IO, &cedar_magnet_sprite_device::cedar_magnet_sprite_io);

	Z80PIO(config, m_pio0, 4000000/2);
//  m_pio0->out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);
	m_pio0->in_pa_callback().set(FUNC(cedar_magnet_sprite_device::pio0_pa_r));
	m_pio0->out_pa_callback().set(FUNC(cedar_magnet_sprite_device::pio0_pa_w));
//  m_pio0->in_pb_callback().set(FUNC(cedar_magnet_sprite_device::pio0_pb_r));
	m_pio0->out_pb_callback().set(FUNC(cedar_magnet_sprite_device::pio0_pb_w));

	Z80PIO(config, m_pio1, 4000000/2);
//  m_pio1->out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);
//  m_pio1->in_pa_callback().set(FUNC(cedar_magnet_sprite_device::pio1_pa_r));
	m_pio1->out_pa_callback().set(FUNC(cedar_magnet_sprite_device::pio1_pa_w));
//  m_pio1->in_pb_callback().set(FUNC(cedar_magnet_sprite_device::pio1_pb_r));
	m_pio1->out_pb_callback().set(FUNC(cedar_magnet_sprite_device::pio1_pb_w));

	Z80PIO(config, m_pio2, 4000000/2);
//  m_pio2->out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);
//  m_pio2->in_pa_callback().set(FUNC(cedar_magnet_sprite_device::pio2_pa_r));
	m_pio2->out_pa_callback().set(FUNC(cedar_magnet_sprite_device::pio2_pa_w));
//  m_pio2->in_pb_callback().set(FUNC(cedar_magnet_sprite_device::pio2_pb_r));
	m_pio2->out_pb_callback().set(FUNC(cedar_magnet_sprite_device::pio2_pb_w));


	ADDRESS_MAP_BANK(config, m_sprite_ram_bankdev).set_map(&cedar_magnet_sprite_device::cedar_magnet_sprite_sub_ram_map).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);
}


READ8_MEMBER(cedar_magnet_sprite_device::pio0_pa_r)
{
//  actually read
//  printf("%s: pio0_pa_r\n", machine().describe_context().c_str());
	return 0x00;
}



WRITE8_MEMBER(cedar_magnet_sprite_device::pio0_pa_w)
{
	m_spritesize = data;
}

WRITE8_MEMBER(cedar_magnet_sprite_device::pio0_pb_w)
{
	pio0_pb_data = data;
	//printf("%s: pio0_pb_w %02x\n", machine().describe_context().c_str(), data);
}

WRITE8_MEMBER(cedar_magnet_sprite_device::pio1_pa_w)
{
	//printf("%s: pio1_pa_w %02x\n", machine().describe_context().c_str(), data);
}

WRITE8_MEMBER(cedar_magnet_sprite_device::pio1_pb_w)
{
	//printf("%s: pio1_pb_w %02x\n", machine().describe_context().c_str(), data);
}


WRITE8_MEMBER(cedar_magnet_sprite_device::pio2_pb_w)
{
	// this feels like a hack
	// the game writes here when creating the sprite list so that it can access the correct gfd data
	// however the actual LIST data is always in bank 0 (it can't be in any other bank, those areas are occupied by actual gfx)
	if (m_high_write)
	{
		m_uppersprite = data & 0x03;
		m_high_write = 0;
		return;

	}

	pio2_pb_data = data;
	//printf("%s: ******************************************* BANK? **** pio2_pb_w %02x\n", machine().describe_context().c_str(), data);
	// yes, it ends up banking the ram right out from under itself during startup execution...
	// during this time the main cpu is waiting in a loop, after which it copies the startup code again, and reboots it.
	m_sprite_ram_bankdev->set_bank(data & 0x03);
}


void cedar_magnet_sprite_device::device_start()
{
}


void cedar_magnet_sprite_device::device_reset()
{
	halt_assert();
	m_sprite_ram_bankdev->set_bank(0);
	pio2_pb_data = 0x00;
	m_spritesize = 0xff;
}

uint32_t cedar_magnet_sprite_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palbase)
{
//  printf("-----------------------------------------------------------------------------------------------------------\n");
//  printf("--------------------------------------------- FRAME -------------------------------------------------------\n");
//  printf("-----------------------------------------------------------------------------------------------------------\n");

	uint8_t* mem = m_framebuffer;
	int count = 0;

//  if (!(m_m_spritesize & 0x40))
//      return 0;

	for (int y = 0;y < 256;y++)
	{
		uint16_t *dst = &bitmap.pix16((y)&0xff);

		for (int x = 0; x < 256;x++)
		{
			uint8_t pix = mem[count];
			count++;

			if (pix) dst[(x)&0xff] = pix + palbase*0x100;
		}
	}

	return 0;
}
