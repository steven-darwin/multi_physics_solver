/**
 * @file SolverCore.cpp
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

#include "input-output/InputXDMFAdapter.hpp"
#include "input-output/OutputXDMFAdapter.hpp"

#include "geometry-topology/GeometryTopologyVertex.hpp"

#include "core/SolverCore.hpp" 
#include "solver/FiniteDifference.hpp"

SolverCore::SolverCore() {
    // TBA
}

SolverCore::~SolverCore() {
    // TBA
}

void SolverCore::setup() {
    // Pre Solver
    _inputAdapterInfo._adapterObj = std::make_shared<InputXDMFAdapter>("slab_mesh", "slab_mesh");
    _inputAdapterInfo._neutralGeometryTopology = std::dynamic_pointer_cast<InputAdapter>(_inputAdapterInfo._adapterObj)->deserialize();
    std::shared_ptr<GeometryTopology> pre_solver_neutral_geometry_topology = _inputAdapterInfo._neutralGeometryTopology;
    
    std::unordered_map<std::shared_ptr<GeometryTopology>, unsigned int> vertex_list;
    pre_solver_neutral_geometry_topology->getDescendants(vertex_list, GeometryTopology::Type::VERTEX);

    std::array<double, 3> min_coordinate, max_coordinate;
    std::array<GeometryTopologyVertex::Coordinate, 3> axis = { GeometryTopologyVertex::Coordinate::X, GeometryTopologyVertex::Coordinate::Y , GeometryTopologyVertex::Coordinate::Z };

    for (auto vertex_iter = vertex_list.begin(); vertex_iter != vertex_list.end(); vertex_iter++) {
        for (auto axis_type : axis) {
            if (vertex_iter == vertex_list.begin()) {
                min_coordinate[axis_type] = dynamic_cast<GeometryTopologyVertex*>((*vertex_iter).first.get())->getCoordinate(axis_type);
                max_coordinate[axis_type] = dynamic_cast<GeometryTopologyVertex*>((*vertex_iter).first.get())->getCoordinate(axis_type);
            }
            else {
                if (min_coordinate[axis_type] > dynamic_cast<GeometryTopologyVertex*>((*vertex_iter).first.get())->getCoordinate(axis_type)) {
                    min_coordinate[axis_type] = dynamic_cast<GeometryTopologyVertex*>((*vertex_iter).first.get())->getCoordinate(axis_type);
                }

                if (max_coordinate[axis_type] < dynamic_cast<GeometryTopologyVertex*>((*vertex_iter).first.get())->getCoordinate(axis_type)) {
                    max_coordinate[axis_type] = dynamic_cast<GeometryTopologyVertex*>((*vertex_iter).first.get())->getCoordinate(axis_type);
                }
            }
        }
    }

    for (auto vertex_iter = vertex_list.begin(); vertex_iter != vertex_list.end(); vertex_iter++) {
        bool is_outer_vertex = false;
                    
        if (!is_outer_vertex) {
            for (auto axis_type : axis) {
                if (std::abs(min_coordinate[axis_type] - dynamic_cast<GeometryTopologyVertex*>((*vertex_iter).first.get())->getCoordinate(axis_type)) < 0.001) {
                    is_outer_vertex = true;
                    break;
                }
            }
        }

        if (!is_outer_vertex) {
            for (auto axis_type : axis) {
                if (std::abs(max_coordinate[axis_type] - dynamic_cast<GeometryTopologyVertex*> ((*vertex_iter).first.get())->getCoordinate(axis_type)) < 0.001) {
                    is_outer_vertex = true;
                    break;
                }
            }
        }

        ((*vertex_iter).first.get())->upsertAttribute("temperature", { (is_outer_vertex ? 100.0f : 20.0f) });
    }

    _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>("slab_pre_solver", "slab_pre_solver");
    _outputAdapterInfo._neutralGeometryTopology = pre_solver_neutral_geometry_topology;
    std::dynamic_pointer_cast<OutputAdapter>(_outputAdapterInfo._adapterObj)->serialize(_outputAdapterInfo._neutralGeometryTopology);

    // Solver
    FiniteDifference solver(pre_solver_neutral_geometry_topology);
    std::shared_ptr<GeometryTopology> post_solver_neutral_geometry_topology = solver.pointIterative(4, 1, 100, 0.001);

    // Post Solver
    _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>("slab_post_solver", "slab_post_solver");
    _outputAdapterInfo._neutralGeometryTopology = post_solver_neutral_geometry_topology;
    std::dynamic_pointer_cast<OutputAdapter>(_outputAdapterInfo._adapterObj)->serialize(_outputAdapterInfo._neutralGeometryTopology);

    /*std::shared_ptr<std::vector<std::vector<double>>> temperature = std::make_shared<std::vector<std::vector<double>>>();
    for (unsigned int i = 0; i < 125; i++) {
        (*temperature).push_back({ 20 });
    }*/
    //std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->addAttribute("Temperature", temperature, 1);

    //std::shared_ptr<std::vector<std::vector<double>>> pressure = std::make_shared<std::vector<std::vector<double>>>();
    //for (unsigned int i = 0; i < 125; i++) {
    //    (*pressure).push_back({ 5 });
    //}
    //std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->addAttribute("Pressure", pressure, 1);

    //std::shared_ptr<std::vector<std::vector<double>>> velocity = std::make_shared<std::vector<std::vector<double>>>();
    //for (unsigned int i = 0; i < 125; i++) {
    //    (*velocity).push_back({ 1, 2, 3 });
    //}
    //std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->addAttribute("Velocity", velocity, 3);

    //std::dynamic_pointer_cast<OutputAdapter>(_outputAdapterInfo._adapterObj)->serialize(_outputAdapterInfo._neutralGeometryTopology);

    //_inputAdapterInfo._neutralGeometryTopology = std::dynamic_pointer_cast<InputAdapter>(_inputAdapterInfo._adapterObj)->deserialize();
    //
    //FiniteDifference solver(_inputAdapterInfo._neutralGeometryTopology);
    //solver.pointIterative(4, 1, 100, 0.001);

    //_outputAdapterInfo._neutralGeometryTopology = _inputAdapterInfo._neutralGeometryTopology;
    //std::dynamic_pointer_cast<OutputAdapter>(_outputAdapterInfo._adapterObj)->serialize(_outputAdapterInfo._neutralGeometryTopology);
}