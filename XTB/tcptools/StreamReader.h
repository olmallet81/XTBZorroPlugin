#pragma once

namespace TCPTools
{
	class StreamReader
	{
	private:
		SSL* _ssl;
		SOCKET _sock;

		char* _buffer;
		char* _currentMsg;
		char* _nextMsg;
		int _bufferSize;
		int _increment;
		int _bytesReceived;
		int _totalBytesParsed;
		int _bytesLeftover;
		int _bytesToParse;

		void Initialize()
		{
			_bufferSize = 1024;
			_increment = _bufferSize;
			_buffer = new char[_bufferSize];
			_currentMsg = _buffer;
			_nextMsg = nullptr;
			_bytesReceived = 0;
			_totalBytesParsed = 0;
			_bytesLeftover = 0;
			_bytesToParse = 0;

			memset(_buffer, 0, _bufferSize);
		}

		int Read(char*& message, int bufferSize, int bytesLeftover = 0)
		{
			int nbytes = 0;

			if (_ssl)
			{
				if ((nbytes = SSL_read(_ssl, message, bufferSize - bytesLeftover - 1)) <= 0) // SSL_read as other OpenSSL method can throw exceptions
				{
					if (nbytes < 0)
						throw std::exception(GetWSAErrorString().c_str());
					else
						throw std::exception("No bytes received from the server.");
				}
			}
			else if (_sock)
			{
				if ((nbytes = recv(_sock, message, bufferSize - bytesLeftover - 1, 0)) <= 0)
				{
					if (nbytes == SOCKET_ERROR)
						throw std::exception(GetWSAErrorString().c_str());
					else
						throw std::exception("Socket is not connected.");
				}
			}

			return nbytes;
		}

	public:
		StreamReader(SSL* ssl)
		{
			_ssl = ssl;
			_sock = 0;

			Initialize();
		}

		StreamReader(SOCKET sock)
		{
			_ssl = nullptr;
			_sock = sock;

			Initialize();
		}

		~StreamReader()
		{
			delete _buffer;
		}

		// _bytesReceived = 0 when all the response has been read?
		const char* ReadLine(const char* separator)
		{
			if (!_nextMsg)
			{
				_bytesReceived = Read(_currentMsg, _bufferSize, _bytesLeftover);

				if (_bytesReceived <= 0)
					return nullptr;

				// calculating number of bytes to process
				_bytesToParse = _bytesReceived;
				_bytesToParse += _bytesLeftover;
				_bytesLeftover = 0;

				// moving pointer back to the beginning of the buffer
				_currentMsg = _buffer;
			}

			if ((_nextMsg = strstr(_currentMsg, separator)) != nullptr)
			{
				int bytesToCopy = _nextMsg - _currentMsg;
				char* line = new char[bytesToCopy + 1];
				line = strncpy(line, _currentMsg, bytesToCopy);
				line[bytesToCopy] = '\0';

				// updating pointer to the next msg
				_currentMsg = _nextMsg + strlen(separator);

				return line;
			}

			// checking for an incomplete message and copying it to the beginning of the buffer for the next read from the socket
			// this is when a line has been returned but there is some characters left needing to be saved before calling GetResponse again
			if (_currentMsg - _buffer < _bytesToParse)
			{
				// we have an incomplete message
				_bytesLeftover = _bytesToParse;
				_bytesLeftover -= (_currentMsg - _buffer);

				// increasing buffer size for the next call
				if (_currentMsg == _buffer)
				{
					_bufferSize += _increment;
					char* newBuffer = new char[_bufferSize];
					memcpy_s(newBuffer, _bufferSize, _currentMsg, _bytesLeftover);
					delete[] _buffer;
					_buffer = newBuffer;
				}
				else
					// copy the left over bytes to the front of the buffer
					memcpy_s(_buffer, _bufferSize, _currentMsg, _bytesLeftover);

				// reset our pointer to the current location in the message buffer for the next read from the socket
				_currentMsg = _buffer + _bytesLeftover;
			}
			else
				_currentMsg = _buffer;

			// zeroing out any of the buffer that was used in the read starting at the end of what we copied to the front
			memset(_currentMsg, 0, _bufferSize - _bytesLeftover);

			return this->ReadLine(separator);
		}
	};
}
