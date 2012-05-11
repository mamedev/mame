/* Megadrive / Genesis 32X emulation */

/*

    Fix 32X support (not used by any arcade systems?)
     - this seems to require far greater sync and timing accuracy on rom / ram access than MAME can provide
     - split NTSC / PAL drivers
     - 36greatju: missing backup ram, has issues with golfer select due of that
     - bcracers: write to undefined PWM register?
     - fifa96 / nbajamte: dies on the gameplay, waiting for a comm change that never occurs;
     - marsch1: doesn't boot, Master / Slave communicates through SCI
     - nbajamte: missing I2C hookup, startup fails due of that (same I2C type as plain MD version);
     - nflquart: black screen, missing h irq?
     - sangoku4: black screen after the Sega logo
     - soulstar: OSD and player sprite isn't drawn;
     - tempo: intro is too fast, mostly noticeable with the PWM sound that cuts off too early when it gets to the title screen;
     - tmek: gameplay is clearly too fast
     - vrdxu: has 3d geometry bugs, caused by a SH-2 DRC bug;
     - vrdxu: crashes if you attempt to enter into main menu;
     - wwfraw: writes fb data to the cart area and expects it to be read back, kludging the cart area to be writeable makes the 32x gfxs to appear, why?
     - wwfwre: no 32x gfxs
     - xmen: black screen after that you choose the level, needs bare minimum SH-2 SCI support


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
		 
#include "includes/megadriv.h"


/* the main Megadrive emulation needs to know this */
int _32x_is_connected;
cpu_device *_32x_master_cpu;
cpu_device *_32x_slave_cpu;

/* our current main rendering code needs to know this for mixing in */
int _32x_displaymode;
int _32x_videopriority;

/* our main vblank handler resets this */
int _32x_hcount_compare_val;


static int sh2_are_running;
static int _32x_adapter_enabled;
static int _32x_access_auth;
static int _32x_screenshift;

static int _32x_240mode;

static UINT16 _32x_68k_a15104_reg;


static int sh2_master_vint_enable, sh2_slave_vint_enable;
static int sh2_master_hint_enable, sh2_slave_hint_enable;
static int sh2_master_cmdint_enable, sh2_slave_cmdint_enable;
static int sh2_master_pwmint_enable, sh2_slave_pwmint_enable;
static int sh2_hint_in_vbl;

static int sh2_master_vint_pending;
static int sh2_slave_vint_pending;
static int _32x_fb_swap;
static int _32x_hcount_reg;

void _32x_check_irqs(running_machine& machine);

#define SH2_VRES_IRQ_LEVEL 14
#define SH2_VINT_IRQ_LEVEL 12
#define SH2_HINT_IRQ_LEVEL 10
#define SH2_CINT_IRQ_LEVEL 8
#define SH2_PINT_IRQ_LEVEL 6


static UINT16* _32x_dram0;
static UINT16* _32x_dram1;
static UINT16 *_32x_display_dram, *_32x_access_dram;
static UINT16* _32x_palette;
static UINT16* _32x_palette_lookup;




/****************************************** 32X related ******************************************/

/**********************************************************************************************/
// Function Prototypes
/**********************************************************************************************/


static READ16_HANDLER( _32x_common_vdp_regs_r );
static WRITE16_HANDLER( _32x_common_vdp_regs_w );

static UINT16 _32x_autofill_length;
static UINT16 _32x_autofill_address;
static UINT16 _32x_autofill_data;






static READ16_HANDLER( _32x_68k_palette_r )
{
	return _32x_palette[offset];
}

static WRITE16_HANDLER( _32x_68k_palette_w )
{
	int r,g,b, p;

	COMBINE_DATA(&_32x_palette[offset]);
	data = _32x_palette[offset];

	r = ((data >> 0)  & 0x1f);
	g = ((data >> 5)  & 0x1f);
	b = ((data >> 10) & 0x1f);
	p = ((data >> 15) & 0x01); // priority 'through' bit

	_32x_palette_lookup[offset] = (r << 10) | (g << 5) | (b << 0) | (p << 15);

	palette_set_color_rgb(space->machine(),offset+0x40,pal5bit(r),pal5bit(g),pal5bit(b));

}

static READ16_HANDLER( _32x_68k_dram_r )
{
	return _32x_access_dram[offset];
}

static WRITE16_HANDLER( _32x_68k_dram_w )
{
	if ((mem_mask&0xffff) == 0xffff)
	{
		// 16-bit accesses are normal
		COMBINE_DATA(&_32x_access_dram[offset]);
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
				_32x_access_dram[offset] = (data & 0xff00) |  (_32x_access_dram[offset] & 0x00ff);
			}
		}
		else if ((mem_mask & 0xffff) == 0x00ff)
		{
			if ((data & 0x00ff) != 0x0000)
			{
				_32x_access_dram[offset] = (data & 0x00ff) |  (_32x_access_dram[offset] & 0xff00);
			}
		}
	}
}

static READ16_HANDLER( _32x_68k_dram_overwrite_r )
{
	return _32x_access_dram[offset];
}

static WRITE16_HANDLER( _32x_68k_dram_overwrite_w )
{
	//COMBINE_DATA(&_32x_access_dram[offset+0x10000]);

	if (ACCESSING_BITS_8_15)
	{
		if (data & 0xff00)
		{
			_32x_access_dram[offset] = (_32x_access_dram[offset]&0x00ff) | (data & 0xff00);
		}
	}

	if (ACCESSING_BITS_0_7)
	{
		if (data & 0x00ff)
		{
			_32x_access_dram[offset] = (_32x_access_dram[offset]&0xff00) | (data & 0x00ff);
		}
	}
}

/**********************************************************************************************/
// 68k side a15112
// FIFO
/**********************************************************************************************/

static UINT16 fifo_block_a[4];
static UINT16 fifo_block_b[4];
static UINT16* current_fifo_block;
static UINT16* current_fifo_readblock;
int current_fifo_write_pos;
int current_fifo_read_pos;
int fifo_block_a_full;
int fifo_block_b_full;




/*

15106 DREQ

 ---- ---- F--- -K0R

 F = Fifo FULL
 K = 68k CPU Write mode (0 = no, 1 = CPU write)
 0 = always 0? no, marsch test wants it to be latched or 1
 R = RV (0 = no operation, 1 = DMA Start allowed) <-- RV bit actually affects memory mapping, this is misleading..  it just sets the memory up in a suitable way to use the genesis VDP DMA

*/

static UINT16 a15106_reg;


static READ16_HANDLER( _32x_68k_a15106_r )
{
	UINT16 retval;

	retval = a15106_reg;

	if (fifo_block_a_full && fifo_block_b_full) retval |= 0x8080;

	return retval;
}

