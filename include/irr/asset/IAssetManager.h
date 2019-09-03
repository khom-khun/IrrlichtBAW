// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_I_ASSET_MANAGER_H_INCLUDED__
#define __IRR_I_ASSET_MANAGER_H_INCLUDED__

#include <array>
#include <ostream>

#include "irr/core/Types.h"
#include "CConcurrentObjectCache.h"

#include "IFileSystem.h"
#include "IReadFile.h"
#include "IWriteFile.h"

#include "irr/asset/IGeometryCreator.h"
#include "irr/asset/IMeshManipulator.h"
#include "irr/asset/IAssetLoader.h"
#include "irr/asset/IAssetWriter.h"

#define USE_MAPS_FOR_PATH_BASED_CACHE //benchmark and choose, paths can be full system paths

namespace irr
{
namespace asset
{
    class IAssetManager;

    std::function<void(SAssetBundle&)> makeAssetGreetFunc(const IAssetManager* const _mgr);
    std::function<void(SAssetBundle&)> makeAssetDisposeFunc(const IAssetManager* const _mgr);

	class IAssetManager : public core::IReferenceCounted
	{
        // the point of those functions is that lambdas returned by them "inherits" friendship
        friend std::function<void(SAssetBundle&)> makeAssetGreetFunc(const IAssetManager* const _mgr);
        friend std::function<void(SAssetBundle&)> makeAssetDisposeFunc(const IAssetManager* const _mgr);

    public:
#ifdef USE_MAPS_FOR_PATH_BASED_CACHE
        using AssetCacheType = core::CConcurrentMultiObjectCache<std::string, SAssetBundle, std::multimap>;
#else
        using AssetCacheType = core::CConcurrentMultiObjectCache<std::string, IAssetBundle, std::vector>;
#endif //USE_MAPS_FOR_PATH_BASED_CACHE

        using CpuGpuCacheType = core::CConcurrentObjectCache<const IAsset*, core::smart_refctd_ptr<core::IReferenceCounted> >;

    private:
        struct WriterKey
        {
            IAsset::E_TYPE first;
            std::string second;

            inline bool operator<(const WriterKey& _rhs) const
            {
                if (first != _rhs.first)
                    return first < _rhs.first;
                return second < _rhs.second;
            }

            inline friend std::ostream& operator<<(std::ostream& _outs, const WriterKey& _item)
            {
                return _outs << "{ " << static_cast<uint64_t>(_item.first) << ", " << _item.second << " }";
            }
        };

        template<typename T>
        static void refCtdGreet(T* _asset) { _asset->grab(); }
        template<typename T>
        static void refCtdDispose(T* _asset) { _asset->drop(); }

        core::smart_refctd_ptr<io::IFileSystem> m_fileSystem;
        IAssetLoader::IAssetLoaderOverride m_defaultLoaderOverride;

        std::array<AssetCacheType*, IAsset::ET_STANDARD_TYPES_COUNT> m_assetCache;
        std::array<CpuGpuCacheType*, IAsset::ET_STANDARD_TYPES_COUNT> m_cpuGpuCache;

        struct Loaders {
            Loaders() : perFileExt{&refCtdGreet<IAssetLoader>, &refCtdDispose<IAssetLoader>} {}

            core::vector<core::smart_refctd_ptr<IAssetLoader> > vector;
            //! The key is file extension
            core::CMultiObjectCache<std::string, IAssetLoader*, std::vector> perFileExt;

            void pushToVector(core::smart_refctd_ptr<IAssetLoader>&& _loader)
			{
                vector.push_back(std::move(_loader));
            }
            void eraseFromVector(decltype(vector)::const_iterator _loaderItr)
			{
                vector.erase(_loaderItr);
            }
        } m_loaders;

        struct Writers {
            Writers() : perTypeAndFileExt{&refCtdGreet<IAssetWriter>, &refCtdDispose<IAssetWriter>}, perType{&refCtdGreet<IAssetWriter>, &refCtdDispose<IAssetWriter>} {}

            core::CMultiObjectCache<WriterKey, IAssetWriter*, std::vector> perTypeAndFileExt;
            core::CMultiObjectCache<IAsset::E_TYPE, IAssetWriter*, std::vector> perType;
        } m_writers;

