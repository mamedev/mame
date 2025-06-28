// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Megadrive / Genesis 32X emulation */

/*

    Fix 32X support (not used by any arcade systems?)
     - this seems to require far greater sync and timing accuracy on rom / ram access than MAME can provide
     - split NTSC / PAL drivers
     - 36greatju: missing backup ram, has issues with golfer select due of that
     - bcracers: write to undefined PWM register?
     - fifa96 / nbajamte: dies on the gameplay, waiting for a comm change that never occurs;
     - marsch1: doesn't boot, Master / Slave communicates through SCI
     - tempo: intro is too fast, mostly noticeable with the PWM sound that cuts off too early when it gets to the title screen;
     - tmek: gameplay is clearly too fast


32x Marsch tests documentation (keep start pressed at start-up for individual tests):

MD side check:
#1 Communication Check
#2 FM Bit
#3 Irq Register
#4 Bank Control Register
#5 DREQ Control FULL bit
#6 DREQ SRC Address
#7 DREQ DST Address
#8 DREQ SIZE Address
#9 SEGA TV Register
#10 H IRQ Vector
#11 PWM Control Register
#12 PWM Frequency Register
#13 PWM Lch Pulse Width Register
#14 PWM Rch Pulse Width Register
#15 PWM MONO Pulse Width Register
32x side check:
#16 SH-2 Master Communication Check
#17 SH-2 Slave Communication Check
#18 SH-2 Master FM Bit
#19 SH-2 Slave FM Bit
#20 SH-2 Master IRQ Mask Register
#21 SH-2 Slave IRQ Mask Register
#22 SH-2 Master H Counter Register
#23 SH-2 Slave H Counter Register
#24 SH-2 Master PWM Timer Register
#25 SH-2 Slave PWM Timer Register
#26 SH-2 Master PWM Cont. Register
#27 SH-2 Slave PWM Cont. Register
#28 SH-2 Master PWM Freq. Register
#29 SH-2 Slave PWM Freq. Register
#30 SH-2 Master PWM Lch Register
#31 SH-2 Slave PWM Lch Register
#32 SH-2 Master PWM Rch Register
#33 SH-2 Slave PWM Rch Register
#34 SH-2 Master PWM Mono Register
#35 SH-2 Slave PWM Mono Register
#36 SH-2 Master ROM Read Check
#37 SH-2 Slave ROM Read Check
#38 SH-2 Serial Communication (ERROR - returns a Timeout Error)
MD & 32x check:
#39 MD&SH-2 Master Communication
#40 MD&SH-2 Slave Communication
#41 MD&SH-2 Master FM Bit R/W
#42 MD&SH-2 Slave FM Bit R/W
#43 MD&SH-2 Master DREQ CTL
#44 MD&SH-2 Slave DREQ CTL
#45 MD&SH-2 Master DREQ SRC address
#46 MD&SH-2 Slave DREQ SRC address
#47 MD&SH-2 Master DREQ DST address
#48 MD&SH-2 Slave DREQ DST address
#49 MD&SH-2 Master DREQ SIZE address
#50 MD&SH-2 Slave DREQ SIZE address
#51 SH-2 Master V IRQ
#52 SH-2 Slave V IRQ
#53 SH2 Master H IRQ (MD 0)
#54 SH2 Slave H IRQ (MD 0)
#55 SH2 Master H IRQ (MD 1)
#56 SH2 Slave H IRQ (MD 1)
#57 SH2 Master H IRQ (MD 2)
#58 SH2 Slave H IRQ (MD 2)
MD VDP check:
#59 Bitmap Mode Register
#60 Shift Register
#61 Auto Fill Length Register
#62 Auto Fill Start Address Register
#63 V Blank BIT
#64 H Blank BIT
#65 Palette Enable BIT
SH-2 VDP check:
#66 Frame Swap BIT
#67 SH-2 Master Bitmap MD
#68 SH-2 Slave Bitmap MD
#69 SH-2 Master Shift
#70 SH-2 Slave Shift
#71 SH-2 Master Fill SIZE
#72 SH-2 Slave Fill SIZE
#73 SH-2 Master Fill START
#74 SH-2 Slave Fill START
#75 SH-2 Master V Blank Bit
#76 SH-2 Slave V Blank Bit
#77 SH-2 Master H Blank Bit
#78 SH-2 Slave H Blank Bit
#79 SH-2 Master Palette Enable Bit
#80 SH-2 Slave Palette Enable Bit
#81 SH-2 Master Frame Swap Bit
#82 SH-2 Slave Frame Swap Bit
Framebuffer Check:
#83 MD Frame Buffer 0
#84 MD Frame Buffer 1
#85 SH-2 Master Frame Buffer 0
#86 SH-2 Slave Frame Buffer 0
#87 SH-2 Master Frame Buffer 1
#88 SH-2 Slave Frame Buffer 1
#89 MD Frame Buffer 0 Overwrite
#90 MD Frame Buffer 1 Overwrite
#91 MD Frame Buffer 0 Byte Write
#92 MD Frame Buffer 1 Byte Write
#93 SH-2 Master Frame Buffer 0 Overwrite
#94 SH-2 Slave Frame Buffer 0 Overwrite
#95 SH-2 Master Frame Buffer 1 Overwrite
#96 SH-2 Slave Frame Buffer 1 Overwrite
#97 SH-2 Master Frame Buffer 0 Byte Write
#98 SH-2 Slave Frame Buffer 0 Byte Write
#99 SH-2 Master Frame Buffer 1 Byte Write
#100 SH-2 Slave Frame Buffer 1 Byte Write
#101 MD Frame Buffer 0 Fill Data
#102 MD Frame Buffer 1 Fill Data
#103 MD Frame Buffer 0 Fill Length & Address
#104 MD Frame Buffer 1 Fill Length & Address
#105 SH-2 Master Frame Buffer 0 Fill Data
#106 SH-2 Slave Frame Buffer 0 Fill Data
#107 SH-2 Master Frame Buffer 1 Fill Data
#108 SH-2 Slave Frame Buffer 1 Fill Data
#109 SH-2 Master Frame Buffer 0 Fill Address
#110 SH-2 Slave Frame Buffer 0 Fill Address
#111 SH-2 Master Frame Buffer 1 Fill Address
#112 SH-2 Slave Frame Buffer 1 Fill Address
#113 MD Palette R/W (Blank Mode)
#114 MD Palette R/W (Display Mode)
#115 MD Palette R/W (Fill Mode)
#116 SH-2 Master Palette R/W (Blank Mode)
#117 SH-2 Slave Palette R/W (Blank Mode)
#118 SH-2 Master Palette R/W (Display Mode)
#119 SH-2 Slave Palette R/W (Display Mode)
#120 SH-2 Master Palette R/W (Fill Mode)
#121 SH-2 Slave Palette R/W (Fill Mode)
MD or SH-2 DMA check:
#122 SH-2 Master CPU Write DMA (68S) (ERROR)
#123 SH-2 Slave CPU Write DMA (68S) (ERROR)
#124 MD ROM to VRAM DMA (asserts after this)
-----
#127 SH-2 Master ROM to SDRAM DMA
#128 SH-2 Slave ROM to SDRAM DMA
#129 SH-2 Master ROM to Frame DMA
#130 SH-2 Slave ROM to Frame DMA
#131 SH-2 Master SDRAM to Frame DMA
#132 SH-2 Slave SDRAM to Frame DMA
#133 SH-2 Master Frame to SDRAM DMA
#134 SH-2 Slave Frame to SDRAM DMA
Sound Test (these don't explicitly fails):
#135 MD 68k Monaural Sound
#136 MD 68k L Sound
#137 MD 68k R Sound
#138 MD 68k L -> R Sound
#139 MD 68k R -> L Sound
#140 SH-2 Master Monaural Sound
#141 SH-2 Master L Sound
#142 SH-2 Master R Sound
#143 SH-2 Master L -> R Pan
#144 SH-2 Master R -> L Pan
#145 SH-2 Slave Monaural Sound
#146 SH-2 Slave L Sound
#147 SH-2 Slave R Sound
#148 SH-2 Slave L -> R Pan
#149 SH-2 Slave R -> L Pan
#150 SH-2 Master PWM Interrupt
#151 SH-2 Slave PWM Interrupt
#152 SH-2 Master PWM DMA Write (!)
#153 SH-2 Slave PWM DMA Write (!)
#154 Z80 PWM Monaural Sound (!)
#155 Z80 PWM L Sound (!)
#156 Z80 PWM R Sound (!)
GFX check (these don't explicitly fails):
#157 Direct Color Mode
#158 Packed Pixel Mode
#159 Runlength Mode
#160 Runlength Mode
#161 Runlength Mode

*/
#include "emu.h"
#include "mega32x.h"
#include "machine/timer.h"


