#pragma once
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>
#include "utility.h"


struct Stat{
     
    std::string name;
    size_t count = 0 ;
    double sum = 0 ;
    double min = 0 ;
    double max = 0 ;
    double mean = 0 ;
    bool decimal = true;
    Stat(std::string name, bool decimal=true): name(name), decimal(decimal) {}
    
    virtual std::string toString(int ind = 0);
    virtual void add_value(float value);
};

struct IntStat{
    bool decimal = false;
};



struct HandleStats{

    static HandleStats* instance;
    size_t count =  0 ;
    static HandleStats* getInstance(size_t freshRate = 1000){
        if(instance == nullptr){
            instance = new HandleStats(freshRate);
        }
        return instance;
    }

    size_t freshRate; 
    std::unordered_map<std::string, Stat*> stats;

    
    virtual ~HandleStats(){
        for(auto& [name, stat]: stats){
            delete stat;
        }
    }

    virtual std::string toString();
   
    void add_stat(Stat* stat);
    void remove_stat(Stat* stat);
    void add_value(std::string name, float value);
    bool canPrint(){
        return count % freshRate == 0 && count != 0;
    }

    void incrementCount(){
        count++;
    }
    void resetCount(){
        count = 0 ;
    }

private:
    HandleStats(size_t freshRate = 50): freshRate(freshRate) {}
    
};