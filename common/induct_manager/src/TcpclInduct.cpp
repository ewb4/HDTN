#include "TcpclInduct.h"
#include <iostream>
#include <boost/make_unique.hpp>
#include <boost/make_shared.hpp>


//TCPCL INDUCT
TcpclInduct::TcpclInduct(const InductProcessBundleCallback_t & inductProcessBundleCallback, const induct_element_config_t & inductConfig) :
    Induct(inductProcessBundleCallback, inductConfig),
    m_tcpAcceptor(m_ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), inductConfig.boundPort))
{
    StartTcpAccept();
    m_ioServiceThreadPtr = boost::make_unique<boost::thread>(boost::bind(&boost::asio::io_service::run, &m_ioService));
}
TcpclInduct::~TcpclInduct() {
    
    if (m_tcpAcceptor.is_open()) {
        try {
            m_tcpAcceptor.close();
        }
        catch (const boost::system::system_error & e) {
            std::cerr << "Error closing TCP Acceptor in TcpclInduct::~TcpclInduct:  " << e.what() << std::endl;
        }
    }
    
    m_listTcpclBundleSinks.clear(); //tcp bundle sink destructor is thread safe
    
    if (m_ioServiceThreadPtr) {
        m_ioServiceThreadPtr->join();
        m_ioServiceThreadPtr.reset(); //delete it
    }
}

void TcpclInduct::StartTcpAccept() {
    std::cout << "waiting for tcpcl tcp connections" << std::endl;
    boost::shared_ptr<boost::asio::ip::tcp::socket> newTcpSocketPtr = boost::make_shared<boost::asio::ip::tcp::socket>(m_ioService); //get_io_service() is deprecated: Use get_executor()

    m_tcpAcceptor.async_accept(*newTcpSocketPtr,
        boost::bind(&TcpclInduct::HandleTcpAccept, this, newTcpSocketPtr, boost::asio::placeholders::error));
}

void TcpclInduct::HandleTcpAccept(boost::shared_ptr<boost::asio::ip::tcp::socket> & newTcpSocketPtr, const boost::system::error_code& error) {
    if (!error) {
        std::cout << "tcpcl tcp connection: " << newTcpSocketPtr->remote_endpoint().address() << ":" << newTcpSocketPtr->remote_endpoint().port() << std::endl;
        m_listTcpclBundleSinks.emplace_back(newTcpSocketPtr, m_ioService,
            m_inductProcessBundleCallback,
            m_inductConfig.numRxCircularBufferElements,
            m_inductConfig.numRxCircularBufferBytesPerElement,
            m_inductConfig.endpointIdStr,
            boost::bind(&TcpclInduct::ConnectionReadyToBeDeletedNotificationReceived, this));

        StartTcpAccept(); //only accept if there was no error
    }
    else if (error != boost::asio::error::operation_aborted) {
        std::cerr << "tcp accept error: " << error.message() << std::endl;
    }


}

void TcpclInduct::RemoveInactiveTcpConnections() {
    m_listTcpclBundleSinks.remove_if([](TcpclBundleSink & sink) { return sink.ReadyToBeDeleted(); });
}

void TcpclInduct::ConnectionReadyToBeDeletedNotificationReceived() {
    boost::asio::post(m_ioService, boost::bind(&TcpclInduct::RemoveInactiveTcpConnections, this));
}