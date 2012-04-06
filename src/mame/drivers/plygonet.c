/*
    Polygonet Commanders (Konami, 1993)
    Poly-Net Warriors (Konami, 1993)

    Preliminary driver by R. Belmont
    Additional work by Andrew Gardner

    This is Konami's first 3D game!

    Hardware:
    68EC020 @ 16 MHz
    Motorola XC56156-40 DSP @ 40 MHz
    Z80 + K054539 for sound
    Network to connect up to 4 PCBs.

    Video hardware:
    TTL text plane similar to Run and Gun.
    Konami K054009(x2) + K054010(x2) (polygon rasterizers)
    Konami K053936 "PSAC2" (3d roz plane, used for backgrounds)
    24.0 MHz crystal to drive the video hardware

    Driver includes:
    - 68020 memory map
    - Z80 + sound system
    - EEPROM
    - service switch
    - TTL text plane

    Driver needs:
    - Handle network at 580800 so game starts
    - Polygon rasterization (K054009 + K054010)
    - Hook up PSAC2 (gfx decode for it is already present and correct)
    - Palettes
    - Controls
    - Priorities.  From the original board it appears they're fixed, in front to back order:
      (all the way in front) TTL text layer -> polygons -> PSAC2 (all the way in back)

    Tech info by Phil Bennett, from the schematics:

    68000 address map
    =================

    400000-43ffff = PSAC
    440000-47ffff = PSVR
    480000-4bffff = IO
    4c0000-4fffff = SYS
    500000-53ffff = DSP
    540000-57ffff = FIX
    580000-5bffff = OP1
    5c0000-5fffff = UNUSED


    SYS (Write only?)
    =================

    D28 = /FIXKILL     - Disable 'FIX' layer?
    D27 = MUTE
    D26 = EEPROM CLK
    D25 = EEPROM CS
    D24 = EEPROM DATA
    D23 = BRMAS        - 68k bus error mask
    D22 = L7MAS        - L7 interrupt mask (unusued - should always be '1')
    D21 = /L5MAS       - L5 interrupt mask/acknowledge
    D20 = L3MAS        - L3 interrupt mask
    D19 = VFLIP        - Flip video vertically
    D18 = HFLIP        - Flip video horizontally
    D17 = COIN2        - Coin counter 2
    D16 = COIN1        - Coin counter 1


    DSP
    ===

    500000-503fff = HCOM     - 16kB common RAM
    504000-504fff = CONTROL  - DSP/Host Control
                    D10? = COMBNK - Switch between 68k and DSP access to common RAM
                    D08? = RESN   - Reset DSP
    506000-506fff = HEN      - DSP/Host interface

*/

#include "emu.h"
#include "video/konicdev.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "cpu/dsp56k/dsp56k.h"
#include "sound/k054539.h"
#include "machine/eeprom.h"
#include "includes/plygonet.h"

enum { BANK_GROUP_A, BANK_GROUP_B, INVALID_BANK_GROUP };

static const eeprom_interface eeprom_intf =
{
	7,				/* address bits */
	8,				/* data bits */
	"011000",		/* read command */
	"010100",		/* write command */
	"0100100000000",/* erase command */
	"0100000000000",/* lock command */
	"0100110000000" /* unlock command */
};

static READ32_DEVICE_HANDLER( polygonet_eeprom_r )
{
	if (ACCESSING_BITS_0_15)
	{
		eeprom_device *eeprom = downcast<eeprom_device *>(device);
		return 0x0200 | (eeprom->read_bit() << 8);
	}
	else
	{
		UINT8 lowInputBits = input_port_read(device->machine(), "IN1");
		UINT8 highInputBits = input_port_read(device->machine(), "IN0");
		return ((highInputBits << 24) | (lowInputBits << 16));
	}

	logerror("unk access to eeprom port (mask %x)\n", mem_mask);
	return 0;
}


WRITE32_MEMBER(polygonet_state::polygonet_eeprom_w)
{
	if (ACCESSING_BITS_24_31)
	{
		input_port_write(machine(), "EEPROMOUT", data, 0xffffffff);
		return;
	}

	logerror("unknown write %x (mask %x) to eeprom\n", data, mem_mask);
}

