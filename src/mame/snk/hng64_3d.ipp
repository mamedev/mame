// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner, Andrew Zaferakis

/////////////////////////////////
/// Hyper NeoGeo 64 - 3D bits ///
/////////////////////////////////

// Polygon rasterizer interface
hng64_poly_renderer::hng64_poly_renderer(hng64_state& state)
	: poly_manager<float, hng64_poly_data, 7>(state.machine())
	, m_state(state)
{
	const s32 bufferSize = 512 * 512;
	m_depthBuffer3d = std::make_unique<float[]>(bufferSize);
	m_colorBuffer3d = std::make_unique<u16[]>(bufferSize);

}



/* Hardware calls these '3d buffers'

    They're only read during the startup check, never written

    They're definitely mirrored in the startup test, according to ElSemi

    The games run in interlace mode, so buffer resolution can be half the effective screen height

    30100000-3011ffff is framebuffer A0 (512x256 8-bit?) (pal data?)
    30120000-3013ffff is framebuffer A1 (512x256 8-bit?) (pal data?)
    30140000-3015ffff is ZBuffer A  (512x256 8-bit?)
*/

u32 hng64_state::fbram1_r(offs_t offset)
{
	return m_fbram1[offset];
}

void hng64_state::fbram1_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_fbram1[offset]);
}

u32 hng64_state::fbram2_r(offs_t offset)
{
	return m_fbram2[offset];
}

void hng64_state::fbram2_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_fbram2[offset]);
}

// The 3d 'display list'
void hng64_state::dl_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dl[offset]);
}

void hng64_state::dl_unk_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGDISPLAYLIST("%s: dl_unk_w %08x (%08x)\n", machine().describe_context(), data, mem_mask);
}

void hng64_state::dl_upload_w(u32 data)
{
	//m_paletteState3d = 0; // no, breaks fatfurwa characters

	// Data is:
	// 00000b50 for the sams64 games
	// 00000f00 for everything else
	// TODO: different param for the two sams64 games, less FIFO to process?

	// This is written after the game uploads 16 packets, each 16 words long
	// We're assuming it to be a 'send to 3d hardware' trigger.
	// This can be called multiple times per frame (at least 2, as long as it gets the expected interrupt / status flags)
	auto profile = g_profiler.start(PROFILER_USER1);
	for (int packetStart = 0; packetStart < 0x100; packetStart += 16)
	{
		// Send it off to the 3d subsystem.
		if (!command3d(&m_dl[packetStart]))
			break;
	}

	// Schedule a small amount of time to let the 3d hardware rasterize the display buffer
	m_3dfifo_timer->adjust(m_maincpu->cycles_to_attotime(0x200 * 8));
}

TIMER_CALLBACK_MEMBER(hng64_state::_3dfifo_processed)
{
	set_irq(0x0008);
}

void hng64_state::dl_control_w(u32 data)
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
	LOGDISPLAYLIST("dl_control_w %08x %08x\n", data, mem_mask);

	if (data & 2) // swap buffers
	{
	    clear3d();
	}
	*/
}

u32 hng64_state::dl_vreg_r()
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

