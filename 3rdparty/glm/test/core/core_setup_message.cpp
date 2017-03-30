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
		case GLM_COMPILER_VC10:
			std::printf("Visual C++ 10 - 2010\n");
			break;
		case GLM_COMPILER_VC11:
			std::printf("Visual C++ 11 - 2012\n");
			break;
		case GLM_COMPILER_VC12:
			std::printf("Visual C++ 12 - 2013\n");
			break;
		case GLM_COMPILER_VC14:
			std::printf("Visual C++ 14 - 2015\n");
			break;
		case GLM_COMPILER_VC15:
			std::printf("Visual C++ 15 - 201X\n");
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
			std::printf("GCC 4.4\n");
			break;
		case GLM_COMPILER_GCC45:
			std::printf("GCC 4.5\n");
			break;
		case GLM_COMPILER_GCC46:
			std::printf("GCC 4.6\n");
			break;
		case GLM_COMPILER_GCC47:
			std::printf("GCC 4.7\n");
			break;
		case GLM_COMPILER_GCC48:
			std::printf("GCC 4.8\n");
			break;
		case GLM_COMPILER_GCC49:
			std::printf("GCC 4.9\n");
			break;
		case GLM_COMPILER_GCC50:
			std::printf("GCC 5.0\n");
			break;
		case GLM_COMPILER_GCC51:
			std::printf("GCC 5.1\n");
			break;
		case GLM_COMPILER_GCC52:
			std::printf("GCC 5.2\n");
			break;
		case GLM_COMPILER_GCC53:
			std::printf("GCC 5.3\n");
			break;
		case GLM_COMPILER_GCC54:
			std::printf("GCC 5.4\n");
			break;
		case GLM_COMPILER_GCC60:
			std::printf("GCC 6.0\n");
			break;
		case GLM_COMPILER_GCC61:
			std::printf("GCC 6.1\n");
			break;
		case GLM_COMPILER_GCC62:
			std::printf("GCC 6.2\n");
			break;
		case GLM_COMPILER_GCC70:
			std::printf("GCC 7.0\n");
			break;
		case GLM_COMPILER_GCC71:
			std::printf("GCC 7.1\n");
			break;
		case GLM_COMPILER_GCC72:
			std::printf("GCC 7.2\n");
			break;
		case GLM_COMPILER_GCC80:
			std::printf("GCC 8.0\n");
			break;
		default:
			std::printf("GCC version not detected\n");
			Error += 1;
			break;
		}
	}
	else if(GLM_COMPILER & GLM_COMPILER_CUDA)
	{
		std::printf("CUDA\n");
	}
	else if(GLM_COMPILER & GLM_COMPILER_CLANG)
	{
		switch(GLM_COMPILER)
		{
		case GLM_COMPILER_CLANG32:
			std::printf("Clang 3.2\n");
			break;
		case GLM_COMPILER_CLANG33:
			std::printf("Clang 3.3\n");
			break;
		case GLM_COMPILER_CLANG34:
			std::printf("Clang 3.4\n");
			break;
		case GLM_COMPILER_CLANG35:
			std::printf("Clang 3.5\n");
			break;
		case GLM_COMPILER_CLANG36:
			std::printf("Clang 3.6\n");
			break;
		case GLM_COMPILER_CLANG37:
			std::printf("Clang 3.7\n");
			break;
		case GLM_COMPILER_CLANG38:
			std::printf("Clang 3.8\n");
			break;
		case GLM_COMPILER_CLANG39:
			std::printf("Clang 3.9\n");
			break;
		case GLM_COMPILER_CLANG40:
			std::printf("Clang 4.0\n");
			break;
		case GLM_COMPILER_CLANG41:
			std::printf("Clang 4.1\n");
			break;
		case GLM_COMPILER_CLANG42:
			std::printf("Clang 4.2\n");
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
			std::printf("ICC 12\n");
			break;
		case GLM_COMPILER_INTEL12_1:
			std::printf("ICC 12.1\n");
			break;
		case GLM_COMPILER_INTEL13:
			std::printf("ICC 13\n");
			break;
		case GLM_COMPILER_INTEL14:
			std::printf("ICC 14\n");
			break;
		case GLM_COMPILER_INTEL15:
			std::printf("ICC 15\n");
			break;
		case GLM_COMPILER_INTEL16:
			std::printf("ICC 16\n");
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
	std::printf("__cplusplus: %d\n", static_cast<int>(__cplusplus));
	
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
