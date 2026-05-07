#include "lab_m1/Tema2/Tema2.h"
#include "components/text_renderer.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;

gfxc::TextRenderer* txtrainderer = nullptr;

Tema2::Tema2() {}
Tema2::~Tema2() {}


void Tema2::Init()
{
	// Initializare camera
	camera = new implemented::Camera();
	cameraTarget = 5.5f;
	camera->Set(glm::vec3(0, 23, 25), glm::vec3(0, 0, cameraTarget), glm::vec3(0, 1, 0));
	fov = RADIANS(40);
	orthoSize = 15.0f;
	projectionMatrix = glm::perspective(fov, window->props.aspectRatio, 0.01f, 200.0f);

	// Mesh-uri
	{
		Mesh* mesh = new Mesh("quad");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}
	{
		Mesh* mesh = new Mesh("box");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}
	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}
	{
		Mesh* mesh = new Mesh("teapot");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "teapot.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}
	meshes["cylinder"] = CreateCylinder("cylinder");

	// Shader
	{
		Shader *shader = new Shader("MyShader");
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;

		lightPosition = glm::vec3(0, 50, 3);
		materialShininess = 5;
		materialKd = 0.8;
		materialKs = 0.5;
	}

	// Text renderer
	txtrainderer = new gfxc::TextRenderer(window->props.selfDir, window->GetResolution().x, window->GetResolution().y);
	txtrainderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 50);

	// Initializare harta si joc
	CreateTrack();
	CreateStations();
    InitTrain();
	GenerateNewOrder();
    
	timer = 0;
	gameOver = false;
	score = 0;
	scorePosition.x = window->GetResolution().x / 2.0f - 110;
	scorePosition.y = window->GetResolution().y - 50;
}


void Tema2::CreateTrack() {
	nodes.clear();
	
	// Node 0: Gara Centrala (Sus Centru)
	nodes.push_back({0, glm::vec3(0, 0, -4), {{'S', 2}}}); 
	// Node 1: staite dreapta sus   
	nodes.push_back({1, glm::vec3(10, 0, 0), {{'A', 2}}});
	// Node 2: intersectie sus
	nodes.push_back({2, glm::vec3(0, 0, 0), {{'W', 0}, {'A', 3}, {'S', 5}, {'D', 1}}});
	// Node 3: statie stanga sus
	nodes.push_back({3, glm::vec3(-6, 0, 0), {{'S', 4}, {'D', 2}}});
	// Node 4: colt
	nodes.push_back({4, glm::vec3(-6, 0, 6), {{'W', 3}, {'D', 5}}});
	// Node 5: intersectie centru
	nodes.push_back({5, glm::vec3(0, 0, 6), {{'W', 2}, {'A', 4}, {'S', 8}, {'D', 6}}});
	// Node 6: colt
	nodes.push_back({6, glm::vec3(6, 0, 6), {{'A', 5}, {'S', 7}}});
	// Node 7: statie dreapta jos
	nodes.push_back({7, glm::vec3(6, 0, 12), {{'W', 6}, {'A', 8}}});
	// Node 8: intersectie jos
	nodes.push_back({8, glm::vec3(0, 0, 12), {{'W', 5}, {'A', 9}, {'D', 7}}});
	// Node 9: statie stanga jos
	nodes.push_back({9, glm::vec3(-9, 0, 12), {{'D', 8}}});

}


void Tema2::CreateStations() {
	stations.clear();
	// Gara centrala: nod 0
	stations.push_back({0, 0, 0, nodes[0].position});
	// Statii cu resurse: nodurile 1, 3, 7 si 9
	stations.push_back({1, 1, 0, nodes[1].position});
	stations.push_back({3, 2, 0, nodes[3].position});
	stations.push_back({7, 3, 0, nodes[7].position});
	stations.push_back({9, 4, 0, nodes[9].position});
}

