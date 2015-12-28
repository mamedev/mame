// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
/*
 * geforce 3d (NV2A) vertex program disassembler
 */
class vertex_program_disassembler {
	static const char *srctypes[];
	static const char *scaops[];
	static const int scapar2[];
	static const char *vecops[];
	static const int vecpar2[];
	static const char *vecouts[];
	static const char compchar[];
	int o[6];
	int state;

	struct sourcefields
	{
		int Sign;
		int SwizzleX;
		int SwizzleY;
		int SwizzleZ;
		int SwizzleW;
		int TempIndex;
		int ParameterType;
	};

	struct fields
	{
		int ScaOperation;
		int VecOperation;
		int SourceConstantIndex;
		int InputIndex;
		sourcefields src[3];
		int VecTempWriteMask;
		int VecTempIndex;
		int ScaTempWriteMask;
		int OutputWriteMask;
		int OutputSelect;
		int OutputIndex;
		int MultiplexerControl;
		int Usea0x;
		int EndOfProgram;
	};
	fields f;

	void decodefields(unsigned int *dwords, int offset, fields &decoded);
	int disassemble_mask(int mask, char *s);
	int disassemble_swizzle(sourcefields f, char *s);
	int disassemble_source(sourcefields f, fields fi, char *s);
	int disassemble_output(fields f, char *s);
	int output_types(fields f, int *o);
public:
	vertex_program_disassembler() { state = 0; }
	int disassemble(unsigned int *instruction, char *line);
};

/*
 * geforce 3d (NV2A) vertex structure
 */
struct vertex_nv {
	union {
		float fv[4];
		UINT32 iv[4];
	} attribute[16];
};

/*
 * geforce 3d (NV2A) vertex program simulator
 */
class vertex_program_simulator {
public:
	enum VectorialOperation {
		VecNOP=0,
		VecMOV,
		VecMUL,
		VecADD,
		VecMAD,
		VecDP3,
		VecDPH,
		VecDP4,
		VecDST,
		VecMIN,
		VecMAX,
		VecSLT,
		VecSGE,
		VecARL
	};
	enum ScalarOperation {
		ScaNOP=0,
		ScaIMV,
		ScaRCP,
		ScaRCC,
		ScaRSQ,
		ScaEXP,
		ScaLOG,
		ScaLIT
	};
	vertex_program_simulator();
	// input vertex
	vertex_nv *input;
	// input parameters
	union constant {
		float fv[4];
		unsigned int iv[4];
	} c_constant[192];
	union temp {
		float fv[4];
		unsigned int iv[4];
	} r_temp[32];
	// output vertex
	vertex_nv *output;
	// instructions
	struct instruction {
		unsigned int i[4];
		int modified;
		struct decoded {
			int SwizzleA[4], SignA, ParameterTypeA, TempIndexA;
			int SwizzleB[4], SignB, ParameterTypeB, TempIndexB;
			int SwizzleC[4], SignC, ParameterTypeC, TempIndexC;
			VectorialOperation VecOperation;
			ScalarOperation ScaOperation;
			int OutputWriteMask, MultiplexerControl;
			int VecTempWriteMask, ScaTempWriteMask;
			int VecTempIndex, OutputIndex;
			int InputIndex;
			int SourceConstantIndex;
			int OutputSelect;
			int Usea0x;
			int EndOfProgram;
		} d;
	} op[256];
public:
	void set_data(vertex_nv *in, vertex_nv *out);
	void reset();
	int step();
	void decode_instruction(int address);
	void execute();
	void jump(int address);
	void process(int address, vertex_nv *in, vertex_nv *out, int count);
	int status();
private:
	void initialize_outputs();
	void initialize_temps();
	void initialize_constants();
	void generate_input(float t[4], int sign, int type, int temp, int swizzle[4]);
	void compute_vectorial_operation(float t[4], int instruction, float par[3 * 4]);
	void compute_scalar_operation(float t[4], int instruction, float par[3 * 4]);

	int ip;
	int a0x;
};

class nv2a_renderer; // forward declaration
struct nvidia_object_data
{
	nv2a_renderer *data;
};

/*
 * geforce 3d (NV2A) accellerator
 */
