#pragma once

#include <fstream>
#include <vector>
#include <thread>

#include <stencila/array-static.hpp>
using Stencila::Array;
#include <stencila/query.hpp>
using namespace Stencila::Queries;

#include <fsl/math/probability/truncated.hpp>
using Fsl::Math::Probability::Truncated;
#include <fsl/math/probability/normal.hpp>
using Fsl::Math::Probability::Normal;
#include <fsl/math/probability/lognormal.hpp>
using Fsl::Math::Probability::Lognormal;
#include <fsl/math/probability/exponential.hpp>
using Fsl::Math::Probability::Exponential;
#include <fsl/math/probability/discrete.hpp>
using Fsl::Math::Probability::Discrete;

#include <fsl/population/growth/von-bert.hpp>
using Fsl::Population::Growth::VonBert;

#include "dimensions.hpp"
#include "environ.hpp"

/**
 * Fish parameters
 */
class FishParameters {
public:

    /**
     * Stock distribution of initial seed population
     */
    Discrete<Stock,2> seed_stock = {
        {W,E},
        {0.6,0.4}
    };

    /**
     * Stock specific area distribution of initial seed population
     */
    Array<
        Discrete<Area,3>,
        Stocks
    > seed_area;

    /**
     * Total mortality of the initial seed population
     *
     * Determines the equilibrium age structure of the seed population.
     */
    double seed_total_mortality = 0.1;

    /**
     * Exponential distribution for the age structure of the 
     * seed population
     */
    Exponential seed_age;

    /**
     * Sex at birth
     */
    Discrete<Sex,2> sex_at_birth = {
        {male,female},
        {0.5,0.5}
    };

    /**
     * Instantaneous rate of natural mortality
     */
    float natural_mortality_rate = 0.1;

    /**
     * Length at age
     *
     * Uses a normal ditribtion around the mean age at length
     * determined by the von-Bertallanfy growth function.
     */
    VonBert length_at_age_func = {
        k:0.1,
        linf:100
    };
    float length_at_age_cv = 0.1;
    Array<Normal,Ages> length_at_age;

    /**
     * Initialise the parameters
     */
    void initialise(void){

        seed_area(W) = Discrete<Area,3>({EN,HG,BP},{0.5,0.4,0.1});
        seed_area(E) = Discrete<Area,3>({EN,HG,BP},{0.1,0.4,0.5});

        seed_age = Exponential(seed_total_mortality);

        for(auto age : ages){
            auto mean = length_at_age_func.value(age.index());
            auto sd = std::max(mean*length_at_age_cv,1.);
            length_at_age(age) = Normal(mean,sd);
        }
        length_at_age.write("output/fishes/length_at_age.tsv",{"mean","sd"},[](std::ostream& file,const Normal& distribution){
            file<<distribution.mean()<<"\t"<<distribution.sd();
        });
    }
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
    Area area;

    /**
     * Is this fish tagged?
     */
    bool tagged;

    /**
     * Parameters for `Fish` dynamics
     */
    static FishParameters params;

    /**
     * Create a fish 
     * 
     * This constructor is needed for intial seeding of the 
     * population prior to burning it in. In other circumstances
     * the `Fish(const Fish& father, const Fish& mother)` constructor
     * is used.
     */
    Fish(void) {
        stock = params.seed_stock.random();
        area = params.seed_area(stock).random();
        auto age = std::max(1.,std::min(params.seed_age.random(),30.));
        birth = now-age;
        death = 0;
        sex = params.sex_at_birth.random();
        length = std::max(params.length_at_age(age).random(),0.);
        tagged = false;
    }

    /**
     * Spawn a fish
     */
    Fish(const Fish& father, const Fish& mother) {
        stock = father.stock;
        area = father.area;
        birth = now;
        death = 0;
        sex = params.sex_at_birth.random();
        length = 1;
        tagged = false;
    }

    /*************************************************************
     * Attributes
     ************************************************************/

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
     * Get the age bin of this fish
     */
    int age_bin(void) const {
        return ::age_bin(age());
    }

