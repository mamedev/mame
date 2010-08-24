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
    L   H   X   X | H   L
    H   L   X   X | L   H
    L   L   X   X | H   H  (Note 1)
    H   H  _-   X | D  /D
    H   H   L   X | Q0 /Q01
    --------------+-------
    L   = lo (0)
    H   = hi (1)
    X   = any state
    _-  = raising edge
    Q0  = previous state

    Note 1: Non-stable configuration

*****************************************************************************/

#pragma once

#ifndef __TTL7474_H__
#define __TTL7474_H__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_7474_ADD(_tag, _target_tag, _output_cb, _comp_output_cb) \
    MDRV_DEVICE_ADD(_tag, MACHINE_TTL7474, 0) \
    MDRV_7474_TARGET_TAG(_target_tag) \
    MDRV_7474_OUTPUT_CB(_output_cb) \
    MDRV_7474_COMP_OUTPUT_CB(_comp_output_cb)

#define MDRV_7474_REPLACE(_tag, _target_tag, _output_cb, _comp_output_cb) \
    MDRV_DEVICE_REPLACE(_tag, TTL7474, 0) \
    MDRV_7474_TARGET_TAG(_target_tag) \
    MDRV_7474_OUTPUT_CB(_output_cb) \
    MDRV_7474_COMP_OUTPUT_CB(_comp_output_cb)

#define MDRV_7474_TARGET_TAG(_target_tag) \
    MDRV_DEVICE_INLINE_DATAPTR(ttl7474_device_config::INLINE_TARGET_TAG, _target_tag)

#define MDRV_7474_OUTPUT_CB(_cb) \
    MDRV_DEVICE_INLINE_DATAPTR(ttl7474_device_config::INLINE_OUTPUT_CB, _cb)

#define MDRV_7474_COMP_OUTPUT_CB(_cb) \
    MDRV_DEVICE_INLINE_DATAPTR(ttl7474_device_config::INLINE_COMP_OUTPUT_CB, _cb)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ttl7474_device_config

class ttl7474_device_config :  public device_config
{
    friend class ttl7474_device;

    // construction/destruction
    ttl7474_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

    // inline configuration indexes go here
    enum
    {
		INLINE_TARGET_TAG,
        INLINE_OUTPUT_CB,
        INLINE_COMP_OUTPUT_CB
    };

protected:
    // device_config overrides
    virtual void device_config_complete();

    // internal state goes here
	const char *m_target_tag;
	void (*m_base_output_cb)(device_t *device, INT32);
	void (*m_base_comp_output_cb)(device_t *device, INT32);

    devcb_write_line m_output_cb;
    devcb_write_line m_comp_output_cb;
};



// ======================> ttl7474_device

class ttl7474_device : public device_t
{
    friend class ttl7474_device_config;

    // construction/destruction
    ttl7474_device(running_machine &_machine, const ttl7474_device_config &config);

public:
    void clear_w(UINT8 state);
    void preset_w(UINT8 state);
    void clock_w(UINT8 state);
    void d_w(UINT8 state);
    UINT8 output_r();
    UINT8 output_comp_r();    /* NOT strictly the same as !ttl7474_output_r() */

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

    // internal state
    const ttl7474_device_config &m_config;

private:
    /* callbacks */
    devcb_resolved_write_line m_output_cb;
    devcb_resolved_write_line m_comp_output_cb;

    /* inputs */
    UINT8 m_clear;              /* pin 1/13 */
    UINT8 m_preset;             /* pin 4/10 */
    UINT8 m_clk;              	/* pin 3/11 */
    UINT8 m_d;                  /* pin 2/12 */

    /* outputs */
    UINT8 m_output;             /* pin 5/9 */
    UINT8 m_output_comp;        /* pin 6/8 */

    /* internal */
    UINT8 m_last_clock;
    UINT8 m_last_output;
    UINT8 m_last_output_comp;

    void update();
    void init();
};


// device type definition
extern const device_type MACHINE_TTL7474;



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_DEVICE_HANDLER( ttl7474_clear_w );
WRITE_LINE_DEVICE_HANDLER( ttl7474_preset_w );
WRITE_LINE_DEVICE_HANDLER( ttl7474_clock_w );
WRITE_LINE_DEVICE_HANDLER( ttl7474_d_w );
READ_LINE_DEVICE_HANDLER( ttl7474_output_r );
READ_LINE_DEVICE_HANDLER( ttl7474_output_comp_r );    /* NOT strictly the same as !ttl7474_output_r() */


#endif /* __TTL7474_H__ */