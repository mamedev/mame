// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    sgi.c

    Silicon Graphics MC (Memory Controller) code

*********************************************************************/

#include "emu.h"
#include "sgi.h"

#define VERBOSE_LEVEL ( 2 )

#if 0
static inline void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%08x: %s", (unsigned) machine.device("maincpu")->safe_pc(), buf );
	}
}
#endif

const device_type SGI_MC = &device_creator<sgi_mc_device>;

sgi_mc_device::sgi_mc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SGI_MC, "SGI Memory Controller", tag, owner, clock, "sgi_mc", __FILE__),
		m_tUpdateTimer(nullptr),
		m_nCPUControl0(0),
		m_nCPUControl1(0),
		m_nWatchdog(0),
		m_nSysID(0),
		m_nRPSSDiv(0),
		m_nRefCntPreload(0),
		m_nRefCnt(0),
		m_nGIO64ArbParam(0),
		m_nArbCPUTime(0),
		m_nArbBurstTime(0),
		m_nMemCfg0(0),
		m_nMemCfg1(0),
		m_nCPUMemAccCfg(0),
		m_nGIOMemAccCfg(0),
		m_nCPUErrorAddr(0),
		m_nCPUErrorStatus(0),
		m_nGIOErrorAddr(0),
		m_nGIOErrorStatus(0),
		m_nSysSemaphore(0),
		m_nGIOLock(0),
		m_nEISALock(0),
		m_nGIO64TransMask(0),
		m_nGIO64Subst(0),
		m_nDMAIntrCause(0),
		m_nDMAControl(0),
		m_nDMATLBEntry0Hi(0),
		m_nDMATLBEntry0Lo(0),
		m_nDMATLBEntry1Hi(0),
		m_nDMATLBEntry1Lo(0),
		m_nDMATLBEntry2Hi(0),
		m_nDMATLBEntry2Lo(0),
		m_nDMATLBEntry3Hi(0),
		m_nDMATLBEntry3Lo(0),
		m_nRPSSCounter(0),
		m_nDMAMemAddr(0),
		m_nDMALineCntWidth(0),
		m_nDMALineZoomStride(0),
		m_nDMAGIO64Addr(0),
		m_nDMAMode(0),
		m_nDMAZoomByteCnt(0),
		m_nDMARunning(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sgi_mc_device::device_start()
{
	// if Indigo2, ID appropriately
	if (!strcmp(machine().system().name, "ip244415"))
	{
		m_nSysID = 0x11; // rev. B MC, EISA bus present
	}

	timer_init();

	save_item(NAME(m_nCPUControl0));
	save_item(NAME(m_nCPUControl1));
	save_item(NAME(m_nWatchdog));
	save_item(NAME(m_nSysID));
	save_item(NAME(m_nRPSSDiv));
	save_item(NAME(m_nRefCntPreload));
	save_item(NAME(m_nRefCnt));
	save_item(NAME(m_nGIO64ArbParam));
	save_item(NAME(m_nArbCPUTime));
	save_item(NAME(m_nArbBurstTime));
	save_item(NAME(m_nMemCfg0));
	save_item(NAME(m_nMemCfg1));
	save_item(NAME(m_nCPUMemAccCfg));
	save_item(NAME(m_nGIOMemAccCfg));
	save_item(NAME(m_nCPUErrorAddr));
	save_item(NAME(m_nCPUErrorStatus));
	save_item(NAME(m_nGIOErrorAddr));
	save_item(NAME(m_nGIOErrorStatus));
	save_item(NAME(m_nSysSemaphore));
	save_item(NAME(m_nGIOLock));
	save_item(NAME(m_nEISALock));
	save_item(NAME(m_nGIO64TransMask));
	save_item(NAME(m_nGIO64Subst));
	save_item(NAME(m_nDMAIntrCause));
	save_item(NAME(m_nDMAControl));
	save_item(NAME(m_nDMATLBEntry0Hi));
	save_item(NAME(m_nDMATLBEntry0Lo));
	save_item(NAME(m_nDMATLBEntry1Hi));
	save_item(NAME(m_nDMATLBEntry1Lo));
	save_item(NAME(m_nDMATLBEntry2Hi));
	save_item(NAME(m_nDMATLBEntry2Lo));
	save_item(NAME(m_nDMATLBEntry3Hi));
	save_item(NAME(m_nDMATLBEntry3Lo));
	save_item(NAME(m_nRPSSCounter));
	save_item(NAME(m_nDMAMemAddr));
	save_item(NAME(m_nDMALineCntWidth));
	save_item(NAME(m_nDMALineZoomStride));
	save_item(NAME(m_nDMAGIO64Addr));
	save_item(NAME(m_nDMAMode));
	save_item(NAME(m_nDMAZoomByteCnt));
	save_item(NAME(m_nDMARunning));
}

READ32_MEMBER( sgi_mc_device::read )
{
	offset <<= 2;
	switch( offset )
	{
	case 0x0000:
	case 0x0004:
		//verboselog( space.machine(), 3, "CPU Control 0 Read: %08x (%08x)\n", m_nCPUControl0, mem_mask );
		return m_nCPUControl0;
	case 0x0008:
	case 0x000c:
		//verboselog( space.machine(), 2, "CPU Control 1 Read: %08x (%08x)\n", m_nCPUControl1, mem_mask );
		return m_nCPUControl1;
	case 0x0010:
	case 0x0014:
		//verboselog( space.machine(), 2, "Watchdog Timer Read: %08x (%08x)\n", m_nWatchdog, mem_mask );
		return m_nWatchdog;
	case 0x0018:
	case 0x001c:
		//verboselog( space.machine(), 2, "System ID Read: %08x (%08x)\n", m_nSysID, mem_mask );
		return m_nSysID;
	case 0x0028:
	case 0x002c:
		//verboselog( space.machine(), 2, "RPSS Divider Read: %08x (%08x)\n", m_nRPSSDiv, mem_mask );
		return m_nRPSSDiv;
	case 0x0030:
	case 0x0034:
		//verboselog( space.machine(), 2, "R4000 EEPROM Read\n" );
		return 0;
	case 0x0040:
	case 0x0044:
		//verboselog( space.machine(), 2, "Refresh Count Preload Read: %08x (%08x)\n", m_nRefCntPreload, mem_mask );
		return m_nRefCntPreload;
	case 0x0048:
	case 0x004c:
		//verboselog( space.machine(), 2, "Refresh Count Read: %08x (%08x)\n", m_nRefCnt, mem_mask );
		return m_nRefCnt;
	case 0x0080:
	case 0x0084:
		//verboselog( space.machine(), 2, "GIO64 Arbitration Param Read: %08x (%08x)\n", m_nGIO64ArbParam, mem_mask );
		return m_nGIO64ArbParam;
	case 0x0088:
	case 0x008c:
		//verboselog( space.machine(), 2, "Arbiter CPU Time Read: %08x (%08x)\n", m_nArbCPUTime, mem_mask );
		return m_nArbCPUTime;
	case 0x0098:
	case 0x009c:
		//verboselog( space.machine(), 2, "Arbiter Long Burst Time Read: %08x (%08x)\n", m_nArbBurstTime, mem_mask );
		return m_nArbBurstTime;
	case 0x00c0:
	case 0x00c4:
		//verboselog( space.machine(), 3, "Memory Configuration Register 0 Read: %08x (%08x)\n", m_nMemCfg0, mem_mask );
		return m_nMemCfg0;
	case 0x00c8:
	case 0x00cc:
		//verboselog( space.machine(), 3, "Memory Configuration Register 1 Read: %08x (%08x)\n", m_nMemCfg1, mem_mask );
		return m_nMemCfg1;
	case 0x00d0:
	case 0x00d4:
		//verboselog( space.machine(), 2, "CPU Memory Access Config Params Read: %08x (%08x)\n", m_nCPUMemAccCfg, mem_mask );
		return m_nCPUMemAccCfg;
	case 0x00d8:
	case 0x00dc:
		//verboselog( space.machine(), 2, "GIO Memory Access Config Params Read: %08x (%08x)\n", m_nGIOMemAccCfg, mem_mask );
		return m_nGIOMemAccCfg;
	case 0x00e0:
	case 0x00e4:
		//verboselog( space.machine(), 2, "CPU Error Address Read: %08x (%08x)\n", m_nCPUErrorAddr, mem_mask );
		return m_nCPUErrorAddr;
	case 0x00e8:
	case 0x00ec:
		//verboselog( space.machine(), 2, "CPU Error Status Read: %08x (%08x)\n", m_nCPUErrorStatus, mem_mask );
		return m_nCPUErrorStatus;
	case 0x00f0:
	case 0x00f4:
		//verboselog( space.machine(), 2, "GIO Error Address Read: %08x (%08x)\n", m_nGIOErrorAddr, mem_mask );
		return m_nGIOErrorAddr;
	case 0x00f8:
	case 0x00fc:
		//verboselog( space.machine(), 2, "GIO Error Status Read: %08x (%08x)\n", m_nGIOErrorStatus, mem_mask );
		return m_nGIOErrorStatus;
	case 0x0100:
	case 0x0104:
		//verboselog( space.machine(), 2, "System Semaphore Read: %08x (%08x)\n", m_nSysSemaphore, mem_mask );
		return m_nSysSemaphore;
	case 0x0108:
	case 0x010c:
		//verboselog( space.machine(), 2, "GIO Lock Read: %08x (%08x)\n", m_nGIOLock, mem_mask );
		return m_nGIOLock;
	case 0x0110:
	case 0x0114:
		//verboselog( space.machine(), 2, "EISA Lock Read: %08x (%08x)\n", m_nEISALock, mem_mask );
		return m_nEISALock;
	case 0x0150:
	case 0x0154:
		//verboselog( space.machine(), 2, "GIO64 Translation Address Mask Read: %08x (%08x)\n", m_nGIO64TransMask, mem_mask );
		return m_nGIO64TransMask;
	case 0x0158:
	case 0x015c:
		//verboselog( space.machine(), 2, "GIO64 Translation Address Substitution Bits Read: %08x (%08x)\n", m_nGIO64Subst, mem_mask );
		return m_nGIO64Subst;
	case 0x0160:
	case 0x0164:
		//verboselog( space.machine(), 2, "DMA Interrupt Cause: %08x (%08x)\n", m_nDMAIntrCause, mem_mask );
		return m_nDMAIntrCause;
	case 0x0168:
	case 0x016c:
		//verboselog( space.machine(), 2, "DMA Control Read: %08x (%08x)\n", m_nDMAControl, mem_mask );
		return m_nDMAControl;
	case 0x0180:
	case 0x0184:
		//verboselog( space.machine(), 2, "DMA TLB Entry 0 High Read: %08x (%08x)\n", m_nDMATLBEntry0Hi, mem_mask );
		return m_nDMATLBEntry0Hi;
	case 0x0188:
	case 0x018c:
		//verboselog( space.machine(), 2, "DMA TLB Entry 0 Low Read: %08x (%08x)\n", m_nDMATLBEntry0Lo, mem_mask );
		return m_nDMATLBEntry0Lo;
	case 0x0190:
	case 0x0194:
		//verboselog( space.machine(), 2, "DMA TLB Entry 1 High Read: %08x (%08x)\n", m_nDMATLBEntry1Hi, mem_mask );
		return m_nDMATLBEntry1Hi;
	case 0x0198:
	case 0x019c:
		//verboselog( space.machine(), 2, "DMA TLB Entry 1 Low Read: %08x (%08x)\n", m_nDMATLBEntry1Lo, mem_mask );
		return m_nDMATLBEntry1Lo;
	case 0x01a0:
	case 0x01a4:
		//verboselog( space.machine(), 2, "DMA TLB Entry 2 High Read: %08x (%08x)\n", m_nDMATLBEntry2Hi, mem_mask );
		return m_nDMATLBEntry2Hi;
	case 0x01a8:
	case 0x01ac:
		//verboselog( space.machine(), 2, "DMA TLB Entry 2 Low Read: %08x (%08x)\n", m_nDMATLBEntry2Lo, mem_mask );
		return m_nDMATLBEntry2Lo;
	case 0x01b0:
	case 0x01b4:
		//verboselog( space.machine(), 2, "DMA TLB Entry 3 High Read: %08x (%08x)\n", m_nDMATLBEntry3Hi, mem_mask );
		return m_nDMATLBEntry3Hi;
	case 0x01b8:
	case 0x01bc:
		//verboselog( space.machine(), 2, "DMA TLB Entry 3 Low Read: %08x (%08x)\n", m_nDMATLBEntry3Lo, mem_mask );
		return m_nDMATLBEntry3Lo;
	case 0x1000:
	case 0x1004:
		//verboselog( space.machine(), 2, "RPSS 100ns Counter Read: %08x (%08x)\n", m_nRPSSCounter, mem_mask );
		return m_nRPSSCounter;
	case 0x2000:
	case 0x2004:
	case 0x2008:
	case 0x200c:
		//verboselog( space.machine(), 0, "DMA Memory Address Read: %08x (%08x)\n", m_nDMAMemAddr, mem_mask );
		return m_nDMAMemAddr;
	case 0x2010:
	case 0x2014:
		//verboselog( space.machine(), 0, "DMA Line Count and Width Read: %08x (%08x)\n", m_nDMALineCntWidth, mem_mask );
		return m_nDMALineCntWidth;
	case 0x2018:
	case 0x201c:
		//verboselog( space.machine(), 0, "DMA Line Zoom and Stride Read: %08x (%08x)\n", m_nDMALineZoomStride, mem_mask );
		return m_nDMALineZoomStride;
	case 0x2020:
	case 0x2024:
	case 0x2028:
	case 0x202c:
		//verboselog( space.machine(), 0, "DMA GIO64 Address Read: %08x (%08x)\n", m_nDMAGIO64Addr, mem_mask );
		return m_nDMAGIO64Addr;
	case 0x2030:
	case 0x2034:
		//verboselog( space.machine(), 0, "DMA Mode Write: %08x (%08x)\n", m_nDMAMode, mem_mask );
		return m_nDMAMode;
	case 0x2038:
	case 0x203c:
		//verboselog( space.machine(), 0, "DMA Zoom Count Read: %08x (%08x)\n", m_nDMAZoomByteCnt, mem_mask );
		return m_nDMAZoomByteCnt;
//  case 0x2040:
//  case 0x2044:
//      //verboselog( space.machine(), 2, "DMA Start Write: %08x (%08x)\n", data, mem_mask );
		// Start DMA
//      m_nDMARunning = 1;
//      break;
	case 0x2048:
	case 0x204c:
		//verboselog( space.machine(), 0, "VDMA Running Read: %08x (%08x)\n", m_nDMARunning, mem_mask );
		if( m_nDMARunning == 1 )
		{
			m_nDMARunning = 0;
			return 0x00000040;
		}
		else
		{
			return 0;
		}
	}
	//verboselog( space.machine(), 0, "Unmapped MC read: 0x%08x (%08x)\n", 0x1fa00000 + offset, mem_mask );
	return 0;
}

WRITE32_MEMBER( sgi_mc_device::write )
{
	offset <<= 2;
	switch( offset )
	{
	case 0x0000:
	case 0x0004:
		//verboselog( space.machine(), 2, "CPU Control 0 Write: %08x (%08x)\n", data, mem_mask );
		m_nCPUControl0 = data;
		break;
	case 0x0008:
	case 0x000c:
		//verboselog( space.machine(), 2, "CPU Control 1 Write: %08x (%08x)\n", data, mem_mask );
		m_nCPUControl1 = data;
		break;
	case 0x0010:
	case 0x0014:
		//verboselog( space.machine(), 2, "Watchdog Timer Clear" );
		m_nWatchdog = 0;
		break;
	case 0x0028:
	case 0x002c:
		//verboselog( space.machine(), 2, "RPSS Divider Write: %08x (%08x)\n", data, mem_mask );
		m_nRPSSDiv = data;
		break;
	case 0x0030:
	case 0x0034:
		//verboselog( space.machine(), 2, "R4000 EEPROM Write\n" );
		break;
	case 0x0040:
	case 0x0044:
		//verboselog( space.machine(), 2, "Refresh Count Preload Write: %08x (%08x)\n", data, mem_mask );
		m_nRefCntPreload = data;
		break;
	case 0x0080:
	case 0x0084:
		//verboselog( space.machine(), 2, "GIO64 Arbitration Param Write: %08x (%08x)\n", data, mem_mask );
		m_nGIO64ArbParam = data;
		break;
	case 0x0088:
	case 0x008c:
		//verboselog( space.machine(), 3, "Arbiter CPU Time Write: %08x (%08x)\n", data, mem_mask );
		m_nArbCPUTime = data;
		break;
	case 0x0098:
	case 0x009c:
		//verboselog( space.machine(), 3, "Arbiter Long Burst Time Write: %08x (%08x)\n", data, mem_mask );
		m_nArbBurstTime = data;
		break;
	case 0x00c0:
	case 0x00c4:
		//verboselog( space.machine(), 3, "Memory Configuration Register 0 Write: %08x (%08x)\n", data, mem_mask );
		m_nMemCfg0 = data;
		break;
	case 0x00c8:
	case 0x00cc:
		//verboselog( space.machine(), 3, "Memory Configuration Register 1 Write: %08x (%08x)\n", data, mem_mask );
		m_nMemCfg1 = data;
		break;
	case 0x00d0:
	case 0x00d4:
		//verboselog( space.machine(), 2, "CPU Memory Access Config Params Write: %08x (%08x)\n", data, mem_mask );
		m_nCPUMemAccCfg = data;
		break;
	case 0x00d8:
	case 0x00dc:
		//verboselog( space.machine(), 2, "GIO Memory Access Config Params Write: %08x (%08x)\n", data, mem_mask );
		m_nGIOMemAccCfg = data;
		break;
	case 0x00e8:
	case 0x00ec:
		//verboselog( space.machine(), 2, "CPU Error Status Clear\n" );
		m_nCPUErrorStatus = 0;
		break;
	case 0x00f8:
	case 0x00fc:
		//verboselog( space.machine(), 2, "GIO Error Status Clear\n" );
		m_nGIOErrorStatus = 0;
		break;
	case 0x0100:
	case 0x0104:
		//verboselog( space.machine(), 2, "System Semaphore Write: %08x (%08x)\n", data, mem_mask );
		m_nSysSemaphore = data;
		break;
	case 0x0108:
	case 0x010c:
		//verboselog( space.machine(), 2, "GIO Lock Write: %08x (%08x)\n", data, mem_mask );
		m_nGIOLock = data;
		break;
	case 0x0110:
	case 0x0114:
		//verboselog( space.machine(), 2, "EISA Lock Write: %08x (%08x)\n", data, mem_mask );
		m_nEISALock = data;
		break;
	case 0x0150:
	case 0x0154:
		//verboselog( space.machine(), 2, "GIO64 Translation Address Mask Write: %08x (%08x)\n", data, mem_mask );
		m_nGIO64TransMask = data;
		break;
	case 0x0158:
	case 0x015c:
		//verboselog( space.machine(), 2, "GIO64 Translation Address Substitution Bits Write: %08x (%08x)\n", data, mem_mask );
		m_nGIO64Subst = data;
		break;
	case 0x0160:
	case 0x0164:
		//verboselog( space.machine(), 0, "DMA Interrupt Cause Write: %08x (%08x)\n", data, mem_mask );
		m_nDMAIntrCause = data;
		break;
	case 0x0168:
	case 0x016c:
		//verboselog( space.machine(), 0, "DMA Control Write: %08x (%08x)\n", data, mem_mask );
		m_nDMAControl = data;
		break;
	case 0x0180:
	case 0x0184:
		//verboselog( space.machine(), 0, "DMA TLB Entry 0 High Write: %08x (%08x)\n", data, mem_mask );
		m_nDMATLBEntry0Hi = data;
		break;
	case 0x0188:
	case 0x018c:
		//verboselog( space.machine(), 0, "DMA TLB Entry 0 Low Write: %08x (%08x)\n", data, mem_mask );
		m_nDMATLBEntry0Lo = data;
		break;
	case 0x0190:
	case 0x0194:
		//verboselog( space.machine(), 0, "DMA TLB Entry 1 High Write: %08x (%08x)\n", data, mem_mask );
		m_nDMATLBEntry1Hi = data;
		break;
	case 0x0198:
	case 0x019c:
		//verboselog( space.machine(), 0, "DMA TLB Entry 1 Low Write: %08x (%08x)\n", data, mem_mask );
		m_nDMATLBEntry1Lo = data;
		break;
	case 0x01a0:
	case 0x01a4:
		//verboselog( space.machine(), 0, "DMA TLB Entry 2 High Write: %08x (%08x)\n", data, mem_mask );
		m_nDMATLBEntry2Hi = data;
		break;
	case 0x01a8:
	case 0x01ac:
		//verboselog( space.machine(), 0, "DMA TLB Entry 2 Low Write: %08x (%08x)\n", data, mem_mask );
		m_nDMATLBEntry2Lo = data;
		break;
	case 0x01b0:
	case 0x01b4:
		//verboselog( space.machine(), 0, "DMA TLB Entry 3 High Write: %08x (%08x)\n", data, mem_mask );
		m_nDMATLBEntry3Hi = data;
		break;
	case 0x01b8:
	case 0x01bc:
		//verboselog( space.machine(), 0, "DMA TLB Entry 3 Low Write: %08x (%08x)\n", data, mem_mask );
		m_nDMATLBEntry3Lo = data;
		break;
	case 0x2000:
	case 0x2004:
		//verboselog( space.machine(), 0, "DMA Memory Address Write: %08x (%08x)\n", data, mem_mask );
		m_nDMAMemAddr = data;
		break;
	case 0x2008:
	case 0x200c:
		//verboselog( space.machine(), 0, "DMA Memory Address + Default Params Write: %08x (%08x)\n", data, mem_mask );
		m_nDMAMemAddr = data;
		break;
	case 0x2010:
	case 0x2014:
		//verboselog( space.machine(), 0, "DMA Line Count and Width Write: %08x (%08x)\n", data, mem_mask );
		m_nDMALineCntWidth = data;
		break;
	case 0x2018:
	case 0x201c:
		//verboselog( space.machine(), 0, "DMA Line Zoom and Stride Write: %08x (%08x)\n", data, mem_mask );
		m_nDMALineZoomStride = data;
		break;
	case 0x2020:
	case 0x2024:
		//verboselog( space.machine(), 0, "DMA GIO64 Address Write: %08x (%08x)\n", data, mem_mask );
		m_nDMAGIO64Addr = data;
		break;
	case 0x2028:
	case 0x202c:
		//verboselog( space.machine(), 0, "DMA GIO64 Address Write + Start DMA: %08x (%08x)\n", data, mem_mask );
		m_nDMAGIO64Addr = data;
		// Start DMA
		m_nDMARunning = 1;
		break;
	case 0x2030:
	case 0x2034:
		//verboselog( space.machine(), 0, "DMA Mode Write: %08x (%08x)\n", data, mem_mask );
		m_nDMAMode = data;
		break;
	case 0x2038:
	case 0x203c:
		//verboselog( space.machine(), 0, "DMA Zoom Count + Byte Count Write: %08x (%08x)\n", data, mem_mask );
		m_nDMAZoomByteCnt = data;
		break;
	case 0x2040:
	case 0x2044:
		//verboselog( space.machine(), 0, "DMA Start Write: %08x (%08x)\n", data, mem_mask );
		// Start DMA
		m_nDMARunning = 1;
		break;
	case 0x2070:
	case 0x2074:
		//verboselog( space.machine(), 0, "DMA GIO64 Address Write + Default Params Write + Start DMA: %08x (%08x)\n", data, mem_mask );
		m_nDMAGIO64Addr = data;
		// Start DMA
		m_nDMARunning = 1;
		break;
	default:
		//verboselog( space.machine(), 0, "Unmapped MC write: 0x%08x: %08x (%08x)\n", 0x1fa00000 + offset, data, mem_mask );
		break;
	}
}

void sgi_mc_device::update()
{
	m_nRPSSCounter += 1000;
}

TIMER_CALLBACK_MEMBER(sgi_mc_device::update_callback)
{
	update();
}

void sgi_mc_device::timer_init()
{
	m_tUpdateTimer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sgi_mc_device::update_callback), this));
	m_tUpdateTimer->adjust(attotime::from_hz(10000), 0, attotime::from_hz(10000));
}
