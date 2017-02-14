#pragma once

#include "requirements.hpp"
#include "dimensions.hpp"
#include "parameters.hpp"
#include "environ.hpp"

/**
 * A fish
 */
class Fish {
 public:
    /**
     * Home region for this fish
     */
    Region home;

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
    float growth_intercept;

    /**
     * Assymptotic length (von Bertalanffy Linf) for this fish
     */
    float growth_slope;

    /**
     * Current length (cm) of this fish
     */
    float length;

    /**
     * Is this fish mature?
     */
    bool mature;

    /**
     * Current region of this fish
     */
    Region region;

    /**
     * Tag number for fish
     */
    unsigned int tag;


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
        return year(now)-year(birth);
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
     * Calculate the length at age for this fish based on its growth parameters
     */
    double length_at_age(uint age) const {
        auto pars = vonb();
        return length_at_age(age, pars[0], pars[1]);
    }

    /**
     * Calculate the length at age of a fish given vonB growth parameters
     */
    static double length_at_age(uint age, const double& k, const double& linf) {
        return linf*(1-std::exp(-k*age));
    }

    /**
     * Set the von Bert growth parameters for this fish
     *
     * Converts `k` and `linf` to `growth_intercept` and `growth_slope`
     */
    void vonb(const double& k, const double& linf, const double& time = 1) {
        growth_slope = std::exp(-k*time)-1;
        growth_intercept = -growth_slope * linf;
    }

    /**
     * Get the von Bert growth parameters for this fish
     *
     * Converts `growth_intercept` and `growth_slope` to `k` and `linf`
     */
    std::array<double, 2> vonb(void) const {
        // TODO
        auto k = 0.0;
        auto linf = 0.0;
        return {k, linf};
    }

    /**
     * Get the weight of this fish
     *
     * Currently, all fish have the same condition factor so weight is
     * simply a function of length
     */
    double weight(void) const {
        return parameters.fishes_a*std::pow(length, parameters.fishes_b);
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
        home = Region(int(parameters.fishes_seed_region_dist.random()));
        region = home;

        auto age = std::max(1.,std::min(parameters.fishes_seed_age_dist.random(),100.));
        birth = now-age;
        death = 0;

        sex = (chance()<parameters.fishes_males)?male:female;

        // Set von Bert growth parameters from their distributions
        double k;
        double linf;
        if (parameters.fishes_growth_type == 't') {
            k = parameters.fishes_k_mean;
            linf = parameters.fishes_linf_mean;
        } else {
            k = parameters.fishes_k_dist.random();
            linf = parameters.fishes_linf_dist.random();
        }
        vonb(k, linf);
        length = length_at_age(age, k, linf);

        // The is an approximation
        mature = chance()<parameters.fishes_maturation(age);

        tag = 0;
    }

    /**
     * Birth this fish
     *
     * Initialises attributes as though this fish is close
     * to age 0
     */
    void born(Region region_) {
        home = region_;
        region = home;

        birth = now;
        death = 0;
        
        sex = (chance()<parameters.fishes_males)?male:female;

        // Set von Bert growth parameters from their distributions
        double k;
        double linf;
        if (parameters.fishes_growth_type == 't') {
            k = parameters.fishes_k_mean;
            linf = parameters.fishes_linf_mean;
        } else {
            k = parameters.fishes_k_dist.random();
            linf = parameters.fishes_linf_dist.random();
        }
        vonb(k, linf);
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
    void dies(void) {
        death = now;
    }

    /**
     * Does this fish survive this time step?
     */
    bool survival(void) {
        auto survives = chance() > parameters.fishes_m_rate;
        if (not survives) dies();
        return survives;
    }

    /**
     * Increase the length of this fish
     */
    void growth(void) {
        // Calculate growth increment
        double incr = growth_intercept + growth_slope * length;
        // Apply temporal variation in growth if needed
        if (parameters.fishes_growth_type == 't' or parameters.fishes_growth_type == 'm') {
            int sd = std::max(parameters.fishes_growth_temporal_sdmin, incr * parameters.fishes_growth_temporal_cv);
            incr += standard_normal_rand() * sd;
            if (incr < parameters.fishes_growth_temporal_incrmin) incr = parameters.fishes_growth_temporal_incrmin;
        }
        // Add increment but ensure fish size does not go below zero
        length += incr;
        if (length < 0) length = 0;
    }

    /**
     * Change the maturation status of this fish
     */
    void maturation(void) {
        if (not mature) {
            if (chance()<parameters.fishes_maturation(age_bin())) {
                mature = true;
            }
        }
    }

    /**
     * Move this fish
     */
    void movement(void) {
        // If no movement, don't do anything
        if (parameters.fishes_movement_type == 'n') return;
        // Instantaneous movement between regions is eith Markovian (based on where fish is)
        // or home fidelity (based on fish's home)
        Region basis = region;
        switch (parameters.fishes_movement_type) {
            case 'm':
                basis = region;
            break;
            case 'h':
                basis = home;
            break; 
        };
        // Randomly move a region (note that rows of the movement matrix sum to 1)
        auto region_to = Region(regions.select(chance()).index());
        if (chance() < parameters.fishes_movement(basis, region_to)) {
            region = region_to;
        }
    }

};  // end class Fish


/**
 * The population of `Fish`
 * 
 * We don't attempt to model every single fish in the population. Instead,
 * the vector of `Fish` objects is intended to be a representative sample of the overall population.
 * The variable, `scalar` is then used to scale other variables, like biomass, to population levels.
 */
class Fishes : public std::vector<Fish> {
 public:

