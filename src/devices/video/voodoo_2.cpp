// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_2.c

    3dfx Voodoo Graphics SST-1/2 emulator.

****************************************************************************

    Specs:

    Voodoo 2:
        2,4MB frame buffer RAM
        2,4,8,16MB texture RAM
        90MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 16 pixels/clock
        128 entry PCI FIFO
        memory FIFO up to 65536 entries

**************************************************************************/

#include "emu.h"
#include "voodoo_2.h"

using namespace voodoo;


//**************************************************************************
//  COMMAND FIFO
//**************************************************************************

//-------------------------------------------------
//  command_fifo - constructor
//-------------------------------------------------

command_fifo::command_fifo(voodoo_2_device &device) :
	m_device(device),
	m_ram(nullptr),
	m_mask(0),
	m_enable(false),
	m_count_holes(false),
	m_ram_base(0),
	m_ram_end(0),
	m_read_index(0),
	m_address_min(0),
	m_address_max(0),
	m_depth(0),
	m_holes(0)
{
}


//-------------------------------------------------
//  register_save - register for save states
//-------------------------------------------------

void command_fifo::register_save(save_proxy &save)
{
	save.save_item(NAME(m_enable));
	save.save_item(NAME(m_count_holes));
	save.save_item(NAME(m_ram_base));
	save.save_item(NAME(m_ram_end));
	save.save_item(NAME(m_read_index));
	save.save_item(NAME(m_address_min));
	save.save_item(NAME(m_address_max));
	save.save_item(NAME(m_depth));
	save.save_item(NAME(m_holes));
}


//-------------------------------------------------
//  execute_if_ready - execute everything we have
//  the data for, until we encounter an operation
//  that consumes a non-zero number of cycles
//-------------------------------------------------

u32 command_fifo::execute_if_ready()
{
	while (1)
	{
		// all CMDFIFO commands need at least one word
		if (m_depth == 0)
			return 0;

		// see if we have enough for the current command
		u32 const needed_depth = words_needed(peek_next());
		if (m_depth < needed_depth)
			return 0;

		// read the next command and handle it based on the low 3 bits
		u32 command = read_next();
		u32 cycles = (this->*s_packet_handler[BIT(command, 0, 3)])(command);

		// if the number of cycles is non-zero, return
		if (cycles > 0)
			return cycles;
	}
}


//-------------------------------------------------
//  write - handle a write to the FIFO
//-------------------------------------------------

void command_fifo::write(offs_t addr, u32 data)
{
	if (LOG_CMDFIFO_VERBOSE)
		m_device.logerror("CMDFIFO_w(%04X) = %08X\n", addr, data);

	// write the data if it's within range
	if (addr < m_ram_end)
		m_ram[(addr / 4) & m_mask] = data;

	// count holes?
	if (m_count_holes)
	{
		// in-order, no holes
		if (m_holes == 0 && addr == m_address_min + 4)
		{
			m_address_min = m_address_max = addr;
			m_depth++;
		}

		// out-of-order, below the minimum
		else if (addr < m_address_min)
		{
			if (m_holes != 0)
				m_device.logerror("Unexpected CMDFIFO: AMin=%08X AMax=%08X Holes=%d WroteTo:%08X\n", m_address_min, m_address_max, m_holes, addr);
			m_holes += (addr - m_ram_base) / 4;
			m_address_min = m_ram_base;
			m_address_max = addr;
			m_depth++;
		}

		// out-of-order, but within the min-max range
		else if (addr < m_address_max)
		{
			m_holes--;
			if (m_holes == 0)
			{
				m_depth += (m_address_max - m_address_min) / 4;
				m_address_min = m_address_max;
			}
		}

		// out-of-order, bumping max
		else
		{
			m_holes += (addr - m_address_max) / 4 - 1;
			m_address_max = addr;
		}
	}

	// execute if we can
	if (!m_device.operation_pending())
	{
		s32 cycles = execute_if_ready();
		if (cycles > 0)
		{
			attotime curtime = m_device.machine().time();
			m_device.m_operation_end = curtime + m_device.clocks_to_attotime(cycles);

			if (LOG_FIFO_VERBOSE)
				m_device.logerror("VOODOO.FIFO:direct write start at %s end at %s\n", curtime.as_string(18), m_device.m_operation_end.as_string(18));
		}
	}
}


//-------------------------------------------------
//  words_needed - return the total number of
//  words needed for the given command and all its
//  parameters
//-------------------------------------------------

u32 command_fifo::words_needed(u32 command)
{
	// low 3 bits specify the packet type
	switch (BIT(command, 0, 3))
	{
		case 0:
			// Packet type 0: 1 or 2 words
			//
			//   Word  Bits
			//     0  31:29 = reserved
			//     0  28:6  = Address [24:2]
			//     0   5:3  = Function (0 = NOP, 1 = JSR, 2 = RET, 3 = JMP LOCAL, 4 = JMP AGP)
			//     0   2:0  = Packet type (0)
			return (BIT(command, 3, 3) == 4) ? 2 : 1;

		case 1:
			// Packet type 1: 1 + N words
			//
			//   Word  Bits
			//     0  31:16 = Number of words
			//     0    15  = Increment?
			//     0  14:3  = Register base
			//     0   2:0  = Packet type (1)
			return 1 + BIT(command, 16, 16);

		case 2:
			// Packet type 2: 1 + N words
			//
			//   Word  Bits
			//     0  31:3  = 2D Register mask
			//     0   2:0  = Packet type (2)
			return 1 + population_count_32(BIT(command, 3, 29));

		case 3:
		{
			// Packet type 3: 1 + N words
			//
			//   Word  Bits
			//     0  31:29 = Number of dummy entries following the data
			//     0   28   = Packed color data?
			//     0   25   = Disable ping pong sign correction (0=normal, 1=disable)
			//     0   24   = Culling sign (0=positive, 1=negative)
			//     0   23   = Enable culling (0=disable, 1=enable)
			//     0   22   = Strip mode (0=strip, 1=fan)
			//     0   17   = Setup S1 and T1
			//     0   16   = Setup W1
			//     0   15   = Setup S0 and T0
			//     0   14   = Setup W0
			//     0   13   = Setup Wb
			//     0   12   = Setup Z
			//     0   11   = Setup Alpha
			//     0   10   = Setup RGB
			//     0   9:6  = Number of vertices
			//     0   5:3  = Command (0=Independent tris, 1=Start new strip, 2=Continue strip)
			//     0   2:0  = Packet type (3)

			// determine words per vertex
			u32 count = 2;  // X/Y
			if (BIT(command, 28))
				count += (BIT(command, 10, 2) != 0) ? 1 : 0;       // ARGB in one word
			else
				count += 3 * BIT(command, 10) + BIT(command, 11);  // RGB + A
			count += BIT(command, 12);     // Z
			count += BIT(command, 13);     // Wb
			count += BIT(command, 14);     // W0
			count += 2 * BIT(command, 15); // S0/T0
			count += BIT(command, 16);     // W1
			count += 2 * BIT(command, 17); // S1/T1

			// multiply by the number of verticies
			count *= BIT(command, 6, 4);
			return 1 + count + BIT(command, 29, 3);
		}

		case 4:
			// Packet type 4: 1 + N words
			//
			//   Word  Bits
			//     0  31:29 = Number of dummy entries following the data
			//     0  28:15 = General register mask
			//     0  14:3  = Register base
			//     0   2:0  = Packet type (4)
			return 1 + population_count_32(BIT(command, 15, 14)) + BIT(command, 29, 3);

		case 5:
			// Packet type 5: 2 + N words
			//
			//  Word  Bits
			//    0  31:30 = Space (0,1=reserved, 2=LFB, 3=texture)
			//    0  29:26 = Byte disable W2
			//    0  25:22 = Byte disable WN
			//    0  21:3  = Num words
			//    0   2:0  = Packet type (5)
			return 2 + BIT(command, 3, 19);

		default:
			m_device.logerror("cmdfifo unknown packet type %d\n", command & 7);
			return 1;
	}
}


//-------------------------------------------------
//  packet_type_0 - handle FIFO packet type 0
//-------------------------------------------------

u32 command_fifo::packet_type_0(u32 command)
{
	// Packet type 0: 1 or 2 words
	//
	//   Word  Bits
	//     0  31:29 = reserved
	//     0  28:6  = Address [24:2]
	//     0   5:3  = Function (0 = NOP, 1 = JSR, 2 = RET, 3 = JMP LOCAL, 4 = JMP AGP)
	//     0   2:0  = Packet type (0)
	//     1  31:11 = reserved (JMP AGP only)
	//     1  10:0  = Address [35:25]
	u32 target = BIT(command, 6, 23) << 2;

	// switch off of the specific command; many are unimplemented until we
	// see them in real life
	switch (BIT(command, 3, 3))
	{
		case 0:     // NOP
			if (LOG_CMDFIFO)
				m_device.logerror("  NOP\n");
			break;

		case 1:     // JSR
			if (LOG_CMDFIFO)
				m_device.logerror("  JSR $%06X\n", target);
			m_device.logerror("cmdFifo: Unsupported JSR");
			break;

		case 2:     // RET
			if (LOG_CMDFIFO)
				m_device.logerror("  RET $%06X\n", target);
			m_device.logerror("cmdFifo: Unsupported RET");
			break;

		case 3:     // JMP LOCAL FRAME BUFFER
			if (LOG_CMDFIFO)
				m_device.logerror("  JMP LOCAL FRAMEBUF $%06X\n", target);
			m_read_index = target / 4;
			break;

		case 4:     // JMP AGP
			if (LOG_CMDFIFO)
				m_device.logerror("  JMP AGP $%06X\n", target);
			m_device.logerror("cmdFifo: Unsupported JMP AGP");
			break;

		default:
			m_device.logerror("cmdFifo: Invalid jump command %d", BIT(command, 3, 3));
			break;
	}
	return 0;
}


