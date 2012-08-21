/***************************************************************************

    TTL74145

    BCD-to-Decimal decoder

***************************************************************************/

#ifndef __TTL74145_H__
#define __TTL74145_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************
#define MCFG_TTL74145_ADD(_tag, _intf) \
	MCFG_DEVICE_ADD(_tag, TTL74145, 0) \
	MCFG_DEVICE_CONFIG(_intf)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ttl74145_interface

struct ttl74145_interface
{
	devcb_write_line m_output_line_0_cb;
	devcb_write_line m_output_line_1_cb;
	devcb_write_line m_output_line_2_cb;
	devcb_write_line m_output_line_3_cb;
	devcb_write_line m_output_line_4_cb;
	devcb_write_line m_output_line_5_cb;
	devcb_write_line m_output_line_6_cb;
	devcb_write_line m_output_line_7_cb;
	devcb_write_line m_output_line_8_cb;
	devcb_write_line m_output_line_9_cb;
};

// ======================> ttl74145_device

class ttl74145_device :  public device_t,
						public ttl74145_interface
{
public:
    // construction/destruction
    ttl74145_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	UINT16 read();
	void write(UINT8 data);
protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete();

private:
	devcb_resolved_write_line	m_output_line_0_func;
	devcb_resolved_write_line	m_output_line_1_func;
	devcb_resolved_write_line	m_output_line_2_func;
	devcb_resolved_write_line	m_output_line_3_func;
	devcb_resolved_write_line	m_output_line_4_func;
	devcb_resolved_write_line	m_output_line_5_func;
	devcb_resolved_write_line	m_output_line_6_func;
	devcb_resolved_write_line	m_output_line_7_func;
	devcb_resolved_write_line	m_output_line_8_func;
	devcb_resolved_write_line	m_output_line_9_func;

	/* decoded number */
	UINT16 m_number;
};

// device type definition
extern const device_type TTL74145;

//**************************************************************************
//  DEFAULT INTERFACES
//**************************************************************************

extern const ttl74145_interface default_ttl74145;


#endif /* TTL74145 */
