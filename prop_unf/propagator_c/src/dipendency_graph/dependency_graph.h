#pragma once
#include <vector>
#include <cassert>
using namespace std;

class AdjacencyList;
typedef unsigned int Var;

class DependencyGraph 
{
    public:
        DependencyGraph();    
        ~DependencyGraph();    

        void addEdge( unsigned int v1, unsigned int v2 );
        void computeStrongConnectedComponents();        

        vector< Var >& getComponent( unsigned int pos ){ assert( pos < components_.size() ); return components_[ pos ]; }
        unsigned int numberComponents() const{ return components_.size(); }
        vector< vector< Var > >& getComponents() { return components_; }
        size_t componentSize( unsigned int pos ){
            assert( pos < components_.size() );
            return components_[ pos ].size(); 
        }
        
        bool tight() const { return tight_; } //return numberOfCyclicComponents() == 0; }

    private:        
        DependencyGraph( const DependencyGraph& orig );

        vector< vector< Var > > components_;
        AdjacencyList& graph;
        bool tight_;        
};
 