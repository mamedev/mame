// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, ElSemi, Angelo Salese
/*********************************************************************************************************************************

	Model 2 Debugger functions

*********************************************************************************************************************************/

#include "emu.h"
#include "includes/model2.h"

#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debugger.h"
 
void model2_state::debug_init()
{
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("m2", CMDFLAG_NONE, 0, 1, 2, std::bind(&model2_state::debug_commands, this, _1, _2));
	}
}

void model2_state::debug_commands(int ref, const std::vector<std::string> &params)
{
	if (params.size() < 1)
		return;
	
	if (params[0] == "geodasm")
		debug_geo_dasm_command(ref, params);
	else if(params[0] == "trilist")
		debug_tri_dump_command(ref, params);
	else
		debug_help_command(ref, params);
}

void model2_state::debug_help_command(int ref, const std::vector<std::string> &params)
{
	debugger_console &con = machine().debugger().console();
	
	con.printf("Available Sega Model 2 commands:\n");
	con.printf("  m2 geodasm,<filename> -- dump current geometrizer DASM in <filename>\n");
	con.printf("  m2 trilist,<filename> -- dump current parsed triangles in <filename>\n");
	con.printf("  m2 help -- this list\n");
}

/*****************************************
 *
 * GEO DASM dumping
 *
 ****************************************/

