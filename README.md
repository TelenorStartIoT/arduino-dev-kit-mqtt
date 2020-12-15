# Arduino_M1_MQTT

## Choose cert/key format
Make sure to import either the `DER` or `PEM` format cert or keys, these imports are commented out by default so you have to make a conscious decision.

### PEM
Import the `MICcertificates_YOUR_THING_ID.pem.h` file and make sure to replace `MIC_CLIENT_CERTIFICATE` and `MIC_SECRET_PRIVKEY_PEM` with the keys downloaded from the MIC dashboard for your device. You need to newline escape these `\n` to put them into a string, but take note that you do not modify the size value for the escaped newlines, you still use the value of the unmodified key as reported by the filesystem.

## Root certificate
`MIC_ROOT_CERT` is by default `AmazonRootCA1` ([DER](https://www.amazontrust.com/repository/AmazonRootCA1.cer)|[PEM](https://www.amazontrust.com/repository/AmazonRootCA1.pem)) as you can download from https://www.amazontrust.com/repository/.

As documented at https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html
>If you are experiencing server certificate validation issues, your device may need to explicitly trust the root CA. Try adding the Starfield Root CA Certificate to your trust store.

This has been the case on some devkits for us, and a working solution has been to replace the `MIC_ROOT_CERT` with the `SFSRootCAG2` ([DER](https://www.amazontrust.com/repository/SFSRootCAG2.cer)|[PEM](https://www.amazontrust.com/repository/SFSRootCAG2.pem)) certificate also available from the amazon trust repo linked above.