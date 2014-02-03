/**********************************************************************

  Copyright (C) Antoine Mine' 2007

  Motorola 6843 Floppy Disk Controller emulation.

**********************************************************************/

#ifndef MC6843_H
#define MC6843_H

#include "imagedev/flopdrv.h"


struct mc6843_interface
{
	devcb_write_line m_irq_cb;
};

class mc6843_device : public device_t,
								public mc6843_interface
{
public:
	mc6843_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mc6843_device() {}
	
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void set_drive(int drive);
	void set_side(int side);
	void set_index_pulse(int index_pulse);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	
private:
	enum
	{
		TIMER_CONT
	};
	
	devcb_resolved_write_line m_irq_func;

	/* registers */
	UINT8 m_CTAR;       /* current track */
	UINT8 m_CMR;        /* command */
	UINT8 m_ISR;        /* interrupt status */
	UINT8 m_SUR;        /* set-up */
	UINT8 m_STRA;       /* status */
	UINT8 m_STRB;       /* status */
	UINT8 m_SAR;        /* sector address */
	UINT8 m_GCR;        /* general count */
	UINT8 m_CCR;        /* CRC control */
	UINT8 m_LTAR;       /* logical address track (=track destination) */

	/* internal state */
	UINT8  m_drive;
	UINT8  m_side;
	UINT8  m_data[128];   /* sector buffer */
	UINT32 m_data_size;   /* size of data */
	UINT32 m_data_idx;    /* current read/write position in data */
	UINT32 m_data_id;     /* chrd_id for sector write */
	UINT8  m_index_pulse;

	/* trigger delayed actions (bottom halves) */
	emu_timer* m_timer_cont;
	
	device_t* floppy_image(UINT8 drive);
	device_t* floppy_image();
	void status_update();
	void cmd_end();
	void finish_STZ();
	void finish_SEK();
	int address_search(chrn_id* id);
	int address_search_read(chrn_id* id);
	void finish_RCR();
	void cont_SR();
	void cont_SW();
	
};

extern const device_type MC6843;


#define MCFG_MC6843_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MC6843, 0)          \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MC6843_REMOVE(_tag)        \
	MCFG_DEVICE_REMOVE(_tag)


#endif
