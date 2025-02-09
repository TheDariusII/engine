#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <optional>

#include "Engine/Window.h"
#include "Engine/GameState.h"
#include "Rendering/Shader.h"
#include <Rendering/ShaderLibrary.h>
#include "Engine/InputManager.h"
#include "Rendering/VBO.h"
#include "Rendering/VAO.h"
#include "Texture.h"
#include "EBO.h"
#include "Model.h"
#include <Rendering/CubeMesh.h>
#include <Rendering/FlatMesh.h>
#include <Engine/Scene.h>
#include <Engine/Transform.h>
#include <Engine/ParticleSystem.h>
#include <Engine/Script.h>
#include <Engine/Color.h>
#include <Engine/PointLight.h>
#include <Rendering/OpenglWrapper.h>
#include <Rendering/FrameBuffer.h>
#include <Rendering/RenderQuad.h>
#include <Rendering/GraphicsStateCache.h>
#include <Rendering/CubeMap.h>

/*
TODO(all):
	code:
        - move and drag - subentities
        - multithreaded objects loading
        - debug thread
        - benchmark
        - camera as object
        - mouse clickable objects
        - entities destruction
        - scene serialization(JSON(try simdjson eventually?) and binary)

    renderer:
        - better renderer
        - matrials
        - mesh optimization - https://github.com/zeux/meshoptimizer
	
    Physics:
        - Basic Collission detection and Physics         v
        - Inertia Matrix physics                         v
        - Particles system                               v
        - Soft Bodyies
        - Liquid Simulation 
        - Cloth Simulation
        - Hair Simulation

    imgui:
        - Panel system
        - general menu
        - show file system traversal
        - better properties traverse

    refs:
        - unity coliders - https://www.youtube.com/watch?v=bh9ArKrPY8w&ab_channel=Unity
        - titanfall SIMD collisions - https://www.youtube.com/watch?v=6BIfqfC1i7U&t=81s&ab_channel=GDC
        - GJK explained - https://www.youtube.com/watch?v=ajv46BSqcK4&ab_channel=Reducible
        - Minkowski diff by Casey - https://www.youtube.com/watch?v=_g8DLrNyVsQ&t=2933s&ab_channel=MollyRocket
        - Gregory - http://ce.eng.usc.ac.ir/files/1511334027376.pd
        - RigidBody Dynamics - https://ocw.mit.edu/courses/16-07-dynamics-fall-2009/dd277ec654440f4c2b5b07d6c286c3fd_MIT16_07F09_Lec26.pdf
        - Quaternions - https://www.3dgep.com/understanding-quaternions/
        - Hair - https://github.com/hankeyyh/FurSim/tree/master
        - Grass - https://www.youtube.com/watch?v=Ibe1JBF5i5Y&t=0s&ab_channel=GDC
        - Vertex optimisation - https://tomforsyth1000.github.io/papers/fast_vert_cache_opt.html
        - Mikkelsen - https://gamedev.stackexchange.com/questions/146855/how-do-you-compute-the-tangent-space-vectors-with-normals-given-in-the-mesh
        - GPU RigidBody - https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-29-real-time-rigid-body-simulation-gpus
        - Physics - https://theswissbay.ch/pdf/Gentoomen%20Library/Game%20Development/Programming/Game%20Physics%20Engine%20Development.pdf
        - particles - https://www.youtube.com/watch?v=G6OGKP3MaUI&list=PLnuhp3Xd9PYTt6svyQPyRO_AAuMWGxPzU&index=168&ab_channel=MollyRocket
        - Geometry - https://www.geometrictools.com/Source/Mathematics.html
        - https://en.wikipedia.org/wiki/Gauss%E2%80%93Seidel_method
        - https://raphaelpriatama.medium.com/sequential-impulses-explained-from-the-perspective-of-a-game-physics-beginner-72a37f6fea05
        - "Predictive contacts"
        - "Inastanced rendering"
        - https://hitokageproduction.com/article/11
        - https://gafferongames.com/post/collision_response_and_coulomb_friction/
        - https://www.youtube.com/watch?v=DGVZYdlw_uo&ab_channel=kuju
        - AOE networking - https://www.gamedeveloper.com/programming/1500-archers-on-a-28-8-network-programming-in-age-of-empires-and-beyond
        - Porting - https://gamedev.stackexchange.com/questions/103941/porting-sdl-opengl-game-to-android-and-ios
        - Net - https://web.archive.org/web/20210419133753/https://gameserverarchitecture.com/
        - Android port - https://arm-software.github.io/opengl-es-sdk-for-android/tutorials.html
                         https://keasigmadelta.com/store/wp-content/uploads/2017/03/GLES3-and-SDL2-Tutorials.pdf
        - mesh optimizer - https://github.com/zeux/meshoptimizer
        - https://www.youtube.com/watch?v=k2h9FORbLa4&ab_channel=Gamefromscratch
        - https://www.boost.org/doc/libs/1_82_0/doc/html/boost_dll/tutorial.html
        - https://www.youtube.com/watch?v=nmxZmIOAosY&ab_channel=mohamedshaalan
        - https://github.com/GarrettGunnell/CS2-Smoke-Grenades 
        - https://github.com/frtru/GemParticles
        - profiler - https://github.com/FlaxEngine/FlaxEngine/blob/master/Source/Engine/Profiler/ProfilerCPU.cpp
        - reddit thread om gpu memory - https://www.reddit.com/r/opengl/comments/oxl2fi/what_are_some_good_practices_to_manage_gpu_memory/
        - lecture - https://www.youtube.com/watch?v=-bCeNzgiJ8I&list=PLckFgM6dUP2hc4iy-IdKFtqR9TeZWMPjm&ab_channel=SteamworksDevelopment
        - opengl PBO - http://www.songho.ca/opengl/gl_pbo.html
        - fast texture example - https://stackoverflow.com/questions/13358455/opengl-combine-textures
        - J.Blow skeletal animations stuff - https://www.youtube.com/watch?v=4MBXWFfGYpo&ab_channel=UNOFFICIALJonathanBlowstreamarchive
        - Texture framebuffer multisample  https://stackoverflow.com/questions/42878216/opengl-how-to-draw-to-a-multisample-framebuffer-and-then-use-the-result-as-a-n
        - RDR2 - https://imgeself.github.io/posts/2020-06-19-graphics-study-rdr2/
        - DOOM - https://www.adriancourreges.com/blog/2016/09/09/doom-2016-graphics-study/
        - Godot - https://godotengine.org/article/godot-3-renderer-design-explained/
        - Frustrum culling https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/frustum-culling-r4613/
        - GTA5 - https://www.adriancourreges.com/blog/2015/11/02/gta-v-graphics-study/
        - Far Objects - https://www.reddit.com/r/opengl/comments/8z5egn/rendering_large_and_distant_object/?utm_source=share&utm_medium=web2x&context=3
        - GTA6 - https://vk.com/video-120800128_456251871?ysclid=lkfo8jp8lg311066769
        - Meshes optim, Greedy Meshing - https://codereview.stackexchange.com/questions/62562/voxel-world-optimization
        - Top dude - https://fgiesen.wordpress.com/
*/

