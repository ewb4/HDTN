#ifndef _BUNDLE_STORAGE_MANAGER_ASIO_H
#define _BUNDLE_STORAGE_MANAGER_ASIO_H

#include "BundleStorageManagerBase.h"
#include <boost/asio.hpp>



class CLASS_VISIBILITY_STORAGE_LIB BundleStorageManagerAsio : public BundleStorageManagerBase {
public:
    STORAGE_LIB_EXPORT BundleStorageManagerAsio();
    STORAGE_LIB_EXPORT BundleStorageManagerAsio(const std::string & jsonConfigFileName);
    STORAGE_LIB_EXPORT BundleStorageManagerAsio(const StorageConfig_ptr & storageConfigPtr);
    STORAGE_LIB_EXPORT virtual ~BundleStorageManagerAsio();
    STORAGE_LIB_EXPORT virtual void Start();


private:
    STORAGE_LIB_NO_EXPORT void TryDiskOperation_Consume_NotThreadSafe(const unsigned int diskId);
    STORAGE_LIB_NO_EXPORT void HandleDiskOperationCompleted(const boost::system::error_code& error, std::size_t bytes_transferred,
        const unsigned int diskId, const unsigned int consumeIndex, const bool wasReadOperation);

    STORAGE_LIB_NO_EXPORT virtual void NotifyDiskOfWorkToDo_ThreadSafe(const unsigned int diskId);

private:

    boost::asio::io_service m_ioService;
    std::unique_ptr<boost::asio::io_service::work> m_workPtr;
    std::unique_ptr<boost::thread> m_ioServiceThreadPtr;
    
#ifdef _WIN32
    std::vector<std::unique_ptr<boost::asio::windows::random_access_handle> > m_asioHandlePtrsVec;
#else
    std::vector<std::unique_ptr<boost::asio::posix::stream_descriptor> > m_asioHandlePtrsVec;
#endif

    std::vector<bool> m_diskOperationInProgressVec;
};


#endif //_BUNDLE_STORAGE_MANAGER_ASIO_H
