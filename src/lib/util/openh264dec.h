// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
Interface to dynamically loaded H.264 decoder, compatible with Cisco OpenH264

Original OpenH264 license:

Copyright (c) 2013, Cisco Systems
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MAME_UTIL_OPENH264DEC_H
#define MAME_UTIL_OPENH264DEC_H

#include <memory>


namespace util::openh264 {

enum EWelsErr
{
	ERR_NONE = 0,
	ERR_INVALID_PARAMETERS = 1,
	ERR_MALLOC_FAILED = 2,
	ERR_API_FAILED = 3,
	ERR_BOUND = 31
};

enum DECODING_STATE
{
	dsErrorFree = 0x00,
	dsFramePending = 0x01,
	dsRefLost = 0x02,
	dsBitstreamError = 0x04,
	dsDepLayerLost = 0x08,
	dsNoParamSets = 0x10,
	dsDataErrorConcealed = 0x20,
	dsRefListNullPtrs = 0x40,
	dsInvalidArgument = 0x1000,
	dsInitialOptExpected = 0x2000,
	dsOutOfMemory = 0x4000,
	dsDstBufNeedExpan = 0x8000
};

enum DECODER_OPTION
{
	DECODER_OPTION_END_OF_STREAM = 1,
	DECODER_OPTION_VCL_NAL,
	DECODER_OPTION_TEMPORAL_ID,
	DECODER_OPTION_FRAME_NUM,
	DECODER_OPTION_IDR_PIC_ID,
	DECODER_OPTION_LTR_MARKING_FLAG,
	DECODER_OPTION_LTR_MARKED_FRAME_NUM,
	DECODER_OPTION_ERROR_CON_IDC,
	DECODER_OPTION_TRACE_LEVEL,
	DECODER_OPTION_TRACE_CALLBACK,
	DECODER_OPTION_TRACE_CALLBACK_CONTEXT,
	DECODER_OPTION_GET_STATISTICS,
	DECODER_OPTION_GET_SAR_INFO,
	DECODER_OPTION_PROFILE,
	DECODER_OPTION_LEVEL,
	DECODER_OPTION_STATISTICS_LOG_INTERVAL,
	DECODER_OPTION_IS_REF_PIC,
	DECODER_OPTION_NUM_OF_FRAMES_REMAINING_IN_BUFFER,
	DECODER_OPTION_NUM_OF_THREADS
};

enum ERROR_CON_IDC
{
	ERROR_CON_DISABLE = 0,
	ERROR_CON_FRAME_COPY,
	ERROR_CON_SLICE_COPY,
	ERROR_CON_FRAME_COPY_CROSS_IDR,
	ERROR_CON_SLICE_COPY_CROSS_IDR,
	ERROR_CON_SLICE_COPY_CROSS_IDR_FREEZE_RES_CHANGE,
	ERROR_CON_SLICE_MV_COPY_CROSS_IDR,
	ERROR_CON_SLICE_MV_COPY_CROSS_IDR_FREEZE_RES_CHANGE
};

enum VIDEO_BITSTREAM_TYPE
{
	VIDEO_BITSTREAM_AVC = 0,
	VIDEO_BITSTREAM_SVC = 1,
	VIDEO_BITSTREAM_DEFAULT = VIDEO_BITSTREAM_SVC
};

struct SSysMEMBuffer
{
	int iWidth;
	int iHeight;
	int iFormat;
	int iStride[2];
};

struct SBufferInfo
{
	int iBufferStatus;
	unsigned long long uiInBsTimeStamp;
	unsigned long long uiOutYuvTimeStamp;
	union
	{
		SSysMEMBuffer sSystemBuffer;
	} UsrData;
	unsigned char *pDst[3];
};

struct SVideoProperty
{
	unsigned int size;
	VIDEO_BITSTREAM_TYPE eVideoBsType;
};

struct SDecodingParam
{
	char *pFileNameRestructured;
	unsigned int uiCpuLoad;
	unsigned char uiTargetDqLayer;
	ERROR_CON_IDC eEcActiveIdc;
	bool bParseOnly;
	SVideoProperty sVideoProperty;
};

struct SParserBsInfo
{
  int iNalNum;
  int *pNalLenInByte;
  unsigned char *pDstBuff;
  int iSpsWidthInPixel;
  int iSpsHeightInPixel;
  unsigned long long uiInBsTimeStamp;
  unsigned long long uiOutBsTimeStamp;
};

class ISVCDecoder
{
public:
	virtual long Initialize(SDecodingParam const *) = 0;
	virtual long Uninitialize() = 0;
	virtual DECODING_STATE DecodeFrame(unsigned char const *, int, unsigned char **, int *, int &, int &) = 0;
	virtual DECODING_STATE DecodeFrameNoDelay(unsigned char const *, int, unsigned char **, SBufferInfo *) = 0;
	virtual DECODING_STATE DecodeFrame2(unsigned char const *, int, unsigned char **, SBufferInfo *) = 0;
	virtual DECODING_STATE FlushFrame(unsigned char **, SBufferInfo *) = 0;
	virtual DECODING_STATE DecodeParser(unsigned char const *, int, SParserBsInfo *) = 0;
	virtual DECODING_STATE DecodeFrameEx(unsigned char const *, int, unsigned char *, int, int &, int &, int &, int &) = 0;
	virtual long SetOption(DECODER_OPTION, void *) = 0;
	virtual long GetOption(DECODER_OPTION, void *) = 0;
	virtual ~ISVCDecoder() { }
};


class decoder_api
{
public:
	using ptr = std::unique_ptr<decoder_api>;

	virtual ~decoder_api() { }
	virtual long create_decoder(ISVCDecoder **) = 0;
	virtual void destroy_decoder(ISVCDecoder *) = 0;

	static ptr initialize();
};

} // namespace util::openh264

#endif // MAME_UTIL_OPENH264DEC_H
