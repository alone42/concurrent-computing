// matrix_multiplication_openmp.cpp -pthread -O3 -fopenmp -o binarka
// ./binarka

#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <ctime>
#include <fstream>
#include <omp.h>
#include <ctime>

using namespace std;

int** C; 

void multiplying_matrix(int** A, int** B, int size, int start, int stop){
	for(int i = start; i < stop; ++i) 
		for(int j = 0; j < size; ++j) 
			for(int k = 0; k < size; ++k) { 
				C[i][j] += A[i][k] * B[k][j]; 
			}
}

void multiplying_transpose_matrix(int** A, int** B, int size, int start, int stop){
	for(int i = start; i < stop; ++i) 
		for(int j = 0; j < size; ++j) 
			for(int k = 0; k < size; ++k) { 
				C[i][j] += A[k][j] * B[k][j]; 
			}
}

int** transposing_matrix(int** A, int** T, int size){
    for (int i = 0; i < size; i++) 
        for (int j = 0; j < size; j++)
            T[i][j] = A[j][i];
    return T;
}

int** allocation(int** M, int size){
    for (int i = 0; i < size; i++)
        M[i] = new int[size];
    for (int i = 0; i < size; i++) 
        for (int j = 0; j < size; j++)
            M[i][j] = j+1;
    return M;
}

double task(int size, int n_parallels, bool isTranspose)
{
    
    int start = 0;
    int stop = 0;
    int n_rows = size / n_parallels;

    int** A = new int*[size];
    A = allocation(A, size);

    int** T = new int*[size];
    T = allocation(T, size);
    T = transposing_matrix(A, T, size);
    
    C = new int*[size];
    for (int i = 0; i < size; i++)
        C[i] = new int[size];
    for (int i = 0; i < size; i++) 
        for (int j = 0; j < size; j++)
            C[i][j] = 0;
    
    double begin = omp_get_wtime();
    if (isTranspose)
    {
        #pragma omp parallel for
        for (int i = 0; i < n_parallels; i++){
            start = i * n_rows;
            stop = start + n_rows;
            if (i + 1 == n_parallels){
                stop = size;
            }
            multiplying_matrix(A, A, size, start, stop); 
        }
    }
    else
    {
        #pragma omp parallel for
        for (int i = 0; i < n_parallels; i++){
            start = i * n_rows;
            stop = start + n_rows;
            if (i + 1 == n_parallels){
                stop = size;
            }
            multiplying_transpose_matrix(A, T, size, start, stop); 
        }
    }
    double end = omp_get_wtime();
    for (int i = 0; i < size; i++){
        delete[] A[i];
        delete[] T[i];
        delete[] C[i];
    }
        
    delete[] A;
    delete[] T;
    delete[] C;
    return end - begin;
}

int main(){   

    int N = 500;
    int n = 2; //poszerznie tablicy
    int n_parallels = 5; 
    double time = 0;

    ofstream myfile;
    myfile.open("odm_zad_2b_Skyba.txt", ios::app);
    myfile << "A * A" << "\n";
    for (int i = 1; i <= n; i++){
        int size = i*N;
        time = task(size, n_parallels, false);
        myfile << "number of threads: " << n_parallels << "| size " << size << "| time "<< time << "(s)" << "\n";
    }
    myfile << "A * A_transpose " << "\n";
    for (int i = 1; i <= n; i++){
        int size = i*N;
        time = task(size, n_parallels, true);
        myfile << "number of threads: " << n_parallels << "| size " << size << "| time "<< time << "(s)" << "\n";
    }
    myfile.close();

    return 0;
}