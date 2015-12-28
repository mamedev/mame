// license:BSD-3-Clause
// copyright-holders:R. Belmont, Peter Ferrie
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
#include "machine/ds128x.h"
#include "bus/isa/sblaster.h"

class savquest_state : public pcat_base_state
{
public:
	savquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag),
		m_vga(*this, "vga"),
		m_voodoo(*this, "voodoo")
	{
	}

	std::unique_ptr<UINT32[]> m_bios_f0000_ram;
	std::unique_ptr<UINT32[]> m_bios_e0000_ram;
	std::unique_ptr<UINT32[]> m_bios_e4000_ram;
	std::unique_ptr<UINT32[]> m_bios_e8000_ram;
	std::unique_ptr<UINT32[]> m_bios_ec000_ram;

	std::unique_ptr<UINT8[]> m_smram;

	required_device<s3_vga_device> m_vga;
	required_device<voodoo_2_device> m_voodoo;

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
	UINT8 m_hasp_tmppass[0x29];
	UINT8 m_port379;
	int m_hasp_passmode;
	int m_hasp_prodind;

	UINT8 m_mtxc_config_reg[256];
	UINT8 m_piix4_config_reg[8][256];
	UINT32 m_pci_3dfx_regs[0x40];

	DECLARE_WRITE32_MEMBER( bios_f0000_ram_w );
	DECLARE_WRITE32_MEMBER( bios_e0000_ram_w );
	DECLARE_WRITE32_MEMBER( bios_e4000_ram_w );
	DECLARE_WRITE32_MEMBER( bios_e8000_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ec000_ram_w );

	DECLARE_READ8_MEMBER(parallel_port_r);
	DECLARE_WRITE8_MEMBER(parallel_port_w);

	DECLARE_WRITE_LINE_MEMBER(vblank_assert);

	DECLARE_READ8_MEMBER(smram_r);
	DECLARE_WRITE8_MEMBER(smram_w);

protected:


	// driver_device overrides
//  virtual void video_start();
public:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void intel82439tx_init();
	void vid_3dfx_init();
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
				state->membank("bios_f0000")->set_base(state->m_bios_f0000_ram.get());
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
				state->membank("bios_e4000")->set_base(state->m_bios_e4000_ram.get());
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				state->membank("bios_e4000")->set_base(state->memregion("bios")->base() + 0x24000);
			}

			if (data & 1)       // enable RAM access to region 0xe0000 - 0xe3fff
			{
				state->membank("bios_e0000")->set_base(state->m_bios_e0000_ram.get());
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
				state->membank("bios_ec000")->set_base(state->m_bios_ec000_ram.get());
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				state->membank("bios_ec000")->set_base(state->memregion("bios")->base() + 0x2c000);
			}

			if (data & 1)       // enable RAM access to region 0xe8000 - 0xebfff
			{
				state->membank("bios_e8000")->set_base(state->m_bios_e8000_ram.get());
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
	m_smram = std::make_unique<UINT8[]>(0x20000);
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

void savquest_state::vid_3dfx_init()
{
	m_pci_3dfx_regs[0x00 / 4] = 0x0002121a; // 3dfx Multimedia device
	m_pci_3dfx_regs[0x08 / 4] = 2; // revision ID
	m_pci_3dfx_regs[0x10 / 4] = 0xff000000;
	m_pci_3dfx_regs[0x40 / 4] = 0x4000; //INITEN_SECONDARY_REV_ID
	voodoo_set_init_enable(m_voodoo, 0x4000); //INITEN_SECONDARY_REV_ID
}

static UINT32 pci_3dfx_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
//osd_printf_warning("PCI read: %x\n", reg);
	savquest_state *state = busdevice->machine().driver_data<savquest_state>();
	return state->m_pci_3dfx_regs[reg / 4];
}

static void pci_3dfx_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
osd_printf_warning("PCI write: %x %x\n", reg, data);

	savquest_state *state = busdevice->machine().driver_data<savquest_state>();

	if (reg == 0x10)
	{
		data &= 0xff000000;
	}
	else if (reg == 0x40)
	{
		voodoo_set_init_enable(state->m_voodoo, data);
	}
	else if (reg == 0x54)
	{
		data &= 0xf000ffff; /* bits 16-27 are read-only */
	}

	state->m_pci_3dfx_regs[reg / 4] = data;
}

