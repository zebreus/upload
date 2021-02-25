upload -- upload files to the internet
=========================================

## SYNOPSIS

`upload` [<var>options</var>...] <var>file</var><br>
`upload` [<var>options</var>...] <var>file</var>...<br>
`upload` [<var>backend-selection-options</var>...] `-l`|`--list`<br>
`upload` `-h`|`--help`

## DESCRIPTION

**upload** uploads files to the internet and prints a http(s) url, where you can download them.

In its default mode, **upload** uploads the files at <var>path</var> to a random site on the internet.
**upload** guarantees to print the url of the file to stdout or to exit with a nonzero code, unless '-v' is set.
If no upload mode is specified and there is no or one <var>file</var> individual mode will be used, if there are more than one <var>file</var>s archive mode will be used.

The entries of <var>file</var> can be files, directories, character special files, FIFO devices/pipes or `-` for standart input.
Regular files and directories will be processed according to the upload mode(individual or archive).
Character special files and FIFO files will be read for additional filenames. These can only be regular files or directories and they have to be seperated by newlines. Empty lines will be ignored.
If <var>file</var> is empty and the standart input is a FIFO file/pipe, `-` will be added implicitly.
Symlinks in <var>file</var> will be followed.

In individual mode:  
 When uploading a file, the file will be uploaded raw.
 When uploading a directory, an archive with the name of the directory will be created and uploaded. The files/directories contained in the directory will be placed at root-level of the archive.
 If reading character special or FIFO files, each file will uploaded as soon as it is read.
 If one <var>file</var> fails to upload, **upload** will exit with errorcode 3, unless `-c`/`--continue` is set, than it will continue and exit with 0, after there has been an attempt to upload each <var>file</var>.

In archive mode:  
 All specified files and directories are combined into an archive, that will be uploaded. If you specified a directory it will be create as a directory in the archive.
 If reading character special or FIFO files, all read files and all <var>file</var>s will be packed and uploaded afterwards.
 
In list mode:  
 Backends are selected based on the backend selection options and printed.
 No files are checked, loaded or uploaded.
 Most of the options that only affect individual or archive mode are ignored.

<var>time</var> in options is formatted in milliseconds by default. You can suffix your value to change the unit of time. Valid suffixes are
`s` for seconds,
`m` for minutes,
`h` for hours and
`d` for days.
All letters are converted to lowercase before evaluation.

<var>size</var> in options is formatted in bytes by default. You can suffix your value to change the unit of size. Valid suffixes are
`KB` for Kilobyte(10^3),
`KiB`/`K` for Kibibyte(2^10),
`MB` for Megabyte(10^6),
`MiB`/`M` for Mebibyte(2^20),
`GB` for Gigabyte(10^9) and
`GiB`/`G` for Gibibyte(2^30).
All letters are converted to lowercase before evaluation.

## OPTIONS

 * `-h`, `--help` :
   Displays the help screen.

 * `--version` : 
   Displays version information.

 * `-v` :
   Increase the verbosity of the output.

 * `-a`, `--archive` :
   Pack all files and directories into an archive.
   Mutually exclusive with `-i`/`--individual`.

 * `-i`, `--individual` :
   Upload all files or directory individually. For directories an archive with the name of the directory will be created and uploaded.
   Mutually exclusive with `-a`/`--archive`.

 * `-l`, `--list` :
   List all available upload backends for the current request, ordered by preference and exit without uploading anything.

### Mode independend options apply in archive and in individual mode.


 * `--archive-type`=_zip_ :
   Sets the archive type.

 * `-r`, `--root-archive` :
   When archiving a directory, files inside that directory will be put in a directory with the same name in the archive.
   This is the default setting in archive mode.
   Mutually exclusive with `-d`/`--directory-archive.

 * `-d`, `--directory-archive` :
   When archiving a directory, files inside that directory will be put on the root level of the archive.
   This is the default setting in individual mode.
   Mutually exclusive with `-r`/`--root-archive.

