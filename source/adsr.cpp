#include "adsr.hpp"
#include <cmath>
#include <cstdio>

void VolEnv::tick(double delta_time, const VolEnvParams& params) {
    this->stage_time += delta_time;

    if (this->stage == VolEnvStage::delay) {
        this->adsr_volume = 0.0;
        if (this->stage_time >= params.delay) {
            this->stage_time -= params.delay;
            this->stage = VolEnvStage::attack;
        }
    }

    if (this->stage == VolEnvStage::attack) {
        if (params.attack == 0.0) {
            this->stage = VolEnvStage::hold;
        } else {
            this->adsr_volume = std::min(this->stage_time / params.attack, 1.0);
            if (this->stage_time >= params.attack) {
                this->stage_time -= params.attack;
                this->stage = VolEnvStage::hold;
            }
        }
    }

    if (this->stage == VolEnvStage::hold) {
        this->adsr_volume = 1.0;
        if (this->stage_time >= params.hold) {
            this->stage_time -= params.hold;
            this->stage = VolEnvStage::decay;
        }
    }

    if (this->stage == VolEnvStage::decay) {
        if (params.decay == 0) {
            this->stage = VolEnvStage::sustain;
        } else {
            this->adsr_volume = std::lerp(1.0, params.sustain, this->stage_time / params.decay);
            if (this->stage_time >= params.decay) {
                this->stage_time -= params.decay;
                this->stage = VolEnvStage::sustain;
            }
        }
    }

    if (this->stage == VolEnvStage::sustain) {
        this->adsr_volume = params.sustain;
        if (params.sustain == 0.0) {
            this->stage = VolEnvStage::idle;
            this->adsr_volume = 0.0;
        }
    }

    if (this->stage == VolEnvStage::release) {
        if (params.release == 0.0) {
            this->stage = VolEnvStage::idle;
            this->adsr_volume = 0.0;
        } else {
            this->adsr_volume -= params.release * delta_time;
            if (this->adsr_volume <= 0.0) {
                this->stage = VolEnvStage::idle;
                this->adsr_volume = 0.0;
                this->stage_time = 0.0;
            }
        }
    }
}
