# Audacity Packer

A small program that uses [WavPack](https://github.com/dbry/WavPack) to
reduce the size of Audacity's "AU" files. Meant for archiving Audacity
projects.

So far, I've only tested this with 32-bit float projects on Intel
platforms. 32-bit float is the default.

The "AU" files are compressed to WavPack files - you can actually use
`wvunpack` to extract them and get bit-for-bit identical copies of
your original files.

# License

Unless otherwise stated, all files are released under
an MIT-style license. Details in `LICENSE`

Some exceptions:

* `tinydir.h` - retains original licensing (simplified BSD), details found
within the file.
* `yxml.c` and `yxml.h` - retains original licensing (MIT), details found
within the file.
