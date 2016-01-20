// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "includes/newbrain.h"
#include "formats/mfi_dsk.h"

/*

    NewBrain
    Grundy Business Systems Ltd.

    32K RAM
    28K ROM

    Z80 @ 2MHz
    COP420 @ 2MHz

    Z80 @ 4MHz (416): INT/NMI=+5V, WAIT=EXTBUSRQ|BUSAKD, RESET=_FDC RESET,
    NEC 765AC @ 4 MHz (418)
    MC6850 ACIA (459)
    Z80CTC (458)
    ADC0809 (427)
    DAC0808 (461)

    Models according to the Software Technical Manual:

    Model M: 'Page Register', Expansion Port onto Z80 bus, Video, ACIA/CTC, User Port
    Model A: Expansion Port, Video, no User Port but has software driver serial port - s/w Printer, s/w V24
    Model V: ACIA/CTC, User Port

*/

/*

    TODO:

    - bitmapped graphics mode
    - COP420 microbus access
    - escape key is missing
    - CP/M 2.2 ROMs
    - floppy disc controller
    - convert FDC into a device
    - convert EIM into a device

    - Micropage ROM/RAM card
    - Z80 PIO board
    - peripheral (PI) box
    - sound card

*/

void newbrain_state::check_interrupt()
{
	int level = (!m_clkint || !m_copint) ? ASSERT_LINE : CLEAR_LINE;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, level);
}

/* Bank Switching */

#define memory_install_unmapped(program, bank, bank_start, bank_end) \
	program.unmap_readwrite(bank_start, bank_end);

#define memory_install_rom_helper(program, bank, bank_start, bank_end) \
	program.install_read_bank(bank_start, bank_end, bank); \
	program.unmap_write(bank_start, bank_end);

#define memory_install_ram_helper(program, bank, bank_start, bank_end) \
	program.install_readwrite_bank(bank_start, bank_end, bank);

void newbrain_eim_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	int bank;

	for (bank = 1; bank < 9; bank++)
	{
		int page = (m_a16 << 3) | bank;
		UINT8 data = ~m_pr[page];
		int ch = (data >> 3) & 0x03;
		int eim_bank = data & 0x07;

		UINT16 eim_bank_start = eim_bank * 0x2000;
		UINT16 bank_start = (bank - 1) * 0x2000;
		UINT16 bank_end = bank_start + 0x1fff;
		char bank_name[10];
		sprintf(bank_name,"bank%d",bank);
		switch (ch)
		{
		case 0:
			/* ROM */
			memory_install_rom_helper(program, bank_name, bank_start, bank_end);
			membank(bank_name)->configure_entry(0, m_eim_rom->base() + eim_bank_start);
			break;

		case 2:
			/* RAM */
			memory_install_ram_helper(program, bank_name, bank_start, bank_end);
			membank(bank_name)->configure_entry(0, m_eim_ram + eim_bank_start);
			break;

		default:
			logerror("Invalid memory channel %u!\n", ch);
			break;
		}
	}
}

void newbrain_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	int bank;

	for (bank = 1; bank < 9; bank++)
	{
		UINT16 bank_start = (bank - 1) * 0x2000;
		UINT16 bank_end = bank_start + 0x1fff;
		char bank_name[10];
		sprintf(bank_name,"bank%d",bank);

		if (m_pwrup)
		{
			/* all banks point to ROM at 0xe000 */
			memory_install_rom_helper(program, bank_name, bank_start, bank_end);
			membank(bank_name)->configure_entry(0, m_rom->base() + 0xe000);
		}
		else
		{
			membank(bank_name)->configure_entry(0, m_rom->base() + bank_start);

			if (bank < 5)
			{
				/* bank is RAM */
				memory_install_ram_helper(program, bank_name, bank_start, bank_end);
			}
			else if (bank == 5)
			{
				/* 0x8000-0x9fff */
				if (m_eim_rom)
				{
					/* expansion interface ROM */
					memory_install_rom_helper(program, bank_name, bank_start, bank_end);
					membank(bank_name)->configure_entry(0, m_eim_rom->base() + 0x4000);
				}
				else
				{
					/* mirror of 0xa000-0xbfff */
					if (m_rom->base()[0xa001] == 0)
					{
						/* unmapped on the M model */
						memory_install_unmapped(program, bank_name, bank_start, bank_end);
					}
					else
					{
						/* bank is ROM on the A model */
						memory_install_rom_helper(program, bank_name, bank_start, bank_end);
					}

					membank(bank_name)->configure_entry(0, m_rom->base() + 0xa000);
				}
			}
			else if (bank == 6)
			{
				/* 0xa000-0xbfff */
				if (m_rom->base()[0xa001] == 0)
				{
					/* unmapped on the M model */
					memory_install_unmapped(program, bank_name, bank_start, bank_end);
				}
				else
				{
					/* bank is ROM on the A model */
					memory_install_rom_helper(program, bank_name, bank_start, bank_end);
				}
			}
			else
			{
				/* bank is ROM */
				memory_install_rom_helper(program, bank_name, bank_start, bank_end);
			}
		}

		membank(bank_name)->set_entry(0);
	}
}

/* Enable/Status */

WRITE8_MEMBER( newbrain_state::enrg1_w )
{
	/*

	    bit     signal      description

	    0       _CLK        enable frame frequency clock interrupts
	    1                   enable user interrupt
	    2       TVP         enable video display
	    3                   enable V24
	    4                   V24 Select Receive Bit 0
	    5                   V24 Select Receive Bit 1
	    6                   V24 Select Transmit Bit 0
	    7                   V24 Select Transmit Bit 1

	*/

	m_enrg1 = data;
}

WRITE8_MEMBER( newbrain_state::a_enrg1_w )
{
	/*

	    bit     signal      description

	    0       _CLK        Clock Enable
	    1
	    2       TVP         TV Enable
	    3       IOPOWER
	    4       _CTS        Clear to Send V24
	    5       DO          Transmit Data V24
	    6
	    7       PO          Transmit Data Printer

	*/

	m_enrg1 = data;
}

