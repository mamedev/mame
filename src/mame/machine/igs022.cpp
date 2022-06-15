// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/************************************************************************************************************

 IGS022 is an encrypted DMA device, most likely an MCU of some sort.
 It can safely be swapped between games, so doesn't appear to have any kind of game specific programming.

************************************************************************************************************/

#include "emu.h"
#include "igs022.h"
#include <sstream>

#define LOG_DMA      (1U << 1)
#define LOG_STACK    (1U << 2)
#define LOG_CMD_6D   (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_DMA | LOG_STACK | LOG_CMD_6D)
#define VERBOSE (0)
#include "logmacro.h"

igs022_device::igs022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS022, tag, owner, clock)
	, m_sharedprotram(*this, "sharedprotram")
	, m_rom(*this, DEVICE_SELF)
{
}

void igs022_device::device_start()
{
	save_item(NAME(m_regs));
	save_item(NAME(m_stack));
	save_item(NAME(m_stack_ptr));
}

void igs022_device::device_reset()
{
	if (!m_sharedprotram)
		fatalerror("%s: IGS022 sharedprotram was not set!\n", machine().describe_context());

	// the internal MCU boot code automatically does this DMA
	// and puts the version # of the data rom in ram

	// reset regs and stack
	std::fill(std::begin(m_regs),  std::end(m_regs),  0);
	std::fill(std::begin(m_stack), std::end(m_stack), 0);
	m_stack_ptr = 0;

	// fill ram with 0xa55a pattern
	for (int i = 0; i < 0x4000 / 2; i++)
		m_sharedprotram[i] = 0xa55a;

	// the initial auto-DMA
	const u16 * const PROTROM = (u16 *)m_rom->base();

	u16 src        = PROTROM[0x100 / 2];
	const u32 dst  = PROTROM[0x102 / 2];
	const u16 size = PROTROM[0x104 / 2];
	u16 mode       = PROTROM[0x106 / 2];

	mode = (mode >> 8) | (mode << 8);

	src >>= 1;

	do_dma(src, dst, size, mode);

	// there is also a version ID? (or is it some kind of checksum) that is stored in the data rom, and gets copied..
	// Dragon World 3 checks it
	// Setting 0x3002a0 to #3 causes Dragon World 3 to skip this check
	m_sharedprotram[0x2a2 / 2] = PROTROM[0x114 / 2];
}

// From IGS022 ROM to shared protection RAM
void igs022_device::do_dma(u16 src, u16 dst, u16 size, u16 mode)
{
	LOGMASKED(LOG_DMA, "%s: IGS022 DMA src %04x, dst %04x, size %04x, mode %04x\n", machine().describe_context(), src, dst, size, mode);

	/*
	P_SRC  = 0x300290 (offset from prot rom base)
	P_DST  = 0x300292 (words from 0x300000)
	P_SIZE = 0x300294 (words)
	P_MODE = 0x300296

	Mode 0 plain copy
	Mode 1,2,3 rom table based ops
	Mode 4 fixed data ('IGS ') based ops
	Mode 5 swap bytes
	Mode 6 swap nibbles
	*/

	const u16 param = mode >> 8;

	// the initial auto-DMA on killbld/slqz2/lhzb2 has 0x10 set, drgw3 has 0x18 set, not sure how they affect the operation.
	if (mode & 0x00f8)
		logerror("%s: IGS022 unknown DMA mode bits %04x set\n", machine().describe_context(), mode & 0x00f8);

	mode &= 0x7; // what are the other bits?

	const u16 * const PROTROM = (u16 *)m_rom->base();

	switch (mode)
	{
	case 0: case 1: case 2: case 3: case 4:
		/*
		    modes 1-3 modify the data being transferred using a 0x100 byte table stored at the start of the protection rom.

		    The param used with the mode gives a start offset into the table.

		    Odd offsets cause an overflow.
		*/
		for (int x = 0; x < size; x++)
		{
			u16 dat = PROTROM[src + x];

			const u8 extraoffset        =   param & 0xff;
			const u8 * const dectable   =   (u8 *)m_rom->base(); // the basic decryption table is at the start of the mcu data rom!
			const u8 taboff             =   ((x * 2) + extraoffset) & 0xff; // must allow for overflow in instances of odd offsets

			u16 extraxor                =   ((dectable[taboff + 1]) << 8) | (dectable[taboff + 0] << 0);

			switch (mode)
			{
//          case 0: plain copy
			case 1: dat -= extraxor;    break;
			case 2: dat += extraxor;    break;
			case 3: dat ^= extraxor;    break;
			case 4:
				extraxor = 0;

				if ((x & 0x003) == 0x000) extraxor |= 0x0049; // 'I'
				if ((x & 0x003) == 0x001) extraxor |= 0x0047; // 'G'
				if ((x & 0x003) == 0x002) extraxor |= 0x0053; // 'S'
				if ((x & 0x003) == 0x003) extraxor |= 0x0020; // ' '

				if ((x & 0x300) == 0x000) extraxor |= 0x4900; // 'I'
				if ((x & 0x300) == 0x100) extraxor |= 0x4700; // 'G'
				if ((x & 0x300) == 0x200) extraxor |= 0x5300; // 'S'
				if ((x & 0x300) == 0x300) extraxor |= 0x2000; // ' '

				LOGMASKED(LOG_DMA, "%s: IGS022 DMA mode 4 -> %06x | %04x (%04x)\n", machine().describe_context(), (dst + x) * 2, dat, (u16)(dat - extraxor));

				dat -= extraxor;
				break;
			}

			m_sharedprotram[dst + x] = dat;
		}
		break;

	case 5: // byteswapped copy
		for (int x = 0; x < size; x++)
		{
			u16 dat = PROTROM[src + x];

			dat = ((dat &0x00ff) << 8) | ((dat &0xff00) >> 8);

			m_sharedprotram[dst + x] = dat;
		}
		break;

	case 6: // nibble swapped copy
		for (int x = 0; x < size; x++)
		{
			u16 dat = PROTROM[src + x];

			dat = ((dat & 0xf0f0) >> 4) | ((dat & 0x0f0f) << 4);

			m_sharedprotram[dst + x] = dat;
		}
		break;

	case 7:
		logerror("%s: IGS022 DMA unhandled copy mode %04x!\n", machine().describe_context(), mode);
		// not used by killbld
		// weird mode, the params get left in memory? - maybe it's a NOP?
		break;

	default:
		logerror("%s: IGS022 DMA unhandled copy mode!: %d, src: %04x, dst: %04x, size: %04x, param: %02x\n", machine().describe_context(), mode, src, dst, size, param);
		// not used by killbld
		// invalid?
	}
}