void hng64_state::printPacket(const u16* packet, int hex)
{
	if (hex)
	{
		LOG3D("Packet : %04x %04x  2:%04x %04x  4:%04x %04x  6:%04x %04x  8:%04x %04x  10:%04x %04x  12:%04x %04x  14:%04x %04x\n",
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
		LOG3D("Packet : %04x %3.4f  2:%3.4f %3.4f  4:%3.4f %3.4f  6:%3.4f %3.4f  8:%3.4f %3.4f  10:%3.4f %3.4f  12:%3.4f %3.4f  14:%3.4f %3.4f\n",
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
void hng64_state::setCameraTransformation(const u16* packet)
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
	// following might be unused leftover data
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
void hng64_state::setLighting(const u16* packet)
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
	// following could just be leftover data
	// [10] - ???? ... ? Used in fatfurwa
	// [11] - ???? ... ? Used in fatfurwa
	// [12] - ???? ... ? Used in fatfurwa
	// [13] - ???? ... ? Used in fatfurwa
	// [14] - ???? ... ? Used in fatfurwa
	// [15] - ???? ... ? Used in fatfurwa
	////////////*/
	if (packet[1] != 0x0000) LOG3D("packet[1] in setLighting function is non-zero!\n");
	if (packet[2] != 0x0000) LOG3D("packet[2] in setLighting function is non-zero!\n");

	m_lightVector[0] = uToF(packet[3]);
	m_lightVector[1] = uToF(packet[4]);
	m_lightVector[2] = uToF(packet[5]);
	m_lightStrength = uToF(packet[9]);
}

// Operation 0011
// Palette / Model flags?
void hng64_state::set3dFlags(const u16* packet)
{
	/*//////////////
	// PACKET FORMAT
	// [0]  - 0011 ... ID
	// [1]  - ???? ... texture scrolling x (c000 - ffff)
	// [2]  - ???? ... texture scrolling y (c000 - ffff)
	// [3]  - ???? ...
	// [4]  - ???? ...
	// [5]  - ???? ... scale?
	// [6]  - ???? ... scale?
	// [7]  - ???? ... scale?
	// [8]  - xx?? ... Palette offset & ??
	// following could just be leftover data
	// [9]  - ???? ... ? Very much used - seem to bounce around when characters are on screen
	// [10] - ???? ... ? ''  ''
	// [11] - ???? ... ? ''  ''
	// [12] - ???? ... ? ''  ''
	// [13] - ???? ... ? ''  ''
	// [14] - ???? ... ? ''  ''
	// [15] - ???? ... ? ''  ''
	////////////*/
	m_texturescrollx = packet[1];
	m_texturescrolly = packet[2];
	m_paletteState3d = packet[8];

	m_modelscalex = packet[5];
	m_modelscaley = packet[6];
	m_modelscalez = packet[7];
}

// Operation 0012
// Projection Matrix.
void hng64_state::setCameraProjectionMatrix(const u16* packet)
{
	/*//////////////
	// PACKET FORMAT
	// [0]  - 0012 ... ID
	// [1]  - ???? ... ? Contains a value in buriki's 'how to play' - probably a projection window/offset.  value used is 0xffc0 ( -0x40 )  64 pixels?  not used anywhere else?
	// [2]  - ???? ... ? Contains a value in buriki's 'how to play' - probably a projection window/offset.  value used is 0x0018            24 pixels?  not used anywhere else?
	// [3]  - ???? ... ? Contains a value   (always? 0x0a00)
	// [4]  - xxxx ... Camera projection Z scale
	// [5]  - xxxx ... Camera projection near Z
	// [6]  - xxxx ... Camera projection screen Z
	// [7]  - xxxx ... Camera projection (?)  (always? 0x0b10)
	// [8]  - xxxx ... Camera projection (?)  (always? 0x0a00)
	// [9]  - xxxx ... Camera projection (?)  (always? 0x0b00)
	// [10] - xxxx ... Camera projection right  - confirmed by sams64_2
	// [11] - xxxx ... Camera projection left   - confirmed by sams64_2
	// [12] - xxxx ... Camera projection top    - confirmed by sams64_2
	// [13] - xxxx ... Camera projection bottom - confirmed by sams64_2
	// following could just be leftover data
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

#if 0
	if ((packet[1] != 0x0000) || (packet[2] != 0x0000))
		printf("camera packet[1] %04x packet[2] %04x\n", packet[1], packet[2]);

	if (packet[3] != 0x0a00)
		printf("camera packet[3] %04x\n", packet[3]);

	if (packet[7] != 0x0b10)
		printf("camera packet[7] %04x\n", packet[7]);

	if (packet[8] != 0x0a00)
		printf("camera packet[7] %04x\n", packet[8]);

	if (packet[9] != 0x0b00)
		printf("camera packet[9] %04x\n", packet[9]);
#endif
}

void hng64_state::recoverStandardVerts(polygon& currentPoly, int m, const u16* chunkOffset_verts, int& counter, const u16* packet)
{
	currentPoly.vert[m].worldCoords[0] = uToF(chunkOffset_verts[counter++]);
	currentPoly.vert[m].worldCoords[1] = uToF(chunkOffset_verts[counter++]);
	currentPoly.vert[m].worldCoords[2] = uToF(chunkOffset_verts[counter++]);
	currentPoly.vert[m].worldCoords[3] = 1.0f;
	currentPoly.n = 3;

	// this seems to be some kind of default lighting value / brightness, probably for unlit polys? used in various places, eg side of house in ice stage of ss64, some shadows
	[[maybe_unused]] u16 maybe_blend = chunkOffset_verts[counter++];

	currentPoly.vert[m].texCoords[0] = uToF(chunkOffset_verts[counter]);
	if (currentPoly.flatShade)
		currentPoly.colorIndex = chunkOffset_verts[counter] >> 5;
	counter++;
	currentPoly.vert[m].texCoords[1] = uToF(chunkOffset_verts[counter++]);

	// set on the Hyper 64 logos for roadedge and xrally which are known to be scaled
	// also set on the car select screen in roadedge, and the car on the stage name screen
	// not set anywhere else?
	//
	// params for car select screen / stage name screen are always 0x100, which would be
	// 'no scale' anyway, and no scaling is observed in hardware videos
	//
	// the m_modelscalex etc. do contain values in fatal fury intro, but not valid looking
	// ones, so probably unrelated to how that screen is scaled
	if (packet[1] & 0x0040)
	{
		currentPoly.vert[m].worldCoords[0] = (currentPoly.vert[m].worldCoords[0] * m_modelscalez) / 0x100;
		currentPoly.vert[m].worldCoords[1] = (currentPoly.vert[m].worldCoords[1] * m_modelscaley) / 0x100;
		currentPoly.vert[m].worldCoords[2] = (currentPoly.vert[m].worldCoords[2] * m_modelscalex) / 0x100;

	//  if ((m_modelscalex != 0x100) || (m_modelscaley != 0x100) || (m_modelscalez != 0x100))
	//      LOG3D("maybe using model scale %04x %04x %04x\n", m_modelscalex, m_modelscaley, m_modelscalez);
	}
}

// Operation 0100
// Polygon rasterization.
void hng64_state::recoverPolygonBlock(const u16* packet, int& numPolys)
{
	//printPacket(packet, 1);

	/*//////////////
	// PACKET FORMAT
	// [0]  - 0100 ... ID
	// [1]  - ---- --cp os0b l--?
	//      l = use lighting
	//      p = use dynamic palette (maybe not just this, wrong for roadedge car select where it isn't set but needs to be)
	//      o = use dynamic texture offset (sky reflection in xrally/roadedge windows, also waterfalls?)
	//      s = use dynamic scaling (hyper64 logos on xrally/roadedge)
	//      0 = always 0?
	//      b = backface culling?
	//      c = set on objects a certain distance away (maybe optimization to disable clipping against camera?)
	//      ? = roadedge: all vehicles ingame + select screen (also 3d maps on select screen), vehicle lights+windows only in attract, nothing else?
	//          all vehicles ingame + select screen, vehicle lights+windows only in attract, NOT on vehicles between stages, nothing else?
	//          nothing on other games?
	//
	//
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

	u32 size[4];
	u32 address[4];
	u32 megaOffset;
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
	const u32 threeDOffset = (u32(packet[2]) << 16) | u32(packet[3]);
	const u16 *const threeDPointer = &m_vertsrom[threeDOffset * 3];

	if (threeDOffset >= m_vertsrom.length())
	{
		// bbust2 quite often spams this invalid pointer
		if ((packet[2] == 0x2347) && (packet[3] == 0x5056))
			return;

		LOG3D("Strange geometry packet: (ignoring)\n");
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
	if (threeDPointer[5] != 0x0000) LOG3D("3dPointer[5] is non-zero!\n");

	size[0]    = threeDPointer[6];
	size[1]    = threeDPointer[7];
	if (threeDPointer[8] != 0x0000) LOG3D("3dPointer[8] is non-zero!\n");

	size[2]    = threeDPointer[9];
	size[3]    = threeDPointer[10];

	// the low 8-bits of each of these is used (or at least contains data, probably one byte for each hunk?)
	//if (threeDPointer[11] != 0x0000) LOG3D("3dPointer[11] is %04x!\n", threeDPointer[11]); //           ????         [11]; Used.
	//if (threeDPointer[12] != 0x0000) LOG3D("3dPointer[12] is %04x!\n", threeDPointer[12]); //           ????         [12]; Used.
	//if (threeDPointer[13] != 0x0000) LOG3D("3dPointer[13] is %04x!\n", threeDPointer[13]); //           ????         [13]; Used.
	//if (threeDPointer[14] != 0x0000) LOG3D("3dPointer[14] is %04x!\n", threeDPointer[14]); //           ????         [14]; Used.


	if (threeDPointer[15] != 0x0000) LOG3D("3dPointer[15] is non-zero!\n");
	if (threeDPointer[16] != 0x0000) LOG3D("3dPointer[16] is non-zero!\n");
	if (threeDPointer[17] != 0x0000) LOG3D("3dPointer[17] is non-zero!\n");

	if (threeDPointer[18] != 0x0000) LOG3D("3dPointer[18] is non-zero!\n");
	if (threeDPointer[19] != 0x0000) LOG3D("3dPointer[19] is non-zero!\n");
	if (threeDPointer[20] != 0x0000) LOG3D("3dPointer[20] is non-zero!\n");

	// Concatenate the megaOffset with the addresses
	address[0] |= (megaOffset << 16);
	address[1] |= (megaOffset << 16);
	address[2] |= (megaOffset << 16);
	address[3] |= (megaOffset << 16);

	// For all 4 polygon chunks
	for (int k = 0; k < 4; k++)
	{
		const u16 *chunkOffset = &m_vertsrom[address[k] * 3];
		for (int l = 0; l < size[k]; l++)
		{
			////////////////////////////////////////////
			// GATHER A SINGLE TRIANGLE'S INFORMATION //
			////////////////////////////////////////////
			// SINGLE POLY CHUNK FORMAT
			// [0] 0000 0000 cccc cccc    0 = always 0 | c = chunk type / format of data that follows (see below)
			// [1] ta-4 pppp pppp ssss    t = texture, always on for most games, on for the backgrounds only on sams64
			//                                if not set, u,v fields of vertices are direct palette indices, used on roadedge hng64 logo animation shadows
			//                            a = blend this sprite (blend might use 'lighting' level as alpha?)
			//                            4 = 4bpp texture  p = palette?  s = texture sheet (1024 x 1024 pages)
			// [2] S?hh hhhh hvvv vvvv    S = use texture offsets in lower bits
			//                            ? = unknown, sometimes used on objects further way, then flipped off, maybe precision related / texturing sampling mode?
			//                            h = horizontal offset into texture
			//                            v = vertical offset into texture

			// we currently use one of the palette bits to enable a different palette mode.. seems hacky...
			// looks like vertical / horizontal sub-pages might be 3 bits, not 2,  ? could be enable bit for that..

			// 'Welcome to South Africa' roadside banner on xrally | 000e 8c0d d870 or 0096 8c0d d870  (8c0d, d870 seems key 1000 1100 0000 1101
			//                                                                                                               1101 1000 0111 0000 )

			u8 chunkType = chunkOffset[0] & 0x00ff;

			// Debug - ajg
			if (chunkOffset[0] & 0xff00)
			{
				LOG3D("Weird!  The top byte of the chunkType has a value %04x!\n", chunkOffset[0]);
				continue;
			}

			// Syntactical simplification
			polygon& currentPoly = m_polys[numPolys];

			// Debug - ajg
			//LOG3D("%d (%08x) : %04x %04x %04x\n", k, address[k]*3*2, chunkOffset[0], chunkOffset[1], chunkOffset[2]);
			//break;

			if (chunkOffset[1] & 0x1000) currentPoly.tex4bpp = 0x1;
			else                         currentPoly.tex4bpp = 0x0;

			currentPoly.texPageSmall = chunkOffset[2];
			currentPoly.texIndex = chunkOffset[1] & 0x000f;

			// only values 07/08/09 have been observed.  Textures are only 1024 wide, so values above 09 (512) make little sense anyway
			currentPoly.tex_mask_x = 1 << m_texture_wrapsize_table[(currentPoly.texIndex * 2) + 0];
			currentPoly.tex_mask_y = 1 << m_texture_wrapsize_table[(currentPoly.texIndex * 2) + 1];

			// Flat shaded polygon, no texture, no lighting
			if (chunkOffset[1] & 0x8000)
				currentPoly.flatShade = false;
			else
				currentPoly.flatShade = true;

			currentPoly.blend = false;

			// PALETTE
			currentPoly.palOffset = 0;

			//u16 explicitPaletteValue0 = ((chunkOffset[?] & 0x????) >> ?) * 0x800;
			u16 explicitPaletteValue = ((chunkOffset[1] & 0x0ff0) >> 4);
			explicitPaletteValue = explicitPaletteValue << 3;

			// HACK: this is not the enable, the cars in roadedge rely on this to switch palettes
			// on the select screen, where this bit is not enabled.
			// (to see the cars on the select screen disable sprite rendering, as there are
			// currently priority issues)
			//
			// however for sams64 this is enabled on the 2nd character, but not the 1st character
			// and the additional palette offset definitely only applies to the 2nd
			//
			// Apply the dynamic palette offset if its flag is set, otherwise stick with the fixed one
			if ((packet[1] & 0x0100))
			{
				// bbust2 has m_paletteState3d & 0x40 set, which takes the palette out of range
				// used for 2nd car on roadedge, used for 2nd player on buriki
				// used for buildings in fatfurwa intro and characters
				explicitPaletteValue |= ((m_paletteState3d >> 8) & 0x3f) * 0x80;
			}

			currentPoly.palOffset += explicitPaletteValue;

			//if (chunkOffset[1] & 0x4000)
			//  currentPoly.palOffset = machine().rand()&0x3ff;

			//if (packet[1] & 0x0006)
			//  currentPoly.palOffset = machine().rand()&0x3ff;

			if (chunkOffset[1] & 0x4000)
				currentPoly.blend = true;

			// These are definitely the scroll values, used on player car windows and waterfalls
			// but if we always use them things get very messy when they're used for the waterfalls on xrally
			// as they never get turned back off again for the objects after the waterfalls
			// Must be a conditional enable?
			// 0x0100 is set on the cars, but not the waterfall
			// 0x0080 is set on the cars, and the waterfall - maybe correct?
			// 0x0001 is set on the cars, but not the waterfall
			if ((packet[1] & 0x0080))
			{
				currentPoly.texscrollx = m_texturescrollx;
				currentPoly.texscrolly = m_texturescrolly;
			}
			else
			{
				currentPoly.texscrollx = 0;
				currentPoly.texscrolly = 0;
			}

			u8 chunkLength = 0;
			int counter = 3;
			switch (chunkType)
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
			//
			/////////////////////////*/

			// 33 word chunk, 3 vertices, per-vertex UVs & normals, per-face normal
			case 0x05:  // 0000 0101
			case 0x0f:  // 0000 1111
			{
				for (int m = 0; m < 3; m++)
				{
					recoverStandardVerts(currentPoly, m, chunkOffset, counter, packet);

					currentPoly.vert[m].normal[0] = uToF(chunkOffset[counter++]);
					currentPoly.vert[m].normal[1] = uToF(chunkOffset[counter++]);
					currentPoly.vert[m].normal[2] = uToF(chunkOffset[counter++]);
					currentPoly.vert[m].normal[3] = 0.0f;
				}

				// Redundantly called, but it works...
				currentPoly.faceNormal[0] = uToF(chunkOffset[counter++]);
				currentPoly.faceNormal[1] = uToF(chunkOffset[counter++]);
				currentPoly.faceNormal[2] = uToF(chunkOffset[counter++]);
				currentPoly.faceNormal[3] = 0.0f;

				chunkLength = counter;
				break;
			}


			// 24 word chunk, 3 vertices, per-vertex UVs
			case 0x04:  // 0000 0100
			case 0x0e:  // 0000 1110
			case 0x24:  // 0010 0100
			case 0x2e:  // 0010 1110
			{
				for (int m = 0; m < 3; m++)
				{
					recoverStandardVerts(currentPoly, m, chunkOffset, counter, packet);
				}

				currentPoly.vert[0].normal[0] = currentPoly.vert[1].normal[0] = currentPoly.vert[2].normal[0] = uToF(chunkOffset[counter++]);
				currentPoly.vert[0].normal[1] = currentPoly.vert[1].normal[1] = currentPoly.vert[2].normal[1] = uToF(chunkOffset[counter++]);
				currentPoly.vert[0].normal[2] = currentPoly.vert[1].normal[2] = currentPoly.vert[2].normal[2] = uToF(chunkOffset[counter++]);
				currentPoly.vert[0].normal[3] = currentPoly.vert[1].normal[3] = currentPoly.vert[2].normal[3] = 0.0f;

				// Redundantly called, but it works...
				currentPoly.faceNormal[0] = currentPoly.vert[2].normal[0];
				currentPoly.faceNormal[1] = currentPoly.vert[2].normal[1];
				currentPoly.faceNormal[2] = currentPoly.vert[2].normal[2];
				currentPoly.faceNormal[3] = 0.0f;

				chunkLength = counter;
				break;
			}

			// 15 word chunk, 1 vertex, per-vertex UVs & normals, face normal
			case 0x87:  // 1000 0111
			case 0x97:  // 1001 0111
			case 0xd7:  // 1101 0111
			case 0xc7:  // 1100 0111
			{
				// Copy over the proper vertices from the previous triangle...
				memcpy(&currentPoly.vert[1], &lastPoly.vert[0], sizeof(polyVert));
				memcpy(&currentPoly.vert[2], &lastPoly.vert[2], sizeof(polyVert));

				recoverStandardVerts(currentPoly, 0, chunkOffset, counter, packet);

				currentPoly.vert[0].normal[0] = uToF(chunkOffset[counter++]);
				currentPoly.vert[0].normal[1] = uToF(chunkOffset[counter++]);
				currentPoly.vert[0].normal[2] = uToF(chunkOffset[counter++]);
				currentPoly.vert[0].normal[3] = 0.0f;

				currentPoly.faceNormal[0] = uToF(chunkOffset[counter++]);
				currentPoly.faceNormal[1] = uToF(chunkOffset[counter++]);
				currentPoly.faceNormal[2] = uToF(chunkOffset[counter++]);
				currentPoly.faceNormal[3] = 0.0f;

				chunkLength = counter;
				break;
			}

			// 12 word chunk, 1 vertex, per-vertex UVs
			case 0x86:  // 1000 0110
			case 0x96:  // 1001 0110
			case 0xb6:  // 1011 0110
			case 0xc6:  // 1100 0110
			case 0xd6:  // 1101 0110
			{
				// Copy over the proper vertices from the previous triangle...
				memcpy(&currentPoly.vert[1], &lastPoly.vert[0], sizeof(polyVert));
				memcpy(&currentPoly.vert[2], &lastPoly.vert[2], sizeof(polyVert));

				recoverStandardVerts(currentPoly, 0, chunkOffset, counter, packet);

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
				[[maybe_unused]] u16 unused;
				unused = chunkOffset[counter++];
				unused = chunkOffset[counter++];
				unused = chunkOffset[counter++];

				chunkLength = counter;
				break;
			}
			default:
				LOG3D("UNKNOWN geometry CHUNK TYPE : %02x\n", chunkType);
				chunkLength = counter;
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

					currentPoly.vert[v].light = intensity;
				}
			}
			else
			{
				// Just clear out the light values
				for (int v = 0; v < 3; v++)
				{
					currentPoly.vert[v].light = 0;
				}
			}

			float cullRay[4];
			float cullNorm[4];

			// Cast a ray out of the camera towards the polygon's point in eyespace.
			vecmatmul4(cullRay, m_modelViewMatrix, currentPoly.vert[0].worldCoords);
			normalize(cullRay);

			// Dot product that with the normal to see if you're negative...
			vecmatmul4(cullNorm, m_modelViewMatrix, currentPoly.faceNormal);

			// BACKFACE CULL
			// roadedge has various one-way barriers that you can drive through, but need to be invisible from behind, so needs this culling
			// this bit IS set on the objects that roadedge needs to vanish if viewed from behind, but it is NOT set on the bbust2 school bus
			// which needs to be visible with backfacing polys. further test cases need to be found.
			if (packet[1] & 0x0010)
			{
				const float backfaceCullResult = vecDotProduct(cullRay, cullNorm);
				if (backfaceCullResult < 0.0f)
					currentPoly.visible = true;
				else
					currentPoly.visible = false;
			}


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
					clipVerts[m].p[2] = currentPoly.vert[m].light;
				}

				if (currentPoly.visible)
				{
					// Clip against all edges of the view frustum
					const int num_vertices = frustum_clip_all<float, 5>(clipVerts, currentPoly.n, clipVerts);

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
						currentPoly.vert[m].light = clipVerts[m].p[2];
					}

					//const rectangle& visarea = m_screen->visible_area();
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
						windowCoords[0] = (ndCoords[0]+1.0f) * (512.0f / 2.0f) + 0.0f;
						windowCoords[1] = (ndCoords[1]+1.0f) * (512.0f / 2.0f) + 0.0f;
						windowCoords[2] = (ndCoords[2]+1.0f) * 0.5f;

						// Flip Y
						windowCoords[1] = 512.0f - windowCoords[1];

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


bool hng64_state::command3d(const u16* packet)
{
	int numPolys = 0;

	//LOG3D("packet type : %04x %04x|%04x %04x|%04x %04x|%04x %04x  | %04x %04x %04x %04x %04x %04x %04x %04x\n", packet[0],packet[1],packet[2],packet[3],packet[4],packet[5],packet[6],packet[7],     packet[8], packet[9], packet[10], packet[11], packet[12], packet[13], packet[14], packet[15]);

	switch (packet[0])
	{
	case 0x0000:    // NOP? / End current list (doesn't stop additional lists being sent this frame)
		return false;

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

	case 0x0100:  // Geometry with full transformations
		// xrally/roadedge cars (not track, that uses 102), buriki, fatfurwa
		recoverPolygonBlock(packet, numPolys);
		break;

	case 0x0101: // Geometry with full transformations (same as 0x100?)
		// sams64, sams64_2, bbust2
		recoverPolygonBlock(packet, numPolys);
		break;

	case 0x0102:    // Geometry with only translation
		// 'world' in roadedge/xrally (track, trackside objects etc.)
		// Split the packet and call recoverPolygonBlock on each half.
		u16 miniPacket[16];
		std::fill(std::begin(miniPacket), std::end(miniPacket), 0);
		std::copy_n(&packet[0], 7, std::begin(miniPacket));
		miniPacket[7] = 0x7fff;
		miniPacket[11] = 0x7fff;
		miniPacket[15] = 0x7fff;
		recoverPolygonBlock(miniPacket, numPolys);

		if (packet[7] == 1)
		{
			if (packet[8] == 0x0102)
			{
				std::fill(std::begin(miniPacket), std::end(miniPacket), 0);
				std::copy_n(&packet[8], 7, std::begin(miniPacket));
				miniPacket[7] = 0x7fff;
				miniPacket[11] = 0x7fff;
				miniPacket[15] = 0x7fff;
				recoverPolygonBlock(miniPacket, numPolys);

				// packet[15] is always 1?
			}
			else
			{
				/* if the 2nd value isn't 0x0102 don't render it
				   it could just be that the display list is corrupt at this point tho, see note above
				*/
			}
		}

		break;

	case 0x1000:    // Unknown: Some sort of global flags?
		//printPacket(packet, 1); LOG3D("\n");
		break;

	case 0x1001:    // Unknown: Some sort of global flags?  Almost always comes in a group of 4 with an index [0,3].
		//printPacket(packet, 1);
		break;

	default:
		LOG3D("HNG64: Unknown 3d command %04x.\n", packet[0]);
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
	return true;
}

void hng64_state::clear3d()
{
	// Reset the buffers...
	//const rectangle& visarea = m_screen->visible_area();
	for (int i = 0; i < 512*512; i++)
	{
		m_poly_renderer->depthBuffer3d()[i] = 100.0f;
		m_poly_renderer->colorBuffer3d()[i] = 0;
	}

	m_paletteState3d = 0;

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

u8 hng64_state::fbcontrol_r(offs_t offset)
{
	LOGFRAMEBUFFER("%s: fbcontrol_r (%03x)\n", machine().describe_context(), offset);
	return m_fbcontrol[offset];
}

void hng64_state::fbcontrol_w(offs_t offset, u8 data)
{

	/* roadedge does the following to turn off the framebuffer clear (leave trails) and then turn it back on when selecting a car
	   ':maincpu' (8001EDE0): fbcontrol_w (002) 10 (disable frame buffer clear)
	   ':maincpu' (8001FE4C): fbcontrol_w (002) 38 (normal)

	   during the Hyper Neogeo 64 logo it has a value of
	   ':maincpu' (8005AA44): fbcontrol_w (002) 18

	   sams64 does
	   ':maincpu' (800C13C4): fbcontrol_r (002)     (ANDs with 0x07, ORs with 0x18)
	   ':maincpu' (800C13D0): fbcontrol_w (002) 18

	   other games use either mix of 0x18 and 0x38.  bit 0x08 must prevent the framebuffer clear tho
	   according to above table bit 0x20 is color base, but implementation for it is a hack
	   roadedge ends up leaving 0x20 set after the car selection, which breaks ingame 3D palette if
	   we use it as a palette base? see hack

	   (3d car currently not visible on roadedge select screen due to priority issue, disable sprites to see it)

	*/

	LOGFRAMEBUFFER("%s: fbcontrol_w (%03x) %02x\n", machine().describe_context(), offset, data);
	m_fbcontrol[offset] = data;
}


// the framebuffer scroll and scale registers are used in fatfurwa (intro scaling) and xrally (course select, car select)
// they are NOT used for buriki 'how to play' scren, which uses unhandled values in the 3d packets to reposition the fighters instead
void hng64_state::fbscale_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_fbscale[offset]);

	if (mem_mask & 0xffff0000)
	{
		// NORMAL value is 3fe0 (0x400 / 2 = 0x200 = 512)
		// ':maincpu' (8006E46C): fb_scale_x 3fe00000 ffff0000

		// on xrally course select this is 39e0 (0x3a0 / 2 = 0x1d0 = 464)
		// fb_scale_x 39e00000 ffff0000

		//LOGFRAMEBUFFER("%s: fb_scale_x %08x %08x\n", machine().describe_context(), data, mem_mask);
	}

	if (mem_mask & 0x0000ffff)
	{
		// NORMAL value is 37e0  (0x380 / 2 = 0x1c0 = 448)
		// fb_scale_y 000037e0 0000ffff

		// on xrally course select this is 32e0 (0x330 / 2 = 408)
		// fb_scale_y 000032e0 0000ffff

		// during fatfurwa scaled intro it uses 2de0, although writes 37e0 in the same frame; presumably rendering takes place while it is 2de0 though
		// 0x2e0 / 2 = 0x170 = 368    (needs to be ~287 pixels though)
		// ':maincpu' (800667A0): fb_scale_y 00002de0 0000ffff
		//LOGFRAMEBUFFER("%s: fb_scale_y %08x %08x\n", machine().describe_context(), data, mem_mask);
	}
}

void hng64_state::fbscroll_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_fbscroll[offset]);

	// this is used ingame on the samsho games, and on the car select screen in xrally (youtube video confirms position of car needs to change)
	if (mem_mask & 0xffff0000)
	{
		// NORMAL value is e000 (e0 = 224)
		// fbscroll x e0000000 ffff0000

		// on xrally course select this is e600  (+0600 from normal)
		// ':maincpu' (8002327C): fbscroll x e6000000 ffff0000

		// on xrally car select this is e680
		// fbscroll x e6800000 ffff0000
		//LOGFRAMEBUFFER("%s: fbscroll x %08x (%d) %08x\n", machine().describe_context(), data, ((data&0x7fff0000) >> 21), mem_mask);
	}

	if (mem_mask & 0x0000ffff)
	{
		// NORMAL value is 1c00  (1c0 = 448) /2 = 224 (midpoint y?)
		// ':maincpu' (8006FA18): fbscroll y 00001c00 0000ffff

		// on xrally course select this is 1700  (0x170 = 368)
		// ':maincpu' (8002327C): fbscroll y 00001700 0000ffff (-0500 from normal)

		// on xrally car select this is 1260 (and needs to be higher up)
		// ':maincpu' (80012820): fbscroll y 00001260 0000ffff
		// 00001a60 on screen after, not quite as high up, but higher than 1c00
		//LOGFRAMEBUFFER("%s: fbscroll y %08x (%d) %08x\n", machine().describe_context(), data, (data >> 5), mem_mask);
	}
}

void hng64_state::fbunkbyte_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_fbunk[offset]);

	// | ---- --?x ---- ---- ---- ---- ---- ----| unknown
	// is 02 in most games, 03 in samsh4 games
	// could be related to how fbscrolly is applied?

	LOGFRAMEBUFFER("%s: fbunkbyte_w %08x %08x\n", machine().describe_context(), data, mem_mask);
}

