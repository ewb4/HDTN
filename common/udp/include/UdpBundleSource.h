#ifndef _UDP_BUNDLE_SOURCE_H
#define _UDP_BUNDLE_SOURCE_H 1

#include <string>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <map>
#include <queue>
#include "CircularIndexBufferSingleProducerSingleConsumerConfigurable.h"
#include "RateManagerAsync.h"
#include <zmq.hpp>
#include "udp_lib_export.h"

class UdpBundleSource {
private:
    UdpBundleSource();
public:
    typedef boost::function<void()> OnSuccessfulAckCallback_t;
    UDP_LIB_EXPORT UdpBundleSource(const uint64_t rateBps, const unsigned int maxUnacked); //const uint64_t rateBps = 50, const unsigned int maxUnacked = 100

    UDP_LIB_EXPORT ~UdpBundleSource();
    UDP_LIB_EXPORT void Stop();
    UDP_LIB_EXPORT bool Forward(const uint8_t* bundleData, const std::size_t size);
    UDP_LIB_EXPORT bool Forward(zmq::message_t & dataZmq);
    UDP_LIB_EXPORT bool Forward(std::vector<uint8_t> & dataVec);
    UDP_LIB_EXPORT std::size_t GetTotalUdpPacketsAcked();
    UDP_LIB_EXPORT std::size_t GetTotalUdpPacketsSent();
    UDP_LIB_EXPORT std::size_t GetTotalUdpPacketsUnacked();
    UDP_LIB_EXPORT std::size_t GetTotalBundleBytesAcked();
    UDP_LIB_EXPORT std::size_t GetTotalBundleBytesSent();
    UDP_LIB_EXPORT std::size_t GetTotalBundleBytesUnacked();
    UDP_LIB_EXPORT void UpdateRate(uint64_t rateBitsPerSec);
    UDP_LIB_EXPORT void Connect(const std::string & hostname, const std::string & port);
    UDP_LIB_EXPORT bool ReadyToForward() const;
    UDP_LIB_EXPORT void SetOnSuccessfulAckCallback(const OnSuccessfulAckCallback_t & callback);
private:
    UDP_LIB_NO_EXPORT void OnResolve(const boost::system::error_code & ec, boost::asio::ip::udp::resolver::results_type results);
    UDP_LIB_NO_EXPORT void OnConnect(const boost::system::error_code & ec);
    UDP_LIB_NO_EXPORT void HandleUdpSend(boost::shared_ptr<std::vector<boost::uint8_t> > dataSentPtr, const boost::system::error_code& error, std::size_t bytes_transferred);
    UDP_LIB_NO_EXPORT void HandleUdpSendZmqMessage(boost::shared_ptr<zmq::message_t> dataZmqSentPtr, const boost::system::error_code& error, std::size_t bytes_transferred);

    UDP_LIB_NO_EXPORT void DoUdpShutdown();
    UDP_LIB_NO_EXPORT void DoHandleSocketShutdown();
    UDP_LIB_NO_EXPORT void PacketsSentCallback();
    



    
    boost::asio::io_service m_ioService;
    boost::asio::io_service::work m_work;
    boost::asio::ip::udp::resolver m_resolver;
    RateManagerAsync m_rateManagerAsync;
    boost::asio::ip::udp::socket m_udpSocket;
    boost::asio::ip::udp::endpoint m_udpDestinationEndpoint;
    std::unique_ptr<boost::thread> m_ioServiceThreadPtr;
    boost::condition_variable m_localConditionVariableAckReceived;

    OnSuccessfulAckCallback_t m_onSuccessfulAckCallback;
    volatile bool m_readyToForward;
    volatile bool m_useLocalConditionVariableAckReceived;

};



#endif //_UDP_BUNDLE_SOURCE_H
