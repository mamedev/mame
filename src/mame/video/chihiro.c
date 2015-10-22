// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
#include "emu.h"
#include "video/poly.h"
#include "bitmap.h"
#include "machine/pic8259.h"
#include "includes/chihiro.h"

//#define LOG_NV2A

const char *vertex_program_disassembler::srctypes[] = { "??", "Rn", "Vn", "Cn" };
const char *vertex_program_disassembler::scaops[] = { "NOP", "IMV", "RCP", "RCC", "RSQ", "EXP", "LOG", "LIT", "???", "???", "???", "???", "???", "???", "???", "???", "???" };
const int vertex_program_disassembler::scapar2[] = { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const char *vertex_program_disassembler::vecops[] = { "NOP", "MOV", "MUL", "ADD", "MAD", "DP3", "DPH", "DP4", "DST", "MIN", "MAX", "SLT", "SGE", "ARL", "???", "???", "???" };
const int vertex_program_disassembler::vecpar2[] = { 0, 4, 6, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 4, 0, 0, 0 };
const char *vertex_program_disassembler::vecouts[] = { "oPos", "???", "???", "oD0", "oD1", "oFog", "oPts", "oB0", "oB1", "oT0", "oT1", "oT2", "oT3" };
const char vertex_program_disassembler::compchar[] = { 'x', 'y', 'z', 'w' };

/*
Each vertex program instruction is a 128 bit word made of the fields:
d         f
w   b     i
o   i     e
r   t     l
d   s     d
+-+-----+-------
|0|31-0 |not used
+-+-----+-------
| |31-29|not used
| +-----+-------
| |28-25|scalar operation
| +-----+-------
| |24-21|vectorial operation
| +-----+-------
| |20-13|index for source constant C[]
| +-----+-------
| |12-9 |input vector index
| +-----+-------
|1|  8  |parameter A:sign
| +-----+-------
| | 7-6 |parameter A:swizzle x
| +-----+-------
| | 5-4 |parameter A:swizzle y
| +-----+-------
| | 3-2 |parameter A:swizzle z
| +-----+-------
| | 1-0 |parameter A:swizzle w
|-+-----+-------
| |31-28|parameter A:parameter Rn index
| +-----+-------
| |27-26|parameter A:input type 1:Rn 2:Vn 3:C[n]
| +-----+-------
| | 25  |parameter B:sign
| +-----+-------
| |24-23|parameter B:swizzle x
| +-----+-------
| |22-21|parameter B:swizzle y
| +-----+-------
| |20-19|parameter B:swizzle z
| +-----+-------
|2|18-17|parameter B:swizzle w
| +-----+-------
| |16-13|parameter B:parameter Rn index
| +-----+-------
| |12-11|parameter B:input type 1:Rn 2:Vn 3:C[n]
| +-----+-------
| | 10  |parameter C:sign
| +-----+-------
| | 9-8 |parameter C:swizzle x
| +-----+-------
| | 7-6 |parameter C:swizzle y
| +-----+-------
| | 5-4 |parameter C:swizzle z
| +-----+-------
| | 3-2 |parameter C:swizzle w
| +-----+-------
| | 1-0 |
|-+     |parameter C:parameter Rn index
| |31-30|
| +-----+-------
| |29-28|parameter C:input type 1:Rn 2:Vn 3:C[n]
| +-----+-------
| |27-24|output Rn mask from vectorial operation
| +-----+-------
| |23-20|output Rn index from vectorial operation
| +-----+-------
| |19-16|output Rn mask from scalar operation
| +-----+-------
|3|15-12|output vector write mask
| +-----+-------
| | 11  |1:output is output vector 0:output is constant C[]
| +-----+-------
| |10-3 |output vector/constant index
| +-----+-------
| |  2  |0:output Rn from vectorial operation 1:output Rn from scalar operation
| +-----+-------
| |  1  |1:add a0x to index for source constant C[]
| +-----+-------
| |  0  |1:end of program
+-+-----+-------
Each vertex program instruction can generate up to three destination values using up to three source values.
The first possible destination is to Rn from a vectorial operation.
The second possible destination is to a vertex shader output or C[n] from a vectorial or scalar operation.
The third possible destination is to Rn from a scalar operation.
*/
void vertex_program_disassembler::decodefields(unsigned int *dwords, int offset, fields &decoded)
{
	unsigned int srcbits[3];
	int a;

	srcbits[0] = ((dwords[1 + offset] & 0x1ff) << 6) | (dwords[2 + offset] >> 26);
	srcbits[1] = (dwords[2 + offset] >> 11) & 0x7fff;
	srcbits[2] = ((dwords[2 + offset] & 0x7ff) << 4) | (dwords[3 + offset] >> 28);
	decoded.ScaOperation = (int)(dwords[1 + offset] >> 25) & 0xf;
	decoded.VecOperation = (int)(dwords[1 + offset] >> 21) & 0xf;
	decoded.SourceConstantIndex = (int)(dwords[1 + offset] >> 13) & 0xff;
	decoded.InputIndex = (int)(dwords[1 + offset] >> 9) & 0xf;
	for (a = 0; a < 3; a++)
	{
		decoded.src[a].Sign = (int)(srcbits[a] >> 14) & 1;
		decoded.src[a].SwizzleX = (int)(srcbits[a] >> 12) & 3;
		decoded.src[a].SwizzleY = (int)(srcbits[a] >> 10) & 3;
		decoded.src[a].SwizzleZ = (int)(srcbits[a] >> 8) & 3;
		decoded.src[a].SwizzleW = (int)(srcbits[a] >> 6) & 3;
		decoded.src[a].TempIndex = (int)(srcbits[a] >> 2) & 0xf;
		decoded.src[a].ParameterType = (int)(srcbits[a] >> 0) & 3;
	}

	decoded.VecTempWriteMask = (int)(dwords[3 + offset] >> 24) & 0xf;
	decoded.VecTempIndex = (int)(dwords[3 + offset] >> 20) & 0xf;
	decoded.ScaTempWriteMask = (int)(dwords[3 + offset] >> 16) & 0xf;
	decoded.OutputWriteMask = (int)(dwords[3 + offset] >> 12) & 0xf;
	decoded.OutputSelect = (int)(dwords[3 + offset] >> 11) & 0x1;
	decoded.OutputIndex = (int)(dwords[3 + offset] >> 3) & 0xff;
	decoded.MultiplexerControl = (int)(dwords[3 + offset] >> 2) & 0x1;
	decoded.Usea0x = (int)(dwords[3 + offset] >> 1) & 0x1;
	decoded.EndOfProgram = (int)(dwords[3 + offset] >> 0) & 0x1;
}

int vertex_program_disassembler::disassemble_mask(int mask, char *s)
{
	int l;

	*s = 0;
	if (mask == 15)
		return 0;
	s[0] = '.';
	l = 1;
	if ((mask & 8) != 0) {
		s[l] = 'x';
		l++;
	}
	if ((mask & 4) != 0){
		s[l] = 'y';
		l++;
	}
	if ((mask & 2) != 0){
		s[l] = 'z';
		l++;
	}
	if ((mask & 1) != 0){
		s[l] = 'w';
		l++;
	}
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::disassemble_swizzle(sourcefields f, char *s)
{
	int t, l;

	t = 4;
	if (f.SwizzleW == 3)
	{
		t = t - 1;
		if (f.SwizzleZ == 2)
		{
			t = t - 1;
			if (f.SwizzleY == 1)
			{
				t = t - 1;
				if (f.SwizzleX == 0)
				{
					t = t - 1;
				}
			}
		}
	}
	*s = 0;
	if (t == 0)
		return 0;
	s[0] = '.';
	l = 1;
	if (t > 0)
	{
		s[l] = compchar[f.SwizzleX];
		l++;
	}
	if (t > 1)
	{
		s[l] = compchar[f.SwizzleY];
		l++;
	}
	if (t > 2)
	{
		s[l] = compchar[f.SwizzleZ];
		l++;
	}
	if (t > 3)
	{
		s[l] = compchar[f.SwizzleW];
		l++;
	}
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::disassemble_source(sourcefields f, fields fi, char *s)
{
	int l;

	if (f.ParameterType == 0) {
		strcpy(s, ",???");
		return 4;
	}
	l = 0;
	if (f.Sign != 0) {
		s[l] = '-';
		l++;
	}
	if (f.ParameterType == 1) {
		s[l] = 'r';
		l = l + 1 + sprintf(s + l + 1, "%d", f.TempIndex);
	}
	else if (f.ParameterType == 2){
		s[l] = 'v';
		l = l + 1 + sprintf(s + l + 1, "%d", fi.InputIndex);
	}
	else
	{
		if (fi.Usea0x != 0)
		{
			if (fi.SourceConstantIndex >= 96) {
				strcpy(s + l, "c[");
				l = l + 2;
				l = l + sprintf(s + l, "%d", fi.SourceConstantIndex - 96);
				strcpy(s + l, "+a0.x]");
				l = l + 6;
			}
			else {
				strcpy(s + l, "c[a0.x");
				l = l + 6;
				l = l + sprintf(s + l, "%d", fi.SourceConstantIndex - 96);
				s[l] = ']';
				l++;
			}
		}
		else {
			strcpy(s + l, "c[");
			l = l + 2;
			l = l + sprintf(s + l, "%d", fi.SourceConstantIndex - 96);
			s[l] = ']';
			l++;
		}
	}
	l = l + disassemble_swizzle(f, s + l);
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::disassemble_output(fields f, char *s)
{
	int l;

	if (f.OutputSelect == 1) {
		strcpy(s, vecouts[f.OutputIndex]);
		return strlen(s);
	}
	else {
		strcpy(s, "c[");
		l = 2;
		l = l + sprintf(s + l, "%d", f.OutputIndex - 96);
		s[l] = ']';
		l++;
	}
	s[l] = 0;
	return l;
}

int vertex_program_disassembler::output_types(fields f, int *o)
{
	o[0] = o[1] = o[2] = o[3] = o[4] = o[5] = 0;
	if ((f.VecOperation > 0) && (f.VecTempWriteMask != 0))
		o[0] = 1;
	if ((f.VecOperation > 0) && (f.OutputWriteMask != 0) && (f.MultiplexerControl == 0))
		o[1] = 1;
	if ((f.ScaOperation > 0) && (f.OutputWriteMask != 0) && (f.MultiplexerControl == 1))
		o[2] = 1;
	if ((f.ScaOperation > 0) && (f.ScaTempWriteMask != 0))
		o[3] = 1;
	if (f.VecOperation == 13)
		o[4] = 1;
	if (f.EndOfProgram == 1)
		o[5] = 1;
	return o[0] + o[1] + o[2] + o[3] + o[4] + o[5];
}

int vertex_program_disassembler::disassemble(unsigned int *instruction, char *line)
{
	int b, p;
	char *c;

	if (state == 0) {
		decodefields(instruction, 0, f);
		output_types(f, o);
		state = 1;
	}
	if (o[0] != 0)
	{
		o[0] = 0;
		c = line;
		strcpy(c, vecops[f.VecOperation]);
		c = c + strlen(c);
		strcpy(c, " r");
		c = c + 2;
		c = c + sprintf(c, "%d", f.VecTempIndex);
		c = c + disassemble_mask(f.VecTempWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((vecpar2[f.VecOperation] & p) != 0) {
				c[0] = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[1] != 0)
	{
		o[1] = 0;
		c = line;
		strcpy(c, vecops[f.VecOperation]);
		c = c + strlen(c);
		*c = ' ';
		c++;
		c = c + disassemble_output(f, c);
		c = c + disassemble_mask(f.OutputWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((vecpar2[f.VecOperation] & p) != 0) {
				*c = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[2] != 0)
	{
		o[2] = 0;
		c = line;
		strcpy(c, scaops[f.ScaOperation]);
		c = c + strlen(c);
		*c = ' ';
		c++;
		c = c + disassemble_output(f, c);
		c = c + disassemble_mask(f.OutputWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((scapar2[f.ScaOperation] & p) != 0) {
				*c = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[3] != 0)
	{
		if (f.VecOperation > 0)
			b = 1;
		else
			b = f.VecTempIndex;
		o[3] = 0;
		c = line;
		strcpy(c, scaops[f.ScaOperation]);
		c = c + strlen(c);
		strcpy(c, " r");
		c = c + 2;
		c = c + sprintf(c, "%d", b);
		c = c + disassemble_mask(f.ScaTempWriteMask, c);
		b = 0;
		for (p = 4; p != 0; p = p >> 1)
		{
			if ((scapar2[f.ScaOperation] & p) != 0) {
				*c = ',';
				c++;
				c = c + disassemble_source(f.src[b], f, c);
			}
			b++;
		}
		*c = 0;
		return 1;
	}
	if (o[4] != 0)
	{
		o[4] = 0;
		c = line;
		c = c + sprintf(c, "MOV a0.x,");
		c = c + disassemble_source(f.src[0], f, c);
		*c = 0;
		return 1;
	}
	if (o[5] != 0)
	{
		o[5] = 0;
		strcpy(line, "END");
		return 1;
	}
	state = 0;
	return 0;
}

vertex_program_simulator::vertex_program_simulator()
{
	for (int i = 0; i < 256; i++)
		op[i].modified = 0;
	initialize_constants();
}

void vertex_program_simulator::set_data(vertex_nv *in, vertex_nv *out)
{
	input = in;
	output = out;
}

void vertex_program_simulator::reset()
{
	ip = 0;
	a0x = 0;
	initialize_outputs();
	initialize_temps();
}

void vertex_program_simulator::decode_instruction(int address)
{
	instruction *i;

	i = &op[address];
	i->d.SignA = i->i[1] & (1 << 8);
	i->d.ParameterTypeA = (i->i[2] >> 26) & 3;
	i->d.TempIndexA = (i->i[2] >> 28) & 15;
	i->d.SwizzleA[0] = (i->i[1] >> 6) & 3;
	i->d.SwizzleA[1] = (i->i[1] >> 4) & 3;
	i->d.SwizzleA[2] = (i->i[1] >> 2) & 3;
	i->d.SwizzleA[3] = (i->i[1] >> 0) & 3;
	i->d.SignB = i->i[2] & (1 << 25);
	i->d.ParameterTypeB = (i->i[2] >> 11) & 3;
	i->d.TempIndexB = (i->i[2] >> 13) & 15;
	i->d.SwizzleB[0] = (i->i[2] >> 23) & 3;
	i->d.SwizzleB[1] = (i->i[2] >> 21) & 3;
	i->d.SwizzleB[2] = (i->i[2] >> 19) & 3;
	i->d.SwizzleB[3] = (i->i[2] >> 17) & 3;
	i->d.SignC = i->i[2] & (1 << 10);
	i->d.ParameterTypeC = (i->i[3] >> 28) & 3;
	i->d.TempIndexC = ((i->i[2] & 3) << 2) + (i->i[3] >> 30);
	i->d.SwizzleC[0] = (i->i[2] >> 8) & 3;
	i->d.SwizzleC[1] = (i->i[2] >> 6) & 3;
	i->d.SwizzleC[2] = (i->i[2] >> 4) & 3;
	i->d.SwizzleC[3] = (i->i[2] >> 2) & 3;
	i->d.VecOperation = (VectorialOperation)((i->i[1] >> 21) & 15);
	i->d.ScaOperation = (ScalarOperation)((i->i[1] >> 25) & 15);
	i->d.OutputWriteMask = ((i->i[3] >> 12) & 15);
	i->d.MultiplexerControl = i->i[3] & 4; // 0 : output Rn from vectorial operation 4 : output Rn from scalar operation
	i->d.VecTempIndex = (i->i[3] >> 20) & 15;
	i->d.OutputIndex = (i->i[3] >> 3) & 255;
	i->d.OutputSelect = i->i[3] & 0x800;
	i->d.VecTempWriteMask = (i->i[3] >> 24) & 15;
	i->d.ScaTempWriteMask = (i->i[3] >> 16) & 15;
	i->d.InputIndex = (i->i[1] >> 9) & 15;
	i->d.SourceConstantIndex = (i->i[1] >> 13) & 255;
	i->d.Usea0x = i->i[3] & 2;
	i->d.EndOfProgram = i->i[3] & 1;
}

int vertex_program_simulator::step()
{
	int p1, p2;
	float tmp[3 * 4];
	float tmpv[4] = { 0, 0, 0, 0};
	float tmps[4] = { 0, 0, 0, 0};
	instruction::decoded *d;

#if 0 // useful while debugging to see what instrucion is being executed
	static int debugvpi = 0;
	char disbuffer[256];
	if (debugvpi) {
		char *pp;
		vertex_program_disassembler vdis;

		pp = disbuffer;
		while (vdis.disassemble(op[ip].i, pp) != 0) {
			pp = pp + strlen(pp);
			*pp = '\n';
			pp++;
			*pp = 0;
		}
	}
#endif

	if (op[ip].modified)
		decode_instruction(ip);
	d = &(op[ip].d);
	// prepare inputs
	//  input A
	generate_input(&tmp[0], d->SignA, d->ParameterTypeA, d->TempIndexA, d->SwizzleA);
	//  input B
	generate_input(&tmp[4], d->SignB, d->ParameterTypeB, d->TempIndexB, d->SwizzleB);
	//  input C
	generate_input(&tmp[8], d->SignC, d->ParameterTypeC, d->TempIndexC, d->SwizzleC);
	// compute 2 instructions
	//  vectorial
	compute_vectorial_operation(tmpv, d->VecOperation, tmp);
	//  scalar
	compute_scalar_operation(tmps, d->ScaOperation, tmp);
	// assign destinations
	if (d->VecOperation > 0) {
		if (d->VecOperation == 13)
			//o[4] = 1;
			a0x = (int)tmpv[0];
		else {
			if (d->VecTempWriteMask != 0) { // assign to Rn
				//o[0] = 1;
				int wm = d->VecTempWriteMask;
				for (p1 = 0; p1 < 4; p1++) {
					if (wm & 8)
						r_temp[d->VecTempIndex].fv[p1] = tmpv[p1];
					wm = wm << 1;
				}
			}
			if ((d->OutputWriteMask != 0) && (d->MultiplexerControl == 0)) {
				//o[1] = 1;
				if (d->OutputSelect) { // assign to output
					int wm = d->OutputWriteMask;
					for (p1 = 0; p1 < 4; p1++) {
						if (wm & 8)
							output->attribute[d->OutputIndex].fv[p1] = tmpv[p1];
						wm = wm << 1;
					}
					// remeber, output position == r12
					if (d->OutputIndex == 0)
						for (p1 = 0; p1 < 4; p1++) {
							r_temp[12].fv[p1] = output->attribute[d->OutputIndex].fv[p1];
						}
				}
				else { // assign to constant
					int wm = d->OutputWriteMask;
					for (p1 = 0; p1 < 4; p1++) {
						if (wm & 8)
							c_constant[d->OutputIndex].fv[p1] = tmpv[p1];
						wm = wm << 1;
					}
				}
			}
		}
	}
	if (d->ScaOperation > 0) {
		if (d->ScaTempWriteMask != 0) { // assign to Rn
			//o[3] = 1;
			if (d->VecOperation > 0)
				p2 = 1;
			else
				p2 = d->VecTempIndex;
			int wm = d->ScaTempWriteMask;
			for (p1 = 0; p1 < 4; p1++) {
				if (wm & 8)
					r_temp[p2].fv[p1] = tmps[p1];
				wm = wm << 1;
			}
		}
		if ((d->OutputWriteMask != 0) && (d->MultiplexerControl != 0)) { // assign to output
			//o[2] = 1;
			int wm = d->OutputWriteMask;
			for (p1 = 0; p1 < 4; p1++) {
				if (wm & 8)
					output->attribute[d->OutputIndex].fv[p1] = tmps[p1];
				wm = wm << 1;
			}
			// remeber, output position == r12
			if (d->OutputIndex == 0) {
				for (p1 = 0; p1 < 4; p1++) {
					r_temp[12].fv[p1] = output->attribute[d->OutputIndex].fv[p1];
				}
			}
		}
	}
	return d->EndOfProgram;
}

void vertex_program_simulator::execute()
{
	int c;

	c = 0;
	do {
		c = step();
		ip++;
	} while (c == 0);
}

void vertex_program_simulator::jump(int address)
{
	ip = address;
}

void vertex_program_simulator::process(int address, vertex_nv *in, vertex_nv *out, int count)
{
#if 0 // useful while debugging to see what is being executed
	static int debugvps = 0;
	if (debugvps) {
		char *pp;
		vertex_program_disassembler vdis;
		char disbuffer[128];

		jump(address);
		debugvps--;
		for (int t = 0; t < 128; t++) {
			pp = disbuffer;
			while (vdis.disassemble(op[ip + t].i, pp) != 0) {
				pp = pp + strlen(pp);
				*pp = '\n';
				pp++;
				*pp = 0;
			}
			printf("%08X %08X %08X %s", op[ip + t].i[1], op[ip + t].i[2], op[ip + t].i[3], disbuffer);
			if (op[ip + t].i[3] & 1)
				break;
		}
	}
#endif
	set_data(in, out);
	while (count > 0) {
		reset();
		jump(address);
		execute();
		input++;
		output++;
		count--;
	}
}

int vertex_program_simulator::status()
{
	return ip;
}

void vertex_program_simulator::initialize_outputs()
{
	for (int n = 0; n < 16; n++) {
		output->attribute[n].fv[0] = output->attribute[n].fv[1] = output->attribute[n].fv[2] = 0;
		output->attribute[n].fv[3] = 1;
	}
}

void vertex_program_simulator::initialize_temps()
{
	for (int n = 0; n < 32; n++) {
		for (int m = 0; m < 4; m++)
			r_temp[n].fv[m] = 0;
	}
}

void vertex_program_simulator::initialize_constants()
{
	for (int n = 0; n < 192; n++) {
		for (int m = 0; m < 4;m++)
			c_constant[n].fv[m] = 0;
	}
}

void vertex_program_simulator::generate_input(float t[4], int sign, int type, int temp, int swizzle[4])
{
	float sgn = 1;

	if (sign)
		sgn = -1;
	if (type == 1) {
		t[0] = sgn*r_temp[temp].fv[swizzle[0]];
		t[1] = sgn*r_temp[temp].fv[swizzle[1]];
		t[2] = sgn*r_temp[temp].fv[swizzle[2]];
		t[3] = sgn*r_temp[temp].fv[swizzle[3]];
	}
	else if (type == 2) {
		int InputIndex = op[ip].d.InputIndex;
		t[0] = sgn*input->attribute[InputIndex].fv[swizzle[0]];
		t[1] = sgn*input->attribute[InputIndex].fv[swizzle[1]];
		t[2] = sgn*input->attribute[InputIndex].fv[swizzle[2]];
		t[3] = sgn*input->attribute[InputIndex].fv[swizzle[3]];
	}
	else if (type == 3) {
		int SourceConstantIndex = op[ip].d.SourceConstantIndex;
		if (op[ip].d.Usea0x)
			SourceConstantIndex = SourceConstantIndex + a0x;
		t[0] = sgn*c_constant[SourceConstantIndex].fv[swizzle[0]];
		t[1] = sgn*c_constant[SourceConstantIndex].fv[swizzle[1]];
		t[2] = sgn*c_constant[SourceConstantIndex].fv[swizzle[2]];
		t[3] = sgn*c_constant[SourceConstantIndex].fv[swizzle[3]];
	}
}

void vertex_program_simulator::compute_vectorial_operation(float t_out[4], int instruction, float par_in[3 * 4])
{
	const int p1_A = 0;
	const int p2_B = 4;
	const int p3_C = 8;

	// t_out <= instruction(par_in)
	switch (instruction) {
	case 0: // "NOP"
		break;
	case 1: // "MOV"
		t_out[0] = par_in[p1_A + 0];
		t_out[1] = par_in[p1_A + 1];
		t_out[2] = par_in[p1_A + 2];
		t_out[3] = par_in[p1_A + 3];
		break;
	case 2: // "MUL"
		t_out[0] = par_in[p1_A + 0] * par_in[p2_B + 0];
		t_out[1] = par_in[p1_A + 1] * par_in[p2_B + 1];
		t_out[2] = par_in[p1_A + 2] * par_in[p2_B + 2];
		t_out[3] = par_in[p1_A + 3] * par_in[p2_B + 3];
		break;
	case 3: // "ADD"
		t_out[0] = par_in[p1_A + 0] + par_in[p3_C + 0];
		t_out[1] = par_in[p1_A + 1] + par_in[p3_C + 1];
		t_out[2] = par_in[p1_A + 2] + par_in[p3_C + 2];
		t_out[3] = par_in[p1_A + 3] + par_in[p3_C + 3];
		break;
	case 4: // "MAD"
		t_out[0] = par_in[p1_A + 0] * par_in[p2_B + 0] + par_in[p3_C + 0];
		t_out[1] = par_in[p1_A + 1] * par_in[p2_B + 1] + par_in[p3_C + 1];
		t_out[2] = par_in[p1_A + 2] * par_in[p2_B + 2] + par_in[p3_C + 2];
		t_out[3] = par_in[p1_A + 3] * par_in[p2_B + 3] + par_in[p3_C + 3];
		break;
	case 5: // "DP3"
		t_out[0] = par_in[p1_A + 0] * par_in[p2_B + 0] + par_in[p1_A + 1] * par_in[p2_B + 1] + par_in[p1_A + 2] * par_in[p2_B + 2];
		t_out[1] = t_out[2] = t_out[3] = t_out[0];
		break;
	case 6: // "DPH"
		t_out[0] = par_in[p1_A + 0] * par_in[p2_B + 0] + par_in[p1_A + 1] * par_in[p2_B + 1] + par_in[p1_A + 2] * par_in[p2_B + 2] + par_in[p2_B + 3];
		t_out[1] = t_out[2] = t_out[3] = t_out[0];
		break;
	case 7: // "DP4"
		t_out[0] = par_in[p1_A + 0] * par_in[p2_B + 0] + par_in[p1_A + 1] * par_in[p2_B + 1] + par_in[p1_A + 2] * par_in[p2_B + 2] + par_in[p1_A + 3] * par_in[p2_B + 3];
		t_out[1] = t_out[2] = t_out[3] = t_out[0];
		break;
	case 8: // "DST"
		t_out[0] = 1.0;
		t_out[1] = par_in[p1_A + 1] * par_in[p2_B + 1];
		t_out[2] = par_in[p1_A + 2];
		t_out[3] = par_in[p2_B + 3];
		break;
	case 9: // "MIN"
		t_out[0] = fmin(par_in[p1_A + 0], par_in[p2_B + 0]);
		t_out[1] = fmin(par_in[p1_A + 1], par_in[p2_B + 1]);
		t_out[2] = fmin(par_in[p1_A + 2], par_in[p2_B + 2]);
		t_out[3] = fmin(par_in[p1_A + 3], par_in[p2_B + 3]);
		break;
	case 10: // "MAX"
		t_out[0] = fmax(par_in[p1_A + 0], par_in[p2_B + 0]);
		t_out[1] = fmax(par_in[p1_A + 1], par_in[p2_B + 1]);
		t_out[2] = fmax(par_in[p1_A + 2], par_in[p2_B + 2]);
		t_out[3] = fmax(par_in[p1_A + 3], par_in[p2_B + 3]);
		break;
	case 11: // "SLT"
		t_out[0] = (par_in[p1_A + 0] < par_in[p2_B + 0]) ? 1.0 : 0;
		t_out[1] = (par_in[p1_A + 1] < par_in[p2_B + 1]) ? 1.0 : 0;
		t_out[2] = (par_in[p1_A + 2] < par_in[p2_B + 2]) ? 1.0 : 0;
		t_out[3] = (par_in[p1_A + 3] < par_in[p2_B + 3]) ? 1.0 : 0;
		break;
	case 12: // "SGE"
		t_out[0] = (par_in[p1_A + 0] >= par_in[p2_B + 0]) ? 1.0 : 0;
		t_out[1] = (par_in[p1_A + 1] >= par_in[p2_B + 1]) ? 1.0 : 0;
		t_out[2] = (par_in[p1_A + 2] >= par_in[p2_B + 2]) ? 1.0 : 0;
		t_out[3] = (par_in[p1_A + 3] >= par_in[p2_B + 3]) ? 1.0 : 0;
		break;
	case 13: // "ARL"
		t_out[0] = par_in[p1_A + 0];
	}
}

void vertex_program_simulator::compute_scalar_operation(float t_out[4], int instruction, float par_in[3 * 4])
{
	//const int p1_A = 0;
	//const int p2_B = 4;
	const int p3_C = 8;
	union {
		float f;
		unsigned int i;
	} t;
	int e;

	// t_out <= instruction(par_in)
	switch (instruction) {
	case 0: // "NOP"
		break;
	case 1: // "IMV"
		t_out[0] = par_in[p3_C + 0];
		t_out[1] = par_in[p3_C + 1];
		t_out[2] = par_in[p3_C + 2];
		t_out[3] = par_in[p3_C + 3];
		break;
	case 2: // "RCP"
		t_out[0] = t_out[1] = t_out[2] = t_out[3] = 1.0f / par_in[p3_C + 0];
		break;
	case 3: // "RCC"
		t_out[0] = t_out[1] = t_out[2] = t_out[3] = 1.0f / par_in[p3_C + 0]; // ?
		break;
	case 4: // "RSQ"
		/*
		 *  NOTE: this was abs which is "int abs(int x)" - and changed to fabsf due to clang 3.6 warning
		 */
		t_out[0] = t_out[1] = t_out[2] = t_out[3] = 1.0f / sqrtf(fabsf(par_in[p3_C + 0]));
		break;
	case 5: // "EXP"
		t_out[0] = pow(2, floor(par_in[p3_C + 0]));
		t_out[1] = par_in[p3_C + 0] - floorf(par_in[p3_C + 0]);
		t.f = pow(2, par_in[p3_C + 0]);
		t.i = t.i & 0xffffff00;
		t_out[2] = t.f;
		t_out[3] = 1.0;
		break;
	case 6: // "LOG"
		t_out[1] = frexp(par_in[p3_C + 0], &e)*2.0; // frexp gives mantissa as 0.5....1
		t_out[0] = e - 1;
#ifndef __OS2__
		/*
		 *  NOTE: this was abs which is "int abs(int x)" - and changed to fabsf due to clang 3.6 warning
		 */
		t.f = log2(fabsf(par_in[p3_C + 0]));
#else
		static double log_2 = 0.0;
		if (log_2 == 0.0)
			log_2 = log(2);
		t.f = log(abs(par_in[p3_C + 0])) / log_2;
#endif
		t.i = t.i & 0xffffff00;
		t_out[2] = t.f;
		t_out[3] = 1.0;
		break;
	case 7: // "LIT"
		t_out[0] = 1.0;
		t_out[1] = fmax(0, fmin(par_in[p3_C + 0], 1.0f));
		t_out[2] = par_in[p3_C + 0] > 0 ? pow(fmax(par_in[p3_C + 1], 0), par_in[p3_C + 3]) : 0;
		t_out[3] = 1.0;
		break;
	}
}

/*
 * Graphics
 */

UINT32 nv2a_renderer::dilate0(UINT32 value, int bits) // dilate first "bits" bits in "value"
{
	UINT32 x, m1, m2, m3;
	int a;

	x = value;
	for (a = 0; a < bits; a++)
	{
		m2 = 1 << (a << 1);
		m1 = m2 - 1;
		m3 = (~m1) << 1;
		x = (x & m1) + (x & m2) + ((x & m3) << 1);
	}
	return x;
}

UINT32 nv2a_renderer::dilate1(UINT32 value, int bits) // dilate first "bits" bits in "value"
{
	UINT32 x, m1, m2, m3;
	int a;

	x = value;
	for (a = 0; a < bits; a++)
	{
		m2 = 1 << (a << 1);
		m1 = m2 - 1;
		m3 = (~m1) << 1;
		x = (x & m1) + ((x & m2) << 1) + ((x & m3) << 1);
	}
	return x;
}

void nv2a_renderer::computedilated(void)
{
	int a, b;

	for (b = 0; b < 16; b++)
		for (a = 0; a < 2048; a++) {
		dilated0[b][a] = dilate0(a, b);
		dilated1[b][a] = dilate1(a, b);
		}
	for (b = 0; b < 16; b++)
		for (a = 0; a < 16; a++)
			dilatechose[(b << 4) + a] = (a < b ? a : b);
}

inline UINT8 *nv2a_renderer::direct_access_ptr(offs_t address)
{
	return basemempointer + address;
}

int nv2a_renderer::geforce_commandkind(UINT32 word)
{
	if ((word & 0x00000003) == 0x00000002)
		return 7; // call
	if ((word & 0x00000003) == 0x00000001)
		return 6; // jump
	if ((word & 0xE0030003) == 0x40000000)
		return 5; // non increasing
	if ((word & 0xE0000003) == 0x20000000)
		return 4; // old jump
	if ((word & 0xFFFF0003) == 0x00030000)
		return 3; // long non icreasing
	if ((word & 0xFFFFFFFF) == 0x00020000)
		return 2; // return
	if ((word & 0xFFFF0003) == 0x00010000)
		return 1; // sli conditional
	if ((word & 0xE0030003) == 0x00000000)
		return 0; // increasing
	return -1;
}

UINT32 nv2a_renderer::geforce_object_offset(UINT32 handle)
{
	UINT32 h = ((((handle >> 11) ^ handle) >> 11) ^ handle) & 0x7ff;
	UINT32 o = (pfifo[0x210 / 4] & 0x1ff) << 8; // 0x1ff is not certain
	UINT32 e = o + h * 8; // at 0xfd000000+0x00700000
	UINT32 w;

	if (ramin[e / 4] != handle) {
		// this should never happen
		for (UINT32 aa = o / 4; aa < (sizeof(ramin) / 4); aa = aa + 2) {
			if (ramin[aa] == handle) {
				e = aa * 4;
			}
		}
	}
	w = ramin[e / 4 + 1];
	return (w & 0xffff) * 0x10; // 0xffff is not certain
}

void nv2a_renderer::geforce_read_dma_object(UINT32 handle, UINT32 &offset, UINT32 &size)
{
	//UINT32 objclass,pt_present,pt_linear,access,target,rorw;
	UINT32 dma_adjust, dma_frame;
	UINT32 o = geforce_object_offset(handle);

	o = o / 4;
	//objclass=ramin[o] & 0xfff;
	//pt_present=(ramin[o] >> 12) & 1;
	//pt_linear=(ramin[o] >> 13) & 1;
	//access=(ramin[o] >> 14) & 3;
	//target=(ramin[o] >> 16) & 3;
	dma_adjust = (ramin[o] >> 20) & 0xfff;
	size = ramin[o + 1];
	//rorw=ramin[o+2] & 1;
	dma_frame = ramin[o + 2] & 0xfffff000;
	offset = dma_frame + dma_adjust;
}

/*void myline(bitmap_rgb32 &bmp,float x1,float y1,float x2,float y2)
{
int xx1,yy1,xx2,yy2;

    xx1=x1;
    xx2=x2;
    yy1=y1;
    yy2=y2;
    if (xx1 == xx2) {
        if (yy1 > yy2) {
            int t=yy1;
            yy1=yy2;
            yy2=t;
        }
        for (int y=yy1;y <= yy2;y++)
            *((UINT32 *)bmp.raw_pixptr(y,xx1))= -1;
        } else if (yy1 == yy2) {
        if (xx1 > xx2) {
            int t=xx1;
            xx1=xx2;
            xx2=t;
        }
        for (int x=xx1;x <= xx2;x++)
            *((UINT32 *)bmp.raw_pixptr(yy1,x))= -1;
    }
}*/

inline UINT32 convert_a4r4g4b4_a8r8g8b8(UINT32 a4r4g4b4)
{
	UINT32 a8r8g8b8;
	int ca, cr, cg, cb;

	cb = pal4bit(a4r4g4b4 & 0x000f);
	cg = pal4bit((a4r4g4b4 & 0x00f0) >> 4);
	cr = pal4bit((a4r4g4b4 & 0x0f00) >> 8);
	ca = pal4bit((a4r4g4b4 & 0xf000) >> 12);
	a8r8g8b8 = (ca << 24) | (cr << 16) | (cg << 8) | (cb); // color converted to 8 bits per component
	return a8r8g8b8;
}

inline UINT32 convert_a1r5g5b5_a8r8g8b8(UINT32 a1r5g5b5)
{
	UINT32 a8r8g8b8;
	int ca, cr, cg, cb;

	cb = pal5bit(a1r5g5b5 & 0x001f);
	cg = pal5bit((a1r5g5b5 & 0x03e0) >> 5);
	cr = pal5bit((a1r5g5b5 & 0x7c00) >> 10);
	ca = a1r5g5b5 & 0x8000 ? 0xff : 0;
	a8r8g8b8 = (ca << 24) | (cr << 16) | (cg << 8) | (cb); // color converted to 8 bits per component
	return a8r8g8b8;
}

inline UINT32 convert_r5g6b5_r8g8b8(UINT32 r5g6b5)
{
	UINT32 r8g8b8;
	int cr, cg, cb;

	cb = pal5bit(r5g6b5 & 0x001f);
	cg = pal6bit((r5g6b5 & 0x07e0) >> 5);
	cr = pal5bit((r5g6b5 & 0xf800) >> 11);
	r8g8b8 = (cr << 16) | (cg << 8) | (cb); // color converted to 8 bits per component
	return r8g8b8;
}

UINT32 nv2a_renderer::texture_get_texel(int number, int x, int y)
{
	UINT32 to, s, c, sa, ca;
	UINT32 a4r4g4b4, a1r5g5b5, r5g6b5;
	int bx, by;
	int color0, color1, color0m2, color1m2, alpha0, alpha1;
	UINT32 codes;
	UINT64 alphas;
	int cr, cg, cb;

	// force to [0,size-1]
	x = (unsigned int)x & (texture[number].sizeu - 1);
	y = (unsigned int)y & (texture[number].sizev - 1);
	switch (texture[number].format) {
	case A8R8G8B8:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		return *(((UINT32 *)texture[number].buffer) + to); // get texel color
	case DXT1:
		bx = x >> 2;
		by = y >> 2;
		x = x & 3;
		y = y & 3;
		to = bx + by*(texture[number].sizeu >> 2);
		color0 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 0);
		color1 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 1);
		codes = *((UINT32 *)(((UINT64 *)texture[number].buffer) + to) + 1);
		s = (y << 3) + (x << 1);
		c = (codes >> s) & 3;
		c = c + (color0 > color1 ? 0 : 4);
		color0m2 = color0 << 1;
		color1m2 = color1 << 1;
		switch (c) {
		case 0:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color0);
		case 1:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color1);
		case 2:
			cb = pal5bit(((color0m2 & 0x003e) + (color1 & 0x001f)) / 3);
			cg = pal6bit(((color0m2 & 0x0fc0) + (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color0m2 & 0x1f000) + color1) / 3 >> 11);
			return 0xff000000 | (cr << 16) | (cg << 8) | (cb);
		case 3:
			cb = pal5bit(((color1m2 & 0x003e) + (color0 & 0x001f)) / 3);
			cg = pal6bit(((color1m2 & 0x0fc0) + (color0 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color1m2 & 0x1f000) + color0) / 3 >> 11);
			return 0xff000000 | (cr << 16) | (cg << 8) | (cb);
		case 4:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color0);
		case 5:
			return 0xff000000 + convert_r5g6b5_r8g8b8(color1);
		case 6:
			cb = pal5bit(((color0 & 0x001f) + (color1 & 0x001f)) / 2);
			cg = pal6bit(((color0 & 0x07e0) + (color1 & 0x07e0)) / 2 >> 5);
			cr = pal5bit(((color0 & 0xf800) + (color1 & 0xf800)) / 2 >> 11);
			return 0xff000000 | (cr << 16) | (cg << 8) | (cb);
		default:
			return 0xff000000;
		}
	case DXT3:
		bx = x >> 2;
		by = y >> 2;
		x = x & 3;
		y = y & 3;
		to = (bx + by*(texture[number].sizeu >> 2)) << 1;
		color0 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 4);
		color1 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 5);
		codes = *((UINT32 *)(((UINT64 *)texture[number].buffer) + to) + 3);
		alphas = *(((UINT64 *)texture[number].buffer) + to);
		s = (y << 3) + (x << 1);
		sa = ((y << 2) + x) << 2;
		c = (codes >> s) & 3;
		ca = (alphas >> sa) & 15;
		switch (c) {
		case 0:
			return ((ca + (ca << 4)) << 24) + convert_r5g6b5_r8g8b8(color0);
		case 1:
			return ((ca + (ca << 4)) << 24) + convert_r5g6b5_r8g8b8(color1);
		case 2:
			cb = pal5bit((2 * (color0 & 0x001f) + (color1 & 0x001f)) / 3);
			cg = pal6bit((2 * (color0 & 0x07e0) + (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit((2 * (color0 & 0xf800) + (color1 & 0xf800)) / 3 >> 11);
			return ((ca + (ca << 4)) << 24) | (cr << 16) | (cg << 8) | (cb);
		default:
			cb = pal5bit(((color0 & 0x001f) + 2 * (color1 & 0x001f)) / 3);
			cg = pal6bit(((color0 & 0x07e0) + 2 * (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color0 & 0xf800) + 2 * (color1 & 0xf800)) / 3 >> 11);
			return ((ca + (ca << 4)) << 24) | (cr << 16) | (cg << 8) | (cb);
		}
	case A4R4G4B4:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		a4r4g4b4 = *(((UINT16 *)texture[number].buffer) + to); // get texel color
		return convert_a4r4g4b4_a8r8g8b8(a4r4g4b4);
	case A1R5G5B5:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		a1r5g5b5 = *(((UINT16 *)texture[number].buffer) + to); // get texel color
		return convert_a1r5g5b5_a8r8g8b8(a1r5g5b5);
	case R5G6B5:
		to = dilated0[texture[number].dilate][x] + dilated1[texture[number].dilate][y]; // offset of texel in texture memory
		r5g6b5 = *(((UINT16 *)texture[number].buffer) + to); // get texel color
		return 0xff000000 + convert_r5g6b5_r8g8b8(r5g6b5);
	case R8G8B8_RECT:
		to = texture[number].rectangle_pitch*y + (x << 2);
		return *((UINT32 *)(((UINT8 *)texture[number].buffer) + to));
	case A8R8G8B8_RECT:
		to = texture[number].rectangle_pitch*y + (x << 2);
		return *((UINT32 *)(((UINT8 *)texture[number].buffer) + to));
	case DXT5:
		bx = x >> 2;
		by = y >> 2;
		x = x & 3;
		y = y & 3;
		to = (bx + by*(texture[number].sizeu >> 2)) << 1;
		color0 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 4);
		color1 = *((UINT16 *)(((UINT64 *)texture[number].buffer) + to) + 5);
		codes = *((UINT32 *)(((UINT64 *)texture[number].buffer) + to) + 3);
		alpha0 = *((UINT8 *)(((UINT64 *)texture[number].buffer) + to) + 0);
		alpha1 = *((UINT8 *)(((UINT64 *)texture[number].buffer) + to) + 1);
		alphas = *(((UINT64 *)texture[number].buffer) + to);
		s = (y << 3) + (x << 1);
		sa = ((y << 2) + x) * 3;
		c = (codes >> s) & 3;
		ca = (alphas >> sa) & 7;
		ca = ca + (alpha0 > alpha1 ? 0 : 8);
		switch (ca) {
		case 0:
			ca = alpha0;
			break;
		case 1:
			ca = alpha1;
			break;
		case 2:
			ca = (6 * alpha0 + 1 * alpha1) / 7;
			break;
		case 3:
			ca = (5 * alpha0 + 2 * alpha1) / 7;
			break;
		case 4:
			ca = (4 * alpha0 + 3 * alpha1) / 7;
			break;
		case 5:
			ca = (3 * alpha0 + 4 * alpha1) / 7;
			break;
		case 6:
			ca = (2 * alpha0 + 5 * alpha1) / 7;
			break;
		case 7:
			ca = (1 * alpha0 + 6 * alpha1) / 7;
			break;
		case 8:
			ca = alpha0;
			break;
		case 9:
			ca = alpha1;
			break;
		case 10:
			ca = (4 * alpha0 + 1 * alpha1) / 5;
			break;
		case 11:
			ca = (3 * alpha0 + 2 * alpha1) / 5;
			break;
		case 12:
			ca = (2 * alpha0 + 3 * alpha1) / 5;
			break;
		case 13:
			ca = (1 * alpha0 + 4 * alpha1) / 5;
			break;
		case 14:
			ca = 0;
			break;
		case 15:
			ca = 255;
			break;
		}
		switch (c) {
		case 0:
			return (ca << 24) + convert_r5g6b5_r8g8b8(color0);
		case 1:
			return (ca << 24) + convert_r5g6b5_r8g8b8(color1);
		case 2:
			cb = pal5bit((2 * (color0 & 0x001f) + (color1 & 0x001f)) / 3);
			cg = pal6bit((2 * (color0 & 0x07e0) + (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit((2 * (color0 & 0xf800) + (color1 & 0xf800)) / 3 >> 11);
			return (ca << 24) | (cr << 16) | (cg << 8) | (cb);
		default:
			cb = pal5bit(((color0 & 0x001f) + 2 * (color1 & 0x001f)) / 3);
			cg = pal6bit(((color0 & 0x07e0) + 2 * (color1 & 0x07e0)) / 3 >> 5);
			cr = pal5bit(((color0 & 0xf800) + 2 * (color1 & 0xf800)) / 3 >> 11);
			return (ca << 24) | (cr << 16) | (cg << 8) | (cb);
		}
	default:
		return 0xff00ff00;
	}
}

inline UINT8 *nv2a_renderer::read_pixel(int x, int y, UINT32 c[4])
{
	UINT32 offset;
	UINT32 color;
	UINT32 *addr;
	UINT16 *addr16;
	UINT8 *addr8;

	if (type_rendertarget == SWIZZLED)
		offset = (dilated0[dilate_rendertarget][x] + dilated1[dilate_rendertarget][y]) * bytespixel_rendertarget;
	else // type_rendertarget == LINEAR*/
		offset = pitch_rendertarget * y + x * bytespixel_rendertarget;
	switch (colorformat_rendertarget) {
	case NV2A_COLOR_FORMAT_R5G6B5:
		addr16 = (UINT16 *)((UINT8 *)rendertarget + offset);
		color = *addr16;
		c[3] = 0xff;
		c[2] = pal5bit((color & 0xf800) >> 11);
		c[1] = pal6bit((color & 0x07e0) >> 5);
		c[0] = pal5bit(color & 0x1f);
		return (UINT8 *)addr16;
	case NV2A_COLOR_FORMAT_UNKNOWN4:
	case NV2A_COLOR_FORMAT_X8R8G8B8:
		addr = (UINT32 *)((UINT8 *)rendertarget + offset);
		color = *addr;

		c[3] = 0xff;
		c[2] = (color >> 16) & 255;
		c[1] = (color >> 8) & 255;
		c[0] = color & 255;
		return (UINT8 *)addr;
	case NV2A_COLOR_FORMAT_A8R8G8B8:
		addr = (UINT32 *)((UINT8 *)rendertarget + offset);
		color = *addr;
		c[3] = color >> 24;
		c[2] = (color >> 16) & 255;
		c[1] = (color >> 8) & 255;
		c[0] = color & 255;
		return (UINT8 *)addr;
	case NV2A_COLOR_FORMAT_B8:
		addr8 = (UINT8 *)rendertarget + offset;
		c[0] = *addr8;
		c[1] = c[2] = 0;
		c[3] = 0xff;
		return addr8;
	}
	return NULL;
}

void nv2a_renderer::write_pixel(int x, int y, UINT32 color, UINT32 depth)
{
	UINT8 *addr;
	UINT32 *daddr32;
	UINT16 *daddr16;
	UINT32 deptsten;
	UINT32 c[4], fb[4], s[4], d[4], cc[4];
	UINT32 dep, sten, stenc, stenv;
	bool stencil_passed;
	bool depth_passed;

	fb[3] = fb[2] = fb[1] = fb[0] = 0;
	addr = NULL;
	if (color_mask != 0)
		addr = read_pixel(x, y, fb);
	if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z24S8) {
		daddr32 = depthbuffer + (pitch_depthbuffer / 4) * y + x;
		deptsten = *daddr32;
		dep = deptsten >> 8;
		sten = deptsten & 255;
		daddr16 = NULL;
	}
	else if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z16) {
		daddr16 = (UINT16 *)depthbuffer + (pitch_depthbuffer / 2) * y + x;
		deptsten = *daddr16;
		dep = (deptsten << 8) | 0xff;
		sten = 0;
		daddr32 = NULL;
	}
	else {
		daddr32 = NULL;
		daddr16 = NULL;
		dep = 0xffffff;
		sten = 0;
	}
	if (depth > 0xffffff)
		depth = 0xffffff;
	c[3] = color >> 24;
	c[2] = (color >> 16) & 255;
	c[1] = (color >> 8) & 255;
	c[0] = color & 255;
	cc[3] = blend_color >> 24;
	cc[2] = (blend_color >> 16) & 255;
	cc[1] = (blend_color >> 8) & 255;
	cc[0] = blend_color & 255;
	// ownership test and scissor test not done
	// alpha test
	if (alpha_test_enabled) {
		switch (alpha_func) {
			case nv2a_renderer::NEVER:
				return;
			case nv2a_renderer::ALWAYS:
			default:
				break;
			case nv2a_renderer::LESS:
				if (c[3] >= alpha_reference)
					return;
				break;
			case nv2a_renderer::LEQUAL:
				if (c[3] > alpha_reference)
					return;
				break;
			case nv2a_renderer::EQUAL:
				if (c[3] != alpha_reference)
					return;
				break;
			case nv2a_renderer::GEQUAL:
				if (c[3] < alpha_reference)
					return;
				break;
			case nv2a_renderer::GREATER:
				if (c[3] <= alpha_reference)
					return;
				break;
			case nv2a_renderer::NOTEQUAL:
				if (c[3] == alpha_reference)
					return;
				break;
		}
	}
	// stencil test
	stencil_passed = true;
	if (stencil_test_enabled) {
		stenc=stencil_mask & stencil_ref;
		stenv=stencil_mask & sten;
		switch (stencil_func) {
		case nv2a_renderer::NEVER:
			stencil_passed = false;
			break;
		case nv2a_renderer::LESS:
			if (stenc >= stenv)
				stencil_passed = false;
			break;
		case nv2a_renderer::EQUAL:
			if (stenc != stenv)
				stencil_passed = false;
			break;
		case nv2a_renderer::LEQUAL:
			if (stenc > stenv)
				stencil_passed = false;
			break;
		case nv2a_renderer::GREATER:
			if (stenc <= stenv)
				stencil_passed = false;
			break;
		case nv2a_renderer::NOTEQUAL:
			if (stenc == stenv)
				stencil_passed = false;
			break;
		case nv2a_renderer::GEQUAL:
			if (stenc < stenv)
				stencil_passed = false;
			break;
		case nv2a_renderer::ALWAYS:
		default:
			break;
		}
		if (stencil_passed == false) {
			switch (stencil_op_fail) {
			case nv2a_renderer::ZEROOP:
				sten = 0;
				break;
			case nv2a_renderer::INVERTOP:
				sten = sten ^ 255;
				break;
			case nv2a_renderer::KEEP:
			default:
				break;
			case nv2a_renderer::REPLACE:
				sten = stencil_ref;
				break;
			case nv2a_renderer::INCR:
				if (sten < 255)
					sten++;
				break;
			case nv2a_renderer::DECR:
				if (sten > 0)
					sten--;
				break;
			case nv2a_renderer::INCR_WRAP:
				if (sten < 255)
					sten++;
				else
					sten = 0;
				break;
			case nv2a_renderer::DECR_WRAP:
				if (sten > 0)
					sten--;
				else
					sten = 255;
				break;
			}
			if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z24S8) {
				deptsten = (dep << 8) | sten;
				*daddr32 = deptsten;
			}
			else if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z16) {
				deptsten = dep >> 8;
				*daddr16 = (UINT16)deptsten;
			}
			return;
		}
	}
	// depth buffer test
	depth_passed = true;
	if (depth_test_enabled) {
		switch (depth_function) {
			case nv2a_renderer::NEVER:
				depth_passed = false;
				break;
			case nv2a_renderer::LESS:
				if (depth >= dep)
					depth_passed = false;
				break;
			case nv2a_renderer::EQUAL:
				if (depth != dep)
					depth_passed = false;
				break;
			case nv2a_renderer::LEQUAL:
				if (depth > dep)
					depth_passed = false;
				break;
			case nv2a_renderer::GREATER:
				if (depth <= dep)
					depth_passed = false;
				break;
			case nv2a_renderer::NOTEQUAL:
				if (depth == dep)
					depth_passed = false;
				break;
			case nv2a_renderer::GEQUAL:
				if (depth < dep)
					depth_passed = false;
				break;
			case nv2a_renderer::ALWAYS:
			default:
				break;
		}
		if (depth_passed == false) {
			switch (stencil_op_zfail) {
			case nv2a_renderer::ZEROOP:
				sten = 0;
				break;
			case nv2a_renderer::INVERTOP:
				sten = sten ^ 255;
				break;
			case nv2a_renderer::KEEP:
			default:
				break;
			case nv2a_renderer::REPLACE:
				sten = stencil_ref;
				break;
			case nv2a_renderer::INCR:
				if (sten < 255)
					sten++;
				break;
			case nv2a_renderer::DECR:
				if (sten > 0)
					sten--;
				break;
			case nv2a_renderer::INCR_WRAP:
				if (sten < 255)
					sten++;
				else
					sten = 0;
				break;
			case nv2a_renderer::DECR_WRAP:
				if (sten > 0)
					sten--;
				else
					sten = 255;
				break;
			}
			if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z24S8) {
				deptsten = (dep << 8) | sten;
				*daddr32 = deptsten;
			}
			else if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z16) {
				deptsten = dep >> 8;
				*daddr16 = (UINT16)deptsten;
			}
			return;
		}
		switch (stencil_op_zpass) {
		case nv2a_renderer::ZEROOP:
			sten = 0;
			break;
		case nv2a_renderer::INVERTOP:
			sten = sten ^ 255;
			break;
		case nv2a_renderer::KEEP:
		default:
			break;
		case nv2a_renderer::REPLACE:
			sten = stencil_ref;
			break;
		case nv2a_renderer::INCR:
			if (sten < 255)
				sten++;
			break;
		case nv2a_renderer::DECR:
			if (sten > 0)
				sten--;
			break;
		case nv2a_renderer::INCR_WRAP:
			if (sten < 255)
				sten++;
			else
				sten = 0;
			break;
		case nv2a_renderer::DECR_WRAP:
			if (sten > 0)
				sten--;
			else
				sten = 255;
			break;
		}
	}
	// blending
	if (blending_enabled) {
		switch (blend_function_source) {
			case nv2a_renderer::ZERO:
				s[3] = s[2] = s[1] = s[0] = 0;
				break;
			case nv2a_renderer::ONE:
			default:
				s[3] = s[2] = s[1] = s[0] = 255;
				break;
			case nv2a_renderer::DST_COLOR:
				s[3] = fb[3];
				s[2] = fb[2];
				s[1] = fb[1];
				s[0] = fb[0];
				break;
			case nv2a_renderer::ONE_MINUS_DST_COLOR:
				s[3] = fb[3] ^ 255;
				s[2] = fb[2] ^ 255;
				s[1] = fb[1] ^ 255;
				s[0] = fb[0] ^ 255;
				break;
			case nv2a_renderer::SRC_ALPHA:
				s[3] = s[2] = s[1] = s[0] = c[3];
				break;
			case nv2a_renderer::ONE_MINUS_SRC_ALPHA:
				s[3] = s[2] = s[1] = s[0] = c[3] ^ 255;
				break;
			case nv2a_renderer::DST_ALPHA:
				s[3] = s[2] = s[1] = s[0] = fb[3];
				break;
			case nv2a_renderer::ONE_MINUS_DST_ALPHA:
				s[3] = s[2] = s[1] = s[0] = fb[3] ^ 255;
				break;
			case nv2a_renderer::CONSTANT_COLOR:
				s[3] = cc[3];
				s[2] = cc[2];
				s[1] = cc[1];
				s[0] = cc[0];
				break;
			case nv2a_renderer::ONE_MINUS_CONSTANT_COLOR:
				s[3] = cc[3] ^ 255;
				s[2] = cc[2] ^ 255;
				s[1] = cc[1] ^ 255;
				s[0] = cc[0] ^ 255;
				break;
			case nv2a_renderer::CONSTANT_ALPHA:
				s[3] = s[2] = s[1] = s[0] = cc[3];
				break;
			case nv2a_renderer::ONE_MINUS_CONSTANT_ALPHA:
				s[3] = s[2] = s[1] = s[0] = cc[3] ^ 255;
				break;
			case nv2a_renderer::SRC_ALPHA_SATURATE:
				s[3] = 255;
				if (c[3] < (fb[3] ^ 255))
					s[2] = c[3];
				else
					s[2] = fb[3];
				s[1] = s[0] = s[2];
				break;
		}
		switch (blend_function_destination) {
			case nv2a_renderer::ZERO:
			default:
				d[3] = d[2] = d[1] = d[0] = 0;
				break;
			case nv2a_renderer::ONE:
				d[3] = d[2] = d[1] = d[0] = 255;
				break;
			case nv2a_renderer::SRC_COLOR:
				d[3] = c[3];
				d[2] = c[2];
				d[1] = c[1];
				d[0] = c[0];
				break;
			case nv2a_renderer::ONE_MINUS_SRC_COLOR:
				d[3] = c[3] ^ 255;
				d[2] = c[2] ^ 255;
				d[1] = c[1] ^ 255;
				d[0] = c[0] ^ 255;
				break;
			case nv2a_renderer::SRC_ALPHA:
				d[3] = d[2] = d[1] = d[0] = c[3];
				break;
			case nv2a_renderer::ONE_MINUS_SRC_ALPHA:
				d[3] = d[2] = d[1] = d[0] = c[3] ^ 255;
				break;
			case nv2a_renderer::DST_ALPHA:
				d[3] = d[2] = d[1] = d[0] = fb[3];
				break;
			case nv2a_renderer::ONE_MINUS_DST_ALPHA:
				d[3] = d[2] = d[1] = d[0] = fb[3] ^ 255;
				break;
			case nv2a_renderer::CONSTANT_COLOR:
				d[3] = cc[3];
				d[2] = cc[2];
				d[1] = cc[1];
				d[0] = cc[0];
				break;
			case nv2a_renderer::ONE_MINUS_CONSTANT_COLOR:
				d[3] = cc[3] ^ 255;
				d[2] = cc[2] ^ 255;
				d[1] = cc[1] ^ 255;
				d[0] = cc[0] ^ 255;
				break;
			case nv2a_renderer::CONSTANT_ALPHA:
				d[3] = d[2] = d[1] = d[0] = cc[3];
				break;
			case nv2a_renderer::ONE_MINUS_CONSTANT_ALPHA:
				d[3] = d[2] = d[1] = d[0] = cc[3] ^ 255;
				break;
		}
		switch (blend_equation) {
			case nv2a_renderer::FUNC_ADD:
				c[3] = (c[3] * s[3] + fb[3] * d[3]) / 255;
				if (c[3] > 255)
					c[3] = 255;
				c[2] = (c[2] * s[2] + fb[2] * d[2]) / 255;
				if (c[2] > 255)
					c[2] = 255;
				c[1] = (c[1] * s[1] + fb[1] * d[1]) / 255;
				if (c[1] > 255)
					c[1] = 255;
				c[0] = (c[0] * s[0] + fb[0] * d[0]) / 255;
				if (c[0] > 255)
					c[0] = 255;
				break;
			case nv2a_renderer::FUNC_SUBTRACT:
				c[3] = (c[3] * s[3] - fb[3] * d[3]) / 255;
				if (c[3] < 0)
					c[3] = 255;
				c[2] = (c[2] * s[2] - fb[2] * d[2]) / 255;
				if (c[2] < 0)
					c[2] = 255;
				c[1] = (c[1] * s[1] - fb[1] * d[1]) / 255;
				if (c[1] < 0)
					c[1] = 255;
				c[0] = (c[0] * s[0] - fb[0] * d[0]) / 255;
				if (c[0] < 0)
					c[0] = 255;
				break;
			case nv2a_renderer::FUNC_REVERSE_SUBTRACT:
				c[3] = (fb[3] * d[3] - c[3] * s[3]) / 255;
				if (c[3] < 0)
					c[3] = 255;
				c[2] = (fb[2] * d[2] - c[2] * s[2]) / 255;
				if (c[2] < 0)
					c[2] = 255;
				c[1] = (fb[1] * d[1] - c[1] * s[1]) / 255;
				if (c[1] < 0)
					c[1] = 255;
				c[0] = (fb[0] * d[0] - c[0] * s[0]) / 255;
				if (c[0] < 0)
					c[0] = 255;
				break;
			case nv2a_renderer::MIN:
				c[3] = s[3];
				if (d[3] < c[3])
					c[3] = d[3];
				c[2] = s[2];
				if (d[2] < c[2])
					c[2] = d[2];
				c[1] = s[1];
				if (d[1] < c[1])
					c[1] = d[1];
				c[0] = s[0];
				if (d[0] < c[0])
					c[0] = d[0];
				break;
			case nv2a_renderer::MAX:
				c[3] = s[3];
				if (d[3] > c[3])
					c[3] = d[3];
				c[2] = s[2];
				if (d[2] > c[2])
					c[2] = d[2];
				c[1] = s[1];
				if (d[1] > c[1])
					c[1] = d[1];
				c[0] = s[0];
				if (d[0] > c[0])
					c[0] = d[0];
				break;
		}
	}
	// dithering not done
	// logical operation
	if (logical_operation_enabled) {
		switch (logical_operation) {
			case  nv2a_renderer::CLEAR:
				c[3] = 0;
				c[2] = 0;
				c[1] = 0;
				c[0] = 0;
				break;
			case  nv2a_renderer::AND:
				c[3] = c[3] & fb[3];
				c[2] = c[2] & fb[2];
				c[1] = c[1] & fb[1];
				c[0] = c[0] & fb[0];
				break;
			case  nv2a_renderer::AND_REVERSE:
				c[3] = c[3] & (fb[3] ^ 255);
				c[2] = c[2] & (fb[2] ^ 255);
				c[1] = c[1] & (fb[1] ^ 255);
				c[0] = c[0] & (fb[0] ^ 255);
				break;
			case  nv2a_renderer::COPY:
			default:
				break;
			case  nv2a_renderer::AND_INVERTED:
				c[3] = (c[3] ^ 255) & fb[3];
				c[2] = (c[2] ^ 255) & fb[2];
				c[1] = (c[1] ^ 255) & fb[1];
				c[0] = (c[0] ^ 255) & fb[0];
				break;
			case  nv2a_renderer::NOOP:
				c[3] = fb[3];
				c[2] = fb[2];
				c[1] = fb[1];
				c[0] = fb[0];
				break;
			case  nv2a_renderer::XOR:
				c[3] = c[3] ^ fb[3];
				c[2] = c[2] ^ fb[2];
				c[1] = c[1] ^ fb[1];
				c[0] = c[0] ^ fb[0];
				break;
			case  nv2a_renderer::OR:
				c[3] = c[3] | fb[3];
				c[2] = c[2] | fb[2];
				c[1] = c[1] | fb[1];
				c[0] = c[0] | fb[0];
				break;
			case  nv2a_renderer::NOR:
				c[3] = (c[3] | fb[3]) ^ 255;
				c[2] = (c[2] | fb[2]) ^ 255;
				c[1] = (c[1] | fb[1]) ^ 255;
				c[0] = (c[0] | fb[0]) ^ 255;
				break;
			case  nv2a_renderer::EQUIV:
				c[3] = (c[3] ^ fb[3]) ^ 255;
				c[2] = (c[2] ^ fb[2]) ^ 255;
				c[1] = (c[1] ^ fb[1]) ^ 255;
				c[0] = (c[0] ^ fb[0]) ^ 255;
				break;
			case  nv2a_renderer::INVERT:
				c[3] = fb[3] ^ 255;
				c[2] = fb[2] ^ 255;
				c[1] = fb[1] ^ 255;
				c[0] = fb[0] ^ 255;
				break;
			case  nv2a_renderer::OR_REVERSE:
				c[3] = c[3] | (fb[3] ^ 255);
				c[2] = c[2] | (fb[2] ^ 255);
				c[1] = c[1] | (fb[1] ^ 255);
				c[0] = c[0] | (fb[0] ^ 255);
				break;
			case  nv2a_renderer::COPY_INVERTED:
				c[3] = c[3] ^ 255;
				c[2] = c[2] ^ 255;
				c[1] = c[1] ^ 255;
				c[0] = c[0] ^ 255;
				break;
			case  nv2a_renderer::OR_INVERTED:
				c[3] = (c[3] ^ 255) | fb[3];
				c[2] = (c[2] ^ 255) | fb[2];
				c[1] = (c[1] ^ 255) | fb[1];
				c[0] = (c[0] ^ 255) | fb[0];
				break;
			case  nv2a_renderer::NAND:
				c[3] = (c[3] & fb[3]) ^ 255;
				c[2] = (c[2] & fb[2]) ^ 255;
				c[1] = (c[1] & fb[1]) ^ 255;
				c[0] = (c[0] & fb[0]) ^ 255;
				break;
			case  nv2a_renderer::SET:
				c[3] = 255;
				c[2] = 255;
				c[1] = 255;
				c[0] = 255;
				break;
		}
	}
	if (color_mask != 0) {
		UINT32 ct,ft,w;

		ct = (c[3] << 24) | (c[2] << 16) | (c[1] << 8) | c[0];
		ft = (fb[3] << 24) | (fb[2] << 16) | (fb[1] << 8) | fb[0];
		w = (ft & ~color_mask) | (ct & color_mask);
		switch (colorformat_rendertarget) {
		case NV2A_COLOR_FORMAT_R5G6B5:
			w = ((w >> 8) & 0xf800) + ((w >> 5) & 0x7e0) + ((w >> 3) & 0x1f);
			*((UINT16 *)addr) = (UINT16)w;
			break;
		case NV2A_COLOR_FORMAT_UNKNOWN4:
		case NV2A_COLOR_FORMAT_X8R8G8B8:
			*((UINT32 *)addr) = w;
			break;
		case NV2A_COLOR_FORMAT_A8R8G8B8:
			*((UINT32 *)addr) = w;
			break;
		case NV2A_COLOR_FORMAT_B8:
			*addr = (UINT8)w;
			break;
		}
	}
	if (depth_write_enabled)
		dep = depth;
	if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z24S8) {
		deptsten = (dep << 8) | sten;
		*daddr32 = deptsten;
	}
	else if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z16) {
		deptsten = dep >> 8;
		*daddr16 = (UINT16)deptsten;
	}
}

