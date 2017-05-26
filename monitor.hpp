#pragma once

#include "monitor-tagging.hpp"

class Monitor {
 public:
    Tagging tagging;

    // An optimization to store the current year's monitoring components
    MonitoringComponents components;

    /**
     * Catches by year, region and method
     */
    Array<double, Years, Regions, Methods> catches;

    /**
     * Biomass of spawners by year, region and method
     */
    Array<double, Years, Regions> biomass_spawners;

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
     * Current lengths of fish in population by region
     */
    Array<double, Regions, Lengths> length_pop;

    /**
     * Sample of measured fish by region, method and length bin
     */
    Array<double, Regions, Methods, Lengths> length_sample;

    /**
     * Samples of measured fish by year, region, method and length bin
     */
    Array<double, Years, Regions, Methods, Lengths> length_samples;


    void initialise(void) {
        tagging.initialise();
    }

    /**
     * Reset things at the start of each time step
     */
    void reset() {
        auto y = year(now);

        components = parameters.monitoring_programme(y);
        length_pop = 0;
        cpue = 0;
        age_sample = 0;
        length_sample = 0;
    }

    void population_sample(Region region, const Fish& fish) {
        length_pop(region, fish.length_bin())++;
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
    }


};  // class Monitor
