
#ifndef LATTE_LUA_HELPERS_H
#define LATTE_LUA_HELPERS_H

#include <sol/sol.hpp>

namespace latte
{
	inline bool shallowTableEqual(const sol::table& a, const sol::table& b)
	{
        // Fast-path pointer equality or both nil
        if (!a.valid() && !b.valid()) return true;
        if (!a.valid() || !b.valid()) return false;
        if (a == b) return true;

        // Compare all values in a to b
        std::size_t count_a = 0;
        for (auto& kvp : a) 
        {
            const sol::object& key = kvp.first;
            const sol::object& a_value = kvp.second;
            sol::object b_value = b.get<sol::object>(key);

            // Use sol::object's == for shallow comparison
            if (!(a_value == b_value)) {
                return false;
            }
            ++count_a;
        }

        // Make sure b doesn't have extra keys
        std::size_t count_b = 0;
        for (auto& kvp : b) 
        {
            ++count_b;
        }

        return count_a == count_b;
	}
}

#endif // LATTE_LUA_HELPERS_H