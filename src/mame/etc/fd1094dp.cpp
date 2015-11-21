// license:BSD-3-Clause
// copyright-holders:Charles MacDonald, Nicola Salmoria
/*****************************************************************************

FD1094 key dumper

This file is not part of MAME, it is a standalone program that reads 128MB of
data extracted from the FD1094 CPU and produces a 8KB keyfile, used by
fd1094.c to decrypt the ROM data.

*****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int state2;
char srcpath[250],dstfilename[250];

#define BIT(x,n) (((x)>>(n))&1)

#define BITSWAP16(val,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
		((BIT(val,B15) << 15) | \
			(BIT(val,B14) << 14) | \
			(BIT(val,B13) << 13) | \
			(BIT(val,B12) << 12) | \
			(BIT(val,B11) << 11) | \
			(BIT(val,B10) << 10) | \
			(BIT(val, B9) <<  9) | \
			(BIT(val, B8) <<  8) | \
			(BIT(val, B7) <<  7) | \
			(BIT(val, B6) <<  6) | \
			(BIT(val, B5) <<  5) | \
			(BIT(val, B4) <<  4) | \
			(BIT(val, B3) <<  3) | \
			(BIT(val, B2) <<  2) | \
			(BIT(val, B1) <<  1) | \
			(BIT(val, B0) <<  0))

UINT16 buffer[0x2000];
int little_endian;

int is_little_endian(void)
{
	short magic, test;
	char *ptr;

	magic = 0xABCD;
	ptr = (char *)&magic;
	test = (ptr[1]<<8) + (ptr[0]&0xFF);
	return (magic == test);
}



void load_table(int n)
{
	static int last_loaded = -1;

	if (last_loaded != n)
	{
		FILE *f1;
		int i;
		char name[250];


		last_loaded = n;

		sprintf(name,"%s/00%04x.tbl",srcpath,n);

		f1 = fopen(name,"rb");
		if (!f1)
		{
			printf("Can't open %s for reading.\n",name);
			exit(1);
		}

		/* read repeatedly so we support both full dumps and reduced E000-FFFF dumps */
		while (fread(buffer,1,0x4000,f1) == 0x4000) ;

		fclose(f1);

		if (little_endian)
		{
			for (i = 0;i < 0x2000;i++)
				buffer[i] = (buffer[i] >> 8) | (buffer[i] << 8);
		}
	}
}


