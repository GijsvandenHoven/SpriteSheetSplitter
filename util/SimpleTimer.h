#include <utility>

#ifndef SPRITESHEETSPLITTER_SIMPLETIMER_H
#define SPRITESHEETSPLITTER_SIMPLETIMER_H

struct SimpleTimer {
    SimpleTimer() = delete;

    [[maybe_unused]] explicit SimpleTimer(std::string s) : name(std::move(s)), p(std::chrono::high_resolution_clock::now()), outStream(std::cout) {}
    SimpleTimer(std::string s, std::basic_ostream<char>& os) : name(std::move(s)), p(std::chrono::high_resolution_clock::now()), outStream(os) {}

    ~SimpleTimer() {
        auto duration = std::chrono::high_resolution_clock::now() - p;
        outStream << "[INFO] " << name << " took: " << std::chrono::duration_cast<std::chrono::duration<double>>(duration).count() << " seconds.\n";
    }

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point p;
    std::basic_ostream<char>& outStream;
};

#endif //SPRITESHEETSPLITTER_SIMPLETIMER_H
