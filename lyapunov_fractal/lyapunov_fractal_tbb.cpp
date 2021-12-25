//g++ lyapunov_fractal_tbb.cpp -ltbb -o binarka `Magick++-config --cppflags --cxxflags --ldflags --libs`
// ./binarka

#include <iostream>
#include <cmath>
#include <Magick++.h> 
#include <tbb/tbb.h>
#include "tbb/tick_count.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include <fstream> 

using namespace std;
using namespace Magick; 
using namespace tbb;

const int size = 500;
char pixels[size][size][3];
unsigned char random_color[][3] = {{178, 255, 102}, {153, 204, 255}, {178, 102, 255}, \
{255, 153, 204}, {255, 178, 102}, {255, 255, 102}, {102, 255, 102}, {192, 192, 192}};
int grainSizeX = 50;
int grainSizeY = 25;


class Lapunov{
     public:
        char Sequence[16];
        const int noColors = 255;
		float MinR, MaxR, MinX, MaxX;
		char NoSeq = 16;
		float LapMin=-2;
		int sample;

		int DecToBin(int Dec)
		{
			int pow;
			char rest;
			pow = 65536;
			rest = 0;

			while(rest==0 && pow>0)
			{
				rest = (char)floor(Dec/pow);
				if (rest == 0)
					pow = floor(pow/2);
			}
			while(pow>1)
			{
				Dec = Dec - pow*rest;
				pow = floor(pow/2);
				rest = (char)floor(Dec/pow);
				NoSeq++;
				cout << NoSeq;
				Sequence[NoSeq]=rest;
			}
			cout << '\n';
			return 0;
		}
		/*void DecToBin(int Dec){
		  int i;
		  for(i=0; i<16; i++){
		     Sequence[i] = Dec >> i & 1;

		  }
		}*/

		void Draw(float Seed, int NoIter, int RozX, int RozY, int start, int koniec, float RXMin, float RXMax, float RYMin, float RYMax, int val, int s)
		{
			float rx, ry, deltaX, deltaY, tmpLap=0;
			int k, w;
			char tmp;

			for(k = 0; k<16; k++)
			  Sequence[k] = 0;

			sample = s;
			NoSeq = 0;
			Sequence[0] = 1;
			DecToBin(val);
			LapMin = -2;
			MinR = RXMin;
			MaxR = RXMax;
			MinX = RYMin;
			MaxX = RYMax;
			deltaX = (MaxR-MinR)/RozX;
			deltaY = (MaxX-MinX)/RozY;
			rx = MinR;
			ry = MaxX-(start-1)*deltaY;
			double z;
			for(k=0; k<15; k++)
			  std::cout << (int)Sequence[k];

			  std::cout << "\n";

            parallel_for(
                blocked_range2d<size_t>(0, RozX, grainSizeX, 0, RozY, grainSizeY ),
                [=](const blocked_range2d<size_t> &r)
                {
                    float rx = 0;
                    float ry = 0;
                    float tmpLap = 0;
                    float z;
                    char tmp;
                    float rxstart = MinR;
                    float rystart = MaxX - (start - 1) * deltaY;
                    for (int w = r.rows().begin(); w < r.rows().end(); w++)
				    {
                        for (int k = r.cols().begin(); k < r.cols().end(); k++)
                        {
                            float tmpLap = ValLap(Seed, NoIter, rx, ry);
                            auto threadNumber = task_arena::current_thread_index();
                            //std::cout << (int)tmpLap;
                            if (tmpLap<=0)
                            {
                                z = noColors*tmpLap/LapMin;
                                tmp = (int)(z)%noColors;
                                pixels[k][w][0] = tmp;
                                pixels[k][w][1] = tmp;
                                pixels[k][w][2] = tmp;
                            }
                            else{
                                pixels[k][w][0] = random_color[threadNumber][0];
                                pixels[k][w][1] = random_color[threadNumber][1];
                                pixels[k][w][2] = random_color[threadNumber][2];
                            }
                            rx = rxstart + deltaX * k;
				        }
                        ry = rystart - (deltaY * w);
			        }
                }
            );
			//return pixels;
		}

		float ValLap(float Seed, int NoIter, float rx, float ry)
		{

			float x, sumlap, elem, ValLap;
			int i, poz, NoElem;
			float R;

			x = Seed;
			sumlap = 0;
			NoElem=0;
			poz=0;

			for(i=1; i<=NoIter; i++)
			{
				if (Sequence[poz]==0)
					R=ry;
				else
					R=rx;
				poz++;
				if (poz>NoSeq)
					poz=0;
				x=fun(x, R);
				elem=(float)abs(dfun(x,R));
				if (elem>1000)
				{
					ValLap=10;
					break;
				}
				if (elem!=0)
				{
					sumlap=sumlap+(float)log2(elem);
					NoElem++;
				}
			}
			if (NoElem>0)
				ValLap = sumlap/NoElem;
			else
				ValLap = 0;
			return ValLap;
		}


		float fun(float x, float r)
		{
			float y=0;
			switch(sample)
			{
				case(0) : y = r*sin(x)+r; break;
				case(1) : y = r*cos(x)+r; break;
				case(2) : y = r*cos(x)*(1-sin(x)); break;
			}
			return y;
		}

		float dfun(float x, float r)
		{
			float y=0;
			switch(sample)
			{
				case(0) : y = r*cos(x); break;
				case(1) : y = -r*sin(x); break;
				case(2) : y = r*(1-sin(x))-2*cos(x)*cos(x); break;
			}
			return y;
		}


};

int main(int argc,char **argv)
{
    InitializeMagick(*argv);
    Image image;

    // ofstream myfile;
    // myfile.open("lab12Times.txt", ios::app);
    // myfile << "\n";
    // myfile << "image size | grain size in rows | in columns | time (s)" << endl;
    // myfile.close();

    FILE * fp;
    int i,j;
    string filename_str = to_string(size) + "_" + to_string(grainSizeX) + "_" + to_string(grainSizeY) + ".ppm";
    const char *filename = filename_str.c_str(); 
    char *comment=(char *)"# ";/* comment should start with # */
    const int MaxColorComponentValue=255;
    int a;
    Lapunov lp;
    fp= fopen(filename,"wb"); /* b -  binary mode */
    /*write ASCII header to the file*/

    fprintf(fp,"P6\n %s\n %d\n %d\n %d\n",comment,size,size,MaxColorComponentValue);
    tick_count t0 = tick_count::now();
    lp.Draw(5, 100, size, size, 0, size, -3, 9, -5, 2, 2477, 1);
    tick_count t1 = tick_count::now();
    fwrite(pixels,1,3*size*size,fp);
    fclose(fp);

    ofstream myfile;
    myfile.open("lab12Times.txt", ios::app);
    myfile << size << "        | " << grainSizeX << "                 | " << grainSizeY << "         | " << (t1-t0).seconds() << endl;
    myfile.close();

    string imagename_ppm = to_string(size) + "_" + to_string(grainSizeX) + "_" + to_string(grainSizeY) + ".ppm";
    string imagename_png = to_string(size) + "_" + to_string(grainSizeX) + "_" + to_string(grainSizeY) + ".png";
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

   return 0;
}
