// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    GROM emulation
    See grom.c for documentation,

    Michael Zapf

    February 2012: Rewritten as class

***************************************************************************/
#ifndef __TMC0430__
#define __TMC0430__

extern const device_type TMC0430;

#ifndef READ8Z_MEMBER
#define DECLARE_READ8Z_MEMBER(name)     void name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 *value, ATTR_UNUSED UINT8 mem_mask = 0xff)
#define READ8Z_MEMBER(name)             void name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 *value, ATTR_UNUSED UINT8 mem_mask)
#endif

enum
{
	GROM_M_LINE = 1,
	GROM_MO_LINE = 2
};

class tmc0430_device : public device_t
{
public:
	tmc0430_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_ready_wr_callback(device_t &device, _Object object) { return downcast<tmc0430_device &>(device).m_gromready.set_callback(object); }

	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_WRITE_LINE_MEMBER(m_line);
	DECLARE_WRITE_LINE_MEMBER(mo_line);
	DECLARE_WRITE_LINE_MEMBER(gsq_line);

	DECLARE_WRITE_LINE_MEMBER(gclock_in);

	DECLARE_WRITE8_MEMBER( set_lines );

	static void set_region_and_ident(device_t &device, const char *regionname, int offset, int ident)
	{
		downcast<tmc0430_device &>(device).m_regionname = regionname;
		downcast<tmc0430_device &>(device).m_offset = offset;
		downcast<tmc0430_device &>(device).m_ident = ident<<13;
	}

	int debug_get_address();

protected:
	void device_start(void) override;
	void device_reset(void) override;

private:
	// Ready callback. This line is usually connected to the READY pin of the CPU.
	devcb_write_line   m_gromready;

	// Clock line level
	line_state m_current_clock_level;

	// Currently active GROM ident
	int m_current_ident;

	// Phase of the state machine
	int m_phase;

	// Address or data mode?
	bool m_address_mode;

	// Reading or writing?
	bool m_read_mode;

	// Selected?
	bool    m_selected;

	// Toggle for address loading
	bool        m_address_lowbyte;

	// Region name
	const char* m_regionname;

	// Offset in the region. We cannot rely on the ident because the GROMs
	// in the cartridges begin with ident 3, but start at the beginning of their region.
	int         m_offset;

	// Identification of this GROM (0-7 <<13)
	int         m_ident;

	// The address pointer is always expected to be in the range 0x0000 - 0xffff, even
	// when this GROM is not addressed.
	UINT16 m_address;

	/* GROM data buffer. */
	UINT8 m_buffer;

	/* Pointer to the memory region contained in this GROM. */
	UINT8 *m_memptr;
};

#define MCFG_GROM_ADD(_tag, _ident, _region, _offset, _ready)    \
	MCFG_DEVICE_ADD(_tag, TMC0430, 0)  \
	tmc0430_device::set_region_and_ident(*device, _region, _offset, _ident); \
	tmc0430_device::set_ready_wr_callback(*device, DEVCB_##_ready);

#endif
