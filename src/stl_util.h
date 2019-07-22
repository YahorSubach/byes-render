#ifndef VK_VISUAL_FACADE_STL_UTIL_H_
#define VK_VISUAL_FACADE_STL_UTIL_H_

namespace vkvf
{
	namespace stl_util
	{
		namespace map
		{
			template<class Map, typename... ValueConstructorTypes>
			Map::iterator TryConstructEmplace(Map& map, const Map::key_type& key, ValueConstructorTypes&& ... args)
			{
				auto it = map.lower_bound(key);
				if (it != map.end() && it->first != key)
				{

				}
			}
		}
	}
}
#endif  // VK_VISUAL_FACADE_STL_UTIL_H_