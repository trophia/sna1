#pragma once

#include "requirements.hpp"
#include "random.hpp"
#include "dimensions.hpp"


/**
 * Monitoring components
 *
 * Provides a convienient yet computationally efficient way of specifying
 * annual monitoring programme
 */
class MonitoringComponents : public std::string {
 public:

    bool C, L, A;

    MonitoringComponents(const char* value = ""):
        std::string(value) {
        update();
    }

    void update() {
        C = find('C') != std::string::npos;
        L = find('L') != std::string::npos;
        A = find('A') != std::string::npos;
    }
};

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
    Array<double, Years, Regions> fishes_rec_strengths = 1;

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
    Array<double, Regions, RegionTos> fishes_movement = 0;

    /**
     * The degree of shyness of a fish to the last fishing method that it
     * was caught by (assuming it was released because undersized or tagged).
     *
     * 1 = complete shyness, will never get by the method again
     * 0 = no shyness
     */
    Array<double, Methods> fishes_shyness = 0;

    /**
     * Catch history
     */
    Array<double, Years, Regions, Methods> harvest_catch_history = 0;

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
     * Monitoring programme by year
     */
    Array<MonitoringComponents, Years> monitoring_programme = "";

    /**
     * The number of target tagging releases by year, region and method
     */
    Array<int, Years, Regions, Methods> tagging_releases = 0;

    /**
     * The proportion of catch scanned by year, region and method
     */
    Array<double, Years, Regions, Methods> tagging_scanning = 0;

    /**
     * Mortality of fish that have been tagged (note, this is independent of `harvest_handling_mortality`)
     */
    double tagging_mortality = 0;

    /**
     * Probability that a tag will be shed by the fish
     */
    double tagging_shedding = 0;

    /**
     * The probability that a tagged fish is detected when scanned
     */
    double tagging_detection = 1;

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
        
        #define IFE(FILE, WHAT) if(boost::filesystem::exists(FILE)) WHAT(FILE)

        IFE("input/parameters.json", read);
        IFE("input/fishes_b0.tsv", fishes_b0.read);
        IFE("input/fishes_rec_strengths.tsv", fishes_rec_strengths.read);
        IFE("input/fishes_movement.tsv", fishes_movement.read);
        IFE("input/fishes_shyness.tsv", fishes_shyness.read);
        IFE("input/harvest_mls.tsv", harvest_mls.read);
        IFE("input/harvest_catch_history.tsv", harvest_catch_history.read);
        IFE("input/monitoring_programme.tsv", monitoring_programme.read);
        IFE("input/tagging_releases.tsv", tagging_releases.read);
        IFE("input/tagging_scanning.tsv", tagging_scanning.read);

        #undef IFE

        // Derived values
        
        fishes_seed_region_dist = Uniform(0,3);
        fishes_seed_age_dist = Exponential(fishes_seed_z);

        fishes_m_rate = 1 - std::exp(-fishes_m);

        fishes_k_dist = Lognormal(fishes_k_mean, fishes_k_sd);
        fishes_linf_dist = Lognormal(fishes_linf_mean, fishes_linf_sd);

        for (auto& item : monitoring_programme) item.update();
    }

    void finalise(void) {
        boost::filesystem::create_directories("output");

        write("output/parameters.json");
        fishes_b0.write("output/fishes_b0.tsv");
        fishes_rec_strengths.write("output/fishes_rec_strengths.tsv");
        fishes_movement.write("output/fishes_movement.tsv");
        fishes_shyness.write("output/fishes_shyness.tsv");
        harvest_mls.write("output/harvest_mls.tsv");
        harvest_catch_history.write("output/harvest_catch_history.tsv");
        monitoring_programme.write("output/monitoring_programme.tsv");
        tagging_releases.write("output/tagging_releases.tsv");
        tagging_scanning.write("output/tagging_scanning.tsv");
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

            .data(tagging_mortality, "tagging_mortality")
            .data(tagging_shedding, "tagging_shedding")
            .data(tagging_detection, "tagging_detection")
        ;
    }

};  // class Parameters

Parameters parameters;
