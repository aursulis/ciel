#!/bin/bash

(( BSIZE=1024*1024 ))
(( FSIZE=$BSIZE-1 ))
(( NBLOCKS=1024 ))
(( MAXFILE=$NBLOCKS*$BSIZE-1 ))

TESTDIR=$(mktemp -d)

reinit_fs() {
	rm -f /dev/shm/shmfs-control
	rm -f /dev/shm/shmfs-data
	./shminit
}

make_test_file() {
	head -c$2 /dev/urandom > $1
}

test_seq_fill() {
	for ((i = 0; i < NBLOCKS; i++))
	do
		make_test_file $TESTDIR/test-$i $FSIZE
		./shmld $TESTDIR/test-$i
	done

	((failed = 0))

	for ((i = 0; i < NBLOCKS; i++))
	do
		rm -f $TESTDIR/output
		./shmst test-$i $TESTDIR/output
		if ! diff -q $TESTDIR/test-$i $TESTDIR/output; then
			((failed++))
		fi
	done

	return $failed
}

test_big_fill() {
	make_test_file $TESTDIR/bigtest $MAXFILE
	./shmld $TESTDIR/bigtest
	./shmst bigtest $TESTDIR/output
	if ! diff -q $TESTDIR/bigtest $TESTDIR/output; then
		return 1
	else
		return 0
	fi
}

run_test() {
	reinit_fs
	rm -f $TESTDIR/*
	$1
}

TESTS="test_seq_fill test_big_fill"

echo "Running shmfs unit tests"
for t in $TESTS
do
	echo -n "-- Test $t"
	if run_test $t; then
		echo " -- OK"
	else
		echo " -- FAIL"
	fi
done

rm -r $TESTDIR