/* TTL tile readback for ROM test */
READ32_MEMBER(polygonet_state::ttl_rom_r)
{
	UINT32 *ROM;
	ROM = (UINT32 *)machine().region("gfx1")->base();

	return ROM[offset];
}

/* PSAC2 tile readback for ROM test */
READ32_MEMBER(polygonet_state::psac_rom_r)
{
	UINT32 *ROM;
	ROM = (UINT32 *)machine().region("gfx2")->base();

	return ROM[offset];
}

/* irqs 3, 5, and 7 have valid vectors                */
/* irq 3 is network.  don't generate if you don't emulate the network h/w! */
/* irq 5 is vblank */
/* irq 7 does nothing (it jsrs to a rts and then rte) */
static INTERRUPT_GEN(polygonet_interrupt)
{
	device_set_input_line(device, M68K_IRQ_5, HOLD_LINE);
}

/* sound CPU communications */
READ32_MEMBER(polygonet_state::sound_r)
{
	int latch = soundlatch3_r(space, 0);

	if ((latch == 0xd) || (latch == 0xe)) latch = 0xf;	/* hack: until 54539 NMI disable found */

	return latch<<8;
}

WRITE32_MEMBER(polygonet_state::sound_w)
{
	if (ACCESSING_BITS_8_15)
	{
		soundlatch_w(space, 0, (data>>8)&0xff);
	}
	else
	{
		soundlatch2_w(space, 0, data&0xff);
	}
}

WRITE32_MEMBER(polygonet_state::sound_irq_w)
{
	cputag_set_input_line(machine(), "soundcpu", 0, HOLD_LINE);
}

/* DSP communications */
READ32_MEMBER(polygonet_state::dsp_host_interface_r)
{
	UINT32 value;
	UINT8 hi_addr = offset << 1;

	if (mem_mask == 0x0000ff00)	{ hi_addr++; }	/* Low byte */
	if (mem_mask == 0xff000000) {}				/* High byte */

	value = dsp56k_host_interface_read(machine().device("dsp"), hi_addr);

	if (mem_mask == 0x0000ff00)	{ value <<= 8;  }
	if (mem_mask == 0xff000000) { value <<= 24; }

	logerror("Dsp HI Read (host-side) %08x (HI %04x) = %08x (@%x)\n", mem_mask, hi_addr, value, cpu_get_pc(&space.device()));

	return value;
}

WRITE32_MEMBER(polygonet_state::shared_ram_write)
{

	COMBINE_DATA(&m_shared_ram[offset]) ;

	if (mem_mask == 0xffff0000)
	{
		logerror("68k WRITING %04x to shared ram %x (@%x)\n", (m_shared_ram[offset] & 0xffff0000) >> 16,
															   0xc000 + (offset<<1),
															   cpu_get_pc(&space.device()));
	}
	else if (mem_mask == 0x0000ffff)
	{
		logerror("68k WRITING %04x to shared ram %x (@%x)\n", (m_shared_ram[offset] & 0x0000ffff),
															   0xc000 +((offset<<1)+1),
															   cpu_get_pc(&space.device()));
	}
	else
	{
		logerror("68k WRITING %04x & %04x to shared ram %x & %x [%08x] (@%x)\n", (m_shared_ram[offset] & 0xffff0000) >> 16,
																				 (m_shared_ram[offset] & 0x0000ffff),
																				  0xc000 + (offset<<1),
																				  0xc000 +((offset<<1)+1),
																				  mem_mask,
																				  cpu_get_pc(&space.device()));
	}

	/* write to the current dsp56k word */
	if (mem_mask | (0xffff0000))
	{
		m_dsp56k_shared_ram_16[(offset<<1)] = (m_shared_ram[offset] & 0xffff0000) >> 16 ;
	}

	/* write to the next dsp56k word */
	if (mem_mask | (0x0000ffff))
	{
		m_dsp56k_shared_ram_16[(offset<<1)+1] = (m_shared_ram[offset] & 0x0000ffff) ;
	}
}