// Fifa96 needs the CPUs swapped for the gameplay to enter due to some race conditions
// when using the DRC core.  Needs further investigation, the non-DRC core works either
// way
#define _32X_SWAP_MASTER_SLAVE_HACK
#define _32X_COMMS_PORT_SYNC 0
#define MAX_HPOSITION 480
/* need to make some pwm stuff part of device */



#define SH2_VRES_IRQ_LEVEL 14
#define SH2_VINT_IRQ_LEVEL 12
#define SH2_HINT_IRQ_LEVEL 10
#define SH2_CINT_IRQ_LEVEL 8
#define SH2_PINT_IRQ_LEVEL 6


DEFINE_DEVICE_TYPE(SEGA_32X_NTSC, sega_32x_ntsc_device, "sega_32x_ntsc", "Sega 32X (NTSC)")
DEFINE_DEVICE_TYPE(SEGA_32X_PAL,  sega_32x_pal_device,  "sega_32x_pal",  "Sega 32X (PAL)")

sega_32x_device::sega_32x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, device_sound_interface(mconfig, *this)
	, device_video_interface(mconfig, *this, false)
	, m_sh2_shared(*this, "sh2_shared")
	, m_main_cpu(*this, finder_base::DUMMY_TAG)
	, m_master_cpu(*this, "32x_master_sh2")
	, m_slave_cpu(*this, "32x_slave_sh2")
	, m_ldac(*this, "ldac")
	, m_rdac(*this, "rdac")
	, m_scan_timer(*this, finder_base::DUMMY_TAG)
	, m_rombank(*this, "rombank")
	, m_stream(nullptr)
{
}

sega_32x_ntsc_device::sega_32x_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sega_32x_device(mconfig, SEGA_32X_NTSC, tag, owner, clock)
{
}

sega_32x_pal_device::sega_32x_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sega_32x_device(mconfig, SEGA_32X_PAL, tag, owner, clock)
{
}

uint16_t sega_32x_device::m68k_palette_r(offs_t offset)
{
	return m_32x_palette[offset];
}

void sega_32x_device::m68k_palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_32x_palette[offset]);
}

uint16_t sega_32x_device::m68k_dram_r(offs_t offset)
{
	return m_32x_access_dram[offset];
}

void sega_32x_device::m68k_dram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if ((mem_mask&0xffff) == 0xffff)
	{
		// 16-bit accesses are normal
		COMBINE_DATA(&m_32x_access_dram[offset]);
	}
	else
	{
		// 8-bit writes act as if they were going to the overwrite region!
		// bc-racers and world series baseball rely on this!
		// (tested on real hw)

		if ((mem_mask & 0xffff) == 0xff00)
		{
			if ((data & 0xff00) != 0x0000)
			{
				m_32x_access_dram[offset] = (data & 0xff00) |  (m_32x_access_dram[offset] & 0x00ff);
			}
		}
		else if ((mem_mask & 0xffff) == 0x00ff)
		{
			if ((data & 0x00ff) != 0x0000)
			{
				m_32x_access_dram[offset] = (data & 0x00ff) |  (m_32x_access_dram[offset] & 0xff00);
			}
		}
	}
}

uint16_t sega_32x_device::m68k_dram_overwrite_r(offs_t offset)
{
	return m_32x_access_dram[offset];
}

void sega_32x_device::m68k_dram_overwrite_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//COMBINE_DATA(&m_32x_access_dram[offset+0x10000]);

	if (ACCESSING_BITS_8_15)
	{
		if (data & 0xff00)
		{
			m_32x_access_dram[offset] = (m_32x_access_dram[offset]&0x00ff) | (data & 0xff00);
		}
	}

	if (ACCESSING_BITS_0_7)
	{
		if (data & 0x00ff)
		{
			m_32x_access_dram[offset] = (m_32x_access_dram[offset]&0xff00) | (data & 0x00ff);
		}
	}
}

/**********************************************************************************************/
// 68k side a15112
// FIFO
/**********************************************************************************************/




/*

15106 DREQ

 ---- ---- F--- -K0R

 F = Fifo FULL
 K = 68k CPU Write mode (0 = no, 1 = CPU write)
 0 = always 0? no, marsch test wants it to be latched or 1
 R = RV (0 = no operation, 1 = DMA Start allowed) <-- RV bit actually affects memory mapping, this is misleading..  it just sets the memory up in a suitable way to use the genesis VDP DMA

*/




uint16_t sega_32x_device::m68k_a15106_r()
{
	uint16_t retval;

	retval = m_a15106_reg;

	if (m_fifo_block_a_full && m_fifo_block_b_full) retval |= 0x8080;

	return retval;
}

void sega_32x_device::m68k_a15106_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_a15106_reg = data & 0x7;

		if (m_a15106_reg & 0x1) /* NBA Jam TE relies on this */
		{
			// install the game rom in the normal 0x000000-0x03fffff space used by the genesis - this allows VDP DMA operations to work as they have to be from this area or RAM
			// it should also UNMAP the banked rom area...
			space.install_rom(0x0000100, 0x03fffff, machine().root_device().memregion("gamecart")->base() + 0x100);
		}
		else
		{
			// we should be careful and map back any rom overlay (hint) and backup ram too I think...

			// this is actually blank / nop area
			// we should also map the banked area back (we don't currently unmap it tho)
			space.install_rom(0x0000100, 0x03fffff, machine().root_device().memregion("maincpu")->base()+0x100);
		}

		if((m_a15106_reg & 4) == 0) // clears the FIFO state
		{
			m_current_fifo_block = m_fifo_block_a;
			m_current_fifo_readblock = m_fifo_block_b;
			m_current_fifo_write_pos = 0;
			m_current_fifo_read_pos = 0;
			m_fifo_block_a_full = 0;
			m_fifo_block_b_full = 0;
		}

		//printf("m68k_a15106_w %04x\n", data);
		/*
		if (m_a15106_reg & 0x4)
		    printf(" --- 68k Write Mode enabled\n");
		else
		    printf(" --- 68k Write Mode disabled\n");

		if (m_a15106_reg & 0x1)
		    printf(" --- DMA Start Allowed \n");
		else
		    printf(" --- DMA Start No Operation\n");

		*/
	}
}



uint16_t sega_32x_device::dreq_common_r(address_space &space, offs_t offset)
{
	address_space& _68kspace = m_main_cpu->space(AS_PROGRAM);

	switch (offset)
	{
		case 0x00/2: // a15108 / 4008
		case 0x02/2: // a1510a / 400a
			return m_dreq_src_addr[offset&1];

		case 0x04/2: // a1510c / 400c
		case 0x06/2: // a1510e / 400e
			return m_dreq_dst_addr[offset&1];

		case 0x08/2: // a15110 / 4010
			return m_dreq_size;

		case 0x0a/2: // a15112 / 4012
			if (&space == &_68kspace)
			{
				printf("attempting to READ FIFO with 68k!\n");
				return 0xffff;
			}

			uint16_t retdat = m_current_fifo_readblock[m_current_fifo_read_pos];

			m_current_fifo_read_pos++;

		//  printf("reading FIFO!\n");

			if (m_current_fifo_readblock == m_fifo_block_a && !m_fifo_block_a_full)
				logerror("Fifo block a isn't filled!\n");

			if (m_current_fifo_readblock == m_fifo_block_b && !m_fifo_block_b_full)
				logerror("%s Fifo block b isn't filled!\n", machine().describe_context());


			if (m_current_fifo_read_pos==4)
			{
				if (m_current_fifo_readblock == m_fifo_block_a)
				{
					m_fifo_block_a_full = 0;

					if (m_fifo_block_b_full)
					{
						m_current_fifo_readblock = m_fifo_block_b;
						m_current_fifo_block = m_fifo_block_a;
					}

					m_current_fifo_read_pos = 0;
				}
				else if (m_current_fifo_readblock == m_fifo_block_b)
				{
					m_fifo_block_b_full = 0;

					if (m_fifo_block_a_full)
					{
						m_current_fifo_readblock = m_fifo_block_a;
						m_current_fifo_block = m_fifo_block_b;
					}

					m_current_fifo_read_pos = 0;
				}
			}

			return retdat;
	}

	return 0x0000;
}

