///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Mathematics (glm.g-truc.net)
///
/// Copyright (c) 2005 - 2015 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// Restrictions:
///		By making use of the Software for military purposes, you choose to make
///		a Bunny unhappy.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @file test/gtc/gtc_noise.cpp
/// @date 2011-04-21 / 2014-11-25
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <glm/gtc/noise.hpp>
#include <gli/gli.hpp>
#include <gli/gtx/loader.hpp>

int test_simplex()
{
	std::size_t const Size = 256;

	{
		std::vector<glm::byte> ImageData(Size * Size * 3);
		
		for(std::size_t y = 0; y < Size; ++y)
		for(std::size_t x = 0; x < Size; ++x)
		{
			ImageData[(x + y * Size) * 3 + 0] = glm::byte(glm::simplex(glm::vec2(x / 64.f, y / 64.f)) * 128.f + 127.f);
			ImageData[(x + y * Size) * 3 + 1] = ImageData[(x + y * Size) * 3 + 0];
			ImageData[(x + y * Size) * 3 + 2] = ImageData[(x + y * Size) * 3 + 0];
		}

		gli::texture2D Texture(1);
		Texture[0] = gli::image2D(glm::uvec2(Size), gli::RGB8U);
		memcpy(Texture[0].data(), &ImageData[0], ImageData.size());
		gli::saveDDS9(Texture, "texture_simplex2d_256.dds");
	}

	{
		std::vector<glm::byte> ImageData(Size * Size * 3);
		
		for(std::size_t y = 0; y < Size; ++y)
		for(std::size_t x = 0; x < Size; ++x)
		{
			ImageData[(x + y * Size) * 3 + 0] = glm::byte(glm::simplex(glm::vec3(x / 64.f, y / 64.f, 0.5f)) * 128.f + 127.f);
			ImageData[(x + y * Size) * 3 + 1] = ImageData[(x + y * Size) * 3 + 0];
			ImageData[(x + y * Size) * 3 + 2] = ImageData[(x + y * Size) * 3 + 0];
		}

		gli::texture2D Texture(1);
		Texture[0] = gli::image2D(glm::uvec2(Size), gli::RGB8U);
		memcpy(Texture[0].data(), &ImageData[0], ImageData.size());
		gli::saveDDS9(Texture, "texture_simplex3d_256.dds");
	}
	
	{
		std::vector<glm::byte> ImageData(Size * Size * 3);
		
		for(std::size_t y = 0; y < Size; ++y)
		for(std::size_t x = 0; x < Size; ++x)
		{
			ImageData[(x + y * Size) * 3 + 0] = glm::byte(glm::simplex(glm::vec4(x / 64.f, y / 64.f, 0.5f, 0.5f)) * 128.f + 127.f);
			ImageData[(x + y * Size) * 3 + 1] = ImageData[(x + y * Size) * 3 + 0];
			ImageData[(x + y * Size) * 3 + 2] = ImageData[(x + y * Size) * 3 + 0];
		}

		gli::texture2D Texture(1);
		Texture[0] = gli::image2D(glm::uvec2(Size), gli::RGB8U);
		memcpy(Texture[0].data(), &ImageData[0], ImageData.size());
		gli::saveDDS9(Texture, "texture_simplex4d_256.dds");
	}

	return 0;
}

