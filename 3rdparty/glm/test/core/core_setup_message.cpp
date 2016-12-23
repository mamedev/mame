#define GLM_FORCE_MESSAGES
#include <glm/vec3.hpp>
#include <cstdio>

int test_compiler()
{
	int Error(0);
	
	if(GLM_COMPILER & GLM_COMPILER_VC)
	{
		switch(GLM_COMPILER)
		{
		case GLM_COMPILER_VC2010:
			std::printf("GLM_COMPILER_VC2010\n");
			break;
		case GLM_COMPILER_VC2012:
			std::printf("GLM_COMPILER_VC2012\n");
			break;
		case GLM_COMPILER_VC2013:
			std::printf("GLM_COMPILER_VC2013\n");
			break;
		case GLM_COMPILER_VC2015:
			std::printf("GLM_COMPILER_VC2015\n");
			break;
		default:
			std::printf("Visual C++ version not detected\n");
			Error += 1;
			break;
		}
	}
	else if(GLM_COMPILER & GLM_COMPILER_GCC)
	{
		switch(GLM_COMPILER)
		{
		case GLM_COMPILER_GCC44:
			std::printf("GLM_COMPILER_GCC44\n");
			break;
		case GLM_COMPILER_GCC45:
			std::printf("GLM_COMPILER_GCC45\n");
			break;
		case GLM_COMPILER_GCC46:
			std::printf("GLM_COMPILER_GCC46\n");
			break;
		case GLM_COMPILER_GCC47:
			std::printf("GLM_COMPILER_GCC47\n");
			break;
		case GLM_COMPILER_GCC48:
			std::printf("GLM_COMPILER_GCC48\n");
			break;
		case GLM_COMPILER_GCC49:
			std::printf("GLM_COMPILER_GCC49\n");
			break;
		case GLM_COMPILER_GCC50:
			std::printf("GLM_COMPILER_GCC50\n");
			break;
		case GLM_COMPILER_GCC51:
			std::printf("GLM_COMPILER_GCC51\n");
			break;
		case GLM_COMPILER_GCC52:
			std::printf("GLM_COMPILER_GCC52\n");
			break;
		case GLM_COMPILER_GCC53:
			std::printf("GLM_COMPILER_GCC53\n");
			break;
		case GLM_COMPILER_GCC54:
			std::printf("GLM_COMPILER_GCC54\n");
			break;
		case GLM_COMPILER_GCC60:
			std::printf("GLM_COMPILER_GCC60\n");
			break;
		case GLM_COMPILER_GCC61:
			std::printf("GLM_COMPILER_GCC61\n");
			break;
		case GLM_COMPILER_GCC62:
			std::printf("GLM_COMPILER_GCC62\n");
			break;
		case GLM_COMPILER_GCC70:
			std::printf("GLM_COMPILER_GCC70\n");
			break;
		case GLM_COMPILER_GCC71:
			std::printf("GLM_COMPILER_GCC71\n");
			break;
		case GLM_COMPILER_GCC72:
			std::printf("GLM_COMPILER_GCC72\n");
			break;
		case GLM_COMPILER_GCC80:
			std::printf("GLM_COMPILER_GCC80\n");
			break;
		default:
			std::printf("GCC version not detected\n");
			Error += 1;
			break;
		}
	}
	else if(GLM_COMPILER & GLM_COMPILER_CUDA)
	{
		std::printf("GLM_COMPILER_CUDA\n");
	}
	else if(GLM_COMPILER & GLM_COMPILER_CLANG)
	{
		switch(GLM_COMPILER)
		{
		case GLM_COMPILER_CLANG32:
			std::printf("GLM_COMPILER_CLANG32\n");
			break;
		case GLM_COMPILER_CLANG33:
			std::printf("GLM_COMPILER_CLANG33\n");
			break;
		case GLM_COMPILER_CLANG34:
			std::printf("GLM_COMPILER_CLANG34\n");
			break;
		case GLM_COMPILER_CLANG35:
			std::printf("GLM_COMPILER_CLANG35\n");
			break;
		case GLM_COMPILER_CLANG36:
			std::printf("GLM_COMPILER_CLANG36\n");
			break;
		case GLM_COMPILER_CLANG37:
			std::printf("GLM_COMPILER_CLANG37\n");
			break;
		case GLM_COMPILER_CLANG38:
			std::printf("GLM_COMPILER_CLANG38\n");
			break;
		case GLM_COMPILER_CLANG39:
			std::printf("GLM_COMPILER_CLANG39\n");
			break;
		default:
			std::printf("LLVM version not detected\n");
			break;
		}
	}
	else if(GLM_COMPILER & GLM_COMPILER_INTEL)
	{
		switch(GLM_COMPILER)
		{
		case GLM_COMPILER_INTEL12:
			std::printf("GLM_COMPILER_INTEL12\n");
			break;
		case GLM_COMPILER_INTEL12_1:
			std::printf("GLM_COMPILER_INTEL12_1\n");
			break;
		case GLM_COMPILER_INTEL13:
			std::printf("GLM_COMPILER_INTEL13\n");
			break;
		case GLM_COMPILER_INTEL14:
			std::printf("GLM_COMPILER_INTEL14\n");
			break;
		case GLM_COMPILER_INTEL15:
			std::printf("GLM_COMPILER_INTEL15\n");
			break;
		case GLM_COMPILER_INTEL16:
			std::printf("GLM_COMPILER_INTEL16\n");
			break;
		default:
			std::printf("Intel compiler version not detected\n");
			Error += 1;
			break;
		}
	}
	else
	{
		std::printf("Undetected compiler\n");
		Error += 1;
	}
	
	return Error;
}

