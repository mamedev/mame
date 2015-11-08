// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"

// This just attempts to parse some of the header structures in the DVI data Dragon Gun uses...
// see http://www.fileformat.info/format/dvi/egff.htm
// I don't know if there are any software decoders for this format, I think it was a hardware solution?
// The Dragon Gun data doesn't seem to have all fields filled in..
//
// Hopefully somebody who actually understands video (de)compression can actually flesh this out...


//#define dviprintf printf
#define dviprintf device->logerror


struct DVI_Header
{
	UINT32 dv1;     // header ID (VDVI)
	UINT16 dv2;     // header size (sometimes just 1)
	UINT16 dv3;     // header version
	UINT32 dv4;     // offset of annotation data
};


struct AVSS_Header
{
	UINT32 av1;       // header ID (AVSS)
	UINT16 av2;     // header size
	UINT16 av3;     // header version
	UINT16 av4;       // number of stream groups
	UINT16 av5;       // size of each stream group
	UINT32 av6;       // offset to first stream group
	UINT16 av7;       // format of stream groups
	UINT16 av8;       // size of stream header
	UINT16 av9;       // format of stream header
	UINT16 av10;      // number of stream headers
	UINT32 av11;      // offset to stream structures array
	UINT32 av12;      // offset to substream headers array
	UINT32 av13;      // number of labels
	UINT32 av14;      // offset to first label
	UINT16 av15;      // label size
	UINT16 av16;      // label format
	UINT32 av17;      // offset to video sequence header
	UINT16 av18;      // size of video sequence header
	UINT16 av19;      // version of frame headers
	UINT32 av20;      // number of frame headers
	UINT32 av21;      // size of frame headers
	UINT32 av22;      // offset to first frame
	UINT32 av23;      // offset of last frame
	UINT16 av24;      // size of frame header
	UINT16 av25;      // size of frame dictionary
	UINT32 av26;      // offset of frame dictionary
	UINT16 av27;      // format of frame dictionary
	UINT16 av28;      // framerate
	UINT32 av29;      // streaming data?
	UINT32 av30;      // unused
	UINT8  av31[32];  // unused
};

struct STRM_Header
{
	UINT32 st1;       // header ID (STRM)
	UINT16 st2;       // header size
	UINT16 st3;       // stream type
	UINT16 st4;       // stream subtype
	UINT16 st5;       // number of substream headers
	UINT16 st6;       // next stream ID
	UINT16 st7;       // group IP
	UINT16 st8;       // unused (Pad)
	UINT16 st9;       // frame size flag..
	UINT32 st10;      // max data per frame
	UINT32 st11;      // offset to first substream header
	UINT8  st12[16];  // name
};

#if 0
struct AUDI_Header
{
	UINT32 au1;      // header iID (AUDI)
	UINT16 au2;      // header size
	UINT16 au3;      // header format
	UINT8  au4[80];  // original media filename
	UINT32 au5;      // original media frame ID
	UINT16 au6;    // original media frame ID
	UINT16 au7;      // unused (Pad)
	UINT32 au8;      // number of frames
	UINT32 au9;      // offset of next header
	UINT8  au10[16]; // library stream name
	UINT8  au11[16]; // compression algorithm name
	UINT32 au12;     // sample freq
	UINT16 au13;     // filter cutoff
	UINT16 au14;     // unused
	UINT16 au15;     // left volume
	UINT16 au16;     // right volume
	UINT32 au17;     // unused
	UINT32 au18;     // first frame ID
	UINT32 au19;     // mono / stereo
	UINT16 au20;     // playback rate
	UINT16 au21;     // unused (Pad)
	UINT32 au22;     // Digital Compression ID
};
#endif

struct CIMG_Header
{
	UINT32 ci1;     // header ID (CIMG)
	UINT16 ci2;     // header size
	UINT16 ci3;     // header format
	UINT8  ci4[80]; // source media filename
	UINT32 ci5;     // source media frame ID
	UINT16 ci6;     // source media stream ID
	UINT16 ci7;     // Unused (Pad value)
	UINT32 ci8;     // Frames before next header
	UINT32 ci9;     // Next Header Pos
	UINT16 ci10;    // X Pos
	UINT16 ci11;    // Y Pos
	UINT16 ci12;    // Width
	UINT16 ci13;    // Height
	UINT16 ci14;    // X Crop
	UINT16 ci15;    // Y Crop
	UINT16 ci16;    // Unused
	UINT16 ci17;    // Unused
	UINT32 ci18;    // Interframe Frequency
	UINT16 ci19;    // Min Buffer Size
	UINT16 ci20;    // Max Buffer Size
	UINT16 ci21;    // Decompression ID
	UINT16 ci22;    // Unused (Pad Value)
	UINT32 ci23;    // Digitial Compression ID
};