int test_perlin()
{
	std::size_t const Size = 256;

	{
		std::vector<glm::byte> ImageData(Size * Size * 3);
		
		for(std::size_t y = 0; y < Size; ++y)
		for(std::size_t x = 0; x < Size; ++x)
		{
			ImageData[(x + y * Size) * 3 + 0] = glm::byte(glm::perlin(glm::vec2(x / 64.f, y / 64.f)) * 128.f + 127.f);
			ImageData[(x + y * Size) * 3 + 1] = ImageData[(x + y * Size) * 3 + 0];
			ImageData[(x + y * Size) * 3 + 2] = ImageData[(x + y * Size) * 3 + 0];
		}

		gli::texture2D Texture(1);
		Texture[0] = gli::image2D(glm::uvec2(Size), gli::RGB8U);
		memcpy(Texture[0].data(), &ImageData[0], ImageData.size());
		gli::saveDDS9(Texture, "texture_perlin2d_256.dds");
	}

	{
		std::vector<glm::byte> ImageData(Size * Size * 3);
		
		for(std::size_t y = 0; y < Size; ++y)
		for(std::size_t x = 0; x < Size; ++x)
		{
			ImageData[(x + y * Size) * 3 + 0] = glm::byte(glm::perlin(glm::vec3(x / 64.f, y / 64.f, 0.5f)) * 128.f + 127.f);
			ImageData[(x + y * Size) * 3 + 1] = ImageData[(x + y * Size) * 3 + 0];
			ImageData[(x + y * Size) * 3 + 2] = ImageData[(x + y * Size) * 3 + 0];
		}

		gli::texture2D Texture(1);
		Texture[0] = gli::image2D(glm::uvec2(Size), gli::RGB8U);
		memcpy(Texture[0].data(), &ImageData[0], ImageData.size());
		gli::saveDDS9(Texture, "texture_perlin3d_256.dds");
	}
	
	{
		std::vector<glm::byte> ImageData(Size * Size * 3);
		
		for(std::size_t y = 0; y < Size; ++y)
		for(std::size_t x = 0; x < Size; ++x)
		{
			ImageData[(x + y * Size) * 3 + 0] = glm::byte(glm::perlin(glm::vec4(x / 64.f, y / 64.f, 0.5f, 0.5f)) * 128.f + 127.f);
			ImageData[(x + y * Size) * 3 + 1] = ImageData[(x + y * Size) * 3 + 0];
			ImageData[(x + y * Size) * 3 + 2] = ImageData[(x + y * Size) * 3 + 0];
		}

		gli::texture2D Texture(1);
		Texture[0] = gli::image2D(glm::uvec2(Size), gli::RGB8U);
		memcpy(Texture[0].data(), &ImageData[0], ImageData.size());
		gli::saveDDS9(Texture, "texture_perlin4d_256.dds");
	}

	return 0;
}

int test_perlin_pedioric()
{
	std::size_t const Size = 256;

	{
		std::vector<glm::byte> ImageData(Size * Size * 3);
		
		for(std::size_t y = 0; y < Size; ++y)
		for(std::size_t x = 0; x < Size; ++x)
		{
			ImageData[(x + y * Size) * 3 + 0] = glm::byte(glm::perlin(glm::vec2(x / 64.f, y / 64.f), glm::vec2(2.0f)) * 128.f + 127.f);
			ImageData[(x + y * Size) * 3 + 1] = ImageData[(x + y * Size) * 3 + 0];
			ImageData[(x + y * Size) * 3 + 2] = ImageData[(x + y * Size) * 3 + 0];
		}

		gli::texture2D Texture(1);
		Texture[0] = gli::image2D(glm::uvec2(Size), gli::RGB8U);
		memcpy(Texture[0].data(), &ImageData[0], ImageData.size());
		gli::saveDDS9(Texture, "texture_perlin_pedioric_2d_256.dds");
	}

	{
		std::vector<glm::byte> ImageData(Size * Size * 3);
		
		for(std::size_t y = 0; y < Size; ++y)
		for(std::size_t x = 0; x < Size; ++x)
		{
			ImageData[(x + y * Size) * 3 + 0] = glm::byte(glm::perlin(glm::vec3(x / 64.f, y / 64.f, 0.5f), glm::vec3(2.0f)) * 128.f + 127.f);
			ImageData[(x + y * Size) * 3 + 1] = ImageData[(x + y * Size) * 3 + 0];
			ImageData[(x + y * Size) * 3 + 2] = ImageData[(x + y * Size) * 3 + 0];
		}

		gli::texture2D Texture(1);
		Texture[0] = gli::image2D(glm::uvec2(Size), gli::RGB8U);
		memcpy(Texture[0].data(), &ImageData[0], ImageData.size());
		gli::saveDDS9(Texture, "texture_perlin_pedioric_3d_256.dds");
	}
	
	{
		std::vector<glm::byte> ImageData(Size * Size * 3);
		
		for(std::size_t y = 0; y < Size; ++y)
		for(std::size_t x = 0; x < Size; ++x)
		{
			ImageData[(x + y * Size) * 3 + 0] = glm::byte(glm::perlin(glm::vec4(x / 64.f, y / 64.f, 0.5f, 0.5f), glm::vec4(2.0f)) * 128.f + 127.f);
			ImageData[(x + y * Size) * 3 + 1] = ImageData[(x + y * Size) * 3 + 0];
			ImageData[(x + y * Size) * 3 + 2] = ImageData[(x + y * Size) * 3 + 0];
		}

		gli::texture2D Texture(1);
		Texture[0] = gli::image2D(glm::uvec2(Size), gli::RGB8U);
		memcpy(Texture[0].data(), &ImageData[0], ImageData.size());
		gli::saveDDS9(Texture, "texture_perlin_pedioric_4d_256.dds");
	}

	return 0;
}

int main()
{
	int Error = 0;

	Error += test_simplex();
	Error += test_perlin();
	Error += test_perlin_pedioric();

	return Error;
}
