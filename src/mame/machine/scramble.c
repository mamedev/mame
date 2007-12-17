/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "includes/galaxian.h"

static UINT8 moonwar_port_select;

static UINT8 cavelon_bank;

static UINT8 security_2B_counter;

MACHINE_RESET( scramble )
{
	machine_reset_galaxian(machine);

	if (cpu_gettotalcpu() > 1)
	{
		scramble_sh_init();
	}

  security_2B_counter = 0;
}

MACHINE_RESET( sfx )
{
	machine_reset_scramble(machine);

	sfx_sh_init();
}

int monsterz_count = 0;
MACHINE_RESET( monsterz )
{
/*
// patch rom crc
    UINT8 *ROM = memory_region(REGION_CPU1);
    ROM[0x363f] = 0;
    ROM[0x3640] = 0;
    ROM[0x3641] = 0;

    ROM[0xc5bc] = 0xaf;
*/
	machine_reset_scramble(machine);

	sfx_sh_init();

	monsterz_count = 0;
}

MACHINE_RESET( explorer )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	RAM[0x47ff] = 0; /* If not set, it doesn't reset after the 1st time */

	machine_reset_galaxian(machine);
}

static READ8_HANDLER( scrambls_input_port_2_r )
{
  static UINT8 mask[] = { 0x20, 0x20, 0x80, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0 };

	UINT8 res;

	res = readinputport(2);

/*logerror("%04x: read IN2\n",activecpu_get_pc());*/

  /*
    p_security_2B : process(security_count)
    begin
      -- I am not sure what this chip does yet, but this gets us past the initial check for now.
      case security_count is
        when "000" => net_1e10_i <= '0'; net_1e12_i <= '1';
        when "001" => net_1e10_i <= '0'; net_1e12_i <= '1';
        when "010" => net_1e10_i <= '1'; net_1e12_i <= '0';
        when "011" => net_1e10_i <= '1'; net_1e12_i <= '1';
        when "100" => net_1e10_i <= '1'; net_1e12_i <= '1';
        when "101" => net_1e10_i <= '1'; net_1e12_i <= '1';
        when "110" => net_1e10_i <= '1'; net_1e12_i <= '1';
        when "111" => net_1e10_i <= '1'; net_1e12_i <= '1';
        when others => null;
      end case;
    end process;
  */
  res = (res & ~((1<<7)|(1<<5))) | mask[security_2B_counter];
  security_2B_counter = (security_2B_counter + 1) & 0x07;

	return res;
}

static READ8_HANDLER( ckongs_input_port_1_r )
{
	return (readinputport(1) & 0xfc) | ((readinputport(2) & 0x06) >> 1);
}

static READ8_HANDLER( ckongs_input_port_2_r )
{
	return (readinputport(2) & 0xf9) | ((readinputport(1) & 0x03) << 1);
}


static WRITE8_HANDLER( moonwar_port_select_w )
{
	moonwar_port_select = data & 0x10;
}

static READ8_HANDLER( moonwar_input_port_0_r )
{
	UINT8 sign;
	UINT8 delta;

	delta = (moonwar_port_select ? readinputport(3) : readinputport(4));

	sign = (delta & 0x80) >> 3;
	delta &= 0x0f;

	return ((readinputport(0) & 0xe0) | delta | sign );
}


/* the coinage DIPs are spread accross two input ports */
static READ8_HANDLER( stratgyx_input_port_2_r )
{
	return (readinputport(2) & ~0x06) | ((readinputport(4) << 1) & 0x06);
}

static READ8_HANDLER( stratgyx_input_port_3_r )
{
	return (readinputport(3) & ~0x03) | ((readinputport(4) >> 2) & 0x03);
}


static READ8_HANDLER( darkplnt_input_port_1_r )
{
	static const UINT8 remap[] = {0x03, 0x02, 0x00, 0x01, 0x21, 0x20, 0x22, 0x23,
							  0x33, 0x32, 0x30, 0x31, 0x11, 0x10, 0x12, 0x13,
							  0x17, 0x16, 0x14, 0x15, 0x35, 0x34, 0x36, 0x37,
							  0x3f, 0x3e, 0x3c, 0x3d, 0x1d, 0x1c, 0x1e, 0x1f,
							  0x1b, 0x1a, 0x18, 0x19, 0x39, 0x38, 0x3a, 0x3b,
							  0x2b, 0x2a, 0x28, 0x29, 0x09, 0x08, 0x0a, 0x0b,
							  0x0f, 0x0e, 0x0c, 0x0d, 0x2d, 0x2c, 0x2e, 0x2f,
							  0x27, 0x26, 0x24, 0x25, 0x05, 0x04, 0x06, 0x07 };
	UINT8 val;

	val = readinputport(1);

	return ((val & 0x03) | (remap[val >> 2] << 2));
}

