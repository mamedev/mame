// license:BSD-3-Clause
// copyright-holders:David Haywood
/* helper functions for BFM SC4/5 set identification and layout file creation
 - these are not strictly required by the emulation

 */

#include "emu.h"

//#define sc45helperlog printf
#define sc45helperlog machine.logerror

// addrxor used for endianness stuff, mode used for when we have a missing pair half
int find_project_string(running_machine &machine, int addrxor, int mode)
{
	// search for the title
	const int strlength = 14;
	char title_string[] = "PROJECT NUMBER";
	UINT8 *src = machine.root_device().memregion( "maincpu" )->base();
	int size = machine.root_device().memregion( "maincpu" )->bytes();

	int search_start = 0;
	int search_step = 1;

	if (mode==1)
	{
		search_start = 0;
		search_step = 2;
	}

	if (mode==2)
	{
		search_start = 1;
		search_step = 2;
	}

	for (int i=0;i<size-strlength;i++)
	{
	//  sc45helperlog("%02x", src[i]);

		int j;
		int found = 1;
		for (j=search_start;j<strlength;j+=search_step)
		{
			UINT8 rom = src[(i+j)^addrxor];
			UINT8 chr = title_string[j];

			if (rom != chr)
			{
				found = 0;
				break;
			}
		}

		if (found!=0)
		{
			int end=0;
			int count = 0;
			int blankcount = 0;
			sc45helperlog("ID String @ %08x\n", i);

			if (mode==2)
			{
				count = -1;
			}

			while (!end)
			{
				UINT8 rom;
				int addr;
				if (mode==0)
				{
					addr = (i+count)^addrxor;
					rom = src[addr];
				}
				else if (mode == 1)
				{
					addr = (i+count)^addrxor;

					if (addr&1)
						rom = src[addr];
					else
						rom = '_';
				}
				else
				{
					addr = (i+count)^addrxor;

					if (addr&1)
						rom = '_';
					else
						rom = src[addr];
				}


				//if (rom==0xff)
				//  end = 1;
				//else
				{
					if ((rom>=0x20) && (rom<0x7f))
					{
						sc45helperlog("%c", rom);
						blankcount = 0;
					}
					else
					{
						blankcount++;
						if (blankcount<10) sc45helperlog(" ");
					}
				}

				count++;

				if (count>=0x100)
					end = 1;
			}
			sc45helperlog("\n");

			return 1;
		}
	}

	return 0;
}

// find where the button definitions are in the ROM to make creating input ports easier for games using common test mode code
// not ALL games have a comprehensive list, but enough do to make this a worthwile debugging aid.

struct sc4inputinfo
{
	std::string name;
	bool used;
};

sc4inputinfo sc4inputs[32][16];

bool compare_input_code(running_machine &machine, int addr)
{
	UINT16 *src = (UINT16*)machine.root_device().memregion( "maincpu" )->base();
	UINT16* rom = &src[addr];


	if ((rom[0] != 0x48e7) || (rom[1] != 0x3020) || (rom[2] != 0x322f) || (rom[3] != 0x0010) || (rom[4] != 0x227c))
		return false;

	if ((rom[7] != 0x4242) || (rom[8] != 0x2449) || (rom[9] != 0x3639))
		return false;

	return true;
}

