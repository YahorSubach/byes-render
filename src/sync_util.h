#ifndef RENDER_ENGINE_SYNC_UTIL_H_
#define RENDER_ENGINE_SYNC_UTIL_H_

#include <vector>
#include <atomic>
#include <chrono>
#include <thread>

#include "common.h"

namespace byes::sync_util
{

	template<typename ItemType>
	class ConcQueue1P1C
	{
	public:

		ConcQueue1P1C(unsigned int size): push_position_(0), pop_position_(0), items_(size)
		{

		}

		void Push(const ItemType& item)
		{
			unsigned int push_pos = push_position_.load(std::memory_order_relaxed);
			unsigned int pop_pos = pop_position_.load(std::memory_order_acquire);

			while ((push_pos + 1) % items_.size() == pop_pos)
			{
				std::this_thread::yield();
				pop_pos = pop_position_.load(std::memory_order_acquire);
			}

			items_[push_pos] = item;

			push_position_.store((push_pos + 1) % items_.size(), std::memory_order_release);
		}

		ItemType Pop()
		{
			unsigned int push_pos = push_position_.load(std::memory_order_acquire);
			unsigned int pop_pos = pop_position_.load(std::memory_order_relaxed);

			while (push_pos == pop_pos)
			{
				std::this_thread::yield();
				push_pos = push_position_.load(std::memory_order_acquire);
			}

			ItemType res = items_[pop_pos];

			pop_position_.store((pop_pos + 1) % items_.size(), std::memory_order_release);

			return res;
		}

		unsigned int Size()
		{
			unsigned int push_pos = push_position_.load(std::memory_order_relaxed);
			unsigned int pop_pos = pop_position_.load(std::memory_order_relaxed);
			
			return (u32(items_.size()) + push_pos - pop_pos) % u32(items_.size());
		}


	private:

		std::vector<ItemType> items_;

		std::atomic<unsigned int> push_position_;
		std::atomic<unsigned int> pop_position_;
	};

}
#endif  // RENDER_ENGINE_SYNC_UTIL_H_
