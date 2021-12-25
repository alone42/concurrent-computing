// g++ matrix_multiplication.cpp -pthread -O3 -o binarka
// ./binarka

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <ctime>
#include <fstream>

using namespace std;

int** C; 

void multiplying_matrix(int** A, int** B, int size, int start, int stop){
	for(int i = start; i < stop; ++i) 
		for(int j = 0; j < size; ++j) 
			for(int k = 0; k < size; ++k) { 
				C[i][j] += A[i][k] * B[k][j]; 
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

int task(int size, int n_thread, bool is_transpose){
    int start = 0;
    int stop = 0;
    int n_rows = size / n_thread;

    std::vector <std::thread> thread_vector;

    int** A = new int*[size];
    A = allocation(A, size);

    int** T = new int*[size];
    T = allocation(T, size);
    
    C = new int*[size];
    for (int i = 0; i < size; i++)
        C[i] = new int[size];
    for (int i = 0; i < size; i++) 
        for (int j = 0; j < size; j++)
            C[i][j] = 0;

    if (is_transpose){
        T = transposing_matrix(A, T, size);
    }

    //counting time    
    auto begin = std::chrono::steady_clock::now();

    //threads
    for (int i = 0; i < n_thread; i++){
        start = i * n_rows;
        stop = start + n_rows;
        if (i + 1 == n_thread){
            stop = size;
        }
        thread_vector.push_back(thread (multiplying_matrix, A, T, size, start, stop));
        //std::thread threadObj(multiplying_matrix, A, T, size, start, stop); 
    }
    for (auto& t: thread_vector)
        t.join();
    //threadObj.join(); 
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - begin;

    //print the 2D array 
    /*
    for (int i = 0; i < size; i++){
       for (int j = 0; j < size; j++)
            std::cout << C[i][j] << " ";
        std::cout << std::endl;
    }
    */
   
    for (int i = 0; i < size; i++){
        delete[] A[i];
        delete[] T[i];
        delete[] C[i];
    }
        
    delete[] A;
    delete[] T;
    delete[] C;
    return elapsed_seconds.count();
}

//____________________________________________________________________________________________________________________________
int main(){   
    cout << "number of threads: " ;
    int n_thread;
    cin >> n_thread;

    int N = 500;
    int n = 6;
    int time = 0;

    ofstream myfile;
    myfile.open("odm_lab1_2b_Skyba.txt", ios::app);
    myfile << "A * A" << "\n";
    for (int i = 1; i <= n; i++){
        int size = i*N;
        time = task(size, n_thread, false);
        myfile << "number of threads: " << n_thread << "| size " << size << "| time "<< time << "(s)" << "\n";
    }
    myfile << "A * A_transpose " << "\n";
    for (int i = 1; i <= n; i++){
        int size = i*N;
        time = task(size, n_thread, true);
        myfile << "number of threads: " << n_thread << "| size " << size << "| time "<< time << "(s)" << "\n";
    }
    myfile.close();

    return 0;
}