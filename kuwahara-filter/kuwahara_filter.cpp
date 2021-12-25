#include <iostream>
#include <tbb/tbb.h>
#include <stdio.h>
#include <algorithm>
#include <cstdlib> 
#include <cstdio> 
#include <fstream> 
#include "tbb/tick_count.h"

using namespace std;
using namespace tbb;

unsigned char*** color;
unsigned char*** colorKuwahara;
unsigned char*** colorMin;
int height;
int width;
int max_value;
unsigned char **red, **green, **blue;
int Size;


void load_image() 
{

    std::ifstream ifs; 
    ifs.open("sample_1920×1280.ppm", std::ios::binary); 
    // need to spec. binary mode for Windows users
    
    try { 
        if (ifs.fail()) 
        { 
            throw("Can't open input file"); 
        } 
        std::string header; 
        int w, h, b; 
        ifs >> header; 
        if (strcmp(header.c_str(), "P6") != 0) throw("Can't read input file"); 
        ifs >> w >> h >> b; 
        width = w; 
        height = h;
        max_value = b;
        cout  << width << ", " << height << endl; 
        color = new unsigned char**[width];
        for (int i=0; i<width; i++) 
        {
            color[i] = new unsigned char*[height];
            for (int j=0; j<height; j++) 
            {
                color[i][j] = new unsigned char[3];
            }
        }
        ifs.ignore(256, '\n'); // skip empty lines in necessary until we get to the binary data 
        unsigned char pix[3]; // read each pixel one by one and convert bytes to floats 
        for (int i = 0; i < w; ++i) 
        {
            for (int j = 0; j < h; ++j) 
            {
                ifs.read(reinterpret_cast<char *>(pix), 3); 
                color[i][j][0] = (int)pix[0];
                color[i][j][1] = (int)pix[1]; 
                color[i][j][2] = (int)pix[2]; 
            }
        } 
        ifs.close(); 
    } 
    catch (const char *err) { 
        fprintf(stderr, "%s\n", err); 
        ifs.close(); 
    }

    colorKuwahara = new unsigned char**[width];
    colorMin = new unsigned char**[width];
    red =  new unsigned char*[width];
    green =  new unsigned char*[width];
    blue =  new unsigned char*[width];
    for (int i=0; i<width; i++) 
    {
        colorKuwahara[i] = new unsigned char*[height];
        colorMin[i] = new unsigned char*[height];
        red[i] =  new unsigned char[height];
        green[i] =  new unsigned char[height];
        blue[i] =  new unsigned char[height];
        for (int j=0; j<height; j++) 
        {
            colorKuwahara[i][j] = new unsigned char[3];
            colorMin[i][j] = new unsigned char[3];
            red[i][j] = color[i][j][0];
            green[i][j] = color[i][j][1];
            blue[i][j] = color[i][j][2];
            for (int k=0; k<3; k++) 
            {
                colorKuwahara[i][j][k] = color[i][j][k];
                colorMin[i][j][k] = color[i][j][k];
            }
        }
    }
}