READ8_MEMBER( newbrain_state::ust_r )
{
	/*

	    bit     signal      description

	    0       variable
	    1       variable
	    2       MNS         mains present
	    3       _USRINT0    user status
	    4       _USRINT     user interrupt
	    5       _CLKINT     clock interrupt
	    6       _ACINT      ACIA interrupt
	    7       _COPINT     COP interrupt

	*/

	UINT8 data = (m_copint << 7) | (m_aciaint << 6) | (m_clkint << 5) | (m_userint << 4) | 0x04;

	switch ((m_enrg1 & NEWBRAIN_ENRG1_UST_BIT_0_MASK) >> 6)
	{
	case 0:
		// excess, 1=24, 0=4
		if (m_tvctl & NEWBRAIN_VIDEO_32_40)
		{
			data |= 0x01;
		}
		break;

	case 1:
		// characters per line, 1=40, 0=80
		if (m_tvctl & NEWBRAIN_VIDEO_80L)
		{
			data |= 0x01;
		}
		break;

	case 2:
		// tape in
		break;

	case 3:
		// calling indicator
		break;
	}

	switch ((m_enrg1 & NEWBRAIN_ENRG1_UST_BIT_1_MASK) >> 4)
	{
	case 0:
		// PWRUP, if set indicates that power is supplied to Z80 and memory
		if (m_pwrup)
		{
			data |= 0x02;
		}
		break;

	case 1:
		// TVCNSL, if set then processor has video device as primary console output
		if (m_tvcnsl)
		{
			data |= 0x02;
		}
		break;

	case 2:
		// _BEE, if set then processor is Model A type
		if (m_bee)
		{
			data |= 0x02;
		}
		break;

	case 3:
		// _CALLIND, calling indicator
		data |= 0x02;
		break;
	}

	return data;
}

READ8_MEMBER( newbrain_state::a_ust_r )
{
	/*

	    bit     signal      description

	    0                   +5V
	    1       PWRUP
	    2
	    3
	    4
	    5       _CLKINT     clock interrupt
	    6
	    7       _COPINT     COP interrupt

	*/

	return (m_copint << 7) | (m_clkint << 5) | (m_pwrup << 1) | 0x01;
}

READ8_MEMBER( newbrain_state::user_r )
{
	/*

	    bit     signal      description

	    0       RDDK        Received Data V24
	    1       _CTSD       _Clear to Send V24
	    2
	    3
	    4
	    5       TPIN        Tape in
	    6
	    7       _CTSP       _Clear to Send Printer

	*/

	m_user = 0;

	return 0xff;
}

WRITE8_MEMBER( newbrain_state::user_w )
{
	m_user = data;
}

/* Interrupts */

READ8_MEMBER( newbrain_state::clclk_r )
{
	m_clkint = 1;
	check_interrupt();

	return 0xff;
}

WRITE8_MEMBER( newbrain_state::clclk_w )
{
	m_clkint = 1;
	check_interrupt();
}

READ8_MEMBER( newbrain_state::clusr_r )
{
	m_userint = 1;

	return 0xff;
}

WRITE8_MEMBER( newbrain_state::clusr_w )
{
	m_userint = 1;
}

/* COP420 */

READ8_MEMBER( newbrain_state::cop_l_r )
{
	// connected to the Z80 data bus
	return m_cop_bus;
}

WRITE8_MEMBER( newbrain_state::cop_l_w )
{
	// connected to the Z80 data bus
	m_cop_bus = data;
}

