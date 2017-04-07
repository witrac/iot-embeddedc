cmake_minimum_required(VERSION 2.8)
include_directories ("${PROJECT_SOURCE_DIR}/lib/" "${PROJECT_SOURCE_DIR}/src/")

set(src_mqtt
    MQTTClient.c
    MQTTFormat.c
    MQTTPacket.c
    MQTTDeserializePublish.c
    MQTTConnectClient.c
    MQTTSubscribeClient.c
    MQTTSerializePublish.c
    MQTTConnectServer.c
    MQTTSubscribeServer.c
    MQTTUnsubscribeServer.c
    MQTTUnsubscribeClient.c
    )

set(src_crypto
    aes.c
    aesni.c
    arc4.c
    asn1parse.c
    asn1write.c
    base64.c
    bignum.c
    blowfish.c
    camellia.c
    ccm.c
    cipher.c
    cipher_wrap.c
    cmac.c
    ctr_drbg.c
    des.c
    dhm.c
    ecdh.c
    ecdsa.c
    ecjpake.c
    ecp.c
    ecp_curves.c
    entropy.c
    entropy_poll.c
    error.c
    gcm.c
    havege.c
    hmac_drbg.c
    md.c
    md2.c
    md4.c
    md5.c
    md_wrap.c
    memory_buffer_alloc.c
    oid.c
    padlock.c
    pem.c
    pk.c
    pk_wrap.c
    pkcs12.c
    pkcs5.c
    pkparse.c
    pkwrite.c
    platform.c
    ripemd160.c
    rsa.c
    sha1.c
    sha256.c
    sha512.c
    threading.c
    timing.c
    version.c
    version_features.c
    xtea.c
)

set(src_x509
    certs.c
    pkcs11.c
    x509.c
    x509_create.c
    x509_crl.c
    x509_crt.c
    x509_csr.c
    x509write_crt.c
    x509write_csr.c
)

set(src_tls
    debug.c
    net_sockets.c
    ssl_cache.c
    ssl_ciphersuites.c
    ssl_cli.c
    ssl_cookie.c
    ssl_srv.c
    ssl_ticket.c
    ssl_tls.c
)

add_library(cJSON cJSON.c)
add_library(mqttlib ${src_mqtt})
add_library(mbedcrypto ${src_crypto})
add_library(mbedx509 ${src_x509})
add_library(mbedtls ${src_tls})
