//g++ mandelbrot_set.cpp -pthread -O3 -o binarka `Magick++-config --cppflags --cxxflags --ldflags --libs`
// ./binarka

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <Magick++.h> 

using namespace std;
using namespace Magick; 

const int iXmax = 800; 
const int iYmax = 800;
unsigned char color[iYmax][iXmax][3];
const double CxMin=-2.5;
const double CxMax=1.5;
const double CyMin=-2.0;
const double CyMax=2.0;
const int IterationMax=200;
const double EscapeRadius=2;
const int MaxColorComponentValue = 255; 
int n_threads[] {1, 2, 4};

void mandelbrot(int iYstart, int iYstop, int n, vector <vector<int>> random_color)
{
    int iX,iY;
    double Cx,Cy;
    double Zx, Zy;
    double Zx2, Zy2;
    int Iteration;

    double ER2=EscapeRadius*EscapeRadius;

    double PixelWidth=(CxMax-CxMin)/iXmax;
    double PixelHeight=(CyMax-CyMin)/iYmax;

    for(iY=iYstart;iY<iYstop;iY++)
    {
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
                color[iY][iX][0] = random_color[n][0];
                color[iY][iX][1] = random_color[n][1];
                color[iY][iX][2] = random_color[n][2];  
            };
        }
    }
}

void color_scheme(int n_threads, vector <vector<int>> &random_color)
{
    for (int i = 0; i < n_threads; i++)
        random_color.push_back({rand() % MaxColorComponentValue, rand() % MaxColorComponentValue, rand() % MaxColorComponentValue});
}

int task(int n_threads)
{
    int start, stop;
    int n_iYset = iYmax / n_threads;
    vector <thread> thread_vector;
    vector <vector<int>> random_color;
    color_scheme(n_threads, random_color);

    //counting time  
    auto begin = chrono::steady_clock::now();
    
    //threads
    for (int i = 0; i < n_threads; i++)
    {
        start = i * n_iYset;
        stop = start + n_iYset;
        if (i + 1 == n_threads)
        {
            stop = iYmax;
        }
        thread_vector.push_back(thread(mandelbrot, start, stop, i, random_color));
    }
    for (auto& t: thread_vector)
        t.join();
    
    auto end = chrono::steady_clock::now();
    chrono::duration<double, milli> elapsed_seconds = end - begin;
    
    FILE * fp;
    string filename_str = "number_of_threads" + to_string(n_threads) + ".ppm";
    const char *filename = filename_str.c_str(); 
    char *comment=(char*)"# ";
    fp = fopen(filename,"wb"); 
    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n",comment,iXmax,iYmax,MaxColorComponentValue);

    //write image data bytes to the ppm file
    for (int i = 0; i < iYmax; i++)
        for (int j = 0; j < iXmax; j++)
            fwrite(color[i][j],1,3,fp);
    fclose(fp);

    return elapsed_seconds.count();
}

int main(int argc,char **argv)
{
    int i = 0;
    int time = 0;
    int threads_time[sizeof(n_threads)/sizeof(int)];
    InitializeMagick(*argv);
    Image image;

    for (auto& t: n_threads)
    {
        time = task(t);
        threads_time[i] = time;
        i++;
    }

    ofstream myfile;
    myfile.open("odm_lab2_2b_Skyba.txt", ios::app);
    myfile << "number of threads | time (s)" << "\n";
    i = 0;
    for (auto& t: n_threads)
    {
        myfile << t <<"		     "<< threads_time[i]*0.001 << "\n";
        i++;
    }
    myfile.close();

    //converting ppm to png
    for (auto& t: n_threads)
    {
        string imagename_ppm = "number_of_threads" + to_string(t) + ".ppm";
        string imagename_png = "number_of_threads" + to_string(t) + ".png";
        const char *imagename_ppm_char = imagename_ppm.c_str(); 
        try 
        {
        image.read(imagename_ppm);
        image.write(imagename_png);
        //remove(imagename_ppm_char);
        } 
        catch( Exception &error_ ) 
        { 
            cout << "Caught exception: " << error_.what() << endl; 
            return 1; 
        }
    }
    return 0;
}