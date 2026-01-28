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

#ifndef otbTupleOptionalIterator_h
#define otbTupleOptionalIterator_h

// Work around ZipIterator that expects iterators of the same type, and that
// ZipIterator<variant<....>> will be much too complex to define...
// => with this one tuple of optional<ZipIterator>...

#include "OTBCommonExport.h"
#include "otbMetaProgrammingLibrary.h"
#include "otbAlgoTuple.h"
#include <boost/optional.hpp>

#include <boost/hana.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/ext/std/tuple.hpp>
#include <stdexcept>
#include <tuple>
namespace otb
{

namespace details
{

template <typename Xs, typename Command>
inline
constexpr void try_for_each(Xs&& xs, Command&& to_try) {
  auto f = [&](auto& opt) { if (opt) to_try(*opt); };
  boost::hana::for_each(std::forward<Xs>(xs), f);
}

template <typename T>
struct remove_optional
{
  using type = T;
};

template <typename T>
struct remove_optional<boost::optional<T>>
{
  using type = T;
};

template <typename T>
using remove_optional_t = typename remove_optional<remove_cvref_t<T>>::type;

} // details namespace

/**
 * Kind of _zip iterator_ for iterators of different types.
 * While `otb::ZipIterator<>` works well for a list of same-type iterators, it doesn't support the
 * case where different kind of iterators are zipped together. A solution would have been to have a
 * `otb::ZipIterator<VariantIterator<TIterator...>>`.
 *
 * Alas a few services are still missing in the
 * current flavour of `otb::ZipIterator`:
 * - There is no way to disable some definitions when they cannot be forwarded uniformally from the
 * effective iterator type: and defining those common types and definitions from a
 * `Variant<Iterators...>` is really tricky.
 *
 * Hence `TupleOfOptionalIterators` which is a tuple of optional iterators.
 *
 * \tparam TIterator  List of iterator types that'll be encapsulated into optionals.
 *
 * \invariant All defined iterators shall share the same region, position...
 * \invariant At least one iterator shall be defined.
 * \todo disable Get<IteratorType> when all iterators share the same type.
 * \author Luc Hermitte (CS Group)
 * \copyright CNES
 * \sa `otb::TupleIterator` and `otb::ZipIterator`
 */
template <typename... TIterator>
class OTBCommon_EXPORT_TEMPLATE TupleOfOptionalIterators
{
  using FrontIterator = otb::FrontType<TIterator...>;

public:
  /**
   * Construct from a list of iterators.
   * \tparam TITKIterator  May be real ITK iterators or `optional<ITKIterator>`
   * \param[in] iterator   List of may-be-iterators to track in the tuple.
   *
   * \throw std::runtime_error if no iterator is defined: not all optionals can be _empty_.
   */
  template <typename... TITKIterator>
  TupleOfOptionalIterators(TITKIterator&&... iterator)
  : m_iterators(std::forward<TITKIterator>(iterator)...)
  {
    static_assert(
        sizeof...(TITKIterator) == sizeof...(TIterator),
        "Number of parameters shall match the number of iterators declared"
    );
    bool const at_least_one_is_set = tuple::find_and_apply(
        m_iterators,
        [](auto const& opt) { return !!opt;},
        [](auto const&    ) { return true; },
        []()                { return false; }
    );
    if (!at_least_one_is_set)
      throw std::runtime_error("No iterator is defined!");
  }

  // static_assert TIterator::ImageIteratorDimension is the same for all

  using Self = TupleOfOptionalIterators;

  static constexpr unsigned int ImageIteratorDimension = FrontIterator::ImageIteratorDimension;
  using IndexType  = typename FrontIterator::IndexType;
  using SizeType   = typename FrontIterator::SizeType;
  using OffsetType = typename FrontIterator::OffsetType;
  using RegionType = typename FrontIterator::RegionType;

  TupleOfOptionalIterators() = default;

  /** Get the region iterated by the iterator(s).
   * Actually, only check the first iterator defined. The others are expected to watch the same
   * region.
   * \pre at least one iterator shall be defined (invariant)
   */
  auto const& GetRegion() const
  {
    return tuple::find_and_apply(
        m_iterators,
        [](auto const& opt) { return !!opt;},
        [](auto const& opt) { return opt->GetRegion(); },
        []() -> RegionType  { throw std::logic_error("TupleOfOptionalIterators::GetRegion(): No iterator is set!"); }
    );
  }

