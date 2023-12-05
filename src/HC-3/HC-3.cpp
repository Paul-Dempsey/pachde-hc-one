#include "HC-3.hpp"

namespace pachde
{

Hc3Module::Hc3Module()
: loaded_id(-1)
{
    clearFiles();
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configSwitch(SELECTED_PARAM, -1.f, 15.f, -1.f, "Current", {
        "None",
        "1.1", "1.2", "1.3", "1.4", "1.5", "1.6", "1.7", "1.8",
        "2.1", "2.2", "2.3", "2.4", "2.5", "2.6", "2.7", "2.8",
    });
    partner_binding.setClient(this);
}

Hc3Module::~Hc3Module()
{
    partner_binding.unsubscribe();
}

Hc1Module* Hc3Module::getPartner()
{
    return partner_binding.getPartner();
}

void Hc3Module::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_binding.onDeviceChanged(e);
    if (ui_event_sink) {
        ui_event_sink->onDeviceChanged(e);
    }
}

void Hc3Module::onDisconnect(const DisconnectEvent& e)
{
    partner_binding.onDisconnect(e);
    if (ui_event_sink) {
        ui_event_sink->onDisconnect(e);
    }
}

void Hc3Module::onFavoritesFileChanged(const FavoritesFileChangedEvent& e)
{
    if (e.path.empty()) {
        setSynchronizedLoadedId(-1);
        return;
    }
    if (loaded_id >= 0 && files[loaded_id] == e.path) {
        return;
    }
    auto it = std::find_if(files.cbegin(), files.cend(), [&](std::string const& f)->bool { return f == e.path; });
    setSynchronizedLoadedId(it == files.cend() ? -1 : static_cast<int>(std::distance(files.cbegin(), it)));
    if (ui_event_sink) {
        ui_event_sink->onFavoritesFileChanged(e);
    }
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
    if (ui_event_sink) {
        ui_event_sink->onFavoritesFileChanged(FavoritesFileChangedEvent{""});
    }
}

void Hc3Module::onRandomize()
{
    auto partner = getPartner();
    if (!partner) return;

    std::vector<int> items;
    int n = 0;
    for (auto f: files) {
        if (!f.empty()) {
            items.push_back(n);
        }
        ++n;
    }
    if (items.empty()) return;
    setSynchronizedLoadedId(items[randomZeroTo(items.size())]);
    partner->openFavoritesFile(files[loaded_id]);
}

void Hc3Module::useCurrentFavoriteFile(int id)
{
    const Hc1Module* partner = getPartner();
    if (!partner) return;
    files[id] = partner->favoritesFile;
    if (ui_event_sink) {
        ui_event_sink->onFavoritesFileChanged(FavoritesFileChangedEvent{""});
    }
}

json_t * Hc3Module::dataToJson()
{
    auto root = json_object();
    json_object_set_new(root, "device-claim", json_string(partner_binding.claim.c_str()));
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
    auto j = json_object_get(root, "device-claim");
    if (j) {
        partner_binding.setClaim(json_string_value(j));
    }
    auto jar = json_object_get(root, "fav_files");
    if (jar) {
        files.clear();
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            files.push_back(json_string_value(jp));
        }
    }
    getPartner();
}

void Hc3Module::openFile(int id) {
    if (id == loaded_id) return;
    setSynchronizedLoadedId(id);
    auto partner = getPartner();
    if (loaded_id != -1 && !files[loaded_id].empty()) {
        if (!partner) { 
            setSynchronizedLoadedId(-1);
            return;
        }
        partner->openFavoritesFile(files[loaded_id]);
    } else {
        setSynchronizedLoadedId(-1);
        partner->openFavoritesFile("");
    }
}

// no side effect on HC-1
void Hc3Module::setSynchronizedLoadedId(int id) {
    loaded_id = id;
    getParamQuantity(Params::SELECTED_PARAM)->setValue(id);
}

void Hc3Module::process(const ProcessArgs& args)
{
    float time = poll_timer.process(args.sampleTime);
    if (time > POLL_RATE) {
        poll_timer.reset();
        auto partner = getPartner();
        if (partner) {
            const std::string &partnerFile = partner->favoritesFile;
            if (partnerFile.empty()) {
                setSynchronizedLoadedId(-1);
            } else {
                if (loaded_id >= 0 && files[loaded_id] == partner->favoritesFile) {
                    // sync'd, nothing to do
                } else {
                    loaded_id = -1;
                    for (auto n = 0; n < 16; ++n) {
                        if (partnerFile == files[n]) {
                            setSynchronizedLoadedId(n);
                            break;
                        }
                    }
                }
            }
        } else {
            setSynchronizedLoadedId(-1);
        }
    }

    if (1 == args.frame % 1024) {
        auto pq = getParamQuantity(Params::SELECTED_PARAM);
        int paramValue = static_cast<int>(std::round(pq->getValue()));
        if (paramValue != loaded_id) {
            loaded_id = paramValue;
            if (loaded_id >= 0) {
                auto partner = getPartner();
                if (partner) {
                    partner->openFavoritesFile(files[loaded_id]);
                }
            }
        }

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
Model *modelHc3 = createModel<pachde::Hc3Module, pachde::Hc3ModuleWidget>("pachde-hc-favorites");