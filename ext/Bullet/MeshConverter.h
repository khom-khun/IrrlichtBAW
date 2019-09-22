#ifndef _IRR_EXT_BULLET_MESH_CONVERTER_INCLUDED_
#define _IRR_EXT_BULLET_MESH_CONVERTER_INCLUDED_

#include "irrlicht.h"
#include "irr/core/IReferenceCounted.h"
#include "btBulletDynamicsCommon.h"

namespace irr
{
namespace ext
{
namespace Bullet3
{
	btTriangleMesh *convertToNaiveTrimesh(asset::ICPUMeshBuffer *data);
}
}
}

#endif