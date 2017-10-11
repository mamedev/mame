## Contributing to Sol

Looking to contribute to Sol? Well, first thing I want to mention is thank you!
Second of all, this is probably where you should look :)

## Reporting Issues

If you found a bug, please make sure to make an issue in the issue tracker.

The guidelines for reporting a bug are relatively simple and are as follows:

1. Produce a simple, short, compilable test case that reproduces your problem.
2. Make a descriptive title that summarises the bug as a whole.
3. Explain the bug in as much detail as you can in the body of the issue.

If you have all of those requirements set, then your issue reporting is golden. 

## Submitting a pull request

Submitting a pull request is fairly simple, just make sure it focuses on a single aspect and doesn't 
manage to have scope creep and it's probably good to go. It would be incredibly lovely if the style is 
consistent to those found in the repository.

They are as follows (more will be added as they come up):

- No spaces between parentheses. e.g. `f(g())` not `f ( g ( ) )`.
- Tabs for indentation, spaces for alignment.
- Bracing style is

```cpp
if(my_bool) {
    
}
else if(my_other_bool) {
    
}
else {
    
}
```

- Variable names follow those in the C++ standard, basically snake_case.
- Maximum column length is 125
- Trailing return type will always be in the same line. Even if it goes off the column length. e.g.
`auto f() -> decltype(/* my long expression */) {`
- Since this is a header-only library, all free functions must be marked `inline`.
- Use braces in optional contexts like `if`, `for`, `else`, `while`, etc. e.g.

```cpp
if(x > 12) {
    return x * 2;
}
```

- Use `typename` instead of `class` for template parameters. e.g. `template<typename T>`.

If you don't meet all of these style guidelines, don't fret. I'll probably fix it. But please
do make an effort to actually meet them. Otherwise I'm more likely to reject the pull request.
