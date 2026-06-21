# Compiler

Compiler for BF code (and custom extension, see later)

The compiler can be utilized via command line (see [help file](Res/help.txt))
or via the API (see to [CompilationParams.h](Src/CompilationParams.h))

Works on Windows and Linux, and can generate assembly for either platform.<br>
Note that if you generate code for another OS, assembly and linking will not be performed.

## Requirements

The compiler just emits the assembly code of the BF code provided, assembling and linking rely on third pary programs.
- NASM for assembling
- Visual Studio 2022 for linking on Windows
- GNU ld for linking on Linux

## Compilation Steps

The code goes through the following phases
- Tokenize: turns the code in a sequence of tokens, each token corresponds to a sequence of identical symbols
            on the same line (to improve error messages returned by the compiler),
            might return errors, like unrecognized tokens
- Parse: creates the AST, might return parsing errors, like unmatched parenthesis
- Analyze: checks that the code is valid, returning all the remaining types of error
- Optimize: performs simple simplifications of the code (if enabled), like 
            removing opposite adjacent operations (`+-><`) and nested loops (`[[code]]`)
- Generate IR
- Emit ASM: currently supports only Windows 64 bit
- Assemble: requires NASM installed
- Link: requires Visual Studio 2022

TODO: extend this to other ASM and other linkers, including VS2026

## About the optimization

The code performs very simple optimizations, the most important of all is
performing up to 256 consecutive `+`, `-`, `>` or `<` operations at once.
Note that this optimization is performed regardless of the `optimization` flag in `CompilationParames`

Another optimization is removing opposite consecutive operations, which sometimes happen
in auto-generated BF code or when the programmer resets the state of the machine to a default
after performing a function.

## Custom Language Extensions

Standard BF code includes only the symbols `+-><[].,`, starts the execution at the beginning
of the file and ends it at the EOF.

This compiler supports the usage of multiple source files and the definition of functions.<br>
This is done like in Assembly, where each file can declare, export or import labels.

When multiple files are provided to the compiler, it's necessary to specify which one is the main
file: the execution will start there and end when ANY EOF is reached, no matter the file.

Labels are
- declared like in assembly (`label_name:`);
- exported using `#export` followed by a space, tab or comma separated list of the labels to export
- imported in another file using `#extern`, again followed by the list of labels.

To jump to a label just use its name.

The existance of function also requires a way to exit them and return to the caller,
here it is done using the token `;`

> Note that if a function does NOT end with `;`, the execution will fallthrough to the next label
  until a `;` or the end of the file, where the execution terminates.

Here follows an example
```
// File1.bf
#extern himom

himom // call function
```

```
// File2.bf
#export himom

himom: // declare function
    +[----->+++<]>+.+.[--->+<]>---.+[----->+<]>.++.--.
    [-]<[-]<[-]<[-]
    ; // return
```

> The language supports comments, starting with `//`