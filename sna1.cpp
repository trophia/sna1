//#define FISHES_PARALLEL

#include "model.hpp"

Model model;

/**
 * Run the model
 */
void run(void) {
    std::function<void()> callback([&](){
        auto fishes = model.fishes;
        auto bs = fishes.biomass_spawners_area();
        std::cout
            << year(now) << "\t"
            << quarter(now) << "\t"
            << fishes.size() << "\t"
            << fishes.number(false) << "\t"
            << fishes.number() << "\t"
            << fishes.biomass() << "\t"
            << fishes.biomass_spawners << "\t"
            << bs(EN) << "\t" << bs(HG) << "\t" << bs(BP) << "\t"
            << fishes.age_mean() << "\t"
            << fishes.length_mean() << std::endl;
    });
    model.run(1960, 2025, &callback);
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
                    << model.fishes.biomass_spawners << "\t"
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
 * Generate trajectories using various MLS and selectivities
 */
void mls_changes_example(void) {
    std::ofstream tracks("output/mls_changes_example.tsv");
    model.fishes.instances_seed = 100000;
    for (auto mls : {26,27,28,29,30}) {
        for (auto iter = 0; iter < 10; iter++) {
            std::function<void()> callback([&](){
                auto y = year(now);
                if (y<2017){
                    model.harvest.mls = 27;
                } else {
                    model.harvest.mls = mls;
                }
                if (y>1980) {
                    tracks
                        << mls << "\t"
                        << iter << "\t"
                        << now << "\t"
                        << model.fishes.biomass_spawners << "\t"
                        << model.fishes.length_mean() << "\n";
                }
            });
            model.run(1960, 2030, &callback);
        }
    }
}


/**
 * Main entry point; dispatches to one of the above tasks
 */
int main(int argc, char** argv) {
    parameters.initialise();
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
        } else if (task == "mls_changes_example") {
            mls_changes_example();
        } else {
            std::cout << "No task specified" <<std::endl;
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
    parameters.finalise();

    return 0;
}
