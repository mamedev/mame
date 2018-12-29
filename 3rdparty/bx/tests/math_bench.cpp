/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
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

void math_bench()
{
	bx::WriterI* writer = bx::getStdOut();
	bx::Error err;
	bx::write(writer, &err, "Math bench\n\n");

	mathTest<  ::sqrtf    >("  ::sqrtf");
	mathTest<bx::sqrtRef  >("bx::sqrtRef");
	mathTest<bx::sqrtSimd >("bx::sqrtSimd");
	mathTest<bx::sqrt     >("bx::sqrt");

	bx::write(writer, &err, "\n");
	mathTest<  ::rsqrt    >("  ::rsqrtf");
	mathTest<bx::rsqrtRef >("bx::rsqrtRef");
	mathTest<bx::rsqrtSimd>("bx::rsqrtSimd");
	mathTest<bx::rsqrt    >("bx::rsqrt");

	bx::write(writer, &err, "\n");
	mathTest<  ::sinf >("  ::sinf");
	mathTest<bx::sin  >("bx::sin");

	bx::write(writer, &err, "\n");
	mathTest<  ::asinf>("  ::asinf");
	mathTest<bx::asin >("bx::asin");

	bx::write(writer, &err, "\n");
	mathTest<  ::cosf >("  ::cosf");
	mathTest<bx::cos  >("bx::cos");

	bx::write(writer, &err, "\n");
	mathTest<  ::acosf>("  ::acosf");
	mathTest<bx::acos >("bx::acos");

	bx::write(writer, &err, "\n");
	mathTest<  ::tanf >("  ::tanf");
	mathTest<bx::tan  >("bx::tan");

	bx::write(writer, &err, "\n");
	mathTest<  ::atanf>("  ::atanf");
	mathTest<bx::atan >("bx::atan");
}
