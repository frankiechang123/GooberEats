#include "provided.h"
#include <vector>
#include<random>
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;

private:
    //randomly select two locations and swap them
    //substitude the former solution if a better delivery solution ocuurs
    //Big O: O(n)
    bool refreshToFindBetterSolution(vector<DeliveryRequest>& deliveries, double& CrowDistance,const GeoCoord& depot) const{
        default_random_engine generator;
        uniform_int_distribution<int> distribution(0, deliveries.size() - 1);
        int pos1 = 0; int pos2 = 0;
        vector<DeliveryRequest> copy = deliveries;  //O(n)
        while (pos1 == pos2) {
            pos1 = distribution(generator);
            pos2 = distribution(generator);
        }
        DeliveryRequest temp = copy[pos1];
        copy[pos1] = copy[pos2];
        copy[pos2] = temp;
        double newDistance = getCrowDistance(copy, depot);  //O(n)
        if (newDistance < CrowDistance) {
            CrowDistance = newDistance;
            deliveries = copy;
            return true;
       }
        return false;
    }

    //getCrowDistance form depot to first delivery and all the deliveries and from last deivery to depot
    double getCrowDistance(const vector<DeliveryRequest>& deliveries,const GeoCoord& depot) const {
        GeoCoord lastLoc = depot;
        double res = 0;
        for (int i = 0; i < deliveries.size(); i++) {
            res += distanceEarthMiles(lastLoc, deliveries[i].location);
        }
        if(deliveries.size()!=0)
            res += distanceEarthMiles(deliveries[deliveries.size() - 1].location, depot);
        return res;
    }
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    oldCrowDistance = getCrowDistance(deliveries, depot); //getOldCrowDistance---> O(n)
    newCrowDistance = 0;
    vector<DeliveryRequest> res;
    GeoCoord lastLoc = depot;

    while (!deliveries.empty()) {
        int minPos = 0;
        double minDis = distanceEarthMiles(lastLoc, deliveries[0].location);
        for (int i = 1; i < deliveries.size(); i++) {
            double distance = distanceEarthMiles(lastLoc, deliveries[i].location);
            if (distance < minDis) {
                minPos = i;
                minDis = distance;
            }
        }
        res.push_back(deliveries[minPos]);
        lastLoc = deliveries[minPos].location;
        deliveries.erase(deliveries.begin() + minPos);
        newCrowDistance += minDis;
    }
    if(res.size()!=0)
        newCrowDistance += distanceEarthMiles(depot, res[res.size() - 1].location);
    if (res.size() <= 1) {
        deliveries = res;
        return;
    }
    //O(n^2*n) loop to find better solution
    for (int t = 0; t < res.size() * res.size(); t++) {
        refreshToFindBetterSolution(res, newCrowDistance,depot); //O(n)
    }

    deliveries = res;
}


//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
