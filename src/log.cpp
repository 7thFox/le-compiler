#include "log.hpp"

#include "execinfo.h"
#include "signal.h"

#include <cstdlib>
#include <cxxabi.h>

namespace log
{

static FILE    *_logfile       = stderr;
static bool     _color_enabled = true;
static severity _severity      = severity::info;

#define COLOR_ERROR "\x1b[31m"
#define COLOR_WARN "\x1b[33m"
#define COLOR_INFO "\x1b[36m"
#define COLOR_DEBUG "\x1b[35m"
#define COLOR_TRACE "\x1b[90m"
#define COLOR_CLEAR "\x1b[0m"

#define _logf(l, U, prefix)                      \
    void l##f(const char *__restrict__ fmt, ...) \
    {                                            \
        if (severity::l < _severity)             \
            return;                              \
                                                 \
        if (_color_enabled) {                    \
            fprintf(_logfile, COLOR_##U);        \
        }                                        \
        fprintf(_logfile, prefix);               \
                                                 \
        va_list args;                            \
        va_start(args, fmt);                     \
        vfprintf(_logfile, fmt, args);           \
        va_end(args);                            \
                                                 \
        if (_color_enabled) {                    \
            fprintf(_logfile, COLOR_CLEAR);      \
        }                                        \
        fprintf(_logfile, "\n");                 \
    }

void set_file(FILE *f, bool color)
{
    _logfile       = f;
    _color_enabled = color;
}

void set_severity(severity s)
{
    _severity = s;
}

_logf(trace, TRACE, "[TRACE] ");
_logf(debug, DEBUG, "[DEBUG] ");
_logf(info, INFO, "[INFO]  ");
_logf(warn, WARN, "[WARN]  ");
_logf(error, ERROR, "[ERROR] ");

void fatalf(const char *__restrict__ fmt, ...)
{
    if (severity::error < _severity) {
        std::exit(1);
    }

    if (_color_enabled) {
        fprintf(_logfile, COLOR_ERROR);
    }
    fprintf(_logfile, "[ERROR] ");
    va_list args;
    va_start(args, fmt);
    vfprintf(_logfile, fmt, args);
    va_end(args);
    if (_color_enabled) {
        fprintf(_logfile, COLOR_CLEAR);
    }
    fprintf(_logfile, "\n");

    print_stacktrace();

    std::exit(1);
}

void print_stacktrace()
{
    const int BT_BUFFER_SIZE = 255;
    void     *bt[BT_BUFFER_SIZE];
    int       nbt     = backtrace(bt, BT_BUFFER_SIZE);
    char    **symbols = backtrace_symbols(bt, BT_BUFFER_SIZE);

    fprintf(_logfile, "Stack Trace:\n");
    for (int i = 0; i < nbt; i++) {
        char *mangled = NULL;
        char *offset  = NULL;

        // Typical format: binary(function+0x15c) [addr]
        for (char *p = symbols[i]; *p != 0; ++p) {
            if (*p == '(')
                mangled = p + 1;
            else if (*p == '+')
                offset = p;
        }

        if (mangled != NULL && offset != NULL && mangled < offset) {
            *offset = '\0';

            int   status    = 0;
            char *demangled = abi::__cxa_demangle(mangled, NULL, NULL, &status);

            fprintf(_logfile,
                    "\t%.*s%s+%s\n",
                    static_cast<int>(mangled - symbols[i]),
                    symbols[i],
                    (status == 0 && demangled != NULL) ? demangled : mangled,
                    offset + 1);

            std::free(demangled);
        } else {
            fprintf(_logfile, "\t%s\n", symbols[i]);
        }
    }

    std::free(symbols);
}

void _print_stacktrace(int)
{
    print_stacktrace();
    exit(1);
}

void enable_stacktrace()
{
    signal(SIGILL, _print_stacktrace);
    signal(SIGABRT, _print_stacktrace);
    signal(SIGFPE, _print_stacktrace);
    signal(SIGSEGV, _print_stacktrace);
    signal(SIGTERM, _print_stacktrace);
}

}