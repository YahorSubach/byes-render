#ifndef VK_VISUAL_FACADE_COMMON_H_
#define VK_VISUAL_FACADE_COMMON_H_

#include <iostream>

#ifndef NDEBUG
#define LOG(level, message) std::clog<<message<<std::endl
#else
#define LOG(level, message)
#endif // DEBUG

#endif //VK_VISUAL_FACADE_COMMON_H_
