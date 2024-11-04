// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************

  IBM 21S850/1 IEEE 1394 400Mb/s Physical Layer
  Transceiver (PHY)

  Skeleton device

**************************************************************/

#ifndef MAME_MACHINE_IBM21S850_H
#define MAME_MACHINE_IBM21S850_H

#pragma once

class ibm21s85x_base_device : public device_t
{
public:
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	auto reset_cb() { return m_reset_cb.bind(); }

	static constexpr feature_type unemulated_features() { return feature::COMMS; }

protected:
	ibm21s85x_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(reset_tick);

	void power_on_reset();

	enum : uint32_t
	{
		PHYSICAL_ID_OFFS    = 0,
		PHYSICAL_ID_SHIFT   = 2,
		PHYSICAL_ID_MASK    = 0xfc,
		ROOT_OFFS           = 0,
		ROOT_MASK           = 0x02,
		CABLE_PWR_OFFS      = 0,
		CABLE_PWR_MASK      = 0x01,

		ROOT_HOLD_OFFS      = 1,
		ROOT_HOLD_MASK      = 0x80,
		BUS_RESET_OFFS      = 1,
		BUS_RESET_MASK      = 0x40,
		GAP_COUNT_OFFS      = 1,
		GAP_COUNT_MASK      = 0x3f,

		SPEED_OFFS          = 2,
		SPEED_SHIFT         = 6,
		SPEED_MASK          = 0xc0,
		ENHANCED_REGS_OFFS  = 2,
		ENHANCED_REGS_MASK  = 0x20,
		NUM_PORTS_OFFS      = 2,
		NUM_PORTS_MASK      = 0x1f,
		SPEED_100MBIT       = 0,
		SPEED_200MBIT       = 1,
		SPEED_400MBIT       = 2,
		SPEED_RESERVED      = 3,

		ASTAT1_OFFS         = 3,
		ASTAT1_SHIFT        = 6,
		ASTAT1_MASK         = 0xc0,
		BSTAT1_OFFS         = 3,
		BSTAT1_SHIFT        = 4,
		BSTAT1_MASK         = 0x30,
		CHILD1_OFFS         = 3,
		CHILD1_MASK         = 0x08,
		CONNECTION1_OFFS    = 3,
		CONNECTION1_MASK    = 0x04,
		PEER_SPEED1_OFFS    = 3,
		PEER_SPEED1_MASK    = 0x03,

		LPS_OFFS            = 11,
		LPS_MASK            = 0x80,
		PHY_DELAY_OFFS      = 11,
		PHY_DELAY_SHIFT     = 5,
		PHY_DELAY_MASK      = 0x60,
		CONFIG_MGR_CAP_OFFS = 11,
		CONFIG_MGR_CAP_MASK = 0x10,
		POWER_CLASS_OFFS    = 11,
		POWER_CLASS_SHIFT   = 1,
		POWER_CLASS_MASK    = 0x0e,

		CMC_PIN_OFFS        = 12,
		CMC_PIN_MASK        = 0x80,
		CPS_INT_OFFS        = 12,
		CPS_INT_MASK        = 0x40,
		LP_TEST_ERR_OFFS    = 12,
		LP_TEST_ERR_MASK    = 0x20,
		ARB_PHASE_OFFS      = 12,
		ARB_PHASE_SHIFT     = 3,
		ARB_PHASE_MASK      = 0x18,
		ARB_STATE_OFFS      = 12,
		ARB_STATE_MASK      = 0x07,
		PHASE_BUS_RESET     = 0,
		PHASE_TREE_ID       = 1,
		PHASE_SELF_ID       = 2,
		PHASE_NORMAL        = 3,

		LP_TEST_EN_OFFS         = 13,
		LP_TEST_EN_MASK         = 0x40,
		ACK_ACCEL_EN_OFFS       = 13,
		ACK_ACCEL_EN_MASK       = 0x08,
		MULTISP_CONCAT_EN_OFFS  = 13,
		MULTISP_CONCAT_EN_MASK  = 0x02,
		MASK_LPS_OFFS           = 13,
		MASK_LPS_MASK           = 0x01,

		EN_TIMEOUT_OFFS     = 14,
		EN_TIMEOUT_MASK     = 0x80,
		IGNORE_UNPLUG_OFFS  = 14,
		IGNORE_UNPLUG_MASK  = 0x40,
		OVERRIDE_CMC_OFFS   = 14,
		OVERRIDE_CMC_MASK   = 0x20,
		SOFT_CMC_OFFS       = 14,
		SOFT_CMC_MASK       = 0x10,
		DISABLE_P1_OFFS     = 14,
		DISABLE_P1_MASK     = 0x04,

		SOFT_POR_OFFS       = 15,
		SOFT_POR_MASK       = 0x80,
		SEND_PL_DIAG_OFFS   = 15,
		SEND_PL_DIAG_MASK   = 0x20,
		ACK_ACCEL_SYNC_OFFS = 15,
		ACK_ACCEL_SYNC_MASK = 0x10,
		ISBR_OFFS           = 15,
		ISBR_MASK           = 0x08
	};

	uint8_t m_regs[16];

	emu_timer *m_reset_timer;
	devcb_write_line m_reset_cb;
};

class ibm21s850_device : public ibm21s85x_base_device
{
public:
	ibm21s850_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual void device_reset() override ATTR_COLD;

	enum : uint32_t
	{
		ENV_OFFS            = 4,
		ENV_SHIFT           = 6,
		ENV_MASK            = 0xc0,
		REG_COUNT_OFFS      = 4,
		REG_COUNT_MASK      = 0x3f
	};
};

class ibm21s851_device : public ibm21s85x_base_device
{
public:
	ibm21s851_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual void device_reset() override ATTR_COLD;

	enum : uint32_t
	{
		ASTAT2_OFFS         = 4,
		ASTAT2_SHIFT        = 6,
		ASTAT2_MASK         = 0xc0,
		BSTAT2_OFFS         = 4,
		BSTAT2_SHIFT        = 4,
		BSTAT2_MASK         = 0x30,
		CHILD2_OFFS         = 4,
		CHILD2_MASK         = 0x08,
		CONNECTION2_OFFS    = 4,
		CONNECTION2_MASK    = 0x04,
		PEER_SPEED2_OFFS    = 4,
		PEER_SPEED2_MASK    = 0x03,

		ASTAT3_OFFS         = 5,
		ASTAT3_SHIFT        = 6,
		ASTAT3_MASK         = 0xc0,
		BSTAT3_OFFS         = 5,
		BSTAT3_SHIFT        = 4,
		BSTAT3_MASK         = 0x30,
		CHILD3_OFFS         = 5,
		CHILD3_MASK         = 0x08,
		CONNECTION3_OFFS    = 5,
		CONNECTION3_MASK    = 0x04,
		PEER_SPEED3_OFFS    = 5,
		PEER_SPEED3_MASK    = 0x03,

		ENV_OFFS            = 6,
		ENV_SHIFT           = 6,
		ENV_MASK            = 0xc0,
		REG_COUNT_OFFS      = 6,
		REG_COUNT_MASK      = 0x3f
	};
};

//device type definition
DECLARE_DEVICE_TYPE(IBM21S850, ibm21s850_device)
DECLARE_DEVICE_TYPE(IBM21S851, ibm21s851_device)

#endif // MAME_MACHINE_IBM21S850_H
