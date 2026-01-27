/*
 * Copyright (C) 2005-2026 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**@file otbAlgoTuple.h
 * Defines algorithms that can be applied on tuple-like structures.
 */

#ifndef otbAlgoTuple_h
#define otbAlgoTuple_h

#include <utility> // std::forward

#include <boost/hana.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/ext/std/tuple.hpp>

namespace otb
{
namespace tuple
{

/*-------------------------------[ find_and_transform ]----------------------*/

namespace details
{

template <typename Tuple, std::size_t N, std::size_t I>
struct find_and_apply_impl
{
  template <typename TransformStrategy, typename Predicate, typename ElseStrategy>
  static auto apply(Tuple&& tp, Predicate p, TransformStrategy f, ElseStrategy el)
  {
    auto&& crt = std::get<I>(std::forward<Tuple>(tp));
    if (p(crt))
      return f(crt);
    return find_and_apply_impl<Tuple, N, I + 1u>::apply(std::forward<Tuple>(tp), p, f, el);
  }
};

template <class Tuple, std::size_t N>
struct find_and_apply_impl<Tuple, N, N>
{
  template <typename TransformStrategy, typename Predicate, typename ElseStrategy>
  static auto apply(Tuple&& tp, Predicate p, TransformStrategy f, ElseStrategy el)
  {
    auto&& crt = std::get<0>(std::forward<Tuple>(tp));
    if (p(crt))
      return f(std::forward<decltype(crt)>(crt));
    return el();
  }
};

} // details namespace

/**
 * Finds the first tuple-element matching a predicate and returns its value transformed.
 *
 * \tparam Tuple             Any tuple-like structure as long as it's compatible with `std::get<42>()`
 * \tparam Predicate         Predicate type, expected to evaluate as bool
 * \tparam TransformStrategy Type of the transformation applied on the element found
 * \tparam ElseStrategy      Type of the object-function called when there is no match
 *
 * \param[in] tp  tuple where element are searched
 * \param[in] p   predicate used for the search
 * \param[in] f   transformation to apply
 * \param[in] el  object-function executed when nothing is found matching the predicate. Can either
 *                throw or return a default value.
 *
 * \return `f(e)` such as `e` is the first element of `tp`  for which `p(e)` is true.
 * \throw Whatever `el` may throw if no matching element can be found. Exceptions for `p` or `f`
 *                 will also be forwarded, but keep in mind these _object functions_ aren't expected
 *                 to throw anything.
 * \note The transformation strategy is required to unify the type of the elements returned by this
 *       function.
 *
 * \todo Overload with a flavour without a transformation strategy for the case where all elements
 * have the same type.
 */
template <class Tuple, typename Predicate, typename TransformStrategy, typename ElseStrategy>
auto find_and_apply(Tuple&& tp, Predicate p, TransformStrategy f, ElseStrategy el)
{
  constexpr auto N = std::tuple_size<std::decay_t<Tuple>>::value;
  return details::find_and_apply_impl<Tuple, N, 0u>::apply(std::forward<Tuple>(tp), p, f, el);
}

/*-------------------------------[ for_each ]--------------------------------*/

/**
 * Applies a command on all elements from a a tuple.
 *
 * \tparam Tuple    Tuple-like structure
 * \tparam Command  Command type
 * \param[in] tp   Tuple on which the command is applied
 * \param[in] cmd  Command to apply
 *
 * \throw Neutral Throws whatever `cmd` may throw
 * \note just a wrapper around `boost::hana::for_each`
 */
template <typename Tuple, typename Command>
constexpr void for_each(Tuple&& tp, Command&& cmd)
{
  boost::hana::for_each(std::forward<Tuple>(tp), cmd);
}

} // tuple namespace
} // otb namespace

#endif // otbAlgoTuple_h
