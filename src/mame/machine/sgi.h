// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    sgi.h

    Silicon Graphics MC (Memory Controller) code

*********************************************************************/

#ifndef _SGIMC_H
#define _SGIMC_H

class sgi_mc_device : public device_t
{
public:
	sgi_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~sgi_mc_device() {}

DECLARE_READ32_MEMBER(read);
DECLARE_WRITE32_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	emu_timer *m_tUpdateTimer;
	UINT32 m_nCPUControl0;
	UINT32 m_nCPUControl1;
	UINT32 m_nWatchdog;
	UINT32 m_nSysID;
	UINT32 m_nRPSSDiv;
	UINT32 m_nRefCntPreload;
	UINT32 m_nRefCnt;
	UINT32 m_nGIO64ArbParam;
	UINT32 m_nArbCPUTime;
	UINT32 m_nArbBurstTime;
	UINT32 m_nMemCfg0;
	UINT32 m_nMemCfg1;
	UINT32 m_nCPUMemAccCfg;
	UINT32 m_nGIOMemAccCfg;
	UINT32 m_nCPUErrorAddr;
	UINT32 m_nCPUErrorStatus;
	UINT32 m_nGIOErrorAddr;
	UINT32 m_nGIOErrorStatus;
	UINT32 m_nSysSemaphore;
	UINT32 m_nGIOLock;
	UINT32 m_nEISALock;
	UINT32 m_nGIO64TransMask;
	UINT32 m_nGIO64Subst;
	UINT32 m_nDMAIntrCause;
	UINT32 m_nDMAControl;
	UINT32 m_nDMATLBEntry0Hi;
	UINT32 m_nDMATLBEntry0Lo;
	UINT32 m_nDMATLBEntry1Hi;
	UINT32 m_nDMATLBEntry1Lo;
	UINT32 m_nDMATLBEntry2Hi;
	UINT32 m_nDMATLBEntry2Lo;
	UINT32 m_nDMATLBEntry3Hi;
	UINT32 m_nDMATLBEntry3Lo;
	UINT32 m_nRPSSCounter;
	UINT32 m_nDMAMemAddr;
	UINT32 m_nDMALineCntWidth;
	UINT32 m_nDMALineZoomStride;
	UINT32 m_nDMAGIO64Addr;
	UINT32 m_nDMAMode;
	UINT32 m_nDMAZoomByteCnt;
	UINT32 m_nDMARunning;

	void update();
	TIMER_CALLBACK_MEMBER(update_callback);
	void timer_init();
};

extern const device_type SGI_MC;


#endif /* _SGIMC_H */