void nv2a_renderer::render_color(INT32 scanline, const extent_t &extent, const nvidia_object_data &objectdata, int threadid)
{
	int x;

	if ((extent.startx < 0) || (extent.stopx > 640))
		return;
	x = extent.stopx - extent.startx - 1; // number of pixels to draw
	while (x >= 0) {
		UINT32 a8r8g8b8;
		UINT32 z;
		int ca, cr, cg, cb;
		int xp = extent.startx + x; // x coordinate of current pixel

		cb = ((extent.param[PARAM_COLOR_B].start + (float)x*extent.param[PARAM_COLOR_B].dpdx))*255.0f;
		cg = ((extent.param[PARAM_COLOR_G].start + (float)x*extent.param[PARAM_COLOR_G].dpdx))*255.0f;
		cr = ((extent.param[PARAM_COLOR_R].start + (float)x*extent.param[PARAM_COLOR_R].dpdx))*255.0f;
		ca = ((extent.param[PARAM_COLOR_A].start + (float)x*extent.param[PARAM_COLOR_A].dpdx))*255.0f;
		a8r8g8b8 = (ca << 24) + (cr << 16) + (cg << 8) + cb; // pixel color obtained by interpolating the colors of the vertices
		z = (extent.param[PARAM_Z].start + (float)x*extent.param[PARAM_Z].dpdx);
		write_pixel(xp, scanline, a8r8g8b8, z);
		x--;
	}
}