int find_input_strings(running_machine &machine)
{
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			sc4inputs[i][j].name = strformat("IN%d-%d", i, j);
			sc4inputs[i][j].used = false;
		}
	}


	int foundat = -1;
	UINT32 startblock = 0;
	UINT32 endblock = 0;

	UINT16 *rom = (UINT16*)machine.root_device().memregion( "maincpu" )->base();
	UINT8 *rom8 = machine.root_device().memregion( "maincpu" )->base();

	for (int i=0;i<(0x100000-0x40)/2;i++)
	{
		bool found = compare_input_code(machine, i);

		if (found==true)
		{
			startblock = (rom[i + 5] << 16) | rom[i + 6];
			endblock = (rom[i + 10] << 16) | rom[i + 11];
			sc45helperlog("------------ INPUTS -----------------\n");
			sc45helperlog("input strings found at %08x (start of ponter block %08x end of pointer block %08x\n", i*2, startblock, endblock);
			foundat = i;

			if (endblock > startblock)
			{
				for (int j = startblock / 2; j < endblock / 2; j+=4)
				{
					UINT16 portpos = rom[j + 0];
					int port = (portpos & 0x1f);
					int pos = (portpos >> 5);

					UINT16 unk2 = rom[j + 1];
					UINT32 stringaddr = (rom[j + 2] << 16) | rom[j + 3];

					sc45helperlog("(port %02x position %02x) unk %04x addr %08x  ", port,pos, unk2, stringaddr);
					std::string tempstring;

					for (int k = stringaddr; k < stringaddr + 6; k++)
					{
						UINT8 chr = rom8[k^1];

						if ((chr == 0xff) || (chr == 0x00))
						{
							k = stringaddr + 6;
						}
						else
						{
							tempstring.push_back(chr);
						}

					}

					strtrimspace(tempstring);
					strmakelower(tempstring);


					//if (pos <= 5)
					{
						assert(pos >= 0 && pos < ARRAY_LENGTH(sc4inputs[port]));
						if (sc4inputs[port][pos].used == false)
						{
							sc4inputs[port][pos].used = true;
							sc4inputs[port][pos].name = tempstring;
						}
						else
						{
							printf("position already used?\n");

							sc4inputs[port][pos].name.append(" OR ");
							sc4inputs[port][pos].name.append(tempstring);
						}
					}
					//else
					//{
					//  printf("invalid port position?\n");
					//}



					sc45helperlog("%s", tempstring.c_str());

					sc45helperlog("\n");

				}
			}

		}
	}

	sc45helperlog("------------ INPUT STRUCTURE -----------------\n");

	// i'm not sure exactly how many ports are valid for inputs, or if different
	// hardware can also be connected to them, buttons typically map in
	// ports 1,2, then jump to 8,9, and sometimes 10 (sc4dndys)
	// some games appear to have mistakes in their tables (sc4dndclc claims the
	// %age key maps over the dips?)
	// dips are on the motherboard and the 16 switches appear mapped at 16,17,18,19 split 5-5-5-1 (I'm assuming this can't move)
	// the green service mode button in port 20 is also on the motherboard (I'm assuming this can't move)
	//
	// sometimes an upper bit (0x100) is set for hopper related ports?

	int ignoreports[32][16] =
	{
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{  1,  2,  3,  4,  5,  6, -1, -1,    -7, -1, -1, -1, -1, -1, -1, -1, }, // port 1
		{  7,  8,  9, 10, 11, 12, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }, // port 2
		{ -1, -1, -2, -2, -2, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }, // many of the dutch games map inputs here instead of the stake key(!!)
		{ -1, -9, -1, -10,-8, -1, -1, -1,    -6, -1, -1, -1, -1, -1, -1, -1, }, // port 4
		{ -2, -2, -2, -2, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -2, -2, -2, -2, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ 31, 32, 33, 34, 35, 36, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }, // port 7 (sc4cfqpse?)
		{ 13, 14, 15, 16, 17, 18, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }, // port 8
		{ 19, 20, 21, 22, 23, 24, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }, // port 9
		{ 25, 26, 27, 28, 29, 30, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }, // port 10
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -3, -3, -3, -3, -3, -1, -1, -1,    -5, -1, -1, -1, -1, -1, -1, -1, }, // port 16
		{ -3, -3, -3, -3, -3, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }, // port 17
		{ -3, -3, -3, -3, -3, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }, // port 18
		{ -3, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }, // port 19
		{ -3, -2, -2, -2, -2, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }, // port 20
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, },
		{ -1, -1, -1, -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1, -1, -1, -1, }
	};

	int buttons_used = 1;

	printf("INPUT_PORTS_START( %s ) // this structure is generated\n", machine.system().name);
	printf("    PORT_INCLUDE( sc4_base )\n");

	for (int i = 0; i < 32; i++)
	{
		int thisportused = 0;

		for (int j = 0; j < 16; j++)
		{
			if (sc4inputs[i][j].used == true)
			{
				if (thisportused == 0)
				{
					printf("    PORT_MODIFY(\"IN-%d\")\n", i);
					thisportused = 1;
				}

				if (ignoreports[i][j] > 0)
				{
					printf("    PORT_BIT( 0x%04x, IP_ACTIVE_HIGH, SC45_BUTTON_MATRIX_%d_%d ) PORT_NAME(\"%s\")\n", 1 << j, i,j/*ignoreports[i][j]*/, sc4inputs[i][j].name.c_str());
					buttons_used++;
				}
				else if (ignoreports[i][j] == -3)
				{
					printf("    // 0x%04x - \"%s\" // standard input (motherboard)\n", 1 << j, sc4inputs[i][j].name.c_str());
				}
				else if (ignoreports[i][j] == -2)
				{
					printf("    // 0x%04x - \"%s\" // standard input (expected here)\n", 1 << j, sc4inputs[i][j].name.c_str());
				}
				else if (ignoreports[i][j] == -1)
				{
					printf("    // 0x%04x - \"%s\" // unexpected here\n", 1 << j, sc4inputs[i][j].name.c_str());
				}
				else if (ignoreports[i][j] == -4)
				{
					printf("    // 0x%04x - \"%s\" // known extended input, mapping not understood\n", 1 << j, sc4inputs[i][j].name.c_str());
				}
				else if (ignoreports[i][j] == -5)
				{
					printf("    // 0x%04x - \"%s\" // known extended input, usually 'top up'\n", 1 << j, sc4inputs[i][j].name.c_str());
				}
				else if (ignoreports[i][j] == -6)
				{
					printf("    // 0x%04x - \"%s\" // known extended input, usually 'hopper low'\n", 1 << j, sc4inputs[i][j].name.c_str());
				}
				else if (ignoreports[i][j] == -7)
				{
					printf("    // 0x%04x - \"%s\" // known extended input, usually 'hopper fit'\n", 1 << j, sc4inputs[i][j].name.c_str());
				}
				else if (ignoreports[i][j] == -8)
				{
					printf("    // 0x%04x - \"%s\" // known extended(?) input, sometimes 'top up'\n", 1 << j, sc4inputs[i][j].name.c_str());
				}
				else if (ignoreports[i][j] == -9)
				{
					printf("    // 0x%04x - \"%s\" // known extended(?) input, sometimes 'hop hi'\n", 1 << j, sc4inputs[i][j].name.c_str());
				}
				else if (ignoreports[i][j] == -10)
				{
					printf("    // 0x%04x - \"%s\" // known extended(?) input, sometimes 'hop top'\n", 1 << j, sc4inputs[i][j].name.c_str());
				}
				buttons_used++;
			}
		}
	}

	printf("INPUT_PORTS_END\n");

	return foundat;
}

