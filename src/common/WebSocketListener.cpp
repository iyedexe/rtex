#include "common/logger.hpp"
#include "common/WebSocketListener.h"
#include <websocketpp/client.hpp>
#include <iostream>

std::shared_ptr<sslcontext> WebSocketListener::on_tls_init() {
  auto ctx = std::make_shared<sslcontext>(
      boost::asio::ssl::context::sslv23);
  return ctx;
}

WebSocketListener::WebSocketListener() {
    tls_client_.clear_access_channels(websocketpp::log::alevel::all);  // Disable all access log channels
    tls_client_.clear_error_channels(websocketpp::log::elevel::all);   // Disable all error log channels

    tls_client_.init_asio();
    tls_client_.set_tls_init_handler(websocketpp::lib::bind(&WebSocketListener::on_tls_init, this));    
    tls_client_.set_open_handler(websocketpp::lib::bind(&WebSocketListener::onOpen, this, websocketpp::lib::placeholders::_1));
    tls_client_.set_close_handler(websocketpp::lib::bind(&WebSocketListener::onClose, this, websocketpp::lib::placeholders::_1));
    tls_client_.set_fail_handler(websocketpp::lib::bind(&WebSocketListener::onFail, this, websocketpp::lib::placeholders::_1));
    tls_client_.set_message_handler(websocketpp::lib::bind(&WebSocketListener::onMessage, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
}

WebSocketListener::~WebSocketListener() {}


void WebSocketListener::connect(const std::string& uri) {
    websocketpp::lib::error_code ec;
    con_ = tls_client_.get_connection(uri, ec);
    if (ec) {
        LOG_ERROR("TLS Connection error: {}", ec.message());
        return;
    }
    tls_client_.connect(con_);
    LOG_INFO("[WSListener][CONNECT] TLS Connection established {}:{}{}", con_->get_host(), con_->get_port() ,con_->get_resource());

}

void WebSocketListener::writeWS(const std::string& message){
    try 
    {
        if (~isConnected_) // gain speed by not locking if already connected
        {
            std::unique_lock<std::mutex> lock(connectionMutex_);
            connectionCond_.wait(lock, [this] { return isConnected_; });
        }

        LOG_DEBUG("[WSListener][SEND] Sending over WS {}", message);
        tls_client_.send(hdl_, message, websocketpp::frame::opcode::text);
    } catch (const std::exception& e) {
        LOG_ERROR("[WSListener][SEND] Error while writing on WS message :[{}]: {}", message, e.what());
    }
}

void WebSocketListener::startClient() {
    LOG_INFO("[WSListener][START_CLIENT] Running client on {}:{}{}", con_->get_host(), con_->get_port() ,con_->get_resource());
    tls_client_.run();
}


void WebSocketListener::stopClient() {
    tls_client_.stop();
}

void WebSocketListener::onOpen(websocketpp::connection_hdl hdl) {
    hdl_ = hdl;
    std::unique_lock<std::mutex> lock(connectionMutex_);
    isConnected_ = true;
    connectionCond_.notify_all();
    LOG_INFO("[WSListener][ON_OPEN] WebSocket connection opened.");
}

void WebSocketListener::onClose(websocketpp::connection_hdl hdl) {
    LOG_INFO("[WSListener][ON_CLOSE] WebSocket connection closed.");
    std::string m_server = con_->get_response_header("Server");
    std::string m_error_reason = con_->get_ec().message();

    LOG_INFO("[WSListener][ON_CLOSE] Remote Server: {}", (m_server.empty() ? "None Specified" : m_server));
    LOG_INFO("[WSListener][ON_CLOSE] Error/close reason: {}", (m_error_reason.empty() ? "N/A" : m_error_reason));
}

void WebSocketListener::onFail(websocketpp::connection_hdl hdl) {
    LOG_INFO("[WSListener][ON_FAIL] WebSocket connection failed.");
    std::string m_server = con_->get_response_header("Server");
    std::string m_error_reason = con_->get_ec().message();

    LOG_INFO("[WSListener][ON_FAIL] Remote Server: {}", (m_server.empty() ? "None Specified" : m_server));
    LOG_INFO("[WSListener][ON_FAIL] Error/close reason: {}", (m_error_reason.empty() ? "N/A" : m_error_reason));
}