//-------------------------------------------------
//  packet_type_1 - handle FIFO packet type 1
//-------------------------------------------------

u32 command_fifo::packet_type_1(u32 command)
{
	// Packet type 1: 1 + N words
	//
	//   Word  Bits
	//     0  31:16 = Number of words
	//     0    15  = Increment?
	//     0  14:3  = Register base
	//     0   2:0  = Packet type (1)
	//     1  31:0  = Data word
	u32 count = BIT(command, 16, 16);
	u32 inc = BIT(command, 15);
	u32 target = BIT(command, 3, 12);

	if (LOG_CMDFIFO)
		m_device.logerror("  PACKET TYPE 1: count=%d inc=%d reg=%04X\n", count, inc, target);

	// loop over all registers and write them one at a time
	u32 cycles = 0;
	for (u32 regbit = 0; regbit < count; regbit++, target += inc)
		cycles += m_device.cmdfifo_register_w(target, read_next());
	return cycles;
}


//-------------------------------------------------
//  packet_type_2 - handle FIFO packet type 2
//-------------------------------------------------

u32 command_fifo::packet_type_2(u32 command)
{
	// Packet type 2: 1 + N words
	//
	//   Word  Bits
	//     0  31:3  = 2D Register mask
	//     0   2:0  = Packet type (2)
	//     1  31:0  = Data word
	if (LOG_CMDFIFO)
		m_device.logerror("  PACKET TYPE 2: mask=%X\n", BIT(command, 3, 29));

	// loop over all registers and write them one at a time
	u32 cycles = 0;
	for (u32 regbit = 3; regbit <= 31; regbit++)
		if (BIT(command, regbit))
			cycles += m_device.cmdfifo_2d_w(regbit - 3, read_next());
	return cycles;
}


//-------------------------------------------------
//  packet_type_3 - handle FIFO packet type 3
//-------------------------------------------------

u32 command_fifo::packet_type_3(u32 command)
{
	// Packet type 3: 1 + N words
	//
	//   Word  Bits
	//     0  31:29 = Number of dummy entries following the data
	//     0   28   = Packed color data?
	//     0   25   = Disable ping pong sign correction (0=normal, 1=disable)
	//     0   24   = Culling sign (0=positive, 1=negative)
	//     0   23   = Enable culling (0=disable, 1=enable)
	//     0   22   = Strip mode (0=strip, 1=fan)
	//     0   17   = Setup S1 and T1
	//     0   16   = Setup W1
	//     0   15   = Setup S0 and T0
	//     0   14   = Setup W0
	//     0   13   = Setup Wb
	//     0   12   = Setup Z
	//     0   11   = Setup Alpha
	//     0   10   = Setup RGB
	//     0   9:6  = Number of vertices
	//     0   5:3  = Command (0=Independent tris, 1=Start new strip, 2=Continue strip)
	//     0   2:0  = Packet type (3)
	//     1  31:0  = Data word
	u32 count = BIT(command, 6, 4);
	u32 code = BIT(command, 3, 3);

	if (LOG_CMDFIFO)
		m_device.logerror("  PACKET TYPE 3: count=%d code=%d mask=%03X smode=%02X pc=%d\n", count, code, BIT(command, 10, 12), BIT(command, 22, 6), BIT(command, 28));

	// copy relevant bits into the setup mode register
	m_device.m_reg.write(voodoo_regs::reg_sSetupMode, BIT(command, 10, 8) | (BIT(command, 22, 4) << 16));

	// loop over triangles
	setup_vertex svert = { 0 };
	u32 cycles = 0;
	for (u32 trinum = 0; trinum < count; trinum++)
	{
		// always extract X/Y
		svert.x = read_next_float();
		svert.y = read_next_float();

		// load ARGB values
		if (BIT(command, 28))
		{
			// packed form
			if (BIT(command, 10, 2) != 0)
			{
				rgb_t argb = read_next();
				if (BIT(command, 10))
				{
					svert.r = argb.r();
					svert.g = argb.g();
					svert.b = argb.b();
				}
				if (BIT(command, 11))
					svert.a = argb.a();
			}
		}
		else
		{
			// unpacked form
			if (BIT(command, 10))
			{
				svert.r = read_next_float();
				svert.g = read_next_float();
				svert.b = read_next_float();
			}
			if (BIT(command, 11))
				svert.a = read_next_float();
		}

		// load Z and Wb values
		if (BIT(command, 12))
			svert.z = read_next_float();
		if (BIT(command, 13))
			svert.wb = svert.w0 = svert.w1 = read_next_float();

		// load W0, S0, T0 values
		if (BIT(command, 14))
			svert.w0 = svert.w1 = read_next_float();
		if (BIT(command, 15))
		{
			svert.s0 = svert.s1 = read_next_float();
			svert.t0 = svert.t1 = read_next_float();
		}

		// load W1, S1, T1 values
		if (BIT(command, 16))
			svert.w1 = read_next_float();
		if (BIT(command, 17))
		{
			svert.s1 = read_next_float();
			svert.t1 = read_next_float();
		}

		// if we're starting a new strip, or if this is the first of a set of verts
		// for a series of individual triangles, initialize all the verts
		if ((code == 1 && trinum == 0) || (code == 0 && trinum % 3 == 0))
		{
			m_device.m_sverts = 1;
			m_device.m_svert[0] = m_device.m_svert[1] = m_device.m_svert[2] = svert;
		}

		// otherwise, add this to the list
		else
		{
			// for strip mode, shuffle vertex 1 down to 0
			if (!BIT(command, 22))
				m_device.m_svert[0] = m_device.m_svert[1];

			// copy 2 down to 1 and add our new one regardless
			m_device.m_svert[1] = m_device.m_svert[2];
			m_device.m_svert[2] = svert;

			// if we have enough, draw
			if (++m_device.m_sverts >= 3)
				cycles += m_device.setup_and_draw_triangle();
		}
	}

	// account for the extra dummy words
	consume(BIT(command, 29, 3));
	return cycles;
}


//-------------------------------------------------
//  packet_type_4 - handle FIFO packet type 4
//-------------------------------------------------

u32 command_fifo::packet_type_4(u32 command)
{
	// Packet type 4: 1 + N words
	//
	//   Word  Bits
	//     0  31:29 = Number of dummy entries following the data
	//     0  28:15 = General register mask
	//     0  14:3  = Register base
	//     0   2:0  = Packet type (4)
	//     1  31:0  = Data word
	u32 target = BIT(command, 3, 12);

	if (LOG_CMDFIFO)
		m_device.logerror("  PACKET TYPE 4: mask=%X reg=%04X pad=%d\n", BIT(command, 15, 14), target, BIT(command, 29, 3));

	// loop over all registers and write them one at a time
	u32 cycles = 0;
	for (u32 regbit = 15; regbit <= 28; regbit++, target++)
		if (BIT(command, regbit))
			cycles += m_device.cmdfifo_register_w(target, read_next());

	// account for the extra dummy words
	consume(BIT(command, 29, 3));
	return cycles;
}


//-------------------------------------------------
//  packet_type_5 - handle FIFO packet type 5
//-------------------------------------------------

