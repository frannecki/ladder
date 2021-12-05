#ifndef LADDER_PORT_H
#define LADDER_PORT_H

#ifdef _MSC_VER
#ifdef _CONSOLE
#define LADDER_API
#elif defined(_WINDLL)
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
#define OS_WINDOWS
#elif defined(__unix__)
#define OS_UNIX
#ifdef __linux__
#define OS_LINUX
#elif defined(__FreeBSD__)
#define OS_FREEBSD
#endif
#endif

#endif