/* state of the security PAL (6J) */
static UINT8 xb;

static WRITE8_HANDLER( scramble_protection_w )
{
	xb = data;
}

static READ8_HANDLER( scramble_protection_r )
{
	switch (activecpu_get_pc())
	{
	case 0x00a8: return 0xf0;
	case 0x00be: return 0xb0;
	case 0x0c1d: return 0xf0;
	case 0x0c6a: return 0xb0;
	case 0x0ceb: return 0x40;
	case 0x0d37: return 0x60;
	case 0x1ca2: return 0x00;  /* I don't think it's checked */
	case 0x1d7e: return 0xb0;
	default:
		logerror("%04x: read protection\n",activecpu_get_pc());
		return 0;
	}
}

static READ8_HANDLER( scrambls_protection_r )
{
	/*logerror("%04x: read protection\n",activecpu_get_pc());*/

  /*
    p_security_6J : process(xb)
    begin
      -- chip K10A PAL16L8
      -- equations from Mark @ http://www.leopardcats.com/
      xbo(3 downto 0) <= xb(3 downto 0);
      xbo(4) <= not(xb(0) or xb(1) or xb(2) or xb(3));
      xbo(5) <= not((not xb(2) and not xb(0)) or (not xb(2) and not xb(1)) or (not xb(3) and not xb(0)) or (not xb(3) and not xb(1)));

      xbo(6) <= not(not xb(0) and not xb(3));
      xbo(7) <= not((not xb(1)) or xb(2));
    end process;
  */
  UINT8 xbo = xb & 0x0f;

  xbo |= ( ~(xb | (xb>>1) | (xb>>2) | (xb>>3)) & 0x01) << 4;
  xbo |= ( ~( (~(xb>>2)&~xb) | (~(xb>>2)&~(xb>>1)) | (~(xb>>3)&~xb) | (~(xb>>3)&~(xb>>1)) ) & 0x01) << 5;
  xbo |= ( ~(~xb&~(xb>>3)) & 0x01) << 6;
  xbo |= ( ~(~(xb>>1)|(xb>>2)) & 0x01) << 7;

  return (xbo);
}


static WRITE8_HANDLER( theend_coin_counter_w )
{
	coin_counter_w(0, data & 0x80);
}


static READ8_HANDLER( mariner_protection_1_r )
{
	return 7;
}

static READ8_HANDLER( mariner_protection_2_r )
{
	return 3;
}


READ8_HANDLER( triplep_pip_r )
{
	logerror("PC %04x: triplep read port 2\n",activecpu_get_pc());
	if (activecpu_get_pc() == 0x015a) return 0xff;
	else if (activecpu_get_pc() == 0x0886) return 0x05;
	else return 0;
}

READ8_HANDLER( triplep_pap_r )
{
	logerror("PC %04x: triplep read port 3\n",activecpu_get_pc());
	if (activecpu_get_pc() == 0x015d) return 0x04;
	else return 0;
}



static void cavelon_banksw(void)
{
	/* any read/write access in the 0x8000-0xffff region causes a bank switch.
       Only the lower 0x2000 is switched but we switch the whole region
       to keep the CPU core happy at the boundaries */

	cavelon_bank = !cavelon_bank;
	memory_set_bank(1, cavelon_bank);
}

static READ8_HANDLER( cavelon_banksw_r )
{
	cavelon_banksw();

	if      ((offset >= 0x0100) && (offset <= 0x0103))
		return ppi8255_0_r(offset - 0x0100);
	else if ((offset >= 0x0200) && (offset <= 0x0203))
		return ppi8255_1_r(offset - 0x0200);

	return 0xff;
}

static WRITE8_HANDLER( cavelon_banksw_w )
{
	cavelon_banksw();

	if      ((offset >= 0x0100) && (offset <= 0x0103))
		ppi8255_0_w(offset - 0x0100, data);
	else if ((offset >= 0x0200) && (offset <= 0x0203))
		ppi8255_1_w(offset - 0x0200, data);
}


