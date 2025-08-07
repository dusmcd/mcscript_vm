# McScript Programming Language v. 0.1

**How to use**
- In the `build` directory, run the following commands:
    - `cmake ..`
    - `make`
- Run the executable at `build/mcscript_vm <optional: source file>`
- If no source file is provided, this will open a REPL where you can start typing commands (see below for syntax)

**Testing**
- In the `test/build` directory, run the following commands:
    - `cmake ..`
    - `make`
- Run executable at `test/build/test`
- Test results will be printed to standard output => failures will be printed to standard error

## Syntax and Basic Usage

**Variables**
- Declaring variables:
  ```
  var x = 10;
  var y = true;
  ```
- Variables are dynamically typed

**Loops**
- While loops function the same way as all C-style languages
- Locally scoped
- They are not expressions, i.e., they do not return a value and cannot be passed around
    like functions
```
while (true) {
    print(i);
}
```

**Types**
- Floating-point Numbers (double in C)
- Boolean (true and false)
- Strings (e.g., "Hello", "Foo bar")

**Control Flow**
```
if (true) {
  return 10;
} else {
  return 11;
}
```
- If expressions will evaluate any expression as truthy or falsey
  - NULL and (boolean) false values are falsey
  - Everything else is truthy

**Functions**
```
  function add(a, b) { return a + b; };
  function num() {return 10;}
  function printVal(val) {print(val);}
```
- Functions can be passed around as arguments and they can be returned from other functions
```
function handleNums(num, callBack) {
    var i = 0;
    while (i < num) {
        callBack(i + 1);
        i = i + 1;
    }
}

function printValue(val) {
    print(val);
}
handleNums(10, printValue);
```

**Built-in Functions**
- print: can print any expression

**Currently Supported Operators**
- Infix operators:
  - "+" (addition)
  - "-" (subtraction)
  - "*" (multiplication)
  - "/" (division)
  - "<" (greater than)
  - ">" (less than)
  - "==" (equals)
  - "!=" (not equal)
- Prefix operators:
  - "!" will make truthy expressions falsey and vice versa
  - "-" will make positive numbers negative and vice versa
- Boolean operators:
  - "and" both expressions must be truthy
  - "or" only one expressions needs to be truthy 

