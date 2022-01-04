#ifndef LADDER_COMPAT_H
#define LADDER_COMPAT_H

#ifdef _MSC_VER
#if defined(_WINDLL)
#define LADDER_API __declspec(dllexport)
#else
#define LADDER_API __declspec(dllimport)
#endif
//#elif defined(__GNUC__)
//#define LADDER_API__attribute__((visibility("default")))
#else
#define LADDER_API
#endif

#ifdef _MSC_VER
#define LADDER_OS_WINDOWS
#elif defined(__unix__)
#define LADDER_OS_UNIX
#ifdef __linux__
#define LADDER_OS_LINUX
#elif defined(__FreeBSD__)
#define LADDER_OS_FREEBSD
#endif
#endif

#endif