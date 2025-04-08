#pragma once
#include <cstdint>

// todo: PascalCase as per naming convention
enum class VolEnvStage {
    idle = 0,
    delay,
    attack,
    hold,
    decay,
    sustain,
    release,
    n_env_stages,
};

struct VolEnvParams {
    double delay   = 0.0;   // Delay stage length in seconds
    double attack  = 0.002; // Attack stage length in seconds
    double hold    = 0.0;   // Hold stage length in seconds
    double decay   = 0.7;   // Decay stage length in seconds
    double sustain = 0.0;   // Sustain volume between 0.0 and 1.0
    double release = 2.0;   // Release stage in volume units per second
};

struct VolEnv {
    double adsr_volume = 0.0;
    double stage_time  = 0.0;               // Time in seconds how long we've been at this envelope stage
    VolEnvStage stage  = VolEnvStage::idle; // What stage we're at now

    void tick(double delta_time, const VolEnvParams& params);
};
