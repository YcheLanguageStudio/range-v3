// Boost.Range library
//
//  Copyright Eric Niebler 2013.
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Acknowledgements: Thanks for Paul Fultz for the suggestions that
//                   concepts can be ordinary Boolean metafunctions.
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef RANGES_V3_UTILITY_CONCEPTS_HPP
#define RANGES_V3_UTILITY_CONCEPTS_HPP

#include <utility>
#include <type_traits>
#include <range/v3/utility/swap.hpp>
#include <range/v3/utility/typelist.hpp>
#include <range/v3/utility/logical_ops.hpp>

namespace ranges
{
    inline namespace v3
    {
        namespace detail
        {
            constexpr struct void_tester
            {
                template<typename T>
                friend int operator,(T&&, void_tester);
            } void_ {};

            constexpr struct is_void_t
            {
                int operator()(detail::void_tester) const;
            } is_void {};

            constexpr struct valid_expr_t
            {
                template<typename ...T>
                std::true_type operator()(T &&...) const;
            } valid_expr {};

            constexpr struct same_type_t
            {
                template<typename T, typename U>
                auto operator()(T &&, U &&) const ->
                    typename std::enable_if<std::is_same<T,U>::value, int>::type;
            } same_type {};

            constexpr struct is_true_t
            {
                template<typename Bool>
                auto operator()(Bool) const ->
                    typename std::enable_if<Bool::value, int>::type;
            } is_true {};

            constexpr struct is_false_t
            {
                template<typename Bool>
                auto operator()(Bool) const ->
                    typename std::enable_if<!Bool::value, int>::type;
            } is_false {};

            template<typename Concept, typename ...Ts>
            struct models_;

            template<typename Concept>
            struct base_concept
            {
                using type = Concept;
            };

            template<typename Concept, typename ...Args>
            struct base_concept<Concept(Args...)>
            {
                using type = Concept;
            };
        }

        namespace concepts
        {
            using detail::void_;
            using detail::is_void;
            using detail::valid_expr;
            using detail::same_type;
            using detail::is_true;
            using detail::is_false;

            using _1 = std::integral_constant<int, 0>;
            using _2 = std::integral_constant<int, 1>;
            using _3 = std::integral_constant<int, 2>;
            using _4 = std::integral_constant<int, 3>;
            using _5 = std::integral_constant<int, 4>;
            using _6 = std::integral_constant<int, 5>;
            using _7 = std::integral_constant<int, 6>;
            using _8 = std::integral_constant<int, 7>;
            using _9 = std::integral_constant<int, 8>;

            template<typename Ret, typename T>
            Ret returns_(T const &);

            template<typename T, typename U>
            auto convertible_to(U && u) ->
                decltype(concepts::returns_<int>(static_cast<T>(u)));

            template<typename T, typename U>
            auto has_common_type(T && t, U && u) ->
                decltype(true ? static_cast<T &&>(t) : static_cast<U &&>(u));

            template<typename T, typename U>
            auto has_type(U &&) ->
                typename std::enable_if<std::is_same<T,U>::value, int>::type;

            template<typename ...Concepts>
            struct refines
              : detail::base_concept<Concepts>::type...
            {};

            template<typename Concept, typename ...Ts>
            using models = decltype(detail::models_<Concept, Ts...>{}());

            template<typename Concept, typename ...Ts>
            auto model_of(Ts &&...) ->
                typename std::enable_if<(concepts::models<Concept, Ts...>()), int>::type;
        }

        namespace detail
        {
            ////////////////////////////////////////////////////////////////////////////////////
            // models_
            template<typename Concept, typename ...Ts>
            struct models_
            {
            private:
                static std::true_type test_refines(void *); // No refinements, ok
                template<typename ...Bases>
                static auto test_refines(concepts::refines<Bases...> *) ->
                    logical_and<static_cast<bool>(concepts::models<Bases, Ts...>())...>;
            public:
                std::false_type operator()() const;
                template<typename C = Concept>
                auto operator()() ->
                    std::integral_constant<bool, (decltype(C{}.requires(std::declval<Ts>()...))() &&
                           decltype(models_::test_refines((C *)nullptr))())>;
            };

            template<typename Concept, typename...Args, typename ...Ts>
            struct models_<Concept(Args...), Ts...>
              : models_<Concept, typelist_element_t<Args::value, typelist<Ts...>>...>
            {};

            typelist<> base_concepts_of_impl_(void *);

            template<typename...BaseConcepts>
            typelist<BaseConcepts...> base_concepts_of_impl_(concepts::refines<BaseConcepts...> *);

            template<typename Concept>
            using base_concepts_of_t = decltype(detail::base_concepts_of_impl_((Concept *)nullptr));

            struct not_a_concept
            {};

            template<typename...Ts>
            struct most_refined_impl_
            {
                static not_a_concept invoke(typelist<> *);

                template<typename Head, typename...Tail, typename Impl = most_refined_impl_>
                static auto invoke(typelist<Head, Tail...> *) ->
                    typename std::conditional<
                        (concepts::models<Head, Ts...>()),
                        Head,
                        decltype(Impl::invoke(
                            (typelist_concat_t<typelist<Tail...>, base_concepts_of_t<Head>> *)nullptr))
                    >::type;
            };
        }

        namespace concepts
        {
            using detail::not_a_concept;

            ////////////////////////////////////////////////////////////////////////////////////////////
            // most_refined_t
            template<typename Concept, typename...Ts>
            using most_refined_t =
                decltype(detail::most_refined_impl_<Ts...>::invoke((typelist<Concept> *)nullptr));

            template<typename Concept, typename...Ts>
            struct most_refined
            {
                using type = most_refined_t<Concept, Ts...>;
            };

            ////////////////////////////////////////////////////////////////////////////////////////////
            // requires_t
            template<bool Requires>
            using requires_t =
                typename std::enable_if<Requires>::type;

            struct SameType
            {
                template<typename T, typename U>
                auto requires(T && t, U && u) -> decltype(
                    concepts::valid_expr(
                        concepts::same_type(t, u)
                    ));
            };

            struct Convertible
            {
                template<typename T, typename U>
                auto requires(T && t, U && u) -> decltype(
                    concepts::valid_expr(
                        concepts::convertible_to<U>(t)
                    ));
            };

            struct Integral
            {
                template<typename T>
                auto requires(T &&) -> decltype(
                    concepts::valid_expr(
                        concepts::is_true(std::is_integral<T>{})
                    ));
            };

            struct SignedIntegral
              : refines<Integral>
            {
                template<typename T>
                auto requires(T &&) -> decltype(
                    concepts::valid_expr(
                        concepts::is_true(std::is_signed<T>{})
                    ));
            };

            struct Destructible
            {
                template<typename T>
                auto requires(T && t) -> decltype(
                    concepts::valid_expr(
                        (t.~T(), 42)
                    ));
            };

            struct DefaultConstructible
            {
                template<typename T>
                auto requires(T && t) -> decltype(
                    concepts::valid_expr(
                        T{}
                    ));
            };

            struct CopyConstructible
            {
                template<typename T>
                auto requires(T && t) -> decltype(
                    concepts::valid_expr(
                        T(t)
                    ));
            };

            struct MoveConstructible
            {
                template<typename T>
                auto requires(T && t) -> decltype(
                    concepts::valid_expr(
                        T(std::move(t))
                    ));
            };

            struct CopyAssignable
            {
                template<typename T>
                auto requires(T && t) -> decltype(
                    concepts::valid_expr(
                        t = t
                    ));
            };

            struct MoveAssignable
            {
                template<typename T>
                auto requires(T && t) -> decltype(
                    concepts::valid_expr(
                        t = std::move(t)
                    ));
            };

            struct EqualityComparable
            {
                template<typename T>
                auto requires(T && t) -> decltype(
                    concepts::valid_expr(
                        concepts::convertible_to<bool>(t == t),
                        concepts::convertible_to<bool>(t != t)
                    ));

                template<typename T, typename U>
                auto requires(T && t, U && u) -> decltype(
                    concepts::valid_expr(
                        concepts::convertible_to<bool>(t == u),
                        concepts::convertible_to<bool>(t != u)
                    ));
            };

            struct LessThanComparable
            {
                template<typename T>
                auto requires(T && t) -> decltype(
                    concepts::valid_expr(
                        concepts::convertible_to<bool>(t < t)
                    ));

                template<typename T, typename U>
                auto requires(T && t, U && u) -> decltype(
                    concepts::valid_expr(
                        concepts::convertible_to<bool>(t < u)
                    ));
            };

            struct Orderable
            {
                template<typename T>
                auto requires(T && t) -> decltype(
                    concepts::valid_expr(
                        concepts::convertible_to<bool>(t < t),
                        concepts::convertible_to<bool>(t > t),
                        concepts::convertible_to<bool>(t <= t),
                        concepts::convertible_to<bool>(t >= t)
                    ));
            };

            struct Swappable
            {
                template<typename T>
                auto requires(T && t) -> decltype(
                    concepts::valid_expr(
                        ranges::swap((T&&)t, (T&&)t)
                    ));

                template<typename T, typename U>
                auto requires(T && t, U && u) -> decltype(
                    concepts::valid_expr(
                        ranges::swap((T&&)t, (U&&)u),
                        ranges::swap((U&&)u, (T&&)t)
                    ));
            };

            struct Callable
            {
                template<typename Fun, typename ...Args>
                using result_t = decltype(std::declval<Fun>()(std::declval<Args>()...));

                template<typename Fun, typename ...Args>
                auto requires(Fun && fun, Args &&... args) -> decltype(
                    concepts::valid_expr(
                        (static_cast<void>(std::forward<Fun>(fun)(
                            std::forward<Args>(args)...)), 42)
                    ));
            };

            struct Predicate
              : refines<Callable>
            {
                template<typename Fun, typename ...Args>
                auto requires(Fun && fun, Args &&... args) -> decltype(
                    concepts::valid_expr(
                        concepts::convertible_to<bool>(
                            std::forward<Fun>(fun)(std::forward<Args>(args)...))
                    ));
            };

            struct UnaryPredicate
              : refines<Predicate>
            {
                template<typename Fun, typename Arg>
                auto requires(Fun && fun, Arg && arg) -> decltype(
                    concepts::valid_expr(
                        std::forward<Fun>(fun)(std::forward<Arg>(arg))
                    ));
            };

            struct BinaryPredicate
              : refines<Predicate>
            {
                template<typename Fun, typename Arg0, typename Arg1>
                auto requires(Fun && fun, Arg0 && arg0, Arg1 && arg1) -> decltype(
                    concepts::valid_expr(
                        std::forward<Fun>(fun)(std::forward<Arg0>(arg0),
                                                  std::forward<Arg1>(arg1))
                    ));
            };
        }

        template<typename T, typename U>
        using SameType = concepts::models<concepts::SameType, T, U>;

        template<typename T, typename U>
        using Convertible = concepts::models<concepts::Convertible, T, U>;

        template<typename T>
        using Integral = concepts::models<concepts::Integral, T>;

        template<typename T>
        using SignedIntegral = concepts::models<concepts::SignedIntegral, T>;

        template<typename T>
        using CopyConstructible = concepts::models<concepts::CopyConstructible, T>;

        template<typename T>
        using MoveConstructible = concepts::models<concepts::MoveConstructible, T>;

        template<typename T>
        using DefaultConstructible = concepts::models<concepts::DefaultConstructible, T>;

        template<typename T>
        using Destructible = concepts::models<concepts::Destructible, T>;

        template<typename T>
        using CopyAssignable = concepts::models<concepts::CopyAssignable, T>;

        template<typename T>
        using MoveAssignable = concepts::models<concepts::MoveAssignable, T>;

        template<typename T, typename U = T>
        using EqualityComparable = concepts::models<concepts::EqualityComparable, T, U>;

        template<typename T, typename U = T>
        using LessThanComparable = concepts::models<concepts::LessThanComparable, T, U>;

        template<typename T>
        using Orderable = concepts::models<concepts::Orderable, T>;

        template<typename T, typename U = T>
        using Swappable = concepts::models<concepts::Swappable, T, U>;

        template<typename Fun, typename ...Args>
        using Callable = concepts::models<concepts::Callable, Fun, Args...>;

        template<typename Fun, typename ...Args>
        using Predicate = concepts::models<concepts::Predicate, Fun, Args...>;

        template<typename Fun, typename Arg>
        using UnaryPredicate = concepts::models<concepts::UnaryPredicate, Fun, Arg>;

        template<typename Fun, typename Arg0, typename Arg1>
        using BinaryPredicate = concepts::models<concepts::BinaryPredicate, Fun, Arg0, Arg1>;
    }
}

#define CONCEPT_REQUIRES(...) typename = ranges::concepts::requires_t<(__VA_ARGS__)>
#define CONCEPT_ASSERT(...) static_assert((__VA_ARGS__), "Concept check failed");

#endif // RANGES_V3_UTILITY_CONCEPTS_HPP