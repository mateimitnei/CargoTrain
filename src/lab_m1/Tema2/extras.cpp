#include "extras.h"

Mesh* CreateCylinder(const std::string &name) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    int segments = 32;
    float height = 1.0f;
    float radius = 0.5f;

    // baza sus
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(glm::vec3(cos(angle) * radius, height / 2, sin(angle) * radius));
        normals.push_back(glm::vec3(0, 1, 0)); // normala pentru discul de sus
    }
    // baza jos
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(glm::vec3(cos(angle) * radius, -height / 2, sin(angle) * radius));
        normals.push_back(glm::vec3(0, -1, 0)); // normala pentru discul de jos
    }
    // Pereti
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        // varf de sus
        vertices.push_back(glm::vec3(cos(angle) * radius, height / 2, sin(angle) * radius));
        normals.push_back(glm::normalize(glm::vec3(cos(angle), 0, sin(angle))));
        // varf de jos
        vertices.push_back(glm::vec3(cos(angle) * radius, -height / 2, sin(angle) * radius));
        normals.push_back(glm::normalize(glm::vec3(cos(angle), 0, sin(angle))));
    }

    // Indici baza sus
    for (int i = 1; i < segments - 1; i++) {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i);
    }
    // Indici baza jos
    for (int i = 1; i < segments - 1; i++) {
        indices.push_back(segments);
        indices.push_back(segments + i);
        indices.push_back(segments + i + 1);
    }

    // Indici pereti
    int side_start_index = 2 * segments;
    for (int i = 0; i < segments; i++) {
        int current = side_start_index + 2 * i;
        int next = side_start_index + 2 * ((i + 1) % segments);
        indices.push_back(current);
        indices.push_back(next + 1);
        indices.push_back(next);
        indices.push_back(current);
        indices.push_back(current + 1);
        indices.push_back(next + 1);
    }

    Mesh* cylinder = new Mesh(name);
    cylinder->InitFromData(vertices, normals, indices);
    return cylinder;
}