  /** Set the region iterated by the iterator(s). */
  void SetRegion(RegionType const& region)
  {
    auto const set_region = [&](auto const& it){ it.SetRegion(region); };
    details::try_for_each(m_iterators, set_region);
  }

  /** Moves the iterator(s) to the begin of the region iterated. */
  Self& GoToBegin()
  {
    details::try_for_each(m_iterators, [](auto& it) { it.GoToBegin(); });
    return *this;
  }
  /** Moves the iterator(s) to the end of the region iterated. */
  Self& GoToEnd()
  {
    details::try_for_each(m_iterators, [](auto& it) { it.GoToEnd(); });
    return *this;
  }

  /** Tells whether the iterator(s) is at the begin of the region iterated.
   * Actually, only check the first iterator defined. The others are expected to be at the same
   * position.
   * \pre at least one iterator shall be defined (invariant)
   */
  bool IsAtBegin() const
  {
    return tuple::find_and_apply(
        m_iterators,
        [](auto const& opt) { return !!opt;},
        [](auto const& opt) { return opt->IsAtBegin(); },
        []() -> bool  { throw std::logic_error("TupleOfOptionalIterators::IsAtBegin(): No iterator is set!"); }
    );
  }

  /** Tells the iterator(s) is at the end of the region iterated.
   * Actually, only check the first iterator defined. The others are expected to be at the same
   * position.
   * \pre at least one iterator shall be defined (invariant)
   */
  bool IsAtEnd() const
  {
    return tuple::find_and_apply(
        m_iterators,
        [](auto const& opt) { return !!opt;},
        [](auto const& opt) { return opt->IsAtEnd(); },
        []() -> bool { throw std::logic_error("TupleOfOptionalIterators::IsAtEnd(): No iterator is set!"); }
    );
  }

  /** Pre-increment the iterator(s).
   * As post-increment is less efficient, it hasn't been provided.
   */
  Self& operator++()
  {
    assert(!IsAtEnd());
    details::try_for_each(m_iterators, [](auto& it) { ++it; });
    return *this;
  }
  /** Removed post-increment operator(s).
   * Please use the  preincrement operator!
   */
  Self operator++(int) = delete;

  /**\name ScanLine Iterator Interface */
  //@{
  /** Moves iterator(s) to next line. */
  Self& NextLine()
  {
    details::try_for_each(m_iterators, [](auto& it) { it.NextLine(); });
    return *this;
  }
  /** Moves iterator(s) to the beginning of the current line. */
  Self& GoToBeginOfLine()
  {
    details::try_for_each(m_iterators, [](auto& it) { it.GoToBeginOfLine(); });
    return *this;
  }
  /** Moves iterator(s) to the end of the current line. */
  Self& GoToEndOfLine()
  {
    details::try_for_each(m_iterators, [](auto& it) { it.GoToEndOfLine(); });
    return *this;
  }
  /** Tells whether the iterator(s) is a the end of a line.
   * Actually, only check the first iterator defined. The others are expected to be at the same
   * position.
   * \pre at least one iterator shall be defined (invariant)
   */
  bool IsAtEndOfLine() const
  {
    return tuple::find_and_apply(
        m_iterators,
        [](auto const& opt) { return !!opt;},
        [](auto const& opt) { return opt->IsAtEndOfLine(); },
        []() -> bool { throw std::logic_error("TupleOfOptionalIterators::IsAtEndOfLine(): No iterator is set!"); }
    );
#if 0
    assert(!m_iterators.empty());
    // Const qualifier has been added to ScanLineIterator::IsAtEndOfLine in ITK
    // 5.1 => Use const_cast in the mean time...
    return const_cast<typename ImageIteratorList_t::value_type &>(m_iterators.front()).IsAtEndOfLine();
#endif
  }
  //@}

