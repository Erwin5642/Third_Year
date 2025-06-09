#include "gasStationProtocol.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Simula um erro no arquivo 
char simulateError(){
    return ((float)rand() / RAND_MAX) > 0.5;
}

static const char *type[LOG_LEVEL_LEN] = {
    "INFO",
    "SUCC",
    "WARN",
    "ERR"
};

static const char *colors[LOG_LEVEL_LEN] = {
    "\x1b[0m",      // INFO
    "\x1b[32m",     // SUCCESS
    "\x1b[1;33m",   // WARNING
    "\x1b[31m"      // ERROR
};

static time_t current_time;
static struct tm *m_time;

static void get_time() {
    time(&current_time);
    m_time = localtime(&current_time);
}

// Exibe uma mensagem formatada seguindo padrao de log
void logMessage(LOG_LEVEL level, const char *fmt, ...)
{    
    va_list args;
    va_start(args, fmt);
    get_time();

    printf("%s[%02d/%02d/%d -> %02d:%02d:%02d][%s]",
        colors[level],
        m_time->tm_mday,
        m_time->tm_mon + 1,
        m_time->tm_year + 1900,
        m_time->tm_hour,
        m_time->tm_min,
        m_time->tm_sec,
        type[level]);

    vprintf(fmt, args);
    if (level == LOG_ERROR) {
        perror(NULL);
    
    }
    else{
        printf("\n");
    }
    printf(colors[LOG_INFO]); //Reseta cor
    va_end(args);
}
