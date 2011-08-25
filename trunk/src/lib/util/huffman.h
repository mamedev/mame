/***************************************************************************

    huffman.h

    Huffman compression routines.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef __HUFFMAN_H__

#include "osdcore.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum _huffman_error
{
	HUFFERR_NONE = 0,
	HUFFERR_OUT_OF_MEMORY,
	HUFFERR_TOO_MANY_BITS,
	HUFFERR_INVALID_DATA,
	HUFFERR_INPUT_BUFFER_TOO_SMALL,
	HUFFERR_OUTPUT_BUFFER_TOO_SMALL,
	HUFFERR_INTERNAL_INCONSISTENCY,
	HUFFERR_TOO_MANY_CONTEXTS
};
typedef enum _huffman_error huffman_error;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef UINT16 huffman_lookup_value;

typedef struct _huffman_context huffman_context;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

huffman_error huffman_create_context(huffman_context **context, int maxbits);
void huffman_free_context(huffman_context *context);

huffman_error huffman_import_tree(huffman_context *context, const UINT8 *source, UINT32 slength, UINT32 *actlength);
huffman_error huffman_export_tree(huffman_context *context, UINT8 *dest, UINT32 dlength, UINT32 *actlength);
huffman_error huffman_deltarle_import_tree(huffman_context *context, const UINT8 *source, UINT32 slength, UINT32 *actlength);
huffman_error huffman_deltarle_export_tree(huffman_context *context, UINT8 *dest, UINT32 dlength, UINT32 *actlength);

huffman_error huffman_compute_tree(huffman_context *context, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor);
huffman_error huffman_compute_tree_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor);
huffman_error huffman_deltarle_compute_tree(huffman_context *context, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor);
huffman_error huffman_deltarle_compute_tree_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor);

huffman_error huffman_encode_data(huffman_context *context, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 dlength, UINT32 *actlength);
huffman_error huffman_encode_data_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 dlength, UINT32 *actlength);
huffman_error huffman_deltarle_encode_data(huffman_context *context, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 dlength, UINT32 *actlength);
huffman_error huffman_deltarle_encode_data_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 dlength, UINT32 *actlength);

huffman_error huffman_decode_data(huffman_context *context, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dwidth, UINT32 dheight, UINT32 dstride, UINT32 dxor, UINT32 *actlength);
huffman_error huffman_decode_data_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dwidth, UINT32 dheight, UINT32 dstride, UINT32 dxor, UINT32 *actlength);
huffman_error huffman_deltarle_decode_data(huffman_context *context, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dwidth, UINT32 dheight, UINT32 dstride, UINT32 dxor, UINT32 *actlength);
huffman_error huffman_deltarle_decode_data_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dwidth, UINT32 dheight, UINT32 dstride, UINT32 dxor, UINT32 *actlength);

#endif
