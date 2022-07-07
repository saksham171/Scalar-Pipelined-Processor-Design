# How to compile the provided files

There are three files needed for compilation : pipeline.cc, processor.h and processor.cc. 
The command for compilation is:
$ g++ processor.cc pipeline.cc -o pipeline

The provided compilation script will compile these files as well as initialise the output/ODCache.txt, 
which at the start would copy the contents of input/DCache.txt .

$ sh compileFiles.sh

To run for a different assembly program, run the given compile script first to change the input/ICache.txt

$ sh compile.sh arithmetic.asm input/ICache.txt