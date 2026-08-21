/**
 * @file SlvrCore.cpp
 * @author Steven Darwin
 * @version 0.0.1
 * @date Created : 2026-04-18
 * @date Last Modified : 2026-04-18
 *
 * @brief [Implementation] ...
 */

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <array>
#include <fstream>
#include <unordered_set>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "input-output/InputXDMFAdapter.hpp"
#include "input-output/OutputXDMFAdapter.hpp"

#include "geometry-topology/GeometryTopologyVertex.hpp"

#include "utility/ConfigReader.hpp"

#include "core/SlvrCore.hpp" 
#include "solver/FiniteDifference.hpp"
#include "report/SlvrReport.hpp"

void SlvrCore::setup() {
    unsigned int progress = 0;

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

    std::vector<json> parsed_zone = json::parse(raw_zone).at("zone").get<std::vector<json>>();

    std::unordered_map<std::array<uint8_t, 16>, std::shared_ptr<GeometryTopology>, InputHDF5Adapter::UUIDHash> zone_entity_list;

    if (ConfigReader::instance().getRuntimeConfigValue("slvr", "strategy") == "gauss_seidel") {
        std::string existing_hdf5_file_path;
        char* hdf5_buffer;

        std::vector<OutputXDMFAdapter::ParameterMetadata> output_parameter_metadata_list;
        std::unordered_set<std::string> output_parameter_label_list;

        output_parameter_metadata_list.push_back(
            {
                "computational_grid",
                {1, 3},
                GeometryTopology::Type::VERTEX
            }
        );
        output_parameter_label_list.insert("computational_grid");

        // Pre Solver
        _inputAdapterInfo._adapterObj = std::make_shared<InputXDMFAdapter>("extended.mesh");
        InputXDMFAdapter::ParameterMetadata input_computational_grid_parameter = {
            "computational_grid",
            {1, 3},
            GeometryTopology::Type::VERTEX
        };
        std::dynamic_pointer_cast<InputXDMFAdapter>(_inputAdapterInfo._adapterObj)->addSolverParameter({ input_computational_grid_parameter });
        std::vector<std::shared_ptr<GeometryTopology>> pre_solver_neutral_geometry_topology = std::dynamic_pointer_cast<InputAdapter>(_inputAdapterInfo._adapterObj)->deserialize();

        SlvrReport::instance().addTimePoint("mesh_in", std::chrono::system_clock::now());
        SlvrReport::instance().addFileSuffix("in", "extended.mesh", "xmf");
        SlvrReport::instance().addFileSuffix("in", "extended.mesh", "h5");

        std::vector<std::shared_ptr<GeometryTopology>> non_whole_zone_geometry_topology_list;
        for (auto geometry_topology_iter = pre_solver_neutral_geometry_topology.begin(); geometry_topology_iter != pre_solver_neutral_geometry_topology.end(); geometry_topology_iter++) {
            for (auto zone_iter = parsed_zone.begin(); zone_iter != parsed_zone.end(); zone_iter++) {
                if ((*zone_iter).at("entity_id").get<std::array<uint8_t, 16>>() == (*geometry_topology_iter)->getID()) {
                    zone_entity_list.insert({ (*geometry_topology_iter)->getID(), (*geometry_topology_iter) });

                    if ((*zone_iter).at("type").get<std::string>() != "whole") {
                        non_whole_zone_geometry_topology_list.push_back((*geometry_topology_iter));
                    }
                    else {
                        _inputAdapterInfo._neutralGeometryTopology = (*geometry_topology_iter);
                    }
                }
            }
        }

        unsigned int initial_zone_total = 0;
        for (auto zone_iter = parsed_zone.begin(); zone_iter != parsed_zone.end(); zone_iter++) {
            if (zone_iter->contains("initial")) {
                initial_zone_total++;
            }
        }
        
        unsigned int initial_counter = 0;
        for (auto zone_iter = parsed_zone.begin(); zone_iter != parsed_zone.end(); zone_iter++) {
            if (zone_iter->contains("initial")) {
                std::unordered_map<std::shared_ptr<GeometryTopology>, unsigned int> vertex_list;
                zone_entity_list.at((*zone_iter).at("entity_id").get<std::array<uint8_t, 16>>())->getDescendants(vertex_list, GeometryTopology::Type::VERTEX);

                unsigned int counter = 0;
                for (auto vertex_iter = vertex_list.begin(); vertex_iter != vertex_list.end(); vertex_iter++) {
                    std::vector<json> parameter_list = zone_iter->at("initial");

                    for (auto parameter_iter = parameter_list.begin(); parameter_iter != parameter_list.end(); parameter_iter++) {
                        (*vertex_iter).first->upsertAttribute(parameter_iter->at("parameter").get<std::string>(), {1}, { std::stod(parameter_iter->at("value").get<std::string>()) });
                        
                        if (!output_parameter_label_list.contains(parameter_iter->at("parameter").get<std::string>())) {
                            output_parameter_metadata_list.push_back(
                                {
                                    parameter_iter->at("parameter").get<std::string>(),
                                    {1},
                                    GeometryTopology::Type::VERTEX
                                }
                            );

                            output_parameter_label_list.insert(parameter_iter->at("parameter").get<std::string>());
                        }
                    }

                    progress = 0 + (initial_counter / static_cast<double>(initial_zone_total) * 50) + (counter / static_cast<double>(vertex_list.size()) / initial_zone_total * 50);
                    std::cout << "\r" << "Processing Initial & Boundary Condition " << std::string(static_cast<size_t>(std::floor(progress / 10)), '=') << "> " << progress << "%";
                    counter++;
                }

                initial_counter++;
            }
        }

        SlvrReport::instance().addTimePoint("init_setup_complete", std::chrono::system_clock::now());
        std::cout << "\r" << "Processing Initial & Boundary Condition ==========> 50%";

        unsigned int boundary_zone_total = 0;
        for (auto zone_iter = parsed_zone.begin(); zone_iter != parsed_zone.end(); zone_iter++) {
            if (zone_iter->contains("boundary")) {
                boundary_zone_total++;
            }
        }

        unsigned int boundary_counter = 0;
        std::unordered_set<std::array<uint8_t, 16>, UUIDHash> boundary_vertex_list;
        for (auto zone_iter = parsed_zone.begin(); zone_iter != parsed_zone.end(); zone_iter++) {
            if (zone_iter->contains("boundary")) {
                std::unordered_map<std::shared_ptr<GeometryTopology>, unsigned int> vertex_list;
                zone_entity_list.at((*zone_iter).at("entity_id").get<std::array<uint8_t, 16>>())->getDescendants(vertex_list, GeometryTopology::Type::VERTEX);

                unsigned int counter = 0;
                for (auto vertex_iter = vertex_list.begin(); vertex_iter != vertex_list.end(); vertex_iter++) {
                    std::vector<json> parameter_list = zone_iter->at("boundary");

                    for (auto parameter_iter = parameter_list.begin(); parameter_iter != parameter_list.end(); parameter_iter++) {
                        (*vertex_iter).first->upsertAttribute(parameter_iter->at("parameter").get<std::string>(), { 1 }, { std::stod(parameter_iter->at("value").get<std::string>()) });

                        if (!boundary_vertex_list.contains((*vertex_iter).first->getID())) {
                            boundary_vertex_list.insert((*vertex_iter).first->getID());
                        }

                        if (!output_parameter_label_list.contains(parameter_iter->at("parameter").get<std::string>())) {
                            output_parameter_metadata_list.push_back(
                                {
                                    parameter_iter->at("parameter").get<std::string>(),
                                    {1},
                                    GeometryTopology::Type::VERTEX
                                }
                            );

                            output_parameter_label_list.insert(parameter_iter->at("parameter").get<std::string>());
                        }
                    }

                    progress = 50 + (boundary_counter / static_cast<double>(boundary_zone_total) * 50) + (counter / static_cast<double>(vertex_list.size()) / boundary_zone_total * 50);
                    std::cout << "\r" << "Processing Initial & Boundary Condition " << std::string(static_cast<size_t>(std::floor(progress / 10)), '=') << "> " << progress << "%";
                    counter++;
                }

                boundary_counter++;
            }
        }

        SlvrReport::instance().addTimePoint("bdry_setup_complete", std::chrono::system_clock::now());
        std::cout << "\r" << "Processing Initial & Boundary Condition ==========> 100%" << std::endl;

        _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>("original.pre.slvr");
        _outputAdapterInfo._neutralGeometryTopology = _inputAdapterInfo._neutralGeometryTopology;
        std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->addSolverParameter(output_parameter_metadata_list);
        std::dynamic_pointer_cast<OutputAdapter>(_outputAdapterInfo._adapterObj)->serialize(_outputAdapterInfo._neutralGeometryTopology);

        SlvrReport::instance().addTimePoint("original_pre_slvr_out", std::chrono::system_clock::now());
        SlvrReport::instance().addFileSuffix("out", "original.pre.slvr", "xmf");
        SlvrReport::instance().addFileSuffix("out", "original.pre.slvr", "h5");

        existing_hdf5_file_path =
            ConfigReader::instance().getRuntimeConfigValue("scmp", "staging_directory_path") +
            "/" +
            ConfigReader::instance().getRuntimeConfigValue("scmp", "file_name_prefix") +
            "." +
            "original.pre.slvr" +
            ".h5";

        hdf5_buffer = new char[existing_hdf5_file_path.length() + 1];
        std::memcpy(hdf5_buffer, existing_hdf5_file_path.c_str(), existing_hdf5_file_path.length());
        hdf5_buffer[existing_hdf5_file_path.length()] = '\0';
        
        _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>("extended.pre.slvr");
        std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->appendZoneCreationData(non_whole_zone_geometry_topology_list, hdf5_buffer);

        SlvrReport::instance().addTimePoint("extended_pre_slvr_out", std::chrono::system_clock::now());
        SlvrReport::instance().addFileSuffix("out", "extended.pre.slvr", "xmf");
        SlvrReport::instance().addFileSuffix("out", "extended.pre.slvr", "h5");


        // Solver
        SlvrReport::instance().addTimePoint("slvr_begin", std::chrono::system_clock::now());
        FiniteDifference solver(_inputAdapterInfo._neutralGeometryTopology);
        std::shared_ptr<GeometryTopology> post_solver_neutral_geometry_topology = solver.pointIterative(
            std::stoi(ConfigReader::instance().getRuntimeConfigValue("mesh", "transfinite_interpolation_segment_count")),
            std::stod(ConfigReader::instance().getRuntimeConfigValue("slvr", "gauss_seidel_relaxation_factor")),
            std::stoi(ConfigReader::instance().getRuntimeConfigValue("slvr", "gauss_seidel_max_iter")),
            std::stod(ConfigReader::instance().getRuntimeConfigValue("slvr", "gauss_seidel_error_tolerance")),
            output_parameter_metadata_list,
            non_whole_zone_geometry_topology_list,
            boundary_vertex_list
        );
        SlvrReport::instance().addTimePoint("slvr_finish", std::chrono::system_clock::now());

        // Post Solver
        _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>("original.post.slvr");
        _outputAdapterInfo._neutralGeometryTopology = post_solver_neutral_geometry_topology;
        std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->addSolverParameter(output_parameter_metadata_list);
        std::dynamic_pointer_cast<OutputAdapter>(_outputAdapterInfo._adapterObj)->serialize(_outputAdapterInfo._neutralGeometryTopology);
        
        SlvrReport::instance().addTimePoint("extended_post_slvr_out", std::chrono::system_clock::now());
        SlvrReport::instance().addFileSuffix("out", "original.post.slvr", "xmf");
        SlvrReport::instance().addFileSuffix("out", "original.post.slvr", "h5");

        existing_hdf5_file_path =
            ConfigReader::instance().getRuntimeConfigValue("scmp", "staging_directory_path") +
            "/" +
            ConfigReader::instance().getRuntimeConfigValue("scmp", "file_name_prefix") +
            "." +
            "original.post.slvr" +
            ".h5";

        hdf5_buffer = new char[existing_hdf5_file_path.length() + 1];
        std::memcpy(hdf5_buffer, existing_hdf5_file_path.c_str(), existing_hdf5_file_path.length());
        hdf5_buffer[existing_hdf5_file_path.length()] = '\0';

        _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>("extended.post.slvr");
        std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->appendZoneCreationData(non_whole_zone_geometry_topology_list, hdf5_buffer);
        
        SlvrReport::instance().addTimePoint("extended_post_slvr_out", std::chrono::system_clock::now());
        SlvrReport::instance().addFileSuffix("out", "extended.post.slvr", "xmf");
        SlvrReport::instance().addFileSuffix("out", "extended.post.slvr", "h5");
    }
}