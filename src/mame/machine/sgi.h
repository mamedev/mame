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
	sgi_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~sgi_mc_device() {}

uint32_t read(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
void write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	emu_timer *m_tUpdateTimer;
	uint32_t m_nCPUControl0;
	uint32_t m_nCPUControl1;
	uint32_t m_nWatchdog;
	uint32_t m_nSysID;
	uint32_t m_nRPSSDiv;
	uint32_t m_nRefCntPreload;
	uint32_t m_nRefCnt;
	uint32_t m_nGIO64ArbParam;
	uint32_t m_nArbCPUTime;
	uint32_t m_nArbBurstTime;
	uint32_t m_nMemCfg0;
	uint32_t m_nMemCfg1;
	uint32_t m_nCPUMemAccCfg;
	uint32_t m_nGIOMemAccCfg;
	uint32_t m_nCPUErrorAddr;
	uint32_t m_nCPUErrorStatus;
	uint32_t m_nGIOErrorAddr;
	uint32_t m_nGIOErrorStatus;
	uint32_t m_nSysSemaphore;
	uint32_t m_nGIOLock;
	uint32_t m_nEISALock;
	uint32_t m_nGIO64TransMask;
	uint32_t m_nGIO64Subst;
	uint32_t m_nDMAIntrCause;
	uint32_t m_nDMAControl;
	uint32_t m_nDMATLBEntry0Hi;
	uint32_t m_nDMATLBEntry0Lo;
	uint32_t m_nDMATLBEntry1Hi;
	uint32_t m_nDMATLBEntry1Lo;
	uint32_t m_nDMATLBEntry2Hi;
	uint32_t m_nDMATLBEntry2Lo;
	uint32_t m_nDMATLBEntry3Hi;
	uint32_t m_nDMATLBEntry3Lo;
	uint32_t m_nRPSSCounter;
	uint32_t m_nDMAMemAddr;
	uint32_t m_nDMALineCntWidth;
	uint32_t m_nDMALineZoomStride;
	uint32_t m_nDMAGIO64Addr;
	uint32_t m_nDMAMode;
	uint32_t m_nDMAZoomByteCnt;
	uint32_t m_nDMARunning;

	void update();
	void update_callback(void *ptr, int32_t param);
	void timer_init();
};

extern const device_type SGI_MC;


#endif /* _SGIMC_H */