/*
this is a table filled with 0x0? data, seems to be 8-bit values

roadedge  08080808 08080808 08080808 08080808 08080808 08080808 08080707 08080909 (ingame)
          08080808 08080808 08080808 08080808 08080808 08080808 08080808 08080808 (hyper logo)

xrally    08080808 08080808 08070707 08070807 07070807 08070707 08070707 07070808 (comms screen + ingame)
          08080808 08080808 08080808 08080808 08080808 08080808 08080808 08080808 (hyper logo)

buriki    08080808 08080808 08080808 08080808 08080808 08080808 08080808 08080808
fatfurwa  08080808 08080808 08080808 08080808 08080808 08080808 08080808 08080808
bbust2    08080808 08080808 08080808 08080808 08080808 08080808 08080808 08080808

sams64    00000000 00000000 00000000 00000000 00000000 07070000 00000000 00000000 (only inits one value to 0707?)
sams64_2  00000000 00000000 00000000 00000000 00000000 07070000 00000000 00000000 (only inits one value to 0707?)

these values are used in the rendering, to control the size at which a texture wraps on each of
the texture pages.
*/
u8 hng64_state::texture_wrapsize_table_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOGTEXTURE("%s: texture_wrapsize_table_r (%03x)\n", machine().describe_context(), offset * 4);
	return m_texture_wrapsize_table[offset];
}

