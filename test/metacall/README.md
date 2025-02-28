This proof of concept is for testing MetaCall use case: https://github.com/metacall/core/pull/533

First of all we have the following preconditions:
 - `libmetacall` which loads `libnode_loader`.
 - `libnode_loader` is not linked to anything but we are going to weakly link it to `libnode`.

There are two possible cases, this happens before loading libnode_loader:
  - MetaCall is not being executed by `node.exe`, then:
    1) Windows:
        - `libmetacall` loads dynamically all the dependencies of `libnode_loader` (aka `libnode`).
        - We list all the symbols of each dependency (aka `libnode`) so we construct a hashmap of symbol string to symbol function pointer.
        - We list all the unresolved symbols of `libnode_loader` and we link them to `libnode`.
        
    2) MacOS & Linux:
        - `libmetacall` loads dynamically all the dependencies of `libnode_loader` (aka `libnode`).
        - Linking is resolved by the linker automatically.

  - MetaCall is being executed by node.exe, then we have two possible cases:
    1) `node.exe` compiled statically (without `libnode`):
        - We get all the library dependencies from `node.exe` and we do not find `libnode`, so we get the handle of the currrent process.
        - We list all symbols of `node.exe` and we construct a hash map a hashmap of symbol string to symbol function pointer.
        - We list all the unresolved symbols of `libnode_loader` and we link them to `node.exe`.

    2) `node.exe` compiled dynamically (with `libnode`):
        - We get all the library dependencies from `node.exe` and we find `libnode` so we get the handle from it.
        - We list all the symbols of each dependency (aka `libnode`) so we construct a hashmap of symbol string to symbol function pointer of those dependencies (`libnode`).
        - We list all the unresolved symbols of `libnode_loader` and we link them to `libnode` of `node.exe`.
