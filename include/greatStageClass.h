#pragma once
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <string>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <functional>
#include "greatLogger.h"
#include "greatCommon.h"
#include "greatMyObjectsPool.h"
#include "greatOpenGLHandlers.h"
#include "greatRays.h"

int initEverything(sf::RenderWindow& window, uint x, uint y){
    sf::ContextSettings settings;
    settings.depthBits = 24;
    window.create(sf::VideoMode({x,y}), 
                sf::String("VOVASOFT_TECH"), 
                sf::Style::Default, 
                sf::State::Windowed,
                settings);
    Logger::begin("Initializing everything");
    Logger::log("Depth bits: ", settings.depthBits);
    Logger::log("size of Uniforms: ",sizeof(Uniforms));
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Помилка ініціалізації GLEW!" << std::endl;
        return -1;
    }
    //window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    ImGui::CreateContext();
    if(!ImGui::SFML::Init(window, true)){
        return -1;
    }
    ImGuiIO& io = ImGui::GetIO();
    ImFont* myFont = io.Fonts->AddFontFromFileTTF("DejaVuSans.ttf", 21.0f,nullptr, io.Fonts->GetGlyphRangesDefault());
    io.Fonts->Build();
    
    Logger::end("Everything is initialized");
    return 0;
}


void drawOpenGL(sf::RenderWindow& window, MyRays& rays, State& state, UBOHandler& ubo){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ubo.update();
    rays.draw(state.display_x,state.display_y);
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        Logger::log("OpenGL Error: ",err);
    }
}

void drawImGui(sf::RenderWindow& window, sf::Time& elapsed, int& fps, State& state){
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SFML::Update(window, elapsed);
    //ImGui::ShowDemoWindow();
    ImGui::Begin("Control Panel     (1234567890.-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz)",nullptr);
    ImGui::GetStyle().ItemSpacing.y = 10.0f; 
    if (ImGui::BeginCombo(" ", state.stages[state.currect_stage].name.c_str()))
    {
        for (int i = 0; i < state.stages.size(); i++)
        {
            const bool is_selected = (state.currect_stage == i);
            if (ImGui::Selectable(state.stages[i].name.c_str(), is_selected))
                state.intended_to_be_selected_stage = i;
                
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Text("FPS: %0.1f", (float)fps);
    ImGui::End();
}

void applyStateToUbo(State& state, UBOHandler& ubo){
    ubo.uniforms.view = Matrix::getViewMatrix({0.,0.,0.}, 2., state.angle_of_vertical/180.*PI, state.angle_of_horizontal/180.*PI);
    ubo.uniforms.projection = Matrix::getProjectionMatrix(45.f/180.*PI, (float)state.display_ratio, 0.1f, 5.f);
    ubo.uniforms.cameraPos = state.cameraPos;

    ubo.uniforms.amount_of_objs = state.amount_of_objs;
    ubo.uniforms.display_ratio = state.display_ratio;
}


class Stage {
    private:

    public:
        std::function<void(std::vector<MyObj>&,State&)> defineStage;
        MyRays rays;
        std::vector<MyObj> obj_vector;
        Stage(std::function<void(std::vector<MyObj>&, State&)> defineStage)
        :
        defineStage(defineStage)
        {

        }
        void activate(sf::RenderWindow& window, State& state){
            //window.pushGLStates();
                        Logger::begin("Activating the stage... ");
                        Logger::log("defining stage");
            defineStage(obj_vector,state);
                        Logger::log("Objects amount: ", obj_vector.size());
            state.amount_of_objs = obj_vector.size();
                        Logger::begin("initializing rays");
            rays.init(state,obj_vector, "shaders/rays");
                        Logger::end("Rays are initialized");
            //window.popGLStates();
        }
        void draw(sf::RenderWindow& window, sf::Time& elapsed, int fps, UBOHandler& ubo, State& state){

                        Logger::off();
                        Logger::begin("trying to draw");
            window.clear();
                        Logger::log("clearing GL_DEPTH_BUFFER_BIT");
                        Logger::log("pushing GLStates");
            window.pushGLStates();
                        Logger::log("applying state to ubo");
            applyStateToUbo(state,ubo);
                        Logger::begin("drawing OpenGL");
            drawOpenGL(window,rays,state,ubo);
                        Logger::end("opengl drawwed");
                        Logger::log("popping GLStates");
                        Logger::end("drawwed");
            window.popGLStates();

            if (state.show_context_menu){
                Logger::log("drawing ImGui");
                drawImGui(window, elapsed, fps, state);
                        Logger::log("rendering SFML");
                ImGui::SFML::Render(window);
            }
                        Logger::end("drawwed");
            window.display();
        }
        void handleEvents(State& state, sf::RenderWindow& window){
            Logger::log("angle_x ",state.angle_of_horizontal);
            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K)){
                    state.show_context_menu = !state.show_context_menu;
                }
                
                
                if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)){
                    state.angle_of_horizontal = state.angle_of_horizontal + 5.;
                }
                if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)){
                    state.angle_of_horizontal = state.angle_of_horizontal - 5.;
                }
                if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)){
                    state.angle_of_vertical = state.angle_of_vertical + 5.;
                }
                if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)){
                    state.angle_of_vertical = state.angle_of_vertical - 5.;
                }
                if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)){
                    state.cameraPos.z = state.cameraPos.z-0.005*std::cos(state.angle_of_horizontal/180.*PI);
                    state.cameraPos.x = state.cameraPos.x+0.005*std::sin(state.angle_of_horizontal/180.*PI);
                    
                }
                if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)){
                    state.cameraPos.z = state.cameraPos.z+0.005*std::cos(state.angle_of_horizontal/180.*PI);
                    state.cameraPos.x = state.cameraPos.x-0.005*std::sin(state.angle_of_horizontal/180.*PI);
                }
                if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)){
                    state.cameraPos.z = state.cameraPos.z+0.005*std::sin(state.angle_of_horizontal/180.*PI);
                    state.cameraPos.x = state.cameraPos.x+0.005*std::cos(state.angle_of_horizontal/180.*PI);
                }
                if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)){
                    state.cameraPos.z = state.cameraPos.z-0.005*std::sin(state.angle_of_horizontal/180.*PI);
                    state.cameraPos.x = state.cameraPos.x-0.005*std::cos(state.angle_of_horizontal/180.*PI);
                }
                ImGui::SFML::ProcessEvent(window, *event);
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
            }
        }
        ~Stage(){
                        Logger::log("Stage is being destructed");
        }
};

std::unique_ptr<Stage> createStage(std::function<void(std::vector<MyObj>&, State&)> defineStage) {
    return std::make_unique<Stage>(defineStage);
}

void handleReplacingOfStage(State&state, std::unique_ptr<Stage>& stage, sf::RenderWindow& window){
    if(state.intended_to_be_selected_stage>-1){
        state.currect_stage=state.intended_to_be_selected_stage;
        state.intended_to_be_selected_stage=-1;
        stage = createStage(state.stages[state.currect_stage].defineStage);
        stage->activate(window,state);
    }
}