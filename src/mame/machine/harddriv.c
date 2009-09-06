/***************************************************************************

    Hard Drivin' machine hardware

****************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/m68000/m68000.h"
#include "cpu/dsp32/dsp32.h"
#include "machine/atarigen.h"
#include "machine/asic65.h"
#include "audio/atarijsa.h"
#include "includes/slapstic.h"
#include "harddriv.h"


/*************************************
 *
 *  Constants and macros
 *
 *************************************/

#define DUART_CLOCK			(36864000)
#define DS3_TRIGGER			7777

/* debugging tools */
#define LOG_COMMANDS		0



/*************************************
 *
 *  External definitions
 *
 *************************************/

/* externally accessible */
const device_config *hdcpu_main;
const device_config *hdcpu_gsp;
const device_config *hdcpu_msp;
const device_config *hdcpu_adsp;
const device_config *hdcpu_sound;
const device_config *hdcpu_sounddsp;
const device_config *hdcpu_jsa;
const device_config *hdcpu_dsp32;

UINT8 hd34010_host_access;
UINT8 hddsk_pio_access;

UINT16 *hdmsp_ram;
UINT16 *hddsk_ram;
UINT16 *hddsk_rom;
UINT16 *hddsk_zram;
UINT16 *hd68k_slapstic_base;
UINT16 *st68k_sloop_alt_base;

UINT16 *hdadsp_data_memory;
UINT32 *hdadsp_pgm_memory;

UINT16 *hdgsp_protection;
UINT16 *stmsp_sync[3];

UINT16 *hdgsp_speedup_addr[2];
offs_t hdgsp_speedup_pc;

UINT16 *hdmsp_speedup_addr;
offs_t hdmsp_speedup_pc;

UINT16 *hdds3_speedup_addr;
offs_t hdds3_speedup_pc;
offs_t hdds3_transfer_pc;

UINT32 *rddsp32_sync[2];

UINT32 gsp_speedup_count[4];
UINT32 msp_speedup_count[4];
UINT32 adsp_speedup_count[4];


/*************************************
 *
 *  Static globals
 *
 *************************************/

static UINT8 irq_state;
static UINT8 gsp_irq_state;
static UINT8 msp_irq_state;
static UINT8 adsp_irq_state;
static UINT8 duart_irq_state;

static UINT8 duart_read_data[16];
static UINT8 duart_write_data[16];
static UINT8 duart_output_port;
static emu_timer *duart_timer;

static UINT8 last_gsp_shiftreg;

static UINT8 m68k_zp1, m68k_zp2;
static UINT8 m68k_adsp_buffer_bank;

static UINT8 adsp_halt, adsp_br;
static UINT8 adsp_xflag;

static UINT16 adsp_sim_address;
static UINT16 adsp_som_address;
static UINT32 adsp_eprom_base;

static UINT16 *sim_memory;
static UINT16 *som_memory;
static UINT32 sim_memory_size;
static UINT16 *adsp_pgm_memory_word;

static UINT8 ds3_gcmd, ds3_gflag, ds3_g68irqs, ds3_gfirqs, ds3_g68flag, ds3_send, ds3_reset;
static UINT16 ds3_gdata, ds3_g68data;
static UINT32 ds3_sim_address;

static UINT16 adc_control;
static UINT8 adc8_select;
static UINT8 adc8_data;
static UINT8 adc12_select;
static UINT8 adc12_byte;
static UINT16 adc12_data;

static UINT16 hdc68k_last_wheel;
static UINT16 hdc68k_last_port1;
static UINT8 hdc68k_wheel_edge;
static UINT8 hdc68k_shifter_state;

static UINT8 st68k_sloop_bank = 0;
static offs_t st68k_last_alt_sloop_offset;

#define MAX_MSP_SYNC	16
static UINT32 *dataptr[MAX_MSP_SYNC];
static UINT32 dataval[MAX_MSP_SYNC];
static int next_msp_sync;

static void hd68k_update_interrupts(running_machine *machine);
static TIMER_CALLBACK( duart_callback );



#if 0
#pragma mark * DRIVER/MULTISYNC BOARD
#endif


/*************************************
 *
 *  Initialization
 *
 *************************************/

MACHINE_START( harddriv )
{
	/* predetermine memory regions */
	sim_memory = (UINT16 *)memory_region(machine, "user1");
	som_memory = auto_alloc_array(machine, UINT16, 0x8000/2);
	sim_memory_size = memory_region_length(machine, "user1") / 2;
	adsp_pgm_memory_word = (UINT16 *)((UINT8 *)hdadsp_pgm_memory + 1);
}


MACHINE_RESET( harddriv )
{
	/* generic reset */
	atarigen_eeprom_reset();
	slapstic_reset();
	atarigen_interrupt_reset(hd68k_update_interrupts);

	/* halt several of the DSPs to start */
	if (hdcpu_adsp != NULL) cpu_set_input_line(hdcpu_adsp, INPUT_LINE_HALT, ASSERT_LINE);
	if (hdcpu_dsp32 != NULL) cpu_set_input_line(hdcpu_dsp32, INPUT_LINE_HALT, ASSERT_LINE);
	if (hdcpu_sounddsp != NULL) cpu_set_input_line(hdcpu_sounddsp, INPUT_LINE_HALT, ASSERT_LINE);

	/* if we found a 6502, reset the JSA board */
	if (hdcpu_jsa != NULL)
		atarijsa_reset();

	last_gsp_shiftreg = 0;

	m68k_adsp_buffer_bank = 0;

	/* reset IRQ states */
	irq_state = gsp_irq_state = msp_irq_state = adsp_irq_state = duart_irq_state = 0;

	/* reset the DUART */
	memset(duart_read_data, 0, sizeof(duart_read_data));
	memset(duart_write_data, 0, sizeof(duart_write_data));
	duart_output_port = 0;
	duart_timer = timer_alloc(machine, duart_callback, NULL);

	/* reset the ADSP/DSIII/DSIV boards */
	adsp_halt = 1;
	adsp_br = 0;
	adsp_xflag = 0;
}



/*************************************
 *
 *  68000 interrupt handling
 *
 *************************************/

