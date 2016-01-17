// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*********************************************************************

    53c7xx.h

    NCR 53C700 SCSI I/O Processor

*********************************************************************/

#ifndef _NCR53C7XX_H_
#define _NCR53C7XX_H_

#include "machine/nscsi_bus.h"


//**************************************************************************
//  REGISTER DEFINES (INCOMPLETE)
//**************************************************************************

#define SCNTL0_TRG          0x01
#define SCNTL0_AAP          0x02
#define SCNTL0_EPG          0x04
#define SCNTL0_EPC          0x08
#define SCNTL0_WATN         0x10
#define SCNTL0_START        0x20
#define SCNTL0_ARB_MASK     3
#define SCNTL0_ARB_SHIFT    6

#define SSTAT0_PAR          0x01
#define SSTAT0_RST          0x02
#define SSTAT0_UDC          0x04
#define SSTAT0_SGE          0x08
#define SSTAT0_SEL          0x10
#define SSTAT0_STO          0x20
#define SSTAT0_CMP          0x40
#define SSTAT0_MA           0x80

#define SSTAT1_SDP          0x01
#define SSTAT1_RST          0x02
#define SSTAT1_WOA          0x04
#define SSTAT1_LOA          0x08
#define SSTAT1_AIP          0x10
#define SSTAT1_ORF          0x20
#define SSTAT1_OLF          0x40
#define SSTAT1_ILF          0x80

#define ISTAT_DIP           0x01
#define ISTAT_SIP           0x02
#define ISTAT_PRE           0x04
#define ISTAT_CON           0x08
#define ISTAT_ABRT          0x80

#define DSTAT_OPC           0x01
#define DSTAT_WTD           0x02
#define DSTAT_SIR           0x04
#define DSTAT_SSI           0x08
#define DSTAT_ABRT          0x10
#define DSTAT_DFE           0x80

#define MCFG_NCR53C7XX_IRQ_HANDLER(_devcb) \
	devcb = &ncr53c7xx_device::set_irq_handler(*device, DEVCB_##_devcb);

#define MCFG_NCR53C7XX_HOST_WRITE(_devcb) \
	devcb = &ncr53c7xx_device::set_host_write(*device, DEVCB_##_devcb);

#define MCFG_NCR53C7XX_HOST_READ(_devcb) \
	devcb = &ncr53c7xx_device::set_host_read(*device, DEVCB_##_devcb);

class ncr53c7xx_device : public nscsi_device,
							public device_execute_interface
{
public:
	// construction/destruction
	ncr53c7xx_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ncr53c7xx_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_host_write(device_t &device, _Object object) { return downcast<ncr53c7xx_device &>(device).m_host_write.set_callback(object); }
	template<class _Object> static devcb_base &set_host_read(device_t &device, _Object object) { return downcast<ncr53c7xx_device &>(device).m_host_read.set_callback(object); }

	// our API
	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void execute_run() override;

	int m_icount;

private:

	enum
	{
		STATE_MASK = 0x00ff,
		SUB_SHIFT  = 8,
		SUB_MASK   = 0xff00
	};

	enum
	{
		MODE_I,
		MODE_T,
		MODE_D
	};

	enum scsi_state
	{
		IDLE,
		FINISHED,
		ARBITRATE_WAIT_FREE,
		ARBITRATE_CHECK_FREE,
		ARBITRATE_EXAMINE_BUS,
		ARBITRATE_SELECT_DEST,
		ARBITRATE_ASSERT_SEL,
		ARBITRATE_RELEASE_BSY,
		ARBITRATE_DESKEW_WAIT,
		SELECT,
		SELECT_COMPLETE,
		INIT_XFER,
		INIT_XFER_WAIT_REQ,
		INIT_XFER_SEND_BYTE,
		INIT_XFER_RECV_PAD,
		INIT_XFER_RECV_BYTE_ACK,
		INIT_XFER_RECV_BYTE_NACK,
		SEND_WAIT_REQ_0,
		SEND_WAIT_SETTLE,
		RECV_WAIT_SETTLE,
		RECV_WAIT_REQ_0,
		RECV_WAIT_REQ_1
	};

	void update_irqs();
	void set_scsi_state(int state);
	void delay(const attotime &delay);
	void scsi_ctrl_changed() override;
	void send_byte();
	void recv_byte();
	void step(bool timeout);


	enum scripts_state
	{
		SCRIPTS_IDLE,
		SCRIPTS_WAIT_MANUAL_START,
		SCRIPTS_FETCH,
		SCRIPTS_EXECUTE
	};

	void set_scripts_state(scripts_state state);
	void scripts_yield();
	void scripts_decode_bm(void);
	void scripts_decode_io(void);
	void scripts_decode_tc(void);
	void bm_t_move();
	void bm_i_move();
	void bm_i_wmov();
	void io_t_reselect();
	void io_t_disconnect();
	void io_t_waitselect();
	void io_t_set();
	void io_t_clear();
	void io_i_select();
	void io_i_waitdisconnect();
	void io_i_waitreselect();
	void io_i_set();
	void io_i_clear();
	void tc_jump();
	void tc_call();
	void tc_return();
	void tc_int();
	void illegal();
	const char* disassemble_scripts();


	// SCSI registers
	UINT8   m_scntl[2];
	UINT8   m_sdid;
	UINT8   m_sien;
	UINT8   m_scid;
	UINT8   m_sxfer;
	UINT8   m_sodl;
	UINT8   m_socl;
	UINT8   m_sfbr;
	UINT8   m_sidl;
	UINT8   m_sbdl;
	UINT8   m_sbcl;
	UINT8   m_dstat;
	UINT8   m_sstat[3];
	UINT8   m_ctest[8];
	UINT32  m_temp;
	UINT8   m_dfifo;
	UINT8   m_istat;
	UINT32  m_dbc;
	UINT8   m_dcmd;
	UINT32  m_dnad;
	UINT32  m_dsp;
	UINT32  m_dsps;
	UINT8   m_dmode;
	UINT8   m_dien;
	UINT8   m_dwt;
	UINT8   m_dcntl;


	// other state
	int     m_scsi_state;
	bool    m_connected;
	bool    m_finished;
	UINT8   m_last_data;
	UINT32  m_xfr_phase;
	emu_timer *m_tm;

	int     m_scripts_state;
	//int     m_scripts_substate;
	void    (ncr53c7xx_device::*m_scripts_op)();

	// callbacks
	devcb_write_line m_irq_handler;
	devcb_write32 m_host_write;
	devcb_read32 m_host_read;
};

// device type definition
extern const device_type NCR53C7XX;
#endif
