#!/bin/bash

make
compiled=$?
if [[ $compiled != 0 ]]; then
	echo "does not compile"
	exit 1
fi

echo "Compiles"

echo -n "Example Test : "
./a.out example.txt -v <<< '1 7 3 2 4 0' > output.txt
executed=$?
if [[ $executed !=  0 ]]; then
	echo ":'("
	exit 1
else
	diff -w -B output.txt exampleoutput.txt &> /dev/null
	correct=$?
	if [[ $correct != 0 ]]; then
		echo ":'("
		exit 1
	else
		echo "───==≡≡ΣΣ((( つºل͜º)つ"
	fi
fi


echo -n "Unnested Test : "

./a.out unnested.txt -v <<< '4' > output.txt
executed=$?
if [[ $executed !=  0 ]]; then
	echo ":'("
else
	diff -w -B output.txt unnestedout.txt &> /dev/null
	correct=$?
	if [[ $correct != 0 ]]; then
		echo ":'("
	else
		echo "───==≡≡ΣΣ((( つºل͜º)つ"
	fi
fi
