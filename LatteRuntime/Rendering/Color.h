
#ifndef LATTE_COLOR_H
#define LATTE_COLOR_H

#include <string>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <iomanip>

namespace latte
{
	class Color
	{
	public:

		Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) { }
		
		// String in the format #RGB or #RGBA or #RRGGBB or #RRGGBBAA
		static Color fromStringHex(const std::string& hexStr)
		{
            if (hexStr.empty() || hexStr[0] != '#')
                throw std::invalid_argument("Hex string must start with '#'");

            std::string hex = hexStr.substr(1); // Remove '#'
            size_t len = hex.length();

            unsigned int r = 0, g = 0, b = 0, a = 255;

            auto hexToInt = [](const std::string& s) -> unsigned int 
            {
                unsigned int v;
                std::istringstream iss(s);
                iss >> std::hex >> v;
                if (iss.fail())
                    throw std::invalid_argument("Invalid hex component");
                return v;
            };

            if (len == 3 || len == 4) 
            {
                // Expand to 6 or 8 digits
                r = hexToInt(std::string(2, hex[0]));
                g = hexToInt(std::string(2, hex[1]));
                b = hexToInt(std::string(2, hex[2]));
                if (len == 4)
                    a = hexToInt(std::string(2, hex[3]));
            }
            else if (len == 6 || len == 8) 
            {
                r = hexToInt(hex.substr(0, 2));
                g = hexToInt(hex.substr(2, 2));
                b = hexToInt(hex.substr(4, 2));
                if (len == 8)
                    a = hexToInt(hex.substr(6, 2));
            }
            else 
            {
                throw std::invalid_argument("Hex string has invalid length");
            }

            return Color(
                static_cast<float>(r) / 255.0f,
                static_cast<float>(g) / 255.0f,
                static_cast<float>(b) / 255.0f,
                static_cast<float>(a) / 255.0f
            );
		}

		float r, g, b, a;

	private:
	};
}

#endif // LATTE_COLOR_H