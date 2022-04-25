// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner, Andrew Zaferakis

/////////////////////////////////
/// Hyper NeoGeo 64 - 3D bits ///
/////////////////////////////////

// Polygon rasterizer interface
hng64_poly_renderer::hng64_poly_renderer(hng64_state& state)
	: poly_manager<float, hng64_poly_data, 7>(state.machine())
	, m_state(state)
	, m_colorBuffer3d(state.m_screen->visible_area().width(), state.m_screen->visible_area().height())
{
	const int32_t bufferSize = state.m_screen->visible_area().width() * state.m_screen->visible_area().height();
	m_depthBuffer3d = std::make_unique<float[]>(bufferSize);
}



/* Hardware calls these '3d buffers'

    They're only read during the startup check, never written

    They're definitely mirrored in the startup test, according to ElSemi

    The games run in interlace mode, so buffer resolution can be half the effective screen height

    30100000-3011ffff is framebuffer A0 (512x256 8-bit?) (pal data?)
    30120000-3013ffff is framebuffer A1 (512x256 8-bit?) (pal data?)
    30140000-3015ffff is ZBuffer A  (512x256 8-bit?)
*/

uint32_t hng64_state::hng64_fbram1_r(offs_t offset)
{
	return m_fbram1[offset];
}

void hng64_state::hng64_fbram1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA (&m_fbram1[offset]);
}

uint32_t hng64_state::hng64_fbram2_r(offs_t offset)
{
	return m_fbram2[offset];
}

void hng64_state::hng64_fbram2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA (&m_fbram2[offset]);
}

// The 3d 'display list'
void hng64_state::dl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dl[offset]);
}

void hng64_state::dl_unk_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("%s: dl_unk_w %08x (%08x)\n", machine().describe_context(), data, mem_mask);
}

void hng64_state::dl_upload_w(uint32_t data)
{
	// Data is:
	// 00000b50 for the sams64 games
	// 00000f00 for everything else
	// TODO: different param for the two sams64 games, less FIFO to process?

	// This is written after the game uploads 16 packets, each 16 words long
	// We're assuming it to be a 'send to 3d hardware' trigger.
	// This can be called multiple times per frame (at least 2, as long as it gets the expected interrupt / status flags)
g_profiler.start(PROFILER_USER1);
	for(int packetStart = 0; packetStart < 0x100; packetStart += 16)
	{
		// Send it off to the 3d subsystem.
		hng64_command3d(&m_dl[packetStart]);
	}

	// Schedule a small amount of time to let the 3d hardware rasterize the display buffer
	m_3dfifo_timer->adjust(m_maincpu->cycles_to_attotime(0x200*8));
g_profiler.stop();
}

TIMER_CALLBACK_MEMBER(hng64_state::hng64_3dfifo_processed)
{
	set_irq(0x0008);
}

void hng64_state::dl_control_w(uint32_t data)
{
	/* m_activeDisplayList is not currently connected to anything, it seems unlikely there are different banks.
	   games typically write up to 8 lots of 0x200 data, writing bit 0 between them

	    bit 0 (0x01)  process DMA from 3d FIFO to framebuffer?
	    bit 1 (0x02)  written before first set of dl data each frame on some games, but not on SS?
	    bit 2 (0x04)  reset buffer count (startup only)
	*/

	/*
	if (data & 0x01)
	    m_activeDisplayList = 0;
	else if (data & 0x02)
	    m_activeDisplayList = 1;
	*/

	/*
	printf("dl_control_w %08x %08x\n", data, mem_mask);

	if(data & 2) // swap buffers
	{
	    clear3d();
	}
	*/
}

uint32_t hng64_state::dl_vreg_r()
{
	/* tested with possible masked bits 0xf003 (often masking 0xf000 or 0x0003)

	  various wait loops on bit 0x02 (bit 1) after sending 3d commands
	  tests failing on other bits can cause display list writes to be skipped

	  Sams64 after the title screen) tests bit 15 of this to be high, unknown reason

	*/
	return 0;
}

////////////////////
// 3d 'Functions' //
////////////////////

void hng64_state::printPacket(const uint16_t* packet, int hex)
{
	if (hex)
	{
		printf("Packet : %04x %04x  2:%04x %04x  4:%04x %04x  6:%04x %04x  8:%04x %04x  10:%04x %04x  12:%04x %04x  14:%04x %04x\n",
				packet[0],  packet[1],
				packet[2],  packet[3],
				packet[4],  packet[5],
				packet[6],  packet[7],
				packet[8],  packet[9],
				packet[10], packet[11],
				packet[12], packet[13],
				packet[14], packet[15]);
	}
	else
	{
		printf("Packet : %04x %3.4f  2:%3.4f %3.4f  4:%3.4f %3.4f  6:%3.4f %3.4f  8:%3.4f %3.4f  10:%3.4f %3.4f  12:%3.4f %3.4f  14:%3.4f %3.4f\n",
				packet[0],            uToF(packet[1] )*128,
				uToF(packet[2] )*128, uToF(packet[3] )*128,
				uToF(packet[4] )*128, uToF(packet[5] )*128,
				uToF(packet[6] )*128, uToF(packet[7] )*128,
				uToF(packet[8] )*128, uToF(packet[9] )*128,
				uToF(packet[10])*128, uToF(packet[11])*128,
				uToF(packet[12])*128, uToF(packet[13])*128,
				uToF(packet[14])*128, uToF(packet[15])*128);
	}
}

// Operation 0001
// Camera transformation.
void hng64_state::setCameraTransformation(const uint16_t* packet)
{
	/*//////////////
	// PACKET FORMAT
	// [0]  - 0001 ... ID
	// [1]  - xxxx ... Extrinsic camera matrix
	// [2]  - xxxx ... Extrinsic camera matrix
	// [3]  - xxxx ... Extrinsic camera matrix
	// [4]  - xxxx ... Extrinsic camera matrix
	// [5]  - xxxx ... Extrinsic camera matrix
	// [6]  - xxxx ... Extrinsic camera matrix
	// [7]  - xxxx ... Extrinsic camera matrix
	// [8]  - xxxx ... Extrinsic camera matrix
	// [9]  - xxxx ... Extrinsic camera matrix
	// [10] - xxxx ... Extrinsic camera matrix
	// [11] - xxxx ... Extrinsic camera matrix
	// [12] - xxxx ... Extrinsic camera matrix
	// [13] - ???? ... ? Flips per-frame during fatfurwa 'HNG64'
	// [14] - ???? ... ? Could be some floating-point values during buriki 'door run'
	// [15] - ???? ... ? Same as 13 & 14
	////////////*/
	// CAMERA TRANSFORMATION MATRIX
	m_cameraMatrix[0]  = uToF(packet[1]);
	m_cameraMatrix[4]  = uToF(packet[2]);
	m_cameraMatrix[8]  = uToF(packet[3]);
	m_cameraMatrix[3]  = 0.0f;

	m_cameraMatrix[1]  = uToF(packet[4]);
	m_cameraMatrix[5]  = uToF(packet[5]);
	m_cameraMatrix[9]  = uToF(packet[6]);
	m_cameraMatrix[7]  = 0.0f;

	m_cameraMatrix[2]  = uToF(packet[7]);
	m_cameraMatrix[6]  = uToF(packet[8]);
	m_cameraMatrix[10] = uToF(packet[9]);
	m_cameraMatrix[11] = 0.0f;

	m_cameraMatrix[12] = uToF(packet[10]);
	m_cameraMatrix[13] = uToF(packet[11]);
	m_cameraMatrix[14] = uToF(packet[12]);
	m_cameraMatrix[15] = 1.0f;
}

