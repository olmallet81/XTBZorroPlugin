#pragma once

namespace TCPTools
{
	std::string GetWSAErrorString()
	{
		wchar_t *s = NULL;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&s, 0, NULL);
		std::wstring ws(s);
		std::string ret(ws.begin(), ws.end() - 2);
		LocalFree(s);
		return ret;
	}

	std::string GetSSLErrorString(int code)
	{
		switch (code)
		{
		case SSL_ERROR_NONE:
			return "The TLS/SSL I/O operation completed.";
			break;

		case SSL_ERROR_ZERO_RETURN:
			return "The TLS/SSL connection has been closed.";
			break;

		case SSL_ERROR_WANT_READ:
			return "The read operation did not complete; "
				"the same TLS/SSL I/O function should be called again later.";
			break;

		case SSL_ERROR_WANT_WRITE:
			return "The write operation did not complete; "
				"the same TLS/SSL I/O function should be called again later.";
			break;

		case SSL_ERROR_WANT_CONNECT:
			return "The connect operation did not complete; "
				"the same TLS/SSL I/O function should be called again later.";
			break;

		case SSL_ERROR_WANT_ACCEPT:
			return "The accept operation did not complete; "
				"the same TLS/SSL I/O function should be called again later.";
			break;

		case SSL_ERROR_WANT_X509_LOOKUP:
			return "The operation did not complete because an application callback set"
				" by SSL_CTX_set_client_cert_cb() has asked to be called again. "
				"The TLS/SSL I/O function should be called again later.";
			break;

		case SSL_ERROR_SYSCALL:
			return "Some I/O error occurred. The OpenSSL error queue may contain"
				" more information on the error.";
			break;

		case SSL_ERROR_SSL:
			return "A failure in the SSL library occurred, usually a protocol error. "
				"The OpenSSL error queue contains more information on the error.";
			break;

		default:
			return "Unknown error !";
			break;
		}
	}
}
