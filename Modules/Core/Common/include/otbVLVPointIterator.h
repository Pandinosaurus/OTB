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

#ifndef otbVLVPointVLVPointIterator_h
#define otbVLVPointVLVPointIterator_h

#include "OTBCommonExport.h"
#include "itkVariableLengthVector.h"
#include <iterator>
#include <type_traits>
#include <cassert>

namespace otb
{

/**
 * C++ standard compliant iterator for contiguous arrays of
 * `itk::VariableLengthVector`.
 * This minimalist iterator permits the building block to define iterable
 * ranges over set of `itk::VariableLengthVector` that can then be used in C++
 * standards algorithms.
 *
 * \internal The important part is that `+1` needs to take the number of
 * components/bands into account.
 * \tparam T  sub-pixel data type
 */
template <typename T>
struct OTBCommon_EXPORT_TEMPLATE VLVPointIterator
{
  using iterator_category = std::random_access_iterator_tag;
  using value_type        = T;
  using difference_type   = std::ptrdiff_t;
  using pointer           = T*;
  using reference         = itk::VariableLengthVector<typename std::remove_cv<T>::type>;

  explicit VLVPointIterator(T* p, unsigned nb_bands) noexcept
  : m_p(p), m_nb_bands(nb_bands)
  {}

  VLVPointIterator& operator+=(difference_type o) noexcept
  {
    m_p += o * m_nb_bands;
    return *this;
  }
  friend VLVPointIterator operator+(VLVPointIterator it, difference_type o) noexcept
  {
    it += o;
    return it;
  }
  friend VLVPointIterator operator+(difference_type o, VLVPointIterator it) noexcept
  {
    it += o;
    return it;
  }

  VLVPointIterator& operator-=(difference_type o) noexcept
  {
    m_p -= o * m_nb_bands;
    return *this;
  }
  friend VLVPointIterator operator-(VLVPointIterator it, difference_type o) noexcept
  {
    it -= o;
    return it;
  }

  friend difference_type operator-(VLVPointIterator const& lhs, VLVPointIterator const& rhs) noexcept
  {
    assert(lhs.m_nb_bands == rhs.m_nb_bands);
    return (lhs.m_p - rhs.m_p) / lhs.m_nb_bands;
  }

  VLVPointIterator& operator++() noexcept
  {
    m_p += m_nb_bands;
    return *this;
  }
  VLVPointIterator operator++(int) noexcept
  {
    auto tmp = *this;
    m_p += m_nb_bands;
    return tmp;
  }

  VLVPointIterator& operator--() noexcept
  {
    m_p -= m_nb_bands;
    return *this;
  }
  VLVPointIterator operator--(int) noexcept
  {
    auto tmp = *this;
    m_p -= m_nb_bands;
    return tmp;
  }

#if 0
  T& operator[](unsigned p) noexcept {
    assert(p < m_nb_bands);
    return m_p[p];
  }
  T const& operator[](unsigned p) const noexcept {
    assert(p < m_nb_bands);
    return m_p[p];
  }
#endif

  reference operator*() const noexcept
  {
    return itk::VariableLengthVector<typename std::remove_cv<T>::type>(m_p, m_nb_bands);
  }

  decltype(auto) data() const noexcept { return m_p; }

  friend bool operator==(VLVPointIterator const& rhs, VLVPointIterator const& lhs) noexcept
  { return rhs.m_p == lhs.m_p; }
  friend bool operator!=(VLVPointIterator const& rhs, VLVPointIterator const& lhs) noexcept
  { return rhs.m_p != lhs.m_p; }
  friend bool operator<=(VLVPointIterator const& rhs, VLVPointIterator const& lhs) noexcept
  { return rhs.m_p <= lhs.m_p; }
  friend bool operator<(VLVPointIterator const& rhs, VLVPointIterator const& lhs) noexcept
  { return rhs.m_p < lhs.m_p; }
  friend bool operator>(VLVPointIterator const& rhs, VLVPointIterator const& lhs) noexcept
  { return rhs.m_p > lhs.m_p; }
  friend bool operator>=(VLVPointIterator const& rhs, VLVPointIterator const& lhs) noexcept
  { return rhs.m_p >= lhs.m_p; }

  // itk::VariableLengthVector<typename std::remove_cv<T>::type> operator*() noexcept {
  //   return itk::VariableLengthVector<T>(m_p, m_nb_bands);
  // }

private:
  T*       m_p;
  unsigned m_nb_bands;
};

}  // otb namespace

#endif  // otbVLVPointVLVPointIterator_h
