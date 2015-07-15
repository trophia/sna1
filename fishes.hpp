#pragma once

#include <fstream>
#include <vector>
#include <thread>

#include <stencila/query.hpp>
using namespace Stencila::Queries;

#include <fsl/math/probability/lognormal.hpp>
using Fsl::Math::Probability::Lognormal;

#include "dimensions.hpp"
#include "environ.hpp"

/**
 * Fish parameters
 */
class FishParameters {
 public:
    Lognormal create_length = {30,30};
    float natural_mortality_rate = 0.1;
};

/**
 * A fish
 */
class Fish {
 public:
    /**
     * Stock this fish belongs to
     */
    Stock stock;

    /**
     * Time of birth of this fish
     */
    Time birth;

    /**
     * Time of death of this fish
     */
    Time death;

    /**
     * Sex of this fish
     */
    Sex sex;

    /**
     * Length (cm) of this fish
     */
    float length;

    /**
     * Location of this fish
     */
    Area location;

    /**
     * Is this fish tagged?
     */
    bool tagged;


    static FishParameters params;

    /**
     * Create a fish 
     * 
     * Create as in the Book of Genesis creation. This constructor is needed 
     * for intial seeding of the population prior to burning it in.
     */
    Fish(void) {
        auto stock = E;//create_stock.random();
        auto location = EN;//create_location.random();
        auto length = params.create_length.random();
        init(stock,location,length);
    }

    /**
     * Spawn a fish
     */
    Fish(const Fish& father, const Fish& mother) {
        init(father.stock,father.location);
    }

    /**
     * Initialise a fish
     * 
     * Should only be called from the constructors above
     */
    void init(Stock stock, Area location, uint length = 0){
        Fish::stock = stock;
        birth = now;
        death = 0;
        sex = male;//sex_at_birth.random();
        Fish::length = length;
        Fish::location = location;
        tagged = false;
    }

    /**
     * Is this fish alive?
     */
    bool alive(void) const {
        return death == 0;
    }

    /**
     * Get the age of this fish
     */
    float age(void) const {
        return years(now, birth);
    }

    /**
     * Does this fish survive this time step?
     */
    bool survive(void) {
        auto dies = chance.random() < params.natural_mortality_rate;
        if (dies) death = now;
        return dies;
    }

    /**
     * Increase the length of this fish
     */
    Fish& grow(void) {
        auto incr = 0;//growth_function(length);
        length += incr;
        return *this;
    }

    /**
     * Move this fish
     */
    Fish& move(void) {
        location = EN;//movement_probabilities(location);
        return *this;
    }

    /**
     * Update this fish (i.e. operate all processes on it)
     */
    bool update(const Environ& environ){
        if(survive()){
            grow();
            move();
            return true;
        }
        else return false;
    }
};  // end class Fish

FishParameters Fish::params;

class Fishes {
 public:

    std::vector<Fish> fishes;

    /**
     * The actual number of fish in the entire population
     */
    unsigned int number;

    uint start_number;
    uint burn_times;

    Count alive;
    Mean length_mean;
    Frequency length_freq;

    void start(const Environ& environ) {
        // Seed the population
        fishes.clear();
        fishes.resize(start_number);
        // Burn in
        
    }

    void update(const Environ& environ) {

        #if !defined(FISHES_PARALLEL)

            for (Fish& fish : fishes) {
                fish.update(environ);
            }

        #else

            int each = fishes.size()/6;

            std::thread thread0(update_task, &fishes, environ, 0, each);
            std::thread thread1(update_task, &fishes, environ, each, each);
            std::thread thread2(update_task, &fishes, environ, each*2, each);
            std::thread thread3(update_task, &fishes, environ, each*3, each);
            std::thread thread4(update_task, &fishes, environ, each*4, each);
            std::thread thread5(update_task, &fishes, environ, each*5, each);

            thread0.join();
            thread1.join();
            thread2.join();
            thread3.join();
            thread4.join();
            thread5.join();

        #endif

        // Spawn 
        for(auto& fish : fishes){
            if(not fish.alive()){
                Fish egg;
                fish = egg;
            };
        }
    }

    static void update_task(std::vector<Fish>* fishes, const Environ& environ, const uint& start, const uint& num) {
        auto fish = fishes->begin()+start;
        auto end = fish+num;
        while (fish != end) {
            fish->update(environ);
            fish++;
        }
    }

    void enumerate(void) {
        alive.reset();
        length_mean.reset();
        length_freq.reset(100);
        for (auto fish : fishes){
            if(fish.alive()){
                alive.append();
                length_mean.append(fish.length);
                length_freq.append(fish.length);
                //location_freq(fish.location);
            }
        }
    }

    void track(void){ 
        static std::ofstream* general = nullptr;
        if(not general) general = new std::ofstream("output/fish/general.tsv");
        static std::ofstream* length_freq_file = nullptr;
        if(not length_freq_file) length_freq_file = new std::ofstream("output/fish/length_freq.tsv");

        enumerate();

        (*general)
                <<now<<"\t"
                <<fishes.size()<<"\t"
                <<alive<<"\t"
                <<length_mean<<std::endl;

        uint index = 0;
        for(auto count : length_freq.result()){
            (*length_freq_file)
                <<now<<"\t"
                <<index++<<"\t"
                <<count<<std::endl;
        };
    }

};  // end class Fishes