static WRITE16_HANDLER( _32x_68k_a15106_w )
{
	if (ACCESSING_BITS_0_7)
	{
		a15106_reg = data & 0x7;

        if (a15106_reg & 0x1) /* NBA Jam TE relies on this */
		{

			// install the game rom in the normal 0x000000-0x03fffff space used by the genesis - this allows VDP DMA operations to work as they have to be from this area or RAM
			// it should also UNMAP the banked rom area...
			space->install_rom(0x0000100, 0x03fffff, space->machine().root_device().memregion("gamecart")->base() + 0x100);
		}
		else
		{
			// we should be careful and map back any rom overlay (hint) and backup ram too I think...

			// this is actually blank / nop area
			// we should also map the banked area back (we don't currently unmap it tho)
			space->install_rom(0x0000100, 0x03fffff, space->machine().root_device().memregion("maincpu")->base()+0x100);
		}

		if((a15106_reg & 4) == 0) // clears the FIFO state
		{
			current_fifo_block = fifo_block_a;
			current_fifo_readblock = fifo_block_b;
			current_fifo_write_pos = 0;
			current_fifo_read_pos = 0;
			fifo_block_a_full = 0;
			fifo_block_b_full = 0;
		}

		//printf("_32x_68k_a15106_w %04x\n", data);
		/*
        if (a15106_reg & 0x4)
            printf(" --- 68k Write Mode enabled\n");
        else
            printf(" --- 68k Write Mode disabled\n");

        if (a15106_reg & 0x1)
            printf(" --- DMA Start Allowed \n");
        else
            printf(" --- DMA Start No Operation\n");

        */
	}
}

static UINT16 dreq_src_addr[2],dreq_dst_addr[2],dreq_size;

static READ16_HANDLER( _32x_dreq_common_r )
{
	address_space* _68kspace = space->machine().device("maincpu")->memory().space(AS_PROGRAM);

	switch (offset)
	{
		case 0x00/2: // a15108 / 4008
		case 0x02/2: // a1510a / 400a
			return dreq_src_addr[offset&1];

		case 0x04/2: // a1510c / 400c
		case 0x06/2: // a1510e / 400e
			return dreq_dst_addr[offset&1];

		case 0x08/2: // a15110 / 4010
			return dreq_size;

		case 0x0a/2: // a15112 / 4012
			if (space == _68kspace)
			{
				printf("attempting to READ FIFO with 68k!\n");
				return 0xffff;
			}

			UINT16 retdat = current_fifo_readblock[current_fifo_read_pos];

			current_fifo_read_pos++;

		//  printf("reading FIFO!\n");

			if (current_fifo_readblock == fifo_block_a && !fifo_block_a_full)
				printf("Fifo block a isn't filled!\n");

			if (current_fifo_readblock == fifo_block_b && !fifo_block_b_full)
				printf("%08x Fifo block b isn't filled!\n",cpu_get_pc(&space->device()));


			if (current_fifo_read_pos==4)
			{
				if (current_fifo_readblock == fifo_block_a)
				{
					fifo_block_a_full = 0;

					if (fifo_block_b_full)
					{
						current_fifo_readblock = fifo_block_b;
						current_fifo_block = fifo_block_a;
					}

					current_fifo_read_pos = 0;
				}
				else if (current_fifo_readblock == fifo_block_b)
				{
					fifo_block_b_full = 0;

					if (fifo_block_a_full)
					{
						current_fifo_readblock = fifo_block_a;
						current_fifo_block = fifo_block_b;
					}

					current_fifo_read_pos = 0;
				}
			}

			return retdat;
	}

	return 0x0000;
}

static WRITE16_HANDLER( _32x_dreq_common_w )
{
	address_space* _68kspace = space->machine().device("maincpu")->memory().space(AS_PROGRAM);

	switch (offset)
	{
		case 0x00/2: // a15108 / 4008
		case 0x02/2: // a1510a / 400a
			if (space != _68kspace)
			{
				printf("attempting to WRITE DREQ SRC with SH2!\n");
				return;
			}

			dreq_src_addr[offset&1] = ((offset&1) == 0) ? (data & 0xff) : (data & 0xfffe);

			//if((dreq_src_addr[0]<<16)|dreq_src_addr[1])
			//  printf("DREQ set SRC = %08x\n",(dreq_src_addr[0]<<16)|dreq_src_addr[1]);

			break;

		case 0x04/2: // a1510c / 400c
		case 0x06/2: // a1510e / 400e
			if (space != _68kspace)
			{
				printf("attempting to WRITE DREQ DST with SH2!\n");
				return;
			}

			dreq_dst_addr[offset&1] = ((offset&1) == 0) ? (data & 0xff) : (data & 0xffff);

			//if((dreq_dst_addr[0]<<16)|dreq_dst_addr[1])
			//  printf("DREQ set DST = %08x\n",(dreq_dst_addr[0]<<16)|dreq_dst_addr[1]);

			break;

		case 0x08/2: // a15110 / 4010
			if (space != _68kspace)
			{
				printf("attempting to WRITE DREQ SIZE with SH2!\n");
				return;
			}

			dreq_size = data & 0xfffc;

			//  if(dreq_size)
			//      printf("DREQ set SIZE = %04x\n",dreq_size);

			break;

		case 0x0a/2: // a15112 / 4012 - FIFO Write (68k only!)
			if (space != _68kspace)
			{
				printf("attempting to WRITE FIFO with SH2!\n");
				return;
			}

			if (current_fifo_block==fifo_block_a && fifo_block_a_full)
			{
				printf("attempt to write to Full Fifo block a!\n");
				return;
			}

			if (current_fifo_block==fifo_block_b && fifo_block_b_full)
			{
				printf("attempt to write to Full Fifo block b!\n");
				return;
			}

			if((a15106_reg & 4) == 0)
			{
				printf("attempting to WRITE FIFO with 68S cleared!\n"); // corpse32
				return;
			}

			current_fifo_block[current_fifo_write_pos] = data;
			current_fifo_write_pos++;

			if (current_fifo_write_pos==4)
			{
				if (current_fifo_block==fifo_block_a)
				{
					fifo_block_a_full = 1;
					if (!fifo_block_b_full)
					{
						current_fifo_block = fifo_block_b;
						current_fifo_readblock = fifo_block_a;
						// incase we have a stalled DMA in progress, let the SH2 know there is data available
						sh2_notify_dma_data_available(space->machine().device("32x_master_sh2"));
						sh2_notify_dma_data_available(space->machine().device("32x_slave_sh2"));

					}
					current_fifo_write_pos = 0;
				}
				else
				{
					fifo_block_b_full = 1;

					if (!fifo_block_a_full)
					{
						current_fifo_block = fifo_block_a;
						current_fifo_readblock = fifo_block_b;
						// incase we have a stalled DMA in progress, let the SH2 know there is data available
						sh2_notify_dma_data_available(space->machine().device("32x_master_sh2"));
						sh2_notify_dma_data_available(space->machine().device("32x_slave_sh2"));

					}

					current_fifo_write_pos = 0;
				}
			}

			break;
	}
}


static UINT8 sega_tv;

static READ16_HANDLER( _32x_68k_a1511a_r )
{
	return sega_tv;
}

static WRITE16_HANDLER( _32x_68k_a1511a_w )
{
	sega_tv = data & 1;

	printf("SEGA TV register set = %04x\n",data);
}

/*
000070 H interrupt vector can be overwritten apparently
*/

static UINT16 hint_vector[2];

static READ16_HANDLER( _32x_68k_hint_vector_r )
{
	return hint_vector[offset];
}

static WRITE16_HANDLER( _32x_68k_hint_vector_w )
{
	hint_vector[offset] = data;
}

// returns MARS, the system ID of the 32x
static READ16_HANDLER( _32x_68k_MARS_r )
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

static UINT16 a15100_reg;

