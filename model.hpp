#pragma once

#include <iostream>

#include "environ.hpp"
#include "fishes.hpp"
#include "fleet.hpp"
#include "monitor.hpp"

/**
 * The model
 *
 * Links together the sub-models `Environ`, 'Fishes` and `Fleet`
 */
class Model {
public:

    Environ environ;
    Fishes fishes;
    Fleet fleet;
    Monitor monitor;

    void initialise(void) {
        environ.initialise();
        fishes.initialise();
        fleet.initialise();
        monitor.initialise();
    }

    void finalise(void) {
        environ.finalise();
        fishes.finalise();
        fleet.finalise();
        monitor.finalise();
    }

    void trace(void) {
        #if TRACE_LEVEL>0
            std::cout<<now<<"\t";
            fishes.trace();
            std::cout<<"\n";
        #endif
    }

    void update(void) {
        auto y = year(now);
        auto q = quarter(now);

        // First loop over fish...
        fishes.aggregates_reset();
        for(auto fish : fishes){
            if (fish.alive()) {
                // Natural mortality
                bool survives = fish.survival();
                if (survives) {
                    // Growth
                    fish.growth();
                    // Maturation
                    fish.maturation();
                    // Movement
                    fish.movement();
                    
                    // Update population statistics used for recruitment,
                    // fishing and tagging below
                    fishes.aggregates_add(fish);
                }
            }
        }

        // Recruitment
        // Create a vector of recruits which will replace fish that die below
        auto recruits = fishes.recruitment();
        // Iterator pointing to the current recruit to be used as a replacement
        auto replacement = recruits.begin();

        // Second loop over fish...
        auto iterator = fishes.begin();
        while(iterator != fishes.end()){
            Fish& fish = *iterator;
            // Fishing
            // Apply exploitation
            // TODO this is just placeholder code
            if (now >= 1970) {
                if (chance.random() < 0.05) {  // encountered
                    if (chance.random() < fleet.selectivity_at_length(fish.length_bin())) {  // caught
                        if (fish.length < fleet.mls) {  // returned
                            if (chance.random() < fleet.handling_mortality) {  // returned but dies
                                fish.dies();
                            }
                        } else {
                            fish.dies();
                        }
                        // Is this fish already tagged?
                        if(fish.tag)

                        // Does this fish get tagged?
                        // Currently assumes we don't tag a fish that has already been tagged.
                        // TODO determine tagging probability from tag release design and catches by method area
                        if (not fish.tag and chance.random()< 0.1) {
                            monitor.tagging.mark(fish);
                        }
                        // Is this fish already tagged and recovered?
                        // TODO recovery probability willl be dependent upon method and area
                        if (fish.tag and chance.random()< 0.01) {
                            monitor.tagging.recover(fish);
                        }
                    }
                }
            }

            // Recruitment
            // If fish has died (either this loop or previously), 
            // and there are still recruits to be inserted, replace it
            if (not fish.alive() and replacement != recruits.end()) {
                *iterator = *replacement;
                replacement++;
            }

            iterator++;
        }

        // Recruitment
        // If there are still recruits to be inserted then append them
        while (replacement != recruits.end()) {
            fishes.push_back(*replacement);
            replacement++;
        }

    }

    /**
     * An update task for use when `update()` is parallelized
     * 
     * @param fishes  Vector of `Fish`
     * @param environ Current environment
     * @param start   Index of fish to start updating
     * @param num     Number of fish to update
     */
    static void update_task(std::vector<Fish>* fishes, const uint& start, const uint& num) {
        auto fish = fishes->begin()+start;
        auto end = fish+num;
        while (fish != end) {
            //fish->update(environ);
            fish++;
        }
    }

    /**
     * Take the population to equilibrium
     */
    void equilibrium(Time time) {
        // Set `now` to some arbitrary time
        now = 0;
        // Seed the population. `create()` creates seed individuals that have attribute values 
        // intended to reduce the burn in time for the initial population
        fishes.clear();
        fishes.resize(fishes.instances_seed);
        for(auto fish : fishes) fish.seed();
        // Burn in
        // TODO Currently just burns in for an arbitarty number of iterations
        // Should instead exit when stability in population characteristics
        while(now<100){
            #if TRACE_LEVEL>0
                std::cout<<now<<"\t";
                trace();
                std::cout<<"\n";
            #endif
            update();
            now++;
        }
        // Re-age the fish to current time
        // The fish have arbitrary `birth` times so we need to "re-age"
        // them so that the population is in equilbrium AND "current"
        auto diff = time-now;
        for (auto& fish : fishes) {
            fish.birth += diff;
        }
        now = time;
    }

    /**
     * Take the population to pristine equilibium
     *
     * This method simply calls `equilibrium()` and then sets population level attributes
     * like `biomass_spawners_pristine` and `scalar`
     */
    void pristine(Time time){
        // Take population to equilibrium
        equilibrium(time);
        // Adjust scalar so that the current spawner biomass 
        // matches the intended value
        fishes.scalar *= fishes.biomass_spawners_pristine/fishes.biomass_spawners;
    }

    void run(Time start, Time finish, std::function<void()>* callback = 0) {
        // Create initial population of fish
        pristine(start);
        // Iterate over times
        now = start;
        while (now <= finish) {
            trace();
            if (callback) (*callback)();
            update();
            now++;
        }
    }

};  // end class Model