u32 command_fifo::packet_type_5(u32 command)
{
	// Packet type 5: 2 + N words
	//
	//  Word  Bits
	//    0  31:30 = Space (0,1=reserved, 2=LFB, 3=texture)
	//    0  29:26 = Byte disable W2
	//    0  25:22 = Byte disable WN
	//    0  21:3  = Num words
	//    0   2:0  = Packet type (5)
	//    1  31:30 = Reserved
	//    1  29:0  = Base address [24:0]
	//    2  31:0  = Data word
	u32 count = BIT(command, 3, 19);
	u32 target = read_next() / 4;

	// handle LFB writes
	switch (BIT(command, 30, 2))
	{
		// Linear FB
		case 0:
			if (LOG_CMDFIFO)
				m_device.logerror("  PACKET TYPE 5: FB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, BIT(command, 26, 4), BIT(command, 22, 4));

			m_device.renderer().wait("packet_type_5(0)");
			for (u32 word = 0; word < count; word++)
				m_ram[target++ & m_mask] = little_endianize_int32(read_next());
			break;

		// 3D LFB
		case 2:
			if (LOG_CMDFIFO)
				m_device.logerror("  PACKET TYPE 5: 3D LFB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, BIT(command, 26, 4), BIT(command, 22, 4));

			for (u32 word = 0; word < count; word++)
				m_device.internal_lfb_w(target++, read_next(), 0xffffffff);
			break;

		// Planar YUV
		case 1:
			if (LOG_CMDFIFO)
				m_device.logerror("  PACKET TYPE 5: Planar YUV count=%d dest=%08X bd2=%X bdN=%X\n", count, target, BIT(command, 26, 4), BIT(command, 22, 4));

			fatalerror("%s: Unsupported planar YUV write via cmdFifo", m_device.tag());
			break;

		// Texture port
		case 3:
			if (LOG_CMDFIFO)
				m_device.logerror("  PACKET TYPE 5: textureRAM count=%d dest=%08X bd2=%X bdN=%X\n", count, target, BIT(command, 26, 4), BIT(command, 22, 4));

			for (u32 word = 0; word < count; word++)
				m_device.internal_texture_w(target++, read_next());
			break;
	}
	return 0;
}


//-------------------------------------------------
//  packet_type_unknown - error out on unhandled
//  packets
//-------------------------------------------------

u32 command_fifo::packet_type_unknown(u32 command)
{
	fatalerror("%s: Unsupported cmdFifo packet type %d\n", m_device.tag(), BIT(command, 0, 3));
}


//-------------------------------------------------
//  s_packet_handler - static array of pointers to
//  handler functions
//-------------------------------------------------

command_fifo::packet_handler command_fifo::s_packet_handler[8] =
{
	&command_fifo::packet_type_0,
	&command_fifo::packet_type_1,
	&command_fifo::packet_type_2,
	&command_fifo::packet_type_3,
	&command_fifo::packet_type_4,
	&command_fifo::packet_type_5,
	&command_fifo::packet_type_unknown,
	&command_fifo::packet_type_unknown
};



//**************************************************************************
//  VOODOO 2 DEVICE
//**************************************************************************

//-------------------------------------------------
//  voodoo_2_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(VOODOO_2, voodoo_2_device, "voodoo_2", "3dfx Voodoo 2")

voodoo_2_device::voodoo_2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model) :
	voodoo_1_device(mconfig, type, tag, owner, clock, model),
	m_sverts(0),
	m_cmdfifo(*this)
{
	for (int index = 0; index < std::size(m_regtable); index++)
		m_regtable[index].unpack(s_register_table[index], *this);
}


//-------------------------------------------------
//  core_map - device map for core memory access
//-------------------------------------------------

void voodoo_2_device::core_map(address_map &map)
{
	// Voodoo-2 memory map:
	//
	// cmdfifo = fbi_init7().cmdfifo_enable()
	//
	//   00ab----`--ccccrr`rrrrrr-- Register access (if cmdfifo == 0)
	//                                a = alternate register map if fbi_init3().tri_register_remap()
	//                                b = byte swizzle data if fbi_init0().swizzle_reg_writes()
	//                                c = chip mask select
	//                                r = register index ($00-$FF)
	//   000-----`------rr`rrrrrr-- Register access (if cmdfifo == 1)
	//                                r = register index ($00-$FF)
	//   001--boo`oooooooo`oooooo-- CMDFifo write (if cmdfifo == 1)
	//                                b = byte swizzle data
	//                                o = cmdfifo offset
	//   01-yyyyy`yyyyyxxx`xxxxxxx- Linear frame buffer access (16-bit)
	//   01yyyyyy`yyyyxxxx`xxxxxx-- Linear frame buffer access (32-bit)
	//   1-ccllll`tttttttt`sssssss- Texture memory access, where:
	//                                c = chip mask select
	//                                l = LOD
	//                                t = Y index
	//                                s = X index
	//
	map(0x000000, 0x3fffff).rw(FUNC(voodoo_2_device::map_register_r), FUNC(voodoo_2_device::map_register_w));
	map(0x400000, 0x7fffff).rw(FUNC(voodoo_2_device::map_lfb_r), FUNC(voodoo_2_device::map_lfb_w));
	map(0x800000, 0xffffff).w(FUNC(voodoo_2_device::map_texture_w));
}


//-------------------------------------------------
//  read - generic read handler until everyone is
//  using the memory map
//-------------------------------------------------

u32 voodoo_2_device::read(offs_t offset, u32 mem_mask)
{
	switch (offset >> (22-2))
	{
		case 0x000000 >> 22:
			return map_register_r(offset);

		case 0x400000 >> 22:
			return map_lfb_r(offset - 0x400000/4);

		default:
			return 0xffffffff;
	}
}


//-------------------------------------------------
//  write - generic write handler until everyone is
//  using the memory map
//-------------------------------------------------

void voodoo_2_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset >> (22-2))
	{
		case 0x000000 >> 22:
			map_register_w(offset, data, mem_mask);
			break;

		case 0x400000 >> 22:
			map_lfb_w(offset - 0x400000/4, data, mem_mask);
			break;

		case 0x800000 >> 22:
		case 0xc00000 >> 22:
			map_texture_w(offset - 0x800000/4, data, mem_mask);
			break;
	}
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void voodoo_2_device::device_start()
{
	// start like a Voodoo-1
	voodoo_1_device::device_start();

	// fogDelta skips the low 2 bits
	m_renderer->set_fogdelta_mask(0xfc);

	// bilinear is full resolution
	m_renderer->set_bilinear_mask(0xff);

	// TMU configuration has an extra bit
	m_renderer->set_tmu_config(m_renderer->tmu_config() | 0x800);

	// initialize Voodoo 2 additions
	m_sverts = 0;
	m_cmdfifo.init(m_fbram, m_fbmask + 1);
}



//-------------------------------------------------
//  map_register_w - handle a mapped write to
//  regular register space
//-------------------------------------------------

void voodoo_2_device::map_register_w(offs_t offset, u32 data, u32 mem_mask)
{
	bool pending = prepare_for_write();

	// handle cmdfifo writes
	if (BIT(offset, 21-2) && m_reg.fbi_init7().cmdfifo_enable())
	{
		// check for byte swizzling (bit 18)
		if (BIT(offset, 18-2))
			data = swapendian_int32(data);
		m_cmdfifo.write_direct(BIT(offset, 0, 16), data);
		return;
	}

	// extract chipmask and register
	u32 chipmask = chipmask_from_offset(offset);
	u32 regnum = BIT(offset, 0, 8);

	// handle register swizzling
	if (BIT(offset, 20-2) && m_reg.fbi_init0().swizzle_reg_writes())
		data = swapendian_int32(data);

	// handle aliasing
	if (BIT(offset, 21-2) && m_reg.fbi_init3().tri_register_remap())
		regnum = voodoo_regs::alias(regnum);

	// look up the register
	auto const &regentry = m_regtable[regnum];

	// if this is non-FIFO command, execute immediately
	if (!regentry.is_fifo())
		return void(regentry.write(*this, chipmask, regnum, data));

	// track swap buffers
	if (regnum == voodoo_regs::reg_swapbufferCMD)
		m_swaps_pending++;

	// if cmdfifo is enabled, ignore everything else
	if (m_reg.fbi_init7().cmdfifo_enable())
	{
		logerror("Ignoring write to %s when CMDFIFO is enabled\n", regentry.name());
		return;
	}

	// if we're busy add to the fifo
	if (pending && m_init_enable.enable_pci_fifo())
		return add_to_fifo(memory_fifo::TYPE_REGISTER | (chipmask << 8) | regnum, data, mem_mask);

	// if we get a non-zero number of cycles back, mark things pending
	int cycles = regentry.write(*this, chipmask, regnum, data);
	if (cycles > 0)
	{
		m_operation_end = machine().time() + clocks_to_attotime(cycles);
		if (LOG_FIFO_VERBOSE)
			logerror("VOODOO.FIFO:direct write start at %s end at %s\n", machine().time().as_string(18), m_operation_end.as_string(18));
	}
}


//-------------------------------------------------
//  soft_reset - handle reset when initiated by
//  a register write
//-------------------------------------------------

void voodoo_2_device::soft_reset()
{
	voodoo_1_device::soft_reset();
	m_cmdfifo.set_enable(0);
}


//-------------------------------------------------
//  register_save - register for save states
//-------------------------------------------------

void voodoo_2_device::register_save(save_proxy &save, u32 total_allocation)
{
	voodoo_1_device::register_save(save, total_allocation);

	// Voodoo 2 stuff
	save.save_item(NAME(m_sverts));
	save.save_class(NAME(m_svert[0]));
	save.save_class(NAME(m_svert[1]));
	save.save_class(NAME(m_svert[2]));
	save.save_class(NAME(m_cmdfifo));
}


//-------------------------------------------------
//  execute_fifos - execute commands from the FIFOs
//  until a non-zero cycle count operation is run
//-------------------------------------------------