void filtrKuwahara()
{
    int granularity = 0;
    int n_threads = 1;  
    double rm[4], gm[4], bm[4];  //wartosci srednie
    double rs[4], gs[4], bs[4];  //wariancje
    int mr, mg, mb;
    Size = 5;
    int margin = ((Size-1)/2);
    // filtr dla obrazu kolorowego
    task_scheduler_init init(n_threads);
    tick_count t0 = tick_count::now();
    parallel_for(blocked_range2d<int, int>(margin, width-margin, granularity, margin, height-margin, granularity),
        [&](const blocked_range2d<int, int> &r)
        {
            for (int i = r.rows().begin(); i < r.rows().end(); i++)
                for (int j = r.cols().begin(); j < r.cols().end(); j++)
                {
                    //policz srednie
                    for (int k=0; k<4; k++)
                    {
                        rm[k] = 0;
                        gm[k] = 0;
                        bm[k] = 0;
                    }
                    for (int k=0; k<3; k++)
                        for (int l=0; l<3; l++)
                        {
                            rm[0] += red[i+k-margin][j+l-margin] / 9.0;
                            rm[1] += red[i+k][j+l-margin] / 9.0;
                            rm[2] += red[i+k-margin][j+l] / 9.0;
                            rm[3] += red[i+k][j+l] / 9.0;
                            
                            gm[0] += green[i+k-margin][j+l-margin] / 9.0;
                            gm[1] += green[i+k][j+l-margin] / 9.0;
                            gm[2] += green[i+k-margin][j+l] / 9.0;
                            gm[3] += green[i+k][j+l] / 9.0;
                            
                            bm[0] += blue[i+k-margin][j+l-margin] / 9.0;
                            bm[1] += blue[i+k][j+l-margin] / 9.0;
                            bm[2] += blue[i+k-margin][j+l] / 9.0;
                            bm[3] += blue[i+k][j+l] / 9.0;
                        }
                
                    //policz wariancje
                    for (int k=0; k<4; k++)
                    {
                        rs[k] = 0;
                        gs[k] = 0;
                        bs[k] = 0;
                    }
                    for (int k=0; k<3; k++)
                        for (int l=0; l<3; l++)
                        {
                            rs[0] += (rm[0] - red[i+k-margin][j+l-margin]) * (rm[0] - red[i+k-margin][j+l-margin]);
                            rs[1] += (rm[1] - red[i+k][j+l-margin]) * (rm[1] - red[i+k][j+l-margin]);
                            rs[2] += (rm[2] - red[i+k-margin][j+l]) * (rm[2] - red[i+k-margin][j+l]);
                            rs[3] += (rm[3] - red[i+k][j+l]) * (rm[3] - red[i+k][j+l]);
                            
                            gs[0] += (gm[0] - green[i+k-margin][j+l-margin]) * (gm[0] - green[i+k-margin][j+l-margin]);
                            gs[1] += (gm[1] - green[i+k][j+l-margin]) * (gm[1] - green[i+k][j+l-margin]);
                            gs[2] += (gm[2] - green[i+k-margin][j+l]) * (gm[2] - green[i+k-margin][j+l]);
                            gs[3] += (gm[3] - green[i+k][j+l]) * (gm[3] - green[i+k][j+l]);
                            
                            bs[0] += (bm[0] - blue[i+k-margin][j+l-margin]) * (bm[0] - blue[i+k-margin][j+l-margin]);
                            bs[1] += (bm[1] - blue[i+k][j+l-margin]) * (bm[1] - blue[i+k][j+l-margin]);
                            bs[2] += (bm[2] - blue[i+k-margin][j+l]) * (bm[2] - blue[i+k-margin][j+l]);
                            bs[3] += (bm[3] - blue[i+k][j+l]) * (bm[3] - blue[i+k][j+l]);
                        }
                
                    //znajdz najmniejsza wariancje
                    mr=0;
                    for (int k=1; k<4; k++)
                        if (rs[k] < rs[mr])
                            mr = k;
                    
                    mg=0;
                    for (int k=1; k<4; k++)
                        if (gs[k] < gs[mg])
                            mg = k;
                    
                    mb=0;
                    for (int k=1; k<4; k++)
                        if (bs[k] < bs[mb])
                            mb = k;
                    
                    colorKuwahara[i][j][0] = (int)rm[mr];
                    colorKuwahara[i][j][1] = (int)gm[mg];
                    colorKuwahara[i][j][2] = (int)bm[mb];
                }
        }
    );
    tick_count t1 = tick_count::now();
    ofstream myfile;
    myfile.open("Kuwahara.txt", ios::app);
    myfile << "\n";
    myfile << "n threads: " << n_threads << "| granularity = "<< granularity << "| time: " << (t1-t0).seconds()<< endl;
    myfile.close();

}

void filtrMin()
{
    int rmin, gmin, bmin;
    Size = 3;
    int margin = ((Size-1)/2);
    int granularity = 100;
    int n_threads = 4;  
    //filtr dla obrazu kolorowego
    task_scheduler_init init(n_threads);
    tick_count t0 = tick_count::now();
    parallel_for(blocked_range2d<int, int>(margin, width-margin, granularity, margin, height-margin, granularity),
        [&](const blocked_range2d<int, int> &r)
        {
            for (int i = r.rows().begin(); i < r.rows().end(); i++)
                for (int j = r.cols().begin(); j < r.cols().end(); j++)
                {
                    rmin = 255;
                    gmin = 255;
                    bmin = 255;
                    for (int k=0; k<Size; k++)
                        for (int l=0; l<Size; l++)
                        {
                            if (rmin > red[i+k-margin][j+l-margin]) rmin = red[i+k-margin][j+l-margin];
                            if (gmin > green[i+k-margin][j+l-margin]) gmin = green[i+k-margin][j+l-margin];
                            if (bmin > blue[i+k-margin][j+l-margin]) bmin = blue[i+k-margin][j+l-margin];
                        }
                    colorMin[i][j][0] = (int)rmin;
                    colorMin[i][j][1] = (int)gmin;
                    colorMin[i][j][2] = (int)bmin;
                }
        }
    );
    tick_count t1 = tick_count::now();
    ofstream myfile;
    myfile.open("min.txt", ios::app);
    myfile << "\n";
    myfile << "n threads: " << n_threads << "| granularity = "<< granularity << "| time: " << (t1-t0).seconds()<< endl;
    myfile.close();
}

void write_image(string fileName, unsigned char*** filteredColor)
{
    FILE * fp;
    string filename_str = fileName;
    const char *filename = filename_str.c_str(); 
    char *comment=(char*)"# ";
    fp = fopen(filename,"wb"); 
    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n", comment, width, height, max_value);

    //write image data bytes to the ppm file
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
                fwrite(filteredColor[i][j], 1, 3, fp);       
    fclose(fp); 
}

int main()
{
    load_image();
    // filtrKuwahara();
    // write_image("Kuwahara.ppm", colorKuwahara);
    filtrMin();
    write_image("min.ppm", colorMin);

    for (int i=0; i<width; i++) {
        for (int j=0; j<height; j++) {
            delete[] color[i][j];
            delete[] colorKuwahara[i][j];
            delete[] colorMin[i][j];
        }
        delete[] color[i];
        delete[] colorKuwahara[i];
        delete[] colorMin[i];
        delete[] red[i];
        delete[] green[i];
        delete[] blue[i];
    }
    delete[] color;
    delete[] colorKuwahara;
    delete[] colorMin;
    delete[] red;
    delete[] green;
    delete[] blue;
}