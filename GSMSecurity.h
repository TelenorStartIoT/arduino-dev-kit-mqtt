#ifndef _GSM_SECURITY_H_INCLUDED
#define _GSM_SECURITY_H_INCLUDED

#include <Arduino.h>

#include "Modem.h"

enum {
  SSL_VERSION_ANY     = 0,
  SSL_VERSION_TLS_1_0 = 1,
  SSL_VERSION_TLS_1_1 = 2,
  SSL_VERSION_TLS_1_2 = 3
};

enum {
  SSL_VALIDATION_NONE                           = 0,
  SSL_VALIDATION_ROOT_CERT                      = 1,
  SSL_VALIDATION_ROOT_CERT_URL_CHECK            = 2,
  SSL_VALIDATION_ROOT_CERT_URL_CHECK_DATE_CHECK = 3
};

enum {
  SSL_CIPHER_AUTO                                = 0,
  SSL_CIPHER_TLS_RSA_WITH_AES_128_CBC_SHA        = 1,
  SSL_CIPHER_TLS_RSA_WITH_AES_128_CBC_SHA256     = 2,
  SSL_CIPHER_TLS_RSA_WITH_AES_256_CBC_SHA        = 3,
  SSL_CIPHER_TLS_RSA_WITH_AES_256_CBC_SHA256     = 4,
  SSL_CIPHER_TLS_RSA_WITH_3DES_EDE_CBC_SHA       = 5,
  SSL_CIPHER_TLS_PSK_WITH_AES_128_CBC_SHA        = 6,
  SSL_CIPHER_TLS_PSK_WITH_AES_256_CBC_SHA        = 7,
  SSL_CIPHER_TLS_PSK_WITH_3DES_EDE_CBC_SHA       = 8,
  SSL_CIPHER_TLS_RSA_PSK_WITH_AES_128_CBC_SHA    = 9,
  SSL_CIPHER_TLS_RSA_PSK_WITH_AES_256_CBC_SHA    = 10,
  SSL_CIPHER_TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA   = 11,
  SSL_CIPHER_TLS_PSK_WITH_AES_128_CBC_SHA256     = 12,
  SSL_CIPHER_TLS_PSK_WITH_AES_256_CBC_SHA384     = 13,
  SSL_CIPHER_TLS_RSA_PSK_WITH_AES_128_CBC_SHA256 = 14,
  SSL_CIPHER_TLS_RSA_PSK_WITH_AES_256_CBC_SHA384 = 15
};

class GSMSecurity : public ModemUrcHandler {

public:
  GSMSecurity();
  virtual ~GSMSecurity();

  operator int() { return _id; }

  /** Check modem response and restart it
   */
  void begin();
  void test(const unsigned char* cert, int size);
  void handleUrc(const String& urc);

  int listAllCertificates();
  int removeCertificate(const char* name);
  int setValidation(int val);
  int setVersion(int val);
  int setCipher(int val);
  int setRootCertificate(const char* name, const unsigned char* cert, int size);
  int setClientCertificate(const char* name, const unsigned char* cert, int size);
  int setPrivateKey(const char* name, const unsigned char* cert, int size);

private:
  int setCertificate(int type, const char* name, const unsigned char* cert, int size);

  // TODO: make _id autoincrement
  int _id;
  int _sslValidation;
  int _sslVersion;
  int _sslCipher;
  const char* _sslServerHostname;

  #define MAX_CERTS 3
  struct {
    int type = -1;
    String name;
    String hash;
  } _certs[MAX_CERTS];
};

#endif