    Fishes(int size = 0):
        std::vector<Fish>(size){}

    /**
     * Population scalar
     *
     * Used to scale the things like biomass etc from the size of `fishes` to the 
     * total population size
     */
    double scalar = 1.0;

    /**
     * Seed the population with individuals that have attribute values 
     * whose distributions approximate that of a pristine population
     * 
     * This method is usually used in `Model::pristine` to reduce burn-in times
     * but is a separate method so that it can also be used in unit tests. 
     */
    void seed(unsigned int number) {
        clear();
        resize(number);
        for (auto& fish : *this) {
            fish.seed();
        }
    }

    /**
     * Aggregate properties that get calculated at various times
     */
    
    /**
     * Current total biomass (t)
     */
    double biomass;

    void biomass_update(void) {
        biomass = 0.0;
        for (auto& fish : *this) {
            if (fish.alive()) {
                biomass += fish.weight();
            }
        }
        biomass *= scalar;
    }

    /**
     * Current spawner biomass (t)
     */
    Array<double, Regions> biomass_spawners;

    void biomass_spawners_update(void) {
        biomass_spawners = 0.0;
        for (auto& fish : *this) {
            if (fish.alive() and fish.mature) {
                biomass_spawners(fish.region) += fish.weight();
            }
        }
        biomass_spawners *= scalar;
    }


    char recruitment_mode = 'n';

    /**
     * Recruitment for pristine population (see `Model::pristine()`)
     */
    Array<double, Regions> recruitment_pristine;

    /**
     * Current recruitment (no.)
     */
    Array<double, Regions> recruitment;

    /**
     * Current recruitment (instances)
     */
    Array<uint, Regions> recruitment_instances;


    void recruitment_update(void) {
        for(auto region : regions) {
            if (recruitment_mode == 'p') {
                recruitment(region) = recruitment_pristine(region);
            } else {
                auto s = biomass_spawners(region);
                auto r0 = recruitment_pristine(region);
                auto s0 = parameters.fishes_b0(region);
                auto h = parameters.fishes_steepness;
                recruitment(region) = 4*h*r0*s/((5*h-1)*s+s0*(1-h));
            }
            recruitment_instances(region) = std::round(recruitment(region)/scalar);
        }
    }


    /**
     * Counts of fish by model dimensions
     */
    Array<uint,Regions,Sexes,Ages,Lengths> counts;


    /**
     * Initialise parameters etc
     */
    void initialise(void){
        boost::filesystem::create_directories("output/fishes");
    }

    /**
     * Finalise (e.g. write values to file)
     */
    void finalise(void){
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
                    fish.region,
                    fish.sex,
                    fish.age_bin(),
                    fish.length_bin()
                )++;
            }
        }
    }

    /**
     * Track the population by writing attributes and structure to files
     */
    void track(void){ 
        static std::ofstream* counts_file = nullptr;
        if(not counts_file) counts_file = new std::ofstream("output/fishes/counts.tsv");

        enumerate();

        for(auto region : regions){
            for(auto sex : sexes){
                for(auto age: ages){
                    for(auto length : lengths){
                        (*counts_file)
                            <<now<<"\t"
                            <<region<<"\t"
                            <<sex<<"\t"
                            <<age<<"\t"
                            <<length<<"\t"
                            <<counts(region,sex,age,length)<<"\n"
                        ;
                    }
                }
            }
        }
        (*counts_file).flush();
    }

};  // end class Fishes
