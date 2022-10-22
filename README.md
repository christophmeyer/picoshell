# picoshell

A rudimentary interactive shell written in C.


![](demo.gif)

## Getting started

To build the psh executable, run
```
cd ./build && cmake ..
```
and 
```
cmake --build .  
```

Then you can play around with it by running `./psh`. If, for some reason you want to install `psh` onto your system, run

```
sudo cmake --install .  
```

## Features

Picoshell comes with a few basic features, like

- Pipes
- [Readline](https://tiswww.cwru.edu/php/chet/readline/readline.html) line editing
- Command resolution and execution
- Built-in commands: exit, pwd, cd
- Resolution of environment variables
- Double quoting

that enable its use as a rudimentary interactive shell, but it lacks many central aspects of a typical shell (control flow, scripting, I/O redirection, environment variable assignments, ...). 

TODO:
- I/O redirection
- Environment variable assignments with `export`
- ...