/*
317-0162 CPU also needs to mask:
0x107a,
0x127a,
0x147a,
0x167a,
0x187a,
0x1a7a,
0x1c7a,
0x1e7a,
this only happens with 317-0162 so far; I assume it is a fault in the CPU.
*/
int masked_opcodes[] =
{
	0x013a,0x033a,0x053a,0x073a,0x083a,0x093a,0x0b3a,0x0d3a,0x0f3a,

	0x103a,       0x10ba,0x10fa,    0x113a,0x117a,0x11ba,0x11fa,
	0x123a,       0x12ba,0x12fa,    0x133a,0x137a,0x13ba,0x13fa,
	0x143a,       0x14ba,0x14fa,    0x153a,0x157a,0x15ba,
	0x163a,       0x16ba,0x16fa,    0x173a,0x177a,0x17ba,
	0x183a,       0x18ba,0x18fa,    0x193a,0x197a,0x19ba,
	0x1a3a,       0x1aba,0x1afa,    0x1b3a,0x1b7a,0x1bba,
	0x1c3a,       0x1cba,0x1cfa,    0x1d3a,0x1d7a,0x1dba,
	0x1e3a,       0x1eba,0x1efa,    0x1f3a,0x1f7a,0x1fba,

	0x203a,0x207a,0x20ba,0x20fa,    0x213a,0x217a,0x21ba,0x21fa,
	0x223a,0x227a,0x22ba,0x22fa,    0x233a,0x237a,0x23ba,0x23fa,
	0x243a,0x247a,0x24ba,0x24fa,    0x253a,0x257a,0x25ba,
	0x263a,0x267a,0x26ba,0x26fa,    0x273a,0x277a,0x27ba,
	0x283a,0x287a,0x28ba,0x28fa,    0x293a,0x297a,0x29ba,
	0x2a3a,0x2a7a,0x2aba,0x2afa,    0x2b3a,0x2b7a,0x2bba,
	0x2c3a,0x2c7a,0x2cba,0x2cfa,    0x2d3a,0x2d7a,0x2dba,
	0x2e3a,0x2e7a,0x2eba,0x2efa,    0x2f3a,0x2f7a,0x2fba,

	0x303a,0x307a,0x30ba,0x30fa,    0x313a,0x317a,0x31ba,0x31fa,
	0x323a,0x327a,0x32ba,0x32fa,    0x333a,0x337a,0x33ba,0x33fa,
	0x343a,0x347a,0x34ba,0x34fa,    0x353a,0x357a,0x35ba,
	0x363a,0x367a,0x36ba,0x36fa,    0x373a,0x377a,0x37ba,
	0x383a,0x387a,0x38ba,0x38fa,    0x393a,0x397a,0x39ba,
	0x3a3a,0x3a7a,0x3aba,0x3afa,    0x3b3a,0x3b7a,0x3bba,
	0x3c3a,0x3c7a,0x3cba,0x3cfa,    0x3d3a,0x3d7a,0x3dba,
	0x3e3a,0x3e7a,0x3eba,0x3efa,    0x3f3a,0x3f7a,0x3fba,

	0x41ba,0x43ba,0x44fa,0x45ba,0x46fa,0x47ba,0x49ba,0x4bba,0x4cba,0x4cfa,0x4dba,0x4fba,

	0x803a,0x807a,0x80ba,0x80fa,    0x81fa,
	0x823a,0x827a,0x82ba,0x82fa,    0x83fa,
	0x843a,0x847a,0x84ba,0x84fa,    0x85fa,
	0x863a,0x867a,0x86ba,0x86fa,    0x87fa,
	0x883a,0x887a,0x88ba,0x88fa,    0x89fa,
	0x8a3a,0x8a7a,0x8aba,0x8afa,    0x8bfa,
	0x8c3a,0x8c7a,0x8cba,0x8cfa,    0x8dfa,
	0x8e3a,0x8e7a,0x8eba,0x8efa,    0x8ffa,

	0x903a,0x907a,0x90ba,0x90fa,    0x91fa,
	0x923a,0x927a,0x92ba,0x92fa,    0x93fa,
	0x943a,0x947a,0x94ba,0x94fa,    0x95fa,
	0x963a,0x967a,0x96ba,0x96fa,    0x97fa,
	0x983a,0x987a,0x98ba,0x98fa,    0x99fa,
	0x9a3a,0x9a7a,0x9aba,0x9afa,    0x9bfa,
	0x9c3a,0x9c7a,0x9cba,0x9cfa,    0x9dfa,
	0x9e3a,0x9e7a,0x9eba,0x9efa,    0x9ffa,

	0xb03a,0xb07a,0xb0ba,0xb0fa,    0xb1fa,
	0xb23a,0xb27a,0xb2ba,0xb2fa,    0xb3fa,
	0xb43a,0xb47a,0xb4ba,0xb4fa,    0xb5fa,
	0xb63a,0xb67a,0xb6ba,0xb6fa,    0xb7fa,
	0xb83a,0xb87a,0xb8ba,0xb8fa,    0xb9fa,
	0xba3a,0xba7a,0xbaba,0xbafa,    0xbbfa,
	0xbc3a,0xbc7a,0xbcba,0xbcfa,    0xbdfa,
	0xbe3a,0xbe7a,0xbeba,0xbefa,    0xbffa,

	0xc03a,0xc07a,0xc0ba,0xc0fa,    0xc1fa,
	0xc23a,0xc27a,0xc2ba,0xc2fa,    0xc3fa,
	0xc43a,0xc47a,0xc4ba,0xc4fa,    0xc5fa,
	0xc63a,0xc67a,0xc6ba,0xc6fa,    0xc7fa,
	0xc83a,0xc87a,0xc8ba,0xc8fa,    0xc9fa,
	0xca3a,0xca7a,0xcaba,0xcafa,    0xcbfa,
	0xcc3a,0xcc7a,0xccba,0xccfa,    0xcdfa,
	0xce3a,0xce7a,0xceba,0xcefa,    0xcffa,

	0xd03a,0xd07a,0xd0ba,0xd0fa,    0xd1fa,
	0xd23a,0xd27a,0xd2ba,0xd2fa,    0xd3fa,
	0xd43a,0xd47a,0xd4ba,0xd4fa,    0xd5fa,
	0xd63a,0xd67a,0xd6ba,0xd6fa,    0xd7fa,
	0xd83a,0xd87a,0xd8ba,0xd8fa,    0xd9fa,
	0xda3a,0xda7a,0xdaba,0xdafa,    0xdbfa,
	0xdc3a,0xdc7a,0xdcba,0xdcfa,    0xddfa,
	0xde3a,0xde7a,0xdeba,0xdefa,    0xdffa
};


