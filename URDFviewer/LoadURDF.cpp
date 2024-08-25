#include "tinyxml2.h"
#include "LoadURDF.h"
#include <iostream>
#include <list>
#include <Eigen/Dense>
#include <sstream>
#include <string>

using namespace tinyxml2;
using namespace std;

void LoadURDF::ParseInertialData(const XMLElement* pInertial) {
    if (pInertial) {
        cout << "  Inertial:" << endl;
        const XMLElement* pMass = pInertial->FirstChildElement("mass");
        if (pMass) cout << "    Mass: " << pMass->Attribute("value") << endl;
    }
}

void LoadURDF::ParseCollisionData(const XMLElement* pCollision) {
    if (pCollision) {
        cout << "  Collision:" << endl;
        const XMLElement* pGeometry = pCollision->FirstChildElement("geometry");
        if (pGeometry) {

            const XMLElement* pMesh = pGeometry->FirstChildElement("mesh");
            if (pMesh) cout << "    Mesh filename: " << pMesh->Attribute("filename") << endl;
        }
    }
}

Eigen::Vector3d stringToVector3d(const std::string& s) {
    std::istringstream iss(s);
    double x, y, z;
    iss >> x >> y >> z; // 공백으로 구분된 값을 읽어서 x, y, z에 저장
    return Eigen::Vector3d(x, y, z); 
}

void LoadURDF::Load(const char* filename,std::list<std::string>& stlnames, std::list<Eigen::Vector3d>& xyzvectors, std::list<Eigen::Vector3d>& rpyvectors) {
    XMLDocument doc;
    XMLError eResult = doc.LoadFile(filename);
    std::string temp_filename(filename);

    std::list<std::string> temp_stlnames;

    if (eResult != XML_SUCCESS) {
        cout << "Error loading file!" << endl;
        return;
    }

    XMLElement* pRoot = doc.FirstChildElement("robot");
    if (!pRoot) {
        cout << "Failed to load file: No root element." << endl;
        return;
    }

    cout << "Robot name: " << pRoot->Attribute("name") << endl;

    for (XMLElement* pLink = pRoot->FirstChildElement("link"); pLink != nullptr; pLink = pLink->NextSiblingElement("link")) {
        cout << "Link: " << pLink->Attribute("name") << endl;

        const XMLElement* pInertial = pLink->FirstChildElement("inertial");
        ParseInertialData(pInertial);

        const XMLElement* pVisual = pLink->FirstChildElement("visual");
        const XMLElement* pGeometry = pVisual->FirstChildElement("geometry");
        if (pGeometry) {

            const XMLElement* pMesh = pGeometry->FirstChildElement("mesh");
            if (pMesh) cout << "    Mesh filename: " << pMesh->Attribute("filename") << endl;

            stlnames.push_back(pMesh->Attribute("filename"));
        }

        const XMLElement* pCollision = pLink->FirstChildElement("collision");
        ParseCollisionData(pCollision);
    }

    std::list<string> parsedxyzvectors, parsedrpyvectors;

    for (XMLElement* pJoint = pRoot->FirstChildElement("joint"); pJoint != nullptr; pJoint = pJoint->NextSiblingElement("joint")) {
        cout << "Joint: " << pJoint->Attribute("name") << ", Type: " << pJoint->Attribute("type") << endl;

        const XMLElement* porigin = pJoint->FirstChildElement("origin");
        if (porigin) {
            cout << " xyz=" << porigin->Attribute("xyz") << "rpy=" << porigin->Attribute("rpy") << endl;
            parsedxyzvectors.push_back(porigin->Attribute("xyz"));
            parsedrpyvectors.push_back(porigin->Attribute("rpy"));
        }

    }

    for (const std::string& str : parsedxyzvectors) {
        Eigen::Vector3d vec = stringToVector3d(str);
        xyzvectors.push_back(vec);
    }
    for (const std::string& str : parsedrpyvectors) {
        Eigen::Vector3d vec = stringToVector3d(str);
        rpyvectors.push_back(vec);
    }

}

