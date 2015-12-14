#pragma once

#include <iostream>

#include "environ.hpp"
#include "fishes.hpp"
#include "fleet.hpp"

/**
 * The model
 *
 * Links together the sub-models `Environ`, 'Fishes` and `Fleet`
 */
class Model {
public:

    Environ environ;
    Fishes fishes;
    Fleet fleet;

    void initialise(void) {
        environ.initialise();
        fishes.initialise();
        fleet.initialise();
    }

    void trace(void) {
        #if TRACE_LEVEL>0
            std::cout<<now<<"\t";
            fishes.trace();
            std::cout<<"\n";
        #endif
    }

    void update(void) {
        environ.update();
        fishes.update(environ);
        fleet.update(fishes, environ);
    }

    void run(Time start, Time finish, std::function<void()>* callback = 0) {
        // Create initial population of fish
        fishes.pristine(start, environ);
        // Iterate over times
        now = start;
        while (now <= finish) {
            trace();
            if (callback) (*callback)();
            update();
            now++;
        }
    }
};  // end class Model
