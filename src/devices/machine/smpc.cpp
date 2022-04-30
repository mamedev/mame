// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
/************************************************************************************

Sega Saturn SMPC - System Manager and Peripheral Control MCU simulation

The SMPC is actually a 4-bit Hitachi HD404920FS MCU, labeled with a Sega custom
315-5744 (that needs decapping)

MCU simulation by Angelo Salese & R. Belmont

TODO:
- timings;
- fix intback issue with inputs (according to the docs, it should fall in between
  VBLANK-IN and OUT, for obvious reasons);
- clean-ups;
- RTC subdevice (unknown type, handled here for convenience);
- Does ST-V even has a battery backed NVRAM?

Notes:
SMPC NVRAM contents:
[0] unknown (always 0)
[1] unknown (always 0)
[2] ---- -x-- Button Labels (0=enable)
    ---- --x- Audio Out (1=Mono 0=Stereo)
    ---- ---x BIOS audio SFXs enable (0=enable)
[3] language select (0=English, 5=Japanese)

*************************************************************************************/
/* SMPC Addresses

00
01 -w  Input Register 0 (IREG)
02
03 -w  Input Register 1
04
05 -w  Input Register 2
06
07 -w  Input Register 3
08
09 -w  Input Register 4
0a
0b -w  Input Register 5
0c
0d -w  Input Register 6
0e
0f
10
11
12
13
14
15
16
17
18
19
1a
1b
1c
1d
1e
1f -w  Command Register (COMREG)
20
21 r-  Output Register 0 (OREG)
22
23 r-  Output Register 1
24
25 r-  Output Register 2
26
27 r-  Output Register 3
28
29 r-  Output Register 4
2a
2b r-  Output Register 5
2c
2d r-  Output Register 6
2e
2f r-  Output Register 7
30
31 r-  Output Register 8
32
33 r-  Output Register 9
34
35 r-  Output Register 10
36
37 r-  Output Register 11
38
39 r-  Output Register 12
3a
3b r-  Output Register 13
3c
3d r-  Output Register 14
3e
3f r-  Output Register 15
40
41 r-  Output Register 16
42
43 r-  Output Register 17
44
45 r-  Output Register 18
46
47 r-  Output Register 19
48
49 r-  Output Register 20
4a
4b r-  Output Register 21
4c
4d r-  Output Register 22
4e
4f r-  Output Register 23
50
51 r-  Output Register 24
52
53 r-  Output Register 25
54
55 r-  Output Register 26
56
57 r-  Output Register 27
58
59 r-  Output Register 28
5a
5b r-  Output Register 29
5c
5d r-  Output Register 30
5e
5f r-  Output Register 31
60
61 r-  SR
62
63 rw  SF
64
65
66
67
68
69
6a
6b
6c
6d
6e
6f
70
71
72
73
74
75 rw PDR1
76
77 rw PDR2
78
79 -w DDR1
7a
7b -w DDR2
7c
7d -w IOSEL2/1
7e
7f -w EXLE2/1
*/

#include "emu.h"
#include "machine/smpc.h"

#include "screen.h"
#include "coreutil.h"


#define LOG_SMPC 0
#define LOG_PAD_CMD 0

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SMPC_HLE, smpc_hle_device, "smpc_hle", "Sega Saturn SMPC HLE (HD404920FS)")

