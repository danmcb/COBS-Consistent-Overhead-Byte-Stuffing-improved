gcc -shared cobs.c -o libcobs.a
gcc -shared cobs_old.c -o liboldcobs.a
gcc -L. -lcobs cobs_test.c -o cobs_test.exe
gcc -L. -loldcobs cobs_test.c -o old_cobs_test.exe
