/* Queen */

/*

TODO:
- goes to la-la-land almost immediately:
000F4023: sbb     [42F9h],ah
000F4027: loop    0F4003h
000F4003: ror     word ptr [si+48B2h],1
000F4007: cmp     ax,44B2h
000F400A: retf
000903D3: add     [bx+si],al
000903D5: add     [bx+si],al
(Reads stack from BIOS ROM?)


Produttore  STG
N.revisione
CPU main PCB is a standard EPIA
ROMs    epia BIOS + solid state HD

1x VIA EPIA5000EAG (main PCB) with:
VT8231 South Bridge
VIA Eden Processor
VIA EPIA Companion Chip VT1612A (Audio CODEC)
VIA EPIA Companion Chip VT6103 (Networking)
processor speed is 533MHz <- likely to be a Celeron or a Pentium III class CPU -AS

 it's a 2002 era PC at least based on the BIOS,
  almost certainly newer than the standard 'PENTIUM' CPU

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"
#include "machine/pci.h"
#include "machine/8042kbdc.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "video/pc_vga.h"


class queen_state : public driver_device
{
public:
	queen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_pit8254(*this, "pit8254"),
		  m_dma8237_1(*this, "dma8237_1"),
		  m_dma8237_2(*this, "dma8237_2"),
		  m_pic8259_1(*this, "pic8259_1"),
		  m_pic8259_2(*this, "pic8259_2")
		  { }

	UINT32 *m_bios_ram;
	UINT32 *m_bios_ext1_ram;
	UINT32 *m_bios_ext2_ram;
	UINT32 *m_bios_ext3_ram;
	UINT32 *m_bios_ext4_ram;
	UINT32 *m_isa_ram1;
	UINT32 *m_isa_ram2;
	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
	UINT8 m_mxtc_config_reg[256];
	UINT8 m_piix4_config_reg[4][256];

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<pit8254_device> m_pit8254;
	required_device<i8237_device> m_dma8237_1;
	required_device<i8237_device> m_dma8237_2;
	required_device<pic8259_device> m_pic8259_1;
	required_device<pic8259_device> m_pic8259_2;

	DECLARE_READ8_MEMBER( get_slave_ack );

	DECLARE_WRITE32_MEMBER( isa_ram1_w );
	DECLARE_WRITE32_MEMBER( isa_ram2_w );

	DECLARE_WRITE32_MEMBER( bios_ext1_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext2_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext3_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext4_ram_w );

	DECLARE_WRITE32_MEMBER( bios_ram_w );
	DECLARE_READ8_MEMBER(at_page8_r);
	DECLARE_WRITE8_MEMBER(at_page8_w);
	DECLARE_READ8_MEMBER(pc_dma_read_byte);
	DECLARE_WRITE8_MEMBER(pc_dma_write_byte);
	DECLARE_READ8_MEMBER(vga_setting);
	DECLARE_READ32_MEMBER(ide_r);
	DECLARE_WRITE32_MEMBER(ide_w);
	DECLARE_READ32_MEMBER(fdc_r);
	DECLARE_WRITE32_MEMBER(fdc_w);
	DECLARE_READ8_MEMBER(at_dma8237_2_r);
	DECLARE_WRITE8_MEMBER(at_dma8237_2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(pc_dack0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack3_w);
	DECLARE_WRITE_LINE_MEMBER(queen_pic8259_1_set_int_line);
};


// Intel 82439TX System Controller (MXTC)

static UINT8 mxtc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	queen_state *state = busdevice->machine().driver_data<queen_state>();
//  mame_printf_debug("MXTC: read %d, %02X\n", function, reg);

	return state->m_mxtc_config_reg[reg];
}

static void mxtc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	queen_state *state = busdevice->machine().driver_data<queen_state>();
	printf("MXTC: write %d, %02X, %02X\n",  function, reg, data);

	/*
    memory banking with North Bridge:
    0x59 (PAM0) xxxx ---- BIOS area 0xf0000-0xfffff
                ---- xxxx Reserved
    0x5a (PAM1) xxxx ---- ISA add-on BIOS 0xc4000 - 0xc7fff
                ---- xxxx ISA add-on BIOS 0xc0000 - 0xc3fff
    0x5b (PAM2) xxxx ---- ISA add-on BIOS 0xcc000 - 0xcffff
                ---- xxxx ISA add-on BIOS 0xc8000 - 0xcbfff
    0x5c (PAM3) xxxx ---- ISA add-on BIOS 0xd4000 - 0xd7fff
                ---- xxxx ISA add-on BIOS 0xd0000 - 0xd3fff
    0x5d (PAM4) xxxx ---- ISA add-on BIOS 0xdc000 - 0xdffff
                ---- xxxx ISA add-on BIOS 0xd8000 - 0xdbfff
    0x5e (PAM5) xxxx ---- BIOS extension 0xe4000 - 0xe7fff
                ---- xxxx BIOS extension 0xe0000 - 0xe3fff
    0x5f (PAM6) xxxx ---- BIOS extension 0xec000 - 0xeffff
                ---- xxxx BIOS extension 0xe8000 - 0xebfff

    3210 -> 3 = reserved, 2 = Cache Enable, 1 = Write Enable, 0 = Read Enable
    */

	switch(reg)
	{
		case 0x59: // PAM0
		{
			if (data & 0x10)		// enable RAM access to region 0xf0000 - 0xfffff
				state->membank("bios_bank")->set_base(state->m_bios_ram);
			else					// disable RAM access (reads go to BIOS ROM)
				state->membank("bios_bank")->set_base(state->memregion("bios")->base() + 0x10000);
			break;
		}
		case 0x5a: // PAM1
		{
			if (data & 0x1)
				state->membank("video_bank1")->set_base(state->m_isa_ram1);
			else
				state->membank("video_bank1")->set_base(state->memregion("video_bios")->base() + 0);

			if (data & 0x10)
				state->membank("video_bank2")->set_base(state->m_isa_ram2);
			else
				state->membank("video_bank2")->set_base(state->memregion("video_bios")->base() + 0x4000);

			break;
		}
		case 0x5e: // PAM5
		{
			if (data & 0x1)
				state->membank("bios_ext1")->set_base(state->m_bios_ext1_ram);
			else
				state->membank("bios_ext1")->set_base(state->memregion("bios")->base() + 0);

			if (data & 0x10)
				state->membank("bios_ext2")->set_base(state->m_bios_ext2_ram);
			else
				state->membank("bios_ext2")->set_base(state->memregion("bios")->base() + 0x4000);

			break;
		}
		case 0x5f: // PAM6
		{
			if (data & 0x1)
				state->membank("bios_ext3")->set_base(state->m_bios_ext3_ram);
			else
				state->membank("bios_ext3")->set_base(state->memregion("bios")->base() + 0x8000);

			if (data & 0x10)
				state->membank("bios_ext4")->set_base(state->m_bios_ext4_ram);
			else
				state->membank("bios_ext4")->set_base(state->memregion("bios")->base() + 0xc000);

			break;
		}
	}

	state->m_mxtc_config_reg[reg] = data;
}

