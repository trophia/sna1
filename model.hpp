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

        // Update each fish
        std::vector<Fish> eggs;
        #if !defined(FISHES_PARALLEL)
            for (Fish& fish : fishes) {
                if (fish.survival()) {
                    fish.growth();
                    fish.maturation();
                    fish.movement();
                    fish.spawning();
                }
            }
        #else
            int each = fishes.size()/4;

            std::thread thread0(update_task, &fishes, environ, 0, each);
            std::thread thread1(update_task, &fishes, environ, each, each);
            std::thread thread2(update_task, &fishes, environ, each*2, each);
            std::thread thread3(update_task, &fishes, environ, each*3, each);

            thread0.join();
            thread1.join();
            thread2.join();
            thread3.join();
        #endif

        // Insert eggs into population
        auto replace = fishes.begin();
        auto end = fishes.end();
        for (Fish egg : eggs) {
            // Try to find a dead fish in `fishes` which can be replaced by
            // the new egg
            while(replace!=end and replace->alive()) replace++;
            // If possible replace a dead fish, otherwise add to fishes
            if(replace!=end) *replace = egg;
            else fishes.push_back(egg);
        }
    
        if (now >= 1970) {
            for (Fish& fish : fishes) {
                if (chance.random() < 0.05) {  // encountered
                    if (chance.random() < fleet.selectivity_at_length(fish.length_bin())) {  // caught
                        if (fish.length < fleet.mls) {  // returned
                            if (chance.random() < fleet.handling_mortality) {  // returned but dies
                                fish.dies();
                            }
                        } else {
                            fish.dies();
                        }
                    }
                }
            }
        }

        /**
         * Update of the tagging programme
         *
         * Currently very simplistic and not integrated with fleet
         * dynamics for release or recpature
         */
        auto tagging = monitor.tagging;
        // Marking fish
        if(y==1994 or y==2016) {
            for (Fish& fish : fishes) {
                if (fish.alive() and fish.length>30 and chance.random()< 0.1) {
                    tagging.mark(fish);
                }
            }
        }
        // Recovering fish
        if(y==1994 or y==1995 or y==2016 or y==2017) {
            for (Fish& fish : fishes) {
                if (fish.alive() and fish.tag and chance.random()< 0.01) {
                    tagging.recover(fish);
                }
            }
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
        // Seed the population. `resize()` uses the default
        // constructor which creates seed individuals that have attribute values 
        // intended to reduce the burn in time for the initial population
        fishes.clear();
        fishes.resize(fishes.instances_seed);
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
        fishes.scalar *= fishes.biomass_spawners_pristine/fishes.biomass_spawners();
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
