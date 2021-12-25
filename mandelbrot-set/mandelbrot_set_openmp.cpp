//g++ mandelbrot_set_openmp.cpp -lgomp -fopenmp -O3 -o binarka `Magick++-config --cppflags --cxxflags --ldflags --libs`
// ./binarka

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <omp.h>
#include <vector>
#include <fstream>
#include <Magick++.h> 

using namespace std;
using namespace Magick; 

const int size = 6400;
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
int n_threads_list[] {1, 2, 4};
unsigned char random_color[][3] = {{178, 255, 102}, {153, 204, 255}, {178, 102, 255}, \
{255, 153, 204}, {255, 178, 102}, {255, 255, 102}, {102, 255, 102}, {192, 192, 192}};
int iteration_list[] {0, 0, 0, 0};
const int l = 500;

void time_note(int n_threads, double time)
{
    ofstream myfile;
    myfile.open(to_string(size) + ".txt", ios::app);
    myfile << "number of threads: " << n_threads << "| size " << size << "| time "<< time << "(s)" << "\n";
    myfile.close();
}

void image_making(int n_threads)
{
    FILE * fp;
    string imagename_ppm = "odm_lab7_Skyba_2b_" + to_string(n_threads) + ".ppm";
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

void mandelbrot(int n_threads)
{
    int iX,iY;
    double Cx,Cy;
    double Zx, Zy;
    double Zx2, Zy2;
    int Iteration;
    double ER2=EscapeRadius*EscapeRadius;
    double PixelWidth=(CxMax-CxMin)/iXmax;
    double PixelHeight=(CyMax-CyMin)/iYmax;
    for (auto & i : iteration_list)
        i = 0;
    int *n_iteration = new int[n_threads];
    int thread_num;
    double begin = omp_get_wtime();
    //static, dynamic, guided
    #pragma omp parallel shared(PixelHeight, iYmax, iXmax, PixelWidth, IterationMax, ER2, color, random_color, iteration_list, l) private(thread_num, iY, iX, Cx, Cy, Zx, Zy, Zx2, Zy2, Iteration) num_threads(n_threads)
    {
        thread_num = omp_get_thread_num();
        #pragma omp for schedule(dynamic, l) nowait
        for(iY=0;iY<iYmax;iY++)
        {
            Cy=CyMin + iY*PixelHeight;
            if (fabs(Cy)< PixelHeight/2) Cy=0.0; 
            for(iX=0;iX<iXmax;iX++){
                iteration_list[thread_num]++;         
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
                    int n = omp_get_thread_num();
                    color[iY][iX][0] = random_color[n][0];
                    color[iY][iX][1] = random_color[n][1];
                    color[iY][iX][2] = random_color[n][2];  
                };
            }
        }
    }
    ofstream myfile;
    myfile.open(to_string(size) + ".txt", ios::app);
    for (int j = 0; j < n_threads; j++)
        myfile << iteration_list[j] << '|';
    myfile << "\n";
    myfile.close();

    time_note(n_threads, omp_get_wtime() - begin);
    image_making(n_threads);

    delete[] n_iteration;
}

int main(int argc,char **argv)
{
    InitializeMagick(*argv);
    Image image;
    ofstream myfile;
    myfile.open(to_string(size) + ".txt", ios::app);
    myfile << "\n";
    myfile << "chunk " + to_string(l) << "\n";
    myfile.close();

    for (auto& n_threads: n_threads_list)
    {
        mandelbrot(n_threads);
        string imagename_ppm = "odm_lab7_Skyba_2b_" + to_string(n_threads) + ".ppm";
        string imagename_png = "odm_lab7_Skyba_2b_" + to_string(n_threads) + ".png";
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
}
