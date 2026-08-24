# CasmLang
CasmLang is a low-level Assembly transpiler (not a compiler!). With its C-inspired syntax, it aims to provide a more visually appealing way of writing raw assembly.
## Now with syntax highlighting!
The previous commit added a VSCode syntax highlighter. To use, run:
```bash
code --install-extension casmlang-0.0.3-vsix
```
Then press `Ctrl+Shift+P` and type `Developer: Reload Window` and press enter. Then, after creating a `.casm` file, you *should* be able to see it syntax highlighted!
## Why is it a transpiler and not a compiler?
Technically, it _is_ indeed a compiler. However, because it doesn't provide many features compilers are known to provide (mainly register allocation and recursive expressions), I choose to call it a transpiler because the information for every instruction is pretty much self-contained.
## Syntax example
```
rtn _start();

sect text() {
    global _start();
}

rtn _start() noret {
  rsi myVar = 5;
  rdi myVar2 = 3;

  @mov rax, 60
  @mov rdi, 0
  @syscall
}
```
### Explanation
`rtn` means routine. It is the language's version of a function. `rtn _start();` makes a forward declaration for the routine _start such that `text()` knows it exists.
`sect text()` defines a section called `text`, which is standard in x86_64 assembly. It then uses `global _start()` to tell the machine that the entry point is `_start`.
`rtn _start()` defines a routine called `_start()`. `noret` makes it that the transpiler doesn't add a `ret` at the end of the function, which is not needed for the entry point.
`rsi myVar = 5;` and `rdi myVar2 = 3;` define two variables that take up the registers `rsi` and `rdi` respectively. Note that the user manually declares which registers they should take.
```
@mov rax, 60
@mov rdi, 0
@syscall
```
The `@` prefix means that those instructions will translate literally to assembly. Because the language does not currently provide `mov` or `syscall` instructions, they have to be manually placed by the user.

## To-do list
- [ ] Add register freeing (e.g. `free rax`, `free rdi`)
- [x] Add multiline comments
- [ ] Add `syscall` operation
- [ ] Raw register access (e.g. `raw rax = 5`)
- [ ] Variable support for assignment (allow `rax a = b;`)
- [ ] Four operations
- [ ] Branching
- [ ] Goto and labels
- [ ] Add linking. Probably C-style, because it's easier.
  - [ ] Make a standard library
- [ ] `#noheader` option that removes the autogeneration header
- [ ] Stack memory acces (e.g. `stk[bytecount] varname = 0;`, `stk[8] myint = 17;`)
- [ ] Function parameters/arguments
  - Maybe give an optional `autopush` keyword that finds free spots in the stack and pushes the registers used by parameters into them, then loads them on exit, so users don't have to worry about register data disappearing.