void hng64_state::texture_wrapsize_table_w(offs_t offset, u8 data)
{
	LOGTEXTURE("%s: texture_wrapsize_table_w (%03x) %08x\n", machine().describe_context(), offset * 4, data);
	m_texture_wrapsize_table[offset] = data;

#if 0
	{
		printf("m_texture_wrapsize_table is now\n");
		for (int i = 0; i < 0x20; i+=2)
		{
			printf("%01x:%02x,%02x| ", i/2, m_texture_wrapsize_table[i],m_texture_wrapsize_table[i+1]);
		}
		printf("\n");
	}
#endif
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
	return ((a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]));
}

void hng64_state::setIdentity(float *matrix)
{
	for (int i = 0; i < 16; i++)
	{
		matrix[i] = 0.0f;
	}

	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}

float hng64_state::uToF(u16 input)
{
	float retVal;
	retVal = float(s16(input)) / 32768.0f;
	return retVal;

#if 0
	if (s16(input) < 0)
		retVal = float(s16(input)) / 32768.0f;
	else
		retVal = float(s16(input)) / 32767.0f;
#endif
}

void hng64_state::normalize(float* x)
{
	const double l2 = (x[0] * x[0]) + (x[1] * x[1]) + (x[2] * x[2]);
	const double l = sqrt(l2);

	x[0] = float(x[0] / l);
	x[1] = float(x[1] / l);
	x[2] = float(x[2] / l);
}


