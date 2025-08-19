# Touy

This is my touy language

Based on Jonathan Blow's "Ideas about a new programming language for games."

## Syntax

Variables are statically typed. Types are specified with a `:` or guessed by
the compiler with `:=`:

```c
x : int = 10;
y := 20; // assumes type int
```

### Primitive data types

Numerical types are named after their signedness and bit count:

| Unsigned | Signed |
| -------- | ------ |
|     `u8` |   `s8` |
|    `u16` |  `s16` |
|    `u32` |  `s32` |
|    `u64` |  `s64` |

Type `int` is defined as an alias to `s32`

Floating point numerical types are named after bit count:

- `f32`
- `f64`

Pointer types are denoted by `'`:

```c
x : ' int;
```

Addresses are obtained with `'`:

```c
x := 500;
y := 'x;
```

Pointers can be dereferenced with `^`:

```c
x := 10;
y := 'x;
z := ^y;
```
