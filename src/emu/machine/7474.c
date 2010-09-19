/*****************************************************************************

  7474 positive-edge-triggered D-type flip-flop with preset, clear and
       complementary outputs.  There are 2 flip-flops per chips


  Pin layout and functions to access pins:

  clear_w        [1] /1CLR         VCC [14]
  d_w            [2]  1D         /2CLR [13]  clear_w
  clock_w        [3]  1CLK          2D [12]  d_w
  preset_w       [4] /1PR         2CLK [11]  clock_w
  output_r       [5]  1Q          /2PR [10]  preset_w
  output_comp_r  [6] /1Q            2Q [9]   output_r
                 [7]  GND          /2Q [8]   output_comp_r


  Truth table (all logic levels indicate the actual voltage on the line):

        INPUTS    | OUTPUTS
                  |
    PR  CLR CLK D | Q  /Q
    --------------+-------
 1  L   H   X   X | H   L
 2  H   L   X   X | L   H
 3  L   L   X   X | H   H  (Note 1)
 4  H   H  _-   X | D  /D
 5  H   H   L   X | Q0 /Q0
    --------------+-------
    L   = lo (0)
    H   = hi (1)
    X   = any state
    _-  = raising edge
    Q0  = previous state

    Note 1: Non-stable configuration

*****************************************************************************/

#include "emu.h"
#include "7474.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MACHINE_TTL7474 = ttl7474_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  ttl7474_device_config - constructor
//-------------------------------------------------

