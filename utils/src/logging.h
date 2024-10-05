enum LOG_LEVEL_ENUM {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_INFO  = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
};

void start_logging(LOG_LEVEL_ENUM max_level = LOG_LEVEL_TRACE);
void log_frame_start(size_t frame_number);
void stop_logging();

void log_message_str(LOG_LEVEL_ENUM level, char* fmt, ...);

#define log_fatal(fmt, ...) log_message_str(LOG_LEVEL_FATAL, fmt, __VA_ARGS__)
#define log_error(fmt, ...) log_message_str(LOG_LEVEL_ERROR, fmt, __VA_ARGS__)
#define log_warn(fmt, ...)  log_message_str(LOG_LEVEL_WARN,  fmt, __VA_ARGS__)
#define log_info(fmt, ...)  log_message_str(LOG_LEVEL_INFO,  fmt, __VA_ARGS__)
#define log_debug(fmt, ...) log_message_str(LOG_LEVEL_DEBUG, fmt, __VA_ARGS__)
#define log_trace(fmt, ...) log_message_str(LOG_LEVEL_TRACE, fmt, __VA_ARGS__)