WRITE32_MEMBER(polygonet_state::dsp_w_lines)
{
	logerror("2w %08x %08x %08x\n", offset, mem_mask, data);

	/* 0x01000000 is the reset line - 0 is high, 1 is low */
	if ((data >> 24) & 0x01)
	{
//      logerror("RESET CLEARED\n");
		cputag_set_input_line(machine(), "dsp", DSP56K_IRQ_RESET, CLEAR_LINE);
	}
	else
	{
//      logerror("RESET ASSERTED\n");
		cputag_set_input_line(machine(), "dsp", DSP56K_IRQ_RESET, ASSERT_LINE);

		/* A little hacky - I can't seem to set these lines anywhere else where reset is asserted, so i do it here */
		cputag_set_input_line(machine(), "dsp", DSP56K_IRQ_MODA, ASSERT_LINE);
		cputag_set_input_line(machine(), "dsp", DSP56K_IRQ_MODB, CLEAR_LINE);
	}

	/* 0x04000000 is the COMBNK line - it switches who has access to the shared RAM - the dsp or the 68020 */
}

WRITE32_MEMBER(polygonet_state::dsp_host_interface_w)
{
	UINT8 hi_data = 0x00;
	UINT8 hi_addr = offset << 1;

	if (mem_mask == 0x0000ff00)	{ hi_addr++; }	/* Low byte */
	if (mem_mask == 0xff000000) {}				/* High byte */

	if (mem_mask == 0x0000ff00)	{ hi_data = (data & 0x0000ff00) >> 8;  }
	if (mem_mask == 0xff000000) { hi_data = (data & 0xff000000) >> 24; }

	logerror("write (host-side) %08x %08x %08x (HI %04x)\n", offset, mem_mask, data, hi_addr);
	dsp56k_host_interface_write(machine().device("dsp"), hi_addr, hi_data);
}


READ32_MEMBER(polygonet_state::network_r)
{
	return 0x08000000;
}


WRITE32_MEMBER(polygonet_state::plygonet_palette_w)
{
	int r,g,b;

	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	r = (m_generic_paletteram_32[offset] >>16) & 0xff;
	g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
	b = (m_generic_paletteram_32[offset] >> 0) & 0xff;

	palette_set_color(machine(),offset,MAKE_RGB(r,g,b));
}


/**********************************************************************************/
/*******                            DSP56k maps                             *******/
/**********************************************************************************/

/* It's believed this is hard-wired to return (at least) bit 15 as 0 - causes a host interface bootup */
READ16_MEMBER(polygonet_state::dsp56k_bootload_r)
{
	return 0x7fff;
}

DIRECT_UPDATE_HANDLER( plygonet_dsp56k_direct_handler )
{
	polygonet_state *state = machine.driver_data<polygonet_state>();

	/* Call the dsp's update handler first */
	if (!state->m_dsp56k_update_handler.isnull())
	{
		if (state->m_dsp56k_update_handler(direct, address) == ~0)
			return ~0;
	}

	/* If the requested region wasn't in there, see if it needs to be caught driver-side */
	if (address >= (0x7000<<1) && address <= (0x7fff<<1))
	{
		direct.explicit_configure(0x7000<<1, 0x7fff<<1, (0xfff<<1) | 1, state->m_dsp56k_p_mirror);
		return ~0;
	}
	else if (address >= (0x8000<<1) && address <= (0x87ff<<1))
	{
		direct.explicit_configure(0x8000<<1, 0x87ff<<1, (0x7ff<<1) | 1, state->m_dsp56k_p_8000);
		return ~0;
	}

	return address;
}

/* The dsp56k's Port C Data register (0xffe3) :
   Program code (function 4e) configures it as general purpose output I/O pins (ffc1 = 0000 & ffc3 = 0fff).

   XXXX ---- ---- ----  . Reserved bits
   ---- ???- -?-- ----  . unknown
   ---- ---- --x- ----  . [Bank Group A] Enable bit for "001c banking"?
   ---- ---- ---x xx--  . [Group A bank control] Believed to bank memory from 0x8000-0xbfff
   ---- ---- ---- --x-  . [Bank Group B] Enable bit for "0181 banking"?
   ---- ---x x--- ---x  . [Group B bank control] Believed to bank various other memory regions

   001c banking is fairly easy - it happens in a loop and writes from 8000 to bfff
   0181 banking is very weird  - it happens in a nested loop and writes from 6000-6fff, 7000-7fff, and 8000-ffbf
                                 bit 0002 turns on *just* before this happens.
*/

