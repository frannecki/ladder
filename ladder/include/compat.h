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
#elif defined(__unix__) || defined(__APPLE__)
#define LADDER_OS_UNIX
#ifdef __linux__
#define LADDER_OS_LINUX
#define LADDER_HAVE_EPOLL
#elif defined(__FreeBSD__)
#define LADDER_OS_FREEBSD
#define LADDER_HAVE_KQUEUE
#elif defined(__APPLE__)
#define LADDER_OS_MAC
#define LADDER_HAVE_KQUEUE
#endif
#endif

#endif
