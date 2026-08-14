/**
 * @file SlvrCore.hpp
 * @author Steven Darwin
 * @version 0.0.1
 * @date Created : 2026-04-18
 * @date Last Modified : 2026-04-18
 *
 * @brief [Header] ...
 */

#ifndef SLVR_CORE_HPP
#define SLVR_CORE_HPP

#include "general/Generic.hpp"

#include "geometry-topology/GeometryTopology.hpp"

class SlvrCore {
public:
    struct UUIDHash {
        std::size_t operator()(const std::array<uint8_t, 16>& uuid) const noexcept {
            std::size_t hash = 0;
            for (auto byte : uuid) {
                hash ^= static_cast<std::size_t>(byte)
                    + static_cast<std::size_t>(0x9e3779b9)
                    + (hash << 6)
                    + (hash >> 2);
            }
            return hash;
        }
    };

    /** Constructor of SolverCore object
     */
    SlvrCore();
    SlvrCore(const char* runtime_config_file_path);

    /** Destructor of SolverCore object */
    ~SlvrCore();

    void setup();

private:
    const char* _runtimeConfigFilePath;

    /** Attribute to store input adapter metadata */
    scmp::AdapterInfo _inputAdapterInfo;

    /** Attribute to store modified neutral geometry topology data */
    std::shared_ptr<GeometryTopology> _modifiedNeutralGeometryTopology;

    /** Attribute to store output adapter metadata */
    scmp::AdapterInfo _outputAdapterInfo;
};

#endif