        friend class IAssetLoader;
        friend class IAssetLoader::IAssetLoaderOverride; // for access to non-const findAssets

        core::smart_refctd_ptr<IGeometryCreator> m_geometryCreator;
        core::smart_refctd_ptr<IMeshManipulator> m_meshManipulator;
        // called as a part of constructor only
        void initializeMeshTools();

    public:
        //! Constructor
        explicit IAssetManager(core::smart_refctd_ptr<io::IFileSystem>&& _fs) :
            m_fileSystem(std::move(_fs)),
            m_defaultLoaderOverride{nullptr}
        {
            initializeMeshTools();

            for (size_t i = 0u; i < m_assetCache.size(); ++i)
                m_assetCache[i] = new AssetCacheType(asset::makeAssetGreetFunc(this), asset::makeAssetDisposeFunc(this));
            for (size_t i = 0u; i < m_cpuGpuCache.size(); ++i)
                m_cpuGpuCache[i] = new CpuGpuCacheType();
            m_defaultLoaderOverride = IAssetLoader::IAssetLoaderOverride{this};

			addLoadersAndWriters();
        }

		inline io::IFileSystem* getFileSystem() const { return m_fileSystem.get(); }

        const IGeometryCreator* getGeometryCreator() const;
        const IMeshManipulator* getMeshManipulator() const;

    protected:
		virtual ~IAssetManager()
		{
			for (size_t i = 0u; i < m_assetCache.size(); ++i)
				if (m_assetCache[i])
					delete m_assetCache[i];

			core::vector<typename CpuGpuCacheType::MutablePairType> buf;
			for (size_t i = 0u; i < m_cpuGpuCache.size(); ++i)
			{
				if (m_cpuGpuCache[i])
				{
					size_t sizeToReserve{};
					m_cpuGpuCache[i]->outputAll(sizeToReserve, nullptr);
					buf.resize(sizeToReserve);
					m_cpuGpuCache[i]->outputAll(sizeToReserve, buf.data());
					for (auto& pair : buf)
						pair.first->drop(); // drop keys (CPU "empty cache handles")
					delete m_cpuGpuCache[i]; // drop on values (GPU objects) will be done by cache's destructor
				}
			}
		}

        //! _supposedFilename is filename as it was, not touched by loader override with _override->getLoadFilename()
        //TODO change name
        SAssetBundle getAssetInHierarchy(io::IReadFile* _file, const std::string& _supposedFilename, const IAssetLoader::SAssetLoadParams& _params, uint32_t _hierarchyLevel, IAssetLoader::IAssetLoaderOverride* _override)
        {
            IAssetLoader::SAssetLoadContext ctx{_params, _file};

            std::string filename = _file ? _file->getFileName().c_str() : _supposedFilename;
            io::IReadFile* file = _override->getLoadFile(_file, filename, ctx, _hierarchyLevel);
            filename = file ? file->getFileName().c_str() : _supposedFilename;

            const uint64_t levelFlags = _params.cacheFlags >> ((uint64_t)_hierarchyLevel * 2ull);

            SAssetBundle asset;
            if ((levelFlags & IAssetLoader::ECF_DUPLICATE_TOP_LEVEL) != IAssetLoader::ECF_DUPLICATE_TOP_LEVEL)
            {
                core::vector<SAssetBundle> found = findAssets(filename);
                if (found.size())
                    return _override->chooseRelevantFromFound(found, ctx, _hierarchyLevel);
                else if (!(asset = _override->handleSearchFail(filename, ctx, _hierarchyLevel)).isEmpty())
                    return asset;
            }

            // if at this point, and after looking for an asset in cache, file is still nullptr, then return nullptr
            if (!file)
                return {};//return empty bundle

            auto capableLoadersRng = m_loaders.perFileExt.findRange(getFileExt(filename.c_str()));

            for (auto loaderItr = capableLoadersRng.first; loaderItr != capableLoadersRng.second; ++loaderItr) // loaders associated with the file's extension tryout
            {
                if (loaderItr->second->isALoadableFileFormat(file) && !(asset = loaderItr->second->loadAsset(file, _params, _override, _hierarchyLevel)).isEmpty())
                    break;
            }
            for (auto loaderItr = std::begin(m_loaders.vector); asset.isEmpty() && loaderItr != std::end(m_loaders.vector); ++loaderItr) // all loaders tryout
            {
                if ((*loaderItr)->isALoadableFileFormat(file) && !(asset = (*loaderItr)->loadAsset(file, _params, _override, _hierarchyLevel)).isEmpty())
                    break;
            }

            if (!asset.isEmpty() && 
                ((levelFlags & IAssetLoader::ECF_DONT_CACHE_TOP_LEVEL) != IAssetLoader::ECF_DONT_CACHE_TOP_LEVEL) &&
                ((levelFlags & IAssetLoader::ECF_DUPLICATE_TOP_LEVEL) != IAssetLoader::ECF_DUPLICATE_TOP_LEVEL))
            {
                _override->insertAssetIntoCache(asset, filename, ctx, _hierarchyLevel);
            }
            else if (asset.isEmpty())
            {
                bool addToCache;
                asset = _override->handleLoadFail(addToCache, file, filename, filename, ctx, _hierarchyLevel);
                if (!asset.isEmpty() && addToCache)
                    _override->insertAssetIntoCache(asset, filename, ctx, _hierarchyLevel);
            }
            
            return asset;
        }
        //TODO change name
        SAssetBundle getAssetInHierarchy(const std::string& _filename, const IAssetLoader::SAssetLoadParams& _params, uint32_t _hierarchyLevel, IAssetLoader::IAssetLoaderOverride* _override)
        {
            IAssetLoader::SAssetLoadContext ctx{_params, nullptr};

            std::string filename = _filename;
            _override->getLoadFilename(filename, ctx, _hierarchyLevel);
            io::IReadFile* file = m_fileSystem->createAndOpenFile(filename.c_str());

            SAssetBundle asset = getAssetInHierarchy(file, _filename, _params, _hierarchyLevel, _override);

            if (file)
                file->drop();

            return asset;
        }