static UINT8 dsp56k_bank_group(device_t* cpu)
{
	UINT16 portC = dsp56k_get_peripheral_memory(cpu, 0xffe3);

	/* If bank group B is on, it overrides bank group A */
	if (portC & 0x0002)
		return BANK_GROUP_B;
	else if (portC & 0x0020)
		return BANK_GROUP_A;

	return INVALID_BANK_GROUP;
}

static UINT8 dsp56k_bank_num(device_t* cpu, UINT8 bank_group)
{
	UINT16 portC = dsp56k_get_peripheral_memory(cpu, 0xffe3);

	if (bank_group == BANK_GROUP_A)
	{
		const UINT16 bit3   = (portC & 0x0010) >> 2;
		const UINT16 bits21 = (portC & 0x000c) >> 2;
		return (bit3 | bits21);
	}
	else if (bank_group == BANK_GROUP_B)
	{
		const UINT16 bits32 = (portC & 0x0180) >> 6;
		const UINT16 bit1   = (portC & 0x0001) >> 0;
		return (bits32 | bit1);
	}
	else if (bank_group == INVALID_BANK_GROUP)
	{
		fatalerror("Plygonet: dsp56k bank num invalid.\n");
	}

	return 0;
}


/* BANK HANDLERS */
READ16_MEMBER(polygonet_state::dsp56k_ram_bank00_read)
{
	UINT8 en_group = dsp56k_bank_group(&space.device());
	UINT8 bank_num = dsp56k_bank_num(&space.device(), en_group);
	UINT32 driver_bank_offset = (en_group * dsp56k_bank00_size * 8) + (bank_num * dsp56k_bank00_size);

	return m_dsp56k_bank00_ram[driver_bank_offset + offset];
}

WRITE16_MEMBER(polygonet_state::dsp56k_ram_bank00_write)
{
	UINT8 en_group = dsp56k_bank_group(&space.device());
	UINT8 bank_num = dsp56k_bank_num(&space.device(), en_group);
	UINT32 driver_bank_offset = (en_group * dsp56k_bank00_size * 8) + (bank_num * dsp56k_bank00_size);

	COMBINE_DATA(&m_dsp56k_bank00_ram[driver_bank_offset + offset]);
}


READ16_MEMBER(polygonet_state::dsp56k_ram_bank01_read)
{
	UINT8 en_group = dsp56k_bank_group(&space.device());
	UINT8 bank_num = dsp56k_bank_num(&space.device(), en_group);
	UINT32 driver_bank_offset = (en_group * dsp56k_bank01_size * 8) + (bank_num * dsp56k_bank01_size);

	return m_dsp56k_bank01_ram[driver_bank_offset + offset];
}

WRITE16_MEMBER(polygonet_state::dsp56k_ram_bank01_write)
{
	UINT8 en_group = dsp56k_bank_group(&space.device());
	UINT8 bank_num = dsp56k_bank_num(&space.device(), en_group);
	UINT32 driver_bank_offset = (en_group * dsp56k_bank01_size * 8) + (bank_num * dsp56k_bank01_size);

	COMBINE_DATA(&m_dsp56k_bank01_ram[driver_bank_offset + offset]);

	/* For now, *always* combine P:0x7000-0x7fff with bank01 with no regard to the banking hardware. */
	m_dsp56k_p_mirror[offset] = data;
}


READ16_MEMBER(polygonet_state::dsp56k_ram_bank02_read)
{
	UINT8 en_group = dsp56k_bank_group(&space.device());
	UINT8 bank_num = dsp56k_bank_num(&space.device(), en_group);
	UINT32 driver_bank_offset = (en_group * dsp56k_bank02_size * 8) + (bank_num * dsp56k_bank02_size);

	return m_dsp56k_bank02_ram[driver_bank_offset + offset];
}

WRITE16_MEMBER(polygonet_state::dsp56k_ram_bank02_write)
{
	UINT8 en_group = dsp56k_bank_group(&space.device());
	UINT8 bank_num = dsp56k_bank_num(&space.device(), en_group);
	UINT32 driver_bank_offset = (en_group * dsp56k_bank02_size * 8) + (bank_num * dsp56k_bank02_size);

	COMBINE_DATA(&m_dsp56k_bank02_ram[driver_bank_offset + offset]);
}


