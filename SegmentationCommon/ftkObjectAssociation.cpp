/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/

/*=========================================================================

  Program:   Farsight Biological Image Segmentation and Visualization Toolkit
  Language:  C++
  Date:      $Date:  $
  Version:   $Revision: 0.00 $

=========================================================================*/
#include "ftkObjectAssociation.h"
#include <iostream>
#include <fstream>
#include <iomanip>

namespace ftk 
{	

/* The constructor of the Association Rule Class */
AssociationRule::AssociationRule(string name)
{
	SetRuleName(name);	
	segFileName = "";
	targFileName = "";
	outsideDistance = 0;
	insideDistance = 0;
	useWholeObject = false;
	assocType = ASSOC_AVERAGE;	
}

/* From here, we start defining the member functions of the ObjectAssociation class */
ObjectAssociation::ObjectAssociation(string AssocFName, int numOfRules)
{
	segImageName = AssocFName;
	numOfAssocRules = numOfRules;		
	assocMeasurementsList=NULL;
	numOfLabels=0;
}

/* Add association rules to the list of rules */
void ObjectAssociation::AddAssociation(string ruleName,string targFileName, int outsideDistance, int insideDistance,	bool useAllObject, int assocType)
{
	AssociationRule *assocRule = new AssociationRule(ruleName);
	assocRule->SetSegmentationFileNmae(segImageName);
	assocRule->SetTargetFileNmae(targFileName);
	assocRule->SetOutDistance(outsideDistance);
	assocRule->SetInDistance(insideDistance);
	assocRule->SetUseWholeObject(useAllObject);
	switch(assocType)
	{
	case 1:
		assocRule->SetAssocType(ASSOC_MIN);
		break;
	case 2:
		assocRule->SetAssocType(ASSOC_MAX);
		break;
	case 3:
		assocRule->SetAssocType(ASSOC_TOTAL);
		break;
	case 4:
		assocRule->SetAssocType(ASSOC_AVERAGE);
		break;
	default:
		assocRule->SetAssocType(ASSOC_AVERAGE);
	}
	assocRulesList.push_back(*assocRule);
	
}

/* Write the defined Association Rules into an XML file */
void ObjectAssociation::WriteRulesToXML(string xmlFname)
{
	TiXmlDocument doc;   
 
	//Root node
	TiXmlElement * root = new TiXmlElement( "ObjectAssociationRules" );  
	doc.LinkEndChild( root );  
	root->SetAttribute("SegmentationSource", segImageName.c_str());
	root->SetAttribute("NumberOfAssociativeMeasures", numOfAssocRules);

	TiXmlComment * comment = new TiXmlComment();
	comment->SetValue(" Definition of Association Rules between different objects " );  
	root->LinkEndChild( comment );  

	//Add the Association Rules one by one
	for(int i=0; i<numOfAssocRules; i++)
	{
		TiXmlElement *element = new TiXmlElement("AssociationRule");
		element->SetAttribute("Name",assocRulesList[i].GetRuleName().c_str());
		element->SetAttribute("Target_Image",assocRulesList[i].GetTargetFileNmae().c_str());
		element->SetAttribute("Outside_Distance",assocRulesList[i].GetOutDistance());
		element->SetAttribute("Inside_Distance",assocRulesList[i].GetInDistance());
		if(assocRulesList[i].IsUseWholeObject())
			element->SetAttribute("Use_Whole_Object","True");
		else
			element->SetAttribute("Use_Whole_Object","False");

		switch(assocRulesList[i].GetAssocType())
		{
		case ASSOC_MIN:
			element->SetAttribute("Association_Type","MIN");
			break;
		case ASSOC_MAX:
			element->SetAttribute("Association_Type","MAX");
			break;
		case ASSOC_TOTAL:
			element->SetAttribute("Association_Type","TOTAL");
			break;
		case ASSOC_AVERAGE:
			element->SetAttribute("Association_Type","AVERAGE");
			break;
		default:
			element->SetAttribute("Association_Type","AVERAGE");
		}

		root->LinkEndChild(element);
	}
	
	doc.SaveFile( xmlFname.c_str() );
}

/* Read the defined Association Rules from an XML file */
int ObjectAssociation::ReadRulesFromXML(string xmlFname)
{
	//open the xml file
	TiXmlDocument doc;
	if ( !doc.LoadFile( xmlFname.c_str() ) )
	{
		cout<<"Unable to load XML File";
		return 0;
	}

	//get the root node (exit on error)
	TiXmlElement* rootElement = doc.FirstChildElement();
	const char* docname = rootElement->Value();
	if ( strcmp( docname, "ObjectAssociationRules" ) != 0 )
	{
		cout<<"Incorrect XML root Element";		
		return 0;
	}
		
	//get the segmentation image name and the number of association rulese (exit on error)
	segImageName = rootElement->Attribute("SegmentationSource");
	numOfAssocRules = atoi(rootElement->Attribute("NumberOfAssociativeMeasures"));
	if(numOfAssocRules<=0)
	{
		cout<<"Incorrect number of association rules";		
		return 0;
	}
	

	//now get the rules one by one
	TiXmlElement* parentElement = rootElement->FirstChildElement();
	while (parentElement)
	{		
		const char * parent = parentElement->Value();

		if ( strcmp( parent, "AssociationRule" ) != 0 )
		{
			cout<<"The XML file format is incorrect!";
			return 0;
		}
		//get the attributes one by one
		TiXmlAttribute *atrib = parentElement->FirstAttribute();
		if(strcmp(atrib->Name(),"Name")!=0)
		{
			cout<<"First attribute in an Association rule must be its name";
			return 0;
		}
		int numAttribs = 6;
		//an association rule object
		AssociationRule *assocRule = new AssociationRule(atrib->ValueStr());		
		while(atrib)
		{							
			if(strcmp(atrib->Name(),"Name")==0) 
			{
				//Processed above
				//assocRulesList[i].SetRuleName(atrib->ValueStr());
				numAttribs--;
			}
			else if(strcmp(atrib->Name(),"Target_Image")==0)
			{
				assocRule->SetTargetFileNmae(atrib->ValueStr());
				numAttribs--;
			}
			else if(strcmp(atrib->Name(),"Outside_Distance")==0)
			{
				assocRule->SetOutDistance(atoi(atrib->ValueStr().c_str()));
				numAttribs--;
			}
			else if(strcmp(atrib->Name(),"Inside_Distance")==0)
			{
				assocRule->SetInDistance(atoi(atrib->ValueStr().c_str()));
				numAttribs--;
			}
			else if(strcmp(atrib->Name(),"Use_Whole_Object")==0)
			{
				const char* V = atrib->Value();
				if(strcmp(V,"True")==0)
					assocRule->SetUseWholeObject(true);
				else
					assocRule->SetUseWholeObject(false);

				numAttribs--;
			}
			else if(strcmp(atrib->Name(),"Association_Type")==0)
			{
				const char* V = atrib->Value();
				if(strcmp(V,"MIN")==0)
					assocRule->SetAssocType(ASSOC_MIN);
				else if(strcmp(V,"MAX")==0)
					assocRule->SetAssocType(ASSOC_MAX);
				else if(strcmp(V,"TOTAL")==0)
					assocRule->SetAssocType(ASSOC_TOTAL);
				else 
					assocRule->SetAssocType(ASSOC_AVERAGE);

				numAttribs--;
			}			
			//go to the next attribute
			atrib = atrib->Next();
		}

		//if you have a wrong number of attributes, then exit
		if(numAttribs != 0)
		{
			cout<<"The XML file has incorrect format";
			return 0;
		}

		//add the assocuation rule to the association rules list
		assocRulesList.push_back(*assocRule);

		//go to the next association rule element
		parentElement = parentElement->NextSiblingElement();
	} 
	
	return 1;
}

/* Write the computer Associative features of all the objects to an XML file */
void ObjectAssociation::WriteAssociativeFeaturesToXML(string xmlFname)
{
	TiXmlDocument doc;   
 
	//Root node
	TiXmlElement * root = new TiXmlElement( "ObjectAssociationRules" );  
	doc.LinkEndChild( root );  
	root->SetAttribute("SegmentationSource", segImageName.c_str());
	root->SetAttribute("NumberOfAssociativeMeasures", numOfAssocRules);
	root->SetAttribute("NumberOfObjects", numOfLabels);

	TiXmlComment * comment = new TiXmlComment();
	comment->SetValue(" List Of Associative Measurements " );  
	root->LinkEndChild( comment );  

	//go over the objects one by one	
	for(int j=0; j<numOfLabels; j++)
	{
		TiXmlElement *element = new TiXmlElement("Object");
		element->SetAttribute("Type",objectType.c_str());		
		stringstream out1;
		out1 << setprecision(2) << fixed << j+1;
		element->SetAttribute("ID",out1.str());		
		//Add the Associative measurements one by one
		for(int i=0; i<numOfAssocRules; i++)
		{	
			TiXmlElement *element2 = new TiXmlElement("Association");
			stringstream out2;
			out2 << setprecision(2) << fixed << assocMeasurementsList[i][j];
			//element2->SetAttribute(assocRulesList[i].GetRuleName(),out2.str());			
			element2->SetAttribute("Name",assocRulesList[i].GetRuleName());			
			element2->SetAttribute("Value",out2.str());
			element->LinkEndChild(element2);
		}
		root->LinkEndChild(element);
	}	
	
	doc.SaveFile( xmlFname.c_str() );
}

void ObjectAssociation::PrintSelf()
{
	//Print the header
	cout<<"\n---------------------------------------------------------------\n";	
	cout<<"Object Association Rules\n"; 
	cout<<"SegmentationSource "<<segImageName.c_str()<<endl;
	cout<<"NumberOfAssociativeMeasures "<<numOfAssocRules<<endl;
	cout<<".................................................................\n";	
	//Print the Association Rules one by one
	for(int i=0; i<numOfAssocRules; i++)
	{		
		cout<<"Name("<<i<<"): "<<assocRulesList[i].GetRuleName().c_str()<<endl;
		cout<<"Target_Image("<<i<<"): "<<assocRulesList[i].GetTargetFileNmae().c_str()<<endl;
		cout<<"Outside_Distance("<<i<<"): "<<assocRulesList[i].GetOutDistance()<<endl;
		cout<<"Inside_Distance("<<i<<"): "<<assocRulesList[i].GetInDistance()<<endl;		
		if(assocRulesList[i].IsUseWholeObject())
			cout<<"Use_Whole_Object("<<i<<"): True"<<endl;			
		else
			cout<<"Use_Whole_Object("<<i<<"): False"<<endl;

		switch(assocRulesList[i].GetAssocType())
		{
		case ASSOC_MIN:
			cout<<"Association_Type("<<i<<"): MIN"<<endl;			
			break;
		case ASSOC_MAX:
			cout<<"Association_Type("<<i<<"): MAX"<<endl;
			break;
		case ASSOC_TOTAL:
			cout<<"Association_Type("<<i<<"): TOTAL"<<endl;
			break;
		case ASSOC_AVERAGE:
			cout<<"Association_Type("<<i<<"): AVERAGE"<<endl;
			break;
		default:
			cout<<"Association_Type("<<i<<"): AVERAGE"<<endl;
		}		
	}
	cout<<"---------------------------------------------------------------\n";
}

} //end namespace ftk

