/***************************************************************************

    savquest.c

    "Savage Quest" (c) 1999 Interactive Light, developed by Angel Studios.
    Skeleton by R. Belmont

    TODO:
    - currently asserts by selecting a s3 video bank above 1M (register 0x6a)

    PCI list:
    Bus no. Device No. Func No. Vendor ID Device ID Device Class          IRQ
    0       7          1        8086      7111      IDE Controller        14
    0       7          2        8086      7112      Serial Bus Controller 11
    0       9          0        5333      8901      Display Controller    10
    0       13         0        121a      0002      Multimedia Device     NA
    - First two are PIIX4/4E/4M IDE Controller / PIIX4/4E/4M USB Interface
      Third is S3 trio64uv+
      Fourth is Voodoo 2 3D Accelerator
    Sound Blaster is ISA/PNP

============================================================================
    H/W is a white-box PC consisting of:
    Pentium II 450 CPU
    DFI P2XBL motherboard (i440BX chipset)
    128 MB RAM
    Guillemot Maxi Gamer 3D2 Voodoo II
    Sound Blaster AWE64

    Protected by a HASP brand parallel port dongle.
    I/O board has a PIC17C43 which is not readable.

    On boot it reports: S3 86C775/86C705 Video BIOS. Version 2.04.11 Copyright 1996 S3 Incorporated.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

- update by Peter Ferrie:
- split BIOS region into 16kb blocks and implement missing PAM registers

- HASP emulator by Peter Ferrie


***************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "video/pc_vga.h"
#include "video/voodoo.h"


class savquest_state : public pcat_base_state
{
public:
	savquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag),
		m_vga(*this, "vga")
	{
	}

	UINT32 *m_bios_f0000_ram;
	UINT32 *m_bios_e0000_ram;
	UINT32 *m_bios_e4000_ram;
	UINT32 *m_bios_e8000_ram;
	UINT32 *m_bios_ec000_ram;

	UINT8 *m_smram;

	required_device<s3_vga_device> m_vga;

	int m_haspind;
	int m_haspstate;
	enum hasp_states
	{
		HASPSTATE_NONE,
		HASPSTATE_PASSBEG,
		HASPSTATE_PASSEND,
		HASPSTATE_READ
	};
	int m_hasp_passind;
	UINT8 m_hasp_tmppass[15];
	UINT8 m_port379;
	int m_hasp_passmode;

	UINT8 m_mtxc_config_reg[256];
	UINT8 m_piix4_config_reg[8][256];

	DECLARE_WRITE32_MEMBER( bios_f0000_ram_w );
	DECLARE_WRITE32_MEMBER( bios_e0000_ram_w );
	DECLARE_WRITE32_MEMBER( bios_e4000_ram_w );
	DECLARE_WRITE32_MEMBER( bios_e8000_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ec000_ram_w );

	DECLARE_READ32_MEMBER(parallel_port_r);
	DECLARE_WRITE32_MEMBER(parallel_port_w);

	DECLARE_WRITE_LINE_MEMBER(vblank_assert);

	DECLARE_READ8_MEMBER(smram_r);
	DECLARE_WRITE8_MEMBER(smram_w);

protected:


	// driver_device overrides
//  virtual void video_start();
public:
	virtual void machine_start();
	virtual void machine_reset();
	void intel82439tx_init();
};

// Intel 82439TX System Controller (MTXC)

static UINT8 mtxc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	savquest_state *state = busdevice->machine().driver_data<savquest_state>();
//  osd_printf_debug("MTXC: read %d, %02X\n", function, reg);

	if((reg & 0xfe) == 0)
		return (reg & 1) ? 0x80 : 0x86; // Vendor ID, Intel

	if((reg & 0xfe) == 2)
		return (reg & 1) ? 0x70 : 0x00; // Device ID, MTXC

	return state->m_mtxc_config_reg[reg];
}

static void mtxc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	savquest_state *state = busdevice->machine().driver_data<savquest_state>();
//  osd_printf_debug("%s:MXTC: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);

	#if 1
	switch(reg)
	{
		case 0x59:      // PAM0
		{
			if (data & 0x10)        // enable RAM access to region 0xf0000 - 0xfffff
			{
				state->membank("bios_f0000")->set_base(state->m_bios_f0000_ram);
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				state->membank("bios_f0000")->set_base(state->memregion("bios")->base() + 0x30000);
			}
			break;
		}

		case 0x5e:      // PAM5
		{
			if (data & 0x10)        // enable RAM access to region 0xe4000 - 0xe7fff
			{
				state->membank("bios_e4000")->set_base(state->m_bios_e4000_ram);
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				state->membank("bios_e4000")->set_base(state->memregion("bios")->base() + 0x24000);
			}

			if (data & 1)       // enable RAM access to region 0xe0000 - 0xe3fff
			{
				state->membank("bios_e0000")->set_base(state->m_bios_e0000_ram);
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				state->membank("bios_e0000")->set_base(state->memregion("bios")->base() + 0x20000);
			}
			break;
		}

		case 0x5f:      // PAM6
		{
			if (data & 0x10)        // enable RAM access to region 0xec000 - 0xeffff
			{
				state->membank("bios_ec000")->set_base(state->m_bios_ec000_ram);
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				state->membank("bios_ec000")->set_base(state->memregion("bios")->base() + 0x2c000);
			}

			if (data & 1)       // enable RAM access to region 0xe8000 - 0xebfff
			{
				state->membank("bios_e8000")->set_base(state->m_bios_e8000_ram);
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				state->membank("bios_e8000")->set_base(state->memregion("bios")->base() + 0x28000);
			}
			break;
		}
	}
	#endif

	state->m_mtxc_config_reg[reg] = data;
}

void savquest_state::intel82439tx_init()
{
	m_mtxc_config_reg[0x60] = 0x02;
	m_mtxc_config_reg[0x61] = 0x02;
	m_mtxc_config_reg[0x62] = 0x02;
	m_mtxc_config_reg[0x63] = 0x02;
	m_mtxc_config_reg[0x64] = 0x02;
	m_mtxc_config_reg[0x65] = 0x02;
	m_smram = auto_alloc_array(machine(), UINT8, 0x20000);
}

static UINT32 intel82439tx_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= mtxc_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82439tx_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		mtxc_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		mtxc_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		mtxc_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		mtxc_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}

// Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)

static UINT8 piix4_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	savquest_state *state = busdevice->machine().driver_data<savquest_state>();
//  osd_printf_debug("PIIX4: read %d, %02X\n", function, reg);

	if((reg & 0xfe) == 0)
		return (reg & 1) ? 0x80 : 0x86; // Vendor ID, Intel

	if((reg & 0xfe) == 2)
	{
		/* TODO: it isn't detected properly (i.e. PCI writes always goes to function == 0) */
		if(function == 1)
			return (reg & 1) ? 0x71 : 0x11; // Device ID, 82371AB IDE Controller
		if(function == 2)
			return (reg & 1) ? 0x71 : 0x12; // Device ID, 82371AB Serial Bus Controller
	}

	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	savquest_state *state = busdevice->machine().driver_data<savquest_state>();