READ16_MEMBER(polygonet_state::dsp56k_shared_ram_read)
{
	UINT8 en_group = dsp56k_bank_group(&space.device());
	UINT8 bank_num = dsp56k_bank_num(&space.device(), en_group);
	UINT32 driver_bank_offset = (en_group * dsp56k_shared_ram_16_size * 8) + (bank_num * dsp56k_shared_ram_16_size);

	return m_dsp56k_shared_ram_16[driver_bank_offset + offset];
}

WRITE16_MEMBER(polygonet_state::dsp56k_shared_ram_write)
{
	UINT8 en_group = dsp56k_bank_group(&space.device());
	UINT8 bank_num = dsp56k_bank_num(&space.device(), en_group);
	UINT32 driver_bank_offset = (en_group * dsp56k_shared_ram_16_size * 8) + (bank_num * dsp56k_shared_ram_16_size);

	COMBINE_DATA(&m_dsp56k_shared_ram_16[driver_bank_offset + offset]);

	/* Bank group A with offset 0 is believed to be the shared region */
	if (en_group == BANK_GROUP_A && bank_num == 0)
	{
		if (offset % 2)
			m_shared_ram[offset>>1] = ((m_dsp56k_shared_ram_16[offset-1]) << 16) | m_dsp56k_shared_ram_16[offset];
		else
			m_shared_ram[offset>>1] = ((m_dsp56k_shared_ram_16[offset])   << 16) | m_dsp56k_shared_ram_16[offset+1];
	}
}


READ16_MEMBER(polygonet_state::dsp56k_ram_bank04_read)
{
	UINT8 en_group = dsp56k_bank_group(&space.device());
	UINT8 bank_num = dsp56k_bank_num(&space.device(), en_group);
	UINT32 driver_bank_offset = (en_group * dsp56k_bank04_size * 8) + (bank_num * dsp56k_bank04_size);

	return m_dsp56k_bank04_ram[driver_bank_offset + offset];
}

WRITE16_MEMBER(polygonet_state::dsp56k_ram_bank04_write)
{
	UINT8 en_group = dsp56k_bank_group(&space.device());
	UINT8 bank_num = dsp56k_bank_num(&space.device(), en_group);
	UINT32 driver_bank_offset = (en_group * dsp56k_bank04_size * 8) + (bank_num * dsp56k_bank04_size);

	COMBINE_DATA(&m_dsp56k_bank04_ram[driver_bank_offset + offset]);
}


/**********************************************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, polygonet_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM_WRITE(plygonet_palette_w) AM_SHARE("paletteram")
	AM_RANGE(0x400000, 0x40001f) AM_DEVREADWRITE16_LEGACY("k053936", k053936_ctrl_r, k053936_ctrl_w, 0xffffffff)
	AM_RANGE(0x440000, 0x440fff) AM_READWRITE(polygonet_roz_ram_r, polygonet_roz_ram_w)
	AM_RANGE(0x480000, 0x4bffff) AM_DEVREAD_LEGACY("eeprom", polygonet_eeprom_r)
	AM_RANGE(0x4C0000, 0x4fffff) AM_WRITE(polygonet_eeprom_w)
	AM_RANGE(0x500000, 0x503fff) AM_RAM_WRITE(shared_ram_write) AM_BASE(m_shared_ram)
	AM_RANGE(0x504000, 0x504003) AM_WRITE(dsp_w_lines)
	AM_RANGE(0x506000, 0x50600f) AM_READWRITE(dsp_host_interface_r, dsp_host_interface_w)
	AM_RANGE(0x540000, 0x540fff) AM_READWRITE(polygonet_ttl_ram_r, polygonet_ttl_ram_w)
	AM_RANGE(0x541000, 0x54101f) AM_RAM
	AM_RANGE(0x580000, 0x5807ff) AM_RAM
	AM_RANGE(0x580800, 0x580803) AM_READ(network_r) AM_WRITENOP	/* network RAM | registers? */
	AM_RANGE(0x600004, 0x600007) AM_WRITE(sound_w)
	AM_RANGE(0x600008, 0x60000b) AM_READ(sound_r)
	AM_RANGE(0x640000, 0x640003) AM_WRITE(sound_irq_w)
	AM_RANGE(0x680000, 0x680003) AM_WRITE(watchdog_reset32_w)
	AM_RANGE(0x700000, 0x73ffff) AM_READ(psac_rom_r)
	AM_RANGE(0x780000, 0x79ffff) AM_READ(ttl_rom_r)
	AM_RANGE(0xff8000, 0xffffff) AM_RAM
