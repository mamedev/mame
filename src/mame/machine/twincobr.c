/****************************************************************************
 *  Twin Cobra                                                              *
 *  Communications and memory functions between shared CPU memory spaces    *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "includes/twincobr.h"


#define LOG_DSP_CALLS 0
#define LOG(x) do { if (LOG_DSP_CALLS) logerror x; } while (0)
#define CLEAR  0
#define ASSERT 1


int toaplan_main_cpu;	/* Main CPU type.  0 = 68000, 1 = Z80 */
static int twincobr_intenable;
int wardner_membank;
static int twincobr_dsp_on;
static int twincobr_dsp_BIO;
static int fsharkbt_8741;
static int dsp_execute;
static UINT32 dsp_addr_w, main_ram_seg;

static const int toaplan_port_type[2] = { 0x7800c, 0x5c };

UINT8 *twincobr_sharedram;



INTERRUPT_GEN( twincobr_interrupt )
{
	if (twincobr_intenable) {
		twincobr_intenable = 0;
		cpu_set_input_line(device, M68K_IRQ_4, HOLD_LINE);
	}
}

INTERRUPT_GEN( wardner_interrupt )
{
	if (twincobr_intenable) {
		twincobr_intenable = 0;
		cpu_set_input_line(device, 0, HOLD_LINE);
	}
}


WRITE16_HANDLER( twincobr_dsp_addrsel_w )
{
	/* This sets the main CPU RAM address the DSP should */
	/*  read/write, via the DSP IO port 0 */
	/* Top three bits of data need to be shifted left 3 places */
	/*  to select which memory bank from main CPU address */
	/*  space to use */
	/* Lower thirteen bits of this data is shifted left one position */
	/*  to move it to an even address word boundary */

	main_ram_seg = ((data & 0xe000) << 3);
	dsp_addr_w   = ((data & 0x1fff) << 1);

	LOG(("DSP PC:%04x IO write %04x (%08x) at port 0\n",cpu_get_previouspc(space->cpu),data,main_ram_seg + dsp_addr_w));
}

READ16_HANDLER( twincobr_dsp_r )
{
	/* DSP can read data from main CPU RAM via DSP IO port 1 */

	const address_space *mainspace;
	UINT16 input_data = 0;
	switch (main_ram_seg) {
		case 0x30000:
		case 0x40000:
		case 0x50000:	mainspace = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
						input_data = memory_read_word(mainspace, main_ram_seg + dsp_addr_w);
						break;
		default:		logerror("DSP PC:%04x Warning !!! IO reading from %08x (port 1)\n",cpu_get_previouspc(space->cpu),main_ram_seg + dsp_addr_w); break;
	}
	LOG(("DSP PC:%04x IO read %04x at %08x (port 1)\n",cpu_get_previouspc(space->cpu),input_data,main_ram_seg + dsp_addr_w));
	return input_data;
}

WRITE16_HANDLER( twincobr_dsp_w )
{
	const address_space *mainspace;

	/* Data written to main CPU RAM via DSP IO port 1 */
	dsp_execute = 0;
	switch (main_ram_seg) {
		case 0x30000:	if ((dsp_addr_w < 3) && (data == 0)) dsp_execute = 1;
		case 0x40000:
		case 0x50000:	mainspace = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
						memory_write_word(mainspace, main_ram_seg + dsp_addr_w, data);
						break;
		default:		logerror("DSP PC:%04x Warning !!! IO writing to %08x (port 1)\n",cpu_get_previouspc(space->cpu),main_ram_seg + dsp_addr_w); break;
	}
	LOG(("DSP PC:%04x IO write %04x at %08x (port 1)\n",cpu_get_previouspc(space->cpu),data,main_ram_seg + dsp_addr_w));
}

WRITE16_HANDLER( wardner_dsp_addrsel_w )
{
	/* This sets the main CPU RAM address the DSP should */
	/*  read/write, via the DSP IO port 0 */
	/* Lower twelve bits of this data is shifted left one position */
	/*  to move it to an even address boundary */

	main_ram_seg =  (data & 0xe000);
	dsp_addr_w   = ((data & 0x07ff) << 1);

	if (main_ram_seg == 0x6000) main_ram_seg = 0x7000;

	LOG(("DSP PC:%04x IO write %04x (%08x) at port 0\n",cpu_get_previouspc(space->cpu),data,main_ram_seg + dsp_addr_w));
}

READ16_HANDLER( wardner_dsp_r )
{
	/* DSP can read data from main CPU RAM via DSP IO port 1 */

	const address_space *mainspace;
	UINT16 input_data = 0;
	switch (main_ram_seg) {
		case 0x7000:
		case 0x8000:
		case 0xa000:	mainspace = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
						input_data =  memory_read_byte(mainspace, main_ram_seg + (dsp_addr_w + 0))
								   | (memory_read_byte(mainspace, main_ram_seg + (dsp_addr_w + 1)) << 8);
						break;
		default:		logerror("DSP PC:%04x Warning !!! IO reading from %08x (port 1)\n",cpu_get_previouspc(space->cpu),main_ram_seg + dsp_addr_w); break;
	}
	LOG(("DSP PC:%04x IO read %04x at %08x (port 1)\n",cpu_get_previouspc(space->cpu),input_data,main_ram_seg + dsp_addr_w));
	return input_data;
}

