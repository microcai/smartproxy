
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(asn1)
include_directories(evp)
include_directories(modes)

file(GLOB SRCS
	aes/*.c
	asn1/*.c
	bf/*.c
	bio/*.c
	bn/*.c
	buffer/*.c
	camellia/*.c
	cast/*.c
	cmac/*.c
	cms/*.c
	conf/*.c
	dh/*.c

	des/*.c


	dsa/*.c
	dso/*.c

	ec/ec_key.c ec/ec_asn1.c ec/ec_err.c ec/ec_lib.c ec/ec_oct.c ec/ec_check.c ec/ecp_oct.c
	ec/ec2_oct.c ec/ec_mult.c ec/ec_curve.c ec/ec_cvt.c ec/ec2_smpl.c ec/ec_ameth.c ec/ec_print.c
	ec/eck_prn.c ec/ec2_mult.c  ec/ecp_smpl.c ec/ecp_mont.c ec/ec_pmeth.c

	ecdsa/*.c
	ecdh/*.c
	err/*.c
	evp/*.c
	hmac/*.c
	idea/*.c
	lhash/*.c
	md5/*.c
	modes/*.c
	objects/*.c
	ocsp/*.c
	pem/*.c
	pqueue/*.c
	pkcs7/*.c
	pkcs12/*.c
	rand/*.c
	rc2/*.c
	rc4/*.c
	rsa/*.c
	seed/*.c
	sha/*.c
	srp/*.c
	stack/*.c
	ts/*.c
	txt_db/*.c
	ui/*.c
	x509/*.c
	x509v3/*.c
	o_init.c o_dir.c o_str.c o_time.c ex_data.c cpt_err.c cryptlib.c mem.c mem_dbg.c mem_clr.c  cversion.c
	uid.c
)

if (ANDROID)
add_library(crypto SHARED ${SRCS})
else()
add_library(crypto STATIC ${SRCS})
endif()

set_target_properties(crypto PROPERTIES FOLDER "third_party/")

target_compile_definitions(crypto PUBLIC
	-DOPENSSL_NO_ASM
	-DOPENSSL_NO_ENGINE
	-DOPENSSL_NO_RC1
	-DOPENSSL_NO_RC3
	-DOPENSSL_NO_MD1
	-DOPENSSL_NO_MD2
	-DOPENSSL_NO_MD3
	-DOPENSSL_NO_MD4
	-DOPENSSL_NO_MDC2
	-DOPENSSL_NO_WHIRLPOOL
	-DOPENSSL_NO_RIPEMD
	-DOPENSSL_NO_COMP
	-DOPENSSL_NO_OCSP

	-DOPENSSL_BN_ASM_MONT
	-DWIN32_LEAN_AND_MEAN )

target_include_directories(crypto PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include/)

target_compile_definitions(crypto PRIVATE -DECP_NISTZ256_REFERENCE_IMPLEMENTATION)