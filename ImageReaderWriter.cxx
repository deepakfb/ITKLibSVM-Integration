#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkImageToImageFilter.h"
#include "itkRGBToLuminanceImageFilter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "ImageReaderWriter.h"

// Reads image and labels into featurevectors and labels. Ignores background (value - 0).
bool readImageandLabels(char* imagepath, char* labelpath, float** labels, float*** featurevectors, int* width, int* height, int* numannotations)
{
	typedef itk::Image<itk::RGBPixel<unsigned char>, 2> RGBImageType;
	typedef itk::Image<unsigned char,2>    	  ImageType;
	typedef itk::ImageFileReader<ImageType> 	  ReaderType;
	typedef itk::ImageFileReader<RGBImageType> 	  RGBReaderType;

	ReaderType::Pointer reader = ReaderType::New();
	RGBReaderType::Pointer rgbreader = RGBReaderType::New();

	RGBImageType::Pointer rgbinput;
	ImageType::Pointer input;

	typedef itk::  RGBToLuminanceImageFilter<
  				RGBImageType,ImageType>     RGBGrayFilterType;
	RGBGrayFilterType::Pointer rgbGrayFilter = RGBGrayFilterType::New();

	int numvectors = 0;
	try
	{
		// Read labels in RGB and convert to gray
		  rgbreader->SetFileName( labelpath );
		  rgbreader->Update();
		  rgbGrayFilter->SetInput( rgbreader->GetOutput() );
		  rgbGrayFilter->Update();
		  input = rgbGrayFilter->GetOutput();
		// End of labels

		  rgbreader->SetFileName( imagepath );
		  rgbreader->Update();
		  rgbinput = rgbreader->GetOutput();

		  RGBImageType::SizeType inputSize = rgbinput->GetLargestPossibleRegion().GetSize();
		  std::cout << "Input size: " << inputSize << std::endl;

		  *width = inputSize[0];
		  *height = inputSize[1];

		  *labels = (float*)malloc(sizeof(float));
		  *featurevectors =  (float**)malloc(sizeof(float*));

		  RGBImageType::PixelType rgbPixel;
		  ImageType::PixelType pixel;

		  for(int i=0;i<inputSize[0];i++)
		  {
			  for(int j=0;j<inputSize[1];j++)
			  {
				   RGBImageType::IndexType rgbPixelIndex;
				   ImageType::IndexType pixelIndex;

 				   rgbPixelIndex[0] = i;
				   rgbPixelIndex[1] = j;
				   rgbPixel = rgbinput->GetPixel(rgbPixelIndex);
				   int blue = rgbPixel.GetBlue();
				   int green = rgbPixel.GetGreen();
				   int red = rgbPixel.GetRed();


				   pixelIndex[0] = i;
				   pixelIndex[1] = j;
				   pixel = input->GetPixel(pixelIndex);
				   int lblvalue = pixel;

				   if(lblvalue!=BGVALUE)
				   {
					   if((i==0 && j==0) || (numvectors ==0))
					   {
						   (*labels)[0] = lblvalue;
						   (*featurevectors)[0] = (float*)malloc(3*sizeof(float));
						   (*featurevectors)[numvectors][0] = ((float)red)/255;;
						   (*featurevectors)[numvectors][1] = ((float)green)/255;;
						   (*featurevectors)[numvectors][2] = ((float)blue)/255;;

						   ++numvectors;
					   }
					   else
					   {
						   ++numvectors;
						   *labels = (float*) realloc(*labels, numvectors*sizeof(float));
						   (*labels)[numvectors -1] = lblvalue;

						   *featurevectors = (float**) realloc(*featurevectors, numvectors*sizeof(float*));
						   (*featurevectors)[numvectors-1] = (float*)malloc(3*sizeof(float));
						   (*featurevectors)[numvectors-1][0] = ((float)red)/255;
						   (*featurevectors)[numvectors-1][1] = ((float)green)/255;
						   (*featurevectors)[numvectors-1][2] = ((float)blue)/255;

						   //++numvectors;
					   }
				   }
			  }
		  }
	}
	catch(itk::ExceptionObject & err)
	{
	std::cerr << "ExceptionObject caught !" << std::endl;
	std::cerr << err << std::endl;
	return false;
	}
	*numannotations = numvectors;
	return true;
}


