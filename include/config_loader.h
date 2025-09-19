// config_loader.h

#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <string>
#include "../external/json.hpp"

struct Config {
    std::string input_data_path;

    std::string output_base;
    std::string output_disease;
    std::string output_mge_group;
    std::string output_emerge;
    std::string output_disappear;
    std::string output_transfer;
    std::string output_persist;

    std::string viz_interaction;
    std::string viz_parent;
};

Config loadConfig(const std::string& filename);

#endif // CONFIG_LOADER_H