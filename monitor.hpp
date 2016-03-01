#pragma once

#include "monitor-tagging.hpp"

class Monitor {
 public:
    Tagging tagging;

    void initialise(void) {
    	boost::filesystem::create_directories("output/monitor");

        tagging.initialise();
    }

    void finalise(void) {
        tagging.finalise();
    }
    
};  // class Monitor
