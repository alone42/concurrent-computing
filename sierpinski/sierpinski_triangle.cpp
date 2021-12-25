//g++ sierpinski_triangle.cpp -pthread -O3 -o binarka
// ./binarka

//cat /proc/sys/kernel/threads-max
//38075

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <cmath>

using namespace std; 

const int triangle_size = pow(2, 10);
const int iXmax = triangle_size; 
const int iYmax = triangle_size;
const int MaxColorComponentValue = 255; 
unsigned char filling_data[iYmax][iXmax][3];

void painting_triangle(int l, int iY_vertex, int iX_vertex){
    int iYmax_triangle = triangle_size / pow(2, l);
    int iXmax_triangle = triangle_size / pow(2, l);
    
    for (int i = 0; i < triangle_size / pow(2, l); i++)
        for (int k = 0; k < 3; k++){
            filling_data[i + iY_vertex][i + iX_vertex][k] = 0; //diagonal
            filling_data[iY_vertex + iYmax_triangle][iX_vertex + i][k] = 0; //vertical
            filling_data[iY_vertex + i][iX_vertex][k] = 0; //horizontal
        }
}

void task(int l, int iY_vertex, int iX_vertex){

    std::vector <std::thread> thread_vector;
    painting_triangle(l, iY_vertex, iX_vertex);
    
    //checking the level of nesting
    if (l == 9) //29475
        return;
    
    thread_vector.push_back(thread(task, l+1, iY_vertex, iX_vertex));
    thread_vector.push_back(thread(task, l+1, iY_vertex + triangle_size / pow(2, l+1), iX_vertex));
    thread_vector.push_back(thread(task, l+1, iY_vertex + triangle_size / pow(2, l+1), iX_vertex + triangle_size / pow(2, l+1)));

    for (auto& t: thread_vector)
        t.join();
}

int main()
{
    //bleaching the image
    for (int iY = 0; iY < iYmax; iY++)
        for (int iX = 0; iX < iXmax; iX++)
            for (int k = 0; k < 3; k++)
                filling_data[iY][iX][k] = MaxColorComponentValue;
    
    task(0, 0, 0);
    
    FILE * fp;
    string filename_str = "odm_lab3_2b_Skyba.ppm";
    const char *filename = filename_str.c_str(); 
    char *comment=(char*)"# ";
    fp = fopen(filename,"wb"); 
    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n", comment, iXmax, iYmax, MaxColorComponentValue);

    //write image data bytes to the ppm file
    for (int i = 0; i < iYmax; i++)
        for (int j = 0; j < iXmax; j++)
                fwrite(filling_data[i][j], 1, 3, fp);       
    fclose(fp);
}