
#ifndef LATTE_UI_H
#define LATTE_UI_H

#include <string>

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

	bool LATTEUI_API hotRestart();
}

#endif // LATTE_UI_H