//Skyba Alona 42189 2b
//g++ sierpinski_square.cpp -pthread -O3 -o binarka
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

const int square_size = pow(3, 6);
const int iXmax = square_size; 
const int iYmax = square_size;
const int MaxColorComponentValue = 255; 
unsigned char filling_data[iYmax][iXmax][3];

void painting_square(int l, int iY_vertex, int iX_vertex){
    int iYmax_square = square_size / pow(3, l);
    int iXmax_square = square_size / pow(3, l);
    
    for (int i = 0; i < square_size / pow(3, l); i++)
        for (int k = 0; k < 3; k++){
            filling_data[i + iY_vertex][iX_vertex][k] = 0; //left side
            filling_data[iY_vertex][i + iX_vertex][k] = 0; //top side
            filling_data[iY_vertex + iYmax_square][iX_vertex + i][k] = 0; //bottom side
            filling_data[iY_vertex + i][iX_vertex + iXmax_square][k] = 0; //right side
        }
}

void task(int l, int iY_vertex, int iX_vertex){

    std::vector <std::thread> thread_vector;
    painting_square(l, iY_vertex, iX_vertex);
    
    //checking the level of nesting
    if (l == 5) //5) //37378
        return;
    
    /*
    1 2 3
    4   5
    6 7 8
    */

    thread_vector.push_back(thread(task, l+1, iY_vertex, iX_vertex)); //1
    thread_vector.push_back(thread(task, l+1, iY_vertex, iX_vertex + square_size / pow(3, l+1))); //2
    thread_vector.push_back(thread(task, l+1, iY_vertex, iX_vertex + 2 * square_size / pow(3, l+1))); //3
    thread_vector.push_back(thread(task, l+1, iY_vertex + square_size / pow(3, l+1), iX_vertex)); //4
    thread_vector.push_back(thread(task, l+1, iY_vertex + square_size / pow(3, l+1), iX_vertex + 2 * square_size / pow(3, l+1))); //5
    thread_vector.push_back(thread(task, l+1, iY_vertex + 2 * square_size / pow(3, l+1), iX_vertex)); //6
    thread_vector.push_back(thread(task, l+1, iY_vertex + 2 * square_size / pow(3, l+1), iX_vertex + square_size / pow(3, l+1))); //7
    thread_vector.push_back(thread(task, l+1, iY_vertex + 2 * square_size / pow(3, l+1), iX_vertex + 2 * square_size / pow(3, l+1))); //8

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
    string filename_str = "odm_lab3_2b_Skyba_dodatkowe.ppm";
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