READ8_HANDLER( hunchbks_mirror_r )
{
	return program_read_byte(0x1000+offset);
}

WRITE8_HANDLER( hunchbks_mirror_w )
{
	program_write_byte(0x1000+offset,data);
}


static ppi8255_interface ppi8255_intf =
{
	2, 								/* 2 chips */
	{input_port_0_r, 0},			/* Port A read */
	{input_port_1_r, 0},			/* Port B read */
	{input_port_2_r, 0},			/* Port C read */
	{0, soundlatch_w},				/* Port A write */
	{0, scramble_sh_irqtrigger_w},	/* Port B write */
	{0, 0}, 						/* Port C write */
};

/* extra chip for sample latch */
static ppi8255_interface sfx_ppi8255_intf =
{
	3, 									/* 3 chips */
	{input_port_0_r, 0, soundlatch2_r},	/* Port A read */
	{input_port_1_r, 0, 0},				/* Port B read */
	{input_port_2_r, 0, 0},				/* Port C read */
	{0, soundlatch_w, 0},				/* Port A write */
	{0, scramble_sh_irqtrigger_w, 0},	/* Port B write */
	{0, 0, 0}, 							/* Port C write */
};

/* extra chip for sample latch */
static ppi8255_interface monsterz_ppi8255_intf =
{
	3, 									/* 3 chips */
	{input_port_0_r, 0, 0},	/* Port A read */
	{input_port_1_r, 0, 0},				/* Port B read */
	{input_port_2_r, 0, 0},				/* Port C read */
	{0, soundlatch_w, 0},				/* Port A write */
	{0, scramble_sh_irqtrigger_w, 0},	/* Port B write */
	{0, 0, 0}, 							/* Port C write */
};


DRIVER_INIT( scramble_ppi )
{
	ppi8255_init(&ppi8255_intf);
}

DRIVER_INIT( scobra )
{
	driver_init_scramble_ppi(machine);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa803, 0xa803, 0, 0, scramble_background_enable_w);
}

DRIVER_INIT( atlantis )
{
	driver_init_scramble_ppi(machine);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6803, 0x6803, 0, 0, scramble_background_enable_w);
}

DRIVER_INIT( scramble )
{
	driver_init_atlantis(machine);

	ppi8255_set_portCread (1, scramble_protection_r);
	ppi8255_set_portCwrite(1, scramble_protection_w);
}

DRIVER_INIT( scrambls )
{
	driver_init_atlantis(machine);

	ppi8255_set_portCread(0, scrambls_input_port_2_r);
	ppi8255_set_portCread(1, scrambls_protection_r);
	ppi8255_set_portCwrite(1, scramble_protection_w);
}

DRIVER_INIT( theend )
{
	driver_init_scramble_ppi(machine);

	ppi8255_set_portCwrite(0, theend_coin_counter_w);
}

DRIVER_INIT( stratgyx )
{
	driver_init_scramble_ppi(machine);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xb000, 0xb000, 0, 0, scramble_background_green_w);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xb002, 0xb002, 0, 0, scramble_background_blue_w);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xb00a, 0xb00a, 0, 0, scramble_background_red_w);

	ppi8255_set_portCread(0, stratgyx_input_port_2_r);
	ppi8255_set_portCread(1, stratgyx_input_port_3_r);
}

DRIVER_INIT( tazmani2 )
{
	driver_init_scramble_ppi(machine);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xb002, 0xb002, 0, 0, scramble_background_enable_w);
}

DRIVER_INIT( amidar )
{
	driver_init_scramble_ppi(machine);

	/* Amidar has a the DIP switches connected to port C of the 2nd 8255 */
	ppi8255_set_portCread(1, input_port_3_r);
}

DRIVER_INIT( ckongs )
{
	driver_init_scramble_ppi(machine);

	ppi8255_set_portBread(0, ckongs_input_port_1_r);
	ppi8255_set_portCread(0, ckongs_input_port_2_r);
}

DRIVER_INIT( mariner )
{
	driver_init_scramble_ppi(machine);

	/* extra ROM */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5800, 0x67ff, 0, 0, MRA8_BANK1);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5800, 0x67ff, 0, 0, MWA8_ROM);
	memory_set_bankptr(1, memory_region(REGION_CPU1) + 0x5800);

	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x9008, 0x9008, 0, 0, mariner_protection_2_r);
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0xb401, 0xb401, 0, 0, mariner_protection_1_r);

	/* ??? (it's NOT a background enable) */
	/*memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6803, 0x6803, 0, 0, MWA8_NOP);*/
}

