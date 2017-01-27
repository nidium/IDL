# Generate CPP for JS based on IDL

See [WebIDL spec](http://heycam.github.io/webidl/) for details

# Usage:

Currently there are 3 example projects.
In any of these directories you can start with:

```
make
```
If not all dependencies are met, make should complain.

The idl input files are supposed to be in the 'idl' subdirectory
The generated cpp code will be written in the cpp subdirectory.

# Status

This is a work in progress, allthough the first resulst look promissing.

The following fields require more work:

* UnionType
* OtherType
* callbacks
* dictionaries
* events
* ..

# Spec

There is no final specification yet, only an rough outline that is under discussion.
Details will follow asap.

# Dependencies

executables: 'npm', 'nodejs'
NodeJs modules: 'nunjucks', 'webidl2'

# License

Copyright 2016 Nidium Inc. All rights reserved. Use of this source code is governed by a MIT license that can be found in the LICENSE file.

