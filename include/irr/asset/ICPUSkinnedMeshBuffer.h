// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_SKIN_MESH_BUFFER_H_INCLUDED__
#define __I_SKIN_MESH_BUFFER_H_INCLUDED__

#include "irr/asset/ICPUMeshBuffer.h"
#include "irr/asset/bawformat/blobs/SkinnedMeshBufferBlob.h"


namespace irr
{
namespace asset
{


class ICPUSkinnedMeshBuffer final : public ICPUMeshBuffer
{
        uint32_t indexValMin;
        uint32_t indexValMax;
        uint32_t maxVertexBoneInfluences;
    public:
        //! Default constructor
        ICPUSkinnedMeshBuffer() : indexValMin(0), indexValMax(0), maxVertexBoneInfluences(1)
        {
            #ifdef _IRR_DEBUG
            setDebugName("ICPUSkinnedMeshBuffer");
            #endif
        }

		inline void* serializeToBlob(void* _stackPtr = NULL, const size_t& _stackSize = 0) const override
		{
			return asset::CorrespondingBlobTypeFor<ICPUSkinnedMeshBuffer>::type::createAndTryOnStack(this, _stackPtr, _stackSize);
		}

        virtual asset::E_MESH_BUFFER_TYPE getMeshBufferType() const override { return asset::EMBT_ANIMATED_SKINNED; }

        inline void setIndexRange(const uint32_t &minBeforeBaseVxAdd, const uint32_t &maxBeforeBaseVxAdd)
        {
            indexValMin = minBeforeBaseVxAdd;
            indexValMax = maxBeforeBaseVxAdd;
        }

        inline const uint32_t& getIndexMinBound() const {return indexValMin;}
        inline const uint32_t& getIndexMaxBound() const {return indexValMax;}


        inline void setMaxVertexBoneInfluences(const uint32_t& val) {maxVertexBoneInfluences = val;}
        inline const uint32_t& getMaxVertexBoneInfluences() const {return maxVertexBoneInfluences;}
};


} // end namespace asset
} // end namespace irr

#endif