void nv2a_renderer::render_texture_simple(INT32 scanline, const extent_t &extent, const nvidia_object_data &objectdata, int threadid)
{
	int x;
	UINT32 a8r8g8b8;
	UINT32 z;

	if (!objectdata.data->texture[0].enabled) {
		return;
	}
	if ((extent.startx < 0) || (extent.stopx > 640))
		return;
	x = extent.stopx - extent.startx - 1;
	while (x >= 0) {
		int up, vp;
		int xp = extent.startx + x; // x coordinate of current pixel

		up = (extent.param[PARAM_TEXTURE0_U].start + (float)x*extent.param[PARAM_TEXTURE0_U].dpdx)*(float)(objectdata.data->texture[0].sizeu - 1); // x coordinate of texel in texture
		vp = (extent.param[PARAM_TEXTURE0_V].start + (float)x*extent.param[PARAM_TEXTURE0_V].dpdx)*(float)(objectdata.data->texture[0].sizev - 1); // y coordinate of texel in texture
		a8r8g8b8 = texture_get_texel(0, up, vp);
		z = (extent.param[PARAM_Z].start + (float)x*extent.param[PARAM_Z].dpdx);
		write_pixel(xp, scanline, a8r8g8b8, z);
		x--;
	}
}

void nv2a_renderer::render_register_combiners(INT32 scanline, const extent_t &extent, const nvidia_object_data &objectdata, int threadid)
{
	int x, xp;
	int up, vp;
	int ca, cr, cg, cb;
	UINT32 color[6];
	UINT32 a8r8g8b8;
	UINT32 z;
	int n;//,m,i,j,k;

	color[0] = color[1] = color[2] = color[3] = color[4] = color[5] = 0;

	if ((extent.startx < 0) || (extent.stopx > 640))
		return;
	osd_lock_acquire(combiner.lock); // needed since multithreading is not supported yet
	x = extent.stopx - extent.startx - 1; // number of pixels to draw
	while (x >= 0) {
		xp = extent.startx + x;
		// 1: fetch data
		// 1.1: interpolated color from vertices
		cb = ((extent.param[PARAM_COLOR_B].start + (float)x*extent.param[PARAM_COLOR_B].dpdx))*255.0f;
		cg = ((extent.param[PARAM_COLOR_G].start + (float)x*extent.param[PARAM_COLOR_G].dpdx))*255.0f;
		cr = ((extent.param[PARAM_COLOR_R].start + (float)x*extent.param[PARAM_COLOR_R].dpdx))*255.0f;
		ca = ((extent.param[PARAM_COLOR_A].start + (float)x*extent.param[PARAM_COLOR_A].dpdx))*255.0f;
		color[0] = (ca << 24) + (cr << 16) + (cg << 8) + cb; // pixel color obtained by interpolating the colors of the vertices
		color[1] = 0; // lighting not yet
		// 1.2: color for each of the 4 possible textures
		for (n = 0; n < 4; n++) {
			if (texture[n].enabled) {
				up = (extent.param[PARAM_TEXTURE0_U + n * 2].start + (float)x*extent.param[PARAM_TEXTURE0_U + n * 2].dpdx)*(float)(objectdata.data->texture[n].sizeu - 1);
				vp = extent.param[PARAM_TEXTURE0_V + n * 2].start*(float)(objectdata.data->texture[n].sizev - 1);
				color[n + 2] = texture_get_texel(n, up, vp);
			}
		}
		// 2: compute
		// 2.1: initialize
		combiner_initialize_registers(color);
		// 2.2: general cmbiner stages
		for (n = 0; n < combiner.stages; n++) {
			// 2.2.1 initialize
			combiner_initialize_stage(n);
			// 2.2.2 map inputs
			combiner_map_input(n);
			// 2.2.3 compute possible outputs
			combiner_compute_rgb_outputs(n);
			combiner_compute_a_outputs(n);
			// 2.2.4 map outputs to registers
			combiner_map_output(n);
		}
		// 2.3: final cmbiner stage
		combiner_initialize_final();
		combiner_map_final_input();
		combiner_final_output();
		a8r8g8b8 = combiner_float_argb8(combiner.output);
		// 3: write pixel
		z = (extent.param[PARAM_Z].start + (float)x*extent.param[PARAM_Z].dpdx);
		write_pixel(xp, scanline, a8r8g8b8, z);
		x--;
	}
	osd_lock_release(combiner.lock);
}

