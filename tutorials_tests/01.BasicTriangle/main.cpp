#define _IRR_STATIC_LIB_
#include <irrlicht.h>
#include "../source/Irrlicht/COpenGLExtensionHandler.h"
#include "../Shared/Utility.h"


const char *vS = R"(
#version 430 core
layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;   

out vec4 vColor;   
        
void main(){
    gl_Position = pos;
    vColor = color;
}
    )";

const char *fS = R"(
#version 330 core

in vec4 vColor;

layout(location = 0) out vec4 color;

void main(){
    color = vColor;
}
)";



using namespace irr;


#include "irr/irrunpack.h"
struct VertexLayout{
    float pos[3];
    float color[3];
} PACK_STRUCT;
#include "irr/irrpack.h"


video::IGPUMeshBuffer *createTestQuad(video::IVideoDriver *driver) {
    VertexLayout vertexData[4];
    vertexData[0] = { 0.5f, 0.5f,  0.0f, 1.0f, 0.0f, 0.0f };
    vertexData[1] = {  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f };
    vertexData[2] = {  -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f };
    vertexData[3] = { -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f };

    uint16_t indexData[] = { 0,1,2, 0,2,3 };

    auto upBuf = driver->getDefaultUpStreamingBuffer();
    const void *dataToPlace[] = { vertexData, indexData };
    uint32_t offsets[2] = { video::StreamingTransientDataBufferMT<>::invalid_address, 
                            video::StreamingTransientDataBufferMT<>::invalid_address };
    uint32_t alignments[2] = { sizeof(decltype(vertexData[0u])), 
                              sizeof(decltype(indexData[0u]))};
    uint32_t sizes[2] = { sizeof(vertexData), sizeof(indexData) };

    upBuf->multi_place(2u, (const void* const*)dataToPlace, (uint32_t*)offsets, (uint32_t*)sizes, (uint32_t*)alignments);
    if (upBuf->needsManualFlushOrInvalidate())
    {
        auto upStreamMem = upBuf->getBuffer()->getBoundMemory();
        driver->flushMappedMemoryRanges({ video::IDriverMemoryAllocation::MappedMemoryRange(upStreamMem,offsets[0],sizes[0]),video::IDriverMemoryAllocation::MappedMemoryRange(upStreamMem,offsets[1],sizes[1]) });
    }


    auto buf = upBuf->getBuffer();
    video::IGPUMeshDataFormatDesc* desc = driver->createGPUMeshDataFormatDesc();
    desc->setVertexAttrBuffer(buf, asset::EVAI_ATTR0, asset::EF_R32G32B32_SFLOAT, sizeof(VertexLayout), offsetof(VertexLayout, pos[0]) + offsets[0]);
    desc->setVertexAttrBuffer(buf, asset::EVAI_ATTR1, asset::EF_R32G32B32_SFLOAT, sizeof(VertexLayout), offsetof(VertexLayout, color[0]) + offsets[0]);
    desc->setIndexBuffer(buf);

    video::IGPUMeshBuffer *mb = new video::IGPUMeshBuffer();
    
    mb->setMeshDataAndFormat(desc);
    mb->setIndexBufferOffset(offsets[1]);
    mb->setIndexType(asset::EIT_16BIT);
    mb->setIndexCount(6);
    desc->drop();

    return mb;
}

int main(){
	// create device with full flexibility over creation parameters
	// you can add more parameters if desired, check irr::SIrrlichtCreationParameters
	irr::SIrrlichtCreationParameters params;
	params.Bits = 24; //may have to set to 32bit for some platforms
	params.ZBufferBits = 24; //we'd like 32bit here
	params.DriverType = video::EDT_OPENGL; //! Only Well functioning driver, software renderer left for sake of 2D image drawing
	params.WindowSize = core::dimension2d<uint32_t>(1280, 720);
	params.Fullscreen = false;
	params.Vsync = true; //! If supported by target platform
	params.Doublebuffer = true;
	params.Stencilbuffer = false; //! This will not even be a choice soon
	params.AuxGLContexts = 16;
	IrrlichtDevice* device = createDeviceEx(params);



	if (device == 0)
		return 1; // could not create selected driver.
    
    BaseReceiver eventMgr;
    device->setEventReceiver(&eventMgr);
   

	video::IVideoDriver* driver = device->getVideoDriver();


    video::SGPUMaterial material;
    material.MaterialType = (video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterial(
        vS, nullptr, nullptr, nullptr, fS, 3, video::EMT_SOLID, 0, 0);
 

    material.BackfaceCulling = false;

    video::IGPUMeshBuffer *mb = createTestQuad(driver);


	scene::ISceneManager* smgr = device->getSceneManager();

	uint64_t lastFPSTime = 0;

	while(device->run())
	if (device->isWindowActive()){
		driver->beginScene(true, true, video::SColor(0,0,0,0) ); //this gets 11k FPS

        driver->setTransform(video::E4X3TS_WORLD, core::matrix4x3());
        driver->setMaterial(material);
        driver->drawMeshBuffer(mb);

		driver->endScene();

		// display frames per second in window title
		uint64_t time = device->getTimer()->getRealTime();
		if (time-lastFPSTime > 1000){
		    std::wostringstream str(L"Hello World - Irrlicht Engine [");
		    str.seekp(0,std::ios_base::end);
            str << driver->getName() << "] FPS:" << driver->getFPS() << " PrimitivesDrawn: " << driver->getPrimitiveCountDrawn();
            
			device->setWindowCaption(str.str());
			lastFPSTime = time;
		}
	}

    writeScreenshot(device, params.WindowSize);

	return 0;
}