////////////////////////////////
// POLYGON RASTERIZATION CODE //
////////////////////////////////

void hng64_poly_renderer::render_texture_scanline(s32 scanline, const extent_t& extent, const hng64_poly_data& renderData, int threadid)
{
	if ((scanline > 511) | (scanline < 0))
		return;

	// Pull the parameters out of the extent structure
	float z = extent.param[0].start;
	float w = extent.param[1].start;
	float light = extent.param[2].start;
	float s = extent.param[5].start;
	float t = extent.param[6].start;

	const float dz = extent.param[0].dpdx;
	const float dw = extent.param[1].dpdx;
	const float dlight = extent.param[2].dpdx;
	const float ds = extent.param[5].dpdx;
	const float dt = extent.param[6].dpdx;

	// Pointers to the pixel buffers
	u16* colorBuffer = &m_colorBuffer3d[(scanline * 512)];
	float* depthBuffer = &m_depthBuffer3d[(scanline * 512)];

	const u8 *textureOffset = &m_state.m_texturerom[renderData.texIndex * 1024 * 1024];

	// Step over each pixel in the horizontal span
	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z < depthBuffer[x & 511])
		{
			// Multiply back through by w for everything that was interpolated perspective-correctly
			const float sCorrect = s / w;
			const float tCorrect = t / w;

			const float rCorrect = light / w;

			{
				float textureS = 0.0f;
				float textureT = 0.0f;

				// sCorrect and tCorrect have range 0.0f - 1.0f, multiply by 1024 to get texture offset
				textureS = sCorrect * 1024.0f;
				textureT = tCorrect * 1024.0f;

				textureS += (renderData.texscrolly & 0x3fff)>>5;
				textureT += (renderData.texscrollx & 0x3fff)>>5;

				const int textPageSub = (renderData.texPageSmall & 0xc000) >> 14;
				const int texPageHorizOffset = (renderData.texPageSmall & 0x3f80) >> 7;
				const int texPageVertOffset =  (renderData.texPageSmall & 0x007f) >> 0;

				// Small-Page textures
				// what is textPageSub & 1 used for? seems to be enabled on almost everything? it does not control wrap enable?
				if (textPageSub & 2)
				{
					textureT = fmod(textureT, (float)renderData.tex_mask_x); textureT += (8.0f * texPageHorizOffset);
					textureS = fmod(textureS, (float)renderData.tex_mask_y); textureS += (8.0f * texPageVertOffset);
				}

				u8 paletteEntry;
				int t = (int)textureT;
				int s = (int)textureS;

				t &= 1023;
				s &= 1023; // 4bpp pages seem to be limited to 1024 pixels, with page numbering being the same, the bottom 1024 pixels of 4bpp pages are unused?

				if (renderData.tex4bpp)
				{
					paletteEntry = textureOffset[s * 512 + (t >> 1)];

					if (t & 1)
						paletteEntry = (paletteEntry >> 4) & 0x0f;
					else
						paletteEntry &= 0x0f;
				}
				else
				{
					paletteEntry = textureOffset[s * 1024 + t];
				}

				// pen 0 in textures is always transparent
				if (paletteEntry != 0)
				{
					float rIntensity = rCorrect / 16.0f;
					u8 lightval = (u8)rIntensity;
					u16 color = ((renderData.palOffset + paletteEntry) & 0x7ff) | (lightval << 12);
					if (renderData.blend)
						color |= 0x800;

					colorBuffer[x & 511] = color;
					depthBuffer[x & 511] = z;
				}
			}
		}

		z += dz;
		w += dw;
		light += dlight;
		s += ds;
		t += dt;
	}
}