static READ16_HANDLER( _32x_68k_a15100_r )
{
	return (_32x_access_auth<<15) | 0x0080;
}

static WRITE16_HANDLER( _32x_68k_a15100_w )
{
	if (ACCESSING_BITS_0_7)
	{
		a15100_reg = (a15100_reg & 0xff00) | (data & 0x00ff);

		if (data & 0x02)
		{
			device_set_input_line(_32x_master_cpu, INPUT_LINE_RESET, CLEAR_LINE);
			device_set_input_line(_32x_slave_cpu, INPUT_LINE_RESET, CLEAR_LINE);
		}

		if (data & 0x01)
		{
			_32x_adapter_enabled = 1;
			space->install_rom(0x0880000, 0x08fffff, space->machine().root_device().memregion("gamecart")->base()); // 'fixed' 512kb rom bank

			space->install_read_bank(0x0900000, 0x09fffff, "bank12"); // 'bankable' 1024kb rom bank
			space->machine().root_device().membank("bank12")->set_base(space->machine().root_device().memregion("gamecart")->base()+((_32x_68k_a15104_reg&0x3)*0x100000) );

			space->install_rom(0x0000000, 0x03fffff, space->machine().root_device().memregion("32x_68k_bios")->base());

			/* VDP area */
			space->install_legacy_readwrite_handler(0x0a15180, 0x0a1518b, FUNC(_32x_common_vdp_regs_r), FUNC(_32x_common_vdp_regs_w)); // common / shared VDP regs
			space->install_legacy_readwrite_handler(0x0a15200, 0x0a153ff, FUNC(_32x_68k_palette_r), FUNC(_32x_68k_palette_w)); // access to 'palette' xRRRRRGGGGGBBBBB
			space->install_legacy_readwrite_handler(0x0840000, 0x085ffff, FUNC(_32x_68k_dram_r), FUNC(_32x_68k_dram_w)); // access to 'display ram' (framebuffer)
			space->install_legacy_readwrite_handler(0x0860000, 0x087ffff, FUNC(_32x_68k_dram_overwrite_r), FUNC(_32x_68k_dram_overwrite_w)); // access to 'display ram' (framebuffer)



			space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x000070, 0x000073, FUNC(_32x_68k_hint_vector_r), FUNC(_32x_68k_hint_vector_w)); // h interrupt vector
		}
		else
		{
			_32x_adapter_enabled = 0;

			space->install_rom(0x0000000, 0x03fffff, space->machine().root_device().memregion("gamecart")->base());
			space->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x000070, 0x000073, FUNC(_32x_68k_hint_vector_r), FUNC(_32x_68k_hint_vector_w)); // h interrupt vector
		}
	}

	if (ACCESSING_BITS_8_15)
	{
		a15100_reg = (a15100_reg & 0x00ff) | (data & 0xff00);
		_32x_access_auth = (data & 0x8000)>>15;
	}
}

/**********************************************************************************************/
// 68k side a15102
// command interrupt to SH2
/**********************************************************************************************/

static int _32x_68k_a15102_reg;

static READ16_HANDLER( _32x_68k_a15102_r )
{
	//printf("_32x_68k_a15102_r\n");
	return _32x_68k_a15102_reg;
}

static WRITE16_HANDLER( _32x_68k_a15102_w )
{
	if (ACCESSING_BITS_0_7)
	{
		_32x_68k_a15102_reg = data & 3;

		if (data&0x1)
		{
			if (sh2_master_cmdint_enable) device_set_input_line(_32x_master_cpu,SH2_CINT_IRQ_LEVEL,ASSERT_LINE);
			else printf("master cmdint when masked!\n");
		}

		if (data&0x2)
		{
			if (sh2_slave_cmdint_enable) device_set_input_line(_32x_slave_cpu,SH2_CINT_IRQ_LEVEL,ASSERT_LINE);
			else printf("slave cmdint when masked!\n");
		}
	}
}

/**********************************************************************************************/
// 68k side a15104
// ROM banking for 68k rom
/**********************************************************************************************/


static READ16_HANDLER( _32x_68k_a15104_r )
{
	return _32x_68k_a15104_reg;
}

static WRITE16_HANDLER( _32x_68k_a15104_w )
{
	if (ACCESSING_BITS_0_7)
	{
		_32x_68k_a15104_reg = (_32x_68k_a15104_reg & 0xff00) | (data & 0x00ff);
	}

	if (ACCESSING_BITS_8_15)
	{
		_32x_68k_a15104_reg = (_32x_68k_a15104_reg & 0x00ff) | (data & 0xff00);
	}

	space->machine().root_device().membank("bank12")->set_base(space->machine().root_device().memregion("gamecart")->base()+((_32x_68k_a15104_reg&0x3)*0x100000) );
}

/**********************************************************************************************/
// 68k side a15120 - a1512f
// Communication Port 0
// access from the SH2 via 4020 - 402f
/**********************************************************************************************/
#define _32X_COMMS_PORT_SYNC 0
static UINT16 commsram[8];

/**********************************************************************************************/

// reads
static READ16_HANDLER( _32x_68k_commsram_r )
{
	if (_32X_COMMS_PORT_SYNC) space->machine().scheduler().synchronize();
	return commsram[offset];
}