WRITE8_MEMBER( newbrain_state::cop_g_w )
{
	/*

	    bit     description

	    G0      _COPINT
	    G1      _TM1
	    G2      not connected
	    G3      _TM2

	*/

	m_copint = BIT(data, 0);
	check_interrupt();

	/* tape motor enable */

	m_cassette1->change_state(BIT(data,1) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
	m_cassette2->change_state(BIT(data,3) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
}

READ8_MEMBER( newbrain_state::cop_g_r )
{
	/*

	    bit     description

	    G0      not connected
	    G1      K9
	    G2      K7
	    G3      K3

	*/

	return (BIT(m_keydata, 3) << 3) | (BIT(m_keydata, 0) << 2) | (BIT(m_keydata, 1) << 1);
}

WRITE8_MEMBER( newbrain_state::cop_d_w )
{
	/*
	    bit     description

	    D0      inverted to K4 -> CD4024 pin 2 (reset)
	    D1      TDO
	    D2      inverted to K6 -> CD4024 pin 1 (clock), CD4076 pin 7 (clock), inverted to DS8881 pin 3 (enable)
	    D3      not connected

	*/

	/* keyboard row reset */

	if (!BIT(data, 0))
	{
		m_keylatch = 0;
	}

	/* tape data output */

	m_cop_tdo = BIT(data, 1);

	m_cassette1->output(m_cop_tdo ? -1.0 : +1.0);
	m_cassette2->output(m_cop_tdo ? -1.0 : +1.0);

	/* keyboard and display clock */

	if (!BIT(data, 2))
	{
		m_keylatch++;

		if (m_keylatch == 16)
		{
			m_keylatch = 0;
		}

		m_keydata = m_key_row[m_keylatch]->read();

		output().set_digit_value(m_keylatch, m_segment_data[m_keylatch]);
	}
}

READ8_MEMBER( newbrain_state::cop_in_r )
{
	/*

	    bit     description

	    IN0     K8
	    IN1     _RD
	    IN2     _COP
	    IN3     _WR

	*/

	return (m_cop_wr << 3) | (m_cop_access << 2) | (m_cop_rd << 1) | BIT(m_keydata, 2);
}

WRITE_LINE_MEMBER( newbrain_state::cop_so_w )
{
	// connected to K1
	m_cop_so = state;
}

WRITE_LINE_MEMBER( newbrain_state::cop_sk_w )
{
	// connected to K2
	m_segment_data[m_keylatch] >>= 1;
	m_segment_data[m_keylatch] = (m_cop_so << 15) | (m_segment_data[m_keylatch] & 0x7fff);
}

READ_LINE_MEMBER( newbrain_state::cop_si_r )
{
	// connected to TDI
	m_cop_tdi = (((m_cassette1)->input() > +1.0) || ((m_cassette2)->input() > +1.0)) ^ m_cop_tdo;

	return m_cop_tdi;
}

/* Video */

void newbrain_state::tvram_w(UINT8 data, int a6)
{
	/* latch video address counter bits A5-A0 */
	m_tvram = (m_tvctl & NEWBRAIN_VIDEO_80L) ? 0x04 : 0x02;

	/* latch video address counter bit A6 */
	m_tvram |= a6 << 6;

	/* latch data to video address counter bits A14-A7 */
	m_tvram = (data << 7);
}

READ8_MEMBER( newbrain_state::tvl_r )
{
	UINT8 data = 0xff;

	tvram_w(data, !offset);

	return data;
}

WRITE8_MEMBER( newbrain_state::tvl_w )
{
	tvram_w(data, !offset);
}

WRITE8_MEMBER( newbrain_state::tvctl_w )
{
	/*

	    bit     signal      description

	    0       RV          1 reverses video over entire field, ie. black on white
	    1       FS          0 generates 128 characters and 128 reverse field characters from 8 bit character code. 1 generates 256 characters from 8 bit character code
	    2       32/_40      0 generates 320 or 640 horizontal dots in pixel graphics mode. 1 generates 256 or 512 horizontal dots in pixel graphics mode
	    3       UCR         0 selects 256 characters expressed in an 8x10 matrix, and 25 lines (max) displayed. 1 selects 256 characters in an 8x8 matrix, and 31 lines (max) displayed
	    4
	    5
	    6       80L         0 selects 40 character line length. 1 selects 80 character line length
	    7

	*/

	m_tvctl = data;
}

/* Disc Controller */

WRITE8_MEMBER( newbrain_eim_state::fdc_auxiliary_w )
{
	/*

	    bit     description

	    0       MOTON
	    1       765 RESET
	    2       TC
	    3
	    4
	    5       PA15
	    6
	    7

	*/

	m_floppy->mon_w(!BIT(data, 0));

	if(BIT(data, 1))
		m_fdc->reset();

	m_fdc->tc_w(BIT(data, 2));
}

READ8_MEMBER( newbrain_eim_state::fdc_control_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       FDC INT
	    6       PAGING
	    7       FDC ATT

	*/

	return (m_fdc_att << 7) | (m_paging << 6) | (m_fdc_int << 5);
}

READ8_MEMBER( newbrain_eim_state::ust2_r )
{
	/*

	    bit     description

	    0       RDDK (V24 RxD)
	    1       _CTSD (V24 Clear to Send)
	    2
	    3
	    4
	    5       TPIN
	    6
	    7       _CTSP (Printer Clear to Send)

	*/

	return 0;
}

#define NEWBRAIN_COPCMD_NULLCOM     0xd0
#define NEWBRAIN_COPCMD_DISPCOM     0xa0
//#define NEWBRAIN_COPCMD_TIMCOM        0x
#define NEWBRAIN_COPCMD_PDNCOM      0xb8
#define NEWBRAIN_COPCMD_TAPECOM     0x80

#define NEWBRAIN_COP_TAPE_RECORD    0x00
#define NEWBRAIN_COP_TAPE_PLAYBK    0x04
#define NEWBRAIN_COP_TAPE_MOTOR1    0x08
#define NEWBRAIN_COP_TAPE_MOTOR2    0x02

#define NEWBRAIN_COP_TIMER0         0x01

#define NEWBRAIN_COP_NO_DATA        0x01
#define NEWBRAIN_COP_BREAK_PRESSED  0x02

#define NEWBRAIN_COP_REGINT         0x00
/*#define NEWBRAIN_COP_CASSERR      0x
#define NEWBRAIN_COP_CASSIN         0x
#define NEWBRAIN_COP_KBD            0x
#define NEWBRAIN_COP_CASSOUT        0x*/

enum
{
	NEWBRAIN_COP_STATE_COMMAND = 0,
	NEWBRAIN_COP_STATE_DATA
};


READ8_MEMBER( newbrain_state::cop_r )
{
	m_copint = 1;
	check_interrupt();

	return m_copdata;
}

WRITE8_MEMBER( newbrain_state::cop_w )
{
	m_copdata = data;

	switch (m_copstate)
	{
	case NEWBRAIN_COP_STATE_COMMAND:
		logerror("COP command %02x\n", data);

		switch (data)
		{
		case NEWBRAIN_COPCMD_NULLCOM:
			break;

		case NEWBRAIN_COPCMD_DISPCOM:
			m_copregint = 0;
			m_copbytes = 18;
			m_copstate = NEWBRAIN_COP_STATE_DATA;

			m_copdata = NEWBRAIN_COP_NO_DATA;
			m_copint = 0;
			check_interrupt();

			break;

#if 0
		case NEWBRAIN_COPCMD_TIMCOM:
			m_copregint = 0;
			m_copbytes = 6;
			m_copstate = NEWBRAIN_COP_STATE_DATA;
			break;
#endif
		case NEWBRAIN_COPCMD_PDNCOM:
			/* power down */
			m_copregint = 0;
			break;

		default:
			if (data & NEWBRAIN_COPCMD_TAPECOM)
			{
				m_copregint = 0;
			}
		}
		break;

	case NEWBRAIN_COP_STATE_DATA:
		logerror("COP data %02x\n", data);
		m_copbytes--;

		if (m_copbytes == 0)
		{
			m_copstate = NEWBRAIN_COP_STATE_COMMAND;
			m_copregint = 1;
		}

		m_copdata = NEWBRAIN_COP_NO_DATA;
		m_copint = 0;
		check_interrupt();

		break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(newbrain_state::cop_regint_tick)
{
	if (m_copregint)
	{
		logerror("COP REGINT\n");
		m_copint = 0;
		check_interrupt();
	}
}

/* Expansion Interface Module */

WRITE8_MEMBER( newbrain_eim_state::enrg2_w )
{
	/*

	    bit     signal      description

	    0       _USERP      0 enables user data bus interrupt and also parallel latched data output (or centronics printer) interrupt
	    1       ANP         1 enables ADC conversion complete interrupt and also calling indicator interrupt
	    2       MLTMD       1 enables serial receive clock into multiplier input of DAC and signals data terminal not ready
	    3       MSPD        1 enables 50K Baud serial data rate to be obtained ie. CTC input clock of 800 kHz. 0 selects xxx.692 kHz
	    4       ENOR        1 enables serial receive clock to sound output summer, and also selects serial input from the printer port. 0 selects serial input from the comms port
	    5       ANSW        1 enables second bank of 4 analogue inputs (voltage, non-ratiometric), ie. ch4-7, and enabled sound output, 0 selects ch03
	    6       ENOT        1 enables serial transmit clock to sound ouput summer, and also selects serial output to the printer port. 0 selects serial output to the comms port
	    7                   9th output bit for centronics printer port

	*/

	m_enrg2 = data;
}

WRITE8_MEMBER( newbrain_eim_state::pr_w )
{
	/*

	    bit     signal      description

	    0       HP0
	    1       HP1
	    2       HP2
	    3       HP3
	    4       HP4
	    5       HP5
	    6       HP6
	    7       HP11

	    HP0-HP2 are decoded to _ROM0..._ROM7 signals
	    HP3-HP4 are decoded to _CH0..._CH3 signals

	*/

	int page = (BIT(offset, 12) >> 9) | (BIT(offset, 15) >> 13) | (BIT(offset, 14) >> 13) | (BIT(offset, 13) >> 13);
	int bank = (BIT(offset, 11) >> 3) | (data & 0x7f);

	m_pr[page] = bank;

	bankswitch();
}

READ8_MEMBER( newbrain_eim_state::user_r )
{
	m_user = 0xff;

	return 0xff;
}

WRITE8_MEMBER( newbrain_eim_state::user_w )
{
	m_user = data;
}

READ8_MEMBER( newbrain_eim_state::anout_r )
{
	return 0xff;
}

WRITE8_MEMBER( newbrain_eim_state::anout_w )
{
}

READ8_MEMBER( newbrain_eim_state::anin_r )
{
//  int channel = offset & 0x03;

	return 0;
}

WRITE8_MEMBER( newbrain_eim_state::anio_w )
{
//  int channel = offset & 0x03;
}

READ8_MEMBER( newbrain_eim_state::st0_r )
{
	/*

	    bit     signal      description

	    0                   fixed at 1 - indicates excess of 24 or 48, obsolete
	    1       PWRUP       1 indicates power up from 'cold' - necessary in battery machines with power switching
	    2                   1 indicates analogue or calling indicator interrupts
	    3       _USRINT0    0 indicates centronics printer (latched output data) port interrupt
	    4       _USRINT     0 indicates parallel data bus port interrupt
	    5       _CLKINT     0 indicates frame frequency clock interrupt
	    6       _ACINT      0 indicates ACIA interrupt
	    7       _COPINT     0 indicates interrupt from micro-controller COP420M

	*/

	return (m_copint << 7) | (m_aciaint << 6) | (m_clkint << 5) | (m_userint << 4) | (m_userint0 << 3) | (m_pwrup) << 1 | 0x01;
}

READ8_MEMBER( newbrain_eim_state::st1_r )
{
	/*

	    bit     signal      description

	    0
	    1
	    2       N/_RV       1 selects normal video on power up (white on black), 0 selects reversed video (appears as D0 on the first 200 EI's)
	    3       ANCH        1 indicates power is being taken from the mains supply
	    4       40/_80      1 indicates that 40 column video is selected on power up. 0 selects 80 column video
	    5
	    6       TVCNSL      1 indicates that a video display is required on power up
	    7

	*/

	return (m_tvcnsl << 6) | 0x10 | 0x08 | 0x04;
}

READ8_MEMBER( newbrain_eim_state::st2_r )
{
	/*

	    bit     signal      description

	    0                   received serial data from communications port
	    1                   0 indicates 'clear-to-send' condition at communications port
	    2
	    3
	    4
	    5                   logic level tape input
	    6
	    7                   0 indicates 'clear-to-send' condition at printer port

	*/

	return 0;
}

READ8_MEMBER( newbrain_eim_state::usbs_r )
{
	return 0xff;
}

WRITE8_MEMBER( newbrain_eim_state::usbs_w )
{
}

WRITE8_MEMBER( newbrain_eim_state::paging_w )
{
	if (BIT(offset, 8))
	{
		// expansion interface module

		/*

		    bit     signal      description

		    0       PG          1 enables paging circuits
		    1       WPL         unused
		    2       A16         1 sets local A16 to 1 (ie. causes second set of 8 page registers to select addressed memory)
		    3       _MPM        0 selects multi-processing mode. among other effects this extends the page registers from 8 to 12 bits in length
		    4       HISLT       1 isolates the local machine. this is used in multi-processing mode
		    5
		    6
		    7

		*/

		m_paging = BIT(data, 0);
		m_a16 = BIT(data, 2);
		m_mpm = BIT(data, 3);
	}
	else if (BIT(offset, 9))
	{
		// disc controller

		/*

		    bit     signal      description

		    0       PAGING      1 enables paging circuits
		    1
		    2       HA16        1 sets local A16 to 1 (ie. causes second set of 8 page registers to select addressed memory)
		    3       MPM         0 selects multi-processing mode. among other effects this extends the page registers from 8 to 12 bits in length
		    4
		    5       _FDC RESET
		    6
		    7       FDC ATT

		*/

		m_fdccpu->set_input_line(INPUT_LINE_RESET, BIT(data, 5) ? HOLD_LINE : CLEAR_LINE);

		m_paging = BIT(data, 0);
		m_a16 = BIT(data, 2);
		m_mpm = BIT(data, 3);
		m_fdc_att = BIT(data, 7);
	}
	else if (BIT(offset, 10))
	{
		// network controller
	}
}

/* A/D Converter */

WRITE_LINE_MEMBER( newbrain_eim_state::adc_eoc_w )
{
	m_anint = state;
}

ADC0808_ANALOG_READ_CB( newbrain_eim_state::adc_vref_pos_r )
{
	return 5.0;
}

ADC0808_ANALOG_READ_CB( newbrain_eim_state::adc_vref_neg_r )
{
	return 0.0;
}

ADC0808_ANALOG_READ_CB( newbrain_eim_state::adc_input_r )
{
	return 0.0;
}

/* Memory Maps */

static ADDRESS_MAP_START( newbrain_map, AS_PROGRAM, 8, newbrain_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_RAMBANK("bank1")
	AM_RANGE(0x2000, 0x3fff) AM_RAMBANK("bank2")
	AM_RANGE(0x4000, 0x5fff) AM_RAMBANK("bank3")
	AM_RANGE(0x6000, 0x7fff) AM_RAMBANK("bank4")
	AM_RANGE(0x8000, 0x9fff) AM_RAMBANK("bank5")
	AM_RANGE(0xa000, 0xbfff) AM_RAMBANK("bank6")
	AM_RANGE(0xc000, 0xdfff) AM_RAMBANK("bank7")
	AM_RANGE(0xe000, 0xffff) AM_RAMBANK("bank8")
ADDRESS_MAP_END

static ADDRESS_MAP_START( newbrain_ei_io_map, AS_IO, 8, newbrain_eim_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff00) AM_READWRITE(clusr_r, clusr_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0xff00) AM_WRITE(enrg2_w)
	AM_RANGE(0x02, 0x02) AM_MIRROR(0xff00) AM_MASK(0xff00) AM_WRITE(pr_w)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0xff00) AM_READWRITE(user_r, user_w)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0xff00) AM_READWRITE(clclk_r, clclk_w)
	AM_RANGE(0x05, 0x05) AM_MIRROR(0xff00) AM_READWRITE(anout_r, anout_w)
	AM_RANGE(0x06, 0x06) AM_MIRROR(0xff00) AM_READWRITE(cop_r, cop_w)
	AM_RANGE(0x07, 0x07) AM_MIRROR(0xff00) AM_WRITE(enrg1_w)
	AM_RANGE(0x08, 0x09) AM_MIRROR(0xff02) AM_READWRITE(tvl_r, tvl_w)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0xff03) AM_WRITE(tvctl_w)
	AM_RANGE(0x10, 0x13) AM_MIRROR(0xff00) AM_READWRITE(anin_r, anio_w)
	AM_RANGE(0x14, 0x14) AM_MIRROR(0xff00) AM_READ(st0_r)
	AM_RANGE(0x15, 0x15) AM_MIRROR(0xff00) AM_READ(st1_r)
	AM_RANGE(0x16, 0x16) AM_MIRROR(0xff00) AM_READ(st2_r)
	AM_RANGE(0x17, 0x17) AM_MIRROR(0xff00) AM_READWRITE(usbs_r, usbs_w)
	AM_RANGE(0x18, 0x18) AM_MIRROR(0xff00) AM_DEVREADWRITE(MC6850_TAG, acia6850_device, status_r, control_w)
	AM_RANGE(0x19, 0x19) AM_MIRROR(0xff00) AM_DEVREADWRITE(MC6850_TAG, acia6850_device, data_r, data_w)
	AM_RANGE(0x1c, 0x1f) AM_MIRROR(0xff00) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0xff, 0xff) AM_MIRROR(0xff00) AM_MASK(0xff00) AM_WRITE(paging_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( newbrain_a_io_map, AS_IO, 8, newbrain_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xffc0) AM_READWRITE(clusr_r, clusr_w)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0xffc0) AM_WRITE(user_w)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0xffc0) AM_READWRITE(clclk_r, clclk_w)
	AM_RANGE(0x06, 0x06) AM_MIRROR(0xffc0) AM_READWRITE(cop_r, cop_w)
	AM_RANGE(0x07, 0x07) AM_MIRROR(0xffc0) AM_WRITE(a_enrg1_w)
	AM_RANGE(0x08, 0x09) AM_MIRROR(0xffc2) AM_READWRITE(tvl_r, tvl_w)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0xffc3) AM_WRITE(tvctl_w)
	AM_RANGE(0x14, 0x14) AM_MIRROR(0xffc3) AM_READ(a_ust_r)
	AM_RANGE(0x16, 0x16) AM_MIRROR(0xffc0) AM_READ(user_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( newbrain_fdc_map, AS_PROGRAM, 8, newbrain_eim_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( newbrain_fdc_io_map, AS_IO, 8, newbrain_eim_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVICE(UPD765_TAG, upd765a_device, map)
	AM_RANGE(0x20, 0x20) AM_WRITE(fdc_auxiliary_w)
	AM_RANGE(0x40, 0x40) AM_READ(fdc_control_r)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( newbrain )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STOP") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('(') PORT_CHAR('[')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(')') PORT_CHAR(']')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("* \xC2\xA3") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('*') PORT_CHAR(0x00A3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("VIDEO TEXT") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT)) // Vd
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("Y10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("Y11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('-') PORT_CHAR('\\')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("Y12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('+') PORT_CHAR('^')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INSERT") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("Y13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NEW LINE") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) // NL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("Y14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) // CH

	PORT_START("Y15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) // SH
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRAPHICS") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) // GR
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REPEAT") // RPT
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) // GL
INPUT_PORTS_END

/* Machine Initialization */

WRITE_LINE_MEMBER( newbrain_eim_state::acia_tx )
{
	m_acia_txd = state;
}

WRITE_LINE_MEMBER( newbrain_eim_state::acia_interrupt )
{
	m_aciaint = state;
}

WRITE_LINE_MEMBER( newbrain_eim_state::fdc_interrupt )
{
	m_fdc_int = state;
}

WRITE_LINE_MEMBER( newbrain_eim_state::ctc_z2_w )
{
	/* connected to CTC channel 0/1 clock inputs */
	m_ctc->trg0(state);
	m_ctc->trg1(state);
}

TIMER_DEVICE_CALLBACK_MEMBER(newbrain_eim_state::ctc_c2_tick)
{
	m_ctc->trg2(1);
	m_ctc->trg2(0);
}

inline int newbrain_state::get_reset_t()
{
	return RES_K(220) * CAP_U(10) * 1000; // t = R128 * C125 = 2.2s
}

inline int newbrain_state::get_pwrup_t()
{
	return RES_K(560) * CAP_U(10) * 1000; // t = R129 * C127 = 5.6s
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void newbrain_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_RESET:
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_copcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		break;

	case TIMER_ID_PWRUP:
		m_pwrup = 0;
		bankswitch();
		break;
	}
}

void newbrain_state::machine_start()
{
	m_copregint = 1;

	/* set power up timer */
	timer_set(attotime::from_usec(get_pwrup_t()), TIMER_ID_PWRUP);

	/* initialize variables */
	m_pwrup = 1;
	m_userint = 1;
	m_userint0 = 1;
	m_clkint = 1;
	m_aciaint = 1;
	m_copint = 1;
	m_bee = 1;
	m_tvcnsl = 1;

	/* set up memory banking */
	bankswitch();

	// find keyboard rows
	m_key_row[0] = m_y0;
	m_key_row[1] = m_y1;
	m_key_row[2] = m_y2;
	m_key_row[3] = m_y3;
	m_key_row[4] = m_y4;
	m_key_row[5] = m_y5;
	m_key_row[6] = m_y6;
	m_key_row[7] = m_y7;
	m_key_row[8] = m_y8;
	m_key_row[9] = m_y9;
	m_key_row[10] = m_y10;
	m_key_row[11] = m_y11;
	m_key_row[12] = m_y12;
	m_key_row[13] = m_y13;
	m_key_row[14] = m_y14;
	m_key_row[15] = m_y15;

	/* register for state saving */
	save_item(NAME(m_pwrup));
	save_item(NAME(m_userint));
	save_item(NAME(m_userint0));
	save_item(NAME(m_clkint));
	save_item(NAME(m_aciaint));
	save_item(NAME(m_copint));
	save_item(NAME(m_anint));
	save_item(NAME(m_bee));
	save_item(NAME(m_enrg1));
	save_item(NAME(m_enrg2));
	save_item(NAME(m_cop_bus));
	save_item(NAME(m_cop_so));
	save_item(NAME(m_cop_tdo));
	save_item(NAME(m_cop_tdi));
	save_item(NAME(m_cop_rd));
	save_item(NAME(m_cop_wr));
	save_item(NAME(m_cop_access));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_keydata));
	save_item(NAME(m_user));
}

