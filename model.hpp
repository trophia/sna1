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
        fishes.initialise();
        return *this;
    }

    Model& start(void){
        environ.start();
        fishes.start(environ);
        fleet.start(fishes,environ);
        return *this;
    }

    Model& step(void){
        fishes.track();

        environ.update();
        fishes.update(environ);
        fleet.update(fishes,environ);
        now++;

        return *this;
    }

    Model& stop(void){
        return *this;
    }

    Model& run(unsigned int from, unsigned int to){
        now = from;
        start();
        while(now<=to){
            std::cout<<now<<std::endl;
            step();
        }
        stop();
        return *this;
    }
}; // end class Model
