#pragma once

#include "monitor-tagging.hpp"

class Monitor {
 public:
    Tagging tagging;

    void initialise(void) {
        tagging.initialise();
    }

    void finalise(void) {
        tagging.finalise();
    }
    
};  // class Monitor
