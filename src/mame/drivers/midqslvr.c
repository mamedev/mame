/***************************************************************************

    Midway Quicksilver skeleton driver

    TODO:
    - offrthnd: illegal opcode tripped just after that PIIX4 is recognized

    Main CPU : Intel Celeron 333/366MHz
    Motherboard : Intel SE440BX-2
    RAM : 64MB PC100-222-620 non-ecc
    Sound: Integrated YMF740G
    Networking: SMC EZ Card 10 / SMC1208T (probably 10ec:8029 1113:1208)
    Graphics Chips : Quantum Obsidian 3DFX
    Storage : Hard Drive

    Chipsets (440BX AGPset):
    - 82371EB PCI-ISA bridge
    - 82371EB Power Management Controller
    - 82371AB/EB Universal Host Controller (USB UHCI)
    - 82371AB/EB PCI Bus Master IDE Controller

***************************************************************************/


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

class midqslvr_state : public driver_device
{
public:
	midqslvr_state(const machine_config &mconfig, device_type type, const char *tag)
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
	DECLARE_WRITE_LINE_MEMBER(midqslvr_pic8259_1_set_int_line);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_READ8_MEMBER(vga_setting);
};


// Intel 82439TX System Controller (MXTC)

static UINT8 mxtc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	midqslvr_state *state = busdevice->machine().driver_data<midqslvr_state>();
//  mame_printf_debug("MXTC: read %d, %02X\n", function, reg);

	if((reg & 0xfc) == 0 && function == 0) // return vendor ID
		return (0x71008086 >> (reg & 3)*8) & 0xff;

	return state->m_mxtc_config_reg[reg];
}

static void mxtc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	midqslvr_state *state = busdevice->machine().driver_data<midqslvr_state>();
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
				state->membank("bios_bank")->set_base(state->memregion("bios")->base() + 0x70000);
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
				state->membank("bios_ext1")->set_base(state->memregion("bios")->base() + 0x60000);

			if (data & 0x10)
				state->membank("bios_ext2")->set_base(state->m_bios_ext2_ram);
			else
				state->membank("bios_ext2")->set_base(state->memregion("bios")->base() + 0x64000);

			break;
		}
		case 0x5f: // PAM6
		{
			if (data & 0x1)
				state->membank("bios_ext3")->set_base(state->m_bios_ext3_ram);
			else
				state->membank("bios_ext3")->set_base(state->memregion("bios")->base() + 0x68000);

			if (data & 0x10)
				state->membank("bios_ext4")->set_base(state->m_bios_ext4_ram);
			else
				state->membank("bios_ext4")->set_base(state->memregion("bios")->base() + 0x6c000);

			break;
		}
	}

	state->m_mxtc_config_reg[reg] = data;
}

static void intel82439tx_init(running_machine &machine)
{
	midqslvr_state *state = machine.driver_data<midqslvr_state>();
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
	address_space &space = busdevice->machine().firstcpu->space( AS_PROGRAM );
	midqslvr_state *state = busdevice->machine().driver_data<midqslvr_state>();

	function &= 3;

	if((reg & 0xfc) == 0) // return vendor ID
		return (((0x71108086 | (function & 3) << 16) >> (reg & 3)*8) & 0xff);

	if(reg == 0xe)
	{
		const UINT8 header_type_val[4] = { 0x80, 0x00, 0x00, 0x00 };
		return header_type_val[function];
	}

	if((reg & 0xfc) == 0x8)
	{
		/* TODO: reg 8 indicates Revision ID */
		const UINT32 class_code_val[4] = { 0x06010000, 0x01018000, 0x0c030000, 0x06800000 };

		return (((class_code_val[function]) >> (reg & 3)*8) & 0xff);
	}

	printf("%08x PIIX4: read %d, %02X\n", space.device().safe_pc(), function, reg);

	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	midqslvr_state *state = busdevice->machine().driver_data<midqslvr_state>();
	printf("PIIX4: write %d, %02X, %02X\n", function, reg, data);

	function &= 3;

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


WRITE32_MEMBER(midqslvr_state::isa_ram1_w)
{
	if (m_mxtc_config_reg[0x5a] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram1 + offset);
	}
}