ADDRESS_MAP_END

/**********************************************************************************/

static ADDRESS_MAP_START( dsp_program_map, AS_PROGRAM, 16, polygonet_state )
	AM_RANGE(0x7000, 0x7fff) AM_RAM AM_BASE(m_dsp56k_p_mirror)	/* Unsure of size, but 0x1000 matches bank01 */
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE(m_dsp56k_p_8000)
	AM_RANGE(0xc000, 0xc000) AM_READ(dsp56k_bootload_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp_data_map, AS_DATA, 16, polygonet_state )
	AM_RANGE(0x0800, 0x5fff) AM_RAM      /* Appears to not be affected by banking? */
	AM_RANGE(0x6000, 0x6fff) AM_READWRITE(dsp56k_ram_bank00_read, dsp56k_ram_bank00_write)
	AM_RANGE(0x7000, 0x7fff) AM_READWRITE(dsp56k_ram_bank01_read, dsp56k_ram_bank01_write)	/* Mirrored in program space @ 0x7000 */
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE(dsp56k_ram_bank02_read, dsp56k_ram_bank02_write)
	AM_RANGE(0xc000, 0xdfff) AM_READWRITE(dsp56k_shared_ram_read, dsp56k_shared_ram_write)
	AM_RANGE(0xe000, 0xffbf) AM_READWRITE(dsp56k_ram_bank04_read, dsp56k_ram_bank04_write)
ADDRESS_MAP_END

/**********************************************************************************/

static void reset_sound_region(running_machine &machine)
{
	polygonet_state *state = machine.driver_data<polygonet_state>();
	memory_set_bankptr(machine, "bank2", machine.region("soundcpu")->base() + 0x10000 + state->m_cur_sound_region*0x4000);
}

WRITE8_MEMBER(polygonet_state::sound_bankswitch_w)
{
	m_cur_sound_region = (data & 0x1f);
	reset_sound_region(machine());
}

static INTERRUPT_GEN(audio_interrupt)
{
	device_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, polygonet_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")
	AM_RANGE(0x0000, 0xbfff) AM_WRITENOP
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe22f) AM_DEVREADWRITE("konami1", k054539_device, read, write)
	AM_RANGE(0xe230, 0xe3ff) AM_RAM
	AM_RANGE(0xe400, 0xe62f) AM_DEVREADWRITE("konami2", k054539_device, read, write)
	AM_RANGE(0xe630, 0xe7ff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_WRITE(soundlatch3_w)
	AM_RANGE(0xf002, 0xf002) AM_READ(soundlatch_r)
	AM_RANGE(0xf003, 0xf003) AM_READ(soundlatch2_r)
	AM_RANGE(0xf800, 0xf800) AM_WRITE(sound_bankswitch_w)
	AM_RANGE(0xfff1, 0xfff3) AM_WRITENOP
ADDRESS_MAP_END

static const k054539_interface k054539_config =
{
	"shared"
};

/**********************************************************************************/
static const gfx_layout bglayout =
{
	16,16,
	1024,
	4,
	{ 0, 1, 2, 3 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	  8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4, 8*4,
	  9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },

	  128*8
};

static GFXDECODE_START( plygonet )
	GFXDECODE_ENTRY( "gfx2", 0, bglayout, 0x0000, 64 )
GFXDECODE_END

static MACHINE_START(polygonet)
{
	logerror("Polygonet machine start\n");

	/* Set the dsp56k lines */
	/* It's presumed the hardware has hard-wired operating mode 1 (MODA = 1, MODB = 0) */
	/* TODO: This should work, but the MAME core appears to do something funny.
             Not a big deal - it's hacked in dsp_w_lines. */
	//cputag_set_input_line(machine, "dsp", INPUT_LINE_RESET, ASSERT_LINE);
	//cputag_set_input_line(machine, "dsp", DSP56K_IRQ_MODA, ASSERT_LINE);
	//cputag_set_input_line(machine, "dsp", DSP56K_IRQ_MODB, CLEAR_LINE);
}

static const k053936_interface polygonet_k053936_intf =
{
	0, 0, 0	/* wrap, xoff, yoff */
};

static MACHINE_CONFIG_START( plygonet, polygonet_state )

	MCFG_CPU_ADD("maincpu", M68EC020, 16000000)	/* 16 MHz (xtal is 32.0 MHz) */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen", polygonet_interrupt)

	MCFG_CPU_ADD("dsp", DSP56156, 40000000)		/* xtal is 40.0 MHz, DSP has an internal divide-by-2 */
	MCFG_CPU_PROGRAM_MAP(dsp_program_map)
	MCFG_CPU_DATA_MAP(dsp_data_map)

	MCFG_CPU_ADD("soundcpu", Z80, 8000000)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT(audio_interrupt, 480)

	MCFG_MACHINE_START(polygonet)

	MCFG_GFXDECODE(plygonet)

	MCFG_EEPROM_ADD("eeprom", eeprom_intf)

	/* TODO: TEMPORARY!  UNTIL A MORE LOCALIZED SYNC CAN BE MADE */
	MCFG_QUANTUM_TIME(attotime::from_hz(1200000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(64, 64+368-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(polygonet)

	MCFG_PALETTE_LENGTH(32768)

	MCFG_VIDEO_START(polygonet)

	MCFG_K053936_ADD("k053936", polygonet_k053936_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_K054539_ADD("konami1", 48000, k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.75)

	MCFG_K054539_ADD("konami2", 48000, k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.75)
MACHINE_CONFIG_END


/**********************************************************************************/
static INPUT_PORTS_START( polygonet )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )	/* SW1 (changes player color).  It's mapped on the JAMMA connector and plugs into an external switch mech. */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 )	/* SW2 (changes player color).  It's mapped on the JAMMA connector and plugs into an external switch mech. */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )	PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )	PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )	PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )	PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
INPUT_PORTS_END

