#include "param_eq.hpp"
#include "../graph.hpp"
#include <components.hpp>
#include <ui/panel_manager.hpp>

ParamEq::ParamEq() {
    this->ui_panel_index = UI::load_panel("assets/layout/param_eq.toml");
    UI::Panel& panel = UI::get_panel(this->ui_panel_index);

    params.push_back((Param){63, 1, (float)panel.scene.value_pool.get<double>("gain_low"), (float)Graph::get_sample_rate(), identity()});
    params.push_back((Param){250, 1, (float)panel.scene.value_pool.get<double>("gain_low_mid"), (float)Graph::get_sample_rate(), identity()});
    params.push_back((Param){1000, 1, (float)panel.scene.value_pool.get<double>("gain_mid"), (float)Graph::get_sample_rate(), identity()});
    params.push_back((Param){4000, 1, (float)panel.scene.value_pool.get<double>("gain_high_mid"), (float)Graph::get_sample_rate(), identity()});
    params.push_back((Param){16000, 1, (float)panel.scene.value_pool.get<double>("gain_high"), (float)Graph::get_sample_rate(), identity()});
    update_biquads();
}

void ParamEq::update_biquads() {
    params[0].biquad[0] = params[0].biquad[1] = low_shelf(params[0].f0, params[0].q, params[0].gain, params[0].sample_rate);
    params[1].biquad[0] = params[1].biquad[1] = peak(params[1].f0, params[1].q, params[1].gain, params[1].sample_rate);
    params[2].biquad[0] = params[2].biquad[1] = peak(params[2].f0, params[2].q, params[2].gain, params[2].sample_rate);
    params[3].biquad[0] = params[3].biquad[1] = peak(params[3].f0, params[3].q, params[3].gain, params[3].sample_rate);
    params[4].biquad[0] = params[4].biquad[1] = high_shelf(params[4].f0, params[4].q, params[4].gain, params[4].sample_rate);
}

void ParamEq::audio_process_block(const size_t n_samples, const double output_sample_rate, const Sample* input, Sample* output, std::vector<std::shared_ptr<Processor>> targets) {
    if (this->ui_panel_index == -1) return;
    if (!input) return;
    if (!output) return;

    UI::Panel& panel = UI::get_panel(this->ui_panel_index);
    bool recalc = false;

    const char* gain_names[] = { "gain_low", "gain_low_mid", "gain_mid", "gain_high_mid", "gain_high" };
    const char* q_names[] = { "q_low", "q_low_mid", "q_mid", "q_high_mid", "q_high" };

    for (size_t i = 0; i < 5; ++i) {
        if (params[i].gain != panel.scene.value_pool.get<double>(gain_names[i])) {
            params[i].gain = panel.scene.value_pool.get<double>(gain_names[i]);
            recalc = true;
        }
        if (params[i].q != panel.scene.value_pool.get<double>(q_names[i])) {
            params[i].q = panel.scene.value_pool.get<double>(q_names[i]);
            recalc = true;
        }
    }

    if (recalc) {
        update_biquads();
    }

    for (size_t i = 0; i < n_samples; ++i) {
        Sample sample = input[i];

        for (Param& param : this->params) {
            sample.left = param.biquad[0].tick(sample.left);
            sample.right = param.biquad[1].tick(sample.right);
        }

        output[i] = sample;
    }
}

float Biquad::tick(float x0) {
    // biquad direct form 1
    double inv_a0 = 1.0 / a0;
    double y0 = (b0 * inv_a0) * x0
             + (b1 * inv_a0) * x1
             + (b2 * inv_a0) * x2
             - (a1 * inv_a0) * y1
             - (a2 * inv_a0) * y2;

    // update history
    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;

    if (fabs(x1) < 1e-20) x1 = 0;
    if (fabs(x2) < 1e-20) x2 = 0;
    if (fabs(y1) < 1e-20) y1 = 0;
    if (fabs(y2) < 1e-20) y2 = 0;

    return y0;
}