void sega_32x_device::dreq_common_w(address_space &space, offs_t offset, uint16_t data)
{
	address_space& _68kspace = m_main_cpu->space(AS_PROGRAM);

	switch (offset)
	{
		case 0x00/2: // a15108 / 4008
		case 0x02/2: // a1510a / 400a
			if (&space != &_68kspace)
			{
				printf("attempting to WRITE DREQ SRC with SH2!\n");
				return;
			}

			m_dreq_src_addr[offset&1] = ((offset&1) == 0) ? (data & 0xff) : (data & 0xfffe);

			//if((m_dreq_src_addr[0]<<16)|m_dreq_src_addr[1])
			//  printf("DREQ set SRC = %08x\n",(m_dreq_src_addr[0]<<16)|m_dreq_src_addr[1]);

			break;

		case 0x04/2: // a1510c / 400c
		case 0x06/2: // a1510e / 400e
			if (&space != &_68kspace)
			{
				printf("attempting to WRITE DREQ DST with SH2!\n");
				return;
			}

			m_dreq_dst_addr[offset&1] = ((offset&1) == 0) ? (data & 0xff) : (data & 0xffff);

			//if((m_dreq_dst_addr[0]<<16)|m_dreq_dst_addr[1])
			//  printf("DREQ set DST = %08x\n",(m_dreq_dst_addr[0]<<16)|m_dreq_dst_addr[1]);

			break;

		case 0x08/2: // a15110 / 4010
			if (&space != &_68kspace)
			{
				printf("attempting to WRITE DREQ SIZE with SH2!\n");
				return;
			}

			m_dreq_size = data & 0xfffc;

			//  if(m_dreq_size)
			//      printf("DREQ set SIZE = %04x\n",m_dreq_size);

			break;

		case 0x0a/2: // a15112 / 4012 - FIFO Write (68k only!)
			if (&space != &_68kspace)
			{
				printf("attempting to WRITE FIFO with SH2!\n");
				return;
			}

			if (m_current_fifo_block==m_fifo_block_a && m_fifo_block_a_full)
			{
				printf("attempt to write to Full Fifo block a!\n");
				return;
			}

			if (m_current_fifo_block==m_fifo_block_b && m_fifo_block_b_full)
			{
				printf("attempt to write to Full Fifo block b!\n");
				return;
			}

			if((m_a15106_reg & 4) == 0)
			{
				printf("attempting to WRITE FIFO with 68S cleared!\n"); // corpse32
				return;
			}

			m_current_fifo_block[m_current_fifo_write_pos] = data;
			m_current_fifo_write_pos++;

			if (m_current_fifo_write_pos==4)
			{
				if (m_current_fifo_block==m_fifo_block_a)
				{
					m_fifo_block_a_full = 1;
					if (!m_fifo_block_b_full)
					{
						m_current_fifo_block = m_fifo_block_b;
						m_current_fifo_readblock = m_fifo_block_a;
						// incase we have a stalled DMA in progress, let the SH2 know there is data available
						m_master_cpu->sh2_notify_dma_data_available();
						m_slave_cpu->sh2_notify_dma_data_available();

					}
					m_current_fifo_write_pos = 0;
				}
				else
				{
					m_fifo_block_b_full = 1;

					if (!m_fifo_block_a_full)
					{
						m_current_fifo_block = m_fifo_block_a;
						m_current_fifo_readblock = m_fifo_block_b;
						// incase we have a stalled DMA in progress, let the SH2 know there is data available
						m_master_cpu->sh2_notify_dma_data_available();
						m_slave_cpu->sh2_notify_dma_data_available();
					}

					m_current_fifo_write_pos = 0;
				}
			}

			break;
	}
}




uint16_t sega_32x_device::m68k_a1511a_r()
{
	return m_sega_tv;
}

void sega_32x_device::m68k_a1511a_w(uint16_t data)
{
	m_sega_tv = data & 1;

	printf("SEGA TV register set = %04x\n",data);
}

/*
000070 H interrupt vector can be overwritten apparently
*/



uint16_t sega_32x_device::m68k_m_hint_vector_r(offs_t offset)
{
	return m_hint_vector[offset];
}

void sega_32x_device::m68k_m_hint_vector_w(offs_t offset, uint16_t data)
{
	m_hint_vector[offset] = data;
}

// returns MARS, the system ID of the 32x
uint16_t sega_32x_device::m68k_MARS_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return 0x4d41;

		case 1:
			return 0x5253;
	}

	return 0x0000;
}


/**********************************************************************************************/
// 68k side a15100
// control register - used to enable 32x etc.
/**********************************************************************************************/



uint16_t sega_32x_device::m68k_a15100_r()
{
	return (m_32x_access_auth<<15) | 0x0080;
}

void sega_32x_device::m68k_a15100_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_a15100_reg = (m_a15100_reg & 0xff00) | (data & 0x00ff);

		if (data & 0x02)
		{
			m_master_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			m_slave_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		}

		if (data & 0x01)
		{
			m_32x_adapter_enabled = 1;
			space.install_rom(0x0880000, 0x08fffff, machine().root_device().memregion("gamecart")->base()); // 'fixed' 512kb rom bank

			space.install_read_bank(0x0900000, 0x09fffff, m_rombank); // 'bankable' 1024kb rom bank
			m_rombank->set_base(machine().root_device().memregion("gamecart")->base()+((m_32x_68k_a15104_reg&0x3)*0x100000) );

			space.install_rom(0x0000000, 0x03fffff, machine().root_device().memregion("32x_68k_bios")->base());

			/* VDP area */
			space.install_read_handler(0x0a15180, 0x0a1518b, read16sm_delegate(*this, FUNC(sega_32x_device::common_vdp_regs_r))); // common / shared VDP regs
			space.install_write_handler(0x0a15180, 0x0a1518b, write16_delegate(*this, FUNC(sega_32x_device::common_vdp_regs_w))); // common / shared VDP regs
			space.install_read_handler(0x0a15200, 0x0a153ff, read16sm_delegate(*this, FUNC(sega_32x_device::m68k_palette_r))); // access to 'palette' xRRRRRGGGGGBBBBB
			space.install_write_handler(0x0a15200, 0x0a153ff, write16s_delegate(*this, FUNC(sega_32x_device::m68k_palette_w))); // access to 'palette' xRRRRRGGGGGBBBBB
			space.install_read_handler(0x0840000, 0x085ffff, read16sm_delegate(*this, FUNC(sega_32x_device::m68k_dram_r))); // access to 'display ram' (framebuffer)
			space.install_write_handler(0x0840000, 0x085ffff, write16s_delegate(*this, FUNC(sega_32x_device::m68k_dram_w))); // access to 'display ram' (framebuffer)
			space.install_read_handler(0x0860000, 0x087ffff, read16sm_delegate(*this, FUNC(sega_32x_device::m68k_dram_overwrite_r))); // access to 'display ram' (framebuffer)
			space.install_write_handler(0x0860000, 0x087ffff, write16s_delegate(*this, FUNC(sega_32x_device::m68k_dram_overwrite_w))); // access to 'display ram' (framebuffer)



			m_main_cpu->space(AS_PROGRAM).install_readwrite_handler(0x000070, 0x000073, read16sm_delegate(*this, FUNC(sega_32x_device::m68k_m_hint_vector_r)), write16sm_delegate(*this, FUNC(sega_32x_device::m68k_m_hint_vector_w))); // h interrupt vector
		}
		else
		{
			m_32x_adapter_enabled = 0;

			space.install_rom(0x0000000, 0x03fffff, machine().root_device().memregion("gamecart")->base());
			m_main_cpu->space(AS_PROGRAM).install_readwrite_handler(0x000070, 0x000073, read16sm_delegate(*this, FUNC(sega_32x_device::m68k_m_hint_vector_r)), write16sm_delegate(*this, FUNC(sega_32x_device::m68k_m_hint_vector_w))); // h interrupt vector
		}
	}

	if (ACCESSING_BITS_8_15)
	{
		m_a15100_reg = (m_a15100_reg & 0x00ff) | (data & 0xff00);
		m_32x_access_auth = (data & 0x8000)>>15;
	}
}

/**********************************************************************************************/
// 68k side a15102
// command interrupt to SH2
/**********************************************************************************************/



uint16_t sega_32x_device::m68k_a15102_r()
{
	//printf("_32x_68k_a15102_r\n");
	return m_32x_68k_a15102_reg;
}