struct lampinfo
{
	std::string lampname;
	std::string lampname_alt;
	bool used;
	int x, y;
	int width, height;
	bool draw_label;
	std::string lamptypename;
	int clickport;
	int clickmask;
};

lampinfo lamps[16][16];

void set_clickable_temp(running_machine &machine, const std::string &teststring, int clickport, int clickmask)
{
	for (auto & lamp : lamps)
	{
		for (int x = 0; x < 16; x++)
		{
			if (!strcmp(teststring.c_str(), lamp[x].lampname_alt.c_str()))
			{
				lamp[x].clickport = clickport;
				lamp[x].clickmask = clickmask;

				lamp[x].lamptypename = "buttonlamp";
			}

		}
	}
}

int find_lamp_strings(running_machine &machine)
{
	int startblock = -1;
	int endblock = -1;


	if (!strcmp(machine.system().name, "sc4dnd"))
	{
		// these are for sc4dnd ONLY, need to work out how the code calculates them
		startblock = 0x1cac0;
		endblock =   0x1cf9a;
	}
	else if (!strcmp(machine.system().name, "sc4dndtp"))
	{
		startblock = 0x2175c;
		endblock = 0x21cb4;
	}
	else if (!strcmp(machine.system().name, "sc4dnddw"))
	{
		startblock = 0x18a8e;
		endblock = 0x18fc2;
	}

	if (startblock == -1)
		return 0;



	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			char tmp[32];

			sprintf(tmp, "(%02d:%02d)", y, x);

			lamps[y][x].lampname = std::string(tmp);
			lamps[y][x].used = false;
			lamps[y][x].y = (y * 28);
			lamps[y][x].x = 380 + (x * 24);
			lamps[y][x].draw_label = true;
			lamps[y][x].width = 23;
			lamps[y][x].height = 16;
			lamps[y][x].lamptypename = "unusedlamp";
			lamps[y][x].clickport = -1;
			lamps[y][x].clickmask = 0;

		}
	}








	UINT16 *rom = (UINT16*)machine.root_device().memregion( "maincpu" )->base();
	UINT8 *rom8 = machine.root_device().memregion( "maincpu" )->base();