        //TODO change name
        SAssetBundle getAssetInHierarchy(io::IReadFile* _file, const std::string& _supposedFilename, const IAssetLoader::SAssetLoadParams& _params, uint32_t _hierarchyLevel)
        {
            return getAssetInHierarchy(_file, _supposedFilename, _params, _hierarchyLevel, &m_defaultLoaderOverride);
        }

        //TODO change name
        SAssetBundle getAssetInHierarchy(const std::string& _filename, const IAssetLoader::SAssetLoadParams& _params, uint32_t _hierarchyLevel)
        {
            return getAssetInHierarchy(_filename, _params, _hierarchyLevel, &m_defaultLoaderOverride);
        }

    public:
        //! These can be grabbed and dropped, but you must not use drop() to try to unload/release memory of a cached IAsset (which is cached if IAsset::isInAResourceCache() returns true). See IAsset::E_CACHING_FLAGS
        /** Instead for a cached asset you call IAsset::removeSelfFromCache() instead of IAsset::drop() as the cache has an internal grab of the IAsset and it will drop it on removal from cache, which will result in deletion if nothing else is holding onto the IAsset through grabs (in that sense the last drop will delete the object). */
        //TODO change name
        SAssetBundle getAsset(const std::string& _filename, const IAssetLoader::SAssetLoadParams& _params, IAssetLoader::IAssetLoaderOverride* _override)
        {
            return getAssetInHierarchy(_filename, _params, 0u, _override);
        }
        //TODO change name
        SAssetBundle getAsset(io::IReadFile* _file, const std::string& _supposedFilename, const IAssetLoader::SAssetLoadParams& _params, IAssetLoader::IAssetLoaderOverride* _override)
        {
            return getAssetInHierarchy(_file, _supposedFilename, _params,  0u, _override);
        }

        //TODO change name
        SAssetBundle getAsset(const std::string& _filename, const IAssetLoader::SAssetLoadParams& _params)
        {
            return getAsset(_filename, _params, &m_defaultLoaderOverride);
        }

        //TODO change name
        SAssetBundle getAsset(io::IReadFile* _file, const std::string& _supposedFilename, const IAssetLoader::SAssetLoadParams& _params)
        {
            return getAsset(_file, _supposedFilename, _params, &m_defaultLoaderOverride);
        }

