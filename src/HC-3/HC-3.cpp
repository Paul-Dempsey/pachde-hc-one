#include "HC-3.hpp"
#include "../HcOne.hpp"

namespace pachde
{

Hc3Module::Hc3Module()
{
    clearFiles();
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configSwitch(SELECTED_PARAM, -1.f, 15.f, -1.f, "Current", {
        "None",
        "1.1", "1.2", "1.3", "1.4", "1.5", "1.6", "1.7", "1.8",
        "2.1", "2.2", "2.3", "2.4", "2.5", "2.6", "2.7", "2.8",
    });
}

void Hc3Module::clearFiles()
{
    loaded_id = -1;
    files.clear();
    for (auto n = 0; n < 16; ++n) {
        files.push_back("");
    }
}

void Hc3Module::onReset()
{
    clearFiles();
}

json_t * Hc3Module::dataToJson()
{
    auto root = json_object();

    auto hc1 = HcOne::get()->getSoleHc1();
    std::string device = hc1 ? hc1->deviceName() : "";
    json_object_set_new(root, "device", json_stringn(device.c_str(), device.size()));

    if (!files.empty()) {
        auto jar = json_array();
        for (auto f: files) {
            json_array_append_new(jar, json_stringn(f.c_str(), f.size()));
        }
        json_object_set_new(root, "fav_files", jar);
    }
    return root;
}

void Hc3Module::dataFromJson(json_t *root)
{
    auto jar = json_object_get(root, "fav_files");
    if (jar) {
        files.clear();
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            files.push_back(json_string_value(jp));
        }
    }
}

void Hc3Module::process(const ProcessArgs& args)
{
    if (1 == args.frame % 1024) {
        for (int n = 0; n < 16; ++n) {
            if (n == loaded_id) {
                 getLight(n).setBrightness(1.f);
                 assert(!files[n].empty());
            } else {
                 getLight(n).setBrightness(.08f * (!files[n].empty()));
            }
        }
    }
}

}
Model *modelHc3 = createModel<pachde::Hc3Module, pachde::Hc3ModuleWidget>("pachde-hc-3");