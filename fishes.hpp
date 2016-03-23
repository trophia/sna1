#pragma once

#include "requirements.hpp"
#include "dimensions.hpp"
#include "environ.hpp"

/**
 * Fish parameters
 */
class FishParameters {
 public:
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
     * Movement matrix
     */
    Array<double,Areas,AreaTos> movement;

    /**
     * Initialise the parameters
     *
     * In the future, these could be read in from file, but for the moment done here
     */
    void initialise(void){

        seed_age = Exponential(
            seed_total_mortality
        );

        sex_at_birth = {
            {male,female},
            {0.5,0.5}
        };

        for(auto age : ages){
            // TODO implement something more sophisticated
            maturation_at_age(age) = age.index()>4;
        }

        movement = 0;
    }
};

/**
 * A fish
 */
class Fish {
 public:
    /**
     * Home area for this fish
     *
     * Initially the area this fish was born
     */
    Area home;

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
     * Growth coefficient (von Bertalanffy k) for this fish
     */
    float k;

    /**
     * Assymptotic length (von Bertalanffy Linf) for this fish
     */
    float linf;

    /**
     * Current length (cm) of this fish
     */
    float length;

    /**
     * Is this fish mature?
     */
    bool mature;

    /**
     * Current location of this fish
     */
    Area area;

    /**
     * Tag number for fish
     */
    unsigned int tag;

    /**
     * Parameters for `Fish` dynamics
     */
    static FishParameters params;


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
     * Create a seed fish 
     * 
     * Needed for intial seeding of the population prior to burning it in. In other circumstances
     * `birth()` should used be used. Currently, a number of approximations are used so that the
     * seed population is closer to an equilibrium population:
     *
     *  - exponential distribution of ages
     *  - seed fish are distributed evenly across areas
     *  - maturity is approximated by maturation schedule
     */
    void seed(void) {
        area = chance.random()*Areas::size();
        home = area;
        auto age = std::max(1.,std::min(params.seed_age.random(),30.));
        birth = now-age;
        death = 0;
        sex = params.sex_at_birth.random();
        length = 0;
        mature = chance.random()<params.maturation_at_age(age);
        tag = 0;
    }

    /**
     * Birth this fish
     *
     * Initialises attributes as though this fish is close
     * to age 0
     */
    void born(Area area) {
        home = area;
        area = area;
        birth = now;
        death = 0;
        sex = params.sex_at_birth.random();
        length = 0;
        mature = false;
        tag = 0;
    }

    /**
     * Kill this fish
     *
     * This method is separate from `survive()` because it
     * is also used by `Fleets` to kill a fish from harvest or
     * incidental mortality
     */
    Fish& dies(void) {
        death = now;
        return *this;
    }

    /**
     * Does this fish survive this time step?
     */
    bool survival(void) {
        auto survives = chance.random() > params.natural_mortality_rate;
        if (not survives) dies();
        return survives;
    }

    /**
     * Increase the length of this fish
     */
    Fish& growth(void) {
        length = 0; // TODO von B
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
     * Move this fish
     */
    Fish& movement(void) {
        for (auto area_to : area_tos){
            if (chance.random() < params.movement(area,area_to)) {
                area = Area(area_to.index());
                break;
            }
        }
        return *this;
    }

};  // end class Fish

FishParameters Fish::params;

/**
 * The population of `Fish`
 * 
 * We don't attempt to model every single fish in the population. Rather,
 * the population `Fish` objects is intended to be representative of the larger population.
 */
class Fishes : public std::vector<Fish> {
 public:

    /**
     * Population scalar
     *
     * Used to scale the things like biomass etc from the size of `fishes` to the 
     * total population size
     */
    double scalar = 1.0;

    /**
     * Number of instances of `Fish` to seed the population with
     *
     * Preliminary sensitity analyses (see `instances_seed_sensitivity` in `sna1.cpp`)
     * suggested 100,000 was a good trade-off between run duration and precision at least
     * during development. Should be increased for final runs.
     */
    unsigned int instances_seed = 10000;

    /**
     * Pristine spawner biomass (t)
     *
     * Externally defined and used to calculate the value for `scalar`
     *
     * Total across stocks from Francis & McKenzie (2014) "Table 30: Base case 
     * estimates of unfished biomass, B0 , and current biomass by stock and area"
     */
    double biomass_spawners_pristine = 376000;

    /**
     * 
     */
    double biomass_spawners;

    /**
     * Counts of fish by model dimensions
     */
    Array<uint,Areas,Sexes,Ages,Lengths> counts;

    /**
     * Initialise parameters etc
     */
    void initialise(void){
        Fish::params.initialise();

        boost::filesystem::create_directories("output/fishes");
    }

    /**
     * Finalise (e.g. write values to file)
     */
    void finalise(void){
    }