void igs022_device::push_stack(u32 data)
{
	if (m_stack_ptr < STACK_SIZE - 1)
		++m_stack_ptr;

	m_stack[m_stack_ptr] = data;
}

u32 igs022_device::pop_stack()
{
	const u32 data = m_stack[m_stack_ptr];

	if (m_stack_ptr > 0)
		--m_stack_ptr;

	return data;
}

std::string igs022_device::stack_as_string() const
{
	std::ostringstream stream;
	stream << "stack:";

	for (int i = 0; i <= m_stack_ptr; ++i)
		util::stream_format(stream, " %08x", m_stack[i]);

	return std::move(stream).str();
}

u32 igs022_device::read_reg(u16 offset)
{
	if (offset < NUM_REGS)
	{
		return m_regs[offset];
	}
	else if (offset == 0x400)
	{
		return pop_stack();
	}
	else
	{
		return 0; // Invalid!
	}
}

void igs022_device::write_reg(u16 offset, u32 data)
{
	if (offset < NUM_REGS)
	{
		m_regs[offset] = data;
	}
	else if (offset == 0x300)
	{
		push_stack(data);
	}
	else
	{
		// Invalid!
	}
}

// What does this do? write the completion byte for now...
void igs022_device::handle_incomplete_command(u16 cmd, u16 res)
{
	logerror("%s: IGS022 command %04x: INCOMPLETE (NOP)\n", machine().describe_context(), cmd);
	m_sharedprotram[0x202 / 2] = res;
}

void igs022_device::handle_command()
{
	const u16 cmd = m_sharedprotram[0x200 / 2];

	switch (cmd)
	{
		case 0x12: // Push
		{
			const u32 data = (m_sharedprotram[0x288 / 2] << 16) + m_sharedprotram[0x28a / 2];

			push_stack(data);

			LOGMASKED(LOG_STACK, "%s: IGS022 command %04x: PUSH {288, 28a} (%08x) %s\n", machine().describe_context(), cmd, data, stack_as_string());

			m_sharedprotram[0x202 / 2] = 0x23; // this mode complete?
			break;
		}

		case 0x2d: handle_incomplete_command(cmd, 0x3c);    break; // killbld

//      case 0x42: break; // killbld

		case 0x45: // Pop
		{
			const u32 data = pop_stack();

			m_sharedprotram[0x28c / 2] = (data >> 16) & 0xffff;
			m_sharedprotram[0x28e / 2] = data & 0xffff;

			LOGMASKED(LOG_STACK, "%s: IGS022 command %04x: POP {28c, 28e} (%08x) %s\n", machine().describe_context(), cmd, data, stack_as_string());

			m_sharedprotram[0x202 / 2] = 0x56; // this mode complete?
			break;
		}

//      case 0x47: // NOP? slqz2/lhzb2
//          break;

		case 0x4f: // DMA from protection ROM (memcpy with encryption / scrambling)
		{
			LOGMASKED(LOG_DMA, "%s: IGS022 command %04x: DMA\n", machine().describe_context(), cmd);

			const u16 src  = m_sharedprotram[0x290 / 2] >> 1; // External mcu data is 8 bit and addressed as such
			const u32 dst  = m_sharedprotram[0x292 / 2];
			const u16 size = m_sharedprotram[0x294 / 2];
			const u16 mode = m_sharedprotram[0x296 / 2];

			do_dma(src, dst, size, mode);

			m_sharedprotram[0x202 / 2] = 0x5e; // this mode complete?
			break;
		}

		case 0x5a: handle_incomplete_command(cmd, 0x4b);    break; // killbld, uses {284} as input

		case 0x6d: // Set/Get values to/from ASIC RAM, arithmetic operations on them
			handle_command_6d();
			break;

		default:
			logerror("%s: IGS022 command %04x: UNKNOWN!\n", machine().describe_context(), cmd);
	}
}