  /**\name Pixel accessor Interface */
  //@{
  /**
   * Return pixel value from iterator type.
   * \tparam WhichIterator  Unique type identitying the iterator in the tuple.
   * \warning this overload cannot be used when several iterators have the same type -- which may
   *          happen when each iterator type comes from a different template parameters of the OTB
   *          filter where it's used.
   *
   * \return the exact value/reference to the underlying pixel
   * \throw std::logic_error if not iterator is set. Actually `[[unreachable]]` would be more exact
   *                         given the class invariant.
   */
  template <typename WhichIterator>
  decltype(auto) Get() const
  {
    auto&& opt_iterator = std::get<boost::optional<WhichIterator>>(m_iterators);
    if (opt_iterator)
      return opt_iterator->Get();
    throw std::logic_error("TupleOfOptionalIterators::Get<It>(): No iterator is set!");
  }

  /**
   * Return pixel value from its index.
   * \tparam Index compiled-time used to identify the actual iterator used.
   * \note use this overload cannot be used when several iterators have the same type -- which may
   *       happen when each iterator type comes from a different template parameters of the OTB
   *       filter where it's used.
   *
   * \return the exact value/reference to the underlying pixel
   * \throw std::logic_error if not iterator is set. Actually `[[unreachable]]` would be more exact
   *                         given the class invariant.
   *
   * \note don't forget to call the function with `it.template Get<0>()` when defining (template)
   *       generic filter as the function is a _dependent template name_.
   */
  template <std::size_t Index>
  decltype(auto) Get() const
  {
    auto&& opt_iterator = std::get<Index>(m_iterators);
    if (opt_iterator)
      return opt_iterator->Get();
    throw std::logic_error("TupleOfOptionalIterators::Get<Idx>(): No iterator is set!");
  }

  /**
   * Set pixel value from iterator type.
   * \tparam WhichIterator  Unique type identitying the iterator in the tuple.
   * \tparam TValue         Type on data that needs to be convertible to what `WhichIterator::Set()`
   *                        expects.
   * \warning this overload cannot be used when several iterators have the same type -- which may
   *          happen when each iterator type comes from a different template parameters of the OTB
   *          filter where it's used.
   * \param[in] value  Value to assign to the pixel
   * \note if the underlying `optional<WhichIterator>` is unset, nothing is done
   *
   * \throw Neutral This function will forward whatever `WhichIterator::Set()` may throw
   */
  template <typename WhichIterator, typename TValue>
  void Set(TValue const& value)
  {
    auto&& opt_iterator = std::get<boost::optional<WhichIterator>>(m_iterators);
    if (opt_iterator)
      opt_iterator->Set(value);
  }

  /**
   * Set pixel value from its index.
   * \tparam Index  Compiled-time used to identify the actual iterator used.
   * \tparam TValue Type on data that needs to be convertible to what `WhichIterator::Set()`
   *                expects.
   * \note use this overload cannot be used when several iterators have the same type -- which may
   *       happen when each iterator type comes from a different template parameters of the OTB
   *       filter where it's used.
   * \param[in] value  Value to assign to the pixel
   * \note if the underlying `optional<Iterator<Index>>` is unset, nothing is done
   *
   * \throw Neutral This function will forward whatever `m_iterators<Index>::Set()` may throw
   */
  template <std::size_t Index, typename TValue>
  void Set(TValue const& value)
  {
    auto&& opt_iterator = std::get<Index>(m_iterators);
    if (opt_iterator)
      opt_iterator->Set(value);
  }
  //@}
private:
  std::tuple<boost::optional<TIterator>...> m_iterators;
};

/**
 * Helper function to create `TupleOfOptionalIterators`.
 * Workaround lack of CTAD in C++14
 *
 * \tparam TIterator    Auto-deduced effective type of the may-be-optional iterators.
 *                      May be real ITK iterators or `optional<ITKIterator>`.
 * \param[in] iterator  List of may-be-iterators to track in the tuple.
 *
 * \return An instance of `otb::TupleOfOptionalIterators<non-optional iterator_types>`
 * \note the optional nature of the iterator parameters is removed from the type returned.
 * \throw std::runtime_error if no iterator is defined: not all optionals can be _empty_.
 */
template <typename... TIterator>
inline
auto make_optional_iterator_tuple(TIterator&& ... iterator)
{
  return TupleOfOptionalIterators<details::remove_optional_t<TIterator>...>(
      std::forward<TIterator>(iterator)...
  );
}

}  // otb namespace

#endif  // otbTupleOptionalIterator_h
