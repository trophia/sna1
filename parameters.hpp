#pragma once

#include "requirements.hpp"
#include "random.hpp"
#include "dimensions.hpp"

/**
 * Parameters
 *
 * Provides for the input, mapping and output of 
 * parameters of the model
 */
class Parameters : public Structure<Parameters> {
 public:

    /**
     * Number of instances of `Fish` to seed the population with
     *
     * Preliminary sensitity analyses (see `instances_seed_sensitivity` in `sna1.cpp`)
     * suggested 100,000 was a good trade-off between run duration and precision at least
     * during development. Should be increased for final runs.
     */
    unsigned int fishes_seed_number = 10000;

    /**
     * Total mortality of the initial seed population
     *
     * Determines the equilibrium age structure of the seed population.
     */
    double fishes_seed_z = 0.1;

    /**
     * Exponential distribution for ages of the seed population
     */
    Exponential fishes_seed_age_dist;

    /**
     * Stock recruitment
     */
    double fishes_steepness = 0.85;

    /**
     * Sex ratio
     */
    double fishes_males = 0.5;

    /**
     * Instantaneous rate of natural mortality
     */
    double fishes_m = 0.075;

    /**
     * Probabiliy of fish dying of natural mortality
     * in a time step. Derived from `fishes_m` in `initialise()`
     */
    double fishes_m_rate;

    /**
     * Length-weight relation
     */
    double fishes_a = 4.467e-08;
    double fishes_b = 2.793;

    /**
     * Distribution of growth coefficients (von Bertalanffy k)
     * across population of fish
     */
    double fishes_k_mean = 0.1;
    double fishes_k_cv = 0.1;

    /**
     * Lognormal distribution for fishes k
     */
    Lognormal fishes_k_dist;
    /**
     * Distribution of assymptotic length (von Bertalanffy Linf)
     * across population of fish
     */
    double fishes_linf_mean = 60;
    double fishes_linf_cv = 0.1;

    /**
     * Lognormal distribution for fishes Linf
     */
    Lognormal fishes_linf_dist;

    /**
     * Maturity-at-age
     *
     * This is not the proportion mature at an age but rather the probability
     * of maturing
     */
    double fishes_mature = 4;

    /**
     * Movement matrix
     */
    Array<double, Areas, AreaTos> movement;

    /**
     * Pristine spawner biomass (t)
     *
     * Externally defined and used to calculate the value for `scalar`
     *
     * Total across stocks from Francis & McKenzie (2014) "Table 30: Base case 
     * estimates of unfished biomass, B0 , and current biomass by stock and area"
     */
    double fishes_b0 = 376000;

    /**
     * Initialise parameters
     */
    void initialise(void) {
        // Ensure output directory is present
        boost::filesystem::create_directories("output");

        // Read from file
        read("input/parameters.cila");

        fishes_seed_age_dist = Exponential(fishes_seed_z);

        fishes_m_rate = 1 - std::exp(-fishes_m);

        fishes_k_dist = Lognormal(fishes_k_mean,fishes_k_cv);
        fishes_linf_dist = Lognormal(fishes_linf_mean,fishes_linf_cv);

        movement = 0;

        // Write to file (for checking)
        write("output/parameters.json");
    }

    void finalise(void) {
    }

    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(fishes_k_mean,"fishes_k_mean")
            .data(fishes_linf_mean,"fishes_linf_mean")
        ;
    }
};  // class Parameters

Parameters parameters;