DRIVER_INIT( frogger )
{
	offs_t A;
	UINT8 *ROM;


	driver_init_scramble_ppi(machine);


	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	ROM = memory_region(REGION_CPU2);
	for (A = 0;A < 0x0800;A++)
		ROM[A] = BITSWAP8(ROM[A],7,6,5,4,3,2,0,1);

	/* likewise, the 2nd gfx ROM has data lines D0 and D1 swapped. Decode it. */
	ROM = memory_region(REGION_GFX1);
	for (A = 0x0800;A < 0x1000;A++)
		ROM[A] = BITSWAP8(ROM[A],7,6,5,4,3,2,0,1);
}

DRIVER_INIT( froggers )
{
	offs_t A;
	UINT8 *ROM;


	driver_init_scramble_ppi(machine);

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	ROM = memory_region(REGION_CPU2);
	for (A = 0;A < 0x0800;A++)
		ROM[A] = BITSWAP8(ROM[A],7,6,5,4,3,2,0,1);
}

DRIVER_INIT( devilfsh )
{
	offs_t i;
	UINT8 *RAM;


	driver_init_scramble_ppi(machine);


	/* Address lines are scrambled on the main CPU */

	/* A0 -> A2 */
	/* A1 -> A0 */
	/* A2 -> A3 */
	/* A3 -> A1 */

	RAM = memory_region(REGION_CPU1);
	for (i = 0; i < 0x10000; i += 16)
	{
		offs_t j;
		UINT8 swapbuffer[16];

		for (j = 0; j < 16; j++)
		{
			offs_t new = BITSWAP8(j,7,6,5,4,2,0,3,1);

			swapbuffer[j] = RAM[i + new];
		}

		memcpy(&RAM[i], swapbuffer, 16);
	}
}

DRIVER_INIT( mars )
{
	driver_init_devilfsh(machine);

	/* extra port */
	ppi8255_set_portCread(1, input_port_3_r);
}

DRIVER_INIT( hotshock )
{
	/* protection??? The game jumps into never-neverland here. I think
       it just expects a RET there */
	memory_region(REGION_CPU1)[0x2ef9] = 0xc9;
}

DRIVER_INIT( cavelon )
{
	UINT8 *ROM = memory_region(REGION_CPU1);

	driver_init_scramble_ppi(machine);

	/* banked ROM */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x3fff, 0, 0, MRA8_BANK1);
	memory_configure_bank(1, 0, 2, &ROM[0x00000], 0x10000);
	cavelon_banksw();

	/* A15 switches memory banks */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, cavelon_banksw_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, cavelon_banksw_w);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2000, 0x2000, 0, 0, MWA8_NOP);	/* ??? */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x3800, 0x3801, 0, 0, MWA8_NOP);  /* looks suspicously like
                                                               an AY8910, but not sure */
	state_save_register_global(cavelon_bank);
}

DRIVER_INIT( moonwar )
{
	driver_init_scramble_ppi(machine);

	/* special handler for the spinner */
	ppi8255_set_portAread (0, moonwar_input_port_0_r);
	ppi8255_set_portCwrite(0, moonwar_port_select_w);

	state_save_register_global(moonwar_port_select);
}

DRIVER_INIT( darkplnt )
{
	driver_init_scramble_ppi(machine);

	/* special handler for the spinner */
	ppi8255_set_portBread(0, darkplnt_input_port_1_r);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xb00a, 0xb00a, 0, 0, darkplnt_bullet_color_w);
}

