/***************************************************************************

    Intel 8089 I/O Processor

***************************************************************************/

#include "i8089.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define VERBOSE 1


//**************************************************************************
//  I/O CHANNEL
//**************************************************************************

#define MCFG_I8089_CHANNEL_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, I8089_CHANNEL, 0)

#define MCFG_I8089_CHANNEL_SINTR(_sintr) \
	downcast<i8089_channel *>(device)->set_sintr_callback(DEVCB2_##_sintr);

const device_type I8089_CHANNEL = &device_creator<i8089_channel>;

class i8089_channel : public device_t,
                      public device_execute_interface
{
public:
	// construction/destruction
	i8089_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _sintr> void set_sintr_callback(_sintr sintr) { m_write_sintr.set_callback(sintr); }

	void set_control_block(offs_t address);
	void attention();
	bool priority();
	bool bus_load_limit();

	DECLARE_WRITE_LINE_MEMBER( ext_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void execute_run();

	int m_icount;

private:

	// opcodes
	void hlt();
	void lpd(int p, int m, int o = 0);
	void lpdi(int p, int i, int o = 0);
	void movb_mr(int m, int r, int o = 0);
	void movb_rm(int r, int m, int o = 0);
	void movb_mm(int m1, int m2, int o1 = 0, int o2 = 0);
	void movbi_ri(int r, int i);
	void movbi_mi(int m, int i, int o = 0);
	void movi_ri(int r, int i);
	void movi_mi(int m, int i, int o = 0);
	void movp_mp(int m, int p, int o = 0);
	void movp_pm(int p, int m, int o = 0);
	void nop();
	void sintr();

	void examine_ccw(UINT8 ccw);

	devcb2_write_line m_write_sintr;

	enum
	{
		STATE_IDLE,
		STATE_EXECUTING,
		STATE_DMA
	};

	i8089_device *m_iop;

	int m_state;

	bool m_interrupts_enabled;

	// register
	enum
	{
		GA, // 20-bit general purpose address a
		GB, // 20-bit general purpose address b
		GC, // 20-bit general purpose address c
		BC, // byte count
		TP, // 20-bit task pointer
		IX, // byte count
		CC, // mask compare
		MC, // channel control

		// internal use register
		CP, // 20-bit control block pointer
		PP, // 20-bit parameter pointer
		PSW // program status word
	};

	struct
	{
		int w; // 20-bit address
		int t; // tag-bit
	}
	m_r[11];
};

i8089_channel::i8089_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, I8089_CHANNEL, "Intel 8089 I/O Channel", tag, owner, clock, "i8089_channel", __FILE__),
	device_execute_interface(mconfig, *this),
	m_icount(0),
	m_write_sintr(*this),
	m_state(STATE_IDLE),
	m_interrupts_enabled(true)
{
}

void i8089_channel::device_start()
{
	// get parent device
	m_iop = downcast<i8089_device *>(owner());

	// set our instruction counter
	m_icountptr = &m_icount;

	// resolve callbacks
	m_write_sintr.resolve_safe();

	// register for save states
	save_item(NAME(m_state));
	save_item(NAME(m_interrupts_enabled));

	for (int i = 0; i < ARRAY_LENGTH(m_r); i++)
	{
		save_item(NAME(m_r[i].w), i);
		save_item(NAME(m_r[i].t), i);
	}
}

void i8089_channel::execute_run()
{
	do
	{
		switch (m_state)
		{
		case STATE_IDLE:
			m_icount = 0;
			break;

		case STATE_DMA:
			m_icount = 0;
			break;

		case STATE_EXECUTING:
			m_icount--;
			break;
		}
	}
	while (m_icount > 0);
}

void i8089_channel::set_control_block(offs_t address)
{
	m_r[CP].w = address;
}

bool i8089_channel::bus_load_limit()
{
	return BIT(m_r[CP].w, 5);
}

bool i8089_channel::priority()
{
	return BIT(m_r[CP].w, 7);
}

void i8089_channel::examine_ccw(UINT8 ccw)
{
	// interrupt control field
	switch ((ccw >> 3) & 0x03)
	{
	// no effect
	case 0:
		break;

	// acknowledge interrupt
	case 1:
		if (VERBOSE)
			logerror("%s('%s'): acknowledge interrupt\n", shortname(), tag());

		m_write_sintr(0);
		break;

	// enable interrupts
	case 2:
		if (VERBOSE)
			logerror("%s('%s'): interrupts enabled\n", shortname(), tag());

		m_interrupts_enabled = true;
		break;

	// disable interrupts
	case 3:
		if (VERBOSE)
			logerror("%s('%s'): interrupts disabled\n", shortname(), tag());

		m_write_sintr(0);
		m_interrupts_enabled = false;
		break;
	}
}

void i8089_channel::attention()
{
	// examine control byte
	UINT8 ccw = m_iop->read_byte(m_r[CP].w);

	switch (ccw & 0x07)
	{
	// no channel command
	case 0:
		if (VERBOSE)
			logerror("%s('%s'): command received: no channel command\n", shortname(), tag());

		examine_ccw(ccw);
		break;

	// start channel, tb in local space
	case 1:
		if (VERBOSE)
			logerror("%s('%s'): command received: start channel in local space\n", shortname(), tag());

		examine_ccw(ccw);

		lpd(PP, CP, 2);
		movp_pm(TP, PP);
		movbi_mi(CP, 0xff, 1);

		m_state = STATE_EXECUTING;

		break;

	// reserved
	case 2:
		if (VERBOSE)
			logerror("%s('%s'): command received: invalid command 010\n", shortname(), tag());

		break;

	// start channel, tb in system space
	case 3:
		if (VERBOSE)
			logerror("%s('%s'): command received: start channel in system space\n", shortname(), tag());

		examine_ccw(ccw);

		lpd(PP, CP, 2);
		lpd(TP, PP);
		movbi_mi(CP, 0xff, 1);

		m_state = STATE_EXECUTING;

		if (VERBOSE)
		{
			logerror("%s('%s'): ---- starting channel ----\n", shortname(), tag());
			logerror("%s('%s'): parameter block address: %06x\n", shortname(), tag(), m_r[PP].w);
			logerror("%s('%s'): task pointer: %06x\n", shortname(), tag(), m_r[TP].w);
		}

		break;

	case 4:
		if (VERBOSE)
			logerror("%s('%s'): command received: invalid command 100\n", shortname(), tag());

		break;

	// continue channel processing
	case 5:
		if (VERBOSE)
			logerror("%s('%s'): command received: continue channel processing\n", shortname(), tag());

		// restore task pointer and parameter block
		movp_pm(TP, PP);
		movb_rm(PSW, PP, 3);
		movbi_mi(CP, 0xff, 1);

		break;

	// halt channel, save tp
	case 6:
		if (VERBOSE)
			logerror("%s('%s'): command received: halt channel and save tp\n", shortname(), tag());

		// save task pointer and psw to parameter block
		movp_mp(PP, TP);
		movb_mr(PP, PSW, 3);
		hlt();

		break;

	// halt channel, don't save tp
	case 7:
		if (VERBOSE)
			logerror("%s('%s'): command received: halt channel\n", shortname(), tag());

		hlt();

		break;
	}
}

WRITE_LINE_MEMBER( i8089_channel::ext_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ext_w: %d\n", shortname(), tag(), state);
}

WRITE_LINE_MEMBER( i8089_channel::drq_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ext_w: %d\n", shortname(), tag(), state);
}


//**************************************************************************
//  OPCODES
//**************************************************************************

// halt
void i8089_channel::hlt()
{
	movbi_mi(CP, 0x00, 1);
	m_state = STATE_IDLE;
}

// load pointer from memory
void i8089_channel::lpd(int p, int m, int o)
{
	UINT16 offset = m_iop->read_word(m_r[m].w + o);
	UINT16 segment = m_iop->read_word(m_r[m].w + o + 2);
	m_r[p].w = (segment << 4) + (offset & 0xfffff);
	m_r[p].t = 0;
}

// load pointer from immediate data
void i8089_channel::lpdi(int p, int i, int o)
{
	m_r[p].w = (o << 4) + (i & 0xffff);
	m_r[p].t = 0;
}

// move register to memory byte
void i8089_channel::movb_mr(int m, int r, int o)
{
	m_iop->write_byte(m_r[m].w + o, m_r[r].w & 0xff);
}

// move memory byte to register
void i8089_channel::movb_rm(int r, int m, int o)
{
	UINT8 byte = m_iop->read_byte(m_r[m].w + o);
	m_r[r].w = (BIT(byte, 7) ? 0xffff0 : 0x00000) | byte;
	m_r[r].t = 1;
}

// move memory byte to memory byte
void i8089_channel::movb_mm(int m1, int m2, int o1, int o2)
{
	UINT8 byte = m_iop->read_byte(m_r[m2].w + o2);
	m_iop->write_byte(m_r[m1].w + o1, byte);
}

// move immediate byte to register
void i8089_channel::movbi_ri(int r, int i)
{
	m_r[r].w = (BIT(i, 7) ? 0xffff0 : 0x00000) | (i & 0xff);
	m_r[r].t = 1;
}

// move immediate byte to memory byte
void i8089_channel::movbi_mi(int m, int i, int o)
{
	m_iop->write_byte(m_r[m].w + o, i & 0xff);
}

// move immediate word to register
void i8089_channel::movi_ri(int r, int i)
{
	m_r[r].w = (BIT(i, 15) ? 0xffff0 : 0x00000) | (i & 0xffff);
	m_r[r].t = 1;
}

// move immediate word to memory word
void i8089_channel::movi_mi(int m, int i, int o)
{
	m_iop->write_word(m_r[m].w + o, (BIT(i, 15) ? 0xffff0 : 0x00000) | (i & 0xffff));
}

// move pointer to memory (store)
void i8089_channel::movp_mp(int m, int p, int o)
{
	m_iop->write_word(m_r[m].w + o, m_r[p].w & 0xffff);
	m_iop->write_word(m_r[m].w + o + 2, (m_r[p].w >> 12 & 0xf0) | (m_r[p].t << 3 & 0x01));
}

// move memory to pointer (restore)
void i8089_channel::movp_pm(int p, int m, int o)
{
	UINT16 offset = m_iop->read_word(m_r[m].w + o);
	UINT16 segment = m_iop->read_word(m_r[m].w +o + 2);

	m_r[p].w = ((segment << 4) + offset) & 0x0fffff;
	m_r[p].t = segment >> 3 & 0x01;
}

// no operation
void i8089_channel::nop()
{
}

// set interrupt service flip-flop
void i8089_channel::sintr()
{
	if (m_interrupts_enabled)
		m_write_sintr(1);
}


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type I8089 = &device_creator<i8089_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8089_device - constructor
//-------------------------------------------------

i8089_device::i8089_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, I8089, "Intel 8089", tag, owner, clock, "i8089", __FILE__),
	m_ch1(*this, "1"),
	m_ch2(*this, "2"),
	m_write_sintr1(*this),
	m_write_sintr2(*this),
	m_16bit_system(false),
	m_16bit_remote(false),
	m_master(false),
	m_request_grant(false),
	m_ca(0),
	m_sel(0)
{
}