u32 voodoo_2_device::execute_fifos()
{
	// we might be in CMDFIFO mode
	if (m_cmdfifo.enabled())
		return m_cmdfifo.execute_if_ready();

	// otherwise, run the traditional memory FIFOs
	return voodoo_1_device::execute_fifos();
}


//-------------------------------------------------
//  reg_hvretrace_r - hvRetrace register read
//-------------------------------------------------

u32 voodoo_2_device::reg_hvretrace_r(u32 chipmask, u32 regnum)
{
	// return 0 for vertical if vblank is active
	u32 result = m_vblank ? 0 : screen().vpos();
	return result |= screen().hpos() << 16;
}


//-------------------------------------------------
//  reg_cmdfifoptr_r - cmdFifoRdPtr register read
//-------------------------------------------------

u32 voodoo_2_device::reg_cmdfifoptr_r(u32 chipmask, u32 regnum)
{
	return m_cmdfifo.read_pointer();
}


//-------------------------------------------------
//  reg_cmdfifodepth_r - cmdFifoDepth register read
//-------------------------------------------------

u32 voodoo_2_device::reg_cmdfifodepth_r(u32 chipmask, u32 regnum)
{
	return m_cmdfifo.depth();
}


//-------------------------------------------------
//  reg_cmdfifoholes_r - cmdFifoHoles register read
//-------------------------------------------------

u32 voodoo_2_device::reg_cmdfifoholes_r(u32 chipmask, u32 regnum)
{
	return m_cmdfifo.holes();
}


//-------------------------------------------------
//  reg_intrctrl_w - intrCtrl register write
//-------------------------------------------------

u32 voodoo_2_device::reg_intrctrl_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_reg.write(regnum, data);

		// Setting bit 31 clears the PCI interrupts
		if (BIT(data, 31) && !m_pciint_cb.isnull())
			m_pciint_cb(false);
	}
	return 0;
}


//-------------------------------------------------
//  reg_video2_w -- write to a video configuration
//  register; synchronize then recompute everything
//-------------------------------------------------

u32 voodoo_2_device::reg_video2_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_renderer->wait("reg_video2_w");
		m_reg.write(regnum, data);

		auto const hsync = m_reg.hsync<false>();
		auto const vsync = m_reg.vsync<false>();
		auto const back_porch = m_reg.back_porch<false>();
		auto const video_dimensions = m_reg.video_dimensions<false>();
		if (hsync.raw() != 0 && vsync.raw() != 0 && video_dimensions.raw() != 0 && back_porch.raw() != 0)
		{
			recompute_video_timing(
					hsync.hsync_on(), hsync.hsync_off(),
					video_dimensions.xwidth(), back_porch.horizontal() + 2,
					vsync.vsync_on(), vsync.vsync_off(),
					video_dimensions.yheight(), back_porch.vertical());
		}
	}
	return 0;
}


//-------------------------------------------------
//  reg_sargb_w -- sARGB register write
//-------------------------------------------------

u32 voodoo_2_device::reg_sargb_w(u32 chipmask, u32 regnum, u32 data)
{
	rgb_t rgbdata(data);

	// expand ARGB values into their float registers
	m_reg.write_float(voodoo_regs::reg_sAlpha, float(rgbdata.a()));
	m_reg.write_float(voodoo_regs::reg_sRed, float(rgbdata.r()));
	m_reg.write_float(voodoo_regs::reg_sGreen, float(rgbdata.g()));
	m_reg.write_float(voodoo_regs::reg_sBlue, float(rgbdata.b()));
	return 0;
}


//-------------------------------------------------
//  reg_userintr_w -- userIntr register write
//-------------------------------------------------

u32 voodoo_2_device::reg_userintr_w(u32 chipmask, u32 regnum, u32 data)
{
	m_renderer->wait("reg_userintr_w");

	// Bit 5 of intrCtrl enables user interrupts
	if (m_reg.intr_ctrl().user_interrupt_enable())
	{
		// Bits 19:12 are set to cmd 9:2, bit 11 is user interrupt flag
		m_reg.clear_set(voodoo_regs::reg_intrCtrl,
			reg_intr_ctrl::EXTERNAL_PIN_ACTIVE | reg_intr_ctrl::USER_INTERRUPT_TAG_MASK,
			((data << 10) & reg_intr_ctrl::USER_INTERRUPT_TAG_MASK) | reg_intr_ctrl::USER_INTERRUPT_GENERATED);

		// Signal pci interrupt handler
		if (!m_pciint_cb.isnull())
			m_pciint_cb(true);
	}
	return 0;
}


//-------------------------------------------------
//  reg_cmdfifo_w -- general cmdFifo-related
//  register writes
//-------------------------------------------------

u32 voodoo_2_device::reg_cmdfifo_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_renderer->wait("reg_cmdfifo_w");
		m_reg.write(regnum, data);
		m_cmdfifo.set_base(BIT(m_reg.read(voodoo_regs::reg_cmdFifoBaseAddr), 0, 10) << 12);
		m_cmdfifo.set_end((BIT(m_reg.read(voodoo_regs::reg_cmdFifoBaseAddr), 16, 10) + 1) << 12);
		m_cmdfifo.set_address_min(m_reg.read(voodoo_regs::reg_cmdFifoAMin));
		m_cmdfifo.set_address_max(m_reg.read(voodoo_regs::reg_cmdFifoAMax));
	}
	return 0;
}


//-------------------------------------------------
//  reg_cmdfifoptr_w -- cmdFifoRdPtr register write
//-------------------------------------------------

u32 voodoo_2_device::reg_cmdfifoptr_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_reg.write(regnum, data);
		m_cmdfifo.set_read_pointer(data);
	}
	return 0;
}


//-------------------------------------------------
//  reg_cmdfifodepth_w -- cmdFifoDepth register
//  write
//-------------------------------------------------

u32 voodoo_2_device::reg_cmdfifodepth_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_reg.write(regnum, data);
		m_cmdfifo.set_depth(data);
	}
	return 0;
}


//-------------------------------------------------
//  reg_cmdfifoholes_w -- cmdFifoHoles register
//  write
//-------------------------------------------------

u32 voodoo_2_device::reg_cmdfifoholes_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_reg.write(regnum, data);
		m_cmdfifo.set_holes(data);
	}
	return 0;
}


//-------------------------------------------------
//  reg_fbiinit5_7_w -- fbiInit5/6/7 register write
//-------------------------------------------------

u32 voodoo_2_device::reg_fbiinit5_7_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0) && m_init_enable.enable_hw_init())
	{
		u32 delta = m_reg.read(regnum) ^ data;
		m_reg.write(regnum, data);

		// a few bits affect video memory configuration
		if ((regnum == voodoo_regs::reg_fbiInit5 && BIT(delta, 9, 2) != 0) ||
			(regnum == voodoo_regs::reg_fbiInit6 && BIT(delta, 30, 1) != 0))
		{
			m_renderer->wait("reg_fbiinit5_7_w");
			recompute_video_memory();
		}
		m_cmdfifo.set_enable(m_reg.fbi_init7().cmdfifo_enable());
		m_cmdfifo.set_count_holes(!m_reg.fbi_init7().disable_cmdfifo_holes());
	}
	return 0;
}


//-------------------------------------------------
//  reg_draw_tri_w -- sDrawTri register write
//-------------------------------------------------

u32 voodoo_2_device::reg_draw_tri_w(u32 chipmask, u32 regnum, u32 data)
{
	return draw_triangle();
}


//-------------------------------------------------
//  reg_begin_tri_w -- sBeginTri register write
//-------------------------------------------------

u32 voodoo_2_device::reg_begin_tri_w(u32 chipmask, u32 regnum, u32 data)
{
	return begin_triangle();
}


//-------------------------------------------------
//  cmdfifo_register_w -- handle a register write
//  from the cmdfifo
//-------------------------------------------------

u32 voodoo_2_device::cmdfifo_register_w(u32 offset, u32 data)
{
	u32 chipmask = chipmask_from_offset(offset);
	u32 regnum = BIT(offset, 0, 8);
	return m_regtable[regnum].write(*this, chipmask, regnum, data);
}


//-------------------------------------------------
//  cmdfifo_2d_w -- handle a 2D register write
//  from the cmdfifo
//-------------------------------------------------

u32 voodoo_2_device::cmdfifo_2d_w(u32 offset, u32 data)
{
	u32 regnum = voodoo_regs::reg_bltSrcBaseAddr + offset;
	return m_regtable[regnum].write(*this, 0x1, regnum, data);
}


//-------------------------------------------------
//  vblank_start -- timer callback for the start
//  of VBLANK
//-------------------------------------------------

void voodoo_2_device::vblank_start(s32 param)
{
	voodoo_1_device::vblank_start(param);

	// signal PCI VBLANK rising IRQ on Voodoo-2 and later
	if (m_reg.intr_ctrl().vsync_rising_enable())
	{
		m_reg.clear_set(voodoo_regs::reg_intrCtrl, reg_intr_ctrl::EXTERNAL_PIN_ACTIVE, reg_intr_ctrl::VSYNC_RISING_GENERATED);
		if (!m_pciint_cb.isnull())
			m_pciint_cb(true);
	}
}


