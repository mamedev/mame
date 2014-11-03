// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 floppy disk controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

	- floppy format
	- spindle speed
	- stepper
    - read PLL
    - write logic

*/

#include "victor9k_fdc.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define I8048_TAG       "5d"
#define M6522_4_TAG     "1f"
#define M6522_5_TAG     "1k"
#define M6522_6_TAG     "1h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VICTOR_9000_FDC = &device_creator<victor_9000_fdc_t>;


//-------------------------------------------------
//  ROM( victor_9000_fdc )
//-------------------------------------------------

ROM_START( victor_9000_fdc )
	ROM_REGION( 0x400, I8048_TAG, 0)
	ROM_LOAD( "36080.5d", 0x000, 0x400, CRC(9bf49f7d) SHA1(b3a11bb65105db66ae1985b6f482aab6ea1da38b) )

	ROM_REGION( 0x800, "gcr", 0 )
	ROM_LOAD( "100836-001.4k", 0x000, 0x800, CRC(adc601bd) SHA1(6eeff3d2063ae2d97452101aa61e27ef83a467e5) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *victor_9000_fdc_t::device_rom_region() const
{
	return ROM_NAME( victor_9000_fdc );
}


//-------------------------------------------------
//  ADDRESS_MAP( floppy_io )
//-------------------------------------------------

static ADDRESS_MAP_START( floppy_io, AS_IO, 8, victor_9000_fdc_t )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ(floppy_p1_r) AM_WRITENOP
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(floppy_p2_r, floppy_p2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(tach0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(tach1_r)
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_WRITE(da_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  SLOT_INTERFACE( victor9k_floppies )
//-------------------------------------------------

void victor_9000_fdc_t::ready0_cb(floppy_image_device *device, int state)
{
	m_rdy0 = state;

	m_via5->write_ca2(m_rdy0);
}

int victor_9000_fdc_t::load0_cb(floppy_image_device *device)
{
	m_ds0 = 0;

	m_via4->write_ca1(m_ds0);

	return IMAGE_INIT_PASS;
}

void victor_9000_fdc_t::unload0_cb(floppy_image_device *device)
{
	m_ds0 = 1;

	m_via4->write_ca1(m_ds0);
}

void victor_9000_fdc_t::index0_cb(floppy_image_device *device, int state)
{
	m_tach0 = state;
}

void victor_9000_fdc_t::ready1_cb(floppy_image_device *device, int state)
{
	m_rdy1 = state;

	m_via5->write_cb2(m_rdy1);
}

int victor_9000_fdc_t::load1_cb(floppy_image_device *device)
{
	m_ds1 = 0;

	m_via4->write_cb1(m_ds1);

	return IMAGE_INIT_PASS;
}

void victor_9000_fdc_t::unload1_cb(floppy_image_device *device)
{
	m_ds1 = 1;

	m_via4->write_cb1(m_ds1);
}

void victor_9000_fdc_t::index1_cb(floppy_image_device *device, int state)
{
	m_tach1 = state;
}

static SLOT_INTERFACE_START( victor9k_floppies )
	SLOT_INTERFACE( "525ssqd", FLOPPY_525_SSQD ) // Tandon TM100-3 with custom electronics
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD ) // Tandon TM100-4 with custom electronics
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( victor_9000_fdc_t::floppy_formats )
	FLOPPY_VICTOR_9000_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( victor_9000_fdc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( victor_9000_fdc )
	MCFG_CPU_ADD(I8048_TAG, I8048, XTAL_30MHz/6)
	MCFG_CPU_IO_MAP(floppy_io)

	MCFG_DEVICE_ADD(M6522_4_TAG, VIA6522, XTAL_30MHz/30)
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(victor_9000_fdc_t, via4_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(victor_9000_fdc_t, via4_pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(victor_9000_fdc_t, wrsync_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(victor_9000_fdc_t, via4_irq_w))

	MCFG_DEVICE_ADD(M6522_5_TAG, VIA6522, XTAL_30MHz/30)
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(victor_9000_fdc_t, via5_irq_w))
	MCFG_VIA6522_READPA_HANDLER(READ8(victor_9000_fdc_t, via5_pa_r))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(victor_9000_fdc_t, via5_pb_w))

	MCFG_DEVICE_ADD(M6522_6_TAG, VIA6522, XTAL_30MHz/30)
	MCFG_VIA6522_READPA_HANDLER(READ8(victor_9000_fdc_t, via6_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(victor_9000_fdc_t, via6_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(victor_9000_fdc_t, via6_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(victor_9000_fdc_t, via6_pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(victor_9000_fdc_t, drw_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(victor_9000_fdc_t, erase_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(victor_9000_fdc_t, via6_irq_w))

	MCFG_FLOPPY_DRIVE_ADD(I8048_TAG":0", victor9k_floppies, "525qd", victor_9000_fdc_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(I8048_TAG":1", victor9k_floppies, "525qd", victor_9000_fdc_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor victor_9000_fdc_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( victor_9000_fdc );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  victor_9000_fdc_t - constructor
//-------------------------------------------------

victor_9000_fdc_t::victor_9000_fdc_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VICTOR_9000_FDC, "Victor 9000 FDC", tag, owner, clock, "victor9k_fdc", __FILE__),
	m_irq_cb(*this),
	m_syn_cb(*this),
	m_lbrdy_cb(*this),
	m_maincpu(*this, I8048_TAG),
	m_via4(*this, M6522_4_TAG),
	m_via5(*this, M6522_5_TAG),
	m_via6(*this, M6522_6_TAG),
	m_floppy0(*this, I8048_TAG":0:525qd"),
	m_floppy1(*this, I8048_TAG":1:525qd"),
	m_gcr_rom(*this, "gcr"),
	m_da(0),
	m_da0(0),
	m_da1(0),
	m_sel0(0),
	m_sel1(0),
	m_tach0(0),
	m_tach1(0),
	m_rdy0(0),
	m_rdy1(0),
	m_ds0(1),
	m_ds1(1),
	m_l0ms(0),
	m_l1ms(0),
	m_st0(0),
	m_st1(0),
	m_stp0(0),
	m_stp1(0),
	m_drive(0),
	m_side(0),
	m_via4_irq(CLEAR_LINE),
	m_via5_irq(CLEAR_LINE),
	m_via6_irq(CLEAR_LINE),
	m_syn(0),
	m_lbrdy(1)
{
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.write_position = 0;
	cur_live.write_start_time = attotime::never;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victor_9000_fdc_t::device_start()
{
	// allocate timer
	t_gen = timer_alloc(0);

	// state saving
	save_item(NAME(m_da));
	save_item(NAME(m_da0));
	save_item(NAME(m_da1));
	save_item(NAME(m_sel0));
	save_item(NAME(m_sel1));
	save_item(NAME(m_tach0));
	save_item(NAME(m_tach1));
	save_item(NAME(m_rdy0));
	save_item(NAME(m_rdy1));
	save_item(NAME(m_ds0));
	save_item(NAME(m_ds1));
	save_item(NAME(m_l0ms));
	save_item(NAME(m_l1ms));
	save_item(NAME(m_st0));
	save_item(NAME(m_st1));
	save_item(NAME(m_stp0));
	save_item(NAME(m_stp1));
	save_item(NAME(m_drive));
	save_item(NAME(m_side));
	save_item(NAME(m_drw));
	save_item(NAME(m_erase));
	save_item(NAME(m_via4_irq));
	save_item(NAME(m_via5_irq));
	save_item(NAME(m_via6_irq));
	save_item(NAME(m_syn));
	save_item(NAME(m_lbrdy));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void victor_9000_fdc_t::device_reset()
{
	live_abort();

	// resolve callbacks
	m_irq_cb.resolve_safe();
	m_syn_cb.resolve_safe();
	m_lbrdy_cb.resolve_safe();

	// reset devices
	m_via4->reset();
	m_via5->reset();
	m_via6->reset();

	// set floppy callbacks
	m_floppy0->setup_ready_cb(floppy_image_device::ready_cb(FUNC(victor_9000_fdc_t::ready0_cb), this));
	m_floppy0->setup_load_cb(floppy_image_device::load_cb(FUNC(victor_9000_fdc_t::load0_cb), this));
	m_floppy0->setup_unload_cb(floppy_image_device::unload_cb(FUNC(victor_9000_fdc_t::unload0_cb), this));
	m_floppy0->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(FUNC(victor_9000_fdc_t::index0_cb), this));
	m_floppy1->setup_ready_cb(floppy_image_device::ready_cb(FUNC(victor_9000_fdc_t::ready1_cb), this));
	m_floppy1->setup_load_cb(floppy_image_device::load_cb(FUNC(victor_9000_fdc_t::load1_cb), this));
	m_floppy1->setup_unload_cb(floppy_image_device::unload_cb(FUNC(victor_9000_fdc_t::unload1_cb), this));
	m_floppy1->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(FUNC(victor_9000_fdc_t::index1_cb), this));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void victor_9000_fdc_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	live_sync();
	live_run();
}


//-------------------------------------------------
//  floppy_p1_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::floppy_p1_r )
{
	/*

	    bit     description

	    0       L0MS0
	    1       L0MS1
	    2       L0MS2
	    3       L0MS3
	    4       L1MS0
	    5       L1MS1
	    6       L1MS2
	    7       L1MS3

	*/

	return (m_l1ms << 4) | m_l0ms;
}


//-------------------------------------------------
//  floppy_p2_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::floppy_p2_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6       RDY0
	    7       RDY1

	*/

	UINT8 data = 0;

	data |= m_rdy0 << 6;
	data |= m_rdy1 << 7;

	return data;
}


//-------------------------------------------------
//  floppy_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( victor_9000_fdc_t::floppy_p2_w )
{
	/*

	    bit     description

	    0       START0
	    1       STOP0
	    2       START1
	    3       STOP1
	    4       SEL1
	    5       SEL0
	    6
	    7

	*/

	bool sync = false;

	int mtr0 = m_mtr0;
	if ((data & 0x03) == 0x01) mtr0 = 0;
	if ((data & 0x03) == 0x02) mtr0 = 1;
	if (m_mtr0 != mtr0) sync = true;

	int mtr1 = m_mtr1;
	if ((data & 0x0c) == 0x04) mtr1 = 0;
	if ((data & 0x0c) == 0x08) mtr1 = 1;
	if (m_mtr1 != mtr1) sync = true;

	int sel0 = BIT(data, 5);
	if (m_sel0 != sel0) sync = true;

	int sel1 = BIT(data, 4);
	if (m_sel1 != sel1) sync = true;

	if (sync)
	{
		live_sync();

		m_mtr0 = mtr0;
		m_mtr1 = mtr1;
		m_sel0 = sel0;
		m_sel1 = sel1;

		if (LOG) logerror("%s MTR0 %u MTR1 %u SEL0 %u SEL1 %u\n", machine().time().as_string(), m_mtr0, m_mtr1, m_sel0, m_sel1);

		update_spindle_motor();
		checkpoint();

		if (!m_mtr0 || !m_mtr1) {
			if(cur_live.state == IDLE) {
				live_start();
			}
		} else {
			live_abort();
		}

		live_run();
	}
}


//-------------------------------------------------
//  tach0_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::tach0_r )
{
	return m_tach0;
}


//-------------------------------------------------
//  tach1_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::tach1_r )
{
	return m_tach1;
}


void victor_9000_fdc_t::update_stepper_motor(floppy_image_device *floppy, int stp, int old_st, int st)
{
	// TODO
}

void victor_9000_fdc_t::update_spindle_motor()
{
	if (m_sel0) m_da0 = m_da;
	m_floppy0->mon_w(m_mtr0);
	m_floppy0->set_rpm(300); // TODO

	if (m_sel1) m_da1 = m_da;
	m_floppy1->mon_w(m_mtr1);
	m_floppy1->set_rpm(300); // TODO
}


//-------------------------------------------------
//  da_w -
//-------------------------------------------------

WRITE8_MEMBER( victor_9000_fdc_t::da_w )
{
	if (m_da != data)
	{
		live_sync();
		m_da = data;
		update_spindle_motor();
		checkpoint();
		live_run();
	}
}

WRITE8_MEMBER( victor_9000_fdc_t::via4_pa_w )
{
	/*

	    bit     description

	    PA0     L0MS0
	    PA1     L0MS1
	    PA2     L0MS2
	    PA3     L0MS3
	    PA4     ST0A
	    PA5     ST0B
	    PA6     ST0C
	    PA7     ST0D

	*/

	m_l0ms = data & 0x0f;

	UINT8 st0 = data >> 4;

	if (m_st0 != st0)
	{
		live_sync();
		update_stepper_motor(m_floppy0, m_stp0, st0, m_st0);
		m_st0 = st0;
		checkpoint();
		live_run();
	}
}

WRITE8_MEMBER( victor_9000_fdc_t::via4_pb_w )
{
	/*

	    bit     description

	    PB0     L1MS0
	    PB1     L1MS1
	    PB2     L1MS2
	    PB3     L1MS3
	    PB4     ST1A
	    PB5     ST1B
	    PB6     ST1C
	    PB7     ST1D

	*/

	m_l1ms = data & 0x0f;

	UINT8 st1 = data >> 4;

	if (m_st1 != st1)
	{
		live_sync();
		update_stepper_motor(m_floppy1, m_stp1, st1, m_st1);
		m_st1 = st1;
		checkpoint();
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::wrsync_w )
{
	if (m_wrsync != state)
	{
		live_sync();
		m_wrsync = state;
		cur_live.wrsync = state;
		checkpoint();
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::via4_irq_w )
{
	m_via4_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}

READ8_MEMBER( victor_9000_fdc_t::via5_pa_r )
{
	/*

	    bit     description

	    PA0     E0
	    PA1     E1
	    PA2     I1
	    PA3     E2
	    PA4     E4
	    PA5     E5
	    PA6     I7
	    PA7     E6

	*/

	UINT8 e = checkpoint_live.e;
	UINT8 i = checkpoint_live.i;

	return BIT(e, 6) << 7 | BIT(i, 7) << 6 | BIT(e, 5) << 5 | BIT(e, 4) << 4 | BIT(e, 2) << 3 | BIT(i, 1) << 2 | (e & 0x03);
}

WRITE8_MEMBER( victor_9000_fdc_t::via5_pb_w )
{
	/*

	    bit     description

	    PB0     WD0
	    PB1     WD1
	    PB2     WD2
	    PB3     WD3
	    PB4     WD4
	    PB5     WD5
	    PB6     WD6
	    PB7     WD7

	*/

	if (m_wd != data)
	{
		live_sync();
		m_wd = data;
		cur_live.wd = data;
		checkpoint();
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::via5_irq_w )
{
	m_via5_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}


READ8_MEMBER( victor_9000_fdc_t::via6_pa_r )
{
	/*

	    bit     description

	    PA0
	    PA1     _TRK0D0
	    PA2
	    PA3     _TRK0D1
	    PA4
	    PA5
	    PA6     WPS
	    PA7     _SYNC

	*/

	UINT8 data = 0;

	// track 0 drive A sense
	data |= m_floppy0->trk00_r() << 1;

	// track 0 drive B sense
	data |= m_floppy1->trk00_r() << 3;

	// write protect sense
	data |= (m_drive ? m_floppy1->wpt_r() : m_floppy0->wpt_r()) << 6;

	// disk sync detect
	data |= checkpoint_live.sync << 7;

	return data;
}

WRITE8_MEMBER( victor_9000_fdc_t::via6_pa_w )
{
	/*

	    bit     description

	    PA0     LED0A
	    PA1
	    PA2     LED1A
	    PA3
	    PA4     SIDE SELECT
	    PA5     DRIVE SELECT
	    PA6
	    PA7

	*/

	// LED, drive A
	output_set_led_value(LED_A, BIT(data, 0));

	// LED, drive B
	output_set_led_value(LED_B, BIT(data, 2));

	bool sync = false;

	// dual side select
	int side = BIT(data, 4);
	if (m_side != side) sync = true;

	// select drive A/B
	int drive = BIT(data, 5);
	if (m_drive != drive) sync = true;

	if (sync)
	{
		live_sync();

		m_side = side;
		cur_live.side = side;
		m_floppy0->ss_w(side);
		m_floppy1->ss_w(side);

		m_drive = drive;
		cur_live.drive = drive;

		checkpoint();
		live_run();
	}
}

READ8_MEMBER( victor_9000_fdc_t::via6_pb_r )
{
	/*

	    bit     description

	    PB0     RDY0
	    PB1     RDY1
	    PB2
	    PB3     _DS1
	    PB4     _DS0
	    PB5     SINGLE/_DOUBLE SIDED
	    PB6
	    PB7

	*/

	UINT8 data = 0;

	// motor speed status, drive A
	data |= m_rdy0;

	// motor speed status, drive B
	data |= m_rdy1 << 1;

	// door B sense
	data |= m_ds1 << 3;

	// door A sense
	data |= m_ds0 << 4;

	// single/double sided
	data |= (m_drive ? m_floppy1->twosid_r() : m_floppy0->twosid_r()) << 5;

	return data;
}

WRITE8_MEMBER( victor_9000_fdc_t::via6_pb_w )
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2     _SCRESET
	    PB3
	    PB4
	    PB5
	    PB6     STP0
	    PB7     STP1

	*/

	// motor speed controller reset
	if (!BIT(data, 2))
		m_maincpu->reset();

	bool sync = false;

	// stepper enable A
	int stp0 = BIT(data, 6);
	if (m_stp0 != stp0) sync = true;

	// stepper enable B
	int stp1 = BIT(data, 7);
	if (m_stp1 != stp1) sync = true;

	if (sync)
	{
		live_sync();

		m_stp0 = stp0;
		update_stepper_motor(m_floppy0, m_stp0, m_st0, m_st0);

		m_stp1 = stp1;
		update_stepper_motor(m_floppy1, m_stp1, m_st1, m_st1);

		checkpoint();
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::drw_w )
{
	if (m_drw != state)
	{
		live_sync();
		m_drw = cur_live.drw = state;
		checkpoint();
		if (LOG) logerror("%s DRW %u\n", machine().time().as_string(), state);
		if (state) {
			stop_writing(machine().time());
		} else {
			start_writing(machine().time());
		}
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::erase_w )
{
	if (m_erase != state)
	{
		live_sync();
		m_erase = cur_live.erase = state;
		checkpoint();
		if (LOG) logerror("%s ERASE %u\n", machine().time().as_string(), state);
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::via6_irq_w )
{
	m_via6_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}

READ8_MEMBER( victor_9000_fdc_t::cs7_r )
{
	if (!checkpoint_live.lbrdy)
	{
		live_sync();
		cur_live.lbrdy = 1;
		m_lbrdy_cb(1);
		checkpoint();
		live_run();
	}

	return m_via5->read(space, offset);
}

WRITE8_MEMBER( victor_9000_fdc_t::cs7_w )
{
	if (!checkpoint_live.lbrdy)
	{
		live_sync();
		cur_live.lbrdy = 1;
		m_lbrdy_cb(1);
		checkpoint();
		live_run();
	}

	m_via5->write(space, offset, data);
}

floppy_image_device* victor_9000_fdc_t::get_floppy()
{
	return m_drive ? m_floppy1 : m_floppy0;
}

void victor_9000_fdc_t::live_start()
{
	cur_live.tm = machine().time();
	cur_live.state = RUNNING;
	cur_live.next_state = -1;

	cur_live.shift_reg = 0;
	cur_live.shift_reg_write = 0;
	cur_live.bit_counter = 0;

	cur_live.drive = m_drive;
	cur_live.side = m_side;
	cur_live.drw = m_drw;
	cur_live.wd = m_wd;
	cur_live.wrsync = m_wrsync;
	cur_live.erase = m_erase;

	checkpoint_live = cur_live;

	live_run();
}

void victor_9000_fdc_t::checkpoint()
{
	get_next_edge(machine().time());
	checkpoint_live = cur_live;
}

void victor_9000_fdc_t::rollback()
{
	cur_live = checkpoint_live;
	get_next_edge(cur_live.tm);
}

void victor_9000_fdc_t::start_writing(const attotime &tm)
{
	cur_live.write_start_time = tm;
	cur_live.write_position = 0;
}

void victor_9000_fdc_t::stop_writing(const attotime &tm)
{
	commit(tm);
	cur_live.write_start_time = attotime::never;
}

bool victor_9000_fdc_t::write_next_bit(bool bit, const attotime &limit)
{
	if(cur_live.write_start_time.is_never()) {
		cur_live.write_start_time = cur_live.tm;
		cur_live.write_position = 0;
	}

	attotime etime = cur_live.tm + m_period;
	if(etime > limit)
		return true;

	if(bit && cur_live.write_position < ARRAY_LENGTH(cur_live.write_buffer))
		cur_live.write_buffer[cur_live.write_position++] = cur_live.tm;

	if (LOG) logerror("%s write bit %u (%u)\n", cur_live.tm.as_string(), cur_live.bit_counter, bit);

	return false;
}

void victor_9000_fdc_t::commit(const attotime &tm)
{
	if(cur_live.write_start_time.is_never() || tm == cur_live.write_start_time || !cur_live.write_position)
		return;

	if (LOG) logerror("%s committing %u transitions since %s\n", tm.as_string(), cur_live.write_position, cur_live.write_start_time.as_string());

	if(get_floppy())
		get_floppy()->write_flux(cur_live.write_start_time, tm, cur_live.write_position, cur_live.write_buffer);

	cur_live.write_start_time = tm;
	cur_live.write_position = 0;
}

void victor_9000_fdc_t::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		t_gen->adjust(cur_live.tm - machine().time());
	else
		live_sync();
}

void victor_9000_fdc_t::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			rollback();
			live_run(machine().time());
			commit(cur_live.tm);
		} else {
			commit(cur_live.tm);
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				stop_writing(cur_live.tm);
				cur_live.tm = attotime::never;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void victor_9000_fdc_t::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	stop_writing(cur_live.tm);

	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.write_position = 0;
	cur_live.write_start_time = attotime::never;

	cur_live.brdy = 1;
	cur_live.lbrdy = 1;
	cur_live.sync = 1;
	cur_live.gcr_err = 1;
}

void victor_9000_fdc_t::live_run(const attotime &limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	for(;;) {
		switch(cur_live.state) {
		case RUNNING: {
			bool syncpoint = false;

			if (cur_live.tm > limit)
				return;

			// read bit
			int bit = get_next_bit(cur_live.tm, limit);
			if(bit < 0)
				return;

			cur_live.shift_reg <<= 1;
			cur_live.shift_reg |= bit;
			cur_live.shift_reg &= 0x3ff;

			// sync
			int sync = !(cur_live.shift_reg == 0x3ff);

			// bit counter
			if (!sync) {
				cur_live.bit_counter = 0;
			} else if (cur_live.sync) {
				cur_live.bit_counter++;
				if (cur_live.bit_counter == 10) {
					cur_live.bit_counter = 0;
				}
			}

			// GCR decoder
			if (cur_live.drw) {
				cur_live.i = cur_live.drw << 10 | cur_live.shift_reg;
			} else {
				cur_live.i = 0x300 | ((cur_live.wd & 0xf0) << 1) | cur_live.wrsync << 4 | (cur_live.wd & 0x0f);
			}

			cur_live.e = m_gcr_rom->base()[cur_live.i];

			// byte ready
			int brdy = cur_live.bit_counter == 9;

			// GCR error
			int gcr_err = !(brdy || BIT(cur_live.e, 3));

			if (brdy != cur_live.brdy) {
				if (LOG) logerror("%s BRDY %u\n", cur_live.tm.as_string(),brdy);
				cur_live.brdy = brdy;
				if (!brdy) cur_live.lbrdy = 0;
				syncpoint = true;
			}

			if (sync != cur_live.sync) {
				if (LOG) logerror("%s SYNC %u\n", cur_live.tm.as_string(),sync);
				cur_live.sync = sync;
				syncpoint = true;
			}

			if (gcr_err != cur_live.gcr_err) {
				if (LOG) logerror("%s GCR ERR %u\n", cur_live.tm.as_string(),gcr_err);
				cur_live.gcr_err = gcr_err;
				syncpoint = true;
			}

			if (syncpoint) {
				commit(cur_live.tm);

				cur_live.tm += m_period;
				live_delay(RUNNING_SYNCPOINT);
				return;
			}

			cur_live.tm += m_period;
			break;
		}

		case RUNNING_SYNCPOINT: {
			m_lbrdy_cb(cur_live.lbrdy);

			cur_live.state = RUNNING;
			checkpoint();
			break;
		}
		}
	}
}

void victor_9000_fdc_t::get_next_edge(const attotime &when)
{
	// TODO
}

int victor_9000_fdc_t::get_next_bit(attotime &tm, const attotime &limit)
{
	return -1; // TODO
}
