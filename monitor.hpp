#pragma once

#include "monitor-tagging.hpp"

class Monitor {
 public:
    Tagging tagging;

    /**
     * Current CPUE by region and method
     *
     * Equivalent to vulnerable biomas with observation error
     * and opotentially other things (e.g hyperdepletion) added to it
     */
    Array<double, Regions, Methods> cpue;

    /**
     * Sample of aged fish by region, method and age bin
     */
    Array<double, Regions, Methods, Ages> age_sample;

    /**
     * Lengths of fish in population by region
     */
    Array<double, Regions, Lengths> length_pop;

    /**
     * Sample of measured fish by region, method and length bin
     */
    Array<double, Regions, Methods, Lengths> length_sample;


    void initialise(void) {
    	boost::filesystem::create_directories("output/monitor");

        tagging.initialise();
    }

    void finalise(void) {
        tagging.finalise();
    }

};  // class Monitor
