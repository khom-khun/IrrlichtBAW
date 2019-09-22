#include "CDebugRender.h"
#include "MeshConverter.h"
#include "BulletUtility.h"

using namespace irr;
using namespace ext;
using namespace Bullet3;

template<class Type>
void writeWith(btTriangleMesh *mesh, asset::ICPUMeshBuffer *data) {
	Type *indexData = reinterpret_cast<Type*>(data->getIndices());
	for (uint64_t i = 0; i < data->getIndexCount(); i += 3) {
		mesh->addTriangle(
			tobtVec3(data->getPosition(indexData[i + 0])),
			tobtVec3(data->getPosition(indexData[i + 1])),
			tobtVec3(data->getPosition(indexData[i + 2]))
		);
	}
}

btTriangleMesh *Bullet3::convertToNaiveTrimesh(asset::ICPUMeshBuffer *data) {
	_IRR_DEBUG_BREAK_IF(data->getIndexType() == asset::EIT_UNKNOWN);
	btTriangleMesh *mesh = createType<btTriangleMesh>();

	switch(data->getIndexType()) {
	case asset::EIT_16BIT:
		writeWith<uint16_t>(mesh, data);
		break;
	case asset::EIT_32BIT:
		writeWith<uint32_t>(mesh, data);
		break;
	}
	return mesh;
}