/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include <bx/math.h>
#include <bx/timer.h>
#include <bx/file.h>

#include <math.h>

typedef float (*MathFn)(float);

template<MathFn mfn>
float mathTest(const char* _name)
{
	bx::WriterI* writer = bx::getStdOut();
	int64_t elapsed = -bx::getHPCounter();

	float result = 0.0f;
	const float max = 1389.0f;

	for (float xx = 0.0f; xx < max; xx += 0.1f)
	{
		result += mfn(xx);
	}

	bx::Error err;

	elapsed += bx::getHPCounter();
	bx::write(writer, &err, "%-20s: %15f\n", _name, double(elapsed) );

	return result;
}

float rsqrt(float _a)
{
	return 1.0f/::sqrtf(_a);
}

float sinCosNonApproxBench()
{
	bx::WriterI* writer = bx::getStdOut();
	int64_t elapsed = -bx::getHPCounter();

	float result = 0.0f;
	const float max = 1389.0f;

	for (float xx = 0.0f; xx < max; xx += 0.1f)
	{
		float ss, cc;
		ss = bx::sin(xx);
		cc = bx::cos(xx);

		result += ss + cc;
	}

	bx::Error err;

	elapsed += bx::getHPCounter();
	bx::write(writer, &err, "%-20s: %15f\n", "sin + cos", double(elapsed) );

	return result;
}

float sinCosApproxBench()
{
	bx::WriterI* writer = bx::getStdOut();
	int64_t elapsed = -bx::getHPCounter();

	float result = 0.0f;
	const float max = 1389.0f;

	for (float xx = 0.0f; xx < max; xx += 0.1f)
	{
		float ss, cc;
		bx::sinCosApprox(ss, cc, xx);

		result += ss + cc;
	}

	bx::Error err;

	elapsed += bx::getHPCounter();
	bx::write(writer, &err, "%-20s: %15f\n", "sinCosApprox", double(elapsed) );

	return result;
}

float g_result; // trick compiler to not discard results

void math_bench()
{
	bx::WriterI* writer = bx::getStdOut();
	bx::Error err;
	bx::write(writer, &err, "Math bench\n\n");

	g_result += mathTest<  ::sqrtf    >("  ::sqrtf");
	g_result += mathTest<bx::sqrtRef  >("bx::sqrtRef");
	g_result += mathTest<bx::sqrtSimd >("bx::sqrtSimd");
	g_result += mathTest<bx::sqrt     >("bx::sqrt");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::rsqrt    >("  ::rsqrtf");
	g_result += mathTest<bx::rsqrtRef >("bx::rsqrtRef");
	g_result += mathTest<bx::rsqrtSimd>("bx::rsqrtSimd");
	g_result += mathTest<bx::rsqrt    >("bx::rsqrt");

	bx::write(writer, &err, "\n");
	g_result += sinCosNonApproxBench();
	g_result += sinCosApproxBench();

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::sinf >("  ::sinf");
	g_result += mathTest<bx::sin  >("bx::sin");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::sinhf>("  ::sinhf");
	g_result += mathTest<bx::sinh >("bx::sinh");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::asinf>("  ::asinf");
	g_result += mathTest<bx::asin >("bx::asin");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::cosf >("  ::cosf");
	g_result += mathTest<bx::cos  >("bx::cos");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::coshf>("  ::coshf");
	g_result += mathTest<bx::cosh >("bx::cosh");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::acosf>("  ::acosf");
	g_result += mathTest<bx::acos >("bx::acos");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::tanf >("  ::tanf");
	g_result += mathTest<bx::tan  >("bx::tan");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::tanhf>("  ::tanhf");
	g_result += mathTest<bx::tanh >("bx::tanh");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::atanf>("  ::atanf");
	g_result += mathTest<bx::atan >("bx::atan");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::expf>("  ::expf");
	g_result += mathTest<bx::exp >("bx::exp");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::exp2f>("  ::exp2f");
	g_result += mathTest<bx::exp2 >("bx::exp2");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::logf >("  ::logf");
	g_result += mathTest<bx::log  >("bx::log");

	bx::write(writer, &err, "\n");
	g_result += mathTest<  ::log2f>("  ::log2f");
	g_result += mathTest<bx::log2 >("bx::log2");
}