WRITE16_HANDLER( wardner_dsp_w )
{
	const address_space *mainspace;

	/* Data written to main CPU RAM via DSP IO port 1 */
	dsp_execute = 0;
	switch (main_ram_seg) {
		case 0x7000:	if ((dsp_addr_w < 3) && (data == 0)) dsp_execute = 1;
		case 0x8000:
		case 0xa000:	mainspace = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
						memory_write_byte(mainspace, main_ram_seg + (dsp_addr_w + 0), (data & 0xff));
						memory_write_byte(mainspace, main_ram_seg + (dsp_addr_w + 1), ((data >> 8) & 0xff));
						break;
		default:		logerror("DSP PC:%04x Warning !!! IO writing to %08x (port 1)\n",cpu_get_previouspc(space->cpu),main_ram_seg + dsp_addr_w); break;
	}
	LOG(("DSP PC:%04x IO write %04x at %08x (port 1)\n",cpu_get_previouspc(space->cpu),data,main_ram_seg + dsp_addr_w));
}

WRITE16_HANDLER( twincobr_dsp_bio_w )
{
	/* data 0xffff  means inhibit BIO line to DSP and enable */
	/*              communication to main processor */
	/*              Actually only DSP data bit 15 controls this */
	/* data 0x0000  means set DSP BIO line active and disable */
	/*              communication to main processor*/
	LOG(("DSP PC:%04x IO write %04x at port 3\n",cpu_get_previouspc(space->cpu),data));
	if (data & 0x8000) {
		twincobr_dsp_BIO = CLEAR_LINE;
	}
	if (data == 0) {
		if (dsp_execute) {
			LOG(("Turning the main CPU on\n"));
			cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_HALT, CLEAR_LINE);
			dsp_execute = 0;
		}
		twincobr_dsp_BIO = ASSERT_LINE;
	}
}

READ16_HANDLER( fsharkbt_dsp_r )
{
	/* IO Port 2 used by Flying Shark bootleg */
	/* DSP reads data from an extra MCU (8741) at IO port 2 */
	/* Port is read three times during startup. First and last data */
	/*   read must equal, but second data read must be different */
	fsharkbt_8741 += 1;
	LOG(("DSP PC:%04x IO read %04x from 8741 MCU (port 2)\n",cpu_get_previouspc(space->cpu),(fsharkbt_8741 & 0x08)));
	return (fsharkbt_8741 & 1);
}

WRITE16_HANDLER( fsharkbt_dsp_w )
{
	/* Flying Shark bootleg DSP writes data to an extra MCU (8741) at IO port 2 */
#if 0
	logerror("DSP PC:%04x IO write from DSP RAM:%04x to 8741 MCU (port 2)\n",cpu_get_previouspc(space->cpu),fsharkbt_8741);
#endif
}

READ16_HANDLER ( twincobr_BIO_r )
{
	return twincobr_dsp_BIO;
}


static void twincobr_dsp(running_machine *machine, int enable)
{
	twincobr_dsp_on = enable;
	if (enable) {
		LOG(("Turning DSP on and main CPU off\n"));
		cputag_set_input_line(machine, "dsp", INPUT_LINE_HALT, CLEAR_LINE);
		cputag_set_input_line(machine, "dsp", 0, ASSERT_LINE); /* TMS32010 INT */
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_HALT, ASSERT_LINE);
	}
	else {
		LOG(("Turning DSP off\n"));
		cputag_set_input_line(machine, "dsp", 0, CLEAR_LINE); /* TMS32010 INT */
		cputag_set_input_line(machine, "dsp", INPUT_LINE_HALT, ASSERT_LINE);
	}
}

static STATE_POSTLOAD( twincobr_restore_dsp )
{
	twincobr_dsp(machine, twincobr_dsp_on);
}


static void toaplan0_control_w(running_machine *machine, int offset, int data)
{
	LOG(("%s:Writing %08x to %08x.\n",cpuexec_describe_context(machine),data,toaplan_port_type[toaplan_main_cpu] - offset));

	if (toaplan_main_cpu == 1) {
		if (data == 0x0c) { data = 0x1c; wardner_sprite_hack=0; }	/* Z80 ? */
		if (data == 0x0d) { data = 0x1d; wardner_sprite_hack=1; }	/* Z80 ? */
	}

	switch (data) {
		case 0x0004: twincobr_intenable = 0; break;
		case 0x0005: twincobr_intenable = 1; break;
		case 0x0006: twincobr_flipscreen(machine, 0); break;
		case 0x0007: twincobr_flipscreen(machine, 1); break;
		case 0x0008: twincobr_bg_ram_bank = 0x0000; break;
		case 0x0009: twincobr_bg_ram_bank = 0x1000; break;
		case 0x000a: twincobr_fg_rom_bank = 0x0000; break;
		case 0x000b: twincobr_fg_rom_bank = 0x1000; break;
		case 0x000c: twincobr_dsp(machine, 1); break;	 /* Enable the INT line to the DSP */
		case 0x000d: twincobr_dsp(machine, 0); break;	 /* Inhibit the INT line to the DSP */
		case 0x000e: twincobr_display(0); break; /* Turn display off */
		case 0x000f: twincobr_display(1); break; /* Turn display on */
	}
}

