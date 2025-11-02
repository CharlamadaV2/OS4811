#This will run the filesystem formatter
#The make file will ignore other files that won't be used for formatting

output: format.o
	g++ format.o -o output

format.o: knkfs_formating.c
	g++ -c knkfs_formating.c  

target: dependencies
	action
