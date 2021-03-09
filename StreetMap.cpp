#include "provided.h"
#include <string>
#include <vector>
#include <functional>
#include"ExpandableHashMap.h"
#include<fstream>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> m_map;
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{
    ifstream inFile(mapFile);
    if (!inFile)
        return false;

    //loading m_segments
    string streetName;
    while (getline(inFile, streetName)) {
        int t;
        inFile >> t;
        inFile.ignore(10000, '\n');
        for (int i = 0; i < t; i++) {
            string segmentStr;
            getline(inFile, segmentStr);
            string latitudeStr = segmentStr.substr(0, 10);
            string longtitudeStr = segmentStr.substr(11, 12);
            GeoCoord start = GeoCoord(latitudeStr, longtitudeStr);

            latitudeStr = segmentStr.substr(24, 10);
            longtitudeStr = segmentStr.substr(35, 12);
            GeoCoord end = GeoCoord(latitudeStr, longtitudeStr);
            StreetSegment newSegment =StreetSegment(start, end, streetName); 
            StreetSegment reverseSegment = StreetSegment(end, start, streetName);
            
            auto segVectorPtrforStart=m_map.find(start);
            auto segVectorPtrforEnd = m_map.find(end);
            if (segVectorPtrforStart == nullptr) {
                vector<StreetSegment> newVector;
                newVector.push_back(newSegment);
                m_map.associate(start, newVector);
            }
            else {
                segVectorPtrforStart->push_back(newSegment);
            }

            if (segVectorPtrforEnd == nullptr) {
                vector<StreetSegment> newVector;
                newVector.push_back(reverseSegment);
                m_map.associate(end, newVector);
            }
            else {
                segVectorPtrforEnd->push_back(reverseSegment);
            }
        }
    }

    
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    auto vectorPtr=m_map.find(gc);
    if (vectorPtr == nullptr)
        return false;
    vector<StreetSegment> res;
    for (auto it = vectorPtr->begin(); it != vectorPtr->end(); it++) {
        res.push_back((*it));
    }
    segs = res;
    return true;
    
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
