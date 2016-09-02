#pragma once

#include "requirements.hpp"

/**
 * Simulation of a tagging programme
 *
 * Creates a "database" of tag release and recapture pairs which can be
 * analysed by various methods.
 */
class Tagging {
 public:


    /**
     * The number of releases by year, region and method
     */
    Array<int, Years, Regions, Methods> releases;

    /**
     * A boolean flag for years in which releases are made
     */
    Array<bool, Years> release_years;

    /**
     * A boolean flag for years in which recoveries are recorded
     */
    Array<bool, Years> recovery_years;

    /**
     * The minimum size of release
     */
    double min_release_length = 25;

    /**
     * The current tag number
     *
     * Is incremented in the `release()` method and applied
     * to each fish.
     */
    unsigned int number = 0;


    /**
     * A tagging event
     *
     * Copies the fish and adds a time stamp
     */
    class Event : public Fish {
     public:
        Time time;
        Method method;

        /**
         * Constructor used for both release and
         * recapture events
         */
        Event(const Fish& fish, Time time, Method method):
            Fish(fish),
            time(time),
            method(method){
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
     * A database of tagged fish
     *
     * A fish instance is stored at the time of release and recovery
     */
    std::map<int, std::pair<Event, Event> > tags;


    void initialise(void) {
        releases = 0;
        release_years = false;
        recovery_years = false;

        boost::filesystem::create_directories("output/monitor/tagging");
    }

    void finalise(void) {
        write();
    }

    /**
     * A mark and release of a fish.
     */
    void release(Fish& fish, Method method) {
        // Increment the tag number
        number++;
        // Apply the tag to the fish
        fish.tag = number;
        // Record the fish in the database
        tags[number].first = Event(fish, now, method);
    }

    /**
     * A recovery of a tagged fish.
     *
     * Note that this method does not actually kill the 
     * fish (done elsewhere) it just records it
     */
    void recover(const Fish& fish, Method method) {
        // Record the fish in the database
        tags[fish.tag].second = Event(fish, now, method);
    }

    void read(void) {
    }

    void write(void) {
        std::ofstream file("output/monitor/tagging/tags.tsv");
        for(const auto& iter : tags){
            auto number = iter.first;
            auto release = iter.second.first;
            auto recapture = iter.second.second;
            if(recapture.time){
                file<< number << "\t"
                    << release.time << "\t" << recapture.time << "\t"
                    << region_code(release.region) << "\t" << region_code(recapture.region) << "\t"
                    << method_code(release.method) << "\t" << method_code(recapture.method) << "\t"
                    << release.length << "\t" << recapture.length << "\n";
            }
        }
    }

};  // class Tagging