ttl7474_device_config::ttl7474_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
    : device_config(mconfig, static_alloc_device_config, "7474", tag, owner, clock)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *ttl7474_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
    return global_alloc(ttl7474_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *ttl7474_device_config::alloc_device(running_machine &machine) const
{
    return auto_alloc(&machine, ttl7474_device(machine, *this));
}


//-------------------------------------------------
//  static_set_target_tag - configuration helper
//  to set the target tag
//-------------------------------------------------

void ttl7474_device_config::static_set_target_tag(device_config *device, const char *tag)
{
	ttl7474_device_config *ttl7474 = downcast<ttl7474_device_config *>(device);
	ttl7474->m_output_cb.tag = tag;
	ttl7474->m_comp_output_cb.tag = tag;
}


//-------------------------------------------------
//  static_set_output_cb - configuration helper
//  to set the output callback
//-------------------------------------------------

void ttl7474_device_config::static_set_output_cb(device_config *device, write_line_device_func callback)
{
	ttl7474_device_config *ttl7474 = downcast<ttl7474_device_config *>(device);
	if (callback != NULL)
	{
		ttl7474->m_output_cb.type = DEVCB_TYPE_DEVICE;
		ttl7474->m_output_cb.writeline = callback;
	}
	else
		ttl7474->m_output_cb.type = DEVCB_TYPE_NULL;
}


//-------------------------------------------------
//  static_set_comp_output_cb - configuration
//  helper to set the comp. output callback
//-------------------------------------------------

void ttl7474_device_config::static_set_comp_output_cb(device_config *device, write_line_device_func callback)
{
	ttl7474_device_config *ttl7474 = downcast<ttl7474_device_config *>(device);
	if (callback != NULL)
	{
		ttl7474->m_comp_output_cb.type = DEVCB_TYPE_DEVICE;
		ttl7474->m_comp_output_cb.writeline = callback;
	}
	else
		ttl7474->m_comp_output_cb.type = DEVCB_TYPE_NULL;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ttl7474_device - constructor
//-------------------------------------------------

ttl7474_device::ttl7474_device(running_machine &_machine, const ttl7474_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{
    init();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl7474_device::device_start()
{
    state_save_register_device_item(this, 0, m_clear);
    state_save_register_device_item(this, 0, m_preset);
    state_save_register_device_item(this, 0, m_clk);
    state_save_register_device_item(this, 0, m_d);
    state_save_register_device_item(this, 0, m_output);
    state_save_register_device_item(this, 0, m_output_comp);
    state_save_register_device_item(this, 0, m_last_clock);
    state_save_register_device_item(this, 0, m_last_output);
    state_save_register_device_item(this, 0, m_last_output_comp);

	devcb_resolve_write_line(&m_output_cb, &m_config.m_output_cb, this);
	devcb_resolve_write_line(&m_comp_output_cb, &m_config.m_comp_output_cb, this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ttl7474_device::device_reset()
{
    init();
}


//-------------------------------------------------
//  update - update internal state
//-------------------------------------------------

void ttl7474_device::update()
{
    if (!m_preset && m_clear)       	/* line 1 in truth table */
	{
        m_output    = 1;
        m_output_comp = 0;
	}
    else if (m_preset && !m_clear)      /* line 2 in truth table */
	{
        m_output    = 0;
        m_output_comp = 1;
	}
    else if (!m_preset && !m_clear)     /* line 3 in truth table */
	{
        m_output    = 1;
        m_output_comp = 1;
	}
    else if (!m_last_clock && m_clk)	/* line 4 in truth table */
	{
        m_output    =  m_d;
        m_output_comp = !m_d;
	}

    m_last_clock = m_clk;


	/* call callback if any of the outputs changed */
    if (m_output != m_last_output)
	{
        m_last_output = m_output;
		devcb_call_write_line(&m_output_cb, m_output);
	}
	/* call callback if any of the outputs changed */
    if (m_output_comp != m_last_output_comp)
	{
        m_last_output_comp = m_output_comp;
		devcb_call_write_line(&m_comp_output_cb, m_output_comp);
	}
}


//-------------------------------------------------
//  clear_w - set the clear line state
//-------------------------------------------------

WRITE_LINE_DEVICE_HANDLER( ttl7474_clear_w )
{
    downcast<ttl7474_device *>(device)->clear_w(state);
}

void ttl7474_device::clear_w(UINT8 state)
{
    m_clear = state & 1;
	update();
}


//-------------------------------------------------
//  clear_w - set the clear line state
//-------------------------------------------------

WRITE_LINE_DEVICE_HANDLER( ttl7474_preset_w )
{
    downcast<ttl7474_device *>(device)->preset_w(state);
}

void ttl7474_device::preset_w(UINT8 state)
{
    m_preset = state & 1;
	update();
}


//-------------------------------------------------
//  clock_w - set the clock line state
//-------------------------------------------------

WRITE_LINE_DEVICE_HANDLER( ttl7474_clock_w )
{
    downcast<ttl7474_device *>(device)->clock_w(state);
}

void ttl7474_device::clock_w(UINT8 state)
{
    m_clk = state & 1;
	update();
}


//-------------------------------------------------
//  d_w - set the d line state
//-------------------------------------------------

WRITE_LINE_DEVICE_HANDLER( ttl7474_d_w )
{
    downcast<ttl7474_device *>(device)->d_w(state);
}

void ttl7474_device::d_w(UINT8 state)
{
    m_d = state & 1;
	update();
}


//-------------------------------------------------
//  output_r - get the output line state
//-------------------------------------------------

READ_LINE_DEVICE_HANDLER( ttl7474_output_r )
{
    return downcast<ttl7474_device *>(device)->output_r();
}

UINT8 ttl7474_device::output_r()
{
    return m_output;
}


//-----------------------------------------------------
//  output_comp_r - get the output-compare line state
//-----------------------------------------------------

READ_LINE_DEVICE_HANDLER( ttl7474_output_comp_r )
{
    return downcast<ttl7474_device *>(device)->output_comp_r();
}

UINT8 ttl7474_device::output_comp_r()
{
    return m_output_comp;
}

void ttl7474_device::init()
{
    m_clear = 1;
    m_preset = 1;
    m_clk = 1;
    m_d = 1;

    m_output = -1;
    m_last_clock = 1;
    m_last_output = -1;
}
