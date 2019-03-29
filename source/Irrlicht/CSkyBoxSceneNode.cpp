// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CSkyBoxSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "ICameraSceneNode.h"
#include "IGPUProgrammingServices.h"

#include "os.h"

const char *vS = R"(
#version 430 core
layout(location = 0) in vec4 pos;

uniform mat4 MVP;

out vec4 tCoords;

void main(){
    gl_Position = MVP * pos;
    tCoords = pos;
}

)";

const char *fS = R"(
#version 430 core

layout(location = 0) out vec4 color;

layout(location = 0) uniform samplerCube cubeMap;

in vec4 tCoords;

void main(){
    vec4 c = texture(cubeMap, tCoords.xyz);
    c.w = 1.0;
    color = c;
}

)";


namespace irr
{



namespace scene
{

//! constructor
CSkyBoxSceneNode::CSkyBoxSceneNode(video::ITexture *cubemap,
			video::IGPUBuffer* vertPositions, size_t positionsOffsetInBuf,
			IDummyTransformationSceneNode* parent, ISceneManager* mgr, int32_t id)
: ISceneNode(parent, mgr, id)
{
	#ifdef _DEBUG
	setDebugName("CSkyBoxSceneNode");
	#endif

	setAutomaticCulling(scene::EAC_OFF);
	Box.MaxEdge.set(0,0,0);
	Box.MinEdge.set(0,0,0);


	// create material
    mat.BackfaceCulling = false;
	mat.ZBuffer = video::ECFN_NEVER;
	mat.ZWriteEnable = false;
	mat.TextureLayer[0].SamplingParams.TextureWrapU = video::ETC_CLAMP_TO_EDGE;
	mat.TextureLayer[0].SamplingParams.TextureWrapV = video::ETC_CLAMP_TO_EDGE;

	video::IVideoDriver* driver = SceneManager->getVideoDriver();
    
    SkyboxMVPCallback *callback = new SkyboxMVPCallback;
    mat.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterial(vS, nullptr, nullptr, nullptr, fS, 3, video::EMT_SOLID, callback, 0);
    callback->drop();

   
	mat.setTexture(0, cubemap);
    buffer = createBuffer();

    
}

CSkyBoxSceneNode::CSkyBoxSceneNode(CSkyBoxSceneNode* other,
			IDummyTransformationSceneNode* parent, ISceneManager* mgr, int32_t id)
: ISceneNode(parent, mgr, id)
{
	#ifdef _DEBUG
	setDebugName("CSkyBoxSceneNode");
	#endif

	setAutomaticCulling(scene::EAC_OFF);
	Box.MaxEdge.set(0,0,0);
	Box.MinEdge.set(0,0,0);


    other->buffer->grab();
    buffer = other->buffer;
    mat = other->mat;
}


//! renders the node.
void CSkyBoxSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	scene::ICameraSceneNode* camera = SceneManager->getActiveCamera();

	if (!camera || !driver || !canProceedPastFence())
		return;

    core::matrix4x3 translate(AbsoluteTransformation);
    translate.setTranslation(camera->getAbsolutePosition());

    // Draw the sky box between the near and far clip plane
    const float viewDistance = (camera->getNearValue() + camera->getFarValue()) * 0.5f;
    core::matrix4x3 scale;
    scale.setScale(core::vector3df(viewDistance, viewDistance, viewDistance));

    driver->setTransform(video::E4X3TS_WORLD, concatenateBFollowedByA(translate, scale));
    driver->setMaterial(mat);
    driver->drawMeshBuffer(buffer);
}



//! returns the axis aligned bounding box of this node
const core::aabbox3d<float>& CSkyBoxSceneNode::getBoundingBox()
{
	return Box;
}


void CSkyBoxSceneNode::OnRegisterSceneNode()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this, ESNRP_SKY_BOX);

	ISceneNode::OnRegisterSceneNode();
}


//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
video::SGPUMaterial& CSkyBoxSceneNode::getMaterial()
{
    return mat;
}


//! returns amount of materials used by this scene node.
uint32_t CSkyBoxSceneNode::getMaterialCount() const
{
    return 1;
}


//! Creates a clone of this scene node and its children.
ISceneNode* CSkyBoxSceneNode::clone(IDummyTransformationSceneNode* newParent, ISceneManager* newManager)
{
	if (!newParent) newParent = Parent;
	if (!newManager) newManager = SceneManager;

	CSkyBoxSceneNode* nb = new CSkyBoxSceneNode(this, newParent,
		newManager, ID);

	nb->cloneMembers(this, newManager);

	if ( newParent )
		nb->drop();
	return nb;
}

video::IGPUMeshBuffer* CSkyBoxSceneNode::createBuffer() 
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();
    float vertexData[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
    };

    auto upBuf = driver->getDefaultUpStreamingBuffer();
    const void *dTP[] = { vertexData };
    uint32_t offsets[] = { video::StreamingTransientDataBufferMT<>::invalid_address };
    uint32_t alignments[] = { sizeof(decltype(vertexData[0u])) };
    uint32_t sizes[] = { sizeof(vertexData) };

    upBuf->multi_place(1u, (const void* const*)dTP, (uint32_t*)offsets, (uint32_t*)sizes, (uint32_t*)alignments);

    auto buf = upBuf->getBuffer();
    video::IGPUMeshDataFormatDesc *desc = driver->createGPUMeshDataFormatDesc();
    desc->setVertexAttrBuffer(buf, asset::EVAI_ATTR0, asset::EF_R32G32B32_SFLOAT, sizeof(float) * 3, offsets[0]);
    video::IGPUMeshBuffer *mb = new video::IGPUMeshBuffer;
    mb->setMeshDataAndFormat(desc);
    mb->setIndexType(asset::EIT_UNKNOWN);
    mb->setIndexCount(36);
    desc->drop();
    mb->grab();
    return mb;
}

} // end namespace scene
} // end namespace irr