static void hd68k_update_interrupts(running_machine *machine)
{
	cpu_set_input_line(hdcpu_main, 1, msp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	cpu_set_input_line(hdcpu_main, 2, adsp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	cpu_set_input_line(hdcpu_main, 3, gsp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	cpu_set_input_line(hdcpu_main, 4, atarigen_sound_int_state ? ASSERT_LINE : CLEAR_LINE);	/* /LINKIRQ on STUN Runner */
	cpu_set_input_line(hdcpu_main, 5, irq_state ? ASSERT_LINE : CLEAR_LINE);
	cpu_set_input_line(hdcpu_main, 6, duart_irq_state ? ASSERT_LINE : CLEAR_LINE);
}


INTERRUPT_GEN( hd68k_irq_gen )
{
	irq_state = 1;
	atarigen_update_interrupts(device->machine);
}


WRITE16_HANDLER( hd68k_irq_ack_w )
{
	irq_state = 0;
	atarigen_update_interrupts(space->machine);
}


void hdgsp_irq_gen(const device_config *device, int state)
{
	gsp_irq_state = state;
	atarigen_update_interrupts(device->machine);
}


void hdmsp_irq_gen(const device_config *device, int state)
{
	msp_irq_state = state;
	atarigen_update_interrupts(device->machine);
}



/*************************************
 *
 *  68000 access to GSP
 *
 *************************************/

READ16_HANDLER( hd68k_gsp_io_r )
{
	UINT16 result;
	offset = (offset / 2) ^ 1;
	hd34010_host_access = 1;
	result = tms34010_host_r(hdcpu_gsp, offset);
	hd34010_host_access = 0;
	return result;
}


WRITE16_HANDLER( hd68k_gsp_io_w )
{
	offset = (offset / 2) ^ 1;
	hd34010_host_access = 1;
	tms34010_host_w(hdcpu_gsp, offset, data);
	hd34010_host_access = 0;
}



/*************************************
 *
 *  68000 access to MSP
 *
 *************************************/

READ16_HANDLER( hd68k_msp_io_r )
{
	UINT16 result;
	offset = (offset / 2) ^ 1;
	hd34010_host_access = 1;
	result = (hdcpu_msp != NULL) ? tms34010_host_r(hdcpu_msp, offset) : 0xffff;
	hd34010_host_access = 0;
	return result;
}


WRITE16_HANDLER( hd68k_msp_io_w )
{
	offset = (offset / 2) ^ 1;
	if (hdcpu_msp != NULL)
	{
		hd34010_host_access = 1;
		tms34010_host_w(hdcpu_msp, offset, data);
		hd34010_host_access = 0;
	}
}



/*************************************
 *
 *  68000 input handlers
 *
 *************************************/

READ16_HANDLER( hd68k_port0_r )
{
	/* port is as follows:

        0x0001 = DIAGN
        0x0002 = /HSYNCB
        0x0004 = /VSYNCB
        0x0008 = EOC12
        0x0010 = EOC8
        0x0020 = SELF-TEST
        0x0040 = COIN2
        0x0080 = COIN1
        0x0100 = SW1 #8
        0x0200 = SW1 #7
            .....
        0x8000 = SW1 #1
    */
	int temp = input_port_read(space->machine, "IN0");
	if (atarigen_get_hblank(space->machine->primary_screen)) temp ^= 0x0002;
	temp ^= 0x0018;		/* both EOCs always high for now */
	return temp;
}


READ16_HANDLER( hdc68k_port1_r )
{
	UINT16 result = input_port_read(space->machine, "a80000");
	UINT16 diff = result ^ hdc68k_last_port1;

	/* if a new shifter position is selected, use it */
	/* if it's the same shifter position as last time, go back to neutral */
	if ((diff & 0x0100) && !(result & 0x0100))
		hdc68k_shifter_state = (hdc68k_shifter_state == 1) ? 0 : 1;
	if ((diff & 0x0200) && !(result & 0x0200))
		hdc68k_shifter_state = (hdc68k_shifter_state == 2) ? 0 : 2;
	if ((diff & 0x0400) && !(result & 0x0400))
		hdc68k_shifter_state = (hdc68k_shifter_state == 4) ? 0 : 4;
	if ((diff & 0x0800) && !(result & 0x0800))
		hdc68k_shifter_state = (hdc68k_shifter_state == 8) ? 0 : 8;

	/* merge in the new shifter value */
	result = (result | 0x0f00) ^ (hdc68k_shifter_state << 8);

	/* merge in the wheel edge latch bit */
	if (hdc68k_wheel_edge)
		result ^= 0x4000;

	hdc68k_last_port1 = result;
	return result;
}


READ16_HANDLER( hda68k_port1_r )
{
	UINT16 result = input_port_read(space->machine, "a80000");

	/* merge in the wheel edge latch bit */
	if (hdc68k_wheel_edge)
		result ^= 0x4000;

	return result;
}


READ16_HANDLER( hdc68k_wheel_r )
{
	/* grab the new wheel value and upconvert to 12 bits */
	UINT16 new_wheel = input_port_read(space->machine, "12BADC0") << 4;

	/* hack to display the wheel position */
	if (input_code_pressed(space->machine, KEYCODE_LSHIFT))
		popmessage("%04X", new_wheel);

	/* if we crossed the center line, latch the edge bit */
	if ((hdc68k_last_wheel / 0xf0) != (new_wheel / 0xf0))
		hdc68k_wheel_edge = 1;

	/* remember the last value and return the low 8 bits */
	hdc68k_last_wheel = new_wheel;
	return (new_wheel << 8) | 0xff;
}


READ16_HANDLER( hd68k_adc8_r )
{
	return adc8_data;
}


READ16_HANDLER( hd68k_adc12_r )
{
	return adc12_byte ? ((adc12_data >> 8) & 0x0f) : (adc12_data & 0xff);
}


READ16_HANDLER( hd68k_sound_reset_r )
{
	if (hdcpu_jsa != NULL)
		atarijsa_reset();
	return ~0;
}



/*************************************
 *
 *  68000 output handlers
 *
 *************************************/

WRITE16_HANDLER( hd68k_adc_control_w )
{
	static const char *const adc8names[] = { "8BADC0", "8BADC1", "8BADC2", "8BADC3", "8BADC4", "8BADC5", "8BADC6", "8BADC7" };
	static const char *const adc12names[] = { "12BADC0", "12BADC1", "12BADC2", "12BADC3" };

	COMBINE_DATA(&adc_control);

	/* handle a write to the 8-bit ADC address select */
	if (adc_control & 0x08)
	{
		adc8_select = adc_control & 0x07;
		adc8_data = input_port_read(space->machine, adc8names[adc8_select]);
	}

	/* handle a write to the 12-bit ADC address select */
	if (adc_control & 0x40)
	{
		adc12_select = (adc_control >> 4) & 0x03;
		adc12_data = input_port_read(space->machine, adc12names[adc12_select]) << 4;
	}

	/* bit 7 selects which byte of the 12 bit data to read */
	adc12_byte = (adc_control >> 7) & 1;
}


WRITE16_HANDLER( hd68k_wr0_write )
{
	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 1:	/* SEL1 */
		case 2:	/* SEL2 */
		case 3:	/* SEL3 */
		case 4:	/* SEL4 */
		default:
			/* just ignore */
			break;

		case 6:	/* CC1 */
		case 7:	/* CC2 */
			coin_counter_w(offset - 6, data);
			break;
	}
}


WRITE16_HANDLER( hd68k_wr1_write )
{
	if (offset == 0) { //   logerror("Shifter Interface Latch = %02X\n", data);
	} else { 				logerror("/WR1(%04X)=%02X\n", offset, data);
	}
}


WRITE16_HANDLER( hd68k_wr2_write )
{
	if (offset == 0) { //   logerror("Steering Wheel Latch = %02X\n", data);
	} else { 				logerror("/WR2(%04X)=%02X\n", offset, data);
	}
}


WRITE16_HANDLER( hd68k_nwr_w )
{
	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0:	/* CR2 */
		case 1:	/* CR1 */
			set_led_status(offset, data);
			break;
		case 2:	/* LC1 */
			break;
		case 3:	/* LC2 */
			break;
		case 4:	/* ZP1 */
			m68k_zp1 = data;
			break;
		case 5:	/* ZP2 */
			m68k_zp2 = data;
			break;
		case 6:	/* /GSPRES */
			logerror("Write to /GSPRES(%d)\n", data);
			if (hdcpu_gsp != NULL)
				cpu_set_input_line(hdcpu_gsp, INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
		case 7:	/* /MSPRES */
			logerror("Write to /MSPRES(%d)\n", data);
			if (hdcpu_msp != NULL)
				cpu_set_input_line(hdcpu_msp, INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
	}
}


WRITE16_HANDLER( hdc68k_wheel_edge_reset_w )
{
	/* reset the edge latch */
	hdc68k_wheel_edge = 0;
}



/*************************************
 *
 *  68000 ZRAM access
 *
 *************************************/

READ16_HANDLER( hd68k_zram_r )
{
	return atarigen_eeprom[offset];
}


WRITE16_HANDLER( hd68k_zram_w )
{
	if (m68k_zp1 == 0 && m68k_zp2 == 1)
		COMBINE_DATA(&atarigen_eeprom[offset]);
}



/*************************************
 *
 *  68000 DUART interface
 *
 *************************************/

/*
                                    DUART registers

            Read                                Write
            ----------------------------------  -------------------------------------------
    0x00 =  Mode Register A (MR1A, MR2A)        Mode Register A (MR1A, MR2A)
    0x02 =  Status Register A (SRA)             Clock-Select Register A (CSRA)
    0x04 =  Clock-Select Register A 1 (CSRA)    Command Register A (CRA)
    0x06 =  Receiver Buffer A (RBA)             Transmitter Buffer A (TBA)
    0x08 =  Input Port Change Register (IPCR)   Auxiliary Control Register (ACR)
    0x0a =  Interrupt Status Register (ISR)     Interrupt Mask Register (IMR)
    0x0c =  Counter Mode: Current MSB of        Counter/Timer Upper Register (CTUR)
                    Counter (CUR)
    0x0e =  Counter Mode: Current LSB of        Counter/Timer Lower Register (CTLR)
                    Counter (CLR)
    0x10 = Mode Register B (MR1B, MR2B)         Mode Register B (MR1B, MR2B)
    0x12 = Status Register B (SRB)              Clock-Select Register B (CSRB)
    0x14 = Clock-Select Register B 2 (CSRB)     Command Register B (CRB)
    0x16 = Receiver Buffer B (RBB)              Transmitter Buffer B (TBB)
    0x18 = Interrupt-Vector Register (IVR)      Interrupt-Vector Register (IVR)
    0x1a = Input Port (IP)                      Output Port Configuration Register (OPCR)
    0x1c = Start-Counter Command 3              Output Port Register (OPR): Bit Set Command 3
    0x1e = Stop-Counter Command 3               Output Port Register (OPR): Bit Reset Command 3
*/


INLINE int duart_clock(void)
{
	int mode = (duart_write_data[0x04] >> 4) & 7;
	if (mode != 3)
		logerror("DUART: unsupported clock mode %d\n", mode);
	return DUART_CLOCK / 16;
}


INLINE attotime duart_clock_period(void)
{
	return ATTOTIME_IN_HZ(duart_clock());
}


static TIMER_CALLBACK( duart_callback )
{
	logerror("DUART timer fired\n");
	if (duart_write_data[0x05] & 0x08)
	{
		logerror("DUART interrupt generated\n");
		duart_read_data[0x05] |= 0x08;
		duart_irq_state = (duart_read_data[0x05] & duart_write_data[0x05]) != 0;
		atarigen_update_interrupts(machine);
	}
	timer_adjust_oneshot(duart_timer, attotime_mul(duart_clock_period(), 65536), 0);
}


READ16_HANDLER( hd68k_duart_r )
{
	switch (offset)
	{
		case 0x00:		/* Mode Register A (MR1A, MR2A) */
		case 0x08:		/* Mode Register B (MR1B, MR2B) */
			return (duart_write_data[0x00] << 8) | 0x00ff;
		case 0x01:		/* Status Register A (SRA) */
		case 0x02:		/* Clock-Select Register A 1 (CSRA) */
		case 0x03:		/* Receiver Buffer A (RBA) */
		case 0x04:		/* Input Port Change Register (IPCR) */
		case 0x05:		/* Interrupt Status Register (ISR) */
		case 0x06:		/* Counter Mode: Current MSB of Counter (CUR) */
		case 0x07:		/* Counter Mode: Current LSB of Counter (CLR) */
		case 0x09:		/* Status Register B (SRB) */
		case 0x0a:		/* Clock-Select Register B 2 (CSRB) */
		case 0x0b:		/* Receiver Buffer B (RBB) */
		case 0x0c:		/* Interrupt-Vector Register (IVR) */
		case 0x0d:		/* Input Port (IP) */
			return (duart_read_data[offset] << 8) | 0x00ff;
		case 0x0e:		/* Start-Counter Command 3 */
		{
			int reps = (duart_write_data[0x06] << 8) | duart_write_data[0x07];
			timer_adjust_oneshot(duart_timer, attotime_mul(duart_clock_period(), reps), 0);
			logerror("DUART timer started (period=%f)\n", attotime_to_double(attotime_mul(duart_clock_period(), reps)));
			return 0x00ff;
		}
		case 0x0f:		/* Stop-Counter Command 3 */
			{
				int reps = attotime_to_double(attotime_mul(timer_timeleft(duart_timer), duart_clock()));
				timer_adjust_oneshot(duart_timer, attotime_never, 0);
				duart_read_data[0x06] = reps >> 8;
				duart_read_data[0x07] = reps & 0xff;
				logerror("DUART timer stopped (final count=%04X)\n", reps);
			}
			duart_read_data[0x05] &= ~0x08;
			duart_irq_state = (duart_read_data[0x05] & duart_write_data[0x05]) != 0;
			atarigen_update_interrupts(space->machine);
			return 0x00ff;
	}
	return 0x00ff;
}


WRITE16_HANDLER( hd68k_duart_w )
{
	if (ACCESSING_BITS_8_15)
	{
		int newdata = (data >> 8) & 0xff;
		duart_write_data[offset] = newdata;

		switch (offset)
		{
			case 0x00:		/* Mode Register A (MR1A, MR2A) */
			case 0x01:		/* Clock-Select Register A (CSRA) */
			case 0x02:		/* Command Register A (CRA) */
			case 0x03:		/* Transmitter Buffer A (TBA) */
			case 0x04:		/* Auxiliary Control Register (ACR) */
			case 0x05:		/* Interrupt Mask Register (IMR) */
			case 0x06:		/* Counter/Timer Upper Register (CTUR) */
			case 0x07:		/* Counter/Timer Lower Register (CTLR) */
			case 0x08:		/* Mode Register B (MR1B, MR2B) */
			case 0x09:		/* Clock-Select Register B (CSRB) */
			case 0x0a:		/* Command Register B (CRB) */
			case 0x0b:		/* Transmitter Buffer B (TBB) */
			case 0x0c:		/* Interrupt-Vector Register (IVR) */
			case 0x0d:		/* Output Port Configuration Register (OPCR) */
				break;
			case 0x0e:		/* Output Port Register (OPR): Bit Set Command 3 */
				duart_output_port |= newdata;
				break;
			case 0x0f:		/* Output Port Register (OPR): Bit Reset Command 3 */
				duart_output_port &= ~newdata;
				break;
		}
		logerror("DUART write %02X @ %02X\n", (data >> 8) & 0xff, offset);
	}
	else
		logerror("Unexpected DUART write %02X @ %02X\n", data, offset);
}



/*************************************
 *
 *  GSP I/O register writes
 *
 *************************************/

WRITE16_HANDLER( hdgsp_io_w )
{
	/* detect an enabling of the shift register and force yielding */
	if (offset == REG_DPYCTL)
	{
		UINT8 new_shiftreg = (data >> 11) & 1;
		if (new_shiftreg != last_gsp_shiftreg)
		{
			last_gsp_shiftreg = new_shiftreg;
			if (new_shiftreg)
				cpu_yield(space->cpu);
		}
	}

	/* detect changes to HEBLNK and HSBLNK and force an update before they change */
	if ((offset == REG_HEBLNK || offset == REG_HSBLNK) && data != tms34010_io_register_r(space, offset, 0xffff))
		video_screen_update_partial(space->machine->primary_screen, video_screen_get_vpos(space->machine->primary_screen) - 1);

	tms34010_io_register_w(space, offset, data, mem_mask);
}



/*************************************
 *
 *  GSP protection workarounds
 *
 *************************************/

WRITE16_HANDLER( hdgsp_protection_w )
{
	/* this memory address is incremented whenever a protection check fails */
	/* after it reaches a certain value, the GSP will randomly trash a */
	/* register; we just prevent it from ever going above 0 */
	*hdgsp_protection = 0;
}



/*************************************
 *
 *  MSP synchronization helpers
 *
 *************************************/

static TIMER_CALLBACK( stmsp_sync_update )
{
	int which = param >> 28;
	offs_t offset = (param >> 16) & 0xfff;
	UINT16 data = param;
	stmsp_sync[which][offset] = data;
	cpu_triggerint(hdcpu_msp);
}


INLINE void stmsp_sync_w(const address_space *space, offs_t offset, UINT16 data, UINT16 mem_mask, int which)
{
	UINT16 newdata = stmsp_sync[which][offset];
	COMBINE_DATA(&newdata);

	/* if being written from the 68000, synchronize on it */
	if (hd34010_host_access)
		timer_call_after_resynch(space->machine, NULL, newdata | (offset << 16) | (which << 28), stmsp_sync_update);

	/* otherwise, just update */
	else
		stmsp_sync[which][offset] = newdata;
}


WRITE16_HANDLER( stmsp_sync0_w )
{
	stmsp_sync_w(space, offset, data, mem_mask, 0);
}


WRITE16_HANDLER( stmsp_sync1_w )
{
	stmsp_sync_w(space, offset, data, mem_mask, 1);
}


WRITE16_HANDLER( stmsp_sync2_w )
{
	stmsp_sync_w(space, offset, data, mem_mask, 2);
}



#if 0
#pragma mark -
#pragma mark * ADSP BOARD
#endif

/*************************************
 *
 *  68000 access to ADSP program memory
 *
 *************************************/

READ16_HANDLER( hd68k_adsp_program_r )
{
	UINT32 word = hdadsp_pgm_memory[offset/2];
	return (!(offset & 1)) ? (word >> 16) : (word & 0xffff);
}


WRITE16_HANDLER( hd68k_adsp_program_w )
{
	UINT32 *base = &hdadsp_pgm_memory[offset/2];
	UINT32 oldword = *base;
	UINT16 temp;

	if (!(offset & 1))
	{
		temp = oldword >> 16;
		COMBINE_DATA(&temp);
		oldword = (oldword & 0x0000ffff) | (temp << 16);
	}
	else
	{
		temp = oldword & 0xffff;
		COMBINE_DATA(&temp);
		oldword = (oldword & 0xffff0000) | temp;
	}
	*base = oldword;
}



/*************************************
 *
 *  68000 access to ADSP data memory
 *
 *************************************/

READ16_HANDLER( hd68k_adsp_data_r )
{
	return hdadsp_data_memory[offset];
}


WRITE16_HANDLER( hd68k_adsp_data_w )
{
	COMBINE_DATA(&hdadsp_data_memory[offset]);

	/* any write to $1FFF is taken to be a trigger; synchronize the CPUs */
	if (offset == 0x1fff)
	{
		logerror("%06X:ADSP sync address written (%04X)\n", cpu_get_previouspc(space->cpu), data);
		timer_call_after_resynch(space->machine, NULL, 0, 0);
		cpu_triggerint(hdcpu_adsp);
	}
	else
		logerror("%06X:ADSP W@%04X (%04X)\n", cpu_get_previouspc(space->cpu), offset, data);
}



/*************************************
 *
 *  68000 access to ADSP output memory
 *
 *************************************/

READ16_HANDLER( hd68k_adsp_buffer_r )
{
/*  logerror("hd68k_adsp_buffer_r(%04X)\n", offset);*/
	return som_memory[m68k_adsp_buffer_bank * 0x2000 + offset];
}


WRITE16_HANDLER( hd68k_adsp_buffer_w )
{
	COMBINE_DATA(&som_memory[m68k_adsp_buffer_bank * 0x2000 + offset]);
}



/*************************************
 *
 *  68000 access to ADSP control regs
 *
 *************************************/

static TIMER_CALLBACK( deferred_adsp_bank_switch )
{
	if (LOG_COMMANDS && m68k_adsp_buffer_bank != param && input_code_pressed(machine, KEYCODE_L))
	{
		static FILE *commands;
		if (!commands) commands = fopen("commands.log", "w");
		if (commands)
		{
			INT16 *base = (INT16 *)&som_memory[param * 0x2000];
			INT16 *end = base + (UINT16)*base;
			INT16 *current = base + 1;
			INT16 *table = base + 1 + (UINT16)*current++;

			fprintf(commands, "\n---------------\n");

			while ((current + 5) < table)
			{
				int offset = (int)(current - base);
				int c1 = *current++;
				int c2 = *current++;
				int c3 = *current++;
				int c4 = *current++;
				fprintf(commands, "Cmd @ %04X = %04X  %d-%d @ %d\n", offset, c1, c2, c3, c4);
				while (current < table)
				{
					UINT32 rslope, lslope;
					rslope = (UINT16)*current++,
					rslope |= *current++ << 16;
					if (rslope == 0xffffffff)
					{
						fprintf(commands, "  (end)\n");
						break;
					}
					lslope = (UINT16)*current++,
					lslope |= *current++ << 16;
					fprintf(commands, "  L=%08X R=%08X count=%d\n",
							(int)lslope, (int)rslope, (int)*current++);
				}
			}
			fprintf(commands, "\nTable:\n");
			current = table;
			while (current < end)
				fprintf(commands, "  %04X\n", *current++);
		}
	}

	m68k_adsp_buffer_bank = param;
	logerror("ADSP bank = %d\n", param);
}


WRITE16_HANDLER( hd68k_adsp_control_w )
{
	/* bit 3 selects the value; data is ignored */
	int val = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0:
		case 1:
			/* LEDs */
			break;

		case 3:
			logerror("ADSP bank = %d (deferred)\n", val);
			timer_call_after_resynch(space->machine, NULL, val, deferred_adsp_bank_switch);
			break;

		case 5:
			/* connected to the /BR (bus request) line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			adsp_br = !val;
			logerror("ADSP /BR = %d\n", !adsp_br);
			if (adsp_br || adsp_halt)
				cpu_set_input_line(hdcpu_adsp, INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				cpu_set_input_line(hdcpu_adsp, INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				cpu_spin(space->cpu);
			}
			break;

		case 6:
			/* connected to the /HALT line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			adsp_halt = !val;
			logerror("ADSP /HALT = %d\n", !adsp_halt);
			if (adsp_br || adsp_halt)
				cpu_set_input_line(hdcpu_adsp, INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				cpu_set_input_line(hdcpu_adsp, INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				cpu_spin(space->cpu);
			}
			break;

		case 7:
			logerror("ADSP reset = %d\n", val);
			cpu_set_input_line(hdcpu_adsp, INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			cpu_yield(space->cpu);
			break;

		default:
			logerror("ADSP control %02X = %04X\n", offset, data);
			break;
	}
}


WRITE16_HANDLER( hd68k_adsp_irq_clear_w )
{
	logerror("%06X:68k clears ADSP interrupt\n", cpu_get_previouspc(space->cpu));
	adsp_irq_state = 0;
	atarigen_update_interrupts(space->machine);
}


READ16_HANDLER( hd68k_adsp_irq_state_r )
{
	int result = 0xfffd;
	if (adsp_xflag) result ^= 2;
	if (adsp_irq_state) result ^= 1;
	logerror("%06X:68k reads ADSP interrupt state = %04x\n", cpu_get_previouspc(space->cpu), result);
	return result;
}



/*************************************
 *
 *  ADSP memory-mapped I/O
 *
 *************************************/

READ16_HANDLER( hdadsp_special_r )
{
	switch (offset & 7)
	{
		case 0:	/* /SIMBUF */
			if (adsp_eprom_base + adsp_sim_address < sim_memory_size)
				return sim_memory[adsp_eprom_base + adsp_sim_address++];
			else
				return 0xff;

		case 1:	/* /SIMLD */
			break;

		case 2:	/* /SOMO */
			break;

		case 3:	/* /SOMLD */
			break;

		default:
			logerror("%04X:hdadsp_special_r(%04X)\n", cpu_get_previouspc(space->cpu), offset);
			break;
	}
	return 0;
}


WRITE16_HANDLER( hdadsp_special_w )
{
	switch (offset & 7)
	{
		case 1:	/* /SIMCLK */
			adsp_sim_address = data;
			break;

		case 2:	/* SOMLATCH */
			som_memory[(m68k_adsp_buffer_bank ^ 1) * 0x2000 + (adsp_som_address++ & 0x1fff)] = data;
			break;

		case 3:	/* /SOMCLK */
			adsp_som_address = data;
			break;

		case 5:	/* /XOUT */
			adsp_xflag = data & 1;
			break;

		case 6:	/* /GINT */
			logerror("%04X:ADSP signals interrupt\n", cpu_get_previouspc(space->cpu));
			adsp_irq_state = 1;
			atarigen_update_interrupts(space->machine);
			break;

		case 7:	/* /MP */
			adsp_eprom_base = 0x10000 * data;
			break;

		default:
			logerror("%04X:hdadsp_special_w(%04X)=%04X\n", cpu_get_previouspc(space->cpu), offset, data);
			break;
	}
}



#if 0
#pragma mark -
#pragma mark * DS III BOARD
#endif

/*************************************
 *
 *  General DS III I/O
 *
 *************************************/

static void update_ds3_irq(void)
{
	/* update the IRQ2 signal to the ADSP2101 */
	if (!(!ds3_g68flag && ds3_g68irqs) && !(ds3_gflag && ds3_gfirqs))
		cpu_set_input_line(hdcpu_adsp, ADSP2100_IRQ2, ASSERT_LINE);
	else
		cpu_set_input_line(hdcpu_adsp, ADSP2100_IRQ2, CLEAR_LINE);
}


WRITE16_HANDLER( hd68k_ds3_control_w )
{
	int val = (offset >> 3) & 1;

	switch (offset & 7)
	{
		case 0:
			/* SRES - reset sound CPU */
			break;

		case 1:
			/* XRES - reset sound helper CPU */
			break;

		case 2:
			/* connected to the /BR (bus request) line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			adsp_br = !val;
			if (adsp_br)
				cpu_set_input_line(hdcpu_adsp, INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				cpu_set_input_line(hdcpu_adsp, INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				cpu_spin(space->cpu);
			}
			break;

		case 3:
			cpu_set_input_line(hdcpu_adsp, INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			if (val && !ds3_reset)
			{
				ds3_gflag = 0;
				ds3_gcmd = 0;
				ds3_gfirqs = 0;
				ds3_g68irqs = !ds3_gfirqs;
				ds3_send = 0;
				update_ds3_irq();
			}
			ds3_reset = val;
			cpu_yield(space->cpu);
			logerror("DS III reset = %d\n", val);
			break;

		case 7:
			/* LED */
			break;

		default:
			logerror("DS III control %02X = %04X\n", offset, data);
			break;
	}
}



/*************************************
 *
 *  DS III graphics I/O
 *
 *************************************/

READ16_HANDLER( hd68k_ds3_girq_state_r )
{
	int result = 0x0fff;
	if (ds3_g68flag) result ^= 0x8000;
	if (ds3_gflag) result ^= 0x4000;
	if (ds3_g68irqs) result ^= 0x2000;
	if (!adsp_irq_state) result ^= 0x1000;
	return result;
}


READ16_HANDLER( hd68k_ds3_gdata_r )
{
	offs_t pc = cpu_get_pc(space->cpu);

	ds3_gflag = 0;
	update_ds3_irq();

	logerror("%06X:hd68k_ds3_gdata_r(%04X)\n", cpu_get_previouspc(space->cpu), ds3_gdata);

	/* attempt to optimize the transfer if conditions are right */
	if (space->cpu == cputag_get_cpu(space->machine, "maincpu") && pc == hdds3_transfer_pc &&
		!(!ds3_g68flag && ds3_g68irqs) && !(ds3_gflag && ds3_gfirqs))
	{
		UINT32 destaddr = cpu_get_reg(space->cpu, M68K_A1);
		UINT16 count68k = cpu_get_reg(space->cpu, M68K_D1);
		UINT16 mstat = cpu_get_reg(hdcpu_adsp, ADSP2100_MSTAT);
		UINT16 i6 = cpu_get_reg(hdcpu_adsp, (mstat & 1) ? ADSP2100_MR0 : ADSP2100_MR0_SEC);
		UINT16 l6 = cpu_get_reg(hdcpu_adsp, ADSP2100_L6) - 1;
		UINT16 m7 = cpu_get_reg(hdcpu_adsp, ADSP2100_M7);

		logerror("%06X:optimizing 68k transfer, %d words\n", cpu_get_previouspc(space->cpu), count68k);

		while (count68k > 0 && hdadsp_data_memory[0x16e6] > 0)
		{
			memory_write_word(space, destaddr, ds3_gdata);
			{
				hdadsp_data_memory[0x16e6]--;
				ds3_gdata = hdadsp_pgm_memory[i6] >> 8;
				i6 = (i6 & ~l6) | ((i6 + m7) & l6);
			}
			count68k--;
		}
		cpu_set_reg(space->cpu, M68K_D1, count68k);
		cpu_set_reg(hdcpu_adsp, (mstat & 1) ? ADSP2100_MR0 : ADSP2100_MR0_SEC, i6);
		adsp_speedup_count[1]++;
	}

	/* if we just cleared the IRQ, we are going to do some VERY timing critical reads */
	/* it is important that all the CPUs be in sync before we continue, so spin a little */
	/* while to let everyone else catch up */
	cpu_spinuntil_trigger(space->cpu, DS3_TRIGGER);
	cpuexec_triggertime(space->machine, DS3_TRIGGER, ATTOTIME_IN_USEC(5));

	return ds3_gdata;
}


WRITE16_HANDLER( hd68k_ds3_gdata_w )
{
	logerror("%06X:hd68k_ds3_gdata_w(%04X)\n", cpu_get_previouspc(space->cpu), ds3_gdata);

	COMBINE_DATA(&ds3_g68data);
	ds3_g68flag = 1;
	ds3_gcmd = offset & 1;
	cpu_triggerint(hdcpu_adsp);
	update_ds3_irq();
}



/*************************************
 *
 *  DS III sound I/O
 *
 *************************************/

READ16_HANDLER( hd68k_ds3_sirq_state_r )
{
	return 0x4000;
}


READ16_HANDLER( hd68k_ds3_sdata_r )
{
	return 0;
}


WRITE16_HANDLER( hd68k_ds3_sdata_w )
{
}


/*************************************
 *
 *  DS III internal I/O
 *
 *************************************/

READ16_HANDLER( hdds3_special_r )
{
	int result;

	switch (offset & 7)
	{
		case 0:
			ds3_g68flag = 0;
			update_ds3_irq();
			return ds3_g68data;

		case 1:
			result = 0x0fff;
			if (ds3_gcmd) result ^= 0x8000;
			if (ds3_g68flag) result ^= 0x4000;
			if (ds3_gflag) result ^= 0x2000;
			return result;

		case 6:
			logerror("ADSP r @ %04x\n", ds3_sim_address);
			if (ds3_sim_address < sim_memory_size)
				return sim_memory[ds3_sim_address];
			else
				return 0xff;
	}
	return 0;
}


WRITE16_HANDLER( hdds3_special_w )
{
	/* IMPORTANT! these data values also write through to the underlying RAM */
	hdadsp_data_memory[offset] = data;

	switch (offset & 7)
	{
		case 0:
			logerror("%04X:ADSP sets gdata to %04X\n", cpu_get_previouspc(space->cpu), data);
			ds3_gdata = data;
			ds3_gflag = 1;
			update_ds3_irq();

			/* once we've written data, trigger the main CPU to wake up again */
			cpuexec_trigger(space->machine, DS3_TRIGGER);
			break;

		case 1:
			logerror("%04X:ADSP sets interrupt = %d\n", cpu_get_previouspc(space->cpu), (data >> 1) & 1);
			adsp_irq_state = (data >> 1) & 1;
			hd68k_update_interrupts(space->machine);
			break;

		case 2:
			ds3_send = (data >> 0) & 1;
			break;

		case 3:
			ds3_gfirqs = (data >> 1) & 1;
			ds3_g68irqs = !ds3_gfirqs;
			update_ds3_irq();
			break;

		case 4:
			ds3_sim_address = (ds3_sim_address & 0xffff0000) | (data & 0xffff);
			break;

		case 5:
			ds3_sim_address = (ds3_sim_address & 0xffff) | ((data << 16) & 0x00070000);
			break;
	}
}


READ16_HANDLER( hdds3_control_r )
{
	logerror("adsp2101 control r @ %04X\n", 0x3fe0 + offset);
	return 0;
}


WRITE16_HANDLER( hdds3_control_w )
{
	if (offset != 0x1e && offset != 0x1f)
		logerror("adsp2101 control w @ %04X = %04X\n", 0x3fe0 + offset, data);
}



/*************************************
 *
 *  DS III program memory handlers
 *
 *************************************/

READ16_HANDLER( hd68k_ds3_program_r )
{
	UINT32 *base = &hdadsp_pgm_memory[offset & 0x1fff];
	UINT32 word = *base;
	return (!(offset & 0x2000)) ? (word >> 8) : (word & 0xff);
}


WRITE16_HANDLER( hd68k_ds3_program_w )
{
	UINT32 *base = &hdadsp_pgm_memory[offset & 0x1fff];
	UINT32 oldword = *base;
	UINT16 temp;

	if (!(offset & 0x2000))
	{
		temp = oldword >> 8;
		COMBINE_DATA(&temp);
		oldword = (oldword & 0x000000ff) | (temp << 8);
	}
	else
	{
		temp = oldword & 0xff;
		COMBINE_DATA(&temp);
		oldword = (oldword & 0xffffff00) | (temp & 0xff);
	}
	*base = oldword;
}



#if 0
#pragma mark -
#pragma mark * DSK BOARD
#endif

/*************************************
 *
 *  DSK board IRQ generation
 *
 *************************************/

void hddsk_update_pif(const device_config *device, UINT32 pins)
{
	atarigen_sound_int_state = ((pins & DSP32_OUTPUT_PIF) != 0);
	hd68k_update_interrupts(device->machine);
}



/*************************************
 *
 *  DSK board control handlers
 *
 *************************************/

WRITE16_HANDLER( hd68k_dsk_control_w )
{
	int val = (offset >> 3) & 1;
	switch (offset & 7)
	{
		case 0:	/* DSPRESTN */
			cpu_set_input_line(hdcpu_dsp32, INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 1:	/* DSPZN */
			cpu_set_input_line(hdcpu_dsp32, INPUT_LINE_HALT, val ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 2:	/* ZW1 */
			break;

		case 3:	/* ZW2 */
			break;

		case 4:	/* ASIC65 reset */
			asic65_reset(space->machine, !val);
			break;

		case 7:	/* LED */
			break;

		default:
			logerror("hd68k_dsk_control_w(%d) = %d\n", offset & 7, val);
			break;
	}
}



/*************************************
 *
 *  DSK board RAM/ZRAM/ROM handlers
 *
 *************************************/

READ16_HANDLER( hd68k_dsk_ram_r )
{
	return hddsk_ram[offset];
}


WRITE16_HANDLER( hd68k_dsk_ram_w )
{
	COMBINE_DATA(&hddsk_ram[offset]);
}


READ16_HANDLER( hd68k_dsk_zram_r )
{
	return hddsk_zram[offset];
}


WRITE16_HANDLER( hd68k_dsk_zram_w )
{
	COMBINE_DATA(&hddsk_zram[offset]);
}


READ16_HANDLER( hd68k_dsk_small_rom_r )
{
	return hddsk_rom[offset & 0x1ffff];
}


READ16_HANDLER( hd68k_dsk_rom_r )
{
	return hddsk_rom[offset];
}



/*************************************
 *
 *  DSK board DSP32C I/O handlers
 *
 *************************************/

WRITE16_HANDLER( hd68k_dsk_dsp32_w )
{
	hddsk_pio_access = 1;
	dsp32c_pio_w(hdcpu_dsp32, offset, data);
	hddsk_pio_access = 0;
}


READ16_HANDLER( hd68k_dsk_dsp32_r )
{
	UINT16 result;
	hddsk_pio_access = 1;
	result = dsp32c_pio_r(hdcpu_dsp32, offset);
	hddsk_pio_access = 0;
	return result;
}


/*************************************
 *
 *  DSP32C synchronization
 *
 *************************************/

static TIMER_CALLBACK( rddsp32_sync_cb )
{
	*dataptr[param] = dataval[param];
}


WRITE32_HANDLER( rddsp32_sync0_w )
{
	if (hddsk_pio_access)
	{
		UINT32 *dptr = &rddsp32_sync[0][offset];
		UINT32 newdata = *dptr;
		COMBINE_DATA(&newdata);
		dataptr[next_msp_sync % MAX_MSP_SYNC] = dptr;
		dataval[next_msp_sync % MAX_MSP_SYNC] = newdata;
		timer_call_after_resynch(space->machine, NULL, next_msp_sync++ % MAX_MSP_SYNC, rddsp32_sync_cb);
	}
	else
		COMBINE_DATA(&rddsp32_sync[0][offset]);
}


WRITE32_HANDLER( rddsp32_sync1_w )
{
	if (hddsk_pio_access)
	{
		UINT32 *dptr = &rddsp32_sync[1][offset];
		UINT32 newdata = *dptr;
		COMBINE_DATA(&newdata);
		dataptr[next_msp_sync % MAX_MSP_SYNC] = dptr;
		dataval[next_msp_sync % MAX_MSP_SYNC] = newdata;
		timer_call_after_resynch(space->machine, NULL, next_msp_sync++ % MAX_MSP_SYNC, rddsp32_sync_cb);
	}
	else
		COMBINE_DATA(&rddsp32_sync[1][offset]);
}



#if 0
#pragma mark -
#pragma mark * DSPCOM BOARD
#endif

/*************************************
 *
 *  DSPCOM control handlers
 *
 *************************************/

WRITE16_HANDLER( hddspcom_control_w )
{
	int val = (offset >> 3) & 1;
	switch (offset & 7)
	{
		case 2:	/* ASIC65 reset */
			asic65_reset(space->machine, !val);
			break;

		default:
			logerror("hddspcom_control_w(%d) = %d\n", offset & 7, val);
			break;
	}
}



#if 0
#pragma mark -
#pragma mark * GAME-SPECIFIC PROTECTION
#endif

/*************************************
 *
 *  Race Drivin' slapstic handling
 *
 *************************************/

WRITE16_HANDLER( rd68k_slapstic_w )
{
	slapstic_tweak(space, offset & 0x3fff);
}


READ16_HANDLER( rd68k_slapstic_r )
{
	int bank = slapstic_tweak(space, offset & 0x3fff) * 0x4000;
	return hd68k_slapstic_base[bank + (offset & 0x3fff)];
}



/*************************************
 *
 *  Steel Talons SLOOP handling
 *
 *************************************/

static int st68k_sloop_tweak(offs_t offset)
{
	static int last_offset;

	if (last_offset == 0)
	{
		switch (offset)
		{
			case 0x78e8:
				st68k_sloop_bank = 0;
				break;
			case 0x6ca4:
				st68k_sloop_bank = 1;
				break;
			case 0x15ea:
				st68k_sloop_bank = 2;
				break;
			case 0x6b28:
				st68k_sloop_bank = 3;
				break;
		}
	}
	last_offset = offset;
	return st68k_sloop_bank;
}


WRITE16_HANDLER( st68k_sloop_w )
{
	st68k_sloop_tweak(offset & 0x3fff);
}


READ16_HANDLER( st68k_sloop_r )
{
	int bank = st68k_sloop_tweak(offset) * 0x4000;
	return hd68k_slapstic_base[bank + (offset & 0x3fff)];
}


READ16_HANDLER( st68k_sloop_alt_r )
{
	if (st68k_last_alt_sloop_offset == 0x00fe)
	{
		switch (offset*2)
		{
			case 0x22c:
				st68k_sloop_bank = 0;
				break;
			case 0x1e2:
				st68k_sloop_bank = 1;
				break;
			case 0x1fa:
				st68k_sloop_bank = 2;
				break;
			case 0x206:
				st68k_sloop_bank = 3;
				break;
		}
	}
	st68k_last_alt_sloop_offset = offset*2;
	return st68k_sloop_alt_base[offset];
}


static int st68k_protosloop_tweak(offs_t offset)
{
	static int last_offset;

	if (last_offset == 0)
	{
		switch (offset)
		{
			case 0x0001:
				st68k_sloop_bank = 0;
				break;
			case 0x0002:
				st68k_sloop_bank = 1;
				break;
			case 0x0003:
				st68k_sloop_bank = 2;
				break;
			case 0x0004:
				st68k_sloop_bank = 3;
				break;
		}
	}
	last_offset = offset;
	return st68k_sloop_bank;
}


WRITE16_HANDLER( st68k_protosloop_w )
{
	st68k_protosloop_tweak(offset & 0x3fff);
}


READ16_HANDLER( st68k_protosloop_r )
{
	int bank = st68k_protosloop_tweak(offset) * 0x4000;
	return hd68k_slapstic_base[bank + (offset & 0x3fff)];
}



#if 0
#pragma mark -
#pragma mark * GSP OPTIMIZATIONS
#endif

/*************************************
 *
 *  GSP Optimizations - case 1
 *  Works for:
 *      Hard Drivin'
 *      STUN Runner
 *
 *************************************/

READ16_HANDLER( hdgsp_speedup_r )
{
	int result = hdgsp_speedup_addr[0][offset];

	/* if both this address and the other important address are not $ffff */
	/* then we can spin until something gets written */
	if (result != 0xffff && hdgsp_speedup_addr[1][0] != 0xffff &&
		space->cpu == hdcpu_gsp && cpu_get_pc(space->cpu) == hdgsp_speedup_pc)
	{
		gsp_speedup_count[0]++;
		cpu_spinuntil_int(space->cpu);
	}

	return result;
}


WRITE16_HANDLER( hdgsp_speedup1_w )
{
	COMBINE_DATA(&hdgsp_speedup_addr[0][offset]);

	/* if $ffff is written, send an "interrupt" trigger to break us out of the spin loop */
	if (hdgsp_speedup_addr[0][offset] == 0xffff)
		cpu_triggerint(hdcpu_gsp);
}


WRITE16_HANDLER( hdgsp_speedup2_w )
{
	COMBINE_DATA(&hdgsp_speedup_addr[1][offset]);

	/* if $ffff is written, send an "interrupt" trigger to break us out of the spin loop */
	if (hdgsp_speedup_addr[1][offset] == 0xffff)
		cpu_triggerint(hdcpu_gsp);
}



/*************************************
 *
 *  GSP Optimizations - case 2
 *  Works for:
 *      Race Drivin'
 *
 *************************************/

READ16_HANDLER( rdgsp_speedup1_r )
{
	int result = hdgsp_speedup_addr[0][offset];

	/* if this address is equal to $f000, spin until something gets written */
	if (space->cpu == hdcpu_gsp && cpu_get_pc(space->cpu) == hdgsp_speedup_pc &&
		(result & 0xff) < cpu_get_reg(space->cpu, TMS34010_A1))
	{
		gsp_speedup_count[0]++;
		cpu_spinuntil_int(space->cpu);
	}

	return result;
}


WRITE16_HANDLER( rdgsp_speedup1_w )
{
	COMBINE_DATA(&hdgsp_speedup_addr[0][offset]);
	if (space->cpu != hdcpu_gsp)
		cpu_triggerint(hdcpu_gsp);
}



#if 0
#pragma mark -
#pragma mark * MSP OPTIMIZATIONS
#endif

/*************************************
 *
 *  MSP Optimizations
 *
 *************************************/

READ16_HANDLER( hdmsp_speedup_r )
{
	int data = hdmsp_speedup_addr[offset];

	if (data == 0 && space->cpu == hdcpu_msp && cpu_get_pc(space->cpu) == hdmsp_speedup_pc)
	{
		msp_speedup_count[0]++;
		cpu_spinuntil_int(space->cpu);
	}

	return data;
}


WRITE16_HANDLER( hdmsp_speedup_w )
{
	COMBINE_DATA(&hdmsp_speedup_addr[offset]);
	if (offset == 0 && hdmsp_speedup_addr[offset] != 0)
		cpu_triggerint(hdcpu_msp);
}


READ16_HANDLER( stmsp_speedup_r )
{
	/* assumes: stmsp_sync[0] -> $80010, stmsp_sync[1] -> $99680, stmsp_sync[2] -> $99d30 */
	if (stmsp_sync[0][0] == 0 &&		/* 80010 */
		stmsp_sync[0][1] == 0 &&		/* 80020 */
		stmsp_sync[0][2] == 0 &&		/* 80030 */
		stmsp_sync[0][3] == 0 &&		/* 80040 */
		stmsp_sync[0][4] == 0 &&		/* 80050 */
		stmsp_sync[0][5] == 0 && 		/* 80060 */
		stmsp_sync[0][6] == 0 && 		/* 80070 */
		stmsp_sync[1][0] == 0 && 		/* 99680 */
		stmsp_sync[2][0] == 0xffff && 	/* 99d30 */
		stmsp_sync[2][1] == 0xffff && 	/* 99d40 */
		stmsp_sync[2][2] == 0 &&	 	/* 99d50 */
		cpu_get_pc(space->cpu) == 0x3c0)
	{
		msp_speedup_count[0]++;
		cpu_spinuntil_int(space->cpu);
	}
	return stmsp_sync[0][1];
}



#if 0
#pragma mark -
#pragma mark * ADSP OPTIMIZATIONS
#endif

/*************************************
 *
 *  ADSP Optimizations
 *
 *************************************/

READ16_HANDLER( hdadsp_speedup_r )
{
	int data = hdadsp_data_memory[0x1fff];

	if (data == 0xffff && space->cpu == hdcpu_adsp && cpu_get_pc(space->cpu) <= 0x3b)
	{
		adsp_speedup_count[0]++;
		cpu_spinuntil_int(space->cpu);
	}

	return data;
}


READ16_HANDLER( hdds3_speedup_r )
{
	int data = *hdds3_speedup_addr;

	if (data != 0 && space->cpu == hdcpu_adsp && cpu_get_pc(space->cpu) == hdds3_speedup_pc)
	{
		adsp_speedup_count[2]++;
		cpu_spinuntil_int(space->cpu);
	}

	return data;
}
