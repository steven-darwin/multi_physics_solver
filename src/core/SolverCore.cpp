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

#include "core/SolverCore.hpp" 
#include "input-output/InputXDMFAdapter.hpp"
#include "input-output/OutputXDMFAdapter.hpp"

SolverCore::SolverCore() {
    // TBA
}

SolverCore::~SolverCore() {
    // TBA
}

void SolverCore::setup() {
    _inputAdapterInfo._adapterObj = std::make_shared<InputXDMFAdapter>("slab", "slab");
    _inputAdapterInfo._neutralGeometryTopology = std::dynamic_pointer_cast<InputAdapter>(_inputAdapterInfo._adapterObj)->deserialize();

    _outputAdapterInfo._adapterObj = std::make_shared<OutputXDMFAdapter>("slab", "slab");
    _outputAdapterInfo._neutralGeometryTopology = _inputAdapterInfo._neutralGeometryTopology;

    std::shared_ptr<std::vector<std::vector<double>>> temperature = std::make_shared<std::vector<std::vector<double>>>();
    for (unsigned int i = 0; i < 27; i++) {
        (*temperature).push_back({ 20 });
    }
    std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->addAttribute("Temperature", temperature, 1);

    std::shared_ptr<std::vector<std::vector<double>>> pressure = std::make_shared<std::vector<std::vector<double>>>();
    for (unsigned int i = 0; i < 27; i++) {
        (*pressure).push_back({ 5 });
    }
    std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->addAttribute("Pressure", pressure, 1);

    std::shared_ptr<std::vector<std::vector<double>>> velocity = std::make_shared<std::vector<std::vector<double>>>();
    for (unsigned int i = 0; i < 27; i++) {
        (*velocity).push_back({ 1, 2, 3 });
    }
    std::dynamic_pointer_cast<OutputXDMFAdapter>(_outputAdapterInfo._adapterObj)->addAttribute("Velocity", velocity, 3);

    std::dynamic_pointer_cast<OutputAdapter>(_outputAdapterInfo._adapterObj)->serialize(_outputAdapterInfo._neutralGeometryTopology);
}