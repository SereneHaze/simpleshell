# simpleshell

This is a simple shell implimentation in the C language. The main goal of this is to demonstrate the use of linux syscalls to create a shell enviroment with limited functionality. The main draw of this shell is its focus on memory leak and error free code using valgrind.

## Commands

The folowing commands are implimeted in a basic sense, and follow similar conventions to thier real linux counterparts. 
* mv
* pwd
* ls
* mkdir
* cd
* cp
* rm
* cat

## Additional Notes
Each command only takes the basic inputs, and there is no support for flags. For example, `rm /target/directory` will work, but `rm -r /target/directory` is an unsupported function and will throw an error. 

## Future Features
There is no real plan for more future features, unless I get bored and want to work with more linux syscalls. 
