#pragma once

#include "random.hpp"

#include "parameters.hpp"

#include "environ.hpp"
#include "fishes.hpp"
#include "harvest.hpp"
#include "monitor.hpp"

/**
 * The model
 *
 * Links together the sub-models e.g `Environ`, 'Fishes` and `Harvest`
 */
class Model {
 public:

    Environ environ;
    Fishes fishes;
    Harvest harvest;
    Monitor monitor;

    void initialise(void) {
        environ.initialise();
        fishes.initialise();
        harvest.initialise();
        monitor.initialise();
        parameters.initialise();
    }

    void finalise(void) {
        environ.finalise();
        fishes.finalise();
        harvest.finalise();
        monitor.finalise();
        parameters.initialise();
    }

    /**
     * The main update function for the model
     * called at each time step.
     *
     * This is optimised to reduce the number of loops through
     * the population of fish
     */
    void update(void) {
        auto y = year(now);
        //auto q = quarter(now);

        // As an optimisation, only do harvesting after a particular time
        // step 
        bool harvesting = y>=1900;

        /*****************************************************************
         * Spawning and recruitment
         ****************************************************************/
        
        // Update spawning biomass
        fishes.biomass_spawners_update();

        // Update recruitment
        fishes.recruitment_update();

        // Create and insert each recruit into the population
        uint slot = 0;
        for (auto region : regions) {
            for (uint index = 0; index < fishes.recruitment_instances(region); index++){
                Fish recruit;
                recruit.born(Region(region.index()));

                // Find a "slot" in population to insert this recruit
                // If no empty slot found add to end of fish population
                while (slot < fishes.size()) {
                    if (not fishes[slot].alive()) {
                        fishes[slot] = recruit;
                        break;
                    } else {
                        slot++;
                    }
                }
                if (slot == fishes.size()) {
                    fishes.push_back(recruit);
                }
            }
        }

        /*****************************************************************
         * Fish population dynamics
         *
         * Also calculates some vulnerable biomass by region and method.
         * This may be used for harvest calculations and/or monitoring
         ****************************************************************/

        harvest.biomass_vulnerable = 0;
        for (Fish& fish : fishes) {
            if (fish.alive()) {
                bool survives = fish.survival();
                if (survives) {
                    fish.growth();
                    fish.maturation();

                    if (harvesting) {
                        auto weight = fish.weight();
                        auto length_bin = fish.length_bin();
                        for (auto method : methods) {
                            harvest.biomass_vulnerable(fish.region,method) += weight * harvest.selectivity_at_length(method,length_bin);
                        }
                    }
                }
            }
        }
        harvest.biomass_vulnerable *= fishes.scalar;


        /*****************************************************************
         * Harvesting and monitoring
         ****************************************************************/

        if (harvesting) {

            // Update the current catches by region/method
            // from the catch history
            harvest.catch_observed_update();

            // Monitoring of CPUE (currently perfect)
            for (auto region : regions) {
                for (auto method : methods) {
                    monitor.cpue(region, method) = harvest.biomass_vulnerable(region, method);
                }
            }

            // Randomly draw fish and "assign" them with varying probabilities
            // to a particular region/method catch if that
            harvest.attempts = 0;
            harvest.catch_taken = 0;
            Array<bool,Regions,Methods> catch_caught = false;
            monitor.age_sample = 0;
            while(true) {
                Fish& fish = fishes[chance()*fishes.size()];
                if (fish.alive()) {
                    auto region = fish.region;
                    auto method = Method(int(chance()*Methods::size()));
                    if (not catch_caught(region,method)) {
                        // Is this fish caught?
                        auto caught = harvest.selectivity_at_length(method,fish.length_bin());
                        if (caught) {
                            // Is this fish retained?
                            if (fish.length >= parameters.harvest_mls(method)) {
                                fish.dies();
                                
                                // Add to catch taken for region/method
                                harvest.catch_taken(region,method) += fish.weight() * fishes.scalar;
                                catch_caught(region,method) = harvest.catch_taken(region,method) >= harvest.catch_observed(region,method);

                                // Age sampling, currently 100% sampling of catch
                                monitor.age_sample(region, method, fish.age_bin())++;

                                // If catch is taken for all region/methods then quit
                                if (sum(catch_caught) == catch_caught.size()) break;
                            } else {
                                // Does this fish die after released?
                                if (chance() < parameters.harvest_handling_mortality) {
                                    fish.dies();
                                }
                            }

                        }

                        // Tagging currently not active
                        // Need to consider in relation to harvest or
                        // harvest induced mortality above
                        #if 0
                        // Is this fish already tagged?
                        if(fish.tag) {
                            // Is it reported?
                            // TODO reporting probability may be dependent upon method and area
                            if (fish.tag and chance()< 0.01) {
                                monitor.tagging.recover(fish);
                            }
                        } else {
                            // Does this fish get tagged?
                            // TODO determine tagging probability from tag release design and catches by method area
                            if (not fish.tag and chance()< 0.1) {
                                monitor.tagging.mark(fish);
                            }
                        }
                        #endif

                    }
                    harvest.attempts++;
                    if (harvest.attempts > fishes.size() * 100) {
                        std::cerr << y << std::endl
                                  << "Catch taken so far:\n" << harvest.catch_taken << std::endl
                                  << "Catch observed:\n" << harvest.catch_observed << std::endl;
                        throw std::runtime_error("Too many attempts to take catch. Something is probably wrong.");
                    };
                }
            }

        };

    }