        //TODO change name
        inline bool findAssets(size_t& _inOutStorageSize, SAssetBundle* _out, const std::string& _key, const IAsset::E_TYPE* _types = nullptr) const
        {
            size_t availableSize = _inOutStorageSize;
            _inOutStorageSize = 0u;
            bool res = true;
            if (_types)
            {
                uint32_t i = 0u;
                while ((_types[i] != (IAsset::E_TYPE)0u) && (availableSize > 0u))
                {
                    uint32_t typeIx = IAsset::typeFlagToIndex(_types[i]);
                    size_t readCnt = availableSize;
                    res = m_assetCache[typeIx]->findAndStoreRange(_key, readCnt, _out);
                    availableSize -= readCnt;
                    _inOutStorageSize += readCnt;
                    _out += readCnt;
                    ++i;
                }
            }
            else
            {
                for (uint32_t typeIx = 0u; typeIx < IAsset::ET_STANDARD_TYPES_COUNT; ++typeIx)
                {
                    size_t readCnt = availableSize;
                    res = m_assetCache[typeIx]->findAndStoreRange(_key, readCnt, _out);
                    availableSize -= readCnt;
                    _inOutStorageSize += readCnt;
                    _out += readCnt;
                }
            }
            return res;
        }
        //TODO change name (plural)
        inline core::vector<SAssetBundle> findAssets(const std::string& _key, const IAsset::E_TYPE* _types = nullptr) const
        {
            size_t reqSz = 0u;
            if (_types)
            {
                uint32_t i = 0u;
                while ((_types[i] != (IAsset::E_TYPE)0u))
                {
                    const uint32_t typeIx = IAsset::typeFlagToIndex(_types[i]);
                    reqSz += m_assetCache[typeIx]->getSize();
                    ++i;
                }
            }
            else
            {
                for (const auto& cache : m_assetCache)
                    reqSz += cache->getSize();
            }
            core::vector<SAssetBundle> res(reqSz);
            findAssets(reqSz, res.data(), _key, _types);
            res.resize(reqSz);
            return res;
        }

        inline void setAssetMetadata(IAsset* _asset, core::smart_refctd_ptr<IAssetMetadata>&& _metadata)
        {
            _asset->setMetadata(std::move(_metadata));
        }

        //! Changes the lookup key
        //TODO change name
        inline void changeAssetKey(SAssetBundle& _asset, const std::string& _newKey)
        {
            _asset.setNewCacheKey(_newKey);
            m_assetCache[IAsset::typeFlagToIndex(_asset.getAssetType())]->changeObjectKey(_asset, _asset.getCacheKey(), _newKey);
        }

        //! Insert an asset into the cache (calls the private methods of IAsset behind the scenes)
        /** \return boolean if was added into cache (no duplicate under same key found) and grab() was called on the asset. */
        //TODO change name
        bool insertAssetIntoCache(SAssetBundle& _asset)
        {
            const uint32_t ix = IAsset::typeFlagToIndex(_asset.getAssetType());
            return m_assetCache[ix]->insert(_asset.getCacheKey(), _asset);
        }

        //! Remove an asset from cache (calls the private methods of IAsset behind the scenes)
        //TODO change key
        bool removeAssetFromCache(SAssetBundle& _asset) //will actually look up by asset�s key instead
        {
            const uint32_t ix = IAsset::typeFlagToIndex(_asset.getAssetType());
            return m_assetCache[ix]->removeObject(_asset, _asset.getCacheKey());
        }

        //! Removes all assets from the specified caches, all caches by default
        void clearAllAssetCache(const uint64_t& _assetTypeBitFlags = 0xffffffffffffffffull)
        {
            for (size_t i = 0u; i < IAsset::ET_STANDARD_TYPES_COUNT; ++i)
                if ((_assetTypeBitFlags>>i) & 1ull)
                    m_assetCache[i]->clear();
        }

