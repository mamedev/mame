#define GLM_ENABLE_EXPERIMENTAL
#include <glm/exponential.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/integer.hpp>
#include <cstdio>
/*
int test_floor_log2()
{
	int Error = 0;

	for(std::size_t i = 1; i < 1000000; ++i)
	{
		glm::uint A = glm::floor_log2(glm::uint(i));
		glm::uint B = glm::uint(glm::floor(glm::log2(double(i)))); // Will fail with float, lack of accuracy

		Error += A == B ? 0 : 1;
		assert(!Error);
	}

	return Error;
}
*/
int test_log2()
{
	int Error = 0;

	for(std::size_t i = 1; i < 24; ++i)
	{
		glm::uint A = glm::log2(glm::uint(1 << i));
		glm::uint B = glm::uint(glm::log2(double(1 << i)));

		//Error += glm::equalEpsilon(double(A), B, 1.0) ? 0 : 1;
		Error += glm::abs(double(A) - B) <= 24 ? 0 : 1;
		assert(!Error);

		printf("Log2(%d) error A=%d, B=%d\n", 1 << i, A, B);
	}

	printf("log2 error=%d\n", Error);

	return Error;
}

int test_nlz()
{
	int Error = 0;

	for(glm::uint i = 1; i < glm::uint(33); ++i)
		Error += glm::nlz(i) == glm::uint(31u) - glm::findMSB(i) ? 0 : 1;
		//printf("%d, %d\n", glm::nlz(i), 31u - glm::findMSB(i));

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_nlz();
//	Error += test_floor_log2();
	Error += test_log2();

	return Error;
}

