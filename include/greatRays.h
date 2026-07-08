#pragma once
#include "greatCommon.h"
#include "greatOpenGLHandlers.h"
#include <GL/glew.h>

class MyRays{
    private:
        ProgramHandler program;
        VAOHandler vao;
        VBOHandler vbo_arrow_points;
        SSBOHandler ssbo_objs;
        TextureHandler texture;
        TextureHandler texture_sky;

        SSBOHandler debugger;

        float quad_points[12] = {
            -1., -1.,
             1., -1.,
             1.,  1.,
            
            -1., -1.,
             1.,  1.,
            -1.,  1.
        };

    public:
        
        MyRays(){

        }
        void init(State& state, std::vector<MyObj>& obj_vector, std::string path){
            program.init(path,{1,0,1},true);
            
            vbo_arrow_points.init(quad_points,sizeof(quad_points),GL_STATIC_DRAW); 
            vbo_arrow_points.configure(0,2,0,0,0);
            vao.bind();
            vbo_arrow_points.bind_configuration();

            texture.init(state.display_x,state.display_y);
            texture_sky.initFromFile("sky.jpg");
            
            texture_sky.bindRead(2);
            texture.bindRead(1);
            texture.bindWrite(0);   

            ssbo_objs.init((float*) obj_vector.data(), sizeof(MyObj)*obj_vector.size(),0, GL_STATIC_DRAW);  

            debugger.init(NULL, 1000*sizeof(float), 4, GL_DYNAMIC_READ);

            TextureHandler::static_unbind();
            VAOHandler::unbind();
        }
        MyRays (State& state, std::vector<MyObj>& obj_vector, std::string path)
        {
            init(state, obj_vector, path);
        }
        ~MyRays (){

        }
        void draw(int pixels_X, int pixels_Y){
            
            program.compute((pixels_X+7)/8,(pixels_Y+7)/8,1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            std::vector<float> debugger_data(1000,0.);
            debugger.read(debugger_data.data(),debugger_data.size()*sizeof(float),0);
            Logger::flog("debugger: ",debugger_data[0], ", ",debugger_data[1], ", ",debugger_data[2],", ",debugger_data[3],",  ",debugger_data[4],", ",debugger_data[5], ", ",debugger_data[6]);
        
            vao.bind();
            program.use();
            glDrawArrays(GL_TRIANGLES, 0, 6);
            VAOHandler::unbind();
            ProgramHandler::stop();
        }

};
