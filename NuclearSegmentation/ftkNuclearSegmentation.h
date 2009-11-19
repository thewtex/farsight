/* 
 * Copyright 2009 Rensselaer Polytechnic Institute
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*=========================================================================

  Program:   Farsight Biological Image Segmentation and Visualization Toolkit
  Language:  C++
  Date:      $Date:  $
  Version:   $Revision: 0.00 $

=========================================================================*/
#ifndef __ftkNuclearSegmentation_h
#define __ftkNuclearSegmentation_h

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <ftkImage/ftkImage.h>
#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>
#include <vtkVariantArray.h>
#include <vtkTable.h>
#include <ftkCommon/ftkLabelImageToFeatures.h>
#include <ftkCommon/ftkUtils.h>
#include <yousef_core/yousef_seg.h>
#include <tinyxml/tinyxml.h>
#include <ftkCommon/ftkObject.h>
#include <map>
#include <set>

namespace ftk
{ 
/** \class NuclearSegmentation
 *  \brief For storage of a complete nuclear segmentation 
 *   
 *  Handles the execution, result, and editing of a nuclear segmentation
 *  
 */
typedef unsigned char IPixelT;
typedef unsigned short LPixelT;
typedef ftk::LabelImageToFeatures< IPixelT, LPixelT, 3 > FeatureCalcType;

class NuclearSegmentation
{
public:
	NuclearSegmentation();
	~NuclearSegmentation();

	//This is for beginning a completely new segmenation:
	bool SetInputs(std::string datafile, std::string paramfile);		
	void SetChannel(int number){channelNumber = number;};		//Set the channel number to use for segmentation
	bool LoadData();											//Will load the data image into memory
	bool Binarize(bool getResultImg = true);					//Will binarize the data image
	bool DetectSeeds(bool getResultImg = true);					//If binarization has been done it will detect seeds
	bool RunClustering();										//Will use binary image and seeds to do initial clustering
	bool Finalize();											//Will finilize the output using alpha expansion
	void ReleaseSegMemory();									//Delete the NucleusSeg object to release all of its memory.

	//Segmentation is basically done at this point (hopefully), now move on to calculating the features and classification:
	bool ComputeFeatures(void);									//Compute Intrinsic Features and create vtkTable
	bool LoadAssociationsFromFile(std::string fName);			//Add the Associative Features to the objects

	//Save functions:
	bool SaveChanges(std::string filename);						//Save changes made to label image, features table, project def file(xml)
	bool SaveResultImage();										//Save the output image of the last step executed (image format)
	bool SaveLabelByClass();									//Will save a different label image for each class

	//Save features in various other supported formats:
	bool WriteToMETA(std::string filename);
	bool WriteToLibSVM(std::string filename);
	
	//We may also want to restore from previously found results:
	bool RestoreFromXML(std::string filename);					//Complete Restore from FORMER XML file format
	bool LoadAll(std::string filename);							//Complete Restore from NEW FORMAT

	//OTHER LOAD METHEDS
	bool LoadFromImages(std::string dfile, std::string rfile);	//Load from images -> then convert to objects
	bool LoadFromDAT(std::string dfile, std::string rfile);		//Load from .dat -> then convert to objects

	//Editing Functions 
	bool EditsNotSaved(void){ return this->editsNotSaved; };
	std::vector< int > Split(ftk::Object::Point P1, ftk::Object::Point P2);
	std::vector< int > SplitAlongZ(int objID, int cutSlice);
	int Merge(vector<int> ids);
	bool Delete(vector<int> ids);
	bool Exclude(int xy, int z);
	int AddObject(int x1, int y1, int z1, int x2, int y2, int z2);
	bool SetClass(vector<int> ids, int clss);
	bool MarkAsVisited(vector<int> ids, int val);

	//Edits applied on initial segmentation and updates LoG resp image
	ftk::Object::Point MergeInit(ftk::Object::Point P1, ftk::Object::Point P2, int* new_id); 
	std::vector< int > SplitInit(ftk::Object::Point P1, ftk::Object::Point P2);
	bool DeleteInit(ftk::Object::Point P1);

