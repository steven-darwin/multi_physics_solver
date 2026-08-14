/**
 * @file FiniteDifference.hpp
 * @author Steven Darwin
 * @version 0.0.1
 * @date Created : 2026-05-30
 * @date Last Modified : 2026-05-30
 *
 * @brief [Header] ...
 */

#ifndef FINITE_DIFFERENCE_HPP
#define FINITE_DIFFERENCE_HPP

#include <vector>
#include <unordered_set>
#include <array>

#include "geometry-topology/GeometryTopology.hpp"
#include "input-output/OutputXDMFAdapter.hpp"

#include "core/SlvrCore.hpp"

class FiniteDifference {
public:


    /** Constructor of FiniteDifference object
     */
    FiniteDifference(std::shared_ptr<GeometryTopology> neutral_geometry_topology, const char* runtime_config_file_path);

    /** Destructor of FiniteDifference object */
    ~FiniteDifference();

    /** Method to solve the differential equation using point iterative method */
    std::shared_ptr<GeometryTopology> pointIterative(
        unsigned int step_count, 
        double relaxation_factor, 
        unsigned int iter_count, 
        double error_tolerance, 
        std::vector<OutputXDMFAdapter::ParameterMetadata> output_parameter_metadata_list, 
        std::vector<std::shared_ptr<GeometryTopology>> zone_neutral_geometry_topology_list,
        std::unordered_set<std::array<uint8_t, 16>, SlvrCore::UUIDHash>& boundary_vertex_list
    );

    /** Method to solve the differential equation using predictive corrective method */
    void predictiveCorrective();

private:
    const char* _runtimeConfigFilePath;

    /** Attribute to store nwutral geometry-topology data */
    std::shared_ptr<GeometryTopology> _neutralGeometryTopology;
};

#endif