#ifndef _UDP_BUNDLE_SINK_H
#define _UDP_BUNDLE_SINK_H 1

#include <stdint.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "CircularIndexBufferSingleProducerSingleConsumerConfigurable.h"
#include "PaddedVectorUint8.h"
#include "udp_lib_export.h"

class UdpBundleSink {
private:
    UdpBundleSink();
public:
    typedef boost::function<void(padded_vector_uint8_t & wholeBundleVec)> WholeBundleReadyCallbackUdp_t;
    typedef boost::function<void()> NotifyReadyToDeleteCallback_t;

    UDP_LIB_EXPORT UdpBundleSink(boost::asio::io_service & ioService,
        uint16_t udpPort,
        const WholeBundleReadyCallbackUdp_t & wholeBundleReadyCallback,
        const unsigned int numCircularBufferVectors,
        const unsigned int maxUdpPacketSizeBytes,
        const NotifyReadyToDeleteCallback_t & notifyReadyToDeleteCallback = NotifyReadyToDeleteCallback_t());
    UDP_LIB_EXPORT ~UdpBundleSink();
    UDP_LIB_EXPORT bool ReadyToBeDeleted();
private:

    UDP_LIB_NO_EXPORT void StartUdpReceive();
    UDP_LIB_NO_EXPORT void HandleUdpReceive(const boost::system::error_code & error, std::size_t bytesTransferred);
    UDP_LIB_NO_EXPORT void PopCbThreadFunc();
    UDP_LIB_NO_EXPORT void DoUdpShutdown();
    UDP_LIB_NO_EXPORT void HandleSocketShutdown();

    
    //std::vector<uint8_t> m_fragmentedBundleRxConcat;

    const WholeBundleReadyCallbackUdp_t m_wholeBundleReadyCallback;
    const NotifyReadyToDeleteCallback_t m_notifyReadyToDeleteCallback;

    boost::asio::ip::udp::socket m_udpSocket;
    boost::asio::io_service & m_ioServiceRef;

    const unsigned int M_NUM_CIRCULAR_BUFFER_VECTORS;
    const unsigned int M_MAX_UDP_PACKET_SIZE_BYTES;
    padded_vector_uint8_t m_udpReceiveBuffer;
    boost::asio::ip::udp::endpoint m_remoteEndpoint;
    CircularIndexBufferSingleProducerSingleConsumerConfigurable m_circularIndexBuffer;
    std::vector<padded_vector_uint8_t > m_udpReceiveBuffersCbVec;
    std::vector<boost::asio::ip::udp::endpoint> m_remoteEndpointsCbVec;
    std::vector<std::size_t> m_udpReceiveBytesTransferredCbVec;
    boost::condition_variable m_conditionVariableCb;
    std::unique_ptr<boost::thread> m_threadCbReaderPtr;
    volatile bool m_running;
    volatile bool m_safeToDelete;
    uint32_t m_incomingBundleSize;
    uint64_t m_countCircularBufferOverruns;
    bool m_printedCbTooSmallNotice;
};



#endif  //_UDP_BUNDLE_SINK_H