DRIVER_INIT( mimonkey )
{
	static const UINT8 xortable[16][16] =
	{
		{ 0x03,0x03,0x05,0x07,0x85,0x00,0x85,0x85,0x80,0x80,0x06,0x03,0x03,0x00,0x00,0x81 },
		{ 0x83,0x87,0x03,0x87,0x06,0x00,0x06,0x04,0x02,0x00,0x84,0x84,0x04,0x00,0x01,0x83 },
		{ 0x82,0x82,0x84,0x02,0x04,0x00,0x00,0x03,0x82,0x00,0x06,0x80,0x03,0x00,0x81,0x07 },
		{ 0x06,0x06,0x82,0x81,0x85,0x00,0x04,0x07,0x81,0x05,0x04,0x00,0x03,0x00,0x82,0x84 },
		{ 0x07,0x07,0x80,0x07,0x07,0x00,0x85,0x86,0x00,0x07,0x06,0x04,0x85,0x00,0x86,0x85 },
		{ 0x81,0x83,0x02,0x02,0x87,0x00,0x86,0x03,0x04,0x06,0x80,0x05,0x87,0x00,0x81,0x81 },
		{ 0x01,0x01,0x00,0x07,0x07,0x00,0x01,0x01,0x07,0x07,0x06,0x00,0x06,0x00,0x07,0x07 },
		{ 0x80,0x87,0x81,0x87,0x83,0x00,0x84,0x01,0x01,0x86,0x86,0x80,0x86,0x00,0x86,0x86 },
		{ 0x03,0x03,0x05,0x07,0x85,0x00,0x85,0x85,0x80,0x80,0x06,0x03,0x03,0x00,0x00,0x81 },
		{ 0x83,0x87,0x03,0x87,0x06,0x00,0x06,0x04,0x02,0x00,0x84,0x84,0x04,0x00,0x01,0x83 },
		{ 0x82,0x82,0x84,0x02,0x04,0x00,0x00,0x03,0x82,0x00,0x06,0x80,0x03,0x00,0x81,0x07 },
		{ 0x06,0x06,0x82,0x81,0x85,0x00,0x04,0x07,0x81,0x05,0x04,0x00,0x03,0x00,0x82,0x84 },
		{ 0x07,0x07,0x80,0x07,0x07,0x00,0x85,0x86,0x00,0x07,0x06,0x04,0x85,0x00,0x86,0x85 },
		{ 0x81,0x83,0x02,0x02,0x87,0x00,0x86,0x03,0x04,0x06,0x80,0x05,0x87,0x00,0x81,0x81 },
		{ 0x01,0x01,0x00,0x07,0x07,0x00,0x01,0x01,0x07,0x07,0x06,0x00,0x06,0x00,0x07,0x07 },
		{ 0x80,0x87,0x81,0x87,0x83,0x00,0x84,0x01,0x01,0x86,0x86,0x80,0x86,0x00,0x86,0x86 }
	};

	UINT8 *ROM = memory_region(REGION_CPU1);
	int A, ctr = 0, line, col;

	for( A = 0; A < 0x4000; A++ )
	{
		line = (ctr & 0x07) | ((ctr & 0x200) >> 6);
		col = ((ROM[A] & 0x80) >> 4) | (ROM[A] & 0x07);
		ROM[A] = ROM[A] ^ xortable[line][col];
		ctr++;
	}

	driver_init_scramble_ppi(machine);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa804, 0xa804, 0, 0, scramble_background_enable_w);
}

DRIVER_INIT( mimonsco )
{
	driver_init_scramble_ppi(machine);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa804, 0xa804, 0, 0, scramble_background_enable_w);
}

DRIVER_INIT( mimonscr )
{
	driver_init_scramble_ppi(machine);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6804, 0x6804, 0, 0, scramble_background_enable_w);
}


static int bit(int i,int n)
{
	return ((i >> n) & 1);
}


DRIVER_INIT( anteater )
{
	offs_t i;
	UINT8 *RAM;
	UINT8 *scratch;


	driver_init_scobra(machine);

	/*
    *   Code To Decode Lost Tomb by Mirko Buffoni
    *   Optimizations done by Fabio Buffoni
    */

	RAM = memory_region(REGION_GFX1);

	scratch = malloc_or_die(memory_region_length(REGION_GFX1));

		memcpy(scratch, RAM, memory_region_length(REGION_GFX1));

		for (i = 0; i < memory_region_length(REGION_GFX1); i++)
		{
			int j;


			j = i & 0x9bf;
			j |= ( bit(i,4) ^ bit(i,9) ^ ( bit(i,2) & bit(i,10) ) ) << 6;
			j |= ( bit(i,2) ^ bit(i,10) ) << 9;
			j |= ( bit(i,0) ^ bit(i,6) ^ 1 ) << 10;

			RAM[i] = scratch[j];
		}

		free(scratch);
}

