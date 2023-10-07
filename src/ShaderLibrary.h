#pragma once

#include <Shader.h>
#include <GraphicsStateCache.h>

struct Transform;
class Object;

class ShaderLibrary
{
public:
    enum class STAGE
    {
        DEPTH,
        EDITOR_ID,
        ALBEDO,
        SHADOWS,
        BOKE,
        BLOOM,
        DEFERRED,
        DEFERRED_LIGHT,
        PARTICLES,
        TERRAIN,
        SKELETAL,
    };

public:
    ShaderLibrary();

    Shader& getCurrShader();

    Shader& getDepthShader();

    Shader& getAlbedoShader();

    Shader& getShadowsShader();

    Shader& getBlurShader();

    Shader& getTextureCombinerShader();

	Shader& getEditorIdShader();

	Shader& getBokeShader();

	Shader& getGBufferShader();

    Shader& getParticlesShader();

    Shader& getTerrainShader();

    Shader& getSkeletalShader();

    void loadCurrentShader();
    
    void shaderRoutine(Object* obj);

    void skeletalAnimationShaderRoutine(Transform tr);

    void checkForShaderReload(); 

    STAGE stage;

    unsigned int depthMap = 0;
private:

    Shader lightingVertex;
    Shader lightingFragment;

    Shader depthVertex;
    Shader depthFragment;

    Shader shadowVertex;
    Shader shadowFragment;

    Shader blurVertex;
    Shader blurFragment;

    Shader textureCombinerVertex;
    Shader textureCombinerFragment;

    Shader editorIdVertex;
    Shader editorIdFragment;

    Shader bokeVertex;
    Shader bokeFragment;

    Shader gBufferVertex;
    Shader gBufferFragment;

    Shader deferredLightPasVertex;
    Shader deferredLightPasFragment;

    Shader particlesVertex;
    Shader particlesFragment;

    Shader terrainVertex;
    Shader terrainFragment;

    Shader skeletalAnimationVertex;
    Shader skeletalAnimationFragment;

    GraphicsStateCache cache;
};

