// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * machine/tpi6525.h
 *
 * mos tri port interface 6525
 * mos triple interface adapter 6523
 *
 * peter.trauner@jk.uni-linz.ac.at
 *
 * used in commodore b series
 * used in commodore c1551 floppy disk drive
 *
 * tia6523 is a tpi6525 without control register!?
 *
 * tia6523
 *   only some lines of port b and c are in the pinout!
 *
 * connector to floppy c1551 (delivered with c1551 as c16 expansion)
 *   port a for data read/write
 *   port b
 *   0 status 0
 *   1 status 1
 *   port c
 *   6 dav output edge data on port a available
 *   7 ack input edge ready for next datum
 *
 ****************************************************************************/

#ifndef __TPI6525_H__
#define __TPI6525_H__


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class tpi6525_device : public device_t
{
public:
	tpi6525_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tpi6525_device() {}

	template<class _Object> static devcb_base &set_out_irq_callback(device_t &device, _Object object) { return downcast<tpi6525_device &>(device).m_out_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pa_callback(device_t &device, _Object object) { return downcast<tpi6525_device &>(device).m_in_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pa_callback(device_t &device, _Object object) { return downcast<tpi6525_device &>(device).m_out_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pb_callback(device_t &device, _Object object) { return downcast<tpi6525_device &>(device).m_in_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pb_callback(device_t &device, _Object object) { return downcast<tpi6525_device &>(device).m_out_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pc_callback(device_t &device, _Object object) { return downcast<tpi6525_device &>(device).m_in_pc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pc_callback(device_t &device, _Object object) { return downcast<tpi6525_device &>(device).m_out_pc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_ca_callback(device_t &device, _Object object) { return downcast<tpi6525_device &>(device).m_out_ca_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_cb_callback(device_t &device, _Object object) { return downcast<tpi6525_device &>(device).m_out_cb_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( i0_w );
	DECLARE_WRITE_LINE_MEMBER( i1_w );
	DECLARE_WRITE_LINE_MEMBER( i2_w );
	DECLARE_WRITE_LINE_MEMBER( i3_w );
	DECLARE_WRITE_LINE_MEMBER( i4_w );

	DECLARE_READ8_MEMBER( pa_r );
	DECLARE_READ8_MEMBER( pb_r );
	DECLARE_READ8_MEMBER( pc_r );
	DECLARE_WRITE8_MEMBER( pa_w );
	DECLARE_WRITE8_MEMBER( pb_w );
	DECLARE_WRITE8_MEMBER( pc_w );

	WRITE_LINE_MEMBER( pb0_w ) { port_line_w(m_in_b, 0, state); }
	WRITE_LINE_MEMBER( pb1_w ) { port_line_w(m_in_b, 1, state); }
	WRITE_LINE_MEMBER( pb2_w ) { port_line_w(m_in_b, 2, state); }
	WRITE_LINE_MEMBER( pb3_w ) { port_line_w(m_in_b, 3, state); }
	WRITE_LINE_MEMBER( pb4_w ) { port_line_w(m_in_b, 4, state); }
	WRITE_LINE_MEMBER( pb5_w ) { port_line_w(m_in_b, 5, state); }
	WRITE_LINE_MEMBER( pb6_w ) { port_line_w(m_in_b, 6, state); }
	WRITE_LINE_MEMBER( pb7_w ) { port_line_w(m_in_b, 7, state); }

	UINT8 get_ddr_a();
	UINT8 get_ddr_b();
	UINT8 get_ddr_c();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	devcb_write_line    m_out_irq_cb;

	devcb_read8         m_in_pa_cb;
	devcb_write8        m_out_pa_cb;

	devcb_read8         m_in_pb_cb;
	devcb_write8        m_out_pb_cb;

	devcb_read8         m_in_pc_cb;
	devcb_write8        m_out_pc_cb;

	devcb_write_line    m_out_ca_cb;
	devcb_write_line    m_out_cb_cb;

	UINT8 m_port_a, m_ddr_a, m_in_a;
	UINT8 m_port_b, m_ddr_b, m_in_b;
	UINT8 m_port_c, m_ddr_c, m_in_c;

	UINT8 m_ca_level, m_cb_level, m_interrupt_level;

	UINT8 m_cr;
	UINT8 m_air;

	UINT8 m_irq_level[5];

	void set_interrupt();
	void clear_interrupt();

	// helper function to write a single line
	static void port_line_w(UINT8 &port, int line, int state);
};

extern const device_type TPI6525;


#define MCFG_TPI6525_OUT_IRQ_CB(_devcb) \
	devcb = &tpi6525_device::set_out_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_TPI6525_IN_PA_CB(_devcb) \
	devcb = &tpi6525_device::set_in_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_TPI6525_OUT_PA_CB(_devcb) \
	devcb = &tpi6525_device::set_out_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_TPI6525_IN_PB_CB(_devcb) \
	devcb = &tpi6525_device::set_in_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_TPI6525_OUT_PB_CB(_devcb) \
	devcb = &tpi6525_device::set_out_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_TPI6525_IN_PC_CB(_devcb) \
	devcb = &tpi6525_device::set_in_pc_callback(*device, DEVCB_##_devcb);

#define MCFG_TPI6525_OUT_PC_CB(_devcb) \
	devcb = &tpi6525_device::set_out_pc_callback(*device, DEVCB_##_devcb);

#define MCFG_TPI6525_OUT_CA_CB(_devcb) \
	devcb = &tpi6525_device::set_out_ca_callback(*device, DEVCB_##_devcb);

#define MCFG_TPI6525_OUT_CB_CB(_devcb) \
	devcb = &tpi6525_device::set_out_cb_callback(*device, DEVCB_##_devcb);


#endif /* __TPI6525_H__ */