    /**
     * Get the length bin of this fish
     */
    int length_bin(void) const {
        return ::length_bin(length);
    }


    /*************************************************************
     * Processes
     ************************************************************/

    /**
     * Does this fish survive this time step?
     */
    bool survive(void) {
        auto survives = chance.random() > params.natural_mortality_rate;
        if(not survives) death = now;
        return survives;
    }

    /**
     * Increase the length of this fish
     */
    Fish& grow(void) {
        length = std::max(params.length_at_age(age_bin()).random(),1.0);
        return *this;
    }

    /**
     * Move this fish
     */
    Fish& move(void) {
        //! @todo
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

/**
 * The population of `Fish`
 */
class Fishes {
 public:

    /**
     * List of individual fish
     *
     * We don't attempt to model every single fish in the population. Rather,
     * `fishes` are intended to be representative of the larger population.
     */
    std::vector<Fish> fishes;

    /**
     * Number of fish in the population
     *
     * Used to scale the things like biomass etc from the size of `fishes` to the 
     * total population size
     */
    unsigned int number;

    uint start_number;
    uint burn_times;

    Count alive;
    Mean length_mean;

    Array<uint,Stocks,Areas,Sexes,Ages,Lengths> counts;

    Fishes& initialise(void){
        Fish::params.initialise();

        return *this;
    }

    void start(const Environ& environ) {
        // Seed the population
        fishes.clear();
        fishes.resize(start_number);
        // Burn in
        //! @todo
    }

    /**
     * Update the population
     * 
     * @param environ Current state of the environment
     */
    void update(const Environ& environ) {
        // Update each fish
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

        // Spawn new fishes
        auto replace = fishes.begin();
        auto end = fishes.end();
        int eggs = 10000;
        for(int index=0; index<eggs; index++){
            Fish father;
            Fish mother;
            Fish egg(father,mother);
            // Try to find a dead fish in `fishes` which can be replaced by
            // the new egg
            while(replace!=end and replace->alive()) replace++;
            // If possible replace a dead fish, otherwise add to fishes
            if(replace!=end) *replace = egg;
            else fishes.push_back(egg);
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
    static void update_task(std::vector<Fish>* fishes, const Environ& environ, const uint& start, const uint& num) {
        auto fish = fishes->begin()+start;
        auto end = fish+num;
        while (fish != end) {
            fish->update(environ);
            fish++;
        }
    }

    /**
     * Enumerate the population (count number of fish etc)
     */
    void enumerate(void) {
        alive.reset();
        counts = 0;
        for (auto fish : fishes){
            if(fish.alive()){
                alive.append();

                counts(
                    fish.stock,
                    fish.area,
                    fish.sex,
                    fish.age_bin(),
                    fish.length_bin()
                )++;

                length_mean.append(fish.length);
            }
        }
    }

    /**
     * Track the population by writing attributes and structure to files
     */
    void track(void){ 
        static std::ofstream* general = nullptr;
        if(not general) general = new std::ofstream("output/fishes/general.tsv"); 

        static std::ofstream* counts_file = nullptr;
        if(not counts_file) counts_file = new std::ofstream("output/fishes/counts.tsv");

        enumerate();

        (*general)
                <<now<<"\t"
                <<fishes.size()<<"\t"
                <<alive<<"\t"
                <<length_mean<<std::endl;

        for(auto stock : stocks){
            for(auto area : areas){
                for(auto sex : sexes){
                    for(auto age: ages){
                        for(auto length : lengths){
                            (*counts_file)
                                <<now<<"\t"
                                <<stock<<"\t"
                                <<area<<"\t"
                                <<sex<<"\t"
                                <<age<<"\t"
                                <<length<<"\t"
                                <<counts(stock,area,sex,age,length)<<"\n"
                            ;
                        }
                    }
                }
            }
        }
        (*counts_file).flush();
    }

};  // end class Fishes
