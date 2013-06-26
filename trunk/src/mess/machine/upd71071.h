#ifndef __UPD71071_H__
#define __UPD71071_H__

#include "emu.h"

struct upd71071_intf
{
	const char* cputag;
	int m_upd_clock;
	devcb_write_line    m_out_hreq_cb;
	devcb_write_line    m_out_eop_cb;
	devcb_read16        m_dma_read_cb[4];
	devcb_write16       m_dma_write_cb[4];
	devcb_write_line    m_out_dack_cb[4];
};

struct upd71071_reg
{
	UINT8 initialise;
	UINT8 channel;
	UINT16 count_current[4];
	UINT16 count_base[4];
	UINT32 address_current[4];
	UINT32 address_base[4];
	UINT16 device_control;
	UINT8 mode_control[4];
	UINT8 status;
	UINT8 temp_l;
	UINT8 temp_h;
	UINT8 request;
	UINT8 mask;
};

class upd71071_device : public device_t,
						public upd71071_intf
{
public:
	upd71071_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~upd71071_device() {}

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(set_hreq);
	DECLARE_WRITE_LINE_MEMBER(set_eop);

	int dmarq(int state, int channel);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

private:
	// internal state
	void soft_reset();
	TIMER_CALLBACK_MEMBER(dma_transfer_timer);

	struct upd71071_reg m_reg;
	int m_selected_channel;
	int m_buswidth;
	int m_dmarq[4];
	emu_timer* m_timer[4];
	//int m_in_progress[4];
	//int m_transfer_size[4];
	int m_base;
	devcb_resolved_write_line   m_out_hreq_func;
	devcb_resolved_write_line   m_out_eop_func;
	devcb_resolved_read16       m_dma_read[4];
	devcb_resolved_write16      m_dma_write[4];
	devcb_resolved_write_line   m_out_dack_func[4];
	int m_hreq;
	int m_eop;
	cpu_device *m_cpu;
};

extern const device_type UPD71071;


#define MCFG_UPD71071_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, UPD71071, 0) \
	MCFG_DEVICE_CONFIG(_config)


#endif /*UPD71071_H_*/