//  osd_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);
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

WRITE32_MEMBER(savquest_state::bios_f0000_ram_w)
{
	//if (m_mtxc_config_reg[0x59] & 0x20)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mtxc_config_reg[0x59] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_f0000_ram + offset);
	}
	#endif
}

WRITE32_MEMBER(savquest_state::bios_e0000_ram_w)
{
	//if (m_mtxc_config_reg[0x5e] & 2)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mtxc_config_reg[0x5e] & 2)        // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_e0000_ram + offset);
	}
	#endif
}

WRITE32_MEMBER(savquest_state::bios_e4000_ram_w)
{
	//if (m_mtxc_config_reg[0x5e] & 0x20)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mtxc_config_reg[0x5e] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_e4000_ram + offset);
	}
	#endif
}

WRITE32_MEMBER(savquest_state::bios_e8000_ram_w)
{
	//if (m_mtxc_config_reg[0x5f] & 2)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mtxc_config_reg[0x5f] & 2)        // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_e8000_ram + offset);
	}
	#endif
}

WRITE32_MEMBER(savquest_state::bios_ec000_ram_w)
{
	//if (m_mtxc_config_reg[0x5f] & 0x20)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mtxc_config_reg[0x5f] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ec000_ram + offset);
	}
	#endif
}

static const UINT8 m_hasp_cmppass[] = {0xc3, 0xd9, 0xd3, 0xfb, 0x9d, 0x89, 0xb9, 0xa1, 0xb3, 0xc1, 0xf1, 0xcd, 0xdf, 0x9d, 0x9d};

READ32_MEMBER(savquest_state::parallel_port_r)
{
	if (ACCESSING_BITS_8_15)
	{
		return ((UINT32) m_port379 << 8);
	}

	return 0;
}

