// beep

// Copyright (c) 2025 prbegd
// Distribution under the MIT License. See the accompanying file LICENSE or copy at https://opensource.org/licenses/MIT

#include "CLI/CLI11.hpp"
#include <exception>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

class BeepInterface {
public:
    virtual ~BeepInterface() = default;
    virtual void beep(unsigned freq, unsigned dur) = 0;

    static std::unique_ptr<BeepInterface> build(std::string_view backend);
};
class WindowsAPIBeep : public BeepInterface {
public:
    void beep(unsigned freq, unsigned dur) override
    {
        Beep(freq, dur);
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
 * @return unsigned The frequency of the note in Hz.
 */
unsigned noteToFreq_12tet(const std::string& note, double A4Pitch = 440.0)
{
    static const std::regex noteRegex("([A-G]#?)([0-9]+)");
    static const std::regex noteRegexB("([A-G]b)([0-9]+)");
    static const std::unordered_map<std::string, std::string> mapNoteBToNoteC { { "Db", "C#" }, { "Eb", "D#" }, { "Gb", "F#" }, { "Ab", "G#" }, { "Bb", "A#" } };
    static const std::array<std::string, 12> arrNotes { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    std::smatch match;
    std::string noteName;
    int octave = 0;
    if (std::regex_match(note, match, noteRegex))
        noteName = match.str(1);
    else if (std::regex_match(note, match, noteRegexB))
        noteName = mapNoteBToNoteC.at(match.str(1));
    else
        throw std::runtime_error("Invalid note format: What is " + note + '?');
    octave = std::stoi(match.str(2));

    const auto *noteIndex = std::find(arrNotes.begin(), arrNotes.end(), noteName);
    if (noteIndex == arrNotes.end()) 
        throw std::runtime_error("Invalid note name: There's no such a note as " + noteName + '!');
    
    int64_t offsetFromA4 = std::distance(arrNotes.begin(), noteIndex) - 9;
    offsetFromA4 += static_cast<int64_t>((octave - 4) * 12);

    if (offsetFromA4 > 39) 
        throw std::runtime_error(std::string("Note is too high: ") += note + ". Max is C8.");

    return static_cast<unsigned>(std::round(A4Pitch * std::pow(2.0, static_cast<double>(offsetFromA4) / 12.0)));
}
int main(int argc, char** argv)
{
    CLI::App app("CLI program to play Beep sound.");

    std::unordered_map<CLI::App*, std::function<void(const std::unique_ptr<BeepInterface>&)>> subCommandCallbacks;

    std::string backend = "windowsapi";
    unsigned freq = 0;
    unsigned dur = 500;

    std::string noteName;
    double A4Pitch = 440.0;

    app.add_option("-b,--backend", backend, "Backend to use for beep sound.");
    app.set_help_flag("-h,--help", "Show this help message and exit. You can also use this on subcommands.");

    {
        CLI::App* appF = app.add_subcommand("f", "Play a beep sound with the specified frequency and duration.");

        appF->add_option("frequency", freq, "Frequency of the beep sound in Hz.")->required();
        appF->add_option("duration", dur, "Duration of the beep sound in milliseconds. Default is 500 ms.");

        subCommandCallbacks[appF] = [&](const std::unique_ptr<BeepInterface>& beep) {
            beep->beep(freq, dur);
        };
    }
    {
        CLI::App* appN = app.add_subcommand("n", "Play a beep sound with the specified note and duration.");

        appN->add_option("note", noteName, "Note to play. e.g. C4")->required();
        appN->add_option("duration", dur, "Duration of the beep sound in milliseconds. Default is 500 ms.");
        appN->add_option("-a,--A4Pitch", A4Pitch, "Pitch of the A note in 4th octave (A4) in Hz. This pitch is used as the standard pitch for calculating note pitches. Default is 440.0 Hz.");

        subCommandCallbacks[appN] = [&](const std::unique_ptr<BeepInterface>& beep) {
            freq = noteToFreq_12tet(noteName, A4Pitch);
            beep->beep(freq, dur);
        };
    }
    {
        CLI::App* appB = app.add_subcommand("b", "Wait for a specified duration. (break) ");

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