// writes
static WRITE16_HANDLER( _32x_68k_commsram_w )
{
	COMBINE_DATA(&commsram[offset]);
	if (_32X_COMMS_PORT_SYNC) space->machine().scheduler().synchronize();
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
*/

#define PWM_FIFO_SIZE pwm_tm_reg // guess, Marsch calls this register as FIFO width
#define PWM_CLOCK megadrive_region_pal ? ((MASTER_CLOCK_PAL*3) / 7) : ((MASTER_CLOCK_NTSC*3) / 7)

static UINT16 pwm_ctrl,pwm_cycle,pwm_tm_reg;
static UINT16 cur_lch[0x10],cur_rch[0x10];
static UINT16 pwm_cycle_reg; //used for latching
static UINT8 pwm_timer_tick;
static UINT8 lch_index_r,rch_index_r,lch_index_w,rch_index_w;
static UINT16 lch_fifo_state,rch_fifo_state;

emu_timer *_32x_pwm_timer;

static void calculate_pwm_timer(void)
{
	if(pwm_tm_reg == 0) { pwm_tm_reg = 16; } // zero gives max range
	if(pwm_cycle == 0) { pwm_cycle = 4095; } // zero gives max range

	/* if both RMD and LMD are set to OFF or pwm cycle register is one, then PWM timer ticks doesn't occur */
	if(pwm_cycle == 1 || ((pwm_ctrl & 0xf) == 0))
		_32x_pwm_timer->adjust(attotime::never);
	else
	{
		pwm_timer_tick = 0;
		lch_fifo_state = rch_fifo_state = 0x4000;
		lch_index_r = rch_index_r = 0;
		lch_index_w = rch_index_w = 0;
		_32x_pwm_timer->adjust(attotime::from_hz((PWM_CLOCK) / (pwm_cycle - 1)));
	}
}

TIMER_CALLBACK( _32x_pwm_callback )
{
	if(lch_index_r < PWM_FIFO_SIZE)
	{
		switch(pwm_ctrl & 3)
		{
			case 0: lch_index_r++; /*Speaker OFF*/ break;
			case 1: dac_signed_data_16_w(machine.device("lch_pwm"), cur_lch[lch_index_r++]); break;
			case 2: dac_signed_data_16_w(machine.device("rch_pwm"), cur_lch[lch_index_r++]); break;
			case 3: popmessage("Undefined PWM Lch value 3, contact MESSdev"); break;
		}

		lch_index_w = 0;
	}

	lch_fifo_state = (lch_index_r == PWM_FIFO_SIZE) ? 0x4000 : 0x0000;

	if(rch_index_r < PWM_FIFO_SIZE)
	{
		switch((pwm_ctrl & 0xc) >> 2)
		{
			case 0: rch_index_r++; /*Speaker OFF*/ break;
			case 1: dac_signed_data_16_w(machine.device("rch_pwm"), cur_rch[rch_index_r++]); break;
			case 2: dac_signed_data_16_w(machine.device("lch_pwm"), cur_rch[rch_index_r++]); break;
			case 3: popmessage("Undefined PWM Rch value 3, contact MESSdev"); break;
		}

		rch_index_w = 0;
	}

	rch_fifo_state = (rch_index_r == PWM_FIFO_SIZE) ? 0x4000 : 0x0000;

	pwm_timer_tick++;

	if(pwm_timer_tick == pwm_tm_reg)
	{
		pwm_timer_tick = 0;
		if(sh2_master_pwmint_enable) { device_set_input_line(_32x_master_cpu, SH2_PINT_IRQ_LEVEL,ASSERT_LINE); }
		if(sh2_slave_pwmint_enable) { device_set_input_line(_32x_slave_cpu, SH2_PINT_IRQ_LEVEL,ASSERT_LINE); }
	}

	_32x_pwm_timer->adjust(attotime::from_hz((PWM_CLOCK) / (pwm_cycle - 1)));
}

static READ16_HANDLER( _32x_pwm_r )
{
	switch(offset)
	{
		case 0x00/2: return pwm_ctrl; //control register
		case 0x02/2: return pwm_cycle_reg; // cycle register
		case 0x04/2: return lch_fifo_state; // l ch
		case 0x06/2: return rch_fifo_state; // r ch
		case 0x08/2: return lch_fifo_state & rch_fifo_state; // mono ch
	}

	printf("Read at undefined PWM register %02x\n",offset);
	return 0xffff;
}

static WRITE16_HANDLER( _32x_pwm_w )
{
	switch(offset)
	{
		case 0x00/2:
			pwm_ctrl = data & 0xffff;
			pwm_tm_reg = (pwm_ctrl & 0xf00) >> 8;
			calculate_pwm_timer();
			break;
		case 0x02/2:
			pwm_cycle = pwm_cycle_reg = data & 0xfff;
			calculate_pwm_timer();
			break;
		case 0x04/2:
			if(lch_index_w < PWM_FIFO_SIZE)
			{
				cur_lch[lch_index_w++] = ((data & 0xfff) << 4) | (data & 0xf);
				lch_index_r = 0;
			}

			lch_fifo_state = (lch_index_w == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;
			break;
		case 0x06/2:
			if(rch_index_w < PWM_FIFO_SIZE)
			{
				cur_rch[rch_index_w++] = ((data & 0xfff) << 4) | (data & 0xf);
				rch_index_r = 0;
			}

			rch_fifo_state = (rch_index_w == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;

			break;
		case 0x08/2:
			if(lch_index_w < PWM_FIFO_SIZE)
			{
				cur_lch[lch_index_w++] = ((data & 0xfff) << 4) | (data & 0xf);
				lch_index_r = 0;
			}

			if(rch_index_w < PWM_FIFO_SIZE)
			{
				cur_rch[rch_index_w++] = ((data & 0xfff) << 4) | (data & 0xf);
				rch_index_r = 0;
			}

			lch_fifo_state = (lch_index_w == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;
			rch_fifo_state = (rch_index_w == PWM_FIFO_SIZE) ? 0x8000 : 0x0000;

			break;
		default:
			printf("Write at undefined PWM register %02x %04x\n",offset,data);
			break;
	}
}

static WRITE16_HANDLER( _32x_68k_pwm_w )
{
	if(offset == 0/2)
		_32x_pwm_w(space,offset,(data & 0x7f) | (pwm_ctrl & 0xff80),mem_mask);
	else
		_32x_pwm_w(space,offset,data,mem_mask);
}

/**********************************************************************************************/
// 68k side a15180
// framebuffer control
// also accessed from the SH2 @ 4100
/**********************************************************************************************/

static UINT16 _32x_a1518a_reg;

static READ16_HANDLER( _32x_common_vdp_regs_r )
{
	// what happens if the z80 accesses it, what authorization do we use?



//  printf("_32x_68k_a15180_r (a15180) %04x\n",mem_mask);

	// read needs authorization too I think, undefined behavior otherwise
	switch (offset)
	{
		case 0x00:

		// the flag is inverted compared to the megadrive
		int ntsc;
		if (megadrive_region_pal) ntsc = 0;
		else ntsc = 1;

		return (ntsc << 15) |
			   (_32x_videopriority << 7 ) |
			   ( _32x_240mode << 6 ) |
			   ( _32x_displaymode << 0 );



		case 0x02/2:
			return _32x_screenshift;

		case 0x04/2:
			return _32x_autofill_length;

		case 0x06/2:
			return _32x_autofill_address;

		case 0x08/2:
			return _32x_autofill_data;

		case 0x0a/2:
			UINT16 retdata = _32x_a1518a_reg;
			UINT16 hpos = get_hposition();
			int megadrive_hblank_flag = 0;

			if (megadrive_vblank_flag) retdata |= 0x8000;

			if (hpos>400) megadrive_hblank_flag = 1;
			if (hpos>460) megadrive_hblank_flag = 0;

			if (megadrive_hblank_flag) retdata |= 0x4000;

			if (megadrive_vblank_flag) { retdata |= 2; } // framebuffer approval (TODO: condition is unknown at current time)

			if (megadrive_hblank_flag && megadrive_vblank_flag) { retdata |= 0x2000; } // palette approval (TODO: active high or low?)

			return retdata;
	}

	return 0x0000;
}


void _32x_check_framebuffer_swap(void)
{

	if(_32x_is_connected)
	{

		// this logic should be correct, but makes things worse?
		//if (genesis_scanline_counter >= megadrive_irq6_scanline)
		{
			_32x_a1518a_reg = _32x_fb_swap & 1;



			if (_32x_fb_swap & 1)
			{
				_32x_access_dram = _32x_dram0;
				_32x_display_dram = _32x_dram1;
			}
			else
			{
				_32x_display_dram = _32x_dram0;
				_32x_access_dram = _32x_dram1;
			}
		}
	}
}


static WRITE16_HANDLER( _32x_common_vdp_regs_w )
{
	// what happens if the z80 accesses it, what authorization do we use? which address space do we get?? the z80 *can* write here and to the framebuffer via the window

	address_space* _68kspace = space->machine().device("maincpu")->memory().space(AS_PROGRAM);

	if (space!= _68kspace)
	{
		if (_32x_access_auth!=1)
			return;
	}

	if (space== _68kspace)
	{
		if (_32x_access_auth!=0)
			return;
	}


	switch (offset)
	{

		case 0x00:
			//printf("_32x_68k_a15180_w (a15180) %04x %04x   source _32x_access_auth %04x\n",data,mem_mask, _32x_access_auth);

			if (ACCESSING_BITS_0_7)
			{
				_32x_videopriority = (data & 0x80) >> 7;
				_32x_240mode   = (data & 0x40) >> 6;
				_32x_displaymode   = (data & 0x03) >> 0;
			}
			break;

		case 0x02/2:
			if (ACCESSING_BITS_0_7)
			{
				_32x_screenshift = data & 1; // allows 1 pixel shifting
			}
			if (ACCESSING_BITS_8_15)
			{

			}
			break;

		case 0x04/2:
			if (ACCESSING_BITS_0_7)
			{
				_32x_autofill_length = data & 0xff;
			}

			if (ACCESSING_BITS_8_15)
			{

			}
			break;

		case 0x06/2:
			if (ACCESSING_BITS_0_7)
			{
				_32x_autofill_address = (_32x_autofill_address & 0xff00) | (data & 0x00ff);
			}

			if (ACCESSING_BITS_8_15)
			{
				_32x_autofill_address = (_32x_autofill_address & 0x00ff) | (data & 0xff00);
			}
			break;

		case 0x08/2:
			if (ACCESSING_BITS_0_7)
			{
				_32x_autofill_data = (_32x_autofill_data & 0xff00) | (data & 0x00ff);
			}

			if (ACCESSING_BITS_8_15)
			{
				_32x_autofill_data = (_32x_autofill_data & 0x00ff) | (data & 0xff00);
			}

			// do the fill - shouldn't be instant..
			{
				int i;
				for (i=0; i<_32x_autofill_length+1;i++)
				{
					_32x_access_dram[_32x_autofill_address] = _32x_autofill_data;
					_32x_autofill_address = (_32x_autofill_address & 0xff00) | ((_32x_autofill_address+1) & 0x00ff);
				}
			}
			break;

		case 0x0a/2:
			// bit 0 is the framebuffer select, change is delayed until vblank;
		//  _32x_a1518a_reg = (_32x_a1518a_reg & 0xfffe);
			if (ACCESSING_BITS_0_7)
			{
				_32x_fb_swap = data & 1;

				_32x_check_framebuffer_swap();
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
static READ16_HANDLER( _32x_sh2_master_4000_r )
{
	UINT16 retvalue = 0x0200;
	retvalue |= _32x_access_auth << 15;

	retvalue |=	sh2_hint_in_vbl;
	retvalue |= sh2_master_vint_enable;
	retvalue |= sh2_master_hint_enable;
	retvalue |= sh2_master_cmdint_enable;
	retvalue |= sh2_master_pwmint_enable;

	return retvalue;
}

static WRITE16_HANDLER( _32x_sh2_master_4000_w )
{
	if (ACCESSING_BITS_8_15)
	{
		_32x_access_auth = (data &0x8000) >> 15;
	}

	if (ACCESSING_BITS_0_7)
	{
		sh2_hint_in_vbl = data & 0x80;
		sh2_master_vint_enable = data & 0x8;
		sh2_master_hint_enable = data & 0x4;
		sh2_master_cmdint_enable = data & 0x2;
		sh2_master_pwmint_enable = data & 0x1;

		//if (sh2_master_hint_enable) printf("sh2_master_hint_enable enable!\n");
		//if (sh2_master_pwmint_enable) printf("sh2_master_pwn_enable enable!\n");

		_32x_check_irqs(space->machine());
	}
}

/* SLAVE */

static READ16_HANDLER( _32x_sh2_slave_4000_r )
{
	UINT16 retvalue = 0x0200;
	retvalue |= _32x_access_auth << 15;
	retvalue |=	sh2_hint_in_vbl;
	retvalue |= sh2_slave_vint_enable;
	retvalue |= sh2_slave_hint_enable;
	retvalue |= sh2_slave_cmdint_enable;
	retvalue |= sh2_slave_pwmint_enable;

	return retvalue;
}


static WRITE16_HANDLER( _32x_sh2_slave_4000_w )
{
	if (ACCESSING_BITS_8_15)
	{
		_32x_access_auth = (data &0x8000) >> 15;
	}

	if (ACCESSING_BITS_0_7)
	{
		sh2_hint_in_vbl = data & 0x80;
		sh2_slave_vint_enable = data & 0x8;
		sh2_slave_hint_enable = data & 0x4;
		sh2_slave_cmdint_enable = data & 0x2;
		sh2_slave_pwmint_enable = data & 0x1;

		//if (sh2_slave_hint_enable) printf("sh2_slave_hint_enable enable!\n");
		//if (sh2_slave_pwmint_enable) printf("sh2_slave_pwm_enable enable!\n");

		_32x_check_irqs(space->machine());

	}
}

/**********************************************************************************************/
// SH2 side 4002
// Reserved  ( Stand By Change Register )
// Shouldn't be used
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4002_r )
{
	printf("reading 4002!\n");
	return 0x0000;
}

static WRITE16_HANDLER( _32x_sh2_common_4002_w )
{
	printf("write 4002!\n");
}


/**********************************************************************************************/
// SH2 side 4004
// H Count Register (H Interrupt)
// 0 = every line
/**********************************************************************************************/
static READ16_HANDLER( _32x_sh2_common_4004_r )
{
	return _32x_hcount_reg;
}

static WRITE16_HANDLER( _32x_sh2_common_4004_w )
{
	_32x_hcount_reg = data & 0xff;
}


/**********************************************************************************************/
// SH2 side 4006
// DReq Control Register
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_common_4006_r )
{
	//printf("DREQ read!\n"); // tempo reads it, shut up for now
	return _32x_68k_a15106_r(space,offset,mem_mask);
}

static WRITE16_HANDLER( _32x_sh2_common_4006_w )
{
	printf("DREQ write!\n"); //register is read only on SH-2 side
}


/**********************************************************************************************/
// SH2 side 4014
// VRES (md reset button interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_4014_w ){ device_set_input_line(_32x_master_cpu,SH2_VRES_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_4014_w ) { device_set_input_line(_32x_slave_cpu, SH2_VRES_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 4016
// VINT (vertical interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_4016_w ){ sh2_master_vint_pending = 0; _32x_check_irqs(space->machine()); }
static WRITE16_HANDLER( _32x_sh2_slave_4016_w ) { sh2_slave_vint_pending = 0; _32x_check_irqs(space->machine()); }

/**********************************************************************************************/
// SH2 side 4018
// HINT (horizontal interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_4018_w ){ device_set_input_line(_32x_master_cpu,SH2_HINT_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_4018_w ) { device_set_input_line(_32x_slave_cpu, SH2_HINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401A
// HINT (control register interrupt) clear
// Note: flag cleared here is a guess, according to After Burner behaviour
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_401a_w ){ _32x_68k_a15102_reg &= ~1; device_set_input_line(_32x_master_cpu,SH2_CINT_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_401a_w ) { _32x_68k_a15102_reg &= ~2; device_set_input_line(_32x_slave_cpu, SH2_CINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401C
// PINT (PWM timer interrupt) clear
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_401c_w ){ device_set_input_line(_32x_master_cpu,SH2_PINT_IRQ_LEVEL,CLEAR_LINE);}
static WRITE16_HANDLER( _32x_sh2_slave_401c_w ) { device_set_input_line(_32x_slave_cpu, SH2_PINT_IRQ_LEVEL,CLEAR_LINE);}

/**********************************************************************************************/
// SH2 side 401E
// ?? unknown / unused
/**********************************************************************************************/

static WRITE16_HANDLER( _32x_sh2_master_401e_w )
{
	printf("_32x_sh2_master_401e_w\n");
}

static WRITE16_HANDLER( _32x_sh2_slave_401e_w )
{
	printf("_32x_sh2_slave_401e_w\n");
}

/**********************************************************************************************/
// SH2 side 4020 - 402f
// SH2 -> 68k Comms ports,
// access at a15120 - a1512f on 68k
// these just map through to the 68k functions
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_commsram16_r ) { return _32x_68k_commsram_r(space, offset, mem_mask); }
static WRITE16_HANDLER( _32x_sh2_commsram16_w ) { _32x_68k_commsram_w(space, offset, data, mem_mask); }

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

static READ16_HANDLER( _32x_sh2_paletteram16_r ) { return _32x_68k_palette_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_paletteram16_w ) { _32x_68k_palette_w(space,offset,data,mem_mask); }

/**********************************************************************************************/
// SH2 side 4000000 - 401ffff
// framebuffer
// maps through to 68k at 840000 - 85ffff
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_framebuffer_dram16_r ) { return _32x_68k_dram_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_framebuffer_dram16_w ) { _32x_68k_dram_w(space,offset,data,mem_mask); }

/**********************************************************************************************/
// SH2 side 4020000 - 403ffff
// framebuffer overwrite
// maps through to 68k at 860000 - 87ffff
/**********************************************************************************************/

static READ16_HANDLER( _32x_sh2_framebuffer_overwrite_dram16_r ) { return _32x_68k_dram_overwrite_r(space,offset,mem_mask); }
static WRITE16_HANDLER( _32x_sh2_framebuffer_overwrite_dram16_w ) { _32x_68k_dram_overwrite_w(space,offset,data,mem_mask); }



/**********************************************************************************************/
// SH2 access Macros
/**********************************************************************************************/


/* the 32x treats everything as 16-bit registers, so we remap the 32-bit read & writes
   to 2x 16-bit handlers here (TODO: nuke this eventually) */

#define _32X_MAP_READHANDLERS(NAMEA,NAMEB)                                          \
static READ32_HANDLER( _32x_sh2_##NAMEA##_##NAMEB##_r )                             \
{                                                                                   \
	UINT32 retvalue = 0x00000000;                                                   \
	if (ACCESSING_BITS_16_31)                                                       \
	{                                                                               \
		UINT16 ret = _32x_sh2_##NAMEA##_r(space,0,(mem_mask>>16)&0xffff);         \
		retvalue |= ret << 16;                                                      \
	}                                                                               \
	if (ACCESSING_BITS_0_15)                                                        \
	{                                                                               \
		UINT16 ret = _32x_sh2_##NAMEB##_r(space,0,(mem_mask>>0)&0xffff);          \
		retvalue |= ret << 0;                                                       \
	}                                                                               \
                                                                                    \
	return retvalue;                                                                \
}                                                                                   \

#define _32X_MAP_WRITEHANDLERS(NAMEA,NAMEB)                                             \
static WRITE32_HANDLER( _32x_sh2_##NAMEA##_##NAMEB##_w)                                 \
{                                                                                       \
	if (ACCESSING_BITS_16_31)                                                           \
	{                                                                                   \
		_32x_sh2_##NAMEA##_w(space,0,(data>>16)&0xffff,(mem_mask>>16)&0xffff);        \
	}                                                                                   \
	if (ACCESSING_BITS_0_15)                                                            \
	{                                                                                   \
		_32x_sh2_##NAMEB##_w(space,0,(data>>0)&0xffff,(mem_mask>>0)&0xffff);          \
	}                                                                                   \
}                                                                                       \

/* for RAM ranges, eg. Framebuffer, Comms RAM etc. */

#define _32X_MAP_RAM_READHANDLERS(NAMEA)                                            \
static READ32_HANDLER( _32x_sh2_##NAMEA##_r )                                       \
{                                                                                   \
	UINT32 retvalue = 0x00000000;                                                   \
	if (ACCESSING_BITS_16_31)                                                       \
	{                                                                               \
		UINT16 ret = _32x_sh2_##NAMEA##16_r(space,offset*2,(mem_mask>>16)&0xffff);  \
		retvalue |= ret << 16;                                                      \
	}                                                                               \
	if (ACCESSING_BITS_0_15)                                                        \
	{                                                                               \
		UINT16 ret = _32x_sh2_##NAMEA##16_r(space,offset*2+1,(mem_mask>>0)&0xffff); \
		retvalue |= ret << 0;                                                       \
	}                                                                               \
                                                                                    \
	return retvalue;                                                                \
}                                                                                   \

#define _32X_MAP_RAM_WRITEHANDLERS(NAMEA)                                               \
static WRITE32_HANDLER( _32x_sh2_##NAMEA##_w)                                           \
{                                                                                       \
	if (ACCESSING_BITS_16_31)                                                           \
	{                                                                                   \
		_32x_sh2_##NAMEA##16_w(space,offset*2,(data>>16)&0xffff,(mem_mask>>16)&0xffff); \
	}                                                                                   \
	if (ACCESSING_BITS_0_15)                                                            \
	{                                                                                   \
		_32x_sh2_##NAMEA##16_w(space,offset*2+1,(data>>0)&0xffff,(mem_mask>>0)&0xffff); \
	}                                                                                   \
}                                                                                       \



/**********************************************************************************************/
// SH2 access for Memory Map
/**********************************************************************************************/


_32X_MAP_READHANDLERS(master_4000,common_4002)  // _32x_sh2_master_4000_common_4002_r
_32X_MAP_WRITEHANDLERS(master_4000,common_4002) // _32x_sh2_master_4000_common_4002_w

_32X_MAP_READHANDLERS(slave_4000,common_4002)  // _32x_sh2_slave_4000_common_4002_r
_32X_MAP_WRITEHANDLERS(slave_4000,common_4002) // _32x_sh2_slave_4000_common_4002_w

_32X_MAP_READHANDLERS(common_4004,common_4006)
_32X_MAP_WRITEHANDLERS(common_4004,common_4006)

_32X_MAP_WRITEHANDLERS(master_4014,master_4016) // _32x_sh2_master_4014_master_4016_w
_32X_MAP_WRITEHANDLERS(master_4018,master_401a) // _32x_sh2_master_4018_master_401a_w
_32X_MAP_WRITEHANDLERS(master_401c,master_401e) // _32x_sh2_master_401c_master_401e_w

_32X_MAP_WRITEHANDLERS(slave_4014,slave_4016) // _32x_sh2_slave_4014_slave_4016_w
_32X_MAP_WRITEHANDLERS(slave_4018,slave_401a) // _32x_sh2_slave_4018_slave_401a_w
_32X_MAP_WRITEHANDLERS(slave_401c,slave_401e) // _32x_sh2_slave_401c_slave_401e_w

_32X_MAP_RAM_READHANDLERS(commsram) // _32x_sh2_commsram_r
_32X_MAP_RAM_WRITEHANDLERS(commsram) // _32x_sh2_commsram_w

_32X_MAP_RAM_READHANDLERS(framebuffer_dram) // _32x_sh2_framebuffer_dram_r
_32X_MAP_RAM_WRITEHANDLERS(framebuffer_dram) // _32x_sh2_framebuffer_dram_w

_32X_MAP_RAM_READHANDLERS(framebuffer_overwrite_dram) // _32x_sh2_framebuffer_overwrite_dram_r
_32X_MAP_RAM_WRITEHANDLERS(framebuffer_overwrite_dram) // _32x_sh2_framebuffer_overwrite_dram_w

_32X_MAP_RAM_READHANDLERS(paletteram) // _32x_sh2_paletteram_r
_32X_MAP_RAM_WRITEHANDLERS(paletteram) // _32x_sh2_paletteram_w


/**********************************************************************************************/
// SH2 memory maps
/**********************************************************************************************/

ADDRESS_MAP_START( sh2_main_map, AS_PROGRAM, 32, driver_device )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM

	AM_RANGE(0x00004000, 0x00004003) AM_READWRITE_LEGACY(_32x_sh2_master_4000_common_4002_r, _32x_sh2_master_4000_common_4002_w )
	AM_RANGE(0x00004004, 0x00004007) AM_READWRITE_LEGACY(_32x_sh2_common_4004_common_4006_r, _32x_sh2_common_4004_common_4006_w)

	AM_RANGE(0x00004008, 0x00004013) AM_READWRITE16_LEGACY(_32x_dreq_common_r, _32x_dreq_common_w, 0xffffffff )

	AM_RANGE(0x00004014, 0x00004017) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_master_4014_master_4016_w ) // IRQ clear
	AM_RANGE(0x00004018, 0x0000401b) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_master_4018_master_401a_w ) // IRQ clear
	AM_RANGE(0x0000401c, 0x0000401f) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_master_401c_master_401e_w ) // IRQ clear

	AM_RANGE(0x00004020, 0x0000402f) AM_READWRITE_LEGACY(_32x_sh2_commsram_r, _32x_sh2_commsram_w )
	AM_RANGE(0x00004030, 0x0000403f) AM_READWRITE16_LEGACY(_32x_pwm_r, _32x_pwm_w, 0xffffffff )

	AM_RANGE(0x00004100, 0x0000410b) AM_READWRITE16_LEGACY(_32x_common_vdp_regs_r, _32x_common_vdp_regs_w , 0xffffffff)
	AM_RANGE(0x00004200, 0x000043ff) AM_READWRITE_LEGACY(_32x_sh2_paletteram_r, _32x_sh2_paletteram_w)

	AM_RANGE(0x04000000, 0x0401ffff) AM_READWRITE_LEGACY(_32x_sh2_framebuffer_dram_r, _32x_sh2_framebuffer_dram_w)
	AM_RANGE(0x04020000, 0x0403ffff) AM_READWRITE_LEGACY(_32x_sh2_framebuffer_overwrite_dram_r, _32x_sh2_framebuffer_overwrite_dram_w)

	AM_RANGE(0x06000000, 0x0603ffff) AM_RAM AM_SHARE("share10")
	AM_RANGE(0x02000000, 0x023fffff) AM_ROM AM_REGION("gamecart_sh2", 0) // program is writeable (wwfraw)

	AM_RANGE(0x22000000, 0x223fffff) AM_ROM AM_REGION("gamecart_sh2", 0) // cart mirror (fifa96)

	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START( sh2_slave_map, AS_PROGRAM, 32, driver_device )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM

	AM_RANGE(0x00004000, 0x00004003) AM_READWRITE_LEGACY(_32x_sh2_slave_4000_common_4002_r, _32x_sh2_slave_4000_common_4002_w )
	AM_RANGE(0x00004004, 0x00004007) AM_READWRITE_LEGACY(_32x_sh2_common_4004_common_4006_r, _32x_sh2_common_4004_common_4006_w)

	AM_RANGE(0x00004008, 0x00004013) AM_READWRITE16_LEGACY(_32x_dreq_common_r, _32x_dreq_common_w, 0xffffffff )

	AM_RANGE(0x00004014, 0x00004017) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_slave_4014_slave_4016_w ) // IRQ clear
	AM_RANGE(0x00004018, 0x0000401b) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_slave_4018_slave_401a_w ) // IRQ clear
	AM_RANGE(0x0000401c, 0x0000401f) AM_READNOP AM_WRITE_LEGACY(_32x_sh2_slave_401c_slave_401e_w ) // IRQ clear

	AM_RANGE(0x00004020, 0x0000402f) AM_READWRITE_LEGACY(_32x_sh2_commsram_r, _32x_sh2_commsram_w )
	AM_RANGE(0x00004030, 0x0000403f) AM_READWRITE16_LEGACY(_32x_pwm_r, _32x_pwm_w, 0xffffffff )

	AM_RANGE(0x00004100, 0x0000410b) AM_READWRITE16_LEGACY(_32x_common_vdp_regs_r, _32x_common_vdp_regs_w , 0xffffffff)
	AM_RANGE(0x00004200, 0x000043ff) AM_READWRITE_LEGACY(_32x_sh2_paletteram_r, _32x_sh2_paletteram_w)

	AM_RANGE(0x04000000, 0x0401ffff) AM_READWRITE_LEGACY(_32x_sh2_framebuffer_dram_r, _32x_sh2_framebuffer_dram_w)
	AM_RANGE(0x04020000, 0x0403ffff) AM_READWRITE_LEGACY(_32x_sh2_framebuffer_overwrite_dram_r, _32x_sh2_framebuffer_overwrite_dram_w)

	AM_RANGE(0x06000000, 0x0603ffff) AM_RAM AM_SHARE("share10")
	AM_RANGE(0x02000000, 0x023fffff) AM_ROM AM_REGION("gamecart_sh2", 0) // program is writeable (wwfraw)

	AM_RANGE(0x22000000, 0x223fffff) AM_ROM AM_REGION("gamecart_sh2", 0) // cart mirror (fifa96)

	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM
ADDRESS_MAP_END

/****************************************** END 32X related *************************************/



DRIVER_INIT( _32x )
{
	_32x_dram0 = auto_alloc_array(machine, UINT16, 0x40000/2);
	_32x_dram1 = auto_alloc_array(machine, UINT16, 0x40000/2);

	memset(_32x_dram0, 0x00, 0x40000);
	memset(_32x_dram1, 0x00, 0x40000);

	_32x_palette_lookup = auto_alloc_array(machine, UINT16, 0x200/2);
	_32x_palette = auto_alloc_array(machine, UINT16, 0x200/2);

	memset(_32x_palette_lookup, 0x00, 0x200);
	memset(_32x_palette, 0x00, 0x200);


	_32x_display_dram = _32x_dram0;
	_32x_access_dram = _32x_dram1;

	_32x_adapter_enabled = 0;

	if (_32x_adapter_enabled == 0)
	{
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_rom(0x0000000, 0x03fffff, machine.root_device().memregion("gamecart")->base());
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x000070, 0x000073, FUNC(_32x_68k_hint_vector_r), FUNC(_32x_68k_hint_vector_w)); // h interrupt vector
	};


	a15100_reg = 0x0000;
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15100, 0xa15101, FUNC(_32x_68k_a15100_r), FUNC(_32x_68k_a15100_w)); // framebuffer control regs
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15102, 0xa15103, FUNC(_32x_68k_a15102_r), FUNC(_32x_68k_a15102_w)); // send irq to sh2
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15104, 0xa15105, FUNC(_32x_68k_a15104_r), FUNC(_32x_68k_a15104_w)); // 68k BANK rom set
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15106, 0xa15107, FUNC(_32x_68k_a15106_r), FUNC(_32x_68k_a15106_w)); // dreq stuff
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15108, 0xa15113, FUNC(_32x_dreq_common_r), FUNC(_32x_dreq_common_w)); // dreq src / dst / length /fifo

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa1511a, 0xa1511b, FUNC(_32x_68k_a1511a_r), FUNC(_32x_68k_a1511a_w)); // SEGA TV

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15120, 0xa1512f, FUNC(_32x_68k_commsram_r), FUNC(_32x_68k_commsram_w)); // comms reg 0-7
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xa15130, 0xa1513f, FUNC(_32x_pwm_r), FUNC(_32x_68k_pwm_w));

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x0a130ec, 0x0a130ef, FUNC(_32x_68k_MARS_r)); // system ID


	/* Interrupts are masked / disabled at first */
	sh2_master_vint_enable = sh2_slave_vint_enable = 0;
	sh2_master_hint_enable = sh2_slave_hint_enable = 0;
	sh2_master_cmdint_enable = sh2_slave_cmdint_enable = 0;
	sh2_master_pwmint_enable = sh2_slave_pwmint_enable = 0;
	sh2_master_vint_pending = sh2_slave_vint_pending = 0;

	// start in a reset state
	sh2_are_running = 0;

	_32x_a1518a_reg = 0x00; // inital value
	_32x_68k_a15104_reg = 0x00;

	_32x_autofill_length = 0;
	_32x_autofill_address = 0;
	_32x_autofill_data = 0;
	_32x_screenshift = 0;
	_32x_videopriority = 0; // MD priority
	_32x_displaymode = 0;
	_32x_240mode = 0;