void newbrain_eim_state::machine_start()
{
	newbrain_state::machine_start();

	/* allocate expansion RAM */
	m_eim_ram.allocate(NEWBRAIN_EIM_RAM_SIZE);

	/* register for state saving */
	save_item(NAME(m_mpm));
	save_item(NAME(m_a16));
	save_item(NAME(m_pr));
	save_item(NAME(m_fdc_int));
	save_item(NAME(m_fdc_att));
	save_item(NAME(m_paging));
}

void newbrain_state::machine_reset()
{
	timer_set(attotime::from_usec(get_reset_t()), TIMER_ID_RESET);
}

INTERRUPT_GEN_MEMBER(newbrain_state::newbrain_interrupt)
{
	if (!(m_enrg1 & NEWBRAIN_ENRG1_CLK))
	{
		m_clkint = 0;
		check_interrupt();
	}
}

/* Machine Drivers */

/* F4 Character Displayer */
static const gfx_layout newbrain_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*256*8, 1*256*8, 2*256*8, 3*256*8, 4*256*8, 5*256*8, 6*256*8, 7*256*8, 8*256*8, 9*256*8, 10*256*8, 11*256*8, 12*256*8, 13*256*8, 14*256*8, 15*256*8 },
	8                   /* every char takes 16 x 1 bytes */
};