WRITE32_MEMBER(savquest_state::parallel_port_w)
{
	if (ACCESSING_BITS_0_7)
	{
		UINT8 data8 = (UINT8) (data & 0xff);

		/* state machine to determine when password is about to be entered */

		switch (m_haspind)
		{
			case 0:
			{
				if (data8 == 0xc6)
				{
					++m_haspind;
					break;
				}

				m_haspind = 0;
				break;
			}

			case 1:
			{
				if (data8 == 0xc7)
				{
					++m_haspind;
					break;
				}

				m_haspind = 0;
				break;
			}

			case 2:
			{
				if (data8 == 0xc6)
				{
					++m_haspind;
					break;
				}

				m_haspind = 0;
				m_haspstate = HASPSTATE_NONE;
				break;
			}

			case 3:
			{
				if (data8 == 0x80)
				{
					m_haspstate = HASPSTATE_PASSBEG;
					m_hasp_passind = 0;
				}

				m_haspind = 0;
				break;
			}

			default:
			{
			}
		}

		m_port379 = 0x00;

		if (m_haspstate == HASPSTATE_READ)
		{
			/* different passwords causes different values to be returned
			   but there is really only one password of interest
			*/

			if (m_hasp_passmode == 1)
			{
				/* in passmode 1, some values remain unknown: 96, 9a, c4, d4, ec, f8
				   they all return 00, but if that's wrong then there will be failures to start
				*/

				if ((data8 == 0x94)
					|| (data8 == 0x9e)
					|| (data8 == 0xa4)
					|| (data8 == 0xb2)
					|| (data8 == 0xbe)
					|| (data8 == 0xd0)
					)
				{
					return;
				}

				if ((data8 == 0x8a)
					|| (data8 == 0x8e)
					|| (data8 == 0xca)
					|| (data8 == 0xd2)
					|| (data8 == 0xe2)
					|| (data8 == 0xf0)
					|| (data8 == 0xfc)
					)
				{
					/* someone with access to the actual dongle could dump the true values
					   I've never seen it so I just determined the relevant bits instead
					   from the disassembly of the software
					   some of the keys are verified explicitly, the others implicitly
					   I guessed the implicit ones with a bit of trial and error
					*/

					m_port379 = 0x20;
					return;
				}
			}

			switch (data8)
			{
				/* in passmode 0, some values remain unknown: 8a, 8e (inconclusive), 94, 96, 9a, a4, b2, be, c4, d2, d4 (inconclusive), e2, ec, f8, fc
				   this is less of a concern since the contents seem to decrypt correctly
				*/

				case 0x88:
				case 0x94:
				case 0x98:
				case 0x9c:
				case 0x9e:
				case 0xa0:
				case 0xa4:
				case 0xaa:
				case 0xae:
				case 0xb0:
				case 0xb2:
				case 0xbc:
				case 0xbe:
				case 0xc2:
				case 0xc6:
				case 0xc8:
				case 0xce:
				case 0xd0:
				case 0xd6:
				case 0xd8:
				case 0xdc:
				case 0xe0:
				case 0xe6:
				case 0xea:
				case 0xee:
				case 0xf2:
				case 0xf6:
				{
					/* again, just the relevant bits instead of the true values */

					m_port379 = 0x20;
					break;
				}

				default:
				{
				}
			}

			return;
		}

		if (m_haspstate == HASPSTATE_PASSEND)
		{
			m_haspstate = HASPSTATE_READ;
			return;
		}

		if ((m_haspstate == HASPSTATE_PASSBEG)
			&& (data8 & 1)
			)
		{
			m_hasp_tmppass[m_hasp_passind] = data8;

			if (++m_hasp_passind == 15)
			{
				m_haspstate = HASPSTATE_PASSEND;
				m_hasp_passmode = 0;

				if (!memcmp(m_hasp_tmppass, m_hasp_cmppass, sizeof(m_hasp_tmppass)))
				{
					m_hasp_passmode = 1;
				}
			}
		}
	}
}

READ8_MEMBER(savquest_state::smram_r)
{
	/* TODO: way more complex than this */
	if(m_mtxc_config_reg[0x72] & 0x40)
		return m_smram[offset];
	else
		return m_vga->mem_r(space,offset,0xff);
}

WRITE8_MEMBER(savquest_state::smram_w)
{
	/* TODO: way more complex than this */
	if(m_mtxc_config_reg[0x72] & 0x40)
		m_smram[offset] = data;
	else
		m_vga->mem_w(space,offset,data,0xff);

}