static void intel82439tx_init(running_machine &machine)
{
	queen_state *state = machine.driver_data<queen_state>();
	state->m_mxtc_config_reg[0x60] = 0x02;
	state->m_mxtc_config_reg[0x61] = 0x02;
	state->m_mxtc_config_reg[0x62] = 0x02;
	state->m_mxtc_config_reg[0x63] = 0x02;
	state->m_mxtc_config_reg[0x64] = 0x02;
	state->m_mxtc_config_reg[0x65] = 0x02;
}

static UINT32 intel82439tx_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82439tx_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		mxtc_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		mxtc_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		mxtc_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		mxtc_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}

// Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)

static UINT8 piix4_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	queen_state *state = busdevice->machine().driver_data<queen_state>();
//  mame_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	queen_state *state = busdevice->machine().driver_data<queen_state>();
//  mame_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);
	state->m_piix4_config_reg[function][reg] = data;
}

static UINT32 intel82371ab_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82371ab_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		piix4_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		piix4_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		piix4_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		piix4_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}


WRITE32_MEMBER(queen_state::isa_ram1_w)
{
	if (m_mxtc_config_reg[0x5a] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram1 + offset);
	}
}

WRITE32_MEMBER(queen_state::isa_ram2_w)
{
	if (m_mxtc_config_reg[0x5a] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram2 + offset);
	}
}