    /**
     * Reset aggregate statistics
     */
    void aggregates_reset(void) {
        biomass_spawners = 0;
    }

    /**
     * Add a fish to the aggregate statistics
     * 
     * @param fish Fish to add
     */
    void aggregates_add(const Fish& fish) {
        if (fish.alive()) {
            if (fish.mature) {
                biomass_spawners += fish.weight();
            }
        }
    }

    /**
     * Calculate aggregate stattistics
     */
    void aggregates(void) {
        aggregates_reset();
        for(auto fish : *this) aggregates_add(fish);
    }

    /**
     * Recruitment
     * 
     * @return  The new recruits
     */
    std::vector<Fish> recruitment(void) {
        // Calculate the number of recruits (eggs) to produce
        // and initialise the vector with that number
        // TODO stock-recruitment in here
        double number = 1000;
        std::vector<Fish> recruits(number);
        // Initialise each of the recruits
        for(auto recruit : recruits){
            // TODO determine recruits by area
            // instad of this temporary random assignment
            Area area = chance.random()*Areas::size();
            recruit.born(area);
        }

        // Iterator pointing to the current recruit to be used as a replacement
        auto replacement = begin();
        // As an optimisation, we previously had this replacement inside the next
        // fish loop. But, that 
        auto iterator = begin();
        while(iterator != end() and replacement != recruits.end()){
            // If fish has died and there are still recruits to be inserted, replace it
            if (not iterator->alive() and replacement != recruits.end()) {
                *iterator = *replacement;
                replacement++;
            }
        }
        // If there are still recruits to be inserted then append them
        while (replacement != recruits.end()) {
            push_back(*replacement);
            replacement++;
        }

        return recruits;
    }

    /**
     * Calculate the number of fish in the population
     *
     * @param scale Scale up the number?
     */
    double number(bool scale = true) {
        auto sum = 0.0;
        for (auto fish : *this){
            if (fish.alive()) {
                sum++;
            }
        }
        return sum * (scale?scalar:1);
    }

    /**
     * Calculate the biomass
     */
    double biomass(void) {
        auto sum = 0.0;
        for (auto fish : *this) {
            if (fish.alive()) {
                sum += fish.weight();
            }
        }
        return sum * scalar;
    }

    /**
     * Calculate the biomass of spawners by area
     */
    Array<double,Areas> biomass_spawners_area(void) {
        Array<double,Areas> sums = 0.0;
        for (auto fish : *this) {
            if (fish.alive() and fish.mature) {
                sums(fish.area) += fish.weight();
            }
        }
        sums *= scalar;
        return sums;
    }

    /**
     * Calculate the mean age of fish
     */
    double age_mean(void) {
        Mean mean;
        for (auto fish : *this) {
            if (fish.alive()) mean.append(fish.age());
        }
        return mean;
    }

    /**
     * Calculate the mean length of fish
     */
    double length_mean(void) {
        Mean mean;
        for (auto fish : *this) {
            if (fish.alive()) mean.append(fish.length);
        }
        return mean;
    }

    /**
     * Enumerate the population (count number of fish etc)
     */
    void enumerate(void) {
        counts = 0;
        for (auto fish : *this) {
            if(fish.alive()){
                counts(
                    fish.area,
                    fish.sex,
                    fish.age_bin(),
                    fish.length_bin()
                )++;
            }
        }
    }

    /**
     * Trace key attributes to output
     *
     * This is used for 
     */
    void trace(void) {
        #if TRACE_LEVEL >= 1
            std::cout
                << size() << "\t";
        #endif
        #if TRACE_LEVEL >= 2
            std::cout
                << number(false) << "\t"
                << number() << "\t"
                << biomass() << "\t"
                << biomass_spawners << "\t";
        #endif
        #if TRACE_LEVEL >=3
            auto bs = biomass_spawners_area();
            std::cout
                << bs(EN) << "\t" << bs(HG) << "\t" << bs(BP) << "\t"
                << age_mean() << "\t"
                << length_mean() << "\t";
        #endif
    }

    /**
     * Track the population by writing attributes and structure to files
     */
    void track(void){ 
        static std::ofstream* counts_file = nullptr;
        if(not counts_file) counts_file = new std::ofstream("output/fishes/counts.tsv");

        enumerate();

        for(auto area : areas){
            for(auto sex : sexes){
                for(auto age: ages){
                    for(auto length : lengths){
                        (*counts_file)
                            <<now<<"\t"
                            <<area<<"\t"
                            <<sex<<"\t"
                            <<age<<"\t"
                            <<length<<"\t"
                            <<counts(area,sex,age,length)<<"\n"
                        ;
                    }
                }
            }
        }
        (*counts_file).flush();
    }

};  // end class Fishes