class DebugRenderer 
{
public:
    DebugRenderer();

    void setupSceneGrid();

    void renderDebugColider(Window* wind, std::optional<Colider>& collider, std::optional<RigidBody>& body);

    void renderDebugCube(glm::vec3 pos, float x=1, float y=1, float z=1);

    void renderDebugPoint(glm::vec3 a, glm::vec4 color);

    void renderDebugLine(glm::vec3 a, glm::vec3 b, glm::vec4 color = glm::vec4(0,1,0,0));

    void renderDebugGrid();

    void renderDebugLightSource(std::optional<PointLight>& p);

    void updateCamera(glm::mat4 projection, glm::mat4 view);

    void renderPoints();

    void renderAABB();

    void clearPoints();

    struct PointToRender{
        glm::vec3 point;
        glm::vec4 color = {0,1,0,0};

        PointToRender(glm::vec3 pos, glm::vec4 col = glm::vec4(0,1,0,0)) : point(pos), color(col)
        {
    
        }
    };

    std::vector<PointToRender> pointsToRender;
    std::vector<MeshAABB> aabbToRender;

    bool debug_render = true;// Note(darius) when false - we dont draw debug info

private:
    //CubeMap cubemap;
    size_t length = 0;
    size_t slices = 200;
	std::vector<glm::vec3> vertices_grid;
	std::vector<glm::uvec4> indices_grid;
    float grid_scale = 0.1f;
    int grid_mode = 3;

