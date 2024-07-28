#include "cositas.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define LOG_ENUM2_SIZE 5

t_log2 *logger2;

static char *enum_names2[LOG_ENUM2_SIZE] = {"MATI", "NICO", "LEO", "FACU", "FEDE"};
static char *log_colors2[LOG_ENUM2_SIZE] = {
    "\x1B[38;2;222;95;44m", 
    "\x1B[38;2;222;98;255m", 
    "\x1B[38;2;200;25;73m",     
    "\x1B[38;2;44;145;222m", 
    "\x1B[38;2;3;252;115m"
};
static char *reset_color2 = "\x1b[0m";

static void _log_write_in_level2(t_log2* logger, t_log_level2 level, const char* message_template, va_list arguments);
static bool _isEnableLevelInLogger2(t_log2* logger, t_log_level2 level);

#define log_impl_template2(log_function, level_enum)                                \
        void log_function(t_log2* logger, const char* message_template, ...) {     \
            va_list arguments;                                                     \
            va_start(arguments, message_template);                                 \
            _log_write_in_level2(logger, level_enum, message_template, arguments); \
            va_end(arguments);                                                     \
        }

t_log2* log_create2(char* file, char *program_name, bool is_active_console, t_log_level2 detail) {
    t_log2* logger = malloc(sizeof(t_log2));

    if (logger == NULL) {
        perror("Cannot create logger");
        return NULL;
    }

    FILE *file_opened = NULL;

    if (file != NULL) {
        file_opened = fopen(file, "a");

        if (file_opened == NULL) {
            perror("Cannot create/open log file");
            free(logger);
            return NULL;
        }
    }

    logger->file = file_opened;
    logger->is_active_console = is_active_console;
    logger->detail = detail;
    logger->pid = getpid();
    logger->program_name = strdup(program_name);
    return logger;
}

void log_destroy2(t_log2* logger) {
    free(logger->program_name);
    if (logger->file != NULL) {
        fclose(logger->file);
    }
    free(logger);
}

log_impl_template2(log_mati, LOG_LEVEL_MATI);
log_impl_template2(log_nico, LOG_LEVEL_NICO);
log_impl_template2(log_leo, LOG_LEVEL_LEO);
log_impl_template2(log_facu, LOG_LEVEL_FACU);
log_impl_template2(log_fede, LOG_LEVEL_FEDE);

static void _log_write_in_level2(t_log2* logger, t_log_level2 level, const char* message_template, va_list list_arguments) {
    if (_isEnableLevelInLogger2(logger2, level)) {
        char *message, *time_str, *buffer, *console_buffer;
        unsigned int thread_id;

        // Format message
        vasprintf(&message, message_template, list_arguments);
        
        // Get current time
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        asprintf(&time_str, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);

        // Get thread ID
        thread_id = pthread_self();

        // Format log entry
        // asprintf(&buffer, "[%s] %s %s/(%d:%d): %s\n",
        //     enum_names2[level],
        //     time_str,
        //     logger2->program_name,
        //     logger2->pid,
        //     thread_id,
        //     message);       

        // // Write to file if it's open
        // if (logger2->file != NULL) {
        //     fputs(buffer, logger->file);
        // }

        asprintf(&buffer, "[%s] %s %s: %s\n",
            enum_names2[level],
            time_str,
            logger2->program_name,
            message);       

        // Write to file if it's open
        if (logger2->file != NULL) {
            fputs(buffer, logger->file);
        }

        // Write to console if enabled
        if (logger2->is_active_console) {
            asprintf(&console_buffer, "%s%s%s",
                log_colors2[level],
                buffer,
                reset_color2);
            fputs(console_buffer, stdout);
            free(console_buffer);
        }

        // Clean up
        free(time_str);
        free(message);
        free(buffer);
    }
}

static bool _isEnableLevelInLogger2(t_log2* logger, t_log_level2 level) {
    return level >= logger2->detail;
}