	//Misc string Gets
	std::string GetErrorMessage() { return errorMessage; };
	std::string GetDataFilename() { return dataFilename; };
	std::string GetParamFilename() { return paramFilename; };
	std::string GetLabelFilename() { return labelFilename; };

	//Get Data:
	std::vector<std::string> GetFeatureNames();	//Extract Feature Names from the table!!!
	vtkSmartPointer<vtkTable> GetFeatureTable() { return featureTable; };
	ftk::Image::Pointer GetDataImage(void){ return dataImage; };	
	ftk::Image::Pointer GetLabelImage(void){ return labelImage; };
	//*********************************************************************************************

	//ADDED BY YOUSEF/RAGHAV:
	std::vector<std::string> RunGraphColoring(std::string labelname, std::string filename);	//Run Graph coloring on label image and save adjacency file as filename
	std::vector<Seed> getSeeds();

protected:
	std::string dataFilename;	//the filename of the data image		(full path)
	std::string labelFilename;	//the filename of the label image		(full path)
	std::string paramFilename;	//the filename of the parameter file	(full path)
	std::string featureFilename;//the filename of the feature txt file	(full path)
	std::string headerFilename; //the filename of the feature names txt (full path)
	std::string editFilename;	//the filename of the edit record txt	(full path)

	std::string errorMessage;

	ftk::Image::Pointer dataImage;		//The data image
	int channelNumber;					//Use this channel from the dataImage for segmentation
	ftk::Image::Pointer labelImage;		//My label image
	yousef_nucleus_seg *NucleusSeg;		//The Nuclear Segmentation module
	int lastRunStep;					//0,1,2,3,4 for the stages in a nuclear segmentation.
	bool editsNotSaved;					//Will be true if edits have been made and not saved to file.

	typedef struct { string name; int value; } Parameter;				
	std::vector<Parameter> myParameters;
	std::vector<ftk::Object::EditRecord> myEditRecords;
	std::map<int, ftk::Object::Box>		bBoxMap;			//Bounding boxes
	std::map<int, ftk::Object::Point>	centerMap;			//Centroids
	//std::map<int, int>	idToRowMap;						//Mapping from ID to row in table!!!!
	vtkSmartPointer<vtkTable> featureTable;

	void GetParameters(void);								//Retrieve the Parameters from nuclear segmentation.
	void ResetAll(void);									//Clear all memory and variables

	//Saving Utilities:
	bool GetResultImage();										//Gets the result of last module and puts it in labelImage
	bool SaveFeaturesTable();									//Save 2 txt files: one with features other with names of features
	bool SaveEditRecords();										//Append Edit Records to file 
	bool LoadLabel(bool updateMaps = false);					//Load just the label image if the filename is already known
	bool LoadFeatures();

	//Editing Utilities:
	long int maxID(void);										//Get the maximum ID in the table!
	int rowForID(int id);										//Iterate through table and get row for ID:
	void removeFeatures(int ID);
	bool addObjectToTable(int ID, int x1, int y1, int z1, int x2, int y2, int z2);
	bool addObjectsToTable(std::set<int> IDs, int x1, int y1, int z1, int x2, int y2, int z2);
	FeatureCalcType::Pointer computeFeatures(int x1, int y1, int z1, int x2, int y2, int z2);
	void ReassignLabels(std::vector<int> fromIds, int toId);
	void ReassignLabel(int fromId, int toId);
	ftk::Object::Box ExtremaBox(std::vector<int> ids);
	
	//Utilities for parsing XML (on read) (kept for legacy reasons):
	Object parseObject(TiXmlElement *object);
	Object::Point parseCenter(TiXmlElement *centerElement);
	Object::Box parseBound(TiXmlElement *boundElement);
	void parseFeaturesToTable(int id, TiXmlElement *featureElement);

	//FOR PRINTING SEEDS IMAGE:
	void Cleandptr(unsigned short*x,vector<int> y );
	void Restoredptr(unsigned short* );
	std::list<int> negativeseeds;

//********************************************************************************************
//********************************************************************************************

}; // end NuclearSegmentation

}  // end namespace ftk

#endif	// end __ftkNuclearSegmentation_h

