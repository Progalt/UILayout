#include "AssetBundle.h"
#include <filesystem>
#include <fstream>
#include "../Utils/Log.h"

namespace latte
{
    std::map<std::string, std::string> AssetBundle::m_AssetMapping;

    void AssetBundle::registerAssets(const std::map<std::string, std::string>& mapping)
    {
        m_AssetMapping = mapping;
    }

    bool AssetBundle::exists(const std::string& name)
    {
        return m_AssetMapping.contains(name);
    }

    // Partial match ("Roboto-Regular" matches assets/Roboto-Regular.ttf)
    std::string AssetBundle::match(const std::string& name)
    {
        // Exact match first
        if (m_AssetMapping.contains(name)) return m_AssetMapping[name];

        // Search for partial substring matches in logical name or base filename
        auto it = std::find_if(m_AssetMapping.begin(), m_AssetMapping.end(),
            [&](const auto& pair)
            {
                const std::string& logical = pair.first;
                if (logical.find(name) != std::string::npos) return true;
                // Or check just filename
                std::filesystem::path path(logical);
                return path.filename().string().find(name) != std::string::npos;
            });
        if (it != m_AssetMapping.end())
            return it->second;

        return "";
    }

    // Reads the asset into memory as a byte vector
    std::vector<uint8_t> AssetBundle::loadMem(const std::string& name)
    {
        std::string path;
        if (exists(name)) 
        {
            path = m_AssetMapping[name];
        }
        else 
        {
            // Try matching the file
            path = match(name);
        }
        if (path.empty())
        {
            Log::log(Log::Severity::Warning, "Failed to load file with name {}", name);
            return {};
        }

        std::ifstream file(path, std::ios::binary);
        if (!file) return {};

        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        return data;
    }
}