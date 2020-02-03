#include "GSMSecurity.h"

enum {
  SSL_ROOT_CERT   = 0,
  SSL_CLIENT_CERT = 1,
  SSL_PRIVATE_KEY = 2
};

char buffer[100];

GSMSecurity::GSMSecurity() :
  _id(0),
  _sslValidation(SSL_VALIDATION_NONE),
  _sslVersion(SSL_VERSION_ANY),
  _sslCipher(SSL_CIPHER_AUTO)
{
  MODEM.addUrcHandler(this);
}

GSMSecurity::~GSMSecurity()
{
  MODEM.removeUrcHandler(this);
}

void GSMSecurity::begin()
{
  MODEM.begin();
}

void GSMSecurity::test(const unsigned char* cert, int size)
{
  Serial.print("MICsecurity::test() : ");  
  //sprintf (buffer, "AT+USECPRF=%d,0,%d", 0, 1);
  Serial.println(size);
}

int GSMSecurity::setValidation(int val)
{
  _sslValidation = val;
  _id = 0;

  Serial.print("MICSecurity::setValidation() : ");
  sprintf (buffer, "AT+USECPRF=%d,0,%d", _id, _sslValidation);
  Serial.println(buffer);
  MODEM.sendf("AT+USECPRF=%d,0,%d", _id, _sslValidation);
  if (MODEM.waitForResponse() != 1) {
    return 0;
  }

  return 1;
}

int GSMSecurity::setVersion(int val)
{
  Serial.print("MICSecurity::setVersion() : ");
  _sslVersion = val;
  sprintf (buffer, "AT+USECPRF=%d,1,%d", _id, _sslVersion);
  Serial.println(buffer);
  MODEM.sendf("AT+USECPRF=%d,1,%d", _id, _sslVersion);
  if (MODEM.waitForResponse() != 1) {
    return 0;
  }

  return 1;
}

int GSMSecurity::setCipher(int val)
{
  Serial.print("MICSecurity::setCipher() : ");
  _sslCipher = val;
  sprintf (buffer, "AT+USECPRF=%d,2,%d", _id, _sslCipher);
  Serial.println(buffer);
  MODEM.sendf("AT+USECPRF=%d,2,%d", _id, _sslCipher);
  if (MODEM.waitForResponse() != 1) {
    return 0;
  }

  return 1;
}

// TODO: implement this
int GSMSecurity::listAllCertificates()
{
  Serial.print("GSMSecurity::listAllCertificates() : ");
  Serial.println("AT+USECMNG=3");
  MODEM.sendf("AT+USECMNG=3");
  if (MODEM.waitForResponse() != 1) {
    return 0;
  }

  return 1;
}

int GSMSecurity::removeCertificate(const char* name)
{
  Serial.print("GSMSecurity::removeCertificate() : ");
  Serial.print("AT+USECMNG=2,0,");
  Serial.println(name);
  MODEM.sendf("AT+USECMNG=2,0,\"%s\"", name);
  if (MODEM.waitForResponse() != 1) {
    return 0;
  }

  return 1;
}

int GSMSecurity::setRootCertificate(const char* name, const unsigned char* cert, int size)
{
  return setCertificate(SSL_ROOT_CERT, name, cert, size);
}

int GSMSecurity::setClientCertificate(const char* name, const unsigned char* cert, int size)
{
  return setCertificate(SSL_CLIENT_CERT, name, cert, size);
}

int GSMSecurity::setPrivateKey(const char* name, const unsigned char* cert, int size)
{
  return setCertificate(SSL_PRIVATE_KEY, name, cert, size);
}

int GSMSecurity::setCertificate(int type, const char* name, const unsigned char* cert, int size)
{
  Serial.print("MICSecurity::setCertificate() : ");
  // TODO: this bugs if you try to add more than MAX_CERTS
  int i = 0;
  while (i <= MAX_CERTS) {
    if (_certs[i].type == -1) {
      break;
    }
    i++;
  }

  if (i == MAX_CERTS) {
    return -2;
  }

  _certs[i].name = name;
  _certs[i].type = type;

  sprintf (buffer, "GSMSecurity::setCertificate: AT+USECMNG=0,%d,\"%s\",%d", type, name, size);
  Serial.println(buffer);

  MODEM.sendf("AT+USECMNG=0,%d,\"%s\",%d", type, name, size);
  MODEM.waitForResponse(100);
  MODEM.write(cert, size);
  MODEM.waitForResponse(1000);

//  if (_certs[i].hash == "") {
//    return 0;
//  }

  int opCode;
  switch (type) {
  case SSL_ROOT_CERT:
    opCode = 3;
    break;
  case SSL_CLIENT_CERT:
    opCode = 5;
    break;
  case SSL_PRIVATE_KEY:
    opCode = 6;
    break;
  default:
    return -1;
  }

  sprintf (buffer, "AT+USECPRF=%d,%d,\"%s\"", _id, opCode, name);
  Serial.println(buffer);
  MODEM.sendf("AT+USECPRF=%d,%d,\"%s\"", _id, opCode, name);
  return MODEM.waitForResponse();
}

void GSMSecurity::handleUrc(const String& urc)
{
  if (urc.startsWith("+USECMNG: 0,")) {
    String temp = urc;

    int certType = urc.charAt(12) - '0';
    String certName;
    String certHash;

    int firstQuoteIndex = temp.indexOf('"');
    int secondQuoteIndex = temp.indexOf('"', firstQuoteIndex + 1);
    int thirdQuoteIndex = temp.indexOf('"', secondQuoteIndex + 1);
    int lastQuoteIndex = temp.lastIndexOf('"');

    if (firstQuoteIndex != -1 && secondQuoteIndex != -1 && firstQuoteIndex != secondQuoteIndex) {
      certName = temp.substring(firstQuoteIndex + 1, secondQuoteIndex);
    }
    if (thirdQuoteIndex != -1 && lastQuoteIndex != -1 && thirdQuoteIndex != lastQuoteIndex) {
      certHash = temp.substring(thirdQuoteIndex + 1, lastQuoteIndex);
    }

    for (int i = 0; i < MAX_CERTS; i++) {
      if (_certs[i].type == certType && _certs[i].name == certName ) {
        _certs[i].hash = certHash;
        break;
      }
    }
  }
}