static GFXDECODE_START( newbrain )
	GFXDECODE_ENTRY( "chargen", 0x0000, newbrain_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( newbrain_a, newbrain_state )
	// basic system hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_16MHz/8)
	MCFG_CPU_PROGRAM_MAP(newbrain_map)
	MCFG_CPU_IO_MAP(newbrain_a_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER(SCREEN_TAG, newbrain_state,  newbrain_interrupt)

	MCFG_CPU_ADD(COP420_TAG, COP420, XTAL_16MHz/8) // COP420-GUW/N
	MCFG_COP400_CONFIG( COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, COP400_MICROBUS_ENABLED )
	MCFG_COP400_READ_L_CB(READ8(newbrain_state, cop_l_r))
	MCFG_COP400_WRITE_L_CB(WRITE8(newbrain_state, cop_l_w))
	MCFG_COP400_READ_G_CB(READ8(newbrain_state, cop_g_r))
	MCFG_COP400_WRITE_G_CB(WRITE8(newbrain_state, cop_g_w))
	MCFG_COP400_WRITE_D_CB(WRITE8(newbrain_state, cop_d_w))
	MCFG_COP400_READ_IN_CB(READ8(newbrain_state, cop_in_r))
	MCFG_COP400_WRITE_SK_CB(WRITELINE(newbrain_state, cop_sk_w))
	MCFG_COP400_READ_SI_CB(READLINE(newbrain_state, cop_si_r))
	MCFG_COP400_WRITE_SO_CB(WRITELINE(newbrain_state, cop_so_w))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", newbrain)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("cop_regint", newbrain_state, cop_regint_tick, attotime::from_usec(12500))

	// video hardware
	MCFG_FRAGMENT_ADD(newbrain_video)

	// devices
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_CASSETTE_ADD("cassette2")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
MACHINE_CONFIG_END

static SLOT_INTERFACE_START( newbrain_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_DERIVED_CLASS( newbrain_eim, newbrain_a, newbrain_eim_state )
	// basic system hardware
	MCFG_CPU_MODIFY(Z80_TAG)
	MCFG_CPU_IO_MAP(newbrain_ei_io_map)

	MCFG_CPU_ADD(FDC_Z80_TAG, Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(newbrain_fdc_map)
	MCFG_CPU_IO_MAP(newbrain_fdc_io_map)

	// devices
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_16MHz/8)
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE(MC6850_TAG, acia6850_device, write_rxc))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE(MC6850_TAG, acia6850_device, write_txc))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(newbrain_eim_state, ctc_z2_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("z80ctc_c2", newbrain_eim_state, ctc_c2_tick, attotime::from_hz(XTAL_16MHz/4/13))
	MCFG_DEVICE_ADD(ADC0809_TAG, ADC0808, 500000)
	MCFG_ADC0808_OUT_EOC_CB(WRITELINE(newbrain_eim_state, adc_eoc_w))
	MCFG_ADC0808_IN_VREF_POS_CB(newbrain_eim_state, adc_vref_pos_r)
	MCFG_ADC0808_IN_VREF_NEG_CB(newbrain_eim_state, adc_vref_neg_r)
	MCFG_ADC0808_IN_IN_0_CB(newbrain_eim_state, adc_input_r)
	MCFG_ADC0808_IN_IN_1_CB(newbrain_eim_state, adc_input_r)
	MCFG_ADC0808_IN_IN_2_CB(newbrain_eim_state, adc_input_r)
	MCFG_ADC0808_IN_IN_3_CB(newbrain_eim_state, adc_input_r)
	MCFG_ADC0808_IN_IN_4_CB(newbrain_eim_state, adc_input_r)
	MCFG_ADC0808_IN_IN_5_CB(newbrain_eim_state, adc_input_r)
	MCFG_ADC0808_IN_IN_6_CB(newbrain_eim_state, adc_input_r)
	MCFG_ADC0808_IN_IN_7_CB(newbrain_eim_state, adc_input_r)

	MCFG_DEVICE_ADD(MC6850_TAG, ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(newbrain_eim_state, acia_tx))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(newbrain_eim_state, acia_interrupt))

	MCFG_UPD765A_ADD(UPD765_TAG, false, true)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", newbrain_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", newbrain_floppies, "525dd", floppy_image_device::default_floppy_formats)

	// internal ram
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("96K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( newbrain )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "rom20" )

	ROM_SYSTEM_BIOS( 0, "issue1", "Issue 1 (v?)" )
	ROMX_LOAD( "aben.ic6",     0xa000, 0x2000, CRC(308f1f72) SHA1(a6fd9945a3dca47636887da2125fde3f9b1d4e25), ROM_BIOS(1) )
	ROMX_LOAD( "cd iss 1.ic7", 0xc000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(1) )
	ROMX_LOAD( "ef iss 1.ic8", 0xe000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "issue2", "Issue 2 (v1.9)" )
	ROMX_LOAD( "aben19.ic6",   0xa000, 0x2000, CRC(d0283eb1) SHA1(351d248e69a77fa552c2584049006911fb381ff0), ROM_BIOS(2) )
	ROMX_LOAD( "cdi2.ic7",     0xc000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(2) )
	ROMX_LOAD( "ef iss 1.ic8", 0xe000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "issue3", "Issue 3 (v1.91)" )
	ROMX_LOAD( "aben191.ic6",  0xa000, 0x2000, CRC(b7be8d89) SHA1(cce8d0ae7aa40245907ea38b7956c62d039d45b7), ROM_BIOS(3) )
	ROMX_LOAD( "cdi3.ic7",     0xc000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(3) )
	ROMX_LOAD( "ef iss 1.ic8", 0xe000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 3, "series2", "Series 2 (v?)" )
	ROMX_LOAD( "abs2.ic6",     0xa000, 0x2000, CRC(9a042acb) SHA1(80d83a2ea3089504aa68b6cf978d80d296cd9bda), ROM_BIOS(4) )
	ROMX_LOAD( "cds2.ic7",     0xc000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(4) )
	ROMX_LOAD( "efs2.ic8",     0xe000, 0x2000, CRC(b222d798) SHA1(c0c816b4d4135b762f2c5f1b24209d0096f22e56), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 4, "rom20", "? (v2.0)" )
	ROMX_LOAD( "aben20.rom",   0xa000, 0x2000, CRC(3d76d0c8) SHA1(753b4530a518ad832e4b81c4e5430355ba3f62e0), ROM_BIOS(5) )
	ROMX_LOAD( "cd20tci.rom",  0xc000, 0x4000, CRC(f65b2350) SHA1(1ada7fbf207809537ec1ffb69808524300622ada), ROM_BIOS(5) )

	ROM_REGION( 0x400, COP420_TAG, 0 )
	ROM_LOAD( "cop420.419", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x1000, "chargen", ROMREGION_ERASE00 )
	ROM_LOAD( "char eprom iss 1.ic453", 0x0000, 0x0a01, BAD_DUMP CRC(46ecbc65) SHA1(3fe064d49a4de5e3b7383752e98ad35a674e26dd) ) // 8248R7
ROM_END

#define rom_newbraina rom_newbrain

ROM_START( newbraineim )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "rom20" )

	ROM_SYSTEM_BIOS( 0, "issue1", "Issue 1 (v?)" )
	ROMX_LOAD( "aben.ic6",     0xa000, 0x2000, CRC(308f1f72) SHA1(a6fd9945a3dca47636887da2125fde3f9b1d4e25), ROM_BIOS(1) )
	ROMX_LOAD( "cd iss 1.ic7", 0xc000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(1) )
	ROMX_LOAD( "ef iss 1.ic8", 0xe000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "issue2", "Issue 2 (v1.9)" )
	ROMX_LOAD( "aben19.ic6",   0xa000, 0x2000, CRC(d0283eb1) SHA1(351d248e69a77fa552c2584049006911fb381ff0), ROM_BIOS(2) )
	ROMX_LOAD( "cdi2.ic7",     0xc000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(2) )
	ROMX_LOAD( "ef iss 1.ic8", 0xe000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "issue3", "Issue 3 (v1.91)" )
	ROMX_LOAD( "aben191.ic6",  0xa000, 0x2000, CRC(b7be8d89) SHA1(cce8d0ae7aa40245907ea38b7956c62d039d45b7), ROM_BIOS(3) )
	ROMX_LOAD( "cdi3.ic7",     0xc000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(3) )
	ROMX_LOAD( "ef iss 1.ic8", 0xe000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 3, "series2", "Series 2 (v?)" )
	ROMX_LOAD( "abs2.ic6",     0xa000, 0x2000, CRC(9a042acb) SHA1(80d83a2ea3089504aa68b6cf978d80d296cd9bda), ROM_BIOS(4) )
	ROMX_LOAD( "cds2.ic7",     0xc000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(4) )
	ROMX_LOAD( "efs2.ic8",     0xe000, 0x2000, CRC(b222d798) SHA1(c0c816b4d4135b762f2c5f1b24209d0096f22e56), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 4, "rom20", "? (v2.0)" )
	ROMX_LOAD( "aben20.rom",   0xa000, 0x2000, CRC(3d76d0c8) SHA1(753b4530a518ad832e4b81c4e5430355ba3f62e0), ROM_BIOS(5) )
	ROMX_LOAD( "cd20tci.rom",  0xc000, 0x4000, CRC(f65b2350) SHA1(1ada7fbf207809537ec1ffb69808524300622ada), ROM_BIOS(5) )

	ROM_REGION( 0x400, COP420_TAG, 0 )
	ROM_LOAD( "cop420.419",   0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "char eprom iss 1.ic453", 0x0000, 0x0a01, BAD_DUMP CRC(46ecbc65) SHA1(3fe064d49a4de5e3b7383752e98ad35a674e26dd) ) // 8248R7

	ROM_REGION( 0x10000, "eim", 0 ) // Expansion Interface Module
	ROM_LOAD( "e415-2.rom", 0x4000, 0x2000, CRC(5b0e390c) SHA1(0f99cae57af2e64f3f6b02e5325138d6ba015e72) )
	ROM_LOAD( "e415-3.rom", 0x4000, 0x2000, CRC(2f88bae5) SHA1(04e03f230f4b368027442a7c2084dae877f53713) ) // 18/8/83.aci
	ROM_LOAD( "e416-3.rom", 0x6000, 0x2000, CRC(8b5099d8) SHA1(19b0cfce4c8b220eb1648b467f94113bafcb14e0) ) // 10/8/83.mtv
	ROM_LOAD( "e417-2.rom", 0x8000, 0x2000, CRC(6a7afa20) SHA1(f90db4f8318777313a862b3d5bab83c2fd260010) )

	ROM_REGION( 0x10000, FDC_Z80_TAG, 0 ) // Floppy Disk Controller
	ROM_LOAD( "d413-2.rom", 0x0000, 0x2000, CRC(097591f1) SHA1(c2aa1d27d4f3a24ab0c8135df746a4a44201a7f4) )
	ROM_LOAD( "d417-1.rom", 0x0000, 0x2000, CRC(40fad31c) SHA1(5137be4cc026972c0ffd4fa6990e8583bdfce163) )
	ROM_LOAD( "d417-2.rom", 0x0000, 0x2000, CRC(e8bda8b9) SHA1(c85a76a5ff7054f4ef4a472ce99ebaed1abd269c) )
