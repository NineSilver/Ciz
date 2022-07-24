# The Ciz Programming Language

Ciz is an attempt to create a programming language, compiler and environment meant for systems programming.
The lang is inspired by Visual Basic, mainly because of nostalgia, as it was the first programming language I learned.

Right now the compiler is not ready for use, as it only consists in a lexer and a parser; and it lacks fundamental statements such as `if`, `else`, `while`.


## Build

If you have ever built a C project, you know what to do. Otherwise: install gcc and make, and compile the final executable with the `make` command.
To remove object files from the working tree issue `make clean`.

> Note: if you want to use a different compiler, call `make` setting the variable CC to the path of that compiler. Example: `CC=clang make`.

The `make` command defaults to the release target, which optimises the code and does not generate debugging symbols. To test the program it is recommended to use the debug version (`make debug`). Use also the debug target if you're using debuggers.

[Take a look at the build system](./Makefile).


## Syntax example

This is the syntax I'm trying to implement. Note that this might change in the future.

```
proc Main :: Int () do
    ret 0;
end
```

You can read other samples inside the [tests](./tests) folder.


## Roadmap

- [X] Working lexer
- [X] Parse procedures and expressions
- [ ] Conditionals, loops
- [ ] Type checking
- [ ] Evaluate expressions
- [ ] Context, local and global variables
- [ ] Native AMD64 NASM backend
- [ ] LLVM backend (through a C++ binding library)


## License

This project is [licensed](./LICENSE) under the MIT License.