static INPUT_PORTS_START( polynetw )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )	/* SW1 (changes player color).  It's mapped on the JAMMA connector and plugs into an external switch mech. */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )	/* SW2 (changes player color).  It's mapped on the JAMMA connector and plugs into an external switch mech. */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )	PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )	PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )	PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )	PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
INPUT_PORTS_END


/**********************************************************************************/
static DRIVER_INIT(polygonet)
{
	polygonet_state *state = machine.driver_data<polygonet_state>();

	/* Set default bankswitch */
	state->m_cur_sound_region = 2;
	reset_sound_region(machine);

	/* Allocate space for the dsp56k banking */
	memset(state->m_dsp56k_bank00_ram, 0, sizeof(state->m_dsp56k_bank00_ram));
	memset(state->m_dsp56k_bank01_ram, 0, sizeof(state->m_dsp56k_bank01_ram));
	memset(state->m_dsp56k_bank02_ram, 0, sizeof(state->m_dsp56k_bank02_ram));
	memset(state->m_dsp56k_shared_ram_16, 0, sizeof(state->m_dsp56k_shared_ram_16));
	memset(state->m_dsp56k_bank04_ram, 0, sizeof(state->m_dsp56k_bank04_ram));

	/* The dsp56k occasionally executes out of mapped memory */
	address_space *space = machine.device<dsp56k_device>("dsp")->space(AS_PROGRAM);
	state->m_dsp56k_update_handler = space->set_direct_update_handler(direct_update_delegate(FUNC(plygonet_dsp56k_direct_handler), &machine));

    /* save states */
	state->save_item(NAME(state->m_dsp56k_bank00_ram));
	state->save_item(NAME(state->m_dsp56k_bank01_ram));
	state->save_item(NAME(state->m_dsp56k_bank02_ram));
	state->save_item(NAME(state->m_dsp56k_shared_ram_16));
	state->save_item(NAME(state->m_dsp56k_bank04_ram));
	state->save_item(NAME(state->m_cur_sound_region));
}