void sega_32x_device::m68k_a15102_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_32x_68k_a15102_reg = data & 3;

		if (data&0x1)
		{
			if (m_sh2_master_cmdint_enable) m_master_cpu->set_input_line(SH2_CINT_IRQ_LEVEL,ASSERT_LINE);
			else printf("master cmdint when masked!\n");
		}

		if (data&0x2)
		{
			if (m_sh2_slave_cmdint_enable) m_slave_cpu->set_input_line(SH2_CINT_IRQ_LEVEL,ASSERT_LINE);
			else printf("slave cmdint when masked!\n");
		}
	}
}

/**********************************************************************************************/
// 68k side a15104
// ROM banking for 68k rom
/**********************************************************************************************/


uint16_t sega_32x_device::m68k_a15104_r()
{
	return m_32x_68k_a15104_reg;
}

void sega_32x_device::m68k_a15104_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_32x_68k_a15104_reg = (m_32x_68k_a15104_reg & 0xff00) | (data & 0x00ff);
	}

	if (ACCESSING_BITS_8_15)
	{
		m_32x_68k_a15104_reg = (m_32x_68k_a15104_reg & 0x00ff) | (data & 0xff00);
	}

	m_rombank->set_base(machine().root_device().memregion("gamecart")->base()+((m_32x_68k_a15104_reg&0x3)*0x100000) );
}

/**********************************************************************************************/
// 68k side a15120 - a1512f
// Communication Port 0
// access from the SH2 via 4020 - 402f
/**********************************************************************************************/


/**********************************************************************************************/

// reads
uint16_t sega_32x_device::m68k_m_commsram_r(offs_t offset)
{
	if (_32X_COMMS_PORT_SYNC) machine().scheduler().synchronize();
	return m_commsram[offset];
}

// writes
void sega_32x_device::m68k_m_commsram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_commsram[offset]);
	if (_32X_COMMS_PORT_SYNC) machine().scheduler().synchronize();
}

/**********************************************************************************************/
// 68k side a15130 - a1513f
// PWM registers
// access from the SH2 via 4030 - 403f
/**********************************************************************************************/

/*
TODO:
- noticeable static noise on Virtua Fighter Sega logo at start-up
- Understand if Speaker OFF makes the FIFO to advance or not
- move inside a device
*/




void sega_32x_device::calculate_pwm_timer()
{
	if(m_pwm_tm_reg == 0) { m_pwm_tm_reg = 16; } // zero gives max range
	if(m_pwm_cycle == 0) { m_pwm_cycle = 4095; } // zero gives max range

	/* if both RMD and LMD are set to OFF or pwm cycle register is one, then PWM timer ticks doesn't occur */
	if(m_pwm_cycle == 1 || ((m_pwm_ctrl & 0xf) == 0))
		m_32x_pwm_timer->adjust(attotime::never);
	else
	{
		m_pwm_timer_tick = 0;
		m_lch_fifo_state = m_rch_fifo_state = 0x4000;
		m_lch_size = m_rch_size = 0;
		m_32x_pwm_timer->adjust(attotime::from_hz(clock() / (m_pwm_cycle - 1)));
	}
}

void sega_32x_device::lch_pop()
{
	for (int i = 0; i < PWM_FIFO_SIZE - 1; i++)
		m_cur_lch[i] = m_cur_lch[i + 1];
	m_lch_size--;
}

void sega_32x_device::rch_pop()
{
	for (int i = 0; i < PWM_FIFO_SIZE - 1; i++)
		m_cur_rch[i] = m_cur_rch[i + 1];
	m_rch_size--;
}

TIMER_CALLBACK_MEMBER(sega_32x_device::handle_pwm_callback)
{
	if (m_lch_size > 0)
	{
		m_stream->update();
		switch(m_pwm_ctrl & 3)
		{
			case 0: /*Speaker OFF*/ break;
			case 1: m_ldac->write(m_cur_lch[0]); break;
			case 2: m_rdac->write(m_cur_lch[0]); break;
			case 3: popmessage("Undefined PWM Lch value 3, contact MESSdev"); break;
		}

		lch_pop();
	}

	m_lch_fifo_state = (m_lch_size == 0) ? 0x4000 : 0x0000;

	if (m_rch_size > 0)
	{
		m_stream->update();
		switch((m_pwm_ctrl & 0xc) >> 2)
		{
			case 0: /*Speaker OFF*/ break;
			case 1: m_rdac->write(m_cur_rch[0]); break;
			case 2: m_ldac->write(m_cur_rch[0]); break;
			case 3: popmessage("Undefined PWM Rch value 3, contact MESSdev"); break;
		}

		rch_pop();
	}

	m_rch_fifo_state = (m_rch_size == 0) ? 0x4000 : 0x0000;

	m_pwm_timer_tick++;

	if(m_pwm_timer_tick == m_pwm_tm_reg)
	{
		m_pwm_timer_tick = 0;
		if(sh2_master_pwmint_enable) { m_master_cpu->set_input_line(SH2_PINT_IRQ_LEVEL,ASSERT_LINE); }
		if(sh2_slave_pwmint_enable) { m_slave_cpu->set_input_line(SH2_PINT_IRQ_LEVEL,ASSERT_LINE); }
	}

	m_32x_pwm_timer->adjust(attotime::from_hz(clock() / (m_pwm_cycle - 1)));
}

uint16_t sega_32x_device::pwm_r(offs_t offset)
{
	switch(offset)
	{
		case 0x00/2: return m_pwm_ctrl; //control register
		case 0x02/2: return m_pwm_cycle_reg; // cycle register
		case 0x04/2: return m_lch_fifo_state; // l ch
		case 0x06/2: return m_rch_fifo_state; // r ch
		case 0x08/2: return m_lch_fifo_state & m_rch_fifo_state; // mono ch
	}

	printf("Read at undefined PWM register %02x\n",offset);
	return 0xffff;
}