ROM_END

ROM_START( newbrainmd )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "cdmd.rom", 0xc000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a) )
	ROM_LOAD( "efmd.rom", 0xe000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6) )

	ROM_REGION( 0x400, COP420_TAG, 0 )
	ROM_LOAD( "cop420.419", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "char eprom iss 1.ic453", 0x0000, 0x0a01, BAD_DUMP CRC(46ecbc65) SHA1(3fe064d49a4de5e3b7383752e98ad35a674e26dd) ) // 8248R7
ROM_END

/* System Drivers */

//    YEAR  NAME        PARENT      COMPAT  MACHINE         INPUT       INIT    COMPANY                         FULLNAME        FLAGS
COMP( 1981, newbrain,   0,          0,      newbrain_a,     newbrain, driver_device,   0,      "Grundy Business Systems Ltd",   "NewBrain AD",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1981, newbraineim,newbrain,   0,      newbrain_eim,   newbrain, driver_device,   0,      "Grundy Business Systems Ltd",   "NewBrain AD with Expansion Interface", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1981, newbraina,  newbrain,   0,      newbrain_a,     newbrain, driver_device,   0,      "Grundy Business Systems Ltd",   "NewBrain A",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1981, newbrainmd, newbrain,   0,      newbrain_a,     newbrain, driver_device,   0,      "Grundy Business Systems Ltd",   "NewBrain MD",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
