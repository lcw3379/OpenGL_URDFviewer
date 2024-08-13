#ifndef LOAD_URDF_H
#define LOAD_URDF_H

#include "tinyxml2.h"
#include <list>
#include <Eigen/Dense>
#include <string>

class LoadURDF {
public:

    void Load(const char* filename, std::list<std::string>& stlnames, std::list<Eigen::Vector3d>& xyzvectors, std::list<Eigen::Vector3d>& rpyvectors); // URDF 파일 로드 및 파싱

private:
    void ParseInertialData(const tinyxml2::XMLElement* pInertial);
    void ParseVisualData(const tinyxml2::XMLElement* pVisual);
    void ParseCollisionData(const tinyxml2::XMLElement* pCollision);
    void ParseJointParameters(const tinyxml2::XMLElement* pJoint);
};

#endif // LOAD_URDF_H