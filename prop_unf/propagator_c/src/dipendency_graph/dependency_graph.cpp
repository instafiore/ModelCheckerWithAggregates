#include "dependency_graph.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>
 
 class AdjacencyList: public boost::adjacency_list<>
 {
 };
 
 DependencyGraph::DependencyGraph()
 : graph( *new AdjacencyList() ), tight_( true )
 {
 }
 
 DependencyGraph::~DependencyGraph()
 {
     graph.clear();
     assert( graph.vertex_set().empty() );
     delete &graph;
 
 //    for( unsigned int i = 0; i < cyclicComponents.size(); i++ )
 //        delete cyclicComponents[ i ];
 }
 
 void
 DependencyGraph::addEdge(
     unsigned int v1,
     unsigned int v2 )
 {
     assert( v1 != v2 );
     boost::add_edge( v1, v2, graph );
 }
 
 void
 DependencyGraph::computeStrongConnectedComponents()
 {
     vector< unsigned int > strongConnectedComponents( boost::num_vertices( graph ) ), discover_time( boost::num_vertices( graph ) );
     vector< boost::default_color_type > color( boost::num_vertices( graph ) );
     vector< boost::graph_traits< boost::adjacency_list< > >::vertex_descriptor > root( boost::num_vertices( graph ) );
     unsigned int numberOfStrongConnectedComponents = boost::strong_components( graph, &strongConnectedComponents[ 0 ] , boost::root_map( &root[ 0 ] ).color_map( &color[ 0 ] ).discover_time_map( &discover_time[ 0 ] ) );
 
     if(numberOfStrongConnectedComponents <= 0) return ;
     
     vector< vector< Var > > components( numberOfStrongConnectedComponents );
 
     for( vector< int >::size_type i = 0; i != strongConnectedComponents.size(); ++i )
     {
         unsigned int currentComponentId = strongConnectedComponents[ i ];
         assert( currentComponentId < components.size() );
     
         components[ currentComponentId ].push_back( i );
         tight_ = ( components[ currentComponentId ].size() > 1 ) ? false : tight_;        
     }    
     
     components_.swap( components );
 }
 