// Set/Get values to/from ASIC RAM, arithmetic operations on them
void igs022_device::handle_command_6d()
{
	const u32 p1 = (m_sharedprotram[0x298 / 2] << 16) | m_sharedprotram[0x29a / 2];
	const u32 p2 = (m_sharedprotram[0x29c / 2] << 16) | m_sharedprotram[0x29e / 2];

	std::ostringstream stream;
	if (VERBOSE & LOG_CMD_6D)
	{
		util::stream_format(stream, "%s: IGS022 command 006d: ASIC RAM %04x %04x %04x %04x ~ ", machine().describe_context(),
			(p1 >> 16) & 0xffff, (p1 >> 0) & 0xffff, (p2 >> 16) & 0xffff, (p2 >> 0) & 0xffff
		);
	}

	switch (p2 & 0xffff)
	{
		case 0x0: // Add values
		{
			const u16 src1  =   p1 >> 16;
			const u16 src2  =   p1 >> 0;
			const u16 dst   =   p2 >> 16;

			const u32 data1 =   read_reg(src1);
			const u32 data2 =   read_reg(src2);
			const u32 res   =   data1 + data2;

			write_reg(dst, res);
			if (VERBOSE & LOG_CMD_6D)
				util::stream_format(stream, "ADD [%04x] = [%04x] + [%04x] (%08x)\n", dst, src1, src2, res);
			break;
		}

		case 0x1: // Sub values (src1 - src2)
		{
			const u16 src1  =   p1 >> 16;
			const u16 src2  =   p1 >> 0;
			const u16 dst   =   p2 >> 16;

			const u32 data1 =   read_reg(src1);
			const u32 data2 =   read_reg(src2);
			const u32 res   =   data1 - data2;

			write_reg(dst, res);

			if (VERBOSE & LOG_CMD_6D)
				util::stream_format(stream, "SUB1 [%04x] = [%04x] - [%04x] (%08x)\n", dst, src1, src2, res);
			break;
		}

		case 0x6: // Sub values (src2 - src1)
		{
			const u16 src1  =   p1 >> 16;
			const u16 src2  =   p1 >> 0;
			const u16 dst   =   p2 >> 16;

			const u32 data1 =   read_reg(src1);
			const u32 data2 =   read_reg(src2);
			const u32 res   =   data2 - data1;

			write_reg(dst, res);

			if (VERBOSE & LOG_CMD_6D)
				util::stream_format(stream, "SUB2 [%04x] = [%04x] - [%04x] (%08x)\n", dst, src2, src1, res);
			break;
		}

		case 0x9: // Set value (Shared Protection RAM -> ASIC RAM)
		{
			const u16 dst   =   p2 >> 16;

			const u32 data  =   p1;

			write_reg(dst, data);

			if (VERBOSE & LOG_CMD_6D)
				util::stream_format(stream, "SET [%04x] = {298, 29a} (%08x)\n", dst, data);
			break;
		}

		case 0xa: // Get value (ASIC RAM -> Shared Protection RAM)
		{
			const u16 src   =   p1 >> 16;

			const u32 data  =   m_regs[src];

			m_sharedprotram[0x29c / 2] = (data >> 16) & 0xffff;
			m_sharedprotram[0x29e / 2] = data & 0xffff;

			if (VERBOSE & LOG_CMD_6D)
				util::stream_format(stream, "GET {29c, 29e} = [%04x] (%08x)\n", src, data);
			break;
		}
	}

	LOGMASKED(LOG_CMD_6D, "%s", stream.str());
	m_sharedprotram[0x202 / 2] = 0x7c; // this mode complete?
}

DEFINE_DEVICE_TYPE(IGS022, igs022_device, "igs022", "IGS022 encrypted DMA device")
