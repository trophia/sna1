#pragma once

#include "monitor-tagging.hpp"

class Monitor {
 public:
    Tagging tagging;

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
        cpue = 0;
        age_sample = 0;
        length_pop = 0;
        length_sample = 0;
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
        // Calculate current CPUE by region and method
        for (auto region : regions) {
            for (auto method : methods) {
                cpue(region, method) = harvest.biomass_vulnerable(region, method);
            }
        }
        // Store series
        for (auto region : regions) {
            for (auto method : methods) {
                cpues(y, region, method) = cpue(region, method);
                for (auto age : ages) age_samples(y, region, method, age) = age_sample(region, method, age);
                for (auto length : lengths) length_samples(y, region, method, length) = length_sample(region, method, length);
            }
        }       
    }


    void finalise(std::string directory = "output/monitor") {
        boost::filesystem::create_directories(directory);

        tagging.finalise();

        cpues.write(directory + "/cpues.tsv");
        age_samples.write(directory + "/age_samples.tsv");
        length_samples.write(directory + "/length_samples.tsv");

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

        for (auto year : years){
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

                    cpue_file 
                        << year << "\t"
                        << region_code(region) << "\t"
                        << method_code(method) << "\t"
                        << cpues(year, region, method) << "\n";

                    {
                        double sum = 0;
                        for(auto age : ages) sum += age_sample(region, method, age);
                        if (sum > 0) {
                            age_file << year << "\t" << region_code(region) << "\t" << method_code(method) << "\t";
                            for(auto age : ages) age_file << age_samples(year, region, method, age) << "\t";
                            age_file << "\n";
                        }
                    }

                    {
                        double sum = 0;
                        for(auto length : lengths) sum += length_sample(region, method, length);
                        if (sum > 0) {
                            length_file << year << "\t" << region_code(region) << "\t" << method_code(method) << "\t";
                            for(auto length : lengths) length_file << length_samples(year, region, method, length) << "\t";
                            length_file << "\n";
                        }
                    }
                }
            }

        }
    }


};  // class Monitor
