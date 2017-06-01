#pragma once

#include "monitor-tagging.hpp"

class Monitor {
 public:
    Tagging tagging;

    // An optimization to store the current year's monitoring components
    MonitoringComponents components;

    /**
     * Population numbers by year and region
     */
    Array<int, Years, Regions> population_numbers;

    /**
     * Population length distribution by region for current year
     */
    Array<double, Regions, Lengths> population_lengths_sample;

    /**
     * Biomass of spawners by year, region and method
     */
    Array<double, Years, Regions> biomass_spawners;

    /**
     * Catches by year, region and method
     */
    Array<double, Years, Regions, Methods> catches;

    /**
     * Current CPUE by region and method
     *
     * Equivalent to vulnerable biomas with observation error
     * and opotentially other things (e.g hyperdepletion) added to it
     */
    Array<double, Regions, Methods> cpue;

    /**
     * CPUE by year, region and method
     *
     * Equivalent to vulnerable biomas with observation error
     * and opotentially other things (e.g hyperdepletion) added to it
     */
    Array<double, Years, Regions, Methods> cpues;

    /**
     * Current sample of aged fish by region, method and age bin
     */
    Array<double, Regions, Methods, Ages> age_sample;

    /**
     * Sample of aged fish by region, method and age bin
     */
    Array<double, Years, Regions, Methods, Ages> age_samples;

    /**
     * Sample of measured fish by region, method and length bin
     */
    Array<double, Regions, Methods, Lengths> length_sample;

    /**
     * Samples of measured fish by year, region, method and length bin
     */
    Array<double, Years, Regions, Methods, Lengths> length_samples;


    void initialise(void) {
        population_numbers = 0;
        tagging.initialise();
    }

    /**
     * Reset things at the start of each time step
     */
    void reset() {
        auto y = year(now);

        components = parameters.monitoring_programme(y);
        population_lengths_sample = 0;
        cpue = 0;
        age_sample = 0;
        length_sample = 0;
    }

    /**
     * Monitor the fish population
     *
     * In reality, we can never sample the true underlying population of fish. 
     * This method just allows us to capture some true population statistics for things
     * like examining the precision and bias of our estimates.
     * 
     * @param fish   A fish
     */
    void population(const Fish& fish) {
        auto y = year(now);
        // Add fish to numbers by Year and Region
        population_numbers(y, fish.region)++;
        // Add fish to numbers by Region and Length for current year
        population_lengths_sample(fish.region, fish.length_bin())++;
        // Tagging specific population monitoring
        tagging.population(fish);
    }

    void catch_sample(Region region, Method method, const Fish& fish) {
        if (components.A) age_sample(region, method, fish.age_bin())++;
        if (components.L) length_sample(region, method, fish.length_bin())++;
    }

    /**
     * Update things at the end of each time start
     */
    void update(const Fishes& fishes, const Harvest& harvest) {
        auto y = year(now);
        // Record spawning biomass
        for (auto region : regions) {
            biomass_spawners(y, region) = fishes.biomass_spawners(region);
        }   
        // Record catches
        for (auto region : regions) {
            for (auto method : methods) {
                catches(y, region, method) = harvest.catch_taken(region, method);
            }
        }

        // Calculate current CPUE by region and method and store it
        if (components.C) {
            for (auto region : regions) {
                for (auto method : methods) {
                    cpue(region, method) = harvest.biomass_vulnerable(region, method);
                    cpues(y, region, method) = cpue(region, method);
                }
            }
        }

        // Store current age sample
        if (components.A) {
            for (auto region : regions) {
                for (auto method : methods) {
                    for (auto age : ages) age_samples(y, region, method, age) = age_sample(region, method, age);
                }
            }
        }

        // Store current length sample
        if (components.L) {
            for (auto region : regions) {
                for (auto method : methods) {
                    for (auto length : lengths) length_samples(y, region, method, length) = length_sample(region, method, length);
                }
            }
        }
    }


