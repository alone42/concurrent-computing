//Skyba Alona 42189 2b
//g++ odm_lab4_2b_Skyba.cpp -pthread -O3 -o binarka
// ./binarka

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <cmath>
#include <mutex>
#include <string>

using namespace std;

const int height = 50; 
const int width = 50;
int maze_data[height][width];
mutex maze_data_mutex[height][width];
unsigned char ppm_data[height][width][3];
vector <vector<int>> random_color;
int thread_number = 0;
mutex thread_number_mutex;
const int barrier = -1;
const int corridor = 0;
int DOWN[] = {1, 0};
int UP[] = {-1, 0};
int RIGHT[] = {0, 1};
int LEFT[] = {0, -1};
const int max_color_component_value = 255 + 1; 

void color_scheme()
{
    for (int idx = 0; idx < thread_number; idx++)
        random_color.push_back({rand() % max_color_component_value, rand() % max_color_component_value, rand() % max_color_component_value});
}

void maze_import() {
    string line;
    ifstream my_file;
    my_file.open("odm_lab4_2b_Skyba.txt");
    int idx_height = 0;
    while(!my_file.eof() && idx_height < height) {
        getline(my_file, line);
        for (int idx_width = 0; idx_width < width; idx_width++)
            if (line.at(idx_width) == '0')
                maze_data[idx_height][idx_width] = corridor;
            else
                maze_data[idx_height][idx_width] = barrier;
        idx_height++;
        }
    my_file.close();
}

int set_thread_number(){
    thread_number_mutex.lock();
    thread_number++;
    thread_number_mutex.unlock();
    return thread_number;
}

bool is_move_possible(int start_height, int start_width, const int move[]){
    maze_data_mutex[start_height][start_width].lock();
    if (maze_data[start_height + move[0]][start_width + move[1]] == corridor){
        maze_data_mutex[start_height][start_width].unlock();
        return true;
    }
    maze_data_mutex[start_height][start_width].unlock();
    return false;
}

bool maze_passage(int idx_height, int idx_width, int thread_number_){
    maze_data_mutex[idx_height][idx_width].lock();
    if (maze_data[idx_height][idx_width] == corridor)
    {
        maze_data[idx_height][idx_width] = thread_number_;
        maze_data_mutex[idx_height][idx_width].unlock();
        return true;
    }
    maze_data_mutex[idx_height][idx_width].unlock();
    return false;
}

void task(int start_height, int start_width){
    vector <std::thread> thread_vector;
    int thread_number_ = set_thread_number();
    
    while (maze_passage(start_height, start_width, thread_number_)) 
    {
        vector<vector<int>> possible_directions;
        if (is_move_possible(start_height, start_width, DOWN))
            possible_directions.push_back({start_height + DOWN[0], start_width + DOWN[1]});
        if (is_move_possible(start_height, start_width, UP))
            possible_directions.push_back({start_height + UP[0], start_width + UP[1]});
        if (is_move_possible(start_height, start_width, RIGHT))
            possible_directions.push_back({start_height + RIGHT[0], start_width + RIGHT[1]});
        if (is_move_possible(start_height, start_width, LEFT))
            possible_directions.push_back({start_height + LEFT[0], start_width + LEFT[1]});
        
        if (possible_directions.size() != 0)
        {
            start_height = possible_directions[0][0];
            start_width = possible_directions[0][1];
            if (possible_directions.size() != 1)
                for (int idx = 1; idx < possible_directions.size(); idx++)
                {
                    thread_vector.push_back(thread(task, possible_directions[idx][0], possible_directions[idx][1]));
                }
        }
        else
            break;
    }
    for (auto& t: thread_vector)
        t.join();
}

int main()
{
    int start_height = 0;
    int start_width = 0;
    maze_import();
    while (maze_data[start_height][start_width] == barrier) {
        start_height = rand() % (height - 2);
        start_width = rand() % (width - 2); 
    }
    
    task(start_height, start_width);
    color_scheme();

    //bleaching the image
    for (int idx_height = 0; idx_height < height; idx_height++)
        for (int idx_width = 0; idx_width < width; idx_width++)
            if (maze_data[idx_height][idx_width] == barrier)
                for (int idx_rgb = 0; idx_rgb < 3; idx_rgb++)
                    ppm_data[idx_height][idx_width][idx_rgb] = 0;
            else if (maze_data[idx_height][idx_width] == corridor)
                for (int idx_rgb = 0; idx_rgb < 3; idx_rgb++)
                    ppm_data[idx_height][idx_width][idx_rgb] = 255;
            else
                for (int idx_rgb = 0; idx_rgb < 3; idx_rgb++)
                    ppm_data[idx_height][idx_width][idx_rgb] = random_color[maze_data[idx_height][idx_width]][idx_rgb];

    FILE * fp;
    string filename_str = "odm_lab4_2b_Skyba.ppm";
    const char *filename = filename_str.c_str(); 
    char *comment=(char*)"# ";
    fp = fopen(filename,"wb"); 
    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n", comment, width, height, 255);

    //write image data bytes to the ppm file
    for (int idx_height = 0; idx_height < height; idx_height++)
        for (int idx_width = 0; idx_width < width; idx_width++)
                fwrite(ppm_data[idx_height][idx_width], 1, 3, fp);       
    fclose(fp);
}
