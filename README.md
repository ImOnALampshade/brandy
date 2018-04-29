# brandy

Brandy programming language.

The ease of Python in a C/C++ level language!

# IMPORTANT NOTE

IF you're reading this, be aware - this is still being written. The compiler
still needs to be implemented. The spec for the language does exist

# Philosophy

C/C++ are very powerful, pervasive languages. While they have immense
capabilities in terms of performance, they don't do a good job at being easy to
use for either for small or large projects.

For example, C++ on its own does not include any of the following things, which
come "in the box" with Python, and other high-level interpreted languages:

* A build system
* Package management
* Documentation generator
* Unit testing
* Code generation
* Reflection

The idea here is that we are trying to take the features, semantics, and extra
bonus things that come with high level scripting languages, and applying them to
low-level programming languages that run directly on the metal.

Brandy takes inspiration from several high-level languages, including Python,
JavaScript, Coffee script, Ruby, Haskell, F#, C#, and a few others. It is not
meant to serve as a replacement to any of these languages - instead, it is meant
to serve as a replacement to C++.

This means that Brandy includes:

* Manual memory management (Though, garbage collection is also included, but can
be disabled)
* Static typing, enforced at compile time
* Dynamic typing, enforced at run time through the `any` type
* Compile-time code generation
* A plethora of operators for frameworks to overload to fit their own purposes
* A build system
* A documentation generator (WIP)
* Asynchronous functions (WIP)

Perhaps most importantly, brandy is **not** an interpreted language. It compiles
straight into native machine code - not into VM byte code or anything like that.
Given the opportunity, it will run as fast as a similarly optimized C program.

In short - Brandy is meant to serve as a middle ground language, almost in the
same vein as C# or Java. However, it puts a lot of the control over the compiler
itself into your hands - going beyond simple reflection and allowing true
introspection into the code you are writing.

# Examples

Here are a few examples of some brandy programs:

## Hello world

```brandy
    // Print statements work exactly as they do in Python 3
    print("Hello, world!")
```

## Fizzbuzz

```brandy
    // 1 to 100, inclusive
    for i in 1..100
      div3 = i % 3
      div5 = i % 5

      if div3 and div5
        print("Fizzbuzz!")
      elif div3
        print("Fizz")
      elif div5
        print("Buzz")
      else
        print(i)
```

# [License](LICENSE)
