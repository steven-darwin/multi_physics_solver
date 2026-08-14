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

#include "input-output/InputXDMFAdapter.hpp"
#include "input-output/OutputXDMFAdapter.hpp"

#include "geometry-topology/GeometryTopologyVertex.hpp"

#include "utility/ConfigReader.hpp"

#include "core/SlvrCore.hpp" 
#include "solver/FiniteDifference.hpp"

SlvrCore::SlvrCore() {
    // TBA
}

SlvrCore::SlvrCore(const char* runtime_config_file_path) {
    _runtimeConfigFilePath = runtime_config_file_path;
}

SlvrCore::~SlvrCore() {
    // TBA
}

void SlvrCore::setup() {
    ConfigReader config_reader;

    std::string zone_json_file_path =
        config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "scmp", "staging_directory_path") +
        "/" +
        config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "scmp", "file_name_prefix") +
        "_" +
        "zone.json";

    char* zone_buffer = new char[zone_json_file_path.length() + 1];
    std::memcpy(zone_buffer, zone_json_file_path.c_str(), zone_json_file_path.length());
    zone_buffer[zone_json_file_path.length()] = '\0';

    std::ifstream raw_zone(zone_buffer);

    std::vector<json> parsed_zone = json::parse(raw_zone).at("zone").get<std::vector<json>>();

    std::unordered_map<std::array<uint8_t, 16>, std::shared_ptr<GeometryTopology>, InputHDF5Adapter::UUIDHash> zone_entity_list;

    if (config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "slvr", "strategy") == "gauss_seidel") {
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
        _inputAdapterInfo._adapterObj = std::make_shared<InputXDMFAdapter>(_runtimeConfigFilePath, "extended.mesh");
        InputXDMFAdapter::ParameterMetadata input_computational_grid_parameter = {
            "computational_grid",
            {1, 3},
            GeometryTopology::Type::VERTEX
        };
        std::dynamic_pointer_cast<InputXDMFAdapter>(_inputAdapterInfo._adapterObj)->addSolverParameter({ input_computational_grid_parameter });
        std::vector<std::shared_ptr<GeometryTopology>> pre_solver_neutral_geometry_topology = std::dynamic_pointer_cast<InputAdapter>(_inputAdapterInfo._adapterObj)->deserialize();
        
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

        for (auto zone_iter = parsed_zone.begin(); zone_iter != parsed_zone.end(); zone_iter++) {
            if (zone_iter->contains("initial")) {
                std::unordered_map<std::shared_ptr<GeometryTopology>, unsigned int> vertex_list;
                zone_entity_list.at((*zone_iter).at("entity_id").get<std::array<uint8_t, 16>>())->getDescendants(vertex_list, GeometryTopology::Type::VERTEX);

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
                }
            }
        }

        std::unordered_set<std::array<uint8_t, 16>, UUIDHash> boundary_vertex_list;
        for (auto zone_iter = parsed_zone.begin(); zone_iter != parsed_zone.end(); zone_iter++) {
            if (zone_iter->contains("boundary")) {
                std::unordered_map<std::shared_ptr<GeometryTopology>, unsigned int> vertex_list;
                zone_entity_list.at((*zone_iter).at("entity_id").get<std::array<uint8_t, 16>>())->getDescendants(vertex_list, GeometryTopology::Type::VERTEX);

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
                }
            }
        }

        _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>(_runtimeConfigFilePath, "original.pre.slvr");
        _outputAdapterInfo._neutralGeometryTopology = _inputAdapterInfo._neutralGeometryTopology;
        std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->addSolverParameter(output_parameter_metadata_list);
        std::dynamic_pointer_cast<OutputAdapter>(_outputAdapterInfo._adapterObj)->serialize(_outputAdapterInfo._neutralGeometryTopology);

        existing_hdf5_file_path =
            config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "scmp", "staging_directory_path") +
            "/" +
            config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "scmp", "file_name_prefix") +
            "." +
            "original.pre.slvr" +
            ".h5";

        hdf5_buffer = new char[existing_hdf5_file_path.length() + 1];
        std::memcpy(hdf5_buffer, existing_hdf5_file_path.c_str(), existing_hdf5_file_path.length());
        hdf5_buffer[existing_hdf5_file_path.length()] = '\0';
        
        _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>(_runtimeConfigFilePath, "extended.pre.slvr");
        std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->appendZoneCreationData(non_whole_zone_geometry_topology_list, hdf5_buffer);


        // Solver
        FiniteDifference solver(_inputAdapterInfo._neutralGeometryTopology, _runtimeConfigFilePath);
        std::shared_ptr<GeometryTopology> post_solver_neutral_geometry_topology = solver.pointIterative(
            std::stoi(config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "mesh", "transfinite_interpolation_segment_count")),
            std::stod(config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "slvr", "gauss_seidel_relaxation_factor")),
            std::stoi(config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "slvr", "gauss_seidel_max_iter")),
            std::stod(config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "slvr", "gauss_seidel_error_tolerance")),
            output_parameter_metadata_list,
            non_whole_zone_geometry_topology_list,
            boundary_vertex_list
        );

        // Post Solver
        _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>(_runtimeConfigFilePath, "original.post.slvr");
        _outputAdapterInfo._neutralGeometryTopology = post_solver_neutral_geometry_topology;
        std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->addSolverParameter(output_parameter_metadata_list);
        std::dynamic_pointer_cast<OutputAdapter>(_outputAdapterInfo._adapterObj)->serialize(_outputAdapterInfo._neutralGeometryTopology);

        existing_hdf5_file_path =
            config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "scmp", "staging_directory_path") +
            "/" +
            config_reader.getRuntimeConfigValue(_runtimeConfigFilePath, "scmp", "file_name_prefix") +
            "." +
            "original.post.slvr" +
            ".h5";

        hdf5_buffer = new char[existing_hdf5_file_path.length() + 1];
        std::memcpy(hdf5_buffer, existing_hdf5_file_path.c_str(), existing_hdf5_file_path.length());
        hdf5_buffer[existing_hdf5_file_path.length()] = '\0';

        _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>(_runtimeConfigFilePath, "extended.post.slvr");
        std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->appendZoneCreationData(non_whole_zone_geometry_topology_list, hdf5_buffer);
    }
}