// Operation 0010
// Lighting information
void hng64_state::setLighting(const uint16_t* packet)
{
	/*//////////////
	// PACKET FORMAT
	// [0]  - 0010 ... ID
	// [1]  - ???? ... ? Always zero
	// [2]  - ???? ... ? Always zero
	// [3]  - xxxx ... X light vector direction
	// [4]  - xxxx ... Y light vector direction
	// [5]  - xxxx ... Z light vector direction
	// [6]  - ???? ... ? Seems to be another light vector ?
	// [7]  - ???? ... ? Seems to be another light vector ?
	// [8]  - ???? ... ? Seems to be another light vector ?
	// [9]  - xxxx ... Strength according to sams64_2 (in combination with vector length) [0,512]
	// [10] - ???? ... ? Used in fatfurwa
	// [11] - ???? ... ? Used in fatfurwa
	// [12] - ???? ... ? Used in fatfurwa
	// [13] - ???? ... ? Used in fatfurwa
	// [14] - ???? ... ? Used in fatfurwa
	// [15] - ???? ... ? Used in fatfurwa
	////////////*/
	if (packet[1] != 0x0000) printf("ZOMG!  packet[1] in setLighting function is non-zero!\n");
	if (packet[2] != 0x0000) printf("ZOMG!  packet[2] in setLighting function is non-zero!\n");

	m_lightVector[0] = uToF(packet[3]);
	m_lightVector[1] = uToF(packet[4]);
	m_lightVector[2] = uToF(packet[5]);
	m_lightStrength = uToF(packet[9]);
}

// Operation 0011
// Palette / Model flags?
void hng64_state::set3dFlags(const uint16_t* packet)
{
	/*//////////////
	// PACKET FORMAT
	// [0]  - 0011 ... ID
	// [1]  - ???? ...
	// [2]  - ???? ...
	// [3]  - ???? ...
	// [4]  - ???? ...
	// [5]  - ???? ...
	// [6]  - ???? ...
	// [7]  - ???? ...
	// [8]  - xx?? ... Palette offset & ??
	// [9]  - ???? ... ? Very much used - seem to bounce around when characters are on screen
	// [10] - ???? ... ? ''  ''
	// [11] - ???? ... ? ''  ''
	// [12] - ???? ... ? ''  ''
	// [13] - ???? ... ? ''  ''
	// [14] - ???? ... ? ''  ''
	// [15] - ???? ... ? ''  ''
	////////////*/
	m_paletteState3d = (packet[8] & 0xff00) >> 8;
}

// Operation 0012
// Projection Matrix.
void hng64_state::setCameraProjectionMatrix(const uint16_t* packet)
{
	/*//////////////
	// PACKET FORMAT
	// [0]  - 0012 ... ID
	// [1]  - ???? ... ? Contains a value in buriki's 'how to play' - probably a projection window/offset.
	// [2]  - ???? ... ? Contains a value in buriki's 'how to play' - probably a projection window/offset.
	// [3]  - ???? ... ? Contains a value
	// [4]  - xxxx ... Camera projection Z scale
	// [5]  - xxxx ... Camera projection near Z
	// [6]  - xxxx ... Camera projection screen Z
	// [7]  - xxxx ... Camera projection (?)
	// [8]  - xxxx ... Camera projection (?)
	// [9]  - xxxx ... Camera projection (?)
	// [10] - xxxx ... Camera projection right  - confirmed by sams64_2
	// [11] - xxxx ... Camera projection left   - confirmed by sams64_2
	// [12] - xxxx ... Camera projection top    - confirmed by sams64_2
	// [13] - xxxx ... Camera projection bottom - confirmed by sams64_2
	// [14] - ???? ... ? Gets data during buriki door-run
	// [15] - ???? ... ? Gets data during buriki door-run
	////////////*/

	// Heisted from GLFrustum - 6 parameters...
	// Gives the x,y extents for the projection plane at screenZ
	const float left    = uToF(packet[11]);
	const float right   = uToF(packet[10]);
	const float top     = uToF(packet[12]);
	const float bottom  = uToF(packet[13]);

	// Mapping to a canonical view volume of the cube from (-1,-1,-1) to (1,1,1)
	// near maps to z value -1, screenZ (the projection plane) maps to z value 0
	// and far (which can be determined from near and screenZ) maps to z value of +1
	const float screenZ = uToF(packet[6]) * uToF(packet[4]) + uToF(packet[6]);
	const float near    = uToF(packet[5]) * uToF(packet[4]) + uToF(packet[5]);
	const float far     = -(screenZ * near)/(screenZ - 2.0f*near);

	m_projectionMatrix[0]  = (2.0f*screenZ)/(right-left);
	m_projectionMatrix[1]  = 0.0f;
	m_projectionMatrix[2]  = 0.0f;
	m_projectionMatrix[3]  = 0.0f;

	m_projectionMatrix[4]  = 0.0f;
	m_projectionMatrix[5]  = (2.0f*screenZ)/(top-bottom);
	m_projectionMatrix[6]  = 0.0f;
	m_projectionMatrix[7]  = 0.0f;

	m_projectionMatrix[8]  = (right+left)/(right-left);
	m_projectionMatrix[9]  = (top+bottom)/(top-bottom);
	m_projectionMatrix[10] = -((far+near)/(far-near));
	m_projectionMatrix[11] = -1.0f;

	m_projectionMatrix[12] = 0.0f;
	m_projectionMatrix[13] = 0.0f;
	m_projectionMatrix[14] = -((2.0f*far*near)/(far-near));
	m_projectionMatrix[15] = 0.0f;
}

