#ifndef _TCP_ASYNC_SENDER_H
#define _TCP_ASYNC_SENDER_H 1
/*
A class that exists because according to:
    https://www.boost.org/doc/libs/1_75_0/doc/html/boost_asio/reference/async_write/overload1.html

Regarding the use of async_write, it says:

This operation is implemented in terms of zero or more calls to the stream's async_write_some function,
and is known as a composed operation. The program must ensure that the stream performs no other write
operations (such as async_write, the stream's async_write_some function, or any other composed operations
that perform writes) until this operation completes.

*/

#include <string>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <queue>
#include <boost/function.hpp>
#include <zmq.hpp>
#ifdef OPENSSL_SUPPORT_ENABLED
#include <boost/asio/ssl.hpp>
#endif
#include "hdtn_util_export.h"

struct TcpAsyncSenderElement {
    typedef boost::function<void(const boost::system::error_code& error, std::size_t bytes_transferred)> OnSuccessfulSendCallbackByIoServiceThread_t;
    HDTN_UTIL_EXPORT TcpAsyncSenderElement();
    HDTN_UTIL_EXPORT ~TcpAsyncSenderElement();
    
    HDTN_UTIL_EXPORT void DoCallback(const boost::system::error_code& error, std::size_t bytes_transferred);

    std::vector<boost::asio::const_buffer> m_constBufferVec;
    std::vector<std::vector<boost::uint8_t> > m_underlyingData;
    std::unique_ptr<zmq::message_t>  m_underlyingDataZmq;
    OnSuccessfulSendCallbackByIoServiceThread_t * m_onSuccessfulSendCallbackByIoServiceThreadPtr;
};

class TcpAsyncSender {
private:
    TcpAsyncSender();
public:
    
    HDTN_UTIL_EXPORT TcpAsyncSender(boost::shared_ptr<boost::asio::ip::tcp::socket> & tcpSocketPtr, boost::asio::io_service & ioServiceRef);

    HDTN_UTIL_EXPORT ~TcpAsyncSender();
    
    HDTN_UTIL_EXPORT void AsyncSend_NotThreadSafe(TcpAsyncSenderElement * senderElementNeedingDeleted);
    HDTN_UTIL_EXPORT void AsyncSend_ThreadSafe(TcpAsyncSenderElement * senderElementNeedingDeleted);
    //void SetOnSuccessfulAckCallback(const OnSuccessfulAckCallback_t & callback);
private:
    
    HDTN_UTIL_EXPORT void HandleTcpSend(const boost::system::error_code& error, std::size_t bytes_transferred);


    boost::asio::io_service & m_ioServiceRef;
    boost::shared_ptr<boost::asio::ip::tcp::socket> m_tcpSocketPtr;
    std::queue<std::unique_ptr<TcpAsyncSenderElement> > m_queueTcpAsyncSenderElements;

    
    volatile bool m_writeInProgress;

};

#ifdef OPENSSL_SUPPORT_ENABLED
class TcpAsyncSenderSsl {
private:
    TcpAsyncSenderSsl();
public:
    typedef boost::shared_ptr< boost::asio::ssl::stream<boost::asio::ip::tcp::socket> > ssl_stream_sharedptr_t;

    HDTN_UTIL_EXPORT TcpAsyncSenderSsl(ssl_stream_sharedptr_t & sslStreamSharedPtr, boost::asio::io_service & ioServiceRef);

    HDTN_UTIL_EXPORT ~TcpAsyncSenderSsl();

    HDTN_UTIL_EXPORT void AsyncSendSecure_NotThreadSafe(TcpAsyncSenderElement * senderElementNeedingDeleted);
    HDTN_UTIL_EXPORT void AsyncSendSecure_ThreadSafe(TcpAsyncSenderElement * senderElementNeedingDeleted);
    HDTN_UTIL_EXPORT void AsyncSendUnsecure_NotThreadSafe(TcpAsyncSenderElement * senderElementNeedingDeleted);
    HDTN_UTIL_EXPORT void AsyncSendUnsecure_ThreadSafe(TcpAsyncSenderElement * senderElementNeedingDeleted);
    //void SetOnSuccessfulAckCallback(const OnSuccessfulAckCallback_t & callback);
private:

    HDTN_UTIL_EXPORT void HandleTcpSendSecure(const boost::system::error_code& error, std::size_t bytes_transferred);
    HDTN_UTIL_EXPORT void HandleTcpSendUnsecure(const boost::system::error_code& error, std::size_t bytes_transferred);


    boost::asio::io_service & m_ioServiceRef;
    ssl_stream_sharedptr_t m_sslStreamSharedPtr;
    std::queue<std::unique_ptr<TcpAsyncSenderElement> > m_queueTcpAsyncSenderElements;


    volatile bool m_writeInProgress;

};
#endif

#endif //_TCP_ASYNC_SENDER_H
