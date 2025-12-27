#pragma once

#include <cstdarg>
#include <cstdio>

namespace log
{

constexpr const char *COLOR_ERROR = "\x1b[31m";
constexpr const char *COLOR_WARN  = "\x1b[33m";
constexpr const char *COLOR_INFO  = "\x1b[36m";
constexpr const char *COLOR_DEBUG = "\x1b[35m";
constexpr const char *COLOR_TRACE = "\x1b[90m";
constexpr const char *COLOR_CLEAR = "\x1b[0m";

enum class severity {
    all,

    trace,
    debug,
    info,
    warn,
    error,

    none,
};

void set_file(FILE *f, bool color);
void set_severity(severity s);

void tracef(const char *__restrict__ fmt, ...);
void debugf(const char *__restrict__ fmt, ...);
void infof(const char *__restrict__ fmt, ...);
void warnf(const char *__restrict__ fmt, ...);
void errorf(const char *__restrict__ fmt, ...);

[[noreturn]] void fatalf(const char *__restrict__ fmt, ...);

inline void noimpl()
{
    fatalf("Not Implemented");
}

#define assert(cond)                          \
    if (!(cond)) {                            \
        log::fatalf("Assert Failed: " #cond); \
    }

void print_stacktrace();
void enable_stacktrace();

} // namespace log