        //! This function frees most of the memory consumed by IAssets, but not destroying them.
        /** Keeping assets around (by their pointers) helps a lot by letting the loaders retrieve them from the cache and not load cpu objects which have been loaded, converted to gpu resources and then would have been disposed of. However each dummy object needs to have a GPU object associated with it in yet-another-cache for use when we convert CPU objects to GPU objects.*/
        void convertAssetToEmptyCacheHandle(IAsset* _asset, core::smart_refctd_ptr<core::IReferenceCounted>&& _gpuObject)
        {
			const uint32_t ix = IAsset::typeFlagToIndex(_asset->getAssetType());
            _asset->grab();
            _asset->convertToDummyObject();
            m_cpuGpuCache[ix]->insert(_asset, std::move(_gpuObject));
        }

		core::smart_refctd_ptr<core::IReferenceCounted> findGPUObject(const IAsset* _asset)
        {
			const uint32_t ix = IAsset::typeFlagToIndex(_asset->getAssetType());

            core::smart_refctd_ptr<core::IReferenceCounted> storage[1];
            size_t storageSz = 1u;
            m_cpuGpuCache[ix]->findAndStoreRange(_asset, storageSz, storage);
            if (storageSz > 0u)
                return std::move(storage[0]);
            return nullptr;
        }

		//! Removes one GPU object matched to an IAsset.
        bool removeCachedGPUObject(const IAsset* _asset, const core::smart_refctd_ptr<core::IReferenceCounted>& _gpuObject)
        {
			const uint32_t ix = IAsset::typeFlagToIndex(_asset->getAssetType());
			bool success = m_cpuGpuCache[ix]->removeObject(_gpuObject,_asset);
            if (success)
			    _asset->drop();
			return success;
        }

		// we need a removeCachedGPUObjects(const IAsset* _asset) but CObjectCache.h needs a `removeAllAssociatedObjects(const Key& _key)`

        //! Writing an asset
        /** Compression level is a number between 0 and 1 to signify how much storage we are trading for writing time or quality, this is a non-linear scale and has different meanings and results with different asset types and writers. */
        bool writeAsset(const std::string& _filename, const IAssetWriter::SAssetWriteParams& _params, IAssetWriter::IAssetWriterOverride* _override)
        {
            IAssetWriter::IAssetWriterOverride defOverride;
            if (!_override)
                _override = &defOverride;

            io::IWriteFile* file = m_fileSystem->createAndWriteFile(_filename.c_str());
			if (file) // could fail creating file (lack of permissions)
			{
				bool res = writeAsset(file, _params, _override);
				file->drop();
				return res;
			}
			else
				return false;
        }
        bool writeAsset(io::IWriteFile* _file, const IAssetWriter::SAssetWriteParams& _params, IAssetWriter::IAssetWriterOverride* _override)
        {
			if (!_file)
				return false;

            IAssetWriter::IAssetWriterOverride defOverride;
            if (!_override)
                _override = &defOverride;

            auto capableWritersRng = m_writers.perTypeAndFileExt.findRange({_params.rootAsset->getAssetType(), getFileExt(_file->getFileName())});

            for (auto it = capableWritersRng.first; it != capableWritersRng.second; ++it)
                if (it->second->writeAsset(_file, _params, _override))
                    return true;
            return false;
        }
        bool writeAsset(const std::string& _filename, const IAssetWriter::SAssetWriteParams& _params)
        {
            return writeAsset(_filename, _params, nullptr);
        }
        bool writeAsset(io::IWriteFile* _file, const IAssetWriter::SAssetWriteParams& _params)
        {
            return writeAsset(_file, _params, nullptr);
        }

        // Asset Loaders [FOLLOWING ARE NOT THREAD SAFE]
        uint32_t getAssetLoaderCount() { return m_loaders.vector.size(); }

        //! @returns 0xdeadbeefu on failure or 0-based index on success.
        uint32_t addAssetLoader(core::smart_refctd_ptr<IAssetLoader>&& _loader)
        {
            // there's no way it ever fails, so no 0xdeadbeef return
            const char** exts = _loader->getAssociatedFileExtensions();
            size_t extIx = 0u;
            while (const char* ext = exts[extIx++])
                m_loaders.perFileExt.insert(ext, _loader.get());
            m_loaders.pushToVector(std::move(_loader));
            return m_loaders.vector.size()-1u;
        }
        void removeAssetLoader(IAssetLoader* _loader)
        {
            m_loaders.eraseFromVector(
                std::find_if(std::begin(m_loaders.vector), std::end(m_loaders.vector), [_loader](const core::smart_refctd_ptr<IAssetLoader>& a)->bool { return a.get()==_loader; })
            );
            const char** exts = _loader->getAssociatedFileExtensions();
            size_t extIx = 0u;
            while (const char* ext = exts[extIx++])
                m_loaders.perFileExt.removeObject(_loader, ext);
        }

