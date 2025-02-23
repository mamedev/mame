// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Micronics Technology's WIN89


****************************************************************************/

#include "emu.h"

#include "bus/heathzenith/h19/tlb.h"
#include "machine/i8255.h"

#include "mt_win89.h"

//
// Logging defines
//
#define LOG_REG       (1U << 1)
#define LOG_FUNC      (1U << 2)
#define LOG_MEM_READ  (1U << 3)

//#define VERBOSE (0xff)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)
#define LOGMEMREAD(...)    LOGMASKED(LOG_MEM_READ, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


namespace {

class mt_win89_parallel_port : public device_t, public device_h89bus_left_card_interface
{
public:
	mt_win89_parallel_port(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual u8 read(u8 offset) override;
	virtual void write(u8 offset, u8 data) override;
	virtual u8 mem_read(u8 &pri_select_lines, u8 &sec_select_lines, u16 offset) override;

protected:
	mt_win89_parallel_port(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual u8 read_hd(u16 offset);
	virtual void write_hd(u16 offset, u8 data);
	virtual u8 read_par(u16 offset);
	virtual void write_par(u16 offset, u8 data);

	inline bool card_mem_selected(u8 sec_select_lines);

	void set_rom(u8 data);

	static constexpr u16 SELECT_ADDR_MASK = 0xf8;
	static constexpr u16 SELECT_PAR_ADDR  = 0x10; // Parallel port
	static constexpr u16 SELECT_HD_ADDR   = 0x20; // Hard drive ports

private:
	required_region_ptr<u8> m_sys_rom;
	required_device<i8255_device> m_par;

	bool use_original_rom;
};


/**
 * The Micronics Technology's WIN89
 */
mt_win89_parallel_port::mt_win89_parallel_port(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, H89BUS_MT_WIN89_PARALLEL, tag, owner, clock),
	device_h89bus_left_card_interface(mconfig, *this),
	m_sys_rom(*this, "sys_rom"),
	m_par(*this, "parallel_port")
{
}

inline bool mt_win89_parallel_port::card_mem_selected(u8 sec_select_lines)
{
	return bool(sec_select_lines & h89bus_device::H89_MEM_SEC_SYS_ROM);
}

void mt_win89_parallel_port::set_rom(u8 data)
{
	LOGREG("%s: setting rom_active: %d\n", FUNCNAME, data);

	use_original_rom = bool(BIT(data, 0));
}

u8 mt_win89_parallel_port::read(u8 offset)
{
	u8 val = 0;

	if ((offset & SELECT_ADDR_MASK) == SELECT_HD_ADDR)
	{
		val = read_hd(offset & 0x07);
	}
	else if ((offset & SELECT_ADDR_MASK) == SELECT_PAR_ADDR)
	{
		val = read_par(offset & 0x07);
	}

	return val;
}

void mt_win89_parallel_port::write(u8 offset, u8 data)
{
	if ((offset & SELECT_ADDR_MASK) == SELECT_HD_ADDR)
	{
		write_hd(offset & 0x07, data);
	}
	else if ((offset & SELECT_ADDR_MASK) == SELECT_PAR_ADDR)
	{
		write_par(offset & 0x07, data);
	}
}

u8 mt_win89_parallel_port::read_hd(u16 offset)
{
	u8 val = 0;

	// TODO implement HD interface
	// 0x20 - HD data port
	// 0x21 - HD status/command
	// 0x22 - HD configuration port
	// 0x24 - HD controller reset
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
			break;
		case 3:
		case 4:
			// unused
			break;
		case 5:
			val = use_original_rom ? 1 : 0;
			break;
		case 6:
		case 7:
			// unused
			break;
	}

	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, offset, val);

	return val;
}

void mt_win89_parallel_port::write_hd(u16 offset, u8 data)
{
	// TODO implement HD interface
	// 0x20 - HD data port
	// 0x21 - HD status/command
	// 0x22 - HD configuration port
	// 0x24 - HD controller reset
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
			break;
		case 3:
			// unused
			break;
		case 4:
			break;
		case 5:
			set_rom(data);
			break;
		case 6:
			break;
		case 7:
			break;
	}
}

u8 mt_win89_parallel_port::read_par(u16 offset)
{
	u8 val = 0;

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return m_par->read(offset);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			// unused
			break;
	}

	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, offset, val);

	return val;
}

void mt_win89_parallel_port::write_par(u16 offset, u8 data)
{
	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, offset, data);

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			m_par->write(offset, data);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			// unused
			break;
	}
}

u8 mt_win89_parallel_port::mem_read(u8 &pri_select_lines, u8 &sec_select_lines, u16 offset)
{
	if (!card_mem_selected(sec_select_lines) || use_original_rom)
	{
		return 0;
	}

	// clear sys_rom so that h89bus does not try to get it from CPU board.
	sec_select_lines &= ~h89bus_device::H89_MEM_SEC_SYS_ROM;

	u8 val = m_sys_rom[offset & 0xfff];

	LOGMEMREAD("%s: offset: 0x%04x - val: 0x%02x\n", FUNCNAME, offset, val);

	return val;
}

void mt_win89_parallel_port::device_start()
{
	save_item(NAME(use_original_rom));

	use_original_rom = false;
}

void mt_win89_parallel_port::device_add_mconfig(machine_config &config)
{
	I8255(config, m_par, m_clock);
}

ROM_START(mtwin89)

	ROM_REGION( 0x1000, "sys_rom", 0 )
	ROM_LOAD( "2732_win89_2_3.bin", 0x0000, 0x1000, CRC(c54c0b3d) SHA1(f354d494794b0281b9cc074ce46362d3cd065350))

ROM_END


const tiny_rom_entry *mt_win89_parallel_port::device_rom_region() const
{
	return ROM_NAME(mtwin89);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_MT_WIN89_PARALLEL, device_h89bus_left_card_interface, mt_win89_parallel_port, "h89_mt_win89", "Micronics Technology Win89 Parallel Board");