/* very simplified view
there is a set of context objects

context objects are stored in RAMIN
each context object is identified by an handle stored in RAMHT

each context object can be assigned to a channel
to assign you give to the channel an handle for the object

offset in ramht=(((((handle >> 11) xor handle) >> 11) xor handle) & 0x7ff)*8
offset in ramht contains the handle itself
offset in ramht+4 contains in the lower 16 bits the offset in RAMIN divided by 16

objects have methods used to do drawing
most methods set parameters, others actually draw
*/
class nv2a_renderer : public poly_manager<float, nvidia_object_data, 13, 8192>
{
public:
	enum class VERTEX_PARAMETER {
		PARAM_COLOR_B = 0,
		PARAM_COLOR_G = 1,
		PARAM_COLOR_R = 2,
		PARAM_COLOR_A = 3,
		PARAM_TEXTURE0_U = 4,
		PARAM_TEXTURE0_V = 5,
		PARAM_TEXTURE1_U = 6,
		PARAM_TEXTURE1_V = 7,
		PARAM_TEXTURE2_U = 8,
		PARAM_TEXTURE2_V = 9,
		PARAM_TEXTURE3_U = 10,
		PARAM_TEXTURE3_V = 11,
		PARAM_Z = 12
	};
	enum class NV2A_BEGIN_END {
		STOP = 0,
		POINTS = 1,
		LINES = 2,
		LINE_LOOP = 3,
		LINE_STRIP = 4,
		TRIANGLES = 5,
		TRIANGLE_STRIP = 6,
		TRIANGLE_FAN = 7,
		QUADS = 8,
		QUAD_STRIP = 9,
		POLYGON = 10
	};
	enum class NV2A_VERTEX_ATTR {
		POS = 0,
		WEIGHT = 1,
		NORMAL = 2,
		COLOR0 = 3, // diffuse
		COLOR1 = 4, // specular
		FOG = 5,
		BACKCOLOR0 = 7, // diffuse
		BACKCOLOR1 = 8, // specular
		TEX0 = 9,
		TEX1 = 10,
		TEX2 = 11,
		TEX3 = 12
	};
	enum class NV2A_VTXBUF_TYPE {
		UBYTE2 = 0, // what is the difference with UBYTE ?
		FLOAT = 2,
		UBYTE = 4,
		USHORT = 5,
		UNKNOWN_6 = 6 // used for vertex color
	};
	enum class NV2A_TEX_FORMAT {
		L8 = 0x0,
		I8 = 0x1,
		A1R5G5B5 = 0x2,
		A4R4G4B4 = 0x4,
		R5G6B5 = 0x5,
		A8R8G8B8 = 0x6,
		X8R8G8B8 = 0x7,
		INDEX8 = 0xb,
		DXT1 = 0xc,
		DXT3 = 0xe,
		DXT5 = 0xf,
		A1R5G5B5_RECT = 0x10,
		R5G6B5_RECT = 0x11,
		A8R8G8B8_RECT = 0x12,
		L8_RECT = 0x13,
		DSDT8_RECT = 0x17,
		A8L8 = 0x1a,
		I8_RECT = 0x1b,
		A4R4G4B4_RECT = 0x1d,
		R8G8B8_RECT = 0x1e,
		A8L8_RECT = 0x20,
		Z24 = 0x2a,
		Z24_RECT = 0x2b,
		Z16 = 0x2c,
		Z16_RECT = 0x2d,
		DSDT8 = 0x28,
		HILO16 = 0x33,
		HILO16_RECT = 0x36,
		HILO8 = 0x44,
		SIGNED_HILO8 = 0x45,
		HILO8_RECT = 0x46,
		SIGNED_HILO8_RECT = 0x47
	};
	enum class NV2A_LOGIC_OP {
		CLEAR = 0x1500,
		AND = 0x1501,
		AND_REVERSE = 0x1502,
		COPY = 0x1503,
		AND_INVERTED = 0x1504,
		NOOP = 0x1505,
		XOR = 0x1506,
		OR = 0x1507,
		NOR = 0x1508,
		EQUIV = 0x1509,
		INVERT = 0x150a,
		OR_REVERSE = 0x150b,
		COPY_INVERTED = 0x150c,
		OR_INVERTED = 0x150d,
		NAND = 0x150e,
		SET = 0x150f
	};
	enum class NV2A_BLEND_EQUATION {
		FUNC_ADD = 0x8006,
		MIN = 0x8007,
		MAX = 0x8008,
		FUNC_SUBTRACT = 0x800a,
		FUNC_REVERSE_SUBTRACT = 0x80b
	};
	enum class NV2A_BLEND_FACTOR {
		ZERO = 0x0000,
		ONE = 0x0001,
		SRC_COLOR = 0x0300,
		ONE_MINUS_SRC_COLOR = 0x0301,
		SRC_ALPHA = 0x0302,
		ONE_MINUS_SRC_ALPHA = 0x0303,
		DST_ALPHA = 0x0304,
		ONE_MINUS_DST_ALPHA = 0x0305,
		DST_COLOR = 0x0306,
		ONE_MINUS_DST_COLOR = 0x0307,
		SRC_ALPHA_SATURATE = 0x0308,
		CONSTANT_COLOR = 0x8001,
		ONE_MINUS_CONSTANT_COLOR = 0x8002,
		CONSTANT_ALPHA = 0x8003,
		ONE_MINUS_CONSTANT_ALPHA = 0x8004
	};
	enum class NV2A_COMPARISON_OP {
		NEVER = 0x0200,
		LESS = 0x0201,
		EQUAL = 0x0202,
		LEQUAL = 0x0203,
		GREATER = 0x0204,
		NOTEQUAL = 0x0205,
		GEQUAL = 0x0206,
		ALWAYS = 0x0207
	};
	enum class NV2A_STENCIL_OP {
		ZEROOP = 0x0000,
		INVERTOP = 0x150a,
		KEEP = 0x1e00,
		REPLACE = 0x1e01,
		INCR = 0x1e02,
		DECR = 0x1e03,
		INCR_WRAP = 0x8507,
		DECR_WRAP = 0x8508
	};
	enum class NV2A_RT_TYPE {
		LINEAR = 1,
		SWIZZLED = 2
	};
	enum class NV2A_RT_DEPTH_FORMAT {
		Z16 = 0x0001,
		Z24S8 = 0x0002
	};
	enum class NV2A_COLOR_FORMAT {
		X1R5G5B5_Z1R5G5B5 = 1,
		X1R5G5B5_X1R5G5B5 = 2,
		R5G6B5 = 3,
		X8R8G8B8_Z8R8G8B8 = 4,
		X8R8G8B8_X8R8G8B8 = 5,
		X1A7R8G8B8_Z1A7R8G8B8 = 6,
		X1A7R8G8B8_X1A7R8G8B8 = 7,
		A8R8G8B8 = 8,
		B8 = 9,
		G8B8 = 10
	};