        // Asset Writers [FOLLOWING ARE NOT THREAD SAFE]
        uint32_t getAssetWriterCount() { return m_writers.perType.getSize(); } // todo.. well, it's not really writer count.. but rather type<->writer association count

        void addAssetWriter(core::smart_refctd_ptr<IAssetWriter>&& _writer)
        {
            const uint64_t suppTypes = _writer->getSupportedAssetTypesBitfield();
            const char** exts = _writer->getAssociatedFileExtensions();
            for (uint32_t i = 0u; i < IAsset::ET_STANDARD_TYPES_COUNT; ++i)
            {
                const IAsset::E_TYPE type = IAsset::E_TYPE(1u << i);
                if ((suppTypes>>i) & 1u)
                {
                    m_writers.perType.insert(type, _writer.get());
                    size_t extIx = 0u;
                    while (const char* ext = exts[extIx++])
                        m_writers.perTypeAndFileExt.insert({type, ext}, _writer.get());
                }
            }
        }
        void removeAssetWriter(IAssetWriter* _writer)
        {
            const uint64_t suppTypes = _writer->getSupportedAssetTypesBitfield();
            const char** exts = _writer->getAssociatedFileExtensions();
            size_t extIx = 0u;
            for (uint32_t i = 0u; i < IAsset::ET_STANDARD_TYPES_COUNT; ++i)
            {
                if ((suppTypes >> i) & 1u)
                {
                    const IAsset::E_TYPE type = IAsset::E_TYPE(1u << i);
                    m_writers.perType.removeObject(_writer, type);
                    while (const char* ext = exts[extIx++])
                        m_writers.perTypeAndFileExt.removeObject(_writer, {type, ext});
                }
            }
        }

        void dumpDebug(std::ostream& _outs) const
        {
            /*
            TODO rework these prints
            for (uint32_t i = 0u; i < IAsset::ET_STANDARD_TYPES_COUNT; ++i)
            {
                _outs << "Asset cache (asset type " << (1u << i) << "):\n";
                size_t sz = m_assetCache[i]->getSize();
                typename AssetCacheType::MutablePairType* storage = new typename AssetCacheType::MutablePairType[sz];
                m_assetCache[i]->outputAll(sz, storage);
                for (uint32_t j = 0u; j < sz; ++j)
                    _outs << "\tKey: " << storage[j].first << ", Value: " << static_cast<void*>(storage[j].second) << '\n';
                delete[] storage;
            }
            */
            _outs << "Loaders vector:\n";
            for (const auto& ldr : m_loaders.vector)
                _outs << '\t' << static_cast<void*>(ldr.get()) << '\n';
            _outs << "Loaders per-file-ext cache:\n";
            for (const auto& ldr : m_loaders.perFileExt)
                _outs << "\tKey: " << ldr.first << ", Value: " << ldr.second << '\n';
            _outs << "Writers per-asset-type cache:\n";
            for (const auto& wtr : m_writers.perType)
                _outs << "\tKey: " << static_cast<uint64_t>(wtr.first) << ", Value: " << static_cast<void*>(wtr.second) << '\n';
            _outs << "Writers per-asset-type-and-file-ext cache:\n";
            for (const auto& wtr : m_writers.perTypeAndFileExt)
                _outs << "\tKey: " << wtr.first << ", Value: " << static_cast<void*>(wtr.second) << '\n';
        }

    private:
        static inline std::string getFileExt(const io::path& _filename)
        {
            int32_t dot = _filename.findLast('.');
            return _filename
                .subString(dot+1, _filename.size()-dot-1)
                .make_lower()
                .c_str();
        }

        // for greet/dispose lambdas for asset caches so we don't have to make another friend decl.
        //TODO change name
        inline void setAssetCached(SAssetBundle& _asset, bool _val) const { _asset.setCached(_val); }

		//
		void addLoadersAndWriters();
	};
}
}

#endif