#if 0
const char *rc_mapping_str[] = {
	"UNSIGNED_IDENTITY",
	"UNSIGNED_INVERT",
	"EXPAND_NORMAL",
	"EXPAND_NEGATE",
	"HALF_BIAS_NORMAL",
	"HALF_BIAS_NEGATE",
	"SIGNED_IDENTITY",
	"SIGNED_NEGATE"
};

const char *rc_usage_rgb_str[] = {
	"RGB",
	"ALPHA"
};

const char *rc_usage_alpha_str[] = {
	"BLUE",
	"ALPHA"
};

const char *rc_variable_str[] = {
	"ZERO",
	"CONSTANT_COLOR0",
	"CONSTANT_COLOR1",
	"FOG",
	"PRIMARY_COLOR",
	"SECONDARY_COLOR",
	"???",
	"???",
	"TEXTURE0",
	"TEXTURE1",
	"TEXTURE2",
	"TEXTURE3",
	"SPARE0",
	"SPARE1",
	"SPARE0_PLUS_SECONDARY_COLOR",
	"E_TIMES_F"
};

const char *rc_bias_str[] = {
	"NONE",
	"BIAS_BY_NEGATIVE_ONE_HALF"
};

const char *rc_scale_str[] = {
	"NONE",
	"SCALE_BY_TWO",
	"SCALE_BY_FOUR",
	"SCALE_BY_ONE_HALF"
};

