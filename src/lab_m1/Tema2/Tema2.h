#pragma once

#include "extras.h"
#include "components/simple_scene.h"
#include "lab_m1/lab5/lab_camera.h"
#include <vector>
#include <deque>
#include <map>

namespace m1
{

    struct Node {
        int id;
        glm::vec3 position;
        std::map<char, int> neighbors; // de forma directie - id vecin
    };

    struct Station {
        int nodeId;
        int resourceType;
        float cooldownTimer;
        glm::vec3 position;
    };

    class Tema2 : public gfxc::SimpleScene
    {
     public:
        Tema2();
        ~Tema2();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        // Render helper care permite trimiterea culorii catre shader
        void RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix,
            const glm::vec3 &color, bool isCentralStation = false);
        
        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        // Functii auxiliare
        void CreateTrack();
        void CreateStations();
        void InitTrain();
        void GenerateNewOrder();
        void GameOver(float deltaTime);
        void RenderMap();
        void RenderTrain();
        void RenderText();

        glm::vec3 lightPosition;
        unsigned int materialShininess;
        float materialKd;
        float materialKs;

     protected:
        implemented::Camera *camera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;
        
        // Parametrii proiectiei
        float fov;
        float orthoSize;

        // Date harta
        std::vector<Node> nodes;
        std::vector<Station> stations;
        bool isCentralStation;
        
        // Date tren
        struct {
            bool moving;
            int currentNodeId;
            int targetNodeId;
            int previousNodeId;
            float progress; // 0.0 - 1.0
            float speed;
            glm::vec3 position;
            glm::vec3 direction;
        } train;

        // Istroric miscare tren
        struct MovingState {
            glm::vec3 position;
            glm::vec3 direction;
        };
        std::deque<MovingState> movementHistory;
        int wagonDelay; // intarziere fata de locomotiva

        // lista de id-uri de resurse din comanda
        std::vector<int> order;

        float timer;
        float maxTime = 20;
        bool gameOver;
        int score;

        float cameraTarget;

        // Selectia directiei
        
        std::map<char, int> availableNeighbors; // Optiuni valide la intersectie

        // Cate o culoare pentru fiecare tip de resursa (1..4)
        std::map<int, glm::vec3> resourceColors = {
            {1, glm::vec3(1, 0, 0)},
            {2, glm::vec3(0.2, 0.75, 0.45)},
            {3, glm::vec3(0, 0.3, 1)},
            {4, glm::vec3(0.8, 0.8, 0.1)}
        };

        std::map<int, std::string> resourceMeshes = {
            {1, "box"},
            {2, "teapot"},
            {3, "sphere"},
            {4, "cylinder"}
        };
        
        struct {
            float y;
            float x;
        } scorePosition;
    };
}   // namespace m1