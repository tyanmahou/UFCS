#pragma once
#include<type_traits>
#include<tuple>

/*
* Unified Function Call Syntax on C++
*/
namespace ufcs
{
	namespace detail
	{
		template<class Func, class... TupleArgs, class... Args>
		auto param_expand_impl(Func func, [[maybe_unused]] const std::tuple<TupleArgs...>& tuple, Args&&... args)->
			std::enable_if_t<sizeof...(Args) == sizeof...(TupleArgs), std::invoke_result_t<Func, TupleArgs...>>
		{
			return func(args...);
		}
		template<class Func, class... TupleArgs, class... Args>
		auto param_expand_impl(Func func, [[maybe_unused]] const std::tuple<TupleArgs...>& tuple, Args&&... args)->
			std::enable_if_t < sizeof...(Args) < sizeof...(TupleArgs), std::invoke_result_t<Func, TupleArgs...> >
		{
			return param_expand_impl(func, tuple, args..., std::get<sizeof...(Args)>(tuple));
		}
	}

	template<class Func, class Arg>
	decltype(auto) param_expand(Func func, Arg&& arg)
	{
		return detail::param_expand_impl(func, std::forward<Arg>(arg));
	}

	template<class Base, class... Args>
	struct TupleParam {
		std::tuple<Args...> m_param;
	};
}

///<summary>
///method chain
///<summary>
template<class T, class Base, class... Args>
decltype(auto) operator >> (T&& v, ufcs::TupleParam<Base, Args...> tuple)
{
	auto f = [&](auto&&... args) {
		return Base()(std::forward<T>(v), std::forward<decltype(args)>(args)...);
	};
	return ufcs::param_expand(f, tuple.m_param);
}

#define USE_UFCS(funcName,...)\
constexpr struct funcName##_op\
{\
	template<class T,class... Args>\
	auto operator()(T&& v, Args&&...args)const ->decltype(v.funcName(std::forward<Args>(args)...))\
	{\
		return v.funcName(std::forward<Args>(args)...);\
	}\
	template<class T,class... Args>\
	auto operator()(T&& v, Args&&...args)const ->decltype(__VA_ARGS__ funcName(std::forward<T>(v),std::forward<Args>(args)...))\
	{\
		return __VA_ARGS__ funcName(std::forward<T>(v),std::forward<Args>(args)...);\
	}\
	template<class... Args>\
	ufcs::TupleParam<funcName##_op,Args...> operator()(Args&&... args)const\
	{\
		return { std::forward<Args>(args)... };\
	}\
} funcName##_
