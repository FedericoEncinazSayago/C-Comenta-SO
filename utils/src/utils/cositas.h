#ifndef COSITAS_H_
#define COSITAS_H_

    #include <stdio.h>
    #include <stdbool.h>
    #include <sys/types.h>

    typedef enum {
        LOG_LEVEL_MATI,
        LOG_LEVEL_NICO,
        LOG_LEVEL_LEO,
        LOG_LEVEL_FACU,
        LOG_LEVEL_FEDE
    } t_log_level2;

    typedef struct {
        FILE* file;
        bool is_active_console;
        t_log_level2 detail;
        char *program_name;
        pid_t pid;
    } t_log2;

    extern t_log2 *logger2;

    t_log2* log_create2(char* file, char *process_name, bool is_active_console, t_log_level2 level);
    void log_destroy2(t_log2* logger);

    void log_mati(t_log2* logger, const char* message, ...) __attribute__((format(printf, 2, 3)));
    void log_nico(t_log2* logger, const char* message, ...) __attribute__((format(printf, 2, 3)));
    void log_leo(t_log2* logger, const char* message, ...) __attribute__((format(printf, 2, 3)));
    void log_facu(t_log2* logger, const char* message, ...) __attribute__((format(printf, 2, 3)));
    void log_fede(t_log2* logger, const char* message, ...) __attribute__((format(printf, 2, 3)));

#endif /* COSITAS_H_ */