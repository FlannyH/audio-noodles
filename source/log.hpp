namespace Log {
    enum class Level {
        Debug = 0,
        Info,
        Warning,
        Error,
        Fatal,
        Disabled
    };

    #ifdef _DEBUG
    constexpr Level min_level = Level::Debug;
    #else
    constexpr Level min_level = Level::Info;
    #endif
    constexpr bool display_time = true;
    constexpr bool display_log_level = true;
    constexpr bool color = true;

    void write(const Level level, const char* message, ...);
}

// Shorthand macro to save us typing
#define LOG(level, ...) Log::write(Log::Level::level, __VA_ARGS__)
