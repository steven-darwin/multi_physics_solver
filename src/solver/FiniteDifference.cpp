/**
 * @file FiniteDifference.cpp
 * @author Steven Darwin
 * @version 0.0.1
 * @date Created : 2026-05-30
 * @date Last Modified : 2026-05-30
 *
 * @brief [Implementation] ...
 */

#include <memory>
#include <unordered_map>
#include <iostream>
#include <cmath>
#include <array>
#include <vector>

#include "input-output/OutputXDMFAdapter.hpp"
#include "geometry-topology/GeometryTopology.hpp"
#include "geometry-topology/GeometryTopologyVertex.hpp"

#include "solver/FiniteDifference.hpp"

FiniteDifference::FiniteDifference(std::shared_ptr<GeometryTopology> neutral_geometry_topology) {
    _neutralGeometryTopology = neutral_geometry_topology;
}

FiniteDifference::~FiniteDifference() {
    // TBA
}

std::shared_ptr<GeometryTopology> FiniteDifference::pointIterative(unsigned int step_count, double relaxation_factor, unsigned int iter_count, double error_tolerance) {
    std::unordered_map<std::shared_ptr<GeometryTopology>, unsigned int> vertex_list;
    _neutralGeometryTopology->getDescendants(vertex_list, GeometryTopology::Type::VERTEX);

    std::vector<std::shared_ptr<GeometryTopologyVertex>> ordered_vertex;
    ordered_vertex.resize(vertex_list.size());

    for (auto vertex_iter = vertex_list.begin(); vertex_iter != vertex_list.end(); vertex_iter++) {
        std::vector<double> computational_grid_coordinate = (*vertex_iter).first->getAttributeValue("computational_grid");

        unsigned int index = 0;
        for (unsigned int i = 0; i < 3; i++) {
            index += std::pow(step_count + 1, i) * std::round(computational_grid_coordinate[i] * step_count);
        }

        ordered_vertex[index] = std::dynamic_pointer_cast<GeometryTopologyVertex>((*vertex_iter).first);
    }

    std::array<double, 3> step_size = { 0, 0, 0 };
    step_size[0] = ordered_vertex[ordered_vertex.size() - 1]->getCoordinate(GeometryTopologyVertex::X) - ordered_vertex[0]->getCoordinate(GeometryTopologyVertex::X);
    step_size[1] = ordered_vertex[ordered_vertex.size() - 1]->getCoordinate(GeometryTopologyVertex::Y) - ordered_vertex[0]->getCoordinate(GeometryTopologyVertex::Y);
    step_size[2] = ordered_vertex[ordered_vertex.size() - 1]->getCoordinate(GeometryTopologyVertex::Z) - ordered_vertex[0]->getCoordinate(GeometryTopologyVertex::Z);

    double multiplier = (std::pow(step_size[0], 2) * std::pow(step_size[1], 2) * std::pow(step_size[2], 2)) / (
        (2 * std::pow(step_size[1], 2) * std::pow(step_size[2], 2)) +
        (2 * std::pow(step_size[2], 2) * std::pow(step_size[0], 2)) +
        (2 * std::pow(step_size[0], 2) * std::pow(step_size[1], 2))
        );

    for (unsigned int i = 0; i < iter_count; i++) {
        std::shared_ptr<OutputXDMFAdapter> solverOutputAdapter = std::make_shared<OutputXDMFAdapter>("slab_solver_iter" + std::to_string(i), "slab_solver_iter" + std::to_string(i));

        double actual_error = 0;

        for (unsigned int j = 0; j < ordered_vertex.size(); j++) {
            std::vector<double> computational_grid_coordinate = ordered_vertex[j]->getAttributeValue("computational_grid");
            bool is_outer_vertex = (computational_grid_coordinate[0] == 0) || (computational_grid_coordinate[1] == 0) || (computational_grid_coordinate[2] == 0) 
                || (computational_grid_coordinate[0] == 1) || (computational_grid_coordinate[1] == 1) || (computational_grid_coordinate[2] == 1);

            if (!is_outer_vertex) {
                double temp_value = multiplier;
                temp_value *= (
                    ((ordered_vertex[j + std::pow(step_count + 1, 0)]->getAttributeValue("temperature")[0] + ordered_vertex[j - std::pow(step_count + 1, 0)]->getAttributeValue("temperature")[0]) / std::pow(step_size[0], 2)) +
                    ((ordered_vertex[j + std::pow(step_count + 1, 1)]->getAttributeValue("temperature")[0] + ordered_vertex[j - std::pow(step_count + 1, 1)]->getAttributeValue("temperature")[0]) / std::pow(step_size[1], 2)) +
                    ((ordered_vertex[j + std::pow(step_count + 1, 2)]->getAttributeValue("temperature")[0] + ordered_vertex[j - std::pow(step_count + 1, 2)]->getAttributeValue("temperature")[0]) / std::pow(step_size[2], 2))
                    );

                actual_error += std::abs(ordered_vertex[j]->getAttributeValue("temperature")[0] - temp_value);

                ordered_vertex[j]->upsertAttribute("temperature", { ordered_vertex[j]->getAttributeValue("temperature")[0] + (relaxation_factor * (temp_value - ordered_vertex[j]->getAttributeValue("temperature")[0])) });

                std::cout << i << " " << j << " " << ordered_vertex[j]->getAttributeValue("temperature")[0] << " " << actual_error << std::endl;
            }
        }

        if (i > 0 && actual_error < error_tolerance) {
            break;
        }
        else {
            std::dynamic_pointer_cast<OutputXDMFAdapter>(solverOutputAdapter)->serialize(_neutralGeometryTopology);
        }
    }

    return _neutralGeometryTopology;
}

void FiniteDifference::predictiveCorrective() {
    // TBA
}