/* Dump the current setup of the register combiners */
void dumpcombiners(UINT32 *m)
{
	int a, b, n, v;

	n = m[0x1e60 / 4] & 0xf;
	printf("Combiners active: %d\n\r", n);
	for (a = 0; a < n; a++) {
		printf("Combiner %d\n\r", a + 1);
		printf(" RC_IN_ALPHA %08X\n\r", m[0x0260 / 4 + a]);
		for (b = 24; b >= 0; b = b - 8) {
			v = (m[0x0260 / 4 + a] >> b) & 0xf;
			printf("  %c_INPUT %s\n\r", 'A' + 3 - b / 8, rc_variable_str[v]);
			v = (m[0x0260 / 4 + a] >> (b + 4)) & 1;
			printf("  %c_COMPONENT_USAGE %s\n\r", 'A' + 3 - b / 8, rc_usage_alpha_str[v]);
			v = (m[0x0260 / 4 + a] >> (b + 5)) & 7;
			printf("  %c_MAPPING %s\n\r", 'A' + 3 - b / 8, rc_mapping_str[v]);
		}
		printf(" RC_IN_RGB %08X\n\r", m[0x0ac0 / 4 + a]);
		for (b = 24; b >= 0; b = b - 8) {
			v = (m[0x0ac0 / 4 + a] >> b) & 0xf;
			printf("  %c_INPUT %s\n\r", 'A' + 3 - b / 8, rc_variable_str[v]);
			v = (m[0x0ac0 / 4 + a] >> (b + 4)) & 1;
			printf("  %c_COMPONENT_USAGE %s\n\r", 'A' + 3 - b / 8, rc_usage_rgb_str[v]);
			v = (m[0x0ac0 / 4 + a] >> (b + 5)) & 7;
			printf("  %c_MAPPING %s\n\r", 'A' + 3 - b / 8, rc_mapping_str[v]);
		}
		printf(" RC_OUT_ALPHA %08X\n\r", m[0x0aa0 / 4 + a]);
		v = m[0x0aa0 / 4 + a] & 0xf;
		printf("  CD_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x0aa0 / 4 + a] >> 4) & 0xf;
		printf("  AB_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x0aa0 / 4 + a] >> 8) & 0xf;
		printf("  SUM_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x0aa0 / 4 + a] >> 12) & 1;
		printf("  CD_DOT_PRODUCT %d\n\r", v);
		v = (m[0x0aa0 / 4 + a] >> 13) & 1;
		printf("  AB_DOT_PRODUCT %d\n\r", v);
		v = (m[0x0aa0 / 4 + a] >> 14) & 1;
		printf("  MUX_SUM %d\n\r", v);
		v = (m[0x0aa0 / 4 + a] >> 15) & 1;
		printf("  BIAS %s\n\r", rc_bias_str[v]);
		v = (m[0x0aa0 / 4 + a] >> 16) & 3;
		printf("  SCALE %s\n\r", rc_scale_str[v]);
		//v=(m[0x0aa0/4+a] >> 27) & 7;
		printf(" RC_OUT_RGB %08X\n\r", m[0x1e40 / 4 + a]);
		v = m[0x1e40 / 4 + a] & 0xf;
		printf("  CD_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x1e40 / 4 + a] >> 4) & 0xf;
		printf("  AB_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x1e40 / 4 + a] >> 8) & 0xf;
		printf("  SUM_OUTPUT %s\n\r", rc_variable_str[v]);
		v = (m[0x1e40 / 4 + a] >> 12) & 1;
		printf("  CD_DOT_PRODUCT %d\n\r", v);
		v = (m[0x1e40 / 4 + a] >> 13) & 1;
		printf("  AB_DOT_PRODUCT %d\n\r", v);
		v = (m[0x1e40 / 4 + a] >> 14) & 1;
		printf("  MUX_SUM %d\n\r", v);
		v = (m[0x1e40 / 4 + a] >> 15) & 1;
		printf("  BIAS %s\n\r", rc_bias_str[v]);
		v = (m[0x1e40 / 4 + a] >> 16) & 3;
		printf("  SCALE %s\n\r", rc_scale_str[v]);
		//v=(m[0x1e40/4+a] >> 27) & 7;
		printf("\n\r");
	}
	printf("Combiner final %08X %08X\n\r", m[0x0288 / 4], m[0x028c / 4]);
	for (a = 24; a >= 0; a = a - 8) {
		n = (m[0x0288 / 4] >> a) & 0xf;
		printf("  %c_INPUT %s\n\r", 'A' + 3 - a / 8, rc_variable_str[n]);
		n = (m[0x0288 / 4] >> (a + 4)) & 1;
		printf("  %c_COMPONENT_USAGE %s\n\r", 'A' + 3 - a / 8, rc_usage_rgb_str[n]);
		n = (m[0x0288 / 4] >> (a + 5)) & 7;
		printf("  %c_MAPPING %s\n\r", 'A' + 3 - a / 8, rc_mapping_str[n]);
	}
	for (a = 24; a >= 8; a = a - 8) {
		n = (m[0x028c / 4] >> a) & 0xf;
		printf("  %c_INPUT %s\n\r", 'E' + 3 - a / 8, rc_variable_str[n]);
		n = (m[0x028c / 4] >> (a + 4)) & 1;
		printf("  %c_COMPONENT_USAGE %s\n\r", 'E' + 3 - a / 8, rc_usage_rgb_str[n]);
		n = (m[0x028c / 4] >> (a + 5)) & 7;
		printf("  %c_MAPPING %s\n\r", 'E' + 3 - a / 8, rc_mapping_str[n]);
	}
	n = (m[0x028c / 4] >> 7) & 1;
	printf(" color sum clamp: %d\n\r", n);
}
#endif

void nv2a_renderer::read_vertex(address_space & space, offs_t address, vertex_nv &vertex, int attrib)
{
	UINT32 u;
	int c, d, l;

	l = vertexbuffer_size[attrib];
	switch (vertexbuffer_kind[attrib]) {
	case NV2A_VTXBUF_TYPE_FLOAT:
	default:
		vertex.attribute[attrib].fv[0] = 0;
		vertex.attribute[attrib].fv[1] = 0;
		vertex.attribute[attrib].fv[2] = 0;
		vertex.attribute[attrib].fv[3] = 1.0;
		for (c = d = 0; c < l; c++) {
			vertex.attribute[attrib].iv[c] = space.read_dword(address + d);
			d = d + 4;
		}
		break;
	case NV2A_VTXBUF_TYPE_UBYTE:
		u = space.read_dword(address + 0);
		for (c = l-1; c >= 0; c--) {
			vertex.attribute[attrib].fv[c] = (u & 0xff) / 255.0;
			u = u >> 8;
		}
		break;
	case  NV2A_VTXBUF_TYPE_UBYTE2:
		u = space.read_dword(address + 0);
		for (c = 0; c < l; c++) {
			vertex.attribute[attrib].fv[c] = (u & 0xff) / 255.0;
			u = u >> 8;
		}
		break;
	case NV2A_VTXBUF_TYPE_UNKNOWN_6: // ???
		u = space.read_dword(address + 0);
		vertex.attribute[attrib].fv[0] = (u & 0xff) / 255.0; // b
		vertex.attribute[attrib].fv[1] = ((u & 0xff00) >> 8) / 255.0;  // g
		vertex.attribute[attrib].fv[2] = ((u & 0xff0000) >> 16) / 255.0;  // r
		vertex.attribute[attrib].fv[3] = ((u & 0xff000000) >> 24) / 255.0;  // a
		break;
	}
}

/* Read vertices data from system memory. Method 0x1800 */
int nv2a_renderer::read_vertices_0x1800(address_space & space, vertex_nv *destination, UINT32 address, int limit)
{
	UINT32 data;
	UINT32 m, i, c;
	int a, b;

#ifdef MAME_DEBUG
	memset(destination, 0, sizeof(vertex_nv)*limit);
#endif
	c = 0;
	for (m = 0; m < limit; m++) {
		if (indexesleft_count == 0) {
			data = space.read_dword(address);
			i = (indexesleft_first + indexesleft_count) & 7;
			indexesleft[i] = data & 0xffff;
			indexesleft[(i + 1) & 7] = (data >> 16) & 0xffff;
			indexesleft_count = indexesleft_count + 2;
			address += 4;
			c++;
		}
		b = enabled_vertex_attributes;
		for (a = 0; a < 16; a++) {
			if (b & 1) {
				read_vertex(space, vertexbuffer_address[a] + indexesleft[indexesleft_first] * vertexbuffer_stride[a], destination[m], a);
			}
			b = b >> 1;
		}
		indexesleft_first = (indexesleft_first + 1) & 7;
		indexesleft_count--;
	}
	return (int)c;
}

/* Read vertices data from system memory. Method 0x1808 */
int nv2a_renderer::read_vertices_0x1808(address_space & space, vertex_nv *destination, UINT32 address, int limit)
{
	UINT32 data;
	UINT32 m, i, c;
	int a, b;

#ifdef MAME_DEBUG
	memset(destination, 0, sizeof(vertex_nv)*limit);
#endif
	c = 0;
	for (m = 0; m < limit; m++) {
		if (indexesleft_count == 0) {
			data = space.read_dword(address);
			i = (indexesleft_first + indexesleft_count) & 7;
			indexesleft[i] = data;
			indexesleft_count = indexesleft_count + 1;
			address += 4;
			c++;
		}
		b = enabled_vertex_attributes;
		for (a = 0; a < 16; a++) {
			if (b & 1) {
				read_vertex(space, vertexbuffer_address[a] + indexesleft[indexesleft_first] * vertexbuffer_stride[a], destination[m], a);
			}
			b = b >> 1;
		}
		indexesleft_first = (indexesleft_first + 1) & 7;
		indexesleft_count--;
	}
	return (int)c;
}

/* Read vertices data from system memory. Method 0x1810 */
int nv2a_renderer::read_vertices_0x1810(address_space & space, vertex_nv *destination, int offset, int limit)
{
	UINT32 m;
	int a, b;

#ifdef MAME_DEBUG
	memset(destination, 0, sizeof(vertex_nv)*limit);
#endif
	for (m = 0; m < limit; m++) {
		b = enabled_vertex_attributes;
		for (a = 0; a < 16; a++) {
			if (b & 1) {
				read_vertex(space, vertexbuffer_address[a] + (m + offset)*vertexbuffer_stride[a], destination[m], a);
			}
			b = b >> 1;
		}
	}
	return m;
}

/* Read vertices data from system memory. Method 0x1818 */
int nv2a_renderer::read_vertices_0x1818(address_space & space, vertex_nv *destination, UINT32 address, int limit)
{
	UINT32 m, vwords;
	int a, b;

#ifdef MAME_DEBUG
	memset(destination, 0, sizeof(vertex_nv)*limit);
#endif
	vwords = vertex_attribute_words[15] + vertex_attribute_offset[15];
	for (m = 0; m < limit; m++) {
		b = enabled_vertex_attributes;
		for (a = 0; a < 16; a++) {
			if (b & 1) {
				read_vertex(space, address + vertex_attribute_offset[a] * 4, destination[m], a);
			}
			b = b >> 1;
		}
		address = address + vwords * 4;
	}
	return (int)(m*vwords);
}

void nv2a_renderer::convert_vertices_poly(vertex_nv *source, vertex_t *destination, int count)
{
	vertex_nv vert[4];
	int m, u;

	// take each vertex with its attributes and obtain data for drawing
	// should use either the vertex program or transformation matrices
	if (vertex_pipeline == 4) {
		// transformation matrices
		// it is not implemented, so we pretend its always using screen coordinates
		for (m = 0; m < count; m++) {
			destination[m].x = source[m].attribute[0].fv[0];
			destination[m].y = source[m].attribute[0].fv[1];
			for (u = PARAM_COLOR_B; u <= PARAM_COLOR_A; u++) // 0=b 1=g 2=r 3=a
				destination[m].p[u] = source[m].attribute[3].fv[u];
			for (u = 0; u < 4; u++) {
				destination[m].p[PARAM_TEXTURE0_U + u * 2] = source[m].attribute[9 + u].fv[0];
				destination[m].p[PARAM_TEXTURE0_V + u * 2] = source[m].attribute[9 + u].fv[1];
			}
			destination[m].p[PARAM_Z] = 0xffffff;
		}
	}
	else {
		// vertex program
		// run vertex program
		vertexprogram.exec.process(vertexprogram.start_instruction, source, vert, count);
		// copy data for poly.c
		for (m = 0; m < count; m++) {
			destination[m].x = vert[m].attribute[0].fv[0];
			destination[m].y = vert[m].attribute[0].fv[1];
			for (u = PARAM_COLOR_B; u <= PARAM_COLOR_A; u++) // 0=b 1=g 2=r 3=a
				destination[m].p[u] = vert[m].attribute[3].fv[u];
			for (u = 0; u < 4; u++) {
				destination[m].p[PARAM_TEXTURE0_U + u * 2] = vert[m].attribute[9 + u].fv[0];
				destination[m].p[PARAM_TEXTURE0_V + u * 2] = vert[m].attribute[9 + u].fv[1];
			}
			destination[m].p[PARAM_Z] = vert[m].attribute[0].fv[2];
		}
	}
}

void nv2a_renderer::clear_depth_buffer(int what, UINT32 value)
{
	int m;

	if (what == 0)
		return;
	m = antialias_control;
	if (antialiasing_rendertarget != 0)
		m = 2;
	else
		m = 1;
	if (what == 3) {
		if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z24S8) {
			UINT32 *p, *pl;
			int x, y;

			pl = (UINT32 *)depthbuffer;
			for (y = (limits_rendertarget.bottom() + 1) * m; y != 0; y--) {
				p = pl;
				for (x = (limits_rendertarget.right() + 1) * m; x != 0; x--) {
					*p = value;
					p++;
				}
				pl = pl + pitch_rendertarget / 4;
			}
		}
		else if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z16) {
			UINT16 *p, *pl;
			int x, y;

			pl = (UINT16 *)depthbuffer;
			for (y = (limits_rendertarget.bottom() + 1) * m; y != 0; y--) {
				p = pl;
				for (x = (limits_rendertarget.right() + 1) * m; x != 0; x--) {
					*p = (UINT16)value;
					p++;
				}
				pl = pl + pitch_rendertarget / 2;
			}
		}
	}
	else {
		if (depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z24S8) {
			UINT32 mask;
			UINT32 *p, *pl;
			int x, y;

			if ((what & 0x03) == 2)
				mask = 0x000000ff;
			else
				mask = 0xffffff00;
			value = value & mask;
			pl = depthbuffer;
			for (y = (limits_rendertarget.bottom() + 1) * m; y != 0; y--) {
				p = pl;
				for (x = (limits_rendertarget.right() + 1) * m; x != 0; x--) {
					*p = (*p & ~mask) | value;
					p++;
				}
				pl = pl + pitch_rendertarget / 4;
			}
		}
		else if ((depthformat_rendertarget == NV2A_RT_DEPTH_FORMAT_Z16) && (what == 1)) {
			UINT16 *p, *pl;
			int x, y;

			pl = (UINT16 *)depthbuffer;
			for (y = (limits_rendertarget.bottom() + 1) * m; y != 0; y--) {
				p = pl;
				for (x = (limits_rendertarget.right() + 1) * m; x != 0; x--) {
					*p = (UINT16)value;
					p++;
				}
				pl = pl + pitch_rendertarget / 2;
			}
		}
	}
}

int nv2a_renderer::geforce_exec_method(address_space & space, UINT32 chanel, UINT32 subchannel, UINT32 method, UINT32 address, int &countlen)
{
	UINT32 maddress;
	UINT32 data;

	maddress = method * 4;
	data = space.read_dword(address);
	channel[chanel][subchannel].object.method[method] = data;
#ifdef LOG_NV2A
	printf("A:%08X MTHD:%08X D:%08X\n\r",address,maddress,data);
#endif
	if (maddress == 0x17fc) {
		indexesleft_count = 0;
		indexesleft_first = 0;
		primitives_count = 0;
		countlen--;
	}
	if (maddress == 0x1810) {
		// draw vertices
		int offset, count, type;
		UINT32 n;
		render_delegate renderspans;

		offset = data & 0xffffff;
		count = (data >> 24) & 0xff;
		type = channel[chanel][subchannel].object.method[0x17fc / 4];
		if (((channel[chanel][subchannel].object.method[0x1e60 / 4] & 7) > 0) && (combiner.used != 0)) {
			renderspans = render_delegate(FUNC(nv2a_renderer::render_register_combiners), this);
		}
		else if (texture[0].enabled) {
			renderspans = render_delegate(FUNC(nv2a_renderer::render_texture_simple), this);
		}
		else
			renderspans = render_delegate(FUNC(nv2a_renderer::render_color), this);
#ifdef LOG_NV2A
		printf("vertex %d %d %d\n\r", type, offset, count);
#endif
		if (type == nv2a_renderer::QUADS) {
			for (n = 0; n <= count; n += 4) {
				vertex_nv vert[4];
				vertex_t xy[4];

				read_vertices_0x1810(space, vert, n + offset, 4);
				convert_vertices_poly(vert, xy, 4);
				render_polygon<4>(limits_rendertarget, renderspans, 4 + 4 * 2, xy); // 4 rgba, 4 texture units 2 uv
				wait();
			}
		}
		else if (type == nv2a_renderer::TRIANGLE_FAN) {
			vertex_nv vert[3];
			vertex_t xy[3];

			read_vertices_0x1810(space, vert, offset, 2);
			convert_vertices_poly(vert, xy, 2);
			count = count - 2;
			offset = offset + 2;
			for (n = 0; n <= count; n++) {
				read_vertices_0x1810(space, vert + (((n + 1) & 1) + 1), offset + n, 1);
				convert_vertices_poly(vert + (((n + 1) & 1) + 1), xy + (((n + 1) & 1) + 1), 1);
				render_triangle(limits_rendertarget, renderspans, 4 + 4 * 2, xy[0], xy[(~(n + 1) & 1) + 1], xy[((n + 1) & 1) + 1]);
				wait();
			}
		}
		else if (type == nv2a_renderer::TRIANGLE_STRIP) {
			vertex_nv vert[4];
			vertex_t xy[4];

			read_vertices_0x1810(space, vert, offset, 2);
			convert_vertices_poly(vert, xy, 2);
			count = count - 2;
			offset = offset + 2;
			for (n = 0; n <= count; n++) {
				read_vertices_0x1810(space, vert + ((n + 2) & 3), offset + n, 1);
				convert_vertices_poly(vert + ((n + 2) & 3), xy + ((n + 2) & 3), 1);
				render_triangle(limits_rendertarget, renderspans, 4 + 4 * 2, xy[((n & 1) + n) & 3], xy[((~n & 1) + n) & 3], xy[(2 + n) & 3]);
				wait();
			}
		}
		else {
			logerror("Unsupported primitive %d for method 0x1810\n", type);
		}
		countlen--;
	}
	if ((maddress == 0x1800) || (maddress == 0x1808)) {
		UINT32 type, n;
		render_delegate renderspans;
		int mult;

		if (((channel[chanel][subchannel].object.method[0x1e60 / 4] & 7) > 0) && (combiner.used != 0)) {
			renderspans = render_delegate(FUNC(nv2a_renderer::render_register_combiners), this);
		}
		else if (texture[0].enabled) {
			renderspans = render_delegate(FUNC(nv2a_renderer::render_texture_simple), this);
		}
		else
			renderspans = render_delegate(FUNC(nv2a_renderer::render_color), this);
		if (maddress == 0x1800)
			mult = 2;
		else
			mult = 1;
		// vertices are selected from the vertex buffer using an array of indexes
		// each dword after 1800 contains two 16 bit index values to select the vartices
		// each dword after 1808 contains a 32 bit index value to select the vartices
		type = channel[chanel][subchannel].object.method[0x17fc / 4];
		if (type == nv2a_renderer::QUADS) {
			while (1) {
				vertex_nv vert[4];
				vertex_t xy[4];
				int c;

				if ((countlen * mult + indexesleft_count) < 4)
					break;
				if (mult == 1)
					c = read_vertices_0x1808(space, vert, address, 4);
				else
					c = read_vertices_0x1800(space, vert, address, 4);
				address = address + c * 4;
				countlen = countlen - c;
				convert_vertices_poly(vert, xy, 4);
				render_polygon<4>(limits_rendertarget, renderspans, 4 + 4 * 2, xy); // 4 rgba, 4 texture units 2 uv
				wait();
			}
		}
		else if (type == nv2a_renderer::TRIANGLE_FAN) {
			if ((countlen * mult + indexesleft_count) >= 3) {
				vertex_nv vert[3];
				vertex_t xy[3];
				int c, count;

				if (mult == 1)
					c = read_vertices_0x1808(space, vert, address, 2);
				else
					c = read_vertices_0x1800(space, vert, address, 2);
				convert_vertices_poly(vert, xy, 2);
				address = address + c * 4;
				countlen = countlen - c;
				count = countlen * mult + indexesleft_count;
				for (n = 1; n <= count; n++) {
					if (mult == 1)
						c = read_vertices_0x1808(space, vert + ((n & 1) + 1), address, 1);
					else
						c = read_vertices_0x1800(space, vert + ((n & 1) + 1), address, 1);

					convert_vertices_poly(vert + ((n & 1) + 1), xy + ((n & 1) + 1), 1);
					address = address + c * 4;
					countlen = countlen - c;
					render_triangle(limits_rendertarget, renderspans, 4 + 4 * 2, xy[0], xy[(~n & 1) + 1], xy[(n & 1) + 1]);
					wait();
				}
			}
		}
		else if (type == nv2a_renderer::TRIANGLES) {
			while (1) {
				vertex_nv vert[3];
				vertex_t xy[3];
				int c;

				if ((countlen * mult + indexesleft_count) < 3)
					break;
				if (mult == 1)
					c = read_vertices_0x1808(space, vert, address, 3);
				else
					c = read_vertices_0x1800(space, vert, address, 3);
				address = address + c * 4;
				countlen = countlen - c;
				convert_vertices_poly(vert, xy, 3);
				render_triangle(limits_rendertarget, renderspans, 4 + 4 * 2, xy[0], xy[1], xy[2]); // 4 rgba, 4 texture units 2 uv
				wait();
			}
		}
		else if (type == nv2a_renderer::TRIANGLE_STRIP) {
			if ((countlen * mult + indexesleft_count) >= 3) {
				vertex_nv vert[4];
				vertex_t xy[4];
				int c, count;

				if (mult == 1)
					c = read_vertices_0x1808(space, vert, address, 2);
				else
					c = read_vertices_0x1800(space, vert, address, 2);
				convert_vertices_poly(vert, xy, 2);
				address = address + c * 4;
				countlen = countlen - c;
				count = countlen * mult + indexesleft_count;
				for (n = 0; n < count; n++) {
					if (mult == 1)
						c = read_vertices_0x1808(space, vert + ((n + 2) & 3), address, 1);
					else
						c = read_vertices_0x1800(space, vert + ((n + 2) & 3), address, 1);
					address = address + c * 4;
					countlen = countlen - c;
					convert_vertices_poly(vert + ((n + 2) & 3), xy + ((n + 2) & 3), 1);
					if (xy[(n + 2) & 3].y > 293800000.0f)
						xy[(n + 2) & 3].y = xy[(n + 2) & 3].y + 1.0f;
					render_triangle(limits_rendertarget, renderspans, 4 + 4 * 2, xy[((n & 1) + n) & 3], xy[((~n & 1) + n) & 3], xy[(2 + n) & 3]);
					wait();
				}
			}
		}
		else {
			logerror("Unsupported primitive %d for method 0x1800/8\n", type);
			countlen = 0;
		}
		while (countlen > 0) {
			data = space.read_dword(address);
			n = (indexesleft_first + indexesleft_count) & 7;
			if (mult == 2) {
				indexesleft[n] = data & 0xffff;
				indexesleft[(n + 1) & 7] = (data >> 16) & 0xffff;
				indexesleft_count = indexesleft_count + 2;
			}
			else {
				indexesleft[n] = data;
				indexesleft_count = indexesleft_count + 1;
			}
			address += 4;
			countlen--;
		}
		wait();
	}
	if (maddress == 0x1818) {
		int n;
		int type;
		render_delegate renderspans;

		if (((channel[chanel][subchannel].object.method[0x1e60 / 4] & 7) > 0) && (combiner.used != 0)) {
			renderspans = render_delegate(FUNC(nv2a_renderer::render_register_combiners), this);
		}
		else if (texture[0].enabled) {
			renderspans = render_delegate(FUNC(nv2a_renderer::render_texture_simple), this);
		}
		else
			renderspans = render_delegate(FUNC(nv2a_renderer::render_color), this);
		// vertices are taken from the next words, not from a vertex buffer
		// first send primitive type with 17fc
		// then countlen number of dwords with 1818
		// end with 17fc primitive type 0
		// at 1760 16 words specify the vertex format:for each possible vertex attribute the number of components (0=not present) and type of each
		type = channel[chanel][subchannel].object.method[0x17fc / 4];
		if (type == nv2a_renderer::TRIANGLE_FAN) {
			vertex_nv vert[3];
			vertex_t xy[3];
			int c;

			c = read_vertices_0x1818(space, vert, address, 2);
			convert_vertices_poly(vert, xy, 2);
			countlen = countlen - c;
			if (countlen < 0) {
				logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
				countlen = 0;
				return 0;
			}
			address = address + c * 4;
			for (n = 1; countlen > 0; n++) {
				c = read_vertices_0x1818(space, vert + ((n & 1) + 1), address, 1);
				countlen = countlen - c;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
					countlen = 0;
					break;
				}
				address = address + c * 4;
				convert_vertices_poly(vert + ((n & 1) + 1), xy + ((n & 1) + 1), 1);
				render_triangle(limits_rendertarget, renderspans, 4 + 4 * 2, xy[0], xy[(~n & 1) + 1], xy[(n & 1) + 1]);
				wait();
			}
		}
		else if (type == nv2a_renderer::TRIANGLES) {
			while (countlen > 0) {
				vertex_nv vert[3];
				vertex_t xy[3];
				int c;

				c = read_vertices_0x1818(space, vert, address, 3);
				convert_vertices_poly(vert, xy, 3);
				countlen = countlen - c;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
					countlen = 0;
					break;
				}
				address = address + c * 3;
				render_triangle(limits_rendertarget, renderspans, 4 + 4 * 2, xy[0], xy[1], xy[2]); // 4 rgba, 4 texture units 2 uv
				wait();
			}
		}
		else if (type == nv2a_renderer::TRIANGLE_STRIP) {
			vertex_nv vert[4];
			vertex_t xy[4];
			int c;

			c = read_vertices_0x1818(space, vert, address, 2);
			convert_vertices_poly(vert, xy, 2);
			countlen = countlen - c;
			if (countlen < 0) {
				logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
				countlen = 0;
				return 0;
			}
			address = address + c * 4;
			for (n = 0; countlen > 0; n++) {
				c = read_vertices_0x1818(space, vert + ((n + 2) & 3), address, 1);
				convert_vertices_poly(vert + ((n + 2) & 3), xy + ((n + 2) & 3), 1);
				countlen = countlen - c;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
					countlen = 0;
					break;
				}
				address = address + c * 4;
				render_triangle(limits_rendertarget, renderspans, 4 + 4 * 2, xy[((n & 1) + n) & 3], xy[((~n & 1) + n) & 3], xy[(2 + n) & 3]);
				wait();
			}
		}
		else if (type == nv2a_renderer::QUADS) {
			while (countlen > 0) {
				vertex_nv vert[4];
				vertex_t xy[4];
				int c;

				c = read_vertices_0x1818(space, vert, address, 4);
				convert_vertices_poly(vert, xy, 4);
				countlen = countlen - c;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
					countlen = 0;
					break;
				}
				address = address + c * 4;
				render_polygon<4>(limits_rendertarget, renderspans, 4 + 4 * 2, xy); // 4 rgba, 4 texture units 2 uv
				wait();
			}
		}
		else if (type == nv2a_renderer::QUAD_STRIP) {
			vertex_nv vert[4];
			vertex_t xy[4];
			int c;

			c = read_vertices_0x1818(space, vert, address, 2);
			convert_vertices_poly(vert, xy, 2);
			countlen = countlen - c;
			if (countlen < 0) {
				logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
				countlen = 0;
				return 0;
			}
			address = address + c * 4;
			for (n = 0; countlen > 0; n += 2) {
				c = read_vertices_0x1818(space, vert + ((n + 2) & 3), address + ((n + 2) & 3), 2);
				convert_vertices_poly(vert + ((n + 2) & 3), xy + ((n + 2) & 3), 2);
				countlen = countlen - c;
				if (countlen < 0) {
					logerror("Method 0x1818 missing %d words to draw a complete primitive\n", -countlen);
					countlen = 0;
					return 0;
				}
				address = address + c * 4;
				render_triangle(limits_rendertarget, renderspans, 4 + 4 * 2, xy[n & 3], xy[(n + 1) & 3], xy[(n + 2) & 3]);
				render_triangle(limits_rendertarget, renderspans, 4 + 4 * 2, xy[(n + 2) & 3], xy[(n + 1) & 3], xy[(n + 3) & 3]);
				wait();
			}
		}
		else {
			logerror("Unsupported primitive %d for method 0x1818\n", type);
			countlen = 0;
		}
	}
	if ((maddress >= 0x1720) && (maddress < 0x1760)) {
		int bit = method - 0x1720 / 4;

		if (data & 0x80000000)
			vertexbuffer_address[bit] = (data & 0x0fffffff) + dma_offset[1];
		else
			vertexbuffer_address[bit] = (data & 0x0fffffff) + dma_offset[0];
	}
	if ((maddress >= 0x1760) && (maddress < 0x17A0)) {
		int bit = method - 0x1760 / 4;

		vertexbuffer_stride[bit] = (data >> 8) & 255;
		vertexbuffer_kind[bit] = data & 15;
		vertexbuffer_size[bit] = (data >> 4) & 15;
		switch (vertexbuffer_kind[bit]) {
		case NV2A_VTXBUF_TYPE_UBYTE2:
			vertex_attribute_words[bit] = (vertexbuffer_size[bit] * 1) >> 2;
			break;
		case NV2A_VTXBUF_TYPE_FLOAT:
			vertex_attribute_words[bit] = (vertexbuffer_size[bit] * 4) >> 2;
			break;
		case NV2A_VTXBUF_TYPE_UBYTE:
			vertex_attribute_words[bit] = (vertexbuffer_size[bit] * 1) >> 2;
			break;
		case NV2A_VTXBUF_TYPE_USHORT:
			vertex_attribute_words[bit] = (vertexbuffer_size[bit] * 2) >> 2;
			break;
		case NV2A_VTXBUF_TYPE_UNKNOWN_6:
			vertex_attribute_words[bit] = (vertexbuffer_size[bit] * 4) >> 2;
			break;
		default:
			vertex_attribute_words[bit] = 0;
		}
		if (vertexbuffer_size[bit] > 0)
			enabled_vertex_attributes |= (1 << bit);
		else
			enabled_vertex_attributes &= ~(1 << bit);
		for (int n = bit + 1; n < 16; n++) {
			if ((enabled_vertex_attributes & (1 << (n - 1))) != 0)
				vertex_attribute_offset[n] = vertex_attribute_offset[n - 1] + vertex_attribute_words[n - 1];
			else
				vertex_attribute_offset[n] = vertex_attribute_offset[n - 1];
		}
		countlen--;
	}
	if ((maddress == 0x1d6c) || (maddress == 0x1d70) || (maddress == 0x1a4))
		countlen--;
	if (maddress == 0x019c) {
		geforce_read_dma_object(data, dma_offset[0], dma_size[0]);
	}
	if (maddress == 0x01a0) {
		geforce_read_dma_object(data, dma_offset[1], dma_size[1]);
	}
	if (maddress == 0x1d70) {
		// with 1d70 write the value at offest [1d6c] inside dma object [1a4]
		UINT32 offset, base;
		UINT32 dmahand, dmaoff, smasiz;

		offset = channel[chanel][subchannel].object.method[0x1d6c / 4];
		dmahand = channel[chanel][subchannel].object.method[0x1a4 / 4];
		geforce_read_dma_object(dmahand, dmaoff, smasiz);
		base = dmaoff;
		space.write_dword(base + offset, data);
		countlen--;
	}
	if (maddress == 0x1d7c) {
		antialias_control = data;
		countlen--;
	}
	if (maddress == 0x1d98) {
		countlen--;
	}
	if (maddress == 0x1d9c) {
		countlen--;
	}
	if (maddress == 0x1d94) {
		int m;

		m = antialias_control;
		if (antialiasing_rendertarget != 0)
			m = 2;
		else
			m = 1;
		// possible buffers: color, depth, stencil
		// clear framebuffer
		if (data & 0xf0) {
			if (bytespixel_rendertarget == 4) {
				bitmap_rgb32 bm(rendertarget, (limits_rendertarget.right() + 1) * m, (limits_rendertarget.bottom() + 1) * m, pitch_rendertarget / 4);

				UINT32 color = channel[chanel][subchannel].object.method[0x1d90 / 4];
				bm.fill(color);
			}
			else if (bytespixel_rendertarget == 2) {
				bitmap_ind16 bm((UINT16 *)rendertarget, (limits_rendertarget.right() + 1) * m, (limits_rendertarget.bottom() + 1) * m, pitch_rendertarget / 2);

				UINT16 color = channel[chanel][subchannel].object.method[0x1d90 / 4] & 0xffff;

				bm.fill(color);
			}
			else if (bytespixel_rendertarget == 1) {
				UINT8 color = channel[chanel][subchannel].object.method[0x1d90 / 4] & 0xff;

				memset(rendertarget, color, pitch_rendertarget*(limits_rendertarget.bottom() + 1) * m);
			}
#ifdef LOG_NV2A
			printf("clearscreen\n\r");
#endif
		}
		clear_depth_buffer(data & 3, channel[chanel][subchannel].object.method[0x1d8c / 4]);
		countlen--;
	}
	if (maddress == 0x0200) {
		int x, w;

		x = data & 0xffff;
		w = (data >> 16) & 0xffff;
		limits_rendertarget.setx(x,x+w-1);
	}
	if (maddress == 0x0204) {
		int y, h;

		y = data & 0xffff;
		h = (data >> 16) & 0xffff;
		limits_rendertarget.sety(y,y+h-1);
	}
	if (maddress == 0x0208) {
		log2height_rendertarget = (data >> 24) & 255;
		log2width_rendertarget = (data >> 16) & 255;
		antialiasing_rendertarget = (data >> 12) & 15;
		type_rendertarget = (data >> 8) & 15;
		depthformat_rendertarget = (data >> 4) & 15;
		colorformat_rendertarget = (data >> 0) & 15;
		switch (colorformat_rendertarget) {
		case NV2A_COLOR_FORMAT_R5G6B5:
			bytespixel_rendertarget = 2;
			break;
		case NV2A_COLOR_FORMAT_UNKNOWN4:
		case NV2A_COLOR_FORMAT_X8R8G8B8:
		case NV2A_COLOR_FORMAT_A8R8G8B8:
			bytespixel_rendertarget = 4;
			break;
		case NV2A_COLOR_FORMAT_B8:
			bytespixel_rendertarget = 1;
			break;
		default:
			logerror("Unknown render target color format %d\n\r", colorformat_rendertarget);
			bytespixel_rendertarget = 4;
			break;
		}
		dilate_rendertarget = dilatechose[(log2width_rendertarget << 4) + log2height_rendertarget];
	}
	if (maddress == 0x020c) {
		pitch_rendertarget=data & 0xffff;
		pitch_depthbuffer=(data >> 16) & 0xffff;
#ifdef LOG_NV2A
		printf("Pitch color %04X zbuffer %04X\n\r",pitch_rendertarget,pitch_depthbuffer);
#endif
		countlen--;
	}
	if (maddress == 0x0100) {
		countlen--;
		if (data != 0) {
			pgraph[0x704 / 4] = 0x100;
			pgraph[0x708 / 4] = data;
			pgraph[0x100 / 4] |= 1;
			pgraph[0x108 / 4] |= 1;
			if (update_interrupts() == true)
				interruptdevice->ir3_w(1); // IRQ 3
			else
				interruptdevice->ir3_w(0); // IRQ 3
			return 2;
		}
		else
			return 0;
	}
	if (maddress == 0x0130) {
		countlen--;
		if (waitvblank_used == 1)
			return 1; // block until next vblank
		else
			return 0;
	}
	if (maddress == 0x1d8c) {
		countlen--;
		// it is used to specify the clear value for the depth buffer (zbuffer)
		// but also as a parameter for interrupt routines
		pgraph[0x1a88 / 4] = data;
	}
	if (maddress == 0x1d90) {
		countlen--;
		// it is used to specify the clear value for the color buffer
		// but also as a parameter for interrupt routines
		pgraph[0x186c / 4] = data;
	}
	if (maddress == 0x0210) {
		// framebuffer offset ?
		rendertarget = (UINT32 *)direct_access_ptr(data);
#ifdef LOG_NV2A
		printf("Render target at %08X\n\r", data);
#endif
		countlen--;
	}
	if (maddress == 0x0214) {
		// zbuffer offset ?
		depthbuffer = (UINT32 *)direct_access_ptr(data);
#ifdef LOG_NV2A
		printf("Depth buffer at %08X\n\r",data);
#endif
		if ((data == 0) || (data > 0x7ffffffc))
			depth_write_enabled = false;
		else if (channel[chanel][subchannel].object.method[0x035c / 4] != 0)
			depth_write_enabled = true;
		else
			depth_write_enabled = false;
		countlen--;
	}
	if (maddress == 0x0300) {
		alpha_test_enabled = data != 0;
	}
	if (maddress == 0x033c) {
		alpha_func = data;
	}
	if (maddress == 0x0340) {
		alpha_reference = data;
	}
	if (maddress == 0x0304) {
		if (logical_operation_enabled)
			blending_enabled = false;
		else
			blending_enabled = data != 0;
	}
	if (maddress == 0x030c) {
		depth_test_enabled = data != 0;
	}
	if (maddress == 0x0354) {
		depth_function = data;
	}
	if (maddress == 0x0358) {
		//color_mask = data;
		if (data & 0x000000ff)
			data |= 0x000000ff;
		if (data & 0x0000ff00)
			data |= 0x0000ff00;
		if (data & 0x00ff0000)
			data |= 0x00ff0000;
		if (data & 0xff000000)
			data |= 0xff000000;
		color_mask = data;
	}
	if (maddress == 0x035c) {
		UINT32 g = channel[chanel][subchannel].object.method[0x0214 / 4];
		depth_write_enabled = data != 0;
		if ((g == 0) || (g > 0x7ffffffc))
			depth_write_enabled = false;
	}
	if (maddress == 0x032c) {
		stencil_test_enabled = data != 0;
	}
	if (maddress == 0x0364) {
		stencil_func = data;
	}
	if (maddress == 0x0368) {
		if (data > 255)
			data = 255;
		stencil_ref = data;
	}
	if (maddress == 0x036c) {
		stencil_mask = data;
	}
	if (maddress == 0x0370) {
		stencil_op_fail = data;
	}
	if (maddress == 0x0374) {
		stencil_op_zfail = data;
	}
	if (maddress == 0x0378) {
		stencil_op_zpass = data;
	}
	if (maddress == 0x0344) {
		blend_function_source = data;
	}
	if (maddress == 0x0348) {
		blend_function_destination = data;
	}
	if (maddress == 0x034c) {
		blend_color = data;
	}
	if (maddress == 0x0350) {
		blend_equation = data;
	}
	if (maddress == 0x0d40) {
		if (data != 0)
			blending_enabled = false;
		else
			blending_enabled = channel[chanel][subchannel].object.method[0x0304 / 4] != 0;
		logical_operation_enabled = data != 0;
	}
	if (maddress == 0x0d44) {
		logical_operation = data;
	}
	// Texture Units
	if ((maddress >= 0x1b00) && (maddress < 0x1c00)) {
		int unit;//,off;

		unit = (maddress >> 6) & 3;
		//off=maddress & 0xc0;
		maddress = maddress & ~0xc0;
		if (maddress == 0x1b00) {
			UINT32 offset;//,base;
			//UINT32 dmahand,dmaoff,dmasiz;

			offset = data;
			texture[unit].buffer = direct_access_ptr(offset);
			/*if (dma0 != 0) {
			dmahand=channel[channel][subchannel].object.method[0x184/4];
			geforce_read_dma_object(dmahand,dmaoff,smasiz);
			} else if (dma1 != 0) {
			dmahand=channel[channel][subchannel].object.method[0x188/4];
			geforce_read_dma_object(dmahand,dmaoff,smasiz);
			}*/
		}
		if (maddress == 0x1b04) {
			//int dma0,dma1,cubic,noborder,dims,mipmap;
			int basesizeu, basesizev, basesizew, format;

			//dma0=(data >> 0) & 1;
			//dma1=(data >> 1) & 1;
			//cubic=(data >> 2) & 1;
			//noborder=(data >> 3) & 1;
			//dims=(data >> 4) & 15;
			//mipmap=(data >> 19) & 1;
			format = (data >> 8) & 255;
			basesizeu = (data >> 20) & 15;
			basesizev = (data >> 24) & 15;
			basesizew = (data >> 28) & 15;
			texture[unit].sizeu = 1 << basesizeu;
			texture[unit].sizev = 1 << basesizev;
			texture[unit].sizew = 1 << basesizew;
			texture[unit].dilate = dilatechose[(basesizeu << 4) + basesizev];
			texture[unit].format = format;
			if (debug_grab_texttype == format) {
				FILE *f;
				int written;

				debug_grab_texttype = -1;
				f = fopen(debug_grab_textfile, "wb");
				if (f) {
					written = (int)fwrite(texture[unit].buffer, texture[unit].sizeu*texture[unit].sizev * 4, 1, f);
					fclose(f);
					logerror("Written %d bytes of texture to specified file\n", written);
				}
				else
					logerror("Unable to save texture to specified file\n");
			}
		}
		if (maddress == 0x1b0c) {
			// enable texture
			int enable;

			enable = (data >> 30) & 1;
			texture[unit].enabled = enable;
		}
		if (maddress == 0x1b10) {
			texture[unit].rectangle_pitch = data >> 16;
		}
		countlen--;
	}
	// modelview matrix
	if ((maddress >= 0x0480) && (maddress < 0x04c0)) {
		maddress = (maddress - 0x0480) / 4;
		*(UINT32 *)(&matrix.modelview[maddress]) = data;
		countlen--;
	}
	// inverse modelview matrix
	if ((maddress >= 0x0580) && (maddress < 0x05c0)) {
		maddress = (maddress - 0x0580) / 4;
		*(UINT32 *)(&matrix.modelview_inverse[maddress]) = data;
		countlen--;
	}
	// projection matrix
	if ((maddress >= 0x0680) && (maddress < 0x06c0)) {
		maddress = (maddress - 0x0680) / 4;
		*(UINT32 *)(&matrix.projection[maddress]) = data;
		countlen--;
	}
	// viewport translate
	if ((maddress >= 0x0a20) && (maddress < 0x0a30)) {
		maddress = (maddress - 0x0a20) / 4;
		*(UINT32 *)(&matrix.translate[maddress]) = data;
		// set corresponding vertex shader constant too
		vertexprogram.exec.c_constant[59].iv[maddress] = data; // constant -37
		countlen--;
	}
	// viewport scale
	if ((maddress >= 0x0af0) && (maddress < 0x0b00)) {
		maddress = (maddress - 0x0af0) / 4;
		*(UINT32 *)(&matrix.scale[maddress]) = data;
		// set corresponding vertex shader constant too
		vertexprogram.exec.c_constant[58].iv[maddress] = data; // constant -38
		countlen--;
	}
	// Vertex program (shader)
	if (maddress == 0x1e94) {
		/*if (data == 2)
		logerror("Enabled vertex program\n");
		else if (data == 4)
		logerror("Enabled fixed function pipeline\n");
		else if (data == 6)
		logerror("Enabled both fixed function pipeline and vertex program ?\n");
		else
		logerror("Unknown value %d to method 0x1e94\n",data);*/
		vertex_pipeline = data & 6;
		countlen--;
	}
	if (maddress == 0x1e9c) {
		//logerror("VP_UPLOAD_FROM_ID %d\n",data);
		vertexprogram.upload_instruction_index = data;
		vertexprogram.upload_instruction_component = 0;
		countlen--;
	}
	if (maddress == 0x1ea0) {
		//logerror("VP_START_FROM_ID %d\n",data);
		vertexprogram.instructions = vertexprogram.upload_instruction_index;
		vertexprogram.start_instruction = data;
		countlen--;
	}
	if (maddress == 0x1ea4) {
		//logerror("VP_UPLOAD_CONST_ID %d\n",data);
		vertexprogram.upload_parameter_index = data;
		vertexprogram.upload_parameter_component = 0;
		countlen--;
	}
	if ((maddress >= 0x0b00) && (maddress < 0x0b80)) {
		//logerror("VP_UPLOAD_INST\n");
		if (vertexprogram.upload_instruction_index < 192) {
			vertexprogram.exec.op[vertexprogram.upload_instruction_index].i[vertexprogram.upload_instruction_component] = data;
			vertexprogram.exec.op[vertexprogram.upload_instruction_index].modified |= (1 << vertexprogram.upload_instruction_component);
		}
		else
			logerror("Need to increase size of vertexprogram.instruction to %d\n\r", vertexprogram.upload_instruction_index);
		if (vertexprogram.exec.op[vertexprogram.upload_instruction_index].modified == 15) {
			vertexprogram.exec.op[vertexprogram.upload_instruction_index].modified = 0;
			vertexprogram.exec.decode_instruction(vertexprogram.upload_instruction_index);
		}
		vertexprogram.upload_instruction_component++;
		if (vertexprogram.upload_instruction_component >= 4) {
			vertexprogram.upload_instruction_component = 0;
			vertexprogram.upload_instruction_index++;
		}
	}
	if ((maddress >= 0x0b80) && (maddress < 0x0c00)) {
		//logerror("VP_UPLOAD_CONST\n");
		if (vertexprogram.upload_parameter_index < 256)
			vertexprogram.exec.c_constant[vertexprogram.upload_parameter_index].iv[vertexprogram.upload_parameter_component] = data;
		else
			logerror("Need to increase size of vertexprogram.parameter to %d\n\r", vertexprogram.upload_parameter_index);
		vertexprogram.upload_parameter_component++;
		if (vertexprogram.upload_parameter_component >= 4) {
			vertexprogram.upload_parameter_component = 0;
			vertexprogram.upload_parameter_index++;
		}
	}
	// Register combiners
	if (maddress == 0x1e60) {
		combiner.stages = data & 15;
		countlen--;
	}
	if (maddress == 0x0288) {
		combiner.final.mapin_rgbD_input = data & 15;
		combiner.final.mapin_rgbD_component = (data >> 4) & 1;
		combiner.final.mapin_rgbD_mapping = (data >> 5) & 7;
		combiner.final.mapin_rgbC_input = (data >> 8) & 15;
		combiner.final.mapin_rgbC_component = (data >> 12) & 1;
		combiner.final.mapin_rgbC_mapping = (data >> 13) & 7;
		combiner.final.mapin_rgbB_input = (data >> 16) & 15;
		combiner.final.mapin_rgbB_component = (data >> 20) & 1;
		combiner.final.mapin_rgbB_mapping = (data >> 21) & 7;
		combiner.final.mapin_rgbA_input = (data >> 24) & 15;
		combiner.final.mapin_rgbA_component = (data >> 28) & 1;
		combiner.final.mapin_rgbA_mapping = (data >> 29) & 7;
		countlen--;
	}
	if (maddress == 0x028c) {
		combiner.final.color_sum_clamp = (data >> 7) & 1;
		combiner.final.mapin_aG_input = (data >> 8) & 15;
		combiner.final.mapin_aG_component = (data >> 12) & 1;
		combiner.final.mapin_aG_mapping = (data >> 13) & 7;
		combiner.final.mapin_rgbF_input = (data >> 16) & 15;
		combiner.final.mapin_rgbF_component = (data >> 20) & 1;
		combiner.final.mapin_rgbF_mapping = (data >> 21) & 7;
		combiner.final.mapin_rgbE_input = (data >> 24) & 15;
		combiner.final.mapin_rgbE_component = (data >> 28) & 1;
		combiner.final.mapin_rgbE_mapping = (data >> 29) & 7;
		countlen--;
	}
	if (maddress == 0x1e20) {
		combiner_argb8_float(data, combiner.final.register_constantcolor0);
		countlen--;
	}
	if (maddress == 0x1e24) {
		combiner_argb8_float(data, combiner.final.register_constantcolor1);
		countlen--;
	}
	if ((maddress >= 0x0260) && (maddress < 0x0280)) {
		int n;

		n = (maddress - 0x0260) >> 2;
		combiner.stage[n].mapin_aD_input = data & 15;
		combiner.stage[n].mapin_aD_component = (data >> 4) & 1;
		combiner.stage[n].mapin_aD_mapping = (data >> 5) & 7;
		combiner.stage[n].mapin_aC_input = (data >> 8) & 15;
		combiner.stage[n].mapin_aC_component = (data >> 12) & 1;
		combiner.stage[n].mapin_aC_mapping = (data >> 13) & 7;
		combiner.stage[n].mapin_aB_input = (data >> 16) & 15;
		combiner.stage[n].mapin_aB_component = (data >> 20) & 1;
		combiner.stage[n].mapin_aB_mapping = (data >> 21) & 7;
		combiner.stage[n].mapin_aA_input = (data >> 24) & 15;
		combiner.stage[n].mapin_aA_component = (data >> 28) & 1;
		combiner.stage[n].mapin_aA_mapping = (data >> 29) & 7;
		countlen--;
	}
	if ((maddress >= 0x0ac0) && (maddress < 0x0ae0)) {
		int n;

		n = (maddress - 0x0ac0) >> 2;
		combiner.stage[n].mapin_rgbD_input = data & 15;
		combiner.stage[n].mapin_rgbD_component = (data >> 4) & 1;
		combiner.stage[n].mapin_rgbD_mapping = (data >> 5) & 7;
		combiner.stage[n].mapin_rgbC_input = (data >> 8) & 15;
		combiner.stage[n].mapin_rgbC_component = (data >> 12) & 1;
		combiner.stage[n].mapin_rgbC_mapping = (data >> 13) & 7;
		combiner.stage[n].mapin_rgbB_input = (data >> 16) & 15;
		combiner.stage[n].mapin_rgbB_component = (data >> 20) & 1;
		combiner.stage[n].mapin_rgbB_mapping = (data >> 21) & 7;
		combiner.stage[n].mapin_rgbA_input = (data >> 24) & 15;
		combiner.stage[n].mapin_rgbA_component = (data >> 28) & 1;
		combiner.stage[n].mapin_rgbA_mapping = (data >> 29) & 7;
		countlen--;
	}
	if ((maddress >= 0x0a60) && (maddress < 0x0a80)) {
		int n;

		n = (maddress - 0x0a60) >> 2;
		combiner_argb8_float(data, combiner.stage[n].register_constantcolor0);
		countlen--;
	}
	if ((maddress >= 0x0a80) && (maddress < 0x0aa0)) {
		int n;

		n = (maddress - 0x0a80) >> 2;
		combiner_argb8_float(data, combiner.stage[n].register_constantcolor1);
		countlen--;
	}
	if ((maddress >= 0x0aa0) && (maddress < 0x0ac0)) {
		int n;

		n = (maddress - 0x0aa0) >> 2;
		combiner.stage[n].mapout_aCD_output = data & 15;
		combiner.stage[n].mapout_aAB_output = (data >> 4) & 15;
		combiner.stage[n].mapout_aSUM_output = (data >> 8) & 15;
		combiner.stage[n].mapout_aCD_dotproduct = (data >> 12) & 1;
		combiner.stage[n].mapout_aAB_dotproduct = (data >> 13) & 1;
		combiner.stage[n].mapout_a_muxsum = (data >> 14) & 1;
		combiner.stage[n].mapout_a_bias = (data >> 15) & 1;
		combiner.stage[n].mapout_a_scale = (data >> 16) & 3;
		//combiner.=(data >> 27) & 7;
		countlen--;
	}
	if ((maddress >= 0x1e40) && (maddress < 0x1e60)) {
		int n;

		n = (maddress - 0x1e40) >> 2;
		combiner.stage[n].mapout_rgbCD_output = data & 15;
		combiner.stage[n].mapout_rgbAB_output = (data >> 4) & 15;
		combiner.stage[n].mapout_rgbSUM_output = (data >> 8) & 15;
		combiner.stage[n].mapout_rgbCD_dotproduct = (data >> 12) & 1;
		combiner.stage[n].mapout_rgbAB_dotproduct = (data >> 13) & 1;
		combiner.stage[n].mapout_rgb_muxsum = (data >> 14) & 1;
		combiner.stage[n].mapout_rgb_bias = (data >> 15) & 1;
		combiner.stage[n].mapout_rgb_scale = (data >> 16) & 3;
		//combiner.=(data >> 27) & 7;
		countlen--;
	}
	return 0;
}