// Operation 0100
// Polygon rasterization.
void hng64_state::recoverPolygonBlock(const uint16_t* packet, int& numPolys)
{
	//printPacket(packet, 1);

	/*//////////////
	// PACKET FORMAT
	// [0]  - 0100 ... ID
	// [1]  - ?--- ... Flags [?000 = ???
	//                        0?00 = ???
	//                        00?0 = ???
	//                        000? = ???]
	// [1]  - -?-- ... Flags [?000 = ???
	//                        0?00 = ???
	//                        00?0 = ???
	//                        000x = Dynamic palette bit]
	// [1]  - --?- ... Flags [?000 = ???
	//                        0?00 = ???
	//                        00?0 = ???
	//                        000? = ???]
	// [1]  - ---? ... Flags [x000 = Apply lighting bit
	//                        0?00 = ???
	//                        00?0 = ???
	//                        000? = ???]
	// [2]  - xxxx ... offset into ROM
	// [3]  - xxxx ... offset into ROM
	// [4]  - xxxx ... Transformation matrix
	// [5]  - xxxx ... Transformation matrix
	// [6]  - xxxx ... Transformation matrix
	// [7]  - xxxx ... Transformation matrix
	// [8]  - xxxx ... Transformation matrix
	// [9]  - xxxx ... Transformation matrix
	// [10] - xxxx ... Transformation matrix
	// [11] - xxxx ... Transformation matrix
	// [12] - xxxx ... Transformation matrix
	// [13] - xxxx ... Transformation matrix
	// [14] - xxxx ... Transformation matrix
	// [15] - xxxx ... Transformation matrix
	////////////*/

	float objectMatrix[16];
	setIdentity(objectMatrix);
	/////////////////
	// HEADER INFO //
	/////////////////
	// THE OBJECT TRANSFORMATION MATRIX
	objectMatrix[8] = uToF(packet[7]);
	objectMatrix[4] = uToF(packet[8]);
	objectMatrix[0] = uToF(packet[9]);
	objectMatrix[3] = 0.0f;

	objectMatrix[9] = uToF(packet[10]);
	objectMatrix[5] = uToF(packet[11]);
	objectMatrix[1] = uToF(packet[12]);
	objectMatrix[7] = 0.0f;

	objectMatrix[10] = uToF(packet[13]);
	objectMatrix[6 ] = uToF(packet[14]);
	objectMatrix[2 ] = uToF(packet[15]);
	objectMatrix[11] = 0.0f;

	objectMatrix[12] = uToF(packet[4]);
	objectMatrix[13] = uToF(packet[5]);
	objectMatrix[14] = uToF(packet[6]);
	objectMatrix[15] = 1.0f;

	uint32_t size[4];
	uint32_t address[4];
	uint32_t megaOffset;
	polygon lastPoly = { 0 };


	//////////////////////////////////////////////////////////
	// EXTRACT DATA FROM THE ADDRESS POINTED TO IN THE FILE //
	//////////////////////////////////////////////////////////
	/*//////////////////////////////////////////////
	// DIRECTLY-POINTED-TO FORMAT (7 words x 3 ROMs)
	// [0]  - lower word of sub-address 1
	// [1]  - lower word of sub-address 2
	// [2]  - upper word of all sub-addresses
	// [3]  - lower word of sub-address 3
	// [4]  - lower word of sub-address 4
	// [5]  - ???? always 0 ????
	// [6]  - number of chunks in sub-address 1 block
	// [7]  - number of chunks in sub-address 2 block
	// [8]  - ???? always 0 ????
	// [9]  - number of chunks in sub-address 3 block
	// [10] - number of chunks in sub-address 4 block
	// [11] - ? definitely used.
	// [12] - ? definitely used.
	// [13] - ? definitely used.
	// [14] - ? definitely used.
	// [15] - ???? always 0 ????
	// [16] - ???? always 0 ????
	// [17] - ???? always 0 ????
	// [18] - ???? always 0 ????
	// [19] - ???? always 0 ????
	// [20] - ???? always 0 ????
	//////////////////////////////////////////////*/

	// 3d ROM Offset
	uint16_t* threeDRoms = m_vertsrom;
	uint32_t  threeDOffset = (((uint32_t)packet[2]) << 16) | ((uint32_t)packet[3]);
	uint16_t* threeDPointer = &threeDRoms[threeDOffset * 3];

	if (threeDOffset >= m_vertsrom_size)
	{
		printf("Strange geometry packet: (ignoring)\n");
		printPacket(packet, 1);
		return;
	}

#if 0
	// Debug - ajg
	printf("%08x : ", threeDOffset*3*2);
	for (int k = 0; k < 7*3; k++)
	{
		printf("%04x ", threeDPointer[k]);
		if ((k % 3) == 2) printf(" ");
	}
	printf("\n");
#endif

	// There are 4 hunks per address.
	address[0] = threeDPointer[0];
	address[1] = threeDPointer[1];
	megaOffset = threeDPointer[2];

	address[2] = threeDPointer[3];
	address[3] = threeDPointer[4];
	if (threeDPointer[5] != 0x0000) printf("ZOMG!  3dPointer[5] is non-zero!\n");

	size[0]    = threeDPointer[6];
	size[1]    = threeDPointer[7];
	if (threeDPointer[8] != 0x0000) printf("ZOMG!  3dPointer[8] is non-zero!\n");

	size[2]    = threeDPointer[9];
	size[3]    = threeDPointer[10];
	//           ????         [11]; Used.
	//           ????         [12]; Used.
	//           ????         [13]; Used.
	//           ????         [14]; Used.

	if (threeDPointer[15] != 0x0000) printf("ZOMG!  3dPointer[15] is non-zero!\n");
	if (threeDPointer[16] != 0x0000) printf("ZOMG!  3dPointer[16] is non-zero!\n");
	if (threeDPointer[17] != 0x0000) printf("ZOMG!  3dPointer[17] is non-zero!\n");

	if (threeDPointer[18] != 0x0000) printf("ZOMG!  3dPointer[18] is non-zero!\n");
	if (threeDPointer[19] != 0x0000) printf("ZOMG!  3dPointer[19] is non-zero!\n");
	if (threeDPointer[20] != 0x0000) printf("ZOMG!  3dPointer[20] is non-zero!\n");

	// Concatenate the megaOffset with the addresses
	address[0] |= (megaOffset << 16);
	address[1] |= (megaOffset << 16);
	address[2] |= (megaOffset << 16);
	address[3] |= (megaOffset << 16);

	// Debug - ajg
	//uint32_t tdColor = 0xff000000;
	//if (threeDPointer[14] & 0x0002) tdColor |= 0x00ff0000;
	//if (threeDPointer[14] & 0x0001) tdColor |= 0x0000ff00;
	//if (threeDPointer[14] & 0x0000) tdColor |= 0x000000ff;

	// For all 4 polygon chunks
	for (int k = 0; k < 4; k++)
	{
		uint16_t* chunkOffset = &threeDRoms[address[k] * 3];
		for (int l = 0; l < size[k]; l++)
		{
			////////////////////////////////////////////
			// GATHER A SINGLE TRIANGLE'S INFORMATION //
			////////////////////////////////////////////
			// SINGLE POLY CHUNK FORMAT
			// [0] 0000 0000 cccc cccc    0 = always 0 | c = chunk type / format of data that follows (see below)
			// [1] t--l pppp pppp ssss    t = texture, always on for most games, on for the backgrounds only on sams64
			//                                if not set, u,v fields of vertices are direct palette indices, used on roadedge hng64 logo animation shadows
			//                            l = low-res texture?  p = palette?  s = texture sheet (1024 x 1024 pages)
			// [2] S?XX *--- -YY# ----    S = use 4x4 sub-texture pages?  ? = SNK logo roadedge / bbust2 / broken banners in xrally,  XX = horizontal subtexture  * = broken banners in xrally  YY = vertical subtexture  @ = broken banners in xrally

			// we currently use one of the palette bits to enable a different palette mode.. seems hacky...
			// looks like vertical / horizontal sub-pages might be 3 bits, not 2,  ? could be enable bit for that..

			// 'Welcome to South Africa' roadside banner on xrally | 000e 8c0d d870 or 0096 8c0d d870  (8c0d, d870 seems key 1000 1100 0000 1101
			//                                                                                                               1101 1000 0111 0000 )


			uint8_t chunkType = chunkOffset[0] & 0x00ff;

			// Debug - ajg
			if (chunkOffset[0] & 0xff00)
			{
				printf("Weird!  The top byte of the chunkType has a value %04x!\n", chunkOffset[0]);
				continue;
			}

			// Syntactical simplification
			polygon& currentPoly = m_polys[numPolys];

			// Debug - Colors polygons with certain flags bright blue! ajg
			currentPoly.debugColor = 0;
			//currentPoly.debugColor = tdColor;

			// Debug - ajg
			//printf("%d (%08x) : %04x %04x %04x\n", k, address[k]*3*2, chunkOffset[0], chunkOffset[1], chunkOffset[2]);
			//break;

			// TEXTURE
			// There may be more than just high & low res texture types, so I'm keeping texType as a uint8_t. */
			if (chunkOffset[1] & 0x1000) currentPoly.texType = 0x1;
			else                         currentPoly.texType = 0x0;

			currentPoly.texPageSmall       = (chunkOffset[2] & 0xc000)>>14;  // Just a guess.
			currentPoly.texPageHorizOffset = (chunkOffset[2] & 0x3800) >> 11;
			currentPoly.texPageVertOffset  = (chunkOffset[2] & 0x0070) >> 4;

			currentPoly.texIndex = chunkOffset[1] & 0x000f;

			// Flat shaded polygon, no texture, no lighting
			if (chunkOffset[1] & 0x8000)
				currentPoly.flatShade = false;
			else
				currentPoly.flatShade = true;

			// PALETTE
			currentPoly.palOffset = 0;
			currentPoly.palPageSize = 0x100;

			// FIXME: This isn't correct.
			//        Buriki & Xrally need this line.  Roads Edge needs it removed.
			//        So instead we're looking for a bit that is on for XRally & Buriki, but noone else.
			if (m_fbcontrol[2] & 0x20)
			{
				if (!m_roadedge_3d_hack)
					currentPoly.palOffset += 0x800;
			}

			//uint16_t explicitPaletteValue0 = ((chunkOffset[?] & 0x????) >> ?) * 0x800;
			uint16_t explicitPaletteValue1 = ((chunkOffset[1] & 0x0f00) >> 8) * 0x080;
			uint16_t explicitPaletteValue2 = ((chunkOffset[1] & 0x00f0) >> 4) * 0x008;

			// The presence of 0x00f0 *probably* sets 0x10-sized palette addressing.
			if (explicitPaletteValue2) currentPoly.palPageSize = 0x10;

			// Apply the dynamic palette offset if its flag is set, otherwise stick with the fixed one
			if ((packet[1] & 0x0100))
			{
				explicitPaletteValue1 = m_paletteState3d * 0x80;
				explicitPaletteValue2 = 0;      // This is probably hiding somewhere in operation 0011
			}

			currentPoly.palOffset += (explicitPaletteValue1 + explicitPaletteValue2);

#if 0
			if (((chunkOffset[2] & 0xc000) == 0x4000) && (m_screen->frame_number() & 1))
			{
			//  if (chunkOffset[2] == 0xd870)
				{
					currentPoly.debugColor = 0xffff0000;
					printf("%d (%08x) : %04x %04x %04x\n", k, address[k] * 3 * 2, chunkOffset[0], chunkOffset[1], chunkOffset[2]);
				}
			}
#endif

			uint8_t chunkLength = 0;
			switch(chunkType)
			{
			/*/////////////////////////
			// CHUNK TYPE BITS - These are very likely incorrect.
			// x--- ---- - 1 = Has only 1 vertex (part of a triangle fan/strip)
			// -x-- ---- -
			// --x- ---- -
			// ---x ---- -
			// ---- x--- -
			// ---- -x-- - 1 = Has per-vert UVs
			// ---- --x- -
			// ---- ---x - 1 = Has per-vert normals
			/////////////////////////*/

			// 33 word chunk, 3 vertices, per-vertex UVs & normals, per-face normal
			case 0x05:  // 0000 0101
			case 0x0f:  // 0000 1111
				for (int m = 0; m < 3; m++)
				{
					currentPoly.vert[m].worldCoords[0] = uToF(chunkOffset[3 + (9*m)]);
					currentPoly.vert[m].worldCoords[1] = uToF(chunkOffset[4 + (9*m)]);
					currentPoly.vert[m].worldCoords[2] = uToF(chunkOffset[5 + (9*m)]);
					currentPoly.vert[m].worldCoords[3] = 1.0f;
					currentPoly.n = 3;

					// chunkOffset[6 + (9*m)] is almost always 0080, but it's 0070 for the translucent globe in fatfurwa player select
					currentPoly.vert[m].texCoords[0] = uToF(chunkOffset[7 + (9*m)]);
					currentPoly.vert[m].texCoords[1] = uToF(chunkOffset[8 + (9*m)]);
					currentPoly.vert[m].texCoords[2] = 0.0f;
					currentPoly.vert[m].texCoords[3] = 1.0f;

					currentPoly.vert[m].normal[0] = uToF(chunkOffset[9  + (9*m)]);
					currentPoly.vert[m].normal[1] = uToF(chunkOffset[10 + (9*m)]);
					currentPoly.vert[m].normal[2] = uToF(chunkOffset[11 + (9*m)]);
					currentPoly.vert[m].normal[3] = 0.0f;

					if (currentPoly.flatShade)
						currentPoly.vert[m].colorIndex = chunkOffset[7 + (9*m)] >> 5;
				}

				// Redundantly called, but it works...
				currentPoly.faceNormal[0] = uToF(chunkOffset[30]);
				currentPoly.faceNormal[1] = uToF(chunkOffset[31]);
				currentPoly.faceNormal[2] = uToF(chunkOffset[32]);
				currentPoly.faceNormal[3] = 0.0f;

				chunkLength = 33;
				break;


			// 24 word chunk, 3 vertices, per-vertex UVs
			case 0x04:  // 0000 0100
			case 0x0e:  // 0000 1110
			case 0x24:  // 0010 0100
			case 0x2e:  // 0010 1110
				for (int m = 0; m < 3; m++)
				{
					currentPoly.vert[m].worldCoords[0] = uToF(chunkOffset[3 + (6*m)]);
					currentPoly.vert[m].worldCoords[1] = uToF(chunkOffset[4 + (6*m)]);
					currentPoly.vert[m].worldCoords[2] = uToF(chunkOffset[5 + (6*m)]);
					currentPoly.vert[m].worldCoords[3] = 1.0f;
					currentPoly.n = 3;

					// chunkOffset[6 + (6*m)] is almost always 0080, but it's 0070 for the translucent globe in fatfurwa player select
					currentPoly.vert[m].texCoords[0] = uToF(chunkOffset[7 + (6*m)]);
					currentPoly.vert[m].texCoords[1] = uToF(chunkOffset[8 + (6*m)]);
					currentPoly.vert[m].texCoords[2] = 0.0f;
					currentPoly.vert[m].texCoords[3] = 1.0f;

					if (currentPoly.flatShade)
						currentPoly.vert[m].colorIndex = chunkOffset[7 + (6*m)] >> 5;

					currentPoly.vert[m].normal[0] = uToF(chunkOffset[21]);
					currentPoly.vert[m].normal[1] = uToF(chunkOffset[22]);
					currentPoly.vert[m].normal[2] = uToF(chunkOffset[23]);
					currentPoly.vert[m].normal[3] = 0.0f;
				}

				// Redundantly called, but it works...
				currentPoly.faceNormal[0] = currentPoly.vert[2].normal[0];
				currentPoly.faceNormal[1] = currentPoly.vert[2].normal[1];
				currentPoly.faceNormal[2] = currentPoly.vert[2].normal[2];
				currentPoly.faceNormal[3] = 0.0f;

				chunkLength = 24;
				break;


			// 15 word chunk, 1 vertex, per-vertex UVs & normals, face normal
			case 0x87:  // 1000 0111
			case 0x97:  // 1001 0111
			case 0xd7:  // 1101 0111
			case 0xc7:  // 1100 0111
				// Copy over the proper vertices from the previous triangle...
				memcpy(&currentPoly.vert[1], &lastPoly.vert[0], sizeof(polyVert));
				memcpy(&currentPoly.vert[2], &lastPoly.vert[2], sizeof(polyVert));

				// Fill in the appropriate data...
				currentPoly.vert[0].worldCoords[0] = uToF(chunkOffset[3]);
				currentPoly.vert[0].worldCoords[1] = uToF(chunkOffset[4]);
				currentPoly.vert[0].worldCoords[2] = uToF(chunkOffset[5]);
				currentPoly.vert[0].worldCoords[3] = 1.0f;
				currentPoly.n = 3;

				// chunkOffset[6] is almost always 0080, but it's 0070 for the translucent globe in fatfurwa player select
				currentPoly.vert[0].texCoords[0] = uToF(chunkOffset[7]);
				currentPoly.vert[0].texCoords[1] = uToF(chunkOffset[8]);
				currentPoly.vert[0].texCoords[2] = 0.0f;
				currentPoly.vert[0].texCoords[3] = 1.0f;

				if (currentPoly.flatShade)
					currentPoly.vert[0].colorIndex = chunkOffset[7] >> 5;

				currentPoly.vert[0].normal[0] = uToF(chunkOffset[9]);
				currentPoly.vert[0].normal[1] = uToF(chunkOffset[10]);
				currentPoly.vert[0].normal[2] = uToF(chunkOffset[11]);
				currentPoly.vert[0].normal[3] = 0.0f;

				currentPoly.faceNormal[0] = uToF(chunkOffset[12]);
				currentPoly.faceNormal[1] = uToF(chunkOffset[13]);
				currentPoly.faceNormal[2] = uToF(chunkOffset[14]);
				currentPoly.faceNormal[3] = 0.0f;

				chunkLength = 15;
				break;


			// 12 word chunk, 1 vertex, per-vertex UVs
			case 0x86:  // 1000 0110
			case 0x96:  // 1001 0110
			case 0xb6:  // 1011 0110
			case 0xc6:  // 1100 0110
			case 0xd6:  // 1101 0110
				// Copy over the proper vertices from the previous triangle...
				memcpy(&currentPoly.vert[1], &lastPoly.vert[0], sizeof(polyVert));
				memcpy(&currentPoly.vert[2], &lastPoly.vert[2], sizeof(polyVert));

				currentPoly.vert[0].worldCoords[0] = uToF(chunkOffset[3]);
				currentPoly.vert[0].worldCoords[1] = uToF(chunkOffset[4]);
				currentPoly.vert[0].worldCoords[2] = uToF(chunkOffset[5]);
				currentPoly.vert[0].worldCoords[3] = 1.0f;
				currentPoly.n = 3;

				// chunkOffset[6] is almost always 0080, but it's 0070 for the translucent globe in fatfurwa player select
				currentPoly.vert[0].texCoords[0] = uToF(chunkOffset[7]);
				currentPoly.vert[0].texCoords[1] = uToF(chunkOffset[8]);
				currentPoly.vert[0].texCoords[2] = 0.0f;
				currentPoly.vert[0].texCoords[3] = 1.0f;

				if (currentPoly.flatShade)
					currentPoly.vert[0].colorIndex = chunkOffset[7] >> 5;

				// This normal could be right, but I'm not entirely sure - there is no normal in the 18 bytes!
				currentPoly.vert[0].normal[0] = lastPoly.faceNormal[0];
				currentPoly.vert[0].normal[1] = lastPoly.faceNormal[1];
				currentPoly.vert[0].normal[2] = lastPoly.faceNormal[2];
				currentPoly.vert[0].normal[3] = lastPoly.faceNormal[3];

				currentPoly.faceNormal[0] = lastPoly.faceNormal[0];
				currentPoly.faceNormal[1] = lastPoly.faceNormal[1];
				currentPoly.faceNormal[2] = lastPoly.faceNormal[2];
				currentPoly.faceNormal[3] = lastPoly.faceNormal[3];

				// TODO: I'm not reading 3 necessary words here (maybe face normal)

#if 0
				// DEBUG
				printf("0x?6 : %08x (%d/%d)\n", address[k]*3*2, l, size[k]-1);
				for (int m = 0; m < 13; m++)
					printf("%04x ", chunkOffset[m]);
				printf("\n");

				for (int m = 0; m < 13; m++)
					printf("%3.4f ", uToF(chunkOffset[m]));
				printf("\n\n");
#endif

				chunkLength = 12;
				break;

			default:
				printf("UNKNOWN geometry CHUNK TYPE : %02x\n", chunkType);
				chunkLength = 0;
				break;
			}

			currentPoly.visible = true;

			// Backup the last polygon (for triangle fans [strips?])
			memcpy(&lastPoly, &currentPoly, sizeof(polygon));


			////////////////////////////////////
			// Project and clip               //
			////////////////////////////////////
			// Perform the world transformations...
			// TODO: We can eliminate this step with a matrix stack (maybe necessary?)
			// Note: fatfurwa's helicopter tracking in scene 3 of its intro shows one of these matrices isn't quite correct
			setIdentity(m_modelViewMatrix);
			if (!m_samsho64_3d_hack)
			{
				// The sams64 games transform the geometry in front of a stationary camera.
				// This is fine in sams64_2, since it never calls the 'camera transformation' function
				// (thus using the identity matrix for this transform), but sams64 calls the
				// camera transformation function with rotation values.
				// It remains to be seen what those might do...
				matmul4(m_modelViewMatrix, m_modelViewMatrix, m_cameraMatrix);
			}
			matmul4(m_modelViewMatrix, m_modelViewMatrix, objectMatrix);

			// LIGHTING
			if (packet[1] & 0x0008 && m_lightStrength > 0.0f)
			{
				for (int v = 0; v < 3; v++)
				{
					float transformedNormal[4];
					vecmatmul4(transformedNormal, objectMatrix, currentPoly.vert[v].normal);
					normalize(transformedNormal);
					normalize(m_lightVector);

					float intensity = vecDotProduct(transformedNormal, m_lightVector) * -1.0f;
					intensity = (intensity <= 0.0f) ? (0.0f) : (intensity);
					intensity *= m_lightStrength * 128.0f;    // Turns 0x0100 into 1.0
					intensity *= 128.0;                     // Maps intensity to the range [0.0, 2.0]
					if (intensity >= 255.0f) intensity = 255.0f;

					currentPoly.vert[v].light[0] = intensity;
					currentPoly.vert[v].light[1] = intensity;
					currentPoly.vert[v].light[2] = intensity;
				}
			}
			else
			{
				// Just clear out the light values
				for (int v = 0; v < 3; v++)
				{
					currentPoly.vert[v].light[0] = 0;
					currentPoly.vert[v].light[1] = 0;
					currentPoly.vert[v].light[2] = 0;
				}
			}

			// BACKFACE CULL
			// roadedge has various one-way barriers that you can drive through, but need to be invisible from behind, so needs this culling
			float cullRay[4];
			float cullNorm[4];

			// Cast a ray out of the camera towards the polygon's point in eyespace.
			vecmatmul4(cullRay, m_modelViewMatrix, currentPoly.vert[0].worldCoords);
			normalize(cullRay);

			// Dot product that with the normal to see if you're negative...
			vecmatmul4(cullNorm, m_modelViewMatrix, currentPoly.faceNormal);

			const float backfaceCullResult = vecDotProduct(cullRay, cullNorm);
			if (backfaceCullResult < 0.0f)
				currentPoly.visible = true;
			else
				currentPoly.visible = false;

			// BEHIND-THE-CAMERA CULL //
			vecmatmul4(cullRay, m_modelViewMatrix, currentPoly.vert[0].worldCoords);
			if (cullRay[2] > 0.0f)              // Camera is pointing down -Z
			{
				currentPoly.visible = false;
			}


			// TRANSFORM THE TRIANGLE INTO HOMOGENEOUS SCREEN SPACE //
			if (currentPoly.visible)
			{
				hng64_clip_vertex clipVerts[10];

				// Transform and project each vertex into pre-divided homogeneous coordinates
				for (int m = 0; m < currentPoly.n; m++)
				{
					float eyeCoords[4];     // World coordinates transformed by the modelViewMatrix
					vecmatmul4(eyeCoords, m_modelViewMatrix, currentPoly.vert[m].worldCoords);
					vecmatmul4(currentPoly.vert[m].clipCoords, m_projectionMatrix, eyeCoords);

					clipVerts[m].x = currentPoly.vert[m].clipCoords[0];
					clipVerts[m].y = currentPoly.vert[m].clipCoords[1];
					clipVerts[m].z = currentPoly.vert[m].clipCoords[2];
					clipVerts[m].w = currentPoly.vert[m].clipCoords[3];
					clipVerts[m].p[0] = currentPoly.vert[m].texCoords[0];
					clipVerts[m].p[1] = currentPoly.vert[m].texCoords[1];
					clipVerts[m].p[2] = currentPoly.vert[m].light[0];
					clipVerts[m].p[3] = currentPoly.vert[m].light[1];
					clipVerts[m].p[4] = currentPoly.vert[m].light[2];
				}

				if (currentPoly.visible)
				{
					// Clip against all edges of the view frustum
					int num_vertices = frustum_clip_all<float, 5>(clipVerts, currentPoly.n, clipVerts);

					// Copy the results of
					currentPoly.n = num_vertices;
					for (int m = 0; m < num_vertices; m++)
					{
						currentPoly.vert[m].clipCoords[0] = clipVerts[m].x;
						currentPoly.vert[m].clipCoords[1] = clipVerts[m].y;
						currentPoly.vert[m].clipCoords[2] = clipVerts[m].z;
						currentPoly.vert[m].clipCoords[3] = clipVerts[m].w;
						currentPoly.vert[m].texCoords[0] = clipVerts[m].p[0];
						currentPoly.vert[m].texCoords[1] = clipVerts[m].p[1];
						currentPoly.vert[m].light[0] = clipVerts[m].p[2];
						currentPoly.vert[m].light[1] = clipVerts[m].p[3];
						currentPoly.vert[m].light[2] = clipVerts[m].p[4];
					}

					const rectangle& visarea = m_screen->visible_area();
					for (int m = 0; m < currentPoly.n; m++)
					{
						// Convert into normalized device coordinates...
						float ndCoords[4];      // Normalized device coordinates/clipCoordinates (x/w, y/w, z/w)
						ndCoords[0] = currentPoly.vert[m].clipCoords[0] / currentPoly.vert[m].clipCoords[3];
						ndCoords[1] = currentPoly.vert[m].clipCoords[1] / currentPoly.vert[m].clipCoords[3];
						ndCoords[2] = currentPoly.vert[m].clipCoords[2] / currentPoly.vert[m].clipCoords[3];
						ndCoords[3] = currentPoly.vert[m].clipCoords[3];

						// Final pixel values are garnered here :
						float windowCoords[4];  // Mapped ndCoordinates to screen space
						windowCoords[0] = (ndCoords[0]+1.0f) * ((float)(visarea.max_x) / 2.0f) + 0.0f;
						windowCoords[1] = (ndCoords[1]+1.0f) * ((float)(visarea.max_y) / 2.0f) + 0.0f;
						windowCoords[2] = (ndCoords[2]+1.0f) * 0.5f;

						// Flip Y
						windowCoords[1] = (float)visarea.max_y - windowCoords[1];

						// Store the points in a list for later use...
						currentPoly.vert[m].clipCoords[0] = windowCoords[0];
						currentPoly.vert[m].clipCoords[1] = windowCoords[1];
						currentPoly.vert[m].clipCoords[2] = windowCoords[2];
						currentPoly.vert[m].clipCoords[3] = ndCoords[3];
					}
				}
			}

			// Advance to the next polygon chunk...
			chunkOffset += chunkLength;

			numPolys++;
		}
	}
}

