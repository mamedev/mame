/***************************************************************************

    savquest.c

    "Savage Quest" (c) 1999 Interactive Light, developed by Angel Studios.
    Skeleton by R. Belmont

	TODO:
	- currently asserts due of:
	  000F6E09: fld     qword ptr [esi]

    H/W is a white-box PC consisting of:
    Pentium II 450 CPU
    DFI P2XBL motherboard (i440BX chipset)
    128 MB RAM
    Guillemot Maxi Gamer 3D2 Voodoo II
    Sound Blaster AWE64

    Protected by a HASP brand parallel port dongle.
    I/O board has a PIC17C43 which is not readable.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/i386/i386.h"
#include "memconv.h"
#include "devconv.h"
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


class savquest_state : public driver_device
{
public:
	savquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_pit8254(*this, "pit8254"),
		  m_pic8259_1(*this, "pic8259_1"),
		  m_pic8259_2(*this, "pic8259_2")
	{ }

	UINT32 *m_bios_ram;
	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
	UINT8 m_mxtc_config_reg[256];
	UINT8 m_piix4_config_reg[4][256];

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<pit8254_device> m_pit8254;
	required_device<pic8259_device> m_pic8259_1;
	required_device<pic8259_device> m_pic8259_2;

	DECLARE_READ8_MEMBER( get_slave_ack );
	DECLARE_WRITE32_MEMBER( bios_ram_w );

protected:


	// driver_device overrides
//	virtual void video_start();
//	virtual bool screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);

//	device_t	*m_dma8237_1;
//	device_t	*m_dma8237_2;
};

// Intel 82439TX System Controller (MXTC)

static UINT8 mxtc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	savquest_state *state = busdevice->machine().driver_data<savquest_state>();
//  mame_printf_debug("MXTC: read %d, %02X\n", function, reg);

	return state->m_mxtc_config_reg[reg];
}

static void mxtc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	savquest_state *state = busdevice->machine().driver_data<savquest_state>();
//  mame_printf_debug("%s:MXTC: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);

	#if 1
	switch(reg)
	{
		case 0x59:		// PAM0
		{
			if (data & 0x10)		// enable RAM access to region 0xf0000 - 0xfffff
			{
				memory_set_bankptr(busdevice->machine(), "bank1", state->m_bios_ram);
			}
			else					// disable RAM access (reads go to BIOS ROM)
			{
				memory_set_bankptr(busdevice->machine(), "bank1", busdevice->machine().region("bios")->base() + 0);
			}
			break;
		}
	}
	#endif

	state->m_mxtc_config_reg[reg] = data;
}

static void intel82439tx_init(running_machine &machine)
{
	savquest_state *state = machine.driver_data<savquest_state>();
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
	savquest_state *state = busdevice->machine().driver_data<savquest_state>();
//  mame_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	savquest_state *state = busdevice->machine().driver_data<savquest_state>();
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

WRITE32_MEMBER(savquest_state::bios_ram_w)
{
	//if (m_mxtc_config_reg[0x59] & 0x20)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mxtc_config_reg[0x59] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram + offset);
	}
	#endif
}

static ADDRESS_MAP_START(savquest_map, AS_PROGRAM, 32, savquest_state)
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM
	AM_RANGE(0x000c0000, 0x000c7fff) AM_ROM AM_REGION("video_bios", 0)
	AM_RANGE(0x000e0000, 0x000fffff) AM_ROMBANK("bank1")
	AM_RANGE(0x000e0000, 0x000fffff) AM_WRITE(bios_ram_w)
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(savquest_io, AS_IO, 32, savquest_state)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE_LEGACY(kbdc8042_32le_r,			kbdc8042_32le_w)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00e8, 0x00ef) AM_NOP //AMI BIOS write to this ports as delays between I/O ports operations sending al value -> NEWIODELAY
	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE_LEGACY("pcibus", pci_32le_r,	pci_32le_w)

ADDRESS_MAP_END

static INPUT_PORTS_START( savquest )
INPUT_PORTS_END

static const struct pit8253_config savquest_pit8254_config =
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

static WRITE_LINE_DEVICE_HANDLER( savquest_pic8259_1_set_int_line )
{
	savquest_state *drvstate = device->machine().driver_data<savquest_state>();
	device_set_input_line(drvstate->m_maincpu, 0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER( savquest_state::get_slave_ack )
{
	if (offset==2) { // IRQ = 2
		logerror("pic8259_slave_ACK!\n");
		return pic8259_acknowledge(m_pic8259_2);
	}
	return 0x00;
}

static const struct pic8259_interface savquest_pic8259_1_config =
{
	DEVCB_LINE(savquest_pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_MEMBER(savquest_state,get_slave_ack)
};

static const struct pic8259_interface savquest_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

static void set_gate_a20(running_machine &machine, int a20)
{
	savquest_state *state = machine.driver_data<savquest_state>();

	device_set_input_line(state->m_maincpu, INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine &machine, int state)
{
	savquest_state *drvstate = machine.driver_data<savquest_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static int savquest_get_out2(running_machine &machine)
{
	savquest_state *state = machine.driver_data<savquest_state>();
	return pit8253_get_output(state->m_pit8254, 2 );
}

static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, NULL, savquest_get_out2
};

static void savquest_set_keyb_int(running_machine &machine, int state)
{
	savquest_state *drvstate = machine.driver_data<savquest_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static IRQ_CALLBACK(irq_callback)
{
	savquest_state *state = device->machine().driver_data<savquest_state>();
	return pic8259_acknowledge( state->m_pic8259_1);
}

static READ8_HANDLER( vga_setting ) { return 0xff; } // hard-code to color

static const struct pc_vga_interface vga_interface =
{
	NULL,
	NULL,
	vga_setting,
	AS_PROGRAM,
	0xa0000,
	AS_IO,
	0x0000
};

static MACHINE_START( savquest )
{
	savquest_state *state = machine.driver_data<savquest_state>();
	state->m_bios_ram = auto_alloc_array(machine, UINT32, 0x20000/4);

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, savquest_set_keyb_int);

	device_set_irq_callback(state->m_maincpu, irq_callback);
	intel82439tx_init(machine);

	kbdc8042_init(machine, &at8042);
	pc_vga_init(machine, &vga_interface, NULL);
}

static MACHINE_RESET( savquest )
{
	memory_set_bankptr(machine, "bank1", machine.region("bios")->base());
}


static MACHINE_CONFIG_START( savquest, savquest_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 450000000)	// actually Pentium II 450
	MCFG_CPU_PROGRAM_MAP(savquest_map)
	MCFG_CPU_IO_MAP(savquest_io)

	MCFG_MACHINE_START(savquest)
	MCFG_MACHINE_RESET(savquest)

	MCFG_PIT8254_ADD( "pit8254", savquest_pit8254_config )
	MCFG_PIC8259_ADD( "pic8259_1", savquest_pic8259_1_config )
	MCFG_PIC8259_ADD( "pic8259_2", savquest_pic8259_2_config )
	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE(0, NULL, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_DEVICE(7, NULL, intel82371ab_pci_r, intel82371ab_pci_w)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )
MACHINE_CONFIG_END

ROM_START( savquest )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "sq-aflash.bin", 0x00000, 0x020000, CRC(0b4f406f) SHA1(4003b0e6d46dcb47012acc118837f0f7cf529faf) ) // first half is 1-filled
	ROM_CONTINUE(              0x00000, 0x020000 )

	ROM_REGION( 0x8000, "video_bios", 0 ) // TODO: needs proper video BIOS dumped
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	DISK_REGION( "disk" )
	DISK_IMAGE( "savquest", 0, SHA1(b20cacf45e093b533c538bf4fc08f05f9475d640) )
ROM_END


GAME(1999, savquest, 0, savquest, savquest, 0, ROT0, "Interactive Light", "Savage Quest", GAME_IS_SKELETON)