int nv2a_renderer::toggle_register_combiners_usage()
{
	combiner.used = 1 - combiner.used;
	return combiner.used;
}

int nv2a_renderer::toggle_wait_vblank_support()
{
	waitvblank_used = 1 - waitvblank_used;
	return waitvblank_used;
}

void nv2a_renderer::debug_grab_texture(int type, const char *filename)
{
	debug_grab_texttype = type;
	if (debug_grab_textfile == NULL)
		debug_grab_textfile = (char *)malloc(128);
	strncpy(debug_grab_textfile, filename, 127);
}

void nv2a_renderer::debug_grab_vertex_program_slot(int slot, UINT32 *instruction)
{
	if (slot >= 1024 / 4)
		return;
	instruction[0] = vertexprogram.exec.op[slot].i[0];
	instruction[1] = vertexprogram.exec.op[slot].i[1];
	instruction[2] = vertexprogram.exec.op[slot].i[2];
	instruction[3] = vertexprogram.exec.op[slot].i[3];
}

void nv2a_renderer::combiner_argb8_float(UINT32 color, float reg[4])
{
	reg[0] = (float)(color & 0xff) / 255.0f;
	reg[1] = (float)((color >> 8) & 0xff) / 255.0f;
	reg[2] = (float)((color >> 16) & 0xff) / 255.0f;
	reg[3] = (float)((color >> 24) & 0xff) / 255.0f;
}

