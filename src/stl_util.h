#ifndef VK_VISUAL_FACADE_STL_UTIL_H_
#define VK_VISUAL_FACADE_STL_UTIL_H_

#include <algorithm>
#include <any>

namespace vkvf::stl_util
{
	struct ContainerCheckResult
	{
		std::any result_data;
		operator bool() { return !result_data.has_value(); }
	};


	template<class SourceContainer, class DestinationContainer, class Checker>
	ContainerCheckResult All(const SourceContainer& values_to_check, const DestinationContainer& container_to_check_in, const Checker& checker)
	{
		for (auto&& value : values_to_check)
		{
			auto find_it = std::find_if(container_to_check_in.begin(), container_to_check_in.end(), [&value, &checker](auto& check_in_element) { return checker(value, check_in_element); });
			if (find_it == container_to_check_in.end())
			{
				return ContainerCheckResult{ find_it };
			}
		}
		return ContainerCheckResult{};
	}

	template<class SourceContainer, class DestinationContainer, class Checker>
	ContainerCheckResult Any(const SourceContainer& values_to_check, const DestinationContainer& container_to_check_in, const Checker& checker)
	{
		for (auto&& value : values_to_check)
		{
			auto find_it = std::find_if(container_to_check_in.begin(), container_to_check_in.end(), [&value](auto& check_in_element) { return checker(value, check_in_element); });
			if (find_it != container_to_check_in.end())
			{
				return ContainerCheckResult{ find_it };
			}
		}
		return ContainerCheckResult{};
	}
}
#endif  // VK_VISUAL_FACADE_STL_UTIL_H_