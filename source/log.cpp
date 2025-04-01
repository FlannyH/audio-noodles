#include "log.hpp"

#include <chrono>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Log {
    std::mutex mutex;

    void write(const Level level, const char* message, ...) {
        if (level < Log::min_level) return;
        if (level == Level::Disabled) return;

        std::lock_guard<std::mutex> lock(mutex);

        char msg_buf[4096] = {0};
        int offset = 0;

        constexpr uint32_t log_level_colors[] = { 
            7,  // debug: light grey
            15, // info: white
            14, // warn: yellow
            4,  // error: red
            12, // fatal: dark red
        };

#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        HANDLE hConsole = NULL;
#endif

        if (Log::color) {
            const uint32_t color_value = log_level_colors[(size_t)level];

            #ifdef _WIN32
            hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            GetConsoleScreenBufferInfo(hConsole, &csbi);
            SetConsoleTextAttribute(hConsole, *(WORD*)(&color_value));
            #else
            constexpr int color_mapping[] = {
                0, 4, 2, 6, 1, 5, 3, 7
            };
            const int code = color_mapping[color_value & 0x07] + 30;
            if (color_value >= 0x08)    offset += snprintf(msg_buf, sizeof(msg_buf) - offset, "\e[1;%im", code);
            else                        offset += snprintf(msg_buf, sizeof(msg_buf) - offset, "\e[%im", code);
            #endif
        }

        if (Log::display_time) {
            time_t now = time(NULL);
            struct tm local_time;
            #if defined(_WIN32)
            localtime_s(&local_time, &now); // windows
            #else 
            localtime_r(&now, &local_time); // linux
            #endif
            offset += snprintf(
                msg_buf + offset, sizeof(msg_buf) - offset,
                "[%02i:%02i:%02i] ",
                local_time.tm_hour,    
                local_time.tm_min,    
                local_time.tm_sec
            );
        }

        if (Log::display_log_level) {
            const char* log_level_names[] = { 
                "[DEBUG] ", 
                "[INFO]  ", 
                "[WARN]  ", 
                "[ERROR] ", 
                "[FATAL] ",
            };
            offset += snprintf(msg_buf + offset, sizeof(msg_buf) - offset, log_level_names[(size_t)level]);
        }

        va_list args;
        va_start(args, message);
        offset += vsnprintf(msg_buf + offset, sizeof(msg_buf) - offset, message, args);
        va_end(args);

        if (level >= Level::Error)  std::cerr << msg_buf << std::endl;
        else                        std::cout << msg_buf << std::endl;
        
        if (Log::color) {
#ifdef _WIN32
            SetConsoleTextAttribute(hConsole, csbi.wAttributes);
#else
            offset += snprintf(msg_buf + offset, sizeof(msg_buf) - offset, "\e[0m");
#endif
        }
    }
}
