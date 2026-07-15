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

#ifndef otbMPLImageTypeExtractorTraits_h
#define otbMPLImageTypeExtractorTraits_h

// Forward declare itk::SmartPointer
namespace itk {

template <typename TObjectType>
class SmartPointer;

} // itk namespace

namespace otb
{

/** Traits to extract actual image type.
 * Will be secialized for
 * - pointers: `T* --> T`
 * - ITK smart pointers: `SmartPointer<T> --> T`
 * - anything else : `T --> T`
 */
template <typename I> struct get_image_type
{ using type = I; };

template <typename I> struct get_image_type<I*>
{ using type = I; };

template <typename O> struct get_image_type<itk::SmartPointer<O>>
{ using type = O; };

/** Simplified access typedef to traits returning actual image type. */
template <typename I>
using get_image_type_t = typename get_image_type<I>::type;

}

#endif // otbMPLImageTypeExtractorTraits_h
