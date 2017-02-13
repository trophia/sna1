#pragma once

#include "requirements.hpp"

/**
 * Fishing activities
 */
class Harvest {
 public:

    /**
     * Catc history
     */
    Array<double, Years, Regions, Methods> catch_history;

    /**
     * Selectivity by method for each length bin
     */
    Array<double, Methods, Lengths> selectivity_at_length;

    /**
     * Current vulnerable biomass by method
     */
    Array<double, Regions, Methods> biomass_vulnerable;

    Array<double, Regions, Methods> catch_observed;

    Array<double, Regions, Methods> catch_taken;

    uint attempts;



    void initialise(void){
        catch_history = 0;
        catch_history.read("input/harvest/catch-history.tsv");

        for (auto method : methods) {
            for (auto length_bin : lengths) {
                auto steep1 = parameters.harvest_sel_steep1(method);
                auto mode = parameters.harvest_sel_mode(method);
                auto steep2 = parameters.harvest_sel_steep2(method);
                auto length = length_mid(length_bin);
                double selectivity;
                if(length<=mode) selectivity = std::pow(2,-std::pow((length-mode)/steep1,2));
                else selectivity = std::pow(2,-std::pow((length-mode)/steep2,2));
                selectivity_at_length(method,length_bin) = selectivity;
            }
        }
    }

    void catch_observed_update(void) {
        auto y = year(now);
        if (y >= Years_min and y <= Years_max) {
            for (auto region : regions) {
                for(auto method : methods) {
                    auto catches = catch_history(y,region,method);
                    catch_observed(region,method) = catches;
                }
            }
        }
    }

    void finalise(void) {
        boost::filesystem::create_directories("output/harvest");

        catch_history.write("output/harvest/catch-history.tsv");
        selectivity_at_length.write("output/harvest/selectivity_at_length.tsv");
    }

};  // class Harvest
