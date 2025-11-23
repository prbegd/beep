# beep

CLI program to play Beep sound.

## Build
Because we currently use Windows API to play beep sound, So it can only be built on Windows. (You can also create a pull request to add support for other backends.)
1. Clone this repository and `cd` into it.
2. Run command `mkdir build && cd build && cmake .. && cmake --build .`
3. `cd ../bin` and run `./beep`!

## Usage
<details>
<summary>Click Me to Expand</summary>

```
CLI program to play Beep sound.


beep.exe [OPTIONS] [SUBCOMMAND]


OPTIONS:
  -b,     --backend TEXT      Backend to use for beep sound.
  -h,     --help              Show this help message and exit. You can also use this on
                              subcommands.

SUBCOMMANDS:
  f                           Play a beep sound with the specified frequency and duration.
  s                           Play more beep sounds using a music score.
  c                           Compute the frequency of a note in Hz.
```
Subcommands:
```
Play a beep sound with the specified frequency and duration.


f [OPTIONS] frequency [duration]


POSITIONALS:
  frequency FLOAT REQUIRED    Frequency of the beep sound in Hz.
  duration INT [500]          Duration of the beep sound in milliseconds.

OPTIONS:
  -h,     --help              Show this help message and exit. You can also use this on
                              subcommands.
```
```
Play more beep sounds using a music score.


s [OPTIONS] score


POSITIONALS:
  score TEXT REQUIRED         Music score to play.
                              Format: '<note_name>[,duration][;note_name[,duration]...]'
                              note_name: Note name in the format:' <A-G>[#|b]<octave>' e.g. C4,
                              D#3, Gb2. Can also be: 'break', '-', which pause the sound for
                              the specified duration.
                              duration: Duration of the beep sound in milliseconds. INT64
                              value, default: 500 (ms).

                              Example: C4;E4;G4;C5,1000

OPTIONS:
  -h,     --help              Show this help message and exit. You can also use this on
                              subcommands.
  -a,     --A4Pitch FLOAT [440]
                              Pitch of the A note in 4th octave (A4) in Hz. This pitch is used
                              as the standard pitch for calculating note pitches.
```
```
Compute the frequency of a note in Hz.


c [OPTIONS] note [method]


POSITIONALS:
  note TEXT REQUIRED          Note name in the format: '<A-G>[#|b]<octave>' e.g. C4, D#3, Gb2.
  method TEXT:{12tet} [12tet]
                              Method to use for computing the frequency. Only 12 tone equal
                              temperament (12tet) is supported currently.

OPTIONS:
  -h,     --help              Show this help message and exit. You can also use this on
                              subcommands.
  -a,     --A4Pitch FLOAT [440]
                              Pitch of the A note in 4th octave (A4) in Hz. This pitch is used
                              as the standard pitch for calculating note pitches.
```

</details>

## Example
A simple song Twinkle Twinkle Little Star *with just a single line of command!*
```
./beep s "C4;C4;G4;G4;A4;A4;G4,750;-,250;F4;F4;E4;E4;D4;D4;C4,750"
```

## Other

This project uses [CLI11](https://github.com/CLIUtils/CLI11) for command line interface.

This project is just a fun project to play with C++ and Windows API. Probably it will be not maintained.

## License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