WRITE32_MEMBER(queen_state::bios_ext1_ram_w)
{
	if (m_mxtc_config_reg[0x5e] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext1_ram + offset);
	}
}


WRITE32_MEMBER(queen_state::bios_ext2_ram_w)
{
	if (m_mxtc_config_reg[0x5e] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext2_ram + offset);
	}
}


WRITE32_MEMBER(queen_state::bios_ext3_ram_w)
{
	if (m_mxtc_config_reg[0x5f] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext3_ram + offset);
	}
}


WRITE32_MEMBER(queen_state::bios_ext4_ram_w)
{
	if (m_mxtc_config_reg[0x5f] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext4_ram + offset);
	}
}


WRITE32_MEMBER(queen_state::bios_ram_w)
{
	if (m_mxtc_config_reg[0x59] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram + offset);
	}
}

READ32_MEMBER(queen_state::ide_r)
{
	device_t *device = machine().device("ide");
	return ide_controller32_r(device, 0x1f0/4 + offset, mem_mask);
}

WRITE32_MEMBER(queen_state::ide_w)
{
	device_t *device = machine().device("ide");
	ide_controller32_w(device, 0x1f0/4 + offset, data, mem_mask);
}

READ32_MEMBER(queen_state::fdc_r)
{
	device_t *device = machine().device("ide");
	return ide_controller32_r(device, 0x3f0/4 + offset, mem_mask);
}

WRITE32_MEMBER(queen_state::fdc_w)
{
	device_t *device = machine().device("ide");
	//mame_printf_debug("FDC: write %08X, %08X, %08X\n", data, offset, mem_mask);
	ide_controller32_w(device, 0x3f0/4 + offset, data, mem_mask);
}