bool readImage(char* imagepath, float*** featurevectors, int* width, int* height)
{
	typedef itk::Image<itk::RGBPixel<unsigned char>, 2> RGBImageType;
	typedef itk::ImageFileReader<RGBImageType> 	  RGBReaderType;

	RGBReaderType::Pointer rgbreader = RGBReaderType::New();

	RGBImageType::Pointer rgbinput;


	int numvectors = 0;
	try
	{

		  rgbreader->SetFileName( imagepath );
		  rgbreader->Update();
		  rgbinput = rgbreader->GetOutput();

		  RGBImageType::SizeType inputSize = rgbinput->GetLargestPossibleRegion().GetSize();
		  std::cout << "Input size: " << inputSize << std::endl;

		  *width = inputSize[0];
		  *height = inputSize[1];

		  *featurevectors =  (float**)malloc(sizeof(float*));

		  RGBImageType::PixelType rgbPixel;

		for(int i=0;i<inputSize[0];i++)
		{
			for(int j=0;j<inputSize[1];j++)
			{
				RGBImageType::IndexType rgbPixelIndex;

 				rgbPixelIndex[0] = i;
				rgbPixelIndex[1] = j;
				rgbPixel = rgbinput->GetPixel(rgbPixelIndex);
				int blue = rgbPixel.GetBlue();
				int green = rgbPixel.GetGreen();
				int red = rgbPixel.GetRed();


				if((i==0 && j==0) || (numvectors ==0))
				{
					(*featurevectors)[0] = (float*)malloc(3*sizeof(float));
					(*featurevectors)[numvectors][0] = ((float)red)/255;
					(*featurevectors)[numvectors][1] = ((float)green)/255;
					(*featurevectors)[numvectors][2] = ((float)blue)/255;

					++numvectors;
				}
				else
				{
					++numvectors;

					*featurevectors = (float**) realloc(*featurevectors, numvectors*sizeof(float*));
					(*featurevectors)[numvectors-1] = (float*)malloc(3*sizeof(float));
					(*featurevectors)[numvectors-1][0] = ((float)red)/255;
					(*featurevectors)[numvectors-1][1] = ((float)green)/255;
					(*featurevectors)[numvectors-1][2] = ((float)blue)/255;
				}
			}
		}
	}
	catch(itk::ExceptionObject & err)
	{
	std::cerr << "ExceptionObject caught !" << std::endl;
	std::cerr << err << std::endl;
	return false;
	}
	return true;
}


bool writeLabels(float* labels, int width, int height, char* outputfilename)
{
	typedef itk::Image<unsigned char,2>    	  ImageType;
	ImageType::Pointer output;

	try
	{
		  ImageType::PixelType pixel;

		  //create an empty label output image
		  output = ImageType::New();
		  ImageType::IndexType start;
		  start.Fill(0);
		  ImageType::SizeType size;
		  size[0] = width;
		  size[1] = height;

		  ImageType::RegionType region(start,size);

		   output->SetRegions(region);
		   output->Allocate();
		   output->FillBuffer( itk::NumericTraits< ImageType::PixelType >::Zero);

		  //end of empty label image

		  int labelindex = 0;
		  for(int i=0;i<width;i++)
		  {
			  for(int j=0;j<height;j++)
			  {
				   ImageType::IndexType pixelIndex;
				   pixelIndex[0] = i;
				   pixelIndex[1] = j;

				   ImageType::PixelType pixel;
				   pixel = labels[labelindex];

				   output->SetPixel(pixelIndex, pixel);
				   ++labelindex;
			  }
		  }

		  // write label image
		typedef itk::ImageFileWriter<ImageType>	  WriterType;

		WriterType::Pointer writer = WriterType::New();
		writer->SetInput(output);
		writer->SetFileName(outputfilename);
		writer->Update();
	}
	  catch(itk::ExceptionObject & err)
  {
	std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return false;
  }
	  return true;
}