// note 0x0102 packets are only 8 words, it appears they can be in either the upper or lower half of the 16 word packet.
// We currently only draw 0x0102 packets where both halves contain 0x0102 (2 calls), but this causes graphics to vanish in
// xrally because in some cases the 0x0102 packet only exists in the upper or lower half with another value (often 0x0000 - NOP) in the other.
// If we also treat (0x0000 - NOP) as 8 word  instead of 16 so that we can access a 0x0102 in the 2nd half of the 16 word packet
// then we end up with other invalid packets in the 2nd half which should be ignored.
// This would suggest our processing if flawed in other ways, or there is something else to indicate packet length.

void hng64_state::hng64_command3d(const uint16_t* packet)
{
	int numPolys = 0;

	//printf("packet type : %04x %04x|%04x %04x|%04x %04x|%04x %04x  | %04x %04x %04x %04x %04x %04x %04x %04x\n", packet[0],packet[1],packet[2],packet[3],packet[4],packet[5],packet[6],packet[7],     packet[8], packet[9], packet[10], packet[11], packet[12], packet[13], packet[14], packet[15]);

	switch (packet[0])
	{
	case 0x0000:    // NOP?
		 /* Appears to be a NOP (or 'end of list for this frame, ignore everything after' doesn't stop stray 3d objects in game for xrally/roadedge
		    although does stop a partial hng64 logo being displayed assuming that's meant to be kept onscreen by some other means without valid data) */
		break;

	case 0x0001:    // Camera transformation.
		setCameraTransformation(packet);
		break;

	case 0x0010:    // Lighting information.
		setLighting(packet);
		break;

	case 0x0011:    // Palette / Model flags?
		set3dFlags(packet);
		break;

	case 0x0012:    // Projection Matrix
		setCameraProjectionMatrix(packet);
		break;

	case 0x0100:
	case 0x0101:    // Geometry with full transformations
		recoverPolygonBlock(packet, numPolys);
		break;

	case 0x0102:    // Geometry with only translation
		// Split the packet and call recoverPolygonBlock on each half.
		uint16_t miniPacket[16];
		memset(miniPacket, 0, sizeof(uint16_t)*16);
		for (int i = 0; i < 7; i++) miniPacket[i] = packet[i];
		miniPacket[7] = 0x7fff;
		miniPacket[11] = 0x7fff;
		miniPacket[15] = 0x7fff;
		recoverPolygonBlock(miniPacket, numPolys);


		if (packet[8] == 0x0102)
		{
			memset(miniPacket, 0, sizeof(uint16_t) * 16);
			for (int i = 0; i < 7; i++) miniPacket[i] = packet[i + 8];
			miniPacket[7] = 0x7fff;
			miniPacket[11] = 0x7fff;
			miniPacket[15] = 0x7fff;
			recoverPolygonBlock(miniPacket, numPolys);
		}
		else
		{
			/* if the 2nd value isn't 0x0102 don't render it
			   it could just be that the display list is corrupt at this point tho, see note above
			*/
		}

		break;

	case 0x1000:    // Unknown: Some sort of global flags?
		//printPacket(packet, 1); printf("\n");
		break;

	case 0x1001:    // Unknown: Some sort of global flags?  Almost always comes in a group of 4 with an index [0,3].
		//printPacket(packet, 1);
		break;

	default:
		printf("HNG64: Unknown 3d command %04x.\n", packet[0]);
		break;
	}

	// If there are polygons, rasterize them into the display buffer
	for (int i = 0; i < numPolys; i++)
	{
		if (m_polys[i].visible)
		{
			m_poly_renderer->drawShaded(&m_polys[i]);
		}
	}
	m_poly_renderer->wait();
}