/**********************************************************************************/
ROM_START( plygonet )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0)
	ROM_LOAD32_BYTE( "305uaa01.4k", 0x000003, 512*1024, CRC(8bdb6c95) SHA1(e981833842f8fd89b9726901fbe2058444204792) ) /* Boards exist without the "UA" in the label IE: 305a01, ect... */
	ROM_LOAD32_BYTE( "305uaa02.2k", 0x000002, 512*1024, CRC(4d7e32b3) SHA1(25731526535036972577637d186f02ae467296bd) )
	ROM_LOAD32_BYTE( "305uaa03.2h", 0x000001, 512*1024, CRC(36e4e3fe) SHA1(e8fcad4f196c9b225a0fbe70791493ff07c648a9) )
	ROM_LOAD32_BYTE( "305uaa04.4h", 0x000000, 512*1024, CRC(d8394e72) SHA1(eb6bcf8aedb9ba5843204ab8aacb735cbaafb74d) )

	/* Z80 sound program */
	ROM_REGION( 0x30000, "soundcpu", 0 )
	ROM_LOAD("305b05.7b", 0x000000, 0x20000, CRC(2d3d9654) SHA1(784a409df47cee877e507b8bbd3610d161d63753) )
	ROM_RELOAD( 0x10000, 0x20000)

	/* TTL text plane tiles */
	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "305b06.18g", 0x000000, 0x20000, CRC(decd6e42) SHA1(4c23dcb1d68132d3381007096e014ee4b6007086) )

	/* '936 tiles */
	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "305b07.20d", 0x000000, 0x40000, CRC(e4320bc3) SHA1(b0bb2dac40d42f97da94516d4ebe29b1c3d77c37) )

	/* sound data */
	ROM_REGION( 0x200000, "shared", 0 )
	ROM_LOAD( "305b08.2e", 0x000000, 0x200000, CRC(874607df) SHA1(763b44a80abfbc355bcb9be8bf44373254976019) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "plygonet.nv", 0x0000, 0x0080, CRC(627748ac) SHA1(ea1b06739fee235b049ff8daffff7d43cb093112) )
ROM_END

ROM_START( polynetw )
	/* main program */
	ROM_REGION( 0x200000, "maincpu", 0)
	ROM_LOAD32_BYTE( "305jaa01.4k", 0x000003, 0x080000, CRC(ea889bd9) SHA1(102e7c0f0c064662c0f6137ad5da97a9ccd49a97) )
	ROM_LOAD32_BYTE( "305jaa02.2k", 0x000002, 0x080000, CRC(d0710379) SHA1(cf0970d63e8d021edf2d404838c658a5b7cb8fb8) )
	ROM_LOAD32_BYTE( "305jaa03.2h", 0x000001, 0x080000, CRC(278b5928) SHA1(2ea96054e2ef637731cd64f2bef0b5b2bbe7e24f) )
	ROM_LOAD32_BYTE( "305jaa04.4h", 0x000000, 0x080000, CRC(b069353b) SHA1(12fbe2df09328bb7193e89a49d84a61eab5bfdcb) )

	/* Z80 sound program */
	ROM_REGION( 0x30000, "soundcpu", 0 )
	ROM_LOAD( "305jaa05.7b", 0x000000, 0x020000, CRC(06053db6) SHA1(c7d43c2650d949ee552a49db93dece842c17e68d) )
	ROM_RELOAD( 0x10000, 0x20000)

	/* TTL text plane tiles */
	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "305a06.18g", 0x000000, 0x020000, CRC(4b9b7e9c) SHA1(8c3c0f1ec7e26fd9552f6da1e6bdd7ff4453ba57) )

	/* '936 tiles */
	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "305a07.20d", 0x000000, 0x020000, CRC(0959283b) SHA1(482caf96e8e430b87810508b1a1420cd3b58f203) )

	/* sound data */
	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "305a08.2e", 0x000000, 0x200000, CRC(7ddb8a52) SHA1(3199b347fc433ffe0de8521001df77672d40771e) )
	ROM_LOAD( "305a09.3e", 0x200000, 0x200000, CRC(6da1be58) SHA1(d63ac16ac551193ff8a6036724fb59e1d702e06b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "polynetw.nv", 0x0000, 0x0080, CRC(8f39d644) SHA1(8733e1a288ba20c4b04b3aedde52801d05cebdf9) )
ROM_END

/*          ROM       parent   machine   inp        init */
GAME( 1993, plygonet, 0,       plygonet, polygonet, polygonet, ROT90, "Konami", "Polygonet Commanders (ver UAA)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1993, polynetw, 0,       plygonet, polynetw,  polygonet, ROT90, "Konami", "Poly-Net Warriors (ver JAA)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )

