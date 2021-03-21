#ifndef VK_VISUAL_FACADE_STL_UTIL_H_
#define VK_VISUAL_FACADE_STL_UTIL_H_

#include <algorithm>

namespace vkvf::stl_util
{
	template<class SourceContainer, class DestinationContainer, class Checker>
	bool All(const SourceContainer& values_to_check, const DestinationContainer& container_to_check_in, const Checker& checker)
	{
		for (auto&& value : values_to_check)
		{
			auto find_it = std::find_if(container_to_check_in.begin(), container_to_check_in.end(), [&value, &checker](auto& check_in_element) { return checker(value, check_in_element); });
			if (find_it == container_to_check_in.end())
			{
				return false;
			}
		}
		return true;
	}

	template<class SourceContainer, class DestinationContainer, class Checker>
	bool Any(const SourceContainer& values_to_check, const DestinationContainer& container_to_check_in, const Checker& checker)
	{
		for (auto&& value : values_to_check)
		{
			auto find_it = std::find_if(container_to_check_in.begin(), container_to_check_in.end(), [&value](auto& check_in_element) { return checker(value, check_in_element); });
			if (find_it != container_to_check_in.end())
			{
				return true;
			}
		}
		return false;
	}
}
#endif  // VK_VISUAL_FACADE_STL_UTIL_H_