#include "Mesh.h"
#include <OpenglWrapper.h>
#include <Renderer.h>
#include <Printer.h>

#include <ranges>
#include <algorithm>
#include <glm/glm.hpp>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) 
    : vertices(std::move(vertices)), indices(std::move(indices)), textures(std::move(textures))
{
    setupMesh();
}

void Mesh::setDrawMode(DrawMode mode_in)
{
    mode = mode_in;
}

std::vector<Vertex> Mesh::getVertices()
{
    return vertices;
}

std::vector<Vertex>& Mesh::getVerticesRef()
{
    return vertices; 
}

std::vector<unsigned int> Mesh::getIndices()
{
    return indices;
}

std::vector<unsigned int>& Mesh::getIndicesRef()
{
    return indices;
}

std::vector<Texture> Mesh::getTextures()
{
    return textures;
}

std::vector<Texture>& Mesh::getTexturesRef()
{
    return textures;
}

void Mesh::setTexture(const std::string& path, const std::string& name)
{
    //TODO(darius) make it single texture
    std::string fullPath;
    fullPath.reserve(path.size() + name.size() + 1);
    fullPath.append(path);
    fullPath.append("/");
    fullPath.append(name);

    Texture texture(TextureFromFile(name.c_str(), path.c_str(), false, false), fullPath , "texture_diffuse");
    textures.push_back(texture);
}

void Mesh::setTexture(const std::string& path)
{
    //TODO(darius) make it single texture
    Texture texture(TextureFromFile(path.c_str(), false, false), path , "texture_diffuse");
    textures.push_back(texture);
}

VAO Mesh::getVao()
{
    return vao;
}

DrawMode Mesh::getDrawMode()
{
    return mode;
}

MeshType Mesh::getType()
{
    return type;
}

void Mesh::Draw(Shader& shader)
{
    //std::cout << "positions " << vertices[0].Position.x << " " << vertices[0].Position.x << vertices[0].Position.y << "\n";
    //std::unique_lock<std::mutex>(draw_mutex);
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    vao.bind();

    /*if (Renderer::shaderLibInstance->stage == ShaderLibrary::STAGE::SHADOWS)
    {
        std::cout << "suka\n";
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, static_cast<int>(textures[0].get_texture()));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Renderer::shaderLibInstance->depthMap);
    }

    else*/
    {
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            std::string number;
            std::string name = textures[i].get_type();
            if (name == "texture_diffuse"){
                number = std::to_string(diffuseNr++);
            }
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);
            else if (name == "texture_normal")
                number = std::to_string(normalNr++);
            else if (name == "texture_height")
                number = std::to_string(heightNr++);

            OpenglWrapper::SetShaderInt(shader.getShader(), (name + number).c_str(), i);
            OpenglWrapper::BindTexture(static_cast<int>(textures[i].get_texture()));

            //OpenglWrapper::BindTexture(Renderer::shaderLibInstance->depthMap);
            //OpenglWrapper::ActivateTexture();
        }

    }

    //TODO(darius) perfomance issues?
    if(mode == DrawMode::DRAW_AS_ARRAYS)
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    else if(mode == DrawMode::DRAW_AS_INSTANCE)
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);
    else            
        OpenglWrapper::DrawElements(static_cast<int>(indices.size()));

    //NOTE(darius) to draw cubeMesh
    //glDrawArrays(GL_TRIANGLES, 0, 36);
    //OpenglWrapper::UnbindVAO();

    Renderer::drawCallsCount++;
}

void Mesh::Draw(Shader& shader, size_t amount)
{
    // std::unique_lock<std::mutex>(draw_mutex);
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++)
    {

        std::string number;
        std::string name = textures[i].get_type();
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_height")
            number = std::to_string(heightNr++);

        OpenglWrapper::SetShaderInt(shader.getShader(), (name + number).c_str(), i);
        OpenglWrapper::ActivateTexture(GL_TEXTURE0 + i);
        OpenglWrapper::BindTexture(static_cast<int>(textures[i].get_texture()));
    }

    vao.bind();

    //OpenglWrapper::DrawInstances(36, amount);
    glDrawElementsInstanced(
        GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, amount
    );
    //glDrawArraysInstanced(
    //    GL_TRIANGLES, 0, 36, amount
    //);

    //OpenglWrapper::DrawElements(static_cast<int>(indices.size()));
    //OpenglWrapper::DrawArrays(36);
    OpenglWrapper::UnbindVAO();
    //OpenglWrapper::BindTexture(GL_TEXTURE0);

    Renderer::drawCallsInstancedCount++;
}