DRIVER_INIT( rescue )
{
	offs_t i;
	UINT8 *RAM;
	UINT8 *scratch;


	driver_init_scobra(machine);

	/*
    *   Code To Decode Lost Tomb by Mirko Buffoni
    *   Optimizations done by Fabio Buffoni
    */

	RAM = memory_region(REGION_GFX1);

	scratch = malloc_or_die(memory_region_length(REGION_GFX1));

		memcpy(scratch, RAM, memory_region_length(REGION_GFX1));

		for (i = 0; i < memory_region_length(REGION_GFX1); i++)
		{
			int j;


			j = i & 0xa7f;
			j |= ( bit(i,3) ^ bit(i,10) ) << 7;
			j |= ( bit(i,1) ^ bit(i,7) ) << 8;
			j |= ( bit(i,0) ^ bit(i,8) ) << 10;

			RAM[i] = scratch[j];
		}

		free(scratch);
}

DRIVER_INIT( minefld )
{
	offs_t i;
	UINT8 *RAM;
	UINT8 *scratch;


	driver_init_scobra(machine);

	/*
    *   Code To Decode Minefield by Mike Balfour and Nicola Salmoria
    */

	RAM = memory_region(REGION_GFX1);

	scratch = malloc_or_die(memory_region_length(REGION_GFX1));

		memcpy(scratch, RAM, memory_region_length(REGION_GFX1));

		for (i = 0; i < memory_region_length(REGION_GFX1); i++)
		{
			int j;


			j  = i & 0xd5f;
			j |= ( bit(i,3) ^ bit(i,7) ) << 5;
			j |= ( bit(i,2) ^ bit(i,9) ^ ( bit(i,0) & bit(i,5) ) ^
				 ( bit(i,3) & bit(i,7) & ( bit(i,0) ^ bit(i,5) ))) << 7;
			j |= ( bit(i,0) ^ bit(i,5) ^ ( bit(i,3) & bit(i,7) ) ) << 9;

			RAM[i] = scratch[j];
		}

		free(scratch);
}

DRIVER_INIT( losttomb )
{
	offs_t i;
	UINT8 *RAM;
	UINT8 *scratch;


	driver_init_scramble(machine);

	/*
    *   Code To Decode Lost Tomb by Mirko Buffoni
    *   Optimizations done by Fabio Buffoni
    */

	RAM = memory_region(REGION_GFX1);

	scratch = malloc_or_die(memory_region_length(REGION_GFX1));

		memcpy(scratch, RAM, memory_region_length(REGION_GFX1));

		for (i = 0; i < memory_region_length(REGION_GFX1); i++)
		{
			int j;


			j = i & 0xa7f;
			j |= ( (bit(i,1) & bit(i,8)) | ((1 ^ bit(i,1)) & (bit(i,10)))) << 7;
			j |= ( bit(i,7) ^ (bit(i,1) & ( bit(i,7) ^ bit(i,10) ))) << 8;
			j |= ( (bit(i,1) & bit(i,7)) | ((1 ^ bit(i,1)) & (bit(i,8)))) << 10;

			RAM[i] = scratch[j];
		}

		free(scratch);
}

DRIVER_INIT( superbon )
{
	offs_t i;
	UINT8 *RAM;


	driver_init_scramble(machine);

	/* Deryption worked out by hand by Chris Hardy. */

	RAM = memory_region(REGION_CPU1);

	for (i = 0;i < 0x1000;i++)
	{
		/* Code is encrypted depending on bit 7 and 9 of the address */
		switch (i & 0x0280)
		{
		case 0x0000:
			RAM[i] ^= 0x92;
			break;
		case 0x0080:
			RAM[i] ^= 0x82;
			break;
		case 0x0200:
			RAM[i] ^= 0x12;
			break;
		case 0x0280:
			RAM[i] ^= 0x10;
			break;
		}
	}
}


DRIVER_INIT( hustler )
{
	offs_t A;


	driver_init_scramble_ppi(machine);


	for (A = 0;A < 0x4000;A++)
	{
		UINT8 xormask;
		int bits[8];
		int i;
		UINT8 *rom = memory_region(REGION_CPU1);


		for (i = 0;i < 8;i++)
			bits[i] = (A >> i) & 1;

		xormask = 0xff;
		if (bits[0] ^ bits[1]) xormask ^= 0x01;
		if (bits[3] ^ bits[6]) xormask ^= 0x02;
		if (bits[4] ^ bits[5]) xormask ^= 0x04;
		if (bits[0] ^ bits[2]) xormask ^= 0x08;
		if (bits[2] ^ bits[3]) xormask ^= 0x10;
		if (bits[1] ^ bits[5]) xormask ^= 0x20;
		if (bits[0] ^ bits[7]) xormask ^= 0x40;
		if (bits[4] ^ bits[6]) xormask ^= 0x80;

		rom[A] ^= xormask;
	}

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	{
		UINT8 *rom = memory_region(REGION_CPU2);


		for (A = 0;A < 0x0800;A++)
			rom[A] = BITSWAP8(rom[A],7,6,5,4,3,2,0,1);
	}
}