void hng64_state::clear3d()
{
	// Reset the buffers...
	const rectangle& visarea = m_screen->visible_area();
	for (int i = 0; i < (visarea.max_x)*(visarea.max_y); i++)
	{
		m_poly_renderer->depthBuffer3d()[i] = 100.0f;
	}

	// Clear the 3d rasterizer buffer
	m_poly_renderer->colorBuffer3d().fill(0x00000000, m_screen->visible_area());

	// Set some matrices to the identity...
	setIdentity(m_projectionMatrix);
	setIdentity(m_modelViewMatrix);
	setIdentity(m_cameraMatrix);
}

/* 3D/framebuffer video registers
 * ------------------------------
 *
 *        | Bits      | Use
 *        |           |
 * -------+ 7654-3210-+----------------
 *      0 | ---- --xy | x = Reads in Fatal Fury WA, if on then there isn't a 3d refresh (busy flag?).  y = set at POST/service modes, almost likely fb disable
 *      1 | ---- ---- |
 *      2 | ccc- b--- | c = framebuffer color base, 0x311800 in Fatal Fury WA, 0x313800 in Buriki One (or not?)  b = don't clear buffer
 *      3 | ---- ---- |
*/

uint8_t hng64_state::hng64_fbcontrol_r(offs_t offset)
{
	logerror("%s: hng64_fbcontrol_r (%03x)\n", machine().describe_context(), offset);
	return m_fbcontrol[offset];
}

