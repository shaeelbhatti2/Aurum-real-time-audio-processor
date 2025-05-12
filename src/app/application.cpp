#include <aurum/app/application.hpp>
#include <aurum/version.hpp>

#include <iostream>

namespace aurum {

int Application::run() {
    std::cout << kName << " v" << kVersion << '\n';
    return 0;
}

}  // namespace aurum