void model2_state::debug_geo_dasm_command(int ref, const std::vector<std::string> &params)
{
	debugger_console &con = machine().debugger().console();
	FILE *f;
	
	if (params.size() < 2)
	{
		con.printf("Error: not enough parameters for m2 geodasm command\n");
		return;
	}
	
	if (params[1].empty() || params[1].length() > 127)
	{
		con.printf("Error: invalid filename parameter for m2 geodasm command\n");
		return;
	}

	if((f = fopen( params[1].c_str(), "w" )) == nullptr)
	{
		con.printf("Error: while opening %s for writing\n",params[1].c_str());
		return;
	}
		
	int ptr;
	bool end_code = false;
	
	// print some basic info on top
	fprintf(f, "Dump Header info:\n");
	fprintf(f, "GEO address: %08x %08x\n",m_geo_read_start_address+0x900000,m_geo_write_start_address+0x900000);
	fprintf(f, "Screen Info: %lld %d %d\n",m_screen->frame_number(),m_screen->hpos(),m_screen->vpos());
	fprintf(f, "====================\n");

	ptr = m_geo_read_start_address/4;

	while(end_code == false && ptr < 0x20000/4)
	{
		uint32_t opcode;
		uint32_t attr;
		
		fprintf(f, "%08x: \t",ptr*4+0x900000);
		opcode = m_bufferram[ptr++];
		
		// parse jump opcode
		if(opcode & 0x80000000)
		{
			// exit if the operand is illegal
			if(opcode & ~0x8001ffff)
			{
				fprintf(f, "(illegal jump)");
				end_code = true;
			}
			else
			{
				// jump and print into dasm
				ptr = (opcode & 0x1ffff) / 4;
				fprintf(f, "jump %08x",opcode & 0x1ffff);
			}
		}
		else
		{
			
			// parse the opcode info
			switch((opcode >> 23) & 0x1f)
			{
				// nop
				case 0x00: 
					fprintf(f, "nop");
					break;
				// object data display
				case 0x01: 
					fprintf(f, "object data  (point:%08x header:%08x start:%08x count:%08x)",m_bufferram[ptr],m_bufferram[ptr+1],m_bufferram[ptr+2],m_bufferram[ptr+3]);
					ptr+=4;
					break;
				// direct object data display
				case 0x02: 
					fprintf(f, "direct data  (point:%08x header:%08x)",m_bufferram[ptr],m_bufferram[ptr+1]);
					ptr+=2;
					// skip first point
					ptr+=6;
					// parse and get next pointer address
					do{
						attr = m_bufferram[ptr];
						
						ptr++;
						if((attr & 3) == 0)
							continue;
						// check and skip quad/tri data
						ptr += (attr & 1) ? 8 : 5;				
					}while((attr & 3) != 0);					
					
					break;
				// set display window
				case 0x03:
					fprintf(f, "window data  (");
					for(attr=0;attr<6;attr++)
					{
						int x,y;
						x = (m_bufferram[ptr+attr] & 0xfff0000) >> 16;
						if(x & 0x800)
							x = -( 0x800 - (x & 0x7FF) );
						y = (m_bufferram[ptr+attr] & 0xfff) >> 0;
						if(y & 0x800)
							y = -( 0x800 - (y & 0x7FF) );

						fprintf(f,"%d [%d,%d] ",attr,x,y);
					}
					fprintf(f, ")");
					ptr+=6;
					break;
				// set texture color data to RAM
				case 0x04: 
					fprintf(f, "texture data  (start:%08x count:%08x)",m_bufferram[ptr],m_bufferram[ptr+1]);
					ptr+=(2+m_bufferram[ptr+1]);
					break;
				// set polygon data to RAM
				case 0x05: 
					attr = m_bufferram[ptr+1];
					fprintf(f, "polygon data  (start:%08x count:%08x)",m_bufferram[ptr],attr);
					ptr+=(2+attr);
					break;
				// set texture params
				case 0x06: 
					attr = m_bufferram[ptr+1];
					fprintf(f, "texture param  (start:%08x count:%08x)",m_bufferram[ptr],attr);
					ptr+=(2+attr*2);
					break;
				// set mode
				case 0x07: 
					fprintf(f, "mode  (%08x)",m_bufferram[ptr]);
					ptr++;
					break;
				// set z-sort mode
				case 0x08: 
					fprintf(f, "zsort  (%08x)",m_bufferram[ptr]);
					ptr++;
					break;
				// set focus
				case 0x09: 
					fprintf(f, "focus  [%f,%f]",u2f(m_bufferram[ptr]),u2f(m_bufferram[ptr+1]));
					ptr+=2;
					break;
				// set parallel light
				case 0x0a:
					fprintf(f, "light  [%f,%f,%f]",u2f(m_bufferram[ptr]),u2f(m_bufferram[ptr+1]),u2f(m_bufferram[ptr+2]));
					ptr+=3;
					break;
				// set transform matrix
				case 0x0B: 
					fprintf(f, "matrix ");
					for(attr=0;attr<12;attr+=3)
						fprintf(f,"[%f,%f,%f] ",u2f(m_bufferram[ptr]),u2f(m_bufferram[ptr+1]),u2f(m_bufferram[ptr+2]));
					
					ptr+=12;
					break;
				// set translate matrix
				case 0x0C: 
					fprintf(f, "translate  [%f,%f,%f]",u2f(m_bufferram[ptr]),u2f(m_bufferram[ptr+1]),u2f(m_bufferram[ptr+2]));
					ptr+=3;
					break;
				// debug
				case 0x0D:
					fprintf(f, "debug  (%08x %08x)",m_bufferram[ptr],m_bufferram[ptr+1]);
					ptr+=2;
					break;
				// test
				case 0x0E: 
					fprintf(f, "test  (blocks:%08x)",m_bufferram[ptr+32]);
					ptr+=32;
					ptr+=m_bufferram[ptr]*3;
					break;
				// end code
				case 0x0F: 
					fprintf(f, "end");
					end_code = true;
					break;
				// dummy
				case 0x10: 
					fprintf(f, "dummy");
					ptr++;
					break;
				// log data
				case 0x14: 
					attr = m_bufferram[ptr+1];
					fprintf(f, "log  (start:%08x count:%08x)",m_bufferram[ptr],attr);
					ptr+=2+attr;
					break;
				// set lod mode
				case 0x16: 
					fprintf(f, "lod  (%f)",u2f(m_bufferram[ptr]));
					ptr++;
					break;
				// unknown opcode
				default:
					fprintf(f, "unk %02x",(opcode >> 23) & 0x1f);
					break;
			}
		}
		
		// append the raw opcode and new line here
		fprintf(f,"\t%08x\n",opcode);
	}
	
	fclose( f );
	con.printf("Data dumped successfully\n");
}

/*****************************************
 *
 * Sorted Triangles dumping
 *
 ****************************************/

void model2_state::debug_tri_dump_command(int ref, const std::vector<std::string> &params)
{
	debugger_console &con = machine().debugger().console();
	FILE *f;
	
	if (params.size() < 2)
	{
		con.printf("Error: not enough parameters for m2 trilist command\n");
		return;
	}
	
	if (params[1].empty() || params[1].length() > 127)
	{
		con.printf("Error: invalid filename parameter for m2 trilist command\n");
		return;
	}

	if((f = fopen( params[1].c_str(), "w" )) == nullptr)
	{
		con.printf("Error: while opening %s for writing\n",params[1].c_str());
		return;
	}

	tri_list_dump(f);
	con.printf("Data dumped successfully\n");	
}