static ADDRESS_MAP_START(savquest_map, AS_PROGRAM, 32, savquest_state)
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_READWRITE8(smram_r,smram_w,0xffffffff) //AM_DEVREADWRITE8("vga", vga_device, mem_r, mem_w, 0xffffffff)
	AM_RANGE(0x000c0000, 0x000c7fff) AM_ROM AM_REGION("video_bios", 0)
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bios_f0000") AM_WRITE(bios_f0000_ram_w)
	AM_RANGE(0x000e0000, 0x000e3fff) AM_ROMBANK("bios_e0000") AM_WRITE(bios_e0000_ram_w)
	AM_RANGE(0x000e4000, 0x000e7fff) AM_ROMBANK("bios_e4000") AM_WRITE(bios_e4000_ram_w)
	AM_RANGE(0x000e8000, 0x000ebfff) AM_ROMBANK("bios_e8000") AM_WRITE(bios_e8000_ram_w)
	AM_RANGE(0x000ec000, 0x000effff) AM_ROMBANK("bios_ec000") AM_WRITE(bios_ec000_ram_w)
	AM_RANGE(0x00100000, 0x07ffffff) AM_RAM // 128MB RAM
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)    /* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(savquest_io, AS_IO, 32, savquest_state)
	AM_IMPORT_FROM(pcat32_io_common)

	AM_RANGE(0x00e8, 0x00ef) AM_NOP

	AM_RANGE(0x0170, 0x0177) AM_DEVREADWRITE("ide2", ide_controller_32_device, read_cs0, write_cs0)
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE("ide", ide_controller_32_device, read_cs0, write_cs0)
	AM_RANGE(0x0378, 0x037b) AM_READWRITE(parallel_port_r, parallel_port_w)
	AM_RANGE(0x03b0, 0x03bf) AM_DEVREADWRITE8("vga", vga_device, port_03b0_r, port_03b0_w, 0xffffffff)
	AM_RANGE(0x03c0, 0x03cf) AM_DEVREADWRITE8("vga", vga_device, port_03c0_r, port_03c0_w, 0xffffffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("vga", vga_device, port_03d0_r, port_03d0_w, 0xffffffff)
	AM_RANGE(0x0370, 0x0377) AM_DEVREADWRITE("ide2", ide_controller_32_device, read_cs1, write_cs1)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVREADWRITE("ide", ide_controller_32_device, read_cs1, write_cs1)

	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)

//  AM_RANGE(0x5000, 0x5007) // routes to port $eb
ADDRESS_MAP_END

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START( savquest )
	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_S           ) /* F1                          3B  BB */
INPUT_PORTS_END

void savquest_state::machine_start()
{
	m_bios_f0000_ram = auto_alloc_array(machine(), UINT32, 0x10000/4);
	m_bios_e0000_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_bios_e4000_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_bios_e8000_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);
	m_bios_ec000_ram = auto_alloc_array(machine(), UINT32, 0x4000/4);

	intel82439tx_init();
}

void savquest_state::machine_reset()
{
	membank("bios_f0000")->set_base(memregion("bios")->base() + 0x30000);
	membank("bios_e0000")->set_base(memregion("bios")->base() + 0x20000);
	membank("bios_e4000")->set_base(memregion("bios")->base() + 0x24000);
	membank("bios_e8000")->set_base(memregion("bios")->base() + 0x28000);
	membank("bios_ec000")->set_base(memregion("bios")->base() + 0x2c000);
}

WRITE_LINE_MEMBER(savquest_state::vblank_assert)
{
}

static MACHINE_CONFIG_START( savquest, savquest_state )
	MCFG_CPU_ADD("maincpu", PENTIUM2, 450000000) // actually Pentium II 450
	MCFG_CPU_PROGRAM_MAP(savquest_map)
	MCFG_CPU_IO_MAP(savquest_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	MCFG_FRAGMENT_ADD( pcat_common )

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, NULL, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(7, NULL, intel82371ab_pci_r, intel82371ab_pci_w)

	MCFG_IDE_CONTROLLER_32_ADD("ide", ata_devices, "hdd", NULL, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir6_w))

	MCFG_IDE_CONTROLLER_32_ADD("ide2", ata_devices, NULL, NULL, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir7_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_s3_vga )

	MCFG_DEVICE_ADD("voodoo", VOODOO_2, STD_VOODOO_2_CLOCK)
	MCFG_VOODOO_FBMEM(2)
	MCFG_VOODOO_TMUMEM(4,4)
	MCFG_VOODOO_SCREEN_TAG("screen")
	MCFG_VOODOO_CPU_TAG("maincpu")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(savquest_state,vblank_assert))
MACHINE_CONFIG_END

ROM_START( savquest )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD( "p2xbl_award_451pg.bin", 0x00000, 0x040000, CRC(37d0030e) SHA1(c6773d0e02325116f95c497b9953f59a9ac81317) )

	ROM_REGION( 0x10000, "video_bios", 0 ) // 1st half is 2.04.14, second half is 2.01.11
	ROM_LOAD( "vgabios.bin",   0x000000, 0x010000, CRC(a81423d6) SHA1(a099af621ce7fbaa55a2d9947d9f07e04f1b5fca) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "savquest", 0, SHA1(b7c8901172b66706a7ab5f5c91e6912855153fa9) )
ROM_END


GAME(1999, savquest, 0, savquest, savquest, driver_device, 0, ROT0, "Interactive Light", "Savage Quest", GAME_IS_SKELETON)
