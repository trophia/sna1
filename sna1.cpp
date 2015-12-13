#include <iostream>

//#define FISHES_PARALLEL
#include "model.hpp"

Model model;

int main(int argc, char** argv) {
    std::cout << "SNA1 model\n";

    model.initialise();
    model.run(1980, 2020);

    std::cout << std::endl;
    return 0;
}
