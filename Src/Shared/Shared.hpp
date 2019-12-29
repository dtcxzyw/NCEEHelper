#pragma once
#include "../ThirdParty/Bus/BusModule.hpp"
#include "../ThirdParty/Bus/BusReporter.hpp"
#include "../ThirdParty/Bus/BusSystem.hpp"

using Bus::GUID;
using Bus::GUID2Str;
using Bus::Name;
using Bus::ReportLevel;
using Bus::str2GUID;
using Bus::Unmoveable;
namespace fs = std::filesystem;

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

inline size_t hashGUID(const GUID& x) {
    return std::_Fnv1a_append_bytes(std::_FNV_offset_basis,
                                    &reinterpret_cast<const unsigned char&>(x),
                                    sizeof(GUID));
}

struct GUIDHasher final {
    size_t operator()(const GUID& id) const {
        return hashGUID(id);
    }
};
