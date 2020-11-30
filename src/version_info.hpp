#ifndef VERSION_INFO_HPP
#define VERSION_INFO_HPP

#include <SDL2/SDL.h>

#include <string>

/* BAYFIELD VERSION */
#define BFE_MAJOR 1
#define BFE_MINOR 2
#define BFE_PATCH 1

/* COMPILER VERSION */
/* Why is this not standardized?!? */
// Refer to this: https://sourceforge.net/p/predef/wiki/Compilers/
#if __clang__ // tested
    #define CXX_MAJOR __clang_major__
    #define CXX_MINOR __clang_minor__
    #define CXX_PATCH __clang_patchlevel__
    #define CXX_VERSION std::string(__clang_version__)
    #define CXX_BRAND "LLVM/CLANG"

/* these two are unlikely to be usable due to the abundance of Unix-like-oriented code present but remain for potential future use */
/*  
    #elif _MSC_VER // should work
        #define CXX_MAJOR _MSC_VER
        #define CXX_VERSION std::to_string(_MSC_BUILD)
        #define CXX_BRAND "MSVC"
    #elif __INTEL_COMPILER // untested
        #define CXX_MAJOR (__INTEL_COMPILER / 100)
        #define CXX_MINOR ((__INTEL_COMPILER / 10) % 10)
        #define CXX_PATCH (__INTEL_COMPILER % 10)
        #define CXX_VERSION std::to_string(__INTEL_COMPILER_BUILD_DATE)
        #define CXX_BRAND "INTEL"
*/

#elif __GNUC__ // this may be inaccurate - other compilers define __GNUC__ to mark themselves as GCC-compatible
    #define CXX_MAJOR __GNUC__
    #define CXX_MINOR __GNUC_MINOR__
    #define CXX_PATCH __GNUC_PATCHLEVEL__
    #define CXX_VERSION std::string(__VERSION__)
    #define CXX_BRAND "GCC"
#else
    #define CXX_VERSION "UNKNOWN VERSION"
    #define CXX_BRAND "UNKNOWN COMPILER"
#endif

// compiler version string
std::string VERSION_CXX() {
    std::string ver = "";
#ifdef CXX_BRAND
    ver += std::string(CXX_BRAND);
#endif
#ifdef CXX_MAJOR
    ver += ' ' + std::to_string(CXX_MAJOR);
#ifdef CXX_MINOR
    ver += '.' + std::to_string(CXX_MINOR);
#ifdef CXX_PATCH
    ver += '.' + std::to_string(CXX_PATCH);
#endif
#endif
#endif
#ifdef __cplusplus
    ver += " [STD " + std::to_string(__cplusplus) + "]";
#endif
    return ver;
}

// program version string
std::string VERSION_BFE() {
    return std::string(std::to_string(BFE_MAJOR) + '.' + std::to_string(BFE_MINOR) + '.' + std::to_string(BFE_PATCH));
}

// compiled sdl version string
std::string VERSION_SDL_COMPILED() {
    SDL_version v;
    SDL_VERSION(&v);
    return std::string(std::to_string(v.major) + '.' + std::to_string(v.minor) + '.' + std::to_string(v.patch));
}

// linked sdl version string
std::string VERSION_SDL_LINKED() {
    SDL_version v;
    SDL_GetVersion(&v);
    return std::string(std::to_string(v.major) + '.' + std::to_string(v.minor) + '.' + std::to_string(v.patch));
}

#endif
