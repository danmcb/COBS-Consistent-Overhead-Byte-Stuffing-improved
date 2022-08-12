gcc -shared cobs.c -o libcobs.a
gcc -shared cobs_jf.c -o libjfcobs.a
gcc -shared cobs_scmb.c -o libscmbcobs.a
gcc -L. -lcobs cobs_test.c -o cobs_test.exe
gcc -L. -ljfcobs cobs_test.c -o jf_cobs_test.exe
gcc -L. -lscmbcobs cobs_test_scmb.c -o scmb_cobs_test.exe
