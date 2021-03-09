#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
   const StreetMap* m_map;
   string degreeToString(double deg) const {
       if (deg < 0)
           return "invalid degree";
       if (deg < 22.5)
           return "east";
       if (deg < 67.5)
           return "northeast";
       if (deg < 112.5)
           return "north";
       if (deg < 157.5)
           return "northwest";
       if (deg < 202.5)
           return "west";
       if (deg < 247.5)
           return "southwest";
       if (deg < 292.5)
           return "south";
       if (deg < 337.5)
           return "southeast";
       return "east";
   }

   //return "" if we should generate a proceed cmd
   string turningDir(double deg) const {
       if (deg >= 1 && deg < 180)
           return "left";
       if (deg >= 180 && deg <= 359)
           return "right";
       return "";
   }

   vector<DeliveryCommand> translateOneDelivery(list<StreetSegment> delivery,string item, bool isReturning) const {
       vector<DeliveryCommand> res;
       auto firstIt = delivery.begin();
       if (firstIt == delivery.end()) {
           DeliveryCommand com;
           com.initAsDeliverCommand(item);
           res.push_back(com);
           return res;
       }
       auto secondIt = delivery.begin();
       secondIt++;
       double subDistance = distanceEarthMiles((*firstIt).start, (*firstIt).end);
       double startDegree = angleOfLine(*firstIt);
       while(secondIt!=delivery.end()){
           if((*secondIt).name == (*firstIt).name) {
               subDistance += distanceEarthMiles((*secondIt).start, (*secondIt).end);
               secondIt++;
           }
           else {
               DeliveryCommand newProceed;
               newProceed.initAsProceedCommand(degreeToString(startDegree), (*firstIt).name, subDistance);
               res.push_back(newProceed);
               subDistance = 0;
               firstIt = prev(secondIt);//proceed firstIt before the corner

               DeliveryCommand newTurn;
               string turnStrDir = turningDir((angleBetween2Lines(*firstIt, *secondIt)));
               if (turnStrDir == "") {
                   //refresh
                   secondIt++;
                   firstIt++;
                   subDistance = distanceEarthMiles((*firstIt).start, (*firstIt).end);
                   double startDegree = angleOfLine(*firstIt);
                   continue;  //straight line
               }
               newTurn.initAsTurnCommand(turnStrDir, (*secondIt).name);
               res.push_back(newTurn);
               
               //refresh
               secondIt++;
               firstIt++;
               subDistance= distanceEarthMiles((*firstIt).start, (*firstIt).end);
               startDegree = angleOfLine(*firstIt);
           }
       }
       DeliveryCommand newProceed;
       newProceed.initAsProceedCommand(degreeToString(startDegree), (*firstIt).name, subDistance);
       res.push_back(newProceed);
       subDistance = 0;

       if (!isReturning) {
           DeliveryCommand DeliverCom;
           DeliverCom.initAsDeliverCommand(item);
           res.push_back(DeliverCom);
       }

       return res;

   }
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
    m_map = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    vector<DeliveryCommand> res;

    DeliveryOptimizer optimizer(m_map);
    double oldCrowDis, newCrowDis;
    vector<DeliveryRequest> optimizedDeliveries = deliveries;
    optimizer.optimizeDeliveryOrder(depot, optimizedDeliveries, oldCrowDis, newCrowDis);

    PointToPointRouter router(m_map);
    list<list<StreetSegment>> paths; //list of list
    
    double totalDistance = 0;
    GeoCoord lastLoc = depot;
    for (int i = 0; i < optimizedDeliveries.size(); i++) {
        list<StreetSegment> toNextLoc;
        double subDistance;
        DeliveryResult SubResult = router.generatePointToPointRoute(lastLoc, optimizedDeliveries[i].location,toNextLoc , subDistance);
        totalDistance += subDistance;
        if (SubResult != DELIVERY_SUCCESS)
            return SubResult;
        lastLoc = optimizedDeliveries[i].location;
        paths.push_back(toNextLoc);

        ////TESTING
        //for (auto it = toNextLoc.begin(); it != toNextLoc.end(); it++) {
        //    cerr << (*it).start.latitudeText << " " << (*it).start.longitudeText << " " << (*it).end.latitudeText << " " << (*it).end.longitudeText << " " << (*it).name << endl;
        //}
    }
    list<StreetSegment> backPath;
    double backDistance=0;
    DeliveryResult SubResult=DELIVERY_SUCCESS;
    if(optimizedDeliveries.size()!=0)
          SubResult = router.generatePointToPointRoute(optimizedDeliveries.back().location, depot, backPath, backDistance);
    totalDistance += backDistance;
    if (SubResult != DELIVERY_SUCCESS)
        return SubResult;
    paths.push_back(backPath);
    ////Testing
    //for (auto it = backPath.begin(); it != backPath.end(); it++) {
    //    cerr << (*it).start.latitudeText << " " << (*it).start.longitudeText << " " << (*it).end.latitudeText << " " << (*it).end.longitudeText << " " << (*it).name << endl;
    //}

    int vectorPos = 0;
    for (auto it = paths.begin(); it != paths.end(); it++) {
        if (vectorPos < optimizedDeliveries.size()) {
            vector<DeliveryCommand> cmdSegment= translateOneDelivery(*it, optimizedDeliveries[vectorPos].item, false);
            vectorPos++;
            res.insert(res.end(), cmdSegment.begin(), cmdSegment.end());
        }
        else {
            if (optimizedDeliveries.size() != 0) {
                vector<DeliveryCommand> cmdSegment = translateOneDelivery(*it, "", true);
                res.insert(res.end(), cmdSegment.begin(), cmdSegment.end());
            }
        }
    }
    commands = res;
    totalDistanceTravelled = totalDistance;
    return DELIVERY_SUCCESS;
    
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
