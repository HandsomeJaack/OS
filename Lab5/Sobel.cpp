#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <algorithm>
#include <atomic>
#include <cmath>
#include <ctime>
#include <iostream>
#include <pthread.h>
#include <string>
#include <thread>
#include <vector>
#include "stb_image.h"
#include "stb_image_write.h"

using namespace std;

struct args {
    int width;
    int height;
    unsigned char const *image;
    int threads_amount;
 	vector<int>* output_x;
    vector<int>* output_y;
    vector<unsigned char>* output;
    int thread_number;
};

void *Sobel(void *input) {
	int height = ((struct args *) input)->height;
    int width = ((struct args *) input)->width;
    unsigned char const *image = ((struct args *) input)->image;
    int threads_amount = ((struct args *) input)->threads_amount;
    int thread_number = ((struct args *) input)->thread_number;
    vector<int>& output_x_vec = *((struct args *) input)->output_x;
    vector<int>& output_y_vec = *((struct args *) input)->output_y;
    vector<unsigned char>& output_vec = *((struct args *) input)->output;
	int step = ((height - 2) + threads_amount - 1) / threads_amount;
	int start_height = step * thread_number;
	int end_height = (int) fmin(start_height + step, height - 2);

	auto * output_x = output_x_vec.data();
    auto * output_y = output_y_vec.data();
    auto * output = output_vec.data();

    int k = 3 * start_height * (width - 2);
    for (int y = start_height + 1; y < end_height + 1; ++y) {
       for (int x = 1; x < width - 1; ++x) {
          for (int color = 0; color < 3; ++color) {
                                output_x[k] = (-image[(x - 1 + width * (y - 1)) * 3 + color]
                                                - 2 * image[(x - 1 + width * y) * 3 + color]
                                                - image[(x - 1 + width * (y + 1)) * 3 + color]
                                                + image[(x + 1 + width * (y - 1)) * 3 + color]
                                                + 2 * image[(x + 1 + width * y) * 3 + color]
                                                + image[(x + 1 + width * (y + 1)) * 3 + color]);
 
                                 output_y[k] = (-image[(x - 1 + width * (y - 1)) * 3 + color]
                                                - 2 * image[(x + width * (y - 1)) * 3 + color]
                                                - image[(x + 1 + width * (y - 1)) * 3 + color]
                                                + image[(x - 1 + width * (y + 1)) * 3 + color]
                                                + 2 * image[(x + width * (y + 1)) * 3 + color]
                                                + image[(x + 1 + width * (y + 1)) * 3 + color]);
                                 k++;
                         }
                 }
    	 }

    k = 3 * start_height * (width - 2);
 	for(int y = start_height; y < end_height; ++y) {
        for(int x = 0; x < width-2; ++x){
            for(int color = 0; color < 3; ++color){
                int coord = (x + (width-2)*y)*3 + color;
                output[k] = min(sqrt(output_x[coord]*output_x[coord]
								+ output_y[coord]*output_y[coord]),255.0);
                k++;
            }
        }
    }
	return 0;
}


int main(int argc, char* argv[]){
	if(argc < 3){
		cout << "invalid number of arguments. Expected 2." << endl;
		return -1;
	}

	const char* pic_name = argv[1];
	int threads_amount = atoi(argv[2]);
    int width, height, channels;
    unsigned char* image = stbi_load(pic_name, &width, &height, &channels, STBI_rgb);
    cout << "Successful opening. Image size: "<<  width << "x" << height << endl;

    vector<int> output_x((width - 2) * (height - 2) * 3);
    vector<int> output_y((width - 2) * (height - 2) * 3);
    vector<unsigned char> output((width - 2) * (height - 2) * 3);

	pthread_t thr_id[threads_amount];
	atomic_bool finished(false);
	thread points([&]{
			cout << "Processing";
			int point = 0;
			while(!finished){
				string s ="\b\b\b";
				for(int i = 0; i < 3; i++){
					if(i < point)
						s.push_back('.');
					else
						s.push_back(' ');
				}
				cout << s << flush;
				point = (point + 1) % 4;
				this_thread::sleep_for(chrono::milliseconds(1000));
			}
			cout << endl;
		});
	struct timespec start, finish;
	clock_gettime(CLOCK_MONOTONIC, &start);

     struct args params[threads_amount];
	 for (int i = 0; i < threads_amount; ++i) {
                params[i].image = image;
                params[i].output_x = &output_x;
                params[i].output_y = &output_y;
                params[i].output = &output;
                params[i].width = width;
                params[i].height = height;
                params[i].threads_amount = threads_amount;
				pthread_create(&thr_id[i], NULL, Sobel, (void *) (params+i));
	}

	for (int i = 0; i < threads_amount; ++i) {
                pthread_join(thr_id[i], NULL);
    }

   	clock_gettime(CLOCK_MONOTONIC, &finish);
   	double time_elapsed = (finish.tv_sec - start.tv_sec);
  	time_elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;	 

    stbi_write_png("Output.png", width-2, height-2, 3, output.data(), (width-2)*3);
    finished = 1;
	points.join();
	cout << "Finished!" << endl;
	cout << "Time for " << threads_amount << " threads  elapsed: " << time_elapsed << endl;
    stbi_image_free(image);
    return 0;
}