WRITE32_MEMBER(savquest_state::bios_f0000_ram_w)
{
	//if (m_mtxc_config_reg[0x59] & 0x20)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mtxc_config_reg[0x59] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_f0000_ram.get() + offset);
	}
	#endif
}

WRITE32_MEMBER(savquest_state::bios_e0000_ram_w)
{
	//if (m_mtxc_config_reg[0x5e] & 2)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mtxc_config_reg[0x5e] & 2)        // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_e0000_ram.get() + offset);
	}
	#endif
}

WRITE32_MEMBER(savquest_state::bios_e4000_ram_w)
{
	//if (m_mtxc_config_reg[0x5e] & 0x20)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mtxc_config_reg[0x5e] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_e4000_ram.get() + offset);
	}
	#endif
}

WRITE32_MEMBER(savquest_state::bios_e8000_ram_w)
{
	//if (m_mtxc_config_reg[0x5f] & 2)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mtxc_config_reg[0x5f] & 2)        // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_e8000_ram.get() + offset);
	}
	#endif
}

WRITE32_MEMBER(savquest_state::bios_ec000_ram_w)
{
	//if (m_mtxc_config_reg[0x5f] & 0x20)       // write to RAM if this region is write-enabled
	#if 1
	if (m_mtxc_config_reg[0x5f] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ec000_ram.get() + offset);
	}
	#endif
}

static const UINT8 m_hasp_cmppass[] = {0xc3, 0xd9, 0xd3, 0xfb, 0x9d, 0x89, 0xb9, 0xa1, 0xb3, 0xc1, 0xf1, 0xcd, 0xdf, 0x9d}; /* 0x9d or 0x9e */
static const UINT8 m_hasp_prodinfo[] = {0x51, 0x4c, 0x52, 0x4d, 0x53, 0x4e, 0x53, 0x4e, 0x53, 0x49, 0x53, 0x48, 0x53, 0x4b, 0x53, 0x4a,
										0x53, 0x43, 0x53, 0x45, 0x52, 0x46, 0x53, 0x43, 0x53, 0x41, 0xac, 0x40, 0x53, 0xbc, 0x53, 0x42,
										0x53, 0x57, 0x53, 0x5d, 0x52, 0x5e, 0x53, 0x5b, 0x53, 0x59, 0xac, 0x58, 0x53, 0xa4
										};

READ8_MEMBER(savquest_state::parallel_port_r)
{
	if (offset == 1)
	{
		if ((m_haspstate == HASPSTATE_READ)
			&& (m_hasp_passmode == 3)
			)
		{
			/* passmode 3 is used to retrieve the product(s) information
			   it comes in two parts: header and product
			   the header has this format:
			   offset  range      purpose
			   00      01         header type
			   01      01-05      count of used product slots, must be 2
			   02      01-05      count of unused product slots
			                      this is assumed to be 6-(count of used slots)
			                      but it is not enforced here
			                      however a total of 6 structures will be checked
			   03      01-02      unknown
			   04      01-46      country code
			   05-0f   00         reserved
			   the used product slots have this format:
			   (the unused product slots must be entirely zeroes)
			   00-01   0001-000a  product ID, one must be 6, the other 0a
			   02      0001-0003  unknown but must be 0001
			   04      01-05      HASP plug country ID
			   05      01-02      unknown but must be 01
			   06      05         unknown
			   07-0a   any        unknown, not used
			   0b      ff         unknown
			   0c      ff         unknown
			   0d-0f   00         reserved

			   the read is performed by accessing an array of 16-bit big-endian values
			   and returning one bit at a time into bit 5 of the result
			   the 16-bit value is then XORed with 0x534d and the register index
			*/

			if (m_hasp_prodind <= (sizeof(m_hasp_prodinfo) * 8))
			{
				m_port379 = ((m_hasp_prodinfo[(m_hasp_prodind - 1) >> 3] >> ((8 - m_hasp_prodind) & 7)) & 1) << 5; /* return defined info */
			}
			else
			{
				m_port379 = (((0x534d ^ ((m_hasp_prodind - 1) >> 4)) >> ((16 - m_hasp_prodind) & 15)) & 1) << 5; /* then just alternate between the two key values */
			}

			++m_hasp_prodind;
		}

		return m_port379;
	}

	return 0;
}

