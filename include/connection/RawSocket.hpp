#pragma once

#include <string>

#include <linux/if_packet.h>

/**
 * Stats produced by RawSocket
 */
struct RawSocketStats {
	/// Number of bytes written to socket
	size_t sentBytes;
	/// Number of bytes read from socket
	size_t receivedBytes;
	/// Total execution time in nanoseconds
	double processingTime;
};

/**
 * Raw socket reads and writes binary data to the provided interface. Write operations do not modify any field
 * (MAC, IP, etc.). They only write the full data directly, similar to file write operations.
 */
class RawSocket {
  private:
	/// Ready flag
	bool _isReady{false};
	/// Mode indicator. True = Write, False = Read
	bool _writeMode{false};
	/// Socket descriptor
	int _sockFd{-1};
	/// Currently used ethernet interface
	std::string _iFace;
	/// Socket structure
	sockaddr_ll _addr{};
	/// Internal structure for statistics
	RawSocketStats _stats{};

	void init(int domain, int type, int protocol);

  public:
	/**
	 * Construct a new RawSocket object
	 * @param[in] iface Ethernet interface
	 * @param[in] isWrite True if write mode, false if read mode is requested
	 */
	explicit RawSocket(std::string iface, bool isWrite = false);

	/// Copy constructor
	RawSocket(const RawSocket & /*unused*/) = delete;

	/// Move constructor
	RawSocket(RawSocket && /*unused*/) = delete;

	/// Copy assignment operator
	RawSocket &operator=(RawSocket /*unused*/) = delete;

	/// Move assignment operator
	RawSocket &operator=(RawSocket && /*unused*/) = delete;

	/**
	 * Returns the binded ethernet interface
	 * @return std::string Name of the interface
	 */
	[[nodiscard]] const std::string &getInterfaceName() const { return _iFace; }

	/**
	 * Writes data to the interface
	 * @param[in] data Full payload data to write
	 * @param[in] dataLen Length of the data
	 * @return int Status of the operation. Returns the number of written bytes, negative on errors.
	 */
	int writeData(const unsigned char *data, size_t dataLen);

	/**
	 * Reads data from the interface
	 * @param[out] data User-allocated data
	 * @param[out] dataLen Length of the data
	 * @return int Status of the operation. Returns the number of read bytes, negative on errors.
	 */
	int readData(unsigned char *data, size_t dataLen);

	/**
	 * Get the statistics of the class
	 * @param[in] resetInternalStats Whether the internal statistics structure should be reset after being returned
	 * @return RawSocketStats Produced statistics
	 */
	RawSocketStats getStats(bool resetInternalStats = false);

	/**
	 * Destroy the RawSocket object
	 */
	~RawSocket();
};
