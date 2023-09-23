#include "HC-3.hpp"

namespace pachde
{

Hc3Module::Hc3Module()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configSwitch(SELECTED_PARAM, -1.f, 15.f, -1.f, "Current", {
        "None",
        "1.1", "1.2", "1.3", "1.4", "1.5", "1.6", "1.7", "1.8",
        "2.1", "2.2", "2.3", "2.4", "2.5", "2.6", "2.7", "2.8",
    });
    files[4] = "Hello 4";
    files[5] = "Hello 5";
    files[12] = "Twelve";
}

// note: save in device-specific files in plugin folder not patch settings?
// or save in rack settings in the plugin callbacks?
json_t * Hc3Module::dataToJson()
{
    auto root = json_object();
    return root;
}

void Hc3Module::dataFromJson(json_t *root)
{
}

void Hc3Module::process(const ProcessArgs& args)
{
    if (1 == args.frame % 128) {
        for (int n = 0; n < 16; ++n) {
            if (n == loaded_id) {
                 getLight(n).setBrightness(1.f);
            } else {
                 getLight(n).setBrightness(.08f * (!files[n].empty()));
            }
        }
    }
}

}
Model *modelHc3 = createModel<pachde::Hc3Module, pachde::Hc3ModuleWidget>("pachde-hc-3");