void Tema2::InitTrain() {
	train.currentNodeId = 0;
	train.previousNodeId = -1;
	train.targetNodeId = 2;
	train.moving = true;
	train.speed = 10.0f;
	train.progress = 0.0f;
	train.position = nodes[0].position;
	train.direction = glm::vec3(0, 0, 1);

	// Initializare vagon
	wagonDelay = 13;
	movementHistory.clear();
	for (int i = 0; i < wagonDelay; i++) {
		movementHistory.push_back({train.position, train.direction});
	}
}


void Tema2::GenerateNewOrder() {
	// resetare timer joc la completarea unei comenzi
	timer = 0;
	score++;
	order.clear();
	// nr aleatorii de la 1 la 4
	for (int i = 0; i < 5; i++) {
		order.push_back(1 + rand() % 4);
	}
}

void Tema2::GameOver(float deltaTime) {
	// scaderea luminozitatii in timp
	materialKd = fmax(0.2f, materialKd - deltaTime);
	// rotirea camerei in timp (schimbarea punctului tinta)
	cameraTarget = fmax(-70, cameraTarget - deltaTime * 60);
	camera->Set(glm::vec3(0, 23, 25), glm::vec3(0, 0, cameraTarget), glm::vec3(0, 1, 0));

	// Afisare GAME OVER in spatele hartii
	glDisable(GL_DEPTH_TEST);
	glm::ivec2 resolution = window->GetResolution();
	txtrainderer->RenderText("GAME OVER", (resolution.x / 2.0f) - 137,
		(resolution.y / 2.5f), 1, glm::vec3(1, 0, 0)); // text rosu
	glEnable(GL_DEPTH_TEST);
}

void Tema2::FrameStart()
{
	// Clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::ivec2 resolution = window->GetResolution();
	glViewport(0, 0, resolution.x, resolution.y);
}


void Tema2::Update(float deltaTimeSeconds)
{
    // Logica joc:

	// Verificare timer si gameOver
	if (!gameOver) {
		timer += deltaTimeSeconds;
		if (timer >= maxTime) gameOver = true;
	} else {
		GameOver(deltaTimeSeconds);
	}

	// cooldown pentru resurse
	for (Station &s : stations) {
		if (s.cooldownTimer > 0) {
			s.cooldownTimer -= deltaTimeSeconds;
			if (s.cooldownTimer <= 0) s.cooldownTimer = 0;
		}
	}
	
	// Miscare tren
	if (train.moving && !gameOver) {
		glm::vec3 p1 = nodes[train.currentNodeId].position;
		glm::vec3 p2 = nodes[train.targetNodeId].position;
		float distance = glm::length(p2 - p1);

		if (distance > 0) {
			train.progress += (train.speed / distance) * deltaTimeSeconds;
		}
		
		// Daca trenul sta pe loc intr-un nod (currentNodeId)
		if (train.progress >= 1.0f) {
			train.progress = 0.0f;
			train.previousNodeId = train.currentNodeId;
			train.currentNodeId = train.targetNodeId;
			train.moving = false;

			// Daca a ajuns la gara centrala si comanda e completa, se genereaza alta comanda
			if (train.currentNodeId == 0 && order.empty()) {
				GenerateNewOrder();
			}

			// Daca a ajuns la o statie de resurse
			for (Station &s : stations) {
				if (s.nodeId == train.currentNodeId && s.nodeId != 0 && s.cooldownTimer <= 0) {
					// Verificare daca resursa este in comanda
					auto item = std::find(order.begin(), order.end(), s.resourceType);
					if (item != order.end()) {
						// se sterge prima aparitie si se porneste cooldownul
						order.erase(item);
						s.cooldownTimer = 3.0f;
					}
				}
			}

			// Vecinii disponibili (folosit la selectarea directiei la intersectii):
			availableNeighbors.clear();
			for (pair<char, int> neighbor : nodes[train.currentNodeId].neighbors) {
                int neighborId = neighbor.second;
				if (nodes[train.currentNodeId].neighbors.size() == 1 // Daca este capat de linie
                    || neighborId != train.previousNodeId) { // Daca nu este nodul de unde a venit
					availableNeighbors.insert(neighbor);
				}
			}
			// Daca este o singura optiune (capat de linie sau curba 90 de grade), continua automat
			if (availableNeighbors.size() == 1) {
				train.targetNodeId = availableNeighbors.begin()->second;
				train.moving = true;
			}

        // Daca trenul se afla intre noduri (progress < 1):
		} else {
			// Interpolare pozitie intre noduri
			glm::vec3 p1 = nodes[train.currentNodeId].position;
			glm::vec3 p2 = nodes[train.targetNodeId].position;
			train.position = glm::mix(p1, p2, train.progress);
			// Orientare catre directia de mers
			train.direction = glm::normalize(p2 - p1);
		}
		// Update la istoricul de pozitii al locomotivei
		movementHistory.push_back({train.position, train.direction});
		if (movementHistory.size() > wagonDelay) {
			movementHistory.pop_front();
		}
	}

    // Randare:
    RenderMap();
    RenderTrain();
    RenderText();
}

