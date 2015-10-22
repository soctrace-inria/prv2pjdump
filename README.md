#prv2pjdump

prv2pjdump converts paraver traces to pjdump. 

Documentation for [paraver](http://www.bsc.es/computer-sciences/performance-tools/paraver) trace files can be found [in this pdf](www.bsc.es/media/1370.pdf).

Documentation for pjdump trace format can be found [at this address](https://github.com/schnorr/pajeng/wiki/pj_dump).

## Usage

	Usage: prv2pjdump [OPTION] FILE
	Convert the paraver FILE into pjdump.
		-o, --output-file 	Specify an output file.
		
### Compile

	$ make
	
Under Linux it is possible to cross-compile it for Windows (for both 32 and 64 bit versions), with the following command line:

	$ make win32 win64

##Licence

prv2pjdump is distributed under the [MIT License](https://opensource.org/licenses/MIT).

