#define GLM_FORCE_SWIZZLE
#include <glm/vec2.hpp>

struct Foo
{
	static glm::vec2 Bar;
};

glm::vec2 Foo::Bar = glm::vec2(1.f, 1.f);

int main()
{
	return 0;
}