    /**
     * Take the population to pristine equilibium
     *
     * This method simply calls `equilibrium()` and then sets population level attributes
     * like `biomass_spawners_pristine` and `scalar`
     */
    void pristine(Time time, std::function<void()>* callback = 0){
        // Set `now` to some arbitrary time (but high enough that fish
        // will have a birth time (uint) greater than 0)
        now = 200;
        // Keep recruitment fixed at a constant level that will produce the
        // wanted `seed_number` of individuals in equilibrium
        fishes.recruitment_mode = 'p';
        double number = 0;
        for (int age = 0; age < 200; age++) number += std::exp(-parameters.fishes_m*age);
        for (auto region : regions) {
            fishes.recruitment_pristine(region) = 
                parameters.fishes_seed_number/number *
                parameters.fishes_b0(region)/sum(parameters.fishes_b0);
        }
        fishes.scalar = 1;
        // Seed the population with seed individuals that have attribute values 
        // intended to reduce the burn in time for the initial population
        fishes.clear();
        fishes.resize(parameters.fishes_seed_number);
        for (Fish& fish : fishes) fish.seed();
        // Burn in
        // TODO Currently just burns in for an arbitarty number of iterations
        // Should instead exit when stability in population characteristics
        int steps = 0;
        while (steps<100) {
            update();
            if (callback) (*callback)();
            steps++;
            now++;
        }
        // Re-calibrate the fishes birth to current time
        // The fish have arbitrary `birth` times so we need to "re-birth"
        // them so that the population is in equilbrium AND "current"
        auto diff = time-now;
        for (auto& fish : fishes) {
            fish.birth += diff;
        }
        now = time;
        // Set scalar so that the current spawner biomass 
        // matches the intended value
        fishes.scalar = sum(parameters.fishes_b0)/sum(fishes.biomass_spawners);
        // Adjust accordingly
        fishes.biomass_spawners *= fishes.scalar;
        fishes.recruitment_pristine *= fishes.scalar;
        // Go to "normal" recruitment
        fishes.recruitment_mode = 'n';
    }

    void run(Time start, Time finish, std::function<void()>* callback = 0) {
        // Create initial population of fish
        pristine(start, callback);
        // Iterate over times
        now = start;
        while (now <= finish) {
            update();
            if (callback) (*callback)();
            now++;
        }
    }

};  // end class Model
