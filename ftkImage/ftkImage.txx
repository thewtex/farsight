#ifndef _ftkImage_txx
#define _ftkImage_txx
//#include "ftkImage.h"	//This .txx file is included from ftkImage.h!!!
//USAGE: see ftkImage.h

#include <itkFixedArray.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionIterator.h>
#include <itkImportImageContainer.h>

namespace ftk
{

//I NEVER RELEASE MEMORY FOR SLICES!!!
template <typename pixelType> pixelType * Image::GetSlicePtr(int T, int CH, int Z, PtrMode mode)
{
	if( T >= m_Info.numTSlices || CH >= m_Info.numChannels || Z >= m_Info.numZSlices )
		return NULL;
		
	if( !IsMatch<pixelType>(m_Info.dataType) )
		return NULL;

	if( mode == RELEASE_CONTROL)
		return NULL;

	int numPix = (m_Info.numColumns)*(m_Info.numRows);
	pixelType * stack = static_cast<pixelType *>(imageDataPtrs[T][CH].mem);
	pixelType * slice = stack + Z*numPix;
	pixelType * mem;
	if( mode == DEEP_COPY)
	{
		//mem = new pixelType[m_Info.numColumns*m_Info.numRows];
		mem = (pixelType*)malloc(numPix * m_Info.bytesPerPix);
		if(mem == NULL)
			return (pixelType *)NULL;
		memcpy(mem, slice, numPix * m_Info.bytesPerPix);
	}
	else
	{
		mem = slice;
	}
	return mem;
}


template <typename pixelType> typename itk::Image<pixelType, 3>::Pointer Image::GetItkPtr(int T, int CH, PtrMode mode = DEFAULT)
{
	if( !IsMatch<pixelType>(m_Info.dataType) )
		return NULL;
		
	if( T >= m_Info.numTSlices || CH >= m_Info.numChannels )
		return NULL;
		
	bool makeCopy;
	bool itkManageMemory;

	if(mode == RELEASE_CONTROL)
	{
		makeCopy = false;
		itkManageMemory = true;
	}
	else if(mode == DEEP_COPY)
	{
		makeCopy = true;
		itkManageMemory = true;
	}
	else		//DEFAULT
	{
		makeCopy = false;
		itkManageMemory = false;
	}
	
	int numPixels = m_Info.numColumns * m_Info.numRows * m_Info.numZSlices;
	int numBytes = m_Info.bytesPerPix * numPixels;
	
	void * mem = NULL;	//This will point to the data I want to create the array from;
	if(makeCopy)		//Make a copy of the data	
	{
		//mem = (void *)new unsigned char[numBytes];
		mem = malloc(numBytes);
		if(mem == NULL)
			return NULL;
		memcpy(mem,imageDataPtrs[T][CH].mem,numBytes);
	}
	else
	{
		mem = imageDataPtrs[T][CH].mem;
	}
	
	bool letItkManageMemory = false;			//itk DOES NOT manage the memory (default)
	if( itkManageMemory )
	{
		if( imageDataPtrs[T][CH].manager != FTK )	//I can't manage it because someone else already does
		{
			return NULL;
		}
		else
		{
			imageDataPtrs[T][CH].manager = ITK;
			letItkManageMemory = true;	//itk DOES manage the memory
		}
	}
	
	typedef itk::ImportImageContainer<unsigned long, pixelType> ImageContainerType;
	typename ImageContainerType::Pointer container = ImageContainerType::New();
	
	container->Initialize();
	container->Reserve(numPixels);
	container->SetImportPointer( static_cast<pixelType *>(mem), numPixels, letItkManageMemory );

	typedef itk::Image< pixelType, 3 > OutputImageType;
	typename OutputImageType::Pointer image = OutputImageType::New();
	
	typename OutputImageType::PointType origin;
   	origin[0] = 0; 
	origin[1] = 0;    
	origin[2] = 0; 
    image->SetOrigin( origin );
    typename OutputImageType::IndexType start;
    start[0] = 0;  // first index on X
    start[1] = 0;  // first index on Y    
	start[2] = 0;  // first index on Z    
    typename OutputImageType::SizeType  size;
	size[0] = m_Info.numColumns;  // size along X
    size[1] = m_Info.numRows;  // size along Y
	size[2] = m_Info.numZSlices;  // size along Z
    typename OutputImageType::RegionType region;
    region.SetSize( size );
    region.SetIndex( start );
    image->SetRegions( region );
    typename OutputImageType::SpacingType spacing;
    spacing[0] = m_Info.spacing.at(0);
    spacing[1] = m_Info.spacing.at(1);
    spacing[2] = m_Info.spacing.at(2);
    image->SetSpacing(spacing);
	image->Allocate();
	
	image->SetPixelContainer(container);
	image->Update();
		
	return image;
}

template <typename newType> void Image::Cast()
{
	//If I match do nothing
	if( IsMatch<newType>(m_Info.dataType) )
		return;
	
	int x = m_Info.numColumns;
	int y = m_Info.numRows;
	int z = m_Info.numZSlices;
	int n = m_Info.bytesPerPix;
	int numBytes = x*y*z*n;
	
	for (int t=0; t<m_Info.numTSlices; ++t)
	{
		for (int ch=0; ch<m_Info.numChannels; ++ch)
		{
			//Cast existing data to a char for access
			//char *p = static_cast<char *>(imageDataPtrs[t][ch].mem);	//any 8-bit type works here
			
			//Create a new array of the new type to put data into
			//newType *newArray = new newType[numBytes];
			newType *newArray = (newType *)malloc(numBytes);
			if(newArray == NULL)
				return;
			
			for(int k=0; k<z; ++k)
			{
				for(int j=0; j<y; ++j)
				{
					for(int i=0; i<x; ++i)
					{
						newType pix = static_cast<newType>( GetPixel(t,ch,k,j,i) );
						newArray[k*y*x + j*x + i] = pix;
					}
				}
			}
			//delete[] imageDataPtrs[t][ch].mem;
			free( imageDataPtrs[t][ch].mem );
			imageDataPtrs[t][ch].mem = (void *)newArray;
			
		}	//end channels loop
	}	//end t-slices loop
	
	m_Info.bytesPerPix = sizeof(newType);		//Number of bytes per pixel (UCHAR - 1 or USHORT - 2)		
	m_Info.dataType = GetDataType<newType>();	//From enum ImgDataType;
	
}

//IF MULTIPLE T, SAVE THEM IN THEIR OWN FILE:
template <typename TPixel> bool Image::WriteImageITK(int channel, std::string baseName, std::string ext)
{
	if(m_Info.numTSlices == 1)
	{
		std::string fullname = baseName + "." + ext;
		if(!WriteImageITK<TPixel>(fullname, 0, channel))
			return false;
	}
	else
	{
		for(int t=0; t<m_Info.numTSlices; ++t)
		{
			std::string fullname = baseName + "_t" + itoa(t) + "." + ext;
			if(!WriteImageITK<TPixel>(fullname, t, channel))
				return false;
		}
	}
	return true;
}

template <typename TPixel> bool Image::WriteImageITK(std::string fileName, int T, int CH)
{
	//Will not work if wrong pixel type is requested
	if(!IsMatch<TPixel>(m_Info.dataType))
		return false;
		
	if( T >= m_Info.numTSlices || CH >= m_Info.numChannels )
		return false;
		
	typedef itk::ImageBase< 3 >                 ImageBaseType;
	typedef itk::Image< TPixel, 3 >             ImageType;
	typedef itk::ImageFileWriter< ImageType >   WriterType;
	typedef typename WriterType::Pointer        WriterPointer;
	
	WriterPointer writer = WriterType::New(); 
    writer->SetFileName( fileName );
    writer->SetInput( this->GetItkPtr<TPixel>(T, CH) );
    
    try
	{
		writer->Update();
	}
	catch( itk::ExceptionObject & excp )
	{
		std::cerr << excp << std::endl;
		writer = 0;
		return false;
	}
	
	writer = 0;
	return true;
}

template <typename pixelType1> bool Image::IsMatch(DataType pixelType2)
{
	bool retVal = false;
	
	switch(pixelType2)
	{
		case itk::ImageIOBase::CHAR:
			if( typeid(char) == typeid(pixelType1) ) retVal = true;
		break;
		case itk::ImageIOBase::UCHAR:
			if( typeid(unsigned char) == typeid(pixelType1) ) retVal = true;
		break;
		case itk::ImageIOBase::SHORT:
			if( typeid(short) == typeid(pixelType1) ) retVal = true;
		break;
		case itk::ImageIOBase::USHORT:
			if( typeid(unsigned short) == typeid(pixelType1) ) retVal = true;
		break;
		case itk::ImageIOBase::INT:
			if( typeid(int) == typeid(pixelType1) ) retVal = true;
		break;
		case itk::ImageIOBase::UINT:
			if( typeid(unsigned int) == typeid(pixelType1) ) retVal = true;
		break;
		case itk::ImageIOBase::LONG:
			if( typeid(long) == typeid(pixelType1) ) retVal = true;
		break;
		case itk::ImageIOBase::ULONG:
			if( typeid(unsigned long) == typeid(pixelType1) ) retVal = true;
		break;
		case itk::ImageIOBase::FLOAT:
			if( typeid(float) == typeid(pixelType1) ) retVal = true;
		break;
		case itk::ImageIOBase::DOUBLE:
			if( typeid(double) == typeid(pixelType1) ) retVal = true;
		break;
    //just silencing a warning for now...
    case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
    break;
	}
	
	return retVal;	
}

template <typename pixelType> Image::DataType Image::GetDataType()
{
	Image::DataType retVal;

	if( typeid(char) == typeid(pixelType) ) retVal = itk::ImageIOBase::CHAR;
	else if( typeid(unsigned char) == typeid(pixelType) ) retVal = itk::ImageIOBase::UCHAR;
	else if( typeid(short) == typeid(pixelType) ) retVal = itk::ImageIOBase::SHORT;
	else if( typeid(unsigned short) == typeid(pixelType) ) retVal = itk::ImageIOBase::USHORT;
	else if( typeid(int) == typeid(pixelType) ) retVal = itk::ImageIOBase::INT;
	else if( typeid(unsigned int) == typeid(pixelType) ) retVal = itk::ImageIOBase::UINT;
	else if( typeid(long) == typeid(pixelType) ) retVal = itk::ImageIOBase::LONG;
	else if( typeid(unsigned long) == typeid(pixelType) ) retVal = itk::ImageIOBase::ULONG;
	else if( typeid(float) == typeid(pixelType) ) retVal = itk::ImageIOBase::FLOAT;
	else if( typeid(double) == typeid(pixelType) ) retVal = itk::ImageIOBase::DOUBLE;
	else retVal = itk::ImageIOBase::UNKNOWNCOMPONENTTYPE;
	
	return retVal;	
}

//pType is the pixel type to cast to, to retrieve the data from the pointer
//rType is the type to cast to, to return the data to user
template <typename pType, typename rType> rType Image::GetPixelValue(void * p)
{
	pType * pval = static_cast<pType *>(p);
	return static_cast<rType>(*pval);
}

template<typename TComp> void Image::LoadImageITK(std::string fileName, itkPixelType pixType, bool stacksAreForTime)
{
	if(imageDataPtrs.size() > 0)
	{
		if(m_Info.bytesPerPix != sizeof(TComp))
		{
			itk::ExceptionObject excp;
			excp.SetDescription("Component Types do not match");
			throw excp;
			return;
		}
	}

	switch( pixType)
	{
		//All of these types are based on itk::FixedArray (or can be) so I can load them.
		case itk::ImageIOBase::SCALAR:
		case itk::ImageIOBase::RGB:
		case itk::ImageIOBase::RGBA:
		case itk::ImageIOBase::VECTOR:
		case itk::ImageIOBase::POINT:
		case itk::ImageIOBase::COVARIANTVECTOR:
		case itk::ImageIOBase::SYMMETRICSECONDRANKTENSOR:
		case itk::ImageIOBase::DIFFUSIONTENSOR3D:
			LoadImageITK<TComp>( fileName, m_Info.numChannels, stacksAreForTime );
		break;

		case itk::ImageIOBase::OFFSET:
		case itk::ImageIOBase::COMPLEX:
		case itk::ImageIOBase::MATRIX:
		default:
			itk::ExceptionObject excp;
			excp.SetDescription("Pixel type is not supported in this application");
			throw excp;
			return;
		break;
	}
}

//THIS IS AN INTERMEDIATE STEP WHEN A TYPE WITH VARIABLE LENGTH FIELDS IS USED:
template< typename TComp> void Image::LoadImageITK(std::string fileName, unsigned int numChannels, bool stacksAreForTime)
{
	switch(numChannels)
	{
	case 1:
		LoadImageITK< TComp, 1 >( fileName, stacksAreForTime );
	break;
	case 2:
		LoadImageITK< TComp, 2 >( fileName, stacksAreForTime );
	break;
	case 3:
		LoadImageITK< TComp, 3 >( fileName, stacksAreForTime );
	break;
	case 4:
		LoadImageITK< TComp, 4 >( fileName, stacksAreForTime );
	break;
	case 5:
		LoadImageITK< TComp, 5 >( fileName, stacksAreForTime );
	break;
	case 6:
		LoadImageITK< TComp, 6 >( fileName, stacksAreForTime );
	break;
	//NOTE 6 IS MAXIMUM ITK WILL HANDLE
	}
}

//THIS ASSUMES THAT THE IMAGE CAN BE LOADED AS A FIXED ARRAY!!!!
template< typename TComp, unsigned int channels > void Image::LoadImageITK(std::string fileName, bool stacksAreForTime)
{
	typedef itk::FixedArray<TComp,channels>		PixelType;
	typedef itk::Image< PixelType, 3 >			ImageType;
	typedef typename ImageType::Pointer			ImagePointer;		
	typedef itk::ImageFileReader< ImageType >   ReaderType;
	typedef typename ReaderType::Pointer        ReaderPointer;

	ReaderPointer reader = ReaderType::New();
	reader->SetFileName( fileName );
	reader->Update();
	ImagePointer img = reader->GetOutput();

	//Set up the size info of the image:
	typename ImageType::RegionType region = img->GetBufferedRegion();
	typename ImageType::RegionType::IndexType start = region.GetIndex();
	typename ImageType::RegionType::SizeType size = region.GetSize();
	Info nInfo;
	nInfo.numColumns = size[0] - start[0];
	nInfo.numRows = size[1] - start[1];
	nInfo.numZSlices = size[2] - start[2];
	nInfo.numTSlices = 1;
	nInfo.bytesPerPix = sizeof(TComp);		//Already know this matches existing
	nInfo.dataType = m_Info.dataType;		//Preserve this value(set earlier)
	nInfo.numChannels = m_Info.numChannels;	//Preserve this value(set earlier)
	nInfo.spacing = m_Info.spacing;			//Preserve (set earlier)

	if(stacksAreForTime)	//Switch the Z and T numbers:
	{
		nInfo.numTSlices = nInfo.numZSlices;
		nInfo.numZSlices  = 1;
	}

	unsigned short startingT = 0;
	if(imageDataPtrs.size() > 0)		//Already have data
	{
		//Check Size to be sure match:
		if(m_Info.BytesPerChunk() != nInfo.BytesPerChunk())
		{
			itk::ExceptionObject excp;
			excp.SetDescription("Image Sizes do not match");
			throw excp;
			return;
		}
		startingT = m_Info.numTSlices;	//Number of existing T slices changes
	}
	else	//Don't already have data
	{
		m_Info = nInfo;		//Set the Image Info
		this->SetDefaultColors();
	}
	
	//Change the number of Time slices if I'm adding some, and resize the vector:
	m_Info.numTSlices = startingT + nInfo.numTSlices;
	imageDataPtrs.resize(m_Info.numTSlices);

	unsigned int numBytesPerChunk = m_Info.BytesPerChunk();

	//Create a pointer for each channel and time slice (allocate memory)
	ImageMemoryBlock block;
	block.manager = FTK;
	for(int t=startingT; t<m_Info.numTSlices; ++t)
	{
		for(int c=0; c<m_Info.numChannels; ++c)
		{
			//block.mem = (void *)(new TComp[numBytesPerChunk]);
			void * mem = malloc(numBytesPerChunk);
			if(mem == NULL)
				return;
			block.mem = mem;
			imageDataPtrs[t].push_back(block);
		}
	}

	//Iterate through the input image and extract time and channel images:
	typedef itk::ImageRegionConstIterator< ImageType > IteratorType;
	IteratorType it( img, img->GetRequestedRegion() );
	unsigned int b = 0;
	unsigned short t = startingT;
	for ( it.GoToBegin(); !it.IsAtEnd(); ++it) 
	{
		//create a pixel object & Get each channel value
		typename ImageType::PixelType pixelValue = it.Get();
		for(int c=0; c<m_Info.numChannels; ++c)
		{
			TComp *toLoc = ((TComp*)(imageDataPtrs[t][c].mem));
			toLoc[b] = (TComp)pixelValue[c];
		}

		//Update pointers
		b++;
		if(b>=numBytesPerChunk)	//I've finished this time slice
		{
			b=0;
			t++;
		}
	}
	img = 0;		//itk smartpointer cleans itself	
}

}  // end namespace ftk
#endif
