#pragma once

#include <zmq.hpp>

/**
 * @class ZeroMQ
 * A class that provides a wrapper for ZeroMQ functionality.
 *
 * This class encapsulates the ZeroMQ library and provides methods to initialize, start, stop, send, and receive
 * messages using ZeroMQ sockets. It supports both binding and connecting to sockets and provides a convenient interface
 * for working with multipart messages.
 */
class ZeroMQ {
  private:
	// Internal context
	std::shared_ptr<zmq::context_t> _contextPtr;
	// Internal socket
	std::unique_ptr<zmq::socket_t> _socketPtr;

	// Is currently active
	bool _isActive{false};
	// Should be binded
	bool _isBinded{false};
	// Address to bind/connect
	std::string _socketAddr;

	// Initializes class
	void init(const std::shared_ptr<zmq::context_t> &ctx, const zmq::socket_type &type, const std::string_view &addr,
			  bool isBind);

  public:
	/**
	 * Construct a new ZeroMQ class
	 * @param[in] type Type of the socket
	 * @param[in] addr Full socket address
	 * @param[in] isBind True if should be binded, false if should be connected
	 */
	ZeroMQ(const zmq::socket_type &type, const std::string &addr, bool isBind);

	/**
	 * Construct a new ZeroMQ class
	 * @param[in] ctx ZeroMQ context
	 * @param[in] type Type of the socket
	 * @param[in] addr Full socket address
	 * @param[in] isBind True if should be binded, false if should be connected
	 */
	ZeroMQ(const std::shared_ptr<zmq::context_t> &ctx, const zmq::socket_type &type, const std::string &addr,
		   bool isBind);

	/// Copy constructor
	ZeroMQ(const ZeroMQ & /*unused*/) = delete;

	/// Move constructor
	ZeroMQ(ZeroMQ && /*unused*/) = delete;

	/// Copy assignment operator
	ZeroMQ &operator=(ZeroMQ /*unused*/) = delete;

	/// Move assignment operator
	ZeroMQ &operator=(ZeroMQ && /*unused*/) = delete;

	/**
	 * Starts the connection
	 * @return True if successfully initialized
	 */
	bool start();

	/**
	 * Stops the connection
	 */
	void stop();

	/**
	 * Receives multipart message
	 * @return std::vector<zmq::message_t> Received messages
	 */
	std::vector<zmq::message_t> recvMessages();

	/**
	 * Sends multipart message
	 * @param[in] msg Messages to send
	 * @return size_t Number of sent messages
	 */
	size_t sendMessages(std::vector<zmq::message_t> &msg);

	/**
	 * Get the reference of socket
	 * @return const std::unique_ptr<zmq::socket_t>&
	 */
	[[nodiscard]] const std::unique_ptr<zmq::socket_t> &getSocket() const { return _socketPtr; }

	/**
	 * Get the reference of context
	 * @return const std::shared_ptr<zmq::context_t>&
	 */
	[[nodiscard]] const std::shared_ptr<zmq::context_t> &getContext() const { return _contextPtr; }

	/**
	 * Get the address of the socket
	 * @return const std::string& Address of the socket
	 */
	[[nodiscard]] const std::string &getAddress() const { return _socketAddr; }

	/**
	 * Destroy the ZeroMQ class
	 */
	~ZeroMQ();
};
