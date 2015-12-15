#pragma once

#include <fsl/math/functions/double-normal-plateau.hpp>
using Fsl::Math::Functions::DoubleNormalPlateau;

/**
 * A group of fishing activity
 */
class Fleet {
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


    Fleet(void){
        selectivity.inflection_1 = 27;
        selectivity.inflection_2_delta = 100;
        selectivity.steepness_1 = 5;
        selectivity.steepness_2 = 100;

        mls = 30;

        handling_mortality = 0.1;
    }

    void initialise(void){
        for (auto length_bin : lengths) {
            auto length = length_bin.index() + 0.5;
            selectivity_at_length(length_bin) = selectivity(length);
        }
    }

    void exploit(Fish& fish) {
        if (now >= 1970) {
            if (chance.random() < 0.05) {  // encountered
                if (chance.random() < selectivity_at_length(fish.length_bin())) {  // caught
                    if (fish.length < mls) {  // returned
                        if (chance.random() < handling_mortality) {  // returned but dies
                            fish.dies();
                        }
                    } else {
                        fish.dies();
                    }
                }
            }
        }
    }

    void update(Fishes& fishes, const Environ& environ) {
    	/**
    	 * Calculate an exploitable biomass
    	 */
    	double biomass_vuln = 0;

        /**
         * Apply an exploitation rate
         */
        for (Fish& fish : fishes.fishes) {
            exploit(fish);
        }
    }

    void finalise(void) {
        boost::filesystem::create_directories("output/fleet");
        selectivity_at_length.write("output/fleet/selectivity_at_length.tsv");
    }
};  // class Fleet