void sega_32x_device::pwm_w(offs_t offset, uint16_t data)
{
	switch(offset)
	{
		case 0x00/2:
			m_pwm_ctrl = data & 0xffff;
			m_pwm_tm_reg = (m_pwm_ctrl & 0xf00) >> 8;
			calculate_pwm_timer();
			break;
		case 0x02/2:
			m_pwm_cycle = m_pwm_cycle_reg = data & 0xfff;
			calculate_pwm_timer();
			break;
		case 0x04/2:
			if (m_lch_size == PWM_FIFO_SIZE)
				lch_pop();
			m_cur_lch[m_lch_size++] = data;

			m_lch_fifo_state = (m_lch_size == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;
			break;
		case 0x06/2:
			if (m_rch_size == PWM_FIFO_SIZE)
				rch_pop();
			m_cur_rch[m_rch_size++] = data;

			m_rch_fifo_state = (m_rch_size == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;
			break;
		case 0x08/2:
			if (m_lch_size == PWM_FIFO_SIZE)
				lch_pop();
			m_cur_lch[m_lch_size++] = data;

			if (m_rch_size == PWM_FIFO_SIZE)
				rch_pop();
			m_cur_rch[m_rch_size++] = data;

			m_lch_fifo_state = (m_lch_size == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;
			m_rch_fifo_state = (m_rch_size == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;
			break;
		default:
			printf("Write at undefined PWM register %02x %04x\n",offset,data);
			break;
	}
}

void sega_32x_device::m68k_pwm_w(offs_t offset, uint16_t data)
{
	if(offset == 0/2)
		pwm_w(offset,(data & 0x7f) | (m_pwm_ctrl & 0xff80));
	else
		pwm_w(offset,data);
}

void sega_32x_device::sound_stream_update(sound_stream &stream)
{
	stream.copy(0, 0);
	stream.copy(1, 1);
}

/**********************************************************************************************/
// 68k side a15180
// framebuffer control
// also accessed from the SH2 @ 4100
/**********************************************************************************************/

uint16_t sega_32x_device::get_hposition(void)
{
	if (get_framerate() > 0.0)
	{
		attotime time_elapsed_since_megadriv_scanline_timer;
		uint16_t value4;

		time_elapsed_since_megadriv_scanline_timer = m_scan_timer->elapsed();

		if (time_elapsed_since_megadriv_scanline_timer.attoseconds() < (ATTOSECONDS_PER_SECOND/get_framerate() /double(m_total_scanlines)))
		{
			value4 = (uint16_t)(MAX_HPOSITION*((double)(time_elapsed_since_megadriv_scanline_timer.attoseconds()) / (double)(ATTOSECONDS_PER_SECOND/get_framerate() /double(m_total_scanlines))));
		}
		else /* in some cases (probably due to rounding errors) we get some stupid results (the odd huge value where the time elapsed is much higher than the scanline time??!).. hopefully by clamping the result to the maximum we limit errors */
		{
			value4 = MAX_HPOSITION;
		}

		return value4;
	}
	return MAX_HPOSITION;
}

uint16_t sega_32x_device::common_vdp_regs_r(offs_t offset)
{
	// what happens if the z80 accesses it, what authorization do we use?

	int ntsc;

//  printf("_32x_68k_a15180_r (a15180) %04x\n",mem_mask);

	// read needs authorization too I think, undefined behavior otherwise
	switch (offset)
	{
		case 0x00:

		// the flag is inverted compared to the megadrive
		ntsc = m_32x_pal ? 0 : 1;

		return (ntsc << 15) |
				(m_32x_videopriority << 7) |
				(m_32x_240mode << 6) |
				(m_32x_displaymode << 0);

		case 0x02/2:
			return m_32x_screenshift;

		case 0x04/2:
			return m_32x_autofill_length;

		case 0x06/2:
			return m_32x_autofill_address;

		case 0x08/2:
			return m_32x_autofill_data;

		case 0x0a/2:
			uint16_t retdata = m_32x_a1518a_reg;
			uint16_t hpos = get_hposition();
			int megadrive_hblank_flag = 0;

			if (m_32x_vblank_flag) retdata |= 0x8000;

			if (hpos>400) megadrive_hblank_flag = 1;
			if (hpos>460) megadrive_hblank_flag = 0;

			if (megadrive_hblank_flag) retdata |= 0x4000;

			if (m_32x_vblank_flag) { retdata |= 2; } // framebuffer approval (TODO: condition is unknown at current time)

			if (megadrive_hblank_flag && m_32x_vblank_flag) { retdata |= 0x2000; } // palette approval (TODO: active high or low?)

			return retdata;
	}

	return 0x0000;
}


void sega_32x_device::check_framebuffer_swap(bool enabled)
{
	// this logic should be correct, but makes things worse?
	// enabled = (genesis_scanline_counter >= megadrive_irq6_scanline) from video/315_5313.cpp
	//if (enabled)
	{
		m_32x_a1518a_reg = m_32x_fb_swap & 1;

		if (m_32x_fb_swap & 1)
		{
			m_32x_access_dram = m_32x_dram0.get();
			m_32x_display_dram = m_32x_dram1.get();
		}
		else
		{
			m_32x_display_dram = m_32x_dram0.get();
			m_32x_access_dram = m_32x_dram1.get();
		}
	}
}


void sega_32x_device::common_vdp_regs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// what happens if the z80 accesses it, what authorization do we use? which address space do we get?? the z80 *can* write here and to the framebuffer via the window

	address_space& _68kspace = m_main_cpu->space(AS_PROGRAM);

	if (&space!= &_68kspace)
	{
		if (m_32x_access_auth!=1)
			return;
	}

	if (&space== &_68kspace)
	{
		if (m_32x_access_auth!=0)
			return;
	}


	switch (offset)
	{
		case 0x00:
			//printf("_32x_68k_a15180_w (a15180) %04x %04x   source m_32x_access_auth %04x\n",data,mem_mask, m_32x_access_auth);

			if (ACCESSING_BITS_0_7)
			{
				m_32x_videopriority = (data & 0x80) >> 7;
				m_32x_240mode   = (data & 0x40) >> 6;
				m_32x_displaymode   = (data & 0x03) >> 0;
			}
			break;

		case 0x02/2:
			if (ACCESSING_BITS_0_7)
			{
				m_32x_screenshift = data & 1; // allows 1 pixel shifting
			}
			if (ACCESSING_BITS_8_15)
			{
			}
			break;

		case 0x04/2:
			if (ACCESSING_BITS_0_7)
			{
				m_32x_autofill_length = data & 0xff;
			}

			if (ACCESSING_BITS_8_15)
			{
			}
			break;

		case 0x06/2:
			if (ACCESSING_BITS_0_7)
			{
				m_32x_autofill_address = (m_32x_autofill_address & 0xff00) | (data & 0x00ff);
			}

			if (ACCESSING_BITS_8_15)
			{
				m_32x_autofill_address = (m_32x_autofill_address & 0x00ff) | (data & 0xff00);
			}
			break;

		case 0x08/2:
			if (ACCESSING_BITS_0_7)
			{
				m_32x_autofill_data = (m_32x_autofill_data & 0xff00) | (data & 0x00ff);
			}

			if (ACCESSING_BITS_8_15)
			{
				m_32x_autofill_data = (m_32x_autofill_data & 0x00ff) | (data & 0xff00);
			}

			// do the fill - shouldn't be instant..
			{
				int i;
				for (i=0; i<m_32x_autofill_length+1;i++)
				{
					m_32x_access_dram[m_32x_autofill_address] = m_32x_autofill_data;
					m_32x_autofill_address = (m_32x_autofill_address & 0xff00) | ((m_32x_autofill_address+1) & 0x00ff);
				}
			}
			break;

		case 0x0a/2:
			// bit 0 is the framebuffer select, change is delayed until vblank;
		//  m_32x_a1518a_reg = (m_32x_a1518a_reg & 0xfffe);
			if (ACCESSING_BITS_0_7)
			{
				m_32x_fb_swap = data & 1;

				check_framebuffer_swap(true);
			}

			break;


	}


}


/**********************************************************************************************/
// SH2 side 4000
// IRQ Control
// Different for each SH2

/*
f--- --ec h--- VHCP

f = framebuffer permission (0 md, 1 sh2)
e = Adapter enabled (0 no, 1 yes)
c = Cart Inserted (0 yes, 1 no)
h = H Interrupt allowed within Vblank (0 no, 1 yes)

*** these are independent for each SH2 ***
V = V Interrupt Mask (0 masked, 1 allowed)
H = H Interrupt Mask (0 masked, 1 allowed)
C = Command Interrupt Mask (0 masked, 1 allowed)
P = PWM Interrupt Mask (0 masked, 1 allowed)
*/

/**********************************************************************************************/

/* MASTER */
uint16_t sega_32x_device::master_4000_r()
{
	uint16_t retvalue = 0x0200;
	retvalue |= m_32x_access_auth << 15;

	retvalue |= m_sh2_hint_in_vbl;
	retvalue |= m_sh2_master_vint_enable;
	retvalue |= m_sh2_master_hint_enable;
	retvalue |= m_sh2_master_cmdint_enable;
	retvalue |= sh2_master_pwmint_enable;

	return retvalue;
}

void sega_32x_device::master_4000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_32x_access_auth = (data &0x8000) >> 15;
	}

	if (ACCESSING_BITS_0_7)
	{
		m_sh2_hint_in_vbl = data & 0x80;
		m_sh2_master_vint_enable = data & 0x8;
		m_sh2_master_hint_enable = data & 0x4;
		m_sh2_master_cmdint_enable = data & 0x2;
		sh2_master_pwmint_enable = data & 0x1;

		//if (m_sh2_master_hint_enable) printf("m_sh2_master_hint_enable enable!\n");
		//if (sh2_master_pwmint_enable) printf("sh2_master_pwn_enable enable!\n");

		check_irqs();
	}
}

/* SLAVE */

uint16_t sega_32x_device::slave_4000_r()
{
	uint16_t retvalue = 0x0200;
	retvalue |= m_32x_access_auth << 15;
	retvalue |= m_sh2_hint_in_vbl;
	retvalue |= m_sh2_slave_vint_enable;
	retvalue |= m_sh2_slave_hint_enable;
	retvalue |= m_sh2_slave_cmdint_enable;
	retvalue |= sh2_slave_pwmint_enable;

	return retvalue;
}


void sega_32x_device::slave_4000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_32x_access_auth = (data &0x8000) >> 15;
	}

	if (ACCESSING_BITS_0_7)
	{
		m_sh2_hint_in_vbl = data & 0x80;
		m_sh2_slave_vint_enable = data & 0x8;
		m_sh2_slave_hint_enable = data & 0x4;
		m_sh2_slave_cmdint_enable = data & 0x2;
		sh2_slave_pwmint_enable = data & 0x1;

		//if (m_sh2_slave_hint_enable) printf("m_sh2_slave_hint_enable enable!\n");
		//if (sh2_slave_pwmint_enable) printf("sh2_slave_pwm_enable enable!\n");

		check_irqs();
	}
}

