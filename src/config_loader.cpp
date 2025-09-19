#include <fstream>
#include <iostream>
#include "config_loader.h"

Config loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + filename);
    }

    nlohmann::json j;
    file >> j;

    Config cfg;
    cfg.input_data_path = j["input"]["input_data"];
    cfg.output_base = j["output"]["base"];
    cfg.output_disease = j["output"]["disease_type"];
    cfg.output_emerge = j["output"]["temporal_dynamics"]["emerge"];
    cfg.output_disappear = j["output"]["temporal_dynamics"]["disappear"];
    cfg.output_transfer = j["output"]["temporal_dynamics"]["transfer"];
    cfg.output_persist = j["output"]["temporal_dynamics"]["persist"];
    cfg.output_mge_group = j["output"]["mge_group"];
    cfg.viz_interaction = j["viz"]["interaction_json"];
    cfg.viz_parent = j["viz"]["parent_json"];

    return cfg;
}