// checking if these help brutal, they don't.
	sh2drc_set_options(machine.device("32x_master_sh2"), SH2DRC_COMPATIBLE_OPTIONS);
	sh2drc_set_options(machine.device("32x_slave_sh2"), SH2DRC_COMPATIBLE_OPTIONS);

	DRIVER_INIT_CALL(megadriv);
}

void _32x_check_irqs(running_machine& machine)
{

	if (sh2_master_vint_enable && sh2_master_vint_pending) device_set_input_line(_32x_master_cpu,SH2_VINT_IRQ_LEVEL,ASSERT_LINE);
	else device_set_input_line(_32x_master_cpu,SH2_VINT_IRQ_LEVEL,CLEAR_LINE);

	if (sh2_slave_vint_enable && sh2_slave_vint_pending) device_set_input_line(_32x_slave_cpu,SH2_VINT_IRQ_LEVEL,ASSERT_LINE);
	else device_set_input_line(_32x_slave_cpu,SH2_VINT_IRQ_LEVEL,CLEAR_LINE);
}

void _32x_scanline_cb0(running_machine& machine)
{
	sh2_master_vint_pending = 1;
	sh2_slave_vint_pending = 1;
	_32x_check_irqs(machine);
}


void _32x_scanline_cb1()
{
	_32x_hcount_compare_val++;

	if(_32x_hcount_compare_val >= _32x_hcount_reg)
	{
		_32x_hcount_compare_val = -1;

		if(genesis_scanline_counter < 224 || sh2_hint_in_vbl)
		{
			if(sh2_master_hint_enable) { device_set_input_line(_32x_master_cpu,SH2_HINT_IRQ_LEVEL,ASSERT_LINE); }
			if(sh2_slave_hint_enable) { device_set_input_line(_32x_slave_cpu,SH2_HINT_IRQ_LEVEL,ASSERT_LINE); }
		}
	}
}