### Backend selection options. Specify some requirements that the backend must meet.


 * `-b` <var>backend</var>, `--backend`=<var>backend</var> :
   Select a specific <var>backend</var>. If used multiple times, the backends will be selected in their order.
   If this option is used, the default order is discarded.

 * `-p`, `--preserve-name` :
   Ensure, that the selected backend preserves the filename. Some backends replace the filenames with random strings, this option excludes them.

 * `--no-preserve-name` :
   Ensure, that the selected backend randomizes the filename.

 * `-s`, `--ssl` :
   Ensure, that each file will be upload using https and generates a https url

 * `--no-ssl` :
   Ensure, that each file will be upload using http and generates a http url

 * `--min-retention`=<var>time</var> :
   Ensure, that each file is available to download for at least <var>time</var>.

 * `--max-retention`=<var>time</var> :
   Ensure, that each file gets deleted after <var>time</var> passes (or earlier).

 * `--min-size`=<var>size</var> :
   Ensure, that the every choosen backend supports uploading files bigger than <var>size</var> bytes.

 * `--max-downloads`=<var>downloads</var> :
   Ensure, that each file gets deleted after <var>downloads</var> downloads.

 * `--min-random-part`=<var>characters</var> :
   Ensure, that each url has a random part of at least <var>characters</var> characters.

 * `--max-random-part`=<var>characters</var> :
   Ensure, that each url has a random part not bigger than <var>characters</var> characters.

 * `--max-url-length`=<var>characters</var> :
   Ensure, that each url is shorter than characters.

### Indivial mode options. They are only used in individual mode.


 * `-c`, `--continue` :
   Do not abort if the upload of a file failed.

### Archive mode options. They are only used in archive mode.


 * `-n`, `--name` :
   The name of the created archive. By default the name will be randomly generated.

## BACKEND SELECTION 

First a ordered list of possible backends is generated, by default this includes all known backends and is ordered randomly. You can also set the list manually by using `-b`/`--backend`.
 
Then backends that do not meet the requirements are discarded. You can set the requirements with the backend selection options.

Then an attempt is made to upload the files to the first backend, if the upload finished and an url was generated, the url is printed and **upload** exits 0. If uploading the file failed, the next backend is used. When no backends remain the program exits 1.

## EXAMPLES

Upload a file named file.txt

    $ upload file.txt
    http://somefilehost.tld/file.txt

Upload multiple files as an archive

    $ upload file1.txt file2.txt
    http://somefilehost.tld/upload.zip

Upload multiple files individually

    $ upload -i file1.txt file2.txt
    http://somefilehost.tld/file1.txt
    http://somefilehost.tld/file2.txt

Find all cpp files and upload them as an archive

    $ find . -name "*.cpp" | upload -a
    http://somefilehost.tld/sources.zip

## PRIVACY AND SECURITY

Your files are uploaded to a random server of some random internet person.

There is no guarantee, that your files will be unmodified.

There is no guarantee, that your files are in any way private.

There is no guarantee for anything, btw.

## EXITCODES
0

   Success.

3

   Failed to upload the file(s) to any backend.

4

   There is no backend matching the backend selectiong options.
   
5

   There is no reachable backend matching the backend selectiong options.

64

   Command line usage error.

66

   Unable to read one or more input files/directories.

## BUGS

**Yes**, probably a lot.

If you find one, it would be nice, if you could open an issue on github <https://github.com/Zebreus/upload/issues>.

## COPYRIGHT

**Upload** is copyright (C) 2021 Lennart Eichhorn.
License GPLv3. GNU GPL version 3 <https://gnu.org/licenses/gpl.html>.
This is free software: you are free  to  change  and  redistribute  it.
There is NO WARRANTY, to the extent permitted by law.

## SEE ALSO

man(1)


[SYNOPSIS]: #SYNOPSIS "SYNOPSIS"
[DESCRIPTION]: #DESCRIPTION "DESCRIPTION"
[OPTIONS]: #OPTIONS "OPTIONS"
[BACKEND SELECTION]: #BACKEND-SELECTION "BACKEND SELECTION"
[EXAMPLES]: #EXAMPLES "EXAMPLES"
[PRIVACY AND SECURITY]: #PRIVACY-AND-SECURITY "PRIVACY AND SECURITY"
[EXITCODES]: #EXITCODES "EXITCODES"
[BUGS]: #BUGS "BUGS"
[COPYRIGHT]: #COPYRIGHT "COPYRIGHT"
[SEE ALSO]: #SEE-ALSO "SEE ALSO"


[upload(1)]: upload.html