void hng64_state::hng64_fbcontrol_w(offs_t offset, uint8_t data)
{

	/* roadedge does the following to turn off the framebuffer clear (leave trails) and then turn it back on when selecting a car
	   ':maincpu' (8001EDE0): hng64_fbcontrol_w (002) 10 (disable frame buffer clear)
	   ':maincpu' (8001FE4C): hng64_fbcontrol_w (002) 38 (normal)

	   during the Hyper Neogeo 64 logo it has a value of
	   ':maincpu' (8005AA44): hng64_fbcontrol_w (002) 18

	   sams64 does
	   ':maincpu' (800C13C4): hng64_fbcontrol_r (002)     (ANDs with 0x07, ORs with 0x18)
	   ':maincpu' (800C13D0): hng64_fbcontrol_w (002) 18

	   other games use either mix of 0x18 and 0x38.  bit 0x08 must prevent the framebuffer clear tho
	   according to above table bit 0x20 is color base, but implementation for it is a hack

	   (3d car currently not visible on roadedge select screen due to priority issue, disable sprites to see it)

	*/

	logerror("%s: hng64_fbcontrol_w (%03x) %02x\n", machine().describe_context(), offset, data);
	m_fbcontrol[offset] = data;
}

void hng64_state::hng64_fbunkpair_w(offs_t offset, uint16_t data)
{
	// set to fixed values?

	logerror("%s: hng64_fbunkpair_w (%03x) %04x\n", machine().describe_context(), offset, data);
}