void i8089_device::static_set_cputag(device_t &device, const char *tag)
{
	i8089_device &i8089 = downcast<i8089_device &>(device);
	i8089.m_cputag = tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8089_device::device_start()
{
	// make sure our channels have been setup first
	if (!m_ch1->started() || !m_ch2->started())
		throw device_missing_dependencies();

	// resolve callbacks
	m_write_sintr1.resolve_safe();
	m_write_sintr2.resolve_safe();

	// register for save states
	save_item(NAME(m_16bit_system));
	save_item(NAME(m_16bit_remote));
	save_item(NAME(m_master));
	save_item(NAME(m_request_grant));
	save_item(NAME(m_ca));
	save_item(NAME(m_sel));

	// get pointers to memory
	device_t *cpu = machine().device(m_cputag);
	m_mem = &cpu->memory().space(AS_PROGRAM);
	m_io = &cpu->memory().space(AS_IO);

	// initialize channel clock
	m_ch1->set_unscaled_clock(clock());
	m_ch2->set_unscaled_clock(clock());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8089_device::device_reset()
{
	m_initialized = false;
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( i8089 )
	MCFG_I8089_CHANNEL_ADD("1")
	MCFG_I8089_CHANNEL_SINTR(WRITELINE(i8089_device, ch1_sintr_w))
	MCFG_I8089_CHANNEL_ADD("2")
	MCFG_I8089_CHANNEL_SINTR(WRITELINE(i8089_device, ch2_sintr_w))
MACHINE_CONFIG_END

machine_config_constructor i8089_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( i8089 );
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

// the i8089 actually executes a program from internal rom here:
//
// MOVB SYSBUS from FFFF6
// LPD System Configuration Block from FFFF8
// MOVB SOC from (SCB)
// LPD Control Pointer (CP) from (SCB) + 2
// MOVBI "00" to CP + 1 (clear busy flag)

void i8089_device::initialize()
{
	assert(!m_initialized);

	// get system bus width
	UINT8 sys_bus = m_mem->read_byte(0xffff6);
	m_16bit_system = BIT(sys_bus, 0);

	// get system configuration block address
	UINT16 scb_offset = read_word(0xffff8);
	UINT16 scb_segment = read_word(0xffffa);
	offs_t scb_address = ((scb_segment << 4) + scb_offset) & 0x0fffff;

	// get system operation command
	UINT16 soc = read_word(scb_address);
	m_16bit_remote = BIT(soc, 0);
	m_request_grant = BIT(soc, 1);
	m_master = !m_sel;

	// get control block address
	UINT16 cb_offset = read_word(scb_address + 2);
	UINT16 cb_segment = read_word(scb_address + 4);
	offs_t cb_address = ((cb_segment << 4) + cb_offset) & 0x0fffff;

	// initialize channels
	m_ch1->set_control_block(cb_address);
	m_ch2->set_control_block(cb_address + 8);

	// clear busy
	UINT16 ccw = read_word(cb_address);
	write_word(cb_address, ccw & 0x00ff);

	// done
	m_initialized = true;

	// output some debug info
	if (VERBOSE)
	{
		logerror("%s('%s'): ---- initializing ----\n", shortname(), basetag());
		logerror("%s('%s'): %s system bus\n", shortname(), basetag(), m_16bit_system ? "16-bit" : "8-bit");
		logerror("%s('%s'): system configuration block location: %06x\n", shortname(), basetag(), scb_address);
		logerror("%s('%s'): %s remote bus\n", shortname(), basetag(), m_16bit_remote ? "16-bit" : "8-bit");
		logerror("%s('%s'): request/grant: %d\n", shortname(), basetag(), m_request_grant);
		logerror("%s('%s'): is %s\n", shortname(), basetag(), m_master ? "master" : "slave");
		logerror("%s('%s'): channel control block location: %06x\n", shortname(), basetag(), cb_address);
	}
}

UINT8 i8089_device::read_byte(offs_t address)
{
	assert(m_initialized);
	return m_mem->read_byte(address);
}

UINT16 i8089_device::read_word(offs_t address)
{
	assert(m_initialized);

	UINT16 data = 0xffff;

	if (m_16bit_system)
	{
		data = m_mem->read_word(address);
	}
	else
	{
		data  = m_mem->read_byte(address);
		data |= m_mem->read_byte(address + 1) << 8;
	}

	return data;
}

void i8089_device::write_byte(offs_t address, UINT8 data)
{
	assert(m_initialized);
	m_mem->write_byte(address, data);
}

void i8089_device::write_word(offs_t address, UINT16 data)
{
	assert(m_initialized);

	if (m_16bit_system)
	{
		m_mem->write_word(address, data);
	}
	else
	{
		m_mem->write_byte(address, data & 0xff);
		m_mem->write_byte(address + 1, (data >> 8) & 0xff);
	}
}


//**************************************************************************
//  EXTERNAL INPUTS
//**************************************************************************

WRITE_LINE_MEMBER( i8089_device::ca_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ca_w: %u\n", shortname(), basetag(), state);

	if (m_ca == 1 && state == 0)
	{
		if (!m_initialized)
			initialize();
		else
		{
			if (m_sel == 0)
				m_ch1->attention();
			else
				m_ch2->attention();
		}
	}

	m_ca = state;
}

WRITE_LINE_MEMBER( i8089_device::drq1_w ) { m_ch1->drq_w(state); }
WRITE_LINE_MEMBER( i8089_device::drq2_w ) { m_ch2->drq_w(state); }
WRITE_LINE_MEMBER( i8089_device::ext1_w ) { m_ch1->ext_w(state); }
WRITE_LINE_MEMBER( i8089_device::ext2_w ) { m_ch2->ext_w(state); }
