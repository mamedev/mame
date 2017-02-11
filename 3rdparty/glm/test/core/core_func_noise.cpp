struct vec4
{
	static int length();
};

int vec4::length()
{
	return 4;
}

int main()
{
	int Failed = 0;

	vec4 V;

	int LengthA = V.length();
	int LengthB = vec4::length();

	return Failed;
}

