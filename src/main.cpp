// beep

// Copyright (c) 2025 prbegd
// Distribution under the MIT License. See the accompanying file LICENSE or copy at https://opensource.org/licenses/MIT

#include "CLI/CLI11.hpp"
#include <cctype>
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

struct Note {
    double frequency;
    std::chrono::milliseconds duration;
};

class BeepInterface {
public:
    virtual ~BeepInterface() = default;
    virtual void beep(const Note& note) = 0;

    static std::unique_ptr<BeepInterface> build(std::string_view backend);
};
class WindowsAPIBeep : public BeepInterface {
public:
    void beep(const Note& note) override
    {
        Beep(static_cast<DWORD>(std::round(note.frequency)), note.duration.count());
    }
};

std::unique_ptr<BeepInterface> BeepInterface::build(std::string_view backend)
{
    if (backend == "windowsapi") {
        return std::make_unique<WindowsAPIBeep>();
    } else {
        throw std::runtime_error("Unsupported backend: " + std::string(backend));
    }
}

/**
 * @brief Convert a note name to its corresponding frequency in Hz based on the 12-tone equal temperament (12tet) scale.
 *
 * @param note The note name in the format "C4" or "D#3".
 * @param A4Pitch The pitch of the A note in 4th octave (A4) in Hz.
 * @return The frequency of the note in Hz.
 */
double noteToFreq_12tet(std::string note, double A4Pitch = 440.0)
{
    static const std::regex noteRegex("([A-G]#?)([0-9]+)");
    static const std::regex noteRegexB("([A-G]b)([0-9]+)");
    static const std::unordered_map<std::string, std::string> mapNoteBToNoteC { { "Db", "C#" }, { "Eb", "D#" }, { "Gb", "F#" }, { "Ab", "G#" }, { "Bb", "A#" } };
    static const std::array<std::string, 12> arrNotes { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    std::transform(note.begin(), note.end(), note.begin(), toupper);
    std::smatch match;
    std::string noteName;
    int64_t octave = 0;
    if (std::regex_match(note, match, noteRegex))
        noteName = match.str(1);
    else if (std::regex_match(note, match, noteRegexB))
        noteName = mapNoteBToNoteC.at(match.str(1));
    else
        throw std::runtime_error("Invalid note format: What is " + note + '?');
    octave = std::stoll(match.str(2));

    const auto* noteIndex = std::find(arrNotes.begin(), arrNotes.end(), noteName);
    if (noteIndex == arrNotes.end())
        throw std::runtime_error("Invalid note name: There's no such a note as " + noteName + '!');

    int64_t offsetFromA4 = std::distance(arrNotes.begin(), noteIndex) - 9;
    offsetFromA4 += (octave - 4LL) * 12;

    if (offsetFromA4 > 39)
        throw std::runtime_error(std::string("Note is too high: ") += note + ". Max is C8.");

    return A4Pitch * std::pow(2.0, static_cast<double>(offsetFromA4) / 12.0);
}
int main(int argc, char** argv)
{
    CLI::App app("CLI program to play Beep sound.");

    std::unordered_map<CLI::App*, std::function<void(const std::unique_ptr<BeepInterface>&)>> subCommandCallbacks;

    std::string backend = "windowsapi";

    app.add_option("-b,--backend", backend, "Backend to use for beep sound.");
    app.set_help_flag("-h,--help", "Show this help message and exit. You can also use this on subcommands.");

    {
        CLI::App* appF = app.add_subcommand("f", "Play a beep sound with the specified frequency and duration.");

        double freq = 0;
        int64_t dur = 500;

        appF->add_option("frequency", freq, "Frequency of the beep sound in Hz.")->required();
        appF->add_option("duration", dur, "Duration of the beep sound in milliseconds. Default is 500 ms.");

        subCommandCallbacks[appF] = [&](const std::unique_ptr<BeepInterface>& beep) {
            beep->beep({ freq, std::chrono::milliseconds(dur) });
        };
    }
    {
        CLI::App* appS = app.add_subcommand("s", "Play more beep sounds using a music score.");

        std::string score;
        double A4Pitch = 440.0;

        appS->add_option("notes", score, R"(List of notes to play. 
Format: '<note_name>[,duration][;note_name[,duration]...]'
note_name: Note name in the format:' <A-G>[#|b]<octave>' e.g. C4, D#3, Gb2. Can also be: 'break', '-', which pause the sound for the specified duration.
duration: Duration of the beep sound in milliseconds. INT64 value, default: 500 (ms).

Example: C4;E4;G4;C5,1000)")
            ->required();
        appS->add_option("-a,--A4Pitch", A4Pitch, "Pitch of the A note in 4th octave (A4) in Hz. This pitch is used as the standard pitch for calculating note pitches. Default is 440.0 Hz.");

        subCommandCallbacks[appS] = [&](const std::unique_ptr<BeepInterface>& beep) {
            auto scoreSplit = CLI::detail::split(score, ';');
            for (const auto& fullNote : scoreSplit) {
                auto fullNoteSplit = CLI::detail::split(fullNote, ',');
                if (fullNoteSplit.size() > 2) {
                    throw std::runtime_error(std::format("Invalid format for notes: {}.", fullNote));
                } else if (fullNoteSplit[0].empty())
                    continue;

                int64_t dur = fullNoteSplit.size() == 2 ? std::stoll(fullNoteSplit[1]) : 500;
                if (fullNoteSplit[0] == "break" || fullNoteSplit[0] == "-") {
                    std::this_thread::sleep_for(std::chrono::milliseconds(dur));
                    continue;
                }
                double freq = noteToFreq_12tet(fullNoteSplit[0], A4Pitch);

                beep->beep({ freq, std::chrono::milliseconds(dur) });
            }
        };
    }
    {
        CLI::App* appB = app.add_subcommand("b", "Wait for a specified duration. (break) ");

        int64_t dur = 500;

        appB->add_option("duration", dur, "Duration to wait in milliseconds. Default is 500 ms.");

        subCommandCallbacks[appB] = [&](const std::unique_ptr<BeepInterface>&) {
            std::this_thread::sleep_for(std::chrono::milliseconds(dur));
        };
    }

    app.require_subcommand(0, 1);

    try {
        app.parse(argc, argv);
        auto beep = BeepInterface::build(backend);

        for (auto& [subCmd, callback] : subCommandCallbacks) {
            if (subCmd->parsed()) {
                callback(beep);
                return 0;
            }
        }

        throw std::logic_error("No subcommands provided. Use -h or --help for usage information.");
    } catch (const CLI::CallForHelp&) {
        for (const auto& subcmd : app.get_subcommands()) {
            if (subcmd->parsed()) {
                std::cout << subcmd->help() << '\n';
                return 0;
            }
        }
        std::cout << app.help() << '\n';
        return 0;
    } catch (const CLI::ParseError& e) {
        std::cout << argv[0] << ": error: " << e.get_name() << ": " << e.what() << std::endl;
        return 2;
    } catch (const std::exception& e) {
        std::cout << argv[0] << ": error: " << e.what() << std::endl;
        return 2;
    }
}