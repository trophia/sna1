#pragma once

#include "requirements.hpp"

/**
 * Lognormal
 *
 * Structure for a lognormal distribution used
 * for 
 */
class Lognormal : public Structure<Lognormal> {
 public:
    /**
     * Mean of distribution
     */
    double mean;

    /**
     * Coefficient of variation of distribution
     */
    double cv;

    /**
     * Initialisation
     */
    void init(void) {
        auto var = std::pow(mean*cv, 2);
        auto mean2 = std::pow(mean, 2);
        mu_ = std::log(mean2/std::sqrt(var+mean2));
        sd_ = std::sqrt(std::log(var/mean2+1));
    }

    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(mean,"mean")
            .data(cv,"cv")
        ;
    }

 private:

    /**
     * Parameters in log space for simulating random numbers
     */
    double mu_;
    double sd_;
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
     * Distribution of growth coefficients (von Bertalanffy k)
     * across population of fish
     */
    Lognormal fishes_k;

    /**
     * Distribution of assymptotic length (von Bertalanffy Linf)
     * across population of fish
     */
    Lognormal fishes_linf;


    void initialise(void) {
        // Ensure output directory is present
        boost::filesystem::create_directories("output");
        // Read from file
        read("input/parameters.cila");
        // Write to file (for checking)
        write("output/parameters.json");
    }

    void finalise(void) {
    }

    template<class Mirror>
    void reflect(Mirror& mirror){
        mirror
            .data(fishes_k,"fishes_k")
            .data(fishes_linf,"fishes_linf")
        ;
    }
};  // class Parameters
