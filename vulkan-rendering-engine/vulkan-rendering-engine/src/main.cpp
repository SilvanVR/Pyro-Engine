#include "application.h"

#if defined(NDEBUG) && defined(_WIN32)

    // No Console Window popping up. Directly call the windows main function.
    int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
    {
        Application app(800, 600);
        return 0;
    }

#else

    int main()
    {
        Application app(800, 600);
        return 0;
    }

#endif




