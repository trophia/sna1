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
    unsigned int fishes_seed_number = 1e6;

    Uniform fishes_seed_region_dist;

    /**
     * Total mortality of the initial seed population
     *
     * Determines the equilibrium age structure of the seed population.
     */
    double fishes_seed_z = 0.075;

    /**
     * Exponential distribution for ages of the seed population
     */
    Exponential fishes_seed_age_dist;

    /**
     * Pristine spawner biomass (t)
     */
    Array<double, Regions> fishes_b0 = {
        100000,
        200000,
        100000
    };

    /**
     * Stock recruitment
     */
    double fishes_steepness = 0.85;

    /**
     * Recruitment variability
     */
    double fishes_rec_var = 0.6;
    Array<double, Years> fishes_rec_strengths = 1;

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
     * Growth model
     *
     * l = linear
     * e = exponential
     */
    char fishes_growth_model = 'l';

    /**
     * Distribution of growth coefficients (von Bertalanffy k)
     * across population of fish
     */
    double fishes_k_mean = 0.1;
    double fishes_k_sd = 0.02;
    Lognormal fishes_k_dist;

    /**
     * Distribution of assymptotic length (von Bertalanffy Linf)
     * across population of fish
     */
    double fishes_linf_mean = 60;
    double fishes_linf_sd = 10;
    Lognormal fishes_linf_dist;

    /**
     * Growth variation type
     *
     * t = only temporal variation in growth
     * i = only individual variation in growth
     * m = mixed, both individual and temporal variation in growth
     */
    char fishes_growth_variation = 'm';

    /**
     * Coefficient of variation of temporal variation in growth
     */
    double fishes_growth_temporal_cv = 0.3;
    double fishes_growth_temporal_sdmin = 1;
    double fishes_growth_temporal_incrmin = 0;

    /**
     * Maturiation-at-age
     *
     * This is NOT the proportion mature at an age but rather the probability
     * of maturing at a particular age
     */
    Array<double, Ages> fishes_maturation;

    /**
     * Movement type
     */
    char fishes_movement_type = 'm';    

    /**
     * Movement matrix
     */
    Array<double, Regions, RegionTos> fishes_movement;

    /**
     * Minimum legal size limit
     */
    Array<double, Methods> harvest_mls = {
        25, 25, 25, 25
    };

    /**
     * Mortality of fish that are returned to sea
     */
    double harvest_handling_mortality = 0;

    /**
     * Parameters of double normal length based selectivity
     */
    Array<double, Methods> harvest_sel_steep1;
    Array<double, Methods> harvest_sel_mode;
    Array<double, Methods> harvest_sel_steep2;

    /**
     * Because `parameters` is a global variable that always need initialisation
     * we ensure initialisation on construction and finalisation on destruction.
     */
    Parameters() {
        initialise();
    }
    ~Parameters() {
        finalise();
    }

    /**
     * Initialise parameters
     */
    void initialise(void) {
        // Defaults not defined above
        fishes_movement = 0;

        for (auto age : ages) {
            double p = 0;
            if (age <= 4) p = 0;
            else if (age == 5) p = 0.5;
            else p = 1;
            fishes_maturation(age) = p;
        }

        harvest_sel_steep1(LL) = 2.76;
        harvest_sel_mode(LL) = 30.47;
        harvest_sel_steep2(LL) = 1000;
        
        harvest_sel_steep1(BT) = 2.35;
        harvest_sel_mode(BT) = 29.39;
        harvest_sel_steep2(BT) = 29.15;

        harvest_sel_steep1(DS) = 3.13;
        harvest_sel_mode(DS) = 31.63;
        harvest_sel_steep2(DS) = 20.54;

        harvest_sel_steep1(RE) = 1.97;
        harvest_sel_mode(RE) = 30.11;
        harvest_sel_steep2(RE) = 15.27;

        // Parameter values can be overidden by setting them in the following files:
        
        read("input/parameters.json");
        fishes_rec_strengths.read("input/fishes_rec_strengths.tsv");

        // Derived values
        
        fishes_seed_region_dist = Uniform(0,3);
        fishes_seed_age_dist = Exponential(fishes_seed_z);

        fishes_m_rate = 1 - std::exp(-fishes_m);

        fishes_k_dist = Lognormal(fishes_k_mean, fishes_k_sd);
        fishes_linf_dist = Lognormal(fishes_linf_mean, fishes_linf_sd);
    }

    void finalise(void) {
        boost::filesystem::create_directories("output");

        write("output/parameters.json");
        fishes_rec_strengths.write("output/fishes_rec_strengths.tsv");
    }

    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(fishes_seed_number, "fishes_seed_number")
            .data(fishes_seed_z, "fishes_seed_z")
            
            .data(fishes_steepness, "fishes_steepness")
            .data(fishes_rec_var, "fishes_rec_var")

            .data(fishes_males, "fishes_males")
            
            .data(fishes_m, "fishes_m")
            
            .data(fishes_a, "fishes_a")
            .data(fishes_b, "fishes_b")
            
            .data(fishes_growth_model , "fishes_growth_model")
            .data(fishes_k_mean , "fishes_k_mean")
            .data(fishes_k_sd, "fishes_k_sd")
            .data(fishes_linf_mean, "fishes_linf_mean")
            .data(fishes_linf_sd, "fishes_linf_sd")
            .data(fishes_growth_variation , "fishes_growth_variation")
            .data(fishes_growth_temporal_cv , "fishes_growth_temporal_cv")
            .data(fishes_growth_temporal_sdmin, "fishes_growth_temporal_sdmin")
            .data(fishes_growth_temporal_incrmin, "fishes_growth_temporal_incrmin")

            .data(fishes_movement_type, "fishes_movement_type")
            
            .data(harvest_handling_mortality, "harvest_handling_mortality")
        ;
    }

};  // class Parameters

Parameters parameters;
