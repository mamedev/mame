// license:BSD-3-Clause
// copyright-holders:A. Lenard
/***************************************************************************

    Zilog Z8010 Memory Management Unit

***************************************************************************
                            _____   _____
                   _CS   1 |*    \_/     | 48  N/_S
               DMASYNC   2 |             | 47  R/_W
                 _SEGT   3 |             | 46  _AS
                  _SUP   4 |             | 45  _DS
                _RESET   5 |             | 44  ST0
                   A23   6 |             | 43  ST1
                   A22   7 |             | 42  ST2
                   A21   8 |             | 41  ST3
                   A20   9 |             | 40  AD8
                   A19  10 |             | 39  AD9
                   VCC  11 |             | 38  AD10
                   A18  12 |    Z8010    | 37  AD11
                   A17  13 |             | 36  CLK
                   A16  14 |             | 35  GND
                   A15  15 |             | 34  AD12
                   A14  16 |             | 33  AD13
                   A13  17 |             | 32  AD14
                   A12  18 |             | 31  AD15
                   A11  19 |             | 30  SN0
                   A10  20 |             | 29  SN1
                    A9  21 |             | 28  SN2
                    A8  22 |             | 27  SN3
              RESERVED  23 |             | 26  SN4
                   SN6  24 |_____________| 25  SN5

***************************************************************************/

#ifndef MAME_MACHINE_Z8010_H
#define MAME_MACHINE_Z8010_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z8010_device

class z8010_device : public device_t
{
protected:
	enum : uint8_t
	{
		MODE_ID_MASK	= 0x07,	/* mmu id */
		MODE_NMS		= 0x08,	/* normal mode select */
		MODE_MST		= 0x10,	/* multiple segment tables */
		MODE_URS		= 0x20,	/* upper range select */
		MODE_TRNS		= 0x40,	/* translate */
		MODE_MSEN		= 0x80	/* master enable */
	};

	enum : uint8_t
	{
		DSC_SDR_BAH		= 0x00,	/* base address (high byte) */
		DSC_SDR_BAL		= 0x01,	/* base address (low byte) */
		DSC_SDR_LIMIT	= 0x02,	/* limit */
		DSC_SDR_ATTR	= 0x03	/* attribute */
	};

	enum : uint8_t
	{
		SDR_ATTR_RD		= 0x01,	/* read-only */
		SDR_ATTR_SYS	= 0x02,	/* system-only */
		SDR_ATTR_CPUI	= 0x04,	/* cpu-inhibit */
		SDR_ATTR_EXC	= 0x08,	/* execute-only */
		SDR_ATTR_DMAI	= 0x10,	/* dma-inhibit */
		SDR_ATTR_DIRW	= 0x20,	/* direction and warning */
		SDR_ATTR_CHG	= 0x40,	/* changed */
		SDR_ATTR_REF	= 0x80	/* referenced */
	};

	enum : uint8_t
	{
		VTYPE_RDV		= 0x01,	/* read-only violation */
		VTYPE_SYSV		= 0x02,	/* system violation */
		VTYPE_CPUIV		= 0x04,	/* cpu-inhibit violation */
		VTYPE_EXCV		= 0x08,	/* execute-only violation */
		VTYPE_SLV		= 0x10,	/* segment length violation */
		VTYPE_PWW		= 0x20,	/* primary write warning */
		VTYPE_SWW		= 0x40,	/* secondary write warning */
		VTYPE_FATL		= 0x80,	/* fatal condition */
	};

	enum : uint8_t
	{
		BCS_CPU_MASK	= 0x0f,	/* cpu status code */
		BCS_R_W			= 0x10,	/* read/write mode */
		BCS_N_S			= 0x20	/* normal/system mode */
	};

public:
	z8010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_segt_cb() { return m_out_segt.bind(); }
	uint16_t segtack_r();

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	bool translate(offs_t &offset, bool write, bool sys, bool dma, int st);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void INC_SAR();
	void INC_DSC(const uint8_t max_dsc);
	void INC_DSC_SAR(const uint8_t max_dsc);

	devcb_write_line m_out_segt;

	union sdr_entry
	{
		struct
		{
			uint8_t bah;	/* base address (high byte) */
			uint8_t bal;	/* base address (low byte) */
			uint8_t limit;	/* limit */
			uint8_t attr;	/* attribute */
		};
		uint8_t b[4];
	};

	std::unique_ptr<uint8_t[]> m_sdr;	/* segment descriptor registers */

	/* control registers */
	uint8_t m_mode;		/* mode */
	uint8_t m_sar;		/* segment address */
	uint8_t m_dsc;		/* descriptor selection counter */

	/* status registers */
	uint8_t m_vtype;	/* violation type */
	uint8_t m_vseg;		/* violation segment number */
	uint8_t m_vhoffs;	/* violation offset (high byte) */
	uint8_t m_bcs;		/* bus cycle status */
	uint8_t m_iseg;		/* instruction segment number */
	uint8_t m_ihoffs;	/* instruction offset (high byte) */
};

// device type definition
DECLARE_DEVICE_TYPE(Z8010, z8010_device)

#endif // MAME_MACHINE_Z8010_H
