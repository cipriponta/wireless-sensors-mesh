# How to generate a self signed SSL certificate:

https://docs.python.org/3/library/ssl.html#module-ssl

Do not forget to add the ip address of the server inside the common name field of the certificate

```
openssl req -new -x509 -days 3650 -nodes -out <cert_path>.pem -keyout <key_path>.pem
```