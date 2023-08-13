# du-clone
The clone of Unix utility 'du'.

## Usage 
The utility allows to view a size of files by an input paths.
```shell
./du-clone path1 path2 pathN 
``` 
### Flags:
1) --help - list of flags;
2) -a - add a size of all files to an output;
3) -c - add a summary at an output end;
4) -s - output of a summary only;
5) -b - use the byte unit at an output;
6) --from-file %path/to/file% - get paths from the file (the every path must be on an every line).