struct FHEAD_Header
{
	UINT32 fh1;              // frame number
	UINT32 fh2;              // offset of previous frame
	UINT32 fh3;              // checksum
	//UINT32 framesizearray? // just the frame data?
};

struct FDICT_Header
{
	UINT32 fd1; // offset of frame header
};

static UINT32 R32(UINT8 **currptr)
{
	UINT32 ret = 0;
	ret |= *(*currptr)++; ret <<=8;
	ret |= *(*currptr)++; ret <<=8;
	ret |= *(*currptr)++; ret <<=8;
	ret |= *(*currptr)++;

	return ret;
}

static UINT16 R16(UINT8 **currptr)
{
	UINT16 ret = 0;
	ret |= *(*currptr)++; ret <<=8;
	ret |= *(*currptr)++;

	return ret;
}

static UINT8 R8(UINT8 **currptr)
{
	UINT8 ret = 0;
	ret |= *(*currptr)++;
	return ret;
}

void process_dvi_data(device_t *device,UINT8* dvi_data, int baseoffset, int regionsize)
{
	DVI_Header DVI;

	UINT8* currptr = dvi_data + baseoffset;

	DVI.dv1 = R32(&currptr);
	DVI.dv2 = R16(&currptr);
	DVI.dv3 = R16(&currptr);
	DVI.dv4 = R32(&currptr);

	dviprintf("DVI Header\n");
	dviprintf("Header ID          %08x %c%c%c%c\n", DVI.dv1, (DVI.dv1>>24)&0xff,  (DVI.dv1>>16)&0xff,  (DVI.dv1>>8)&0xff,  (DVI.dv1>>0)&0xff);
	dviprintf("Header Size (or 1) %04x\n", DVI.dv2);
	dviprintf("Header Version     %04x\n", DVI.dv3);
	dviprintf("Annotation Offset  %08x\n", DVI.dv4);
	if (DVI.dv1 != 0x56445649) { dviprintf("Header Error\n"); return; }
	dviprintf("\n");

	AVSS_Header AVSS;

	AVSS.av1 = R32(&currptr);
	AVSS.av2 = R16(&currptr);
	AVSS.av3 = R16(&currptr);
	AVSS.av4 = R16(&currptr);
	AVSS.av5 = R16(&currptr);
	AVSS.av6 = R32(&currptr);
	AVSS.av7 = R16(&currptr);
	AVSS.av8 = R16(&currptr);
	AVSS.av9 = R16(&currptr);
	AVSS.av10 = R16(&currptr);
	AVSS.av11 = R32(&currptr);
	AVSS.av12 = R32(&currptr);
	AVSS.av13 = R32(&currptr);
	AVSS.av14 = R32(&currptr);
	AVSS.av15 = R16(&currptr);
	AVSS.av16 = R16(&currptr);
	AVSS.av17 = R32(&currptr);
	AVSS.av18 = R16(&currptr);
	AVSS.av19 = R16(&currptr);
	AVSS.av20 = R32(&currptr);
	AVSS.av21 = R32(&currptr);
	AVSS.av22 = R32(&currptr);
	AVSS.av23 = R32(&currptr);
	AVSS.av24 = R16(&currptr);
	AVSS.av25 = R16(&currptr);
	AVSS.av26 = R32(&currptr);
	AVSS.av27 = R16(&currptr);
	AVSS.av28 = R16(&currptr);
	AVSS.av29 = R32(&currptr);
	AVSS.av30 = R32(&currptr);
	for (int i=0;i<32;i++) { AVSS.av31[i] = R8(&currptr); }

	dviprintf(" AVSS Header\n");
	dviprintf(" Header ID          %08x %c%c%c%c\n", AVSS.av1, (AVSS.av1>>24)&0xff,  (AVSS.av1>>16)&0xff,  (AVSS.av1>>8)&0xff,  (AVSS.av1>>0)&0xff);
	dviprintf(" Header Size        %04x\n", AVSS.av2);
	dviprintf(" Header Version     %04x\n", AVSS.av3);
	dviprintf(" Stream Group Count %04x\n", AVSS.av4);
	dviprintf(" Stream Group Size  %04x\n", AVSS.av5);
	dviprintf(" Stream Group Offs  %08x\n", AVSS.av6);
	dviprintf(" Stream Group Vers  %04x\n", AVSS.av7);
	dviprintf(" Stream Size        %04x\n", AVSS.av8);
	dviprintf(" Stream Version     %04x\n", AVSS.av9);
	dviprintf(" Stream Count       %04x\n", AVSS.av10);
	dviprintf(" Stream Offset      %08x\n", AVSS.av11);
	dviprintf(" Header Pool Offset %08x\n", AVSS.av12);
	dviprintf(" Label Count        %08x\n", AVSS.av13);
	dviprintf(" Label Offset       %08x\n", AVSS.av14);
	dviprintf(" Label Size         %04x\n", AVSS.av15);
	dviprintf(" Label Version      %04x\n", AVSS.av16);
	dviprintf(" Vid Seq Hdr Offset %08x\n", AVSS.av17);
	dviprintf(" Vid Seq Hdr Size   %04x\n", AVSS.av18);
	dviprintf(" Frame Version      %04x\n", AVSS.av19);
	dviprintf(" Frame Count        %08x\n", AVSS.av20);
	dviprintf(" Frame Size         %08x\n", AVSS.av21);
	dviprintf(" First Frame Offset %08x\n", AVSS.av22);
	dviprintf(" EO Frame Offset    %08x\n", AVSS.av23);
	dviprintf(" Frame Header Size  %04x\n", AVSS.av24);
	dviprintf(" Frame Dir Size     %04x\n", AVSS.av25);
	dviprintf(" Frame Dir Offset   %08x\n", AVSS.av26);
	dviprintf(" Frame Dir Vers     %04x\n", AVSS.av27);
	dviprintf(" Frame PerSec       %04x\n", AVSS.av28);
	dviprintf(" UpdateFlag         %08x\n", AVSS.av29);
	dviprintf(" FreeBlock          %08x\n", AVSS.av30);
	dviprintf(" Patch              ");  for (int i=0;i<32;i++) { dviprintf("%02x", AVSS.av31[i]); }; dviprintf("\n");
	dviprintf("\n");

	for (int s=0;s<AVSS.av10;s++)
	{
		STRM_Header STRM;

		STRM.st1 = R32(&currptr);
		STRM.st2 = R16(&currptr);
		STRM.st3 = R16(&currptr);
		STRM.st4 = R16(&currptr);
		STRM.st5 = R16(&currptr);
		STRM.st6 = R16(&currptr);
		STRM.st7 = R16(&currptr);
		STRM.st8 = R16(&currptr);
		STRM.st9 = R16(&currptr);
		STRM.st10 = R32(&currptr);
		STRM.st11 = R32(&currptr);
		for (int i=0;i<16;i++) { STRM.st12[i] = R8(&currptr); }

		dviprintf("     STRM Header\n");
		dviprintf("     Header ID          %08x %c%c%c%c\n", STRM.st1, (STRM.st1>>24)&0xff,  (STRM.st1>>16)&0xff,  (STRM.st1>>8)&0xff,  (STRM.st1>>0)&0xff);
		dviprintf("     Header Size        %04x\n", STRM.st2);
		dviprintf("     Type               %04x\n", STRM.st3);
		dviprintf("     SubType            %04x\n", STRM.st4);
		dviprintf("     Header Count       %04x\n", STRM.st5);
		dviprintf("     Next Stream Number %04x\n", STRM.st6);
		dviprintf("     Stream Group Numbr %04x\n", STRM.st7);
		dviprintf("     Padding            %04x\n", STRM.st8);
		dviprintf("     Flag               %04x\n", STRM.st9);
		dviprintf("     FrameSize          %08x\n", STRM.st10);
		dviprintf("     FirstHeaderOffset  %08x\n", STRM.st11);
		dviprintf("     Name               ");  for (int i=0;i<16;i++) { dviprintf("%02x", STRM.st12[i]); }; dviprintf("\n");
		dviprintf("\n");

		UINT8* subptr = dvi_data+STRM.st11+baseoffset;

		for (int h=0;h<STRM.st5;h++)
		{
			// I believe in these can be either CIMG or AUDI blocks, but we don't have any of the latter
			CIMG_Header CIMG;


			CIMG.ci1 = R32(&subptr);
			CIMG.ci2 = R16(&subptr);
			CIMG.ci3 = R16(&subptr);
			for (int i=0;i<80;i++) { CIMG.ci4[i] = R8(&subptr); }
			CIMG.ci5 = R32(&subptr);
			CIMG.ci6 = R16(&subptr);
			CIMG.ci7 = R16(&subptr);
			CIMG.ci8 = R32(&subptr);
			CIMG.ci9 = R32(&subptr);
			CIMG.ci10 = R16(&subptr);
			CIMG.ci11 = R16(&subptr);
			CIMG.ci12 = R16(&subptr);
			CIMG.ci13 = R16(&subptr);
			CIMG.ci14 = R16(&subptr);
			CIMG.ci15 = R16(&subptr);
			CIMG.ci16 = R16(&subptr);
			CIMG.ci17 = R16(&subptr);
			CIMG.ci18 = R32(&subptr);
			CIMG.ci19 = R16(&subptr);
			CIMG.ci20 = R16(&subptr);
			CIMG.ci21 = R16(&subptr);
			CIMG.ci22 = R16(&subptr);
			CIMG.ci23 = R32(&subptr);

			dviprintf("         CIMG Header\n");
			dviprintf("         Header ID          %08x %c%c%c%c\n", CIMG.ci1, (CIMG.ci1>>24)&0xff,  (CIMG.ci1>>16)&0xff,  (CIMG.ci1>>8)&0xff,  (CIMG.ci1>>0)&0xff);
			dviprintf("         Header Size        %04x\n", CIMG.ci2);
			dviprintf("         Version            %04x\n", CIMG.ci3);
			dviprintf("         Original Name      ");  for (int i=0;i<80;i++) { dviprintf("%02x", CIMG.ci4[i]); }; dviprintf("\n");
			dviprintf("         Original Frame     %08x\n", CIMG.ci5);
			dviprintf("         Original Stream    %04x\n", CIMG.ci6);
			dviprintf("         Padding            %04x\n", CIMG.ci7);
			dviprintf("         Frame Count        %08x\n", CIMG.ci8);
			dviprintf("         Next Header Offset %08x\n", CIMG.ci9);
			dviprintf("         X Position         %04x\n", CIMG.ci10);
			dviprintf("         Y Position         %04x\n", CIMG.ci11);
			dviprintf("         X Length           %04x\n", CIMG.ci12);
			dviprintf("         Y Length           %04x\n", CIMG.ci13);
			dviprintf("         X Crop             %04x\n", CIMG.ci14);
			dviprintf("         Y Crop             %04x\n", CIMG.ci15);
			dviprintf("         Drop Frame         %04x\n", CIMG.ci16);
			dviprintf("         Drop Phrase        %04x\n", CIMG.ci17);
			dviprintf("         Still Period       %08x\n", CIMG.ci18);
			dviprintf("         Buffer Minimum     %04x\n", CIMG.ci19);
			dviprintf("         Buffer Maximum     %04x\n", CIMG.ci20);
			dviprintf("         Decode Algorithm   %04x\n", CIMG.ci21);
			dviprintf("         Padding(2)         %04x\n", CIMG.ci22);
			dviprintf("         DCFID              %08x\n", CIMG.ci23);
			dviprintf("\n");

			subptr = dvi_data+CIMG.ci9+baseoffset;
		}

	}

	/* Frame Dictionaries etc. */
	/* is this just a seek table of sorts? the first doesn't have an entry (pointed to by First Frame Offset - AVSS.av22) the first two here are actually bad values, and the last one points to nothing */
	/* some entries also seem to point to the wrong places?? I'm guessing this isn't used when playing back the data ... it seems like the upper and lower words are out of sync... */
	dviprintf("Frame Dictionaries\n\n");

	UINT8* frameptr = dvi_data + AVSS.av26 + 2 + baseoffset; // +2 ??
//  UINT8* frameptr = dvi_data + AVSS.av26 + baseoffset;

	for (int f=0;f<AVSS.av20;f++)
	{
		FDICT_Header FDICT;
		FDICT.fd1 = R32(&frameptr);

		FDICT.fd1 = ((FDICT.fd1 & 0xffff0000)>>16) |  ((FDICT.fd1 & 0x0000ffff)<<16);

		dviprintf(" %04d Frame Offset %08x\n", f, FDICT.fd1);

			UINT8* frameptr2 = dvi_data + ((FDICT.fd1 + baseoffset)&(regionsize-1)) ;
		FHEAD_Header FHEAD;
		FHEAD.fh1 = R32(&frameptr2);
		FHEAD.fh2 = R32(&frameptr2);
		FHEAD.fh3 = R32(&frameptr2);

		dviprintf("     Frame Num         %08x\n", FHEAD.fh1);
		dviprintf("     Previous Offset   %08x\n", FHEAD.fh2);
		dviprintf("     Frame Checksum    %08x\n", FHEAD.fh3);


	}

	dviprintf("\n");



}
