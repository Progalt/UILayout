
#ifndef LATTE_UI_H
#define LATTE_UI_H

#include <string>
#include <map>

#define LATTEUI_MAJOR_VERSION 0
#define LATTEUI_MINOR_VERSION 1


#if defined(_WIN32) || defined(_WIN64)
#ifdef LATTEUI_EXPORTS
#define LATTEUI_API __declspec(dllexport)
#else
#define LATTEUI_API __declspec(dllimport)
#endif
#endif

namespace latte
{
	bool LATTEUI_API runScript(const std::string& path);

	// Sets the base path of where latte is, so it can read Lua library files
	void LATTEUI_API setLibBasePath(const std::string& path);

	void LATTEUI_API registerAssets(const std::map<std::string, std::string>& assets);

	bool LATTEUI_API hotRestart();
}

#endif // LATTE_UI_H