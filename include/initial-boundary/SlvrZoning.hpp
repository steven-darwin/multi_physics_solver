/**
 * @file SlvrZoning.cpp
 * @author Steven Darwin
 * @version 0.0.1
 * @date Created : 2026-08-11
 * @date Last Modified : 2026-08-11
 *
 * @brief [Header] Mediator class for solver zoning process
 */

#ifndef SLVR_ZONING_HPP
#define SLVR_ZONING_HPP

#include "nlohmann/json.hpp"
using json = nlohmann::json;

class SlvrZoning {
public:

    /** Constructor of VszrZoning object */
    SlvrZoning() = default;

    /** Destructor of MeshZoning object */
    ~SlvrZoning() = default;

    void setupPhase();
    void executionPhase();

private:

};

#endif