#pragma once

#include "requirements.hpp"

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