	nv2a_renderer(running_machine &machine) : poly_manager<float, nvidia_object_data, 13, 8192>(machine)
	{
		memset(channel, 0, sizeof(channel));
		memset(pfifo, 0, sizeof(pfifo));
		memset(pcrtc, 0, sizeof(pcrtc));
		memset(pmc, 0, sizeof(pmc));
		memset(pgraph, 0, sizeof(pgraph));
		memset(ramin, 0, sizeof(ramin));
		computedilated();
		objectdata = &(object_data_alloc());
		objectdata->data = this;
		combiner.used = 0;
		combiner.lock = osd_lock_alloc();
		enabled_vertex_attributes = 0;
		indexesleft_count = 0;
		vertex_pipeline = 4;
		color_mask = 0xffffffff;
		alpha_test_enabled = false;
		alpha_reference = 0;
		alpha_func = NV2A_COMPARISON_OP::ALWAYS;
		depth_test_enabled = false;
		depth_function = NV2A_COMPARISON_OP::LESS;
		depth_write_enabled = false;
		stencil_test_enabled = false;
		stencil_func = NV2A_COMPARISON_OP::ALWAYS;
		stencil_ref = 0;
		stencil_mask = -1;
		stencil_op_fail = NV2A_STENCIL_OP::KEEP;
		stencil_op_zfail = NV2A_STENCIL_OP::KEEP;
		stencil_op_zpass = NV2A_STENCIL_OP::KEEP;
		blending_enabled = false;
		blend_equation = NV2A_BLEND_EQUATION::FUNC_ADD;
		blend_color = 0;
		blend_function_destination = NV2A_BLEND_FACTOR::ZERO;
		blend_function_source = NV2A_BLEND_FACTOR::ONE;
		logical_operation_enabled = false;
		logical_operation = NV2A_LOGIC_OP::COPY;
		limits_rendertarget.set(0, 0, 640, 480);
		pitch_rendertarget = 0;
		pitch_depthbuffer = 0;
		log2height_rendertarget = 0;
		log2width_rendertarget = 0;
		dilate_rendertarget = 0;
		antialiasing_rendertarget = 0;
		type_rendertarget = NV2A_RT_TYPE::LINEAR;
		depthformat_rendertarget = NV2A_RT_DEPTH_FORMAT::Z24S8;
		colorformat_rendertarget = NV2A_COLOR_FORMAT::A8R8G8B8;
		bytespixel_rendertarget = 4;
		clear_rendertarget.set(0, 0, 639, 479);
		primitive_type = NV2A_BEGIN_END::STOP;
		antialias_control = 0;
		rendertarget = nullptr;
		depthbuffer = nullptr;
		displayedtarget = nullptr;
		puller_channel = 0;
		puller_waiting = 0;
		debug_grab_texttype = -1;
		debug_grab_textfile = nullptr;
		waitvblank_used = 1;
		memset(vertex_attribute_words, 0, sizeof(vertex_attribute_words));
		memset(vertex_attribute_offset, 0, sizeof(vertex_attribute_offset));
	}
	DECLARE_READ32_MEMBER(geforce_r);
	DECLARE_WRITE32_MEMBER(geforce_w);
	void vblank_callback(screen_device &screen, bool state);
	UINT32 screen_update_callback(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	bool update_interrupts();
	void set_interrupt_device(pic8259_device *device);

	void render_texture_simple(INT32 scanline, const extent_t &extent, const nvidia_object_data &extradata, int threadid);
	void render_color(INT32 scanline, const extent_t &extent, const nvidia_object_data &extradata, int threadid);
	void render_register_combiners(INT32 scanline, const extent_t &extent, const nvidia_object_data &objectdata, int threadid);

	int geforce_commandkind(UINT32 word);
	UINT32 geforce_object_offset(UINT32 handle);
	void geforce_read_dma_object(UINT32 handle, UINT32 &offset, UINT32 &size);
	void geforce_assign_object(address_space &space, UINT32 chanel, UINT32 subchannel, UINT32 address);
	int geforce_exec_method(address_space &space, UINT32 channel, UINT32 subchannel, UINT32 method, UINT32 address, int &countlen);
	UINT32 texture_get_texel(int number, int x, int y);
	UINT8 *read_pixel(int x, int y, INT32 c[4]);
	void write_pixel(int x, int y, UINT32 color, UINT32 depth);
	void combiner_initialize_registers(UINT32 argb8[6]);
	void combiner_initialize_stage(int stage_number);
	void combiner_initialize_final();
	void combiner_map_input(int stage_number); // map combiner registers to variables A..D
	void combiner_map_output(int stage_number); // map combiner calculation results to combiner registers
	void combiner_map_final_input(); // map final combiner registers to variables A..F
	void combiner_final_output(); // generate final combiner output
	float combiner_map_input_select(int code, int index); // get component index in register code
	float *combiner_map_input_select3(int code); // get pointer to register code
	float *combiner_map_output_select3(int code); // get pointer to register code for output
	float combiner_map_input_function(int code, float value); // apply input mapping function code to value
	void combiner_map_input_function3(int code, float *data); // apply input mapping function code to data
	void combiner_function_AB(float result[4]);
	void combiner_function_AdotB(float result[4]);
	void combiner_function_CD(float result[4]);
	void combiner_function_CdotD(float result[4]);
	void combiner_function_ABmuxCD(float result[4]);
	void combiner_function_ABsumCD(float result[4]);
	void combiner_compute_rgb_outputs(int index);
	void combiner_compute_a_outputs(int index);
	void combiner_argb8_float(UINT32 color, float reg[4]);
	UINT32 combiner_float_argb8(float reg[4]);
	UINT32 dilate0(UINT32 value, int bits);
	UINT32 dilate1(UINT32 value, int bits);
	void computedilated(void);
	int toggle_register_combiners_usage();
	int toggle_wait_vblank_support();
	void debug_grab_texture(int type, const char *filename);
	void debug_grab_vertex_program_slot(int slot, UINT32 *instruction);
	void start(address_space *cpu_space);
	void savestate_items();
	void read_vertex(address_space & space, offs_t address, vertex_nv &vertex, int attrib);
	int read_vertices_0x1800(address_space & space, vertex_nv *destination, UINT32 address, int limit);
	int read_vertices_0x1808(address_space & space, vertex_nv *destination, UINT32 address, int limit);
	int read_vertices_0x1810(address_space & space, vertex_nv *destination, int offset, int limit);
	int read_vertices_0x1818(address_space & space, vertex_nv *destination, UINT32 address, int limit);
	void convert_vertices_poly(vertex_nv *source, vertex_t *destination, int count);
	void clear_render_target(int what, UINT32 value);
	void clear_depth_buffer(int what, UINT32 value);
	inline UINT8 *direct_access_ptr(offs_t address);
	TIMER_CALLBACK_MEMBER(puller_timer_work);

	struct {
		UINT32 regs[0x80 / 4];
		struct {
			UINT32 objhandle;
			UINT32 objclass;
			UINT32 method[0x2000 / 4];
		} object;
	} channel[32][8];
	UINT32 pfifo[0x2000 / 4];
	UINT32 pcrtc[0x1000 / 4];
	UINT32 pmc[0x1000 / 4];
	UINT32 pgraph[0x2000 / 4];
	UINT32 ramin[0x100000 / 4];
	UINT32 dma_offset[2];
	UINT32 dma_size[2];
	UINT8 *basemempointer;
	pic8259_device *interruptdevice;
	rectangle limits_rendertarget;
	UINT32 pitch_rendertarget;
	UINT32 pitch_depthbuffer;
	int log2height_rendertarget;
	int log2width_rendertarget;
	int dilate_rendertarget;
	int antialiasing_rendertarget;
	NV2A_RT_TYPE type_rendertarget;
	NV2A_RT_DEPTH_FORMAT depthformat_rendertarget;
	NV2A_COLOR_FORMAT colorformat_rendertarget;
	int bytespixel_rendertarget;
	rectangle clear_rendertarget;
	UINT32 antialias_control;
	UINT32 *rendertarget;
	UINT32 *depthbuffer;
	UINT32 *displayedtarget;
	UINT32 vertexbuffer_address[16];
	int vertexbuffer_stride[16];
	NV2A_VTXBUF_TYPE vertexbuffer_kind[16];
	int vertexbuffer_size[16];
	struct {
		int enabled;
		int sizeu;
		int sizev;
		int sizew;
		int dilate;
		NV2A_TEX_FORMAT format;
		int rectangle_pitch;
		void *buffer;
	} texture[4];
	NV2A_BEGIN_END primitive_type;
	int primitives_count;
	int indexesleft_count;
	int indexesleft_first;
	UINT32 indexesleft[1024]; // vertex indices sent by the software to the 3d accelerator
	int vertex_count;
	unsigned int vertex_first;
	vertex_nv vertex_software[1024+2]; // vertex attributes sent by the software to the 3d accelerator
	vertex_t vertex_xy[1024+2]; // vertex attributes computed by the 3d accelerator

	struct {
		float variable_A[4]; // 0=R 1=G 2=B 3=A
		float variable_B[4];
		float variable_C[4];
		float variable_D[4];
		float variable_E[4];
		float variable_F[4];
		float variable_G;
		float variable_EF[4];
		float variable_sumclamp[4];
		float function_RGBop1[4]; // 0=R 1=G 2=B
		float function_RGBop2[4];
		float function_RGBop3[4];
		float function_Aop1;
		float function_Aop2;
		float function_Aop3;
		float register_primarycolor[4]; // rw
		float register_secondarycolor[4];
		float register_texture0color[4];
		float register_texture1color[4];
		float register_texture2color[4];
		float register_texture3color[4];
		float register_color0[4];
		float register_color1[4];
		float register_spare0[4];
		float register_spare1[4];
		float register_fogcolor[4]; // ro
		float register_zero[4];
		float output[4];
		struct {
			float register_constantcolor0[4];
			float register_constantcolor1[4];
			int mapin_aA_input;
			int mapin_aA_component;
			int mapin_aA_mapping;
			int mapin_aB_input;
			int mapin_aB_component;
			int mapin_aB_mapping;
			int mapin_aC_input;
			int mapin_aC_component;
			int mapin_aC_mapping;
			int mapin_aD_input;
			int mapin_aD_component;
			int mapin_aD_mapping;
			int mapin_rgbA_input;
			int mapin_rgbA_component;
			int mapin_rgbA_mapping;
			int mapin_rgbB_input;
			int mapin_rgbB_component;
			int mapin_rgbB_mapping;
			int mapin_rgbC_input;
			int mapin_rgbC_component;
			int mapin_rgbC_mapping;
			int mapin_rgbD_input;
			int mapin_rgbD_component;
			int mapin_rgbD_mapping;
			int mapout_aCD_output;
			int mapout_aAB_output;
			int mapout_aSUM_output;
			int mapout_aCD_dotproduct;
			int mapout_aAB_dotproduct;
			int mapout_a_muxsum;
			int mapout_a_bias;
			int mapout_a_scale;
			int mapout_rgbCD_output;
			int mapout_rgbAB_output;
			int mapout_rgbSUM_output;
			int mapout_rgbCD_dotproduct;
			int mapout_rgbAB_dotproduct;
			int mapout_rgb_muxsum;
			int mapout_rgb_bias;
			int mapout_rgb_scale;
		} stage[8];
		struct {
			float register_constantcolor0[4];
			float register_constantcolor1[4];
			int color_sum_clamp;
			int mapin_rgbA_input;
			int mapin_rgbA_component;
			int mapin_rgbA_mapping;
			int mapin_rgbB_input;
			int mapin_rgbB_component;
			int mapin_rgbB_mapping;
			int mapin_rgbC_input;
			int mapin_rgbC_component;
			int mapin_rgbC_mapping;
			int mapin_rgbD_input;
			int mapin_rgbD_component;
			int mapin_rgbD_mapping;
			int mapin_rgbE_input;
			int mapin_rgbE_component;
			int mapin_rgbE_mapping;
			int mapin_rgbF_input;
			int mapin_rgbF_component;
			int mapin_rgbF_mapping;
			int mapin_aG_input;
			int mapin_aG_component;
			int mapin_aG_mapping;
		} final;
		int stages;
		int used;
		osd_lock *lock;
	} combiner;
	UINT32 color_mask;
	bool alpha_test_enabled;
	NV2A_COMPARISON_OP alpha_func;
	int alpha_reference;
	bool depth_test_enabled;
	NV2A_COMPARISON_OP depth_function;
	bool depth_write_enabled;
	bool stencil_test_enabled;
	NV2A_COMPARISON_OP stencil_func;
	int stencil_ref;
	int stencil_mask;
	NV2A_STENCIL_OP stencil_op_fail;
	NV2A_STENCIL_OP stencil_op_zfail;
	NV2A_STENCIL_OP stencil_op_zpass;
	bool blending_enabled;
	NV2A_BLEND_EQUATION blend_equation;
	NV2A_BLEND_FACTOR blend_function_source;
	NV2A_BLEND_FACTOR blend_function_destination;
	UINT32 blend_color;
	bool logical_operation_enabled;
	NV2A_LOGIC_OP logical_operation;
	struct {
		float modelview[16];
		float modelview_inverse[16];
		float projection[16];
		float translate[4];
		float scale[4];
	} matrix;
	struct {
		vertex_program_simulator exec;
		int instructions;
		int upload_instruction_index;
		int upload_instruction_component;
		int start_instruction;
		int upload_parameter_index;
		int upload_parameter_component;
	} vertexprogram;
	int vertex_pipeline;
	int enabled_vertex_attributes;
	int vertex_attribute_words[16];
	int vertex_attribute_offset[16];
	emu_timer *puller_timer;
	int puller_channel;
	int puller_waiting;
	address_space *puller_space;
	UINT32 dilated0[16][2048];
	UINT32 dilated1[16][2048];
	int dilatechose[256];
	nvidia_object_data *objectdata;
	int debug_grab_texttype;
	char *debug_grab_textfile;
	int waitvblank_used;
};
