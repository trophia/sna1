#include <iostream>
#include <string>
#include <ctime>

#define TRACE_LEVEL 10

//#define FISHES_PARALLEL

#include "model.hpp"

Model model;

/**
 * Run the model
 */
void run(void) {
    model.run(1960, 2025);
}

/**
 * Generate trajectories using various seed population sizes
 */
void instances_seed_sensitivity(void) {
    std::ofstream tracks("output/instances_seed_sensitivity.tsv");
    std::ofstream times("output/instances_seed_sensitivity_times.tsv");
    for (auto num : {1e2, 1e3, 5e3, 1e4, 5e4, 1e5, 5e5, 1e6}) {
        model.fishes.instances_seed = num;
        for (auto iter = 0; iter < 30; iter++) {
            std::function<void()> callback([&](){
                tracks
                    << num << "\t"
                    << iter << "\t"
                    << now << "\t"
                    << model.fishes.biomass_spawners() << "\t"
                    << model.fishes.length_mean() << "\n";
            });
            std::clock_t start = std::clock();
            model.run(1960, 2015, &callback);
            times << num << "\t"
                  << iter << "\t"
                  << (std::clock() - start) / (double) CLOCKS_PER_SEC << std::endl;
        }
    }
}

/**
 * Main entry point; dispatches to one of the above tasks
 */
int main(int argc, char** argv) {
    model.initialise();

    try {
        if (argc == 1) {
            throw std::runtime_error("No task given");
        }
        std::string task = argv[1];
        if (task == "run") {
            run();
        } else if (task == "instances_seed_sensitivity") {
            instances_seed_sensitivity();
        } else {
            throw std::runtime_error("Unrecognised task");
        }
    } catch(std::exception& error) {
        std::cout << "************Error************\n"
                  << error.what() <<"\n"
                  << "******************************\n";
        return 1;
    } catch(...) {
        std::cout << "************Unknown error************\n";
        return 1;
    }

    model.finalise();

    return 0;
}
