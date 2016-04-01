//#define FISHES_PARALLEL

#include "model.hpp"

Model model;

/**
 * Run the model
 */
void run(void) {
    std::function<void()> callback([&](){
        auto fishes = model.fishes;
        auto harvest = model.harvest;
        fishes.biomass_update();
        std::cout
            << year(now) << "\t"
            << quarter(now) << "\t"
            << fishes.size() << "\t"
            << fishes.number(false) << "\t"
            << fishes.number()/1e6 << "\t"
            << sum(fishes.biomass_spawners)/sum(parameters.fishes_b0) << "\t"
            << sum(fishes.recruitment)/1e6 << "\t"
            << sum(fishes.recruitment_instances) << "\t"
            << sum(harvest.catch_observed) << "\t"
            << sum(harvest.catch_taken) << "\t"
            << harvest.attempts << "\t"
            << sum(harvest.catch_taken)/sum(harvest.biomass_vulnerable) << "\t"
            << fishes.age_mean() << "\t"
            << fishes.length_mean() << std::endl;
    });
    model.run(1900, 2020, &callback);
}

/**
 * Generate trajectories using various seed population sizes
 */
void instances_seed_sensitivity(void) {
    std::ofstream tracks("output/instances_seed_sensitivity.tsv");
    std::ofstream times("output/instances_seed_sensitivity_times.tsv");
    for (auto num : {1e2, 1e3, 5e3, 1e4, 5e4, 1e5, 5e5, 1e6}) {
        parameters.fishes_seed_number = num;
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
    for (auto mls : {26,27,28,29,30}) {
        for (auto iter = 0; iter < 10; iter++) {
            std::function<void()> callback([&](){
                auto y = year(now);
                if (y<2017){
                    parameters.harvest_mls = 27;
                } else {
                    parameters.harvest_mls = mls;
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