void hng64_state::hng64_fbscroll_w(offs_t offset, uint16_t data)
{
	// this is used ingame on the samsho games, and on the car select screen in xrally (youtube video confirms position of car needs to change)

	logerror("%s: hng64_fbscroll_w (%03x) %04x\n", machine().describe_context(), offset, data);
}

void hng64_state::hng64_fbunkbyte_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		// | ---- --?x | unknown, unsetted by Buriki One and set by Fatal Fury WA, buffering mode?
		logerror("%s: hng64_unkbyte_w (%03x) %02x\n", machine().describe_context(), offset, data);
	}
	else
	{
		logerror("%s: hng64_unkbyte_w (%03x - unexpected) %02x \n", machine().describe_context(), offset, data);
	}
}

// this is a table filled with 0x0? data, seems to be 16-bit values
uint32_t hng64_state::hng64_fbtable_r(offs_t offset, uint32_t mem_mask)
{
	logerror("%s: hng64_fbtable_r (%03x) (%08x)\n", machine().describe_context(), offset * 4, mem_mask);
	return m_fbtable[offset];
}

void hng64_state::hng64_fbtable_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("%s: hng64_fbtable_w (%03x) %08x (%08x)\n", machine().describe_context(), offset * 4, data, mem_mask);
	COMBINE_DATA(&m_fbtable[offset]);
}

/////////////////////
// 3D UTILITY CODE //
/////////////////////

// 4x4 matrix multiplication
void hng64_state::matmul4(float *product, const float *a, const float *b)
{
	for (int i = 0; i < 4; i++)
	{
		const float ai0 = a[0  + i];
		const float ai1 = a[4  + i];
		const float ai2 = a[8  + i];
		const float ai3 = a[12 + i];

		product[0  + i] = ai0 * b[0 ] + ai1 * b[1 ] + ai2 * b[2 ] + ai3 * b[3 ];
		product[4  + i] = ai0 * b[4 ] + ai1 * b[5 ] + ai2 * b[6 ] + ai3 * b[7 ];
		product[8  + i] = ai0 * b[8 ] + ai1 * b[9 ] + ai2 * b[10] + ai3 * b[11];
		product[12 + i] = ai0 * b[12] + ai1 * b[13] + ai2 * b[14] + ai3 * b[15];
	}
}

// vector by 4x4 matrix multiply
void hng64_state::vecmatmul4(float *product, const float *a, const float *b)
{
	const float& bi0 = b[0];
	const float& bi1 = b[1];
	const float& bi2 = b[2];
	const float& bi3 = b[3];

	product[0] = bi0 * a[0] + bi1 * a[4] + bi2 * a[8 ] + bi3 * a[12];
	product[1] = bi0 * a[1] + bi1 * a[5] + bi2 * a[9 ] + bi3 * a[13];
	product[2] = bi0 * a[2] + bi1 * a[6] + bi2 * a[10] + bi3 * a[14];
	product[3] = bi0 * a[3] + bi1 * a[7] + bi2 * a[11] + bi3 * a[15];
}

float hng64_state::vecDotProduct(const float *a, const float *b)
{
	return ((a[0]*b[0]) + (a[1]*b[1]) + (a[2]*b[2]));
}

void hng64_state::setIdentity(float *matrix)
{
	for (int i = 0; i < 16; i++)
	{
		matrix[i] = 0.0f;
	}

	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}

float hng64_state::uToF(uint16_t input)
{
	float retVal;
	retVal = (float)((int16_t)input) / 32768.0f;
	return retVal;

#if 0
	if ((int16_t)input < 0)
		retVal = (float)((int16_t)input) / 32768.0f;
	else
		retVal = (float)((int16_t)input) / 32767.0f;
#endif
}

void hng64_state::normalize(float* x)
{
	double l2 = (x[0]*x[0]) + (x[1]*x[1]) + (x[2]*x[2]);
	double l = sqrt(l2);

	x[0] = (float)(x[0] / l);
	x[1] = (float)(x[1] / l);
	x[2] = (float)(x[2] / l);
}


////////////////////////////////
// POLYGON RASTERIZATION CODE //
////////////////////////////////

void hng64_poly_renderer::render_texture_scanline(int32_t scanline, const extent_t& extent, const hng64_poly_data& renderData, int threadid)
{
	// Pull the parameters out of the extent structure
	float z = extent.param[0].start;
	float w = extent.param[1].start;
	float lightR = extent.param[2].start;
	float lightG = extent.param[3].start;
	float lightB = extent.param[4].start;
	float s = extent.param[5].start;
	float t = extent.param[6].start;

	const float dz = extent.param[0].dpdx;
	const float dw = extent.param[1].dpdx;
	const float dlightR = extent.param[2].dpdx;
	const float dlightG = extent.param[3].dpdx;
	const float dlightB = extent.param[4].dpdx;
	const float ds = extent.param[5].dpdx;
	const float dt = extent.param[6].dpdx;

	// Pointers to the pixel buffers
	uint32_t* colorBuffer = &m_colorBuffer3d.pix(scanline, extent.startx);
	float*  depthBuffer = &m_depthBuffer3d[(scanline * m_state.m_screen->visible_area().width()) + extent.startx];

	const uint8_t *textureOffset = &m_state.m_texturerom[renderData.texIndex * 1024 * 1024];

	// Step over each pixel in the horizontal span
	for(int x = extent.startx; x < extent.stopx; x++)
	{
		if (z < *depthBuffer)
		{
			// Multiply back through by w for everything that was interpolated perspective-correctly
			const float sCorrect = s / w;
			const float tCorrect = t / w;
			const float rCorrect = lightR / w;
			const float gCorrect = lightG / w;
			const float bCorrect = lightB / w;

			if ((renderData.debugColor & 0xff000000) == 0x01000000)
			{
				// ST color mode
				*colorBuffer = rgb_t(255, uint8_t(sCorrect*255.0f), uint8_t(tCorrect*255.0f), uint8_t(0));
			}
			else if ((renderData.debugColor & 0xff000000) == 0x02000000)
			{
				// Lighting only
				*colorBuffer = rgb_t(255, (uint8_t)rCorrect, (uint8_t)gCorrect, (uint8_t)bCorrect);
			}
			else if ((renderData.debugColor & 0xff000000) == 0xff000000)
			{
				// Debug color mode
				*colorBuffer = renderData.debugColor;
			}
			else
			{
				float textureS = 0.0f;
				float textureT = 0.0f;

				// Standard & Half-Res textures
				if (renderData.texType == 0x0)
				{
					textureS = sCorrect * 1024.0f;
					textureT = tCorrect * 1024.0f;
				}
				else if (renderData.texType == 0x1)
				{
					textureS = sCorrect * 512.0f;
					textureT = tCorrect * 512.0f;
				}

				// Small-Page textures
				if (renderData.texPageSmall == 2)
				{
					textureT = fmod(textureT, 256.0f);
					textureS = fmod(textureS, 256.0f);

					textureT += (256.0f * (renderData.texPageHorizOffset>>1));
					textureS += (256.0f * (renderData.texPageVertOffset>>1));
				}
				else if (renderData.texPageSmall == 3)
				{
					textureT = fmod(textureT, 128.0f);
					textureS = fmod(textureS, 128.0f);

					textureT += (128.0f * (renderData.texPageHorizOffset>>0));
					textureS += (128.0f * (renderData.texPageVertOffset>>0));
				}

				uint8_t paletteEntry = textureOffset[((int)textureS)*1024 + (int)textureT];

				// Naive Alpha Implementation (?) - don't draw if you're at texture index 0...
				if (paletteEntry != 0)
				{
					// The color out of the texture
					paletteEntry %= renderData.palPageSize;
					rgb_t color = m_state.m_palette->pen(renderData.palOffset + paletteEntry);

					// Apply the lighting
					float rIntensity = rCorrect / 255.0f;
					float gIntensity = gCorrect / 255.0f;
					float bIntensity = bCorrect / 255.0f;
					float red   = color.r() * rIntensity;
					float green = color.g() * gIntensity;
					float blue  = color.b() * bIntensity;

					// Clamp and finalize
					red = color.r() + red;
					green = color.g() + green;
					blue = color.b() + blue;

					if (red >= 255) red = 255;
					if (green >= 255) green = 255;
					if (blue >= 255) blue = 255;

					color = rgb_t(255, (uint8_t)red, (uint8_t)green, (uint8_t)blue);

					*colorBuffer = color;
					*depthBuffer = z;
				}
			}
		}

		z += dz;
		w += dw;
		lightR += dlightR;
		lightG += dlightG;
		lightB += dlightB;
		s += ds;
		t += dt;

		colorBuffer++;
		depthBuffer++;
	}
}

