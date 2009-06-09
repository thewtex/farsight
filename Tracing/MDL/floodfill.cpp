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

/*  Flood filling
 *  accept a sequence of volumes
 * Input data contains either 0 or values greater or equal to 3
 *  Windows version, taken in from Linux version
 *   Author: Xiaosong Yuan, RPI
 *  Modified on Oct. 2, 2005                 */


#include <stdlib.h>
#include <stdio.h>
//#include <fstream.h>
//#include <iostream.h>
#include <string.h>
#define FillColor 1  //111
#define OBJECT 200
#define COLORseed 0  //usually 0, can be selected as OBJECT

//using namespace std;

#define DATATYPEIN unsigned char
#define DATATYPEOUT unsigned char


DATATYPEIN *volin;
int sizeX, sizeY, sizeZ, sizeTime;
long voxelCount=0;

struct  Position
{
	int x;
	int y;
	int z;
};

struct StackOfPoints
{
	Position ps;
	StackOfPoints *next;
};

StackOfPoints *stack = new StackOfPoints;
int stacktop = 0;

void lineseed(int, int, int);
void push (Position pos);
Position pop (void);
bool empty(void);
int findstartx(Position ps);
int findendx(Position ps);
void spread(Position pos, int startx, int endx, int direction);
void lineSeed();


int main(int argc, char *argv[])
{
	
	if(argc < 7) {
	    printf("\nUsage: <input file> <sizeX> <sizeY> <sizeZ> <sizeTime> <output file> <threshold>.\n");
	    exit(1);
	}

	//string s;
	FILE *infile;
	FILE *outfile;
	char *infilename = new char[80];
	char *outfilename = new char[80];
	int i,j,k, t;
	//int ii, jj, kk;
	//int NearObjFlag;
	DATATYPEOUT *volout;
	long idx;//, iidx;
	float threshold;
	//int kmod8, kdiv8;
	//int FlagIsolated;
	//int NumConnectComp;
	//DATATYPEOUT blockMax;

	infilename = argv[1];
	sizeX = atoi(argv[2]);
	sizeY = atoi(argv[3]);
	sizeZ = atoi(argv[4]);
	sizeTime = atoi(argv[5]);
	outfilename = argv[6];
	threshold = atof(argv[7]);

	volin = (DATATYPEIN*)malloc(sizeX*sizeY*(sizeZ)*sizeof(DATATYPEIN));
	volout = (DATATYPEOUT*)malloc(sizeX*sizeY*(sizeZ)*sizeof(DATATYPEOUT));

	if((infile=fopen(infilename,"rb"))==NULL)
			{printf("Input file open error!\n");
			 exit(-1);
			}

	if((outfile=fopen(outfilename,"wb"))==NULL)
			{printf("Output file open error!\n");
			 exit(-1);
			}

/*	if 0 	// read another file if necessary
	FILE *infile2;
	unsigned char *volin2;
	volin2 = (unsigned char*)malloc(sizeX*sizeY*sizeZ*sizeof(unsigned char));
	if((infile2=fopen("contrast_thres.64x64x32.img","rb"))==NULL)
			{printf("Input file open error!\n");
			 exit(-1);
			}
	fread(volin2,sizeX*sizeY*sizeZ,sizeof(unsigned char), infile2);
*/

	for (t=0; t<sizeTime; t++) {

		fread(volin,sizeX*sizeY*sizeZ,sizeof(DATATYPEIN), infile);
		printf("Growing # %d ...\n", t);

		for (k=0; k<sizeZ; k++)
			for (j=0; j<sizeY; j++)
				for (i=0; i<sizeX; i++) {
					if (volin[k *sizeX*sizeY + j *sizeX + i] == FillColor)  {
						printf("Error input data %d-%d-%d\n", i, j, k);
						exit(0);
					}
				} 


		// Flood/scanlines fill
		lineSeed();
		for (k=0; k<sizeZ; k++) {
			for (j=0; j<sizeY; j++) {
				for (i=0; i<sizeX; i++)
				{
					idx = k *sizeX*sizeY + j *sizeX + i;
					if(volin[idx] != FillColor) {
						if (volin[idx] == COLORseed)  //fill holes with 1
							volout[idx] = 1;
						else
                            volout[idx] = volin[idx];
					}
					else {
						volout[idx] = 0;
					}
				}
			}
		}

		fwrite(volout, sizeX*sizeY*sizeZ, sizeof(DATATYPEOUT), outfile);

	}


	fclose(infile);
	fclose(outfile);
	printf("Done \n");
	return 0;
}

void push (Position pos)
{
	StackOfPoints *temp = new StackOfPoints;
	temp->ps = pos;
	temp->next = stack;
	stack = temp;
	stacktop ++;
}

Position pop (void)
{
	StackOfPoints *temp;
	Position outp = stack->ps;
	temp = stack;
	stack = stack->next;
	stacktop --;
	delete temp;
	return outp;
}

bool empty(void)
{
	return stacktop == 0 ;
}

int findstartx(Position ps)
{
	while (volin[ps.z *sizeX*sizeY + ps.y *sizeX + ps.x] == COLORseed && ps.x >=0)
	{
		ps.x --;
	}
	ps.x ++;
	return ps.x;
}

int findendx(Position ps)
{
	while (volin[ps.z *sizeX*sizeY + ps.y *sizeX + ps.x] == COLORseed && ps.x < sizeX)
	{
		ps.x ++;
	}
	ps.x --;
	return ps.x;
}

void spread(Position pos, int startx, int endx, int direction)
{
	Position pos1; // in a new row
	int newy, newz;
	int startx0, endx0;
	int startx1;//, endx1;
	int laststartx;

	switch (direction)
	{
		case 1:
			newy = pos.y + 1;
			newz = pos.z;
			break;
		case 2:
			newy = pos.y - 1;
			newz = pos.z;
			break;
		case 3:
			newy = pos.y;
			newz = pos.z +1;
			break;
		case 4:
			newy = pos.y;
			newz = pos.z -1;
			break;
	}

	if (newy < sizeY && newy >= 0 && newz < sizeZ && newz >=0) // within boundary
	{
		startx0 = startx;
		endx0 = endx;
		laststartx = -1;
		for (int i = startx0; i<= endx0; i++ )
		{
			if (volin[newz *sizeX*sizeY + newy *sizeX + i] == COLORseed)
			{
				pos1.x = i;
				pos1.y = newy;
				pos1.z = newz;
				startx1 = findstartx(pos1);
				if (startx1 != laststartx)
				{
					pos1.x = startx1;
					pos1.y = newy;
					pos1.z = newz;
					push(pos1);
					laststartx = startx1;
				}
			}


		}

	}

}


void lineSeed()
{
	Position pos;
	int startx, endx;

	Position initialSeed = {0, 0, 0}; // set the seed position
	push(initialSeed);
	while (!empty())
	{
		pos = pop();
		if(volin[pos.z *sizeX*sizeY + pos.y *sizeX + pos.x] == COLORseed)
		{
			startx = pos.x;
			endx = findendx(pos);
			for (int i = startx; i<= endx; i++)
				volin[pos.z *sizeX*sizeY + pos.y *sizeX + i] = FillColor;
			// four directions
			spread(pos, startx, endx, 1);
			spread(pos, startx, endx, 2);
			spread(pos, startx, endx, 3);
			spread(pos, startx, endx, 4);

		}

	}

}

