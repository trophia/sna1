//#define FISHES_PARALLEL

#include "model.hpp"

Model model;

/**
 * Run the model
 */
void run(void) {

    std::ofstream model_annual("output/model-annual.tsv");
    model_annual << "year\t";
    for (auto region : regions) {
        auto rc = region_code(region.index());
        model_annual 
            << rc << "_B\t" 
            << rc << "_R\t";
        for (auto method : methods) {
            auto mc = method_code(method.index());
            model_annual  
                << rc << "_" << mc << "_C\t"
                << rc << "_" << mc << "_E\t";
        }
    }
    model_annual << "\n";
    
    std::ofstream cpue_file("output/monitor/cpue.tsv");
    cpue_file<<"year\tEN_LL\tEN_BT\tEN_DS\tEN_RE\tHG_LL\tHG_BT\tHG_DS\tHG_RE\tBP_LL\tBP_BT\tBP_DS\tBP_RE\n";

    std::ofstream age_file("output/monitor/age.tsv");
    age_file << "year\tregion\tmethod\t";
    for(auto age : ages) age_file << "age" << age << "\t";
    age_file << "\n";

    std::function<void()> callback([&](){
        auto y = year(now);
        auto fishes = model.fishes;
        auto harvest = model.harvest;
        auto monitor = model.monitor;

        // Output to screen
        fishes.biomass_update();
        std::cout
            << y << "\t"
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

        if (y>=1900) {

            model_annual << y << "\t";
            for (auto region : regions) {
                model_annual 
                    << fishes.biomass_spawners(region) << "\t"
                    << fishes.recruitment(region) << "\t";
                for (auto method : methods) {
                    model_annual 
                        << harvest.catch_observed(region, method) << "\t"
                        << harvest.catch_observed(region, method) / harvest.biomass_vulnerable(region, method) << "\t";
                }
            }
            model_annual << "\n";
            
            cpue_file << y << "\t";
            for (auto region : regions) {
                for (auto method : methods) {
                    cpue_file << monitor.cpue(region, method) << "\t";
                }
            }
            cpue_file << "\n";

            for (auto region : regions) {
                for (auto method : methods) {
                    age_file << y << "\t" << region << "\t" << method << "\t";
                    for(auto age : ages) age_file << monitor.age_sample(region, method, age) << "\t";
                    age_file << "\n";
                }
            }

        }
        
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
