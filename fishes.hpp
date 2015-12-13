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
     * Stock distribution of initial seed population e.g. 60% west, 40% east
     */
    Discrete<Stock,2> seed_stock;

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
     * Stock recruitment
     */
    double recruitment_steepness = 0.85;

    /**
     * Sex at birth
     */
    Discrete<Sex,2> sex_at_birth;

    /**
     * Instantaneous rate of natural mortality
     */
    double natural_mortality_rate = 0.075;

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
     * Length-weight relation
     */
    double length_weight_a = 4.467e-08;
    double length_weight_b = 2.793;

    /**
     * Maturity-at-age
     *
     * This is not the proportion mature at an age but rather the probability
     * of maturing
     */
    Array<double,Ages> maturation_at_age;

    /**
     * Initialise the parameters
     *
     * In the future, these could be read in from file, but for the moment done here
     */
    void initialise(void){

        seed_stock = {
            {W,E},
            {0.6,0.4}
        };

        seed_area(W) = {
            {EN,HG,BP},
            {0.5,0.4,0.1}
        };
        seed_area(E) = {
            {EN,HG,BP},
            {0.1,0.4,0.5}
        };

        seed_age = Exponential(
            seed_total_mortality
        );

        sex_at_birth = {
            {male,female},
            {0.5,0.5}
        };

        // Initialise the length-at-age distributions
        for(auto age : ages){
            auto mean = length_at_age_func.value(age.index());
            auto sd = std::max(mean*length_at_age_cv,1.);
            length_at_age(age) = Normal(mean,sd);
        }
        length_at_age.write("output/fishes/length_at_age.tsv",{"mean","sd"},[](std::ostream& file,const Normal& distribution){
            file<<distribution.mean()<<"\t"<<distribution.sd();
        });

        for(auto age : ages){
            // TODO implement something more sophisticated
            maturation_at_age(age) = age.index()>4;
        }
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
     * Is this fish mature?
     */
    bool mature;

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
        mature = chance.random()<params.maturation_at_age(age);
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
        mature = false;
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

    /**
     * Get the weight of this fish
     *
     * Currently, all fish have the same condition factor so weight is
     * simply a function of length
     */
    double weight(void) const {
        return params.length_weight_a*std::pow(length, params.length_weight_b);
    }

    /*************************************************************
     * Processes
     ************************************************************/

    /**
     * Kill this fish
     *
     * This method is separate from `survive()` because it
     * is also used by `Fleets` to kill a fish from harvest or
     * incidental mortality
     */
    Fish& dieing(void) {
        death = now;
        return *this;
    }

    /**
     * Does this fish survive this time step?
     */
    bool survival(void) {
        auto survives = chance.random() > params.natural_mortality_rate;
        if(not survives) dieing();
        return survives;
    }

    /**
     * Increase the length of this fish
     */
    Fish& growth(void) {
        length = std::max(params.length_at_age(age_bin()).random(),1.0);
        return *this;
    }

    /**
     * Change the maturation status of this fish
     */
    Fish& maturation(void) {
        if (not mature) {
            mature = chance.random()<params.maturation_at_age(age_bin());
        }
        return *this;
    }

    /**
     * Spawn a new fish
     * 
     * @return  The newly spawned eggs
     */
    std::vector<Fish> spawning(void) {
        std::vector<Fish> eggs;
        if (mature) {
            if (chance.random() < 0.1) {
                Fish egg(*this, *this);
                eggs.push_back(egg);
            }
        }
        return eggs;
    }

    /**
     * Move this fish
     */
    Fish& movement(void) {
        //! @todo
        return *this;
    }

    /**
     * Update this fish (i.e. operate all processes on it)
     */
    std::vector<Fish> update(const Environ& environ){
        if (survival()) {
            growth();
            maturation();
            movement();
            return spawning();
        }
        else return {};
    }
};  // end class Fish

FishParameters Fish::params;

/**
 * The population of `Fish`
 */
class Fishes {
 public:

    /**
     * Population scalar
     *
     * Used to scale the things like biomass etc from the size of `fishes` to the 
     * total population size
     */
    double scalar = 1.0;

    /**
     * List of individual fish
     *
     * We don't attempt to model every single fish in the population. Rather,
     * `fishes` are intended to be representative of the larger population.
     */
    std::vector<Fish> fishes;


    Count alive;
    Mean length_mean;
    Array<uint,Stocks,Areas,Sexes,Ages,Lengths> counts;

    Fishes& initialise(void){
        Fish::params.initialise();

        return *this;
    }

    /**
     * Update the population
     * 
     * @param environ Current state of the environment
     */
    void update(const Environ& environ) {
        // Update each fish
        std::vector<Fish> eggs;
        #if !defined(FISHES_PARALLEL)
            for (Fish& fish : fishes) {
                auto spawn = fish.update(environ);
                eggs.insert(eggs.end(),spawn.begin(),spawn.end());
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
     * Take the population to equilibrium
     */
    void equilibrium(Time time, const Environ& environ, int seed_number = 1000) {
        // Set `now` to some arbitrary time
        now = 0;
        // Seed the population. `resize()` uses the default
        // constructor which creates seed individuals that have attribute values 
        // intended to reduce the burn in time for the initial population
        fishes.clear();
        fishes.resize(seed_number);
        // Burn in
        // TODO Currently just burns in for an arbitarty number of iterations
        // Should instead exit when stability in population characteristics
        while(now<100){
            update(environ);
            std::cout<<now<<"\t"<<count('s')<<"\t"<<count('a')<<std::endl;
            now++;
        }
        // Re-age the fish to current time
        // The fish have arbitrary `birth` times so we need to "re-age"
        // them so that the population is in equilbrium AND "current"
        auto diff = time-now;
        for(auto fish : fishes){
            fish.birth += diff;
        }
    }

    /**
     * Count the number of fish
     *
     * This is a convienience function for counting the number of fish
     * in the population. It is mainly intended for debugging and tracking.
     * For more sophisticated counting see `enumerate()`
     * 
     * @param what What to count
     */
    unsigned int count(char what='a') {
        auto count = 0u;
        if(what=='a'){
            for (auto fish : fishes){
                if(fish.alive()) count++;
            }
        }
        else if(what=='s'){
            count = fishes.size();
        }
        return count;
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

        /*
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
        */
    }

};  // end class Fishes