void Tema2::FrameEnd()
{
	//DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}

void Tema2::RenderMap() {
    // Camp
	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.2, 3));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(30, 1, 30));
		modelMatrix = glm::rotate(modelMatrix, RADIANS(90), glm::vec3(1, 0, 0));
		RenderMesh(meshes["quad"], shaders["MyShader"], modelMatrix, glm::vec3(0.6, 0.35, 0.65));
	}
    // Rau
    {
        glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-12, -0.11, 9.5));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(10, 1, 0.7));
		modelMatrix = glm::rotate(modelMatrix, RADIANS(90), glm::vec3(1, 0, 0));
		RenderMesh(meshes["quad"], shaders["MyShader"], modelMatrix, glm::vec3(0.1, 0.5, 0.8));
    }
    {
        glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.7, -0.1, 10.3));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7, 1, 1.5));
		modelMatrix = glm::rotate(modelMatrix, RADIANS(90), glm::vec3(1, 0, 0));
		RenderMesh(meshes["quad"], shaders["MyShader"], modelMatrix, glm::vec3(0.1, 0.5, 0.8));
    }
    {
        glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-4, -0.1, 12.2));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(2, 1, 0.7));
		modelMatrix = glm::rotate(modelMatrix, RADIANS(90), glm::vec3(1, 0, 0));
		RenderMesh(meshes["quad"], shaders["MyShader"], modelMatrix, glm::vec3(0.1, 0.5, 0.8));
    }
    {
        glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-6, -0.1, 16.5));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7, 1, 5));
		modelMatrix = glm::rotate(modelMatrix, RADIANS(90), glm::vec3(1, 0, 0));
		RenderMesh(meshes["quad"], shaders["MyShader"], modelMatrix, glm::vec3(0.1, 0.5, 0.8));
    }
    // Munte
    {
        glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(9.5, -0.1, -1));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(6, 1, 4));
		modelMatrix = glm::rotate(modelMatrix, RADIANS(90), glm::vec3(1, 0, 0));
		RenderMesh(meshes["quad"], shaders["MyShader"], modelMatrix, glm::vec3(0.27, 0.21, 0.2));
    }

	// Sine
	for (Node &node : nodes) {
		for (pair<char, int> neighbor : node.neighbors) {
            int neighborId = neighbor.second;
			if (node.id < neighborId) { // evitare randare a doua oara
				glm::vec3 pos1 = node.position;
				glm::vec3 pos2 = nodes[neighborId].position;
				glm::vec3 mid = (pos1 + pos2) / 2.0f;
				glm::vec3 direction = pos2 - pos1;
				float angle = atan2(direction.x, direction.z);

                // Pod (2 dungi negre si un alba)
                if (node.id == 8 && neighborId == 9) {
					for (int i = 0; i < 3; i++) {
						float offset = (i == 0) ? -0.2f : ((i == 1) ? 0.0f : 0.2f);
						glm::vec3 color = (i == 1) ? glm::vec3(0.7, 0.7, 0.7) : glm::vec3(0.1, 0.1, 0.1);
                        glm::mat4 modelMatrix = glm::mat4(1);
                        glm::vec3 position = glm::vec3(mid.x, mid.y, mid.z + offset);
                        modelMatrix = glm::translate(modelMatrix, position);
                        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
                        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.11f, glm::length(direction) - 0.5));
                        RenderMesh(meshes["box"], shaders["MyShader"], modelMatrix, color);
					}

                // Tunel
                } else if (node.id == 1 && neighborId == 2) {
                    int segmentsNum = 20;
                    float segmentLen = glm::length(direction)/segmentsNum;
                    for (int i = 0; i < segmentsNum; i++) {
                        glm::vec3 color = (i % 2 == 1 && i > 6) ? glm::vec3(0.6, 0.6, 0.6) : glm::vec3(0.1, 0.1, 0.1);
                        glm::vec3 position = glm::vec3(mid.x - glm::length(direction)/2 + i * segmentLen, mid.y, mid.z);
                        glm::mat4 modelMatrix = glm::mat4(1);
                        modelMatrix = glm::translate(modelMatrix, position);
                        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
                        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.11f, segmentLen));
                        RenderMesh(meshes["box"], shaders["MyShader"], modelMatrix, color);
                    }

                } else {
                    glm::mat4 modelMatrix = glm::mat4(1);
                    modelMatrix = glm::translate(modelMatrix, mid);
                    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
                    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.1f, glm::length(direction) + 0.5));
                    RenderMesh(meshes["box"], shaders["MyShader"], modelMatrix, glm::vec3(0.1, 0.1, 0.1));
                }
			}
		}
	}

	// Statii
	for (Station &s : stations) {
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, s.position);
		glm::vec3 color;
		Mesh* mesh = nullptr;

		if (s.nodeId == 0) {
			// Gara centrala
			color = glm::vec3(1, 1, 1); 
			modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f));
			mesh = meshes["box"];
            isCentralStation = true;
		} else {
			// Statii de resurse
			color = resourceColors[s.resourceType];
			mesh = meshes[resourceMeshes[s.resourceType]];
            if (s.resourceType == 2) { // ceainic
                modelMatrix = glm::scale(modelMatrix, glm::vec3(1.9f));
            }else {
			    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.5f));
            }
            isCentralStation = false;
		}
		RenderMesh(mesh, shaders["MyShader"], modelMatrix, color, isCentralStation);

		// Resursa disponibila de deasupra statiei
		if (s.nodeId != 0 && s.cooldownTimer <= 0 && !gameOver) {
			glm::mat4 resMatrix = glm::mat4(1);
			resMatrix = glm::translate(resMatrix, s.position + glm::vec3(0, 3, 0));
			resMatrix = glm::rotate(resMatrix, (float)glfwGetTime() * 1.5f, glm::vec3(0, 1, 0)); // Animatie
            if (s.resourceType == 2) { // ceainic
                resMatrix = glm::scale(resMatrix, glm::vec3(0.65f));
            } else {
			    resMatrix = glm::scale(resMatrix, glm::vec3(0.5f));
            }
			RenderMesh(mesh, shaders["MyShader"], resMatrix, color);
		}
	}

	// Lista de comenzi
	if (!order.empty() && !gameOver) {
		for (int i = 0; i < order.size(); i++) {
			glm::mat4 modelMatrix = glm::mat4(1);
            if (order[i] == 2) { // ceainic
                modelMatrix = glm::translate(modelMatrix, nodes[0].position + glm::vec3((i - 2), 2.82f, 0));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8f));
            } else {
                modelMatrix = glm::translate(modelMatrix, nodes[0].position + glm::vec3((i - 2), 3, 0));
			    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
            }
			
			Mesh* mesh = meshes[resourceMeshes[order[i]]];
			glm::vec3 color = resourceColors[order[i]];
			RenderMesh(mesh, shaders["MyShader"], modelMatrix, color);
		}
	}
}

