//Karen Wang
//wangkh@usc.edu
//Sept 18 2016

#include "Pixel.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <utility>
#include <vector>
#include <numeric>

using namespace std;

struct chooseSecond {
    bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right) {
        return left.second < right.second;
    }
};


int main(int argc, char *argv[])

{
    // Define file pointer and variables
    FILE *file;
    int BytesPerPixel;//greyscale or RGB
    int SizeWidth = 256; //default width
    int SizeHeight= 256; //default height
    int newWidth;
    int newHeight;
    
    //------------------ Command Line Input --------------------//
    // Check for proper syntax
    if (argc < 3){
        cout << "Syntax Error - Incorrect Parameter Usage:" << endl;
        cout << "program_name [input_image.raw] [output_image.raw] [BytesPerPixel] [Width] [Height]" << endl;
        return 0;
    }
    // Check if image is grayscale or color
    if (argc < 4){
        BytesPerPixel = 1; // default is grey image
    }
    else {
        BytesPerPixel = atoi(argv[3]);
        // Check if size is specified
        if (argc >= 5){
            SizeWidth = atoi(argv[4]);
            SizeHeight= atoi(argv[5]);
        }
    }
    // Allocate original image data array
    unsigned char Imagedata[SizeHeight][SizeWidth][BytesPerPixel];
    
    // Read image (filename specified by first argument) into image data matrix
    if (!(file=fopen(argv[1],"rb"))) {
        cout << "Cannot open file: " << argv[1] <<endl;
        exit(1);
    }
    fread(Imagedata, sizeof(unsigned char), SizeHeight*SizeWidth*BytesPerPixel, file);
    fclose(file);
    
    if (argc<6) {
        cout << "Syntax Error - Incorrect Parameter Usage:" << endl;
        cout << "program_name input_image.raw output_image.raw [BytesPerPixel] [InputImageWidth] [InputImageHeight]" << endl;
        exit(1);
    }
    
    //------------------ Output to Command Line--------------------//
    //User inputs:
    cout<<"User Inputs:"<<endl;
    //cout<<argv[0]<<endl;
    cout<<"Input filename: "<<argv[1]<<endl;
    cout<<"Output filename: "<<argv[2]<<endl;
    cout<<"Bytes Per Pixel: "<<argv[3]<<endl;
    cout<<"Input Image Width: "<<argv[4]<<endl;
    cout<<"Input Image Height: "<<argv[5]<<endl;
    newHeight=SizeHeight;
    newWidth=SizeWidth;

    
    
    //------------------ Histogram Transform Algorithim --------------------//
    //Make a Pixel object for each pixel
    //Pixel PixelArray[166000];
    //Pixel PixelArray[187500];
    Pixel* PixelArray;
    PixelArray = new Pixel [SizeWidth*SizeHeight];
    int pixelnum=0;
    unsigned char tempImage[newHeight][newWidth][BytesPerPixel];
    for (int i=0; i<newHeight; i++)
    {
        for(int j=0; j<newWidth; j++)
        {
            tempImage[i][j][0]=0;
            PixelArray[pixelnum].setRow(i);
            PixelArray[pixelnum].setCol(j);
            PixelArray[pixelnum].setIntensity(int(Imagedata[i][j][0]));
            PixelArray[pixelnum].setPriority(0);
            pixelnum++;
        }
    }

    
    //Calculate Frequency of Intensity/ # pixels in each "bin"
    float inputFreq[256]={0};//stores input pixel intensity frequency
    for (int i=0; i<newHeight; i++)
        {
            for (int j=0; j<newWidth; j++)
            {
                //Record input image intensity frequencies
                inputFreq[(int)Imagedata[i][j][0]]++;
            }
        }
    
        
    //PDF Calculation
    int diff[256]={0};//stores difference in pixel for each intensity for the input and pdf output
    int pdfMap[256]={0};
    int pixelCount=SizeHeight*SizeWidth;//total number of pixels
    int pdfPixelCount=0;//pdf approximation may not give exactly the total number of pixels
    float pdf=0;//stores result from guassian probability distribution function
    
    for (int i=0; i<256; i++) {
            //Compute PDF for each Intensity (0-255)
            pdf=(exp(-(pow(double(i-125), 2))/(2*1600.0))/sqrt(2*40.0*3.1415926)); //gaussian probability
            pdf=pdf/6.31568;//Normalize probability with area under the curve = 6.31568;
            pdfMap[i]=int(round(pdf*pixelCount));
            pdfPixelCount+=pdfMap[i];//to check total # of pixels generated by pdf
            diff[i]=inputFreq[i]-pdfMap[i];//stores difference in # of pixels in input and desired output intensity frequency
        }
        //END OF PDF CALCULATION
        
        
        
        //PIXEL # ALIGNMENT
        //Align Total # of Pixels in PDF-derived output and input
        int TotalPixelDiff=pdfPixelCount-SizeHeight*SizeWidth;
        int offset=0;
        while (TotalPixelDiff!=0) {
            if (TotalPixelDiff>0)//more total pixels in PDF-derived intensity freq than original image
            {
                if (pdfMap[offset]>=TotalPixelDiff)//check if pdfMap[0] has enough pixels to be subtracted
                {
                    pdfMap[offset]+=TotalPixelDiff;
                    pdfPixelCount+=TotalPixelDiff;
                    TotalPixelDiff=0;
                }
                else // pdfMap[offset]<TotalPixelDiff
                {
                    offset++;
                }
            }
            else //TotalPixelDiff<0, fewer pixel in PDF-derived intensity freq than original image
            {
                pdfMap[125]-=TotalPixelDiff;
                pdfPixelCount-=TotalPixelDiff;
                TotalPixelDiff=0;
            }
        }
        //END PIXEL # ALIGNMENT
    
    //SORTING SET-UP
    vector<pair<int,int>> bin;
    int depth;
    int pixel=0;
    int prioridx=0;
    
    //REDISTRIBUTION OF PIXELS (BULK OF THE CODE)
    //Left to right 0->255

    for (int it=0; it<1; it++) {//# iteration of the algorithim
        
        //////////////////////////Redistribution from 0->255//////////////////////////////////
        for (int i=0; i<255; i++) {
            depth=0;
            bin.erase(bin.begin(),bin.end());
            if (diff[i]>0) { //if there are too many pixels in the bin 'i'
                for (int j=0; j<pixelCount; j++) { //put pixel with desired intensity into vector
                    if (PixelArray[j].intensity ==i)
                    {
                        bin.push_back(make_pair(j, PixelArray[j].getPriority()));
                        depth++;
                    }
                }//end of for loop up to j<pixelCount
                sort(bin.begin(), bin.end(),chooseSecond());//sorting priority of the pixels to be moved
            }//end of if(diff[i]>0
            
            prioridx=0;
            while (diff[i]>0) {
                if(prioridx!=depth+1) {
                    pixel=bin[prioridx].first;
                    PixelArray[pixel].setIntensity(i+1);
                    PixelArray[pixel].setPriority(PixelArray[pixel].getPriority()+1);
                    diff[i]--;
                    diff[i+1]++;
                    prioridx++;
                }
                else
                {
                    cout<<"break! reached the end of bin vector"<<endl;
                    cout<<"prioridx="<<prioridx<<endl;
                    break;
                }
                
            }//end of diff[i]>0
        }//end of for loop up to i<255
        
        ////////////////Redistribution from 255->0//////////////////
        for (int i=255; i>0; i--) {
            depth=0;
            bin.erase(bin.begin(),bin.end());
            if (diff[i]>0) { //if there are too many pixels in the bin 'i'
                for (int j=0; j<pixelCount; j++) { //put pixel with desired intensity into vector
                    if (PixelArray[j].intensity ==i)
                    {
                        bin.push_back(make_pair(j, PixelArray[j].getPriority()));
                        depth++;
                    }
                }//end of for loop up to j<pixelCount
                sort(bin.begin(), bin.end(),chooseSecond());//sorting priority of the pixels to be moved
            }//end of if(diff[i]>0
            
            while (diff[i]>0) {
                if (depth!=0) {
                    pixel=bin[depth].first;
                    PixelArray[pixel].setIntensity(i-1);
                    PixelArray[pixel].setPriority(PixelArray[pixel].getPriority()-1);
                    diff[i]--;
                    diff[i-1]++;
                    depth--;
                }
                else //bin vector is empty
                {
                    break;
                }//end of if (depth!=0)else()
            }//end of diff[i]>0
        }//end of for loop up to i<255
    }//end of iteration loop
    
    
    //Write Data to Output Image Array
    int row,col;
    for (int i=0; i<newHeight*newWidth; i++)
    {
        row=PixelArray[i].getRow();
        col=PixelArray[i].getCol();
        tempImage[row][col][0]=char(PixelArray[i].getIntensity());
    }
    
    delete [] PixelArray;
    
    //Scan Output Image Pixel Frequencies to generate Histogram
    float outputFreq[256]={0};
    for (int i=0; i<newHeight; i++) {
        for (int j=0; j<newWidth; j++) {
            //Record pixel frequencies to output Histogram
            outputFreq[(int) tempImage[i][j][0]]++;
        }
    }
    
    //Uncomment to output Histogram values of the output to command line
    /*
    for (int i=0; i<256; i++) {
        cout<<outputFreq[i]<<endl;
    }
    */
    
   
        
    //------------------ Output to Commandline --------------------//
    //New image specs:
    cout<<"\nNew Image Specifications:"<<endl;
    cout << "newWidth= "<<newWidth<<endl;
    cout << "newHeight= "<<newHeight<<endl;
    
    
    //------------------ Output to Image --------------------//
    // Write image data (filename specified by second argument) from image data matrix
    if (!(file=fopen(argv[2],"wb"))) {
        cout << "Cannot open file: " << argv[2] << endl;
        exit(1);
    }
    fwrite(tempImage, sizeof(unsigned char),  newHeight*newWidth*BytesPerPixel, file);
    fclose(file);
    
    return 0;
}