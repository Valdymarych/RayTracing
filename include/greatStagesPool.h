#pragma once
#include "greatCommon.h"
#include "greatMyObjectsPool.h"
void defineStage1(std::vector<MyObj>& obj_vector, State& state){
    addSphere(obj_vector, {0.,0.,0.}, 0.3, {0.2,0.9,0.9,.1}, 1.0);
}

void defineStage2(std::vector<MyObj>& obj_vector, State& state){
    addSphere(obj_vector, {0.4,-0.4,0.}, 0.3, {0.2,0.9,0.9,.1}, 10.0);
    addSphere(obj_vector, {-0.4,0.4,0.}, 0.3, {0.2,0.9,0.9,.1}, 10.0);
    addSphere(obj_vector, {0.4,0.4,0.}, 0.3, {0.2,0.9,0.9,.1}, 10.0);
    addSphere(obj_vector, {-0.4,-0.4,0.}, 0.3, {0.2,0.9,0.9,.1}, 10.0);
}

void defineStage3(std::vector<MyObj>& obj_vector, State& state){
    addSphere(obj_vector, {0.0,0.4,0.}, 0.3, {0.2,0.9,0.9,.1}, 10.0);
    addPlane(obj_vector, {0.,-0.1,0.}, {0.,1.,0.}, {1.,0.,0.,0.1}, 2.0);
}