void hng64_poly_renderer::render_flat_scanline(s32 scanline, const extent_t& extent, const hng64_poly_data& renderData, int threadid)
{
	if ((scanline > 511) | (scanline < 0))
		return;

	// Pull the parameters out of the extent structure
	float z = extent.param[0].start;
	const float dz = extent.param[0].dpdx;

	// Pointers to the pixel buffers
	float* depthBuffer = &m_depthBuffer3d[(scanline * 512)];
	u16* colorBuffer = &m_colorBuffer3d[(scanline * 512)];

	// Step over each pixel in the horizontal span
	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z < depthBuffer[x & 511])
		{
			colorBuffer[x & 511] = renderData.palOffset + renderData.colorIndex;
			depthBuffer[x & 511] = z;
		}

		z += dz;
	}
}

void hng64_poly_renderer::drawShaded(polygon *p)
{
	// Polygon information for the rasterizer
	hng64_poly_data rOptions;
	rOptions.tex4bpp = p->tex4bpp;
	rOptions.texIndex = p->texIndex;
	rOptions.palOffset = p->palOffset;
	rOptions.texPageSmall = p->texPageSmall;
	rOptions.colorIndex = p->colorIndex;
	rOptions.blend = p->blend;
	rOptions.texscrollx = p->texscrollx;
	rOptions.texscrolly = p->texscrolly;
	rOptions.tex_mask_x = p->tex_mask_x;
	rOptions.tex_mask_y = p->tex_mask_y;

	// Pass the render data into the rasterizer
	hng64_poly_data& renderData = object_data().next();
	renderData = rOptions;

	rectangle visibleArea;
	visibleArea.set(0, 512, 0, 512);

	if (p->flatShade)
	{
		// Rasterize the triangles
		for (int j = 1; j < p->n-1; j++)
		{
			// Build some MAME rasterizer vertices from the hng64 vertices
			vertex_t pVert[3];

			const polyVert& pv0 = p->vert[0];
			pVert[0].x = pv0.clipCoords[0];
			pVert[0].y = pv0.clipCoords[1];
			pVert[0].p[0] = pv0.clipCoords[2];

			const polyVert& pvj = p->vert[j];
			pVert[1].x = pvj.clipCoords[0];
			pVert[1].y = pvj.clipCoords[1];
			pVert[1].p[0] = pvj.clipCoords[2];

			const polyVert& pvjp1 = p->vert[j+1];
			pVert[2].x = pvjp1.clipCoords[0];
			pVert[2].y = pvjp1.clipCoords[1];
			pVert[2].p[0] = pvjp1.clipCoords[2];

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
			p->vert[j].light      = p->vert[j].light     * p->vert[j].clipCoords[3];
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
			pVert[0].p[2] = pv0.light;
			pVert[0].p[5] = pv0.texCoords[0];
			pVert[0].p[6] = pv0.texCoords[1];

			const polyVert& pvj = p->vert[j];
			pVert[1].x = pvj.clipCoords[0];
			pVert[1].y = pvj.clipCoords[1];
			pVert[1].p[0] = pvj.clipCoords[2];
			pVert[1].p[1] = pvj.clipCoords[3];
			pVert[1].p[2] = pvj.light;
			pVert[1].p[5] = pvj.texCoords[0];
			pVert[1].p[6] = pvj.texCoords[1];

			const polyVert& pvjp1 = p->vert[j+1];
			pVert[2].x = pvjp1.clipCoords[0];
			pVert[2].y = pvjp1.clipCoords[1];
			pVert[2].p[0] = pvjp1.clipCoords[2];
			pVert[2].p[1] = pvjp1.clipCoords[3];
			pVert[2].p[2] = pvjp1.light;
			pVert[2].p[5] = pvjp1.texCoords[0];
			pVert[2].p[6] = pvjp1.texCoords[1];

			render_triangle<7>(visibleArea, render_delegate(&hng64_poly_renderer::render_texture_scanline, this), pVert[0], pVert[1], pVert[2]);
		}
	}
}
