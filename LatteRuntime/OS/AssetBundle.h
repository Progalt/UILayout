#ifndef LATTE_ASSET_BUNDLE_H
#define LATTE_ASSET_BUNDLE_H

#include <map>
#include <string>
#include <vector>

namespace latte
{
	class AssetBundle
	{
	public:

		static bool exists(const std::string& name);

		/*
			Does a match for partials
			For instance Roboto-Regular
			Could match to assets/Roboto-Regular.ttf
		*/
		static std::string match(const std::string& name);

		static std::vector<uint8_t> loadMem(const std::string& name);

		static void registerAssets(const std::map<std::string, std::string>& mapping);

	private:

		static std::map<std::string, std::string> m_AssetMapping;
	};
}

#endif // LATTE_ASSET_BUNDLE_H