Biquad identity() {
    return (Biquad) {
        .b0 = 1.0,
        .b1 = 0.0,
        .b2 = 0.0,
        .a0 = 1.0,
        .a1 = 0.0,
        .a2 = 0.0,
    };
}

Biquad low_pass(float f0, float q, float sample_rate) {
    const double w0_base = 2.0 * 3.141592653589789 / (double)sample_rate;
    const double w0 = w0_base * f0;
    const double alpha = sin(w0) / (2.0 * q);
    return (Biquad) {
        .b0 =  (1.0 - cos(w0))/2,
        .b1 =   1.0 - cos(w0),
        .b2 =  (1.0 - cos(w0))/2,
        .a0 =   1.0 + alpha,
        .a1 =  -2.0 * cos(w0),
        .a2 =   1.0 - alpha,
    };
}

Biquad low_shelf(float f0, float q, float gain, float sample_rate) {
    const double a = sqrt(pow(10.0, gain/40.0));
    const double w0_base = 2.0 * 3.141592653589789 / (double)sample_rate;
    const double w0 = w0_base * f0;
    const double alpha = sin(w0) / (2.0 * q);
    return (Biquad) {
        .b0 =   a* ((a+1) - (a-1)*cos(w0) + 2*sqrt(a)*alpha),
        .b1 = 2*a* ((a-1) - (a+1)*cos(w0)                  ),
        .b2 =   a* ((a+1) - (a-1)*cos(w0) - 2*sqrt(a)*alpha),
        .a0 =       (a+1) + (a-1)*cos(w0) + 2*sqrt(a)*alpha,
        .a1 =   -2*((a-1) + (a+1)*cos(w0)                  ),
        .a2 =       (a+1) + (a-1)*cos(w0) - 2*sqrt(a)*alpha,
    };
}

Biquad peak(float f0, float q, float gain, float sample_rate) {
    const double a = sqrt(pow(10.0, gain/40.0));
    const double w0_base = 2.0 * 3.141592653589789 / (double)sample_rate;
    const double w0 = w0_base * f0;
    const double alpha = sin(w0) / (2.0 * q);
    return (Biquad) {
        .b0 =   1.0 + (alpha*a),
        .b1 =  -2.0 * cos(w0),
        .b2 =   1.0 - (alpha*a),
        .a0 =   1.0 + (alpha/a),
        .a1 =  -2.0 * cos(w0),
        .a2 =   1.0 - (alpha/a),
    };
}

Biquad high_shelf(float f0, float q, float gain, float sample_rate) {
    const double a = sqrt(pow(10.0, gain/40.0));
    const double w0_base = 2.0 * 3.141592653589789 / (double)sample_rate;
    const double w0 = w0_base * f0;
    const double alpha = sin(w0) / (2.0 * q);
    return (Biquad) {
        .b0 =    a*( (a+1) + (a-1)*cos(w0) + 2*sqrt(a)*alpha ),
        .b1 = -2*a*( (a-1) + (a+1)*cos(w0)                   ),
        .b2 =    a*( (a+1) + (a-1)*cos(w0) - 2*sqrt(a)*alpha ),
        .a0 =        (a+1) - (a-1)*cos(w0) + 2*sqrt(a)*alpha,
        .a1 =    2*( (a-1) - (a+1)*cos(w0)                   ),
        .a2 =        (a+1) - (a-1)*cos(w0) - 2*sqrt(a)*alpha,
    };
}

Biquad high_pass(float f0, float q, float sample_rate) {
    const double w0_base = 2.0 * 3.141592653589789 / (double)sample_rate;
    const double w0 = w0_base * f0;
    const double alpha = sin(w0) / (2.0 * q);
    return (Biquad) {
        .b0 =  (1.0 + cos(w0))/2,
        .b1 = -(1.0 + cos(w0)),
        .b2 =  (1.0 + cos(w0))/2,
        .a0 =   1.0 + alpha,
        .a1 =  -2.0 * cos(w0),
        .a2 =   1.0 - alpha,
    };
}
