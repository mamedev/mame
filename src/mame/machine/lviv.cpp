// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/***************************************************************************

  lviv.cpp

  Functions to emulate general aspects of PK-01 Lviv (RAM, ROM, interrupts,
  I/O ports)

  Krzysztof Strzecha

***************************************************************************/

#include "emu.h"
#include "includes/lviv.h"

#include "cpu/i8085/i8085.h"


#define LVIV_SNAPSHOT_SIZE  82219


void lviv_state::update_memory()
{
	uint8_t *ram = m_ram->pointer();

	if (BIT(m_ppi_port_outputs[0][2], 1))
	{
		m_bank[0]->set_base(ram);
		m_bank[1]->set_base(ram + 0x4000);
	}
	else
	{
		m_bank[0]->set_base(ram + 0x8000);
		m_bank[1]->set_base(ram + 0xc000);
	}
}

INPUT_CHANGED_MEMBER(lviv_state::reset_button)
{
	machine().schedule_soft_reset();
}

uint8_t lviv_state::ppi0_porta_r()
{
	return 0xff;
}

uint8_t lviv_state::ppi0_portb_r()
{
	return 0xff;
}

uint8_t lviv_state::ppi0_portc_r()
{
	uint8_t data = m_ppi_port_outputs[0][2] & 0x0f;
	if (m_cassette->input() > 0.038)
		data |= 0x10;
	if (m_ppi_port_outputs[0][0] & m_joy_port->read())
		data |= 0x80;
	return data;
}

void lviv_state::ppi0_porta_w(uint8_t data)
{
	m_ppi_port_outputs[0][0] = data;
}

void lviv_state::ppi0_portb_w(uint8_t data)
{
	m_ppi_port_outputs[0][1] = data;
	update_palette(data&0x7f);
}

void lviv_state::ppi0_portc_w(uint8_t data)/* tape in/out, video memory on/off */
{
	m_ppi_port_outputs[0][2] = data;
	if (BIT(m_ppi_port_outputs[0][1], 7))
		m_speaker->level_w(BIT(data, 0));
	m_cassette->output(BIT(data, 0) ? -1.0 : 1.0);
	update_memory();
}

uint8_t lviv_state::ppi1_porta_r()
{
	return 0xff;
}

uint8_t lviv_state::ppi1_portb_r()/* keyboard reading */
{
	return ((m_ppi_port_outputs[1][0] & 0x01) ? 0xff : m_key[0]->read()) &
		   ((m_ppi_port_outputs[1][0] & 0x02) ? 0xff : m_key[1]->read()) &
		   ((m_ppi_port_outputs[1][0] & 0x04) ? 0xff : m_key[2]->read()) &
		   ((m_ppi_port_outputs[1][0] & 0x08) ? 0xff : m_key[3]->read()) &
		   ((m_ppi_port_outputs[1][0] & 0x10) ? 0xff : m_key[4]->read()) &
		   ((m_ppi_port_outputs[1][0] & 0x20) ? 0xff : m_key[5]->read()) &
		   ((m_ppi_port_outputs[1][0] & 0x40) ? 0xff : m_key[6]->read()) &
		   ((m_ppi_port_outputs[1][0] & 0x80) ? 0xff : m_key[7]->read());
}

uint8_t lviv_state::ppi1_portc_r()/* keyboard reading */
{
	return ((m_ppi_port_outputs[1][2] & 0x01) ? 0xff : m_key[ 8]->read()) &
		   ((m_ppi_port_outputs[1][2] & 0x02) ? 0xff : m_key[ 9]->read()) &
		   ((m_ppi_port_outputs[1][2] & 0x04) ? 0xff : m_key[10]->read()) &
		   ((m_ppi_port_outputs[1][2] & 0x08) ? 0xff : m_key[11]->read());
}

void lviv_state::ppi1_porta_w(uint8_t data)/* kayboard scanning */
{
	m_ppi_port_outputs[1][0] = data;
}

void lviv_state::ppi1_portb_w(uint8_t data)
{
	m_ppi_port_outputs[1][1] = data;
}

void lviv_state::ppi1_portc_w(uint8_t data)/* kayboard scanning */
{
	m_ppi_port_outputs[1][2] = data;
}


/* I/O */
uint8_t lviv_state::io_r(offs_t offset)
{
	if (m_startup_mem_map)
	{
		return 0;   /* ??? */
	}
	else
	{
		const uint8_t switch_val = BIT(offset, 4, 2);
		switch (switch_val)
		{
		case 0:
		case 1:
			return m_ppi[switch_val]->read(offset & 3);

		case 2:
		case 3:
		default:
			/* reserved for extension? */
			return 0;   /* ??? */
		}
	}
}

void lviv_state::io_w(offs_t offset, uint8_t data)
{
	address_space &cpuspace = m_maincpu->space(AS_PROGRAM);
	if (m_startup_mem_map)
	{
		uint8_t *ram = m_ram->pointer();

		m_startup_mem_map = 0;

		cpuspace.install_write_bank(0x0000, 0x3fff, m_bank[0]);
		cpuspace.install_write_bank(0x4000, 0x7fff, m_bank[1]);
		cpuspace.install_write_bank(0x8000, 0xbfff, m_bank[2]);
		cpuspace.unmap_write(0xC000, 0xffff);

		m_bank[0]->set_base(ram);
		m_bank[1]->set_base(ram + 0x4000);
		m_bank[2]->set_base(ram + 0x8000);
		m_bank[3]->set_base(m_maincpu_region->base());
	}
	else
	{
		const uint8_t switch_val = BIT(offset, 4, 2);
		switch (switch_val)
		{
		case 0:
		case 1:
			m_ppi[switch_val]->write(offset & 3, data);
			break;

		case 2:
		case 3:
			/* reserved for extension? */
			break;
		}
	}
}