DRIVER_INIT( billiard )
{
	offs_t A;


	driver_init_scramble_ppi(machine);


	for (A = 0;A < 0x4000;A++)
	{
		UINT8 xormask;
		int bits[8];
		int i;
		UINT8 *rom = memory_region(REGION_CPU1);


		for (i = 0;i < 8;i++)
			bits[i] = (A >> i) & 1;

		xormask = 0x55;
		if (bits[2] ^ ( bits[3] &  bits[6])) xormask ^= 0x01;
		if (bits[4] ^ ( bits[5] &  bits[7])) xormask ^= 0x02;
		if (bits[0] ^ ( bits[7] & !bits[3])) xormask ^= 0x04;
		if (bits[3] ^ (!bits[0] &  bits[2])) xormask ^= 0x08;
		if (bits[5] ^ (!bits[4] &  bits[1])) xormask ^= 0x10;
		if (bits[6] ^ (!bits[2] & !bits[5])) xormask ^= 0x20;
		if (bits[1] ^ (!bits[6] & !bits[4])) xormask ^= 0x40;
		if (bits[7] ^ (!bits[1] &  bits[0])) xormask ^= 0x80;

		rom[A] ^= xormask;

		rom[A] = BITSWAP8(rom[A],6,1,2,5,4,3,0,7);
	}

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	{
		UINT8 *rom = memory_region(REGION_CPU2);


		for (A = 0;A < 0x0800;A++)
			rom[A] = BITSWAP8(rom[A],7,6,5,4,3,2,0,1);
	}
}

/************************************************************
 mr kougar protected main cpu - by HIGHWAYMAN
 mr kougar contains a steel module at location S7,
 this module contains a Z80c cpu with the following changes:
 IOREQ pin cut, RD & WR pins swapped and the following
 address lines swapped - a0-a2,a1-a0,a2-a3,a3-a1.
*************************************************************/

DRIVER_INIT( mrkougar )
{
	driver_init_devilfsh(machine);

	/* no sound enabled bit */
	ppi8255_set_portBwrite(1, mrkougar_sh_irqtrigger_w);
}

DRIVER_INIT( mrkougb )
{
	driver_init_scramble_ppi(machine);

	/* no sound enabled bit */
	ppi8255_set_portBwrite(1, mrkougar_sh_irqtrigger_w);
}

DRIVER_INIT( sfx )
{
	ppi8255_init(&sfx_ppi8255_intf);
}

DRIVER_INIT( monsterz )
{
	ppi8255_init(&monsterz_ppi8255_intf);

	/* extra ROM */
	memory_install_read8_handler(1, ADDRESS_SPACE_PROGRAM, 0x3000, 0x3fff, 0, 0, MRA8_BANK1);
	memory_set_bankptr(1, memory_region(REGION_CPU2) + 0x3000);
}

static READ8_HANDLER( scorpion_prot_r )
{
	/* HACK! return register C */
	return activecpu_get_reg(Z80_C) & 0xff;
}

static READ8_HANDLER( scorpion_sound_status_r )
{
	return 1;
}

DRIVER_INIT( scorpion )
{
	ppi8255_init(&ppi8255_intf);

	ppi8255_set_portCread(1, scorpion_prot_r);

	/* extra ROM */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5800, 0x67ff, 0, 0, MRA8_BANK1);
	memory_set_bankptr(1, memory_region(REGION_CPU1) + 0x5800);

	/* no background related */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6803, 0x6803, 0, 0, MWA8_NOP);

	memory_install_read8_handler(1, ADDRESS_SPACE_PROGRAM, 0x3000, 0x3000, 0, 0, scorpion_sound_status_r);
}

DRIVER_INIT( ad2083 )
{
	UINT8 c;
	int i;

	UINT8 *ROM = memory_region(REGION_CPU1);

	for (i=0; i<memory_region_length(REGION_CPU1); i++)
	{
		c = ROM[i] ^ 0x35;
		c = BITSWAP8(c, 6,2,5,1,7,3,4,0); /* also swapped inside of the bigger module */
		ROM[i] = c;
	}
}