int final_decrypt(int i,int moreffff)
{
	int j;

	/* final "obfuscation": invert bits 7 and 14 following a fixed pattern */
	int dec = i;
	if ((i & 0xf080) == 0x8000) dec ^= 0x0080;
	if ((i & 0xf080) == 0xc080) dec ^= 0x0080;
	if ((i & 0xb080) == 0x8000) dec ^= 0x4000;
	if ((i & 0xb100) == 0x0000) dec ^= 0x4000;

	/* mask out opcodes doing PC-relative addressing, replace them with FFFF */
	for (j = 0;j < sizeof(masked_opcodes)/sizeof(masked_opcodes[0]);j++)
	{
		if ((dec & 0xfffe) == masked_opcodes[j])
		{
			dec = 0xffff;
			break;
		}
	}

	/* optionally, even more values can be replaced with FFFF */
	if (moreffff)
	{
		if ((dec & 0xff80) == 0x4e80)
			dec = 0xffff;
		if ((dec & 0xf0f8) == 0x50c8)
			dec = 0xffff;
		if ((dec & 0xf000) == 0x6000)
			dec = 0xffff;
	}

	return dec;
}


static int parametric_decode(int val,int mainkey,int key_F,int gkey1,int gkey2,int gkey3)
{
	int key_6a,key_7a,key_6b;
	int key_0a,key_0b,key_0c;
	int key_1a,key_1b,key_2a,key_2b,key_3a,key_3b,key_4a,key_4b,key_5a,key_5b;
	int global_xor0,global_xor1;
	int global_swap0a,global_swap1,global_swap2,global_swap3,global_swap4;
	int global_swap0b;

	global_xor0         = 1^BIT(gkey1,5);   // could be bit 7
	global_xor1         = 1^BIT(gkey1,2);
	global_swap2        = 1^BIT(gkey1,0);

	global_swap0a       = 1^BIT(gkey2,5);
	global_swap0b       = 1^BIT(gkey2,2);

	global_swap3        = 1^BIT(gkey3,6);
	global_swap1        = 1^BIT(gkey3,4);
	global_swap4        = 1^BIT(gkey3,2);

	key_0a = BIT(mainkey,0) ^ BIT(gkey3,1);
	key_0b = BIT(mainkey,0) ^ BIT(gkey1,7); // could be bit 5
	key_0c = BIT(mainkey,0) ^ BIT(gkey1,1);

	key_1a = BIT(mainkey,1) ^ BIT(gkey2,7);
	key_1b = BIT(mainkey,1) ^ BIT(gkey1,3);

	key_2a = BIT(mainkey,2) ^ BIT(gkey3,7); // could be bit 5
	key_2b = BIT(mainkey,2) ^ BIT(gkey1,4);

	key_3a = BIT(mainkey,3) ^ BIT(gkey2,0);
	key_3b = BIT(mainkey,3) ^ BIT(gkey3,3);

	key_4a = BIT(mainkey,4) ^ BIT(gkey2,3);
	key_4b = BIT(mainkey,4) ^ BIT(gkey3,0);

	key_5a = BIT(mainkey,5) ^ BIT(gkey3,5); // could be bit 7
	key_5b = BIT(mainkey,5) ^ BIT(gkey1,6);

	key_6a = BIT(mainkey,6) ^ BIT(gkey2,1);
	key_6b = BIT(mainkey,6) ^ BIT(gkey2,6);

	key_7a = BIT(mainkey,7) ^ BIT(gkey2,4);


	if ((val & 0xe000) == 0x0000)
		val = BITSWAP16(val, 12,15,14,13,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
	else
	{
		if (val & 0x8000)
		{
			if (!global_xor1)   if (~val & 0x0008)  val ^= 0x2410;                                      // 13,10,4
								if (~val & 0x0004)  val ^= 0x0022;                                      // 5,1
			if (!key_1b)        if (~val & 0x1000)  val ^= 0x0848;                                      // 11,6,3
			if (!global_swap2)  if (!key_0c)        val ^= 0x4101;                                      // 14,8,0
			if (!key_2b)        val = BITSWAP16(val, 15,14,13, 9,11,10,12, 8, 2, 6, 5, 4, 3, 7, 1, 0);  // 12,9,7,2

			val = 0x6561 ^ BITSWAP16(val, 15, 9,10,13, 3,12, 0,14, 6, 5, 2,11, 8, 1, 4, 7);
		}
		if (val & 0x4000)
		{
			if (!global_xor0)   if (val & 0x0800)   val ^= 0x9048;                                      // 15,12,6,3
			if (!key_3a)        if (val & 0x0004)   val ^= 0x0202;                                      // 9,1
			if (!key_6a)        if (val & 0x0400)   val ^= 0x0004;                                      // 2
			if (!key_5b)        if (!key_0b)        val ^= 0x08a1;                                      // 11,7,5,0
			if (!global_swap0b) val = BITSWAP16(val, 15,14,10,12,11,13, 9, 4, 7, 6, 5, 8, 3, 2, 1, 0);  // 13,10,8,4

			val = 0x3523 ^ BITSWAP16(val, 13,14, 7, 0, 8, 6, 4, 2, 1,15, 3,11,12,10, 5, 9);
		}
		if (val & 0x2000)
		{
			if (!key_4a)        if (val & 0x0100)   val ^= 0x4210;                                      // 14,9,4
			if (!key_1a)        if (val & 0x0040)   val ^= 0x0080;                                      // 7
			if (!key_7a)        if (val & 0x0001)   val ^= 0x110a;                                      // 12,8,3,1
			if (!key_4b)        if (!key_0a)        val ^= 0x0040;                                      // 6
			if (!global_swap0a) if (!key_6b)        val ^= 0x0404;                                      // 10,2
			if (!key_5b)        val = BITSWAP16(val,  0,14,13,12,15,10, 9, 8, 7, 6,11, 4, 3, 2, 1, 5);  // 15,11,5,0

			val = 0x99a5 ^ BITSWAP16(val, 10, 2,13, 7, 8, 0, 3,14, 6,15, 1,11, 9, 4, 5,12);
		}

		val = 0x87ff ^ BITSWAP16(val,  5,15,13,14, 6, 0, 9,10, 4,11, 1, 2,12, 3, 7, 8);

		if (!global_swap4)  val = BITSWAP16(val,  6,14,13,12,11,10, 9, 5, 7,15, 8, 4, 3, 2, 1, 0);  // 15-6, 8-5
		if (!global_swap3)  val = BITSWAP16(val, 15,12,14,13,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);  // 12-13-14
		if (!global_swap2)  val = BITSWAP16(val, 15,14,13,12,11, 2, 9, 8,10, 6, 5, 4, 3, 0, 1, 7);  // 10-2-0-7
		if (!key_3b)        val = BITSWAP16(val, 15,14,13,12,11,10, 4, 8, 7, 6, 5, 9, 1, 2, 3, 0);  // 9-4, 3-1

		if (!key_2a)        val = BITSWAP16(val, 15,12,13,14,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);  // 14-12
		if (!global_swap1)  val = BITSWAP16(val, 15,14,13,12, 9, 8,11,10, 7, 6, 5, 4, 3, 2, 1, 0);  // 11...8
		if (!key_5a)        val = BITSWAP16(val, 15,14,13,12,11,10, 9, 8, 4, 5, 7, 6, 3, 2, 1, 0);  // 7...4
		if (!global_swap0a) val = BITSWAP16(val, 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 0, 3, 2, 1);  // 3...0
	}

	return final_decrypt(val,key_F);
}



int check_key(int key,int key_F,int gkey1,int gkey2,int gkey3)
{
	int i;
	int err_ffff = 0;

	if (state2)
	{
		gkey1 ^= 0x01;
		gkey2 ^= 0x10;
		gkey3 ^= 0x01;
	}

	for (i = 0xffff;i >= 0xe000;i--)
	{
		if (parametric_decode(i,key,key_F,gkey1,gkey2,gkey3) != buffer[i - 0xe000])
		{
			if (buffer[i - 0xe000] == 0xffff)
			{
				err_ffff++;
				if (err_ffff > 8) break;
			}
			else
				break;
		}
	}

	if (i < 0xe000)
	{
		/* special handling for CPU 317-0162 */
		if (err_ffff)
		{
			printf("warning: %d FFFF errors: ",err_ffff);
			for (i = 0xe000;i < 0x10000;i++)
			{
				if (parametric_decode(i,key,key_F,gkey1,gkey2,gkey3) != buffer[i - 0xe000])
					printf("%04x ",parametric_decode(i,key,key_F,gkey1,gkey2,gkey3));
			}
			printf("\n");
		}
		return 1;
	}
	else
		return 0;
}


static void print_binary(int num)
{
	int i;

	for (i = 7;i >= 0;i--)
		printf("%d",BIT(num,i));
}


static int global_key1,global_key2,global_key3;


void find_global_key(void)
{
	int gkey1,gkey2,gkey3;
	int found = 0;

	printf("Searching for global key...\n");

	for (gkey1 = 0;gkey1 < 0x100;gkey1++)
	{
		fprintf(stderr,"%02x/ff\r",gkey1);
		for (gkey2 = 0;gkey2 < 0x100;gkey2++)
		{
			for (gkey3 = 0;gkey3 < 0x100;gkey3++)
			{
				load_table(2);
				if (check_key(gkey1,BIT(gkey1,6),gkey1,gkey2,gkey3))
				{
					load_table(4);
					if (check_key(gkey2,BIT(gkey2,6),gkey1,gkey2,gkey3))
					{
						load_table(6);
						if (check_key(gkey3,BIT(gkey3,6),gkey1,gkey2,gkey3))
						{
							global_key1 = gkey1;
							global_key2 = gkey2;
							global_key3 = gkey3;

							printf("global key: ");
							print_binary(gkey1);
							printf(" ");
							print_binary(gkey2);
							printf(" ");
							print_binary(gkey3);
							printf("\n");

							found++;
						}
					}
				}
			}
		}
	}

	if (found > 1)
	{
		printf("The tables from state %02x don't allow to uniquely determine the global key.\n",state2*2);
		printf("You should dump tables from state %02x and run the program again.\n",(state2^1)*2);
		exit(0);
	}

	if (found == 0)
	{
		int key1,key2,key3;

		printf("Couldn't find the global key. This might be caused by:\n");
		printf("1) Incorrectly dumped data\n");
		printf("2) Wrong <state> parameter. 00 instead of 02, or 02 instead of 00.\n");
		printf("3) An error in the current FD1094 emulation.\n\n");

		printf("I will now execute a broader key search, which will take a longer time.\n");
		printf("If 1) and 2) are correct, and this search succeeds, contact MAMEDEV.\n");

		for (gkey1 = 0;gkey1 < 0x100;gkey1++)
		{
			fprintf(stderr,"%02x/ff\r",gkey1);
			for (gkey2 = 0;gkey2 < 0x100;gkey2++)
			{
				for (gkey3 = 0;gkey3 < 0x100;gkey3++)
				{
					load_table(2);
					for (key1 = gkey1 & 0x5f;key1 < 0x100;key1 += 0x20)
					{
						if (check_key(key1,BIT(key1,6),gkey1,gkey2,gkey3) || check_key(key1,BIT(key1,6)^1,gkey1,gkey2,gkey3))
						{
							load_table(4);
							key2 = gkey2;
							{
								if (check_key(key2,BIT(key2,6),gkey1,gkey2,gkey3) || check_key(key2,BIT(key2,6)^1,gkey1,gkey2,gkey3))
								{
									load_table(6);
									for (key3 = gkey3 & 0x5f;key3 < 0x100;key3 += 0x20)
									{
										if (check_key(key3,BIT(key3,6),gkey1,gkey2,gkey3) || check_key(key3,BIT(key3,6)^1,gkey1,gkey2,gkey3))
										{
											printf("key: ");
											print_binary(key1);
											printf("  ");
											print_binary(key2);
											printf("  ");
											print_binary(key3);
											printf("  ");
											printf("global key: ");
											print_binary(gkey1);
											printf(" ");
											print_binary(gkey2);
											printf(" ");
											print_binary(gkey3);
											printf("\n");
										}
									}
								}
							}
						}
					}
				}
			}
		}

		exit(0);
	}
}


UINT8 mainkey[0x2000];

void find_main_key(void)
{
	int n,key,keyF,allfound = 0,found;

	printf("Searching for main key...\n");

	for (n = 0;n < 0x2000;n++)
	{
		if (n % 16 == 0) fprintf(stderr,"%03x/1ff\r",n/16);
		load_table(n*2);
		found = 0;
		for (key = 0;key < 0x100;key++)
		{
			if (n & 0x1000) keyF = BIT(key,7);
			else            keyF = BIT(key,6);

			if (check_key(key,keyF,global_key1,global_key2,global_key3))
			{
				mainkey[n] = key;
				found++;
			}
		}

		if (found == 0)
			printf("00%04x.tbl: can't decrypt\n",n*2);
		else if (found == 1)
			allfound++;
		else if (found > 1)
		{
			printf("00%04x.tbl: ambiguous key\n",n*2);
			printf("The tables from state %02x don't allow to uniquely determine the main key.\n",state2*2);
			printf("You should dump tables from state %02x and run the program again.\n",(state2^1)*2);
			exit(0);
		}
	}

	if (allfound != 0x2000)
	{
		printf("The tables listed above are probably bad dumps.\n");
		printf("You should try redumping them and run the program again.\n");
		exit(0);
	}

	printf("Key generation succeeded!\n");
}

void save_main_key()
{
	FILE *f1;
	f1 = fopen(dstfilename,"wb");
	if (!f1)
	{
		printf("Can't open %s for writing.\n",dstfilename);
		exit(1);
	}
	if (fwrite(mainkey,1,0x2000,f1) == 0x2000)
		printf("Key saved in current directory as %s.\n",dstfilename);
	else
		printf("Error saving key.\n");

	fclose(f1);
}


int main(int argc,char **argv)
{
	if (argc != 4 || (strcmp(argv[2],"00") && strcmp(argv[2],"02")))
	{
		printf("Usage: %s <path> <state> <CPU name>\n\n",argv[0]);
		printf("<path>:     path to the directory holding the tables\n");
		printf("            files must be called 000000.tbl, 000002.tbl ... 003FFE.tbl\n");
		printf("<state>:    must be 00 or 02 - CPU state in which the tables were extracted\n");
		printf("<CPU name>: part name of the CPU\n");
		printf("\nExample: %s c:/dumps/0053-0000 00 317-0053\n",argv[0]);
		return 0;
	}

	sprintf(srcpath,argv[1]);
	if (strcmp(argv[2],"02") == 0)
		state2 = 1;
	else
		state2 = 0;
	sprintf(dstfilename,"%s.key",argv[3]);

	printf("reading tables from %s\n",srcpath);
	printf("tables were read from the CPU in state %s\n",state2 ? "02" : "00");
	printf("output file will be %s\n\n",dstfilename);

	little_endian = is_little_endian();

	find_global_key();
	find_main_key();
	save_main_key();

	return 0;
}