WRITE32_MEMBER(midqslvr_state::isa_ram2_w)
{
	if (m_mxtc_config_reg[0x5a] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram2 + offset);
	}
}

WRITE32_MEMBER(midqslvr_state::bios_ext1_ram_w)
{
	if (m_mxtc_config_reg[0x5e] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext1_ram + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ext2_ram_w)
{
	if (m_mxtc_config_reg[0x5e] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext2_ram + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ext3_ram_w)
{
	if (m_mxtc_config_reg[0x5f] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext3_ram + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ext4_ram_w)
{
	if (m_mxtc_config_reg[0x5f] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext4_ram + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ram_w)
{
	if (m_mxtc_config_reg[0x59] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram + offset);
	}
}

READ32_MEMBER(midqslvr_state::ide_r)
{
	device_t *device = machine().device("ide");
	return ide_controller32_r(device, space, 0x1f0/4 + offset, mem_mask);
}

WRITE32_MEMBER(midqslvr_state::ide_w)
{
	device_t *device = machine().device("ide");
	ide_controller32_w(device, space, 0x1f0/4 + offset, data, mem_mask);
}

READ32_MEMBER(midqslvr_state::fdc_r)
{
	device_t *device = machine().device("ide");
	return ide_controller32_r(device, space, 0x3f0/4 + offset, mem_mask);
}

WRITE32_MEMBER(midqslvr_state::fdc_w)
{
	device_t *device = machine().device("ide");
	//mame_printf_debug("FDC: write %08X, %08X, %08X\n", data, offset, mem_mask);
	ide_controller32_w(device, space, 0x3f0/4 + offset, data, mem_mask);
}

READ8_MEMBER(midqslvr_state::at_page8_r)
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


WRITE8_MEMBER(midqslvr_state::at_page8_w)
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

READ8_MEMBER(midqslvr_state::at_dma8237_2_r)
{
	device_t *device = machine().device("dma8237_2");
	return i8237_r(device, space, offset / 2);
}

WRITE8_MEMBER(midqslvr_state::at_dma8237_2_w)
{
	device_t *device = machine().device("dma8237_2");
	i8237_w(device, space, offset / 2, data);
}

WRITE_LINE_MEMBER(midqslvr_state::pc_dma_hrq_changed)
{
	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( m_dma8237_1, state );
}


READ8_MEMBER(midqslvr_state::pc_dma_read_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	return space.read_byte(page_offset + offset);
}


WRITE8_MEMBER(midqslvr_state::pc_dma_write_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	space.write_byte(page_offset + offset, data);
}

static void set_dma_channel(device_t *device, int channel, int state)
{
	midqslvr_state *drvstate = device->machine().driver_data<midqslvr_state>();
	if (!state) drvstate->m_dma_channel = channel;
}

WRITE_LINE_MEMBER(midqslvr_state::pc_dack0_w){ set_dma_channel(m_dma8237_1, 0, state); }
WRITE_LINE_MEMBER(midqslvr_state::pc_dack1_w){ set_dma_channel(m_dma8237_1, 1, state); }
WRITE_LINE_MEMBER(midqslvr_state::pc_dack2_w){ set_dma_channel(m_dma8237_1, 2, state); }
WRITE_LINE_MEMBER(midqslvr_state::pc_dack3_w){ set_dma_channel(m_dma8237_1, 3, state); }

static I8237_INTERFACE( dma8237_1_config )
{
	DEVCB_DRIVER_LINE_MEMBER(midqslvr_state,pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(midqslvr_state, pc_dma_read_byte),
	DEVCB_DRIVER_MEMBER(midqslvr_state, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_DRIVER_LINE_MEMBER(midqslvr_state,pc_dack0_w), DEVCB_DRIVER_LINE_MEMBER(midqslvr_state,pc_dack1_w), DEVCB_DRIVER_LINE_MEMBER(midqslvr_state,pc_dack2_w), DEVCB_DRIVER_LINE_MEMBER(midqslvr_state,pc_dack3_w) }
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


static ADDRESS_MAP_START(midqslvr_map, AS_PROGRAM, 32, midqslvr_state)
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
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(midqslvr_io, AS_IO, 32, midqslvr_state)
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8_LEGACY("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE8_LEGACY(kbdc8042_8_r, kbdc8042_8_w, 0xffffffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffffffff) /* todo: nvram (CMOS Setup Save)*/
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(at_page8_r,at_page8_w,0xffffffff)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_READWRITE8(at_dma8237_2_r, at_dma8237_2_w, 0xffffffff)
	AM_RANGE(0x00e8, 0x00ef) AM_NOP

	AM_RANGE(0x01f0, 0x01f7) AM_READWRITE(ide_r, ide_w)
	AM_RANGE(0x03f0, 0x03f7) AM_READWRITE(fdc_r, fdc_w)

	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
ADDRESS_MAP_END

static const struct pit8253_config midqslvr_pit8254_config =
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

WRITE_LINE_MEMBER(midqslvr_state::midqslvr_pic8259_1_set_int_line)
{
	m_maincpu->set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER( midqslvr_state::get_slave_ack )
{
	if (offset==2) { // IRQ = 2
		logerror("pic8259_slave_ACK!\n");
		return pic8259_acknowledge(m_pic8259_2);
	}
	return 0x00;
}

static const struct pic8259_interface midqslvr_pic8259_1_config =
{
	DEVCB_DRIVER_LINE_MEMBER(midqslvr_state,midqslvr_pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_MEMBER(midqslvr_state,get_slave_ack)
};

static const struct pic8259_interface midqslvr_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

static void set_gate_a20(running_machine &machine, int a20)
{
	midqslvr_state *state = machine.driver_data<midqslvr_state>();

	state->m_maincpu->set_input_line(INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine &machine, int state)
{
	midqslvr_state *drvstate = machine.driver_data<midqslvr_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static int midqslvr_get_out2(running_machine &machine)
{
	midqslvr_state *state = machine.driver_data<midqslvr_state>();
	return pit8253_get_output(state->m_pit8254, 2 );
}

static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, NULL, midqslvr_get_out2
};

static void midqslvr_set_keyb_int(running_machine &machine, int state)
{
	midqslvr_state *drvstate = machine.driver_data<midqslvr_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static IRQ_CALLBACK(irq_callback)
{
	midqslvr_state *state = device->machine().driver_data<midqslvr_state>();
	return pic8259_acknowledge( state->m_pic8259_1);
}

static void ide_interrupt(device_t *device, int state)
{
	midqslvr_state *drvstate = device->machine().driver_data<midqslvr_state>();
	pic8259_ir6_w(drvstate->m_pic8259_2, state);
}

READ8_MEMBER( midqslvr_state::vga_setting ) { return 0xff; } // hard-code to color

void midqslvr_state::machine_start()
{

	m_bios_ram = auto_alloc_array(machine(), UINT32, 0x10000/4);
	m_bios_ext1_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_bios_ext2_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_bios_ext3_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_bios_ext4_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_isa_ram1 = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_isa_ram2 = auto_alloc_array(machine(), UINT32, 0x4000/4);

	init_pc_common(machine(), PCCOMMON_KEYBOARD_AT, midqslvr_set_keyb_int);

	m_maincpu->set_irq_acknowledge_callback(irq_callback);
	intel82439tx_init(machine());

	kbdc8042_init(machine(), &at8042);
	pc_vga_init(machine(), read8_delegate(FUNC(midqslvr_state::vga_setting),this), NULL);
	pc_vga_io_init(machine(), machine().device("maincpu")->memory().space(AS_PROGRAM), 0xa0000, machine().device("maincpu")->memory().space(AS_IO), 0x0000);
}

void midqslvr_state::machine_reset()
{
	machine().root_device().membank("bios_bank")->set_base(machine().root_device().memregion("bios")->base() + 0x70000);
	machine().root_device().membank("bios_ext1")->set_base(machine().root_device().memregion("bios")->base() + 0x60000);
	machine().root_device().membank("bios_ext2")->set_base(machine().root_device().memregion("bios")->base() + 0x64000);
	machine().root_device().membank("bios_ext3")->set_base(machine().root_device().memregion("bios")->base() + 0x68000);
	machine().root_device().membank("bios_ext4")->set_base(machine().root_device().memregion("bios")->base() + 0x6c000);
	machine().root_device().membank("video_bank1")->set_base(machine().root_device().memregion("video_bios")->base() + 0);
	machine().root_device().membank("video_bank2")->set_base(machine().root_device().memregion("video_bios")->base() + 0x4000);
}

static const ide_config ide_intf =
{
	ide_interrupt,
	NULL,
	0
};

static MACHINE_CONFIG_START( midqslvr, midqslvr_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 333000000)	// actually Celeron 333
	MCFG_CPU_PROGRAM_MAP(midqslvr_map)
	MCFG_CPU_IO_MAP(midqslvr_io)


	MCFG_PIT8254_ADD( "pit8254", midqslvr_pit8254_config )
	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )
	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )
	MCFG_PIC8259_ADD( "pic8259_1", midqslvr_pic8259_1_config )
	MCFG_PIC8259_ADD( "pic8259_2", midqslvr_pic8259_2_config )

	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE( 0, NULL, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(31, NULL, intel82371ab_pci_r, intel82371ab_pci_w)

	MCFG_IDE_CONTROLLER_ADD("ide", ide_intf, ide_devices, "hdd", NULL, true)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )
MACHINE_CONFIG_END


ROM_START( offrthnd )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "offrthnd", 0, SHA1(d88f1c5b75361a1e310565a8a5a09c674a4a1a22) )
ROM_END

ROM_START( hydrthnd )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "hydro", 0,  SHA1(d481d178782943c066b41764628a419cd55f676d) )
ROM_END

ROM_START( arctthnd )
	ROM_REGION32_LE(0x80000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x040000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "arctthnd", 0,  SHA1(f4373e57c3f453ac09c735b5d8d99ff811416a23) )
ROM_END

// this also required a dongle to work
ROM_START( arctthndult )
	ROM_REGION32_LE(0x80000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x040000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x8000, "video_bios", ROMREGION_ERASEFF ) // TODO: no VGA card is hooked up, to be removed
//  ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
//  ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "uarctict", 0, SHA1(8557a1d7ae8dc41c879350cb1c228f4c27a0dd09) )
ROM_END



// there are almost certainly multiple versions of these; updates were offered on floppy disk.  The version numbers for the existing CHDs are unknown.
GAME(1999, hydrthnd,    0,        midqslvr, at_keyboard, driver_device, 0, ROT0, "Midway Games", "Hydro Thunder", GAME_IS_SKELETON)
GAME(2000, offrthnd,    0,        midqslvr, at_keyboard, driver_device, 0, ROT0, "Midway Games", "Offroad Thunder", GAME_IS_SKELETON)
GAME(2001, arctthnd,    0,        midqslvr, at_keyboard, driver_device, 0, ROT0, "Midway Games", "Arctic Thunder (v1.002)", GAME_IS_SKELETON)
GAME(2001, arctthndult, arctthnd, midqslvr, at_keyboard, driver_device, 0, ROT0, "Midway Games", "Ultimate Arctic Thunder", GAME_IS_SKELETON)
