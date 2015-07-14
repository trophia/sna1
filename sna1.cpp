#include <iostream>

#define FISHES_PARALLEL
#include "model.hpp"

Model model;

int main(int argc, char** argv){
    std::cout<<"SNA1 model\n";

    model.fishes.start_number = 1e6;
    model.run();

    return 0;
}
