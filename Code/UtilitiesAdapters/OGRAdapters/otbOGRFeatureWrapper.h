/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbOGRFeatureWrapper_h
#define __otbOGRFeatureWrapper_h

// #include <iosfwd> // std::ostream&
#include <cassert>
#include <boost/shared_ptr.hpp>
#include "itkIndent.h"
#include "otbOGRFieldWrapper.h"

class OGRFeature;
class OGRGeometry;
class OGRFeatureDefn;

namespace otb {
namespace ogr {
class Feature;
bool operator==(Feature const& lhs, Feature const& rhs);

/**\ingroup Geometry
 * \class Feature proxy class.
 * \invariant <tt>m_Feature != 0</tt>
 *
 * \note \c Feature assignment will just make one \c Feature proxy point to
 * another \c OGRFeature. In order to truly assign from one to another, use \c
 * SetFrom.
 */
class Feature
  {
public:
  // no default constructor
  Feature(OGRFeatureDefn & definition);
  Feature(OGRFeature * feature);
  ~Feature();

  Feature clone() const;
  void PrintSelf(std::ostream &os, itk::Indent indent) const;

  OGRFeature & ogr() const;
  OGRFeature & ogr();
  boost::shared_ptr<OGRFeature>      & sptr()       {return m_Feature; }
  boost::shared_ptr<OGRFeature> const& sptr() const {return m_Feature; }

  void SetFrom(Feature const& rhs, bool mustForgive = true);
  void SetFrom(Feature const& rhs, int *map, bool mustForgive = true);

  long GetFID() const;
  void SetFID(long fid);
  OGRFeatureDefn& GetDefn() const;

  /**\name Fields */
  //@{
  size_t GetSize() const;
  Field       operator[](size_t index);
  Field const operator[](size_t index) const;
  Field       operator[](std::string const& name);
  Field const operator[](std::string const& name) const;
  FieldDefn   GetFieldDefn(size_t index) const;
  FieldDefn   GetFieldDefn(std::string const& name) const;
  //@}

  friend bool otb::ogr::operator==(Feature const& lhs, Feature const& rhs);
private:
  void CheckInvariants() const;
  boost::shared_ptr<OGRFeature> m_Feature;
  };


} // ogr namespace
} // otb namespace


#ifndef OTB_MANUAL_INSTANTIATION
#include "otbOGRFeatureWrapper.txx"
#endif

#endif // __otbOGRFeatureWrapper_h
