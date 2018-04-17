#ifndef AUTOCOMPLETEGRAPH_ACGPRIOR_13042018
#define AUTOCOMPLETEGRAPH_ACGPRIOR_13042018

#include <ctime>
#include <fstream>
#include <random>
#include <vector>
#include <set>
#include "g2o/types/slam2d/parameter_se2_offset.h"
//#include "ndt_feature/ndt_feature_graph.h"
#include "Eigen/Core"
//#include "bettergraph/PseudoGraph.hpp"
//#include "vodigrex/linefollower/SimpleNode.hpp"
//#include "ndt_feature_finder/ndt_corner.hpp"
#include "covariance.hpp"
//#include "conversion.hpp"
//#include "OptimizableAutoCompleteGraph.hpp"
#include "PriorLoaderInterface.hpp"
//#include "ndt_feature_finder/conversion.hpp"
//#include "utils.hpp"
//#include "VertexAndEdge/EdgeInterfaceMalcolm.hpp"
//#include "VertexAndEdge/EdgeOdometry.hpp"
//#include "VertexAndEdge/EdgeLandmark.hpp"
//#include "VertexAndEdge/EdgeLinkXY.hpp"
//#include "VertexAndEdge/VertexSE2RobotPose.hpp"


namespace AASS {

	namespace acg {

		/**
		 * @brief The graph class containing all elements from the prior. Needed for the templated version of ACGLocalization :(.
		 */
		template<typename VERTEXTYPE, typename EDGETYPE>
		class AutoCompleteGraphPrior {

		protected:
			///@brief vector storing all node from the prior
			std::set<VERTEXTYPE*> _nodes_prior;
			///@brief vector storing all edge between the prior nodes
			std::set<EDGETYPE*> _edge_prior;

			Eigen::Vector2d _priorNoise;
			double _prior_rot;
			g2o::ParameterSE2Offset* _sensorOffset;

			bool _use_user_prior_cov = false;

		public:

			AutoCompleteGraphPrior(const Eigen::Vector2d& pn, double rp, const g2o::SE2& sensoffset) : _priorNoise(pn), _prior_rot(rp){
				// add the parameter representing the sensor offset ATTENTION was ist das ?
				_sensorOffset = new g2o::ParameterSE2Offset;
				_sensorOffset->setOffset(sensoffset);
				_sensorOffset->setId(0);
			}
			AutoCompleteGraphPrior(const g2o::SE2& sensoffset){
				// add the parameter representing the sensor offset ATTENTION was ist das ?
				_sensorOffset = new g2o::ParameterSE2Offset;
				_sensorOffset->setOffset(sensoffset);
				_sensorOffset->setId(0);
			};


			/** Accessor**/
			typename std::set<VERTEXTYPE*>& getPriorNodes(){return _nodes_prior;}
			const typename std::set<VERTEXTYPE*>& getPriorNodes() const {return _nodes_prior;}
			///@brief vector storing all edge between the prior nodes
			typename std::set<EDGETYPE*>& getPriorEdges(){ return _edge_prior;}
			const typename std::set<EDGETYPE*>& getPriorEdges() const { return _edge_prior;}

			void setPriorNoise(double a, double b){_priorNoise << a, b;}
			void setPriorRot(double r){_prior_rot = r;}

			void useUserCovForPrior(bool u){_use_user_prior_cov = u;}
			bool isUsingUserCovForPrior() const {return _use_user_prior_cov;}


			virtual VERTEXTYPE* addPriorLandmarkPose(const g2o::SE2& se2, const PriorAttr& priorAttr, int index) = 0;
			virtual VERTEXTYPE* addPriorLandmarkPose(const Eigen::Vector3d& lan, const PriorAttr& priorAttr, int index) = 0;
			virtual VERTEXTYPE* addPriorLandmarkPose(double x, double y, double theta, const PriorAttr& priorAttr, int index) = 0;

			virtual EDGETYPE* addEdgePrior(const g2o::SE2& se2, g2o::HyperGraph::Vertex* v1, g2o::HyperGraph::Vertex* v2) = 0;

			//FUNTION TO ADD A PRIOR GRAPH INTO THE GRAPH
			/**
			 * @brief Directly use the prior graph to init the prior part of the ACG
			 *
			 */
			virtual int addPriorGraph(const PriorLoaderInterface::PriorGraph& graph, int first_index) = 0;
			///@remove the prior and all link edges
//			virtual void clearPrior() = 0;
			virtual void checkNoRepeatingPriorEdge();

			virtual void clear(){
				//It's a set so not needed
//				for(typename std::set<VERTEXTYPE*>::iterator it = getPriorNodes().begin() ; it != getPriorNodes().end() ; ++it){
//
////					for(auto it1 = it + 1 ; it1 != getPriorNodes().end() ;++it1){
////						assert(*it != *it1);
//////						++i;
////					}
//				}
				std::cout << "Clear the prior" << std::endl;
				_nodes_prior.clear();
				_edge_prior.clear();
				std::cout << "Prior cleared" << std::endl;
			}

			g2o::HyperGraph::Vertex* removeVertex(g2o::HyperGraph::Vertex* v1){
				//Prior
				VERTEXTYPE* ptr = dynamic_cast<VERTEXTYPE*>(v1);

				if(ptr != NULL){
					std::cout <<"Found vertex" << std::endl;
					auto it = _nodes_prior.find(ptr);
//					int index = findPriorNode(v1);
					_nodes_prior.erase(it);
				}
				return ptr;
			}


		};


		template<typename VERTEXTYPE, typename EDGETYPE>
		inline void AASS::acg::AutoCompleteGraphPrior<VERTEXTYPE, EDGETYPE>::checkNoRepeatingPriorEdge(){
			for(auto it_vertex = _nodes_prior.begin() ; it_vertex != _nodes_prior.end() ; ++it_vertex){
				std::vector<std::pair<double, double> > out;
				// 			std::cout << "edges " << std::endl;
				auto edges = (*it_vertex)->edges();
				// 			std::cout << "edges done " << std::endl;
				std::vector<EDGETYPE*> edges_prior;

				for ( auto ite = edges.begin(); ite != edges.end(); ++ite ){
					// 				std::cout << "pointer " << dynamic_cast<g2o::EdgeXYPrior*>(*ite) << std::endl;
					EDGETYPE* ptr = dynamic_cast<EDGETYPE*>(*ite);
					if(ptr != NULL){
						//Make sure not pushed twice
						for(auto ite2 = edges_prior.begin(); ite2 != edges_prior.end(); ++ite2 ){
							assert(ptr != *ite2);
						}
						// 						std::cout << " pushed edges " << std::endl;
						edges_prior.push_back(ptr);
						// 						std::cout << "pushed edges done " << std::endl;
					}
				}
				for(auto it = edges_prior.begin() ; it != edges_prior.end() ; ++it){
					auto ite2 = it;
					ite2++;
					for( ; ite2 != edges_prior.end() ; ++ite2 ){
						assert((*it)->getOrientation2D(**it_vertex) != (*ite2)->getOrientation2D(**it_vertex));
					}
				}
			}
			for(auto it = _edge_prior.begin() ; it != _edge_prior.end() ; ++it){
				auto ite2 = it;
				ite2++;
				for(; ite2 != _edge_prior.end() ; ++ite2 ){
					assert(it != ite2);
				}
			}
		}
	}
}

#endif