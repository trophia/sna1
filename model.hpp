#pragma once

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

    Model& initialise(void){
        environ.initialise();
        fishes.initialise();
        fleet.initialise();

        return *this;
    }

    Model& update(void){
        fishes.track();

        environ.update();
        fishes.update(environ);
        fleet.update(fishes,environ);

        return *this;
    }

    Model& run(Time start, Time finish){
        // Create initial population of fish
        fishes.equilibrium(start, environ, 10000);
        // Iterate over times
        now = start;
        while(now<=finish){
            std::cout<<now<<"\t"<<fishes.count('s')<<"\t"<<fishes.count('a')<<std::endl;
            update();
            now++;
        }

        return *this;
    }
}; // end class Model
