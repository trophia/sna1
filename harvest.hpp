#pragma once

#include "requirements.hpp"

/**
 * A group of fishing activity
 */
class Harvest {
 public:
	
    /**
     * Selectivity as a function of length
     */
    DoubleNormalPlateau selectivity;

    /**
     * Precalculated selectivity for each length bin
     */
    Array<double,Lengths> selectivity_at_length;

    /**
     * Minimum legal size limit
     */
    double mls;

    /**
     * Mortality of fish that are returned to sea
     */
    double handling_mortality;


    Harvest(void){
        selectivity.inflection_1 = 27;
        selectivity.inflection_2_delta = 100;
        selectivity.steepness_1 = 5;
        selectivity.steepness_2 = 100;

        mls = 30;

        handling_mortality = 0.1;
    }

    void initialise(void){
        boost::filesystem::create_directories("output/harvest");

        for (auto length_bin : lengths) {
            auto length = length_bin.index() + 0.5;
            selectivity_at_length(length_bin) = selectivity(length);
        }
    }

    void finalise(void) {
        selectivity_at_length.write("output/harvest/selectivity_at_length.tsv");
    }
};  // class Harvest