UINT32 nv2a_renderer::combiner_float_argb8(float reg[4])
{
	UINT32 r, g, b, a;

	a = reg[3] * 255.0f;
	r = reg[2] * 255.0f;
	g = reg[1] * 255.0f;
	b = reg[0] * 255.0f;
	return (a << 24) | (r << 16) | (g << 8) | b;
}

float nv2a_renderer::combiner_map_input_select(int code, int index)
{
	switch (code) {
	case 0:
	default:
		return combiner.register_zero[index];
	case 1:
		return combiner.register_color0[index];
	case 2:
		return combiner.register_color1[index];
	case 3:
		return combiner.register_fogcolor[index];
	case 4:
		return combiner.register_primarycolor[index];
	case 5:
		return combiner.register_secondarycolor[index];
	case 8:
		return combiner.register_texture0color[index];
	case 9:
		return combiner.register_texture1color[index];
	case 10:
		return combiner.register_texture2color[index];
	case 11:
		return combiner.register_texture3color[index];
	case 12:
		return combiner.register_spare0[index];
	case 13:
		return combiner.register_spare1[index];
	case 14:
		return combiner.variable_sumclamp[index];
	case 15:
		return combiner.variable_EF[index];
	}

	// never executed
	//return 0;
}

float *nv2a_renderer::combiner_map_input_select3(int code)
{
	switch (code) {
	case 0:
	default:
		return combiner.register_zero;
	case 1:
		return combiner.register_color0;
	case 2:
		return combiner.register_color1;
	case 3:
		return combiner.register_fogcolor;
	case 4:
		return combiner.register_primarycolor;
	case 5:
		return combiner.register_secondarycolor;
	case 8:
		return combiner.register_texture0color;
	case 9:
		return combiner.register_texture1color;
	case 10:
		return combiner.register_texture2color;
	case 11:
		return combiner.register_texture3color;
	case 12:
		return combiner.register_spare0;
	case 13:
		return combiner.register_spare1;
	case 14:
		return combiner.variable_sumclamp;
	case 15:
		return combiner.variable_EF;
	}

	// never executed
	//return 0;
}

float *nv2a_renderer::combiner_map_output_select3(int code)
{
	switch (code) {
	case 0:
		return 0;
	case 1:
		return 0;
	case 2:
		return 0;
	case 3:
		return 0;
	case 4:
		return combiner.register_primarycolor;
	case 5:
		return combiner.register_secondarycolor;
	case 8:
		return combiner.register_texture0color;
	case 9:
		return combiner.register_texture1color;
	case 10:
		return combiner.register_texture2color;
	case 11:
		return combiner.register_texture3color;
	case 12:
		return combiner.register_spare0;
	case 13:
		return combiner.register_spare1;
	case 14:
		return 0;
	case 15:
	default:
		return 0;
	}
}

float nv2a_renderer::combiner_map_input_function(int code, float value)
{
	float t;

	switch (code) {
	case 0:
		return MAX(0.0f, value);
	case 1:
		t = MAX(value, 0.0f);
		return 1.0f - MIN(t, 1.0f);
	case 2:
		return 2.0f * MAX(0.0f, value) - 1.0f;
	case 3:
		return -2.0f * MAX(0.0f, value) + 1.0f;
	case 4:
		return MAX(0.0f, value) - 0.5f;
	case 5:
		return -MAX(0.0f, value) + 0.5f;
	case 6:
		return value;
	case 7:
	default:
		return -value;
	}

	// never executed
	//return 0;
}

void nv2a_renderer::combiner_map_input_function3(int code, float *data)
{
	float t;

	switch (code) {
	case 0:
		data[0] = MAX(0.0f, data[0]);
		data[1] = MAX(0.0f, data[1]);
		data[2] = MAX(0.0f, data[2]);
		break;
	case 1:
		t = MAX(data[0], 0.0f);
		data[0] = 1.0f - MIN(t, 1.0f);
		t = MAX(data[1], 0.0f);
		data[1] = 1.0f - MIN(t, 1.0f);
		t = MAX(data[2], 0.0f);
		data[2] = 1.0f - MIN(t, 1.0f);
		break;
	case 2:
		data[0] = 2.0f * MAX(0.0f, data[0]) - 1.0f;
		data[1] = 2.0f * MAX(0.0f, data[1]) - 1.0f;
		data[2] = 2.0f * MAX(0.0f, data[2]) - 1.0f;
		break;
	case 3:
		data[0] = -2.0f * MAX(0.0f, data[0]) + 1.0f;
		data[1] = -2.0f * MAX(0.0f, data[1]) + 1.0f;
		data[2] = -2.0f * MAX(0.0f, data[2]) + 1.0f;
		break;
	case 4:
		data[0] = MAX(0.0f, data[0]) - 0.5f;
		data[1] = MAX(0.0f, data[1]) - 0.5f;
		data[2] = MAX(0.0f, data[2]) - 0.5f;
		break;
	case 5:
		data[0] = -MAX(0.0f, data[0]) + 0.5f;
		data[1] = -MAX(0.0f, data[1]) + 0.5f;
		data[2] = -MAX(0.0f, data[2]) + 0.5f;
		break;
	case 6:
		return;
	case 7:
	default:
		data[0] = -data[0];
		data[1] = -data[1];
		data[2] = -data[2];
		break;
	}
}

void nv2a_renderer::combiner_initialize_registers(UINT32 argb8[6])
{
	combiner_argb8_float(argb8[0], combiner.register_primarycolor);
	combiner_argb8_float(argb8[1], combiner.register_secondarycolor);
	combiner_argb8_float(argb8[2], combiner.register_texture0color);
	combiner_argb8_float(argb8[3], combiner.register_texture1color);
	combiner_argb8_float(argb8[4], combiner.register_texture2color);
	combiner_argb8_float(argb8[5], combiner.register_texture3color);
	combiner.register_spare0[3] = combiner.register_texture0color[3];
	combiner.register_zero[0] = combiner.register_zero[1] = combiner.register_zero[2] = combiner.register_zero[3] = 0;
}

void nv2a_renderer::combiner_initialize_stage(int stage_number)
{
	int n = stage_number;

	// put register_constantcolor0 in register_color0
	combiner.register_color0[0] = combiner.stage[n].register_constantcolor0[0];
	combiner.register_color0[1] = combiner.stage[n].register_constantcolor0[1];
	combiner.register_color0[2] = combiner.stage[n].register_constantcolor0[2];
	combiner.register_color0[3] = combiner.stage[n].register_constantcolor0[3];
	// put register_constantcolor1 in register_color1
	combiner.register_color1[0] = combiner.stage[n].register_constantcolor1[0];
	combiner.register_color1[1] = combiner.stage[n].register_constantcolor1[1];
	combiner.register_color1[2] = combiner.stage[n].register_constantcolor1[2];
	combiner.register_color1[3] = combiner.stage[n].register_constantcolor1[3];
}

void nv2a_renderer::combiner_initialize_final()
{
	// put register_constantcolor0 in register_color0
	combiner.register_color0[0] = combiner.final.register_constantcolor0[0];
	combiner.register_color0[1] = combiner.final.register_constantcolor0[1];
	combiner.register_color0[2] = combiner.final.register_constantcolor0[2];
	combiner.register_color0[3] = combiner.final.register_constantcolor0[3];
	// put register_constantcolor1 in register_color1
	combiner.register_color1[0] = combiner.final.register_constantcolor1[0];
	combiner.register_color1[1] = combiner.final.register_constantcolor1[1];
	combiner.register_color1[2] = combiner.final.register_constantcolor1[2];
	combiner.register_color1[3] = combiner.final.register_constantcolor1[3];
}

void nv2a_renderer::combiner_map_input(int stage_number)
{
	int n = stage_number;
	int c, d, i;
	float v, *pv;

	// A
	v = combiner_map_input_select(combiner.stage[n].mapin_aA_input, 2 + combiner.stage[n].mapin_aA_component);
	combiner.variable_A[3] = combiner_map_input_function(combiner.stage[n].mapin_aA_mapping, v);
	// B
	v = combiner_map_input_select(combiner.stage[n].mapin_aB_input, 2 + combiner.stage[n].mapin_aB_component);
	combiner.variable_B[3] = combiner_map_input_function(combiner.stage[n].mapin_aB_mapping, v);
	// C
	v = combiner_map_input_select(combiner.stage[n].mapin_aC_input, 2 + combiner.stage[n].mapin_aC_component);
	combiner.variable_C[3] = combiner_map_input_function(combiner.stage[n].mapin_aC_mapping, v);
	// D
	v = combiner_map_input_select(combiner.stage[n].mapin_aD_input, 2 + combiner.stage[n].mapin_aD_component);
	combiner.variable_D[3] = combiner_map_input_function(combiner.stage[n].mapin_aD_mapping, v);

	// A
	pv = combiner_map_input_select3(combiner.stage[n].mapin_rgbA_input);
	c = combiner.stage[n].mapin_rgbA_component * 3;
	i = ~combiner.stage[n].mapin_rgbA_component & 1;
	for (d = 0; d < 3; d++) {
		combiner.variable_A[d] = pv[c];
		c = c + i;
	}
	combiner_map_input_function3(combiner.stage[n].mapin_rgbA_mapping, combiner.variable_A);
	// B
	pv = combiner_map_input_select3(combiner.stage[n].mapin_rgbB_input);
	c = combiner.stage[n].mapin_rgbB_component * 3;
	i = ~combiner.stage[n].mapin_rgbB_component & 1;
	for (d = 0; d < 3; d++) {
		combiner.variable_B[d] = pv[c];
		c = c + i;
	}
	combiner_map_input_function3(combiner.stage[n].mapin_rgbB_mapping, combiner.variable_B);
	// C
	pv = combiner_map_input_select3(combiner.stage[n].mapin_rgbC_input);
	c = combiner.stage[n].mapin_rgbC_component * 3;
	i = ~combiner.stage[n].mapin_rgbC_component & 1;
	for (d = 0; d < 3; d++) {
		combiner.variable_C[d] = pv[c];
		c = c + i;
	}
	combiner_map_input_function3(combiner.stage[n].mapin_rgbC_mapping, combiner.variable_C);
	// D
	pv = combiner_map_input_select3(combiner.stage[n].mapin_rgbD_input);
	c = combiner.stage[n].mapin_rgbD_component * 3;
	i = ~combiner.stage[n].mapin_rgbD_component & 1;
	for (d = 0; d < 3; d++) {
		combiner.variable_D[d] = pv[c];
		c = c + i;
	}
	combiner_map_input_function3(combiner.stage[n].mapin_rgbD_mapping, combiner.variable_D);
}

void nv2a_renderer::combiner_map_output(int stage_number)
{
	int n = stage_number;
	float *f;

	// rgb
	f = combiner_map_output_select3(combiner.stage[n].mapout_rgbAB_output);
	if (f) {
		f[0] = combiner.function_RGBop1[0];
		f[1] = combiner.function_RGBop1[1];
		f[2] = combiner.function_RGBop1[2];
	}
	f = combiner_map_output_select3(combiner.stage[n].mapout_rgbCD_output);
	if (f) {
		f[0] = combiner.function_RGBop2[0];
		f[1] = combiner.function_RGBop2[1];
		f[2] = combiner.function_RGBop2[2];
	}
	if ((combiner.stage[n].mapout_rgbAB_dotproduct | combiner.stage[n].mapout_rgbCD_dotproduct) == 0) {
		f = combiner_map_output_select3(combiner.stage[n].mapout_rgbSUM_output);
		if (f) {
			f[0] = combiner.function_RGBop3[0];
			f[1] = combiner.function_RGBop3[1];
			f[2] = combiner.function_RGBop3[2];
		}
	}
	// a
	f = combiner_map_output_select3(combiner.stage[n].mapout_aAB_output);
	if (f)
		f[3] = combiner.function_Aop1;
	f = combiner_map_output_select3(combiner.stage[n].mapout_aCD_output);
	if (f)
		f[3] = combiner.function_Aop2;
	f = combiner_map_output_select3(combiner.stage[n].mapout_aSUM_output);
	if (f)
		f[3] = combiner.function_Aop3;
}

void nv2a_renderer::combiner_map_final_input()
{
	int i, c, d;
	float *pv;

	// E
	pv = combiner_map_input_select3(combiner.final.mapin_rgbE_input);
	c = combiner.final.mapin_rgbE_component * 3;
	i = ~combiner.final.mapin_rgbE_component & 1;
	for (d = 0; d < 3; d++) {
		combiner.variable_E[d] = pv[c];
		c = c + i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbE_mapping, combiner.variable_E);
	// F
	pv = combiner_map_input_select3(combiner.final.mapin_rgbF_input);
	c = combiner.final.mapin_rgbF_component * 3;
	i = ~combiner.final.mapin_rgbF_component & 1;
	for (d = 0; d < 3; d++) {
		combiner.variable_F[d] = pv[c];
		c = c + i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbF_mapping, combiner.variable_F);
	// EF
	combiner.variable_EF[0] = combiner.variable_E[0] * combiner.variable_F[0];
	combiner.variable_EF[1] = combiner.variable_E[1] * combiner.variable_F[1];
	combiner.variable_EF[2] = combiner.variable_E[2] * combiner.variable_F[2];
	// sumclamp
	combiner.variable_sumclamp[0] = MAX(0, combiner.register_spare0[0]) + MAX(0, combiner.register_secondarycolor[0]);
	combiner.variable_sumclamp[1] = MAX(0, combiner.register_spare0[1]) + MAX(0, combiner.register_secondarycolor[1]);
	combiner.variable_sumclamp[2] = MAX(0, combiner.register_spare0[2]) + MAX(0, combiner.register_secondarycolor[2]);
	if (combiner.final.color_sum_clamp != 0) {
		combiner.variable_sumclamp[0] = MIN(combiner.variable_sumclamp[0], 1.0f);
		combiner.variable_sumclamp[1] = MIN(combiner.variable_sumclamp[1], 1.0f);
		combiner.variable_sumclamp[2] = MIN(combiner.variable_sumclamp[2], 1.0f);
	}
	// A
	pv = combiner_map_input_select3(combiner.final.mapin_rgbA_input);
	c = combiner.final.mapin_rgbA_component * 3;
	i = ~combiner.final.mapin_rgbA_component & 1;
	for (d = 0; d < 3; d++) {
		combiner.variable_A[d] = pv[c];
		c = c + i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbA_mapping, combiner.variable_A);
	// B
	pv = combiner_map_input_select3(combiner.final.mapin_rgbB_input);
	c = combiner.final.mapin_rgbB_component * 3;
	i = ~combiner.final.mapin_rgbB_component & 1;
	for (d = 0; d < 3; d++) {
		combiner.variable_B[d] = pv[c];
		c = c + i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbB_mapping, combiner.variable_B);
	// C
	pv = combiner_map_input_select3(combiner.final.mapin_rgbC_input);
	c = combiner.final.mapin_rgbC_component * 3;
	i = ~combiner.final.mapin_rgbC_component & 1;
	for (d = 0; d < 3; d++) {
		combiner.variable_C[d] = pv[c];
		c = c + i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbC_mapping, combiner.variable_C);
	// D
	pv = combiner_map_input_select3(combiner.final.mapin_rgbD_input);
	c = combiner.final.mapin_rgbD_component * 3;
	i = ~combiner.final.mapin_rgbD_component & 1;
	for (d = 0; d < 3; d++) {
		combiner.variable_D[d] = pv[c];
		c = c + i;
	}
	combiner_map_input_function3(combiner.final.mapin_rgbD_mapping, combiner.variable_D);
	// G
	combiner.variable_G = combiner_map_input_select(combiner.final.mapin_aG_input, 2 + combiner.final.mapin_aG_component);
}

void nv2a_renderer::combiner_final_output()
{
	// rgb
	combiner.output[0] = combiner.variable_A[0] * combiner.variable_B[0] + (1.0f - combiner.variable_A[0])*combiner.variable_C[0] + combiner.variable_D[0];
	combiner.output[1] = combiner.variable_A[1] * combiner.variable_B[1] + (1.0f - combiner.variable_A[1])*combiner.variable_C[1] + combiner.variable_D[1];
	combiner.output[2] = combiner.variable_A[2] * combiner.variable_B[2] + (1.0f - combiner.variable_A[2])*combiner.variable_C[2] + combiner.variable_D[2];
	combiner.output[0] = MIN(combiner.output[0], 1.0f);
	combiner.output[1] = MIN(combiner.output[1], 1.0f);
	combiner.output[2] = MIN(combiner.output[2], 1.0f);
	// a
	combiner.output[3] = combiner_map_input_function(combiner.final.mapin_aG_mapping, combiner.variable_G);
}

void nv2a_renderer::combiner_function_AB(float result[4])
{
	result[0] = combiner.variable_A[0] * combiner.variable_B[0];
	result[1] = combiner.variable_A[1] * combiner.variable_B[1];
	result[2] = combiner.variable_A[2] * combiner.variable_B[2];
}

void nv2a_renderer::combiner_function_AdotB(float result[4])
{
	result[0] = combiner.variable_A[0] * combiner.variable_B[0] + combiner.variable_A[1] * combiner.variable_B[1] + combiner.variable_A[2] * combiner.variable_B[2];
	result[1] = result[0];
	result[2] = result[0];
}