/**********************************************************************************************/
// SH2 side 4002
// Reserved  ( Stand By Change Register )
// Shouldn't be used
/**********************************************************************************************/

uint16_t sega_32x_device::common_4002_r()
{
	printf("reading 4002!\n");
	return 0x0000;
}

void sega_32x_device::common_4002_w(uint16_t data)
{
	printf("write 4002!\n");
}


/**********************************************************************************************/
// SH2 side 4004
// H Count Register (H Interrupt)
// 0 = every line
/**********************************************************************************************/
uint16_t sega_32x_device::common_4004_r()
{
	return m_32x_hcount_reg;
}

void sega_32x_device::common_4004_w(uint16_t data)
{
	m_32x_hcount_reg = data & 0xff;
}


/**********************************************************************************************/
// SH2 side 4006
// DReq Control Register
/**********************************************************************************************/

uint16_t sega_32x_device::common_4006_r()
{
	//printf("DREQ read!\n"); // tempo reads it, shut up for now
	return m68k_a15106_r();
}

void sega_32x_device::common_4006_w(uint16_t data)
{
	printf("DREQ write!\n"); //register is read only on SH-2 side
}


/**********************************************************************************************/
// SH2 side 4014
// VRES (md reset button interrupt) clear
/**********************************************************************************************/