WRITE8_MEMBER(savquest_state::parallel_port_w)
{
	if (!offset)
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
				m_haspind = 0;

				if (data8 == 0x80)
				{
					m_haspstate = HASPSTATE_PASSBEG;
					m_hasp_passind = 0;
					return;
				}

				break;
			}

			default:
			{
			}
		}

		m_port379 = 0x00;

		if (m_haspstate == HASPSTATE_READ)
		{
			/* different passwords cause different values to be returned
			   but there are really only two passwords of interest
			   passmode 2 is used to verify that the dongle is responding correctly
			*/

			if (m_hasp_passmode == 2)
			{
				/* in passmode 2, some values remain unknown: 96, 9a, c4, d4, ec, f8
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
		}
		else if (m_haspstate == HASPSTATE_PASSEND)
		{
			if (data8 & 1)
			{
				if ((m_hasp_passmode == 1)
					&& (data8 == 0x9d)
					)
				{
					m_hasp_passmode = 2;
				}

				m_haspstate = HASPSTATE_READ;
			}
			else if (m_hasp_passmode == 1)
			{
				m_hasp_tmppass[m_hasp_passind] = data8;

				if (++m_hasp_passind == sizeof(m_hasp_tmppass))
				{
					if ((m_hasp_tmppass[0] == 0x9c)
						&& (m_hasp_tmppass[1] == 0x9e)
						)
					{
						int i;

						i = 2;
						m_hasp_prodind = 0;

						do
						{
							m_hasp_prodind = (m_hasp_prodind << 1) + ((m_hasp_tmppass[i] >> 6) & 1);
						}
						while ((i += 3) < sizeof(m_hasp_tmppass));

						m_hasp_prodind = (m_hasp_prodind - 0xc08) << 4;

						if (m_hasp_prodind < (0x38 << 4))
						{
							m_hasp_passmode = 3;
						}
					}

					m_haspstate = HASPSTATE_READ;
				}
			}
		}
		else if ((m_haspstate == HASPSTATE_PASSBEG)
				&& (data8 & 1)
			)
		{
			m_hasp_tmppass[m_hasp_passind] = data8;

			if (++m_hasp_passind == sizeof(m_hasp_cmppass))
			{
				m_haspstate = HASPSTATE_PASSEND;
				m_hasp_passind = 0;
				m_hasp_passmode = (int) !memcmp(m_hasp_tmppass, m_hasp_cmppass, sizeof(m_hasp_cmppass));
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
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_READWRITE8(smram_r,smram_w,0xffffffff) //AM_DEVREADWRITE8("vga", vga_device, mem_r, mem_w, 0xffffffff)
	AM_RANGE(0x000c0000, 0x000c7fff) AM_ROM AM_REGION("video_bios", 0)
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bios_f0000") AM_WRITE(bios_f0000_ram_w)
	AM_RANGE(0x000e0000, 0x000e3fff) AM_ROMBANK("bios_e0000") AM_WRITE(bios_e0000_ram_w)
	AM_RANGE(0x000e4000, 0x000e7fff) AM_ROMBANK("bios_e4000") AM_WRITE(bios_e4000_ram_w)
	AM_RANGE(0x000e8000, 0x000ebfff) AM_ROMBANK("bios_e8000") AM_WRITE(bios_e8000_ram_w)
	AM_RANGE(0x000ec000, 0x000effff) AM_ROMBANK("bios_ec000") AM_WRITE(bios_ec000_ram_w)
	AM_RANGE(0x00100000, 0x07ffffff) AM_RAM // 128MB RAM
	AM_RANGE(0xe0000000, 0xe0fbffff) AM_DEVREADWRITE("voodoo", voodoo_device, voodoo_r, voodoo_w)
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)    /* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(savquest_io, AS_IO, 32, savquest_state)
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", ds12885_device, read, write, 0xffffffff)

	AM_RANGE(0x00e8, 0x00ef) AM_NOP

	AM_RANGE(0x0170, 0x0177) AM_DEVREADWRITE("ide2", ide_controller_32_device, read_cs0, write_cs0)
	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE("ide", ide_controller_32_device, read_cs0, write_cs0)
	AM_RANGE(0x0378, 0x037b) AM_READWRITE8(parallel_port_r, parallel_port_w, 0xffffffff)
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
	m_bios_f0000_ram = std::make_unique<UINT32[]>(0x10000/4);
	m_bios_e0000_ram = std::make_unique<UINT32[]>(0x4000/4);
	m_bios_e4000_ram = std::make_unique<UINT32[]>(0x4000/4);
	m_bios_e8000_ram = std::make_unique<UINT32[]>(0x4000/4);
	m_bios_ec000_ram = std::make_unique<UINT32[]>(0x4000/4);

	intel82439tx_init();
	vid_3dfx_init();
}

void savquest_state::machine_reset()
{
	membank("bios_f0000")->set_base(memregion("bios")->base() + 0x30000);
	membank("bios_e0000")->set_base(memregion("bios")->base() + 0x20000);
	membank("bios_e4000")->set_base(memregion("bios")->base() + 0x24000);
	membank("bios_e8000")->set_base(memregion("bios")->base() + 0x28000);
	membank("bios_ec000")->set_base(memregion("bios")->base() + 0x2c000);
	m_haspstate = HASPSTATE_NONE;
}

WRITE_LINE_MEMBER(savquest_state::vblank_assert)
{
}

SLOT_INTERFACE_START( savquest_isa16_cards )
	SLOT_INTERFACE("sb16", ISA16_SOUND_BLASTER_16)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( savquest, savquest_state )
	MCFG_CPU_ADD("maincpu", PENTIUM2, 450000000) // actually Pentium II 450
	MCFG_CPU_PROGRAM_MAP(savquest_map)
	MCFG_CPU_IO_MAP(savquest_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	MCFG_FRAGMENT_ADD( pcat_common )
	MCFG_DEVICE_REMOVE("rtc")
	MCFG_DS12885_ADD("rtc")

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, nullptr, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(7, nullptr, intel82371ab_pci_r, intel82371ab_pci_w)
	MCFG_PCI_BUS_LEGACY_DEVICE(13, nullptr, pci_3dfx_r, pci_3dfx_w)

	MCFG_IDE_CONTROLLER_32_ADD("ide", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir6_w))

	MCFG_IDE_CONTROLLER_32_ADD("ide2", ata_devices, nullptr, nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir7_w))

	/* sound hardware */

	MCFG_DEVICE_ADD("isa", ISA16, 0)
	MCFG_ISA16_CPU(":maincpu")
	MCFG_ISA16_SLOT_ADD("isa", "isa1", savquest_isa16_cards, "sb16", false)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_s3_vga )

	MCFG_DEVICE_ADD("voodoo", VOODOO_2, STD_VOODOO_2_CLOCK)
	MCFG_VOODOO_FBMEM(4)
	MCFG_VOODOO_TMUMEM(4,4) /* this is the 12Mb card */
	MCFG_VOODOO_SCREEN_TAG("screen")
	MCFG_VOODOO_CPU_TAG("maincpu")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(savquest_state,vblank_assert))
MACHINE_CONFIG_END

ROM_START( savquest )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD( "p2xbl_award_451pg.bin", 0x00000, 0x040000, CRC(37d0030e) SHA1(c6773d0e02325116f95c497b9953f59a9ac81317) )

	ROM_REGION( 0x10000, "video_bios", 0 ) // 1st half is 2.04.14, second half is 2.01.11
	ROM_LOAD( "vgabios.bin",   0x000000, 0x010000, CRC(a81423d6) SHA1(a099af621ce7fbaa55a2d9947d9f07e04f1b5fca) )

	ROM_REGION( 0x080, "rtc", 0 )    /* default NVRAM */
	ROM_LOAD( "savquest_ds12885.bin", 0x0000, 0x080, BAD_DUMP CRC(e9270019) SHA1(4d900ca317d93c915c80a9053528b741746f08a1) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "savquest", 0, SHA1(b7c8901172b66706a7ab5f5c91e6912855153fa9) )
ROM_END


GAME(1999, savquest, 0, savquest, savquest, driver_device, 0, ROT0, "Interactive Light", "Savage Quest", MACHINE_IS_SKELETON)
