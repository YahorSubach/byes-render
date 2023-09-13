#ifndef RENDER_ENGINE_RENDER_util_H_
#define RENDER_ENGINE_RENDER_util_H_

#include <algorithm>
#include <any>
#include <functional>
#include <optional>
#include <utility>
#include <type_traits>

namespace render::util
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


	template<class Result, class Container>
	Result size(const Container& c) { return static_cast<Result>(c.size()); }

	template<typename ReferencedType>
	struct NullableRef : public std::optional<std::reference_wrapper<ReferencedType>>
	{
		template<typename ...Params>
		NullableRef(Params&& ... params) : std::optional<std::reference_wrapper<ReferencedType>>(std::forward<Params>(params)...) {}

		NullableRef(std::nullopt_t t) : std::optional<std::reference_wrapper<ReferencedType>>(t) {}

		std::remove_reference_t<ReferencedType>* operator->()
		{
			return &(std::optional<std::reference_wrapper<ReferencedType>>::value().get());
		}

		const std::remove_reference_t<ReferencedType>* operator->() const
		{
			return &(std::optional<std::reference_wrapper<ReferencedType>>::value().get());
		}

		std::remove_reference_t<ReferencedType>& operator*()
		{
			return (std::optional<std::reference_wrapper<ReferencedType>>::value().get());
		}

		const std::remove_reference_t<ReferencedType>& operator*() const
		{
			return (std::optional<std::reference_wrapper<ReferencedType>>::value().get());
		}

		template<typename RHS>
		NullableRef& operator=(RHS&& rhs)
		{
			std::optional<std::reference_wrapper<ReferencedType>>::operator=(std::forward<RHS>(rhs));
			return *this;
		}

		//operator std::remove_reference_t<ReferencedType>& () { return (std::optional<std::reference_wrapper<ReferencedType>>::value().get()); }
	};

	template<typename ReferencedType>
	NullableRef<ReferencedType> MakeNullableRef(ReferencedType& ref) { return NullableRef<ReferencedType>(ref); }

	namespace enums
	{

		template<class EnumType, class StorageType = unsigned int>
		class Flags
		{
		public:
			Flags() : value_(0) {}

			Flags(EnumType value) : value_(0)
			{
				Set(value);
			}

			Flags(std::initializer_list<EnumType> values) : value_(0)
			{
				for (auto&& value : values)
				{
					Set(value);
				}
			}

			template<typename T1, typename ... Ts>
			bool Check(T1 first_value, Ts ... values) const
			{
				return Check(first_value) && Check(values...);
			}

			template<>
			bool Check(EnumType check_value) const
			{
				return (value_ & (1 << static_cast<StorageType>(check_value))) != 0;
			}

			template<>
			bool Check(Flags<EnumType> check_flags) const
			{
				return (value_ & (check_flags.value_)) == check_flags.value_;
			}

			template<typename T1, typename ... Ts>
			void Set(T1 first_value, Ts ... values)
			{
				Set(first_value);
				Set(values...);
			}

			template<>
			void Set(EnumType set_value)
			{
				value_ |= (1 << static_cast<StorageType>(set_value));
			}

			template<>
			void Set(Flags<EnumType> set_flags)
			{
				value_ |= set_flags.value_;
			}

		private:
			StorageType value_;
		};

		template<class EnumType>
		EnumType Next(const EnumType& current)
		{
			return static_cast<EnumType>(static_cast<int>(current) + 1);
		}
	}

	struct UniId
	{
		uint32_t value = -1;
	};

	namespace container
	{
		template<typename T>
		class ErVec
		{
			std::vector<T> data_;
			std::vector<uint32_t> ids_;
			uint32_t id_ = 0;
			std::unordered_map<uint32_t, uint32_t> id_to_ind_;
		public:

			struct Id: UniId
			{

				Id() = default;

				Id(uint32_t id)
				{
					value = id;
				}

				Id(UniId id)
				{
					value = id.value;
				}

				operator bool()
				{
					return value != -1;
				}
			};

			template<class T>
			Id Add(T&& t)
			{
				data_.push_back(std::forward<T>(t));
				ids_.push_back(id_);
				id_to_ind_.emplace(id_, data_.size() - 1);
				return { id_++ };
			}

			template<bool T = std::is_default_constructible_v<T>>
			std::enable_if_t < T, Id > Add()
			{
				data_.push_back({});
				ids_.push_back(id_);
				id_to_ind_.emplace(id_, data_.size() - 1);
				return { id_++ };
			}


			T& Get(Id id)
			{
				return data_[id_to_ind_[id.value]];
			}

			bool Remove(Id id)
			{
				if (id.value < id_)
				{
					uint32_t ind = id_to_ind_.at(id.value);
					if (ind < data_.size() - 1)
					{
						data_[ind] = std::move(data_.back());
						ids_[ind] = ids_.back();
						id_to_ind_[ids_[ind]] = ind;
					}
					
					data_.pop_back();
					ids_.pop_back();
					if (data_.size() * 1.5 < data_.capacity())
					{
						data_.shrink_to_fit();
						ids_.shrink_to_fit();
					}

					return true;
				}

				return false;
			}

			const std::vector<T>& GetData() const
			{
				return data_;
			}

			auto begin()
			{
				return data_.begin();
			}

			auto cbegin()
			{
				return data_.cbegin();
			}

			auto end()
			{
				return data_.end();
			}

			auto cend()
			{
				return data_.cend();
			}

		};
	}
}
#endif  // RENDER_ENGINE_RENDER_util_H_
