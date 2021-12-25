// g++ ulam_spiral.cpp -O3 -fopenmp -o binarka
// ./binarka

#include <iostream>
#include <string>
#include <fstream>
#include <omp.h>
#include <cmath>

using namespace std;


const int size = 2001;
int** C; 
unsigned int number_of_threads_list[] = {1, 2, 3, 4, 6};
unsigned char ppm_data[size][size][3];
unsigned char random_color[][3] = {{0, 0, 0}, {178, 255, 102}, {153, 204, 255}, {178, 102, 255}, \
{255, 153, 204}, {255, 178, 102}, {255, 255, 102}, {102, 255, 102}, {192, 192, 192}};
string type_ = "dynamic"; //static dynamic guided

int ulam_get_map(int x, int y, int n) 
{ 
    x -= (n - 1) / 2; 
    y -= n / 2; 
    int mx = abs(x), my = abs(y); 
    int l = 2 * max(mx, my); 
    int d = y >= x ? l * 3 + x + y : l - x - y; 
    return pow(l - 1, 2) + d; 
} 

int isprime(int n) 
{ 
        int p; 
        for (p = 2; p*p <= n; p++) 
                if (n%p == 0) return 0; 
        return n > 2; 
}

void time_note(int number_of_threads, double time)
{
    ofstream myfile;
    myfile.open("odm_lab5_2b_Skyba_raport.txt", ios::app);
    myfile << "number of threads: " << number_of_threads << "| size " << size << "| time "<< time << "(s)" << "\n";
    myfile.close();
}

void header_setting()
{
    ofstream myfile;
    myfile.open("odm_lab5_2b_Skyba_raport.txt", ios::app);
    myfile << "\n";
    myfile << type_ + "\n";
    myfile.close();
}

void image_making(int number_of_threads)
{
    for (int idx = 0; idx < size; idx++)
    { 
        for (int jdx = 0; jdx < size; jdx++)
        {
            for (int idx_color = 0; idx_color < 3; idx_color++)
                ppm_data[idx][jdx][idx_color] = random_color[C[idx][jdx]][idx_color];

        }
    }
    FILE * fp;
    string filename_str = "odm_lab5_2b_Skyba_n_" + to_string(number_of_threads) + "_" + type_ + ".ppm";
    const char *filename = filename_str.c_str(); 
    char *comment=(char*)"# ";
    fp = fopen(filename,"wb"); 
    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n", comment, size, size, 255);

    for (int idx = 0; idx < size; idx++)
        for (int jdx = 0; jdx < size; jdx++)
                fwrite(ppm_data[idx][jdx], 1, 3, fp);       
    fclose(fp);
}

int main()
{
    C = new int*[size];
    for (int idx = 0; idx < size; idx++)
        C[idx] = new int[size];
    
    for (int idx = 0; idx < size; idx++) 
        for (int j = 0; j < size; j++)
            C[idx][j] = 0;
    
    header_setting();

    for (auto& number_of_threads: number_of_threads_list)
    {
        double begin = omp_get_wtime();
        int idx, jdx, number;
        //static, dynamic, guided
        #pragma omp parallel shared(C, size) private(idx, jdx, number) num_threads(number_of_threads)
        {
            #pragma omp for schedule(dynamic, 100) nowait
            for (int idx = 0; idx < size; idx++) 
                for (int jdx = 0; jdx < size; jdx++)
                {
                    number = ulam_get_map(idx, jdx, size);
                    if (isprime(number))
                        C[idx][jdx] = 0;
                    else
                        C[idx][jdx] = omp_get_thread_num() + 1;
                }
        }
        time_note(number_of_threads, omp_get_wtime() - begin);
        image_making(number_of_threads);
    }
    for (int i = 0; i < size; i++){
        delete[] C[i];
    }
    delete[] C;
}
