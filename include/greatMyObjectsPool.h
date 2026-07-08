#pragma once
#include "greatCommon.h"


void addSphere(std::vector<MyObj>& obj_vector, Vec3 pos, float radius, Vec4 color, float refractive_index){
    obj_vector.emplace_back(MyObj{Vec4{1.f-color.x,1.f-color.y,1.f-color.z,1.f-color.w}, Vec4{pos.x,pos.y,pos.z,radius}, Vec4{refractive_index,0.,0.,0.},Vec4{0.,0.,0.,0.},0,0.,0.,0.});
}

void addRectangle(std::vector<MyObj>& obj_vector, Vec3 center, Vec3 normal, Vec3 X_direction, Vec4 color, float refractive_index){
    obj_vector.emplace_back(MyObj{Vec4{1.f-color.x,1.f-color.y,1.f-color.z,1.f-color.w}, Vec4{center.x,center.y,center.z,refractive_index}, Vec4{normal.x,normal.y,normal.z,0.},Vec4{X_direction.x,X_direction.y,X_direction.z,0.}, 1, 0.,0.,0.});
}

void addPlane(std::vector<MyObj>& obj_vector, Vec3 point, Vec3 normal, Vec4 color, float refractive_index){
    obj_vector.emplace_back(MyObj{Vec4{1.f-color.x,1.f-color.y,1.f-color.z,1.f-color.w}, Vec4{point.x,point.y,point.z,refractive_index}, Vec4{normal.x,normal.y,normal.z,0.},Vec4{0.,0.,0.,0.}, 1, 0.,0.,0.});
}