/* irrlicht.h -- interface of the 'IrrlichtBAW Engine'

  Copyright (C) 2019 - DevSH Graphics Programming Sp. z O.O.

  This software is provided 'as-is', under the Apache 2.0 license,
  without any express or implied warranty.  In no event will the authors
  be held liable for any damages arising from the use of this software.

  See LICENSE.md for full licensing information.

  Please note that the IrrlichtBAW Engine is based in part on the work of others,
  this means that if you use the IrrlichtBAW Engine in your product,
  you must acknowledge somewhere in your documentation that you've used their code.
  See README.md for all mentions of 3rd party software used.
*/

#ifndef __IRRLICHT_H_INCLUDED__
#define __IRRLICHT_H_INCLUDED__

// core lib
#include "irr/core/core.h"

// system lib (fibers, mutexes, file I/O operations) [DEPENDS: core]
#include "irr/system/system.h"
// should we move "core/parallel" to "system/parallel"

// asset lib (importing and exporting meshes, textures and shaders) [DEPENDS: system]
#include "irr/asset/asset.h"
// ui lib (window set up, software blit, joysticks, multi-touch, keyboard, etc.) [DEPENDS: system]
#include "irr/ui/ui.h"

// video lib (access to Graphics API, remote rendering, etc) [DEPENDS: asset, (optional) ui]
#include "irr/video/video.h"

// scene lib (basic rendering, culling, scene graph etc.) [DEPENDS: video, ui]
#include "irr/scene/scene.h"


#include "IrrCompileConfig.h"
#include "aabbox3d.h"
#include "vector2d.h"
#include "vector3d.h"
#include "vectorSIMD.h"
#include "line2d.h"
#include "line3d.h"
#include "matrix4SIMD.h"
#include "position2d.h"
#include "quaternion.h"
#include "rect.h"
#include "dimension2d.h"
#include "ECullingTypes.h"
#include "EDebugSceneTypes.h"
#include "EDriverTypes.h"
#include "EMaterialTypes.h"
#include "ESceneNodeAnimatorTypes.h"
#include "ESceneNodeTypes.h"
#include "IAnimatedMesh.h"
#include "IAnimatedMeshSceneNode.h"
#include "IBillboardSceneNode.h"
#include "ICameraSceneNode.h"
#include "ICursorControl.h"
#include "IDummyTransformationSceneNode.h"
#include "IEventReceiver.h"
#include "IFileList.h"
#include "IFileSystem.h"
#include "IGPUProgrammingServices.h"
#include "ILogger.h"
#include "IMaterialRenderer.h"
#include "IMaterialRendererServices.h"
#include "IMeshSceneNode.h"
#include "IMeshSceneNodeInstanced.h"
#include "IOSOperator.h"
#include "IReadFile.h"
#include "IrrlichtDevice.h"
#include "path.h"
#include "ISceneManager.h"
#include "ISceneNode.h"
#include "ISceneNodeAnimator.h"
#include "ISceneNodeAnimatorCameraFPS.h"
#include "ISceneNodeAnimatorCameraMaya.h"
#include "IShaderConstantSetCallBack.h"
#include "ISkinnedMeshSceneNode.h"
#include "ITexture.h"
#include "ITextureBufferObject.h"
#include "IMultisampleTexture.h"
#include "ITimer.h"
#include "ITransformFeedback.h"
#include "IVideoDriver.h"
#include "IVideoModeList.h"
#include "IWriteFile.h"
#include "Keycodes.h"
#include "splines.h"


#include "SColor.h"
#include "SCollisionEngine.h"
#include "SExposedVideoData.h"
#include "SIrrCreationParameters.h"
#include "SKeyMap.h"
#include "SMaterial.h"
#include "SViewFrustum.h"


#include "SIrrCreationParameters.h"

//! Everything in the Irrlicht Engine can be found in this namespace.
namespace irr
{
	//! Creates an Irrlicht device. The Irrlicht device is the root object for using the engine.
	/** If you need more parameters to be passed to the creation of the Irrlicht Engine device,
	use the createDeviceEx() function.
	\param deviceType: Type of the device. This can currently be video::EDT_NULL,
	video::EDT_VULKAN, and video::EDT_OPENGL.
	\param windowSize: Size of the window or the video mode in fullscreen mode.
	\param bits: Bits per pixel in fullscreen mode. Ignored if windowed mode.
	\param fullscreen: Should be set to true if the device should run in fullscreen. Otherwise
		the device runs in windowed mode.
	\param stencilbuffer: Specifies if the stencil buffer should be enabled. Set this to true,
	if you want the engine be able to draw stencil buffer shadows. Note that not all
	devices are able to use the stencil buffer. If they don't no shadows will be drawn.
	\param vsync: Specifies vertical syncronisation: If set to true, the driver will wait
	for the vertical retrace period, otherwise not.
	\param receiver: A user created event receiver.
	\return Returns pointer to the created IrrlichtDevice or null if the
	device could not be created.
	*/
	extern "C" IRRLICHT_API IrrlichtDevice* IRRCALLCONV createDevice(
		video::E_DRIVER_TYPE deviceType = video::EDT_OPENGL,
		// parantheses are necessary for some compilers
		const core::dimension2d<uint32_t>& windowSize = (core::dimension2d<uint32_t>(640,480)),
		uint32_t bits = 16,
		bool fullscreen = false,
		bool stencilbuffer = false,
		bool vsync = false,
		IEventReceiver* receiver = 0);

	//! typedef for Function Pointer
	typedef IrrlichtDevice* (IRRCALLCONV *funcptr_createDevice )(
			video::E_DRIVER_TYPE deviceType,
			const core::dimension2d<uint32_t>& windowSize,
			uint32_t bits,
			bool fullscreen,
			bool stencilbuffer,
			bool vsync,
			IEventReceiver* receiver);


	//! Creates an Irrlicht device with the option to specify advanced parameters.
	/** Usually you should used createDevice() for creating an Irrlicht Engine device.
	Use this function only if you wish to specify advanced parameters like a window
	handle in which the device should be created.
	\param parameters: Structure containing advanced parameters for the creation of the device.
	See irr::SIrrlichtCreationParameters for details.
	\return Returns pointer to the created IrrlichtDevice or null if the
	device could not be created. */
	extern "C" IRRLICHT_API IrrlichtDevice* IRRCALLCONV createDeviceEx(
		const SIrrlichtCreationParameters& parameters);

	//! typedef for Function Pointer
	typedef IrrlichtDevice* (IRRCALLCONV *funcptr_createDeviceEx )( const SIrrlichtCreationParameters& parameters );


	// THE FOLLOWING IS AN EMPTY LIST OF ALL SUB NAMESPACES
	// EXISTING ONLY FOR THE DOCUMENTATION SOFTWARE DOXYGEN.

	//! Basic classes such as vectors, planes, arrays, lists, and so on can be found in this namespace.
	namespace core
	{
	}

	//! This namespace provides interfaces for input/output: Reading and writing files, accessing zip archives, xml files, ...
	namespace io
	{
	}

	//! All scene management can be found in this namespace: Mesh loading, special scene nodes like octrees and billboards, ...
	namespace scene
	{
	}

	//! The video namespace contains classes for accessing the video driver. All 2d and 3d rendering is done here.
	namespace video
	{
	}
}

/*! \file irrlicht.h
	\brief Main header file of the irrlicht, the only file needed to include.
*/

#endif