void lviv_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	m_vram = m_ram->pointer() + 0xc000;

	m_startup_mem_map = 1;

	space.unmap_write(0x0000, 0x3fff);
	space.unmap_write(0x4000, 0x7fff);
	space.unmap_write(0x8000, 0xbfff);
	space.unmap_write(0xC000, 0xffff);

	uint8_t *mem = m_maincpu_region->base();
	m_bank[0]->set_base(mem);
	m_bank[1]->set_base(mem);
	m_bank[2]->set_base(mem);
	m_bank[3]->set_base(mem);
}

void lviv_state::machine_start()
{
	save_item(NAME(m_colortable));
	save_item(NAME(m_ppi_port_outputs));
	save_item(NAME(m_startup_mem_map));
}

/*******************************************************************************
Lviv snapshot files (SAV)
-------------------------

00000 - 0000D:  'LVOV/DUMP/2.0/' (like LVT-header)
0000E - 0000F:  'H+' (something additional)
00010           00h
00011 - 0C010:  RAM (0000 - BFFF)
0C011 - 10010:  ROM (C000 - FFFF)
10011 - 14010:  Video RAM (4000 - 7FFF)
14011 - 14110:  Ports map (00 - FF)
14111 - 1411C:  Registers (B,C,D,E,H,L,A,F,SP,PC)
1411D - 1412A:  ??? (something additional)
*******************************************************************************/

void lviv_state::setup_snapshot(uint8_t * data)
{
	/* Set registers */
	uint8_t lo = data[0x14112] & 0x0ff;
	uint8_t hi = data[0x14111] & 0x0ff;
	m_maincpu->set_state_int(i8080_cpu_device::I8085_BC, (hi << 8) | lo);
	lo = data[0x14114] & 0x0ff;
	hi = data[0x14113] & 0x0ff;
	m_maincpu->set_state_int(i8080_cpu_device::I8085_DE, (hi << 8) | lo);
	lo = data[0x14116] & 0x0ff;
	hi = data[0x14115] & 0x0ff;
	m_maincpu->set_state_int(i8080_cpu_device::I8085_HL, (hi << 8) | lo);
	lo = data[0x14118] & 0x0ff;
	hi = data[0x14117] & 0x0ff;
	m_maincpu->set_state_int(i8080_cpu_device::I8085_AF, (hi << 8) | lo);
	lo = data[0x14119] & 0x0ff;
	hi = data[0x1411a] & 0x0ff;
	m_maincpu->set_state_int(i8080_cpu_device::I8085_SP, (hi << 8) | lo);
	lo = data[0x1411b] & 0x0ff;
	hi = data[0x1411c] & 0x0ff;
	m_maincpu->set_state_int(i8080_cpu_device::I8085_PC, (hi << 8) | lo);

	/* Memory dump */
	memcpy (m_ram->pointer(), data+0x0011, 0xc000);
	memcpy (m_ram->pointer()+0xc000, data+0x10011, 0x4000);

	/* Ports */
	m_ppi_port_outputs[0][0] = data[0x14011+0xc0];
	m_ppi_port_outputs[0][1] = data[0x14011+0xc1];
	update_palette(m_ppi_port_outputs[0][1]&0x7f);
	m_ppi_port_outputs[0][2] = data[0x14011+0xc2];
	update_memory();
}

void lviv_state::dump_registers()
{
	logerror("PC   = %04x\n", (unsigned) m_maincpu->state_int(i8080_cpu_device::I8085_PC));
	logerror("SP   = %04x\n", (unsigned) m_maincpu->state_int(i8080_cpu_device::I8085_SP));
	logerror("AF   = %04x\n", (unsigned) m_maincpu->state_int(i8080_cpu_device::I8085_AF));
	logerror("BC   = %04x\n", (unsigned) m_maincpu->state_int(i8080_cpu_device::I8085_BC));
	logerror("DE   = %04x\n", (unsigned) m_maincpu->state_int(i8080_cpu_device::I8085_DE));
	logerror("HL   = %04x\n", (unsigned) m_maincpu->state_int(i8080_cpu_device::I8085_HL));
}

image_verify_result lviv_state::verify_snapshot(uint8_t * data, uint32_t size)
{
	const char* tag = "LVOV/DUMP/2.0/";

	if (strncmp(tag, (char*)data, strlen(tag)))
	{
		logerror("Not a Lviv snapshot\n");
		return image_verify_result::FAIL;
	}

	if (size != LVIV_SNAPSHOT_SIZE)
	{
		logerror ("Incomplete snapshot file\n");
		return image_verify_result::FAIL;
	}

	logerror("returning ID_OK\n");
	return image_verify_result::PASS;
}

SNAPSHOT_LOAD_MEMBER(lviv_state::snapshot_cb)
{
	std::vector<uint8_t> snapshot_data(LVIV_SNAPSHOT_SIZE);

	image.fread(&snapshot_data[0], LVIV_SNAPSHOT_SIZE);

	if (verify_snapshot(&snapshot_data[0], image.length()) != image_verify_result::PASS)
	{
		return image_init_result::FAIL;
	}

	setup_snapshot(&snapshot_data[0]);

	dump_registers();

	logerror("Snapshot file loaded\n");
	return image_init_result::PASS;
}