//-------------------------------------------------
//  vblank_stop -- timer callback for the end of
//  VBLANK
//-------------------------------------------------

void voodoo_2_device::vblank_stop(s32 param)
{
	voodoo_1_device::vblank_stop(param);

	// signal PCI VBLANK falling IRQ on Voodoo-2 and later
	if (m_reg.intr_ctrl().vsync_falling_enable())
	{
		m_reg.clear_set(voodoo_regs::reg_intrCtrl, reg_intr_ctrl::EXTERNAL_PIN_ACTIVE, reg_intr_ctrl::VSYNC_FALLING_GENERATED);
		if (!m_pciint_cb.isnull())
			m_pciint_cb(true);
	}
}


//-------------------------------------------------
//  recompute_video_memory -- compute the layout
//  of video memory
//-------------------------------------------------

void voodoo_2_device::recompute_video_memory()
{
	// for backwards compatibility, the triple-buffered bit is still supported
	u32 config = m_reg.fbi_init2().enable_triple_buf();

	// but if left at 0, configuration comes from fbiInit5 instead
	if (config == 0)
		config = m_reg.fbi_init5().buffer_allocation();

	// 6-bit tile count is assembled from various bits; tiles are 32x32
	u32 xtiles = m_reg.fbi_init6().x_video_tiles_bit0() |
				 (m_reg.fbi_init1().x_video_tiles() << 1) |
				 (m_reg.fbi_init1().x_video_tiles_bit5() << 5);
	recompute_video_memory_common(config, xtiles * 32);
}


//-------------------------------------------------
//  begin_triangle - execute the 'beginTri'
//  command
//-------------------------------------------------

s32 voodoo_2_device::begin_triangle()
{
	// extract setup data
	auto &sv = m_svert[2];
	sv.x = m_reg.read_float(voodoo_regs::reg_sVx);
	sv.y = m_reg.read_float(voodoo_regs::reg_sVy);
	sv.wb = m_reg.read_float(voodoo_regs::reg_sWb);
	sv.w0 = m_reg.read_float(voodoo_regs::reg_sWtmu0);
	sv.s0 = m_reg.read_float(voodoo_regs::reg_sS_W0);
	sv.t0 = m_reg.read_float(voodoo_regs::reg_sT_W0);
	sv.w1 = m_reg.read_float(voodoo_regs::reg_sWtmu1);
	sv.s1 = m_reg.read_float(voodoo_regs::reg_sS_Wtmu1);
	sv.t1 = m_reg.read_float(voodoo_regs::reg_sT_Wtmu1);
	sv.a = m_reg.read_float(voodoo_regs::reg_sAlpha);
	sv.r = m_reg.read_float(voodoo_regs::reg_sRed);
	sv.g = m_reg.read_float(voodoo_regs::reg_sGreen);
	sv.b = m_reg.read_float(voodoo_regs::reg_sBlue);

	// spread it across all three verts and reset the count
	m_svert[0] = m_svert[1] = sv;
	m_sverts = 1;
	return 0;
}


//-------------------------------------------------
//  draw_triangle - execute the 'DrawTri'
//  command
//-------------------------------------------------

s32 voodoo_2_device::draw_triangle()
{
	// for strip mode, shuffle vertex 1 down to 0
	if (!m_reg.setup_mode().fan_mode())
		m_svert[0] = m_svert[1];

	// copy 2 down to 1 regardless
	m_svert[1] = m_svert[2];

	// extract setup data
	auto &sv = m_svert[2];
	sv.x = m_reg.read_float(voodoo_regs::reg_sVx);
	sv.y = m_reg.read_float(voodoo_regs::reg_sVy);
	sv.wb = m_reg.read_float(voodoo_regs::reg_sWb);
	sv.w0 = m_reg.read_float(voodoo_regs::reg_sWtmu0);
	sv.s0 = m_reg.read_float(voodoo_regs::reg_sS_W0);
	sv.t0 = m_reg.read_float(voodoo_regs::reg_sT_W0);
	sv.w1 = m_reg.read_float(voodoo_regs::reg_sWtmu1);
	sv.s1 = m_reg.read_float(voodoo_regs::reg_sS_Wtmu1);
	sv.t1 = m_reg.read_float(voodoo_regs::reg_sT_Wtmu1);
	sv.a = m_reg.read_float(voodoo_regs::reg_sAlpha);
	sv.r = m_reg.read_float(voodoo_regs::reg_sRed);
	sv.g = m_reg.read_float(voodoo_regs::reg_sGreen);
	sv.b = m_reg.read_float(voodoo_regs::reg_sBlue);

	// if we have enough verts, go ahead and draw
	int cycles = 0;
	if (++m_sverts >= 3)
		cycles = setup_and_draw_triangle();
	return cycles;
}


//-------------------------------------------------
//  setup_and_draw_triangle - process the setup
//  parameters and render the triangle
//-------------------------------------------------