    void finalise(std::string directory = "output/monitor") {
        boost::filesystem::create_directories(directory);

        tagging.finalise();

        population_numbers.write(directory + "/population_numbers.tsv");
        
        cpues.write(directory + "/cpues.tsv");
        age_samples.write(directory + "/age_samples.tsv");
        length_samples.write(directory + "/length_samples.tsv");

        parameters.monitoring_programme.write(
            directory + "/programme.tsv", 
            {"cpue", "lengths", "ages"}, 
            [](std::ostream& stream, const MonitoringComponents& components) {
                stream << components.C << "\t" << components.L  << "\t" << components.A;
            }
        );

        // Files for CASAL
        auto casal_directory = directory + "/casal";
        boost::filesystem::create_directories(casal_directory);

        std::ofstream catch_file(casal_directory + "/catch.tsv");
        catch_file << "year\tregion\tmethod\tcatch\n";

        std::ofstream biomass_file(casal_directory + "/biomass.tsv");
        biomass_file << "year\tregion\tbiomass\n";
        
        std::ofstream cpue_file(casal_directory + "/cpue.tsv");
        cpue_file<<"year\tregion\tmethod\tcpue\n";

        std::ofstream age_file(casal_directory + "/age.tsv");
        age_file << "year\tregion\tmethod\t";
        for(auto age : ages) age_file << "age" << age << "\t";
        age_file << "\n";

        std::ofstream length_file(casal_directory + "/length.tsv");
        length_file << "year\tregion\tmethod\t";
        for(auto length : lengths) length_file << "length" << length << "\t";
        length_file << "\n";

        // Override of `method_code` method to output `REC`
        auto method_code = [](Stencila::Level<Methods> method){
            if (method == RE) return std::string("REC");
            else return ::method_code(method);
        };

        for (auto year : years) {
            auto components = parameters.monitoring_programme(year);

            for (auto region : regions) {

                biomass_file 
                    << year << "\t"
                    << region_code(region) << "\t"
                    << biomass_spawners(year, region) << "\n";

                for (auto method : methods) {
                    catch_file
                        << year << "\t"
                        << region_code(region) << "\t"
                        << method_code(method) << "\t"
                        << catches(year, region, method) << "\n";

                    if (components.C) {
                        cpue_file 
                            << year << "\t"
                            << region_code(region) << "\t"
                            << method_code(method) << "\t"
                            << cpues(year, region, method) << "\n";
                    }

                    if (components.A) {
                        age_file << year << "\t" << region_code(region) << "\t" << method_code(method) << "\t";
                        for(auto age : ages) age_file << age_samples(year, region, method, age) << "\t";
                        age_file << "\n";
                    }

                    if (components.L) {
                        length_file << year << "\t" << region_code(region) << "\t" << method_code(method) << "\t";
                        for(auto length : lengths) length_file << length_samples(year, region, method, length) << "\t";
                        length_file << "\n";
                    }
                }
            }

        }

        // Output parameters to be inserted in 'population.csl'
        std::ofstream population_file(casal_directory + "/parameters.tsv");
        population_file << "par\tvalue\n";

        // Growth parameters
        double growth_20;
        double growth_50;
        double growth_cv;
        double growth_sdmin = parameters.fishes_growth_temporal_sdmin;
        if (parameters.fishes_growth_variation == 't') {
            // Parameters calculated from mean of k and linf
            auto growth_slope = std::exp(-parameters.fishes_k_mean)-1;
            auto growth_intercept = -growth_slope * parameters.fishes_linf_mean;
            growth_20 = growth_intercept + 20 * growth_slope;
            growth_50 = growth_intercept + 50 * growth_slope;
            growth_cv = parameters.fishes_growth_temporal_cv;
        } else {
            // Parameters calculated by generating 1000 fish and 
            // calculating mean and cv of growth parameters
            Mean growth_intercept_mean;
            StandardDeviation growth_intercept_sd;
            Mean growth_slope_mean;
            for (int index = 0; index < 1000; index++) {
                Fish fish;
                fish.born(EN);
                growth_intercept_mean.append(fish.growth_intercept);
                growth_intercept_sd.append(fish.growth_intercept);
                growth_slope_mean.append(fish.growth_slope);
            }
            growth_20 = growth_intercept_mean + 20 * growth_slope_mean;
            growth_50 = growth_intercept_mean + 50 * growth_slope_mean;
            growth_cv = growth_intercept_sd/growth_intercept_mean;
        }
        population_file
            << "growth_20\t" << growth_20 << "\n"
            << "growth_50\t" << growth_50 << "\n"
            << "growth_cv\t" << growth_cv << "\n"
            << "growth_sdmin\t" << growth_sdmin << "\n";
        population_file.close();

    }


};  // class Monitor