//  sc45helperlog("------------ LAMPS -----------------\n");

	if (endblock > startblock)
	{
		for (int j = startblock / 2; j < endblock / 2; j+=3)
		{
			UINT16 portpos = rom[j + 0];
			int row = portpos & 0xf;
			int col = (portpos >> 4 ) & 0xf;

			UINT32 stringaddr = (rom[j + 1] << 16) | rom[j + 2];

			//sc45helperlog("(row %02d, col %02d, unused %02x) addr %08x  ", row,col, (portpos&0xff00)>>8, stringaddr);

			std::string tempstring;

			for (int k = stringaddr; k < stringaddr + 10; k++)
			{
				UINT8 chr = rom8[k^1];

				if ((chr == 0xff) || (chr == 0x00))
				{
					k = stringaddr + 10;
				}
				else
				{
					tempstring.push_back(chr);
				}

			}

			if (lamps[row][col].used == false)
			{
				lamps[row][col].used = true;
				lamps[row][col].lampname = tempstring;
				lamps[row][col].lamptypename = "matrixlamp";

				strtrimspace(lamps[row][col].lampname);
				strmakelower(lamps[row][col].lampname);
			}
			else
			{
				fatalerror("duplicate lamp?\n");
			}

			//sc45helperlog("%s", tempstring.c_str());

			//sc45helperlog("\n");

		}
	}

	// layout elements for each of the text labels
	int d = 0;
	for (auto & lamp : lamps)
	{
		//sc45helperlog("---ROW %02d---\n", y);
		for (int x = 0; x < 16; x++)
		{
			sc45helperlog("<element name=\"lamplabelel%d\"><text string=\"%s\"><color red=\"1.0\" green=\"1.0\" blue=\"1.0\" /></text></element>\n", d, lamp[x].lampname.c_str());
			d++;
		}
	}

	sc45helperlog("\n\n");
	// print out some input text labels for specific rows
	d = 0;
	for (int y = 0; y < 7; y++)
	{
		int actualrows[] = { 1, 2, 7, 8, 9, 10, 20 };
		int realy = actualrows[y];

		for (int x = 0; x < 6; x++)
		{
			sc45helperlog("<element name=\"inputlabel%d-%d\"><text string=\"%s\"><color red=\"1.0\" green=\"1.0\" blue=\"1.0\" /></text></element>\n", realy,x,sc4inputs[realy][x].name.c_str());

		}
	}
	sc45helperlog("\n");




	// some stuff needed by the matrix layout
	sc45helperlog("<element name=\"matrixlamp\">\n");
	sc45helperlog("<rect state =\"0\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.7\" green=\"0.7\" blue=\"0.7\" /></rect>\n");
	sc45helperlog("<rect state =\"1\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.0\" green=\"0.0\" blue=\"1.0\" /></rect>\n");
	sc45helperlog("<rect state =\"2\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.0\" green=\"1.0\" blue=\"0.0\" /></rect>\n");
	sc45helperlog("</element>\n");
	sc45helperlog("<element name=\"unusedlamp\">\n");
	sc45helperlog("<rect state =\"0\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.2\" green=\"0.2\" blue=\"0.2\" /></rect>\n");
	sc45helperlog("<rect state =\"1\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.0\" green=\"0.0\" blue=\"0.5\" /></rect>\n");
	sc45helperlog("<rect state =\"2\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.0\" green=\"0.5\" blue=\"0.0\" /></rect>\n");
	sc45helperlog("</element>\n");
	sc45helperlog("<element name=\"reellamp\">\n");
	sc45helperlog("<rect state =\"0\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.1\" green=\"0.1\" blue=\"0.1\" /></rect>\n");
	sc45helperlog("<rect state =\"1\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.6\" green=\"0.6\" blue=\"0.6\" /></rect>\n");
	sc45helperlog("<rect state =\"2\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.6\" green=\"0.6\" blue=\"0.6\" /></rect>\n");
	sc45helperlog("</element>\n");
	sc45helperlog("<element name=\"buttonlamp\">\n");
	sc45helperlog("<rect state =\"0\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.6\" green=\"0.7\" blue=\"0.6\" /></rect>\n");
	sc45helperlog("<rect state =\"1\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.0\" green=\"1.0\" blue=\"0.0\" /></rect>\n");
	sc45helperlog("<rect state =\"2\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.0\" green=\"0.0\" blue=\"1.0\" /></rect>\n");
	sc45helperlog("</element>\n");
	sc45helperlog("<element name=\"button\">\n");
	sc45helperlog("<rect state =\"0\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.2\" green=\"0.8\" blue=\"0.2\" /></rect>\n");
	sc45helperlog("<rect state =\"1\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.2\" green=\"0.9\" blue=\"0.2\" /></rect>\n");
	sc45helperlog("<rect state =\"2\"><bounds x=\"0\" y=\"0\" width=\"7\" height=\"7\" /><color red=\"0.2\" green=\"1.0\" blue=\"0.2\" /></rect>\n");
	sc45helperlog("</element>\n");
	sc45helperlog("<element name=\"vfd0\">\n");
	sc45helperlog("<led14segsc><color red=\"0\" green=\"0.6\" blue=\"1.0\" /></led14segsc>\n");
	sc45helperlog("</element>\n");
	sc45helperlog("<element name=\"digit\" defstate=\"10\">\n");
	sc45helperlog("<led7seg><color red=\"1.0\" green=\"0.3\" blue=\"0.0\" /></led7seg>\n");
	sc45helperlog("</element>\n");
	sc45helperlog("<element name=\"Steppers\" defstate=\"0\">\n");
	sc45helperlog("<simplecounter maxstate=\"999\" digits=\"3\"><color red=\"1.0\" green=\"1.0\" blue=\"1.0\" /><bounds x=\"0\" y=\"0\" width=\"1\" height=\"1\" /></simplecounter>\n");
	sc45helperlog("</element>\n");
	sc45helperlog("\n\n");


	sc45helperlog("<view name=\"AWP Simulated Video (No Artwork)\">\n");

	// copy the button strings so we can modify some for easier comparisons
	d = 0;
	for (auto & lamp : lamps)
	{
		for (int x = 0; x < 16; x++)
		{
			lamp[x].lampname_alt = lamp[x].lampname;

			if (!strcmp(lamp[x].lampname_alt.c_str(), "hold2/hi")) lamp[x].lampname_alt = "hold2";
			if (!strcmp(lamp[x].lampname_alt.c_str(), "hold3/lo")) lamp[x].lampname_alt = "hold3";
			if (!strcmp(lamp[x].lampname_alt.c_str(), "chg stake")) lamp[x].lampname_alt = "chnge stk";
			if (!strcmp(lamp[x].lampname_alt.c_str(), "canc/coll")) lamp[x].lampname_alt = "cancel";
			if (!strcmp(lamp[x].lampname_alt.c_str(), "start")) lamp[x].lampname_alt = "strt exch";

		}
	}


	// try to find some specific named elements and move them around
	d = 0;

	for (int reel = 0; reel < 8; reel++)
	{
		char tempname[32];
		sprintf(tempname, "reel%d ", reel+1);


		for (int pos = 0; pos < 3; pos++)
		{
			char tempname2[32];

			if (pos == 0) sprintf(tempname2, "%stop", tempname);
			if (pos == 1) sprintf(tempname2, "%smid", tempname);
			if (pos == 2) sprintf(tempname2, "%sbot", tempname);


			for (auto & lamp : lamps)
			{
				for (int x = 0; x < 16; x++)
				{
					if (!strcmp(tempname2, lamp[x].lampname_alt.c_str()))
					{
						//sc45helperlog("%s found\n", tempname2);
						lamp[x].draw_label = false;

						lamp[x].x = 0 + (50 * reel);
						lamp[x].y = 300 + (17 * pos);
						lamp[x].width = 50;
						lamp[x].height = 17;
						lamp[x].lamptypename = "reellamp";


					}
					else
					{
						//printf("%s:%s:\n", tempname2, lamps[y][x].lampname_alt.c_str());
					}

				}
			}
		}
	}

	// todo, find these by cross-referencing button names and lamp names instead
	{
		set_clickable_temp(machine, "cancel",    1, 0x01);
		set_clickable_temp(machine, "hold1",     1, 0x02);
		set_clickable_temp(machine, "hold2",     1, 0x04);
		set_clickable_temp(machine, "hold3",     1, 0x08);
		set_clickable_temp(machine, "hold4",     1, 0x10);

		set_clickable_temp(machine, "chnge stk", 2, 0x01);
		set_clickable_temp(machine, "collect",   2, 0x02);
		set_clickable_temp(machine, "transfer",  2, 0x04);

		set_clickable_temp(machine, "strt exch", 2, 0x10);

		set_clickable_temp(machine, "nodeal",   8, 0x01);
		set_clickable_temp(machine, "deal",      8, 0x02);
		set_clickable_temp(machine, "cash bust", 8, 0x04);

		// no 'refill' lamp?


	}


	// dump out basic matrix stuff in layout format
	d = 0;
	for (auto & lamp : lamps)
	{
		//sc45helperlog("---ROW %02d---\n", y);
		for (int x = 0; x < 16; x++)
		{
			if (lamp[x].clickport== -1) sc45helperlog("<bezel name=\"lamp%d\" element=\"%s\" state=\"0\"><bounds x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"/></bezel>\n", d, lamp[x].lamptypename.c_str(), lamp[x].x, lamp[x].y, lamp[x].width, lamp[x].height);
			else sc45helperlog("<bezel name=\"lamp%d\" element=\"%s\" state=\"0\" inputtag=\"IN-%d\" inputmask=\"0x%02x\"><bounds x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" /></bezel>\n", d, lamp[x].lamptypename.c_str(), lamp[x].clickport, lamp[x].clickmask, lamp[x].x, lamp[x].y, lamp[x].width, lamp[x].height);

			if (lamp[x].draw_label == false) sc45helperlog("<!-- Label not drawn\n");
			sc45helperlog("<bezel name=\"lamplabel%d\" element=\"lamplabelel%d\"><bounds x=\"%d\" y=\"%d\" width=\"%d\" height=\"10\" /></bezel>\n", d,d, lamp[x].x, lamp[x].y-10, lamp[x].width);
			if (lamp[x].draw_label == false) sc45helperlog("-->\n");

			d++;
		}
	}

	// print out a simple matrix of some of the most common inputs,
	d = 0;
	for (int y = 0; y < 7; y++)
	{
		int actualrows[] = { 1, 2, 7, 8, 9, 10, 20 };
		int realy = actualrows[y];

		for (int x = 0; x < 6; x++)
		{
			sc45helperlog("<bezel name=\"input%d-%d\" element=\"button\" state=\"0\" inputtag=\"IN-%d\" inputmask=\"0x%02x\"><bounds x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" /></bezel>\n", realy,x, realy, 1<<x, 180+x*20, 0+y*20, 19, 10);

			sc45helperlog("<bezel name=\"inputlab%d-%d\" element=\"inputlabel%d-%d\"><bounds x=\"%d\" y=\"%d\" width=\"%d\" height=\"10\" /></bezel>\n", realy,x,realy,x, 180+x*20,  0+(y*20)-10, 19);

			d++;
		}
	}

	sc45helperlog("\n");

	// other layout debug stuff
	d = 0;
	int extrax = 0;
	for (int x = 0; x < 32; x++)
	{
		if (x % 5 == 0) extrax += 4;

		for (int y = 0; y < 8; y++)
		{
			sc45helperlog("<bezel name=\"matrix%d\" element=\"matrixlamp\" state=\"0\"><bounds x=\"%d\" y=\"%d\" width=\"3\" height=\"3\" /></bezel>\n", d, (x*4)+extrax, 40+y*4);
			d++;
		}
	}

	sc45helperlog("\n");

	d = 0;
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			sc45helperlog("<bezel name=\"digit%d\" element=\"digit\" state=\"0\"><bounds x=\"%d\" y=\"%d\" width=\"9\" height=\"19\"/></bezel>\n", d, (15*10)- (x * 10), 80+(y * 20));
			d++;
		}
	}

	sc45helperlog("\n");

	d = 0;
	for (int x = 0; x < 16; x++)
	{
		int y = 10;

		sc45helperlog("<bezel name=\"vfd%d\" element=\"vfd0\" state=\"0\"><bounds x=\"%d\" y=\"%d\" width=\"9\" height=\"17\"/></bezel>\n", d, 0 + (x * 10), y);
		d++;
	}

	{
		// write reel stuff to layouts
		// sc4dnd specific..
		int num_normal_reels = 4;
		int num_special_reels = 2;
		int current_reel = 0;

		for (int i = 0; i < num_normal_reels + num_special_reels; i++)
		{
			int x = i * 50;
			int y = 300;
			sc45helperlog("<bezel name=\"reel%d\" element=\"Steppers\" state=\"0\"><bounds x=\"%d\" y=\"%d\" width=\"10\" height=\"5\" /></bezel>\n", current_reel+1, x+40, y+50);
			sc45helperlog("<bezel name=\"sreel%d\" element=\"SteppersReel%d\" state=\"0\"><bounds x=\"%d\" y=\"%d\" width=\"50\" height=\"50\" /></bezel>\n",  current_reel+1, current_reel+1, x, y);

			current_reel++;

		}


	}



	sc45helperlog("</view>\n");
	sc45helperlog("</mamelayout>\n");


	return 0;
}


