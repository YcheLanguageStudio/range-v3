//  Copyright Neil Groves 2009.
//  Copyright Eric Niebler 2013
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//
#ifndef RANGES_V3_ALGORITHM_EQUAL_RANGE_HPP
#define RANGES_V3_ALGORITHM_EQUAL_RANGE_HPP

#include <utility>
#include <algorithm>
#include <range/v3/begin_end.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/iterator_range.hpp>
#include <range/v3/utility/bindable.hpp>
#include <range/v3/utility/invokable.hpp>

namespace ranges
{
    inline namespace v3
    {
        struct equal_ranger : bindable<equal_ranger>
        {
            /// \brief template function \c equal_ranger::operator()
            ///
            /// range-based version of the \c equal_range std algorithm
            ///
            /// \pre \c ForwardRange is a model of the ForwardRange concept
            /// \pre \c BinaryPredicate is a model of the BinaryPredicate concept
            template<typename ForwardRange, typename Value,
                CONCEPT_REQUIRES(ranges::Range<ForwardRange>() &&
                                 ranges::LessThanComparable<Value const &, range_reference_t<ForwardRange>>() &&
                                 ranges::LessThanComparable<range_reference_t<ForwardRange>, Value const &>())>
            static iterator_range<range_iterator_t<ForwardRange>>
            invoke(equal_ranger, ForwardRange && rng, Value const & val)
            {
                CONCEPT_ASSERT(ranges::ForwardRange<ForwardRange>());
                return std::equal_range(ranges::begin(rng), ranges::end(rng), val);
            }

            /// \overload
            template<typename ForwardRange, typename Value, typename BinaryPredicate>
            static iterator_range<range_iterator_t<ForwardRange>>
            invoke(equal_ranger, ForwardRange && rng, Value const & val, BinaryPredicate pred)
            {
                CONCEPT_ASSERT(ranges::ForwardRange<ForwardRange>());
                CONCEPT_ASSERT(ranges::BinaryPredicate<invokable_t<BinaryPredicate>,
                                                       range_reference_t<ForwardRange>,
                                                       Value const &>());
                return std::equal_range(ranges::begin(rng), ranges::end(rng), val,
                    ranges::make_invokable(std::move(pred)));
            }

            /// \overload
            /// for rng | equal_range(val)
            template<typename Value>
            static auto invoke(equal_ranger equal_range, Value && val)
                -> decltype(equal_range(std::placeholders::_1, std::forward<Value>(val)))
            {
                return equal_range(std::placeholders::_1, std::forward<Value>(val));
            }

            /// \overload
            /// for rng | equal_range(val, pred)
            template<typename Value, typename BinaryPredicate,
                CONCEPT_REQUIRES(
                    !(ranges::Range<Value>() &&
                      ranges::LessThanComparable<BinaryPredicate, range_reference_t<Value>>() &&
                      ranges::LessThanComparable<range_reference_t<Value>, BinaryPredicate>()))>
            static auto invoke(equal_ranger equal_range, Value && val, BinaryPredicate pred)
                -> decltype(equal_range(std::placeholders::_1, std::forward<Value>(val),
                    std::move(pred)))
            {
                return equal_range(std::placeholders::_1, std::forward<Value>(val),
                    std::move(pred));
            }
        };

        RANGES_CONSTEXPR equal_ranger equal_range {};

    } // inline namespace v3

} // namespace ranges

#endif // include guard