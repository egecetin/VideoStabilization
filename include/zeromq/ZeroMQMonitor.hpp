#pragma once

#include <zmq.hpp>

#include <atomic>
#include <string>
#include <thread>

/**
 * @class ZeroMQMonitor
 * Class for monitoring ZeroMQ events on a socket.
 *
 * The ZeroMQMonitor class provides functionality to monitor ZeroMQ events on a socket.
 * It starts a separate thread to listen for events and invokes corresponding event handlers.
 * The class supports various event types such as connection, binding, acceptance, closure, etc.
 */
class ZeroMQMonitor : private zmq::monitor_t {
  private:
	std::unique_ptr<std::thread> _monitorThread; /**< Thread for monitoring events. */
	std::atomic_flag _shouldStop{false};		 /**< Flag to stop monitoring. */
	std::atomic_int _peerCount{0};				 /**< Number of peers connected. */

	void threadFunc();

	static void on_event(const std::string &messageStr, int level, const char *addr = nullptr);

	void on_monitor_started() override;

	void on_event_connected(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_connect_delayed(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_connect_retried(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_listening(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_bind_failed(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_accepted(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_accept_failed(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_closed(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_close_failed(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_disconnected(const zmq_event_t & /*unused*/, const char *addr_) override;

#if ZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 0) ||                                                                        \
	(defined(ZMQ_BUILD_DRAFT_API) && ZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 2, 3))
	void on_event_handshake_failed_no_detail(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_handshake_failed_protocol(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_handshake_failed_auth(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_handshake_succeeded(const zmq_event_t & /*unused*/, const char *addr_) override;

#elif defined(ZMQ_BUILD_DRAFT_API) && ZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 2, 1)
	void on_event_handshake_failed(const zmq_event_t & /*unused*/, const char *addr_) override;

	void on_event_handshake_succeed(const zmq_event_t & /*unused*/, const char *addr_) override;
#endif

	void on_event_unknown(const zmq_event_t & /*unused*/, const char *addr_) override;

  public:
	/// Constructor.
	ZeroMQMonitor() = default;

	/// Copy constructor
	ZeroMQMonitor(const ZeroMQMonitor & /*unused*/) = delete;

	/// Move constructor
	ZeroMQMonitor(ZeroMQMonitor && /*unused*/) = delete;

	/// Copy assignment operator
	ZeroMQMonitor &operator=(ZeroMQMonitor /*unused*/) = delete;

	/// Move assignment operator
	ZeroMQMonitor &operator=(ZeroMQMonitor && /*unused*/) = delete;

	/**
	 * Start monitoring events on the given socket.
	 *
	 * This method starts a separate thread to listen for events on the given socket.
	 * The method also sets up the event handlers for various event types.
	 *
	 * @param[in] socket Zeromq socket
	 * @param[in] monitorAddress Monitoring address
	 */
	void startMonitoring(zmq::socket_t *socket, const std::string &monitorAddress);

	/**
	 * Get the number of peers connected.
	 * @return Number of peers connected.
	 */
	[[nodiscard]] int getPeerCount() const { return _peerCount; }

	/**
	 * Stop monitoring events on the socket.
	 */
	void stopMonitoring();

	/**
	 * Test internals of the ZeroMQMonitor class.
	 */
	void testInternals();

	/**
	 * Destructor.
	 */
	~ZeroMQMonitor() override { stopMonitoring(); }
};
