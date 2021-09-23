#ifndef RENDER_ENGINE_RENDER_STL_UTIL_H_
#define RENDER_ENGINE_RENDER_STL_UTIL_H_

#include <algorithm>
#include <any>
#include <functional>

namespace render::stl_util
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


	template<typename T>
	struct function_traits;

	template<typename R, typename ...Args>
	struct function_traits<std::function<R(Args...)>>
	{
		static const size_t nargs = sizeof...(Args);

		typedef R result_type;

		template<size_t i>
		using arg_t = typename std::tuple_element<i, std::tuple<Args...>>::type;
	};

	template <typename Function, typename ... StaticArgs>
	auto GetSizeThenAlocThenGetDataPtrPtr(Function function, StaticArgs ... static_args)
	{
		std::function ffunction = function;
		
		using FunctionType = decltype(ffunction);

		const int nargs = function_traits<FunctionType>::nargs;

		using SizeTypePtr = typename function_traits<FunctionType>::template arg_t<nargs - 2>;
		using ResultTypePtr = typename function_traits<FunctionType>::template arg_t<nargs - 1>;

		std::remove_pointer_t<SizeTypePtr> size = 0;

		function(static_args..., &size, nullptr);

		std::vector<std::remove_pointer_t<ResultTypePtr>> result(size);

		function(static_args..., &size, result.data());

		return result;
	}
}
#endif  // RENDER_ENGINE_RENDER_STL_UTIL_H_
