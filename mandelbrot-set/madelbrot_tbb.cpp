// g++ -o madelbrot_tbb.cpp -ltbb `Magick++-config --cppflags --cxxflags --ldflags --libs`
// ./odm

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <Magick++.h> 
#include <iostream>
#include <tbb/tbb.h>
#include "tbb/tick_count.h"


using namespace std;
using namespace Magick; 
using namespace tbb;


const int size = 1600;
const int iXmax = size; 
const int iYmax = size;
unsigned char color[iYmax][iXmax][3];
const double CxMin=-2.5;
const double CxMax=1.5;
const double CyMin=-2.0;
const double CyMax=2.0;
const int IterationMax=200;
const double EscapeRadius=2;
const int MaxColorComponentValue = 255; 

void time_note(int n_threads, double time)
{
    ofstream myfile;
    myfile.open("odm_lab9_Skyba_2b.txt", ios::app);
    myfile << "number of threads: " << n_threads << "| size " << size << "| time "<< time << "(s)" << "\n";
    myfile.close();
}

void image_making(int n_threads)
{
    FILE * fp;
    string imagename_ppm = "odm_lab9_Skyba_2b_" + to_string(n_threads) + ".ppm";
    const char *filename = imagename_ppm.c_str(); 
    char *comment=(char*)"# ";
    fp = fopen(filename,"wb"); 
    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n",comment,iXmax,iYmax,MaxColorComponentValue);

    //write image data bytes to the ppm file
    for (int i = 0; i < iYmax; i++)
        for (int j = 0; j < iXmax; j++)
            fwrite(color[i][j],1,3,fp);
    fclose(fp);
}

void mandelbrot(int iY)
{
    int iX;
    double Cx,Cy;
    double Zx, Zy;
    double Zx2, Zy2;
    int Iteration;
    double ER2=EscapeRadius*EscapeRadius;
    double PixelWidth=(CxMax-CxMin)/iXmax;
    double PixelHeight=(CyMax-CyMin)/iYmax;

    Cy=CyMin + iY*PixelHeight;
    if (fabs(Cy)< PixelHeight/2) Cy=0.0; 
    for(iX=0;iX<iXmax;iX++){        
        Cx=CxMin + iX*PixelWidth;
        Zx=0.0;
        Zy=0.0;
        Zx2=Zx*Zx;
        Zy2=Zy*Zy;

        for (Iteration=0;Iteration<IterationMax && ((Zx2+Zy2)<ER2);Iteration++)
        {
            Zy=2*Zx*Zy + Cy;
            Zx=Zx2-Zy2 +Cx;
            Zx2=Zx*Zx;
            Zy2=Zy*Zy;
        };

        if (Iteration==IterationMax)
        {
            color[iY][iX][0] = 0;
            color[iY][iX][1] = 0;
            color[iY][iX][2] = 0;                 
        }
        else 
        {
            color[iY][iX][0] = MaxColorComponentValue;
            color[iY][iX][1] = MaxColorComponentValue;
            color[iY][iX][2] = MaxColorComponentValue;
        };
    }
}

int main(int argc,char **argv)
{
    InitializeMagick(*argv);
    Image image;
    ofstream myfile;

    int n_threads = 4;
    int granularity = 10;
    // simple_partitioner, auto_partitioner(default), affinity_partitioner, static_partitioner
    task_scheduler_init init(n_threads);
    tick_count t0 = tick_count::now();
    parallel_for(blocked_range<int>(0, iYmax, granularity),
        [&](const blocked_range<int> &r)
        {
            for(int iY = r.begin(); iY != r.end(); ++iY)
                mandelbrot(iY);
        }, simple_partitioner()  
    );
    tick_count t1 = tick_count::now();

    myfile.open("odm_lab9_Skyba_2b.txt", ios::app);
    myfile << "\n";
    myfile << "size = " << size << ", granularity = "<< granularity << ", partitioner: simple_partitioner"<< endl;
    myfile.close();

    time_note(n_threads, (t1-t0).seconds());
    image_making(n_threads);

    mandelbrot(n_threads);
    string imagename_ppm = "odm_lab9_Skyba_2b_" + to_string(n_threads) + ".ppm";
    string imagename_png = "odm_lab9_Skyba_2b_" + to_string(n_threads) + ".png";
    const char *imagename_ppm_char = imagename_ppm.c_str(); 
    try 
    {
        image.read(imagename_ppm);
        image.write(imagename_png);
        remove(imagename_ppm_char);
    } 
    catch( Exception &error_ ) 
    { 
        cout << "Caught exception: " << error_.what() << endl; 
        return 1;
    }
}
