#pragma once
#include "../ThirdParty/Bus/BusSystem.hpp"
#include "../ThirdParty/Bus/BusReporter.hpp"
#include "../ThirdParty/Bus/BusModule.hpp"

using Bus::GUID;
using Bus::GUID2Str;
using Bus::str2GUID;
using Bus::Unmoveable;

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>

class Config;

class PluginHelperAPI final {
public:
};

using PluginHelper = PluginHelperAPI *;
