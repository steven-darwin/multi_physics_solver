/**
 * @file SlvrZoning.cpp
 * @author Steven Darwin
 * @version 0.0.1
 * @date Created : 2026-08-11
 * @date Last Modified : 2026-08-11
 *
 * @brief [Implementation] Mediator class for solver zoning process
 */

#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <unordered_set>
#include <array>
#include <iostream>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "input-output/InputHDF5Adapter.hpp"
#include "input-output/OutputXDMFAdapter.hpp"
#include "geometry-topology/GeometryTopology.hpp"
#include "utility/ConfigReader.hpp"

#include "initial-boundary/SlvrZoning.hpp"
#include "report/SlvrReport.hpp"

void SlvrZoning::setupPhase() {
    std::string zone_json_file_path =
        ConfigReader::instance().getRuntimeConfigValue("scmp", "staging_directory_path") +
        "/" +
        ConfigReader::instance().getRuntimeConfigValue("scmp", "file_name_prefix") +
        "_" +
        "zone.json";

    char* zone_buffer = new char[zone_json_file_path.length() + 1];
    std::memcpy(zone_buffer, zone_json_file_path.c_str(), zone_json_file_path.length());
    zone_buffer[zone_json_file_path.length()] = '\0';

    std::ifstream raw_zone(zone_buffer);
    json full_zone_data = json::parse(raw_zone);
    std::vector<json> parsed_zone = full_zone_data.at("zone").get<std::vector<json>>();
    raw_zone.close();

    bool setup_zone = true;
    while (setup_zone) {
        std::string is_zone_being_added;
        unsigned int counter = 0;

        std::cout << "Adding Initial or Boundary Condition to a Zone? (Y/N)" << std::endl;
        std::cin >> is_zone_being_added;

        if (is_zone_being_added == "Y") {
            SlvrReport::InitBdrySetupItem init_bdry_setup_item;

            std::cout << "-----------------------------------------------" << std::endl;

            std::vector<std::string> condition_list = { "initial", "boundary" };

            counter = 0;
            std::string selected_condition;
            for (auto condition_iter = condition_list.begin(); condition_iter != condition_list.end(); condition_iter++) {
                std::cout << (++counter) << ". " << (*condition_iter) << std::endl;
            }
            std::cout << "selected condition: ";
            std::cin >> selected_condition;
            init_bdry_setup_item.condition_type = condition_list[std::stoi(selected_condition) - 1];

            std::cout << "-----------------------------------------------" << std::endl;

            counter = 0;
            std::string selected_zone_option;
            for (auto zone_iter = parsed_zone.begin(); zone_iter != parsed_zone.end(); zone_iter++) {
                std::cout << (++counter) << ". " << zone_iter->at("name").get<std::string>() << std::endl;
            }
            std::cout << "selected zone : ";
            std::cin >> selected_zone_option;
            init_bdry_setup_item.zone_name = parsed_zone[std::stoi(selected_zone_option) - 1].at("name").get<std::string>();

            std::string inputted_parameter;
            std::cout << "parameter : ";
            std::cin >> inputted_parameter;

            std::cout << "-----------------------------------------------" << std::endl;

            json init_bdry_setup = {};

            std::cout << parsed_zone[std::stoi(selected_zone_option) - 1].at("name").get<std::string>() << " " << condition_list[std::stoi(selected_condition) - 1] << " condition setup: " << std::endl;

            init_bdry_setup["parameter"] = init_bdry_setup_item.parameter = inputted_parameter;

            counter = 0;
            std::vector<std::string> mode_list = { "uniform" };
            std::string selected_mode;
            for (auto mode_iter = mode_list.begin(); mode_iter != mode_list.end(); mode_iter++) {
                std::cout << (++counter) << ". " << (*mode_iter) << std::endl;
            }
            std::cout << "selected mode : ";
            std::cin >> selected_mode;
            init_bdry_setup["mode"] = init_bdry_setup_item.mode = mode_list[std::stoi(selected_mode) - 1];

            std::string inputted_value;
            std::cout << "value : ";
            std::cin >> inputted_value;
            init_bdry_setup["value"] = inputted_value;
            init_bdry_setup_item.value = std::stod(inputted_value);

            std::vector<json> init_bdry_setup_list;

            if (full_zone_data["zone"][std::stoi(selected_zone_option) - 1].contains(condition_list[std::stoi(selected_condition) - 1])) {
                std::vector<json> existing_init_bdry_setup_list = full_zone_data["zone"][std::stoi(selected_zone_option) - 1].at(condition_list[std::stoi(selected_condition) - 1]).get<std::vector<json>>();
                for (auto setup_iter = existing_init_bdry_setup_list.begin(); setup_iter != existing_init_bdry_setup_list.end(); setup_iter++) {
                    init_bdry_setup_list.push_back((*setup_iter));
                }
            }

            SlvrReport::instance().setInitBdrySetupItem(init_bdry_setup_item);

            init_bdry_setup_list.push_back(init_bdry_setup);
            full_zone_data["zone"][std::stoi(selected_zone_option) - 1][condition_list[std::stoi(selected_condition) - 1]] = init_bdry_setup_list;
        }
        else if (is_zone_being_added == "N") {
            break;
        }
        else {
            continue;
        }
    }

    std::ofstream modified_zone(zone_buffer);
    modified_zone << std::setw(4) << full_zone_data << std::endl;
    modified_zone.close();
}

void SlvrZoning::executionPhase() {
    // TBA
}