void nv2a_renderer::combiner_function_CD(float result[4])
{
	result[0] = combiner.variable_C[0] * combiner.variable_D[0];
	result[1] = combiner.variable_C[1] * combiner.variable_D[1];
	result[2] = combiner.variable_C[2] * combiner.variable_D[2];
}

void nv2a_renderer::combiner_function_CdotD(float result[4])
{
	result[0] = combiner.variable_C[0] * combiner.variable_D[0] + combiner.variable_C[1] * combiner.variable_D[1] + combiner.variable_C[2] * combiner.variable_D[2];
	result[1] = result[0];
	result[2] = result[0];
}

void nv2a_renderer::combiner_function_ABmuxCD(float result[4])
{
	if (combiner.register_spare0[3] >= 0.5f)
		combiner_function_AB(result);
	else
		combiner_function_CD(result);
}

void nv2a_renderer::combiner_function_ABsumCD(float result[4])
{
	result[0] = combiner.variable_A[0] * combiner.variable_B[0] + combiner.variable_C[0] * combiner.variable_D[0];
	result[1] = combiner.variable_A[1] * combiner.variable_B[1] + combiner.variable_C[1] * combiner.variable_D[1];
	result[2] = combiner.variable_A[2] * combiner.variable_B[2] + combiner.variable_C[2] * combiner.variable_D[2];
}

void nv2a_renderer::combiner_compute_rgb_outputs(int stage_number)
{
	int n = stage_number;
	int m;
	float biasrgb, scalergb;

	if (combiner.stage[n].mapout_rgb_bias)
		biasrgb = -0.5;
	else
		biasrgb = 0;
	switch (combiner.stage[n].mapout_rgb_scale) {
	case 0:
	default:
		scalergb = 1.0;
		break;
	case 1:
		scalergb = 2.0;
		break;
	case 2:
		scalergb = 4.0;
		break;
	case 3:
		scalergb = 0.5;
		break;
	}
	if (combiner.stage[n].mapout_rgbAB_dotproduct) {
		m = 1;
		combiner_function_AdotB(combiner.function_RGBop1);
	}
	else {
		m = 0;
		combiner_function_AB(combiner.function_RGBop1);
	}
	combiner.function_RGBop1[0] = MAX(MIN((combiner.function_RGBop1[0] + biasrgb) * scalergb, 1.0f), -1.0f);
	combiner.function_RGBop1[1] = MAX(MIN((combiner.function_RGBop1[1] + biasrgb) * scalergb, 1.0f), -1.0f);
	combiner.function_RGBop1[2] = MAX(MIN((combiner.function_RGBop1[2] + biasrgb) * scalergb, 1.0f), -1.0f);
	if (combiner.stage[n].mapout_rgbCD_dotproduct) {
		m = m | 1;
		combiner_function_CdotD(combiner.function_RGBop2);
	}
	else
		combiner_function_CD(combiner.function_RGBop2);
	combiner.function_RGBop2[0] = MAX(MIN((combiner.function_RGBop2[0] + biasrgb) * scalergb, 1.0f), -1.0f);
	combiner.function_RGBop2[1] = MAX(MIN((combiner.function_RGBop2[1] + biasrgb) * scalergb, 1.0f), -1.0f);
	combiner.function_RGBop2[2] = MAX(MIN((combiner.function_RGBop2[2] + biasrgb) * scalergb, 1.0f), -1.0f);
	if (m == 0) {
		if (combiner.stage[n].mapout_rgb_muxsum)
			combiner_function_ABmuxCD(combiner.function_RGBop3);
		else
			combiner_function_ABsumCD(combiner.function_RGBop3);
		combiner.function_RGBop3[0] = MAX(MIN((combiner.function_RGBop3[0] + biasrgb) * scalergb, 1.0f), -1.0f);
		combiner.function_RGBop3[1] = MAX(MIN((combiner.function_RGBop3[1] + biasrgb) * scalergb, 1.0f), -1.0f);
		combiner.function_RGBop3[2] = MAX(MIN((combiner.function_RGBop3[2] + biasrgb) * scalergb, 1.0f), -1.0f);
	}
}

void nv2a_renderer::combiner_compute_a_outputs(int stage_number)
{
	int n = stage_number;
	float biasa, scalea;

	if (combiner.stage[n].mapout_a_bias)
		biasa = -0.5;
	else
		biasa = 0;
	switch (combiner.stage[n].mapout_a_scale) {
	case 0:
	default:
		scalea = 1.0;
		break;
	case 1:
		scalea = 2.0;
		break;
	case 2:
		scalea = 4.0;
		break;
	case 3:
		scalea = 0.5;
		break;
	}
	combiner.function_Aop1 = combiner.variable_A[3] * combiner.variable_B[3];
	combiner.function_Aop1 = MAX(MIN((combiner.function_Aop1 + biasa) * scalea, 1.0f), -1.0f);
	combiner.function_Aop2 = combiner.variable_C[3] * combiner.variable_D[3];
	combiner.function_Aop2 = MAX(MIN((combiner.function_Aop2 + biasa) * scalea, 1.0f), -1.0f);
	if (combiner.stage[n].mapout_a_muxsum) {
		if (combiner.register_spare0[3] >= 0.5f)
			combiner.function_Aop3 = combiner.variable_A[3] * combiner.variable_B[3];
		else
			combiner.function_Aop3 = combiner.variable_C[3] * combiner.variable_D[3];
	}
	else
		combiner.function_Aop3 = combiner.variable_A[3] * combiner.variable_B[3] + combiner.variable_C[3] * combiner.variable_D[3];
	combiner.function_Aop3 = MAX(MIN((combiner.function_Aop3 + biasa) * scalea, 1.0f), -1.0f);
}

void nv2a_renderer::vblank_callback(screen_device &screen, bool state)
{
#ifdef LOG_NV2A
	printf("vblank_callback\n\r");
#endif
	if ((state == true) && (puller_waiting == 1)) {
		puller_waiting = 0;
		puller_timer_work(NULL, 0);
	}
	if (state == true) {
		pcrtc[0x100 / 4] |= 1;
		pcrtc[0x808 / 4] |= 0x10000;
	}
	else {
		pcrtc[0x100 / 4] &= ~1;
		pcrtc[0x808 / 4] &= ~0x10000;
	}
	if (update_interrupts() == true)
		interruptdevice->ir3_w(1); // IRQ 3
	else
		interruptdevice->ir3_w(0); // IRQ 3
}

bool nv2a_renderer::update_interrupts()
{
	if (pcrtc[0x100 / 4] & pcrtc[0x140 / 4])
		pmc[0x100 / 4] |= 0x1000000;
	else
		pmc[0x100 / 4] &= ~0x1000000;
	if (pgraph[0x100 / 4] & pgraph[0x140 / 4])
		pmc[0x100 / 4] |= 0x1000;
	else
		pmc[0x100 / 4] &= ~0x1000;
	if (((pmc[0x100 / 4] & 0x7fffffff) && (pmc[0x140 / 4] & 1)) || ((pmc[0x100 / 4] & 0x80000000) && (pmc[0x140 / 4] & 2))) {
		// send interrupt
		return true;
	}
	else
		return false;
}

UINT32 nv2a_renderer::screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (displayedtarget != NULL) {
		bitmap_rgb32 bm(displayedtarget, 640, 480, 640);
		UINT32 *dst = (UINT32 *)bitmap.raw_pixptr(0, 0);

		//printf("updatescreen %08X\n\r",pcrtc[0x800/4]);
		memcpy(dst, displayedtarget, bitmap.rowbytes()*bitmap.height());
	}
	return 0;
}

TIMER_CALLBACK_MEMBER(nv2a_renderer::puller_timer_work)
{
	int chanel, subchannel;
	int method, count, handle, objclass;
	UINT32 *dmaput, *dmaget;
	UINT32 cmd, cmdtype;
	int countlen;
	int ret;
	address_space *space = puller_space;
#ifdef LOG_NV2A
	UINT32 subch;
#endif

	chanel = puller_channel;
	subchannel = puller_subchannel;
	dmaput = &channel[chanel][subchannel].regs[0x40 / 4];
	dmaget = &channel[chanel][subchannel].regs[0x44 / 4];
	chanel = puller_channel;
	subchannel = puller_subchannel;
	while (*dmaget != *dmaput) {
		cmd = space->read_dword(*dmaget);
		*dmaget += 4;
		cmdtype = geforce_commandkind(cmd);
		switch (cmdtype)
		{
		case 6: // jump
#ifdef LOG_NV2A
			printf("jump dmaget %08X", *dmaget);
#endif
			*dmaget = cmd & 0xfffffffc;
#ifdef LOG_NV2A
			printf(" -> %08X\n\r", *dmaget);
#endif
			break;
		case 0: // increasing method
			method = (cmd >> 2) & 2047; // method*4 is address // if method >= 0x40 send it to assigned object
#ifdef LOG_NV2A
			subch = (cmd >> 13) & 7;
#endif
			count = (cmd >> 18) & 2047;
			if ((method == 0) && (count == 1)) {
				handle = space->read_dword(*dmaget);
				handle = geforce_object_offset(handle);
#ifdef LOG_NV2A
				logerror("  assign to subchannel %d object at %d\n", subch, handle);
#endif
				channel[chanel][subchannel].object.objhandle = handle;
				handle = ramin[handle / 4];
				objclass = handle & 0xff;
				channel[chanel][subchannel].object.objclass = objclass;
				*dmaget += 4;
			}
			else {
#ifdef LOG_NV2A
				logerror("  subch. %d method %04x offset %04x count %d\n", subch, method, method * 4, count);
#endif
				ret = 0;
				while (count > 0) {
					countlen = 1;
					ret=geforce_exec_method(*space, chanel, subchannel, method, *dmaget, countlen);
					count--;
					method++;
					*dmaget += 4;
					if (ret != 0)
						break;
				}
				if (ret != 0) {
					puller_timer->enable(false);
					puller_waiting = ret;
					return;
				}
			}
			break;
		case 5: // non-increasing method
			method = (cmd >> 2) & 2047;
#ifdef LOG_NV2A
			subch = (cmd >> 13) & 7;
#endif
			count = (cmd >> 18) & 2047;
			if ((method == 0) && (count == 1)) {
#ifdef LOG_NV2A
				logerror("  assign channel %d\n", subch);
#endif
				handle = space->read_dword(*dmaget);
				handle = geforce_object_offset(handle);
#ifdef LOG_NV2A
				logerror("  assign to subchannel %d object at %d\n", subch, handle);
#endif
				channel[chanel][subchannel].object.objhandle = handle;
				handle = ramin[handle / 4];
				objclass = handle & 0xff;
				channel[chanel][subchannel].object.objclass = objclass;
				*dmaget += 4;
			}
			else {
#ifdef LOG_NV2A
				logerror("  subch. %d method %04x offset %04x count %d\n", subch, method, method * 4, count);
#endif
				while (count > 0) {
					countlen = count;
					ret=geforce_exec_method(*space, chanel, subchannel, method, *dmaget, countlen);
					*dmaget += 4 * (count - countlen);
					count = countlen;
				}
			}
			break;
		case 3: // long non-increasing method
			method = (cmd >> 2) & 2047;
#ifdef LOG_NV2A
			subch = (cmd >> 13) & 7;
#endif
			count = space->read_dword(*dmaget);
			*dmaget += 4;
			if ((method == 0) && (count == 1)) {
				handle = space->read_dword(*dmaget);
				handle = geforce_object_offset(handle);
#ifdef LOG_NV2A
				logerror("  assign to subchannel %d object at %d\n", subch, handle);
#endif
				channel[chanel][subchannel].object.objhandle = handle;
				handle = ramin[handle / 4];
				objclass = handle & 0xff;
				channel[chanel][subchannel].object.objclass = objclass;
				*dmaget += 4;
			}
			else {
#ifdef LOG_NV2A
				logerror("  subch. %d method %04x offset %04x count %d\n", subch, method, method * 4, count);
#endif
				while (count > 0) {
					countlen = count;
					ret=geforce_exec_method(*space, chanel, subchannel, method, *dmaget, countlen);
					*dmaget += 4 * (count - countlen);
					count = countlen;
				}
			}
			break;
		default:
			logerror("  unimplemented command %08X\n", cmd);
		}
	}
}

READ32_MEMBER(nv2a_renderer::geforce_r)
{
	static int x, ret;

	ret = 0;
	if (offset == 0x1804f6) {
		x = x ^ 0x08080808;
		ret = x;
	}
	if ((offset >= 0x00101000 / 4) && (offset < 0x00102000 / 4)) {
		//logerror("NV_2A: read STRAPS[%06X] mask %08X value %08X\n",offset*4-0x00101000,mem_mask,ret);
	}
	else if ((offset >= 0x00002000 / 4) && (offset < 0x00004000 / 4)) {
		ret = pfifo[offset - 0x00002000 / 4];
		// PFIFO.CACHE1.STATUS or PFIFO.RUNOUT_STATUS
		if ((offset == 0x3214 / 4) || (offset == 0x2400 / 4))
			ret = 0x10;
		//logerror("NV_2A: read PFIFO[%06X] value %08X\n",offset*4-0x00002000,ret);
	}
	else if ((offset >= 0x00700000 / 4) && (offset < 0x00800000 / 4)) {
		ret = ramin[offset - 0x00700000 / 4];
		//logerror("NV_2A: read PRAMIN[%06X] value %08X\n",offset*4-0x00700000,ret);
	}
	else if ((offset >= 0x00400000 / 4) && (offset < 0x00402000 / 4)) {
		ret = pgraph[offset - 0x00400000 / 4];
		//logerror("NV_2A: read PGRAPH[%06X] value %08X\n",offset*4-0x00400000,ret);
	}
	else if ((offset >= 0x00600000 / 4) && (offset < 0x00601000 / 4)) {
		ret = pcrtc[offset - 0x00600000 / 4];
		//logerror("NV_2A: read PCRTC[%06X] value %08X\n",offset*4-0x00600000,ret);
	}
	else if ((offset >= 0x00000000 / 4) && (offset < 0x00001000 / 4)) {
		ret = pmc[offset - 0x00000000 / 4];
		//logerror("NV_2A: read PMC[%06X] value %08X\n",offset*4-0x00000000,ret);
	}
	else if ((offset >= 0x00800000 / 4) && (offset < 0x00900000 / 4)) {
		// 32 channels size 0x10000 each, 8 subchannels per channel size 0x2000 each
		int chanel, subchannel, suboffset;

		suboffset = offset - 0x00800000 / 4;
		chanel = (suboffset >> (16 - 2)) & 31;
		subchannel = (suboffset >> (13 - 2)) & 7;
		suboffset = suboffset & 0x7ff;
		if (suboffset < 0x80 / 4)
			ret = channel[chanel][subchannel].regs[suboffset];
		//logerror("NV_2A: read channel[%02X,%d,%04X]=%08X\n",chanel,subchannel,suboffset*4,ret);
		return ret;
	}
	//logerror("NV_2A: read at %08X mask %08X value %08X\n",0xfd000000+offset*4,mem_mask,ret);
	return ret;
}

WRITE32_MEMBER(nv2a_renderer::geforce_w)
{
	UINT32 old;
	bool update_int;

	update_int = false;
	if ((offset >= 0x00101000 / 4) && (offset < 0x00102000 / 4)) {
		//logerror("NV_2A: write STRAPS[%06X] mask %08X value %08X\n",offset*4-0x00101000,mem_mask,data);
	}
	else if ((offset >= 0x00002000 / 4) && (offset < 0x00004000 / 4)) {
		int e = offset - 0x00002000 / 4;
		if (e >= (sizeof(pfifo) / sizeof(UINT32)))
			return;
		COMBINE_DATA(pfifo + e);
		//logerror("NV_2A: read PFIFO[%06X]=%08X\n",offset*4-0x00002000,data & mem_mask); // 2210 pfifo ramht & 1f0 << 12
	}
	else if ((offset >= 0x00700000 / 4) && (offset < 0x00800000 / 4)) {
		int e = offset - 0x00700000 / 4;
		if (e >= (sizeof(ramin) / sizeof(UINT32)))
			return;
		COMBINE_DATA(ramin + e);
		//logerror("NV_2A: write PRAMIN[%06X]=%08X\n",offset*4-0x00700000,data & mem_mask);
	}
	else if ((offset >= 0x00400000 / 4) && (offset < 0x00402000 / 4)) {
		int e = offset - 0x00400000 / 4;
		if (e >= (sizeof(pgraph) / sizeof(UINT32)))
			return;
		old = pgraph[e];
		COMBINE_DATA(pgraph + e);
		if (e == 0x100 / 4) {
			pgraph[e] = old & ~data;
			if (data & 1)
				pgraph[0x108 / 4] = 0;
			update_int = true;
		}
		if (e == 0x140 / 4)
			update_int = true;
		if (e == 0x720 / 4) {
			if ((data & 1) && (puller_waiting == 2)) {
				puller_waiting = 0;
				puller_timer->enable();
				puller_timer->adjust(attotime::zero);
			}
		}
		if ((e >= 0x900 / 4) && (e < 0xa00 / 4))
			pgraph[e] = 0;
		//logerror("NV_2A: write PGRAPH[%06X]=%08X\n",offset*4-0x00400000,data & mem_mask);
	}
	else if ((offset >= 0x00600000 / 4) && (offset < 0x00601000 / 4)) {
		int e = offset - 0x00600000 / 4;
		if (e >= (sizeof(pcrtc) / sizeof(UINT32)))
			return;
		old = pcrtc[e];
		COMBINE_DATA(pcrtc + e);
		if (e == 0x100 / 4) {
			pcrtc[e] = old & ~data;
			update_int = true;
		}
		if (e == 0x140 / 4)
			update_int = true;
		if (e == 0x800 / 4) {
			displayedtarget = (UINT32 *)direct_access_ptr(pcrtc[e]);
#ifdef LOG_NV2A
			printf("crtc buffer %08X\n\r", data);
#endif
		}
		//logerror("NV_2A: write PCRTC[%06X]=%08X\n",offset*4-0x00600000,data & mem_mask);
	}
	else if ((offset >= 0x00000000 / 4) && (offset < 0x00001000 / 4)) {
		int e = offset - 0x00000000 / 4;
		if (e >= (sizeof(pmc) / sizeof(UINT32)))
			return;
		COMBINE_DATA(pmc + e);
		//logerror("NV_2A: write PMC[%06X]=%08X\n",offset*4-0x00000000,data & mem_mask);
	}
	else if ((offset >= 0x00800000 / 4) && (offset < 0x00900000 / 4)) {
		// 32 channels size 0x10000 each, 8 subchannels per channel size 0x2000 each
		int chanel, subchannel, suboffset;
		//int method, count, handle, objclass;

		suboffset = offset - 0x00800000 / 4;
		chanel = (suboffset >> (16 - 2)) & 31;
		subchannel = (suboffset >> (13 - 2)) & 7;
		suboffset = suboffset & 0x7ff;
		//logerror("NV_2A: write channel[%02X,%d,%04X]=%08X\n",chanel,subchannel,suboffset*4,data & mem_mask);
		if (suboffset >= 0x80 / 4)
			return;
		COMBINE_DATA(&channel[chanel][subchannel].regs[suboffset]);
		if ((suboffset == 0x40 / 4) || (suboffset == 0x44 / 4)) { // DMA_PUT or DMA_GET
			UINT32 *dmaput, *dmaget;

			dmaput = &channel[chanel][subchannel].regs[0x40 / 4];
			dmaget = &channel[chanel][subchannel].regs[0x44 / 4];
			//printf("dmaget %08X dmaput %08X\n\r",*dmaget,*dmaput);
			if ((*dmaput == 0x048cf000) && (*dmaget == 0x07f4d000)) {
				*dmaget = *dmaput;
				puller_waiting = 0;
				puller_timer->enable(false);
				return;
			}
			if (*dmaget != *dmaput) {
				if (puller_waiting == 0) {
					puller_channel = chanel;
					puller_subchannel = subchannel;
					puller_space = &space;
					puller_timer->enable();
					puller_timer->adjust(attotime::zero);
				}
			}
		}
	}
	//else
	//      logerror("NV_2A: write at %08X mask %08X value %08X\n",0xfd000000+offset*4,mem_mask,data);
	if (update_int == true) {
		if (update_interrupts() == true)
			interruptdevice->ir3_w(1); // IRQ 3
		else
			interruptdevice->ir3_w(0); // IRQ 3
	}
}

void nv2a_renderer::savestate_items()
{
}

void nv2a_renderer::start(address_space *cpu_space)
{
	basemempointer = (UINT8 *)cpu_space->get_read_ptr(0);
	puller_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(nv2a_renderer::puller_timer_work), this), (void *)"NV2A Puller Timer");
	puller_timer->enable(false);
}

void nv2a_renderer::set_interrupt_device(pic8259_device *device)
{
	interruptdevice = device;
}
