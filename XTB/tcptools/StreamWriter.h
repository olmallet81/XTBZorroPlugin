#pragma once

namespace TCPTools
{
	class StreamWriter
	{
	private:
		SSL* _ssl;
		SOCKET _sock;

	public:
		StreamWriter(SSL* ssl)
		{
			_ssl = ssl;
			_sock = 0;
		}

		StreamWriter(SOCKET socket)
		{
			_ssl = nullptr;
			_sock = socket;
		}

		void WriteLine(const std::string& message)
		{
			if (_ssl)
			{
				int nbytes = SSL_write(_ssl, message.c_str(), message.length());

				if (nbytes < 0)
					throw std::exception(GetWSAErrorString().c_str());
			}
			else if (_sock)
			{
				if (send(_sock, message.c_str(), message.length(), 0) == SOCKET_ERROR)
					throw std::exception(GetWSAErrorString().c_str());
			}
		}
	};
}