    //TODO(darius) use cubeMesh
    float vertices[48 * 6] = {
		-0.5f, -0.5f, -0.5f, 
		 0.5f, -0.5f, -0.5f, 
		 0.5f,  0.5f, -0.5f, 
		 0.5f,  0.5f, -0.5f, 
		-0.5f,  0.5f, -0.5f, 
		-0.5f, -0.5f, -0.5f, 

		-0.5f, -0.5f,  0.5f, 
		 0.5f, -0.5f,  0.5f, 
		 0.5f,  0.5f,  0.5f, 
		 0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f,  0.5f, 
		-0.5f, -0.5f,  0.5f, 

		-0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f, -0.5f, 
		-0.5f, -0.5f, -0.5f, 
		-0.5f, -0.5f, -0.5f, 
		-0.5f, -0.5f,  0.5f, 
		-0.5f,  0.5f,  0.5f, 

		 0.5f,  0.5f,  0.5f, 
		 0.5f,  0.5f, -0.5f, 
		 0.5f, -0.5f, -0.5f, 
		 0.5f, -0.5f, -0.5f, 
		 0.5f, -0.5f,  0.5f, 
		 0.5f,  0.5f,  0.5f, 

		-0.5f, -0.5f, -0.5f, 
		 0.5f, -0.5f, -0.5f, 
		 0.5f, -0.5f,  0.5f, 
		 0.5f, -0.5f,  0.5f, 
		-0.5f, -0.5f,  0.5f, 
		-0.5f, -0.5f, -0.5f, 

		-0.5f,  0.5f, -0.5f, 
		 0.5f,  0.5f, -0.5f, 
		 0.5f,  0.5f,  0.5f, 
		 0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f, -0.5f, 
    };

    unsigned int indices[6] = {
        0, 1, 3,
        1, 2, 3
    };

    VBO vbo;

    VBO lightVbo;

    VAO vao;

    VAO vao_grid;

    EBO ebo;

    Shader dsv;
    Shader dsf;

    FlatMesh flat;
};

class RenderBuffer 
{
public:
    RenderBuffer() = default;
    
    RenderBuffer(unsigned int W, unsigned int H) 
    {
        glGenRenderbuffers(1, &ID);
        glBindRenderbuffer(GL_RENDERBUFFER, ID);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, W, H);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ID);
    }

private:
    unsigned int ID = 0;
};

class Renderer
{
public:
    Renderer() = default;

    Renderer(Scene* currScene_in, GameState* instance, Window* wind);

    void render(Window* wind);

	void updateBuffers(Window* wind);

    size_t getShaderRoutine();

    Shader getShader();

    Scene* getScene();

    DebugRenderer& getDebugRenderer();

    void bloomStage();

    void EditorIDsStage();

    void bokeStage();

    void depthStage();

    void albedoStage();
    
    void deferredStage();

    void resizeFrameBuffers(int W, int H);

    void resizeWindow(int W, int H);

public:
    glm::vec3 backgroundColor = glm::vec3{0.1f, 0.0f, 0.1f};

    //TODO(darius) to much buffers. Refactor it
    //DANGER(darius) looks like its hard to process
    FrameBuffer framebuffer;
    FrameBuffer depthTexture;
    FrameBuffer depthFramebuffer;
    FrameBuffer intermidiateFramebuffer;
    FrameBuffer bloomBuffer;
    FrameBuffer pingPongBlurBufferA;
    FrameBuffer pingPongBlurBufferB;
    RenderBuffer renderBuffer;
	FrameBuffer bufferCombination;
    FrameBuffer bokeBuffer;
    FrameBuffer deferredLightingBuffer;

    static ShaderLibrary* shaderLibInstance;

    static Renderer* currentRendererPtr;

    static int drawCallsCount;

    static int drawCallsInstancedCount;

    GraphicsStateCache state;

private:
    void renderScene();
    void renderAll(Window* wind);

private:
    DebugRenderer dbr;
    RendererQuad quad;
    
    Window* wind = nullptr;

    Scene* currScene;

    //TODO(darius) make it lighting system
    float lastFrame = 0;

    Model m;
};

