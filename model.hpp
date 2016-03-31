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
        auto q = quarter(now);

        /*****************************************************************
         * Spawning and recruitment
         ****************************************************************/
        
        // Update spawning biomass
        fishes.biomass_spawners_update();

        // Update recruitment
        fishes.recruitment_update();

        // Create and insert each recruit into the population
        uint slot = 0;
        for (uint index = 0; index < fishes.recruitment_instances; index++){
            Fish recruit;

            // TODO determine recruits by area
            // instad of this temporary random assignment
            Area area = chance()*Areas::size();
            recruit.born(area);

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

        /*****************************************************************
         * Fish population dynamics
         *
         * Also calculates some aggregate statistics used below for
         * harvest and tagging
         ****************************************************************/

        for (Fish& fish : fishes) {
            if (fish.alive()) {
                bool survives = fish.survival();
                if (survives) {
                    fish.growth();
                    fish.maturation();
                }
            }
        }

        /*****************************************************************
         * Harvesting and tagging
         *
         ****************************************************************/

        for(Fish& fish: fishes){
            // Harvest and tagging
            if (now >= 1970) {
                // TODO encounter rate based on harvest
                if (chance() < 0.5) {  // encountered
                    if (chance() < harvest.selectivity_at_length(fish.length_bin())) {  // caught

                        if (fish.length < harvest.mls) {  // returned
                            if (chance() < harvest.handling_mortality) {  // returned but dies
                                fish.dies();
                            }
                        } else {
                            fish.dies();
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
                }
            }
        }

    }

    /**
     * Take the population to pristine equilibium
     *
     * This method simply calls `equilibrium()` and then sets population level attributes
     * like `biomass_spawners_pristine` and `scalar`
     */
    void pristine(Time time, std::function<void()>* callback = 0){
        // Set `now` to some arbitrary time (but high enough that fish
        // willl have a birth time (uint) greater than 0)
        now = 1000;
        // Keep recruitment fixed at a constant level that will produce the
        // wanted `seed_number` of individuals in equilibrium
        fishes.recruitment_mode = 'p';
        double number = 0;
        for(int age = 0; age < 200; age++) number += std::exp(-parameters.fishes_m*age);
        fishes.recruitment_pristine = parameters.fishes_seed_number/number;
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
            if (callback) (*callback)();
            update();
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
        fishes.scalar = parameters.fishes_b0/fishes.biomass_spawners;
        // Adjust the accordingly
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
            if (callback) (*callback)();
            update();
            now++;
        }
    }

};  // end class Model