int _32x_fifo_available_callback(device_t *device, UINT32 src, UINT32 dst, UINT32 data, int size)
{
	if (src==0x4012)
	{
		if (current_fifo_readblock==fifo_block_a && fifo_block_a_full)
			return 1;

		if (current_fifo_readblock==fifo_block_b && fifo_block_b_full)
			return 1;

		return 0;
	}

	return 1;
}

MACHINE_RESET( _32x )
{
	current_fifo_block = fifo_block_a;
	current_fifo_readblock = fifo_block_b;
	current_fifo_write_pos = 0;
	current_fifo_read_pos = 0;
	fifo_block_a_full = 0;
	fifo_block_b_full = 0;

	_32x_hcount_compare_val = -1;
}

static UINT32 _32x_linerender[320+258]; // tmp buffer (bigger than it needs to be to simplify RLE decode)

UINT32* _32x_render_videobuffer_to_screenbuffer_helper(running_machine &machine, int scanline)
{
	int x;

	/* render 32x output to a buffer */
	if (_32x_is_connected && (_32x_displaymode != 0))
	{
		if (_32x_displaymode==1)
		{

			UINT32 lineoffs;
			int start;

			lineoffs = _32x_display_dram[scanline];

			if (_32x_screenshift == 0) start=0;
			else start = -1;

			for (x=start;x<320;x++)
			{
				UINT16 coldata;
				coldata = _32x_display_dram[lineoffs];

				{
					if (x>=0)
					{
						_32x_linerender[x] = _32x_palette_lookup[(coldata & 0xff00)>>8];
					}

					x++;

					if (x>=0)
					{
						_32x_linerender[x] = _32x_palette_lookup[(coldata & 0x00ff)];
					}
				}

				lineoffs++;

			}
		}
		else if (_32x_displaymode==3) // mode 3 = RLE  (used by BRUTAL intro)
		{
			UINT32 lineoffs;
			int start;

			lineoffs = _32x_display_dram[scanline];

			if (_32x_screenshift == 0) start=0;
			else start = -1;

            x = start;
			while (x<320)
			{
				UINT16 coldata, length, l;
				coldata = _32x_display_dram[lineoffs];
				length = ((coldata & 0xff00)>>8)+1;
				coldata = (coldata & 0x00ff)>>0;
				for (l=0;l<length;l++)
				{
					if (x>=0)
					{
						_32x_linerender[x] = _32x_palette_lookup[(coldata)];
					}
					x++;
				}

				lineoffs++;

			}
		}
		else // MODE 2 - 15bpp mode, not used by any commercial games?
		{
			UINT32 lineoffs;
			int start;

			lineoffs = _32x_display_dram[scanline];

			if (_32x_screenshift == 0) start=0;
			else start = -1;

            x = start;
			while (x<320)
			{
				UINT16 coldata;
				coldata = _32x_display_dram[lineoffs&0xffff];

				// need to swap red and blue around for MAME
				{
					int r = ((coldata >> 0)  & 0x1f);
					int g = ((coldata >> 5)  & 0x1f);
					int b = ((coldata >> 10) & 0x1f);
					int p = ((coldata >> 15) & 0x01); // priority 'through' bit

					coldata = (r << 10) | (g << 5) | (b << 0) | (p << 15);

				}

				if (x>=0)
					_32x_linerender[x] = coldata;

				x++;
				lineoffs++;
			}
		}
	}

	return _32x_linerender;
}
