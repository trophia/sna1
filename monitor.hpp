#pragma once

#include "monitor-tagging.hpp"

class Monitor {
 public:
    Tagging tagging;

    void initialise(void) {
        tagging.initialise();
    }

    void update(Fleet& fleet, Fishes& fishes, const Environ& environ) {
        tagging.update(fleet, fishes, environ);
    }

    void finalise(void) {
        tagging.finalise();
    }
    
};  // class Monitor