void Tema2::RenderTrain() {

    // Locomotiva
	float angle = atan2(train.direction.x, train.direction.z);

	glm::mat4 trainMatrix = glm::mat4(1);
	trainMatrix = glm::translate(trainMatrix, train.position + glm::vec3(0, 0.5f, 0));
	trainMatrix = glm::rotate(trainMatrix, angle, glm::vec3(0, 1, 0));

	// Cabina (cub)
	{
        glm::mat4 cabMatrix = glm::translate(trainMatrix, glm::vec3(0, 0.3f, -0.5f));
		cabMatrix = glm::scale(cabMatrix, glm::vec3(0.8f, 1, 0.8f));
		RenderMesh(meshes["box"], shaders["MyShader"], cabMatrix, glm::vec3(0.4f, 0.4f, 0.4f));
	}
	// Motor (cilindru)
	{
		glm::mat4 engMatrix = glm::translate(trainMatrix, glm::vec3(0, 0.1f, 0.4f));
		engMatrix = glm::rotate(engMatrix, RADIANS(90), glm::vec3(1, 0, 0));
		engMatrix = glm::scale(engMatrix, glm::vec3(0.6f, 1.0f, 0.6f));
		RenderMesh(meshes["cylinder"], shaders["MyShader"], engMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
	}

    // Roti (cilindri orizontali)
	float wheelRadius = 0.2f;
	float wheelThickness = 0.1f;
	float offset = 0.4f;
	glm::vec3 wheelColor = glm::vec3(0.75f, 0.75f, 0.73f);
    glm::mat4 wheelMatrix;
    
    // Roti locomotiva:
    float zPositions[] = {-0.6f, -0.2f, 0.2f, 0.6f};
    for (float z : zPositions) {
        // Roata stanga
        wheelMatrix = glm::translate(trainMatrix, glm::vec3(-offset, wheelRadius - 0.5f, z));
        wheelMatrix = glm::rotate(wheelMatrix, RADIANS(90), glm::vec3(0, 0, 1));
        wheelMatrix = glm::scale(wheelMatrix, glm::vec3(wheelRadius, wheelThickness, wheelRadius));
        RenderMesh(meshes["cylinder"], shaders["MyShader"], wheelMatrix, wheelColor);
        // Roata dreapta
        wheelMatrix = glm::translate(trainMatrix, glm::vec3(offset, wheelRadius - 0.5f, z));
        wheelMatrix = glm::rotate(wheelMatrix, RADIANS(90), glm::vec3(0, 0, 1));
        wheelMatrix = glm::scale(wheelMatrix, glm::vec3(wheelRadius, wheelThickness, wheelRadius));
        RenderMesh(meshes["cylinder"], shaders["MyShader"], wheelMatrix, wheelColor);
    }


	// Vagon (dupa pozitiile locomotivei cu delay)
	MovingState state = movementHistory.front();
	angle = atan2(state.direction.x, state.direction.z);

	glm::mat4 baseMatrix = glm::mat4(1);
	baseMatrix = glm::translate(baseMatrix, state.position + glm::vec3(0, 0.7f, 0));
	baseMatrix = glm::rotate(baseMatrix, angle, glm::vec3(0, 1, 0));

    glm::mat4 wagonMatrix = glm::scale(baseMatrix, glm::vec3(0.8f, 0.8f, 1.5f));
	RenderMesh(meshes["box"], shaders["MyShader"], wagonMatrix, glm::vec3(0.6f, 0.3f, 0.1f));

	// Roti vagon
	float zPositionsWagon[] = {-0.5f, 0.5f};
    for (float z : zPositionsWagon) {
        // Roata stanga
        wheelMatrix = glm::translate(baseMatrix, glm::vec3(-offset, -0.5f, z));
        wheelMatrix = glm::rotate(wheelMatrix, RADIANS(90), glm::vec3(0, 0, 1));
        wheelMatrix = glm::scale(wheelMatrix, glm::vec3(wheelRadius, wheelThickness, wheelRadius));
        RenderMesh(meshes["cylinder"], shaders["MyShader"], wheelMatrix, wheelColor);
        // Roata dreapta
        wheelMatrix = glm::translate(baseMatrix, glm::vec3(offset, -0.5f, z));
        wheelMatrix = glm::rotate(wheelMatrix, RADIANS(90), glm::vec3(0, 0, 1));
        wheelMatrix = glm::scale(wheelMatrix, glm::vec3(wheelRadius, wheelThickness, wheelRadius));
        RenderMesh(meshes["cylinder"], shaders["MyShader"], wheelMatrix, wheelColor);
    }
}


void Tema2::RenderText() {
    // Afisare scor
	txtrainderer->RenderText("Completed: " + std::to_string(score), scorePosition.x,
		scorePosition.y, 0.6f, glm::vec3(1, 1, 1)); // text alb
}


void Tema2::RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 & modelMatrix, const glm::vec3 &color,
    bool isCentralStation)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// Render an object using the specified shader and the specified position
	glUseProgram(shader->program);

    GLint loc_isCentral = glGetUniformLocation(shader->program, "isCentralStation");
    glUniform1i(loc_isCentral, isCentralStation);

    GLint loc_progress = glGetUniformLocation(shader->program, "progress");
    glUniform1f(loc_progress, timer / maxTime);

	// Set shader uniforms for light & material properties
	// TODO(student): Set light position uniform
	GLint loc_light = glGetUniformLocation(shader->program, "light_position");
	glUniform3f(loc_light, lightPosition.x, lightPosition.y, lightPosition.z);

	glm::vec3 eyePosition = camera->position;
	// TODO(student): Set eye position (camera position) uniform
	GLint loc_eye = glGetUniformLocation(shader->program, "eye_position");
	glUniform3f(loc_eye, eyePosition.x, eyePosition.y, eyePosition.z);

	// TODO(student): Set material property uniforms (shininess, kd, ks, object color)
	GLint loc_shininess = glGetUniformLocation(shader->program, "material_shininess");
	glUniform1i(loc_shininess, materialShininess);
	GLint loc_kd = glGetUniformLocation(shader->program, "material_kd");
	glUniform1f(loc_kd, materialKd);
	GLint loc_ks = glGetUniformLocation(shader->program, "material_ks");
	glUniform1f(loc_ks, materialKs);
	GLint loc_color = glGetUniformLocation(shader->program, "object_color");
	glUniform3f(loc_color, color.r, color.g, color.b);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = camera->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix;
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->m_VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
	// Miscare camera
	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		float cameraSpeed = 10.0f;
		if (window->KeyHold(GLFW_KEY_W)) camera->TranslateForward(cameraSpeed * 2 * deltaTime);
		if (window->KeyHold(GLFW_KEY_A)) camera->TranslateRight(-cameraSpeed * 2 * deltaTime);
		if (window->KeyHold(GLFW_KEY_S)) camera->TranslateForward(-cameraSpeed * 2 * deltaTime);
		if (window->KeyHold(GLFW_KEY_D)) camera->TranslateRight(cameraSpeed * 2 * deltaTime);
		if (window->KeyHold(GLFW_KEY_Q)) camera->TranslateUpward(-cameraSpeed * 2 * deltaTime);
		if (window->KeyHold(GLFW_KEY_E)) camera->TranslateUpward(cameraSpeed * 2 * deltaTime);
	}
}

void Tema2::OnKeyPress(int key, int mods)
{
	if (key == GLFW_KEY_R) {
		this->Init();
	}

	// Selectare directie la intersectie
	if (!train.moving && !gameOver) {
		int nextNode = -1;
		// Determinare nod vecin pentru directia selectata
		if (availableNeighbors.find(key) != availableNeighbors.end()) {
			nextNode = availableNeighbors[key];
		}
		if (nextNode != -1) {
			train.targetNodeId = nextNode;
			train.moving = true;
		}
	}
}

void Tema2::OnKeyRelease(int key, int mods)
{
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		float sensivity = 0.001f;
		camera->RotateFirstPerson_OY(-deltaX * sensivity);
		camera->RotateFirstPerson_OX(-deltaY * sensivity);
	}
}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema2::OnWindowResize(int width, int height)
{
	txtrainderer = new gfxc::TextRenderer(window->props.selfDir, window->GetResolution().x,
											window->GetResolution().y);
	txtrainderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 50);

	scorePosition.x = window->GetResolution().x / 2.0f - 110;
	scorePosition.y = window->GetResolution().y - 50;
}