void Mesh::setupMesh()
{
    //std::unique_lock<std::mutex>(draw_mutex);

    //DANGER(darius) TODO(darius) reinit, if was initialized before
    if (!initialized) {
        vao.init();
        vbo.init();
        ebo.init();
        initialized = true;
    }

	vao.bind();
    vbo.bind(vertices.size() * sizeof(Vertex), &vertices[0]);
    
    ebo.bind(indices.size() * sizeof(unsigned int), &indices[0]);

    //NOTE(darius) TODO(darius) try batching approach using glBufferSubData(...)?
    OpenglWrapper::EnableAttribute(0);
    OpenglWrapper::AttributePointer(0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);

    OpenglWrapper::EnableAttribute(1);
    OpenglWrapper::AttributePointer(1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    OpenglWrapper::EnableAttribute(2);
    OpenglWrapper::AttributePointer(2, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    OpenglWrapper::EnableAttribute(3);
    OpenglWrapper::AttributePointer(3, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

    OpenglWrapper::EnableAttribute(4);
    OpenglWrapper::AttributePointer(4, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    
    OpenglWrapper::EnableAttribute(5);
    OpenglWrapper::AttributePointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

    OpenglWrapper::EnableAttribute(6);
    OpenglWrapper::AttributePointer(6, 4, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    
    OpenglWrapper::UnbindVAO();
}

void Mesh::printVertices()
{
    std::cout << "Verts:\n";
    for(int i = 0; i < vertices.size(); ++i)
    {
        std::cout << vertices[i].Position.x << " " << vertices[i].Position.y << " " << vertices[i].Position.z << "\n"; 
    } 

    std::cout << "Inds:\n";
    for(int i = 0; i < indices.size(); ++i)
    {
        std::cout << indices[i] << "\n";
    } 
}

void Mesh::addVerticesBath(Mesh& batchee, glm::vec3 shift)
{   
    //NOTE(darius) for some reason it doesnt change runtime state fo mesh. But it changes its vectors, so  if u serialize it - it will work..
    //NOTE(darius) if u create new mesh from this mesh vectors u can go ril fast
    //TODO(darius) it now only batches first mesh from input. Not urgent, but needed to batch all meshes of batchee

    auto verts = batchee.getVertices() | std::ranges::views::take(4);
    std::ranges::for_each(verts, [shift](auto& v){v.Position += shift; });

    int offset = vertices.size();
    vertices.insert(vertices.end(), verts.begin(), verts.end());

    auto indV = batchee.getIndices();

    //std::cout << "offset" << offset << "\n";
    //std::cout << "indices size " << indV.size() << "\n";

    for(auto& i : indV)
    {
        i += offset;
    }

    indices.insert(indices.end(), indV.begin(), indV.begin() + 6);
    //mode = DrawMode::DRAW_AS_ARRAYS;
}

void Mesh::clearBatch(size_t verticesNum, size_t indicesNum)
{
    vertices.erase(vertices.begin() + verticesNum, vertices.end()); 
    indices.erase(indices.begin() + indicesNum, indices.end()); 
}

void Mesh::calculateAabb(const Transform& tr) 
{
    glm::vec3 vertex = glm::vec3(tr.matrix * glm::vec4(vertices[indices[0]].Position, 1.0f));
    glm::vec3 vmin = vertex;
    glm::vec3 vmax = vertex;

    for (size_t i = 0; i < indices.size(); ++i) {
        vertex = glm::vec3(tr.matrix * glm::vec4(vertices[indices[i]].Position, 1.0f));
        vmin = glm::length(vmin) < glm::length(vertex) ? vmin : vertex;
        vmax = glm::length(vmax) > glm::length(vertex) ? vmax : vertex;
    }

    aabb.min = vmin;
    aabb.max = vmax;
    aabb.center = (vmin + vmax) * 0.5f;
    aabb.size = (vmax - vmin) * 0.5f;
}

std::vector<unsigned int> Mesh::generateLOD()
{   
    std::vector<unsigned int> indicesGenerated; 

    for(int i = 0; i < indices.size(); ++i)
    {
        if(i % 2 == 0)
            indicesGenerated.push_back(indices[i]); 
    }

    return indicesGenerated;
}
