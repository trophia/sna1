#pragma once

#include <map>

#include <boost/filesystem.hpp>

/**
 * Simulation of a tagging programme
 *
 * Currently, a very simple implementation which just tags fish
 * uniformly with a given probability and recovers them with a given 
 * probability.
 *
 * Creates a "database" of tag release and recapture pairs which can be
 * analysed by various methods.
 */
class Tagging {
 public:
    void initialise(void) {
    }

    /**
     * A tagging event
     *
     * Copies the fish and adds a time stamp
     */
    class Event : public Fish {
     public:
        Time time;

        /**
         * Constructor used for both release and
         * recapture events
         */
        Event(Time time, const Fish& fish):
            Fish(fish),
            time(time){
        }

        /**
         * Default constructer used for the recapture
         * event when an entry is made in database. 
         * Signified as empty by `time==0`
         */
        Event(void):
            time(0){
        }
    };

    /**
     * A mark and release of a fish.
     */
    void mark(Fish& fish) {
        // Increment the tag number
        number_++;
        // Apply the tag to the fish
        fish.tag = number_;
        // Record the fish in the database
        tags_[number_].first = Event(now,fish);
    }

    /**
     * A recovery of a tagged fish.
     *
     * Note that this method does not actually kill the 
     * fish (done elsewhere) it just records it
     */
    void recover(const Fish& fish) {
        // Record the fish in the database
        tags_[fish.tag].second = Event(now,fish);
    }

    /**
     * Update of the tagging programme
     *
     * Currently very simplistic and not integrated with fleet
     * dynamics for release or recpature
     */
    void update(Fleet& fleet, Fishes& fishes, const Environ& environ) {
        auto y = year(now);
        // Marking fish
        if(y==1994 or y==2016) {
            for (Fish& fish : fishes.fishes) {
                if (fish.alive() and fish.length>30 and chance.random()< 0.1) {
                    mark(fish);
                }
            }
        }
        // Recovering fish
        if(y==1994 or y==1995 or y==2016 or y==2017) {
            for (Fish& fish : fishes.fishes) {
                if (fish.alive() and fish.tag and chance.random()< 0.01) {
                    recover(fish);
                }
            }
        }
    }

    void write(void) {
        boost::filesystem::create_directories("output/monitor/tagging");
        std::ofstream file("output/monitor/tagging/tags.tsv");
        for(auto iter : tags_){
            auto number = iter.first;
            auto release = iter.second.first;
            auto recapture = iter.second.second;
            if(recapture.time){
                file<<number<<"\t"
                    <<release.time<<"\t"<<recapture.time<<"\t"
                    <<release.area<<"\t"<<recapture.area<<"\t"
                    <<release.length<<"\t"<<recapture.length<<"\n";
            }
        }
    }

    void finalise(void) {
        write();
    }

 private:
    /**
     * The current tag number
     *
     * Is incremented in the `release()` method and applied
     * to each fish.
     */
    unsigned int number_ = 0;

    /**
     * A database of tagged fish
     *
     * A fish instance is stored at the time of release and recovery
     */
    std::map<int,std::pair<Event,Event>> tags_;

};  // class Tagging
