#pragma once

#include <cstdarg>
#include <cstdio>

namespace log
{

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
void fatalf(const char *__restrict__ fmt, ...);

#define assert(cond)                          \
    if (!(cond)) {                            \
        log::fatalf("Assert Failed: " #cond); \
    }

void print_stacktrace();
void enable_stacktrace();

} // namespace log