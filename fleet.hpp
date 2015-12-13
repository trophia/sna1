#pragma once

/**
 * A fleet of fishing activity
 */
class Fleet {
 public:
	/**
	 * Current catches (t)
	 */
	double catches;

    Fleet& initialise(void){
        return *this;
    }

    void encounter(Fish& fish) {
        if(now>=1980){
            auto encountered = chance.random() < 0.1;
            if(encountered) {
                fish.dieing();
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
            encounter(fish);
        }
    }

    Environ& finalise(void) {
        return *this;
    }
};  // class Fleet

/**
 * Fleets
 */