int find_reel_strings(running_machine &machine)
{
	int startblock = -1;
	int endblock = -1;

	std::vector<int> reelsizes;

	// these are for sc4dnd ONLY, need to work out how the code calculates them

	// this list is 4 * 16 symbols for the regular reels, 12 symbols for the number spin, and 2 groups of 16 depending on jackpot/stake keys used for the prize reel
	// code that points at these is likely to be complex because it's conditional on the game code / mode..
	if (!strcmp(machine.system().name, "sc4dnd"))
	{
		reelsizes.push_back(16);
		reelsizes.push_back(16);
		reelsizes.push_back(16);
		reelsizes.push_back(16);
		reelsizes.push_back(12);
		reelsizes.push_back(16);
		reelsizes.push_back(16);

		startblock = 0x8d74c;
	}
	else if (!strcmp(machine.system().name, "sc4dndtp"))
	{
		reelsizes.push_back(16);
		reelsizes.push_back(16);
		reelsizes.push_back(16);
		reelsizes.push_back(12);
		reelsizes.push_back(16);
		reelsizes.push_back(16);
		reelsizes.push_back(16);

		startblock = 0x9d252;
	}
	else if (!strcmp(machine.system().name, "sc4dnddw"))
	{
		reelsizes.push_back(16);
		reelsizes.push_back(16);
		reelsizes.push_back(16);
		reelsizes.push_back(12);
		reelsizes.push_back(20);
		reelsizes.push_back(20);
		reelsizes.push_back(20);

		startblock = 0x9b8c8;
	}


	int total_reel_symbols = 0;

	for (auto & reelsize : reelsizes)
	{
		total_reel_symbols += reelsize;
	}

	endblock =   startblock + 4 * (total_reel_symbols);



	if (startblock == -1)
		return 0;





	UINT16 *rom = (UINT16*)machine.root_device().memregion( "maincpu" )->base();
	UINT8 *rom8 = machine.root_device().memregion( "maincpu" )->base();



	sc45helperlog("------------ LAYOUT -----------------\n");

	sc45helperlog("<?xml version=\"1.0\"?>\n");
	sc45helperlog("<mamelayout version=\"2\">\n");

	if (endblock > startblock)
	{
		int which_reel = 0;
		int current_symbols = 0;



		for (int j = startblock / 2; j < endblock / 2; j+=2)
		{
			if (current_symbols == 0)
			{
				int shifted = ((0x10000 / 16) * 11 ) + 692;
			//  sc45helperlog("REEL %d\n", which_reel+1);
				sc45helperlog("<element name=\"SteppersReel%d\" defstate=\"0\">\n", which_reel+1);
				sc45helperlog("<reel stateoffset=\"%d\" symbollist=\"", shifted);
			}

			UINT32 stringaddr = (rom[j + 0] << 16) | rom[j + 1];
			stringaddr &= 0xfffff;

			if (stringaddr >= (0x10000 - 16))
				continue;

			//sc45helperlog("addr %08x  ", stringaddr);

			std::string tempstring;

			for (int k = stringaddr; k < stringaddr + 10; k++)
			{
				UINT8 chr = rom8[k^1];

				if ((chr == 0xff) || (chr == 0x00))
				{
					k = stringaddr + 10;
				}
				else
				{
					tempstring.push_back(chr);
				}

			}

			strtrimspace(tempstring);
			strmakelower(tempstring);

			if (tempstring[0] == '!')
			{
				strdelchr(tempstring,'!');
				tempstring.append("PND");
			}

			sc45helperlog("%s", tempstring.c_str());



			//sc45helperlog("\n");

			current_symbols++;
			if (current_symbols == reelsizes[which_reel])
			{
				sc45helperlog("\"><color red=\"1.0\" green=\"1.0\" blue=\"1.0\" /><bounds x=\"0\" y=\"0\" width=\"1\" height=\"1\" /></reel>\n");
				sc45helperlog("</element>\n");

				current_symbols = 0;
				which_reel++;
			}
			else
			{
				sc45helperlog(",");
			}


		}
	}

	return 0;
}


void bfm_sc45_layout_helper(running_machine &machine)
{
	find_input_strings(machine);
	find_reel_strings(machine);
	find_lamp_strings(machine);
}