s32 voodoo_2_device::setup_and_draw_triangle()
{
	auto &sv0 = m_svert[0];
	auto &sv1 = m_svert[1];
	auto &sv2 = m_svert[2];

	// compute the divisor, but we only need to know the sign up front
	// for backface culling
	float divisor = (sv0.x - sv1.x) * (sv0.y - sv2.y) - (sv0.x - sv2.x) * (sv0.y - sv1.y);

	// backface culling
	auto const setup_mode = m_reg.setup_mode();
	if (setup_mode.enable_culling())
	{
		int culling_sign = setup_mode.culling_sign();
		int divisor_sign = (divisor < 0);

		// if doing strips and ping pong is enabled, apply the ping pong
		if (!setup_mode.fan_mode() && !setup_mode.disable_ping_pong_correction())
			culling_sign ^= (m_sverts - 3) & 1;

		// if our sign matches the culling sign, we're done for
		if (divisor_sign == culling_sign)
			return TRIANGLE_SETUP_CLOCKS;
	}

	// compute the reciprocal now that we know we need it
	divisor = 1.0f / divisor;

	// grab the X/Ys at least
	m_reg.write(voodoo_regs::reg_vertexAx, s16(sv0.x * 16.0f));
	m_reg.write(voodoo_regs::reg_vertexAy, s16(sv0.y * 16.0f));
	m_reg.write(voodoo_regs::reg_vertexBx, s16(sv1.x * 16.0f));
	m_reg.write(voodoo_regs::reg_vertexBy, s16(sv1.y * 16.0f));
	m_reg.write(voodoo_regs::reg_vertexCx, s16(sv2.x * 16.0f));
	m_reg.write(voodoo_regs::reg_vertexCy, s16(sv2.y * 16.0f));

	// compute the dx/dy values
	float dx1 = sv0.y - sv2.y;
	float dx2 = sv0.y - sv1.y;
	float dy1 = sv0.x - sv1.x;
	float dy2 = sv0.x - sv2.x;

	// set up R,G,B
	float const argbzscale = 4096.0f;
	float const argbzdiv = argbzscale * divisor;
	if (setup_mode.setup_rgb())
	{
		m_reg.write(voodoo_regs::reg_startR, s32(sv0.r * argbzscale));
		m_reg.write(voodoo_regs::reg_dRdX, s32(((sv0.r - sv1.r) * dx1 - (sv0.r - sv2.r) * dx2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_dRdY, s32(((sv0.r - sv2.r) * dy1 - (sv0.r - sv1.r) * dy2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_startG, s32(sv0.g * argbzscale));
		m_reg.write(voodoo_regs::reg_dGdX, s32(((sv0.g - sv1.g) * dx1 - (sv0.g - sv2.g) * dx2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_dGdY, s32(((sv0.g - sv2.g) * dy1 - (sv0.g - sv1.g) * dy2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_startB, s32(sv0.b * argbzscale));
		m_reg.write(voodoo_regs::reg_dBdX, s32(((sv0.b - sv1.b) * dx1 - (sv0.b - sv2.b) * dx2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_dBdY, s32(((sv0.b - sv2.b) * dy1 - (sv0.b - sv1.b) * dy2) * argbzdiv));
	}

	// set up alpha
	if (setup_mode.setup_alpha())
	{
		m_reg.write(voodoo_regs::reg_startA, s32(sv0.a * argbzscale));
		m_reg.write(voodoo_regs::reg_dAdX, s32(((sv0.a - sv1.a) * dx1 - (sv0.a - sv2.a) * dx2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_dAdY, s32(((sv0.a - sv2.a) * dy1 - (sv0.a - sv1.a) * dy2) * argbzdiv));
	}

	// set up Z
	if (setup_mode.setup_z())
	{
		m_reg.write(voodoo_regs::reg_startZ, s32(sv0.z * argbzscale));
		m_reg.write(voodoo_regs::reg_dZdX, s32(((sv0.z - sv1.z) * dx1 - (sv0.z - sv2.z) * dx2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_dZdY, s32(((sv0.z - sv2.z) * dy1 - (sv0.z - sv1.z) * dy2) * argbzdiv));
	}

	// set up Wb
	float const wscale = 65536.0f * 65536.0f;
	float const wdiv = wscale * divisor;
	auto &tmu0reg = m_tmu[0].regs();
	auto &tmu1reg = m_tmu[1].regs();
	if (setup_mode.setup_wb())
	{
		s64 startw = s64(sv0.wb * wscale);
		s64 dwdx = s64(((sv0.wb - sv1.wb) * dx1 - (sv0.wb - sv2.wb) * dx2) * wdiv);
		s64 dwdy = s64(((sv0.wb - sv2.wb) * dy1 - (sv0.wb - sv1.wb) * dy2) * wdiv);
		m_reg.write_start_w(startw);
		m_reg.write_dw_dx(dwdx);
		m_reg.write_dw_dy(dwdy);
		tmu0reg.write_start_w(startw);
		tmu0reg.write_dw_dx(dwdx);
		tmu0reg.write_dw_dy(dwdy);
		tmu1reg.write_start_w(startw);
		tmu1reg.write_dw_dx(dwdx);
		tmu1reg.write_dw_dy(dwdy);
	}

	// set up W0
	if (setup_mode.setup_w0())
	{
		s64 startw = s64(sv0.w0 * wscale);
		s64 dwdx = s64(((sv0.w0 - sv1.w0) * dx1 - (sv0.w0 - sv2.w0) * dx2) * wdiv);
		s64 dwdy = s64(((sv0.w0 - sv2.w0) * dy1 - (sv0.w0 - sv1.w0) * dy2) * wdiv);
		tmu0reg.write_start_w(startw);
		tmu0reg.write_dw_dx(dwdx);
		tmu0reg.write_dw_dy(dwdy);
		tmu1reg.write_start_w(startw);
		tmu1reg.write_dw_dx(dwdx);
		tmu1reg.write_dw_dy(dwdy);
	}

	// set up S0,T0
	float const stscale = 65536.0f * 65536.0f;
	float const stdiv = stscale * divisor;
	if (setup_mode.setup_st0())
	{
		s64 starts = s64(sv0.s0 * stscale);
		s64 dsdx = s64(((sv0.s0 - sv1.s0) * dx1 - (sv0.s0 - sv2.s0) * dx2) * stdiv);
		s64 dsdy = s64(((sv0.s0 - sv2.s0) * dy1 - (sv0.s0 - sv1.s0) * dy2) * stdiv);
		s64 startt = s64(sv0.t0 * stscale);
		s64 dtdx = s64(((sv0.t0 - sv1.t0) * dx1 - (sv0.t0 - sv2.t0) * dx2) * stdiv);
		s64 dtdy = s64(((sv0.t0 - sv2.t0) * dy1 - (sv0.t0 - sv1.t0) * dy2) * stdiv);
		tmu0reg.write_start_s(starts);
		tmu0reg.write_start_t(startt);
		tmu0reg.write_ds_dx(dsdx);
		tmu0reg.write_dt_dx(dtdx);
		tmu0reg.write_ds_dy(dsdy);
		tmu0reg.write_dt_dy(dtdy);
		tmu1reg.write_start_s(starts);
		tmu1reg.write_start_t(startt);
		tmu1reg.write_ds_dx(dsdx);
		tmu1reg.write_dt_dx(dtdx);
		tmu1reg.write_ds_dy(dsdy);
		tmu1reg.write_dt_dy(dtdy);
	}

	// set up W1
	if (setup_mode.setup_w1())
	{
		s64 startw = s64(sv0.w1 * wscale);
		s64 dwdx = s64(((sv0.w1 - sv1.w1) * dx1 - (sv0.w1 - sv2.w1) * dx2) * wdiv);
		s64 dwdy = s64(((sv0.w1 - sv2.w1) * dy1 - (sv0.w1 - sv1.w1) * dy2) * wdiv);
		tmu1reg.write_start_w(startw);
		tmu1reg.write_dw_dx(dwdx);
		tmu1reg.write_dw_dy(dwdy);
	}

	// set up S1,T1
	if (setup_mode.setup_st1())
	{
		s64 starts = s64(sv0.s1 * stscale);
		s64 dsdx = s64(((sv0.s1 - sv1.s1) * dx1 - (sv0.s1 - sv2.s1) * dx2) * stdiv);
		s64 dsdy = s64(((sv0.s1 - sv2.s1) * dy1 - (sv0.s1 - sv1.s1) * dy2) * stdiv);
		s64 startt = s64(sv0.t1 * stscale);
		s64 dtdx = s64(((sv0.t1 - sv1.t1) * dx1 - (sv0.t1 - sv2.t1) * dx2) * stdiv);
		s64 dtdy = s64(((sv0.t1 - sv2.t1) * dy1 - (sv0.t1 - sv1.t1) * dy2) * stdiv);
		tmu1reg.write_start_s(starts);
		tmu1reg.write_start_t(startt);
		tmu1reg.write_ds_dx(dsdx);
		tmu1reg.write_dt_dx(dtdx);
		tmu1reg.write_ds_dy(dsdy);
		tmu1reg.write_dt_dy(dtdy);
	}

	// draw the triangle
	return triangle();
}


//**************************************************************************
//  VOODOO 2 REGISTER MAP
//**************************************************************************

#define REGISTER_ENTRY(name, reader, writer, bits, chips, sync, fifo) \
	{ static_register_table_entry<voodoo_2_device>::make_mask(bits), register_table_entry::CHIPMASK_##chips | register_table_entry::SYNC_##sync | register_table_entry::FIFO_##fifo, #name, &voodoo_2_device::reg_##writer##_w, &voodoo_2_device::reg_##reader##_r },

#define RESERVED_ENTRY REGISTER_ENTRY(reserved, invalid, invalid, 32, FBI, NOSYNC, FIFO)

#define RESERVED_ENTRY_x8 RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY RESERVED_ENTRY

static_register_table_entry<voodoo_2_device> const voodoo_2_device::s_register_table[256] =
{
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(status,          status,      invalid,     32, FBI,      NOSYNC,   FIFO)    // 000
	REGISTER_ENTRY(intrCtrl,        passive,     intrctrl,    32, FBI,      NOSYNC, NOFIFO)    // 004 - cmdFIFO mode
	REGISTER_ENTRY(vertexAx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 008
	REGISTER_ENTRY(vertexAy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 00c
	REGISTER_ENTRY(vertexBx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 010
	REGISTER_ENTRY(vertexBy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 014
	REGISTER_ENTRY(vertexCx,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 018
	REGISTER_ENTRY(vertexCy,        invalid,     passive,     16, FBI_TREX, NOSYNC,   FIFO)    // 01c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(startR,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 020
	REGISTER_ENTRY(startG,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 024
	REGISTER_ENTRY(startB,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 028
	REGISTER_ENTRY(startZ,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 02c
	REGISTER_ENTRY(startA,          invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 030
	REGISTER_ENTRY(startS,          invalid,     starts,      32, TREX,     NOSYNC,   FIFO)    // 034
	REGISTER_ENTRY(startT,          invalid,     startt,      32, TREX,     NOSYNC,   FIFO)    // 038
	REGISTER_ENTRY(startW,          invalid,     startw,      32, FBI_TREX, NOSYNC,   FIFO)    // 03c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(dRdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 040
	REGISTER_ENTRY(dGdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 044
	REGISTER_ENTRY(dBdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 048
	REGISTER_ENTRY(dZdX,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 04c
	REGISTER_ENTRY(dAdX,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 050
	REGISTER_ENTRY(dSdX,            invalid,     dsdx,        32, TREX,     NOSYNC,   FIFO)    // 054
	REGISTER_ENTRY(dTdX,            invalid,     dtdx,        32, TREX,     NOSYNC,   FIFO)    // 058
	REGISTER_ENTRY(dWdX,            invalid,     dwdx,        32, FBI_TREX, NOSYNC,   FIFO)    // 05c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(dRdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 060
	REGISTER_ENTRY(dGdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 064
	REGISTER_ENTRY(dBdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 068
	REGISTER_ENTRY(dZdY,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 06c
	REGISTER_ENTRY(dAdY,            invalid,     passive,     24, FBI,      NOSYNC,   FIFO)    // 070
	REGISTER_ENTRY(dSdY,            invalid,     dsdy,        32, TREX,     NOSYNC,   FIFO)    // 074
	REGISTER_ENTRY(dTdY,            invalid,     dtdy,        32, TREX,     NOSYNC,   FIFO)    // 078
	REGISTER_ENTRY(dWdY,            invalid,     dwdy,        32, FBI_TREX, NOSYNC,   FIFO)    // 07c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(triangleCMD,     invalid,     triangle,    32, FBI_TREX, NOSYNC,   FIFO)    // 080
	RESERVED_ENTRY                                                                             // 084
	REGISTER_ENTRY(fvertexAx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 088
	REGISTER_ENTRY(fvertexAy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 08c
	REGISTER_ENTRY(fvertexBx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 090
	REGISTER_ENTRY(fvertexBy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 094
	REGISTER_ENTRY(fvertexCx,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 098
	REGISTER_ENTRY(fvertexCy,       invalid,     fpassive_4,  32, FBI_TREX, NOSYNC,   FIFO)    // 09c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fstartR,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a0
	REGISTER_ENTRY(fstartG,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a4
	REGISTER_ENTRY(fstartB,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0a8
	REGISTER_ENTRY(fstartZ,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0ac
	REGISTER_ENTRY(fstartA,         invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0b0
	REGISTER_ENTRY(fstartS,         invalid,     fstarts,     32, TREX,     NOSYNC,   FIFO)    // 0b4
	REGISTER_ENTRY(fstartT,         invalid,     fstartt,     32, TREX,     NOSYNC,   FIFO)    // 0b8
	REGISTER_ENTRY(fstartW,         invalid,     fstartw,     32, FBI_TREX, NOSYNC,   FIFO)    // 0bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fdRdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c0
	REGISTER_ENTRY(fdGdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c4
	REGISTER_ENTRY(fdBdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0c8
	REGISTER_ENTRY(fdZdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0cc
	REGISTER_ENTRY(fdAdX,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0d0
	REGISTER_ENTRY(fdSdX,           invalid,     fdsdx,       32, TREX,     NOSYNC,   FIFO)    // 0d4
	REGISTER_ENTRY(fdTdX,           invalid,     fdtdx,       32, TREX,     NOSYNC,   FIFO)    // 0d8
	REGISTER_ENTRY(fdWdX,           invalid,     fdwdx,       32, FBI_TREX, NOSYNC,   FIFO)    // 0dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fdRdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e0
	REGISTER_ENTRY(fdGdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e4
	REGISTER_ENTRY(fdBdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0e8
	REGISTER_ENTRY(fdZdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0ec
	REGISTER_ENTRY(fdAdY,           invalid,     fpassive_12, 32, FBI,      NOSYNC,   FIFO)    // 0f0
	REGISTER_ENTRY(fdSdY,           invalid,     fdsdy,       32, TREX,     NOSYNC,   FIFO)    // 0f4
	REGISTER_ENTRY(fdTdY,           invalid,     fdtdy,       32, TREX,     NOSYNC,   FIFO)    // 0f8
	REGISTER_ENTRY(fdWdY,           invalid,     fdwdy,       32, FBI_TREX, NOSYNC,   FIFO)    // 0fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(ftriangleCMD,    invalid,     triangle,    32, FBI_TREX, NOSYNC,   FIFO)    // 100
	REGISTER_ENTRY(fbzColorPath,    passive,     passive,     30, FBI_TREX, NOSYNC,   FIFO)    // 104
	REGISTER_ENTRY(fogMode,         passive,     passive,      8, FBI_TREX, NOSYNC,   FIFO)    // 108
	REGISTER_ENTRY(alphaMode,       passive,     passive,     32, FBI_TREX, NOSYNC,   FIFO)    // 10c
	REGISTER_ENTRY(fbzMode,         passive,     passive,     22, FBI_TREX,   SYNC,   FIFO)    // 110
	REGISTER_ENTRY(lfbMode,         passive,     passive,     17, FBI_TREX,   SYNC,   FIFO)    // 114
	REGISTER_ENTRY(clipLeftRight,   passive,     passive,     28, FBI_TREX,   SYNC,   FIFO)    // 118
	REGISTER_ENTRY(clipLowYHighY,   passive,     passive,     28, FBI_TREX,   SYNC,   FIFO)    // 11c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nopCMD,          invalid,     nop,          2, FBI_TREX,   SYNC,   FIFO)    // 120
	REGISTER_ENTRY(fastfillCMD,     invalid,     fastfill,     0, FBI,        SYNC,   FIFO)    // 124
	REGISTER_ENTRY(swapbufferCMD,   invalid,     swapbuffer,  10, FBI,        SYNC,   FIFO)    // 128
	REGISTER_ENTRY(fogColor,        invalid,     passive,     24, FBI,        SYNC,   FIFO)    // 12c
	REGISTER_ENTRY(zaColor,         invalid,     passive,     32, FBI,        SYNC,   FIFO)    // 130
	REGISTER_ENTRY(chromaKey,       invalid,     passive,     24, FBI,        SYNC,   FIFO)    // 134
	REGISTER_ENTRY(chromaRange,     invalid,     passive,     29, FBI,        SYNC,   FIFO)    // 138
	REGISTER_ENTRY(userIntrCMD,     invalid,     userintr,    10, FBI,        SYNC,   FIFO)    // 13c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(stipple,         passive,     passive,     32, FBI,        SYNC,   FIFO)    // 140
	REGISTER_ENTRY(color0,          passive,     passive,     32, FBI,        SYNC,   FIFO)    // 144
	REGISTER_ENTRY(color1,          passive,     passive,     32, FBI,        SYNC,   FIFO)    // 148
	REGISTER_ENTRY(fbiPixelsIn,     stats,       invalid,     24, FBI,          NA,     NA)    // 14c
	REGISTER_ENTRY(fbiChromaFail,   stats,       invalid,     24, FBI,          NA,     NA)    // 150
	REGISTER_ENTRY(fbiZfuncFail,    stats,       invalid,     24, FBI,          NA,     NA)    // 154
	REGISTER_ENTRY(fbiAfuncFail,    stats,       invalid,     24, FBI,          NA,     NA)    // 158
	REGISTER_ENTRY(fbiPixelsOut,    stats,       invalid,     24, FBI,          NA,     NA)    // 15c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[0],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 160
	REGISTER_ENTRY(fogTable[1],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 164
	REGISTER_ENTRY(fogTable[2],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 168
	REGISTER_ENTRY(fogTable[3],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 16c
	REGISTER_ENTRY(fogTable[4],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 170
	REGISTER_ENTRY(fogTable[5],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 174
	REGISTER_ENTRY(fogTable[6],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 178
	REGISTER_ENTRY(fogTable[7],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 17c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[8],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 180
	REGISTER_ENTRY(fogTable[9],     invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 184
	REGISTER_ENTRY(fogTable[10],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 188
	REGISTER_ENTRY(fogTable[11],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 18c
	REGISTER_ENTRY(fogTable[12],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 190
	REGISTER_ENTRY(fogTable[13],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 194
	REGISTER_ENTRY(fogTable[14],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 198
	REGISTER_ENTRY(fogTable[15],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 19c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[16],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a0
	REGISTER_ENTRY(fogTable[17],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a4
	REGISTER_ENTRY(fogTable[18],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1a8
	REGISTER_ENTRY(fogTable[19],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1ac
	REGISTER_ENTRY(fogTable[20],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b0
	REGISTER_ENTRY(fogTable[21],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b4
	REGISTER_ENTRY(fogTable[22],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1b8
	REGISTER_ENTRY(fogTable[23],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fogTable[24],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c0
	REGISTER_ENTRY(fogTable[25],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c4
	REGISTER_ENTRY(fogTable[26],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1c8
	REGISTER_ENTRY(fogTable[27],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1cc
	REGISTER_ENTRY(fogTable[28],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d0
	REGISTER_ENTRY(fogTable[29],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d4
	REGISTER_ENTRY(fogTable[30],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1d8
	REGISTER_ENTRY(fogTable[31],    invalid,     fogtable,    32, FBI,        SYNC,   FIFO)    // 1dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(cmdFifoBaseAddr, passive,     cmdfifo,     26, FBI,        SYNC, NOFIFO)    // 1e0 - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoBump,     passive,     unimplemented,16,FBI,        SYNC, NOFIFO)    // 1e4 - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoRdPtr,    cmdfifoptr,  cmdfifoptr,  32, FBI,        SYNC, NOFIFO)    // 1e8 - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoAMin,     passive,     cmdfifo,     32, FBI,        SYNC, NOFIFO)    // 1ec - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoAMax,     passive,     cmdfifo,     32, FBI,        SYNC, NOFIFO)    // 1f0 - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoDepth,    cmdfifodepth,cmdfifodepth,16, FBI,        SYNC, NOFIFO)    // 1f4 - cmdFIFO mode
	REGISTER_ENTRY(cmdFifoHoles,    cmdfifoholes,cmdfifoholes,16, FBI,        SYNC, NOFIFO)    // 1f8 - cmdFIFO mode
	RESERVED_ENTRY                                                                             // 1fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(fbiInit4,        passive,     fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 200
	REGISTER_ENTRY(vRetrace,        vretrace,    invalid,     13, FBI,          NA,     NA)    // 204
	REGISTER_ENTRY(backPorch,       passive,     video2,      25, FBI,      NOSYNC, NOFIFO)    // 208
	REGISTER_ENTRY(videoDimensions, passive,     video2,      27, FBI,      NOSYNC, NOFIFO)    // 20c
	REGISTER_ENTRY(fbiInit0,        passive,     fbiinit,     31, FBI,      NOSYNC, NOFIFO)    // 210
	REGISTER_ENTRY(fbiInit1,        passive,     fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 214
	REGISTER_ENTRY(fbiInit2,        fbiinit2,    fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 218
	REGISTER_ENTRY(fbiInit3,        passive,     fbiinit,     32, FBI,      NOSYNC, NOFIFO)    // 21c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(hSync,           invalid,     video2,      27, FBI,      NOSYNC, NOFIFO)    // 220
	REGISTER_ENTRY(vSync,           invalid,     video2,      29, FBI,      NOSYNC, NOFIFO)    // 224
	REGISTER_ENTRY(clutData,        invalid,     clut,        30, FBI,      NOSYNC, NOFIFO)    // 228
	REGISTER_ENTRY(dacData,         invalid,     dac,         14, FBI,      NOSYNC, NOFIFO)    // 22c
	REGISTER_ENTRY(maxRgbDelta,     invalid,     unimplemented,24,FBI,      NOSYNC, NOFIFO)    // 230
	REGISTER_ENTRY(hBorder,         invalid,     unimplemented,25,FBI,      NOSYNC, NOFIFO)    // 234 - cmdFIFO mode
	REGISTER_ENTRY(vBorder,         invalid,     unimplemented,25,FBI,      NOSYNC, NOFIFO)    // 238 - cmdFIFO mode
	REGISTER_ENTRY(borderColor,     invalid,     unimplemented,24,FBI,      NOSYNC, NOFIFO)    // 23c - cmdFIFO mode
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(hvRetrace,       hvretrace,   invalid,     27, FBI,          NA,     NA)    // 240
	REGISTER_ENTRY(fbiInit5,        passive,     fbiinit5_7,  32, FBI,      NOSYNC, NOFIFO)    // 244 - cmdFIFO mode
	REGISTER_ENTRY(fbiInit6,        passive,     fbiinit5_7,  31, FBI,      NOSYNC, NOFIFO)    // 248 - cmdFIFO mode
	REGISTER_ENTRY(fbiInit7,        passive,     fbiinit5_7,  28, FBI,      NOSYNC, NOFIFO)    // 24c - cmdFIFO mode
	RESERVED_ENTRY                                                                             // 250
	RESERVED_ENTRY                                                                             // 254
	REGISTER_ENTRY(fbiSwapHistory,  passive,     invalid,     32, FBI,          NA,     NA)    // 258
	REGISTER_ENTRY(fbiTrianglesOut, passive,     invalid,     24, FBI,          NA,     NA)    // 25c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sSetupMode,      invalid,     passive,     20, FBI,      NOSYNC,   FIFO)    // 260
	REGISTER_ENTRY(sVx,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 264
	REGISTER_ENTRY(sVy,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 268
	REGISTER_ENTRY(sARGB,           invalid,     sargb,       32, FBI,      NOSYNC,   FIFO)    // 26c
	REGISTER_ENTRY(sRed,            invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 270
	REGISTER_ENTRY(sGreen,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 274
	REGISTER_ENTRY(sBlue,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 278
	REGISTER_ENTRY(sAlpha,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 27c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sVz,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 280
	REGISTER_ENTRY(sWb,             invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 284
	REGISTER_ENTRY(sWtmu0,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 288
	REGISTER_ENTRY(sS_W0,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 28c
	REGISTER_ENTRY(sT_W0,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 290
	REGISTER_ENTRY(sWtmu1,          invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 294
	REGISTER_ENTRY(sS_W1,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 298
	REGISTER_ENTRY(sT_W1,           invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 29c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(sDrawTriCMD,     invalid,     draw_tri,     1, FBI,      NOSYNC,   FIFO)    // 2a0
	REGISTER_ENTRY(sBeginTriCMD,    invalid,     begin_tri,    1, FBI,      NOSYNC,   FIFO)    // 2a4
	RESERVED_ENTRY                                                                             // 2a8
	RESERVED_ENTRY                                                                             // 2ac
	RESERVED_ENTRY                                                                             // 2b0
	RESERVED_ENTRY                                                                             // 2b4
	RESERVED_ENTRY                                                                             // 2b8
	RESERVED_ENTRY                                                                             // 2bc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(bltSrcBaseAddr,  passive,     passive,     22, FBI,      NOSYNC,   FIFO)    // 2c0
	REGISTER_ENTRY(bltDstBaseAddr,  passive,     passive,     22, FBI,      NOSYNC,   FIFO)    // 2c4
	REGISTER_ENTRY(bltXYStrides,    passive,     passive,     28, FBI,      NOSYNC,   FIFO)    // 2c8
	REGISTER_ENTRY(bltSrcChromaRange,passive,    passive,     32, FBI,      NOSYNC,   FIFO)    // 2cc
	REGISTER_ENTRY(bltDstChromaRange,passive,    passive,     32, FBI,      NOSYNC,   FIFO)    // 2d0
	REGISTER_ENTRY(bltClipX,        passive,     passive,     26, FBI,      NOSYNC,   FIFO)    // 2d4
	REGISTER_ENTRY(bltClipY,        passive,     passive,     26, FBI,      NOSYNC,   FIFO)    // 2d8
	RESERVED_ENTRY                                                                             // 2dc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(bltSrcXY,        passive,     passive,     27, FBI,      NOSYNC,   FIFO)    // 2e0
	REGISTER_ENTRY(bltDstXY,        passive,     passive,     32, FBI,      NOSYNC,   FIFO)    // 2e4
	REGISTER_ENTRY(bltSize,         passive,     passive,     32, FBI,      NOSYNC,   FIFO)    // 2e8
	REGISTER_ENTRY(bltRop,          passive,     passive,     16, FBI,      NOSYNC,   FIFO)    // 2ec
	REGISTER_ENTRY(bltColor,        passive,     passive,     32, FBI,      NOSYNC,   FIFO)    // 2f0
	RESERVED_ENTRY                                                                             // 2f4
	REGISTER_ENTRY(bltCommand,      passive,     unimplemented,32,FBI,      NOSYNC,   FIFO)    // 2f8
	REGISTER_ENTRY(bltData,         invalid,     passive,     32, FBI,      NOSYNC,   FIFO)    // 2fc
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(textureMode,     invalid,     texture,     32, TREX,     NOSYNC,   FIFO)    // 300
	REGISTER_ENTRY(tLOD,            invalid,     texture,     32, TREX,     NOSYNC,   FIFO)    // 304
	REGISTER_ENTRY(tDetail,         invalid,     texture,     22, TREX,     NOSYNC,   FIFO)    // 308
	REGISTER_ENTRY(texBaseAddr,     invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 30c
	REGISTER_ENTRY(texBaseAddr_1,   invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 310
	REGISTER_ENTRY(texBaseAddr_2,   invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 314
	REGISTER_ENTRY(texBaseAddr_3_8, invalid,     texture,     19, TREX,     NOSYNC,   FIFO)    // 318
	REGISTER_ENTRY(trexInit0,       invalid,     passive,     32, TREX,       SYNC,   FIFO)    // 31c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(trexInit1,       invalid,     passive,     32, TREX,       SYNC,   FIFO)    // 320
	REGISTER_ENTRY(nccTable0[0],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 324
	REGISTER_ENTRY(nccTable0[1],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 328
	REGISTER_ENTRY(nccTable0[2],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 32c
	REGISTER_ENTRY(nccTable0[3],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 330
	REGISTER_ENTRY(nccTable0[4],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 334
	REGISTER_ENTRY(nccTable0[5],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 338
	REGISTER_ENTRY(nccTable0[6],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 33c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable0[7],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 340
	REGISTER_ENTRY(nccTable0[8],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 344
	REGISTER_ENTRY(nccTable0[9],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 348
	REGISTER_ENTRY(nccTable0[10],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 34c
	REGISTER_ENTRY(nccTable0[11],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 350
	REGISTER_ENTRY(nccTable1[0],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 354
	REGISTER_ENTRY(nccTable1[1],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 358
	REGISTER_ENTRY(nccTable1[2],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 35c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable1[3],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 360
	REGISTER_ENTRY(nccTable1[4],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 364
	REGISTER_ENTRY(nccTable1[5],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 368
	REGISTER_ENTRY(nccTable1[6],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 36c
	REGISTER_ENTRY(nccTable1[7],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 370
	REGISTER_ENTRY(nccTable1[8],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 374
	REGISTER_ENTRY(nccTable1[9],    invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 378
	REGISTER_ENTRY(nccTable1[10],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 37c
	//             name             rd handler   wr handler  bits chips     sync?     fifo?
	REGISTER_ENTRY(nccTable1[11],   invalid,     palette,     32, TREX,       SYNC,   FIFO)    // 380
	RESERVED_ENTRY                                                                             // 384
	RESERVED_ENTRY                                                                             // 388
	RESERVED_ENTRY                                                                             // 38c
	RESERVED_ENTRY                                                                             // 390
	RESERVED_ENTRY                                                                             // 394
	RESERVED_ENTRY                                                                             // 398
	RESERVED_ENTRY                                                                             // 39c

	RESERVED_ENTRY_x8                                                                          // 3a0-3bc
	RESERVED_ENTRY_x8                                                                          // 3c0-3dc
	RESERVED_ENTRY_x8                                                                          // 3e0-3fc
};