READ8_MEMBER(queen_state::at_page8_r)
{
	UINT8 data = m_at_pages[offset % 0x10];

	switch(offset % 8) {
	case 1:
		data = m_dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = m_dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = m_dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = m_dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


WRITE8_MEMBER(queen_state::at_page8_w)
{
	m_at_pages[offset % 0x10] = data;

	switch(offset % 8) {
	case 1:
		m_dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		m_dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		m_dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		m_dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}


READ8_MEMBER(queen_state::at_dma8237_2_r)
{
	device_t *device = machine().device("dma8237_2");
	return i8237_r(device, offset / 2);
}

WRITE8_MEMBER(queen_state::at_dma8237_2_w)
{
	device_t *device = machine().device("dma8237_2");
	i8237_w(device, offset / 2, data);
}

WRITE_LINE_MEMBER(queen_state::pc_dma_hrq_changed)
{
	cputag_set_input_line(machine(), "maincpu", INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( m_dma8237_1, state );
}


READ8_MEMBER(queen_state::pc_dma_read_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	return space.read_byte(page_offset + offset);
}


WRITE8_MEMBER(queen_state::pc_dma_write_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	space.write_byte(page_offset + offset, data);
}

static void set_dma_channel(device_t *device, int channel, int state)
{
	queen_state *drvstate = device->machine().driver_data<queen_state>();
	if (!state) drvstate->m_dma_channel = channel;
}

WRITE_LINE_MEMBER(queen_state::pc_dack0_w){ set_dma_channel(m_dma8237_1, 0, state); }
WRITE_LINE_MEMBER(queen_state::pc_dack1_w){ set_dma_channel(m_dma8237_1, 1, state); }
WRITE_LINE_MEMBER(queen_state::pc_dack2_w){ set_dma_channel(m_dma8237_1, 2, state); }
WRITE_LINE_MEMBER(queen_state::pc_dack3_w){ set_dma_channel(m_dma8237_1, 3, state); }

static I8237_INTERFACE( dma8237_1_config )
{
	DEVCB_DRIVER_LINE_MEMBER(queen_state,pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(queen_state, pc_dma_read_byte),
	DEVCB_DRIVER_MEMBER(queen_state, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_DRIVER_LINE_MEMBER(queen_state,pc_dack0_w), DEVCB_DRIVER_LINE_MEMBER(queen_state,pc_dack1_w), DEVCB_DRIVER_LINE_MEMBER(queen_state,pc_dack2_w), DEVCB_DRIVER_LINE_MEMBER(queen_state,pc_dack3_w) }
};

static I8237_INTERFACE( dma8237_2_config )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL }
};

static ADDRESS_MAP_START( queen_map, AS_PROGRAM, 32, queen_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM
	AM_RANGE(0x000c0000, 0x000c3fff) AM_ROMBANK("video_bank1") AM_WRITE(isa_ram1_w)
	AM_RANGE(0x000c4000, 0x000c7fff) AM_ROMBANK("video_bank2") AM_WRITE(isa_ram2_w)
	AM_RANGE(0x000e0000, 0x000e3fff) AM_ROMBANK("bios_ext1") AM_WRITE(bios_ext1_ram_w)
	AM_RANGE(0x000e4000, 0x000e7fff) AM_ROMBANK("bios_ext2") AM_WRITE(bios_ext2_ram_w)
	AM_RANGE(0x000e8000, 0x000ebfff) AM_ROMBANK("bios_ext3") AM_WRITE(bios_ext3_ram_w)
	AM_RANGE(0x000ec000, 0x000effff) AM_ROMBANK("bios_ext4") AM_WRITE(bios_ext4_ram_w)
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bios_bank") AM_WRITE(bios_ram_w)
	AM_RANGE(0x00100000, 0x01ffffff) AM_RAM
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START( queen_io, AS_IO, 32, queen_state )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8_LEGACY("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE8_LEGACY(kbdc8042_8_r, kbdc8042_8_w, 0xffffffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffffffff) /* todo: nvram (CMOS Setup Save)*/
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(at_page8_r,	at_page8_w, 0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_READWRITE8(at_dma8237_2_r, at_dma8237_2_w, 0xffffffff)
	AM_RANGE(0x00e8, 0x00ef) AM_NOP

	AM_RANGE(0x01f0, 0x01f7) AM_READWRITE(ide_r, ide_w)
	AM_RANGE(0x03f0, 0x03f7) AM_READWRITE(fdc_r, fdc_w)

	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
ADDRESS_MAP_END

static const struct pit8253_config queen_pit8254_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir0_w)
		}, {
			4772720/4,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};

WRITE_LINE_MEMBER(queen_state::queen_pic8259_1_set_int_line)
{
	device_set_input_line(m_maincpu, 0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER( queen_state::get_slave_ack )
{
	if (offset==2) { // IRQ = 2
		logerror("pic8259_slave_ACK!\n");
		return pic8259_acknowledge(m_pic8259_2);
	}
	return 0x00;
}

static const struct pic8259_interface queen_pic8259_1_config =
{
	DEVCB_DRIVER_LINE_MEMBER(queen_state,queen_pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_MEMBER(queen_state,get_slave_ack)
};

static const struct pic8259_interface queen_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

static void set_gate_a20(running_machine &machine, int a20)
{
	queen_state *state = machine.driver_data<queen_state>();

	device_set_input_line(state->m_maincpu, INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine &machine, int state)
{
	queen_state *drvstate = machine.driver_data<queen_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static int queen_get_out2(running_machine &machine)
{
	queen_state *state = machine.driver_data<queen_state>();
	return pit8253_get_output(state->m_pit8254, 2 );
}

static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, NULL, queen_get_out2
};

static void queen_set_keyb_int(running_machine &machine, int state)
{
	queen_state *drvstate = machine.driver_data<queen_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static IRQ_CALLBACK(irq_callback)
{
	queen_state *state = device->machine().driver_data<queen_state>();
	return pic8259_acknowledge( state->m_pic8259_1);
}

static void ide_interrupt(device_t *device, int state)
{
	queen_state *drvstate = device->machine().driver_data<queen_state>();
	pic8259_ir6_w(drvstate->m_pic8259_2, state);
}

static READ8_HANDLER( vga_setting ) { return 0xff; } // hard-code to color

static MACHINE_START( queen )
{
	queen_state *state = machine.driver_data<queen_state>();

	state->m_bios_ram = auto_alloc_array(machine, UINT32, 0x10000/4);
	state->m_bios_ext1_ram = auto_alloc_array(machine, UINT32, 0x4000/4);
	state->m_bios_ext2_ram = auto_alloc_array(machine, UINT32, 0x4000/4);
	state->m_bios_ext3_ram = auto_alloc_array(machine, UINT32, 0x4000/4);
	state->m_bios_ext4_ram = auto_alloc_array(machine, UINT32, 0x4000/4);
	state->m_isa_ram1 = auto_alloc_array(machine, UINT32, 0x4000/4);
	state->m_isa_ram2 = auto_alloc_array(machine, UINT32, 0x4000/4);

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, queen_set_keyb_int);

	device_set_irq_callback(state->m_maincpu, irq_callback);
	intel82439tx_init(machine);

	kbdc8042_init(machine, &at8042);
	pc_vga_init(machine, vga_setting, NULL);
	pc_vga_io_init(machine, machine.device("maincpu")->memory().space(AS_PROGRAM), 0xa0000, machine.device("maincpu")->memory().space(AS_IO), 0x0000);
}

static MACHINE_RESET( queen )
{
	machine.root_device().membank("bios_bank")->set_base(machine.root_device().memregion("bios")->base() + 0x30000);
	machine.root_device().membank("bios_ext1")->set_base(machine.root_device().memregion("bios")->base() + 0x20000);
	machine.root_device().membank("bios_ext2")->set_base(machine.root_device().memregion("bios")->base() + 0x24000);
	machine.root_device().membank("bios_ext3")->set_base(machine.root_device().memregion("bios")->base() + 0x28000);
	machine.root_device().membank("bios_ext4")->set_base(machine.root_device().memregion("bios")->base() + 0x2c000);
	machine.root_device().membank("video_bank1")->set_base(machine.root_device().memregion("video_bios")->base() + 0);
	machine.root_device().membank("video_bank2")->set_base(machine.root_device().memregion("video_bios")->base() + 0x4000);
}





static MACHINE_CONFIG_START( queen, queen_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 533000000/16) // Celeron or Pentium 3, 533 Mhz
	MCFG_CPU_PROGRAM_MAP(queen_map)
	MCFG_CPU_IO_MAP(queen_io)

	MCFG_MACHINE_START(queen)
	MCFG_MACHINE_RESET(queen)

	MCFG_PIT8254_ADD( "pit8254", queen_pit8254_config )
	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )
	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )
	MCFG_PIC8259_ADD( "pic8259_1", queen_pic8259_1_config )
	MCFG_PIC8259_ADD( "pic8259_2", queen_pic8259_2_config )

	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, NULL, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(7, NULL, intel82371ab_pci_r, intel82371ab_pci_w)

	MCFG_IDE_CONTROLLER_ADD("ide", ide_interrupt, ide_devices, "hdd", NULL, true)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )
MACHINE_CONFIG_END




ROM_START( queen )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "bios-original.bin", 0x00000, 0x40000, CRC(feb542d4) SHA1(3cc5d8aeb0e3b7d9ed33248a4f3dc507d29debd9) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "pqiidediskonmodule", 0,SHA1(a56efcc711b1c5a2e63160b3088001a8c4fb56c2) )
ROM_END


GAME( 2002?, queen,  0,    queen, at_keyboard,  0, ROT0, "STG", "Queen?", GAME_IS_SKELETON )
