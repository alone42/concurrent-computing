#include <iostream>
#include <tbb/tbb.h>
#include <stdio.h>
#include <algorithm>
#include <cstdlib> 
#include <cstdio> 
#include <fstream> 
#include "tbb/tick_count.h"
#include "tbb/task_group.h"

using namespace std;
using namespace tbb;

unsigned char*** color;
unsigned char*** colorRedLeftBlueRight;
unsigned char*** colorRedCyan;
unsigned char*** colorNoName;

unsigned char** monochrome;
unsigned char*** filteredMonochrome;

int height;
int width;
int max_value;

int height_mono;
int width_mono;
int max_value_mono;

int Size;
const int margin = 30;


void load_color_image()
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
        color = new unsigned char**[width];
        for (int i=0; i<width; i++) 
        {
            color[i] = new unsigned char*[height];
            for (int j=0; j<height; j++) 
            {
                color[i][j] = new unsigned char[3];
            }
        }
        // i*widht + x, jeden wektor, mnozenie szerokosci
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

    colorRedLeftBlueRight = new unsigned char**[width];
    colorRedCyan = new unsigned char**[width];
    colorNoName = new unsigned char**[width];
    for (int i=0; i<width; i++) 
    {
        colorRedLeftBlueRight[i] = new unsigned char*[height];
        colorRedCyan[i] = new unsigned char*[height];
        colorNoName[i] = new unsigned char*[height];
        for (int j=0; j<height; j++) 
        {
            colorRedLeftBlueRight[i][j] = new unsigned char[3];
            colorRedCyan[i][j] = new unsigned char[3];
            colorNoName[i][j] = new unsigned char[3];
            for (int k=0; k<3; k++) 
            {
                colorRedLeftBlueRight[i][j][k] = color[i][j][k];
                colorRedCyan[i][j][k] = color[i][j][k];
                colorNoName[i][j][k] = color[i][j][k];
            }
        }
    }
}

void load_monochrome_image()
{
    FILE *fp;
    const char *filename = "sample_640×426.ppm";
    char *line = NULL;
    size_t len = 0;

    /*reading metadata*/
    fp = fopen(filename, "rb");
    getline(&line, &len, fp); // P6
    getline(&line, &len, fp);
    getline(&line, &len, fp); 
    string sizes(line);
    getline(&line, &len, fp); // max component value
    string componentValueString(line);
    max_value_mono = 255;
    /*getting size*/
    sscanf(sizes.c_str(), "%d %d", &width_mono, &height_mono);
    monochrome = new unsigned char*[width_mono];
    filteredMonochrome = new unsigned char**[width_mono];
    for (int i=0; i<width_mono; i++) 
    {
        monochrome[i] = new unsigned char[height_mono];
        filteredMonochrome[i] = new unsigned char*[height_mono];
        for (int j=0; j<height_mono; j++) 
        {
            filteredMonochrome[i][j] = new unsigned char[3];
        }
    }
    for (int i = 0; i < width_mono; ++i) 
        for (int j = 0; j < height_mono; ++j) 
        {
            for (int k = 0; k < 3; k++)
            {
                getline(&line, &len, fp);
                filteredMonochrome[i][j][k] = (unsigned char)strtol(line,NULL,10);
            }
            monochrome[i][j] = (unsigned char)strtol(line,NULL,10);
        }

    free(line);
    fclose(fp);
}

void filtrRedLeftBlueRight()
{
    parallel_for(blocked_range2d<int, int>(0, width, margin, height-margin),
        [&](const blocked_range2d<int, int> &r)
        {
            
            for (int i = r.rows().begin(); i < r.rows().end(); i++)
                for (int j = r.cols().begin(); j < r.cols().end(); j++)
                {
                    colorRedLeftBlueRight[i][j][0] = color[i][j-margin][0];
                    colorRedLeftBlueRight[i][j][1] = 0;
                    colorRedLeftBlueRight[i][j][2] = color[i][j+margin][2];
                }
        }
    );
}

void filtrRedCyan()
{
    parallel_for(blocked_range2d<int, int>(0, width, margin, height-margin),
        [](const blocked_range2d<int, int> &r)
        {
            for (int i = r.rows().begin(); i < r.rows().end(); i++)
                for (int j = r.cols().begin(); j < r.cols().end(); j++)
                {
                    colorRedCyan[i][j][0] = color[i][j-margin][0];
                    colorRedCyan[i][j][1] = color[i][j+margin][1];
                    colorRedCyan[i][j][2] = color[i][j+margin][2];
                }
        }
    );
}

void filtrNoName()
{
    parallel_for(blocked_range2d<int, int>(0, width, margin, height-margin),
        [](const blocked_range2d<int, int> &r)
        {
            for (int i = r.rows().begin(); i < r.rows().end(); i++)
                for (int j = r.cols().begin(); j < r.cols().end(); j++)
                {
                    colorNoName[i][j][0] = color[i][j-margin][0];
                    colorNoName[i][j][1] = (color[i][j-margin][1] + color[i][j+margin][1]) / 2;
                    colorNoName[i][j][2] = color[i][j+margin][2];
                }
        }
    );
}

void filtrGreyScale()
{
    parallel_for(blocked_range2d<int, int>(0, width_mono, margin, height_mono-margin),
        [](const blocked_range2d<int, int> &r)
        {
            for (int i = r.rows().begin(); i < r.rows().end(); i++)
                for (int j = r.cols().begin(); j < r.cols().end(); j++)
                {
                    filteredMonochrome[i][j][0] = monochrome[i][j - margin];
                    filteredMonochrome[i][j][1] = monochrome[i][j + margin];
                    filteredMonochrome[i][j][2] = monochrome[i][j + margin];
                }
        }
    );
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

void write_monochromed_image(string fileName, unsigned char*** filteredColor)
{
    FILE * fp;
    string filename_str = fileName;
    const char *filename = filename_str.c_str(); 
    char *comment=(char*)"# ";
    fp = fopen(filename,"wb"); 
    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n", comment, width_mono, height_mono, max_value_mono);

    //write image data bytes to the ppm file
    for (int i = 0; i < width_mono; i++)
        for (int j = 0; j < height_mono; j++)
            fwrite(filteredColor[i][j], 1, 3, fp);  
    fclose(fp); 
}

int main()
{
    load_color_image();
    load_monochrome_image();

    task_group g;
    g.run([&]{filtrRedLeftBlueRight();});
    g.run([&]{filtrRedCyan();});
    g.run([&]{filtrNoName();});
    g.run([&]{filtrGreyScale();});
    g.wait();
    

    write_image("redLeftBlueRight.ppm", colorRedLeftBlueRight);
    write_image("redCyan.ppm", colorRedCyan);
    write_image("nonameFiltr.ppm", colorNoName);

    write_monochromed_image("lefRightRight.ppm", filteredMonochrome);

    for (int i=0; i<width; i++) {
        for (int j=0; j<height; j++) {
            delete[] color[i][j];
            delete[] colorRedLeftBlueRight[i][j];
            delete[] colorRedCyan[i][j];
        }
        delete[] color[i];
        delete[] colorRedLeftBlueRight[i];
        delete[] colorRedCyan[i];
        delete[] colorNoName[i];
    }
    delete[] color;
    delete[] colorRedLeftBlueRight;
    delete[] colorRedCyan;
    delete[] colorNoName;

    for (int i=0; i<width_mono; i++) {
        delete[] monochrome[i];
        for (int j=0; j<height_mono; j++)
            delete[] filteredMonochrome[i][j];
        delete[] filteredMonochrome[i];
    }
    delete[] monochrome;
    delete[] filteredMonochrome;
}