WRITE16_HANDLER( twincobr_control_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan0_control_w(space->machine, offset, data & 0xff);
	}
}

WRITE8_HANDLER( wardner_control_w )
{
	toaplan0_control_w(space->machine, offset, data);
}


READ16_HANDLER( twincobr_sharedram_r )
{
	return twincobr_sharedram[offset];
}

WRITE16_HANDLER( twincobr_sharedram_w )
{
	if (ACCESSING_BITS_0_7)
	{
		twincobr_sharedram[offset] = data & 0xff;
	}
}


static void toaplan0_coin_dsp_w(const address_space *space, int offset, int data)
{
	if (data > 1)
		LOG(("%s:Writing %08x to %08x.\n",cpuexec_describe_context(space->machine),data,toaplan_port_type[toaplan_main_cpu] - offset));
	switch (data) {
		case 0x08: coin_counter_w(space->machine, 0,0); break;
		case 0x09: coin_counter_w(space->machine, 0,1); break;
		case 0x0a: coin_counter_w(space->machine, 1,0); break;
		case 0x0b: coin_counter_w(space->machine, 1,1); break;
		case 0x0c: coin_lockout_w(space->machine, 0,1); break;
		case 0x0d: coin_lockout_w(space->machine, 0,0); break;
		case 0x0e: coin_lockout_w(space->machine, 1,1); break;
		case 0x0f: coin_lockout_w(space->machine, 1,0); break;
		/****** The following apply to Flying Shark/Wardner only ******/
		case 0x00:	/* This means assert the INT line to the DSP */
					LOG(("Turning DSP on and main CPU off\n"));
					cputag_set_input_line(space->machine, "dsp", INPUT_LINE_HALT, CLEAR_LINE);
					cputag_set_input_line(space->machine, "dsp", 0, ASSERT_LINE); /* TMS32010 INT */
					cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_HALT, ASSERT_LINE);
					break;
		case 0x01:	/* This means inhibit the INT line to the DSP */
					LOG(("Turning DSP off\n"));
					cputag_set_input_line(space->machine, "dsp", 0, CLEAR_LINE); /* TMS32010 INT */
					cputag_set_input_line(space->machine, "dsp", INPUT_LINE_HALT, ASSERT_LINE);
					break;
	}
}


WRITE16_HANDLER( fshark_coin_dsp_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan0_coin_dsp_w(space, offset, data & 0xff);
	}
}

WRITE8_HANDLER( twincobr_coin_w )
{
	toaplan0_coin_dsp_w(space, offset, data);
}

WRITE8_HANDLER( wardner_coin_dsp_w )
{
	toaplan0_coin_dsp_w(space, offset, data);
}


MACHINE_RESET( twincobr )
{
	toaplan_main_cpu = 0;		/* 68000 */
	twincobr_display(0);
	twincobr_intenable = 0;
	dsp_addr_w = 0;
	main_ram_seg = 0;
	dsp_execute = 0;
	fsharkbt_8741 = -1;			/* Reset the Flying Shark Bootleg MCU */
	twincobr_dsp_BIO = CLEAR_LINE;
}
void twincobr_driver_savestate(running_machine *machine)
{
	state_save_register_global(machine, toaplan_main_cpu);
	state_save_register_global(machine, twincobr_intenable);
	state_save_register_global(machine, twincobr_dsp_on);
	state_save_register_global(machine, dsp_addr_w);
	state_save_register_global(machine, main_ram_seg);
	state_save_register_global(machine, twincobr_dsp_BIO);
	state_save_register_global(machine, dsp_execute);
	state_save_register_global(machine, fsharkbt_8741);
	state_save_register_postload(machine, twincobr_restore_dsp, NULL);
}

MACHINE_RESET( wardner )
{
	toaplan_main_cpu = 1;		/* Z80 */
	twincobr_display(1);
	twincobr_intenable = 0;
	dsp_addr_w = 0;
	main_ram_seg = 0;
	dsp_execute = 0;
	twincobr_dsp_BIO = CLEAR_LINE;
	wardner_membank = 0;
}
void wardner_driver_savestate(running_machine *machine)
{
	state_save_register_global(machine, toaplan_main_cpu);
	state_save_register_global(machine, twincobr_intenable);
	state_save_register_global(machine, twincobr_dsp_on);
	state_save_register_global(machine, dsp_addr_w);
	state_save_register_global(machine, main_ram_seg);
	state_save_register_global(machine, twincobr_dsp_BIO);
	state_save_register_global(machine, dsp_execute);
	state_save_register_global(machine, wardner_membank);
	state_save_register_postload(machine, wardner_restore_bank, NULL);	/* Restore the Main CPU bank */
	state_save_register_postload(machine, twincobr_restore_dsp, NULL);
}