void hng64_poly_renderer::render_flat_scanline(int32_t scanline, const extent_t& extent, const hng64_poly_data& renderData, int threadid)
{
	// Pull the parameters out of the extent structure
	float z = extent.param[0].start;
	float r = extent.param[1].start;
	float g = extent.param[2].start;
	float b = extent.param[3].start;

	const float dz = extent.param[0].dpdx;
	const float dr = extent.param[1].dpdx;
	const float dg = extent.param[2].dpdx;
	const float db = extent.param[3].dpdx;

	// Pointers to the pixel buffers
	uint32_t* colorBuffer = &m_colorBuffer3d.pix(scanline, extent.startx);
	float*  depthBuffer = &m_depthBuffer3d[(scanline * m_state.m_screen->visible_area().width()) + extent.startx];

	// Step over each pixel in the horizontal span
	for(int x = extent.startx; x < extent.stopx; x++)
	{
		if (z < *depthBuffer)
		{

			// Clamp and finalize
			if (r >= 255) r = 255;
			if (g >= 255) g = 255;
			if (b >= 255) b = 255;

			rgb_t color = rgb_t(255, (uint8_t)r, (uint8_t)g, (uint8_t)b);

			*colorBuffer = color;
			*depthBuffer = z;
		}

		z += dz;
		r += dr;
		g += dg;
		b += db;

		colorBuffer++;
		depthBuffer++;
	}
}

void hng64_poly_renderer::drawShaded(polygon *p)
{
	// Polygon information for the rasterizer
	hng64_poly_data rOptions;
	rOptions.texType = p->texType;
	rOptions.texIndex = p->texIndex;
	rOptions.palOffset = p->palOffset;
	rOptions.palPageSize = p->palPageSize;
	rOptions.debugColor = p->debugColor;
	rOptions.texPageSmall = p->texPageSmall;
	rOptions.texPageHorizOffset = p->texPageHorizOffset;
	rOptions.texPageVertOffset = p->texPageVertOffset;

	// Pass the render data into the rasterizer
	hng64_poly_data& renderData = object_data().next();
	renderData = rOptions;

	const rectangle& visibleArea = m_state.m_screen->visible_area();

	if (p->flatShade)
	{
		// Rasterize the triangles
		for (int j = 1; j < p->n-1; j++)
		{
			// Build some MAME rasterizer vertices from the hng64 vertices
			vertex_t pVert[3];
			rgb_t color;

			const polyVert& pv0 = p->vert[0];
			color = m_state.m_palette->pen(renderData.palOffset + pv0.colorIndex);
			pVert[0].x = pv0.clipCoords[0];
			pVert[0].y = pv0.clipCoords[1];
			pVert[0].p[0] = pv0.clipCoords[2];
			pVert[0].p[1] = color.r();
			pVert[0].p[2] = color.g();
			pVert[0].p[3] = color.b();

			const polyVert& pvj = p->vert[j];
			color = m_state.m_palette->pen(renderData.palOffset + pvj.colorIndex);
			pVert[1].x = pvj.clipCoords[0];
			pVert[1].y = pvj.clipCoords[1];
			pVert[1].p[0] = pvj.clipCoords[2];
			pVert[1].p[1] = color.r();
			pVert[1].p[2] = color.g();
			pVert[1].p[3] = color.b();

			const polyVert& pvjp1 = p->vert[j+1];
			color = m_state.m_palette->pen(renderData.palOffset + pvjp1.colorIndex);
			pVert[2].x = pvjp1.clipCoords[0];
			pVert[2].y = pvjp1.clipCoords[1];
			pVert[2].p[0] = pvjp1.clipCoords[2];
			pVert[2].p[1] = color.r();
			pVert[2].p[2] = color.g();
			pVert[2].p[3] = color.b();

			render_triangle<4>(visibleArea, render_delegate(&hng64_poly_renderer::render_flat_scanline, this), pVert[0], pVert[1], pVert[2]);
		}
	}
	else
	{
		// The perspective-correct texture divide...
		// Note: There is a very good chance the HNG64 hardware does not do perspective-correct texture-mapping - explore
		for (int j = 0; j < p->n; j++)
		{
			p->vert[j].clipCoords[3] = 1.0f / p->vert[j].clipCoords[3];
			p->vert[j].light[0]      = p->vert[j].light[0]     * p->vert[j].clipCoords[3];
			p->vert[j].light[1]      = p->vert[j].light[1]     * p->vert[j].clipCoords[3];
			p->vert[j].light[2]      = p->vert[j].light[2]     * p->vert[j].clipCoords[3];
			p->vert[j].texCoords[0]  = p->vert[j].texCoords[0] * p->vert[j].clipCoords[3];
			p->vert[j].texCoords[1]  = p->vert[j].texCoords[1] * p->vert[j].clipCoords[3];
		}

		// Rasterize the triangles
		for (int j = 1; j < p->n-1; j++)
		{
			// Build some MAME rasterizer vertices from the hng64 vertices
			vertex_t pVert[3];

			const polyVert& pv0 = p->vert[0];
			pVert[0].x = pv0.clipCoords[0];
			pVert[0].y = pv0.clipCoords[1];
			pVert[0].p[0] = pv0.clipCoords[2];
			pVert[0].p[1] = pv0.clipCoords[3];
			pVert[0].p[2] = pv0.light[0];
			pVert[0].p[3] = pv0.light[1];
			pVert[0].p[4] = pv0.light[2];
			pVert[0].p[5] = pv0.texCoords[0];
			pVert[0].p[6] = pv0.texCoords[1];

			const polyVert& pvj = p->vert[j];
			pVert[1].x = pvj.clipCoords[0];
			pVert[1].y = pvj.clipCoords[1];
			pVert[1].p[0] = pvj.clipCoords[2];
			pVert[1].p[1] = pvj.clipCoords[3];
			pVert[1].p[2] = pvj.light[0];
			pVert[1].p[3] = pvj.light[1];
			pVert[1].p[4] = pvj.light[2];
			pVert[1].p[5] = pvj.texCoords[0];
			pVert[1].p[6] = pvj.texCoords[1];

			const polyVert& pvjp1 = p->vert[j+1];
			pVert[2].x = pvjp1.clipCoords[0];
			pVert[2].y = pvjp1.clipCoords[1];
			pVert[2].p[0] = pvjp1.clipCoords[2];
			pVert[2].p[1] = pvjp1.clipCoords[3];
			pVert[2].p[2] = pvjp1.light[0];
			pVert[2].p[3] = pvjp1.light[1];
			pVert[2].p[4] = pvjp1.light[2];
			pVert[2].p[5] = pvjp1.texCoords[0];
			pVert[2].p[6] = pvjp1.texCoords[1];

			render_triangle<7>(visibleArea, render_delegate(&hng64_poly_renderer::render_texture_scanline, this), pVert[0], pVert[1], pVert[2]);
		}
	}
}
