#include "provided.h"
#include"ExpandableHashMap.h"
#include <list>
#include<queue>

using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_streetMap;
    struct PathNode {
        GeoCoord coord;
        double pathCost;
        double toDestCost;
        double totalCost;
        PathNode* lastNode;

        bool operator<(const PathNode& a) const {
            return totalCost > a.totalCost;
        }

        void operator=(const PathNode& src) {
            totalCost = src.totalCost;
            pathCost = src.pathCost;
            toDestCost = src.toDestCost;
            lastNode = src.lastNode;
            coord = src.coord;
        }

        double getToDestCost(const GeoCoord& dest) {
            return distanceEarthMiles(coord, dest);
        }
        double getPathCost(PathNode* prevNode) {
            if (prevNode == nullptr)
                return 0;
            return prevNode->pathCost + distanceEarthMiles(prevNode->coord, coord);
        }

 
    };

    StreetSegment getLastStreetSeg(PathNode* node) const {
        StreetSegment res;
        vector<StreetSegment> candi;
        if (!m_streetMap->getSegmentsThatStartWith((node->lastNode->coord), candi))
            cerr << "Error at getSegment" << endl;
        for (auto it = candi.begin(); it != candi.end(); it++) {
            if ((*it).end == (node->coord)) {
                res = *it;
                break;
            }
        }
        return res;
    }

    list<StreetSegment> getCompletepathwithLastNode(PathNode* node) const {
        list<StreetSegment> res;
        while (node->lastNode != nullptr) {
            res.push_front(getLastStreetSeg(node));
            node = node->lastNode;
        }
        return res;
    }

 


};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
    m_streetMap = sm;
 
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    //data structures
    ExpandableHashMap<GeoCoord, PathNode*> coordToPathNodeMap;  
    priority_queue<PathNode> prQueue;   
    vector<PathNode*> pathNodes;
    vector<StreetSegment> currSegments;

    if(!m_streetMap->getSegmentsThatStartWith(end,currSegments)||!m_streetMap->getSegmentsThatStartWith(start,currSegments))
        return BAD_COORD;
    PathNode* firstNode = new PathNode();

    firstNode->coord = start;
    firstNode->pathCost = firstNode->getPathCost(nullptr);
    firstNode->toDestCost = firstNode->getToDestCost(end);
    firstNode->totalCost = firstNode->pathCost + firstNode->toDestCost;
    firstNode->lastNode = nullptr;
    pathNodes.push_back(firstNode);
    prQueue.push(*firstNode);
    coordToPathNodeMap.associate(firstNode->coord, firstNode);

    list<StreetSegment> res;

    while (!prQueue.empty()) {
        PathNode currNode = prQueue.top();
        /*cout << currNode.coord.latitudeText << " " << currNode.coord.longitudeText << " PathCost: "<<currNode.pathCost<<" totalCost: "<<currNode.totalCost<<endl;*/
        prQueue.pop();
        if (currNode.coord == end) {
            //found dest
            res = getCompletepathwithLastNode(&currNode);
            route = res;
            totalDistanceTravelled = currNode.pathCost;
            //clear pathNodes Vector
            for (auto it = pathNodes.begin(); it != pathNodes.end(); it++) {
                delete* it;
            }
            return DELIVERY_SUCCESS;
        }
        vector<StreetSegment> candidates;
        m_streetMap->getSegmentsThatStartWith(currNode.coord,candidates);
        /*if (candidates.size() >= 3)
            cout << "Intersection" << endl;*/
        for (auto it = candidates.begin(); it != candidates.end(); it++) {
            PathNode** currNodePtr = coordToPathNodeMap.find((*it).start);
            if (currNodePtr == nullptr) {
                cerr << "Impossible error" << endl;
                return NO_ROUTE;
            }
            
            PathNode* newNode = new PathNode();
            newNode->coord = (*it).end;
            newNode->lastNode = *currNodePtr;
            newNode->pathCost = newNode->getPathCost(newNode->lastNode);
            newNode->toDestCost = newNode->getToDestCost(end);
            newNode->totalCost = newNode->pathCost + newNode->toDestCost;
            PathNode** p = coordToPathNodeMap.find((*it).end);
            if (p == nullptr) {
                prQueue.push(*newNode);
                coordToPathNodeMap.associate((newNode->coord), newNode);
                pathNodes.push_back(newNode);
            }
            else {
                if (newNode->totalCost < (*p)->totalCost) {
                    pathNodes.push_back(newNode);
                    prQueue.push(*newNode);
                    //cout << newNode->coord.latitudeText << " " << newNode->coord.longitudeText << " "<<newNode->totalCost<< " over " << (*p)->totalCost<< endl;
                    coordToPathNodeMap.associate(newNode->coord, newNode);
                }
                else {
                    delete newNode;
                }
            }
           
        }
    }
    //clear PathNode vector 
    for (auto it = pathNodes.begin(); it != pathNodes.end(); it++) {
        delete* it;
    }

    return NO_ROUTE;


}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