int test_model()
{
	int Error = 0;
	
	Error += ((sizeof(void*) == 4) && (GLM_MODEL == GLM_MODEL_32)) || ((sizeof(void*) == 8) && (GLM_MODEL == GLM_MODEL_64)) ? 0 : 1;
	
	if(GLM_MODEL == GLM_MODEL_32)
		std::printf("GLM_MODEL_32\n");
	else if(GLM_MODEL == GLM_MODEL_64)
		std::printf("GLM_MODEL_64\n");
	
	return Error;
}

int test_instruction_set()
{
	int Error = 0;

	std::printf("GLM_ARCH: ");

	if(GLM_ARCH == GLM_ARCH_PURE)
		std::printf("GLM_ARCH_PURE ");
	if(GLM_ARCH & GLM_ARCH_ARM_BIT)
		std::printf("ARM ");
	if(GLM_ARCH & GLM_ARCH_NEON_BIT)
		std::printf("NEON ");
	if(GLM_ARCH & GLM_ARCH_AVX2)
		std::printf("AVX2 ");
	if(GLM_ARCH & GLM_ARCH_AVX)
		std::printf("AVX ");
	if(GLM_ARCH & GLM_ARCH_SSE42_BIT)
		std::printf("SSE4.2 ");
	if(GLM_ARCH & GLM_ARCH_SSE41_BIT)
		std::printf("SSE4.1 ");
	if(GLM_ARCH & GLM_ARCH_SSSE3_BIT)
		std::printf("SSSE3 ");
	if(GLM_ARCH & GLM_ARCH_SSE3_BIT)
		std::printf("SSE3 ");
	if(GLM_ARCH & GLM_ARCH_SSE2_BIT)
		std::printf("SSE2 ");

	std::printf("\n");

	return Error;
}

int test_cpp_version()
{
	std::printf("__cplusplus: %lld\n", __cplusplus);
	
	return 0;
}

int test_operators()
{
	glm::vec3 A(1.0f);
	glm::vec3 B(1.0f);
	bool R = A != B;
	bool S = A == B;

	return (S && !R) ? 0 : 1;
}

template <typename T>
struct vec
{

};

template <template <typename> class C, typename T>
struct Class
{

};

template <typename T>
struct Class<vec, T>
{

};

int main()
{
	//Class<vec, float> C;

	int Error = 0;

	Error += test_cpp_version();
	Error += test_compiler();
	Error += test_model();
	Error += test_instruction_set();
	Error += test_operators();
	
	return Error;
}