void sega_32x_device::master_4014_w(uint16_t data){ m_master_cpu->set_input_line(SH2_VRES_IRQ_LEVEL,CLEAR_LINE);}
void sega_32x_device::slave_4014_w(uint16_t data) { m_slave_cpu->set_input_line(SH2_VRES_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 4016
// VINT (vertical interrupt) clear
/**********************************************************************************************/

void sega_32x_device::master_4016_w(uint16_t data){ m_sh2_master_vint_pending = 0; check_irqs(); }
void sega_32x_device::slave_4016_w(uint16_t data) { m_sh2_slave_vint_pending = 0; check_irqs(); }

/**********************************************************************************************/
// SH2 side 4018
// HINT (horizontal interrupt) clear
/**********************************************************************************************/

void sega_32x_device::master_4018_w(uint16_t data){ m_master_cpu->set_input_line(SH2_HINT_IRQ_LEVEL,CLEAR_LINE);}
void sega_32x_device::slave_4018_w(uint16_t data) { m_slave_cpu->set_input_line(SH2_HINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401A
// HINT (control register interrupt) clear
// Note: flag cleared here is a guess, according to After Burner behaviour
/**********************************************************************************************/

void sega_32x_device::master_401a_w(uint16_t data){ m_32x_68k_a15102_reg &= ~1; m_master_cpu->set_input_line(SH2_CINT_IRQ_LEVEL,CLEAR_LINE);}
void sega_32x_device::slave_401a_w(uint16_t data) { m_32x_68k_a15102_reg &= ~2; m_slave_cpu->set_input_line(SH2_CINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401C
// PINT (PWM timer interrupt) clear
/**********************************************************************************************/

void sega_32x_device::master_401c_w(uint16_t data){ m_master_cpu->set_input_line(SH2_PINT_IRQ_LEVEL,CLEAR_LINE);}
void sega_32x_device::slave_401c_w(uint16_t data) { m_slave_cpu->set_input_line(SH2_PINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401E
// ?? unknown / unused
/**********************************************************************************************/

void sega_32x_device::master_401e_w(uint16_t data)
{
	printf("master_401e_w\n");
}

void sega_32x_device::slave_401e_w(uint16_t data)
{
	printf("slave_401e_w\n");
}

/**********************************************************************************************/
// SH2 side 4020 - 402f
// SH2 -> 68k Comms ports,
// access at a15120 - a1512f on 68k
// these just map through to the 68k functions
/**********************************************************************************************/

/* handled directly */

/**********************************************************************************************/
// SH2 side 4030
// PWM Control Register
/**********************************************************************************************/

/**********************************************************************************************/
// SH2 side 4032
// Cycle Register
/**********************************************************************************************/


/**********************************************************************************************/
// SH2 side 4034
// LCH Pulse Width Register
/**********************************************************************************************/

/**********************************************************************************************/
// SH2 side 4036
// RCH Pulse Width Register
/**********************************************************************************************/

/**********************************************************************************************/
// SH2 side 4038
// Mono Pulse Width Register
/**********************************************************************************************/

/* 4100 - 43ff are VDP registers, you need permission to access them - ensure this is true for all! */

/**********************************************************************************************/
// SH2 side 4200 - 43ff
// palette
// maps through to 68k at a15200 - a153ff
/**********************************************************************************************/

/* handled directly */

/**********************************************************************************************/
// SH2 side 4000000 - 401ffff
// framebuffer
// maps through to 68k at 840000 - 85ffff
/**********************************************************************************************/

/* handled directly */

/**********************************************************************************************/
// SH2 side 4020000 - 403ffff
// framebuffer overwrite
// maps through to 68k at 860000 - 87ffff
/**********************************************************************************************/

/* handled directly */


/**********************************************************************************************/
// SH2 memory maps
/**********************************************************************************************/

void sega_32x_device::sh2_common_map(address_map &map)
{
	map(0x00004002, 0x00004003).rw(FUNC(sega_32x_device::common_4002_r), FUNC(sega_32x_device::common_4002_w));
	map(0x00004004, 0x00004005).rw(FUNC(sega_32x_device::common_4004_r), FUNC(sega_32x_device::common_4004_w));
	map(0x00004006, 0x00004007).rw(FUNC(sega_32x_device::common_4006_r), FUNC(sega_32x_device::common_4006_w));

	map(0x00004008, 0x00004013).rw(FUNC(sega_32x_device::dreq_common_r), FUNC(sega_32x_device::dreq_common_w));

	map(0x00004014, 0x0000401f).nopr();

	map(0x00004020, 0x0000402f).rw(FUNC(sega_32x_device::m68k_m_commsram_r), FUNC(sega_32x_device::m68k_m_commsram_w));
	map(0x00004030, 0x0000403f).rw(FUNC(sega_32x_device::pwm_r), FUNC(sega_32x_device::pwm_w));

	map(0x00004100, 0x0000410b).rw(FUNC(sega_32x_device::common_vdp_regs_r), FUNC(sega_32x_device::common_vdp_regs_w));
	map(0x00004200, 0x000043ff).rw(FUNC(sega_32x_device::m68k_palette_r), FUNC(sega_32x_device::m68k_palette_w));

	map(0x04000000, 0x0401ffff).rw(FUNC(sega_32x_device::m68k_dram_r), FUNC(sega_32x_device::m68k_dram_w));
	map(0x04020000, 0x0403ffff).rw(FUNC(sega_32x_device::m68k_dram_overwrite_r), FUNC(sega_32x_device::m68k_dram_overwrite_w));

	map(0x06000000, 0x0603ffff).ram().share("sh2_shared");

	map(0x02000000, 0x023fffff).rom().region(":gamecart_sh2", 0);
	map(0x22000000, 0x223fffff).rom().region(":gamecart_sh2", 0); // cart mirror (fifa96)

	map(0xc0000000, 0xc0000fff).ram();
}

void sega_32x_device::sh2_main_map(address_map &map)
{
	sh2_common_map(map);
	map(0x00000000, 0x00003fff).bankr("masterbios");

	map(0x00004000, 0x00004001).rw(FUNC(sega_32x_device::master_4000_r), FUNC(sega_32x_device::master_4000_w));

	map(0x00004014, 0x00004015).w(FUNC(sega_32x_device::master_4014_w));
	map(0x00004016, 0x00004017).w(FUNC(sega_32x_device::master_4016_w));
	map(0x00004018, 0x00004019).w(FUNC(sega_32x_device::master_4018_w));
	map(0x0000401a, 0x0000401b).w(FUNC(sega_32x_device::master_401a_w));
	map(0x0000401c, 0x0000401d).w(FUNC(sega_32x_device::master_401c_w));
	map(0x0000401e, 0x0000401f).w(FUNC(sega_32x_device::master_401e_w));
}

void sega_32x_device::sh2_slave_map(address_map &map)
{
	sh2_common_map(map);
	map(0x00000000, 0x00003fff).bankr("slavebios");

	map(0x00004000, 0x00004001).rw(FUNC(sega_32x_device::slave_4000_r), FUNC(sega_32x_device::slave_4000_w));

	map(0x00004014, 0x00004015).w(FUNC(sega_32x_device::slave_4014_w));
	map(0x00004016, 0x00004017).w(FUNC(sega_32x_device::slave_4016_w));
	map(0x00004018, 0x00004019).w(FUNC(sega_32x_device::slave_4018_w));
	map(0x0000401a, 0x0000401b).w(FUNC(sega_32x_device::slave_401a_w));
	map(0x0000401c, 0x0000401d).w(FUNC(sega_32x_device::slave_401c_w));
	map(0x0000401e, 0x0000401f).w(FUNC(sega_32x_device::slave_401e_w));
}

/****************************************** END 32X related *************************************/




void sega_32x_device::check_irqs()
{
	if (m_sh2_master_vint_enable && m_sh2_master_vint_pending) m_master_cpu->set_input_line(SH2_VINT_IRQ_LEVEL,ASSERT_LINE);
	else m_master_cpu->set_input_line(SH2_VINT_IRQ_LEVEL,CLEAR_LINE);

	if (m_sh2_slave_vint_enable && m_sh2_slave_vint_pending) m_slave_cpu->set_input_line(SH2_VINT_IRQ_LEVEL,ASSERT_LINE);
	else m_slave_cpu->set_input_line(SH2_VINT_IRQ_LEVEL,CLEAR_LINE);
}

void sega_32x_device::interrupt_cb(int scanline, int irq6)
{
	if (scanline == irq6)
	{
		m_32x_vblank_flag = 1;
		m_sh2_master_vint_pending = 1;
		m_sh2_slave_vint_pending = 1;
		check_irqs();
	}

	check_framebuffer_swap(scanline >= irq6);

	m_32x_hcount_compare_val++;

	if (m_32x_hcount_compare_val >= m_32x_hcount_reg)
	{
		m_32x_hcount_compare_val = -1;

		if (scanline < 224 || m_sh2_hint_in_vbl)
		{
			if (m_sh2_master_hint_enable)
				m_master_cpu->set_input_line(SH2_HINT_IRQ_LEVEL, ASSERT_LINE);
			if (m_sh2_slave_hint_enable)
				m_slave_cpu->set_input_line(SH2_HINT_IRQ_LEVEL, ASSERT_LINE);
		}
	}
}


SH2_DMA_FIFO_DATA_AVAILABLE_CB(sega_32x_device::_32x_fifo_available_callback)
{
	if (src==0x4012)
	{
		if (m_current_fifo_readblock==m_fifo_block_a && m_fifo_block_a_full)
			return 1;

		if (m_current_fifo_readblock==m_fifo_block_b && m_fifo_block_b_full)
			return 1;

		return 0;
	}

	return 1;
}



void sega_32x_device::render_videobuffer_to_screenbuffer_helper(int scanline)
{
	int x;

	/* render 32x output to a buffer */
	if (m_32x_displaymode != 0)
	{
		if (m_32x_displaymode==1)
		{
			uint32_t lineoffs;
			int start;

			lineoffs = m_32x_display_dram[scanline];

			if (m_32x_screenshift == 0) start=0;
			else start = -1;

			for (x=start;x<320;x++)
			{
				uint16_t coldata;
				coldata = m_32x_display_dram[lineoffs];

				{
					if (x>=0)
					{
						m_32x_linerender[x] = m_32x_palette[(coldata & 0xff00)>>8];
					}

					x++;

					if (x>=0)
					{
						m_32x_linerender[x] = m_32x_palette[(coldata & 0x00ff)];
					}
				}

				lineoffs++;

			}
		}
		else if (m_32x_displaymode==3) // mode 3 = RLE  (used by BRUTAL intro)
		{
			uint32_t lineoffs;
			int start;

			lineoffs = m_32x_display_dram[scanline];

			if (m_32x_screenshift == 0) start=0;
			else start = -1;

			x = start;
			while (x<320)
			{
				uint16_t coldata, length, l;
				coldata = m_32x_display_dram[lineoffs];
				length = ((coldata & 0xff00)>>8)+1;
				coldata = (coldata & 0x00ff)>>0;
				for (l=0;l<length;l++)
				{
					if (x>=0)
					{
						m_32x_linerender[x] = m_32x_palette[(coldata)];
					}
					x++;
				}

				lineoffs++;

			}
		}
		else // MODE 2 - 15bpp mode, not used by any commercial games?
		{
			uint32_t lineoffs;
			int start;

			lineoffs = m_32x_display_dram[scanline];

			if (m_32x_screenshift == 0) start=0;
			else start = -1;

			x = start;
			while (x<320)
			{
				if (x>=0)
					m_32x_linerender[x] = m_32x_display_dram[lineoffs&0xffff];

				x++;
				lineoffs++;
			}
		}
	}
}

void sega_32x_device::render_videobuffer_to_screenbuffer(int x, uint32_t priority, uint32_t &lineptr)
{
	if (m_32x_displaymode != 0)
	{
		if (!m_32x_videopriority)
		{
			if (priority && !(m_32x_linerender[x] & 0x8000))
				lineptr = pen(m_32x_linerender[x] & 0x7fff);
			if (m_32x_linerender[x] & 0x8000)
				lineptr = pen(m_32x_linerender[x] & 0x7fff);
		}
		else
		{
			if (priority && m_32x_linerender[x] & 0x8000)
				lineptr = pen(m_32x_linerender[x] & 0x7fff);
			if (!(m_32x_linerender[x] & 0x8000))
				lineptr = pen(m_32x_linerender[x] & 0x7fff);
		}
	}
}

#if 0
// for now we just use the regular loading because we have 2 different BIOS roms, and you can't use -bios within a device for obvious reasons
ROM_START( 32x )
	ROM_REGION( 0x400000, "32x_master_sh2", 0 )
	ROM_REGION( 0x400000, "32x_slave_sh2", 0 )
ROM_END

const rom_entry *sega_32x_device::device_rom_region() const
{
	return ROM_NAME( 32x );
}
#endif


// brutal needs high levels of interleave or the background animations don't work
// and some stages simply freeze the game, the is not good for performance however.
//
// some games appear to dislike 'perfect' levels of interleave, probably due to
// non-emulated cache, ram waitstates and other issues?
#define _32X_INTERLEAVE_LEVEL \
	config.set_maximum_quantum(attotime::from_hz(1800000));

void sega_32x_device::device_add_mconfig(machine_config &config)
{
#ifndef _32X_SWAP_MASTER_SLAVE_HACK
	SH7604(config, m_master_cpu, DERIVED_CLOCK(1, 1));
	m_master_cpu->set_is_slave(0);
	m_master_cpu->set_dma_fifo_data_available_callback(FUNC(sega_32x_device::_32x_fifo_available_callback));
#endif

	SH7604(config, m_slave_cpu, DERIVED_CLOCK(1, 1));
	m_slave_cpu->set_is_slave(1);
	m_slave_cpu->set_dma_fifo_data_available_callback(FUNC(sega_32x_device::_32x_fifo_available_callback));

#ifdef _32X_SWAP_MASTER_SLAVE_HACK
	SH7604(config, m_master_cpu, DERIVED_CLOCK(1, 1));
	m_master_cpu->set_is_slave(0);
	m_master_cpu->set_dma_fifo_data_available_callback(FUNC(sega_32x_device::_32x_fifo_available_callback));
#endif

	_32X_INTERLEAVE_LEVEL
}

void sega_32x_ntsc_device::device_add_mconfig(machine_config &config)
{
	sega_32x_device::device_add_mconfig(config);

	m_master_cpu->set_addrmap(AS_PROGRAM, &sega_32x_ntsc_device::sh2_main_map);
	m_slave_cpu->set_addrmap(AS_PROGRAM, &sega_32x_ntsc_device::sh2_slave_map);

	DAC_12BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, *this, 0.4, 0); // unknown DAC
	DAC_12BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, *this, 0.4, 1); // unknown DAC
}

void sega_32x_pal_device::device_add_mconfig(machine_config &config)
{
	sega_32x_device::device_add_mconfig(config);

	m_master_cpu->set_addrmap(AS_PROGRAM, &sega_32x_pal_device::sh2_main_map);
	m_slave_cpu->set_addrmap(AS_PROGRAM, &sega_32x_pal_device::sh2_slave_map);

	DAC_16BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, *this, 0.4, 0); // unknown DAC
	DAC_16BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, *this, 0.4, 1); // unknown DAC
}


void sega_32x_device::device_start()
{
	for (int i = 0; i < 32*32*32/**2*/; i++)
	{
		int r = (i >> 0) & 0x1f;
		int g = (i >> 5) & 0x1f;
		int b = (i >> 10) & 0x1f;
		//int p = (i >> 15) & 0x01; // priority 'through' bit
		set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}

	m_stream = stream_alloc(2, 2, 48000 * 4);
	m_32x_pwm_timer = timer_alloc(FUNC(sega_32x_device::handle_pwm_callback), this);

	m_32x_dram0 = std::make_unique<uint16_t[]>(0x40000/2);
	m_32x_dram1 = std::make_unique<uint16_t[]>(0x40000/2);

	memset(m_32x_dram0.get(), 0x00, 0x40000);
	memset(m_32x_dram1.get(), 0x00, 0x40000);

	m_32x_palette = std::make_unique<uint16_t[]>(0x200/2);

	memset(m_32x_palette.get(), 0x00, 0x200);

	m_32x_display_dram = m_32x_dram0.get();
	m_32x_access_dram = m_32x_dram1.get();
}

void sega_32x_device::device_reset()
{
	/* Interrupts are masked / disabled at first */
	m_sh2_master_vint_enable = m_sh2_slave_vint_enable = 0;
	m_sh2_master_hint_enable = m_sh2_slave_hint_enable = 0;
	m_sh2_master_cmdint_enable = m_sh2_slave_cmdint_enable = 0;
	sh2_master_pwmint_enable = sh2_slave_pwmint_enable = 0;
	m_sh2_master_vint_pending = m_sh2_slave_vint_pending = 0;
	m_sh2_hint_in_vbl = 0;

	// start in a reset state
	m_sh2_are_running = 0;

	m_32x_a1518a_reg = 0x00; // initial value
	m_32x_68k_a15104_reg = 0x00;

	m_32x_autofill_length = 0;
	m_32x_autofill_address = 0;
	m_32x_autofill_data = 0;
	m_32x_screenshift = 0;
	m_32x_videopriority = 0; // MD priority
	m_32x_displaymode = 0;
	m_32x_240mode = 0;

	m_current_fifo_block = m_fifo_block_a;
	m_current_fifo_readblock = m_fifo_block_b;
	m_current_fifo_write_pos = 0;
	m_current_fifo_read_pos = 0;
	m_fifo_block_a_full = 0;
	m_fifo_block_b_full = 0;

	m_32x_hcount_compare_val = -1;
	m_32x_hcount_reg = 0;

	m_32x_fb_swap = 0;

	m_pwm_tm_reg = 0;
	m_pwm_cycle = m_pwm_cycle_reg = 0;
	m_pwm_ctrl = 0;
	calculate_pwm_timer();

	m_lch_size = 0;
	m_rch_size = 0;

	m_total_scanlines = 262;

// moved from init

	m_32x_adapter_enabled = 0;
	m_32x_access_auth = 0;

	if (m_32x_adapter_enabled == 0)
	{
		m_main_cpu->space(AS_PROGRAM).install_rom(0x0000000, 0x03fffff, machine().root_device().memregion(":gamecart")->base());
		m_main_cpu->space(AS_PROGRAM).install_readwrite_handler(0x000070, 0x000073, read16sm_delegate(*this, FUNC(sega_32x_device::m68k_m_hint_vector_r)), write16sm_delegate(*this, FUNC(sega_32x_device::m68k_m_hint_vector_w))); // h interrupt vector
	};


	m_a15100_reg = 0x0000;
	m_main_cpu->space(AS_PROGRAM).install_read_handler(0xa15100, 0xa15101, read16smo_delegate(*this, FUNC(sega_32x_device::m68k_a15100_r))); // framebuffer control regs
	m_main_cpu->space(AS_PROGRAM).install_write_handler(0xa15100, 0xa15101, write16_delegate(*this, FUNC(sega_32x_device::m68k_a15100_w))); // framebuffer control regs
	m_main_cpu->space(AS_PROGRAM).install_read_handler(0xa15102, 0xa15103, read16smo_delegate(*this, FUNC(sega_32x_device::m68k_a15102_r))); // send irq to sh2
	m_main_cpu->space(AS_PROGRAM).install_write_handler(0xa15102, 0xa15103, write16s_delegate(*this, FUNC(sega_32x_device::m68k_a15102_w))); // send irq to sh2
	m_main_cpu->space(AS_PROGRAM).install_read_handler(0xa15104, 0xa15105, read16smo_delegate(*this, FUNC(sega_32x_device::m68k_a15104_r))); // 68k BANK rom set
	m_main_cpu->space(AS_PROGRAM).install_write_handler(0xa15104, 0xa15105, write16s_delegate(*this, FUNC(sega_32x_device::m68k_a15104_w))); // 68k BANK rom set
	m_main_cpu->space(AS_PROGRAM).install_read_handler(0xa15106, 0xa15107, read16smo_delegate(*this, FUNC(sega_32x_device::m68k_a15106_r))); // dreq stuff
	m_main_cpu->space(AS_PROGRAM).install_write_handler(0xa15106, 0xa15107, write16_delegate(*this, FUNC(sega_32x_device::m68k_a15106_w))); // dreq stuff
	m_main_cpu->space(AS_PROGRAM).install_readwrite_handler(0xa15108, 0xa15113, read16m_delegate(*this, FUNC(sega_32x_device::dreq_common_r)), write16m_delegate(*this, FUNC(sega_32x_device::dreq_common_w))); // dreq src / dst / length /fifo

	m_main_cpu->space(AS_PROGRAM).install_readwrite_handler(0xa1511a, 0xa1511b, read16smo_delegate(*this, FUNC(sega_32x_device::m68k_a1511a_r)), write16smo_delegate(*this, FUNC(sega_32x_device::m68k_a1511a_w))); // SEGA TV

	m_main_cpu->space(AS_PROGRAM).install_read_handler(0xa15120, 0xa1512f, read16sm_delegate(*this, FUNC(sega_32x_device::m68k_m_commsram_r))); // comms reg 0-7
	m_main_cpu->space(AS_PROGRAM).install_write_handler(0xa15120, 0xa1512f, write16s_delegate(*this, FUNC(sega_32x_device::m68k_m_commsram_w))); // comms reg 0-7
	m_main_cpu->space(AS_PROGRAM).install_readwrite_handler(0xa15130, 0xa1513f, read16sm_delegate(*this, FUNC(sega_32x_device::pwm_r)), write16sm_delegate(*this, FUNC(sega_32x_device::m68k_pwm_w)));

	m_main_cpu->space(AS_PROGRAM).install_read_handler(0x0a130ec, 0x0a130ef, read16sm_delegate(*this, FUNC(sega_32x_device::m68k_MARS_r))); // system ID



// checking if these help brutal, they don't.
	m_master_cpu->sh2drc_set_options(SH2DRC_COMPATIBLE_OPTIONS);
	m_slave_cpu->sh2drc_set_options(SH2DRC_COMPATIBLE_OPTIONS);

	uint32_t *cart = (uint32_t *)memregion(":gamecart_sh2")->base();

	m_master_cpu->sh2drc_add_fastram(0x06000000, 0x0603ffff, 0, &m_sh2_shared[0]);
	m_master_cpu->sh2drc_add_fastram(0x02000000, 0x023fffff, 0, cart);
	m_master_cpu->sh2drc_add_fastram(0x22000000, 0x223fffff, 0, cart);
	m_slave_cpu->sh2drc_add_fastram(0x06000000, 0x0603ffff, 0, &m_sh2_shared[0]);
	m_slave_cpu->sh2drc_add_fastram(0x02000000, 0x023fffff, 0, cart);
	m_slave_cpu->sh2drc_add_fastram(0x22000000, 0x223fffff, 0, cart);


// install these now, otherwise we'll get the following (incorrect) warnings on startup..
//   SH-2 device ':sega32x:32x_slave_sh2': program space memory map entry 0-3FFF references non-existent region ':slave'
//   SH-2 device ':sega32x:32x_master_sh2': program space memory map entry 0-3FFF references non-existent region ':master'
	uint8_t* masterbios = (uint8_t*)machine().root_device().memregion(":master")->base();
	uint8_t* slavebios = (uint8_t*)machine().root_device().memregion(":slave")->base();
	membank("masterbios")->configure_entries(0, 1, masterbios, 0x4000);
	membank("slavebios")->configure_entries(0, 1, slavebios, 0x4000);
	membank("masterbios")->set_entry(0);
	membank("slavebios")->set_entry(0);
}

// if the system has a 32x, the extra CPUs are paused at start
void sega_32x_device::pause_cpu()
{
	m_master_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_slave_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}
