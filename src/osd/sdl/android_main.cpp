#ifdef __ANDROID__

extern "C" int SDL_main(int argc, char *argv[]);

// Using this in main library to prevent linker removing SDL_main
int dummy_main(int argc, char** argv)
{
	return SDL_main(argc, argv);
}

#endif /* __ANDROID__ */
