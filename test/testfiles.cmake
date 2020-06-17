configure_file("test/unpack.sh.in" "bin/unpack.sh" @ONLY)
configure_file("test/test_TinyGarble.sh.in" "bin/test_TinyGarble.sh" @ONLY)

configure_file("test/consec_test_a.txt.in" "bin/consec_test_a.txt" @ONLY)
configure_file("test/consec_test_b.txt.in" "bin/consec_test_b.txt" @ONLY)

configure_file("test/mxm_relu_mp_a.txt.in" "bin/mxm_relu_mp_a.txt" @ONLY)
configure_file("test/mxm_relu_mp_b.txt.in" "bin/mxm_relu_mp_b.txt" @ONLY)

configure_file("test/aes_ht_a.txt.in" "bin/aes_ht_a.txt" @ONLY)
configure_file("test/aes_ht_b.txt.in" "bin/aes_ht_b.txt" @ONLY)

configure_file("test/benchmarks.txt.in" "bin/benchmarks.txt" @ONLY)

configure_file("test/benchmarks_aes_cbc.txt.in" "bin/benchmarks_aes_cbc.txt" @ONLY)