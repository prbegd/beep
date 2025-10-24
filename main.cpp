#include "CLI/CLI11.hpp"
#include <exception>
#include <iostream>
#include <regex>
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

unsigned convertNoteToFreq(const std::string& note)
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
        throw std::runtime_error("Invalid note format: " + note);
    octave = std::stoi(match.str(2));

    int offsetFromA4 = std::distance(arrNotes.begin(), std::find(arrNotes.begin(), arrNotes.end(), noteName)) - 9;
    offsetFromA4 += (octave - 4) * 12;

    return static_cast<unsigned>(std::round(440.0 * std::pow(2.0, offsetFromA4 / 12.0)));
}
int main(int argc, char** argv)
{
    CLI::App app("CLI program to play Beep sound.");

    std::string backend = "windowsapi";
    unsigned freq = 440;
    unsigned dur = 500;
    std::string noteName;

    app.add_option("-b,--backend", backend, "Backend to use for beep sound.")->check(CLI::IsMember({ "windowsapi" }));

    {
        CLI::App* appF = app.add_subcommand("f", "Play a beep sound with the specified frequency and duration.");

        appF->add_option("frequency", freq, "Frequency of the beep sound in Hz. Default is 440 Hz.");
        appF->add_option("duration", dur, "Duration of the beep sound in milliseconds. Default is 500 ms.");
    }
    {
        CLI::App* appN = app.add_subcommand("n", "Play a beep sound with the specified note and duration.");

        appN->add_option("note", noteName, "Note to play. Required. Example: C4")->required();
        appN->add_option("duration", dur, "Duration of the beep sound in milliseconds. Default is 500 ms.");
    }
    {
        CLI::App* appB = app.add_subcommand("b", "Wait for a specified duration. (break) ");

        appB->add_option("duration", dur, "Duration to wait in milliseconds. Default is 500 ms.");
    }

    app.require_subcommand(1, 1);

    try {
        app.parse(argc, argv);
        auto beep = BeepInterface::build(backend);

        if (app.get_subcommand("f")->parsed()) {
            beep->beep(freq, dur);
        } else if (app.get_subcommand("n")->parsed()) {
            freq = convertNoteToFreq(noteName);
            beep->beep(freq, dur);
        } else if (app.get_subcommand("b")->parsed()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(dur));
        }
    } catch (const CLI::ParseError& e) {
        std::cout << argv[0] << ": error: " << e.get_name() << ": " << e.what() << std::endl;
        return 2;
    } catch (const std::exception& e) {
        std::cout << argv[0] << ": error: " << e.what() << std::endl;
        return 2;
    }
}