# beep

CLI program to play Beep sound.

## Build
Because we currently use Windows API to play beep sound, So it can only be built on Windows. (You can also create a pull request to add support for other backends.)
1. Clone this repository and `cd` into it.
2. Run command `mkdir build && cd build && cmake .. && cmake --build .`
3. `cd ../bin` and run `./beep`!

## Usage
Run `./beep -h` to see help message.

## Example
A simple song Twinkle Twinkle Little Star *with just a single line of command!*
```
beep s "C4;C4;G4;G4;A4;A4;G4,750;-,250;F4;F4;E4;E4;D4;D4;C4,750"
```

## Other

This project uses [CLI11](https://github.com/CLIUtils/CLI11) for command line interface.

This project is just a fun project to play with C++ and Windows API. Probably it will be not maintained.

## License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

