#ifndef UTIL_SYSTEM_H
#define UTIL_SYSTEM_H

#define YF_PLATFORMID_UNKNOWN 0
#define YF_PLATFORMID_UNIX 1
#define YF_PLATFORMID_WINNT 2
#define YF_PLATFORMID_LINUX 3
#define YF_PLATFORMID_APPLE 4
#define YF_PLATFORMID_BSD 5

#if defined(__unix__) || defined(__APPLE_CC__)
#define YF_PLATFORM_UNIX
#define YF_PLATFORM YF_PLATFORMID_UNIX

#if defined(__linux) || defined(linux) || defined(__linux__)
#define YF_SUBPLATFORM YF_PLATFORMID_LINUX
#elif defined(__APPLE__) || defined(macintosh) || defined(__MACH__)
#define YF_SUBPLATFORM YF_PLATFORMID_APPLE
#elif defined(__DragonFly__) || defined(__FreeBSD) || defined(__NETBSD__) || defined(__OpenBSD__)
#define YF_SUBPLATFORM YF_PLATFORMID_BSD
#else
#define YF_SUBPLATFORM YF_PLATFORMID_UNIX
#endif

#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define YF_PLATFORM_WINNT
#define YF_PLATFORM YF_PLATFORMID_WINNT
#define YF_SUBPLATFORM YF_PLATFORMID_WINNT
#else
#define YF_PLATFORM_UNKNOWN
#define YF_PLATFORM YF_PLATFORMID_UNKNOWN
#define YF_SUBPLATFORM YF_PLATFORMID_UNKNOWN
#endif

#if defined(__has_attribute)
#define __yf_has_attribute __has_attribute
#else
#define __yf_has_attribute(name) 0
#endif

#if __yf_has_attribute(warn_unused_result)
#define yf_nodiscard __attribute__((warn_unused_result))
#else
#define yf_nodiscard
#endif

#if __yf_has_attribute(always_inline)
#define yf_always_inline __attribute__((always_inline)) inline
#else
#define yf_always_inline static inline
#endif

#endif /* UTIL_SYSTEM_H */
