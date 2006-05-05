/*=========================================================================

  Program   :   OTB (ORFEO ToolBox)
  Authors   :   CNES - J.Inglada
  Language  :   C++
  Date      :   28 April 2006
  Version   :   
  Role      :   Test for the SVMClassifier class (instanciation) 
  $Id$

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <fstream>

#include "itkPoint.h"
#include "itkPointSet.h"
#include "itkVectorImage.h"

#include "itkPointSetToListAdaptor.h"
#include "itkSubsample.h"
#include "itkListSample.h"
#include "otbSVMClassifier.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkRescaleIntensityImageFilter.h"



int otbSVMClassifierImage(int argc, char* argv[] )
{

  try
    {
    namespace stat = itk::Statistics ;
 
    if (argc != 4)
      {
      std::cout << "Usage : " << argv[0] << " inputImage modelFile outputImage" 
                << std::endl ;
      return EXIT_FAILURE;
      }

    const char * imageFilename  = argv[1];
    const char * modelFilename  = argv[2];
    const char * outputFilename = argv[3];
       


    /** Read the input image and build the sample */

    typedef double                               InputPixelType;
    typedef std::vector<InputPixelType>          InputVectorType;
    typedef int                                LabelPixelType;


    const   unsigned int        	         Dimension = 2;

    typedef itk::VectorImage< InputPixelType,  Dimension >	InputImageType;


    typedef otb::ImageFileReader< InputImageType  >         ReaderType;


    ReaderType::Pointer reader = ReaderType::New();


    reader->SetFileName( imageFilename  );


    reader->Update();


    typedef itk::PointSet< InputVectorType,  Dimension >
                                                       MeasurePointSetType;


    MeasurePointSetType::Pointer mPSet = MeasurePointSetType::New();

    typedef MeasurePointSetType::PointType    MeasurePointType;

    typedef MeasurePointSetType::PointsContainer      MeasurePointsContainer;

    MeasurePointsContainer::Pointer mCont = MeasurePointsContainer::New();


    typedef itk::ImageRegionIterator< InputImageType>  InputIteratorType;
    InputIteratorType  inIt( reader->GetOutput(),
			     reader->GetOutput()->GetBufferedRegion() );
    
    inIt.GoToBegin();
    

    unsigned int pointId;
    while(!inIt.IsAtEnd() )
      {

      MeasurePointType mP;

      mP[0] = pointId;
      mP[1] = pointId;


      InputVectorType measure; 
      //measure.push_back(pow(pointId,2.0));
      measure.push_back(static_cast<InputPixelType>(2.0*pointId));
      measure.push_back(static_cast<InputPixelType>(-10));


      mCont->InsertElement( pointId , mP );
      mPSet->SetPointData( pointId, measure );   


      ++inIt;
      ++pointId;
      }


    mPSet->SetPoints( mCont );

    std::cout << "PointSet built" << std::endl;  

    typedef itk::Statistics::PointSetToListAdaptor< MeasurePointSetType >
      SampleType;
    SampleType::Pointer sample = SampleType::New();
    sample->SetPointSet( mPSet );


    


    /** preparing classifier and decision rule object */
    typedef otb::SVMModel< InputPixelType, LabelPixelType > ModelType;

    ModelType::Pointer model = ModelType::New();

    model->LoadModel( modelFilename );

    int numberOfClasses = model->GetNumberOfClasses();
    
    typedef otb::SVMClassifier< SampleType, LabelPixelType > ClassifierType ;

    ClassifierType::Pointer classifier = ClassifierType::New() ;
  
    classifier->SetNumberOfClasses(numberOfClasses) ;
    classifier->SetModel( model );
    classifier->SetSample(sample.GetPointer()) ;
    classifier->Update() ;

    /* Build the class map */
    std::cout << "Output image creation" << std::endl;

    typedef ClassifierType::ClassLabelType	                 OutputPixelType;
    typedef itk::Image< OutputPixelType, Dimension >        OutputImageType;

    
    OutputImageType::Pointer outputImage = OutputImageType::New();

    typedef itk::Index<Dimension>         myIndexType;
    typedef itk::Size<Dimension>          mySizeType;
    typedef itk::ImageRegion<Dimension>        myRegionType;

    mySizeType size;
    size[0] = reader->GetOutput()->GetRequestedRegion().GetSize()[0];
    size[1] = reader->GetOutput()->GetRequestedRegion().GetSize()[1];

    myIndexType start;
    start[0] = 0;
    start[1] = 0;

    myRegionType region;
    region.SetIndex( start );
    region.SetSize( size );

    outputImage->SetRegions( region );
    outputImage->Allocate();

    
    std::cout << "classifier get output" << std::endl;  
    ClassifierType::OutputType* membershipSample =
      classifier->GetOutput() ;
    std::cout << "Sample iterators" << std::endl;  
    ClassifierType::OutputType::ConstIterator m_iter =
      membershipSample->Begin() ;
    ClassifierType::OutputType::ConstIterator m_last =
      membershipSample->End() ;

    std::cout << "Image iterator" << std::endl;  
    typedef itk::ImageRegionIterator< OutputImageType>  OutputIteratorType;
    OutputIteratorType  outIt( outputImage,
			   outputImage->GetBufferedRegion() );

    outIt.GoToBegin();


    std::cout << "Iteration for output image = " << (membershipSample->Size()) << std::endl;  

    while (m_iter != m_last && !outIt.IsAtEnd())
    {
    outIt.Set(m_iter.GetClassLabel());
    ++m_iter ;
    ++outIt;
    }


    typedef itk::Image< unsigned char, Dimension >        FileImageType;

    
    typedef itk::RescaleIntensityImageFilter< OutputImageType,
      FileImageType > RescalerType;

    RescalerType::Pointer rescaler = RescalerType::New();
    
    rescaler->SetOutputMinimum( itk::NumericTraits< unsigned char >::min());
    rescaler->SetOutputMaximum( itk::NumericTraits< unsigned char >::max());

    rescaler->SetInput( outputImage );

    typedef otb::ImageFileWriter< FileImageType >         WriterType;
	
    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName( outputFilename  );
    writer->SetInput( rescaler->GetOutput() );
    
    writer->Update();
    

    }
  catch( itk::ExceptionObject & err ) 
    { 
    std::cout << "Exception itk::ExceptionObject levee !" << std::endl; 
    std::cout << err << std::endl; 
    return EXIT_FAILURE;
    } 
  catch( ... ) 
    { 
    std::cout << "Unknown exception !" << std::endl; 
    return EXIT_FAILURE;
    } 
 
  return EXIT_SUCCESS;
}







