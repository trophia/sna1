#include <iostream>

//#define FISHES_PARALLEL
#include "model.hpp"

Model model;

int main(int argc, char** argv){
    std::cout<<"SNA1 model\n";
    #if defined(FISHES_PARALLEL)
    	std::cout<<"  - `fishes` parallelized\n";
    #endif

    model.initialise();
    
    model.fishes.start_number = 1e4;
    model.run(1980,2020);

    std::cout<<std::endl;
    return 0;
}
