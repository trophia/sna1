#pragma once

#include "requirements.hpp"

/**
 * Represents stochasticity
 *
 * Used in numerous places for adding probabalistic 
 * behaviours to dynamics
 */
static Uniform chance = {0, 1};

/**
 * The "environment"
 *
 * Currently just a placeholder
 */
class Environ {
 public:
    void initialise(void) {
    	boost::filesystem::create_directories("output/environ");
    }

    void finalise(void) {
    }
};  // class Environ