// TODO: use DEVICE_ADDRESS_MAP once this fatalerror is fixed:
// "uplift_submaps unhandled case: range straddling slots."
void smpc_hle_device::smpc_regs(address_map &map)
{
//  map.unmap_value_high();
	map(0x00, 0x0d).w(FUNC(smpc_hle_device::ireg_w));
	map(0x1f, 0x1f).w(FUNC(smpc_hle_device::command_register_w));
	map(0x20, 0x5f).r(FUNC(smpc_hle_device::oreg_r));
	map(0x61, 0x61).r(FUNC(smpc_hle_device::status_register_r));
	map(0x63, 0x63).rw(FUNC(smpc_hle_device::status_flag_r), FUNC(smpc_hle_device::status_flag_w));
	map(0x75, 0x75).rw(FUNC(smpc_hle_device::pdr1_r), FUNC(smpc_hle_device::pdr1_w));
	map(0x77, 0x77).rw(FUNC(smpc_hle_device::pdr2_r), FUNC(smpc_hle_device::pdr2_w));
	map(0x79, 0x79).w(FUNC(smpc_hle_device::ddr1_w));
	map(0x7b, 0x7b).w(FUNC(smpc_hle_device::ddr2_w));
	map(0x7d, 0x7d).w(FUNC(smpc_hle_device::iosel_w));
	map(0x7f, 0x7f).w(FUNC(smpc_hle_device::exle_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  smpc_hle_device - constructor
//-------------------------------------------------

smpc_hle_device::smpc_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SMPC_HLE, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("regs", ENDIANNESS_LITTLE, 8, 7, 0, address_map_constructor(FUNC(smpc_hle_device::smpc_regs), this))
	, m_mini_nvram(*this, "smem")
	, m_mshres(*this)
	, m_mshnmi(*this)
	, m_sshres(*this)
	, m_sndres(*this)
	, m_sysres(*this)
	, m_syshalt(*this)
	, m_dotsel(*this)
	, m_pdr1_read(*this)
	, m_pdr2_read(*this)
	, m_pdr1_write(*this)
	, m_pdr2_write(*this)
	, m_irq_line(*this)
	, m_ctrl1(*this, finder_base::DUMMY_TAG)
	, m_ctrl2(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
{
	m_has_ctrl_ports = false;
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void smpc_hle_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "smem", nvram_device::DEFAULT_ALL_0);

	// TODO: custom RTC subdevice
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void smpc_hle_device::device_start()
{
	system_time systime;
	machine().base_datetime(systime);

//  check if SMEM has valid data via byte 4 in the array, if not then simulate a battery backup fail
//  (-> call the RTC / Language select menu for Saturn)
	m_mini_nvram->set_base(&m_smem, 5);

	m_mshres.resolve_safe();
	m_mshnmi.resolve_safe();
	m_sshres.resolve_safe();
	m_sndres.resolve_safe();
	m_sysres.resolve_safe();
	m_syshalt.resolve_safe();
	m_dotsel.resolve_safe();
	m_irq_line.resolve_safe();

	m_pdr1_read.resolve_safe(0xff);
	m_pdr2_read.resolve_safe(0xff);
	m_pdr1_write.resolve_safe();
	m_pdr2_write.resolve_safe();

	save_item(NAME(m_sf));
	save_item(NAME(m_sr));
	save_item(NAME(m_ddr1));
	save_item(NAME(m_ddr2));
	save_item(NAME(m_pdr1_readback));
	save_item(NAME(m_pdr2_readback));
	save_item(NAME(m_iosel1));
	save_item(NAME(m_iosel2));
	save_item(NAME(m_exle1));
	save_item(NAME(m_exle2));
	save_item(NAME(m_ireg));
	save_item(NAME(m_oreg));
	save_item(NAME(m_comreg));
	save_item(NAME(m_command_in_progress));
	save_item(NAME(m_intback_buf));
	save_item(NAME(m_intback_stage));
	save_item(NAME(m_pmode));
	save_item(NAME(m_rtc_data));
	save_item(NAME(m_smem));

	m_cmd_timer = timer_alloc(COMMAND_ID);
	m_rtc_timer = timer_alloc(RTC_ID);
	m_intback_timer = timer_alloc(INTBACK_ID);
	m_sndres_timer = timer_alloc(SNDRES_ID);

	m_rtc_data[0] = DectoBCD(systime.local_time.year / 100);
	m_rtc_data[1] = DectoBCD(systime.local_time.year % 100);
	m_rtc_data[2] = (systime.local_time.weekday << 4) | (systime.local_time.month+1);
	m_rtc_data[3] = DectoBCD(systime.local_time.mday);
	m_rtc_data[4] = DectoBCD(systime.local_time.hour);
	m_rtc_data[5] = DectoBCD(systime.local_time.minute);
	m_rtc_data[6] = DectoBCD(systime.local_time.second);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void smpc_hle_device::device_reset()
{
	m_sr = 0x40; // this bit is always on according to docs (?)
	m_sf = false;
	m_cd_sf = false;
	m_ddr1 = 0;
	m_ddr2 = 0;
	m_pdr1_readback = 0;
	m_pdr2_readback = 0;

	memset(m_ireg,0,7);
	memset(m_oreg,0,32);

	m_cmd_timer->reset();
	m_intback_timer->reset();
	m_sndres_timer->reset();
	m_comreg = 0xff;
	m_command_in_progress = false;
	m_NMI_reset = false;
	m_cur_dotsel = false;

	m_rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));
}

device_memory_interface::space_config_vector smpc_hle_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void smpc_hle_device::ireg_w(offs_t offset, uint8_t data)
{
	if (!(offset & 1)) // avoid writing to even bytes
		return;

	m_ireg[offset >> 1] = data;

	if(offset == 1) // check if we are under intback
	{
		if(m_intback_stage)
		{
			if(data & 0x40)
			{
				if(LOG_PAD_CMD) printf("SMPC: BREAK request\n");
				sr_ack();
				m_intback_stage = 0;
			}
			else if(data & 0x80)
			{
				if(LOG_PAD_CMD) printf("SMPC: CONTINUE request\n");

				m_intback_timer->adjust(attotime::from_usec(700));  // TODO: is timing correct?

				// TODO: following looks wrong here
				m_oreg[31] = 0x10;
				sf_set();
			}
		}
	}
}

uint8_t smpc_hle_device::oreg_r(offs_t offset)
{
	if (!(offset & 1)) // avoid reading to even bytes (TODO: is it 0s or 1s?)
		return 0x00;

	return m_oreg[offset >> 1];
}

uint8_t smpc_hle_device::status_register_r()
{
	return m_sr;
}

uint8_t smpc_hle_device::status_flag_r()
{
	// bit 3: CD enable related?
	return (m_sf<<0) | (m_cd_sf<<3);
}

void smpc_hle_device::status_flag_w(uint8_t data)
{
	m_sf = BIT(data,0);
	m_cd_sf = false;
}

uint8_t smpc_hle_device::pdr1_r()
{
	uint8_t res = (m_pdr1_read() & ~m_ddr1) | m_pdr1_readback;

	return res;
}

uint8_t smpc_hle_device::pdr2_r()
{
	uint8_t res = (m_pdr2_read() & ~m_ddr2) | m_pdr2_readback;

	return res;
}

void smpc_hle_device::pdr1_w(uint8_t data)
{
//  pins defined as output returns in input
	m_pdr1_readback = (data & m_ddr1);
	m_pdr1_readback &= 0x7f;
	m_pdr1_write(m_pdr1_readback);
//  bit 7 can be read back apparently
	m_pdr1_readback |= data & 0x80;
}

void smpc_hle_device::pdr2_w(uint8_t data)
{
//  pins defined as output returns in input
	m_pdr2_readback = (data & m_ddr2);
	m_pdr2_readback &= 0x7f;
	m_pdr2_write(m_pdr2_readback);
//  bit 7 can be read back apparently
	m_pdr2_readback |= data & 0x80;
}

void smpc_hle_device::ddr1_w(uint8_t data)
{
	m_ddr1 = data & 0x7f;
}

void smpc_hle_device::ddr2_w(uint8_t data)
{
	m_ddr2 = data & 0x7f;
}

void smpc_hle_device::iosel_w(uint8_t data)
{
	m_iosel1 = BIT(data,0);
	m_iosel2 = BIT(data,1);
}

void smpc_hle_device::exle_w(uint8_t data)
{
	m_exle1 = BIT(data,0);
	m_exle2 = BIT(data,1);
}

inline void smpc_hle_device::sr_ack()
{
	m_sr &= 0x0f;
}

inline void smpc_hle_device::sr_set(uint8_t data)
{
	m_sr = data;
}

inline void smpc_hle_device::sf_ack(bool cd_enable)
{
	m_sf = false;
	m_cd_sf = cd_enable;
}

inline void smpc_hle_device::sf_set()
{
	m_sf = true;
}

// Saturn Direct Mode polling check for delegate
bool smpc_hle_device::get_iosel(bool which)
{
	return which == true ? m_iosel2 : m_iosel1;
}

uint8_t smpc_hle_device::get_ddr(bool which)
{
	return which == true ? m_ddr2 : m_ddr1;
}


inline void smpc_hle_device::master_sh2_nmi()
{
	m_mshnmi(1);
	m_mshnmi(0);
}

inline void smpc_hle_device::irq_request()
{
	m_irq_line(1);
	m_irq_line(0);
}

// TODO: trampolines that needs to go away

uint8_t smpc_hle_device::read(offs_t offset)
{
	return this->space().read_byte(offset);
}

void smpc_hle_device::write(offs_t offset, uint8_t data)
{
	this->space().write_byte(offset,data);
}

//**************************************************************************
//  Command simulation
//**************************************************************************

void smpc_hle_device::command_register_w(uint8_t data)
{
//  don't send a command if previous one is still in progress
//  ST-V tries to send a sysres command if OREG31 doesn't return the ack command
	if(m_command_in_progress == true)
		return;

	m_comreg = data & 0x1f;

	if(data & 0xe0)
		logerror("%s COMREG = %02x!?\n",this->tag(),data);

	m_command_in_progress = true;
	if(m_comreg == 0x0e || m_comreg == 0x0f)
	{
		/* on ST-V timing of this is pretty fussy, you get 2 credits at start-up otherwise
		 * My current theory is that the PLL device can halt the whole system until the frequency change occurs.
		 *  (cfr. diagram on page 3 of SMPC manual)
		 * I really don't think that the system can do an usable mid-frame clock switching anyway.
		 */
		m_syshalt(1);

		m_cmd_timer->adjust(m_screen->time_until_pos(m_screen->visible_area().max_y,0));
	}
	else if(m_comreg == 0x10)
	{
		// copy ireg to our intback buffer
		for(int i=0;i<3;i++)
			m_intback_buf[i] = m_ireg[i];

		// calculate the timing for intback command
		int timing;

		timing = 8;

		if( m_ireg[0] != 0) // non-peripheral data
			timing += 8;

		// TODO: At vblank-out actually ...
		if( m_ireg[1] & 8) // peripheral data
			timing += 700;

		// TODO: check against ireg2, must be 0xf0

		m_cmd_timer->adjust(attotime::from_usec(timing));
	}
	else
		m_cmd_timer->adjust(attotime::from_usec(m_cmd_table_timing[m_comreg]));
}


void smpc_hle_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch(id)
	{
		case COMMAND_ID:
		{
			switch(m_comreg)
			{
				case 0x00: // MSHON
					// enable Master SH2
					m_mshres(m_comreg & 1);
					break;

				case 0x02: // SSHON
				case 0x03: // SSHOFF
					// enable or disable Slave SH2
					m_sshres(m_comreg & 1);
					break;

				case 0x06: // SNDON
				case 0x07: // SNDOFF
					// enable or disable 68k
					m_sndres(m_comreg & 1);
					break;

				case 0x08: // CDON
				case 0x09: // CDOFF
					// ...
					m_command_in_progress = false;
					m_oreg[31] = m_comreg;
					// TODO: diagnostic also wants this to have bit 3 high
					sf_ack(true); //set hand-shake flag
					return;

				case 0x0a: // NETLINKON
					// TODO: understand where NetLink actually lies and implement delegation accordingly
					// (is it really an SH1 device like suggested by the space access or it overlays on CS2 bus?)
					popmessage("%s: NetLink enabled", this->tag());
					 [[fallthrough]];
				case 0x0b: // NETLINKOFF
					break;

				case 0x0d: // SYSRES
					// send a 1 -> 0 to device reset lines
					m_sysres(1);
					m_sysres(0);

					// send a 1 -> 0 transition to reset line (was PULSE_LINE)
					m_mshres(1);
					m_mshres(0);
					break;

				case 0x0e: // CKCHG352
				case 0x0f: // CKCHG320
					m_dotsel(m_comreg & 1);

					// assert Slave SH2 line
					m_sshres(1);
					// clear PLL system halt
					m_syshalt(0);

					// setup the new dot select
					m_cur_dotsel = (m_comreg & 1) ^ 1;

					// send a NMI to Master SH2 if enabled
					// it is unconditionally requested:
					// bigichig, capgen1, capgen4 and capgen5 triggers a SLEEP opcode from BIOS call and expects this to wake them up.
					//if(m_NMI_reset == false)
					master_sh2_nmi();
					break;

				case 0x10: // INTBACK
					resolve_intback();
					return;

				case 0x16: // SETTIME
				{
					for(int i=0;i<7;i++)
						m_rtc_data[i] = m_ireg[i];
					break;
				}

				case 0x17: // SETSMEM
				{
					for(int i=0;i<4;i++)
						m_smem[i] = m_ireg[i];

					// clear the SETIME variable, simulate a cr2032 battery alive in the system
					m_smem[4] = 0xff;
					break;
				}

				case 0x18: // NMIREQ
					// NMI is unconditionally requested
					master_sh2_nmi();
					break;

				case 0x19: // RESENAB
				case 0x1a: // RESDISA
					m_NMI_reset = m_comreg & 1;
					break;

				default:
					logerror("%s: unemulated %02x command\n",this->tag(),m_comreg);
					return;
			}

			m_command_in_progress = false;
			m_oreg[31] = m_comreg;
			sf_ack(false);
			break;
		}

		case INTBACK_ID: intback_continue_request(); break;
		case RTC_ID: handle_rtc_increment(); break;

		// from m68k reset opcode trigger
		case SNDRES_ID:
			m_sndres(1);
			m_sndres(0);
			break;

		default:
			printf("%d\n",id);
			break;
	}
}

void smpc_hle_device::resolve_intback()
{
	int i;

	m_command_in_progress = false;

	if(m_intback_buf[0] != 0)
	{
		m_oreg[0] = ((m_smem[4] & 0x80) | ((m_NMI_reset & 1) << 6));

		for(i=0;i<7;i++)
			m_oreg[1+i] = m_rtc_data[i];

		m_oreg[8] = 0; // CTG0 / CTG1?

		m_oreg[9] = m_region_code; // TODO: system region on Saturn

		/*
		 0-11 -1-- unknown
		 -x-- ---- VDP2 dot select
		 ---- x--- MSHNMI
		 ---- --x- SYSRES
		 ---- ---x SOUNDRES
		 */
		m_oreg[10] = 0 << 7 |
					 m_cur_dotsel << 6 |
					 1 << 5 |
					 1 << 4 |
					 0 << 3 |
					 1 << 2 |
					 0 << 1 |
					 0 << 0;

		m_oreg[11] = 0 << 6; // CDRES

		for(i=0;i<4;i++)
			m_oreg[12+i] = m_smem[i];

		for(i=0;i<15;i++)
			m_oreg[16+i] = 0xff; // undefined

		m_intback_stage = (m_intback_buf[1] & 8) >> 3; // first peripheral
		sr_set(0x40 | (m_intback_stage << 5));
		m_pmode = m_intback_buf[0]>>4;

		irq_request();

		// put issued command in OREG31
		m_oreg[31] = 0x10; // TODO: doc says 0?
		/* clear hand-shake flag */
		sf_ack(false);
	}
	else if(m_intback_buf[1] & 8)
	{
		m_intback_stage = (m_intback_buf[1] & 8) >> 3; // first peripheral
		sr_set(0x40);
		m_oreg[31] = 0x10;
		intback_continue_request();
	}
	else
	{
		/* Shienryu calls this, it would be plainly illegal on Saturn, I'll just return the command and clear the hs flag for now. */
		m_oreg[31] = 0x10;
		sf_ack(false);
	}
}

void smpc_hle_device::intback_continue_request()
{
	if( m_has_ctrl_ports == true )
		read_saturn_ports();

	if (m_intback_stage == 2)
	{
		sr_set(0x80 | m_pmode);     // pad 2, no more data, echo back pad mode set by intback
		m_intback_stage = 0;
	}
	else
	{
		sr_set(0xc0 | m_pmode);    // pad 1, more data, echo back pad mode set by intback
		m_intback_stage ++;
	}
	irq_request();

	m_oreg[31] = 0x10; // callback for last command issued
	sf_ack(false);
}

int smpc_hle_device::DectoBCD(int num)
{
	int i, cnt = 0, tmp, res = 0;

	while (num > 0) {
		tmp = num;
		while (tmp >= 10) tmp %= 10;
		for (i=0; i<cnt; i++)
			tmp *= 16;
		res += tmp;
		cnt++;
		num /= 10;
	}

	return res;
}

//**************************************************************************
//  RTC handling
//**************************************************************************

void smpc_hle_device::handle_rtc_increment()
{
	const uint8_t dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };
	int year_num, year_count;

	/*
	    m_smpc.rtc_data[0] = DectoBCD(systime.local_time.year /100);
	    m_smpc.rtc_data[1] = DectoBCD(systime.local_time.year %100);
	    m_smpc.rtc_data[2] = (systime.local_time.weekday << 4) | (systime.local_time.month+1);
	    m_smpc.rtc_data[3] = DectoBCD(systime.local_time.mday);
	    m_smpc.rtc_data[4] = DectoBCD(systime.local_time.hour);
	    m_smpc.rtc_data[5] = DectoBCD(systime.local_time.minute);
	    m_smpc.rtc_data[6] = DectoBCD(systime.local_time.second);
	*/

	m_rtc_data[6]++;

	/* seconds from 9 -> 10*/
	if((m_rtc_data[6] & 0x0f) >= 0x0a)         { m_rtc_data[6]+=0x10; m_rtc_data[6]&=0xf0; }
	/* seconds from 59 -> 0 */
	if((m_rtc_data[6] & 0xf0) >= 0x60)         { m_rtc_data[5]++;     m_rtc_data[6] = 0; }
	/* minutes from 9 -> 10 */
	if((m_rtc_data[5] & 0x0f) >= 0x0a)         { m_rtc_data[5]+=0x10; m_rtc_data[5]&=0xf0; }
	/* minutes from 59 -> 0 */
	if((m_rtc_data[5] & 0xf0) >= 0x60)         { m_rtc_data[4]++;     m_rtc_data[5] = 0; }
	/* hours from 9 -> 10 */
	if((m_rtc_data[4] & 0x0f) >= 0x0a)         { m_rtc_data[4]+=0x10; m_rtc_data[4]&=0xf0; }
	/* hours from 23 -> 0 */
	if((m_rtc_data[4] & 0xff) >= 0x24)             { m_rtc_data[3]++; m_rtc_data[2]+=0x10; m_rtc_data[4] = 0; }
	/* week day name sunday -> monday */
	if((m_rtc_data[2] & 0xf0) >= 0x70)             { m_rtc_data[2]&=0x0f; }
	/* day number 9 -> 10 */
	if((m_rtc_data[3] & 0x0f) >= 0x0a)             { m_rtc_data[3]+=0x10; m_rtc_data[3]&=0xf0; }

	// year BCD to dec conversion (for the leap year stuff)
	{
		year_num = (m_rtc_data[1] & 0xf);

		for(year_count = 0; year_count < (m_rtc_data[1] & 0xf0); year_count += 0x10)
			year_num += 0xa;

		year_num += (m_rtc_data[0] & 0xf)*0x64;

		for(year_count = 0; year_count < (m_rtc_data[0] & 0xf0); year_count += 0x10)
			year_num += 0x3e8;
	}

	/* month +1 check */
	/* the RTC have a range of 1980 - 2100, so we don't actually need to support the leap year special conditions */
	if(((year_num % 4) == 0) && (m_rtc_data[2] & 0xf) == 2)
	{
		if((m_rtc_data[3] & 0xff) >= dpm[(m_rtc_data[2] & 0xf)-1]+1+1)
			{ m_rtc_data[2]++; m_rtc_data[3] = 0x01; }
	}
	else if((m_rtc_data[3] & 0xff) >= dpm[(m_rtc_data[2] & 0xf)-1]+1){ m_rtc_data[2]++; m_rtc_data[3] = 0x01; }
	/* year +1 check */
	if((m_rtc_data[2] & 0x0f) > 12)                { m_rtc_data[1]++;  m_rtc_data[2] = (m_rtc_data[2] & 0xf0) | 0x01; }
	/* year from 9 -> 10 */
	if((m_rtc_data[1] & 0x0f) >= 0x0a)             { m_rtc_data[1]+=0x10; m_rtc_data[1]&=0xf0; }
	/* year from 99 -> 100 */
	if((m_rtc_data[1] & 0xf0) >= 0xa0)             { m_rtc_data[0]++; m_rtc_data[1] = 0; }

	// probably not SO precise, here just for reference ...
	/* year from 999 -> 1000 */
	//if((m_rtc_data[0] & 0x0f) >= 0x0a)               { m_rtc_data[0]+=0x10; m_rtc_data[0]&=0xf0; }
	/* year from 9999 -> 0 */
	//if((m_rtc_data[0] & 0xf0) >= 0xa0)               { m_rtc_data[0] = 0; } //roll over
}




/********************************************
 *
 * Saturn handlers
 *
 *******************************************/

/*
    [0] port status:
        0x04 Sega-tap
        0x16 Multi-tap
        0x2x clock serial peripheral
        0xf0 peripheral isn't connected
        0xf1 peripheral is connected
    [1] Peripheral ID (note: lowest four bits determines the size of the input packet)
        0x02 digital pad
        0x25 (tested by Game Basic?)
        0x34 keyboard

 Lower 4 bits of the port status tell the number of controllers to check for the port
 Lower 4 bits of the peripheral ID tell the number of registers used by each controller
 For multitap / segatap, we have implemented the following logic:
 SMPC reads in sequence
 - status for port 1
 - ID first controller, followed by the number of reads needed by the plugged controller
 - ID second controller, followed by the number of reads needed by the plugged controller
 - and so on... until the 4th (for SegaTap) or 6th (for Multitap) controller is read
 TODO: how does the multitap check if a controller is connected? does it ask for the
 controller status of each subport? how does this work exactly?
 currently, there is a small problem in some specific controller config which seems to
 lose track of one controller. E.g. if I put multitap in port2 with inserted joy1, joy2 and joy4
 it does not see joy4 controller, but if I put joy1, joy2, joy4 and joy5 it sees
 all four of them. The same happens if I skip controllers with id = 0xff...
 how did a real unit behave in this case?
*/

void smpc_hle_device::read_saturn_ports()
{
	uint8_t status1 = m_ctrl1 ? m_ctrl1->read_status() : 0xf0;
	uint8_t status2 = m_ctrl2 ? m_ctrl2->read_status() : 0xf0;

	uint8_t reg_offset = 0;
	uint8_t ctrl1_offset = 0;     // this is used when there is segatap or multitap connected
	uint8_t ctrl2_offset = 0;     // this is used when there is segatap or multitap connected

	m_oreg[reg_offset++] = status1;

	// read ctrl1
	for (int i = 0; i < (status1 & 0xf); i++)
	{
		uint8_t id = m_ctrl1->read_id(i);

		m_oreg[reg_offset++] = id;
		for (int j = 0; j < (id & 0xf); j++)
			m_oreg[reg_offset++] = m_ctrl1->read_ctrl(j + ctrl1_offset);

		ctrl1_offset += (id & 0xf);
	}

	m_oreg[reg_offset++] = status2;

	// read ctrl2
	for (int i = 0; i < (status2 & 0xf); i++)
	{
		uint8_t id = m_ctrl2->read_id(i);

		m_oreg[reg_offset++] = id;

		for (int j = 0; j < (id & 0xf); j++)
			m_oreg[reg_offset++] = m_ctrl2->read_ctrl(j + ctrl2_offset);

		ctrl2_offset += (id & 0xf);
	}
}

INPUT_CHANGED_MEMBER(smpc_hle_device::trigger_nmi_r )
{
	// punt if NMI trigger is disabled
	if(!m_NMI_reset)
		return;

	// TODO: generated during the 3VINT period according to manual
	if(newval)
		master_sh2_nmi();
}

/* Official documentation says that the "RESET/TAS opcodes aren't supported", but Out Run definitely contradicts with it.
   Since that m68k can't reset itself via the RESET opcode I suppose that the SMPC actually do it by reading an i/o
   connected to this opcode. */
void smpc_hle_device::m68k_reset_trigger()
{
	m_sndres_timer->adjust(attotime::from_usec(100));
}
