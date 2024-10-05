#include "logging.h"

#include <iostream>
#include <chrono>

#include <stdarg.h>

#include <fstream>

using sys_clock = std::chrono::system_clock;

static sys_clock::time_point logging_start_time;
static size_t current_frame_number = 0;

static LOG_LEVEL_ENUM max_log_level = LOG_LEVEL_TRACE;

static const char* log_filename = "log.txt";
static std::ofstream log_file;

static char* LOG_LEVEL_STRINGS[] = {
    "FATAL",
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG",
    "TRACE"
};

static char* LOG_LEVEL_COLOR_PRE[] = {
    "\033[31;43m",     // FATAL
    "\033[31m",       // ERROR
    "\033[33m",       // WARN
    "\033[32m",       // INFO
    "\033[36m",       // DEBUG
    "\033[37m"        // TRACE
};

static char* LOG_LEVEL_COLOR_POST = "\033[0m";

double get_elapsed_time() {
    const sys_clock::time_point now_time = sys_clock::now();

    const std::chrono::duration<double, std::milli> elapsed_ms = now_time - logging_start_time;
    return (elapsed_ms.count() / 1000.0);
}

void start_logging(LOG_LEVEL_ENUM max_level) {
    max_log_level = max_level;

    #ifndef NDEBUG
        const char* compile_mode = " DEBUG ";
    #else
        const char* compile_mode = "RELEASE";
    #endif

    std::cout << "[" << compile_mode << "] Max log_level: " << LOG_LEVEL_STRINGS[max_log_level] << std::endl;

    log_file.open(log_filename);
    log_file << "[" << compile_mode << "] Max log_level: " << LOG_LEVEL_STRINGS[max_log_level] << std::endl;

    logging_start_time = sys_clock::now();
    log_info("Logger initialized.");
}

void log_frame_start(size_t frame_number) {
    current_frame_number = frame_number;
}

void log_message_str(LOG_LEVEL_ENUM level, char* fmt, ...) {
    if (level > max_log_level) return;

    va_list args;
    va_start(args, fmt);

    #define BUFFER_SIZE 2048
    char BUFFER1[BUFFER_SIZE];
    vsnprintf(BUFFER1, BUFFER_SIZE, fmt, args);
    va_end(args);

    char BUFFER2[BUFFER_SIZE];
#if 1
    // color entire message
    snprintf(BUFFER2, BUFFER_SIZE, "%s[%7.3f | %5llu] %s: %s%s", 
             LOG_LEVEL_COLOR_PRE[level], 
             get_elapsed_time(), 
             current_frame_number,
             LOG_LEVEL_STRINGS[level], 
             BUFFER1, 
             LOG_LEVEL_COLOR_POST);
#else
    // start color after timestamp
    snprintf(BUFFER2, BUFFER_SIZE, "[%7.3f | %5llu] %s%s: %s%s", 
             get_elapsed_time(), 
             current_frame_number,
             LOG_LEVEL_COLOR_PRE[level], 
             LOG_LEVEL_STRINGS[level], 
             BUFFER1, 
             LOG_LEVEL_COLOR_POST);
#endif

    std::cout << BUFFER2 << std::endl;

    snprintf(BUFFER2, BUFFER_SIZE, "[%7.3f | %5llu] %s: %s", 
             get_elapsed_time(), 
             current_frame_number,
             LOG_LEVEL_STRINGS[level], 
             BUFFER1);
    log_file << BUFFER2 << std::endl;

    #undef BUFFER_SIZE
}

void stop_logging() {
    log_info("Shutdown logging.");
    log_file.close();
}