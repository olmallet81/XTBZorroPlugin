#pragma once

namespace TCPTools
{
	class Connector
	{
	private:
		std::string _hostname;
		int	_port;
		bool _secure;

	protected:
		SOCKET sock;
		SSL* ssl;
		std::unique_ptr<StreamReader> streamReader;
		std::unique_ptr<StreamWriter> streamWriter;
		std::atomic<bool> connected;

	public:
		Connector(const std::string& hostname, int port, bool secure)
		{
			_hostname = hostname;
			_port = port;
			_secure = secure;

			connected = false;
		}

		~Connector()
		{
			try
			{
				Disconnect();
			}
			catch (std::exception& ex)
			{
				std::cout << ex.what() << std::endl; // use log instead
			}
		}

	public:
		void Connect()
		{		
			// Windows socket
			WSADATA WsaDat;
			if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != NO_ERROR)
				throw std::exception(GetWSAErrorString().c_str());

			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
				throw std::exception(GetWSAErrorString().c_str());

			struct hostent *host;
			if ((host = gethostbyname(_hostname.c_str())) == nullptr)
			{
				std::string error = GetWSAErrorString() + " The server name is wrong or the internet connection has been lost.";
				throw std::exception(error.c_str());
			}

			struct sockaddr_in addr = {};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(_port);
			addr.sin_addr.s_addr = *(long*)(host->h_addr);

			// connecting the socket
			if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
				throw std::exception(GetWSAErrorString().c_str());

			if (_secure)
			{
				// initializing OpenSSL's SSL libraries, loading encryption & hash algorithms for SSL
				SSL_library_init();

				// loading the error strings for good error reporting
				SSL_load_error_strings();

				// creating a new SSL_CTX object, which holds various configuration and data relevant to TLS/SSL or DTLS session establishment
				SSL_CTX* ctx = nullptr;
				if ((ctx = SSL_CTX_new(SSLv23_client_method())) == nullptr)
					throw std::exception(GetWSAErrorString().c_str());

				// creating a new SSL structure which is needed to hold the data for a TLS / SSL connection
				if ((ssl = SSL_new(ctx)) == nullptr)
					throw std::exception(GetWSAErrorString().c_str());

				SSL_CTX_free(ctx);

				// connecting SSL object with the socket
				int ret = SSL_set_fd(ssl, sock);
				if (ret == 0)
					throw std::exception(GetSSLErrorString(SSL_get_error(ssl, ret)).c_str());
			
				// TLS/SSL handshake with the server
				if ((ret = SSL_connect(ssl)) <= 0)
					throw std::exception(GetSSLErrorString(SSL_get_error(ssl, ret)).c_str());

				streamReader.reset(new StreamReader(ssl));
				streamWriter.reset(new StreamWriter(ssl));
			}
			else
			{
				streamReader.reset(new StreamReader(sock));
				streamWriter.reset(new StreamWriter(sock));
			}

			connected = true;
		}

		bool Connected() const
		{
			return connected;
		}

		void WriteMessage(const std::string& message)
		{
			if (connected)
			{
				try
				{
					streamWriter->WriteLine(message);
				}
				catch (std::exception& ex)
				{
					throw std::exception(ex.what());
				}
			}
			else
			{
				throw std::exception("Socket is not connected.");
			}
		}

		const char* ReadMessage(const char* separator)
		{
			if (connected)
			{
				const char* message = nullptr;

				try
				{
					message = streamReader->ReadLine(separator);
				}
				catch (std::exception& ex)
				{
					throw std::exception(ex.what());
				}

				return message;
			}
			else
			{
				throw std::exception("Socket is not connected.");
			}
		}

		void Disconnect()
		{
			if (connected)
			{
				if (_secure)
				{
					if (ssl != nullptr)
					{
						SSL_free(ssl);				
						ssl = nullptr;
					}

					ERR_clear_error();
					ERR_free_strings();
					EVP_cleanup();
				}

				if (closesocket(sock) == SOCKET_ERROR)
					throw std::exception(GetWSAErrorString().c_str());

				sock = INVALID_SOCKET;
				connected = false;
			}

			WSACleanup();
		}
	};
}


